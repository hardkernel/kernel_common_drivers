/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMITX_DUMP_H
#define __HDMITX_DUMP_H

#include <linux/module.h>
#include <linux/pwm.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/types.h>

#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>

#include "hdmitx_version.h"
#include "hdmitx_sysfs_common.h"

struct hdmitx_dbg_files_s {
	const char *name;
	const umode_t mode;
	const struct file_operations *fops;
	const struct proc_ops *pops;
};

/* common api */
int dump_hdmitx_basic_config(struct seq_file *s, void *p);
int hdmirx_info_show(struct seq_file *s, void *v);
void hdmitx_common_debugfs_init(struct hdmitx_common *tx_comm);
void hdmitx_common_profs_init(struct hdmitx_common *tx_comm);

/* hdmitx21 api */
struct hdmitx_dbg_files_s *hdmitx21_get_dbg_files_s(void);
int hdmitx21_get_dbg_files_count(void);

/* hdmitx20 api*/
struct hdmitx_dbg_files_s *hdmitx20_get_dbg_files_s(void);
int hdmitx20_get_dbg_files_count(void);

#endif
