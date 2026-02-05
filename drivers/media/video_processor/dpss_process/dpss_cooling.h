/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPSS_COOLING_H__
#define __DPSS_COOLING_H__

#include <linux/thermal.h>

struct dpss_cooling_device;
/*setstep: step last set
 *maxstep: the max hotstep which can be setted by the cooling device
 */
struct dpss_cooling_device {
	struct thermal_cooling_device *cool_dev;
	int (*set_dpss_cooling_state)(int state);
	u32 setstep;
	int maxstep;
};

#ifdef CONFIG_AMLOGIC_DPSS_THERMAL
struct thermal_cooling_device *dpss_cooling_register(struct dpss_cooling_device *dpss_cdev,
						     struct device_node *np);
void dpss_cooling_unregister(struct thermal_cooling_device *cdev);
#else
static inline struct thermal_cooling_device *
dpss_cooling_register(struct dpss_cooling_device *dpss_cdev, struct device_node *np)
{
	return NULL;
}

static inline
void dpss_cooling_unregister(struct thermal_cooling_device *cdev)
{
}
#endif
#endif /* __DPSS_COOLING_H__ */
