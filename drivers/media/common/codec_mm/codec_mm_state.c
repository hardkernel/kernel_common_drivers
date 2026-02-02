// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include <linux/amlogic/media/codec_mm/codec_mm_state.h>

#define WRITE_BUFSIZE	(4096)

struct codec_state_mgr {
	struct list_head	cs_head;
	struct mutex		cs_lock; /* Used for cs list locked. */
	struct class *cs_class;
	struct device *parent_device;
	struct cs_data *data[MAX_CODEC_STATE_DEVICES];
	const char *cs_devs_names[MAX_CODEC_STATE_DEVICES];
};

static struct codec_state_mgr *get_cs_mgr(void)
{
	static struct codec_state_mgr mgr = {
		.cs_devs_names = {
			"dump",
			"config"
		}
	};
	static bool inited;

	if (!inited) {
		INIT_LIST_HEAD(&mgr.cs_head);
		mutex_init(&mgr.cs_lock);

		inited = true;
	}

	return &mgr;
};

void *cs_seq_start(struct seq_file *m, loff_t *pos)
{
	struct codec_state_mgr *mgr = get_cs_mgr();

	mutex_lock(&mgr->cs_lock);
	return seq_list_start(&mgr->cs_head, *pos);
}

void *cs_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	struct codec_state_mgr *mgr = get_cs_mgr();

	return seq_list_next(v, &mgr->cs_head, pos);
}

void cs_seq_stop(struct seq_file *m, void *v)
{
	struct codec_state_mgr *mgr = get_cs_mgr();

	mutex_unlock(&mgr->cs_lock);
}

static int cs_seq_show(struct seq_file *m, void *v)
{
	struct codec_state_node *cs = v;

	if (cs->on && cs->ops->show)
		return cs->ops->show(m, cs);

	return 0;
}

static const struct seq_operations seq_op = {
	.start	= cs_seq_start,
	.next	= cs_seq_next,
	.stop	= cs_seq_stop,
	.show	= cs_seq_show
};

static int cs_seq_open(struct inode *inode, struct file *file)
{
	/* file will be reinit in seq_open, clear misc file private */
	file->private_data = NULL;
	return seq_open(file, &seq_op);
}

static int cs_seq_list_show(struct seq_file *m, void *v)
{
	struct codec_state_node *cs = v;

	seq_printf(m, "[ %s ] %s\n", cs->on ? "on" : "off", cs->ops->name);

	return 0;
}

static const struct seq_operations seq_list_op = {
	.start	= cs_seq_start,
	.next	= cs_seq_next,
	.stop	= cs_seq_stop,
	.show	= cs_seq_list_show
};

static int cs_seq_list_open(struct inode *inode, struct file *file)
{
	/* file will be reinit in seq_open, clear misc file private */
	file->private_data = NULL;
	return seq_open(file, &seq_list_op);
}

static void do_cs_switch_only(struct codec_state_node *node)
{
	struct codec_state_mgr *mgr = get_cs_mgr();
	struct codec_state_node *cs;

	list_for_each_entry(cs, &mgr->cs_head, list) {
		cs->on = (cs == node) ? true : false;
	}
}

static int do_store(int argc, char **argv)
{
	struct codec_state_mgr *mgr = get_cs_mgr();
	struct codec_state_node *cs;
	char *module = argv[0];
	char **cmd = argv + 1;
	int clen = argc - 1;
	int ret = 0;

	mutex_lock(&mgr->cs_lock);

	list_for_each_entry(cs, &mgr->cs_head, list) {
		if (strcmp(cs->ops->name, module) &&
			strcmp("all", module))
			continue;

		if (!strcmp("off", cmd[0])) {
			cs->on = false;
		} else if (!strcmp("on", cmd[0])) {
			cs->on = true;
		} else if (!strcmp("only", cmd[0])) {
			do_cs_switch_only(cs);
			break;
		} else if (cs->on) {
			if (cs->ops->store)
				ret = cs->ops->store(clen, (const char **)cmd);
		}
	}

	mutex_unlock(&mgr->cs_lock);

	return ret;
}

static int do_run_cmd(const char *buf, int (*func)(int, char **))
{
	char **argv;
	int argc = 0, ret = 0;

	argv = argv_split(GFP_KERNEL, buf, &argc);
	if (!argv)
		return -ENOMEM;

	if (argc)
		ret = func(argc, argv);

	argv_free(argv);

	return ret;
}

static ssize_t cs_parse_cmd(struct file *file, const char __user *buffer,
				size_t count, loff_t *ppos,
				int (*func)(int, char **))
{
	char *kbuf, *buf, *tmp;
	int ret = 0;
	size_t done = 0;
	size_t size;

	kbuf = kmalloc(WRITE_BUFSIZE, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;

	while (done < count) {
		size = count - done;

		if (size >= WRITE_BUFSIZE)
			size = WRITE_BUFSIZE - 1;

		if (copy_from_user(kbuf, buffer + done, size)) {
			ret = -EFAULT;
			goto out;
		}

		kbuf[size] = '\0';
		buf = kbuf;

		do {
			tmp = strchr(buf, '\n');
			if (tmp) {
				*tmp = '\0';
				size = tmp - buf + 1;
			} else {
				size = strlen(buf);
				if (done + size < count) {
					if (buf != kbuf)
						break;
					/* This can accept WRITE_BUFSIZE - 2 ('\n' + '\0') */
					pr_warn("Line length is too long: Should be less than %d\n",
						WRITE_BUFSIZE - 2);
					ret = -EINVAL;
					goto out;
				}
			}
			done += size;

			/* Remove comments */
			tmp = strchr(buf, '#');

			if (tmp)
				*tmp = '\0';

			ret = do_run_cmd(buf, func);
			if (ret)
				goto out;
			buf += size;

		} while (done < count);
	}
	ret = done;

out:
	kfree(kbuf);

	return ret;
}

static ssize_t cs_seq_write(struct file *file, const char __user *buffer,
				size_t count, loff_t *ppos)
{
	return cs_parse_cmd(file, buffer, count, ppos, do_store);
}

void cs_seq_list_read(struct seq_file *m, void *v)
{
	struct codec_state_mgr *mgr = get_cs_mgr();

	mutex_unlock(&mgr->cs_lock);
}

int codec_state_register(struct codec_state_node *cs, struct codec_state_ops *ops)
{
	struct codec_state_mgr *mgr = get_cs_mgr();

	if (!cs || !ops)
		return -EINVAL;

	mutex_lock(&mgr->cs_lock);

	INIT_LIST_HEAD(&cs->list);
	cs->ops = ops;
	cs->on = true;

	list_add_tail(&cs->list, &mgr->cs_head);

	mutex_unlock(&mgr->cs_lock);

	return 0;
}

void codec_state_unregister(struct codec_state_node *cs)
{
	struct codec_state_mgr *mgr = get_cs_mgr();

	mutex_lock(&mgr->cs_lock);

	list_del_init(&cs->list);

	mutex_unlock(&mgr->cs_lock);
}

static const struct file_operations cs_fops[MAX_CODEC_STATE_DEVICES] = {
	/* dump */
	{
		.owner		= THIS_MODULE,
		.open		= cs_seq_open,
		.read		= seq_read,
		.llseek		= seq_lseek,
		.release	= seq_release,
		.write		= cs_seq_write
	},
	/* config */
	{
		.open		= cs_seq_list_open,
		.write		= cs_seq_write,
		.read		= seq_read,
		.llseek		= seq_lseek
	}
};

int codec_state_init(void)
{
	int i, ret;
	struct codec_state_mgr *mgr = get_cs_mgr();

	mgr->cs_class = class_create(DEVICE_NAME);
	if (IS_ERR(mgr->cs_class)) {
		pr_debug("Failed to create device class\n");
		return PTR_ERR(mgr->cs_class);
	}

	mgr->parent_device = device_create(mgr->cs_class, NULL, 0, NULL, DEVICE_NAME);
	if (IS_ERR(mgr->parent_device)) {
		pr_debug("Failed to create parent device\n");
		ret = PTR_ERR(mgr->parent_device);
		goto err_parent;
	}

	for (i = 0; i < MAX_CODEC_STATE_DEVICES; i++) {
		mgr->data[i] = kzalloc(sizeof(*mgr->data[i]), GFP_KERNEL);
		if (!mgr->data[i]) {
			ret = -ENOMEM;
			goto err_devices;
		}

		snprintf(mgr->data[i]->buffer, sizeof(mgr->data[i]->buffer),
				"This is %s device.\n", mgr->cs_devs_names[i]);
		mgr->data[i]->buffer_size = strlen(mgr->data[i]->buffer);

		mgr->data[i]->misc_dev.minor = MISC_DYNAMIC_MINOR;
		mgr->data[i]->misc_dev.name = kasprintf(GFP_KERNEL, "%s_%s",
					DEVICE_NAME, mgr->cs_devs_names[i]);
		mgr->data[i]->misc_dev.fops = &cs_fops[i];
		mgr->data[i]->misc_dev.parent = mgr->parent_device;

		if (!mgr->data[i]->misc_dev.name) {
			ret = -ENOMEM;
			kfree(mgr->data[i]);
			goto err_devices;
		}

		ret = misc_register(&mgr->data[i]->misc_dev);
		if (ret < 0) {
			pr_err("Failed to register misc device %s\n", mgr->cs_devs_names[i]);
			kfree(mgr->data[i]->misc_dev.name);
			kfree(mgr->data[i]);
			goto err_devices;
		}

		pr_debug("Created device /dev/%s\n", mgr->data[i]->misc_dev.name);
	}

	pr_debug("Codec device driver initialized successfully\n");
	return 0;

err_devices:
	for (i--; i >= 0; i--) {
		if (mgr->data[i]) {
			misc_deregister(&mgr->data[i]->misc_dev);
			kfree(mgr->data[i]->misc_dev.name);
			kfree(mgr->data[i]);
		}
	}
	device_destroy(mgr->cs_class, 0);
err_parent:
	class_destroy(mgr->cs_class);
	return ret;
}

void codec_state_release(void)
{
	int i;
	struct codec_state_mgr *mgr = get_cs_mgr();

	for (i = 0; i < MAX_CODEC_STATE_DEVICES; i++) {
		if (mgr->data[i]) {
			misc_deregister(&mgr->data[i]->misc_dev);
			kfree(mgr->data[i]->misc_dev.name);
			kfree(mgr->data[i]);
		}
	}

	if (mgr->parent_device)
		device_destroy(mgr->cs_class, 0);

	if (mgr->cs_class)
		class_destroy(mgr->cs_class);
}

