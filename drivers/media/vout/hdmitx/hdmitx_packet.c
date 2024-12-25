// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include "hdmitx_log.h"

static struct hdmitx_common *global_tx_common;

void hdmitx_packet_init(struct hdmitx_common *tx_comm)
{
	global_tx_common = tx_comm;
}

void hdmitx_set_cuva_hdr_vs_emds(struct cuva_hdr_vs_emds_para *data)
{
	struct hdmi_packet_t vs_emds[3];
	unsigned long flags;
	struct hdmitx_hw_common *tx_hw = global_tx_common->tx_hw;

	memset(vs_emds, 0, sizeof(vs_emds));
	spin_lock_irqsave(&global_tx_common->edid_spinlock, flags);
	if (!data) {
		hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_EMP, NULL);
		spin_unlock_irqrestore(&global_tx_common->edid_spinlock, flags);
		return;
	}

	vs_emds[0].hb[0] = 0x7f;
	vs_emds[0].hb[1] = 1 << 7;
	/* Sequence_Index */
	vs_emds[0].hb[2] = 0;
	vs_emds[0].pb[0] = (1 << 7) | (1 << 4) | (1 << 2) | (1 << 1);
	/* rsvd */
	vs_emds[0].pb[1] = 0;
	/* Organization_ID */
	vs_emds[0].pb[2] = 0;
	/* Data_Set_Tag_MSB */
	vs_emds[0].pb[3] = 0;
	/* Data_Set_Tag_LSB */
	vs_emds[0].pb[4] = 2;
	/* Data_Set_Length_MSB */
	vs_emds[0].pb[5] = 0;
	/* Data_Set_Length_LSB */
	vs_emds[0].pb[6] = 0x38;
	vs_emds[0].pb[7] = GET_OUI_BYTE0(CUVA_IEEEOUI);
	vs_emds[0].pb[8] = GET_OUI_BYTE1(CUVA_IEEEOUI);
	vs_emds[0].pb[9] = GET_OUI_BYTE2(CUVA_IEEEOUI);
	vs_emds[0].pb[10] = data->system_start_code;
	vs_emds[0].pb[11] = ((data->version_code & 0xf) << 4) |
			     ((data->min_maxrgb_pq >> 8) & 0xf);
	vs_emds[0].pb[12] = data->min_maxrgb_pq & 0xff;
	vs_emds[0].pb[13] = (data->avg_maxrgb_pq >> 8) & 0xf;
	vs_emds[0].pb[14] = data->avg_maxrgb_pq & 0xff;
	vs_emds[0].pb[15] = (data->var_maxrgb_pq >> 8) & 0xf;
	vs_emds[0].pb[16] = data->var_maxrgb_pq & 0xff;
	vs_emds[0].pb[17] = (data->max_maxrgb_pq >> 8) & 0xf;
	vs_emds[0].pb[18] = data->max_maxrgb_pq & 0xff;
	vs_emds[0].pb[19] = (data->targeted_max_lum_pq >> 8) & 0xf;
	vs_emds[0].pb[20] = data->targeted_max_lum_pq & 0xff;
	vs_emds[0].pb[21] = ((data->transfer_character & 1) << 7) |
			     ((data->base_enable_flag & 0x1) << 6) |
			     ((data->base_param_m_p >> 8) & 0x3f);
	vs_emds[0].pb[22] = data->base_param_m_p & 0xff;
	vs_emds[0].pb[23] = data->base_param_m_m & 0x3f;
	vs_emds[0].pb[24] = (data->base_param_m_a >> 8) & 0x3;
	vs_emds[0].pb[25] = data->base_param_m_a & 0xff;
	vs_emds[0].pb[26] = (data->base_param_m_b >> 8) & 0x3;
	vs_emds[0].pb[27] = data->base_param_m_b & 0xff;
	vs_emds[1].hb[0] = 0x7f;
	vs_emds[1].hb[1] = 0;
	/* Sequence_Index */
	vs_emds[1].hb[2] = 1;
	vs_emds[1].pb[0] = data->base_param_m_n & 0x3f;
	vs_emds[1].pb[1] = (((data->base_param_k[0] & 3) << 4) |
			   ((data->base_param_k[1] & 3) << 2) |
			   ((data->base_param_k[2] & 3) << 0));
	vs_emds[1].pb[2] = data->base_param_delta_enable_mode & 0x7;
	vs_emds[1].pb[3] = data->base_param_enable_delta & 0x7f;
	vs_emds[1].pb[4] = (((data->_3spline_enable_num & 0x3) << 3) |
			    ((data->_3spline_enable_flag & 1)  << 2) |
			    (data->_3spline_data[0].th_enable_mode & 0x3));
	vs_emds[1].pb[5] = data->_3spline_data[0].th_enable_mb;
	vs_emds[1].pb[6] = (data->_3spline_data[0].th_enable >> 8) & 0xf;
	vs_emds[1].pb[7] = data->_3spline_data[0].th_enable & 0xff;
	vs_emds[1].pb[8] =
		(data->_3spline_data[0].th_enable_delta[0] >> 8) & 0x3;
	vs_emds[1].pb[9] = data->_3spline_data[0].th_enable_delta[0] & 0xff;
	vs_emds[1].pb[10] =
		(data->_3spline_data[0].th_enable_delta[1] >> 8) & 0x3;
	vs_emds[1].pb[11] = data->_3spline_data[0].th_enable_delta[1] & 0xff;
	vs_emds[1].pb[12] = data->_3spline_data[0].enable_strength;
	vs_emds[1].pb[13] = data->_3spline_data[1].th_enable_mode & 0x3;
	vs_emds[1].pb[14] = data->_3spline_data[1].th_enable_mb;
	vs_emds[1].pb[15] = (data->_3spline_data[1].th_enable >> 8) & 0xf;
	vs_emds[1].pb[16] = data->_3spline_data[1].th_enable & 0xff;
	vs_emds[1].pb[17] =
		(data->_3spline_data[1].th_enable_delta[0] >> 8) & 0x3;
	vs_emds[1].pb[18] = data->_3spline_data[1].th_enable_delta[0] & 0xff;
	vs_emds[1].pb[19] =
		(data->_3spline_data[1].th_enable_delta[1] >> 8) & 0x3;
	vs_emds[1].pb[20] = data->_3spline_data[1].th_enable_delta[1] & 0xff;
	vs_emds[1].pb[21] = data->_3spline_data[1].enable_strength;
	vs_emds[1].pb[22] = data->color_saturation_num;
	vs_emds[1].pb[23] = data->color_saturation_gain[0];
	vs_emds[1].pb[24] = data->color_saturation_gain[1];
	vs_emds[1].pb[25] = data->color_saturation_gain[2];
	vs_emds[1].pb[26] = data->color_saturation_gain[3];
	vs_emds[1].pb[27] = data->color_saturation_gain[4];
	vs_emds[2].hb[0] = 0x7f;
	vs_emds[2].hb[1] = (1 << 6);
	/* Sequence_Index */
	vs_emds[2].hb[2] = 2;
	vs_emds[2].pb[0] = data->color_saturation_gain[5];
	vs_emds[2].pb[1] = data->color_saturation_gain[6];
	vs_emds[2].pb[2] = data->color_saturation_gain[7];
	vs_emds[2].pb[3] = data->graphic_src_display_value;
	/* Reserved */
	vs_emds[2].pb[4] = 0;
	vs_emds[2].pb[5] = data->max_display_mastering_lum >> 8;
	vs_emds[2].pb[6] = data->max_display_mastering_lum & 0xff;

	hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_EMP, (u8 *)&vs_emds);
	spin_unlock_irqrestore(&global_tx_common->edid_spinlock, flags);
}

#ifdef CONFIG_AMLOGIC_HDMITX21
/*
 * EMP packets is different as other packets
 * no checksum, the successive packets in a video frame...
 */
void hdmi_emp_infoframe_set(enum emp_type type, struct emp_packet_st *info)
{
	u8 body[31] = {0};
	u8 *md;
	u16 pkt_type;

	pkt_type = (HDMI_INFOFRAME_TYPE_EMP << 8) | type;
	if (!info) {
		hdmitx_infoframe_send(pkt_type, NULL);
		return;
	}

	/* head body, 3bytes */
	body[0] = info->header.header;
	body[1] = info->header.first << 7 | info->header.last << 6;
	body[2] = info->header.seq_idx;
	/* packet body, 28bytes */
	body[3] = info->body.emp0.new << 7 |
		info->body.emp0.end << 6 |
		info->body.emp0.ds_type << 4 |
		info->body.emp0.afr << 3 |
		info->body.emp0.vfr << 2 |
		info->body.emp0.sync << 1;
	/* RSVD */
	body[4] = 0;
	body[5] = info->body.emp0.org_id;
	body[6] = info->body.emp0.ds_tag >> 8 & 0xff;
	body[7] = info->body.emp0.ds_tag & 0xff;
	body[8] = info->body.emp0.ds_length >> 8 & 0xff;
	body[9] = info->body.emp0.ds_length & 0xff;
	md = &body[10];
	switch (info->type) {
	case EMP_TYPE_VRR_GAME:
		md[0] = info->body.emp0.md.game_md.fva_factor_m1 << 4 |
			info->body.emp0.md.game_md.vrr_en << 0;
		md[1] = info->body.emp0.md.game_md.base_vfront;
		md[2] = info->body.emp0.md.game_md.brr_rate >> 8 & 3;
		md[3] = info->body.emp0.md.game_md.brr_rate & 0xff;
		break;
	case EMP_TYPE_VRR_QMS:
		md[0] = info->body.emp0.md.qms_md.qms_en << 2 |
			info->body.emp0.md.qms_md.m_const << 1;
		md[1] = info->body.emp0.md.qms_md.base_vfront;
		md[2] = info->body.emp0.md.qms_md.next_tfr << 3 |
			(info->body.emp0.md.qms_md.brr_rate >> 8 & 3);
		md[3] = info->body.emp0.md.qms_md.brr_rate & 0xff;
		break;
	case EMP_TYPE_SBTM:
		md[0] = info->body.emp0.md.sbtm_md.sbtm_ver << 0;
		md[1] = info->body.emp0.md.sbtm_md.sbtm_mode << 0 |
			info->body.emp0.md.sbtm_md.sbtm_type << 2 |
			info->body.emp0.md.sbtm_md.grdm_min << 4 |
			info->body.emp0.md.sbtm_md.grdm_lum << 6;
		md[2] = (info->body.emp0.md.sbtm_md.frmpblimitint >> 8) & 0x3f;
		md[3] = info->body.emp0.md.sbtm_md.frmpblimitint & 0xff;
		break;
	default:
		break;
	}
	hdmitx_infoframe_send(pkt_type, body);
}

/*
 * this is only configuring the EMP frame body, not send by HW
 * then call hdmi_emp_infoframe_set to send out
 */
void hdmi_emp_frame_set_member(struct emp_packet_st *info,
	enum emp_component_conf conf, u32 val)
{
	if (!info)
		return;

	switch (conf) {
	case CONF_HEADER_INIT:
		/* fixed value */
		info->header.header = HDMI_INFOFRAME_TYPE_EMP;
		break;
	case CONF_HEADER_LAST:
		info->header.last = !!val;
		break;
	case CONF_HEADER_FIRST:
		info->header.first = !!val;
		break;
	case CONF_HEADER_SEQ_INDEX:
		info->header.seq_idx = val;
		break;
	case CONF_SYNC:
		info->body.emp0.sync = !!val;
		break;
	case CONF_VFR:
		info->body.emp0.vfr = !!val;
		break;
	case CONF_AFR:
		info->body.emp0.afr = !!val;
		break;
	case CONF_DS_TYPE:
		info->body.emp0.ds_type = val & 3;
		break;
	case CONF_END:
		info->body.emp0.end = !!val;
		break;
	case CONF_NEW:
		info->body.emp0.new = !!val;
		break;
	case CONF_ORG_ID:
		info->body.emp0.org_id = val;
		break;
	case CONF_DATA_SET_TAG:
		info->body.emp0.ds_tag = val;
		break;
	case CONF_DATA_SET_LENGTH:
		info->body.emp0.ds_length = val;
		break;
	case CONF_VRR_EN:
		info->body.emp0.md.game_md.vrr_en = !!val;
		break;
	case CONF_FACTOR_M1:
		info->body.emp0.md.game_md.fva_factor_m1 = val;
		break;
	case CONF_QMS_EN:
		info->body.emp0.md.qms_md.qms_en = !!val;
		break;
	case CONF_M_CONST:
		info->body.emp0.md.qms_md.m_const = !!val;
		break;
	case CONF_BASE_VFRONT:
		info->body.emp0.md.qms_md.base_vfront = val;
		break;
	case CONF_NEXT_TFR:
		info->body.emp0.md.qms_md.next_tfr = val;
		break;
	case CONF_BASE_REFRESH_RATE:
		info->body.emp0.md.qms_md.brr_rate = val & 0x3ff;
		break;
	case CONF_SBTM_VER:
		info->body.emp0.md.sbtm_md.sbtm_ver = val & 0xf;
		break;
	case CONF_SBTM_MODE:
		info->body.emp0.md.sbtm_md.sbtm_mode = val & 0x3;
		break;
	case CONF_SBTM_TYPE:
		info->body.emp0.md.sbtm_md.sbtm_type = val & 0x3;
		break;
	case CONF_SBTM_GRDM_MIN:
		info->body.emp0.md.sbtm_md.grdm_min = val & 0x3;
		break;
	case CONF_SBTM_GRDM_LUM:
		info->body.emp0.md.sbtm_md.grdm_lum = val & 0x3;
		break;
	case CONF_SBTM_FRMPBLIMITINT:
		info->body.emp0.md.sbtm_md.frmpblimitint = val & 0x3fff;
		break;
	default:
		break;
	}
}

int hdmi_emp_infoframe_get(enum emp_type type, u8 *body)
{
	int ret;
	u16 pkt_type;

	if (!body)
		return -1;

	pkt_type = (HDMI_INFOFRAME_TYPE_EMP << 8) | type;
	ret = hdmitx_infoframe_rawget(pkt_type, (u8 *)body);
	return ret;
}

/* for only 8bit */
void hdmi_gcppkt_manual_set(bool en)
{
	u8 body[31] = {0};

	body[0] = HDMI_PACKET_TYPE_GCP;
	if (en)
		hdmitx_infoframe_send(HDMI_PACKET_TYPE_GCP, body);
	else
		hdmitx_infoframe_send(HDMI_PACKET_TYPE_GCP, NULL);
}

void hdmi_sbtm_infoframe_rawset(u8 *hb, u8 *pb)
{
	u8 body[31] = {0};

	if (!hb || !pb) {
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_SBTM, NULL);
		return;
	}

	memcpy(body, hb, 3);
	memcpy(&body[3], pb, 28);
	hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_SBTM, body);
}

static struct emp_packet_st pkt_sbtm;
void hdmitx_set_sbtm_pkt(struct vtem_sbtm_st *data)
{
	struct emp_packet_st *sbtm = &pkt_sbtm;

	if (global_tx_common->tx_hw->chip_data->chip_type < MESON_CPU_ID_T7)
		return;
	if (!data) {
		hdmi_emp_infoframe_set(EMP_TYPE_SBTM, NULL);
		return;
	}
	memset(sbtm, 0, sizeof(*sbtm));
	sbtm->type = EMP_TYPE_SBTM;
	hdmi_emp_frame_set_member(sbtm, CONF_HEADER_INIT, HDMI_INFOFRAME_TYPE_EMP);
	hdmi_emp_frame_set_member(sbtm, CONF_HEADER_FIRST, 1);
	hdmi_emp_frame_set_member(sbtm, CONF_HEADER_LAST, 1);
	hdmi_emp_frame_set_member(sbtm, CONF_HEADER_SEQ_INDEX, 0);
	hdmi_emp_frame_set_member(sbtm, CONF_DS_TYPE, 1);
	hdmi_emp_frame_set_member(sbtm, CONF_SYNC, 1);
	hdmi_emp_frame_set_member(sbtm, CONF_VFR, 1);
	hdmi_emp_frame_set_member(sbtm, CONF_AFR, 0);
	hdmi_emp_frame_set_member(sbtm, CONF_NEW, 0);
	hdmi_emp_frame_set_member(sbtm, CONF_END, 0);
	hdmi_emp_frame_set_member(sbtm, CONF_ORG_ID, 1);
	hdmi_emp_frame_set_member(sbtm, CONF_DATA_SET_TAG, 3);
	hdmi_emp_frame_set_member(sbtm, CONF_DATA_SET_LENGTH, 4);
	hdmi_emp_frame_set_member(sbtm, CONF_SBTM_VER, data->sbtm_ver);
	hdmi_emp_frame_set_member(sbtm, CONF_SBTM_MODE, data->sbtm_mode);
	hdmi_emp_frame_set_member(sbtm, CONF_SBTM_TYPE, data->sbtm_type);
	hdmi_emp_frame_set_member(sbtm, CONF_SBTM_GRDM_MIN, data->grdm_min);
	hdmi_emp_frame_set_member(sbtm, CONF_SBTM_GRDM_LUM, data->grdm_lum);
	hdmi_emp_frame_set_member(sbtm, CONF_SBTM_FRMPBLIMITINT, data->frmpblimitint);
	hdmi_emp_infoframe_set(EMP_TYPE_SBTM, sbtm);
}
#endif

