// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/major.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/sysfs.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/file.h>
#include <linux/of.h>
#include <uapi/linux/sched/types.h>
#include <linux/amlogic/media/registers/cpu_version.h>
#include <linux/module.h>
#include <linux/kfifo.h>
#include <linux/compat.h>
#include <linux/amlogic/media/vfm/vframe.h>
#ifdef CONFIG_AMLOGIC_MEDIA_FRC
#include <linux/amlogic/media/frc/frc_common.h>
#endif
#ifdef CONFIG_AMLOGIC_DPSS_PROCESS
#include <linux/amlogic/media/dpss/dpss_frc.h>
#endif
#include <linux/amlogic/media/video_processor/video_pipe_adapter.h>

#define DEVICE_NAME   "video_pipe_adapter"
static u32 print_flag;

static ssize_t print_flag_show(const struct class *class,
		const struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "current print_flag is %d.\n", print_flag);
}

static ssize_t print_flag_store(const struct class *class,
		const struct class_attribute *attr, const char *buf,
		size_t count)
{
	int val;
	ssize_t ret;

	ret = kstrtoint(buf, 0, &val);
	if (ret < 0)
		return -EINVAL;

	if (val > 0)
		print_flag = val;
	else
		print_flag = 0;

	pr_info("set print_flag to %d.\n", print_flag);
	return count;
}

static CLASS_ATTR_RW(print_flag);

static struct attribute *video_pipe_adapter_class_attrs[] = {
	&class_attr_print_flag.attr,
	NULL
};
ATTRIBUTE_GROUPS(video_pipe_adapter_class);

static struct class video_pipe_adapter_class = {
	.name = DEVICE_NAME,
	.class_groups = video_pipe_adapter_class_groups,
};

static const struct of_device_id video_pipe_adapter_dt_match[] = {
	{.compatible = "amlogic, video_pipe_adapter",
	},
	{},
};

static int video_pipe_adapter_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int video_pipe_adapter_release(struct inode *inode, struct file *file)
{
	return 0;
}

static long video_pipe_adapter_ioctl(struct file *file, unsigned int cmd, ulong arg)
{
	long ret = 0;
	void __user *argp = (void __user *)arg;
	struct video_delay_arg_t video_delay_arg;

	switch (cmd) {
	case VIDEO_PIPE_ADAPTER_GET_VIDEO_DELAY:
		memset(&video_delay_arg, 0, sizeof(struct video_delay_arg_t));
		if (copy_from_user(&video_delay_arg, argp, sizeof(struct video_delay_arg_t)))
			return -EINVAL;
#ifdef CONFIG_AMLOGIC_DPSS_PROCESS
		video_delay_arg.video_delay =
			dpss_frc_get_video_latency_new(video_delay_arg.input_fps,
						video_delay_arg.output_fps);
#endif
		if (copy_to_user(argp, &video_delay_arg, sizeof(struct video_delay_arg_t)))
			ret = -EFAULT;
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

#ifdef CONFIG_COMPAT
static long video_pipe_adapter_compat_ioctl(struct file *file, unsigned int cmd, ulong arg)
{
	long ret = 0;

	ret = video_pipe_adapter_ioctl(file, cmd, (ulong)compat_ptr(arg));
	return ret;
}
#endif

static const struct file_operations video_pipe_adapter_fops = {
	.owner = THIS_MODULE,
	.open = video_pipe_adapter_open,
	.release = video_pipe_adapter_release,
	.unlocked_ioctl = video_pipe_adapter_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = video_pipe_adapter_compat_ioctl,
#endif
	.poll = NULL,
};

static int video_pipe_adapter_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev;

	ret = class_register(&video_pipe_adapter_class);
	if (ret < 0) {
		pr_err("video_pipe_adapter class_register fail\n");
		return ret;
	}
	ret = register_chrdev(VIDEO_PIPE_ADAPTER_MAJOR, DEVICE_NAME, &video_pipe_adapter_fops);
	if (ret < 0) {
		ret = 1;
		goto error;
	}

	dev = device_create(&video_pipe_adapter_class,
			NULL,
			MKDEV(VIDEO_PIPE_ADAPTER_MAJOR, 0),
			NULL,
			DEVICE_NAME);

	if (IS_ERR_OR_NULL(dev)) {
		ret = 2;
		goto error;
	}

	return 0;
error:
	pr_err("video_pipe_adapter probe error(%d)\n", ret);
	unregister_chrdev(VIDEO_PIPE_ADAPTER_MAJOR, DEVICE_NAME);
	class_unregister(&video_pipe_adapter_class);
	return -1;
}

static void video_pipe_adapter_remove(struct platform_device *pdev)
{
	device_destroy(&video_pipe_adapter_class, MKDEV(VIDEO_PIPE_ADAPTER_MAJOR, 0));
	unregister_chrdev(VIDEO_PIPE_ADAPTER_MAJOR, DEVICE_NAME);
	class_destroy(&video_pipe_adapter_class);
}

static struct platform_driver video_pipe_adapter_driver = {
	.probe = video_pipe_adapter_probe,
	.remove = video_pipe_adapter_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = DEVICE_NAME,
		.of_match_table = video_pipe_adapter_dt_match,
	}
};

int __init video_pipe_adapter_module_init(void)
{
	return platform_driver_register(&video_pipe_adapter_driver);
}

void __exit video_pipe_adapter_module_exit(void)
{
	platform_driver_unregister(&video_pipe_adapter_driver);
}
