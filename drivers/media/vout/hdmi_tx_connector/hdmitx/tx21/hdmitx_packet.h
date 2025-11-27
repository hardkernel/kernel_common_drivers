/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMITX_PACKET_H
#define __HDMITX_PACKET_H

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/hdmi.h>

#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>

#include "hdmi_tx_connector/hdmitx_infoframe.h"

#define HDMI_INFOFRAME_TYPE_EMP 0x7f
/* SBTM-EM PKT use GEN5*/
#define HDMI_INFOFRAME_TYPE_SBTM 0xA
#define HDMI_PACKET_TYPE_GCP 0x3
#define HDMI_INFOFRAME_EMP_VRR_GAME ((HDMI_INFOFRAME_TYPE_EMP << 8) | (EMP_TYPE_VRR_GAME))
#define HDMI_INFOFRAME_EMP_VRR_QMS ((HDMI_INFOFRAME_TYPE_EMP << 8) | (EMP_TYPE_VRR_QMS))
#define HDMI_INFOFRAME_EMP_VRR_SBTM ((HDMI_INFOFRAME_TYPE_EMP << 8) | (EMP_TYPE_SBTM))
#define HDMI_INFOFRAME_EMP_VRR_DSC ((HDMI_INFOFRAME_TYPE_EMP << 8) | (EMP_TYPE_DSC))
#define HDMI_INFOFRAME_EMP_VRR_DHDR ((HDMI_INFOFRAME_TYPE_EMP << 8) | (EMP_TYPE_DHDR))
#define HDMI_INFOFRAME_TYPE_VENDOR2 (0x81 | 0x100)

enum emp_component_conf {
	CONF_HEADER_INIT,
	CONF_HEADER_LAST,
	CONF_HEADER_FIRST,
	CONF_HEADER_SEQ_INDEX,
	CONF_SYNC,
	CONF_VFR,
	CONF_AFR,
	CONF_DS_TYPE,
	CONF_END,
	CONF_NEW,
	CONF_ORG_ID,
	CONF_DATA_SET_TAG,
	CONF_DATA_SET_LENGTH,
	CONF_VRR_EN,
	CONF_FACTOR_M1,
	CONF_QMS_EN,
	CONF_M_CONST,
	CONF_BASE_VFRONT,
	CONF_NEXT_TFR,
	CONF_BASE_REFRESH_RATE,
	CONF_SBTM_VER,
	CONF_SBTM_MODE,
	CONF_SBTM_TYPE,
	CONF_SBTM_GRDM_MIN,
	CONF_SBTM_GRDM_LUM,
	CONF_SBTM_FRMPBLIMITINT,
};

/* Class 0 video timing extended metadata structure for game/fva, 2.1A P445 */
struct vtem_gamevrr_st {
	u8 vrr_en:1; /* MD0 */
	u8 fva_factor_m1:4;
	u8 base_vfront; /* MD1 */
	u16 brr_rate; /* MD2/3 */
};

/* Class 1 video timing extended metadata structure for qms, 2.1A P445 */
struct vtem_qmsvrr_st {
	u8 m_const:1; /* MD0 */
	u8 qms_en:1;
	u8 base_vfront; /* MD1 */
	u16 brr_rate; /* MD2/3 */
	enum TARGET_FRAME_RATE next_tfr:5;
};

struct emp_packet_header {
	u8 header; /* hb0, fixed value 0x7f */
	u8 last:1; /* hb1 */
	u8 first:1;
	u8 seq_idx; /* hb2 */
};

struct emp_packet_0_body {
	u8 sync:1; /* pb0 synchronous metadata */
	u8 vfr:1; /* video format related, cs/cd/resolution */
	u8 afr:1; /* audio format related */
	/* 2b00: periodic pseudo-static MD
	 * 2b01: periodic dynamic MD
	 * 2b10: unique MD
	 */
	u8 ds_type:2;
	u8 end:1;
	u8 new:1;
	/* pb2  0: vendor specific MD 1: defined by 2.1
	 * 2: defined by CTA-861-G  3: defined by VESA
	 */
	u8 org_id;
	u16 ds_tag; /* pb3/4 */
	u16 ds_length; /* pb5/6 */
	union {
		struct vtem_gamevrr_st game_md;
		struct vtem_qmsvrr_st qms_md;
		struct vtem_sbtm_st sbtm_md;
		u8 md[21]; /* pb7~pb27, md0~md20 */
	} md;
};

struct emp_packet_n_body {
	u8 md[28]; /* md(x)~md(x+27) */
};

/* extended metadata packet, 2.1A P304, no checksum in the PB0 */
struct emp_packet_st {
	enum emp_type type;
	struct emp_packet_header header;
	union {
		struct emp_packet_0_body emp0;
		struct emp_packet_n_body empn;
	} body;
};

/* general packet send api */
void hdmitx_infoframe_send(u16 info_type, u8 *body);
int hdmitx_infoframe_raw_get(u16 info_type, u8 *body);

/* there are 2 ways to send out infoframe
 * xxx_infoframe_set() will take use of struct xxx_infoframe_set
 * xxx_infoframe_rawset() will directly send with rawdata
 * if info, hb, or pb == NULL, disable send infoframe
 */

/* avi raw data get from reg */
int hdmi_avi_infoframe_get(u8 *body);
/* avi set */
void hdmi_avi_infoframe_set(struct hdmi_avi_infoframe *info);
/* avi raw set, for backup */
void hdmi_avi_infoframe_rawset(u8 *hb, u8 *pb);
/* avi param_config */
int hdmi_avi_infoframe_config(struct hdmitx_common *tx_comm, u32 avi_cmd, u8 val);
/* vsif set, deprecated */
void hdmi_vend_infoframe_set(struct hdmitx_common *tx_comm, struct hdmi_vendor_infoframe *info);
/* vsif set, only used for DV_VSIF / HDMI1.4b_VSIF / CUVA_VSIF / HDR10+ VSIF */
void hdmi_vend_infoframe_rawset(struct hdmitx_common *tx_comm, u8 *body);
/* only used for HF-VSIF */
void hdmi_vend_infoframe2_rawset(struct hdmitx_common *tx_comm, u8 *body);
/* vsif raw data get from reg, only used for DV_VSIF / CUVA VSIF / HDMI1.4b_VSIF / HDR10+ VSIF */
int hdmi_vend_infoframe_get(struct hdmitx_common *tx_comm, u8 *body);

/* spd raw data get from reg */
int hdmi_spd_infoframe_get(u8 *body);
/* spd infoframe set */
void hdmi_spd_infoframe_set(struct hdmi_spd_infoframe *info);

/* audio raw data get from reg */
int hdmi_audio_infoframe_get(u8 *body);
/* audio infoframe set */
void hdmi_audio_infoframe_set(struct hdmi_audio_infoframe *info);
/* audio infoframe raw set, for backup */
void hdmi_audio_infoframe_rawset(u8 *hb, u8 *pb);

/* drm raw data get from reg */
int hdmi_drm_infoframe_get(u8 *body);
/* drm infoframe set, both deprecated */
void hdmi_drm_infoframe_set(struct hdmi_drm_infoframe *info);
void hdmi_drm_infoframe_rawset(u8 *hb, u8 *pb);

/*
 * When sending CUVA EMP, there are timing requirements:
 * Step 1: hdmitx_cuva_dhdr_init
 * When setting the mode, write all zeros to the CUVA EMP hardware buffer
 *
 * Step 2: hdmitx_cuva_dhdr_reset
 * Ensure that data from each frame in the software is updated in the
 * hardware buffer and prevent dirty data
 *
 * Step 3: hdmitx_dhdr_send
 * Send the data from the hardware buffer out each frame
 */
void hdmitx_cuva_dhdr_init(struct hdmitx_common *tx_comm);
void hdmitx_cuva_dhdr_reset(struct hdmitx_common *tx_comm);
void hdmitx_dhdr_send(u8 *body, int size);
/* dhdr test api */
void hdmitx21_write_dhdr_sram(void);
void hdmitx21_read_dhdr_sram(void);

void hdmi_emp_infoframe_set(enum emp_type type, struct emp_packet_st *info);
void hdmi_emp_frame_set_member(struct emp_packet_st *info,
	enum emp_component_conf conf, u32 val);
int hdmi_emp_infoframe_get(enum emp_type type, u8 *body);

/* sbtm test api */
void hdmi_sbtm_infoframe_rawset(u8 *hb, u8 *pb);

#endif
