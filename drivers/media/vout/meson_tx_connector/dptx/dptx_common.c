// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <linux/vmalloc.h>

#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_hw_opcode.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>

#include "dptx_infoframe.h"
#include "meson_tx_internal.h"
#include "meson_tx_event_mgr.h"
#include "dptx_log.h"
#include "dptx_internal.h"
#include "dptx_aux_helper.h"
#include "dptx_link.h"
#include "dpcd_reg.h"
#include "meson_tx_task_mgr.h"
#include "dptx_timer.h"

/*
 * Per dp1.4a spec chapter 5.1.4:
 * The Source device shall respond to Hot Plug event/Hot Re-plug
 * event by first reading DPCD Link/Sink Device Status registers
 * at DPCD Addresses 00200h through 00205h or DPCD Addresses
 * 02002h through 0200Fh. If the link is unstable or lost, the
 * Source device then reads the DPCD Receiver Capabilities registers
 * at DPCD Addresses 00000h through 0000Fh to determine
 * the appropriate information needed to train the link.
 * The Source device shall then initiate Link Training.
 */
int dptx_hpd_plugin(struct meson_tx_dev *tx_dev)
{
	struct dptx_common *tx_comm = to_dptx_common(tx_dev);
	int ret = 0;
	int i;

	DPTX_INFO("%s %px\n", __func__, tx_dev);

	ret = dptx_aux_read_dpcd(tx_comm->tx_aux, DP_SINK_COUNT,
			tx_comm->link_sink_status,
			sizeof(tx_comm->link_sink_status));
	if (ret < 0)
		DPTX_ERROR("read dpcd link/sink status failed:%d\n", ret);

	ret = dptx_aux_read_dpcd_caps(tx_comm->tx_aux,
		tx_dev->dpcd_buf, sizeof(tx_dev->dpcd_buf));
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
	return 0;
}

int dptx_hpd_plugout(struct meson_tx_dev *tx_dev)
{
	DPTX_INFO("%s %px\n", __func__, tx_dev);
	/* DP V1.4a chapter5.1.4, disable the Main-Link as a power-saving measure */
	meson_tx_phy_power_off(tx_dev->tx_hw_base->tx_phy);
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

/* map from enum type to actual bits per color component */
u8 dptx_get_mapped_bpc(color_depth_t cd)
{
	u8 bit_depth = 0;

	switch (cd) {
	case COLORDEPTH_18B:
		bit_depth = 6;
		break;
	case COLORDEPTH_24B:
		bit_depth = 8;
		break;
	case COLORDEPTH_30B:
		bit_depth = 10;
		break;
	case COLORDEPTH_36B:
		bit_depth = 12;
		break;
	case COLORDEPTH_42B:
		bit_depth = 14;
		break;
	case COLORDEPTH_48B:
		bit_depth = 16;
		break;
	default:
		DPTX_ERROR("%s cd[%d] not find mapped bit_depth\n", __func__, cd);
		bit_depth = 8;
		break;
	}
	return bit_depth;
}

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

	if (color >= COLORIMETRY_FORMAT_MAX) {
		DPTX_ERROR("[%s]: invalid input color format(%d)\n", __func__, color);
		return base_bpp[COLORIMETRY_FORMAT_RGB] * colordepth / 8;
	}
	return base_bpp[color] * colordepth / 8;
}

/* color component(R/G/B, Y/Cb/Cr) count per pixel */
u8 dptx_get_color_component_cnt(enum colorimetry_format color)
{
	u8 color_component[COLORIMETRY_FORMAT_MAX] = {
		[COLORIMETRY_FORMAT_RGB] = 3,
		[COLORIMETRY_FORMAT_YUV444] = 3,
		[COLORIMETRY_FORMAT_YUV422] = 2,
		/* TO confirm for Y420 */
		[COLORIMETRY_FORMAT_YUV420] = 1,
		[COLORIMETRY_FORMAT_Y_ONLY] = 1,
		[COLORIMETRY_FORMAT_RAW] = 1,
	};

	if (color >= COLORIMETRY_FORMAT_MAX) {
		DPTX_ERROR("[%s]: invalid input color format(%d)\n", __func__, color);
		return color_component[COLORIMETRY_FORMAT_RGB];
	}
	return color_component[color];
}

static u32 calc_video_fmt_bw(u32 pixel_clk, enum colorimetry_format cs, color_depth_t cd)
{
	return pixel_clk * dptx_calc_bpp(cs, dptx_get_mapped_bpc(cd)) / 24;
}

/* calculate HW format para from sw para, such as tmds_clk, lane_count/link_rate */
int dptx_calc_hw_fmt_para(struct meson_tx_hw *tx_hw, struct meson_tx_format_para *sw_para,
	struct dptx_hw_fmt_para *hw_para)
{
	u32 bandwidth;
	struct tx_timing *t;
	enum dp_link_rate_e rate = DPTX_LINK_RATE_1P62GHZ;
	enum dp_lane_count_e count = DPTX_LANE_COUNT_1;

	if (!tx_hw || !sw_para || !hw_para) {
		DPTX_ERROR("[%s]: invalid input param\n", __func__);
		return -EINVAL;
	}

	t = &sw_para->timing;
	/* calculate the current resolution bandwidth with colorspace and depth */
	bandwidth = calc_video_fmt_bw(t->pixel_freq, (enum colorimetry_format)sw_para->cs,
		sw_para->cd);
	/* if use 128b132b coding will cost 0.30%, bandwidth = bandwidth * 33 / 32; */
	/* 8b10b coding will cost 20% */
	bandwidth += bandwidth / 4;
	/* downspread overhead 0.5% */
	bandwidth += bandwidth / 199;
	/* FEC overhead 0.5% */
	bandwidth += bandwidth * 3 / 256;
	/* multiple 10 times */
	bandwidth *= 10;

	hw_para->total_bandwidth = bandwidth;
	/* choose low rate firstly, then count */
	if (bandwidth <= 1620000) {
		rate = DPTX_LINK_RATE_1P62GHZ;
		count = DPTX_LANE_COUNT_1;
	} else if (bandwidth <= 1620000 * 2) {
		rate = DPTX_LINK_RATE_1P62GHZ;
		count = DPTX_LANE_COUNT_2;
	} else if (bandwidth <= 1620000 * 4) {
		rate = DPTX_LINK_RATE_1P62GHZ;
		count = DPTX_LANE_COUNT_4;
	} else if (bandwidth <= 2700000 * 4) {
		rate = DPTX_LINK_RATE_2P70GHZ;
		count = DPTX_LANE_COUNT_4;
	} else if (bandwidth <= 5400000 * 4) {
		rate = DPTX_LINK_RATE_5P40GHZ;
		count = DPTX_LANE_COUNT_4;
	} else if (bandwidth <= 8100000 * 4) {
		rate = DPTX_LINK_RATE_8P10GHZ;
		count = DPTX_LANE_COUNT_4;
	} else {
		DPTX_ERROR("Too high bandwidth %d %s cs %d cd %d\n", bandwidth, sw_para->name,
			sw_para->cs, sw_para->cd);
		return -1;
	}

	hw_para->link_rate = rate;
	hw_para->lane_count = count;

	DPTX_INFO("bandwidth: %s cs %d cd %d bandwidth %dkHz Rate %d Count %d\n", sw_para->name,
		sw_para->cs, sw_para->cd, bandwidth, rate, count);
	return 0;
}

static int dptx_pre_mode_enable(struct meson_tx_dev *tx_dev, struct meson_tx_format_para *para)
{
	return dptx_hw_cntl(tx_dev->tx_hw_base, MODE_FLOW_PRE_ENABLE_MODE, para, NULL);
}

static int dptx_mode_enable(struct meson_tx_dev *tx_dev, struct meson_tx_format_para *para)
{
	struct dptx_common *tx_comm = NULL;
	int ret = 0;

	if (!tx_dev || !para)
		return -EINVAL;
	tx_comm = to_dptx_common(tx_dev);

	ret = dptx_link_training(tx_comm);
	if (ret != 0) {
		DPTX_ERROR("%s link training failed, abort mode set\n", __func__);
		return ret;
	}

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

/* hpd_irq interrupt bottom handler */
static void dptx_irq_hpd_process(void *para)
{
	struct dptx_common *tx_comm = (struct dptx_common *)para;
	ssize_t ret = 0;
	/* 00200h~00205h */
	u8 link_sink_status_buf[6] = {0};
	/* 02002h~0200fh */
	u8 dprx_esi_buf[14] = {0};
	u8 sink_count = 0;

	if (!tx_comm) {
		DPTX_ERROR("%s NULL tx_comm pointer\n", __func__);
		return;
	}

	/*
	 * 1.per DisplayPort Standard v1.4a chapter5.1.4,
	 * The Source device shall respond to an IRQ_HPD pulse by
	 * starting to read the registers located at DPCD Addresses
	 * 00200h through 00205h -or- DPCD Addresses 02002h through
	 * 0200Fh, by way of AUX read transactions within 100ms of
	 * the HPD signal's rising edge.
	 *
	 * 2. TODO for MST case:
	 * Per Table2-170: An MST upstream device shall use 02002h
	 * through 0200Fh instead of the Link/Sink Device Status field
	 * registers, starting from DPCD Address 00200h.
	 */
	ret = dptx_aux_read_dpcd(tx_comm->tx_aux, DP_SINK_COUNT, link_sink_status_buf,
		sizeof(link_sink_status_buf));
	if (ret < 0) {
		DPTX_ERROR("%s link/sink device status read ret:%d\n", __func__, ret);
		return;
	}

	ret = dptx_aux_read_dpcd(tx_comm->tx_aux, DP_SINK_COUNT_ESI, dprx_esi_buf,
		sizeof(dprx_esi_buf));
	if (ret < 0) {
		DPTX_ERROR("%s DPRX Event Status Indicator read ret:%d\n", __func__, ret);
		return;
	}

	/* DEVICE_SERVICE_IRQ_VECTOR: 00201h */
	if (link_sink_status_buf[1] & BIT(4)) {
		DPTX_INFO("%s DOWN_REP_MSG_RDY, TODO for MST\n", __func__);
		/* TODO for MST
		 * per chapter 2.11.4.1.3 of dp1.4a, the Sideband DOWN_REP_MSG is read from
		 * the MST DPRX DOWN_REP DPCD locations. The DPTX device shall clear the
		 * DOWN_REP_MSG_RDY bit to 0 when reading of the message is complete
		 */
	}
	if (link_sink_status_buf[1] & BIT(5)) {
		DPTX_INFO("%s UP_REQ_MSG_RDY, TODO for MST\n", __func__);
		/* TODO for MST
		 * per chapter 2.11.4.1.4 of dp1.4a, the Sideband UP_REQ_MSG is read from
		 * the MST DPRX UP_REQ DPCD locations. The MST DPTX device shall clear the
		 UP_REQ_MSG_RDY bit to 0 when reading of the message is complete.
		 */
	}

	/* DEVICE_SERVICE_IRQ_VECTOR: 00201h */
	if (link_sink_status_buf[1] & BIT(6))
		DPTX_INFO("%s sink Device-specific IRQ, ignore\n", __func__);

	/* LANE_ALIGN_STATUS_UPDATED 0x00204h
	 * refer to chapter 5.3.3.1 of dp1.4a spec and Table 2-163
	 */
	if (link_sink_status_buf[4] & BIT(6)) {
		/* SINK_COUNT: 0x00200h, bit7,5:0 */
		sink_count = (link_sink_status_buf[2] >> 7) << 6 |
			(link_sink_status_buf[2] & 0x3f);
		DPTX_INFO("%s DOWNSTREAM_PORT_STATUS_CHANGED to: %d\n", __func__, sink_count);
	}
	if (link_sink_status_buf[4] & BIT(7)) {
		DPTX_INFO("%s LINK_STATUS_UPDATED\n", __func__, sink_count);
		/* re-training */
		if (dptx_link_training(tx_comm) != 0)
			DPTX_ERROR("%s link training failed, abort mode set\n", __func__);
	}

	/* DEVICE_SERVICE_IRQ_VECTOR_ESI1: 02004 */
	if (dprx_esi_buf[2] & BIT(0)) {
		/* Table 2-170 of dp1.4a spec */
		DPTX_INFO("%s RX_GTC_PRIMARY_REQ_STATUS_CHANGE.\n", __func__);
	}
	if (dprx_esi_buf[2] & BIT(1)) {
		/* Table 2-170 of dp1.4a spec */
		DPTX_INFO("%s GTC: requesting DPTX to initiate lock acquisition.\n",
			__func__);
	}

	if (dprx_esi_buf[2] & BIT(2)) {
		/* Table 2-170 of dp1.4a spec */
		DPTX_INFO("%s CEC-Tunneling-over-AUX status changed.\n", __func__);
	}

	/* LINK_SERVICE_IRQ_VECTOR_ESI0: 02005h */
	if (dprx_esi_buf[3] & BIT(0)) {
		DPTX_INFO("%s RX_CAP_CHANGED: TODO\n", __func__);
		/*
		 * refer to Table 2-170, Upstream device shall read the Receiver
		 * Capability filed(00000h throuth 000ffh)
		 */
		ret = dptx_aux_read_dpcd(tx_comm->tx_aux, DP_DPCD_REV, tx_comm->base.dpcd_buf,
			sizeof(tx_comm->base.dpcd_buf));
		if (ret < 0) {
			DPTX_ERROR("%s link/sink device status read ret:%d\n", __func__, ret);
			return;
		}
		/*
		 * TODO: need to do re-training.
		 * whether need to notify system to change mode?
		 */
	}
	if (dprx_esi_buf[3] & BIT(1)) {
		DPTX_INFO("%s LINK_STATUS_CHANGED\n", __func__);
		/*
		 * per Table 2-170, Upstream device shall read the
		 * Link/Sink Device Status field (DPCD Addresses
		 * 00200h through 002FFh.
		 * Note: spec do not say other actions besides DPCD read
		 */
		ret = dptx_aux_read_dpcd(tx_comm->tx_aux, DP_DPCD_REV, tx_comm->base.dpcd_buf,
			sizeof(tx_comm->base.dpcd_buf));
		if (ret < 0) {
			DPTX_ERROR("%s link/sink device status read ret:%d\n", __func__, ret);
			return;
		}
	}
	if (dprx_esi_buf[3] & BIT(2)) {
		DPTX_INFO("%s STREAM_STATUS_CHANGED: TODO\n", __func__);
		/*
		 * per Table 2-170, Upstream Source device shall
		 * re-check the Stream Status with the
		 * QUERY_STREAM_ENCRYPTION_STATUS message.
		 */
	}
	if (dprx_esi_buf[3] & BIT(3)) {
		DPTX_INFO("%s HDMI_LINK_STATUS_CHANGED: TODO\n", __func__);
		/*
		 * per Table 2-170, Upstream DP device shall read
		 * the DP-to-HDMI protocol converter's downstream
		 * HDMI link status registers.
		 */
	}
	if (dprx_esi_buf[3] & BIT(4)) {
		DPTX_INFO("%s CONNECTED_OFF_ENTRY_REQUESTED\n", __func__);
		/* per Table 2-170, write 1 to l clear this bit in DPRX */
	}

	/* DSC_STATUS: 0020Fh and 02011h */
	if (dprx_esi_buf[13] & BIT(0)) {
		DPTX_INFO("%s RC Buffer Under-run, TODO for DSC\n", __func__);
		/* per Table 2-163, Sticky until cleared by a write of 1 */
	}
	if (dprx_esi_buf[13] & BIT(1)) {
		DPTX_INFO("%s RC Buffer Overflow, TODO for DSC\n", __func__);
		/* per Table 2-163, Sticky until cleared by a write of 1 */
	}
	if (dprx_esi_buf[13] & BIT(2)) {
		DPTX_INFO("%s Chunk Length mismatch with PPS, TODO for DSC\n", __func__);
		/* per Table 2-163, Sticky until cleared by a write of 1 */
	}
}

static struct tx_task_info dptx_common_task_infos[] = {
	{
		.name = "irq_hpd_task",
		.fn = dptx_irq_hpd_process,
		.type = IRQ_HPD_TASK,
		.queue_type = TASK_QUEUE_HPD_IRQ,
		.flag = TASK_FLAG_DELAY_WORK,
		.queue_name = "irq_hpd",
		.queue_flag = WQ_HIGHPRI | WQ_CPU_INTENSIVE,
	}
};

/* task init for dptx common */
static int dptx_common_task_init(struct dptx_common *tx_comm)
{
	int i;
	int ret = 0;
	char queue_name[QUEUE_NAME_MAX_LEN] = {0};

	if (!tx_comm) {
		DPTX_ERROR("%s NULL tx_comm pointer\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(dptx_common_task_infos); i++) {
		sprintf(queue_name, "dptx%d_%s", tx_comm->dev_idx,
			dptx_common_task_infos[i].queue_name);
		dptx_common_task_infos[i].queue_name = queue_name;
		ret = tx_task_mgr_setup_task(tx_comm->base.task_mgr,
			&dptx_common_task_infos[i], tx_comm);
		if (ret)
			DPTX_ERROR("%s setup task[%s] failed\n",
				__func__, dptx_common_task_infos[i].name);
	}
	return 0;
}

int dptx_common_init(struct dptx_common *tx_comm, struct meson_tx_hw *tx_hw)
{
	struct meson_tx_event_mgr *tx_event_mgr;
	struct meson_tx_tracer *tx_tracer;
	struct meson_tx_dev *tx_dev = &tx_comm->base;
	struct dptx_hw_common *hw_comm = to_dptx_hw_common(tx_hw);
	int ret = 0;

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
	/* init the timers' mutex */
	dptx_timer_init(hw_comm);
	/* dptx aux init */
	tx_comm->tx_aux = dptx_aux_init(hw_comm);
	if (!tx_comm->tx_aux)
		return -ENOMEM;

	tx_comm->hw_comm = hw_comm;
	/* pass aux instance to HW side for aux access during mode set */
	tx_comm->hw_comm->tx_aux = tx_comm->tx_aux;
	/* dptx vout init */
	dptx_vout_init(tx_comm);
	/* dptx common event/task init */
	ret = dptx_common_task_init(tx_comm);
	if (ret < 0)
		return ret;

	return 0;
}

int dptx_common_uninit(struct dptx_common *tx_comm)
{
	if (!tx_comm)
		return -EINVAL;

	dptx_vout_uninit(tx_comm);
	kfree(tx_comm->base.tx_cap);
	tx_comm->base.tx_cap = NULL;
	kfree(tx_comm->tx_aux);
	tx_comm->tx_aux = NULL;
	meson_tx_tracer_destroy(tx_comm->base.tx_tracer);
	meson_tx_event_mgr_destroy(tx_comm->base.event_mgr);
	meson_tx_dev_release(&tx_comm->base);
	return 0;
}

