// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <linux/vmalloc.h>

#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_hw_opcode.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>

#include "dptx_infoframe.h"
#include "../meson_tx_internal.h"
#include "../meson_tx_event_mgr.h"
#include "dptx_log.h"
#include "dptx_internal.h"
#include "dptx_aux_helper.h"

int dptx_hpd_plugin(struct meson_tx_dev *tx_dev)
{
	struct dptx_common *tx_comm = to_dptx_common(tx_dev);
	int ret = 0;
	int i;

	DPTX_INFO("%s %px\n", __func__, tx_dev);

	ret = dptx_aux_read_dpcd_caps(tx_comm->tx_aux, tx_dev->dpcd_buf, sizeof(tx_dev->dpcd_buf));
	if (ret < 0)
		DPTX_INFO("read dpcd cap failed\n");

	for (i = 0; i < EDID_READ_MAX_RETRY; i++) {
		ret = dptx_aux_read_edid_data(tx_comm->tx_aux, tx_dev->edid_buf,
			sizeof(tx_dev->edid_buf));
		if (ret < 0)
			DPTX_INFO("read edid failed\n");
		else
			break;
	}
	/* TODO after reading the EDID/DPCD caps, dptx and hdmitx may share the parsing in
	 * the function hdmitx_edid_process()
	 */

	return 0;
}

int dptx_hpd_plugout(struct meson_tx_dev *tx_dev)
{
	DPTX_INFO("%s %px\n", __func__, tx_dev);
	return 0;
}

/* Note: timing list memory need to be freed after DRM get mode list
 * TODO:
 * 1.add vic related timing in EDID to timing list
 * 2.filter repeated timings?
 */
int dptx_get_mode_list(struct dptx_common *tx_comm, struct tx_timing **timing_list)
{
	struct display_id_cap *disp_id_cap = NULL;
	struct tx_timing *disp_id_timings = 0;
	struct tx_timing *timing;
	u32 i = 0;
	int count = 0;

	if (!tx_comm || !timing_list) {
		DPTX_ERROR("%s: invalid param\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&tx_comm->base.valid_mode_mutex);
	disp_id_cap = &tx_comm->base.rxcap.disp_id_cap;

	if (disp_id_cap->timing_count == 0) {
		mutex_unlock(&tx_comm->base.valid_mode_mutex);
		return 0;
	}

	/* Allocate memory for timing list.
	 * note timing list memory may be > PAGE_SIZE
	 */
	disp_id_timings = vzalloc(disp_id_cap->timing_count * sizeof(struct tx_timing));
	if (!disp_id_timings) {
		mutex_unlock(&tx_comm->base.valid_mode_mutex);
		return -ENOMEM;
	}

	/* Filter supported timings */
	for (i = 0; i < disp_id_cap->timing_count; i++) {
		timing = &disp_id_cap->timing[i / BLK_TIMING_CNT][i % BLK_TIMING_CNT];
		if (!dptx_validate_timing_with_basic_cs(&tx_comm->base, timing))
			continue;
		memcpy(&disp_id_timings[count++], timing, sizeof(*timing));
	}
	if (count == 0) {
		vfree(disp_id_timings);
		*timing_list = NULL;
	} else {
		*timing_list = disp_id_timings;
	}
	mutex_unlock(&tx_comm->base.valid_mode_mutex);
	return count;
}
EXPORT_SYMBOL(dptx_get_mode_list);

/*
 * Function: dptx_calc_bpp
 * calculate the bits per pixel from color format and depth
 */
u32 dptx_calc_bpp(enum colorimetry_format color, u8 colordepth)
{
	u8 base_bpp[COLORIMETRY_FORMAT_MAX] = {
		[COLORIMETRY_FORMAT_RGB] = 24,
		[COLORIMETRY_FORMAT_YUV444] = 24,
		[COLORIMETRY_FORMAT_YUV422] = 16,
		[COLORIMETRY_FORMAT_YUV420] = 12,
		[COLORIMETRY_FORMAT_Y_ONLY] = 8,
		[COLORIMETRY_FORMAT_RAW] = 8,
	};

	return base_bpp[color] * colordepth / 8;
}

/* calculate HW format para from sw para, such as tmds_clk, lane_count/link_rate */
int dptx_calc_hw_fmt_para(struct meson_tx_hw *tx_hw, struct meson_tx_format_para *sw_para,
	struct dptx_hw_fmt_para *hw_para)
{
	if (!tx_hw || !sw_para || !hw_para) {
		DPTX_ERROR("[%s]: invalid input param\n", __func__);
		return -EINVAL;
	}

	/* TODO */
	return 0;
}

static int dptx_pre_mode_enable(struct meson_tx_dev *tx_dev, struct meson_tx_format_para *para)
{
	return dptx_hw_cntl(tx_dev->tx_hw_base, MODE_FLOW_PRE_ENABLE_MODE, para, NULL);
}

static int dptx_mode_enable(struct meson_tx_dev *tx_dev, struct meson_tx_format_para *para)
{
	return dptx_hw_cntl(tx_dev->tx_hw_base, MODE_FLOW_ENABLE_MODE, para, NULL);
}

static int dptx_post_mode_enable(struct meson_tx_dev *tx_dev, struct meson_tx_format_para *para)
{
	return dptx_hw_cntl(tx_dev->tx_hw_base, MODE_FLOW_POST_ENABLE_MODE, para, NULL);
}

struct meson_tx_helper_ops dptx_common_helper_ops = {
	.create_sysfs	= dptx_create_sysfs,
	.remove_sysfs	= dptx_remove_sysfs,
	.hpd_plugin		= dptx_hpd_plugin,
	.hpd_plugout	= dptx_hpd_plugout,
	.validate_fmt_para	= dptx_validate_tx_state_fmt_para,
	.pre_mode_enable	= dptx_pre_mode_enable,
	.mode_enable		= dptx_mode_enable,
	.post_mode_enable	= dptx_post_mode_enable,
};

int dptx_common_init(struct dptx_common *tx_comm, struct meson_tx_hw *tx_hw)
{
	struct meson_tx_event_mgr *tx_event_mgr;
	struct meson_tx_tracer *tx_tracer;
	struct meson_tx_dev *tx_dev = &tx_comm->base;
	struct dptx_hw_common *hw_comm = to_dptx_hw_common(tx_hw);

	/* meson_tx_dev common init */
	meson_tx_dev_init(tx_dev, tx_hw, &dptx_common_helper_ops);

	tx_event_mgr = meson_tx_event_mgr_create(&tx_comm->base);
	meson_tx_event_mgr_suspend(tx_event_mgr, false);
	meson_tx_attch_platform_data(&tx_comm->base,
		TX_PLATFORM_UEVENT, tx_event_mgr);
	tx_tracer = meson_tx_tracer_create(tx_event_mgr);
	meson_tx_attch_platform_data(&tx_comm->base,
		TX_PLATFORM_TRACER, tx_tracer);

	mutex_init(&tx_dev->set_mode_mutex);
	mutex_init(&tx_dev->valid_mode_mutex);
	/* parse rx cap in both EDID and displayID */
	tx_dev->edid_parse_mask = 0x3;

	/* dptx aux init */
	tx_comm->tx_aux = dptx_aux_init(hw_comm);
	if (!tx_comm->tx_aux)
		return -ENOMEM;

	tx_comm->hw_comm = hw_comm;

	/* dptx vout init */
	dptx_vout_init(tx_comm);

	return 0;
}
