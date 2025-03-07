// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/major.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/compiler.h>
#include <linux/arm-smccc.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/errno.h>
#include <linux/export.h>
#include <linux/string.h>

#include <linux/amlogic/media/vout/vinfo.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_infoframe.h>
#include "hdmitx_log.h"

#ifdef CONFIG_AMLOGIC_HDMITX21
#include "hdmitx_module.h"
#include "hdmitx.h"
#include "hdmitx_enc_clk_config.h"
#endif

static struct hdmitx_common *global_tx_common;
/*
 * There are 3 callback functions for front HDR/DV/HDR10+ modules to notify
 * hdmi drivers to send out related HDMI infoframe
 * hdmitx_set_drm_pkt() is for HDR 2084 SMPTE, HLG, etc.
 * hdmitx_set_vsif_pkt() is for DV
 * hdmitx_set_hdr10plus_pkt is for HDR10+
 * Front modules may call the 2nd, and next call the 1st, and the realted flags
 * are remained the same. So, add hdr_status_pos and place it in the above 3
 * functions to record the position.
 */
int hdr_status_pos;
/*
 * for SONY-KD-55A8F TV, need to mute more frames
 * when switch DV(LL)->HLG
 */
static int hdr_mute_frame = 20;

#ifdef CONFIG_AMLOGIC_HDMITX21
/*
 * now this interface should be not used, otherwise need
 * adjust as hdmi_vend_infoframe_rawset fistly
 */
void hdmi_vend_infoframe_set(struct hdmi_vendor_infoframe *info)
{
	u8 body[31] = {0};

	if (!info) {
		if (global_tx_common->rxcap.ifdb_present)
			hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_VENDOR, NULL);
		else
			hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_VENDOR2, NULL);
		return;
	}

	hdmi_vendor_infoframe_pack(info, body, sizeof(body));
	if (global_tx_common->rxcap.ifdb_present)
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_VENDOR, body);
	else
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_VENDOR2, body);
}

/*
 * refer to DV Consumer Decoder for Source Devices
 * System Development Guide Kit version chapter 4.4.8 Game
 * content signaling:
 * 1.if DV sink device that supports ALLM with
 * InfoFrame Data Block (IFDB), HF-VSIF with ALLM_Mode = 1
 * should comes after Dolby VSIF with L11_MD_Present = 1 and
 * Content_Type[3:0] = 0x2(case B1)
 * 2.DV sink device that supports ALLM without
 * InfoFrame Data Block (IFDB), Dolby VSIF with L11_MD_Present
 * = 1 and Content_Type[3:0] = 0x2 should comes after HF-VSIF
 * with  ALLM_Mode = 1(case B2), or should only send Dolby VSIF,
 * not send HF-VSIF(case A)
 */
/* only used for DV_VSIF / HDMI1.4b_VSIF / CUVA_VSIF / HDR10+ VSIF */
void hdmi_vend_infoframe_rawset(u8 *hb, u8 *pb)
{
	u8 body[31] = {0};

	if (!hb || !pb) {
		if (!global_tx_common->rxcap.ifdb_present)
			hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_VENDOR2, NULL);
		else
			hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_VENDOR, NULL);
		return;
	}

	memcpy(body, hb, 3);
	memcpy(&body[3], pb, 28);
	if (global_tx_common->rxcap.ifdb_present &&
		global_tx_common->rxcap.additional_vsif_num >= 1) {
		/* dolby cts case93 B1 */
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_VENDOR, body);
	} else if (!global_tx_common->rxcap.ifdb_present) {
		/* dolby cts case92 B2 */
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_VENDOR2, body);
	} else {
		/* case89 ifdb_present but no additional_vsif, should not send HF-VSIF */
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_VENDOR2, NULL);
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_VENDOR, body);
	}
}

/* only used for HF-VSIF */
void hdmi_vend_infoframe2_rawset(u8 *hb, u8 *pb)
{
	u8 body[31] = {0};

	if (!hb || !pb) {
		if (!global_tx_common->rxcap.ifdb_present)
			hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_VENDOR, NULL);
		else
			hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_VENDOR2, NULL);
		return;
	}

	memcpy(body, hb, 3);
	memcpy(&body[3], pb, 28);
	if (global_tx_common->rxcap.ifdb_present &&
		global_tx_common->rxcap.additional_vsif_num >= 1) {
		/* dolby cts case93 B1 */
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_VENDOR2, body);
	} else if (!global_tx_common->rxcap.ifdb_present) {
		/* dolby cts case92 B2 */
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_VENDOR, body);
	} else {
		/* case89 ifdb_present but no additional_vsif, currently
		 * no DV-VSIF enabled, then send HF-VSIF
		 */
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_VENDOR2, body);
	}
}

/* refer DV HDMI 1.4b/2.0/2.1 Transmission
 * 1.DV source device signals the video-timing
 * format by using the CTA VICs in its AVI InfoFrame
 * 2.DV source must not simultaneously transmit
 * a DV and any form of H14b VSIF, even for the case
 * of 4Kp24/25/30
 */
/* only used for DV_VSIF / CUVA VSIF / HDMI1.4b_VSIF / HDR10+ VSIF */
int hdmi_vend_infoframe_get(struct hdmitx_dev *hdev, u8 *body)
{
	int ret;

	if (!hdev || !body)
		return 0;

	if (global_tx_common->rxcap.ifdb_present &&
			global_tx_common->rxcap.additional_vsif_num >= 1) {
		/* dolby cts case93 B1 */
		ret = hdmitx_infoframe_rawget(HDMI_INFOFRAME_TYPE_VENDOR, body);
	} else if (!hdev->tx_comm.rxcap.ifdb_present) {
		/* dolby cts case92 B2 */
		ret = hdmitx_infoframe_rawget(HDMI_INFOFRAME_TYPE_VENDOR2, body);
	} else {
		/* case89 */
		ret = hdmitx_infoframe_rawget(HDMI_INFOFRAME_TYPE_VENDOR, body);
	}
	return ret;
}

/******************sync from upstream start: drivers/video/hdmi.c*************************/
#define HDMI_INFOFRAME_HEADER_SIZE  4
#define HDMI_AVI_INFOFRAME_SIZE    13
#define HDMI_SPD_INFOFRAME_SIZE    25
#define HDMI_AUDIO_INFOFRAME_SIZE  10
#define HDMI_DRM_INFOFRAME_SIZE    26

#define HDMI_INFOFRAME_SIZE(type)	\
	(HDMI_INFOFRAME_HEADER_SIZE + HDMI_ ## type ## _INFOFRAME_SIZE)

/* from upstream */
static u8 hdmi_infoframe_checksum(const u8 *ptr, size_t size)
{
	u8 csum = 0;
	size_t i;

	/* compute checksum */
	for (i = 0; i < size; i++)
		csum += ptr[i];

	return 256 - csum;
}

/* from upstream */
static void hdmi_infoframe_set_checksum(void *buffer, size_t size)
{
	u8 *ptr = buffer;

	ptr[3] = hdmi_infoframe_checksum(buffer, size);
}

/* re-construct from upstream */
static int hdmi_avi_infoframe_check_only_renew(const struct hdmi_avi_infoframe *frame)
{
	/* refer to CTA-861-H Page 69 */
	if (frame->type != HDMI_INFOFRAME_TYPE_AVI ||
	    frame->version < 2 ||
	    frame->version > 4 ||
	    frame->length != HDMI_AVI_INFOFRAME_SIZE)
		return -EINVAL;

	if (frame->picture_aspect > HDMI_PICTURE_ASPECT_16_9)
		return -EINVAL;

	return 0;
}

/*
 * re-construct from upstream
 * hdmi_avi_infoframe_check() - check a HDMI AVI infoframe
 * @frame: HDMI AVI infoframe
 *
 * Validates that the infoframe is consistent and updates derived fields
 * (eg. length) based on other fields.
 *
 * Returns 0 on success or a negative error code on failure.
 */
static int hdmi_avi_infoframe_check_renew(struct hdmi_avi_infoframe *frame)
{
	return hdmi_avi_infoframe_check_only_renew(frame);
}

/*
 * re-construct from upstream
 * hdmi_avi_infoframe_pack_only() - write HDMI AVI infoframe to binary buffer
 * @frame: HDMI AVI infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 *
 * Packs the information contained in the @frame structure into a binary
 * representation that can be written into the corresponding controller
 * registers. Also computes the checksum as required by section 5.3.5 of
 * the HDMI 1.4 specification.
 *
 * Returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
static ssize_t hdmi_avi_infoframe_pack_only_renew(const struct hdmi_avi_infoframe *frame,
				     void *buffer, size_t size)
{
	u8 *ptr = buffer;
	size_t length;
	int ret;

	ret = hdmi_avi_infoframe_check_only_renew(frame);
	if (ret)
		return ret;

	length = HDMI_INFOFRAME_HEADER_SIZE + frame->length;

	if (size < length)
		return -ENOSPC;

	memset(buffer, 0, size);

	ptr[0] = frame->type;
	ptr[1] = frame->version;
	ptr[2] = frame->length;
	ptr[3] = 0; /* checksum */

	/* start infoframe payload */
	ptr += HDMI_INFOFRAME_HEADER_SIZE;

	ptr[0] = ((frame->colorspace & 0x3) << 5) | (frame->scan_mode & 0x3);

	/*
	 * Data byte 1, bit 4 has to be set if we provide the active format
	 * aspect ratio
	 */
	if (frame->active_aspect & 0xf)
		ptr[0] |= BIT(4);

	/* Bit 3 and 2 indicate if we transmit horizontal/vertical bar data */
	if (frame->top_bar || frame->bottom_bar)
		ptr[0] |= BIT(3);

	if (frame->left_bar || frame->right_bar)
		ptr[0] |= BIT(2);

	ptr[1] = ((frame->colorimetry & 0x3) << 6) |
		 ((frame->picture_aspect & 0x3) << 4) |
		 (frame->active_aspect & 0xf);

	ptr[2] = ((frame->extended_colorimetry & 0x7) << 4) |
		 ((frame->quantization_range & 0x3) << 2) |
		 (frame->nups & 0x3);

	if (frame->itc)
		ptr[2] |= BIT(7);

	ptr[3] = frame->video_code;

	ptr[4] = ((frame->ycc_quantization_range & 0x3) << 6) |
		 ((frame->content_type & 0x3) << 4) |
		 (frame->pixel_repeat & 0xf);

	ptr[5] = frame->top_bar & 0xff;
	ptr[6] = (frame->top_bar >> 8) & 0xff;
	ptr[7] = frame->bottom_bar & 0xff;
	ptr[8] = (frame->bottom_bar >> 8) & 0xff;
	ptr[9] = frame->left_bar & 0xff;
	ptr[10] = (frame->left_bar >> 8) & 0xff;
	ptr[11] = frame->right_bar & 0xff;
	ptr[12] = (frame->right_bar >> 8) & 0xff;

	hdmi_infoframe_set_checksum(buffer, length);

	return length;
}

/*
 * re-construct from upstream
 * hdmi_avi_infoframe_pack() - check a HDMI AVI infoframe,
 *                             and write it to binary buffer
 * @frame: HDMI AVI infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 *
 * Validates that the infoframe is consistent and updates derived fields
 * (eg. length) based on other fields, after which it packs the information
 * contained in the @frame structure into a binary representation that
 * can be written into the corresponding controller registers. This function
 * also computes the checksum as required by section 5.3.5 of the HDMI 1.4
 * specification.
 *
 * Returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
static ssize_t hdmi_avi_infoframe_pack_renew(struct hdmi_avi_infoframe *frame,
				void *buffer, size_t size)
{
	int ret;

	ret = hdmi_avi_infoframe_check_renew(frame);
	if (ret)
		return ret;

	return hdmi_avi_infoframe_pack_only_renew(frame, buffer, size);
}

/*
 * re-construct from upstream
 * hdmi_avi_infoframe_unpack() - unpack binary buffer to a HDMI AVI infoframe
 * @frame: HDMI AVI infoframe
 * @buffer: source buffer
 * @size: size of buffer
 *
 * Unpacks the information contained in binary @buffer into a structured
 * @frame of the HDMI Auxiliary Video (AVI) information frame.
 * Also verifies the hdmi_avi_infoframe_unpack_renew checksum as
 * required by section 5.3.5 of the HDMI 1.4
 * specification.
 *
 * Returns 0 on success or a negative error code on failure.
 */
int hdmi_avi_infoframe_unpack_renew(struct hdmi_avi_infoframe *frame,
				     const void *buffer, size_t size)
{
	const u8 *ptr = buffer;

	/*
	 * if (size < HDMI_INFOFRAME_SIZE(AVI))
	 *	return -EINVAL;

	 * if (ptr[0] != HDMI_INFOFRAME_TYPE_AVI ||
	 *   ptr[1] < 2 ||
	 *   ptr[1] > 4 ||

	 *	ptr[2] != HDMI_AVI_INFOFRAME_SIZE)
	 *  return -EINVAL;

	 *  if (hdmi_infoframe_checksum(buffer, HDMI_INFOFRAME_SIZE(AVI)) != 0)
	 *  return -EINVAL;
	 */

	hdmi_avi_infoframe_init(frame);

	ptr += HDMI_INFOFRAME_HEADER_SIZE;

	frame->colorspace = (ptr[0] >> 5) & 0x3;
	if (ptr[0] & 0x10)
		frame->active_aspect = ptr[1] & 0xf;
	if (ptr[0] & 0x8) {
		frame->top_bar = (ptr[6] << 8) | ptr[5];
		frame->bottom_bar = (ptr[8] << 8) | ptr[7];
	}
	if (ptr[0] & 0x4) {
		frame->left_bar = (ptr[10] << 8) | ptr[9];
		frame->right_bar = (ptr[12] << 8) | ptr[11];
	}
	frame->scan_mode = ptr[0] & 0x3;

	frame->colorimetry = (ptr[1] >> 6) & 0x3;
	frame->picture_aspect = (ptr[1] >> 4) & 0x3;
	frame->active_aspect = ptr[1] & 0xf;

	frame->itc = ptr[2] & 0x80 ? true : false;
	frame->extended_colorimetry = (ptr[2] >> 4) & 0x7;
	frame->quantization_range = (ptr[2] >> 2) & 0x3;
	frame->nups = ptr[2] & 0x3;

	frame->video_code = ptr[3];
	/* refer to CTA-861-H Page 69 */
	if (frame->video_code >= 128)
		frame->version = 0x3;
	frame->ycc_quantization_range = (ptr[4] >> 6) & 0x3;
	frame->content_type = (ptr[4] >> 4) & 0x3;

	frame->pixel_repeat = ptr[4] & 0xf;

	return 0;
}

/******************sync from upstream end*************************/

void hdmi_avi_infoframe_set(struct hdmi_avi_infoframe *info)
{
	u8 buffer[31] = {0};
	struct hdmitx_hw_common *tx_hw = global_tx_common->tx_hw;

	if (!info) {
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_AVI, NULL);
		return;
	}

	/* refer to CTA-861-H Page 69 */
	if (info->video_code >= 128)
		info->version = 0x3;
	else
		info->version = 0x2;
	hdmi_avi_infoframe_pack_renew(info, buffer, sizeof(buffer));
	/* the hardware writes the buffer data to the register */
	hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_AVI, buffer);
}

void hdmi_avi_infoframe_rawset(u8 *hb, u8 *pb)
{
	u8 body[31] = {0};

	if (!hb || !pb) {
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_AVI, NULL);
		return;
	}

	memcpy(body, hb, 3);
	memcpy(&body[3], pb, 28);
	hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_AVI, body);
}

int hdmi_avi_infoframe_get(u8 *body)
{
	int ret;

	if (!body)
		return 0;
	ret = hdmitx_infoframe_rawget(HDMI_INFOFRAME_TYPE_AVI, body);
	return ret;
}

void hdmi_spd_infoframe_set(struct hdmi_spd_infoframe *info)
{
	u8 body[31] = {0};

	if (!info) {
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_SPD, NULL);
		return;
	}

	hdmi_spd_infoframe_pack(info, body, sizeof(body));
	hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_SPD, body);
}

void hdmi_audio_infoframe_set(struct hdmi_audio_infoframe *info)
{
	u8 body[31] = {0};

	if (!info) {
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_AUDIO, NULL);
		return;
	}

	hdmi_audio_infoframe_pack(info, body, sizeof(body));
	hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_AUDIO, body);
}

void hdmi_audio_infoframe_rawset(u8 *hb, u8 *pb)
{
	u8 body[31] = {0};

	if (!hb || !pb) {
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_AUDIO, NULL);
		return;
	}

	memcpy(body, hb, 3);
	memcpy(&body[3], pb, 28);
	hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_AUDIO, body);
}

void hdmi_drm_infoframe_set(struct hdmi_drm_infoframe *info)
{
	u8 body[31] = {0};

	if (!info) {
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_DRM, NULL);
		return;
	}

	hdmi_drm_infoframe_pack(info, body, sizeof(body));
	hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_DRM, body);
}

void hdmi_drm_infoframe_rawset(u8 *hb, u8 *pb)
{
	u8 body[31] = {0};

	if (!hb || !pb) {
		hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_DRM, NULL);
		return;
	}

	memcpy(body, hb, 3);
	memcpy(&body[3], pb, 28);
	hdmitx_infoframe_send(HDMI_INFOFRAME_TYPE_DRM, body);
}

int hdmi_avi_infoframe_config(enum avi_component_conf conf, u8 val)
{
	struct hdmi_avi_infoframe *frame = &global_tx_common->infoframes.avi.avi;
	struct hdmi_format_para *para = &global_tx_common->fmt_para;

	switch (conf) {
	case CONF_AVI_CS:
		frame->colorspace = val;
		break;
	case CONF_AVI_BT2020:
		if (val == SET_AVI_BT2020) {
			frame->colorimetry = HDMI_COLORIMETRY_EXTENDED;
			frame->extended_colorimetry = HDMI_EXTENDED_COLORIMETRY_BT2020;
			HDMITX_DEBUG_PACKET("avi_colorimetry: bt2020\n");
		}
		if (val == CLR_AVI_BT2020) {
			if (para->timing.v_total <= 576) {
				/* SD formats */
				frame->colorimetry = HDMI_COLORIMETRY_ITU_601;
				frame->extended_colorimetry = 0;
				HDMITX_DEBUG_PACKET("avi_colorimetry: bt601\n");
			} else {
				frame->colorimetry = HDMI_COLORIMETRY_ITU_709;
				frame->extended_colorimetry = 0;
				HDMITX_DEBUG_PACKET("avi_colorimetry: bt709\n");
			}
		}
		break;
	case CONF_AVI_Q01:
		frame->quantization_range = val;
		break;
	case CONF_AVI_YQ01:
		frame->ycc_quantization_range = val;
		break;
	case CONF_AVI_VIC:
		frame->video_code = val;
		break;
	case CONF_AVI_AR:
		frame->picture_aspect = val;
		break;
	case CONF_AVI_CT_TYPE:
		frame->itc = (val >> 4) & 0x1;
		val = val & 0xf;
		if (val == SET_CT_OFF)
			frame->content_type = 0;
		else if (val == SET_CT_GRAPHICS)
			frame->content_type = 0;
		else if (val == SET_CT_PHOTO)
			frame->content_type = 1;
		else if (val == SET_CT_CINEMA)
			frame->content_type = 2;
		else if (val == SET_CT_GAME)
			frame->content_type = 3;
		break;
	default:
		break;
	}
	hdmi_avi_infoframe_set(frame);
	return 0;
}
#endif

int hdmitx_common_setup_vsif_packet(struct hdmitx_common *tx_comm,
	enum vsif_type type, int on, void *param)
{
	u8 len = 0;
	/* transmission usage, excluding checksum */
	u8 buffer[31] = {0x81, 0x1, 0};
	u32 ieeeoui = 0;
	u32 vic = 0;
	struct hdmitx_hw_common *tx_hw = tx_comm->tx_hw;

	if (type >= VT_MAX)
		return -EINVAL;

	switch (type) {
	case VT_DEFAULT:
		break;
	case VT_HDMI14_4K:
		ieeeoui = HDMI_IEEE_OUI;
		len = 5;
		vic = hdmitx_edid_get_hdmi14_4k_vic(tx_comm->fmt_para.vic);
		if (vic > 0) {
			buffer[2] = len;
			buffer[4] = GET_OUI_BYTE0(ieeeoui);
			buffer[5] = GET_OUI_BYTE1(ieeeoui);
			buffer[6] = GET_OUI_BYTE2(ieeeoui);
			buffer[7] = vic & 0xf;
			buffer[8] = 0x20;
			hdmitx_hw_cntl_config(tx_hw, CONF_AVI_VIC, 0);
			hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, buffer);
		} else {
			hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, NULL);
			HDMITX_INFO("clear vsif for non-4k mode.\n");
			return -EINVAL;
		}
		break;
	case VT_ALLM:
		ieeeoui = HDMI_FORUM_IEEE_OUI;
		len = 5;
		/* Fixed value */
		buffer[7] = 0x1;
		if (on) {
			buffer[2] = len;
			buffer[4] = GET_OUI_BYTE0(ieeeoui);
			buffer[5] = GET_OUI_BYTE1(ieeeoui);
			buffer[6] = GET_OUI_BYTE2(ieeeoui);
			/* set bit1, ALLM_MODE */
			buffer[8] |= 1 << 1;
			/* reset vic which may be reset by VT_HDMI14_4K */
			if (hdmitx_edid_get_hdmi14_4k_vic(tx_comm->fmt_para.vic) > 0)
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_VIC, tx_comm->fmt_para.vic);
			hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR2, buffer);
		} else {
			/* clear bit1, ALLM_MODE */
			buffer[8] &= ~(1 << 1);
			/*
			 * 1.When the Source stops transmitting the HF-VSIF,
			 * the Sink shall interpret this as an indication
			 * that transmission of features described in this
			 * Infoframe has stopped
			 * 2.When a Source is utilizing the HF-VSIF for ALLM
			 * signaling only and indicates the Sink should
			 * revert from low-latency Mode to its previous mode,
			 * the Source should send ALLM Mode = 0 to quickly
			 * and reliably request the change. If a Source
			 * indicates ALLM Mode = 0 in this manner, the
			 * Source should transmit an HF-VSIF with ALLM Mode = 0
			 * for at least 4 frames but for not more than 1 second.
			 */
			/* hdmi_vend_infoframe2_rawset(hb, vsif_db); */
			/* wait for 4frames ~ 1S, then stop send HF-VSIF */
			hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR2, NULL);
		}
		break;
	default:
		break;
	}

	return 1;
}

/*
 *  SDR/HDR uevent
 *  1: SDR to HDR
 *  0: HDR to SDR
 */
static void hdmitx_sdr_hdr_uevent(void)
{
	if (global_tx_common->hdmi_current_hdr_mode != 0)
		/* SDR -> HDR */
		hdmitx_event_mgr_send_uevent(global_tx_common->event_mgr,
				HDMITX_HDR_EVENT, 1, false);
	else if (global_tx_common->hdmi_current_hdr_mode == 0)
		/* HDR -> SDR */
		hdmitx_event_mgr_send_uevent(global_tx_common->event_mgr,
				HDMITX_HDR_EVENT, 0, false);
}

static void hdmitx_video_mute_op(bool flag)
{
	if (!global_tx_common->hdmi_init)
		return;

	/* video mute is handled in the vsync_intr_handler interface */
	if (flag)
		global_tx_common->vid_mute_op = VIDEO_MUTE;
	else
		global_tx_common->vid_mute_op = VIDEO_UNMUTE;
}

void hdr_unmute_work_func(struct work_struct *work)
{
	unsigned int mute_us;

	if (hdr_mute_frame) {
		mute_us = hdr_mute_frame * hdmitx_get_frame_duration();
		usleep_range(mute_us, mute_us + 10);
		hdmitx_video_mute_op(false);
	}
}

void hdr_work_func(struct work_struct *work)
{
	hdmitx_sdr_hdr_uevent();
}

/* Need to be called in edid_spinlock */
static void hdmitx_set_sdr_pkt(void)
{
	struct hdmitx_hw_common *tx_hw = global_tx_common->tx_hw;
	unsigned char buffer[31] = {0x87, 0x1, 26};

	if (global_tx_common->hdr_transfer_feature == T_BT709 &&
			global_tx_common->hdr_color_feature == C_BT709) {
		if (global_tx_common->rxcap.hdr_info2.hdr_support & 0x1) {
			/* edid supports SDR */
			HDMITX_INFO("hdr: [%s]: send zero drm pkt\n", __func__);
			hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_DRM, buffer);
		} else {
			/* edid does not support SDR */
			HDMITX_INFO("hdr: [%s]: disabled drm pkt\n", __func__);
			hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_DRM, NULL);
		}
		hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020, global_tx_common->colormetry);

		/*
		 * disable DRM packets completely ONLY if hdr transfer
		 * feature and color feature still demand SDR.
		 */
		if (hdr_status_pos == 4) {
			/* zero hdr10plus VSIF being sent - disable it */
			HDMITX_INFO("hdr: [%s]: disable hdr10plus vsif pkt\n", __func__);
			hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, NULL);
			hdr_status_pos = 0;
		}
		/* update hdr mode flag */
		global_tx_common->hdmi_current_hdr_mode = 0;
		global_tx_common->hdmi_last_hdr_mode = global_tx_common->hdmi_current_hdr_mode;
	}
}

#define GET_LOW8BIT(a)	((a) & 0xff)
#define GET_HIGH8BIT(a)	(((a) >> 8) & 0xff)
void hdmitx_set_drm_pkt(struct master_display_info_s *data)
{
	unsigned char buffer[31] = {0x87, 0x1, 26};
	struct hdmitx_hw_common *tx_hw = global_tx_common->tx_hw;
	unsigned long flags;
	struct rx_cap *prxcap = &global_tx_common->rxcap;
	enum hdmi_tf_type hdmi_hdr_status = hdmitx_hw_get_state(tx_hw, STAT_TX_HDR, 0);
	struct hdr_info *hdr_info = &prxcap->hdr_info;
	u32 save_last_hdr_mode;
	enum hdmi_hdr_transfer hdr_transfer_feature = T_UNKNOWN;
	enum hdmi_hdr_color hdr_color_feature = C_UNKNOWN;
	unsigned int colormetry = 0;

	HDMITX_DEBUG_PACKET("%s[%d]\n", __func__, __LINE__);
	if (data) {
		memcpy(&global_tx_common->drm_config_data, data,
				sizeof(struct master_display_info_s));
		hdr_transfer_feature = (data->features >> 8) & 0xff;
		hdr_color_feature = (data->features >> 16) & 0xff;
		colormetry = (data->features >> 30) & 0x1;
	} else {
		memset(&global_tx_common->drm_config_data, 0, sizeof(struct master_display_info_s));
	}

	spin_lock_irqsave(&global_tx_common->edid_spinlock, flags);

	if (!data || !global_tx_common->rxcap.hdr_info2.hdr_support) {
		buffer[1] = 0;
		buffer[2] = 0;
		buffer[4] = 0;
		hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_DRM, NULL);
		hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020, CLR_AVI_BT2020);
		global_tx_common->hdr_transfer_feature = T_UNKNOWN;
		global_tx_common->hdr_color_feature = C_UNKNOWN;
		global_tx_common->colormetry = 0;
		global_tx_common->hdmi_current_hdr_mode = 0;
		global_tx_common->hdmi_last_hdr_mode = 0;
		HDMITX_INFO("hdr: [%s]: disable drm pkt\n", __func__);
		spin_unlock_irqrestore(&global_tx_common->edid_spinlock, flags);
		return;
	}

	if (global_tx_common->ready == 0) {
		spin_unlock_irqrestore(&global_tx_common->edid_spinlock, flags);
		return;
	}

	/*
	 * if currently output 8bit, and EDID don't support Y422, and config_csc_en is 0,
	 * switch to SDR output; if hdr_8bit_en is 1, switch to HDR output
	 */
	if (global_tx_common->fmt_para.cd == COLORDEPTH_24B && !global_tx_common->hdr_8bit_en) {
		if (!(global_tx_common->config_csc_en && is_support_y422(prxcap))) {
			global_tx_common->hdr_transfer_feature = T_BT709;
			global_tx_common->hdr_color_feature = C_BT709;
			global_tx_common->colormetry = 0;
			hdmitx_set_sdr_pkt();
			schedule_work(&global_tx_common->work_hdr);
			hdmitx_tracer_write_event(global_tx_common->tx_tracer,
					HDMITX_HDR_MODE_SDR);
			spin_unlock_irqrestore(&global_tx_common->edid_spinlock, flags);
			return;
		}
	}

	if (hdr_status_pos == 4) {
		/* zero hdr10plus VSIF being sent - disable it */
		HDMITX_INFO("hdr: [%s]: disable hdr10plus zero vsif pkt\n", __func__);
		/* TODO, maybe need recover hdmi1.4b_vsif when 4k */
		hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, NULL);
		hdr_status_pos = 0;
	}

	if (global_tx_common->hdr_transfer_feature != hdr_transfer_feature ||
			global_tx_common->hdr_color_feature != hdr_color_feature ||
			global_tx_common->colormetry != colormetry) {
		global_tx_common->hdr_transfer_feature = hdr_transfer_feature;
		global_tx_common->hdr_color_feature = hdr_color_feature;
		global_tx_common->colormetry = colormetry;
		HDMITX_INFO("hdr: [%s]: tf = %d, cf = %d, colormetry = %d\n",
				    __func__, global_tx_common->hdr_transfer_feature,
				    global_tx_common->hdr_color_feature,
					global_tx_common->colormetry);
		if (global_tx_common->hdr_transfer_feature == T_SMPTE_ST_2084 &&
			global_tx_common->hdr_color_feature == C_BT2020)
			HDMITX_INFO("hdr: [%s]: SDR->HDR10\n", __func__);
		else if (global_tx_common->hdr_transfer_feature == T_HLG &&
				 global_tx_common->hdr_color_feature == C_BT2020)
			HDMITX_INFO("hdr: [%s]: SDR->HLG\n", __func__);
	}

	/* if VSIF/DV or VSIF/HDR10P packet is enabled, disable it */
	if (hdmitx_dv_en(tx_hw)) {
		hdmitx_hw_cntl_config(tx_hw, CONF_AVI_RGBYCC_INDIC,
			global_tx_common->fmt_para.cs);
		/* if using VSIF/DOVI, then only clear DV_VS10_SIG, else disable VSIF */
		if (hdmitx_hw_cntl_config(tx_hw, CONF_CLR_DV_VS10_SIG, 0) == 0)
			hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, NULL);
	}

	/* hdr10+ content on a hdr10 sink case */
	if (global_tx_common->hdr_transfer_feature == 0x30) {
		if (hdr_info->hdr10plus_info.ieeeoui != 0x90848B ||
				hdr_info->hdr10plus_info.application_version != 1) {
			global_tx_common->hdr_transfer_feature = T_SMPTE_ST_2084;
			HDMITX_INFO("hdr: [%s]: hdr10plus not supported, treat as hdr10\n",
				__func__);
		}
	}

	/* SDR */
	if (global_tx_common->hdr_transfer_feature == T_BT709 &&
		global_tx_common->hdr_color_feature == C_BT709) {
		/* send zero drm packet on HDR->SDR transition */
		if (hdmi_hdr_status == HDMI_HDR_SMPTE_2084 || hdmi_hdr_status == HDMI_HDR_HLG) {
			if (hdmi_hdr_status == HDMI_HDR_SMPTE_2084)
				HDMITX_INFO("hdr: [%s]: HDR10->SDR\n", __func__);
			else
				HDMITX_INFO("hdr: [%s]: HLG->SDR\n", __func__);
			global_tx_common->colormetry = 0;
			hdmitx_set_sdr_pkt();
			schedule_work(&global_tx_common->work_hdr);
			hdmitx_tracer_write_event(global_tx_common->tx_tracer, HDMITX_HDR_MODE_SDR);
			buffer[4] = 0;
		}
		/* back to previous cs */
		/*
		 * currently output y444,8bit or rgb,8bit, if exit playing,
		 * then switch back to 8bit mode
		 */
		if (global_tx_common->fmt_para.cs == HDMI_COLORSPACE_YUV444 &&
			global_tx_common->fmt_para.cd == COLORDEPTH_24B) {
			hdmitx_hw_cntl_config(tx_hw, CONFIG_CSC,
				CSC_Y444_8BIT | CSC_UPDATE_AVI_CS);
			HDMITX_INFO("hdr: [%s]: switch back to cs: %d, cd: %d\n",
				__func__, global_tx_common->fmt_para.cs,
				global_tx_common->fmt_para.cd);
		} else if (global_tx_common->fmt_para.cs == HDMI_COLORSPACE_RGB &&
			global_tx_common->fmt_para.cd == COLORDEPTH_24B) {
			hdmitx_hw_cntl_config(tx_hw, CONFIG_CSC,
				CSC_RGB_8BIT | CSC_UPDATE_AVI_CS);
			HDMITX_INFO("hdr: [%s]: switch back to cs: %d, cd: %d\n",
				__func__, global_tx_common->fmt_para.cs,
				global_tx_common->fmt_para.cd);
		}
		spin_unlock_irqrestore(&global_tx_common->edid_spinlock, flags);
		return;
	}

	buffer[5] = 0x0;
	buffer[6] = GET_LOW8BIT(data->primaries[0][0]);
	buffer[7] = GET_HIGH8BIT(data->primaries[0][0]);
	buffer[8] = GET_LOW8BIT(data->primaries[0][1]);
	buffer[9] = GET_HIGH8BIT(data->primaries[0][1]);
	buffer[10] = GET_LOW8BIT(data->primaries[1][0]);
	buffer[11] = GET_HIGH8BIT(data->primaries[1][0]);
	buffer[12] = GET_LOW8BIT(data->primaries[1][1]);
	buffer[13] = GET_HIGH8BIT(data->primaries[1][1]);
	buffer[14] = GET_LOW8BIT(data->primaries[2][0]);
	buffer[15] = GET_HIGH8BIT(data->primaries[2][0]);
	buffer[16] = GET_LOW8BIT(data->primaries[2][1]);
	buffer[17] = GET_HIGH8BIT(data->primaries[2][1]);
	buffer[18] = GET_LOW8BIT(data->white_point[0]);
	buffer[19] = GET_HIGH8BIT(data->white_point[0]);
	buffer[20] = GET_LOW8BIT(data->white_point[1]);
	buffer[21] = GET_HIGH8BIT(data->white_point[1]);
	buffer[22] = GET_LOW8BIT(data->luminance[0]);
	buffer[23] = GET_HIGH8BIT(data->luminance[0]);
	buffer[24] = GET_LOW8BIT(data->luminance[1]);
	buffer[25] = GET_HIGH8BIT(data->luminance[1]);
	buffer[26] = GET_LOW8BIT(data->max_content);
	buffer[27] = GET_HIGH8BIT(data->max_content);
	buffer[28] = GET_LOW8BIT(data->max_frame_average);
	buffer[29] = GET_HIGH8BIT(data->max_frame_average);

	/* bt2020 + gamma transfer */
	if (global_tx_common->hdr_transfer_feature == T_BT709 &&
			global_tx_common->hdr_color_feature == C_BT2020) {
		hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_DRM, NULL);
		hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020, SET_AVI_BT2020);
		spin_unlock_irqrestore(&global_tx_common->edid_spinlock, flags);
		return;
	}

	/* must clear hdr mode */
	global_tx_common->hdmi_current_hdr_mode = 0;

	/* HDR10 and (BT2020 or NON_STANDARD) */
	if (global_tx_common->hdr_transfer_feature == T_SMPTE_ST_2084) {
		if (global_tx_common->hdr_color_feature == C_BT2020)
			global_tx_common->hdmi_current_hdr_mode = 1;
		else if (global_tx_common->hdr_color_feature != C_BT2020)
			global_tx_common->hdmi_current_hdr_mode = 2;
	}
	/* HLG and BT2020 */
	if (global_tx_common->hdr_color_feature == C_BT2020 &&
			(global_tx_common->hdr_transfer_feature == T_BT2020_10 ||
			 global_tx_common->hdr_transfer_feature == T_HLG))
		global_tx_common->hdmi_current_hdr_mode = 3;

	switch (global_tx_common->hdmi_current_hdr_mode) {
	case 1:
		/* standard HDR SMPTE ST 2084 */
		buffer[4] = 0x02;
		hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_DRM, buffer);
		hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020, SET_AVI_BT2020);
		hdmitx_tracer_write_event(global_tx_common->tx_tracer, HDMITX_HDR_MODE_SMPTE2084);
		break;
	case 2:
		/* non standard SMPTE ST 2084 */
		buffer[4] = 0x02;
		hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_DRM, buffer);
		hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020, CLR_AVI_BT2020);
		break;
	case 3:
		/* HLG */
		buffer[4] = 0x03;
		hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_DRM, buffer);
		hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020, SET_AVI_BT2020);
		hdmitx_tracer_write_event(global_tx_common->tx_tracer, HDMITX_HDR_MODE_HLG);
		break;
	case 0:
	default:
		/* other case */
		hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_DRM, NULL);
		hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020, CLR_AVI_BT2020);
		break;
	}

	if (global_tx_common->hdmi_current_hdr_mode != global_tx_common->hdmi_last_hdr_mode) {
		save_last_hdr_mode = global_tx_common->hdmi_last_hdr_mode;
		/* NOTE: for HDR <-> HLG, also need update last mode */
		global_tx_common->hdmi_last_hdr_mode = global_tx_common->hdmi_current_hdr_mode;
		/* only SDR->HDR10/SDR->HLG need mute */
		if (hdr_mute_frame && save_last_hdr_mode == 0) {
			hdmitx_video_mute_op(true);
			HDMITX_INFO("hdr: video mute %d frames\n", hdr_mute_frame);
			/*
			 * force unmute after specific frames,
			 * no need to check hdr status when unmute
			 */
			schedule_work(&global_tx_common->work_hdr_unmute);
		}
		schedule_work(&global_tx_common->work_hdr);
	}
	/* if 8bit hdr mode is enabled, no need small mode switch to y422,12bit */
	if (global_tx_common->hdr_8bit_en) {
		spin_unlock_irqrestore(&global_tx_common->edid_spinlock, flags);
		return;
	}
	if (global_tx_common->hdmi_current_hdr_mode == 1 ||
		global_tx_common->hdmi_current_hdr_mode == 2 ||
		global_tx_common->hdmi_current_hdr_mode == 3) {
		/*
		 * currently output y444,8bit or rgb,8bit, and EDID
		 * support Y422, then switch to y422,12bit mode
		 */
		if ((global_tx_common->fmt_para.cs == HDMI_COLORSPACE_YUV444 ||
			global_tx_common->fmt_para.cs == HDMI_COLORSPACE_RGB) &&
			global_tx_common->fmt_para.cd == COLORDEPTH_24B &&
			is_support_y422(prxcap)) {
			hdmitx_hw_cntl_config(tx_hw, CONFIG_CSC,
				CSC_Y422_12BIT | CSC_UPDATE_AVI_CS);
			HDMITX_DEBUG_PACKET("hdr: [%s]: switch to 422,12bit\n", __func__);
		}
	}
	spin_unlock_irqrestore(&global_tx_common->edid_spinlock, flags);
}

void hdmitx_set_hdr10plus_pkt(unsigned int flag, struct hdr10plus_para *data)
{
	unsigned char buffer[31] = {0x81, 0x01, 0x1b};
	struct hdmitx_hw_common *tx_hw = global_tx_common->tx_hw;
	u32 vic = global_tx_common->fmt_para.vic;
	u32 hdmi_vic_4k_flag = 0;
	struct rx_cap *prxcap = &global_tx_common->rxcap;

	HDMITX_DEBUG_PACKET("hdr: [%s]: [%d]\n", __func__, __LINE__);
	if (data)
		memcpy(&global_tx_common->hdr10p_config_data, data, sizeof(struct hdr10plus_para));
	else
		memset(&global_tx_common->hdr10p_config_data, 0, sizeof(struct hdr10plus_para));

	/* if ready is 0, only can clear pkt */
	if (global_tx_common->ready == 0)
		return;

	/* save vic to AVI when send HDR10P_VSIF */
	if (vic == HDMI_95_3840x2160p30_16x9 ||
		vic == HDMI_94_3840x2160p25_16x9 ||
		vic == HDMI_93_3840x2160p24_16x9 ||
		vic == HDMI_98_4096x2160p24_256x135)
		hdmi_vic_4k_flag = 1;

	if (flag == HDR10_PLUS_ZERO_VSIF) {
		/* needed during hdr10plus to sdr transition */
		HDMITX_INFO("hdr: [%s]: HDR10PLUS->SDR, send zero vsif pkt\n", __func__);
		hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, buffer);
		hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020, CLR_AVI_BT2020);
		global_tx_common->hdr10plus_feature = 0;
		if (hdmi_vic_4k_flag)
			hdmitx_hw_cntl_config(tx_hw, CONF_AVI_VIC, vic & 0xff);
		hdr_status_pos = 4;
		/* When hdr10plus mode ends, clear hdr10plus_event flag */
		hdmitx_tracer_clean_hdr10plus_event(global_tx_common->tx_tracer,
					HDMITX_HDR_MODE_HDR10PLUS);
		hdmitx_tracer_write_event(global_tx_common->tx_tracer,
					HDMITX_HDR_MODE_SDR);
		return;
	}

	if (global_tx_common->hdr10plus_feature != flag) {
		global_tx_common->hdr10plus_feature = flag;
		if (flag == HDR10_PLUS_ENABLE_VSIF)
			HDMITX_INFO("hdr: [%s]: SDR->HDR10PLUS\n", __func__);
		else if (flag == HDR10_PLUS_DISABLE_VSIF)
			HDMITX_INFO("hdr: [%s]: HDR10PLUS->SDR\n", __func__);
	}

	if (!data || !flag) {
		HDMITX_INFO("hdr: [%s]: disable hdr10plus vsif pkt\n", __func__);
		/* TODO, maybe need recover hdmi1.4b_vsif when 4k */
		hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR,  NULL);
		hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020, CLR_AVI_BT2020);
		global_tx_common->hdr10plus_feature = 0;
		/* When hdr10plus mode ends, clear hdr10plus_event flag */
		hdmitx_tracer_clean_hdr10plus_event(global_tx_common->tx_tracer,
					HDMITX_HDR_MODE_HDR10PLUS);
		hdmitx_tracer_write_event(global_tx_common->tx_tracer,
					HDMITX_HDR_MODE_SDR);

		return;
	}

	/*
	 * if currently output 8bit, and EDID don't support Y422, and config_csc_en is 0,
	 * switch to SDR output; if hdr_8bit_en is 1, switch to HDR output
	 */
	if (global_tx_common->fmt_para.cd == COLORDEPTH_24B && !global_tx_common->hdr_8bit_en) {
		if (!(global_tx_common->config_csc_en && is_support_y422(prxcap))) {
			global_tx_common->hdr_transfer_feature = T_BT709;
			global_tx_common->hdr_color_feature = C_BT709;
			global_tx_common->colormetry = 0;
			hdmitx_set_sdr_pkt();
			schedule_work(&global_tx_common->work_hdr);
			return;
		}
	}

	buffer[4] = 0x8b;
	buffer[5] = 0x84;
	buffer[6] = 0x90;

	buffer[7] = ((data->application_version & 0x3) << 6) |
		((data->targeted_max_lum & 0x1f) << 1);
	buffer[8] = data->average_maxrgb;
	buffer[9] = data->distribution_values[0];
	buffer[10] = data->distribution_values[1];
	buffer[11] = data->distribution_values[2];
	buffer[12] = data->distribution_values[3];
	buffer[13] = data->distribution_values[4];
	buffer[14] = data->distribution_values[5];
	buffer[15] = data->distribution_values[6];
	buffer[16] = data->distribution_values[7];
	buffer[17] = data->distribution_values[8];
	buffer[18] = ((data->num_bezier_curve_anchors & 0xf) << 4) |
		((data->knee_point_x >> 6) & 0xf);
	buffer[19] = ((data->knee_point_x & 0x3f) << 2) |
		((data->knee_point_y >> 8) & 0x3);
	buffer[20] = data->knee_point_y  & 0xff;
	buffer[21] = data->bezier_curve_anchors[0];
	buffer[22] = data->bezier_curve_anchors[1];
	buffer[23] = data->bezier_curve_anchors[2];
	buffer[24] = data->bezier_curve_anchors[3];
	buffer[25] = data->bezier_curve_anchors[4];
	buffer[26] = data->bezier_curve_anchors[5];
	buffer[27] = data->bezier_curve_anchors[6];
	buffer[28] = data->bezier_curve_anchors[7];
	buffer[29] = data->bezier_curve_anchors[8];
	buffer[30] = ((data->graphics_overlay_flag & 0x1) << 7) |
		((data->no_delay_flag & 0x1) << 6);

	hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, buffer);
	hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020, SET_AVI_BT2020);
	if (hdmi_vic_4k_flag)
		hdmitx_hw_cntl_config(tx_hw, CONF_AVI_VIC, vic & 0xff);
	hdmitx_tracer_write_event(global_tx_common->tx_tracer,
				HDMITX_HDR_MODE_HDR10PLUS);
}

void hdmitx_set_vsif_pkt(enum eotf_type type,
				enum mode_type tunnel_mode,
				struct dv_vsif_para *data,
				bool signal_sdr)
{
	struct dv_vsif_para para = {0};
	unsigned char buffer1[31] = {0x81, 0x01};
	unsigned char buffer2[31] = {0x81, 0x01};
	unsigned char len = 0;
	unsigned int vic = global_tx_common->fmt_para.vic;
	unsigned int hdmi_vic_4k_flag = 0;
	struct hdmitx_hw_common *tx_hw = global_tx_common->tx_hw;
	unsigned long flags;
	enum hdmi_tf_type hdr_type = HDMI_NONE;

	HDMITX_DEBUG_PACKET("hdr: [%s]: [%d]\n", __func__, __LINE__);
	if (!data)
		memcpy(&global_tx_common->vsif_debug_info.data, &para, sizeof(struct dv_vsif_para));
	else
		memcpy(&global_tx_common->vsif_debug_info.data, data, sizeof(struct dv_vsif_para));

	spin_lock_irqsave(&global_tx_common->edid_spinlock, flags);

	/* if ready is 0, only can clear pkt */
	if (global_tx_common->ready == 0 && type != EOTF_T_NULL) {
		spin_unlock_irqrestore(&global_tx_common->edid_spinlock, flags);
		return;
	}

	if (global_tx_common->hdmi_current_eotf_type != type ||
		global_tx_common->hdmi_current_tunnel_mode != tunnel_mode ||
		global_tx_common->hdmi_current_signal_sdr != signal_sdr) {
		global_tx_common->hdmi_current_eotf_type = type;
		global_tx_common->hdmi_current_tunnel_mode = tunnel_mode;
		global_tx_common->hdmi_current_signal_sdr = signal_sdr;
		HDMITX_INFO("hdr: [%s]: type = %d, tunnel_mode = %d, signal_sdr = %d\n",
			__func__, type, tunnel_mode, signal_sdr);
		if (type == EOTF_T_DOLBYVISION &&
				tunnel_mode == RGB_8BIT && signal_sdr != true)
			HDMITX_INFO("hdr: [%s]: SDR->MESON_DOLBYVISION Sink-led\n", __func__);
		else if (type == EOTF_T_LL_MODE &&
				tunnel_mode == YUV422_BIT12 && signal_sdr != true)
			HDMITX_INFO("hdr: [%s]: SDR->MESON_DOLBYVISION Source-led\n", __func__);
	}

	/* if DRM/HDR packet is enabled but next will output DV, then disable it */
	hdr_type = hdmitx_hw_get_hdr_st(tx_hw);
	if (hdr_type != HDMI_NONE &&
		(type == EOTF_T_DV_AHEAD ||
		type == EOTF_T_DOLBYVISION ||
		type == EOTF_T_LL_MODE)) {
		global_tx_common->hdr_transfer_feature = T_BT709;
		global_tx_common->hdr_color_feature = C_BT709;
		global_tx_common->colormetry = 0;
		hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020, CLR_AVI_BT2020);
		hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_DRM, NULL);
	}

	if (vic == HDMI_95_3840x2160p30_16x9 || vic == HDMI_94_3840x2160p25_16x9 ||
			vic == HDMI_93_3840x2160p24_16x9 || vic == HDMI_98_4096x2160p24_256x135)
		hdmi_vic_4k_flag = 1;

	/* ver0 and ver1_15 and ver1_12bit with ll= 0 use hdmi 1.4b VSIF */
	if (global_tx_common->rxcap.dv_info.ver == 0 ||
			(global_tx_common->rxcap.dv_info.ver == 1 &&
			 global_tx_common->rxcap.dv_info.length == 0xE) ||
			(global_tx_common->rxcap.dv_info.ver == 1 &&
			 global_tx_common->rxcap.dv_info.length == 0xB &&
			 global_tx_common->rxcap.dv_info.low_latency == 0)) {
		switch (type) {
		case EOTF_T_DOLBYVISION:
			len = 0x18;
			break;
		case EOTF_T_HDR10:
		case EOTF_T_SDR:
		case EOTF_T_NULL:
		default:
			len = 0x05;
			break;
		}

		buffer1[2] = len;
		buffer1[4] = 0x03;
		buffer1[5] = 0x0c;
		buffer1[6] = 0x00;
		buffer1[7] = 0x00;

		if (hdmi_vic_4k_flag) {
			buffer1[7] = 0x20;
			if (vic == HDMI_95_3840x2160p30_16x9)
				buffer1[8] = 0x1;
			else if (vic == HDMI_94_3840x2160p25_16x9)
				buffer1[8] = 0x2;
			else if (vic == HDMI_93_3840x2160p24_16x9)
				buffer1[8] = 0x3;
			else if (vic == HDMI_98_4096x2160p24_256x135)
				buffer1[8] = 0x4;
		}
		if (type == EOTF_T_DV_AHEAD) {
			hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, buffer1);
			spin_unlock_irqrestore(&global_tx_common->edid_spinlock, flags);
			return;
		}
		if (type == EOTF_T_DOLBYVISION) {
			/*
			 * disable forced gaming in this mode because if we are
			 * here with forced gaming on, it means TV is not DV LL
			 * capable
			 */
			if (global_tx_common->ll_user_set_mode == HDMI_LL_MODE_ENABLE &&
					(global_tx_common->allm_mode == 1 ||
					 global_tx_common->ct_mode == 1)) {
				HDMITX_DEBUG_PACKET("hdr: amdv H14b VSIF, disable game mode\n");
				hdmitx_hw_cntl_config(tx_hw, CONFIG_ALLM, DISABLE_ALLM);
			}
			/* first disable drm package */
			hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_DRM, NULL);
			hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, buffer1);
			/*
			 * amdolby_vision Source System-on-Chip Platform Kit Version 2.6:
			 * 4.4.1 Expected AVI-IF for Dolby Vision output, need BT2020 for DV
			 */
			hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020, SET_AVI_BT2020);
			if (tunnel_mode == RGB_8BIT) {
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_RGBYCC_INDIC,
						HDMI_COLORSPACE_RGB);
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_Q01,
						RGB_RANGE_FUL);
				/* to test, if needed */
				/* hdev->hwop.cntlconfig(hdev, CONFIG_CSC, CSC_Y444_8BIT); */
				/* if (log_level == 0xfd) */
					/* HDMITX_INFO("Dolby H14b VSIF, */
					/* switch to y444 csc\n"); */
				hdmitx_tracer_write_event(global_tx_common->tx_tracer,
						HDMITX_HDR_MODE_DV_STD);
			} else {
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_RGBYCC_INDIC,
						HDMI_COLORSPACE_YUV422);
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_YQ01,
						YCC_RANGE_FUL);
				hdmitx_tracer_write_event(global_tx_common->tx_tracer,
						HDMITX_HDR_MODE_DV_LL);
			}
			if (hdmi_vic_4k_flag)
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_VIC, vic & 0xff);
		} else {
			if (hdmi_vic_4k_flag) {
				hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, buffer1);
				/* clear vic from AVI */
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_VIC, 0);
				HDMITX_DEBUG_PACKET("hdr: amdv exit, send H14b VSIF with vic: %d\n",
						buffer1[8]);
			} else {
				hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, NULL);
				HDMITX_INFO("hdr: amdv exit, disable H14b VSIF\n");
			}
			if (signal_sdr) {
				HDMITX_INFO("hdr: [%s]: switch signal to SDR\n", __func__);
				hdmitx_hw_cntl_config(tx_hw,
					CONF_AVI_RGBYCC_INDIC, global_tx_common->fmt_para.cs);
				hdmitx_hw_cntl_config(tx_hw,
					CONF_AVI_Q01, RGB_RANGE_LIM);
				hdmitx_hw_cntl_config(tx_hw,
					CONF_AVI_YQ01, YCC_RANGE_LIM);
				/* BT709 */
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020,
					CLR_AVI_BT2020);
				/* if TV support traditional SDR, then recover hdr.sdr packet */
				/* if (hdev->tx_comm.rxcap.hdr_info2.hdr_support & 0x1) { */
				/* HDMITX_DEBUG_PACKET("%s: recover hdr.sdr pkt\n", __func__); */
				/* hdmi_drm_infoframe_init(&hdev->infoframes.drm.drm); */
				/* hdmi_drm_infoframe_set(&hdev->infoframes.drm.drm); */
				/* } */
				/* re-enable forced game mode if selected by the user */
				if (global_tx_common->ll_user_set_mode == HDMI_LL_MODE_ENABLE) {
					HDMITX_INFO("hdr: amdv H14b VSIF OFF, enable game mode\n");
					hdmitx_hw_cntl_config(tx_hw, CONFIG_ALLM, ENABLE_ALLM);
				}
				hdmitx_tracer_write_event(global_tx_common->tx_tracer,
					HDMITX_HDR_MODE_SDR);
			}
		}
	}
	/* ver1_12  with low_latency = 1 and ver2 use Dolby VSIF */
	if (global_tx_common->rxcap.dv_info.ver == 2 ||
			(global_tx_common->rxcap.dv_info.ver == 1 &&
			 global_tx_common->rxcap.dv_info.length == 0xB &&
			 global_tx_common->rxcap.dv_info.low_latency == 1) ||
			type == EOTF_T_LL_MODE) {
		if (!data)
			data = &para;
		len = 0x1b;

		buffer2[2] = len;
		buffer2[4] = 0x46;
		buffer2[5] = 0xd0;
		buffer2[6] = 0x00;
		if (data->ver2_l11_flag == 1) {
			buffer2[7] = data->vers.ver2_l11.low_latency |
				     data->vers.ver2_l11.dobly_vision_signal << 1 |
				     data->vers.ver2_l11.src_dm_version << 5;
			/* L11_MD_Present */
			buffer2[8] = data->vers.ver2_l11.eff_tmax_PQ_hi
				     | data->vers.ver2_l11.auxiliary_MD_present << 6
				     | data->vers.ver2_l11.backlt_ctrl_MD_present << 7
				     | 0x20;
			buffer2[9] = data->vers.ver2_l11.eff_tmax_PQ_low;
			buffer2[10] = data->vers.ver2_l11.auxiliary_runmode;
			buffer2[11] = data->vers.ver2_l11.auxiliary_runversion;
			buffer2[12] = data->vers.ver2_l11.auxiliary_debug0;
			buffer2[13] = (data->vers.ver2_l11.content_type)
				| (data->vers.ver2_l11.content_sub_type << 4);
			buffer2[14] = (data->vers.ver2_l11.intended_white_point)
				| (data->vers.ver2_l11.crf << 4);
			buffer2[15] = data->vers.ver2_l11.l11_byte2;
			buffer2[16] = data->vers.ver2_l11.l11_byte3;
		} else {
			buffer2[7] = (data->vers.ver2.low_latency) |
				(data->vers.ver2.dobly_vision_signal << 1) |
				(data->vers.ver2.src_dm_version << 5);
			buffer2[8] = (data->vers.ver2.eff_tmax_PQ_hi)
				| (data->vers.ver2.auxiliary_MD_present << 6)
				| (data->vers.ver2.backlt_ctrl_MD_present << 7);
			buffer2[9] = data->vers.ver2.eff_tmax_PQ_low;
			buffer2[10] = data->vers.ver2.auxiliary_runmode;
			buffer2[11] = data->vers.ver2.auxiliary_runversion;
			buffer2[12] = data->vers.ver2.auxiliary_debug0;
		}
		if (type == EOTF_T_DV_AHEAD) {
			hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, buffer2);
			spin_unlock_irqrestore(&global_tx_common->edid_spinlock, flags);
			return;
		}
		/* Dolby Vision standard case */
		if (type == EOTF_T_DOLBYVISION) {
			/*
			 * disable forced gaming in this mode because if we are
			 * here with forced gaming on, it means TV is not DV LL
			 * capable
			 */
			if (global_tx_common->ll_user_set_mode == HDMI_LL_MODE_ENABLE &&
					(global_tx_common->allm_mode == 1 ||
					 global_tx_common->ct_mode == 1)) {
				HDMITX_DEBUG_PACKET("hdr: amdv VSIF, disable game mode\n");
				hdmitx_hw_cntl_config(tx_hw, CONFIG_ALLM, DISABLE_ALLM);
			}
			/* first disable drm package */
			hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_DRM, NULL);
			hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, buffer2);
			/* Dolby Vision Source System-on-Chip Platform Kit Version 2.6:
			 * 4.4.1 Expected AVI-IF for Dolby Vision output, need BT2020 for DV
			 */
			hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020, SET_AVI_BT2020);
			/* RGB444 */
			if (tunnel_mode == RGB_8BIT) {
				hdmitx_hw_cntl_config(tx_hw,
					CONF_AVI_RGBYCC_INDIC,
					HDMI_COLORSPACE_RGB);
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_Q01, RGB_RANGE_FUL);
				hdmitx_tracer_write_event(global_tx_common->tx_tracer,
					HDMITX_HDR_MODE_DV_STD);
				/* to test, if needed */
				/* hdev->hwop.cntlconfig(hdev, CONFIG_CSC, CSC_Y444_8BIT); */
				/* if (log_level == 0xfd) */
					/*HDMITX_INFO("Dolby STD, switch to y444 csc\n");*/
			} else {
				/* YUV422 */
				hdmitx_hw_cntl_config(tx_hw,
					CONF_AVI_RGBYCC_INDIC,
					HDMI_COLORSPACE_YUV422);
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_YQ01,
					YCC_RANGE_FUL);
			}
			if (hdmi_vic_4k_flag)
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_VIC, vic & 0xff);
		/* Dolby Vision low-latency case */
		} else if (type == EOTF_T_LL_MODE) {
			/*
			 * make sure forced game mode is enabled as there could be DV std
			 * to DV LL transition during uboot to kernel transition because
			 * of game mode forced enabled by user.
			 */
			if (global_tx_common->ll_user_set_mode == HDMI_LL_MODE_ENABLE &&
					global_tx_common->allm_mode == 0 &&
					global_tx_common->ct_mode == 0) {
				HDMITX_DEBUG_PACKET("hdr: amdv LL VSIF, enable forced game mode\n");
				hdmitx_hw_cntl_config(tx_hw, CONFIG_ALLM, ENABLE_ALLM);
			}
			/* first disable drm package */
			hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_DRM, NULL);
			hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, buffer2);
			/* Dolby vision HDMI Signaling Case25,
			 * UCD323 not declare bt2020 colorimetry,
			 * need to forcely send BT.2020
			 */
			hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020, SET_AVI_BT2020);
			/* 10/12bit RGB444 */
			if (tunnel_mode == RGB_10_12BIT) {
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_RGBYCC_INDIC,
						HDMI_COLORSPACE_RGB);
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_Q01, RGB_RANGE_LIM);
			} else if (tunnel_mode == YUV444_10_12BIT) {
				/* 10/12bit YUV444 */
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_RGBYCC_INDIC,
						HDMI_COLORSPACE_YUV444);
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_YQ01, YCC_RANGE_LIM);
			} else {
				/* YUV422 */
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_RGBYCC_INDIC,
						HDMI_COLORSPACE_YUV422);
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_YQ01,
						YCC_RANGE_LIM);
			}
			if (hdmi_vic_4k_flag)
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_VIC, vic & 0xff);

			hdmitx_tracer_write_event(global_tx_common->tx_tracer,
					HDMITX_HDR_MODE_DV_LL);
		} else {
			/* SDR case */
			if (hdmi_vic_4k_flag) {
				/* recover HDMI1.4b_VSIF */
				buffer1[2] = 0x5;
				buffer1[4] = 0x03;
				buffer1[5] = 0x0c;
				buffer1[6] = 0x00;
				buffer1[7] = 0x20;

				if (vic == HDMI_95_3840x2160p30_16x9)
					buffer1[8] = 0x1;
				else if (vic == HDMI_94_3840x2160p25_16x9)
					buffer1[8] = 0x2;
				else if (vic == HDMI_93_3840x2160p24_16x9)
					buffer1[8] = 0x3;
				else if (vic == HDMI_98_4096x2160p24_256x135)
					buffer1[8] = 0x4;

				hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, buffer1);
				HDMITX_DEBUG_PACKET("hdr: amdv exit, send H14b VSIF with vic: %d\n",
						buffer1[8]);
				/* clear vic from AVI */
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_VIC, 0);
			} else {
				HDMITX_INFO("hdr: amdv exit, amdv_signal = %d\n", buffer2[7]);
				hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, buffer2);
			}

			if (signal_sdr) {
				HDMITX_INFO("hdr: [%s]: switch signal to SDR\n", __func__);
				hdmitx_hw_cntl_config(tx_hw,
					CONF_AVI_RGBYCC_INDIC, global_tx_common->fmt_para.cs);
				hdmitx_hw_cntl_config(tx_hw,
					CONF_AVI_Q01, RGB_RANGE_DEFAULT);
				hdmitx_hw_cntl_config(tx_hw,
					CONF_AVI_YQ01, YCC_RANGE_LIM);
				/* BT709 */
				hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020,
					CLR_AVI_BT2020);
				hdmitx_tracer_write_event(global_tx_common->tx_tracer,
					HDMITX_HDR_MODE_SDR);
				/* if TV support traditional SDR, then recover hdr.sdr packet */
				/* if (hdev->tx_comm.rxcap.hdr_info2.hdr_support & 0x1) { */
				/* HDMITX_DEBUG_PACKET("%s: recover hdr.sdr pkt\n", __func__); */
				/* hdmi_drm_infoframe_init(&hdev->infoframes.drm.drm); */
				/* hdmi_drm_infoframe_set(&hdev->infoframes.drm.drm); */
				/* } */
				/* re-enable forced game mode if selected by the user */
				if (global_tx_common->ll_user_set_mode == HDMI_LL_MODE_ENABLE) {
					HDMITX_INFO("hdr: amdv VSIF disabled, enable game mode\n");
					hdmitx_hw_cntl_config(tx_hw, CONFIG_ALLM, ENABLE_ALLM);
				}
			}
		}
	}

	/* config dither */
	hdmitx_hw_cntl_config(tx_hw, CONFIG_DITHER, 0);
	spin_unlock_irqrestore(&global_tx_common->edid_spinlock, flags);
}

void hdmitx_set_cuva_hdr_vsif(struct cuva_hdr_vsif_para *data)
{
	unsigned long flags = 0;
	unsigned char buffer[31] = {0x81, 0x01, 0x1b};
	struct hdmitx_hw_common *tx_hw = global_tx_common->tx_hw;
	unsigned int vic = global_tx_common->fmt_para.vic;
	unsigned int hdmi_vic_4k_flag = 0;

	spin_lock_irqsave(&global_tx_common->edid_spinlock, flags);
	if (vic == HDMI_95_3840x2160p30_16x9 || vic == HDMI_94_3840x2160p25_16x9 ||
			vic == HDMI_93_3840x2160p24_16x9 || vic == HDMI_98_4096x2160p24_256x135)
		hdmi_vic_4k_flag = 1;
	if (!data) {
		if (hdmi_vic_4k_flag) {
			buffer[2] = 0x5;
			buffer[4] = 0x03;
			buffer[5] = 0x0c;
			buffer[6] = 0x00;
			buffer[7] = 0x20;
			if (vic == HDMI_95_3840x2160p30_16x9)
				buffer[8] = 0x1;
			else if (vic == HDMI_94_3840x2160p25_16x9)
				buffer[8] = 0x2;
			else if (vic == HDMI_93_3840x2160p24_16x9)
				buffer[8] = 0x3;
			else if (vic == HDMI_98_4096x2160p24_256x135)
				buffer[8] = 0x4;
			hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, buffer);
			/* clear vic from AVI */
			hdmitx_hw_cntl_config(tx_hw, CONF_AVI_VIC, 0);
			HDMITX_INFO("hdr: [%s]: recover hdmi1.4b_vsif\n", __func__);
		} else {
			hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, NULL);
			HDMITX_INFO("hdr: [%s]: clear vendor infoframe\n", __func__);
		}
		spin_unlock_irqrestore(&global_tx_common->edid_spinlock, flags);
		return;
	}
	buffer[4] = GET_OUI_BYTE0(CUVA_IEEEOUI);
	buffer[5] = GET_OUI_BYTE1(CUVA_IEEEOUI);
	buffer[6] = GET_OUI_BYTE2(CUVA_IEEEOUI);
	buffer[7] = data->system_start_code;
	buffer[8] = (data->version_code & 0xf) << 4;
	hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, buffer);
	if (hdmi_vic_4k_flag)
		hdmitx_hw_cntl_config(tx_hw, CONF_AVI_VIC, vic & 0xff);
	hdmitx_tracer_write_event(global_tx_common->tx_tracer,
				HDMITX_HDR_MODE_CUVA);
	spin_unlock_irqrestore(&global_tx_common->edid_spinlock, flags);
}

void hdmitx_clear_all_infoframe_pkt(struct hdmitx_common *tx_comm)
{
	struct hdmitx_hw_common *tx_hw = tx_comm->tx_hw;
	unsigned long flags;

	spin_lock_irqsave(&tx_comm->edid_spinlock, flags);

	HDMITX_INFO("hdr: clear all hdmitx infoframe\n");
	/* step1 HW: clear packets */
	hdmitx_hw_set_packet(tx_hw, HDMI_PACKET_DRM, NULL);
	hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR, NULL);
	hdmitx_hw_set_packet(tx_hw, HDMI_INFOFRAME_TYPE_VENDOR2, NULL);
	hdmitx_hw_cntl_config(tx_hw, CONF_AVI_BT2020, CLR_AVI_BT2020);
	hdmitx_hw_cntl_config(tx_hw, CONF_CLR_AVI_PACKET, 0);
	/* step2 SW: reset para */
	tx_comm->hdr_transfer_feature = T_UNKNOWN;
	tx_comm->hdr_color_feature = C_UNKNOWN;
	tx_comm->colormetry = 0;
	tx_comm->hdmi_current_hdr_mode = 0;
	tx_comm->hdmi_last_hdr_mode = 0;
	tx_comm->hdr10plus_feature = 0;

	spin_unlock_irqrestore(&tx_comm->edid_spinlock, flags);
}

void hdmitx_hdr_init(struct hdmitx_common *tx_comm)
{
	global_tx_common = tx_comm;
	/* init hdr para */
	global_tx_common->last_drm_eotf = HDMI_EOTF_TRADITIONAL_GAMMA_SDR;
	global_tx_common->hdr_transfer_feature = T_UNKNOWN;
	global_tx_common->hdr_color_feature = C_UNKNOWN;
	global_tx_common->colormetry = 0;
	/* init hdr10plus flag */
	global_tx_common->hdr10plus_feature = 0;
	/* init amdv para */
	global_tx_common->hdmi_current_eotf_type = EOTF_T_NULL;
	global_tx_common->hdmi_current_tunnel_mode = YUV422_BIT12;
	global_tx_common->hdmi_current_signal_sdr = 0;
}

