/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_DPTX_H__
#define __MESON_DPTX_H__

#include <drm/drm_connector.h>
#include <drm/drm_encoder.h>

#include <drm/amlogic/meson_connector_dev.h>

#include "meson_drv.h"
#include "meson_tx_helper.h"

struct meson_dp_tx {
	struct meson_connector base;
	struct drm_encoder encoder;
	struct meson_connector_dev *dptx_dev;
	u32 enc_idx;

	struct meson_tx_venc *venc;
	struct platform_device *venc_pdev;

	/*
	 * Whether the current edid is valid
	 * 0:edid is invalid
	 * 1:edid is valid
	 */
	struct drm_property *edid_valid_prop;

	struct drm_property *color_space_prop;
	struct drm_property *color_depth_prop;
};

int meson_dptx_dev_bind(struct drm_device *drm,
	int type, struct meson_connector_dev *intf);
int meson_dptx_dev_unbind(struct drm_device *drm,
	int type, struct meson_connector_dev *intf);

#endif

