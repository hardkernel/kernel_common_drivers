/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMITX_TYPES_H
#define __HDMITX_TYPES_H

#include <linux/types.h>
#include <linux/hdmi.h>

/* interface for external module: audio/cec/hdmirx/dv... */

/* for notify to cec/hdmirx */
#define HDMITX_PLUG			1
#define HDMITX_UNPLUG			2
#define HDMITX_PHY_ADDR_VALID		3
#define HDMITX_KSVLIST	4

enum hdcp_ver_e {
	HDCPVER_NONE = 0,
	HDCPVER_14,
	HDCPVER_22,
};

#define MAX_KSV_LISTS 127
struct hdcprp14_topo {
	unsigned char max_cascade_exceeded:1;
	unsigned char depth:3;
	unsigned char rsvd : 4;
	unsigned char max_devs_exceeded:1;
	unsigned char device_count:7; /* 1 ~ 127 */
	unsigned char ksv_list[MAX_KSV_LISTS * 5];
} __packed;

struct hdcprp22_topo {
	unsigned int depth;
	unsigned int device_count;
	unsigned int v1_X_device_down;
	unsigned int v2_0_repeater_down;
	unsigned int max_devs_exceeded;
	unsigned int max_cascade_exceeded;
	unsigned char id_num;
	unsigned char id_lists[MAX_KSV_LISTS * 5];
};

struct hdcprp_topo {
	/* hdcp_ver currently used */
	enum hdcp_ver_e hdcp_ver;
	union {
		struct hdcprp14_topo topo14;
		struct hdcprp22_topo topo22;
	} topo;
};

/* global used by hdmitx21/20, and extern module */
/* -----------------Source Physical Address--------------- */
struct vsdb_phyaddr {
	unsigned char a:4;
	unsigned char b:4;
	unsigned char c:4;
	unsigned char d:4;
	unsigned char valid;
};

/*
 * Sampling Freq Fs:
 * 0 - Refer to Stream Header;
 * 1 - 32KHz;
 * 2 - 44.1KHz;
 * 3 - 48KHz;
 * 4 - 88.2KHz...
 */
enum hdmi_audio_fs {
	FS_REFER_TO_STREAM = 0,
	FS_32K = 1,
	FS_44K1 = 2,
	FS_48K = 3,
	FS_88K2 = 4,
	FS_96K = 5,
	FS_176K4 = 6,
	FS_192K = 7,
	FS_768K = 8,
	FS_MAX,
};

/* HDMI Audio Parameters */
/* Refer to CEA-861-D Page 88 */
#define DTS_HD_TYPE_MASK 0xff00
#define DTS_HD_MA  (0X1 << 8)
enum hdmi_audio_type {
	CT_REFER_TO_STREAM = 0,
	CT_PCM,
	CT_AC_3, /* DD */
	CT_MPEG1,
	CT_MP3,
	CT_MPEG2,
	CT_AAC,
	CT_DTS,
	CT_ATRAC,
	CT_ONE_BIT_AUDIO,
	CT_DD_P, /* DD+ */
	CT_DTS_HD,
	CT_MAT, /* TrueHD */
	CT_DST,
	CT_WMA,
	CT_CXT = 0xf, /* Audio Coding Extension Type */
	CT_DTS_HD_MA = CT_DTS_HD + (DTS_HD_MA),
	CT_MAX,
	CT_PREPARE, /* prepare for audio mode switching */
};

#define CT_DOLBY_D CT_DD_P

enum hdmi_audio_chnnum {
	CC_REFER_TO_STREAM = 0,
	CC_2CH,
	CC_3CH,
	CC_4CH,
	CC_5CH,
	CC_6CH,
	CC_7CH,
	CC_8CH,
	CC_MAX_CH
};

enum hdmi_audio_format {
	AF_SPDIF = 0, AF_I2S, AF_DSD, AF_HBR, AT_MAX
};

enum hdmi_audio_sampsize {
	SS_REFER_TO_STREAM = 0, SS_16BITS, SS_20BITS, SS_24BITS, SS_MAX
};

enum hdmi_audio_source_if {
	AUD_SRC_IF_SPDIF = 0,
	AUD_SRC_IF_I2S,
	AUD_SRC_IF_TDM, /* for T7 only */
};

/* should sync with sound/soc */
struct aud_para {
	bool prepare; /* when prepare is true, mute tx audio sample */

	/* below parameters will be compared with the previous setting
	 * if different, then call audio HW setting
	 */
	enum hdmi_audio_type type;
	enum hdmi_audio_fs rate;
	enum hdmi_audio_sampsize size;
	enum hdmi_audio_chnnum chs;
	u8 i2s_ch_mask;
	enum hdmi_audio_source_if aud_src_if; /* 0: spdif 1: i2s */

	unsigned char status[24]; /* AES/IEC958 channel status bits */
	/* aud_output_i2s_ch: bit[3:0] ch_msk  bit[7:4] ch_num
	 * configure for I2S: 8ch in, 2ch out
	 * 0: default setting  1:ch0/1  2:ch2/3  3:ch4/5  4:ch6/7
	 */
	u8 aud_output_i2s_ch;
	bool fifo_rst;
	bool aud_output_en; /* 0, off; 1, on */
	bool aud_notify_update;
};

enum hdmi_hdr_transfer {
	T_UNKNOWN = 0,
	T_BT709,
	T_UNDEF,
	T_BT601,
	T_BT470M,
	T_BT470BG,
	T_SMPTE170M,
	T_SMPTE240M,
	T_LINEAR,
	T_LOG100,
	T_LOG316,
	T_IEC61966_2_4,
	T_BT1361E,
	T_IEC61966_2_1,
	T_BT2020_10,
	T_BT2020_12,
	T_SMPTE_ST_2084,
	T_SMPTE_ST_28,
	T_HLG,
};

enum hdmi_hdr_color {
	C_UNKNOWN = 0,
	C_BT709,
	C_UNDEF,
	C_BT601,
	C_BT470M,
	C_BT470BG,
	C_SMPTE170M,
	C_SMPTE240M,
	C_FILM,
	C_BT2020,
};

enum hdmi_tf_type {
	HDMI_NONE = 0,
	/* HDMI_HDR_TYPE, HDMI_DV_TYPE, and HDMI_HDR10P_TYPE
	 * should be mutexed with each other
	 */
	HDMI_HDR_TYPE = 0x10,
	HDMI_HDR_SMPTE_2084	= HDMI_HDR_TYPE | 1,
	HDMI_HDR_HLG		= HDMI_HDR_TYPE | 2,
	HDMI_HDR_HDR		= HDMI_HDR_TYPE | 3,
	HDMI_HDR_SDR		= HDMI_HDR_TYPE | 4,
	HDMI_DV_TYPE = 0x20,
	HDMI_DV_VSIF_STD	= HDMI_DV_TYPE | 1,
	HDMI_DV_VSIF_LL		= HDMI_DV_TYPE | 2,
	HDMI_HDR10P_TYPE = 0x30,
	HDMI_HDR10P_DV_VSIF	= HDMI_HDR10P_TYPE | 1,
};

enum hdmi_hdr_status {
	HDR10PLUS_VSIF = 0,
	dolbyvision_std = 1,
	dolbyvision_lowlatency = 2,
	HDR10_GAMMA_ST2084 = 3,
	HDR10_others,
	HDR10_GAMMA_HLG,
	SDR,
};

enum hdmi_3d_type {
	T3D_FRAME_PACKING = 0,
	T3D_FIELD_ALTER = 1,
	T3D_LINE_ALTER = 2,
	T3D_SBS_FULL = 3,
	T3D_L_DEPTH = 4,
	T3D_L_DEPTH_GRAPHICS = 5,
	T3D_TAB = 6, /* Top and Buttom */
	T3D_RSVD = 7,
	T3D_SBS_HALF = 8,
	T3D_DISABLE,
};

enum hdmi_aspect_ratio {
	ASPECT_RATIO_SAME_AS_SOURCE = 0x8,
	TV_ASPECT_RATIO_4_3 = 0x9,
	TV_ASPECT_RATIO_16_9 = 0xA,
	TV_ASPECT_RATIO_14_9 = 0xB,
	TV_ASPECT_RATIO_MAX
};

#define HDMI_INFOFRAME_TYPE_VENDOR2 (0x81 | 0x100)

enum frl_rate_enum {
	FRL_NONE = 0,
	FRL_3G3L = 1,
	FRL_6G3L = 2,
	FRL_6G4L = 3,
	FRL_8G4L = 4,
	FRL_10G4L = 5,
	FRL_12G4L = 6,
	FRL_RATE_MAX = 7,
};

enum hdmi_phy_para {
	HDMI_PHYPARA_6G = 1, /* 2160p60hz 444 8bit */
	HDMI_PHYPARA_4p5G, /* 2160p50hz 420 12bit */
	HDMI_PHYPARA_3p7G, /* 2160p30hz 444 10bit */
	HDMI_PHYPARA_3G, /* 2160p24hz 444 8bit */
	HDMI_PHYPARA_LT3G, /* 1080p60hz 444 12bit */
	HDMI_PHYPARA_DEF = HDMI_PHYPARA_LT3G,
	HDMI_PHYPARA_270M, /* 480p60hz 444 8bit */
};

enum vrr_type {
	T_VRR_NONE,
	T_VRR_GAME,
	T_VRR_QMS,
};

enum emp_type {
	EMP_TYPE_NONE,
	EMP_TYPE_VRR_GAME = T_VRR_GAME,
	EMP_TYPE_VRR_QMS = T_VRR_QMS,
	EMP_TYPE_SBTM,
	EMP_TYPE_DSC,
	EMP_TYPE_DHDR,
};

/* refer to HDMI2.1A P447 */
enum TARGET_FRAME_RATE {
	TFR_QMSVRR_INACTIVE = 0,
	TFR_23P97,
	TFR_24,
	TFR_25,
	TFR_29P97,
	TFR_30,
	TFR_47P95,
	TFR_48,
	TFR_50,
	TFR_59P94,
	TFR_60,
	TFR_100,
	TFR_119P88,
	TFR_120,
	TFR_MAX,
};

struct size_map {
	unsigned int sample_bits;
	enum hdmi_audio_sampsize ss;
};

enum hd_ctrl {
	VID_EN, VID_DIS, AUD_EN, AUD_DIS, EDID_EN, EDID_DIS, HDCP_EN, HDCP_DIS,
};

enum hdmitx_aspect_ratio {
	AR_UNKNOWN = 0,
	AR_4X3,
	AR_16X9,
};

#define HDMI_PACKET_TYPE_GCP 0x3

struct hdmitx_infoframe {
	u32 enable;
	union hdmi_infoframe vend;
	union hdmi_infoframe avi;
	union hdmi_infoframe spd;
	union hdmi_infoframe aud;
	union hdmi_infoframe drm;
	union hdmi_infoframe emp;
};

enum hdmi_pixel_repeat {
	NO_REPEAT = 0,
	HDMI_2_TIMES_REPEAT,
	HDMI_3_TIMES_REPEAT,
	HDMI_4_TIMES_REPEAT,
	HDMI_5_TIMES_REPEAT,
	HDMI_6_TIMES_REPEAT,
	HDMI_7_TIMES_REPEAT,
	HDMI_8_TIMES_REPEAT,
	HDMI_9_TIMES_REPEAT,
	HDMI_10_TIMES_REPEAT,
	MAX_TIMES_REPEAT,
};

enum hdmi_scan {
	SS_NO_DATA = 0,
	/* where some active pixelsand lines at the edges are not displayed. */
	SS_SCAN_OVER,
	/* where all active pixels&lines are displayed,
	 * with or without a border.
	 */
	SS_SCAN_UNDER,
	SS_RSV
};

enum hdmi_barinfo {
	B_INVALID = 0, B_BAR_VERT, /* Vert. Bar Infovalid */
	B_BAR_HORIZ, /* Horiz. Bar Infovalid */
	B_BAR_VERT_HORIZ,
/* Vert.and Horiz. Bar Info valid */
};

enum hdmi_colourimetry {
	CC_NO_DATA = 0, CC_ITU601, CC_ITU709, CC_XVYCC601, CC_XVYCC709,
};

enum hdmi_scaling {
	SC_NO_UINFORM = 0,
	/* Picture has been scaled horizontally */
	SC_SCALE_HORIZ,
	SC_SCALE_VERT, /* Picture has been scaled vertically */
	SC_SCALE_HORIZ_VERT,
/* Picture has been scaled horizontally & SC_SCALE_H_V */
};

#define AUDIO_PARA_MAX_NUM       14
struct hdmi_audio_fs_ncts {
	struct {
		u32 tmds_clk;
		unsigned int n; /* 24 bit */
		unsigned int cts; /* 24 bit */
		unsigned int n_30bit; /* 30 bit */
		unsigned int cts_30bit; /* 30bit */
		u32 n_36bit;
		u32 cts_36bit;
		u32 n_48bit;
		u32 cts_48bit;
	} array[AUDIO_PARA_MAX_NUM];
	u32 def_n;
};

struct rate_map_fs {
	unsigned int rate;
	enum hdmi_audio_fs fs;
};

enum hdmi_event_t {
	HDMI_TX_NONE = 0,
	HDMI_TX_HPD_PLUGIN = 1,
	HDMI_TX_HPD_PLUGOUT = 2,
	HDMI_TX_INTERNAL_INTR = 4,
};

/***********************************************************************
 *                   hdmi debug printk
 **********************************************************************/
#define VID         "video: "
#define AUD         "audio: "
#define CEC         "cec: "
#define EDID        "edid: "
#define HDCP        "hdcp: "
#define SYS         "system: "
#define HPD         "hpd: "
#define HW          "hw: "
#define REG         "reg: "

#endif
