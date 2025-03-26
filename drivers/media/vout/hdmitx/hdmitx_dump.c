// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/media/vout/vinfo.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include "hdmitx_dump.h"
#include "hdmitx_check_valid.h"

int dump_hdmitx_basic_config(struct seq_file *s, void *p)
{
	struct hdmitx_common *tx_comm = s->private;
	struct hdmitx_hw_common *hw_comm  = tx_comm->tx_hw;
	u8 *conf;
	u8 *tmp;
	u32 colormetry;
	u32 hcnt, vcnt;
	char bksv_buf[5];
	int cs, cd, i;
	char buf[512];
	enum hdmi_vic vic;
	enum hdmi_hdr_transfer hdr_transfer_feature;
	enum hdmi_hdr_color hdr_color_feature;
	struct dv_vsif_para *data;
	enum hdmi_tf_type type = HDMI_NONE;
	u32 hdcp_ret = 0;
	u32 hdcp_lstore = 0;

	seq_puts(s, "\n--------basic config--------\n");
	seq_puts(s, "************\n");
	seq_puts(s, "hdmi_config_info\n");
	seq_puts(s, "************\n");

	seq_printf(s, "display_mode in:%s\n", get_vout_mode_internal());

	vic = hdmitx_hw_get_state(hw_comm, STAT_VIDEO_VIC, 0);
	seq_printf(s, "display_mode out:%s\n", hdmitx_mode_get_timing_name(vic));

	seq_printf(s, "attr in:%s\n\r", tx_comm->tst_fmt_attr);
	seq_puts(s, "attr out:");
	cs = hdmitx_hw_get_state(hw_comm, STAT_VIDEO_CS, 0);
	switch (cs & 0x3) {
	case 0:
		conf = "RGB";
		break;
	case 1:
		conf = "422";
		break;
	case 2:
		conf = "444";
		break;
	case 3:
		conf = "420";
		break;
	}
	seq_printf(s, "%s,", conf);

	cd = hdmitx_hw_get_state(hw_comm, STAT_VIDEO_CD, 0);
	switch (cd) {
	case 4:
		conf = "8bit";
		break;
	case 5:
		conf = "10bit";
		break;
	case 6:
		conf = "12bit";
		break;
	case 7:
		conf = "16bit";
		break;
	default:
		conf = "reserved";
	}
	seq_printf(s, "%s\n", conf);

	seq_puts(s, "hdr_status:");
	type = hdmitx_hw_get_state(hw_comm, STAT_TX_HDR10P, 0);
	if (type) {
		if (type == HDMI_HDR10P_DV_VSIF)
			seq_puts(s, "HDR10Plus-VSIF");
	}
	type = hdmitx_hw_get_state(hw_comm, STAT_TX_DV, 0);
	if (type) {
		if (type == HDMI_DV_VSIF_STD)
			seq_puts(s, "DolbyVision-Std");
		else if (type == HDMI_DV_VSIF_LL)
			seq_puts(s, "DolbyVision-Lowlatency");
	}
	type = hdmitx_hw_get_state(hw_comm, STAT_TX_HDR, 0);
	if (type) {
		if (type == HDMI_HDR_SMPTE_2084)
			seq_puts(s, "HDR10-GAMMA_ST2084");
		else if (type == HDMI_HDR_HLG)
			seq_puts(s, "HDR10-GAMMA_HLG");
		else if (type == HDMI_HDR_HDR)
			seq_puts(s, "HDR10-others");
		else if (type == HDMI_HDR_SDR)
			seq_puts(s, "SDR");
	}
	if (type == HDMI_NONE)
		/* default is SDR */
		seq_puts(s, "SDR");
	seq_puts(s, "\n");

	seq_puts(s, "******config******\n");
	seq_printf(s, "cur_VIC: %d\n", tx_comm->fmt_para.vic);
	hdmitx_format_para_print(&tx_comm->fmt_para, buf);
	seq_printf(s, "%s\n", buf);

	if (tx_comm->cur_audio_param.aud_output_en)
		conf = "on";
	else
		conf = "off";
	seq_printf(s, "audio config: %s\n", conf);

	switch (tx_comm->cur_audio_param.aud_src_if) {
	case AUD_SRC_IF_SPDIF:
		conf = "SPDIF";
		break;
	case AUD_SRC_IF_I2S:
		conf = "I2S";
		break;
	case AUD_SRC_IF_TDM:
		conf = "TDM";
		break;
	default:
		conf = "none";
	}
	seq_printf(s, "audio source: %s\n", conf);

	switch (tx_comm->cur_audio_param.type) {
	case CT_REFER_TO_STREAM:
		conf = "refer to stream header";
		break;
	case CT_PCM:
		conf = "L-PCM";
		break;
	case CT_AC_3:
		conf = "AC-3";
		break;
	case CT_MPEG1:
		conf = "MPEG1";
		break;
	case CT_MP3:
		conf = "MP3";
		break;
	case CT_MPEG2:
		conf = "MPEG2";
		break;
	case CT_AAC:
		conf = "AAC";
		break;
	case CT_DTS:
		conf = "DTS";
		break;
	case CT_ATRAC:
		conf = "ATRAC";
		break;
	case CT_ONE_BIT_AUDIO:
		conf = "One Bit Audio";
		break;
	case CT_DD_P:
		conf = "Dobly Digital+";
		break;
	case CT_DTS_HD:
		conf = "DTS_HD";
		break;
	case CT_MAT:
		conf = "MAT";
		break;
	case CT_DST:
		conf = "DST";
		break;
	case CT_WMA:
		conf = "WMA";
		break;
	default:
		conf = "MAX";
	}
	seq_printf(s, "audio type: %s\n", conf);

	switch (tx_comm->cur_audio_param.chs) {
	case CC_REFER_TO_STREAM:
		conf = "refer to stream header";
		break;
	case CC_2CH:
		conf = "2 channels";
		break;
	case CC_3CH:
		conf = "3 channels";
		break;
	case CC_4CH:
		conf = "4 channels";
		break;
	case CC_5CH:
		conf = "5 channels";
		break;
	case CC_6CH:
		conf = "6 channels";
		break;
	case CC_7CH:
		conf = "7 channels";
		break;
	case CC_8CH:
		conf = "8 channels";
		break;
	default:
		conf = "MAX";
	}
	seq_printf(s, "audio channel num: %s\n", conf);

	switch (tx_comm->cur_audio_param.rate) {
	case FS_REFER_TO_STREAM:
		conf = "refer to stream header";
		break;
	case FS_32K:
		conf = "32kHz";
		break;
	case FS_44K1:
		conf = "44.1kHz";
		break;
	case FS_48K:
		conf = "48kHz";
		break;
	case FS_88K2:
		conf = "88.2kHz";
		break;
	case FS_96K:
		conf = "96kHz";
		break;
	case FS_176K4:
		conf = "176.4kHz";
		break;
	case FS_192K:
		conf = "192kHz";
		break;
	case FS_768K:
		conf = "768kHz";
		break;
	default:
		conf = "MAX";
	}
	seq_printf(s, "audio sample rate: %s\n", conf);

	switch (tx_comm->cur_audio_param.size) {
	case SS_REFER_TO_STREAM:
		conf = "refer to stream header";
		break;
	case SS_16BITS:
		conf = "16bit";
		break;
	case SS_20BITS:
		conf = "20bit";
		break;
	case SS_24BITS:
		conf = "24bit";
		break;
	default:
		conf = "MAX";
	}
	seq_printf(s, "audio sample size: %s\n", conf);

	if (tx_comm->flag_3dfp)
		conf = "FramePacking";
	else if (tx_comm->flag_3dss)
		conf = "SidebySide";
	else if (tx_comm->flag_3dtb)
		conf = "TopButtom";
	else
		conf = "off";
	seq_printf(s, "3D config: %s\n", conf);
	seq_puts(s, "\n");

	seq_puts(s, "******hdcp******\n");
	seq_puts(s, "hdcp mode:");
	switch (tx_comm->hdcp_mode) {
	case 1:
		seq_puts(s, "14");
		break;
	case 2:
		seq_puts(s, "22");
		break;
	default:
		seq_puts(s, "off");
		break;
	}
	if (tx_comm->hdcp_mode > 0) {
		hdcp_ret = hdmitx_hw_cntl_ddc(hw_comm,
						      DDC_HDCP_GET_AUTH, 0);
		if (hdcp_ret == 1)
			seq_puts(s, ": succeed\n");
		else
			seq_puts(s, ": fail\n");
	}

	seq_puts(s, "hdcp_lstore:");
	/* if current TX is RP-TX, then return lstore as 00 */
	/* hdcp_lstore is used under only TX */
	hdcp_lstore = hw_comm->lstore;
	if (hdcp_lstore < 0x10) {
		hdcp_lstore = 0;
		if (hdmitx_hw_cntl_ddc(hw_comm, DDC_HDCP_14_LSTORE, 0))
			hdcp_lstore |= BIT(0);
		else
			hdmitx_current_status(tx_comm, HDMITX_HDCP_AUTH_NO_14_KEYS_ERROR);
		if (hdmitx_hw_cntl_ddc(hw_comm,
			DDC_HDCP_22_LSTORE, 0))
			hdcp_lstore |= BIT(1);
		else
			hdmitx_current_status(tx_comm, HDMITX_HDCP_AUTH_NO_22_KEYS_ERROR);
	}
	if ((hdcp_lstore & 0x3) == 0x3) {
		seq_puts(s, "14+22\n");
	} else {
		if (hdcp_lstore & 0x1)
			seq_puts(s, "14\n");
		if (hdcp_lstore & 0x2)
			seq_puts(s, "22\n");
		if ((hdcp_lstore & 0xf) == 0)
			seq_puts(s, "00\n");
	}

	seq_puts(s, "hdcp_ver:");
	hdmitx_get_hdcp_ver(tx_comm, buf, 512);
	seq_printf(s, "%s\n", buf);
	hdmitx_hw_cntl_ddc(hw_comm, DDC_HDCP_GET_BKSV,
			(unsigned long)bksv_buf);
	seq_puts(s, "HDCP14 BKSV: ");
	for (i = 0; i < 5; i++)
		seq_printf(s, "%02x", bksv_buf[i]);

	seq_printf(s, "hdcp_ctl_lvl:%d\n", tx_comm->hdcp_ctl_lvl);

	seq_puts(s, "******scdc******\n");
	seq_printf(s, "div40:%d\n", tx_comm->pre_tmds_clk_div40);

	seq_puts(s, "******hdmi_pll******\n");
	seq_printf(s, "sspll:%d\n", tx_comm->sspll);

	seq_puts(s, "******dv_vsif_info******\n");
	data = &tx_comm->vsif_debug_info.data;
	seq_printf(s, "type: %u, tunnel: %u, sigsdr: %u\n",
		tx_comm->vsif_debug_info.type,
		tx_comm->vsif_debug_info.tunnel_mode,
		tx_comm->vsif_debug_info.signal_sdr);
	seq_puts(s, "dv_vsif_para:\n");
	seq_printf(s, "ver: %u len: %u\n", data->ver, data->length);
	seq_printf(s, "ll: %u dvsig: %u\n", data->vers.ver2.low_latency,
		data->vers.ver2.dobly_vision_signal);
	seq_printf(s, "bcMD: %u axMD: %u\n", data->vers.ver2.backlt_ctrl_MD_present,
		data->vers.ver2.auxiliary_MD_present);
	seq_printf(s, "PQhi: %u PQlow: %u\n", data->vers.ver2.eff_tmax_PQ_hi,
		data->vers.ver2.eff_tmax_PQ_low);
	seq_printf(s, "auxiliary_runmode: %u, auxiliary_runversion: %u, ",
		data->vers.ver2.auxiliary_runmode,
		data->vers.ver2.auxiliary_runversion);
	seq_printf(s, "axdbg: %u\n", data->vers.ver2.auxiliary_debug0);
	seq_puts(s, "\n");

	seq_puts(s, "***drm_config_data***\n");
	hdr_transfer_feature = (tx_comm->drm_config_data.features >> 8) & 0xff;
	hdr_color_feature = (tx_comm->drm_config_data.features >> 16) & 0xff;
	colormetry = (tx_comm->drm_config_data.features >> 30) & 0x1;
	seq_printf(s, "tf=%u, cf=%u, colormetry=%u\n", hdr_transfer_feature, hdr_color_feature,
		colormetry);
	seq_puts(s, "primaries:\n");
	for (vcnt = 0; vcnt < 3; vcnt++) {
		for (hcnt = 0; hcnt < 2; hcnt++)
			seq_printf(s, "%u, ",
			tx_comm->drm_config_data.primaries[vcnt][hcnt]);
		seq_puts(s, "\n");
	}
	seq_puts(s, "white_point: ");
	for (hcnt = 0; hcnt < 2; hcnt++)
		seq_printf(s, "%u, ", tx_comm->drm_config_data.white_point[hcnt]);
	seq_puts(s, "\n");
	seq_puts(s, "luminance: ");
	for (hcnt = 0; hcnt < 2; hcnt++)
		seq_printf(s, "%u, ",
		tx_comm->drm_config_data.luminance[hcnt]);
	seq_puts(s, "\n");
	seq_printf(s, "max_content: %u, ",
		tx_comm->drm_config_data.max_content);
	seq_printf(s, "max_frame_average: %u\n",
		tx_comm->drm_config_data.max_frame_average);
	seq_puts(s, "\n");

	seq_puts(s, "***hdr10p_config_data***\n");
	seq_printf(s, "appver: %u, tlum: %u, avgrgb: %u\n",
		tx_comm->hdr10p_config_data.application_version,
		tx_comm->hdr10p_config_data.targeted_max_lum,
		tx_comm->hdr10p_config_data.average_maxrgb);
	tmp = tx_comm->hdr10p_config_data.distribution_values;
	seq_puts(s, "distribution_values:\n");
	for (vcnt = 0; vcnt < 3; vcnt++) {
		for (hcnt = 0; hcnt < 3; hcnt++)
			seq_printf(s, "%u, ", tmp[vcnt * 3 + hcnt]);
		seq_puts(s, "\n");
	}
	seq_printf(s, "nbca: %u, knpx: %u, knpy: %u\n",
		tx_comm->hdr10p_config_data.num_bezier_curve_anchors,
		tx_comm->hdr10p_config_data.knee_point_x,
		tx_comm->hdr10p_config_data.knee_point_y);
	tmp = tx_comm->hdr10p_config_data.bezier_curve_anchors;
	seq_puts(s, "bezier_curve_anchors:\n");
	for (vcnt = 0; vcnt < 3; vcnt++) {
		for (hcnt = 0; hcnt < 3; hcnt++)
			seq_printf(s, "%u, ", tmp[vcnt * 3 + hcnt]);
		seq_puts(s, "\n");
	}
	seq_printf(s, "gof: %u, ndf: %u\n",
		tx_comm->hdr10p_config_data.graphics_overlay_flag,
		tx_comm->hdr10p_config_data.no_delay_flag);
	seq_puts(s, "\n");

	seq_puts(s, "***hdmiaud_config_data***\n");
	seq_printf(s,
		"type: %u, chnum: %u, samrate: %u, samsize: %u\n",
		tx_comm->hdmiaud_config_data.type,
		tx_comm->hdmiaud_config_data.chs,
		tx_comm->hdmiaud_config_data.rate,
		tx_comm->hdmiaud_config_data.size);

	return 0;
}

int hdmirx_info_show(struct seq_file *s, void *v)
{
	int i, j;
	int num;
	int block_num = 0;
	char *buf = NULL;
	int len;
	struct hdmitx_common *tx_comm = s->private;
	struct rx_cap *prxcap = &tx_comm->rxcap;
	const struct dv_info *dv = &prxcap->dv_info;
	const struct hdr_info *hdr = &prxcap->hdr_info;
	const struct cuva_info *cuva = &hdr->cuva_info;
	const struct hdr10_plus_info *hdr10p = &hdr->hdr10plus_info;
	const struct sbtm_info *sbtm = &hdr->sbtm_info;
	const struct hdmi_timing *timing = NULL;
	const char *mode_name;
	enum hdmi_vic vic;
	int vic_len = prxcap->VIC_count + VESA_MAX_TIMING;
	int *edid_vics = vmalloc(vic_len * sizeof(int));
	enum hdmi_vic prefer_vic = HDMI_0_UNKNOWN;
	int hdr10plus_supported = 0;

	len = 2048;
	buf = kcalloc(len, sizeof(char), GFP_KERNEL);
	if (!buf) {
		HDMITX_ERROR("%s kcalloc failed\n", __func__);
		return 0;
	}

	seq_puts(s, "************hdmirx_info************\n");

	seq_puts(s, "******hpd_edid_parsing******\n");
	seq_printf(s, "hpd: %d", tx_comm->hpd_state);

	seq_puts(s, "\n******rawedid******\n");

	block_num = hdmitx_edid_valid_block_num(tx_comm->EDID_buf);
	if (block_num <= 8)
		num = block_num * 128;
	else
		num = 8 * 128;

	for (i = 0; i < num; i++)
		seq_printf(s, "%02x",
			tx_comm->EDID_buf[i]);

	seq_puts(s, "\n");

	seq_puts(s, "\nedid_parsing:");
	if (hdmitx_edid_check_data_valid(tx_comm->rxcap.edid_check,
		tx_comm->EDID_buf))
		seq_puts(s, "ok\n");
	else
		seq_puts(s, "ng\n");

	seq_puts(s, "\n******edid******\n");
	hdmitx_edid_print_sink_cap(&tx_comm->rxcap, buf, len);
	seq_printf(s, buf);
	memset(buf, 0, len * sizeof(char));

	seq_puts(s, "\n******dc_cap******\n");

	/* DVI case, only rgb,8bit */
	if (prxcap->ieeeoui != HDMI_IEEE_OUI) {
		seq_puts(s, "rgb,8bit\n");
	} else {
		if (prxcap->dc_36bit_420)
			seq_puts(s, "420,12bit\n");
		if (prxcap->dc_30bit_420)
			seq_puts(s, "420,10bit\n");

		for (i = 0; i < Y420_VIC_MAX_NUM; i++) {
			if (prxcap->y420_vic[i]) {
				seq_puts(s, "420,8bit\n");
				break;
			}
		}

		if (prxcap->native_Mode & (1 << 5)) {
			if (prxcap->dc_y444) {
				if (prxcap->dc_36bit || dv->sup_10b_12b_444 == 0x2)
					seq_puts(s, "444,12bit\n");
				if (prxcap->dc_30bit || dv->sup_10b_12b_444 == 0x1)
					seq_puts(s, "444,10bit\n");
			}
			seq_puts(s, "444,8bit\n");
		}
		/* y422, not check dc */
		if (prxcap->native_Mode & (1 << 4))
			seq_puts(s, "422,12bit\n");

		if (prxcap->dc_36bit || dv->sup_10b_12b_444 == 0x2)
			seq_puts(s, "rgb,12bit\n");
		if (prxcap->dc_30bit || dv->sup_10b_12b_444 == 0x1)
			seq_puts(s, "rgb,10bit\n");
		seq_puts(s, "rgb,8bit\n");
	}

	seq_puts(s, "\n******disp_cap******\n");
	mutex_lock(&tx_comm->valid_mutex);
	memset(edid_vics, 0, vic_len * sizeof(int));
	/* step1: only select VIC which is supported in EDID */
	/* copy edid vic list */
	if (prxcap->VIC_count > 0)
		memcpy(edid_vics, prxcap->VIC, sizeof(int) * prxcap->VIC_count);
	for (i = 0; i < VESA_MAX_TIMING && prxcap->vesa_timing[i]; i++)
		edid_vics[prxcap->VIC_count + i] = prxcap->vesa_timing[i];
	for (i = 0; i < vic_len; i++) {
		vic = edid_vics[i];
		if (vic == HDMI_0_UNKNOWN)
			continue;
		prefer_vic = hdmitx_get_prefer_vic(tx_comm, vic);
		/* if mode_best_vic is support by RX, try 16x9 first */
		if (prefer_vic != vic) {
			HDMITX_DEBUG("%s: check prefer vic:%d exist, ignore [%d].\n",
					__func__, prefer_vic, vic);
			continue;
		}
		timing = hdmitx_mode_vic_to_hdmi_timing(vic);
		if (!timing) {
			/* HDMITX_ERROR("%s: unsupport vic [%d]\n", __func__, vic); */
			continue;
		}
		/* step2, check if VIC is supported by SOC hdmitx */
		if (hdmitx_common_validate_vic(tx_comm, vic) != 0) {
			/* HDMITX_ERROR("%s: vic[%d] over range.\n", __func__, vic); */
			continue;
		}
		/*
		 * step3, build format with basic mode/attr and check
		 * if it's supported by EDID/hdmitx_cap
		 */
		if (hdmitx_common_check_valid_para_of_vic(tx_comm, vic) != 0) {
			/* HDMITX_ERROR("%s: vic[%d] check fmt attr failed.\n", __func__, vic); */
			continue;
		}
		mode_name = timing->sname ? timing->sname : timing->name;
		seq_printf(s, "%s", mode_name);
		if (vic == prxcap->native_vic)
			seq_puts(s, "*\n");
		else
			seq_puts(s, "\n");
	}
	vfree(edid_vics);
	mutex_unlock(&tx_comm->valid_mutex);

	seq_puts(s, "\n******dv_cap******\n");
	if (dv->ieeeoui != DV_IEEE_OUI || dv->block_flag != CORRECT) {
		seq_puts(s, "The Rx don't support DolbyVision\n");
	} else {
		seq_puts(s, "DolbyVision RX support list:\n");

		if (dv->ver == 0) {
			seq_printf(s, "VSVDB Version: V%d\n", dv->ver);
			seq_printf(s, "2160p%shz: 1\n", dv->sup_2160p60hz ? "60" : "30");
			seq_puts(s, "Support mode:\n");
			seq_puts(s, "  DV_RGB_444_8BIT\n");
			if (dv->sup_yuv422_12bit)
				seq_puts(s, "  DV_YCbCr_422_12BIT\n");
		}
		if (dv->ver == 1) {
			seq_printf(s, "VSVDB Version: V%d(%d-byte)\n",
				dv->ver, dv->length + 1);
			if (dv->length == 0xB) {
				seq_printf(s, "2160p%shz: 1\n",
					dv->sup_2160p60hz ? "60" : "30");
			seq_puts(s, "Support mode:\n");
			seq_puts(s, "  DV_RGB_444_8BIT\n");
			if (dv->sup_yuv422_12bit)
				seq_puts(s, "  DV_YCbCr_422_12BIT\n");
			if (dv->low_latency == 0x01)
				seq_puts(s, "  LL_YCbCr_422_12BIT\n");
			}

			if (dv->length == 0xE) {
				seq_printf(s, "2160p%shz: 1\n",
					dv->sup_2160p60hz ? "60" : "30");
				seq_puts(s, "Support mode:\n");
				seq_puts(s, "  DV_RGB_444_8BIT\n");
				if (dv->sup_yuv422_12bit)
					seq_puts(s, "  DV_YCbCr_422_12BIT\n");
			}
		}
		if (dv->ver == 2) {
			seq_printf(s, "VSVDB Version: V%d\n", dv->ver);
			seq_printf(s, "2160p%shz: 1\n",
				dv->sup_2160p60hz ? "60" : "30");
			seq_printf(s, "Parity: %d\n", dv->parity);
			seq_puts(s, "Support mode:\n");
			if (dv->Interface != 0x00 && dv->Interface != 0x01) {
				seq_puts(s, "  DV_RGB_444_8BIT\n");
				if (dv->sup_yuv422_12bit)
					seq_puts(s, "  DV_YCbCr_422_12BIT\n");
			}
			seq_puts(s, "  LL_YCbCr_422_12BIT\n");
			if (dv->Interface == 0x01 || dv->Interface == 0x03) {
				if (dv->sup_10b_12b_444 == 0x1)
					seq_puts(s, "  LL_RGB_444_10BIT\n");
				if (dv->sup_10b_12b_444 == 0x2)
					seq_puts(s, "  LL_RGB_444_12BIT\n");
			}
		}
		seq_printf(s,
			"IEEEOUI: 0x%06x\n", dv->ieeeoui);
		seq_printf(s, "EMP: %d\n", dv->dv_emp_cap);
		seq_puts(s, "VSVDB: ");
		for (i = 0; i < (dv->length + 1); i++)
			seq_printf(s, "%02x", dv->rawdata[i]);
		seq_puts(s, "\n");
	}

	seq_puts(s, "\n******hdr_cap******\n");
	if (hdr10p->ieeeoui == HDR10_PLUS_IEEE_OUI &&
			hdr10p->application_version != 0xFF)
		hdr10plus_supported = 1;
	seq_printf(s, "HDR10Plus Supported: %d\n",
		hdr10plus_supported);
	seq_puts(s, "HDR Static Metadata:\n");
	seq_puts(s, "    Supported EOTF:\n");
	seq_printf(s, "        Traditional SDR: %d\n",
		!!(hdr->hdr_support & 0x1));
	seq_printf(s, "        Traditional HDR: %d\n",
		!!(hdr->hdr_support & 0x2));
	seq_printf(s, "        SMPTE ST 2084: %d\n",
		!!(hdr->hdr_support & 0x4));
	seq_printf(s, "        Hybrid Log-Gamma: %d\n",
		!!(hdr->hdr_support & 0x8));
	seq_printf(s, "    Supported SMD type1: %d\n",
		hdr->static_metadata_type1);
	seq_puts(s, "    Luminance Data\n");
	seq_printf(s, "        Max: %d\n",
		hdr->lumi_max);
	seq_printf(s, "        Avg: %d\n",
		hdr->lumi_avg);
	seq_printf(s, "        Min: %d\n\n",
		hdr->lumi_min);
	seq_puts(s, "HDR Dynamic Metadata:");

	for (i = 0; i < 4; i++) {
		if (hdr->dynamic_info[i].type == 0)
			continue;
		seq_printf(s, "\n    metadata_version: %x\n",
			hdr->dynamic_info[i].type);
		seq_printf(s, "        support_flags: %x\n",
			hdr->dynamic_info[i].support_flags);
		seq_puts(s, "        optional_fields:");
		for (j = 0; j <
			(hdr->dynamic_info[i].of_len - 3); j++)
			seq_printf(s, " %x", hdr->dynamic_info[i].optional_fields[j]);
	}

	seq_printf(s, "\n\ncolorimetry_data: %x\n",
		hdr->colorimetry_support);
	if (cuva->ieeeoui == CUVA_IEEEOUI) {
		seq_puts(s, "CUVA supported: 1\n");
		seq_printf(s, "  system_start_code: %u\n", cuva->system_start_code);
		seq_printf(s, "  version_code: %u\n", cuva->version_code);
		seq_printf(s, "  display_maximum_luminance: %u\n", cuva->display_max_lum);
		seq_printf(s, "  display_minimum_luminance: %u\n", cuva->display_min_lum);
		seq_printf(s, "  monitor_mode_support: %u\n", cuva->monitor_mode_sup);
		seq_printf(s, "  rx_mode_support: %u\n", cuva->rx_mode_sup);
		for (i = 0; i < (cuva->length + 1); i++)
			seq_printf(s, "%02x", cuva->rawdata[i]);
		seq_puts(s, "\n");
	}
	/* sbtm capability show */
	if (sbtm->sbtm_support) {
		seq_puts(s, "SBTM supported: 1\n");
		if (sbtm->max_sbtm_ver)
			seq_printf(s, "  Max_SBTM_Ver: 0x%x\n",
				sbtm->max_sbtm_ver);
		if (sbtm->grdm_support)
			seq_printf(s, "  grdm_support: 0x%x\n",
				sbtm->grdm_support);
		if (sbtm->drdm_ind)
			seq_printf(s, "  drdm_ind: 0x%x\n",
				sbtm->drdm_ind);
		if (sbtm->hgig_cat_drdm_sel)
			seq_printf(s, "  hgig_cat_drdm_sel: 0x%x\n",
				sbtm->hgig_cat_drdm_sel);
		if (sbtm->use_hgig_drdm)
			seq_printf(s, "  use_hgig_drdm: 0x%x\n",
				sbtm->use_hgig_drdm);
		if (sbtm->maxrgb)
			seq_printf(s, "  maxrgb: 0x%x\n", sbtm->maxrgb);
		if (sbtm->gamut)
			seq_printf(s, "  gamut: 0x%x\n", sbtm->gamut);
		if (sbtm->red_x)
			seq_printf(s, "  red_x: 0x%x\n", sbtm->red_x);
		if (sbtm->red_y)
			seq_printf(s, "  red_y: 0x%x\n", sbtm->red_y);
		if (sbtm->green_x)
			seq_printf(s, "  green_x: 0x%x\n", sbtm->green_x);
		if (sbtm->green_y)
			seq_printf(s, "  green_y: 0x%x\n", sbtm->green_y);
		if (sbtm->blue_x)
			seq_printf(s, "  blue_x: 0x%x\n", sbtm->blue_x);
		if (sbtm->blue_y)
			seq_printf(s, "  blue_y: 0x%x\n", sbtm->blue_y);
		if (sbtm->white_x)
			seq_printf(s, "  white_x: 0x%x\n", sbtm->white_x);
		if (sbtm->white_y)
			seq_printf(s, "  white_y: 0x%x\n", sbtm->white_y);
		if (sbtm->min_bright_10)
			seq_printf(s, "  min_bright_10: 0x%x\n",
				sbtm->min_bright_10);
		if (sbtm->peak_bright_100)
			seq_printf(s, "  peak_bright_100: 0x%x\n",
				sbtm->peak_bright_100);
		if (sbtm->p0_exp)
			seq_printf(s, "  p0_exp: 0x%x\n", sbtm->p0_exp);
		if (sbtm->p0_mant)
			seq_printf(s, "  p0_mant: 0x%x\n", sbtm->p0_mant);
		if (sbtm->peak_bright_p0)
			seq_printf(s, "  peak_bright_p0: 0x%x\n", sbtm->peak_bright_p0);
		if (sbtm->p1_exp)
			seq_printf(s, "  p1_exp: 0x%x\n", sbtm->p1_exp);
		if (sbtm->p1_mant)
			seq_printf(s, "  p1_mant: 0x%x\n", sbtm->p1_mant);
		if (sbtm->peak_bright_p1)
			seq_printf(s, "  peak_bright_p1: 0x%x\n",
				sbtm->peak_bright_p1);
		if (sbtm->p2_exp)
			seq_printf(s, "  p2_exp: 0x%x\n", sbtm->p2_exp);
		if (sbtm->p2_mant)
			seq_printf(s, "  p2_mant: 0x%x\n", sbtm->p2_mant);
		if (sbtm->peak_bright_p2)
			seq_printf(s, "  peak_bright_p2: 0x%x\n", sbtm->peak_bright_p2);
		if (sbtm->p3_exp)
			seq_printf(s, "  p3_exp: 0x%x\n", sbtm->p3_exp);
		if (sbtm->p3_mant)
			seq_printf(s, "  p3_mant: 0x%x\n", sbtm->p3_mant);
		if (sbtm->peak_bright_p3)
			seq_printf(s, "  peak_bright_p3: 0x%x\n", sbtm->peak_bright_p3);
	}

	seq_puts(s, "\n******aud_cap******\n");
	_show_aud_cap(prxcap, buf, len);
	seq_printf(s, buf);
	kfree(buf);

	return 0;
}

void hdmitx_common_debugfs_init(struct hdmitx_common *tx_comm)
{
	struct dentry *entry;
	struct hdmitx_dbg_files_s *dbg_files;
	struct hdmitx_hw_common *hw_comm = tx_comm->tx_hw;
	int count = 0;
	int i;

	dbg_files = hw_comm->chip_data->hdmitx_ops->get_dbg_files();
	count = hw_comm->chip_data->hdmitx_ops->get_dbg_files_count();
	if (!tx_comm->hdmitx_file_dbgfs)
		tx_comm->hdmitx_file_dbgfs = debugfs_create_dir(DEVICE_NAME, NULL);

	if (!tx_comm->hdmitx_file_dbgfs) {
		HDMITX_ERROR("can't create %s debugfs dir\n", DEVICE_NAME);
		return;
	}

	for (i = 0; i < count; i++) {
		entry = debugfs_create_file(dbg_files[i].name,
			dbg_files[i].mode,
			tx_comm->hdmitx_file_dbgfs, tx_comm,
			dbg_files[i].fops);
		if (!entry)
			HDMITX_ERROR("debugfs create file %s failed\n",
				dbg_files[i].name);
	}
}

void hdmitx_common_profs_init(struct hdmitx_common *tx_comm)
{
	struct proc_dir_entry *entry;
	struct hdmitx_dbg_files_s *dbg_files;
	struct hdmitx_hw_common *hw_comm = tx_comm->tx_hw;
	int count = 0;
	int i;

	dbg_files = hw_comm->chip_data->hdmitx_ops->get_dbg_files();
	count = hw_comm->chip_data->hdmitx_ops->get_dbg_files_count();
	if (!tx_comm->hdmitx_proc_dbgfs)
		tx_comm->hdmitx_proc_dbgfs = proc_mkdir("amhdmitx", NULL);
	if (!tx_comm->hdmitx_proc_dbgfs)
		HDMITX_ERROR("Error creating proc entry");

	for (i = 0; i < count; i++) {
		entry = proc_create_data(dbg_files[i].name,
			dbg_files[i].mode,
			tx_comm->hdmitx_proc_dbgfs,
			dbg_files[i].pops, tx_comm);
		if (!entry)
			HDMITX_ERROR("debugfs create file %s failed\n",
				dbg_files[i].name);
	}
}
