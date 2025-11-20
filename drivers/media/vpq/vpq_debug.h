/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __VPQ_DEBUG_H__
#define __VPQ_DEBUG_H__

#include <linux/cdev.h>

ssize_t vpq_debug_cmd_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf);
ssize_t vpq_debug_cmd_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count);

ssize_t vpq_log_lev_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf);
ssize_t vpq_log_lev_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count);

ssize_t vpq_chip_infor_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf);
ssize_t vpq_chip_infor_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count);

ssize_t vpq_reg_table_ver_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf);
ssize_t vpq_reg_table_ver_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count);

ssize_t vpq_pq_setting_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf);
ssize_t vpq_pq_setting_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count);

ssize_t vpq_pq_module_cfg_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf);
ssize_t vpq_pq_module_cfg_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count);

ssize_t vpq_pq_module_status_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf);
ssize_t vpq_pq_module_status_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count);

ssize_t vpq_src_infor_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf);
ssize_t vpq_src_infor_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count);

ssize_t vpq_other_infor_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf);
ssize_t vpq_other_infor_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count);

ssize_t vpq_hist_avg_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf);
ssize_t vpq_hist_avg_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count);

ssize_t vpq_dump_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf);
ssize_t vpq_dump_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count);

#endif // __VPQ_DEBUG_H__
