// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

/* **********************************************************
 * null display support
 * **********************************************************
 */

#include <linux/amlogic/media/vout/vout_notify.h>

int nulldisp_index = VMODE_NULL_DISP_MAX;
struct vinfo_s nulldisp_vinfo[] = {
	{
		.name              = "null",
		.mode              = VMODE_NULL,
		.frac              = 0,
		.width             = 1920,
		.height            = 1080,
		.field_height      = 1080,
		.aspect_ratio_num  = 16,
		.aspect_ratio_den  = 9,
		.sync_duration_num = 60,
		.sync_duration_den = 1,
		.video_clk         = 148500000,
		.htotal            = 2200,
		.vtotal            = 1125,
		.fr_adj_type       = VOUT_FR_ADJ_NONE,
		.viu_color_fmt     = COLOR_FMT_RGB444,
		.viu_mux           = ((3 << 4) | VIU_MUX_MAX),
		.vout_device       = NULL,
		.cur_enc_ppc        = 1,
	},
	{
		.name              = "invalid",
		.mode              = VMODE_INVALID,
		.frac              = 0,
		.width             = 1920,
		.height            = 1080,
		.field_height      = 1080,
		.aspect_ratio_num  = 16,
		.aspect_ratio_den  = 9,
		.sync_duration_num = 60,
		.sync_duration_den = 1,
		.video_clk         = 148500000,
		.htotal            = 2200,
		.vtotal            = 1125,
		.fr_adj_type       = VOUT_FR_ADJ_NONE,
		.viu_color_fmt     = COLOR_FMT_RGB444,
		.viu_mux           = ((3 << 4) | VIU_MUX_MAX),
		.vout_device       = NULL,
		.cur_enc_ppc       = 1,
	},
};

static struct vinfo_s *nulldisp_get_current_info(void *data)
{
	if (nulldisp_index >= ARRAY_SIZE(nulldisp_vinfo))
		return NULL;

	return &nulldisp_vinfo[nulldisp_index];
}

static int nulldisp_set_current_vmode(enum vmode_e mode, void *data)
{
	return 0;
}

static enum vmode_e nulldisp_validate_vmode(char *name, unsigned int frac,
					    void *data)
{
	enum vmode_e vmode = VMODE_MAX;
	int i;

	if (frac)
		return VMODE_MAX;

	for (i = 0; i < ARRAY_SIZE(nulldisp_vinfo); i++) {
		if (strcmp(nulldisp_vinfo[i].name, name) == 0) {
			vmode = nulldisp_vinfo[i].mode;
			nulldisp_index = i;
			break;
		}
	}

	return vmode;
}

static int nulldisp_vmode_is_supported(enum vmode_e mode, void *data)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(nulldisp_vinfo); i++) {
		if (nulldisp_vinfo[i].mode == (mode & VMODE_MODE_BIT_MASK))
			return true;
	}
	return false;
}

static int nulldisp_disable(enum vmode_e cur_vmod, void *data)
{
	return 0;
}

static int nulldisp_vout_set_state(int bit, void *data)
{
	return 0;
}

static int nulldisp_vout_clr_state(int bit, void *data)
{
	return 0;
}

static int nulldisp_vout_get_state(void *data)
{
	return 0;
}

struct vout_server_s nulldisp_vout_server = {
	.name = "nulldisp_vout_server",
	.op = {
		.get_vinfo          = nulldisp_get_current_info,
		.set_vmode          = nulldisp_set_current_vmode,
		.validate_vmode     = nulldisp_validate_vmode,
		.vmode_is_supported = nulldisp_vmode_is_supported,
		.disable            = nulldisp_disable,
		.set_state          = nulldisp_vout_set_state,
		.clr_state          = nulldisp_vout_clr_state,
		.get_state          = nulldisp_vout_get_state,
		.get_disp_cap       = NULL,
		.set_bist           = NULL,
	},
	.data = NULL,
};

/* ********************************************************** */
