// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <drm/drm_modeset_helper.h>
#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_atomic_helper.h>
#include <linux/component.h>
#include <vout/vout_serve/vout_func.h>
#include "meson_crtc.h"
#include "meson_dummy.h"
#include "meson_vpu_pipeline.h"

static struct drm_display_mode dummy_mode;
static struct drm_display_mode dummyp_mode = {
	.name = "dummy_panel",
	.status = MODE_OK,
	.clock = 148500,
	.hdisplay = 1920,
	.hsync_start = 2008,
	.hsync_end = 2052,
	.htotal = 2200,
	.hskew = 0,
	.vdisplay = 1080,
	.vsync_start = 1090,
	.vsync_end = 1095,
	.vtotal = 1125,
	.vscan = 0,
};

static struct drm_display_mode dummyl_mode = {
	.name = "dummy_l",
	.type = DRM_MODE_TYPE_USERDEF,
	.status = MODE_OK,
	.clock = 25000,
	.hdisplay = 720,
	.hsync_start = 736,
	.hsync_end = 798,
	.htotal = 858,
	.hskew = 0,
	.vdisplay = 480,
	.vsync_start = 489,
	.vsync_end = 495,
	.vtotal = 525,
	.vscan = 0,
	.flags =  DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC,
};

int meson_dummy_get_modes(struct drm_connector *connector)
{
	struct drm_display_mode *mode = NULL;
	int count = 0;
#ifdef CONFIG_AMLOGIC_VOUT2_SERVE
	struct vinfo_s *vinfo = NULL;
	u32 dummyp_timing_flip = 0;
	int tmp = 0;

	if (strstr(dummy_mode.name, "dummy_panel")) {
		vinfo = get_current_vinfo2();
		dummyp_timing_flip = get_dummyp_timing_flip();

		if (dummyp_timing_flip) {
			dummy_mode.clock  = vinfo->video_clk / 1000;

			dummy_mode.hdisplay  = vinfo->height;
			tmp = vinfo->vtotal - vinfo->height - vinfo->vbp;
			dummy_mode.hsync_start  = vinfo->height + tmp - vinfo->vsw;
			dummy_mode.hsync_end = vinfo->height + tmp;
			dummy_mode.htotal  = vinfo->vtotal;

			dummy_mode.vdisplay = vinfo->width;
			tmp = vinfo->htotal - vinfo->width - vinfo->hbp;
			dummy_mode.vsync_start  =  vinfo->width + tmp - vinfo->hsw;
			dummy_mode.vsync_end = vinfo->width + tmp;
			dummy_mode.vtotal = vinfo->htotal;
		} else {
			dummy_mode.clock = vinfo->video_clk / 1000;

			dummy_mode.hdisplay = vinfo->width;
			tmp = vinfo->htotal - vinfo->width - vinfo->hbp;
			dummy_mode.hsync_start = vinfo->width + tmp - vinfo->hsw;
			dummy_mode.hsync_end = vinfo->width + tmp;
			dummy_mode.htotal = vinfo->htotal;

			dummy_mode.vdisplay = vinfo->height;
			tmp = vinfo->vtotal - vinfo->height - vinfo->vbp;
			dummy_mode.vsync_start = vinfo->height + tmp - vinfo->vsw;
			dummy_mode.vsync_end = vinfo->height + tmp;
			dummy_mode.vtotal = vinfo->vtotal;
		}
	}
#endif

	mode = drm_mode_duplicate(connector->dev, &dummy_mode);
	if (!mode) {
		DRM_INFO("[%s:%d]dup dummy mode failed.\n", __func__,
			 __LINE__);
	} else {
		drm_mode_probed_add(connector, mode);
		count++;
	}

	return count;
}

enum drm_mode_status meson_dummy_check_mode(struct drm_connector *connector,
	struct drm_display_mode *mode)
{
	return MODE_OK;
}

static struct drm_encoder *meson_dummy_best_encoder
	(struct drm_connector *connector)
{
	struct meson_dummy *am_dummy = connector_to_meson_dummy(connector);

	return &am_dummy->encoder;
}

static const struct drm_connector_helper_funcs meson_dummy_helper_funcs = {
	.get_modes = meson_dummy_get_modes,
	.mode_valid = meson_dummy_check_mode,
	.best_encoder = meson_dummy_best_encoder,
};

static enum drm_connector_status
meson_dummy_detect(struct drm_connector *connector, bool force)
{
	return connector_status_connected;
}

static void am_dummy_connector_destroy(struct drm_connector *connector)
{
	DRM_DEBUG("[%d] called\n", __LINE__);

	drm_connector_unregister(connector);
	drm_connector_cleanup(connector);
}

static void am_dummy_connector_reset(struct drm_connector *connector)
{
	DRM_DEBUG("[%d] called\n", __LINE__);

	drm_atomic_helper_connector_reset(connector);
}

static struct drm_connector_state *
am_dummy_connector_duplicate_state(struct drm_connector *connector)
{
	struct drm_connector_state *state;

	DRM_DEBUG("[%d] called\n", __LINE__);

	state = drm_atomic_helper_connector_duplicate_state(connector);
	return state;
}

static void am_dummy_connector_destroy_state(struct drm_connector *connector,
					   struct drm_connector_state *state)
{
	DRM_DEBUG("[%d] called\n", __LINE__);

	drm_atomic_helper_connector_destroy_state(connector, state);
}

int meson_dummy_atomic_set_property(struct drm_connector *connector,
			   struct drm_connector_state *state,
			   struct drm_property *property,
			   uint64_t val)
{
	return -EINVAL;
}

int meson_dummy_atomic_get_property(struct drm_connector *connector,
			   const struct drm_connector_state *state,
			   struct drm_property *property,
			   uint64_t *val)
{
	return -EINVAL;
}

static const struct drm_connector_funcs meson_dummy_funcs = {
	.detect			= meson_dummy_detect,
	.fill_modes		= drm_helper_probe_single_connector_modes,
	.destroy		= am_dummy_connector_destroy,
	.reset			= am_dummy_connector_reset,
	.atomic_duplicate_state	= am_dummy_connector_duplicate_state,
	.atomic_destroy_state	= am_dummy_connector_destroy_state,
	.atomic_set_property	= meson_dummy_atomic_set_property,
	.atomic_get_property	= meson_dummy_atomic_get_property,
};

static void meson_dummy_encoder_atomic_enable(struct drm_encoder *encoder,
	struct drm_atomic_state *state)
{
	struct am_meson_crtc_state *meson_crtc_state =
				to_am_meson_crtc_state(encoder->crtc->state);
	struct am_meson_crtc *amcrtc = to_am_meson_crtc(encoder->crtc);
	struct drm_display_mode *mode = &encoder->crtc->mode;
	enum vmode_e vmode = meson_crtc_state->vmode;

	if (vmode != VMODE_DUMMY_ENCP &&
	    vmode != VMODE_DUMMY_ENCI &&
	    vmode != VMODE_DUMMY_ENCL) {
		DRM_DEBUG("enable fail! vmode:%d\n", vmode);
		return;
	}

	if (meson_crtc_state->uboot_mode_init == 1)
		vmode |= VMODE_INIT_BIT_MASK;

	DRM_DEBUG("[%d] called\n", __LINE__);

	meson_vout_notify_mode_change(amcrtc->vout_index,
		vmode, EVENT_MODE_SET_START);
	vout_func_set_vmode(amcrtc->vout_index, vmode);
	meson_vout_notify_mode_change(amcrtc->vout_index,
		vmode, EVENT_MODE_SET_FINISH);
	meson_vout_update_mode_name(amcrtc->vout_index, mode->name, "dummy");

	DRM_DEBUG("[%d] exit\n", __LINE__);
}

static void meson_dummy_encoder_atomic_disable(struct drm_encoder *encoder,
	struct drm_atomic_state *state)
{
	DRM_DEBUG("[%d] called\n", __LINE__);
}

static int meson_dummy_encoder_atomic_check(struct drm_encoder *encoder,
				       struct drm_crtc_state *crtc_state,
				struct drm_connector_state *conn_state)
{
	DRM_DEBUG("[%d] called\n", __LINE__);
	return 0;
}

static const struct drm_encoder_helper_funcs meson_dummy_encoder_helper_funcs = {
	.atomic_enable = meson_dummy_encoder_atomic_enable,
	.atomic_disable = meson_dummy_encoder_atomic_disable,
	.atomic_check = meson_dummy_encoder_atomic_check,
};

static const struct drm_encoder_funcs meson_dummy_encoder_funcs = {
	.destroy        = drm_encoder_cleanup,
};

int meson_dummy_dev_bind(struct drm_device *drm,
	int type, struct meson_connector_dev *intf)
{
	struct meson_drm *priv = drm->dev_private;
	struct drm_connector *connector = NULL;
	struct drm_encoder *encoder = NULL;
	struct meson_dummy *am_dummy = NULL;
	char *connector_name = NULL;
	char *encoder_name = NULL;
	u32 i;
	int ret = 0;

	DRM_INFO("[%s]-[%d] called\n", __func__, __LINE__);

	switch (type) {
	case DRM_MODE_CONNECTOR_MESON_DUMMY_L:
		if (priv->dummyl_from_hdmitx) {
			DRM_INFO("[%s]-[%d] dummy from hdmitx, skip register dummyl connector.\n",
				__func__, __LINE__);
			return 0;
		}
		dummy_mode = dummyl_mode;
		connector_name = "MESON-DUMMYL";
		encoder_name = "am_dummyl_encoder";
		break;
	case DRM_MODE_CONNECTOR_MESON_DUMMY_P:
		dummy_mode = dummyp_mode;
		connector_name = "MESON-DUMMYP";
		encoder_name = "am_dummyp_encoder";
		break;
	default:
		DRM_ERROR("not match\n");
		break;
	};

	am_dummy = kzalloc(sizeof(*am_dummy), GFP_KERNEL);
	if (!am_dummy) {
		DRM_ERROR("alloc drm_dummy failed\n");
		return -ENOMEM;
	}

	am_dummy->drm = drm;
	am_dummy->base.drm_priv = priv;
	encoder = &am_dummy->encoder;
	/* Encoder */
	for (i = 0; i < priv->pipeline->num_postblend; i++)
		encoder->possible_crtcs |= BIT(i);

	drm_encoder_helper_add(encoder, &meson_dummy_encoder_helper_funcs);
	ret = drm_encoder_init(drm, encoder, &meson_dummy_encoder_funcs,
			       DRM_MODE_ENCODER_VIRTUAL, encoder_name);
	if (ret) {
		DRM_ERROR("error:%d: Failed to init lcd encoder\n", __LINE__);
		goto free_resource;
	}

	/* Connector */
	connector = &am_dummy->base.connector;
	intf->conn = connector;
	drm_connector_helper_add(connector, &meson_dummy_helper_funcs);
	ret = drm_connector_init(drm, connector, &meson_dummy_funcs,
				 DRM_MODE_CONNECTOR_VIRTUAL);
	if (ret) {
		DRM_ERROR("%d: Failed to init dummy connector\n", __LINE__);
		goto free_resource;
	}

	/*update name to amlogic name*/
	if (connector_name) {
		kfree(connector->name);
		connector->name = kasprintf(GFP_KERNEL, "%s", connector_name);
		if (!connector->name)
			DRM_ERROR("alloc name failed\n");
	}

	ret = drm_connector_attach_encoder(connector, encoder);
	if (ret != 0) {
		DRM_ERROR("%d: attach failed.\n", __LINE__);
		goto free_resource;
	}

	return 0;

free_resource:
	kfree(am_dummy);

	DRM_DEBUG("%d Exit\n", ret);
	return ret;
}

int meson_dummy_dev_unbind(struct drm_device *drm,
	int type, struct meson_connector_dev *intf)
{
	struct drm_connector *connector = intf->conn;

	struct meson_dummy *am_dummy = 0;

	if (!connector)
		DRM_ERROR("got invalid connector id\n");

	am_dummy = connector_to_meson_dummy(connector);
	if (!am_dummy)
		return -EINVAL;

	kfree(am_dummy);
	DRM_DEBUG("[%d] called\n", __LINE__);
	return 0;
}

