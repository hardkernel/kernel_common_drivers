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

/* VSIF: Vendor Specific InfoFrame
 * It has multiple purposes:
 * 1. HDMI1.4 4K, HDMI_VIC=1/2/3/4, 2160p30/25/24hz, smpte24hz, AVI.VIC=0
 *    In CTA-861-G, matched with AVI.VIC=95/94/93/98
 * 2. 3D application, TB/SS/FP
 * 3. DolbyVision, with Len=0x18
 * 4. HDR10plus
 * 5. HDMI20 3D OSD disparity / 3D dual-view / 3D independent view / ALLM
 * Some functions are exclusive, but some may compound.
 * Consider various state transitions carefully, such as play 3D under HDMI14
 * 4K, exit 3D under 4K, play DV under 4K, enable ALLM under 3D dual-view
 */
enum vsif_type {
	/* Below 4 functions are exclusive */
	VT_HDMI14_4K = 1,
	VT_T3D_VIDEO,
	VT_DOLBYVISION,
	VT_HDR10PLUS,
	/* Maybe compound 3D dualview + ALLM */
	VT_T3D_OSD_DISPARITY = 0x10,
	VT_T3D_DUALVIEW,
	VT_T3D_INDEPENDVEW,
	VT_ALLM,
	/* default: if non-HDMI4K, no any vsif; if HDMI4k, = VT_HDMI14_4K */
	VT_DEFAULT,
	VT_MAX,
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
#define SYS         "system: "
#define HW          "hw: "
#define REG         "reg: "

#endif
