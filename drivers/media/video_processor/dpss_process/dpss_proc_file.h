/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _DI_PROC_FILE_H
#define _DI_PROC_FILE_H

#include <linux/amlogic/media/vfm/vframe.h>
#include <linux/amlogic/media/video_sink/v4lvideo_ext.h>
#include <linux/amlogic/media/di/dpss_interface.h>
#include <linux/amlogic/meson_uvm_core.h>

extern u32 dpss_buf_mgr_print_flag;

struct file_private_data *dpss_proc_get_file_private_data(struct file *file_vf, bool alloc_if_null);
void dpss_put_file_ext(int dev_index, struct file *file_vf);
void dpss_total_fill_count_increase(void);

#endif /* _DI_PROC_FILE_H */
