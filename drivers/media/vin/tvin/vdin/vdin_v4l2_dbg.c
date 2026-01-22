// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

/* Standard Linux Headers */
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/amlogic/media/codec_mm/codec_mm.h>

/* Local Headers */
/*#include "../tvin_format_table.h"*/
#include "vdin_drv.h"
#include "vdin_ctl.h"
/*#include "vdin_regs.h"*/
/*#include "vdin_afbce.h"*/

#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include "vdin_canvas.h"
#include "vdin_v4l2_dbg.h"
#include "vdin_v4l2_if.h"

static void vdin_v4l2_status(struct vdin_dev_s *devp)
{
	pr_info("vdin%d v4l_ver:0x%x, ver2:%s\n",
		devp->index, VDIN_DEV_VER, VDIN_DEV_VER2);
	pr_info("v4l_en:%d, mode:%d, type:%d, mem:%d(%s)\n",
		devp->dts_config.v4l_en, devp->work_mode,
		devp->vb_queue.type, devp->vb_queue.memory,
		vb2_memory_sts_to_str(devp->vb_queue.memory));
	pr_info("buf_num:%d, min_buf_num:%d, q_cnt:%d\n",
		devp->vb_queue.max_num_buffers, devp->vb_queue.min_queued_buffers,
		devp->vb_queue.queued_count);
	pr_info("streaming:%d\n", devp->vb_queue.streaming);
	pr_info("buf_struct_size:%d\n", devp->vb_queue.buf_struct_size);
	pr_info("secure:%d, drop_divide:%u\n",
		devp->vdin_v4l2.secure_flg, devp->vdin_v4l2.stats.drop_divide);

	/* vdin v4l2 stats */
	pr_info("done_cnt:%u, dq_cnt:%u, q_cnt:%u\n",
		devp->vdin_v4l2.stats.done_cnt, devp->vdin_v4l2.stats.dque_cnt,
		devp->vdin_v4l2.stats.que_cnt);
	pr_info("pause:%d, no_ioctl:%d, no_event:%d\n",
		devp->dbg_v4l_pause, devp->debug.ignore_vdin_ioctl,
		devp->dbg_v4l_no_vdin_event);
}

static ssize_t v4l_dbg_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	/*struct vdin_dev_s *devp = dev_get_drvdata(dev);*/
	ssize_t len = 0;

	len += sprintf(buf + len, "\n v4l debug interface");
	len += sprintf(buf + len, "\n5 start 0/1\n");
	len += sprintf(buf + len, "\n5 dbg_en 0/1 x\n");
	len += sprintf(buf + len, "\n5 status x\n");
	len += sprintf(buf + len, "\n5 pause 0/1 x\n");
	return len;
}

static ssize_t v4l_dbg_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t len)
{
	char *parm[47] = {NULL};
	char *buf_orig = kstrdup(buf, GFP_KERNEL);
	struct vdin_dev_s *devp = dev_get_drvdata(dev);
	long val = 0;
	int err = 0;
	/*unsigned int temp;*/
	/*unsigned int mode = 0, flag = 0;*/

	if (!buf)
		return len;
	vdin_parse_param(buf_orig, (char **)&parm);

	if (!strncmp(parm[0], "start", 5)) {
		if (kstrtol(parm[1], 10, &val) == 0) {
			devp->work_mode = val;
		} else {
			err = -3;
			goto error;
		}
	} else if (!strncmp(parm[0], "dbg_en", 5)) {
		if (kstrtol(parm[1], 16, &val) == 0) {
			vdin_v4l_debug = val;
		} else {
			err = -3;
			goto error;
		}
	} else if (!strncmp(parm[0], "status", 6)) {
		vdin_v4l2_status(devp);
	} else if (!strncmp(parm[0], "pause", 5)) {
		if (kstrtol(parm[1], 10, &val) == 0) {
			devp->dbg_v4l_pause = val;
		} else {
			err = -3;
			goto error;
		}
	} else if (!strcmp(parm[0], "no_vdin_ioctl")) {/* for v4l2 test app */
		if (kstrtol(parm[1], 0, &val) == 0) {
			devp->debug.ignore_vdin_ioctl = val;
		} else {
			err = -3;
			goto error;
		}
	} else if (!strcmp(parm[0], "no_vdin_event")) {/* for v4l2 test app */
		if (kstrtol(parm[1], 0, &val) == 0) {
			devp->dbg_v4l_no_vdin_event = val;
		} else {
			err = -3;
			goto error;
		}
	} else if (!strcmp(parm[0], "pixfmt")) {/* for v4l2 test app */
		if (kstrtol(parm[1], 0, &val) == 0)
			devp->v4l2_dbg_ctl.dbg_pix_fmt = val;
		pr_info("v4l fmt %#x (uyvy:%#x, nv21:%#x, nv12:%#x, nv21m:%#x, nv12m:%#x)\n",
			devp->v4l2_dbg_ctl.dbg_pix_fmt,
			V4L2_PIX_FMT_UYVY, V4L2_PIX_FMT_NV21, V4L2_PIX_FMT_NV12,
			V4L2_PIX_FMT_NV21M, V4L2_PIX_FMT_NV12M);

	} else {
		pr_info("vdin%d v4l2 unknown cmd\n", devp->index);
	}

	return len;
error:
	pr_err("vdin v4l2 set param error:%d", err);
	return len;
}
static DEVICE_ATTR_RW(v4l_dbg);

static ssize_t v4l_work_mode_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct vdin_dev_s *devp = dev_get_drvdata(dev);

	return sprintf(buf, "0x%x\n", devp->work_mode);
}

static ssize_t v4l_work_mode_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t len)
{
	struct vdin_dev_s *devp = dev_get_drvdata(dev);
	int cnt = 0, val = 0;

	cnt = kstrtoint(buf, 16, &val);
	if (cnt < 0)
		return -EINVAL;

	devp->work_mode = val;

	return len;
}
static DEVICE_ATTR_RW(v4l_work_mode);

int vdin_v4l2_create_device_files(struct device *dev)
{
	int ret = 0;

	ret |= device_create_file(dev, &dev_attr_v4l_dbg);
	ret |= device_create_file(dev, &dev_attr_v4l_work_mode);

	return ret;
}

void vdin_v4l2_remove_device_files(struct device *dev)
{
	device_remove_file(dev, &dev_attr_v4l_dbg);
	device_remove_file(dev, &dev_attr_v4l_work_mode);
}

