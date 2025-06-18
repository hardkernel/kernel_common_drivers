/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_eDP_H_
#define __MESON_eDP_H_

#include "meson_drv.h"
#include <drm/drm_encoder.h>
#include <drm/amlogic/meson_connector_dev.h>

struct meson_eDP {
	struct meson_connector base;
	struct drm_encoder encoder;
	struct drm_device *drm;

	struct meson_panel_dev *eDP_dev;
	struct drm_property *panel_type_prop;
	int panel_type;

	/* vrr related */
	int num_vrr_group;
	struct drm_vrr_mode_group groups[MAX_VRR_MODE_GROUP];
};

#define connector_to_meson_eDP(x) \
		container_of(connector_to_meson_connector(x), struct meson_eDP, base)
#define encoder_to_meson_eDP(x) container_of(x, struct meson_eDP, encoder)
#define drm_panel_to_meson_eDP(x) container_of(x, struct meson_eDP, eDP_dev)

int meson_eDP_dev_bind(struct drm_device *drm, int type, struct meson_connector_dev *intf);
int meson_eDP_dev_unbind(struct drm_device *drm, int type, struct meson_connector_dev *intf);

#endif
