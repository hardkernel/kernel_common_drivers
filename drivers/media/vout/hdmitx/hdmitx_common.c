// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/spinlock.h>
#include <linux/rtc.h>
#include <linux/timekeeping.h>
#include <linux/vmalloc.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include <linux/amlogic/media/vout/vout_notify.h>
#include "hdmitx_boot_parameters.h"
#include "hdmitx_log.h"
#include "hdmitx_check_valid.h"
#include "hdmitx_module.h"
#include "efuse.h"
#include "hdmitx_hdr.h"

#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
#include <linux/amlogic/media/amvecm/amvecm.h>
#endif

int hdmitx_format_para_init(struct hdmi_format_para *para,
		enum hdmi_vic vic, u32 frac_rate_policy,
		enum hdmi_colorspace cs, enum hdmi_color_depth cd,
		enum hdmi_quantization_range cr);
const struct hdmi_timing *hdmitx_mode_match_timing_name(const char *name);

static struct vout_device_s hdmitx_vdev = {
	.fresh_tx_hdr_pkt = hdmitx_set_drm_pkt,
	.fresh_tx_sbtm_pkt = hdmitx_set_sbtm_pkt,
	.fresh_tx_vsif_pkt = hdmitx_set_vsif_pkt,
	.fresh_tx_hdr10plus_pkt = hdmitx_set_hdr10plus_pkt,
	.fresh_tx_cuva_hdr_vsif = hdmitx_set_cuva_hdr_vsif,
	.fresh_tx_cuva_hdr_vs_emds = hdmitx_set_cuva_hdr_vs_emds,
};

void hdmitx_get_init_state(struct hdmitx_common *tx_common,
			   struct hdmitx_common_state *state)
{
	struct hdmi_format_para *para = &tx_common->fmt_para;

	memcpy(&state->para, para, sizeof(*para));
	state->hdr_priority = tx_common->hdr_priority;
}
EXPORT_SYMBOL(hdmitx_get_init_state);

/* init hdmitx_common struct which is done only when driver probe */
int hdmitx_common_init(struct hdmitx_common *tx_comm, struct hdmitx_hw_common *hw_comm)
{
	struct hdmitx_boot_param *boot_param = get_hdmitx_boot_params();

	tx_comm->vdev = &hdmitx_vdev;
	tx_comm->vdev->tx_instance = tx_comm;

	/*load tx boot params*/
	tx_comm->hdr_priority = boot_param->hdr_mask;
	/* rxcap.hdmichecksum save the edid checksum of kernel */
	/* memcpy(tx_comm->rxcap.hdmichecksum, boot_param->edid_chksum, */
		/* sizeof(tx_comm->rxcap.hdmichecksum)); */
	memcpy(tx_comm->fmt_attr, boot_param->color_attr, sizeof(tx_comm->fmt_attr));

	tx_comm->frac_rate_policy = boot_param->fraction_refreshrate;
	tx_comm->config_csc_en = boot_param->config_csc;
	tx_comm->res_1080p = 0;
	tx_comm->max_refreshrate = 60;
	tx_comm->rxcap.edid_check = boot_param->edid_check;
	/* by default, edid parse in plug interrupt is handled in drm */
	tx_comm->edid_parse_dbg = false;

	tx_comm->tx_hw = hw_comm;
	if (tx_comm->tx_hw)
		tx_comm->tx_hw->hdmi_tx_cap.dsc_policy = boot_param->dsc_policy;
	hw_comm->hdcp_repeater_en = 0;

	tx_comm->debug_param.avmute_frame = 0;

	hdmitx_format_para_reset(&tx_comm->fmt_para);

	tx_comm->ready = 0;
	tx_comm->hdcp_user = 1;
	tx_comm->hdr_mute_frame = 20;
	/* no RxSense by default */
	tx_comm->rxsense_policy = 0;
	/* enable or disable HDMITX SSPLL, enable by default */
	tx_comm->sspll = 1;
	tx_comm->flag_3dfp = 0;
	tx_comm->flag_3dss = 0;
	tx_comm->flag_3dtb = 0;
	tx_comm->vid_mute_op = VIDEO_NONE_OP;
	tx_comm->vid_mute_op = VIDEO_NONE_OP;
	tx_comm->hdcp_mode = 0;
	/* default audio configure is on */
	tx_comm->cur_audio_param.aud_output_en = 1;

	/*mutex init*/
	mutex_init(&tx_comm->hdmimode_mutex);
	mutex_init(&tx_comm->valid_mutex);

	spin_lock_init(&tx_comm->edid_spinlock);

	hdmitx_vout_init(tx_comm, hw_comm);
	hdmitx_hdr_init(tx_comm);
	/* get efuse ctrl state */
	get_hdmi_efuse(tx_comm);

	return 0;
}

int hdmitx_common_attch_platform_data(struct hdmitx_common *tx_comm,
	enum HDMITX_PLATFORM_API_TYPE type, void *plt_data)
{
	switch (type) {
	case HDMITX_PLATFORM_TRACER:
		tx_comm->tx_tracer = (struct hdmitx_tracer *)plt_data;
		break;
	case HDMITX_PLATFORM_UEVENT:
		tx_comm->event_mgr = (struct hdmitx_event_mgr *)plt_data;
		break;
	default:
		HDMITX_ERROR("%s unknown platform api %d\n", __func__, type);
		break;
	};

	return 0;
}

int hdmitx_common_destroy(struct hdmitx_common *tx_comm)
{
	if (tx_comm->tx_tracer)
		hdmitx_tracer_destroy(tx_comm->tx_tracer);
	if (tx_comm->event_mgr)
		hdmitx_event_mgr_destroy(tx_comm->event_mgr);
	return 0;
}

/* build format para of current mode + cs/cd + frac */
int hdmitx_common_build_format_para(struct hdmitx_common *tx_comm,
		struct hdmi_format_para *para, enum hdmi_vic vic, u32 frac_rate_policy,
		enum hdmi_colorspace cs, enum hdmi_color_depth cd, enum hdmi_quantization_range cr)
{
	int ret = 0;

	ret = hdmitx_format_para_init(para, vic, frac_rate_policy, cs, cd, cr);
	if (ret == 0)
		ret = hdmitx_hw_calc_format_para(tx_comm->tx_hw, para);
	if (ret < 0)
		hdmitx_format_para_print(para, NULL);

	return ret;
}
EXPORT_SYMBOL(hdmitx_common_build_format_para);

/* similar as valid_mode_store(), but with additional lock */
/*
 * validation step:
 * step1, check if mode related VIC is supported in EDID
 * step2, check if VIC is supported by SOC hdmitx
 * step3, build format with mode/attr and check if it's
 * supported by EDID/hdmitx_cap
 */
int hdmitx_common_validate_mode_locked(struct hdmitx_common *tx_comm,
				       struct hdmitx_common_state *new_state,
				       char *mode, char *attr, bool brr_valid)
{
	int ret = 0;
	struct hdmi_format_para *new_para;
	struct hdmi_format_para tst_para;
	enum hdmi_vic vic = HDMI_0_UNKNOWN;

	new_para = &new_state->para;

	if (new_state->state_sequence_id != tx_comm->tx_hw->hw_sequence_id) {
		HDMITX_ERROR("%s: state_sequence_id failed: %lld\n",
						__func__, new_state->state_sequence_id);
		return -1;
	}

	mutex_lock(&tx_comm->hdmimode_mutex);
	mutex_lock(&tx_comm->valid_mutex);

	if (!mode || !attr) {
		ret = -EINVAL;
		goto out;
	}

	vic = hdmitx_common_parse_vic_in_edid(tx_comm, mode);
	if (vic == HDMI_0_UNKNOWN) {
		HDMITX_DEBUG("%s: get vic from (%s) fail\n", __func__, mode);
		ret = -EINVAL;
		goto out;
	}

	ret = hdmitx_common_validate_vic(tx_comm, vic);
	if (ret != 0) {
		HDMITX_DEBUG("validate vic [%s,%s]-%d return error %d\n", mode, attr, vic, ret);
		goto out;
	}

	hdmitx_parse_color_attr(attr, &tst_para.cs, &tst_para.cd, &tst_para.cr);
	ret = hdmitx_common_build_format_para(tx_comm,
		new_para, vic, brr_valid ? 0 : tx_comm->frac_rate_policy,
		tst_para.cs, tst_para.cd, tst_para.cr);
	if (ret != 0) {
		hdmitx_format_para_reset(new_para);
		HDMITX_DEBUG("build formatpara [%s,%s] return error %d\n", mode, attr, ret);
		goto out;
	}

	ret = hdmitx_common_validate_format_para(tx_comm, new_para);
	if (ret)
		HDMITX_DEBUG("validate formatpara [%s,%s] return error %d\n",
			     mode, attr, ret);
out:
	mutex_unlock(&tx_comm->valid_mutex);
	mutex_unlock(&tx_comm->hdmimode_mutex);
	return ret;
}
EXPORT_SYMBOL(hdmitx_common_validate_mode_locked);

/* init format para only when probe */
int hdmitx_common_init_bootup_format_para(struct hdmitx_common *tx_comm,
		struct hdmi_format_para *para)
{
	int ret = 0;
	struct hdmitx_hw_common *tx_hw = tx_comm->tx_hw;
	enum hdmi_tf_type amdv_type;
	bool dsc_en = false;

	if (hdmitx_hw_get_state(tx_hw, STAT_TX_OUTPUT, 0)) {
		/* TODO: it has not consider VESA mode witch HW VIC = 0 */
		para->vic = hdmitx_hw_get_state(tx_hw, STAT_VIDEO_VIC, 0);
		para->cs = hdmitx_hw_get_state(tx_hw, STAT_VIDEO_CS, 0);
		para->cd = hdmitx_hw_get_state(tx_hw, STAT_VIDEO_CD, 0);
		/*
		 * when STD DV has already output in uboot, the real cs is rgb
		 * but hdmi CSC actually uses the Y444. So here needs to reassign
		 * the para->cs as YUV444
		 */
		if (para->cs == HDMI_COLORSPACE_RGB) {
			amdv_type = hdmitx_hw_get_state(tx_hw, STAT_TX_DV, 0);
			if (amdv_type == HDMI_DV_VSIF_STD)
				para->cs = HDMI_COLORSPACE_YUV444;
		}

		/*
		 * when already output in uboot, use uboot fmt_attr to update cs cd
		 * if dsc is enabled, as cd from HW register is 8bit under dsc
		 */
		if (hdmitx_hw_get_state(tx_hw, STAT_TX_DSC_EN, 0)) {
			hdmitx_parse_color_attr(tx_comm->fmt_attr, &para->cs, &para->cd, &para->cr);
			dsc_en = true;
		}

		ret = hdmitx_common_build_format_para(tx_comm, para, para->vic,
			tx_comm->frac_rate_policy, para->cs, para->cd,
			HDMI_QUANTIZATION_RANGE_FULL);
		if (ret == 0) {
			HDMITX_DEBUG("%s init ok\n", __func__);
			/* for bootup, override build format with HW state */
			para->dsc_en = dsc_en;
			para->frl_rate = hdmitx_hw_cntl_misc(tx_hw, MISC_GET_FRL_MODE, 0);
			hdmitx_format_para_print(para, NULL);
		} else {
			HDMITX_INFO("%s: init uboot format para fail (%d,%d,%d)\n",
				__func__, para->vic, para->cs, para->cd);
		}

		return ret;
	}

	HDMITX_INFO("%s hdmi is not enabled\n", __func__);
	return hdmitx_format_para_reset(para);
}

int hdmitx_fire_drm_hpd_cb_unlocked(struct hdmitx_common *tx_comm)
{
	if (tx_comm->drm_hpd_cb.callback)
		tx_comm->drm_hpd_cb.callback(tx_comm->drm_hpd_cb.data);

	return 0;
}

int hdmitx_register_hpd_cb(struct hdmitx_common *tx_comm, struct connector_hpd_cb *hpd_cb)
{
	tx_comm->drm_hpd_cb.callback = hpd_cb->callback;
	tx_comm->drm_hpd_cb.data = hpd_cb->data;
	return 0;
}
EXPORT_SYMBOL(hdmitx_register_hpd_cb);

/* get hdp plugin sequence id */
u64 hdmitx_get_hpd_hw_sequence_id(struct hdmitx_common *tx_comm)
{
	u64 tmp_sequence_id;

	mutex_lock(&tx_comm->hdmimode_mutex);
	tmp_sequence_id = tx_comm->tx_hw->hw_sequence_id;
	mutex_unlock(&tx_comm->hdmimode_mutex);

	return tmp_sequence_id;
}
EXPORT_SYMBOL(hdmitx_get_hpd_hw_sequence_id);

/* TODO: no mutex */
int hdmitx_get_hpd_state(struct hdmitx_common *tx_comm)
{
	return tx_comm->hpd_state;
}
EXPORT_SYMBOL(hdmitx_get_hpd_state);

/* TODO: no mutex */
unsigned char *hdmitx_get_raw_edid(struct hdmitx_common *tx_comm)
{
	return tx_comm->EDID_buf;
}
EXPORT_SYMBOL(hdmitx_get_raw_edid);

/* TODO: no mutex */
bool hdmitx_common_get_ready_state(struct hdmitx_common *tx_comm)
{
	return tx_comm->ready;
}
EXPORT_SYMBOL(hdmitx_common_get_ready_state);

bool hdmitx_common_get_edid_valid_state(struct hdmitx_common *tx_comm)
{
	return tx_comm->rxcap.edid_parsing;
}
EXPORT_SYMBOL(hdmitx_common_get_edid_valid_state);

bool hdmitx_common_get_hdcp_user_state(struct hdmitx_common *tx_comm)
{
	return tx_comm->hdcp_user;
}
EXPORT_SYMBOL(hdmitx_common_get_hdcp_user_state);

bool hdmitx_common_get_hdmi_used_state(struct hdmitx_common *tx_comm)
{
	return tx_comm->already_used;
}
EXPORT_SYMBOL(hdmitx_common_get_hdmi_used_state);

int hdmitx_get_attr(struct hdmitx_common *tx_comm, char attr[16])
{
	memcpy(attr, tx_comm->fmt_attr, sizeof(tx_comm->fmt_attr));
	return 0;
}
EXPORT_SYMBOL(hdmitx_get_attr);

/*
 * hdr_priority definition:
 *   strategy1: bit[3:0]
 *       0: original cap
 *       1: disable DV cap
 *       2: disable DV and hdr cap
 *   strategy2:
 *       bit4: 1: disable dv  0:enable dv
 *       bit5: 1: disable hdr10/hdr10+  0: enable hdr10/hdr10+
 *       bit6: 1: disable hlg  0: enable hlg
 *   bit28-bit31 choose strategy: bit[31:28]
 *       0: strategy1
 *       1: strategy2
 */

/* dv_info */
static void enable_dv_info(struct dv_info *des, const struct dv_info *src)
{
	if (!des || !src)
		return;

	memcpy(des, src, sizeof(*des));
}

static void disable_dv_info(struct dv_info *des)
{
	if (!des)
		return;

	memset(des, 0, sizeof(*des));
}

/* hdr10 */
static void enable_hdr10_info(struct hdr_info *des, const struct hdr_info *src)
{
	if (!des || !src)
		return;

	des->hdr_support |= (src->hdr_support) & BIT(2);
	des->static_metadata_type1 = src->static_metadata_type1;
	des->lumi_max = src->lumi_max;
	des->lumi_avg = src->lumi_avg;
	des->lumi_min = src->lumi_min;
	des->lumi_peak = src->lumi_peak;
	des->ldim_support = src->ldim_support;
}

static void disable_hdr10_info(struct hdr_info *des)
{
	if (!des)
		return;

	des->hdr_support &= ~BIT(2);
	des->static_metadata_type1 = 0;
	des->lumi_max = 0;
	des->lumi_avg = 0;
	des->lumi_min = 0;
	des->lumi_peak = 0;
	des->ldim_support = 0;
}

/* hdr10plus */
static void disable_hdr10p_info(struct hdr10_plus_info *des)
{
	if (!des)
		return;

	memset(des, 0, sizeof(*des));
}

static void enable_hdr10p_info(struct hdr10_plus_info *des, const struct hdr10_plus_info *src)
{
	if (!des || !src)
		return;

	memcpy(des, src, sizeof(*des));

#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
	/* HDR10plus is only supported by OTT when is_hdr10plus_enable is true */
	if (!is_hdr10plus_enable())
		disable_hdr10p_info(des);
#endif
}

/* hlg */
static void enable_hlg_info(struct hdr_info *des, const struct hdr_info *src)
{
	if (!des || !src)
		return;

	des->hdr_support |= (src->hdr_support) & BIT(3);
}

static void disable_hlg_info(struct hdr_info *des)
{
	if (!des)
		return;

	des->hdr_support &= ~BIT(3);
}

static void enable_all_hdr_info(struct rx_cap *prxcap, struct hdr_info *hdr_info,
		struct dv_info *dv_info)
{
#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
	struct hdr10_plus_info *hdr10p_info = NULL;
#endif
	if (!prxcap || !hdr_info || !dv_info)
		return;

	memcpy(hdr_info, &prxcap->hdr_info, sizeof(prxcap->hdr_info));
	memcpy(dv_info, &prxcap->dv_info, sizeof(prxcap->dv_info));

#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
	hdr10p_info = &hdr_info->hdr10plus_info;
	/* HDR10plus is only supported by OTT when is_hdr10plus_enable is true */
	if (!is_hdr10plus_enable())
		disable_hdr10p_info(hdr10p_info);
#endif
}

static void update_hdr_strategy1(struct hdmitx_common *tx_comm, struct hdr_info *hdr_info,
		struct dv_info *dv_info, u32 strategy)
{
	struct rx_cap *prxcap;

	if (!tx_comm || !hdr_info || !dv_info)
		return;

	prxcap = &tx_comm->rxcap;
	/* init hdr_info and dv_info */
	enable_all_hdr_info(prxcap, hdr_info, dv_info);

	switch (strategy) {
	case 0:
		enable_all_hdr_info(prxcap, hdr_info, dv_info);
		break;
	case 1:
		disable_dv_info(dv_info);
		break;
	case 2:
		disable_dv_info(dv_info);
		disable_hdr10_info(hdr_info);
		disable_hdr10p_info(&hdr_info->hdr10plus_info);
		disable_hlg_info(hdr_info);
		break;
	default:
		break;
	}
}

static void update_hdr_strategy2(struct hdmitx_common *tx_comm, struct hdr_info *hdr_info,
		struct dv_info *dv_info, u32 strategy)
{
	struct rx_cap *prxcap;

	if (!tx_comm || !hdr_info || !dv_info)
		return;

	prxcap = &tx_comm->rxcap;
	/* init hdr_info and dv_info */
	enable_all_hdr_info(prxcap, hdr_info, dv_info);

	/* bit4: 1 disable dv  0 enable dv */
	if (strategy & BIT(4))
		disable_dv_info(dv_info);
	else
		enable_dv_info(dv_info, &prxcap->dv_info);
	/* bit5: 1 disable hdr10/hdr10+   0 enable hdr10/hdr10+ */
	if (strategy & BIT(5)) {
		disable_hdr10_info(hdr_info);
		disable_hdr10p_info(&hdr_info->hdr10plus_info);
	} else {
		enable_hdr10_info(hdr_info, &prxcap->hdr_info);
		enable_hdr10p_info(&hdr_info->hdr10plus_info,
			&prxcap->hdr_info.hdr10plus_info);
	}
	/* bit6: 1 disable hlg   0 enable hlg */
	if (strategy & BIT(6))
		disable_hlg_info(hdr_info);
	else
		enable_hlg_info(hdr_info, &prxcap->hdr_info);
}

int hdmitx_set_hdr_priority(struct hdmitx_common *tx_comm, u32 hdr_priority,
		struct hdr_info *hdr_info, struct dv_info *dv_info)
{
	u32 choose = 0;
	u32 strategy = 0;

	if (!hdr_info || !dv_info || !tx_comm) {
		HDMITX_ERROR("%s fail\n", __func__);
		return 0;
	}

	tx_comm->hdr_priority = hdr_priority;
	HDMITX_DEBUG("%s, set hdr_prio: %u\n", __func__, hdr_priority);
	/* choose strategy: bit[31:28] */
	choose = (tx_comm->hdr_priority >> 28) & 0xf;
	switch (choose) {
	case 0:
		strategy = tx_comm->hdr_priority & 0xf;
		update_hdr_strategy1(tx_comm, hdr_info, dv_info, strategy);
		break;
	case 1:
		strategy = tx_comm->hdr_priority & 0xf0;
		update_hdr_strategy2(tx_comm, hdr_info, dv_info, strategy);
		break;
	default:
		break;
	}
	return 0;
}
EXPORT_SYMBOL(hdmitx_set_hdr_priority);

int hdmitx_get_hdr_priority(struct hdmitx_common *tx_comm, u32 *hdr_priority)
{
	*hdr_priority = tx_comm->hdr_priority;
	return 0;
}
EXPORT_SYMBOL(hdmitx_get_hdr_priority);

int hdmitx_get_hdrinfo_rx(struct hdmitx_common *tx_comm, struct hdr_info *hdrinfo)
{
	struct rx_cap *prxcap = &tx_comm->rxcap;

	memcpy(hdrinfo, &prxcap->hdr_info, sizeof(struct hdr_info));
	hdrinfo->colorimetry_support = prxcap->colorimetry_data;

	return 0;
}

enum hdmi_vic hdmitx_get_prefer_vic(struct hdmitx_common *tx_comm, enum hdmi_vic vic)
{
	int i = 0;
	const struct {
		u32 mode_prefer_vic;
		u32 mode_alternate_vic;
	} vic_pairs[] = {
		{HDMI_7_720x480i60_16x9, HDMI_6_720x480i60_4x3},
		{HDMI_3_720x480p60_16x9, HDMI_2_720x480p60_4x3},
		{HDMI_22_720x576i50_16x9, HDMI_21_720x576i50_4x3},
		{HDMI_18_720x576p50_16x9, HDMI_17_720x576p50_4x3},
	};

	for (i = 0; i < ARRAY_SIZE(vic_pairs); i++) {
		if (vic_pairs[i].mode_alternate_vic == vic || vic_pairs[i].mode_prefer_vic == vic) {
			/* check if mode_best_vic supported by RX */
			if (hdmitx_edid_validate_mode(&tx_comm->rxcap,
						vic_pairs[i].mode_prefer_vic))
				return vic_pairs[i].mode_prefer_vic;
			if (hdmitx_edid_validate_mode(&tx_comm->rxcap,
						vic_pairs[i].mode_alternate_vic))
				return vic_pairs[i].mode_alternate_vic;
			return HDMI_0_UNKNOWN;
		}
	}

	return vic;
}

/* get corresponding vic of mode, and will check if it's supported in EDID */
int hdmitx_common_parse_vic_in_edid(struct hdmitx_common *tx_comm, const char *mode)
{
	const struct hdmi_timing *timing;
	enum hdmi_vic prefer_vic = HDMI_0_UNKNOWN;

	if (!tx_comm || !mode)
		return HDMI_0_UNKNOWN;
	/* parse by name to find default mode */
	timing = hdmitx_mode_match_timing_name(mode);
	if (!timing || timing->vic == HDMI_0_UNKNOWN)
		return HDMI_0_UNKNOWN;

	prefer_vic = hdmitx_get_prefer_vic(tx_comm, timing->vic);

	/* check if vic supported by rx, except:480i 480p 576i 576p */
	if (prefer_vic != HDMI_0_UNKNOWN &&
		hdmitx_edid_validate_mode(&tx_comm->rxcap, prefer_vic) == false)
		prefer_vic = HDMI_0_UNKNOWN;

	/*
	 * Dont call hdmitx_common_check_valid_para_of_vic anymore.
	 * This function used to parse user passed mode name which should already
	 * checked by hdmitx_common_check_valid_para_of_vic().
	 */

	if (prefer_vic == HDMI_0_UNKNOWN)
		HDMITX_DEBUG("parse mode %s not find vic %d in edid\n", mode, prefer_vic);

	return prefer_vic;
}
EXPORT_SYMBOL(hdmitx_common_parse_vic_in_edid);

int hdmitx_common_notify_hpd_status(struct hdmitx_common *tx_comm, bool force_uevent)
{
	if (!tx_comm->suspend_flag) {
		/* notify to userspace by uevent */
		hdmitx_event_mgr_send_uevent(tx_comm->event_mgr,
				HDMITX_HPD_EVENT, tx_comm->hpd_state, force_uevent);
		hdmitx_event_mgr_send_uevent(tx_comm->event_mgr,
				HDMITX_AUDIO_EVENT, tx_comm->hpd_state, force_uevent);
	} else {
		/*
		 * under early suspend, only update uevent state, not
		 * post to system, in case 1.old android system will
		 * set hdmi mode, 2.audio server and audio_hal will
		 * start run, increase power consumption
		 */
		hdmitx_event_mgr_set_uevent_state(tx_comm->event_mgr,
				HDMITX_HPD_EVENT, tx_comm->hpd_state);
		hdmitx_event_mgr_set_uevent_state(tx_comm->event_mgr,
				HDMITX_AUDIO_EVENT, tx_comm->hpd_state);
	}

	/*
	 * always notify to other driver module: CEC/RX
	 * CEC/RX side will decide to update HPD/EDID or
	 * not by product type
	 */
	/* if (tx_comm->hdmi_repeater == 1) { */
	if (tx_comm->hpd_state)
		hdmitx_event_mgr_notify(tx_comm->event_mgr, HDMITX_PLUG, tx_comm->EDID_buf);
	else
		hdmitx_event_mgr_notify(tx_comm->event_mgr, HDMITX_UNPLUG, NULL);

	return 0;
}
EXPORT_SYMBOL(hdmitx_common_notify_hpd_status);

int hdmitx_set_uevent(struct hdmitx_common *tx_comm, enum hdmitx_event type, int val)
{
	return hdmitx_event_mgr_send_uevent(tx_comm->event_mgr,
				type, val, false);
}

bool hdmitx_hdr_en(struct hdmitx_hw_common *tx_hw)
{
	if (!tx_hw)
		return false;

	return (hdmitx_hw_get_state(tx_hw, STAT_TX_HDR, 0) & HDMI_HDR_TYPE) == HDMI_HDR_TYPE;
}

bool hdmitx_dv_en(struct hdmitx_hw_common *tx_hw)
{
	if (!tx_hw)
		return false;

	return (hdmitx_hw_get_state(tx_hw, STAT_TX_DV, 0) & HDMI_DV_TYPE) == HDMI_DV_TYPE;
}

bool hdmitx_hdr10p_en(struct hdmitx_hw_common *tx_hw)
{
	if (!tx_hw)
		return false;

	return (hdmitx_hw_get_state(tx_hw, STAT_TX_HDR10P, 0) & HDMI_HDR10P_TYPE) ==
		HDMI_HDR10P_TYPE;
}

/* index is hdmi 1.4 vic in vsif, value is hdmi2.0 vic */
static const u32 hdmi14_4k_vics[] = {
	/* 0 - dummy */
	0,
	/* 1 - 3840x2160@30Hz */
	95,
	/* 2 - 3840x2160@25Hz */
	94,
	/* 3 - 3840x2160@24Hz */
	93,
	/* 4 - 4096x2160@24Hz (SMPTE) */
	98,
};

/*
 * HDMI 1.4 introduces the 4 types of 4K resolutions
 * 3840x2160@30/25/24Hz and 4096x2160@24Hz
 * but no corresponding CEA-861-D VIC.
 * so it uses VSIF.hdmi_vic as 1/2/3/4 to represent 4 types
 * In CEA-861-F, here assigns the VIC 95/94/93/98 later
 * this function converts 861-F 4k vic to VSIF.hdmi_vic
 */
u32 hdmitx_edid_get_hdmi14_4k_vic(u32 vic)
{
	u32 ret = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(hdmi14_4k_vics); i++) {
		if (vic == hdmi14_4k_vics[i]) {
			ret = i;
			break;
		}
	}

	return ret;
}

/*
 * CE Video Format: Any Video Format listed in Table 1, or listed in
 * Table 11 and Table 12, except the 640x480p Video Format
 *
 * From CTA-861-I Page 26
 */
static bool is_ce_video_format(enum hdmi_vic vic)
{
	bool ret = false;

	if ((vic > HDMI_1_640x480p60_4x3 && vic <= HDMI_127_5120x2160p100_64x27) ||
		(vic >= HDMI_193_5120x2160p120_64x27 && vic <= HDMI_219_4096x2160p120_256x135))
		ret = true;

	/* TODO: RID is also CE video format, need to add in the future */
	return ret;
}

enum hdmi_scan_mode hdmitx_check_scan_info(struct rx_cap *prxcap,
		enum hdmi_scan_mode val, enum hdmi_vic vic)
{
	enum hdmi_scan_mode scan_mode = val;
	u8 vcdb = 0;

	if (!prxcap)
		return scan_mode;

	vcdb = prxcap->video_capability_data;
	/*
	 * If the display does not provide a VCDB then the Source should assume that
	 * CE Video Formats are overscanned by the display and that IT Video Format
	 * behavior is indicated by CEA Extension byte 3 bit 7 (underscan).
	 * If underscan=1 then the Source should assume that IT Video Formats are
	 * underscanned and if underscan=0, that IT Video Formats are overscanned
	 */
	if (!vcdb && !is_ce_video_format(vic)) {
		if (prxcap->underscan)
			scan_mode = HDMI_SCAN_MODE_UNDERSCAN;
		else
			scan_mode = HDMI_SCAN_MODE_OVERSCAN;
	}

	if (vcdb) {
		/*
		 * If the display declares a non-zero value for the S_PT (preferred timing
		 * overscan/underscan behavior) field, and the Source outputs that Video
		 * Format, then the S_PT declaration shall take precedence over both S_CE
		 * and S_IT declarations
		 */
		if (((vcdb & 0x30) != PT_VIDEO_FORMAT_NO_DATA) &&
				prxcap->flag_vfpdb && vic == prxcap->preferred_mode) {
			if ((vcdb & 0x30) == PT_VIDEO_FORMAT_ALWAYS_OVERSCAN)
				scan_mode = HDMI_SCAN_MODE_OVERSCAN;
			if ((vcdb & 0x30) == PT_VIDEO_FORMAT_ALWAYS_UNDERSCANSCAN)
				scan_mode = HDMI_SCAN_MODE_UNDERSCAN;
		} else {
			/*
			 * If the S_PT field is 0 then the overscan/underscan behavior of this
			 * format is indicated by either the S_CE or S_IT fields, depending on
			 * whether the Preferred Video Format is a CE or IT Video Format
			 */
			if (is_ce_video_format(vic)) {
				if ((vcdb & 0x3) == CE_VIDEO_FORMAT_NOT_SUPPORT)
					scan_mode = HDMI_SCAN_MODE_NONE;
				else if ((vcdb & 0x3) == CE_VIDEO_FORMAT_ALWAYS_OVERSCAN)
					scan_mode = HDMI_SCAN_MODE_OVERSCAN;
				else if ((vcdb & 0x3) == CE_VIDEO_FORMAT_ALWAYS_UNDERSCANSCAN)
					scan_mode = HDMI_SCAN_MODE_UNDERSCAN;
			} else {
				if ((vcdb & 0xC) == IT_VIDEO_FORMAT_NOT_SUPPORT)
					scan_mode = HDMI_SCAN_MODE_NONE;
				else if ((vcdb & 0xC) == IT_VIDEO_FORMAT_ALWAYS_OVERSCAN)
					scan_mode = HDMI_SCAN_MODE_OVERSCAN;
				else if ((vcdb & 0xC) == IT_VIDEO_FORMAT_ALWAYS_UNDERSCANSCAN)
					scan_mode = HDMI_SCAN_MODE_UNDERSCAN;
			}
		}
	}
	return scan_mode;
}

/* ALLM HF_VSIF/HDMI14 VSIF */
int hdmitx_common_setup_vsif_packet(struct hdmitx_common *tx_comm,
	enum vsif_type type, int on, void *param)
{
	u8 len = 0;
	/* transmission usage, excluding checksum */
	u8 buffer[31] = {0x81, 0x1, 0};
	u32 ieeeoui = 0;
	u32 vic = 0;
	struct hdmitx_hw_common *tx_hw = NULL;

	if (!tx_comm) {
		HDMITX_ERROR("hdr: [%s]: null tx_instance param\n", __func__);
		return -EINVAL;
	}
	tx_hw = tx_comm->tx_hw;
	if (!tx_hw) {
		HDMITX_ERROR("hdr: [%s]: null tx_hw param\n", __func__);
		return -EINVAL;
	}

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

int hdmitx_common_set_allm_mode(struct hdmitx_common *tx_comm, int mode)
{
	struct hdmitx_hw_common *tx_hw_base = tx_comm->tx_hw;

	/* disable ALLM HF-VSIF, recover HDMI1.4 4k VSIF if it's legacy 4k24/25/30hz */
	if (mode == 0) {
		tx_comm->allm_mode = 0;
		hdmitx_common_setup_vsif_packet(tx_comm, VT_ALLM, 0, NULL);
		if (hdmitx_edid_get_hdmi14_4k_vic(tx_comm->fmt_para.vic) > 0 &&
			!hdmitx_dv_en(tx_hw_base) &&
			!hdmitx_hdr10p_en(tx_hw_base))
			hdmitx_common_setup_vsif_packet(tx_comm, VT_HDMI14_4K, 1, NULL);
	}
	/* enable ALLM HF-VSIF, will config vic in AVI if it's legacy 4k24/25/30hz */
	if (mode == 1) {
		tx_comm->allm_mode = 1;
		hdmitx_common_setup_vsif_packet(tx_comm, VT_ALLM, 1, NULL);
		tx_comm->ct_mode = 0;
		hdmitx_hw_cntl_config(tx_hw_base, CONF_CT_MODE, SET_CT_OFF);
	}
	/* disable ALLM HF-VSIF, not recover HDMI1.4 legacy 4k VSIF */
	if (mode == -1) {
		if (tx_comm->allm_mode == 1) {
			tx_comm->allm_mode = 0;
			/* just for hdmitx20, TODO: remove later */
			hdmitx_hw_disable_packet(tx_hw_base, HDMI_PACKET_VEND);
			/* common for hdmitx20/21 to disable HF-VSIF */
			hdmitx_common_setup_vsif_packet(tx_comm, VT_ALLM, 0, NULL);
		}
	}
	return 0;
}
EXPORT_SYMBOL(hdmitx_common_set_allm_mode);

static unsigned int get_frame_duration(struct vinfo_s *vinfo)
{
	unsigned int frame_duration;

	if (!vinfo || !vinfo->sync_duration_num)
		return 0;

	frame_duration =
		1000000 * vinfo->sync_duration_den / vinfo->sync_duration_num;
	return frame_duration;
}

int hdmitx_common_avmute_locked(struct hdmitx_common *tx_comm,
	int mute_flag, int mute_path_hint)
{
	static DEFINE_MUTEX(avmute_mutex);
	static unsigned int global_avmute_mask;
	unsigned int mute_us =
		tx_comm->debug_param.avmute_frame * get_frame_duration(&tx_comm->hdmitx_vinfo);

	mutex_lock(&avmute_mutex);
	if (mute_flag == SET_AVMUTE) {
		global_avmute_mask |= mute_path_hint;
		HDMITX_DEBUG("%s: AVMUTE path=0x%x\n", __func__, mute_path_hint);
		hdmitx_hw_cntl_misc(tx_comm->tx_hw, MISC_AVMUTE_OP, SET_AVMUTE);
	} else if (mute_flag == CLR_AVMUTE) {
		global_avmute_mask &= ~mute_path_hint;
		/* unmute only if none of the paths are muted */
		if (global_avmute_mask == 0) {
			HDMITX_DEBUG("%s: AV UNMUTE path=0x%x\n", __func__, mute_path_hint);
			hdmitx_hw_cntl_misc(tx_comm->tx_hw, MISC_AVMUTE_OP, CLR_AVMUTE);
		}
	} else if (mute_flag == OFF_AVMUTE) {
		hdmitx_hw_cntl_misc(tx_comm->tx_hw, MISC_AVMUTE_OP, OFF_AVMUTE);
	}
	if (mute_flag == SET_AVMUTE) {
		if (tx_comm->debug_param.avmute_frame > 0)
			msleep(mute_us / 1000);
		else if (mute_path_hint == AVMUTE_PATH_HDMITX)
			msleep(100);
	}

	mutex_unlock(&avmute_mutex);

	return 0;
}
EXPORT_SYMBOL(hdmitx_common_avmute_locked);

/********************************Debug function***********************************/

static int hdmitx_common_edid_tracer_post_proc(struct hdmitx_common *tx_comm, struct rx_cap *prxcap)
{
	struct dv_info *dv;
	struct hdr_info *hdr;

	if (!prxcap)
		return -1;

	if (prxcap->head_err)
		hdmitx_tracer_write_event(tx_comm->tx_tracer, HDMITX_EDID_HEAD_ERROR);
	if (prxcap->chksum_err)
		hdmitx_tracer_write_event(tx_comm->tx_tracer, HDMITX_EDID_CHECKSUM_ERROR);

	dv = &prxcap->dv_info;
	if (dv->ieeeoui == DV_IEEE_OUI && dv->block_flag == CORRECT)
		hdmitx_tracer_write_event(tx_comm->tx_tracer, HDMITX_EDID_DV_SUPPORT);

	hdr = &prxcap->hdr_info;
	if (hdr->hdr_support > 0)
		hdmitx_tracer_write_event(tx_comm->tx_tracer, HDMITX_EDID_HDR_SUPPORT);

	if (prxcap->ieeeoui == HDMI_IEEE_OUI)
		hdmitx_tracer_write_event(tx_comm->tx_tracer, HDMITX_EDID_HDMI_DEVICE);
	else
		hdmitx_tracer_write_event(tx_comm->tx_tracer, HDMITX_EDID_DVI_DEVICE);

	return 0;
}

int hdmitx_common_get_edid(struct hdmitx_common *tx_comm)
{
	struct hdmitx_hw_common *tx_hw_base = tx_comm->tx_hw;

	if (tx_comm->forced_edid) {
		HDMITX_INFO("using fixed edid\n");
		return 0;
	}
	hdmitx_edid_buffer_clear(tx_comm->EDID_buf, sizeof(tx_comm->EDID_buf));

	/* reset i2c before edid read */
	hdmitx_hw_cntl_misc(tx_hw_base, MISC_I2C_RESET, 0);
	hdmitx_hw_cntl_ddc(tx_hw_base, DDC_RESET_EDID, 0);
	hdmitx_hw_cntl_ddc(tx_hw_base, DDC_PIN_MUX_OP, PIN_MUX);
	/* start reading edid first time */
	hdmitx_hw_cntl_ddc(tx_hw_base, DDC_EDID_READ_DATA, 0);
	if (hdmitx_edid_is_all_zeros(tx_comm->EDID_buf)) {
		HDMITX_INFO("First read edid all 0 data\n");
		hdmitx_hw_cntl_ddc(tx_hw_base, DDC_GLITCH_FILTER_RESET, 0);
		hdmitx_hw_cntl_ddc(tx_hw_base, DDC_EDID_READ_DATA, 0);
	}
	/* If EDID is not correct at first time, then retry */
	if (hdmitx_edid_check_data_valid(tx_comm->rxcap.edid_check, tx_comm->EDID_buf) == false) {
		struct timespec64 kts;
		struct rtc_time tm;

		hdmitx_hw_cntl_ddc(tx_hw_base, DDC_I2C_RATE, DDC_I2C_38K);
		HDMITX_INFO("config i2c 37.5k and read edid again\n");
		msleep(20);
		ktime_get_real_ts64(&kts);
		rtc_time64_to_tm(kts.tv_sec, &tm);
		if (tx_hw_base->hdmitx_gpios_scl >= 0)
			HDMITX_INFO("UTC+0 %ptRd %ptRt DDC SCL %s\n", &tm, &tm,
			gpio_get_value(tx_hw_base->hdmitx_gpios_scl) ? "HIGH" : "LOW");
		if (tx_hw_base->hdmitx_gpios_sda >= 0)
			HDMITX_INFO("UTC+0 %ptRd %ptRt DDC SDA %s\n", &tm, &tm,
			gpio_get_value(tx_hw_base->hdmitx_gpios_sda) ? "HIGH" : "LOW");
		msleep(80);

		/* start reading edid second time */
		hdmitx_hw_cntl_ddc(tx_hw_base, DDC_EDID_READ_DATA, 0);
		if (hdmitx_edid_is_all_zeros(tx_comm->EDID_buf)) {
			hdmitx_hw_cntl_ddc(tx_hw_base, DDC_GLITCH_FILTER_RESET, 0);
			hdmitx_hw_cntl_ddc(tx_hw_base, DDC_EDID_READ_DATA, 0);
		}
		hdmitx_hw_cntl_ddc(tx_hw_base, DDC_I2C_RATE, DDC_I2C_75K);
		HDMITX_INFO("Recovery i2c 75k\n");
		msleep_interruptible(20);
	}

	/*
	 * notify phy addr to rx/cec:
	 * rx/cec currently do not use the phy addr of below
	 * two interfaces, just keep for safety
	 */
	/* if (tx_comm->hdmi_repeater == 1) { */
	/* hdmitx_event_mgr_notify(tx_comm->event_mgr, */
	/* HDMITX_PHY_ADDR_VALID, &tx_comm->rxcap.physical_addr); */
	/* rx_edid_physical_addr(tx_comm->rxcap.vsdb_phy_addr.a, */
	/* tx_comm->rxcap.vsdb_phy_addr.b, */
	/* tx_comm->rxcap.vsdb_phy_addr.c, */
	/* tx_comm->rxcap.vsdb_phy_addr.d); */
	/* } */
	hdmitx_edid_print(tx_comm->EDID_buf);

	return 0;
}

/* only for first time plugout */
bool is_tv_changed(char *cur_edid_chksum, char *boot_param_edid_chksum)
{
	char invalidchecksum[11] = {
		'i', 'n', 'v', 'a', 'l', 'i', 'd', 'c', 'r', 'c', '\0'
	};
	char emptychecksum[11] = {0};
	bool ret = false;

	if (!cur_edid_chksum || !boot_param_edid_chksum)
		return ret;

	if (memcmp(boot_param_edid_chksum, cur_edid_chksum, 10) &&
		memcmp(emptychecksum, cur_edid_chksum, 10) &&
		memcmp(invalidchecksum, boot_param_edid_chksum, 10)) {
		ret = true;
		HDMITX_INFO("hdmi crc is diff between uboot and kernel\n");
	}

	return ret;
}

/* common work for plugout/suspend, which is done in lock */
void hdmitx_common_edid_clear(struct hdmitx_common *tx_comm)
{
	/* clear edid */
	hdmitx_edid_buffer_clear(tx_comm->EDID_buf, sizeof(tx_comm->EDID_buf));
	hdmitx_edid_rxcap_clear(&tx_comm->rxcap);
	/* if (tx_comm->hdmi_repeater == 1) */
	/* rx_edid_physical_addr(0, 0, 0, 0); */
}

void hdmitx_current_status(struct hdmitx_common *tx_comm, enum hdmitx_event_log_bits event)
{
	hdmitx_tracer_write_event(tx_comm->tx_tracer, event);
}

void hdmitx_hdr_state_init(struct hdmitx_common *tx_comm)
{
	enum hdmi_tf_type hdr_type = HDMI_NONE;
	unsigned int colorimetry = 0;
	unsigned int hdr_mode = 0;
	struct hdmitx_hw_common *tx_hw_base = tx_comm->tx_hw;

	hdr_type = hdmitx_hw_get_hdr_st(tx_hw_base);
	colorimetry = hdmitx_hw_cntl_config(tx_hw_base, CONF_GET_AVI_BT2020, 0);
	/* 1:standard HDR, 2:non-standard, 3:HLG, 0:other */
	if (hdr_type == HDMI_HDR_SMPTE_2084) {
		if (colorimetry == HDMI_EXTENDED_COLORIMETRY_BT2020)
			hdr_mode = 1;
		else
			hdr_mode = 2;
	} else if (hdr_type == HDMI_HDR_HLG) {
		if (colorimetry == HDMI_EXTENDED_COLORIMETRY_BT2020)
			hdr_mode = 3;
	} else {
		hdr_mode = 0;
	}

	tx_comm->hdmi_last_hdr_mode = hdr_mode;
	tx_comm->hdmi_current_hdr_mode = hdr_mode;
}

u32 hdmitx_calc_tmds_clk(u32 pixel_freq,
	enum hdmi_colorspace cs, enum hdmi_color_depth cd)
{
	u32 tmds_clk = pixel_freq;

	if (cs == HDMI_COLORSPACE_YUV420)
		tmds_clk = tmds_clk / 2;
	if (cs != HDMI_COLORSPACE_YUV422) {
		switch (cd) {
		case COLORDEPTH_48B:
			tmds_clk *= 2;
			break;
		case COLORDEPTH_36B:
			tmds_clk = tmds_clk * 3 / 2;
			break;
		case COLORDEPTH_30B:
			tmds_clk = tmds_clk * 5 / 4;
			break;
		case COLORDEPTH_24B:
		default:
			break;
		}
	}

	return tmds_clk;
}

/* get the corresponding bandwidth of current FRL_RATE, Unit: MHz */
u32 hdmitx_get_frl_bandwidth(const enum frl_rate_enum rate)
{
	const u32 frl_bandwidth[] = {
		[FRL_NONE] = 0,
		[FRL_3G3L] = 9000,
		[FRL_6G3L] = 18000,
		[FRL_6G4L] = 24000,
		[FRL_8G4L] = 32000,
		[FRL_10G4L] = 40000,
		[FRL_12G4L] = 48000,
	};

	if (rate > FRL_12G4L)
		return frl_bandwidth[FRL_12G4L];
	return frl_bandwidth[rate];
}

u32 hdmitx_calc_frl_bandwidth(u32 pixel_freq, enum hdmi_colorspace cs,
	enum hdmi_color_depth cd)
{
	u32 bandwidth;

	bandwidth = hdmitx_calc_tmds_clk(pixel_freq, cs, cd);

	/* bandwidth = tmds_bandwidth * 24 * 1.122 */
	bandwidth = bandwidth * 24;
	bandwidth = bandwidth * 561 / 500;

	return bandwidth;
}

/* for legacy HDMI2.0 or earlier modes, still select TMDS */
enum frl_rate_enum hdmitx_select_frl_rate(u8 *dsc_en, u8 dsc_policy, enum hdmi_vic vic,
	enum hdmi_colorspace cs, enum hdmi_color_depth cd)
{
	const struct hdmi_timing *timing;
	enum frl_rate_enum frl_rate = FRL_NONE;
	u32 tx_frl_bandwidth = 0;
	u32 tx_tmds_bandwidth = 0;

	if (!dsc_en)
		return frl_rate;
	HDMITX_DEBUG("dsc_policy %d  vic %d  cs %d  cd %d\n", dsc_policy, vic, cs, cd);
	*dsc_en = 0;
	timing = hdmitx_mode_vic_to_hdmi_timing(vic);
	if (!timing)
		return FRL_NONE;

	tx_tmds_bandwidth = hdmitx_calc_tmds_clk(timing->pixel_freq / 1000, cs, cd);
	HDMITX_DEBUG("Hactive=%d Vactive=%d Vfreq=%d TMDS_BandWidth=%d\n",
		timing->h_active, timing->v_active,
		timing->v_freq, tx_tmds_bandwidth);
	/* If the tmds bandwidth is less than 594MHz, then select the tmds mode */
	/* the HxVp48hz is new introduced in HDMI 2.1 / CEA-861-H */
	if (tx_tmds_bandwidth <= 594 && timing->pixel_freq / 1000 < 600)
		return FRL_NONE;
	/* tx_frl_bandwidth = tmds_bandwidth * 24 * 1.122 */
	tx_frl_bandwidth = tx_tmds_bandwidth * 24;
	tx_frl_bandwidth = tx_frl_bandwidth * 561 / 500;
	for (frl_rate = FRL_3G3L; frl_rate < FRL_12G4L + 1; frl_rate++) {
		if (tx_frl_bandwidth <= hdmitx_get_frl_bandwidth(frl_rate)) {
			HDMITX_DEBUG("select frl_rate as %d\n", frl_rate);
			break;
		}
	}

#ifdef CONFIG_AMLOGIC_DSC
	/* check tx_cap outside */
	//if (!tx_hw->base.hdmi_tx_cap.dsc_capable) {
	//	HDMITX_DEBUG("%s hdmitx not support DSC\n", __func__);
	//	return frl_rate;
	//}
	/* DSC specific, automatically enable dsc if necessary */
	if (vic == HDMI_199_7680x4320p60_16x9 ||
		vic == HDMI_207_7680x4320p60_64x27) {
		if (cs == HDMI_COLORSPACE_YUV444 ||
			cs == HDMI_COLORSPACE_RGB) {
			*dsc_en = 1;
			/* note: previously spec FRL_6G4L can't work */
			frl_rate = FRL_6G4L;
			HDMITX_DEBUG_DSC("%s automatically dsc enable\n", __func__);
		} else if ((cs == HDMI_COLORSPACE_YUV420 &&
			cd == COLORDEPTH_36B) ||
			(cs == HDMI_COLORSPACE_YUV422)) {
			*dsc_en = 1;
			/* note: previously spec FRL_6G3L can't work */
			frl_rate = FRL_6G3L;
			HDMITX_DEBUG_DSC("%s automatically dsc enable\n", __func__);
		} else if (dsc_policy == 1) {
			/* for 420,8/10bit */
			/* force mode for dsc test, may need to also set manual_frl_rate */
			*dsc_en = 1;
			/* note: previously spec FRL_6G3L can't work */
			frl_rate = FRL_6G3L;
		} else {
			*dsc_en = 0;
		}
		if (*dsc_en == 1)
			HDMITX_DEBUG_DSC("forced DSC rate %d\n", frl_rate);
	} else if (vic == HDMI_198_7680x4320p50_16x9 ||
		vic == HDMI_206_7680x4320p50_64x27) {
		if (cs == HDMI_COLORSPACE_YUV444 ||
			cs == HDMI_COLORSPACE_RGB) {
			*dsc_en = 1;
			frl_rate = FRL_6G4L;
			HDMITX_DEBUG_DSC("%s automatically dsc enable\n", __func__);
		} else if ((cs == HDMI_COLORSPACE_YUV420 &&
			cd == COLORDEPTH_36B) ||
			(cs == HDMI_COLORSPACE_YUV422)) {
			*dsc_en = 1;
			frl_rate = FRL_6G3L;
			HDMITX_DEBUG_DSC("%s automatically dsc enable\n", __func__);
		} else if (dsc_policy == 1) {
			/* for 420,8/10bit */
			/* force mode for dsc test, may need to also set manual_frl_rate */
			*dsc_en = 1;
			frl_rate = FRL_6G3L;
		} else {
			*dsc_en = 0;
		}
		if (*dsc_en)
			HDMITX_DEBUG_DSC("spec recommended DSC frl rate: %d\n", frl_rate);
	} else if (vic == HDMI_195_7680x4320p25_16x9 ||
		vic == HDMI_203_7680x4320p25_64x27 ||
		vic == HDMI_194_7680x4320p24_16x9 ||
		vic == HDMI_202_7680x4320p24_64x27) {
		if ((cs == HDMI_COLORSPACE_YUV444 ||
			cs == HDMI_COLORSPACE_RGB) &&
			cd == COLORDEPTH_36B) {
			*dsc_en = 1;
			frl_rate = FRL_6G3L;
			HDMITX_DEBUG_DSC("%s automatically dsc enable\n", __func__);
		} else if (dsc_policy == 1) {
			/* force mode for test, may need to also set manual_frl_rate */
			*dsc_en = 1;
			/* for y444/rgb,8/10bit */
			if (cs == HDMI_COLORSPACE_YUV444 ||
				cs == HDMI_COLORSPACE_RGB)
				frl_rate = FRL_6G3L;
			else
				/* for 422/420 */
				frl_rate = FRL_3G3L;
		} else {
			*dsc_en = 0;
		}
		if (*dsc_en)
			HDMITX_DEBUG_DSC("spec recommended DSC frl rate: %d\n", frl_rate);
	} else if (vic == HDMI_196_7680x4320p30_16x9 ||
		vic == HDMI_204_7680x4320p30_64x27) {
		if ((cs == HDMI_COLORSPACE_YUV444 ||
			cs == HDMI_COLORSPACE_RGB) &&
			cd == COLORDEPTH_36B) {
			*dsc_en = 1;
			frl_rate = FRL_6G3L;
			HDMITX_DEBUG_DSC("%s automatically dsc enable\n", __func__);
		} else if (dsc_policy == 1) {
			/* force mode for test, may need to also set manual_frl_rate */
			*dsc_en = 1;
			/* for 444/rgb,8/10bit */
			if (cs == HDMI_COLORSPACE_YUV444 ||
				cs == HDMI_COLORSPACE_RGB)
				frl_rate = FRL_6G3L;
			else /* for 422/420, note: previously spec FRL_3G3L can't work */
				frl_rate = FRL_3G3L;
		} else {
			*dsc_en = 0;
		}
		if (*dsc_en)
			HDMITX_DEBUG_DSC("forced DSC frl rate: %d\n", frl_rate);
	} else if (vic == HDMI_97_3840x2160p60_16x9 ||
		vic == HDMI_107_3840x2160p60_64x27 ||
		vic == HDMI_96_3840x2160p50_16x9 ||
		vic == HDMI_106_3840x2160p50_64x27) {
		if (dsc_policy == 1) {
			/* force mode for test, may need to also set manual_frl_rate */
			*dsc_en = 1;
			frl_rate = FRL_3G3L;
		} else {
			*dsc_en = 0;
		}
		if (*dsc_en)
			HDMITX_DEBUG_DSC("spec recommended DSC frl rate: %d\n", frl_rate);
	} else if (vic == HDMI_117_3840x2160p100_16x9 ||
		vic == HDMI_119_3840x2160p100_64x27 ||
		vic == HDMI_118_3840x2160p120_16x9 ||
		vic == HDMI_120_3840x2160p120_64x27) {
		/* need 12G4L under uncompressed format */
		if ((cs == HDMI_COLORSPACE_YUV444 ||
			cs == HDMI_COLORSPACE_RGB) &&
			cd == COLORDEPTH_36B) {
			*dsc_en = 1;
			frl_rate = FRL_6G3L;
			HDMITX_DEBUG_DSC("%s automatically dsc enable\n", __func__);
		} else if (dsc_policy == 1) {
			/* force mode for test, may need to also set manual_frl_rate */
			*dsc_en = 1;
			/* for 444/rgb,8/10bit */
			if (cs == HDMI_COLORSPACE_YUV444 ||
				cs == HDMI_COLORSPACE_RGB)
				frl_rate = FRL_6G3L;
			else
				/* for 422/420, note: previously spec FRL_3G3L can't work */
				frl_rate = FRL_3G3L;
		} else {
			*dsc_en = 0;
		}
		if (*dsc_en)
			HDMITX_DEBUG_DSC("spec recommended DSC frl rate: %d\n", frl_rate);
	} else {
		/* when switch mode to lower resolution, need to back to non-dsc mode */
		if (dsc_policy != 2)
			*dsc_en = 0;
	}
#endif

	return frl_rate;
}

unsigned int hdmitx_get_frame_duration(struct hdmitx_common *tx_comm)
{
	unsigned int frame_duration;
	struct vinfo_s *vinfo = hdmitx_get_current_vinfo(tx_comm);

	if (!vinfo || !vinfo->sync_duration_num)
		return 0;

	frame_duration =
		1000000 * vinfo->sync_duration_den / vinfo->sync_duration_num;
	return frame_duration;
}

static char *efuse_name_table[] = {"FEAT_DISABLE_HDMI_60HZ",
				   "FEAT_DISABLE_OUTPUT_4K",
				   "FEAT_DISABLE_HDCP_TX_22",
				   "FEAT_DISABLE_HDMI_TX_3D",
				   "FEAT_DISABLE_HDMITX",
				    NULL};
void get_hdmi_efuse(struct hdmitx_common *tx_comm)
{
	struct efuse_obj_field_t efuse_field;
	u8 buff[32];
	u32 bufflen = sizeof(buff);
	char *efuse_field_name;
	u32 rc = 0;
	int i = 0;

	memset(&buff[0], 0, sizeof(buff));
	memset(&efuse_field, 0, sizeof(efuse_field));

	for (; efuse_name_table[i]; i++) {
		efuse_field_name = efuse_name_table[i];
		rc = efuse_obj_read(EFUSE_OBJ_EFUSE_DATA, efuse_field_name, buff, &bufflen);
		if (rc == EFUSE_OBJ_SUCCESS) {
			strncpy(efuse_field.name, efuse_field_name, sizeof(efuse_field.name) - 1);
			memcpy(efuse_field.data, buff, bufflen);
			efuse_field.size = bufflen;

			if (*efuse_field.data == 1) {
				switch (i) {
				case 0:
					tx_comm->efuse_dis_hdmi_4k60 = 1;
					HDMITX_DEBUG("get efuse FEAT_DISABLE_HDMI_60HZ = 1\n");
					break;
				case 1:
					tx_comm->efuse_dis_output_4k = 1;
					HDMITX_DEBUG("get efuse FEAT_DISABLE_OUTPUT_4K = 1\n");
					break;
				case 2:
					tx_comm->efuse_dis_hdcp_tx22 = 1;
					HDMITX_DEBUG("get efuse FEAT_DISABLE_HDCP_TX_22 = 1\n");
					break;
				case 3:
					tx_comm->efuse_dis_hdmi_tx3d = 1;
					HDMITX_DEBUG("get efuse FEAT_DISABLE_HDMI_TX_3D = 1\n");
					break;
				case 4:
					tx_comm->efuse_dis_hdcp_tx14 = 1;
					HDMITX_DEBUG("get efuse FEAT_DISABLE_HDMI = 1\n");
					break;
				default:
					break;
				}
			}
		} else {
			HDMITX_DEBUG("warn getting %s: %d\n", efuse_field_name, rc);
		}
	}
}

bool hdmitx_edid_only_support_sd(struct rx_cap *prxcap)
{
	enum hdmi_vic vic;
	u32 i, j;
	bool only_support_sd = true;
	/* EDID of SL8800 equipment only support below formats */
	static enum hdmi_vic sd_fmt[] = {
		1, 3, 4, 17, 18
	};

	if (!prxcap)
		return false;

	for (i = 0; i < prxcap->VIC_count; i++) {
		vic = prxcap->VIC[i];
		for (j = 0; j < ARRAY_SIZE(sd_fmt); j++) {
			if (vic == sd_fmt[j])
				break;
		}
		if (j == ARRAY_SIZE(sd_fmt)) {
			only_support_sd = false;
			break;
		}
	}

	return only_support_sd;
}

/*
 * if the EDID is invalid, then set the fallback mode
 * Resolution & RefreshRate:
 *   1920x1080p60hz 16:9
 *   1280x720p60hz 16:9 (default)
 *   720x480p 16:9
 * ColorSpace: RGB
 * ColorDepth: 8bit
 * audio : pcm
 */
static void edid_set_fallback_mode(struct rx_cap *prxcap)
{
	struct vsdb_phyaddr *phyaddr;

	if (!prxcap)
		return;

	HDMITX_INFO("set fallback mode\n");
	/* EDID extended blk chk error, set the 720p60, rgb,8bit */
	phyaddr = &prxcap->vsdb_phy_addr;
	prxcap->ieeeoui = HDMI_IEEE_OUI;

	/* set the default cec physical address as 0xffff */
	phyaddr->a = 0xf;
	phyaddr->b = 0xf;
	phyaddr->c = 0xf;
	phyaddr->d = 0xf;
	phyaddr->valid = 0;

	/* 165MHZ / 5 */
	prxcap->Max_TMDS_Clock1 = DEFAULT_MAX_TMDS_CLK;
	/* only RGB */
	prxcap->native_Mode = 0;
	/* only 8bit */
	prxcap->dc_y444 = 0;
	prxcap->VIC_count = 0x3;
	prxcap->VIC[0] = HDMI_16_1920x1080p60_16x9;
	prxcap->VIC[1] = HDMI_4_1280x720p60_16x9;
	prxcap->VIC[2] = HDMI_3_720x480p60_16x9;
	prxcap->native_vic = HDMI_4_1280x720p60_16x9;
	HDMITX_INFO("set default vic 720p60hz\n");

	prxcap->AUD_count = 1;
	/* PCM */
	prxcap->RxAudioCap[0].audio_format_code = 1;
	/* 2ch */
	prxcap->RxAudioCap[0].channel_num_max = 1;
	/* 32/44.1/48 kHz */
	prxcap->RxAudioCap[0].freq_cc = 7;
	/* 16bit */
	prxcap->RxAudioCap[0].cc3 = 1;
}

static int hdmitx_update_ced_en(struct hdmitx_common *tx_comm)
{
	/* if cedst_en is 1, ced detection will be enabled in hdmitx_common_post_enable_mode */
	if (tx_comm->cedst_policy == 1)
		tx_comm->cedst_en = !!tx_comm->rxcap.scdc_present;

	return 0;
}

void hdmitx_edid_process(struct hdmitx_common *tx_comm, bool boot_flag, bool drm_parse_part)
{
	unsigned long flags = 0;
	bool edid_valid = false;

	if (!tx_comm)
		return;

	spin_lock_irqsave(&tx_comm->edid_spinlock, flags);

	edid_valid = hdmitx_edid_check_data_valid(tx_comm->rxcap.edid_check, tx_comm->EDID_buf);

	if (boot_flag || tx_comm->edid_parse_dbg) {
		hdmitx_edid_rxcap_clear(&tx_comm->rxcap);
		/* if edid is valid, parse edid, otherwise set fallback mode */
		if (edid_valid) {
			hdmitx_edid_parse(&tx_comm->rxcap, tx_comm->EDID_buf);
			hdmitx_update_ced_en(tx_comm);
			/* parse cec phy addr and audio data block */
			hdmitx_cec_phy_addr_parse(&tx_comm->rxcap, tx_comm->EDID_buf);
			hdmitx_audio_parse(&tx_comm->rxcap, tx_comm->EDID_buf);
		} else {
			edid_set_fallback_mode(&tx_comm->rxcap);
		}
		hdmitx_common_edid_tracer_post_proc(tx_comm, &tx_comm->rxcap);
	} else if (!drm_parse_part) {
		hdmitx_edid_rxcap_clear(&tx_comm->rxcap);
		/* step1: hdmitx parse part */
		if (edid_valid) {
			/* parse cec phy addr and audio data block */
			hdmitx_cec_phy_addr_parse(&tx_comm->rxcap, tx_comm->EDID_buf);
			hdmitx_audio_parse(&tx_comm->rxcap, tx_comm->EDID_buf);
		} else {
			edid_set_fallback_mode(&tx_comm->rxcap);
		}
		HDMITX_DEBUG_EDID("parse phy_addr/audio on hdmi side\n");
	} else {
		/* step2: drm parse part */
		if (edid_valid) {
			hdmitx_edid_parse(&tx_comm->rxcap, tx_comm->EDID_buf);
			hdmitx_update_ced_en(tx_comm);
		}
		hdmitx_common_edid_tracer_post_proc(tx_comm, &tx_comm->rxcap);
		HDMITX_DEBUG_EDID("parse most parts on drm side\n");
	}

	spin_unlock_irqrestore(&tx_comm->edid_spinlock, flags);
}
EXPORT_SYMBOL(hdmitx_edid_process);

#ifdef CONFIG_AMLOGIC_DSC
/*
 * get the needed frl rate, refer to 2.1 spec table 7-37/38,
 * actually it may also need to check bpp
 */
enum frl_rate_enum get_dsc_frl_rate(enum dsc_encode_mode dsc_mode)
{
	enum frl_rate_enum frl_rate = FRL_RATE_MAX;

	switch (dsc_mode) {
	case DSC_RGB_3840X2160_60HZ:
	case DSC_YUV444_3840X2160_60HZ:
	case DSC_YUV422_3840X2160_60HZ:
	case DSC_YUV420_3840X2160_60HZ:
	case DSC_RGB_3840X2160_50HZ:
	case DSC_YUV444_3840X2160_50HZ:
	case DSC_YUV422_3840X2160_50HZ:
	case DSC_YUV420_3840X2160_50HZ:
		frl_rate = FRL_3G3L;
		break;
	case DSC_RGB_3840X2160_120HZ:
	case DSC_YUV444_3840X2160_120HZ:
	case DSC_RGB_3840X2160_100HZ:
	case DSC_YUV444_3840X2160_100HZ:
		frl_rate = FRL_6G3L;
		break;
	case DSC_YUV422_3840X2160_120HZ:
	case DSC_YUV420_3840X2160_120HZ:
	case DSC_YUV422_3840X2160_100HZ:
	case DSC_YUV420_3840X2160_100HZ:
		frl_rate = FRL_3G3L;
		break;

	case DSC_RGB_7680X4320_60HZ:
	case DSC_YUV444_7680X4320_60HZ:
		/*
		 * 6G4L is spec recommended, but actually it can't
		 * work on board, need to work under 8G4L
		 */
		frl_rate = FRL_6G4L;
		break;
	case DSC_YUV422_7680X4320_60HZ:
	case DSC_YUV420_7680X4320_60HZ:
		/*
		 * 6G3L is spec recommended, but actually it can't
		 * work on board, need to work under 6G4L
		 */
		frl_rate = FRL_6G3L;
		break;

	case DSC_RGB_7680X4320_50HZ:
	case DSC_YUV444_7680X4320_50HZ:
		frl_rate = FRL_6G4L;
		break;
	case DSC_YUV422_7680X4320_50HZ:
	case DSC_YUV420_7680X4320_50HZ:
		frl_rate = FRL_6G3L;
		break;

	/* bpp = 12 */
	case DSC_YUV444_7680X4320_30HZ:
	case DSC_RGB_7680X4320_30HZ:
		frl_rate = FRL_6G3L;
		break;
	/* bpp = 7.375 */
	case DSC_YUV422_7680X4320_30HZ:
	case DSC_YUV420_7680X4320_30HZ:
		/*
		 * 3G3L is spec recommended, but actually it can't
		 * work on board, need to work under 6G3L
		 */
		frl_rate = FRL_3G3L;
		break;

	/* bpp = 12 */
	case DSC_YUV444_7680X4320_25HZ:
	case DSC_RGB_7680X4320_25HZ:
	case DSC_YUV444_7680X4320_24HZ:
	case DSC_RGB_7680X4320_24HZ:
		frl_rate = FRL_6G3L;
		break;
	/* bpp = 7.6875 */
	case DSC_YUV422_7680X4320_25HZ:
	case DSC_YUV420_7680X4320_25HZ:
	case DSC_YUV422_7680X4320_24HZ:
	case DSC_YUV420_7680X4320_24HZ:
		frl_rate = FRL_3G3L;
		break;
	case DSC_ENCODE_MAX:
	default:
		frl_rate = FRL_RATE_MAX;
		break;
	}
	return frl_rate;
}
#endif

static inline void hdmitx_notify_hpd(int hpd, void *p)
{
	struct hdmitx_common *tx_comm = get_hdmitx_common();

	if (!tx_comm)
		return;

	if (hpd)
		hdmitx_event_mgr_notify(tx_comm->event_mgr,
				HDMITX_PLUG, p);
	else
		hdmitx_event_mgr_notify(tx_comm->event_mgr,
				HDMITX_UNPLUG, NULL);
}

/* for notify to cec */
int hdmitx_event_notifier_regist(struct notifier_block *nb)
{
	int ret = 0;
	struct hdmitx_common *tx_comm = get_hdmitx_common();

	if (!nb || !tx_comm)
		return ret;

	ret = hdmitx_event_mgr_notifier_register(tx_comm->event_mgr,
		(struct hdmitx_notifier_client *)nb);

	/* update status when register */
	if (!ret && nb->notifier_call) {
		/* if (hdev->tx_comm.hdmi_repeater == 1) */
		hdmitx_notify_hpd(tx_comm->hpd_state,
			tx_comm->rxcap.edid_parsing ?
			tx_comm->EDID_buf : NULL);
		/* actually notify phy_addr is not used by CEC/hdmirx */
		/* if (hdev->tx_comm.rxcap.physical_addr != 0xffff) { */
		/* if (hdev->tx_comm.hdmi_repeater == 1) */
		/* hdmitx_event_mgr_notify(hdev->tx_comm.event_mgr, */
		/* HDMITX_PHY_ADDR_VALID, */
		/* &hdev->tx_comm.rxcap.physical_addr); */
		/* } */
	}

	return ret;
}
EXPORT_SYMBOL(hdmitx_event_notifier_regist);

int hdmitx_event_notifier_unregist(struct notifier_block *nb)
{
	struct hdmitx_common *tx_comm = get_hdmitx_common();

	if (!tx_comm)
		return -1;

	return hdmitx_event_mgr_notifier_unregister(tx_comm->event_mgr,
		(struct hdmitx_notifier_client *)nb);
}
EXPORT_SYMBOL(hdmitx_event_notifier_unregist);

int get_hpd_state(void)
{
	int ret = 0;
	struct hdmitx_common *tx_comm = get_hdmitx_common();

	if (!tx_comm)
		return -1;

	mutex_lock(&tx_comm->hdmimode_mutex);
	ret = tx_comm->hpd_state;
	mutex_unlock(&tx_comm->hdmimode_mutex);

	return ret;
}
EXPORT_SYMBOL(get_hpd_state);

struct vsdb_phyaddr *get_hdmitx_phy_addr(void)
{
	struct hdmitx_common *tx_comm = get_hdmitx_common();

	if (!tx_comm)
		return NULL;

	return &tx_comm->rxcap.vsdb_phy_addr;
}
EXPORT_SYMBOL(get_hdmitx_phy_addr);

void setup_attr(const char *buf)
{
	HDMITX_ERROR("Not support tx20 %s anymore.\n", __func__);
}
EXPORT_SYMBOL(setup_attr);

unsigned int hdmitx_common_get_contenttypes(struct hdmitx_common *tx_comm)
{
	unsigned int types = 1 << DRM_MODE_CONTENT_TYPE_NO_DATA;/*NONE DATA*/
	struct rx_cap *prxcap = &tx_comm->rxcap;

	if (prxcap->cnc0)
		types |= 1 << DRM_MODE_CONTENT_TYPE_GRAPHICS;
	if (prxcap->cnc1)
		types |= 1 << DRM_MODE_CONTENT_TYPE_PHOTO;
	if (prxcap->cnc2)
		types |= 1 << DRM_MODE_CONTENT_TYPE_CINEMA;
	if (prxcap->cnc3)
		types |= 1 << DRM_MODE_CONTENT_TYPE_GAME;

	return types;
}
EXPORT_SYMBOL(hdmitx_common_get_contenttypes);

int hdmitx_common_set_contenttype(struct hdmitx_common *tx_comm, int content_type)
{
	int ret = 0;

	/* for content type game function conflict with ALLM, so
	 * reset allm to enable contenttype.
	 * TODO: follow spec to skip contenttype when ALLM is on.
	 */
	/* recover hdmi1.4 vsif if necessary */
	if (hdmitx_edid_get_hdmi14_4k_vic(tx_comm->fmt_para.vic) &&
		!hdmitx_dv_en(tx_comm->tx_hw) &&
		!hdmitx_hdr10p_en(tx_comm->tx_hw))
		hdmitx_common_setup_vsif_packet(tx_comm, VT_HDMI14_4K, 1, NULL);

	/*reset previous ct.*/
	hdmitx_hw_cntl_config(tx_comm->tx_hw, CONF_CT_MODE, SET_CT_OFF);
	tx_comm->ct_mode = 0;

	switch (content_type) {
	case DRM_MODE_CONTENT_TYPE_GRAPHICS:
		content_type = SET_CT_GRAPHICS;
		break;
	case DRM_MODE_CONTENT_TYPE_PHOTO:
		content_type = SET_CT_PHOTO;
		break;
	case DRM_MODE_CONTENT_TYPE_CINEMA:
		content_type = SET_CT_CINEMA;
		break;
	case DRM_MODE_CONTENT_TYPE_GAME:
		content_type = SET_CT_GAME;
		break;
	default:
		HDMITX_ERROR("[%s]: [%d] unsupported type\n",
			__func__, content_type);
		content_type = 0;
		ret = -EINVAL;
		break;
	};

	hdmitx_hw_cntl_config(tx_comm->tx_hw,
		CONF_CT_MODE, content_type);
	tx_comm->ct_mode = content_type;

	return ret;
}
EXPORT_SYMBOL(hdmitx_common_set_contenttype);

const struct dv_info *hdmitx_common_get_dv_info_rx(struct hdmitx_common *tx_comm)
{
	const struct dv_info *dv = &tx_comm->rxcap.dv_info;

	return dv;
}
EXPORT_SYMBOL(hdmitx_common_get_dv_info_rx);

const struct hdr_info *hdmitx_common_get_hdr_info_rx(struct hdmitx_common *tx_comm)
{
	static struct hdr_info hdrinfo;

	hdmitx_get_hdrinfo_rx(tx_comm, &hdrinfo);
	return &hdrinfo;
}
EXPORT_SYMBOL(hdmitx_common_get_hdr_info_rx);

/* similar as disp_cap_show() */
int hdmitx_common_get_vic_list(struct hdmitx_common *tx_comm, int **vics)
{
	struct rx_cap *prxcap = &tx_comm->rxcap;
	enum hdmi_vic vic;
	int len = prxcap->VIC_count + VESA_MAX_TIMING;
	int i;
	int count = 0;
	int *viclist = 0;
	int *edid_vics = 0;
	enum hdmi_vic prefer_vic = HDMI_0_UNKNOWN;

	mutex_lock(&tx_comm->valid_mutex);
	viclist = kcalloc(len, sizeof(int),  GFP_KERNEL);
	edid_vics = vmalloc(len * sizeof(int));
	memset(edid_vics, 0, len * sizeof(int));

	/* step1: only select VIC which is supported in EDID */
	/*copy edid vic list*/
	if (prxcap->VIC_count > 0)
		memcpy(edid_vics, prxcap->VIC, sizeof(int) * prxcap->VIC_count);
	for (i = 0; i < VESA_MAX_TIMING && prxcap->vesa_timing[i]; i++)
		edid_vics[prxcap->VIC_count + i] = prxcap->vesa_timing[i];

	for (i = 0; i < len; i++) {
		vic = edid_vics[i];
		if (vic == HDMI_0_UNKNOWN)
			continue;

		prefer_vic = hdmitx_get_prefer_vic(tx_comm, vic);
		/* if mode_best_vic is support by RX, try 16x9 first */
		if (prefer_vic != vic) {
			HDMITX_DEBUG("%s: check prefer vic:%d exist, ignore [%d] later.\n",
				__func__, prefer_vic, vic);
			continue;
		}

		/* step2, check if VIC is supported by SOC hdmitx */
		if (hdmitx_common_validate_vic(tx_comm, vic) != 0) {
			//HDMITX_ERROR("%s: vic[%d] over range.\n", __func__, vic);
			continue;
		}
		/* step3, build format with basic mode/attr and check
		 * if it's supported by EDID/hdmitx_cap
		 */
		if (hdmitx_common_check_valid_para_of_vic(tx_comm, vic) != 0) {
			//HDMITX_ERROR("%s: vic[%d] check fmt attr failed.\n", __func__, vic);
			continue;
		}

		viclist[count] = vic;
		count++;
	}

	vfree(edid_vics);

	if (count == 0)
		kfree(viclist);
	else
		*vics = viclist;

	mutex_unlock(&tx_comm->valid_mutex);
	return count;
}
EXPORT_SYMBOL(hdmitx_common_get_vic_list);

/* similar as hdmitx_common_validate_mode_locked() but without lock,
 * it's almost the same as valid_mode_store()
 * validation step:
 * step1, check if mode related VIC is supported in EDID
 * step2, check if VIC is supported by SOC hdmitx
 * step3, build format with mode/attr and check if it's
 * supported by EDID/hdmitx_cap
 */
bool hdmitx_common_chk_mode_attr_sup(struct hdmitx_common *tx_comm, char *mode, char *attr)
{
	struct hdmi_format_para tst_para;
	enum hdmi_vic vic = HDMI_0_UNKNOWN;
	int ret = 0;

	if (!mode || !attr)
		return false;

	mutex_lock(&tx_comm->valid_mutex);
	vic = hdmitx_common_parse_vic_in_edid(tx_comm, mode);
	if (vic == HDMI_0_UNKNOWN) {
		HDMITX_ERROR("%s: get vic from (%s) fail\n", __func__, mode);
		mutex_unlock(&tx_comm->valid_mutex);
		return false;
	}

	ret = hdmitx_common_validate_vic(tx_comm, vic);
	if (ret != 0) {
		HDMITX_ERROR("validate vic [%s,%s]-%d return error %d\n", mode, attr, vic, ret);
		mutex_unlock(&tx_comm->valid_mutex);
		return false;
	}

	hdmitx_parse_color_attr(attr, &tst_para.cs, &tst_para.cd, &tst_para.cr);
	ret = hdmitx_common_build_format_para(tx_comm,
		&tst_para, vic, tx_comm->frac_rate_policy,
		tst_para.cs, tst_para.cd, tst_para.cr);
	if (ret != 0) {
		hdmitx_format_para_reset(&tst_para);
		HDMITX_ERROR("build formatpara [%s,%s] return error %d\n", mode, attr, ret);
		mutex_unlock(&tx_comm->valid_mutex);
		return false;
	}

	if (true) {
		HDMITX_DEBUG("sname = %s\n", tst_para.sname);
		HDMITX_DEBUG("char_clk = %d\n", tst_para.tmds_clk);
		HDMITX_DEBUG("cd = %d\n", tst_para.cd);
		HDMITX_DEBUG("cs = %d\n", tst_para.cs);
	}

	ret = hdmitx_common_validate_format_para(tx_comm, &tst_para);
	if (ret != 0) {
		HDMITX_ERROR("validate formatpara [%s,%s] return error %d\n", mode, attr, ret);
		mutex_unlock(&tx_comm->valid_mutex);
		return false;
	}

	mutex_unlock(&tx_comm->valid_mutex);
	return true;
}
EXPORT_SYMBOL(hdmitx_common_chk_mode_attr_sup);

int hdmitx_common_get_hdr_status(struct hdmitx_common *tx_comm)
{
	enum hdmi_tf_type type = HDMI_NONE;

	type = hdmitx_hw_get_state(tx_comm->tx_hw, STAT_TX_HDR10P, 0);
	if (type) {
		if (type == HDMI_HDR10P_DV_VSIF)
			return HDR10PLUS_VSIF;
	}
	type = hdmitx_hw_get_state(tx_comm->tx_hw, STAT_TX_DV, 0);
	if (type) {
		if (type == HDMI_DV_VSIF_STD)
			return dolbyvision_std;
		else if (type == HDMI_DV_VSIF_LL)
			return dolbyvision_lowlatency;
	}
	type = hdmitx_hw_get_state(tx_comm->tx_hw, STAT_TX_HDR, 0);
	if (type) {
		if (type == HDMI_HDR_SMPTE_2084)
			return HDR10_GAMMA_ST2084;
		else if (type == HDMI_HDR_HLG)
			return HDR10_GAMMA_HLG;
		else if (type == HDMI_HDR_HDR)
			return HDR10_others;
	}

	/* default is SDR */
	return SDR;
}
EXPORT_SYMBOL(hdmitx_common_get_hdr_status);

u32 hdmitx_common_get_vrr_cap(struct hdmitx_common *tx_comm)
{
	if (tx_comm && tx_comm->drm_vrr_ctrl_ops &&
		tx_comm->drm_vrr_ctrl_ops->get_vrr_cap)
		return tx_comm->drm_vrr_ctrl_ops->get_vrr_cap();
	return 0;
}
EXPORT_SYMBOL(hdmitx_common_get_vrr_cap);

int hdmitx_common_get_vrr_mode_group(struct hdmitx_common *tx_comm,
				     struct hdmitx_vrr_mode_group *groups,
				     int max_group)
{
	if (tx_comm && tx_comm->drm_vrr_ctrl_ops &&
		tx_comm->drm_vrr_ctrl_ops->get_vrr_mode_group)
		return tx_comm->drm_vrr_ctrl_ops->get_vrr_mode_group(groups,
							  MAX_VRR_MODE_GROUP);
	return 0;
}
EXPORT_SYMBOL(hdmitx_common_get_vrr_mode_group);

int hdmitx_common_set_vframe_rate_hint(struct hdmitx_common *tx_comm, int rate, void *data)
{
	if (tx_comm && tx_comm->drm_vrr_ctrl_ops &&
		tx_comm->drm_vrr_ctrl_ops->set_vframe_rate_hint)
		return tx_comm->drm_vrr_ctrl_ops->set_vframe_rate_hint(rate, data);
	return 0;
}
EXPORT_SYMBOL(hdmitx_common_set_vframe_rate_hint);

enum hdmi_scan_mode hdmitx_common_get_scan_info(struct hdmitx_common *tx_comm)
{
	struct hdmitx_hw_common *tx_hw_base = tx_comm->tx_hw;
	enum hdmi_scan_mode scan_info = HDMI_SCAN_MODE_NONE;

	scan_info = hdmitx_hw_cntl_config(tx_hw_base, CONF_GET_AVI_SCAN_INFO, 0);

	return scan_info;
}
EXPORT_SYMBOL(hdmitx_common_get_scan_info);

int hdmitx_common_set_scan_info(struct hdmitx_common *tx_comm, enum hdmi_scan_mode val)
{
	enum hdmi_scan_mode scan_info = HDMI_SCAN_MODE_NONE;
	struct hdmitx_hw_common *tx_hw_base = tx_comm->tx_hw;
	struct rx_cap *prxcap = &tx_comm->rxcap;
	enum hdmi_vic vic = HDMI_0_UNKNOWN;

	vic = hdmitx_hw_get_state(tx_hw_base, STAT_VIDEO_VIC, 0);
	/*
	 * need to check whether the value set through the UI is supported by the TV,
	 * If it is supported, set it. If not supported, set the supported value
	 */
	scan_info = hdmitx_check_scan_info(prxcap, val, vic);
	hdmitx_hw_cntl_config(tx_hw_base, CONF_AVI_SCAN_INFO, scan_info);

	return 0;
}
EXPORT_SYMBOL(hdmitx_common_set_scan_info);

/*
 * ARC IN audio capture not working due to init
 * sequence issue of eARC driver and HDMI Tx driver.
 * when eARC driver try to register_earcrx_callback,
 * HDMI Tx driver probe/init is not finish, that lead
 * register_earcrx_callback fail and eARC driver
 * doesn't know if HDMI Tx cable plug in/out.
 * so don't check hdmitx init or not. TODO
 */
int register_earcrx_callback(pf_callback callback)
{
/*
 * when eARC driver try to register_earcrx_callback,
 * HDMI Tx driver probe/init is not finish, that just
 * register_earcrx_callback, and later probe and plugin will
 * notify eARC HDMI Tx cable plug in/out.
 * so don't check hdmitx init
 */
#if defined(CONFIG_AMLOGIC_HDMITX)
	hdmitx_earc_hpdst(callback);
#endif
#if defined(CONFIG_AMLOGIC_HDMITX21)
	hdmitx21_earc_hpdst(callback);
#endif
	return 0;
}
EXPORT_SYMBOL(register_earcrx_callback);

void unregister_earcrx_callback(void)
{
#if defined(CONFIG_AMLOGIC_HDMITX)
	hdmitx_earc_hpdst(NULL);
#endif
#if defined(CONFIG_AMLOGIC_HDMITX21)
	hdmitx21_earc_hpdst(NULL);
#endif
}
EXPORT_SYMBOL(unregister_earcrx_callback);

/* Nofity client */

#include <linux/module.h>

static BLOCKING_NOTIFIER_HEAD(aout_notifier_list);
/*
 * aout_register_client - register a client notifier
 * @nb: notifier block to callback on events
 */
int aout_register_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&aout_notifier_list, nb);
}
EXPORT_SYMBOL(aout_register_client);

/*
 * aout_unregister_client - unregister a client notifier
 * @nb: notifier block to callback on events
 */
int aout_unregister_client(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&aout_notifier_list, nb);
}
EXPORT_SYMBOL(aout_unregister_client);

/*
 * aout_notifier_call_chain - notify clients of fb_events
 */
int aout_notifier_call_chain(unsigned long val, void *v)
{
	return blocking_notifier_call_chain(&aout_notifier_list, val, v);
}
EXPORT_SYMBOL_GPL(aout_notifier_call_chain);

int hdmitx_get_connector(void)
{
	int i = 0;
	int j = 0;
	static const char * const hdmi_types[] = {
		/* venc0 */
		"HDMI-A-A",
		/* venc1 */
		"HDMI-A-B",
		/* venc2 */
		"HDMI-A-C",
	};
	char *conn_types[3] = {};
	char *type;

	conn_types[0] = get_uboot_connector0_type();
#ifdef CONFIG_AMLOGIC_VOUT2_SERVE
	conn_types[1] = get_uboot_connector1_type();
#endif
#ifdef CONFIG_AMLOGIC_VOUT3_SERVE
	conn_types[2] = get_uboot_connector2_type();
#endif
	if (conn_types[0])
		HDMITX_INFO("%s[%d] %s\n", __func__, __LINE__, conn_types[0]);
	if (conn_types[1])
		HDMITX_INFO("%s[%d] %s\n", __func__, __LINE__, conn_types[1]);
	if (conn_types[2])
		HDMITX_INFO("%s[%d] %s\n", __func__, __LINE__, conn_types[2]);

	for (j = 0; j < ARRAY_SIZE(conn_types); j++) {
		type = conn_types[j];
		if (!type)
			continue;
		for (i = 0; i < ARRAY_SIZE(hdmi_types); i++) {
			if (strncmp(type, hdmi_types[i], strlen(hdmi_types[i])) == 0)
				return i;
		}
	}
	if (i < ARRAY_SIZE(hdmi_types))
		return i;

	return 0;
}
