/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _HDMI_TX_MODULE_H
#define _HDMI_TX_MODULE_H

#include <linux/wait.h>
#include <linux/clk.h>
#include <linux/cdev.h>
#include <linux/clk-provider.h>
#include <linux/device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/amlogic/media/vout/vout_notify.h>
#include <linux/amlogic/media/vpu/vpu.h>
#include <linux/amlogic/media/vrr/vrr.h>
#include <drm/amlogic/meson_connector_dev.h>
#include <linux/miscdevice.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_hw_common.h>
#include <linux/amlogic/media/vout/hdmi_tx_repeater.h>
#include "hdmitx_log.h"
#ifdef CONFIG_AMLOGIC_HDMITX
#include "./hw20/hdmitx_hw.h"
#endif
#ifdef CONFIG_AMLOGIC_HDMITX21
#include "./hw21/hdmitx_hw.h"
#include "./hw21/hdmitx_ddc.h"
#endif
#ifdef CONFIG_AMLOGIC_DSC
#include <linux/amlogic/media/vout/dsc.h>
#endif

#define DEVICE_NAME "amhdmitx"
/************************************
 *    hdmitx device structure
 *************************************/
struct hdmitx_dev *get_hdmitx_device(void);

/* for hdmitx 20 */
struct drm_hdmitx_hdcp_cb {
	void (*callback)(void *data, int auth);
	void *data;
};

struct hdmitx_dev {
	struct hdmitx_common tx_comm;
	struct hdmitx_hw_common hw_comm;

#ifdef CONFIG_AMLOGIC_HDMITX
	struct hdmitx20_hw tx20_hw;
	unsigned char hdcp_max_exceed_state;
	unsigned int hdcp_max_exceed_cnt;
	unsigned int max_exceed;
	struct timer_list hdcp_timer;

	int hdcp_hpd_stick;	/* 1 not init & reset at plugout */
	struct delayed_work work_do_hdcp;
	struct drm_hdmitx_hdcp_cb drm_hdcp_cb;

	struct {
		void (*setaudioinfoframe)(unsigned char *AUD_DB,
					  unsigned char *CHAN_STAT_BUF);
	} hwop;

#endif

#ifdef CONFIG_AMLOGIC_HDMITX21
	struct hdmitx21_hw tx21_hw;
	/* dedicated for intr */
	struct workqueue_struct *hdmi_intr_wq;
	/* hdcp */
	struct delayed_work work_start_hdcp;
	struct delayed_work work_drm_start_hdcp;
	bool dw_hdcp22_cap;
	void *am_hdcp;
	struct miscdevice hdcp_comm_device;
	unsigned long up_hdcp_timeout_sec;
	struct delayed_work work_up_hdcp_timeout;
	u32 hdcp_debug_delay;

	atomic_t kref_audio_mute;

	u8 manual_frl_rate; /* for manual setting */
	u8 frl_rate; /* for mode setting */
	u8 dsc_en;

#ifdef CONFIG_AMLOGIC_DSC
	/* pps data and clk info from dsc module */
	struct dsc_offer_tx_data dsc_data;
#endif
	/* 0: RGB444  1: Y444  2: Y422  3: Y420 */
	/* 4: 24bit  5: 30bit  6: 36bit  7: 48bit */
	/* if equals to 1, means current video & audio output are blank */
	enum vrr_type vrr_mode; /* 1: GAME-VRR, 2: QMS-VRR,  0: default no-VRR */

	/* configure for I2S: 8ch in, 2ch out */
	/* 0: default setting  1:ch0/1  2:ch2/3  3:ch4/5  4:ch6/7 */
	u32 edid_mask_qms:1;
	/*
	 * debug only, should be positive value. if it is N, then vysnc_handler
	 * will handle N frames, then it will be 0, and vysnc_handler is pending
	 * value 1 is only for single steps, and -1 will work as normally.
	 */
	int vrr_dbg_vframe;

	int dfm_type; /* for dfm debug */
	/*
	 * for choose VPU_HDMI_if function
	 * 1: yuv2rgb (default)
	 * 2: rgb2yuv
	 */
	int csc_type;
	/* for dsc debug */
	int emp_no;
	int emp_verbose;
#endif
};

#ifdef CONFIG_AMLOGIC_HDMITX
/***********************************************************************
 *    hdmitx protocol level interface
 **********************************************************************/
extern struct aud_para hdmiaud_config_data;
extern struct aud_para hsty_hdmiaud_config_data[8];
extern unsigned int hsty_hdmiaud_config_loc, hsty_hdmiaud_config_num;

int hdmitx_set_display(struct hdmitx_dev *hdmitx_device,
		       enum hdmi_vic videocode);

int hdmi_set_3d(struct hdmitx_dev *hdmitx_device, int type,
		unsigned int param);

int hdmitx20_set_audio(struct hdmitx_dev *hdmitx_device,
		     struct aud_para *audio_param);

int hdmitx20_init_reg_map(struct platform_device *pdev);
/* for hdcp init */
int hdmitx20_hdcp_init(struct hdmitx_dev *hdev);
/* for debug */
void print_hsty_hdmiaud_config_data(void);
/***********************************************************************
 *    hdmitx hardware level interface
 ***********************************************************************/
void hdmitx20_meson_init(struct hdmitx_dev *hdev);

void hdmitx20_ext_set_audio_output(bool enable);
int hdmitx20_ext_get_audio_status(void);
void hdmitx20_audio_mute_op(unsigned int flag);
#endif

#ifdef CONFIG_AMLOGIC_HDMITX21

struct hdr_dynamic_struct {
	u32 type;
	u32 hd_len;/*hdr_dynamic_length*/
	u8 support_flags;
	u8 optional_fields[20];
};

struct cts_conftab {
	u32 fixed_n;
	u32 tmds_clk;
	u32 fixed_cts;
};

struct vic_attrmap {
	enum hdmi_vic VIC;
	u32 tmds_clk;
};

struct hdmi_phy_t {
	unsigned long reg;
	unsigned long val_sleep;
	unsigned long val_save;
};

struct audcts_log {
	u32 val:20;
	u32 stable:1;
};

struct aspect_ratio_list {
	enum hdmi_vic vic;
	int flag;
	char aspect_ratio_num;
	char aspect_ratio_den;
};

/***********************************************************************
 *    hdmitx protocol level interface
 **********************************************************************/
void hdmitx21_dither_config(struct hdmitx_dev *hdev);

int hdmitx21_set_display(struct hdmitx_dev *hdev,
		       enum hdmi_vic videocode);

void hdmi21_vframe_write_reg(u32 value);

int hdmi21_set_3d(struct hdmitx_dev *hdev, int type,
		u32 param);

int hdmitx21_set_audio(struct hdmitx_dev *hdev,
		     struct aud_para *audio_param);

struct hdmi_format_para *hdmitx21_get_vesa_paras(struct vesa_standard_timing *t);
/***********************************************************************
 *    hdmitx hardware level interface
 ***********************************************************************/
void hdmitx21_meson_init(struct hdmitx_dev *hdev);
int hdmitx21_init_reg_map(struct platform_device *pdev);

void hdmitx21_ext_set_audio_output(bool enable);
int hdmitx21_ext_get_audio_status(void);
#endif

#endif
