/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) WITH Linux-syscall-note */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _MESON_HDMI_DIAG_H_
#define _MESON_HDMI_DIAG_H_

#include <drm/drm.h>
#include <drm/drm_fourcc.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>

struct hdmi_basic_info {
	/* 1: HIGH 0: LOW */
	__u8 hpd;
	/* 1: HIGH 0: LOW */
	__u8 rxsense;
	char resolution[20];
	char colorspace[10];
	__u8 color_depth;
	char colorimetry[40];
	char audio_type[40];
	/* 1: HDMI Mode  0: DVI Mode */
	__u8 hdmi_mode;
	char hdr_status[30];
	char vrr[10];
};

struct hdmi_video_info {
	__u32 active_h_resolution;
	__u32 active_v_resolution;
	__u32 total_h_resolution;
	__u32 total_v_resolution;
	__u32 pixel_clk;
	__u32 vrr_frequency;
	__u32 qms_vrr;
	__u32 m_const;
	__u32 next_tfr;
	__u32 base_refresh_rate;
};

struct hdmi_audio_info {
	__u32 n;
	__u32 cts;
	__u32 layout_value;
	__u32 channel_status;
};

struct hdmi_phy_info {
	__u8 status;
	__u32 tmds_clk;
};

struct hdmi_hdcp14_info {
	/* 1: enable  0: disable */
	__u8 enc_en;
	/* 1: successful  0: fail */
	__u8 status;
	__u8 an[8];
	__u8 aksv[5];
	__u8 bksv[5];
	__u16 ri;
	__u8 b_caps;
	__u16 b_status;
};

struct hdmi_hdcp22_info {
	/* 1: enable  0: disable */
	__u8 enc_en;
	/* 1: successful  0: fail */
	__u8 status;
};

struct hdmi_scdc_info {
	__u8 scrambling_enable;
	__u32 tmds_bit_clock_ratio;
	char scrambling_status[10];
	__u32 clock_detecded;
	__u32 ch0_lock;
	__u32 ch1_lock;
	__u32 ch2_lock;
	__u32 lane3_lock;
	__u32 flt_ready;
	__u32 rs_correction_counter;
	__u32 dsc_fail;
	__u32 error_count;
	__u32 ch0;
	__u32 ch1;
	__u32 ch2;
	__u32 lane3;
	__u32 ltp_req;
	__u32 ln0;
	__u32 ln1;
	__u32 frl_rate;
	__u32 ffe_Level;
	__u32 frl_start;
	__u32 flt_update;
};

struct hdmi_avi_infoframe_info {
	char hdmi_colorspace[10];
	char scan_info[20];
	char colorimetry[30];
	char picture_aspect[15];
	char active_aspect[20];
	__u8 itc;
	char extended_colorimetry[40];
	char quantization_range[10];
	char scling[40];
	__u8 vic;
	char ycc_quantization_range[10];
	char content_type[10];
	__u8 pixel_repeat;
	__u8 top_bar;
	__u8 bottom_bar;
	__u8 left_bar;
	__u8 right_bar;
};

struct hdmi_vsif_infoframe_info {
	__u32 h14b_vsif_ieeeoui;
	__u8 hdmi_vic;
	__u32 hf_vsif_ieeeoui;
	__u8 allm;
	__u32 hdr10plus_ieeeoui;
	__u32 cuva_ieeeoui;
	__u32 amdv_ieeeoui;
	__u8 amdv_signal;
	__u8 amdv_low_latency;
};

struct hdmi_spd_infoframe_info {
	char product_description[20];
	char vendor_name[20];
	char source_info[20];
};

struct hdmi_drm_infoframe_info {
	__u8 eotf;
};

struct hdmi_audio_infoframe_info {
	__u8 channels;
	char coding_type[100];
	char sample_size[20];
	char sample_frequency[20];
	char coding_type_ext[40];
	__u8 channel_allocation;
	__u8 level_shift_value;
	__u8 downmix_inhibit;
};

struct hdmi_gcp_packet_info {
	__u8 avmute;
	__u8 color_depth;
};

struct hdmi_reserved_info {
	__u32 reserved0;
	__u32 reserved1;
	__u32 reserved2;
	__u32 reserved3;
	__u32 reserved4;
	__u32 reserved5;
	__u32 reserved6;
	__u32 reserved7;
	__u32 reserved8;
	__u32 reserved9;
	char reserved10[20];
	char reserved11[20];
	char reserved12[20];
	char reserved13[20];
	char reserved14[20];
	char reserved15[20];
	char reserved16[20];
	char reserved17[20];
	char reserved18[20];
	char reserved19[20];
};

struct hdmi_diagnosis_info {
	__u32 conn_id;
	/* hw info */
	struct hdmi_basic_info basic_info;
	struct hdmi_video_info video_info;
	struct hdmi_audio_info audio_info;
	struct hdmi_phy_info phy_info;
	struct hdmi_hdcp14_info hdcp14_info;
	struct hdmi_hdcp22_info hdcp22_info;
	struct hdmi_scdc_info scdc_info;
	struct hdmi_avi_infoframe_info avi_infoframe_info;
	struct hdmi_vsif_infoframe_info vsif_infoframe_info;
	struct hdmi_spd_infoframe_info spd_infoframe_info;
	struct hdmi_drm_infoframe_info drm_infoframe_info;
	struct hdmi_audio_infoframe_info audio_infoframe_info;
	struct hdmi_gcp_packet_info gcp_packet_info;
	struct hdmi_reserved_info reserved_info;
};

#ifdef CONFIG_AMLOGIC_HDMITX21
/* hdmitx diagnosis infoframe related */
#define DRM_IOCTL_MESON_GET_HDMITX_DIAG DRM_IOWR(DRM_COMMAND_BASE + \
		0x14, struct hdmi_diagnosis_info)
#endif

#endif /* _MESON_HDMI_DIAG_H_ */
