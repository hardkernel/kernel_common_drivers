/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMITX_PACKET_H
#define __HDMITX_PACKET_H

struct hdmi_packet_t {
	u8 hb[3];
	u8 pb[28];
	/* padding to 32 bytes */
	u8 no_used;
};

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

#define HDMI_INFOFRAME_TYPE_EMP 0x7f

void hdmitx_packet_init(struct hdmitx_common *tx_comm);
void hdmitx_set_cuva_hdr_vs_emds(struct cuva_hdr_vs_emds_para *data);
void hdmitx_set_sbtm_pkt(struct vtem_sbtm_st *data);
void hdmi_emp_infoframe_set(enum emp_type type, struct emp_packet_st *info);
void hdmi_emp_frame_set_member(struct emp_packet_st *info,
	enum emp_component_conf conf, u32 val);
int hdmi_emp_infoframe_get(enum emp_type type, u8 *body);
void hdmi_sbtm_infoframe_rawset(u8 *hb, u8 *pb);

#endif
