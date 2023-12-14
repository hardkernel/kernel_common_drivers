/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _FRONTEND_H_
#define _FRONTEND_H_
#include <linux/platform_device.h>
#include <linux/amlogic/kernel_versions.h>

int frontend_probe(struct platform_device *pdev);
int frontend_remove(void);
void frontend_config_ts_sid(void);
int frontend_control_tsin_clk(int state);

ssize_t ts_setting_show(KV_CLASS_CONST struct class *class,
			KV_CLASS_ATTR_CONST struct class_attribute *attr,
			char *buf);
ssize_t ts_setting_store(KV_CLASS_CONST struct class *class,
			KV_CLASS_ATTR_CONST struct class_attribute *attr,
			const char *buf, size_t count);

#endif
