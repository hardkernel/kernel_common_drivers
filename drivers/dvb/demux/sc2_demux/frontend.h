/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _FRONTEND_H_
#define _FRONTEND_H_
#include <linux/platform_device.h>

int frontend_probe(struct platform_device *pdev);
int frontend_remove(void);
void frontend_config_ts_sid(void);
int frontend_control_tsin_clk(int state);
int frontend_debug(int direct, char *param_name, int *param_value);

ssize_t ts_setting_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf);
ssize_t ts_setting_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf, size_t count);

/*add alp/tlv interface*/
int alp_tlv_probe(struct platform_device *pdev);
ssize_t alp_tlv_show(const struct class *class,
			const struct class_attribute *attr, char *buf);
ssize_t alp_tlv_store(const struct class *class,
			 const struct class_attribute *attr,
			 const char *buf, size_t count);
#endif
