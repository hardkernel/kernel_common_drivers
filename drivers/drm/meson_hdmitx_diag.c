// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_types.h>
#include <drm/amlogic/meson_hdmi_diag.h>
#include "meson_hdmi.h"

static const char *hdmi_colorspace_get_name(enum hdmi_colorspace colorspace)
{
	switch (colorspace) {
	case HDMI_COLORSPACE_RGB:
		return "RGB";
	case HDMI_COLORSPACE_YUV422:
		return "YCbCr 422";
	case HDMI_COLORSPACE_YUV444:
		return "YCbCr 444";
	case HDMI_COLORSPACE_YUV420:
		return "YCbCr 420";
	case HDMI_COLORSPACE_RESERVED4:
		return "Reserved (4)";
	case HDMI_COLORSPACE_RESERVED5:
		return "Reserved (5)";
	case HDMI_COLORSPACE_RESERVED6:
		return "Reserved (6)";
	case HDMI_COLORSPACE_IDO_DEFINED:
		return "IDO Defined";
	}
	return "Invalid";
}

static const char *hdmi_scan_mode_get_name(enum hdmi_scan_mode scan_mode)
{
	switch (scan_mode) {
	case HDMI_SCAN_MODE_NONE:
		return "No Data";
	case HDMI_SCAN_MODE_OVERSCAN:
		return "Overscan";
	case HDMI_SCAN_MODE_UNDERSCAN:
		return "Underscan";
	case HDMI_SCAN_MODE_RESERVED:
		return "Reserved";
	}
	return "Invalid";
}

static const char *hdmi_colorimetry_get_name(enum hdmi_colorimetry colorimetry)
{
	switch (colorimetry) {
	case HDMI_COLORIMETRY_NONE:
		return "No Data";
	case HDMI_COLORIMETRY_ITU_601:
		return "ITU601";
	case HDMI_COLORIMETRY_ITU_709:
		return "ITU709";
	case HDMI_COLORIMETRY_EXTENDED:
		return "Extended";
	}
	return "Invalid";
}

static const char *
hdmi_picture_aspect_get_name(enum hdmi_picture_aspect picture_aspect)
{
	switch (picture_aspect) {
	case HDMI_PICTURE_ASPECT_NONE:
		return "No Data";
	case HDMI_PICTURE_ASPECT_4_3:
		return "4:3";
	case HDMI_PICTURE_ASPECT_16_9:
		return "16:9";
	case HDMI_PICTURE_ASPECT_64_27:
		return "64:27";
	case HDMI_PICTURE_ASPECT_256_135:
		return "256:135";
	case HDMI_PICTURE_ASPECT_RESERVED:
		return "Reserved";
	}
	return "Invalid";
}

static const char *
hdmi_active_aspect_get_name(enum hdmi_active_aspect active_aspect)
{
	if (active_aspect < 0 || active_aspect > 0xf)
		return "Invalid";

	switch (active_aspect) {
	case HDMI_ACTIVE_ASPECT_16_9_TOP:
		return "16:9 Top";
	case HDMI_ACTIVE_ASPECT_14_9_TOP:
		return "14:9 Top";
	case HDMI_ACTIVE_ASPECT_16_9_CENTER:
		return "16:9 Center";
	case HDMI_ACTIVE_ASPECT_PICTURE:
		return "Same as Picture";
	case HDMI_ACTIVE_ASPECT_4_3:
		return "4:3";
	case HDMI_ACTIVE_ASPECT_16_9:
		return "16:9";
	case HDMI_ACTIVE_ASPECT_14_9:
		return "14:9";
	case HDMI_ACTIVE_ASPECT_4_3_SP_14_9:
		return "4:3 SP 14:9";
	case HDMI_ACTIVE_ASPECT_16_9_SP_14_9:
		return "16:9 SP 14:9";
	case HDMI_ACTIVE_ASPECT_16_9_SP_4_3:
		return "16:9 SP 4:3";
	}
	return "Reserved";
}

static const char *
hdmi_extended_colorimetry_get_name(enum hdmi_extended_colorimetry ext_col)
{
	switch (ext_col) {
	case HDMI_EXTENDED_COLORIMETRY_XV_YCC_601:
		return "xvYCC 601";
	case HDMI_EXTENDED_COLORIMETRY_XV_YCC_709:
		return "xvYCC 709";
	case HDMI_EXTENDED_COLORIMETRY_S_YCC_601:
		return "sYCC 601";
	case HDMI_EXTENDED_COLORIMETRY_OPYCC_601:
		return "opYCC 601";
	case HDMI_EXTENDED_COLORIMETRY_OPRGB:
		return "opRGB";
	case HDMI_EXTENDED_COLORIMETRY_BT2020_CONST_LUM:
		return "BT.2020 YCBCCRC";
	case HDMI_EXTENDED_COLORIMETRY_BT2020:
		return "BT.2020 RGB_OR_YCBCR";
	case HDMI_EXTENDED_COLORIMETRY_RESERVED:
		return "ADDITIONAL_EXTENDED_COLORIMETRY";
	}
	return "Invalid";
}

static const char *
hdmi_quantization_range_get_name(enum hdmi_quantization_range qrange)
{
	switch (qrange) {
	case HDMI_QUANTIZATION_RANGE_DEFAULT:
		return "Default";
	case HDMI_QUANTIZATION_RANGE_LIMITED:
		return "Limited";
	case HDMI_QUANTIZATION_RANGE_FULL:
		return "Full";
	case HDMI_QUANTIZATION_RANGE_RESERVED:
		return "Reserved";
	}
	return "Invalid";
}

static const char *hdmi_nups_get_name(enum hdmi_nups nups)
{
	switch (nups) {
	case HDMI_NUPS_UNKNOWN:
		return "Unknown Non-uniform Scaling";
	case HDMI_NUPS_HORIZONTAL:
		return "Horizontally Scaled";
	case HDMI_NUPS_VERTICAL:
		return "Vertically Scaled";
	case HDMI_NUPS_BOTH:
		return "Horizontally and Vertically Scaled";
	}
	return "Invalid";
}

static const char *
hdmi_ycc_quantization_range_get_name(enum hdmi_ycc_quantization_range qrange)
{
	switch (qrange) {
	case HDMI_YCC_QUANTIZATION_RANGE_LIMITED:
		return "Limited";
	case HDMI_YCC_QUANTIZATION_RANGE_FULL:
		return "Full";
	}
	return "Invalid";
}

static const char *
hdmi_content_type_get_name(enum hdmi_content_type content_type)
{
	switch (content_type) {
	case HDMI_CONTENT_TYPE_GRAPHICS:
		return "Graphics";
	case HDMI_CONTENT_TYPE_PHOTO:
		return "Photo";
	case HDMI_CONTENT_TYPE_CINEMA:
		return "Cinema";
	case HDMI_CONTENT_TYPE_GAME:
		return "Game";
	}
	return "Invalid";
}

static const char *
hdmi_audio_coding_type_get_name(enum hdmi_audio_coding_type coding_type)
{
	switch (coding_type) {
	case HDMI_AUDIO_CODING_TYPE_STREAM:
		return "Refer to Stream Header";
	case HDMI_AUDIO_CODING_TYPE_PCM:
		return "PCM";
	case HDMI_AUDIO_CODING_TYPE_AC3:
		return "AC-3";
	case HDMI_AUDIO_CODING_TYPE_MPEG1:
		return "MPEG1";
	case HDMI_AUDIO_CODING_TYPE_MP3:
		return "MP3";
	case HDMI_AUDIO_CODING_TYPE_MPEG2:
		return "MPEG2";
	case HDMI_AUDIO_CODING_TYPE_AAC_LC:
		return "AAC";
	case HDMI_AUDIO_CODING_TYPE_DTS:
		return "DTS";
	case HDMI_AUDIO_CODING_TYPE_ATRAC:
		return "ATRAC";
	case HDMI_AUDIO_CODING_TYPE_DSD:
		return "One Bit Audio";
	case HDMI_AUDIO_CODING_TYPE_EAC3:
		return "Dolby Digital +";
	case HDMI_AUDIO_CODING_TYPE_DTS_HD:
		return "DTS-HD";
	case HDMI_AUDIO_CODING_TYPE_MLP:
		return "MAT (MLP)";
	case HDMI_AUDIO_CODING_TYPE_DST:
		return "DST";
	case HDMI_AUDIO_CODING_TYPE_WMA_PRO:
		return "WMA PRO";
	case HDMI_AUDIO_CODING_TYPE_CXT:
		return "Refer to CXT";
	}
	return "Invalid";
}

static const char *
hdmi_audio_sample_size_get_name(enum hdmi_audio_sample_size sample_size)
{
	switch (sample_size) {
	case HDMI_AUDIO_SAMPLE_SIZE_STREAM:
		return "Refer to Stream Header";
	case HDMI_AUDIO_SAMPLE_SIZE_16:
		return "16 bit";
	case HDMI_AUDIO_SAMPLE_SIZE_20:
		return "20 bit";
	case HDMI_AUDIO_SAMPLE_SIZE_24:
		return "24 bit";
	}
	return "Invalid";
}

static const char *
hdmi_audio_sample_frequency_get_name(enum hdmi_audio_sample_frequency freq)
{
	switch (freq) {
	case HDMI_AUDIO_SAMPLE_FREQUENCY_STREAM:
		return "Refer to Stream Header";
	case HDMI_AUDIO_SAMPLE_FREQUENCY_32000:
		return "32 kHz";
	case HDMI_AUDIO_SAMPLE_FREQUENCY_44100:
		return "44.1 kHz (CD)";
	case HDMI_AUDIO_SAMPLE_FREQUENCY_48000:
		return "48 kHz";
	case HDMI_AUDIO_SAMPLE_FREQUENCY_88200:
		return "88.2 kHz";
	case HDMI_AUDIO_SAMPLE_FREQUENCY_96000:
		return "96 kHz";
	case HDMI_AUDIO_SAMPLE_FREQUENCY_176400:
		return "176.4 kHz";
	case HDMI_AUDIO_SAMPLE_FREQUENCY_192000:
		return "192 kHz";
	}
	return "Invalid";
}

static const char *
hdmi_audio_coding_type_ext_get_name(enum hdmi_audio_coding_type_ext ctx)
{
	if (ctx < 0 || ctx > 0x1f)
		return "Invalid";

	switch (ctx) {
	case HDMI_AUDIO_CODING_TYPE_EXT_CT:
		return "Refer to CT";
	case HDMI_AUDIO_CODING_TYPE_EXT_HE_AAC:
		return "HE AAC";
	case HDMI_AUDIO_CODING_TYPE_EXT_HE_AAC_V2:
		return "HE AAC v2";
	case HDMI_AUDIO_CODING_TYPE_EXT_MPEG_SURROUND:
		return "MPEG SURROUND";
	case HDMI_AUDIO_CODING_TYPE_EXT_MPEG4_HE_AAC:
		return "MPEG-4 HE AAC";
	case HDMI_AUDIO_CODING_TYPE_EXT_MPEG4_HE_AAC_V2:
		return "MPEG-4 HE AAC v2";
	case HDMI_AUDIO_CODING_TYPE_EXT_MPEG4_AAC_LC:
		return "MPEG-4 AAC LC";
	case HDMI_AUDIO_CODING_TYPE_EXT_DRA:
		return "DRA";
	case HDMI_AUDIO_CODING_TYPE_EXT_MPEG4_HE_AAC_SURROUND:
		return "MPEG-4 HE AAC + MPEG Surround";
	case HDMI_AUDIO_CODING_TYPE_EXT_MPEG4_AAC_LC_SURROUND:
		return "MPEG-4 AAC LC + MPEG Surround";
	}
	return "Reserved";
}

static const char *hdmitx_get_colorimetry(enum hdmi_colorimetry colorimetry,
					enum hdmi_extended_colorimetry extended_colorimetry)
{
	switch (colorimetry) {
	case HDMI_COLORIMETRY_NONE:
		return "NONE";
	case HDMI_COLORIMETRY_ITU_601:
		return "BT601";
	case HDMI_COLORIMETRY_ITU_709:
		return "BT709";
	case HDMI_COLORIMETRY_EXTENDED:
		return hdmi_extended_colorimetry_get_name(extended_colorimetry);
	default:
		return "INVALID";
	}
}

static const char *hdmi_spd_sdi_get_name(enum hdmi_spd_sdi sdi)
{
	if (sdi < 0 || sdi > 0xff)
		return "Invalid";
	switch (sdi) {
	case HDMI_SPD_SDI_UNKNOWN:
		return "Unknown";
	case HDMI_SPD_SDI_DSTB:
		return "Digital STB";
	case HDMI_SPD_SDI_DVDP:
		return "DVD Player";
	case HDMI_SPD_SDI_DVHS:
		return "D-VHS";
	case HDMI_SPD_SDI_HDDVR:
		return "HDD Videorecorder";
	case HDMI_SPD_SDI_DVC:
		return "DVC";
	case HDMI_SPD_SDI_DSC:
		return "DSC";
	case HDMI_SPD_SDI_VCD:
		return "Video CD";
	case HDMI_SPD_SDI_GAME:
		return "Game";
	case HDMI_SPD_SDI_PC:
		return "PC General";
	case HDMI_SPD_SDI_BD:
		return "Blu-Ray Disc (BD)";
	case HDMI_SPD_SDI_SACD:
		return "Super Audio CD";
	case HDMI_SPD_SDI_HDDVD:
		return "HD DVD";
	case HDMI_SPD_SDI_PMP:
		return "PMP";
	}
	return "Reserved";
}

static const char *hdmitx_get_audio_type(enum hdmi_audio_type coding_type)
{
	switch (coding_type) {
	case CT_REFER_TO_STREAM:
		return "Refer to Stream Header";
	case CT_PCM:
		return "PCM";
	case CT_AC_3:
		return "AC-3";
	case CT_MPEG1:
		return "MPEG1";
	case CT_MP3:
		return "MP3";
	case CT_MPEG2:
		return "MPEG2";
	case CT_AAC:
		return "AAC";
	case CT_DTS:
		return "DTS";
	case CT_ATRAC:
		return "ATRAC";
	case CT_ONE_BIT_AUDIO:
		return "One Bit Audio";
	case CT_DD_P:
		return "Dolby Digital +";
	case CT_DTS_HD:
		return "DTS-HD";
	case CT_MAT:
		return "MAT (MLP)";
	case CT_DST:
		return "DST";
	case CT_WMA:
		return "WMA PRO";
	case CT_CXT:
		return "CXT";
	default:
		return "Invalid";
	}
}

static const char *hdmitx_get_hdr_statrus(enum hdmi_hdr_status hdr_status)
{
	switch (hdr_status) {
	case HDR10PLUS_VSIF:
		return "HDR10PLUS";
	case DOLBYVISION_STD:
		return "MESON_DOLBYVISION_STD";
	case DOLBYVISION_LOWLATENCY:
		return "MESON_DOLBYVISION_LL";
	case HDR10_GAMMA_ST2084:
		return "HDR10";
	case HDR10_GAMMA_HLG:
		return "HLG";
	case SDR:
		return "SDR";
	case HDR10_OTHERS:
	default:
		return "HDR10_OTHERS";
	}
}

static void hdmitx_basic_diag_info(struct hdmi_diagnosis_info *diagnosis_info,
		struct hdmitx_common *tx_comm)
{
	unsigned char  *resolution = NULL;
	enum hdmi_colorspace colorspace = HDMI_COLORSPACE_RGB;
	enum hdmi_colorimetry colorimetry = HDMI_COLORIMETRY_NONE;
	enum hdmi_extended_colorimetry extended_colorimetry =
					HDMI_EXTENDED_COLORIMETRY_XV_YCC_601;
	unsigned char hpd = 0;
	unsigned char rxsense = 0;
	enum hdmi_hdr_status hdr_status = SDR;
	enum hdmi_audio_type coding_type = CT_REFER_TO_STREAM;
	u32 brr = 0, qms_en = 0;

	resolution = hdmitx_common_get_resolution(tx_comm);
	colorspace = hdmitx_common_get_colorspace(tx_comm);
	colorimetry = hdmitx_common_get_colorimetry(tx_comm);
	extended_colorimetry = hdmitx_common_get_extended_colorimetry(tx_comm);
	hpd = hdmitx_get_hpd_state(tx_comm);
	rxsense = hdmitx_common_get_rxsense(tx_comm);
	hdr_status = hdmitx_common_get_hdr_status(tx_comm);
	coding_type = tx_comm->cur_audio_param.type;

	diagnosis_info->basic_info.hpd = hpd;
	diagnosis_info->basic_info.rxsense = rxsense;
	if (resolution)
		strncpy(diagnosis_info->basic_info.resolution, resolution,
				sizeof(diagnosis_info->basic_info.resolution) - 1);
	strncpy(diagnosis_info->basic_info.colorspace,
			hdmi_colorspace_get_name(colorspace),
			sizeof(diagnosis_info->basic_info.colorspace) - 1);
	diagnosis_info->basic_info.color_depth =
			hdmitx_common_get_colordepth(tx_comm);
	if (colorimetry || extended_colorimetry)
		strncpy(diagnosis_info->basic_info.colorimetry,
				hdmitx_get_colorimetry(colorimetry, extended_colorimetry),
				sizeof(diagnosis_info->basic_info.colorimetry) - 1);
	if (coding_type)
		strncpy(diagnosis_info->basic_info.audio_type,
				hdmitx_get_audio_type(coding_type),
				sizeof(diagnosis_info->basic_info.audio_type) - 1);
	if (tx_comm->rxcap.ieeeoui == HDMI_IEEE_OUI)
		diagnosis_info->basic_info.hdmi_mode = 1;
	else
		diagnosis_info->basic_info.hdmi_mode = 0;

	strncpy(diagnosis_info->basic_info.hdr_status,
			hdmitx_get_hdr_statrus(hdr_status),
			sizeof(diagnosis_info->basic_info.hdr_status) - 1);
	hdmitx_get_qms_init_state(tx_comm, &brr, &qms_en);
	if (brr && qms_en)
		strncpy(diagnosis_info->basic_info.vrr, "QMS",
				sizeof(diagnosis_info->basic_info.vrr) - 1);
	else
		strncpy(diagnosis_info->basic_info.vrr, "None",
				sizeof(diagnosis_info->basic_info.vrr) - 1);
}

static void hdmitx_video_diag_info(struct hdmi_diagnosis_info *diagnosis_info,
								struct hdmitx_common *tx_comm)
{
	u32 brr = 0, qms_en = 0;
	u32 frequency =
		tx_comm->hdmitx_vinfo.sync_duration_num / tx_comm->hdmitx_vinfo.sync_duration_den;

	diagnosis_info->video_info.active_h_resolution = tx_comm->fmt_para.timing.h_active;
	diagnosis_info->video_info.active_v_resolution = tx_comm->fmt_para.timing.v_active;
	diagnosis_info->video_info.total_h_resolution = tx_comm->fmt_para.timing.h_total;
	diagnosis_info->video_info.total_v_resolution = tx_comm->fmt_para.timing.v_total;
	diagnosis_info->video_info.pixel_clk = hdmitx_common_get_pixel_clk(tx_comm);
	hdmitx_get_qms_init_state(tx_comm, &brr, &qms_en);
	if (brr && qms_en) {
		diagnosis_info->video_info.vrr_frequency = frequency;
		diagnosis_info->video_info.qms_vrr = 1;
		diagnosis_info->video_info.m_const = hdmitx_common_get_m_const(tx_comm);
		diagnosis_info->video_info.next_tfr = hdmitx_common_get_next_tfr(tx_comm);
		diagnosis_info->video_info.base_refresh_rate =
			hdmitx_common_get_base_refresh_rate(tx_comm);
	} else {
		diagnosis_info->video_info.vrr_frequency = 0;
		diagnosis_info->video_info.qms_vrr = 0;
		diagnosis_info->video_info.m_const = 0;
		diagnosis_info->video_info.next_tfr = 0;
		diagnosis_info->video_info.base_refresh_rate = 0;
	}
}

static void hdmitx_audio_diag_info(struct hdmi_diagnosis_info *diagnosis_info,
			struct hdmitx_common *tx_comm)
{
	diagnosis_info->audio_info.n = hdmitx_common_get_audio_n(tx_comm);
	diagnosis_info->audio_info.cts = hdmitx_common_get_audio_cts(tx_comm);
	diagnosis_info->audio_info.layout_value = hdmitx_common_get_audio_layout(tx_comm);
	diagnosis_info->audio_info.channel_status =
			hdmitx_common_get_audio_channel(tx_comm) + 1;
}

static void hdmitx_phy_diag_info(struct hdmi_diagnosis_info *diagnosis_info,
			struct hdmitx_common *tx_comm)
{
	diagnosis_info->phy_info.status = 1;
	diagnosis_info->phy_info.tmds_clk = hdmitx_common_get_tmds_clk(tx_comm);
}

static void hdmitx_hdcp14_diag_info(struct hdmi_diagnosis_info *diagnosis_info,
			struct hdmitx_common *tx_comm)
{
	u8 bstatus[2] = {0};

	if (tx_comm->hdcptx_comm.hdcp_mode == 1) {
		diagnosis_info->hdcp14_info.enc_en = 1;
		diagnosis_info->hdcp14_info.status =
			hdmitx_common_get_hdcp_auth_state(tx_comm);
		hdmitx_common_get_hdcp_an(tx_comm, diagnosis_info->hdcp14_info.an);
		hdmitx_common_get_hdcp_aksv(tx_comm, diagnosis_info->hdcp14_info.aksv);
		hdmitx_common_get_hdcp_bksv(tx_comm, diagnosis_info->hdcp14_info.bksv);
		hdmitx_common_get_hdcp_bstatus(tx_comm, bstatus);
		diagnosis_info->hdcp14_info.b_status = bstatus[1] << 8 | bstatus[0];
		diagnosis_info->hdcp14_info.b_caps = hdmitx_common_get_hdcp_bcaps(tx_comm);
		diagnosis_info->hdcp14_info.ri = hdmitx_common_get_hdcp_ri(tx_comm);
	} else {
		diagnosis_info->hdcp14_info.enc_en = 0;
		diagnosis_info->hdcp14_info.status = 0;
	}
}

static void hdmitx_hdcp22_diag_info(struct hdmi_diagnosis_info *diagnosis_info,
			struct hdmitx_common *tx_comm)
{
	if (tx_comm->hdcptx_comm.hdcp_mode == 2) {
		diagnosis_info->hdcp22_info.enc_en = 1;
		diagnosis_info->hdcp22_info.status =
			hdmitx_common_get_hdcp_auth_state(tx_comm);
	} else {
		diagnosis_info->hdcp22_info.enc_en = 0;
		diagnosis_info->hdcp22_info.status = 0;
	}
}

static void hdmitx_scdc_diag_info(struct hdmi_diagnosis_info *diagnosis_info,
		struct hdmitx_common *tx_comm)
{
	u8 scrambler_en = tx_comm->fmt_para.scrambler_en;
	u16 ch0_cnt = tx_comm->ced_cnt.ch0_cnt;
	u16 ch1_cnt = tx_comm->ced_cnt.ch1_cnt;
	u16 ch2_cnt = tx_comm->ced_cnt.ch2_cnt;
	u16 ch3_cnt = tx_comm->ced_cnt.ch3_cnt;

	diagnosis_info->scdc_info.scrambling_enable = scrambler_en;
	diagnosis_info->scdc_info.tmds_bit_clock_ratio = tx_comm->fmt_para.tmds_clk_div40;
	strncpy(diagnosis_info->scdc_info.scrambling_status, scrambler_en ? "ok" : "disable",
			sizeof(diagnosis_info->scdc_info.scrambling_status) - 1);
	diagnosis_info->scdc_info.clock_detecded = tx_comm->ch_locked_st.clock_detected;
	diagnosis_info->scdc_info.ch0_lock = tx_comm->ch_locked_st.ch0_locked;
	diagnosis_info->scdc_info.ch1_lock = tx_comm->ch_locked_st.ch1_locked;
	diagnosis_info->scdc_info.ch2_lock = tx_comm->ch_locked_st.ch2_locked;
	diagnosis_info->scdc_info.lane3_lock = tx_comm->ch_locked_st.ch3_locked;
	/*
	 * bit6	reg_scdc_flt_rdy
	 * FLT Ready status from SCDC Sink
	 */
	diagnosis_info->scdc_info.flt_ready =
		(hdmitx_common_get_scdc_sts_flag0(tx_comm) & 0x40) >> 6;
	diagnosis_info->scdc_info.rs_correction_counter = tx_comm->ced_cnt.rs_c_cnt;
	/*
	 * bit7	reg_scdc_dsc_dec_fail
	 * DSC Decode Fail status from SCDC Sink
	 */
	diagnosis_info->scdc_info.dsc_fail =
		(hdmitx_common_get_scdc_sts_flag0(tx_comm) & 0x80) >> 7;
	if (ch0_cnt || ch1_cnt || ch2_cnt || ch3_cnt)
		diagnosis_info->scdc_info.error_count = 1;
	else
		diagnosis_info->scdc_info.error_count = 0;
	diagnosis_info->scdc_info.ch0 = ch0_cnt;
	diagnosis_info->scdc_info.ch1 = ch1_cnt;
	diagnosis_info->scdc_info.ch2 = ch2_cnt;
	diagnosis_info->scdc_info.lane3 = ch3_cnt;
	/*
	 * FRL_LTP_OVR_VAL1	0x0	SCDC FRL LTP Req Override1 Register
	 * 3:0	reg_scdc_ln2_ltp_ovr_val	0x0	Lane2 LTP Request pattern Overwrite Value
	 * 7:4	reg_scdc_ln3_ltp_ovr_val	0x0	Lane3 LTP Request pattern Overwrite Value
	 */
	diagnosis_info->scdc_info.ltp_req = hdmitx_common_get_scdc_ln2_ln3_ltp(tx_comm);
	/*
	 * FRL_LTP_OVR_VAL0	0x0	SCDC FRL LTP Req Override0 Register
	 * 3:0	reg_scdc_ln0_ltp_ovr_val	0x0	Lane0 LTP Request pattern Overwrite Value
	 * 7:4	reg_scdc_ln1_ltp_ovr_val	0x0	Lane1 LTP Request pattern Overwrite Value
	 */
	diagnosis_info->scdc_info.ln0 = hdmitx_common_get_scdc_ln0_ln1_ltp(tx_comm) & 0xf;
	diagnosis_info->scdc_info.ln1 = (hdmitx_common_get_scdc_ln0_ln1_ltp(tx_comm) & 0xf0) >> 4;
	diagnosis_info->scdc_info.frl_rate = tx_comm->fmt_para.frl_rate;
}

static void hdmitx_avi_diag_info(struct hdmi_diagnosis_info *diagnosis_info,
		struct hdmitx_common *tx_comm)
{
	enum hdmi_colorimetry colorimetry = HDMI_COLORIMETRY_NONE;
	enum hdmi_extended_colorimetry extended_colorimetry =
					HDMI_EXTENDED_COLORIMETRY_XV_YCC_601;
	enum hdmi_colorspace colorspace = HDMI_COLORSPACE_RGB;
	enum hdmi_scan_mode scan_info = HDMI_SCAN_MODE_NONE;
	enum hdmi_picture_aspect picture_aspect = HDMI_PICTURE_ASPECT_NONE;
	enum hdmi_active_aspect active_aspect = HDMI_ACTIVE_ASPECT_16_9_TOP;
	enum hdmi_quantization_range quantization_range = HDMI_QUANTIZATION_RANGE_DEFAULT;
	enum hdmi_nups nups = HDMI_NUPS_UNKNOWN;
	enum hdmi_ycc_quantization_range ycc_quantization_range =
					HDMI_YCC_QUANTIZATION_RANGE_LIMITED;
	enum hdmi_content_type content_type = HDMI_CONTENT_TYPE_GRAPHICS;

	colorimetry = hdmitx_common_get_colorimetry(tx_comm);
	extended_colorimetry = hdmitx_common_get_extended_colorimetry(tx_comm);
	colorspace = hdmitx_common_get_colorspace(tx_comm);
	scan_info = hdmitx_common_get_scan_info(tx_comm);
	picture_aspect = hdmitx_common_get_picture_aspect(tx_comm);
	active_aspect = hdmitx_common_get_active_aspect(tx_comm);
	quantization_range = hdmitx_common_get_quantization_range(tx_comm);
	nups = hdmitx_common_get_nups(tx_comm);
	ycc_quantization_range = hdmitx_common_get_ycc_quantization_range(tx_comm);
	content_type = hdmitx_common_get_content_type(tx_comm);

	strncpy(diagnosis_info->avi_infoframe_info.hdmi_colorspace,
			hdmi_colorspace_get_name(colorspace),
			sizeof(diagnosis_info->avi_infoframe_info.hdmi_colorspace) - 1);
	strncpy(diagnosis_info->avi_infoframe_info.scan_info,
			hdmi_scan_mode_get_name(scan_info),
			sizeof(diagnosis_info->avi_infoframe_info.scan_info) - 1);
	strncpy(diagnosis_info->avi_infoframe_info.colorimetry,
			hdmi_colorimetry_get_name(colorimetry),
			sizeof(diagnosis_info->avi_infoframe_info.colorimetry) - 1);
	strncpy(diagnosis_info->avi_infoframe_info.picture_aspect,
			hdmi_picture_aspect_get_name(picture_aspect),
			sizeof(diagnosis_info->avi_infoframe_info.picture_aspect) - 1);
	strncpy(diagnosis_info->avi_infoframe_info.active_aspect,
			hdmi_active_aspect_get_name(active_aspect),
			sizeof(diagnosis_info->avi_infoframe_info.active_aspect) - 1);
	strncpy(diagnosis_info->avi_infoframe_info.extended_colorimetry,
			hdmi_extended_colorimetry_get_name(extended_colorimetry),
			sizeof(diagnosis_info->avi_infoframe_info.extended_colorimetry) - 1);
	strncpy(diagnosis_info->avi_infoframe_info.quantization_range,
			hdmi_quantization_range_get_name(quantization_range),
			sizeof(diagnosis_info->avi_infoframe_info.quantization_range) - 1);
	strncpy(diagnosis_info->avi_infoframe_info.scling,
			hdmi_nups_get_name(nups),
			sizeof(diagnosis_info->avi_infoframe_info.scling) - 1);
	diagnosis_info->avi_infoframe_info.vic = hdmitx_common_get_vic(tx_comm);
	strncpy(diagnosis_info->avi_infoframe_info.ycc_quantization_range,
			hdmi_ycc_quantization_range_get_name(ycc_quantization_range),
			sizeof(diagnosis_info->avi_infoframe_info.ycc_quantization_range) - 1);
	diagnosis_info->avi_infoframe_info.itc = hdmitx_common_get_itc(tx_comm);
	strncpy(diagnosis_info->avi_infoframe_info.content_type,
			hdmi_content_type_get_name(content_type),
			sizeof(diagnosis_info->avi_infoframe_info.content_type) - 1);
	diagnosis_info->avi_infoframe_info.pixel_repeat = hdmitx_common_get_pixel_repeat(tx_comm);
	diagnosis_info->avi_infoframe_info.top_bar = hdmitx_common_get_top_bar(tx_comm);
	diagnosis_info->avi_infoframe_info.bottom_bar = hdmitx_common_get_bottom_bar(tx_comm);
	diagnosis_info->avi_infoframe_info.left_bar = hdmitx_common_get_left_bar(tx_comm);
	diagnosis_info->avi_infoframe_info.right_bar = hdmitx_common_get_right_bar(tx_comm);
}

static void hdmitx_vsif_diag_info(struct hdmi_diagnosis_info *diagnosis_info,
				struct hdmitx_common *tx_comm)
{
	unsigned int ieeeoui = 0;

	ieeeoui = hdmitx_common_get_vsif_ieeeoui(tx_comm);
	if (ieeeoui == HDMI_IEEE_OUI)
		diagnosis_info->vsif_infoframe_info.h14b_vsif_ieeeoui = ieeeoui;
	else if (ieeeoui == HDMI_FORUM_IEEE_OUI)
		diagnosis_info->vsif_infoframe_info.hf_vsif_ieeeoui = ieeeoui;
	else if (ieeeoui == DOVI_IEEEOUI)
		diagnosis_info->vsif_infoframe_info.amdv_ieeeoui = ieeeoui;
	else if (ieeeoui == CUVA_IEEEOUI)
		diagnosis_info->vsif_infoframe_info.cuva_ieeeoui = ieeeoui;
	else if (ieeeoui == HDR10PLUS_IEEEOUI)
		diagnosis_info->vsif_infoframe_info.hdr10plus_ieeeoui = ieeeoui;

	diagnosis_info->vsif_infoframe_info.hdmi_vic =
		hdmitx_common_get_vsif_vic(tx_comm);
	diagnosis_info->vsif_infoframe_info.allm =
		hdmitx_common_get_vsif_allm(tx_comm);
	diagnosis_info->vsif_infoframe_info.amdv_signal =
		hdmitx_common_get_vsif_amdv_signal(tx_comm);
	diagnosis_info->vsif_infoframe_info.amdv_low_latency =
		hdmitx_common_get_vsif_amdv_low_latency(tx_comm);
}

static void hdmitx_spd_diag_info(struct hdmi_diagnosis_info *diagnosis_info,
			struct hdmitx_common *tx_comm)
{
	enum hdmi_spd_sdi sdi = HDMI_SPD_SDI_UNKNOWN;
	struct vendor_info_data *vend_data = NULL;

	if (tx_comm->config_data.vend_data) {
		vend_data = tx_comm->config_data.vend_data;
		if (vend_data->vendor_name)
			strncpy(diagnosis_info->spd_infoframe_info.vendor_name,
				vend_data->vendor_name,
				sizeof(diagnosis_info->spd_infoframe_info.vendor_name));

		if (vend_data->product_desc)
			strncpy(diagnosis_info->spd_infoframe_info.product_description,
				vend_data->product_desc,
				sizeof(diagnosis_info->spd_infoframe_info.product_description));
	}
	sdi = hdmitx_common_get_spd_sdi(tx_comm);
	strncpy(diagnosis_info->spd_infoframe_info.source_info, hdmi_spd_sdi_get_name(sdi),
			sizeof(diagnosis_info->spd_infoframe_info.source_info) - 1);
}

static void hdmitx_drm_diag_info(struct hdmi_diagnosis_info *diagnosis_info,
			struct hdmitx_common *tx_comm)
{
	diagnosis_info->drm_infoframe_info.eotf = hdmitx_common_get_drm_eotf(tx_comm);
}

static void hdmitx_audio_infoframe_diag_info(struct hdmi_diagnosis_info *diagnosis_info,
		struct hdmitx_common *tx_comm)
{
	enum hdmi_audio_coding_type coding_type = HDMI_AUDIO_CODING_TYPE_STREAM;
	enum hdmi_audio_sample_size sample_size = HDMI_AUDIO_SAMPLE_SIZE_STREAM;
	enum hdmi_audio_sample_frequency sample_frequency = HDMI_AUDIO_SAMPLE_FREQUENCY_STREAM;
	enum hdmi_audio_coding_type_ext coding_type_ext = HDMI_AUDIO_CODING_TYPE_EXT_CT;

	coding_type = hdmitx_common_get_audio_coding_type(tx_comm);
	sample_size = hdmitx_common_get_audio_sample_size(tx_comm);
	sample_frequency = hdmitx_common_get_audio_sample_frequency(tx_comm);
	coding_type_ext = hdmitx_common_get_audio_coding_type_ext(tx_comm);

	/*
	 * CC2 CC1 CC0
	 * 0   0   0    Refer to Stream Header
	 * 0   0   1    2 Channels
	 * 0   1   0    3 Channels
	 * 0   1   1    4 Channels
	 * 1   0   0    5 Channels
	 * 1   0   1    6 Channels
	 * 1   1   0    7 Channels
	 * 1   1   1    8 Channels
	 */
	diagnosis_info->audio_infoframe_info.channels =
			hdmitx_common_get_audio_channel(tx_comm) + 1;
	strncpy(diagnosis_info->audio_infoframe_info.coding_type,
			hdmi_audio_coding_type_get_name(coding_type),
			sizeof(diagnosis_info->audio_infoframe_info.coding_type) - 1);
	strncpy(diagnosis_info->audio_infoframe_info.sample_size,
			hdmi_audio_sample_size_get_name(sample_size),
			sizeof(diagnosis_info->audio_infoframe_info.sample_size) - 1);
	strncpy(diagnosis_info->audio_infoframe_info.sample_frequency,
			hdmi_audio_sample_frequency_get_name(sample_frequency),
			sizeof(diagnosis_info->audio_infoframe_info.sample_frequency) - 1);
	strncpy(diagnosis_info->audio_infoframe_info.coding_type_ext,
			hdmi_audio_coding_type_ext_get_name(coding_type_ext),
			sizeof(diagnosis_info->audio_infoframe_info.coding_type_ext) - 1);
	diagnosis_info->audio_infoframe_info.channel_allocation =
			hdmitx_common_get_audio_channel_allocation(tx_comm);
	diagnosis_info->audio_infoframe_info.level_shift_value =
			hdmitx_common_get_audio_level_shift_value(tx_comm);
	diagnosis_info->audio_infoframe_info.downmix_inhibit =
			hdmitx_common_get_audio_downmix_inhibit(tx_comm);
}

static void hdmitx_gcp_diag_info(struct hdmi_diagnosis_info *diagnosis_info,
		struct hdmitx_common *tx_comm)
{
	diagnosis_info->gcp_packet_info.avmute =
			hdmitx_common_get_avmute_state(tx_comm);
	diagnosis_info->gcp_packet_info.color_depth =
			hdmitx_common_get_color_depth(tx_comm) * 2;
}

static void hdmitx_reserved_diag_info(struct hdmi_diagnosis_info *diagnosis_info,
		struct hdmitx_common *tx_comm)
{
	diagnosis_info->reserved_info.reserved0 = 0;
	diagnosis_info->reserved_info.reserved1 = 1;
	diagnosis_info->reserved_info.reserved2 = 2;
	diagnosis_info->reserved_info.reserved3 = 3;
	diagnosis_info->reserved_info.reserved4 = 4;
	diagnosis_info->reserved_info.reserved5 = 5;
	diagnosis_info->reserved_info.reserved6 = 6;
	diagnosis_info->reserved_info.reserved7 = 7;
	diagnosis_info->reserved_info.reserved8 = 8;
	diagnosis_info->reserved_info.reserved9 = 9;
	strncpy(diagnosis_info->reserved_info.reserved10, "reserved10",
		sizeof(diagnosis_info->reserved_info.reserved10) - 1);
	strncpy(diagnosis_info->reserved_info.reserved11, "reserved11",
			sizeof(diagnosis_info->reserved_info.reserved11) - 1);
	strncpy(diagnosis_info->reserved_info.reserved12, "reserved12",
			sizeof(diagnosis_info->reserved_info.reserved12) - 1);
	strncpy(diagnosis_info->reserved_info.reserved13, "reserved13",
			sizeof(diagnosis_info->reserved_info.reserved13) - 1);
	strncpy(diagnosis_info->reserved_info.reserved14, "reserved14",
			sizeof(diagnosis_info->reserved_info.reserved14) - 1);
	strncpy(diagnosis_info->reserved_info.reserved15, "reserved15",
			sizeof(diagnosis_info->reserved_info.reserved15) - 1);
	strncpy(diagnosis_info->reserved_info.reserved16, "reserved16",
			sizeof(diagnosis_info->reserved_info.reserved16) - 1);
	strncpy(diagnosis_info->reserved_info.reserved17, "reserved17",
			sizeof(diagnosis_info->reserved_info.reserved17) - 1);
	strncpy(diagnosis_info->reserved_info.reserved18, "reserved18",
			sizeof(diagnosis_info->reserved_info.reserved18) - 1);
	strncpy(diagnosis_info->reserved_info.reserved19, "reserved19",
			sizeof(diagnosis_info->reserved_info.reserved19) - 1);
}

int am_meson_hdmi_get_hdmitx_diag(struct drm_device *dev,
			void *data, struct drm_file *file_priv)
{
	struct hdmi_diagnosis_info *diagnosis_info = data;
	struct drm_connector *connector;
	struct drm_mode_object *mo;
	struct hdmitx_common *tx_comm;

	if (!diagnosis_info) {
		DRM_INFO("%s: diagnosis_info is NULL\n", __func__);
		return 0;
	}

	mo = drm_mode_object_find(dev, file_priv, diagnosis_info->conn_id,
			DRM_MODE_OBJECT_CONNECTOR);
	if (!mo)
		return -ENOENT;

	connector = obj_to_connector(mo);
	tx_comm = meson_get_hdmitx_common(connector);
	if (!tx_comm) {
		DRM_INFO("%s: tx_comm is NULL\n", __func__);
		return 0;
	}

	/* basic */
	hdmitx_basic_diag_info(diagnosis_info, tx_comm);
	/* video */
	hdmitx_video_diag_info(diagnosis_info, tx_comm);
	/* audio */
	hdmitx_audio_diag_info(diagnosis_info, tx_comm);
	/* phy */
	hdmitx_phy_diag_info(diagnosis_info, tx_comm);
	/* hdcp14 */
	hdmitx_hdcp14_diag_info(diagnosis_info, tx_comm);
	/* hdcp22 */
	hdmitx_hdcp22_diag_info(diagnosis_info, tx_comm);
	/* scdc */
	hdmitx_scdc_diag_info(diagnosis_info, tx_comm);
	/* avi */
	hdmitx_avi_diag_info(diagnosis_info, tx_comm);
	/* vsif */
	hdmitx_vsif_diag_info(diagnosis_info, tx_comm);
	/* spd */
	hdmitx_spd_diag_info(diagnosis_info, tx_comm);
	/* drm */
	hdmitx_drm_diag_info(diagnosis_info, tx_comm);
	/* audio */
	hdmitx_audio_infoframe_diag_info(diagnosis_info, tx_comm);
	/* gcp */
	hdmitx_gcp_diag_info(diagnosis_info, tx_comm);
	/* reserved */
	hdmitx_reserved_diag_info(diagnosis_info, tx_comm);

	return 0;
}

