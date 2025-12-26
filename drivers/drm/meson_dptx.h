/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_DPTX_H__
#define __MESON_DPTX_H__

#include <drm/drm_connector.h>
#include <drm/drm_encoder.h>

#include <drm/amlogic/meson_connector_dev.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>

#include "meson_drv.h"
#include "meson_tx_helper.h"

struct meson_dptx_connector_state {
	struct drm_connector_state base;
	struct tx_color_attr color_attr_para;
	struct meson_tx_state base_state;
};

#define to_dptx_connector_state(x)	container_of(x, struct meson_dptx_connector_state, base)

#define encoder_to_meson_dptx(x)	container_of(x, struct meson_dp_tx, encoder)
#define connector_to_meson_dptx(x)  \
	container_of(connector_to_meson_connector(x), struct meson_dp_tx, base)

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

#endif

