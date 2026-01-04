// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/media/vout/meson_tx_connector/hdmitx_common/hdmitx_common.h>
#include "tvin_global.h"
#include "hdmitx_log.h"
#include "hdmi_rx_repeater.h"
#include "hdmitx_module.h"
#include "hdmitx_audio.h"

static struct hdmitx_common *tx_common_instance;
static u8 hdmi_allm_passthough_en;

static inline void hdmitx_notify_hpd(int hpd, void *p)
{
	if (!tx_common_instance)
		return;

	if (hpd)
		hdmitx_event_mgr_notify(tx_common_instance->event_mgr,
				HDMITX_PLUG, p);
	else
		hdmitx_event_mgr_notify(tx_common_instance->event_mgr,
				HDMITX_UNPLUG, NULL);
}

/* for notify to cec */
int hdmitx_event_notifier_regist(struct notifier_block *nb)
{
	int ret = 0;

	if (!nb || !tx_common_instance)
		return ret;

	ret = hdmitx_event_mgr_notifier_register(tx_common_instance->event_mgr,
		(struct hdmitx_notifier_client *)nb);

	/* update status when register */
	if (!ret && nb->notifier_call) {
		/* if (hdev->tx_comm.hdmi_repeater == 1) */
		hdmitx_notify_hpd(tx_common_instance->base.hpd_state,
			tx_common_instance->base.rxcap.edid_parsing ?
			tx_common_instance->base.edid_buf : NULL);
		/* actually notify phy_addr is not used by CEC/hdmirx */
		/* if (hdev->tx_comm.base.rxcap.physical_addr != 0xffff) { */
		/* if (hdev->tx_comm.hdmi_repeater == 1) */
		/* hdmitx_event_mgr_notify(hdev->tx_comm.event_mgr, */
		/* HDMITX_PHY_ADDR_VALID, */
		/* &hdev->tx_comm.base.rxcap.physical_addr); */
		/* } */
	}

	return ret;
}
EXPORT_SYMBOL(hdmitx_event_notifier_regist);

int hdmitx_event_notifier_unregist(struct notifier_block *nb)
{
	if (!tx_common_instance)
		return -1;

	return hdmitx_event_mgr_notifier_unregister(tx_common_instance->event_mgr,
		(struct hdmitx_notifier_client *)nb);
}
EXPORT_SYMBOL(hdmitx_event_notifier_unregist);

int hdmitx_ext_get_hpd_state(void)
{
	int ret = 0;

	if (!tx_common_instance)
		return -1;

	mutex_lock(&tx_common_instance->base.set_mode_mutex);
	ret = tx_common_instance->base.hpd_state;
	mutex_unlock(&tx_common_instance->base.set_mode_mutex);

	return ret;
}
EXPORT_SYMBOL(hdmitx_ext_get_hpd_state);

struct vsdb_phyaddr *hdmitx_get_phy_addr(void)
{
	if (!tx_common_instance)
		return NULL;

	return &tx_common_instance->base.rxcap.vsdb_phy_addr;
}
EXPORT_SYMBOL(hdmitx_get_phy_addr);

/*
 * for game console-> hdmirx -> hdmitx -> TV
 * interface for hdmirx module
 * ret: false if not update, true if updated
 */
bool hdmitx_update_latency_mode(struct tvin_latency_s *latency_info)
{
	struct hdmitx_common *tx_comm = tx_common_instance;
	bool it_content = false;
	u32 arg = 0;

	if (!tx_comm)
		return false;
	if (tx_comm->tx_hw->chip_data->chip_type < MESON_CPU_ID_T7)
		return false;
	/*
	 * when switch between hdmirx source(ALLM) and hdmitx home(non-ALLM),
	 * the ALLM/1.4 Game will change, need to mute before change
	 */
	bool video_mute = false;

	if (!hdmi_allm_passthough_en)
		return false;
	if (!latency_info)
		return false;

	HDMITX_DEBUG("%s: ll_enabled_in_auto_mode: %d, ll_user_set_mode:%d\n",
		__func__, tx_comm->ll_enabled_in_auto_mode, tx_comm->ll_user_set_mode);

	if (tx_comm->ll_user_set_mode != HDMI_LL_MODE_AUTO)
		return false;
	if (tx_comm->allm_mode == latency_info->allm_mode &&
		tx_comm->it_content == latency_info->it_content &&
		tx_comm->ct_mode == latency_info->cn_type) {
		HDMITX_DEBUG("latency_info not changed, exit\n");
		return false;
	}
	/* refer to allm_mode_store() */
	if (latency_info->allm_mode) {
		if (tx_comm->base.rxcap.allm) {
			//if (hdmitx_dv_en(tx_comm->tx_hw) &&
				//(hdev->rxcap.ifdb_present &&
				//hdev->rxcap.additional_vsif_num < 1)) {
				//HDMITX_INFO("%s: DV enabled, but ifdb_present: %d,
				//additional_vsif_num: %d\n",
				//__func__, hdev->rxcap.ifdb_present,
				//hdev->rxcap.additional_vsif_num);
				//return false;
			//}
			if (!get_rx_active_sts()) {
				video_mute = true;
				arg = VIDEO_MUTE;
				/* hdmitx21_video_mute_op(0, VIDEO_MUTE_PATH_4); */
				hdmitx_hw_cntl(tx_comm->tx_hw, VPU_VIDEO_MUTE_OP,
					(void *)&arg, NULL);
			}
			tx_comm->allm_mode = 1;
			hdmitx_common_setup_vsif_packet(tx_comm, VT_ALLM, 1, NULL);
			tx_comm->ct_mode = 0;
			tx_comm->it_content = 0;
			arg = SET_CT_OFF;
			hdmitx_hw_cntl(tx_comm->tx_hw, AUX_PKT_CONF_AVI_CT, (void *)&arg, NULL);
		}
	} else {
		if (!get_rx_active_sts()) {
			video_mute = true;
			arg = VIDEO_MUTE;
			/* hdmitx21_video_mute_op(0, VIDEO_MUTE_PATH_4); */
			hdmitx_hw_cntl(tx_comm->tx_hw, VPU_VIDEO_MUTE_OP, (void *)&arg, NULL);
		}
		/* disable ALLM firstly */
		if (tx_comm->allm_mode == 1) {
			tx_comm->allm_mode = 0;
			hdmitx_common_setup_vsif_packet(tx_comm, VT_ALLM, 0, NULL);
			if (hdmitx_edid_get_hdmi14_4k_vic(tx_comm->fmt_para.vic) > 0 &&
				!hdmitx_dv_en(tx_comm->tx_hw) &&
				!hdmitx_hdr10p_en(tx_comm->tx_hw))
				hdmitx_common_setup_vsif_packet(tx_comm, VT_HDMI14_4K, 1, NULL);
		}
		tx_comm->it_content = latency_info->it_content;
		it_content = tx_comm->it_content;
		if (tx_comm->base.rxcap.cnc3 && latency_info->cn_type == GAME) {
			tx_comm->ct_mode = 1;
			arg = SET_CT_GAME | it_content << 4;
			hdmitx_hw_cntl(tx_comm->tx_hw, AUX_PKT_CONF_AVI_CT,
				(void *)&arg, NULL);
		} else if (tx_comm->base.rxcap.cnc0 && latency_info->cn_type == GRAPHICS &&
		    latency_info->it_content == 1) {
			tx_comm->ct_mode = 2;
			arg = SET_CT_GRAPHICS | it_content << 4;
			hdmitx_hw_cntl(tx_comm->tx_hw, AUX_PKT_CONF_AVI_CT,
				(void *)&arg, NULL);
		} else if (tx_comm->base.rxcap.cnc1 && latency_info->cn_type == PHOTO) {
			tx_comm->ct_mode = 3;
			arg = SET_CT_PHOTO | it_content << 4;
			hdmitx_hw_cntl(tx_comm->tx_hw, AUX_PKT_CONF_AVI_CT,
				(void *)&arg, NULL);
		} else if (tx_comm->base.rxcap.cnc2 && latency_info->cn_type == CINEMA) {
			tx_comm->ct_mode = 4;
			arg = SET_CT_CINEMA | it_content << 4;
			hdmitx_hw_cntl(tx_comm->tx_hw, AUX_PKT_CONF_AVI_CT,
				(void *)&arg, NULL);
		} else {
			tx_comm->ct_mode = 0;
			arg = SET_CT_OFF | it_content << 4;
			hdmitx_hw_cntl(tx_comm->tx_hw, AUX_PKT_CONF_AVI_CT,
				(void *)&arg, NULL);
		}
	}
	return true;
}
EXPORT_SYMBOL(hdmitx_update_latency_mode);

/* for request reauth from upstream side, for example: hdmirx of T7 */
u8 hdmitx_reauth_request(u8 hdcp_version)
{
	if (!tx_common_instance || !tx_common_instance->hdcptx_comm.hdcp_rpt_en)
		return 0;
	return hdmitx_hw_cntl(tx_common_instance->tx_hw, HDCP_REQ_AUTH, &hdcp_version, NULL);
}
EXPORT_SYMBOL(hdmitx_reauth_request);

void hdmitx_ext_set_audio_output(int enable)
{
	if (!tx_common_instance)
		return;

	hdmitx_audio_mute_op(tx_common_instance, enable, AUDIO_MUTE_PATH_1);
	HDMITX_INFO("audio: enable:%d\n", enable);
	hdmitx_tracer_write_event(tx_common_instance->tx_tracer,
			enable ? HDMITX_AUDIO_UNMUTE : HDMITX_AUDIO_MUTE);
}
EXPORT_SYMBOL(hdmitx_ext_set_audio_output);

int hdmitx_ext_get_audio_status(void)
{
	if (!tx_common_instance)
		return 0;

	return !!tx_common_instance->cur_audio_param.aud_output_en;
}
EXPORT_SYMBOL(hdmitx_ext_get_audio_status);

/* Nofity client */
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

static void hdmitx_earc_hpdst(pf_callback cb)
{
	struct hdmitx_common *tx_comm = tx_common_instance;

	if (!tx_comm)
		return;
	tx_comm->earc_hdmitx_hpdst = cb;
	if (cb && hdmitx_hw_cntl(tx_comm->tx_hw, PLATFORM_GET_HPD_GPI_ST, NULL, NULL))
		cb(true);
}

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
	hdmitx_earc_hpdst(callback);

	return 0;
}
EXPORT_SYMBOL(register_earcrx_callback);

void unregister_earcrx_callback(void)
{
	hdmitx_earc_hpdst(NULL);
}
EXPORT_SYMBOL(unregister_earcrx_callback);

void hdmitx_ext_plugin_handler(void)
{
	/* read edid by earc*/
	if (tx_common_instance) {
		mutex_lock(&tx_common_instance->base.set_mode_mutex);
		hdmitx_common_get_edid(tx_common_instance);
		mutex_unlock(&tx_common_instance->base.set_mode_mutex);
		HDMITX_INFO("read edid by E_ARC\n");
	}
}
EXPORT_SYMBOL(hdmitx_ext_plugin_handler);

void hdmitx_ext_instance_init(struct hdmitx_common *tx_comm)
{
	if (!tx_comm)
		return;

	tx_common_instance = tx_comm;
}
