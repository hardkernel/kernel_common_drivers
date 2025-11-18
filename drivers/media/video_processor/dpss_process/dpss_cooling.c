// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/thermal.h>
#include <linux/err.h>
#include <linux/slab.h>
#include "dpss_cooling.h"
#include <linux/amlogic/meson_cooldev.h>
#include <linux/io.h>
#include "thermal_core.h"

static int dpss_get_max_state(struct thermal_cooling_device *cdev,
			      unsigned long *max)
{
	struct dpss_cooling_device *dpss_cdev = cdev->devdata;

	*max = dpss_cdev->maxstep ? dpss_cdev->maxstep : 0;

	return 0;
}

static int dpss_get_cur_state(struct thermal_cooling_device *cdev,
			      unsigned long *state)
{
	struct dpss_cooling_device *dpss_cdev = cdev->devdata;

	*state = dpss_cdev->setstep;

	return 0;
}

static int dpss_set_cur_state(struct thermal_cooling_device *cdev,
			      unsigned long state)
{
	struct dpss_cooling_device *dpss_cdev = cdev->devdata;

	dpss_cdev->setstep = state;
	dpss_cdev->set_dpss_cooling_state(state);

	return 0;
}

static const struct thermal_cooling_device_ops dpss_cooling_ops = {
	.get_max_state = dpss_get_max_state,
	.get_cur_state = dpss_get_cur_state,
	.set_cur_state = dpss_set_cur_state,
};

struct thermal_cooling_device *
dpss_cooling_register(struct dpss_cooling_device *dpss_cdev, struct device_node *np)
{
	struct thermal_cooling_device *cool_dev;

	dpss_cdev->setstep = 0;

	cool_dev = thermal_of_cooling_device_register(np, "thermal-dpss", dpss_cdev,
						      &dpss_cooling_ops);
	if (!cool_dev || !dpss_cdev->set_dpss_cooling_state)
		return NULL;

	dpss_cdev->cool_dev = cool_dev;

	return cool_dev;
}

void dpss_cooling_unregister(struct thermal_cooling_device *cdev)
{
	thermal_cooling_device_unregister(cdev);
}
