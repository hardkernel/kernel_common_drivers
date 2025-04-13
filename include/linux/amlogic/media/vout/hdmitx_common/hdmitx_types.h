/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMITX_TYPES_H
#define __HDMITX_TYPES_H

#include <linux/types.h>
#include <linux/hdmi.h>

enum vrr_type {
	T_VRR_NONE,
	T_VRR_GAME,
	T_VRR_QMS,
	T_VRR_MAX,
};

struct vrr_setting_info {
	enum vrr_type type; /* if type is T_VRR_NONE, means exit VRR mode */
	bool gpu_drive_vsync; /* reserved, not support now */
};

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
	/*
	 * HDMI_HDR_TYPE, HDMI_DV_TYPE, and HDMI_HDR10P_TYPE
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
	HDMI_CUVA_TYPE = 0x40,
};

enum hdmi_hdr_status {
	HDR10PLUS_VSIF = 0,
	DOLBYVISION_STD = 1,
	DOLBYVISION_LOWLATENCY = 2,
	HDR10_GAMMA_ST2084 = 3,
	HDR10_OTHERS,
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

struct hdmitx_infoframe {
	u32 enable;
	union hdmi_infoframe vend;
	union hdmi_infoframe avi;
	union hdmi_infoframe spd;
	union hdmi_infoframe aud;
	union hdmi_infoframe drm;
	union hdmi_infoframe emp;
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
