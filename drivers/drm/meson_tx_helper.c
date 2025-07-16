// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <drm/drm_modes.h>
#include <drm/drm_print.h>

#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_mode.h>

#include "meson_tx_helper.h"

enum hdmi_color_depth bitdepth_to_colordepth(int bitdepth)
{
	enum hdmi_color_depth color_depth;

	switch (bitdepth) {
	case 8:
		color_depth = COLORDEPTH_24B;
		break;
	case 10:
		color_depth = COLORDEPTH_30B;
		break;
	case 12:
		color_depth = COLORDEPTH_36B;
		break;
	case 16:
		color_depth = COLORDEPTH_48B;
		break;
	default:
		color_depth = COLORDEPTH_24B;
		break;
	}

	return color_depth;
}

int colordepth_to_bitdepth(enum hdmi_color_depth color_depth)
{
	int bitdepth;

	switch (color_depth) {
	case COLORDEPTH_24B:
		bitdepth = 8;
		break;
	case COLORDEPTH_30B:
		bitdepth = 10;
		break;
	case COLORDEPTH_36B:
		bitdepth = 12;
		break;
	case COLORDEPTH_48B:
		bitdepth = 16;
		break;
	default:
		bitdepth = 8;
		break;
	}

	return bitdepth;
}

void meson_connector_fill_mode_timing(struct drm_display_mode *mode,
				const struct tx_timing *timing, bool edid_vic)
{
	char *strp;

	DRM_DEBUG("%s %d %d %d %d %d %d %d %d\n", __func__,
		timing->h_active, timing->h_front, timing->h_sync, timing->h_total,
		timing->v_active, timing->v_front, timing->v_sync, timing->v_total);
	mode->type = DRM_MODE_TYPE_DRIVER;
	mode->clock = timing->pixel_freq;

	mode->hdisplay = timing->h_active;
	mode->hsync_start = timing->h_active + timing->h_front;
	mode->hsync_end = timing->h_active + timing->h_front + timing->h_sync;

	mode->htotal = timing->h_total;
	/* for 480i/576i, horizontal timing is repeated */
	if (timing->pixel_repetition_factor) {
		mode->flags |= DRM_MODE_FLAG_DBLSCAN;
		mode->hdisplay >>= 1;
		mode->hsync_start >>= 1;
		mode->hsync_end >>= 1;
		mode->htotal >>= 1;
		mode->clock >>= 1;
	}

	/*use hskew to distinguish whether it's qms mode or edid mode*/
	if (edid_vic)
		mode->hskew = 1;
	else
		mode->hskew = 0;

	mode->flags |= timing->h_pol ?
		DRM_MODE_FLAG_PHSYNC : DRM_MODE_FLAG_NHSYNC;
	mode->vdisplay = timing->v_active;
	mode->vsync_start = timing->v_active + timing->v_front;
	mode->vsync_end = timing->v_active + timing->v_front + timing->v_sync;
	mode->vtotal = timing->v_total;
	mode->vscan = 0;
	mode->flags |= timing->v_pol ?
		DRM_MODE_FLAG_PVSYNC : DRM_MODE_FLAG_NVSYNC;

	if (!timing->pi_mode)
		mode->flags |= DRM_MODE_FLAG_INTERLACE;

	if (timing->sname) {
		memcpy(mode->name, timing->sname,
				(strlen(timing->sname) < DRM_DISPLAY_MODE_LEN) ?
				strlen(timing->sname) : DRM_DISPLAY_MODE_LEN);
	} else if (timing->name) {
		memcpy(mode->name, timing->name,
				(strlen(timing->name) < DRM_DISPLAY_MODE_LEN) ?
				strlen(timing->name) : DRM_DISPLAY_MODE_LEN);
	} else {
		DRM_ERROR(" func %s timing has no name\n", __func__);
		sprintf(mode->name, "%ux%up%d", timing->h_active, timing->v_active,
			drm_mode_vrefresh(mode));
	}

	mode->name[DRM_DISPLAY_MODE_LEN - 1] = '\0';
	/* remove _4x3 suffix, in case misunderstand */
	strp = strstr(mode->name, "_4x3");
	if (strp)
		*strp = '\0';
}

void meson_drm_mode_build_tx_timing(struct drm_display_mode *mode,
				struct tx_timing *timing)
{
	if (!mode || !timing)
		return;

	timing->pixel_freq = mode->clock;

	timing->h_active = mode->hdisplay;
	timing->h_blank = mode->htotal - mode->hdisplay;
	timing->h_sync = mode->hsync_end - mode->hsync_start;
	timing->h_front = mode->hsync_start - timing->h_active;
	timing->h_total = mode->htotal;
	timing->v_active = mode->vdisplay;
	timing->v_blank = mode->vtotal - mode->vdisplay;
	timing->v_sync = mode->vsync_end - mode->vsync_start;
	timing->v_front = mode->vsync_start - timing->v_active;
	timing->v_total = mode->vtotal;

	timing->pi_mode = mode->flags & DRM_MODE_FLAG_INTERLACE ? 0 : 1;
	timing->h_pol = mode->flags & DRM_MODE_FLAG_PHSYNC ? 1 : 0;
	timing->v_pol = mode->flags & DRM_MODE_FLAG_PVSYNC ? 1 : 0;
	/* for 480i/576i, horizontal timing is repeated */
	if (mode->flags & DRM_MODE_FLAG_DBLSCAN) {
		timing->pixel_repetition_factor = 1;
		timing->h_active <<= 1;
		timing->h_total <<= 1;
		timing->h_blank <<= 1;
		timing->h_sync <<= 1;
		timing->h_front <<= 1;
		timing->pixel_freq <<= 1;
	}

	DRM_DEBUG("%s %d %d %d %d %d %d %d %d\n", __func__,
		timing->h_active, timing->h_front, timing->h_sync, timing->h_total,
		timing->v_active, timing->v_front, timing->v_sync, timing->v_total);
}

