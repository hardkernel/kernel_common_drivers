/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __VIR_TSENS_H__
#define __VIR_TSENS_H__

#include <linux/thermal.h>
#include <linux/device.h>

struct vir_tsens {
	struct device *dev;
	u32 num_vtsens;
	struct thermal_zone_device **tzd_array;
	struct thermal_zone_device *soc_tzd;
};

extern struct platform_driver vir_tsens_platdrv;

#endif
