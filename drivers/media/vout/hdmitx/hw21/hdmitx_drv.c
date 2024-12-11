// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/major.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/devinfo.h>
#include <linux/pinctrl/consumer.h>
#include <linux/amlogic/clk_measure.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/ctype.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/reboot.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
/* #include <linux/amlogic/cpu_version.h> */
#include <linux/amlogic/media/vout/vinfo.h>
#include <linux/amlogic/media/vout/vout_notify.h>
#if IS_ENABLED(CONFIG_AMLOGIC_SND_SOC)
#include <linux/amlogic/media/sound/aout_notify.h>
#endif
#include <linux/amlogic/media/vrr/vrr.h>

#include "hdmitx_module.h"
#include "hdmitx.h"
#include "hdmitx_config.h"
#include "hdmitx_ddc.h"
#include "hdmitx_common.h"

#include <linux/amlogic/gki_module.h>
#include <linux/component.h>
#include <uapi/drm/drm_mode.h>
#include <drm/amlogic/meson_drm_bind.h>
#include <../../vin/tvin/tvin_global.h>
#include <../../vin/tvin/hdmirx/hdmi_rx_repeater.h>
#include "../hdmitx_boot_parameters.h"
#include "../hdmitx_drm.h"
#include "../hdmitx_sysfs_common.h"
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_format_para.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_platform_linux.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_audio.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_edid.h>
#include "../hdmitx_compliance.h"
#include "../hdmitx_check_valid.h"

#ifdef CONFIG_AMLOGIC_DSC
#include <linux/amlogic/media/vout/dsc.h>
#endif

#define HDMI_TX_COUNT 32
#define HDMI_TX_POOL_NUM  6
#define HDMI_TX_RESOURCE_NUM 4
#define HDMI_TX_PWR_CTRL_NUM	6

static u8 hdmi_allm_passthough_en;
unsigned int rx_hdcp2_ver;
/* static unsigned int hdcp_ctl_lvl; */

#define to_hdmitx21_dev(x)	container_of(x, struct hdmitx_dev, tx_comm)

static struct class *hdmitx_class;
static int hdmitx_hook_drm(struct device *device);
static int hdmitx_unhook_drm(struct device *device);
const struct hdmi_timing *hdmitx_mode_match_timing_name(const char *name);
static void hdmitx21_vid_pll_clk_check(struct hdmitx_dev *hdev);
const char *hdmitx_mode_get_timing_name(enum hdmi_vic vic);
/*
 * Normally, after the HPD in or late resume, there will reading EDID, and
 * notify application to select a hdmi mode output. But during the mode
 * setting moment, there may be HPD out. It will clear the edid data, ..., etc.
 * To avoid such case, here adds the hdmimode_mutex to let the HPD in, HPD out
 * handler and mode setting sequentially.
 */
/* static DEFINE_MUTEX(hdmimode_mutex); */

#ifdef CONFIG_OF
static struct amhdmitx_data_s amhdmitx_data_t7 = {
	.chip_type = MESON_CPU_ID_T7,
	.chip_name = "t7",
};

static struct amhdmitx_data_s amhdmitx_data_s5 = {
	.chip_type = MESON_CPU_ID_S5,
	.chip_name = "s5",
};

static struct amhdmitx_data_s amhdmitx_data_s1a = {
	.chip_type = MESON_CPU_ID_S1A,
	.chip_name = "s1a",
};

static struct amhdmitx_data_s amhdmitx_data_s7 = {
	.chip_type = MESON_CPU_ID_S7,
	.chip_name = "s7",
};

static struct amhdmitx_data_s amhdmitx_data_s7d = {
	.chip_type = MESON_CPU_ID_S7D,
	.chip_name = "s7d",
};

static struct amhdmitx_data_s amhdmitx_data_s6 = {
	.chip_type = MESON_CPU_ID_S6,
	.chip_name = "s6",
};

static const struct of_device_id meson_amhdmitx_of_match[] = {
	{
		.compatible	 = "amlogic, amhdmitx-t7",
		.data = &amhdmitx_data_t7,
	},
	{
		.compatible	 = "amlogic, amhdmitx-s5",
		.data = &amhdmitx_data_s5,
	},
	{
		.compatible	 = "amlogic, amhdmitx-s1a",
		.data = &amhdmitx_data_s1a,
	},
	{
		.compatible	 = "amlogic, amhdmitx-s7",
		.data = &amhdmitx_data_s7,
	},
	{
		.compatible	 = "amlogic, amhdmitx-s7d",
		.data = &amhdmitx_data_s7d,
	},
	{
		.compatible	 = "amlogic, amhdmitx-s6",
		.data = &amhdmitx_data_s6,
	},
	{},
};
#else
#define meson_amhdmitx_dt_match NULL
#endif

static struct hdmitx_dev *tx21_dev;

struct hdmitx_dev *get_hdmitx21_device(void)
{
	return tx21_dev;
}
EXPORT_SYMBOL(get_hdmitx21_device);

int get_hdmitx21_init(void)
{
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	if (hdev)
		return hdev->tx_comm.hdmi_init;
	return 0;
}

struct vsdb_phyaddr *get_hdmitx21_phy_addr(void)
{
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	return &hdev->tx_comm.rxcap.vsdb_phy_addr;
}

static struct vout_device_s hdmitx_vdev = {
	.fresh_tx_hdr_pkt = hdmitx_set_drm_pkt,
	.fresh_tx_sbtm_pkt = hdmitx_set_sbtm_pkt,
	.fresh_tx_vsif_pkt = hdmitx_set_vsif_pkt,
	.fresh_tx_hdr10plus_pkt = hdmitx_set_hdr10plus_pkt,
	.fresh_tx_cuva_hdr_vsif = hdmitx_set_cuva_hdr_vsif,
	.fresh_tx_cuva_hdr_vs_emds = hdmitx_set_cuva_hdr_vs_emds,
};

int hdmitx21_set_uevent_state(enum hdmitx_event type, int state)
{
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	return hdmitx_event_mgr_set_uevent_state(hdev->tx_comm.event_mgr,
				type, state);
}

int hdmitx21_set_uevent(enum hdmitx_event type, int val)
{
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	return hdmitx_event_mgr_send_uevent(hdev->tx_comm.event_mgr,
				type, val, false);
}

static inline void hdmitx_notify_hpd(int hpd, void *p)
{
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	if (hpd)
		hdmitx_event_mgr_notify(hdev->tx_comm.event_mgr,
				HDMITX_PLUG, p);
	else
		hdmitx_event_mgr_notify(hdev->tx_comm.event_mgr,
				HDMITX_UNPLUG, NULL);
}

static void hdmitx21_clear_packets(struct hdmitx_hw_common *tx_hw_base)
{
	hdmitx_set_drm_pkt(NULL);
	hdmitx_set_vsif_pkt(EOTF_T_NULL, 0, NULL, true);
	hdmitx_set_hdr10plus_pkt(0, NULL);
	/* clear any VSIF packet left over because of vendor<->vendor2 switch */
	hdmi_vend_infoframe_rawset(NULL, NULL);
	/* stop ALLM packet by hdmitx itself
	 * DV CTS case91: clear HF-VSIF for safety
	 */
	hdmi_vend_infoframe2_rawset(NULL, NULL);
	hdmitx_hw_cntl_config(tx_hw_base, CONF_CLR_AVI_PACKET, 0);
}

/* action in suspend/plugout/disable_module(switch mode) */
static void hdmitx21_ops_disable_hdcp(struct hdmitx_common *tx_comm)
{
	struct hdmitx_dev *hdev = to_hdmitx21_dev(tx_comm);

	hdmitx21_disable_hdcp(hdev);
}

/* action in suspend/plugout handler, should not be done when disable_module */
static void hdmitx21_reset_hdcp_param(struct hdmitx_common *tx_comm)
{
	struct hdmitx_dev *hdev = to_hdmitx21_dev(tx_comm);
	struct hdcp_t *p_hdcp = (struct hdcp_t *)hdev->am_hdcp;

	hdmitx21_rst_stream_type(p_hdcp);
	p_hdcp->saved_upstream_type = 0;
	p_hdcp->rx_update_flag = 0;
	rx_hdcp2_ver = 0;
	hdev->dw_hdcp22_cap = false;
	hdev->tx_comm.is_passthrough_switch = 0;
	/* clear audio/video mute flag of stream type */
	hdmitx21_video_mute_op(1, VIDEO_MUTE_PATH_2);
	hdmitx21_audio_mute_op(1, AUDIO_MUTE_PATH_3);
}

#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
/* hdmi21 specific functions disable */
static void hdmitx21_disable_21_work(void)
{
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	frl_tx_stop();
	hdmitx_set_frl_rate_none(hdev);
	hdmitx_vrr_disable();
#ifdef CONFIG_AMLOGIC_DSC
	if (hdev->tx_hw.base.hdmi_tx_cap.dsc_capable) {
		aml_dsc_enable(false);
		hdmitx_dsc_cvtem_pkt_disable();
		hdev->dsc_en = 0;
	}
#endif
}

static void hdmitx21_disable_frl_work(void)
{
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	frl_tx_stop();
	hdmitx_set_frl_rate_none(hdev);
}
#endif

#ifdef CONFIG_AMLOGIC_LEGACY_EARLY_SUSPEND
#include <linux/amlogic/pm.h>
static void hdmitx_early_suspend(struct early_suspend *h)
{
	struct hdmitx_dev *hdev = (struct hdmitx_dev *)h->param;
	bool need_rst_ratio = hdmitx_find_vendor_ratio(hdev->tx_comm.EDID_buf);

	if (hdev->tx_comm.aon_output) {
		HDMITX_INFO("%s return, HDMI signal enabled\n", __func__);
		return;
	}

	mutex_lock(&hdev->tx_comm.hdmimode_mutex);
	/* step1: keep hdcp auth state before suspend */
	hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_SUSFLAG, 1);
	/*
	 * under suspend, driver should not respond to mode setting,
	 * as it may cause logic abnormal, most importantly,
	 * it will enable hdcp and occupy DDC channel with high
	 * priority, though there's protection in system control,
	 * driver still need protection in case of old android version
	 */
	hdev->tx_comm.suspend_flag = true;
	hdev->tx_comm.qms_log_id = 0;

	HDMITX_INFO("Early Suspend\n");
	/* step2: clear ready status/disable phy/packets/hdcp HW */
	hdmitx_common_output_disable(&hdev->tx_comm,
		true, true, true, false);
	hdmitx21_reset_hdcp_param(&hdev->tx_comm);

	/* step3: SW: post uevent to system */
	hdmitx21_set_uevent(HDMITX_HDCPPWR_EVENT, HDMI_SUSPEND);
	hdmitx21_set_uevent(HDMITX_AUDIO_EVENT, 0);
	if (need_rst_ratio)
		hdmitx_hw_cntl_ddc(&hdev->tx_hw.base, DDC_SCDC_DIV40_SCRAMB, 0);

	mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
}

static void hdmitx_late_resume(struct early_suspend *h)
{
	struct hdmitx_dev *hdev = (struct hdmitx_dev *)h->param;

	if (hdev->tx_comm.aon_output) {
		HDMITX_INFO("%s return, HDMI signal already enabled\n", __func__);
		return;
	}

	mutex_lock(&hdev->tx_comm.hdmimode_mutex);
	hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_AUDIO_MUTE_OP, AUDIO_MUTE);
	hdmitx_common_late_resume(&hdev->tx_comm);
	HDMITX_INFO("Late Resume\n");
	mutex_unlock(&hdev->tx_comm.hdmimode_mutex);

	/* notify to drm hdmi  */
	hdmitx_fire_drm_hpd_cb_unlocked(&hdev->tx_comm);
}

/*
 * Note: HPLL is disabled when suspend/shutdown, and should
 * not be called when reboot/early suspend, otherwise
 * there will be no vsync for drm.
 */
static int hdmitx_reboot_notifier(struct notifier_block *nb,
				  unsigned long action, void *data)
{
	struct hdmitx_dev *hdev = container_of(nb, struct hdmitx_dev, nb);

	hdev->tx_comm.ready = 0;
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
	hdmitx_vrr_disable();
#endif

	hdmitx_common_avmute_locked(&hdev->tx_comm, SET_AVMUTE, AVMUTE_PATH_HDMITX);

#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
	hdmitx21_disable_frl_work();
#ifdef CONFIG_AMLOGIC_DSC
	if (hdev->tx_hw.base.hdmi_tx_cap.dsc_capable) {
		aml_dsc_enable(false);
		hdmitx_dsc_cvtem_pkt_disable();
		hdev->dsc_en = 0;
	}
#endif
#endif
	if (hdev->tx_comm.rxsense_policy)
		cancel_delayed_work(&hdev->tx_comm.work_rxsense);
	if (hdev->tx_comm.cedst_en)
		cancel_delayed_work(&hdev->tx_comm.work_cedst);
	hdmitx21_disable_hdcp(hdev);
	hdmitx21_rst_stream_type(hdev->am_hdcp);
	hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_TMDS_PHY_OP, TMDS_PHY_DISABLE);

	return NOTIFY_OK;
}

static struct early_suspend hdmitx_early_suspend_handler = {
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 10,
	.suspend = hdmitx_early_suspend,
	.resume = hdmitx_late_resume,
};
#endif

static void restore_mute(void)
{
	struct hdmitx_dev *hdev = get_hdmitx21_device();
	atomic_t kref_video_mute = hdev->tx_comm.kref_video_mute;
	atomic_t kref_audio_mute = hdev->kref_audio_mute;

	if (!(atomic_sub_and_test(0, &kref_video_mute))) {
		HDMITX_INFO("%s: hdmitx21_video_mute_op(0, 0) call\n", __func__);
		hdmitx21_video_mute_op(0, 0);
	}
	if (!(atomic_sub_and_test(0, &kref_audio_mute))) {
		HDMITX_INFO("%s: hdmitx21_audio_mute_op(0,0) call\n", __func__);
		hdmitx21_audio_mute_op(0, 0);
	}
}

void hdmitx_set_frl_rate_none(struct hdmitx_dev *hdev)
{
	u8 data;

	/* such as T7 unsupport FRL, skip frl flow */
	if (hdev->tx_hw.base.hdmi_tx_cap.tx_max_frl_rate == FRL_NONE)
		return;

	if (hdev->tx_comm.rxcap.max_frl_rate > FRL_NONE &&
		hdev->tx_comm.rxcap.scdc_present == 1 &&
		hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_GET_FRL_MODE, 0) > FRL_NONE) {
		scdc_tx_frl_cfg1_set(0);
		data = scdc_tx_update_flags_get();
		if (data & FLT_UPDATE)
			scdc_tx_update_flags_set(FLT_UPDATE);
		hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_CLR_FRL_MODE, 0);
	}
}

static void hdmitx_pre_display_init(struct hdmitx_dev *hdev)
{
	u8 update_flags = 0;

	if (hdev->tx_comm.rxcap.max_frl_rate > FRL_NONE &&
		hdev->tx_comm.rxcap.scdc_present == 1 &&
		/* hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_GET_FRL_MODE, 0)&& */
		hdev->frl_rate == FRL_NONE) {
		/*
		 * refer to LTS:L Source in FRL link training procedure:
		 * for FRL->legacy TMDS mode(LTS:4->LTS:L),
		 * or source init TMDS mode(LTS:1->LTS:L)
		 * 1. IF (SCDC_Present = 1)
		 * Source shall clear (=0) FRL_Rate indicating TMDS.
		 * IF FLT_update is currently set, Source shall clear
		 * FLT_update by writing "1".
		 * 2.Source shall start legacy TMDS operation when its
		 * content is ready.
		 */
		scdc_tx_frl_cfg1_set(0);
		update_flags = scdc_tx_update_flags_get();
		if ((update_flags & FLT_UPDATE) != 0)
			scdc_tx_update_flags_set(update_flags);
		hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_TMDS_PHY_OP, TMDS_PHY_DISABLE);
	} else if (hdev->tx_comm.rxcap.max_frl_rate > FRL_NONE &&
		hdev->tx_comm.rxcap.scdc_present == 1 &&
		hdev->frl_rate > FRL_NONE &&
		hdev->frl_rate < FRL_RATE_MAX &&
		hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_GET_FRL_MODE, 0) == FRL_NONE) {
		/*
		 * refer to LTS:L Source in FRL link training procedure:
		 * for case(LTS:L->LTS:2) switch from TMDS mode to FRL mode
		 *
		 * IF (Max_FRL_Rate > 0) AND (SCDC_Present = 1) AND (SCDC Sink Version != 0)
		 * IF Source chooses to operate in FRL mode
		 * Source should use AV mute
		 * Source shall stop TMDS transmission
		 * Source shall EXIT to LTS:2
		 * END IF
		 * END IF
		 */
		/* AV mute is set by system earlier */
		hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_TMDS_PHY_OP, TMDS_PHY_DISABLE);
	} else if (hdev->frl_rate == FRL_NONE &&
		hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_GET_FRL_MODE, 0) == FRL_NONE) {
		/*
		 * for cases switch between TMDS modes
		 * per hdmi2.1 spec chapter 6.1.3.2, when source change
		 * tmds_bit_clk_ratio, should follow the steps:
		 * 1.disable tmds clk/data
		 * 2.change tmds_bit_clk_ratio 0 <-> 1
		 * 3.resume tmds clk/data transmission in 1~100 ms
		 */
		hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_TMDS_PHY_OP, TMDS_PHY_DISABLE);
	}
	/*
	 * for cases switch between FRL modes: todo
	 * may refer to LTS:P Source in FRL link training procedure
	 * LTS:P->LTS:2
	 * IF Source initiates request for retraining
	 * Source should use AV mute if video is active
	 * Source shall stop FRL transmission including Gap-character-only transmission
	 * Source shall EXIT to LTS:2
	 */

	/* clear vsif/avi */
}

static void hdmitx_up_hdcp_timeout_handler(struct work_struct *work)
{
	struct hdmitx_dev *hdev = container_of((struct delayed_work *)work,
		struct hdmitx_dev, work_up_hdcp_timeout);

	if (hdcp_need_control_by_upstream(&hdev->tx_hw.base)) {
		HDMITX_INFO("enable hdcp as wait upstream hdcp timeout\n");
		/* note: hdcp should only be started when hdmi signal ready */
		mutex_lock(&hdev->tx_comm.hdmimode_mutex);
		if (!hdev->tx_comm.ready || !hdev->tx_comm.hpd_state) {
			HDMITX_INFO("signal ready: %d, hpd_state: %d, exit hdcp\n",
				hdev->tx_comm.ready, hdev->tx_comm.hpd_state);
			mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
			return;
		}
		hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_AVMUTE_OP, CLR_AVMUTE);
		hdmitx21_enable_hdcp(hdev);
		mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
	} else {
		HDMITX_INFO("wait upstream hdcp timeout, but now not in hdmirx channel\n");
	}
}

static void hdmitx_start_hdcp_handler(struct work_struct *work)
{
	struct hdmitx_dev *hdev = container_of((struct delayed_work *)work,
		struct hdmitx_dev, work_start_hdcp);
	unsigned long timeout_sec;
	struct hdmitx_common *tx_comm = &hdev->tx_comm;

	if (hdcp_need_control_by_upstream(&hdev->tx_hw.base)) {
		if (hdev->tx_comm.is_passthrough_switch) {
			HDMITX_INFO("enable hdcp by passthrough switch mode\n");
			/* note: hdcp should only be started when hdmi signal ready */
			mutex_lock(&hdev->tx_comm.hdmimode_mutex);
			if (!hdev->tx_comm.ready || !tx_comm->hpd_state) {
				HDMITX_INFO("signal ready: %d, hpd_state: %d, eixt hdcp\n",
					hdev->tx_comm.ready, tx_comm->hpd_state);
				hdev->tx_comm.is_passthrough_switch = 0;
				mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
				return;
			}
			hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_AVMUTE_OP, CLR_AVMUTE);
			hdmitx21_enable_hdcp(hdev);
			mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
		} else {
			/* for source->hdmirx->hdmitx->tv, plug on tv side */
			/*
			 * 1.for repeater CTS, only start hdcp by upstream side
			 * 2.however if upstream source side no signal output
			 * or never start hdcp auth with hdmirx(such as PC),
			 * then we add 5S timeout period, after 5S timeout,
			 * it means that no input source start hdcp auth with
			 * hdmirx, or "no signal" on hdmirx side, then hdmitx
			 * side will start hdcp auth itself.
			 * thus both 1/2 will be satisfied.
			 */
			HDMITX_INFO("hdcp should started by upstream, wait...\n");
			/* timeout period: hdcp1.4 5S, hdcp2.2 2S */
			if (rx_hdcp2_ver)
				timeout_sec = 2;
			else
				timeout_sec = hdev->up_hdcp_timeout_sec;
			schedule_delayed_work(&hdev->work_up_hdcp_timeout,
				timeout_sec * HZ);
		}
	} else {
		mutex_lock(&hdev->tx_comm.hdmimode_mutex);
		if (!hdev->tx_comm.ready || !tx_comm->hpd_state) {
			HDMITX_INFO("signal ready: %d, hpd_state: %d, eixt hdcp2\n",
				hdev->tx_comm.ready, tx_comm->hpd_state);
			hdev->tx_comm.is_passthrough_switch = 0;
			mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
			return;
		}
		hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_AVMUTE_OP, CLR_AVMUTE);
		hdmitx21_enable_hdcp(hdev);
		mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
	}
	/* clear after start hdcp */
	hdev->tx_comm.is_passthrough_switch = 0;
}

static DEFINE_MUTEX(aud_mute_mutex);
void hdmitx21_audio_mute_op(u32 flag, unsigned int path)
{
	static unsigned int aud_mute_path;
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	mutex_lock(&aud_mute_mutex);
	if (flag == 0)
		aud_mute_path |= path;
	else
		aud_mute_path &= ~path;
	hdev->tx_comm.cur_audio_param.aud_output_en = !aud_mute_path;

	if (flag == 0) {
		HDMITX_INFO("audio: AUD_MUTE path=0x%x\n", path);
		hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_AUDIO_MUTE_OP, AUDIO_MUTE);
	} else {
		/* unmute only if none of the paths are muted */
		if (aud_mute_path == 0) {
			HDMITX_INFO("audio: AUD_UNMUTE path=0x%x\n", path);
			hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_AUDIO_MUTE_OP, AUDIO_UNMUTE);
		}
	}
	mutex_unlock(&aud_mute_mutex);
}

static DEFINE_MUTEX(vid_mute_mutex);
void hdmitx21_video_mute_op(u32 flag, unsigned int path)
{
	static unsigned int vid_mute_path;
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	mutex_lock(&vid_mute_mutex);
	if (flag == 0)
		vid_mute_path |= path;
	else
		vid_mute_path &= ~path;

	if (flag == 0) {
		HDMITX_INFO("%s: VID_MUTE path=0x%x\n", __func__, path);
		hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_VIDEO_MUTE_OP, VIDEO_MUTE);
	} else {
		/* unmute only if none of the paths are muted */
		if (vid_mute_path == 0) {
			HDMITX_INFO("%s: VID_UNMUTE path=0x%x\n", __func__, path);
			hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_VIDEO_MUTE_OP, VIDEO_UNMUTE);
		}
	}
	mutex_unlock(&vid_mute_mutex);
}

/* SBTM PKT test */
void hdmitx21_send_sbtm_pkt(void)
{
	u8 hb[3] = {0x0};
	u8 pb[28] = {0x0};

	/* header[7:0] packet type; EMP packet = 0x7f */
	hb[0] = 0x7f;
	/* header[15:8]; [7]:first; [6]:last */
	hb[1] = 0xc0;
	/* sequence_index */
	hb[2] = 0x00;

	/* [7]:new; [6]:end; [5:4]:DS_type; [3]:AFR; [2]:VFR */
	pb[0] = 0x96;
	/* reserved */
	pb[1] = 0x00;
	/* Organization_ID */
	pb[2] = 0x01;
	/* data_set_tag(msb) */
	pb[3] = 0x00;
	/* data_set_tag(lsb) */
	pb[4] = 0x03;
	/* data_set_length(msb) when ID>0, then length = 0 */
	pb[5] = 0x00;
	/* data_set_length(lsb) */
	pb[6] = 0x00;

	pb[7] = 0x11;
	pb[8] = 0x12;
	pb[9] = 0x13;
	pb[10] = 0x14;
	pb[11] = 0x15;
	pb[12] = 0x16;
	pb[13] = 0x17;
	pb[14] = 0x18;
	pb[15] = 0x19;
	pb[16] = 0x1a;
	pb[17] = 0x1b;
	pb[18] = 0x1c;
	pb[19] = 0x1d;
	pb[20] = 0x1e;
	pb[21] = 0x1f;
	pb[22] = 0x20;
	pb[23] = 0x21;
	pb[24] = 0x22;
	pb[25] = 0x23;
	pb[26] = 0x24;
	pb[27] = 0x25;

	hdmi_sbtm_infoframe_rawset(hb, pb);
}

static ssize_t config_store(struct device *dev,
			    struct device_attribute *attr,
			    const char *buf, size_t count)
{
	int ret = 0;
	struct master_display_info_s data = {0};
	struct hdr10plus_para hdr_data = {0x1, 0x2, 0x3};
	struct cuva_hdr_vs_emds_para cuva_data = {0x1, 0x2, 0x3};
	unsigned char pb[28] = {0x46, 0xD0, 0x00, 0x00, 0x00, 0x00, 0x46, 0xD0,
	0x00, 0x10, 0x21, 0xaa, 0x9b, 0x96, 0x19, 0xfc, 0x19, 0x75, 0xd5, 0x78,
	0x10, 0x21, 0xaa, 0x9b, 0x96, 0x19, 0xfc, 0x19};
	unsigned char hb[3] = {0x01, 0x02, 0x03};
	struct dv_vsif_para vsif_para = {0};
	/* unsigned int mute_us = 0; */
	/* unsigned int mute_frames = 0; */
	struct hdmitx_dev *hdev = dev_get_drvdata(dev);

	HDMITX_INFO("config: %s\n", buf);

	if (strncmp(buf, "info", 4) == 0) {
		HDMITX_INFO("%x %x %x %x %x %x\n",
			hdmitx_hw_get_hdr_st(&hdev->tx_hw.base),
			hdmitx_hw_get_dv_st(&hdev->tx_hw.base),
			hdmitx_hw_get_hdr10p_st(&hdev->tx_hw.base),
			hdmitx_hdr_en(&hdev->tx_hw.base),
			hdmitx_dv_en(&hdev->tx_hw.base),
			hdmitx_hdr10p_en(&hdev->tx_hw.base)
			);
	} else if (strncmp(buf, "3d", 2) == 0) {
		/* Second, set 3D parameters */
		if (strncmp(buf + 2, "tb", 2) == 0) {
			hdev->tx_comm.flag_3dtb = 1;
			hdev->tx_comm.flag_3dss = 0;
			hdev->tx_comm.flag_3dfp = 0;
			hdmi21_set_3d(hdev, T3D_TAB, 0);
		} else if ((strncmp(buf + 2, "lr", 2) == 0) ||
			(strncmp(buf + 2, "ss", 2) == 0)) {
			unsigned long sub_sample_mode = 0;

			hdev->tx_comm.flag_3dtb = 0;
			hdev->tx_comm.flag_3dss = 1;
			hdev->tx_comm.flag_3dfp = 0;
			if (buf[2])
				ret = kstrtoul(buf + 2, 10,
					       &sub_sample_mode);
			/* side by side */
			hdmi21_set_3d(hdev, T3D_SBS_HALF,
				    sub_sample_mode);
		} else if (strncmp(buf + 2, "fp", 2) == 0) {
			hdev->tx_comm.flag_3dtb = 0;
			hdev->tx_comm.flag_3dss = 0;
			hdev->tx_comm.flag_3dfp = 1;
			hdmi21_set_3d(hdev, T3D_FRAME_PACKING, 0);
		} else if (strncmp(buf + 2, "off", 3) == 0) {
			hdev->tx_comm.flag_3dfp = 0;
			hdev->tx_comm.flag_3dtb = 0;
			hdev->tx_comm.flag_3dss = 0;
			hdmi21_set_3d(hdev, T3D_DISABLE, 0);
		}
	} else if (strncmp(buf, "sdr_hdr_dov", 11) == 0) {
		/*
		 * firstly stay at SDR state, then send hdr->dv packet to
		 * emulate SDR->HDR->DV switch, DRM-TX-47
		 */
		/* step1: SDR-->HDR */
		data.features = 0x00091000;
		hdmitx_set_drm_pkt(&data);
		/* mute_us = mute_frames * hdmitx_get_frame_duration(); */
		/* usleep_range(mute_us, mute_us + 10); */
		/* step2: HDR->DV_LL */
		vsif_para.ver = 0x1;
		vsif_para.length = 0x1b;
		vsif_para.ver2_l11_flag = 0;
		vsif_para.vers.ver2.low_latency = 1;
		vsif_para.vers.ver2.dobly_vision_signal = 1;
		hdmitx_set_vsif_pkt(4, 0, &vsif_para, false);
	} else if (strncmp(buf, "sdr", 3) == 0) {
		data.features = 0x00010100;
		hdmitx_set_drm_pkt(&data);
	} else if (strncmp(buf, "hdr", 3) == 0) {
		data.features = 0x00091000;
		hdmitx_set_drm_pkt(&data);
	} else if (strncmp(buf, "sbtm", 4) == 0) {
		struct vtem_sbtm_st sbtm = {
			.sbtm_ver = 0x2,
			.sbtm_mode = 0x3,
			.sbtm_type = 0x1,
			.grdm_min = 0x1,
			.grdm_lum = 2,
			/* MD2/3 */
			.frmpblimitint = 0xdcba,
		};
		hdmitx_set_sbtm_pkt(&sbtm);
	} else if (strncmp(buf, "hlg", 3) == 0) {
		data.features = 0x00091200;
		hdmitx_set_drm_pkt(&data);
	} else if (strncmp(buf, "vsif", 4) == 0) {
		if (buf[4] == '1' && buf[5] == '1') {
			/* DV STD */
			vsif_para.ver = 0x1;
			vsif_para.length = 0x1b;
			vsif_para.ver2_l11_flag = 0;
			vsif_para.vers.ver2.low_latency = 0;
			vsif_para.vers.ver2.dobly_vision_signal = 1;
			hdmitx_set_vsif_pkt(1, 1, &vsif_para, false);
		} else if (buf[4] == '1' && buf[5] == '0') {
			/* DV STD packet, but dolby_vision_signal bit cleared */
			vsif_para.ver = 0x1;
			vsif_para.length = 0x1b;
			vsif_para.ver2_l11_flag = 0;
			vsif_para.vers.ver2.low_latency = 0;
			vsif_para.vers.ver2.dobly_vision_signal = 0;
			hdmitx_set_vsif_pkt(1, 1, &vsif_para, false);
		} else if (buf[4] == '4' && buf[5] == '1') {
			/* DV LL */
			vsif_para.ver = 0x1;
			vsif_para.length = 0x1b;
			vsif_para.ver2_l11_flag = 0;
			vsif_para.vers.ver2.low_latency = 1;
			vsif_para.vers.ver2.dobly_vision_signal = 1;
			hdmitx_set_vsif_pkt(4, 0, &vsif_para, false);
		}  else if (buf[4] == '4' && buf[5] == '0') {
			/* DV LL packet, but dolby_vision_signal bit cleared */
			vsif_para.ver = 0x1;
			vsif_para.length = 0x1b;
			vsif_para.ver2_l11_flag = 0;
			vsif_para.vers.ver2.low_latency = 1;
			vsif_para.vers.ver2.dobly_vision_signal = 0;
			hdmitx_set_vsif_pkt(4, 0, &vsif_para, false);
		} else if (buf[4] == '0') {
			/* exit DV to SDR */
			hdmitx_set_vsif_pkt(0, 0, NULL, true);
		}
	} else if (strncmp(buf, "hdr10+", 6) == 0) {
		hdmitx_set_hdr10plus_pkt(1, &hdr_data);
	} else if (strncmp(buf, "cuva", 4) == 0) {
		hdmitx_set_cuva_hdr_vs_emds(&cuva_data);
	} else if (strncmp(buf, "w_dhdr", 6) == 0) {
		hdmitx21_write_dhdr_sram();
	} else if (strncmp(buf, "r_dhdr", 6) == 0) {
		hdmitx21_read_dhdr_sram();
	} else if (strncmp(buf, "t_avi", 5) == 0) {
		hdmi_avi_infoframe_rawset(hb, pb);
	} else if (strncmp(buf, "t_audio", 7) == 0) {
		hdmi_audio_infoframe_rawset(hb, pb);
	} else if (strncmp(buf, "t_sbtm", 6) == 0) {
		hdmitx21_send_sbtm_pkt();
	}
	return count;
}

static void hdmitx21_ext_set_audio_output(bool enable)
{
	hdmitx21_audio_mute_op(enable, AUDIO_MUTE_PATH_1);
	HDMITX_INFO("audio: enable:%d\n", enable);
}

static int hdmitx21_ext_get_audio_status(void)
{
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	return !!hdev->tx_comm.cur_audio_param.aud_output_en;
}

static inline int com_str(const char *buf, const char *str)
{
	return strncmp(buf, str, strlen(str)) == 0;
}

/*
 * for game console-> hdmirx -> hdmitx -> TV
 * interface for hdmirx module
 * ret: false if not update, true if updated
 */
bool hdmitx_update_latency_info(struct tvin_latency_s *latency_info)
{
	struct hdmitx_dev *hdev = get_hdmitx21_device();
	struct hdmitx_common *tx_comm = &hdev->tx_comm;
	bool it_content = false;
	/*
	 * when switch between hdmirx source(ALLM) and hdmitx home(non-ALLM),
	 * the ALLM/1.4 Game will change, need to mute before change
	 */
	bool video_mute = false;

	if (!hdmi_allm_passthough_en)
		return false;
	if (!latency_info)
		return false;

	HDMITX_INFO("%s: ll_enabled_in_auto_mode: %d, ll_user_set_mode:%d\n",
		__func__, hdev->tx_comm.ll_enabled_in_auto_mode, hdev->tx_comm.ll_user_set_mode);

	if (hdev->tx_comm.ll_user_set_mode != HDMI_LL_MODE_AUTO) {
		HDMITX_INFO("%s:non-auto mode,return, allm_mode: %d, it_content: %d, cn_type: %d\n",
			__func__,
			latency_info->allm_mode,
			latency_info->it_content,
			latency_info->cn_type);
		return false;
	}
	HDMITX_INFO("%s: allm_mode: %d, it_content: %d, cn_type: %d\n",
		__func__, latency_info->allm_mode, latency_info->it_content, latency_info->cn_type);
	if (tx_comm->allm_mode == latency_info->allm_mode &&
		tx_comm->it_content == latency_info->it_content &&
		tx_comm->ct_mode == latency_info->cn_type) {
		HDMITX_INFO("latency_info not changed, exit\n");
		return false;
	}
	/* refer to allm_mode_store() */
	if (latency_info->allm_mode) {
		if (tx_comm->rxcap.allm) {
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
				/* hdmitx21_video_mute_op(0, VIDEO_MUTE_PATH_4); */
				hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_VIDEO_MUTE_OP,
					VIDEO_MUTE);
			}
			tx_comm->allm_mode = 1;
			HDMITX_INFO("%s: enabling ALLM\n", __func__);
			hdmitx_common_setup_vsif_packet(tx_comm, VT_ALLM, 1, NULL);
			tx_comm->ct_mode = 0;
			tx_comm->it_content = 0;
			hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_CT_MODE, SET_CT_OFF);
		}
	} else {
		if (!get_rx_active_sts()) {
			video_mute = true;
			/* hdmitx21_video_mute_op(0, VIDEO_MUTE_PATH_4); */
			hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_VIDEO_MUTE_OP, VIDEO_MUTE);
		}
		/* disable ALLM firstly */
		if (tx_comm->allm_mode == 1) {
			tx_comm->allm_mode = 0;
			HDMITX_INFO("%s: disabling ALLM before enable/disable game mode\n",
			__func__);
			hdmitx_common_setup_vsif_packet(tx_comm, VT_ALLM, 0, NULL);
			if (hdmitx_edid_get_hdmi14_4k_vic(tx_comm->fmt_para.vic) > 0 &&
				!hdmitx_dv_en(tx_comm->tx_hw) &&
				!hdmitx_hdr10p_en(tx_comm->tx_hw))
				hdmitx_common_setup_vsif_packet(tx_comm, VT_HDMI14_4K, 1, NULL);
		}
		tx_comm->it_content = latency_info->it_content;
		it_content = tx_comm->it_content;
		if (tx_comm->rxcap.cnc3 && latency_info->cn_type == GAME) {
			tx_comm->ct_mode = 1;
			HDMITX_INFO("%s: enabling GAME mode\n", __func__);
			hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_CT_MODE,
				SET_CT_GAME | it_content << 4);
		} else if (tx_comm->rxcap.cnc0 && latency_info->cn_type == GRAPHICS &&
		    latency_info->it_content == 1) {
			tx_comm->ct_mode = 2;
			HDMITX_INFO("%s: enabling GRAPHICS mode\n", __func__);
			hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_CT_MODE,
				SET_CT_GRAPHICS | it_content << 4);
		} else if (tx_comm->rxcap.cnc1 && latency_info->cn_type == PHOTO) {
			tx_comm->ct_mode = 3;
			HDMITX_INFO("%s: enabling PHOTO mode\n", __func__);
			hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_CT_MODE,
				SET_CT_PHOTO | it_content << 4);
		} else if (tx_comm->rxcap.cnc2 && latency_info->cn_type == CINEMA) {
			tx_comm->ct_mode = 4;
			HDMITX_INFO("%s: enabling CINEMA mode\n", __func__);
			hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_CT_MODE,
				SET_CT_CINEMA | it_content << 4);
		} else {
			tx_comm->ct_mode = 0;
			HDMITX_INFO("%s: No GAME or CT mode\n", __func__);
			hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_CT_MODE,
				SET_CT_OFF | it_content << 4);
		}
	}
	return true;
}
EXPORT_SYMBOL(hdmitx_update_latency_info);

static DEVICE_ATTR_WO(config);

static int hdmitx21_pre_enable_mode(struct hdmitx_common *tx_comm, struct hdmi_format_para *para)
{
	struct hdmitx_dev *hdev = to_hdmitx21_dev(tx_comm);
#ifdef CONFIG_AMLOGIC_DSC
	struct dsc_notifier_data_s dsc_notifier_data;
	int ret = -1;
#endif
	enum frl_rate_enum source_test_frl_rate = FRL_NONE;

	/*
	 * disable hdcp before set mode if hdcp enabled.
	 * normally hdcp is disabled before setting mode
	 * when disable phy, but for special case of bootup,
	 * if mode changed as it's different with uboot mode,
	 * hdcp is not stopped firstly, and may hdcp fail
	 */
	if (!hdcp_need_control_by_upstream(&hdev->tx_hw.base))
		hdmitx21_disable_hdcp(hdev);

	hdev->frl_rate = FRL_NONE;
	hdev->dsc_en = 0;
	/* below is only for FRL/DSC */
	if (tx_comm->tx_hw->hdmi_tx_cap.tx_max_frl_rate == FRL_NONE)
		return 0;

	if (tx_comm->rxcap.max_frl_rate) {
		u8 sink_ver = scdc_tx_sink_version_get();
		u8 test_cfg = 0;

		hdev->frl_rate = para->frl_rate;
		hdev->dsc_en = para->dsc_en;

		if (!sink_ver)
			HDMITX_INFO("sink version %d\n", sink_ver);
		scdc_tx_source_version_set(1);
		test_cfg = scdc_tx_source_test_cfg_get();
		/* per 2.1 spec, if both are set, then treat as if both are cleared */
		if ((test_cfg & FRL_MAX) && (test_cfg & DSC_FRL_MAX)) {
			HDMITX_INFO("warning: both FRL_MAX and DSC_FRL_MAX are set, ignore\n");
		} else if (test_cfg & FRL_MAX) {
			source_test_frl_rate = min(tx_comm->tx_hw->hdmi_tx_cap.tx_max_frl_rate,
				hdev->tx_comm.rxcap.max_frl_rate);
			HDMITX_INFO("CTS: choose frl_max %d\n", source_test_frl_rate);
		} else if (test_cfg & DSC_FRL_MAX) {
			source_test_frl_rate = min(tx_comm->tx_hw->hdmi_tx_cap.tx_max_frl_rate,
				hdev->tx_comm.rxcap.dsc_max_frl_rate);
			HDMITX_INFO("CTS: choose dsc_frl_max %d\n", source_test_frl_rate);
		}
		if (hdev->frl_rate > tx_comm->tx_hw->hdmi_tx_cap.tx_max_frl_rate)
			HDMITX_ERROR("Current frl_rate %d is larger than tx_max_frl_rate %d\n",
				hdev->frl_rate, tx_comm->tx_hw->hdmi_tx_cap.tx_max_frl_rate);
	} else {
		hdev->frl_rate = 0;
		hdev->dsc_en = 0;
	}

	/* source_test_frl_rate has the highest priority */
	if (source_test_frl_rate > FRL_NONE && source_test_frl_rate < FRL_RATE_MAX)
		hdev->frl_rate = source_test_frl_rate;

#ifdef CONFIG_AMLOGIC_DSC
	if (hdev->dsc_en) {
		/*
		 * notify hdmitx format to dsc, and dsc module will
		 * calculate pps data and venc/pixel clock
		 */
		dsc_notifier_data.pic_width = para->timing.h_active;
		dsc_notifier_data.pic_height = para->timing.v_active;
		dsc_notifier_data.color_format = para->cs;
		/*
		 * note: for y422 need set bpc to 8 in pps,
		 * otherwise y422 iter in cts HFR1-85 will fail
		 */
		if (para->cs == HDMI_COLORSPACE_YUV422)
			dsc_notifier_data.bits_per_component = 8;
		else if (para->cd == COLORDEPTH_24B)
			dsc_notifier_data.bits_per_component = 8;
		else if (para->cd == COLORDEPTH_30B)
			dsc_notifier_data.bits_per_component = 10;
		else if (para->cd == COLORDEPTH_36B)
			dsc_notifier_data.bits_per_component = 12;
		else
			dsc_notifier_data.bits_per_component = 8;
		dsc_notifier_data.fps = para->timing.v_freq;
		ret = aml_set_dsc_input_param(&dsc_notifier_data);
		if (ret < 0) {
			HDMITX_ERROR("[%s] set dsc input param error\n", __func__);
		} else {
			hdmitx_get_dsc_data(&hdev->dsc_data);
			HDMITX_INFO("dsc provide enc0_clk: %d, cts_hdmi_pixel_clk: %d\n",
				hdev->dsc_data.enc0_clk,
				hdev->dsc_data.cts_hdmi_tx_pixel_clk);
		}
	}
#endif
	/* if manual_frl_rate is true, set to force frl_rate */
	if (hdev->manual_frl_rate) {
		hdev->frl_rate = hdev->manual_frl_rate;
		HDMITX_INFO("manually frl rate %d\n", hdev->frl_rate);
	}

	hdmitx_pre_display_init(hdev);
	return 0;
}

static bool is_frl_ready(struct hdmitx_dev *hdev)
{
	enum frl_rate_enum tx_frl_rate =
		hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_GET_FRL_MODE, 0);
	enum frl_rate_enum rx_frl_rate;

	/* not check frl rate under TMDS output */
	if (tx_frl_rate == FRL_NONE)
		return true;
	scdc_tx_frl_get_rx_rate((u8 *)&rx_frl_rate);
	/* check frl_rate between TX and RX */
	if (tx_frl_rate != rx_frl_rate) {
		HDMITX_ERROR("tx_frl_rate: %d, rx_frl_rate: %d\n",
			tx_frl_rate, rx_frl_rate);
		return false;
	} else {
		return true;
	}
}

static void hdmitx21_bootup_update_vinfo(struct hdmitx_dev *hdev)
{
	struct vinfo_s *vinfo = NULL;
	struct hdmitx_common *tx_comm = &hdev->tx_comm;

	edidinfo_attach_to_vinfo(tx_comm);
	update_vinfo_from_formatpara(tx_comm);
	vinfo = hdmitx_get_current_vinfo(NULL);
	if (vinfo) {
		vinfo->cur_enc_ppc = 1;
		if (hdev->frl_rate > FRL_NONE)
			vinfo->cur_enc_ppc = 4;
#ifdef CONFIG_AMLOGIC_DSC
		/* can also use if (hdev->dsc_en) */
		if (get_dsc_en()) {
			if (hdev->tx_comm.fmt_para.cs == HDMI_COLORSPACE_RGB)
				vinfo->vpp_post_out_color_fmt = 1;
			else
				vinfo->vpp_post_out_color_fmt = 0;
		} else {
			vinfo->vpp_post_out_color_fmt = 0;
		}
#endif
		HDMITX_INFO("vinfo: set cur_enc_ppc as %d, vpp color: %d\n",
			vinfo->cur_enc_ppc, vinfo->vpp_post_out_color_fmt);
	}

	/* started after output setting done */
	if (hdev->tx_comm.cedst_en) {
		cancel_delayed_work(&hdev->tx_comm.work_cedst);
		queue_delayed_work(hdev->tx_comm.cedst_wq, &hdev->tx_comm.work_cedst, 0);
	}
}

static int hdmitx21_enable_mode(struct hdmitx_common *tx_comm, struct hdmi_format_para *para)
{
	int ret;
	struct hdmitx_dev *hdev = to_hdmitx21_dev(tx_comm);

	ret = hdmitx21_set_display(hdev, para->vic);

	if (ret >= 0)
		restore_mute();
	hdmitx21_set_audio(hdev, &hdev->tx_comm.cur_audio_param);

	return 0;
}

static int hdmitx21_post_enable_mode(struct hdmitx_common *tx_comm, struct hdmi_format_para *para)
{
	struct hdmitx_dev *hdev = to_hdmitx21_dev(tx_comm);
	struct vinfo_s *vinfo = NULL;

	/*
	 * wait for TV detect signal stable,
	 * otherwise hdcp may easily auth fail
	 */
	if (hdev->tx_comm.not_restart_hdcp) {
		/* self clear */
		hdev->tx_comm.not_restart_hdcp = 0;
		HDMITX_INFO("special mode switch, not start hdcp\n");
	} else {
		/*
		 * below is only for tmds mode, for FRL mode
		 * hdcp is started after training passed
		 */
		if (hdev->frl_rate == FRL_NONE) {
			if (get_hdcp2_lstore())
				hdev->dw_hdcp22_cap = is_rx_hdcp2ver();
			/*
			 * 0: for hdmitx driver control hdcp
			 * 1/2: drm driver or app control hdcp
			 */
			if (tx_comm->hdcp_ctl_lvl == 0)
				schedule_delayed_work(&hdev->work_start_hdcp, HZ / 4);
		}
	}

	vinfo = get_current_vinfo();
	if (vinfo) {
		vinfo->cur_enc_ppc = 1;
		if (hdev->frl_rate > FRL_NONE)
			vinfo->cur_enc_ppc = 4;
#ifdef CONFIG_AMLOGIC_DSC
		if (hdev->dsc_en) {
			if (para->cs == HDMI_COLORSPACE_RGB)
				vinfo->vpp_post_out_color_fmt = 1;
			else
				vinfo->vpp_post_out_color_fmt = 0;
		} else {
			vinfo->vpp_post_out_color_fmt = 0;
		}
#endif
		HDMITX_INFO("vinfo: set cur_enc_ppc as %d, vpp color: %d\n",
			vinfo->cur_enc_ppc, vinfo->vpp_post_out_color_fmt);
	}

	return 0;
}

static int hdmitx21_disable_mode(struct hdmitx_common *tx_comm, struct hdmi_format_para *para)
{
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	hdmitx_unregister_vrr(hdev);
#endif
	return 0;
}

static int hdmitx21_init_uboot_mode(enum vmode_e mode)
{
	if (!(mode & VMODE_INIT_BIT_MASK))
		HDMITX_ERROR("warning, echo /sys/class/display/mode is disabled\n");
	else
		HDMITX_INFO("already display in uboot\n");

	return 0;
}

static struct hdmitx_ctrl_ops tx21_ctrl_ops = {
	.pre_enable_mode = hdmitx21_pre_enable_mode,
	.enable_mode = hdmitx21_enable_mode,
	.post_enable_mode = hdmitx21_post_enable_mode,
	.disable_mode = hdmitx21_disable_mode,
	.init_uboot_mode = hdmitx21_init_uboot_mode,
	.disable_hdcp = hdmitx21_ops_disable_hdcp,
	.clear_pkt = hdmitx21_clear_packets,
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
	.disable_21_work = hdmitx21_disable_21_work,
	.disable_frl_work = hdmitx21_disable_frl_work,
#endif
};

#if IS_ENABLED(CONFIG_AMLOGIC_SND_SOC)

static int hdmitx_notify_callback_a(struct notifier_block *block,
				    unsigned long cmd, void *para);
static struct notifier_block hdmitx_notifier_nb_a = {
	.notifier_call	= hdmitx_notify_callback_a,
};

static int hdmitx_notify_callback_a(struct notifier_block *block,
				    unsigned long cmd, void *para)
{
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	hdmitx_audio_notify_callback(&hdev->tx_comm, &hdev->tx_hw.base, block, cmd, para);
	return 0;
}
#endif

static void hdmitx_rxsense_process(struct work_struct *work)
{
	int sense;
	struct hdmitx_common *tx_comm = container_of((struct delayed_work *)work,
		struct hdmitx_common, work_rxsense);
	struct hdmitx_dev *hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);

	sense = hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_TMDS_RXSENSE, 0);
	hdmitx21_set_uevent(HDMITX_RXSENSE_EVENT, sense);
	queue_delayed_work(tx_comm->rxsense_wq, &tx_comm->work_rxsense, HZ);
}

static void hdmitx_cedst_process(struct work_struct *work)
{
	int ced;
	struct hdmitx_common *tx_comm = container_of((struct delayed_work *)work,
		struct hdmitx_common, work_cedst);
	struct hdmitx_dev *hdev = to_hdmitx21_dev(tx_comm);

	ced = hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_TMDS_CEDST, 0);
	/* firstly send as 0, then real ced, A trigger signal */
	hdmitx21_set_uevent(HDMITX_CEDST_EVENT, 0);
	hdmitx21_set_uevent(HDMITX_CEDST_EVENT, ced);
	queue_delayed_work(tx_comm->cedst_wq, &tx_comm->work_cedst, HZ);
}

static void hdmitx_bootup_process_plugin(struct hdmitx_dev *hdev, bool set_audio)
{
	struct vinfo_s *info = NULL;

	/* step1: SW: EDID read/parse, notify client modules */
	hdmitx_bootup_plugin_work(&hdev->tx_comm);

	/* During the kernel startup process, the HDR/DV module will use
	 * vinfo information, it needs to attach vinfo after the EDID is
	 * parsed and before the HDR/DV module is enabled.
	 * so do as hdmitx_common_post_enable_mode()
	 */
	hdmitx21_bootup_update_vinfo(hdev);

	/* only bootup plugin will set audio mode, other plugin will not do that */
	if (set_audio) {
		info = hdmitx_get_current_vinfo(NULL);
		if (info && info->mode == VMODE_HDMI)
			hdmitx21_set_audio(hdev, &hdev->tx_comm.cur_audio_param);
	}

	/* step2: SW: notify client modules and update uevent state */
	hdmitx_bootup_notify_hpd_status(&hdev->tx_comm, false);
}

static void hdmitx_process_plugin(struct hdmitx_dev *hdev)
{
	struct hdmitx_common *tx_comm = &hdev->tx_comm;
	int i;
	unsigned char cta_block_count;
	unsigned char *edid_buf = tx_comm->EDID_buf;
	unsigned char edid_check = 0;
	unsigned long flags = 0;

	/* step1: SW: EDID read */
	hdmitx_plugin_common_work(tx_comm);

	/* step2: update cec phy addr and audio data block */
	spin_lock_irqsave(&tx_comm->edid_spinlock, flags);
	hdmitx_edid_rxcap_clear(&tx_comm->rxcap);
	/*
	 * hdmitx edid parsing debug function, parsed in drm by default
	 *
	 * Enable edid parse in hdmitx debug function command
	 * echo edid_parse1 > /sys/class/amhdmitx/amhdmitx0/debug
	 *
	 * Disable edid parse in hdmitx debug function command
	 * echo edid_parse0 > /sys/class/amhdmitx/amhdmitx0/debug
	 */
	if (tx_comm->edid_parse_in_hdmitx) {
		HDMITX_INFO("edid parse in hdmitx\n");
		hdmitx_edid_parse(&tx_comm->rxcap, tx_comm->EDID_buf);
		hdmitx_common_edid_tracer_post_proc(tx_comm, &tx_comm->rxcap);
		/* update the hdr/hdr10+/dv capabilities in the end of parse */
		hdmitx_set_hdr_priority(tx_comm, tx_comm->hdr_priority);
		hdmitx_common_notify_ced_status(tx_comm);
	}

	hdmitx_cec_phy_addr_parse(&tx_comm->rxcap.vsdb_phy_addr, tx_comm->EDID_buf);
	edid_check = tx_comm->rxcap.edid_check;
	cta_block_count = hdmitx_edid_get_cta_block_count(edid_buf);
	for (i = 1; i <= cta_block_count; i++) {
		if (edid_buf[i * 0x80] == 0x02 || edid_check & 0x01)
			hdmitx_edid_audio_block_parse(&tx_comm->rxcap, edid_buf);
	}
	spin_unlock_irqrestore(&tx_comm->edid_spinlock, flags);

	/* step3: SW: notify client modules and update uevent state */
	hdmitx_common_notify_hpd_status(tx_comm, false);
}

/*
 * action which is done in lock, it copy the flow of plugin handler.
 * only set audio if it's already enable in uboot, only check edid
 * if hdmitx output is enabled under uboot.
 * uboot_output_state is indicated in ready flag, can be replaced by
 * HW state later
 */
static void hdmitx_bootup_plugin_handler(struct hdmitx_dev *hdev)
{
	/* if current mode is TMDS/nonFRL, then resend_div40 */
	/* can also use if (hdev->frl_rate == FRL_NONE) */
	if (hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_GET_FRL_MODE, 0) == FRL_NONE) {
		if (hdev->tx_comm.fmt_para.tmds_clk_div40)
			hdmitx_hw_cntl_ddc(&hdev->tx_hw.base, DDC_SCDC_DIV40_SCRAMB, 1);
	} else {
		if (!is_frl_ready(hdev))
			hdev->tx_comm.ready = 0;
	}
	hdmitx_bootup_process_plugin(hdev, hdev->tx_comm.ready);
}

static void hdmitx_hpd_plugin_irq_handler(struct work_struct *work)
{
	/* struct vinfo_s *info = NULL; */
	struct hdmitx_dev *hdev = container_of((struct delayed_work *)work,
		struct hdmitx_dev, work_hpd_plugin);

	mutex_lock(&hdev->tx_comm.hdmimode_mutex);

	/*
	 * this may happen when just queue plugin work,
	 * but plugout event happen at this time. no need
	 * to continue plugin work.
	 */
	if (hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_HPD_GPI_ST, 0) == 0) {
		HDMITX_INFO(SYS "plug out event come when plugin handle, abort plugin handle\n");
		mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
		return;
	}
	/*
	 * only happen in such case:
	 * hpd high when suspend->plugout->plugin->late resume, the
	 * last plugin/resume flow sequence is unknown, will do
	 * plugin handler only once
	 */
	if (hdev->tx_comm.last_hpd_handle_done_stat == HDMI_TX_HPD_PLUGIN) {
		HDMITX_INFO(SYS "warning: continuous plugin, should not happen!\n");
		mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
		return;
	}
	HDMITX_INFO(SYS "hpd_high\n");
	hdmitx_process_plugin(hdev);

	mutex_unlock(&hdev->tx_comm.hdmimode_mutex);

	/* notify to drm hdmi */
	hdmitx_fire_drm_hpd_cb_unlocked(&hdev->tx_comm);
}

/* common work for plugout flow, which should be done in lock */
static void hdmitx_process_plugout(struct hdmitx_dev *hdev)
{
	hdmitx_plugout_common_work(&hdev->tx_comm);
	hdmitx21_reset_hdcp_param(&hdev->tx_comm);
	/* for vsync loss when HPD loss */
	hdmitx21_vid_pll_clk_check(hdev);
	/*
	 * after suspend, hdcp auth state(including topo info) should
	 * keep not changed, thus that encrypted video stream can
	 * recover playing normally after resume, specially for hdcp
	 * repeater case
	 */
	if (!hdev->tx_comm.suspend_flag)
		hdmitx_hw_cntl_ddc(&hdev->tx_hw.base, DDC_HDCP_SET_TOPO_INFO, 0);
	/*
	 * Reset the ll_enabled_in_auto_mode flag used for auto mode
	 * status. If we are in auto mode, gaming signal should be enabled
	 * when the request arrives again from the input device or playback
	 * and not on hotplug.
	 */
	hdev->tx_comm.ll_enabled_in_auto_mode = false;
	/* SW: notify event to user space and other modules */
	hdmitx_common_notify_hpd_status(&hdev->tx_comm, false);
}

static void hdmitx_bootup_plugout_handler(struct hdmitx_dev *hdev)
{
	hdmitx_process_plugout(hdev);
}

static void hdmitx_hpd_plugout_irq_handler(struct work_struct *work)
{
	struct hdmitx_dev *hdev = container_of((struct delayed_work *)work,
		struct hdmitx_dev, work_hpd_plugout);

	mutex_lock(&hdev->tx_comm.hdmimode_mutex);
	if (hdev->tx_comm.last_hpd_handle_done_stat == HDMI_TX_HPD_PLUGOUT) {
		HDMITX_INFO(SYS "continuous plugout handler, ignore\n");
		mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
		return;
	}
	hdmitx_process_plugout(hdev);
	mutex_unlock(&hdev->tx_comm.hdmimode_mutex);

	/* notify to drm hdmi */
	hdmitx_fire_drm_hpd_cb_unlocked(&hdev->tx_comm);
}

int get21_hpd_state(void)
{
	int ret;
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	mutex_lock(&hdev->tx_comm.hdmimode_mutex);
	ret = hdev->tx_comm.hpd_state;
	mutex_unlock(&hdev->tx_comm.hdmimode_mutex);

	return ret;
}

/*****************************
 *	hdmitx driver file_operations
 *
 ******************************/
static int amhdmitx_open(struct inode *node, struct file *file)
{
	struct hdmitx_dev *hdmitx_in_devp;

	/* Get the per-device structure that contains this cdev */
	hdmitx_in_devp = container_of(node->i_cdev, struct hdmitx_dev, cdev);
	file->private_data = hdmitx_in_devp;

	return 0;
}

static int amhdmitx_release(struct inode *node, struct file *file)
{
	return 0;
}

static const struct file_operations amhdmitx_fops = {
	.owner	= THIS_MODULE,
	.open	 = amhdmitx_open,
	.release = amhdmitx_release,
};

static int get_dt_vend_init_data(struct device_node *np,
				 struct vendor_info_data *vend)
{
	int ret;

	ret = of_property_read_string(np, "vendor_name",
				      (const char **)&vend->vendor_name);
	if (ret)
		HDMITX_INFO("not find vendor name\n");

	ret = of_property_read_u32(np, "vendor_id", &vend->vendor_id);
	if (ret)
		HDMITX_INFO("not find vendor id\n");

	ret = of_property_read_string(np, "product_desc",
				      (const char **)&vend->product_desc);
	if (ret)
		HDMITX_INFO("not find product desc\n");
	return 0;
}

/* for notify to cec/rx */
int hdmitx21_event_notifier_regist(struct notifier_block *nb)
{
	int ret = 0;
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	if (!nb)
		return ret;

	ret = hdmitx_event_mgr_notifier_register(hdev->tx_comm.event_mgr,
		(struct hdmitx_notifier_client *)nb);

	/* update status when register */
	if (!ret && nb->notifier_call) {
		/* if (hdev->tx_comm.hdmi_repeater == 1) */
		hdmitx_notify_hpd(hdev->tx_comm.hpd_state,
			hdev->tx_comm.rxcap.edid_parsing ?
			hdev->tx_comm.EDID_buf : NULL);
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

int hdmitx21_event_notifier_unregist(struct notifier_block *nb)
{
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	return hdmitx_event_mgr_notifier_unregister(hdev->tx_comm.event_mgr,
		(struct hdmitx_notifier_client *)nb);
}

void hdmitx21_hdcp_status(int hdmi_authenticated)
{
	hdmitx21_set_uevent(HDMITX_HDCP_EVENT, hdmi_authenticated);
}

static int amhdmitx21_device_init(struct hdmitx_dev *hdev)
{
	const char *hdmi_ver = NULL;

	if (!hdev)
		return 1;

	GET_HDMITX_VER(hdmi_ver);
	HDMITX_INFO("Ver: %s\n", hdmi_ver);

	hdev->hdtx_dev = NULL;

	hdev->tx_comm.ready = 0;
	hdev->tx_comm.hdcp_user = 0;
	hdev->tx_comm.hdr_mute_frame = 20;
	/* no RxSense by default */
	hdev->tx_comm.rxsense_policy = 0;
	/* enable or disable HDMITX SSPLL, enable by default */
	hdev->tx_comm.sspll = 1;
	/*
	 * 0, do not unmux hpd when off or unplug ;
	 * 1, unmux hpd when unplug;
	 * 2, unmux hpd when unplug  or off;
	 */
	hdev->hpdmode = 1;
	hdev->tx_comm.vid_mute_op = VIDEO_NONE_OP;

	hdev->tx_comm.flag_3dfp = 0;
	hdev->tx_comm.flag_3dss = 0;
	hdev->tx_comm.flag_3dtb = 0;
	hdev->tx_comm.def_stream_type = DEFAULT_STREAM_TYPE;

	/* default audio configure is on */
	hdev->tx_comm.cur_audio_param.aud_output_en = 1;
	hdev->tx_comm.need_filter_hdcp_off = false;
	/* default 6S */
	hdev->tx_comm.filter_hdcp_off_period = 6;
	hdev->tx_comm.not_restart_hdcp = false;
	/* wait for upstream start hdcp auth 5S */
	hdev->up_hdcp_timeout_sec = 5;
	hdev->tx_comm.ctrl_ops = &tx21_ctrl_ops;
	hdev->tx_comm.vdev = &hdmitx_vdev;
	set_dummy_dv_info(&hdmitx_vdev);
	/* ll mode init values */
	hdev->tx_comm.ll_enabled_in_auto_mode = false;
	hdev->tx_comm.ll_user_set_mode = HDMI_LL_MODE_AUTO;
	/* vrr function init*/
	hdev->tx_comm.vrr_dbg_vframe = -1;

	hdev->tx_comm.dfm_type = -1;
	hdev->tx_comm.csc_type = 1;
	return 0;
}

static int hdmitx_get_connector(void)
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

static void hdmitx_init_gate_bit_mask(struct hdmitx_dev *hdev)
{
	if (!hdev)
		return;
	switch (hdev->tx_hw.chip_data->chip_type) {
	case MESON_CPU_ID_S5:
		hdev->tx_hw.gate_bit_mask = 0xffc7f;
		break;
	case MESON_CPU_ID_S6:
		hdev->tx_hw.gate_bit_mask = 0x01c7f;
		break;
	case MESON_CPU_ID_S7:
		hdev->tx_hw.gate_bit_mask = 0x01c7f;
		break;
	case MESON_CPU_ID_S7D:
		hdev->tx_hw.gate_bit_mask = 0x01c7f;
		break;
	default:
		hdev->tx_hw.gate_bit_mask = 0;
		break;
	}
}

static int amhdmitx_get_dt_info(struct platform_device *pdev, struct hdmitx_dev *hdev)
{
	int ret = 0;
	struct pinctrl *pin;

#ifdef CONFIG_OF
	int val;
	phandle phandler;
	struct device_node *init_data;
	const struct of_device_id *match;
#endif
	u32 refreshrate_limit = 0;
	struct hdmitx_hw_common *tx_hw_base;

	match = of_match_device(meson_amhdmitx_of_match, &pdev->dev);
	if (!match) {
		HDMITX_INFO("unable to get matched device\n");
		return -1;
	}
	HDMITX_INFO("get matched device\n");

	/* pinmux set */
	if (pdev->dev.of_node) {
		pin = devm_pinctrl_get(&pdev->dev);
		if (!pin) {
			HDMITX_INFO("get pin control fail\n");
			return -1;
		}

		hdev->pinctrl_default = pinctrl_lookup_state(pin, "hdmitx_hpd");
		pinctrl_select_state(pin, hdev->pinctrl_default);

		hdev->pinctrl_i2c = pinctrl_lookup_state(pin, "hdmitx_ddc");
		pinctrl_select_state(pin, hdev->pinctrl_i2c);
		/* rx_pr("hdmirx: pinmux:%p, name:%s\n", */
		/* pin, pin_name); */
		HDMITX_INFO("get pin control\n");

		/* rx_pr("hdmirx: pinmux:%p, name:%s\n", */
		/* pin, pin_name); */
	} else {
		HDMITX_INFO("node null\n");
	}

	tx_hw_base = &hdev->tx_hw.base;
	tx_hw_base->hdmitx_gpios_hpd = of_get_named_gpio(pdev->dev.of_node,
		"hdmitx-gpios-hpd", 0);
	if (tx_hw_base->hdmitx_gpios_hpd < 0)
		HDMITX_ERROR("get hdmitx-gpios-hpd error\n");
	tx_hw_base->hdmitx_gpios_scl = of_get_named_gpio(pdev->dev.of_node,
		"hdmitx-gpios-scl", 0);
	if (tx_hw_base->hdmitx_gpios_scl < 0)
		HDMITX_ERROR("get hdmitx-gpios-scl error\n");
	tx_hw_base->hdmitx_gpios_sda = of_get_named_gpio(pdev->dev.of_node,
		"hdmitx-gpios-sda", 0);
	if (tx_hw_base->hdmitx_gpios_sda < 0)
		HDMITX_ERROR("get hdmitx-gpios-sda error\n");

#ifdef CONFIG_OF
	if (pdev->dev.of_node) {
		int dongle_mode = 0;
		int pxp_mode = 0;

		memset(&hdev->config_data, 0,
		       sizeof(struct hdmi_config_platform_data));
		/* Get chip type and name information */
		match = of_match_device(meson_amhdmitx_of_match, &pdev->dev);

		if (!match) {
			HDMITX_INFO("%s: no match table\n", __func__);
			return -1;
		}
		hdev->tx_hw.chip_data = (struct amhdmitx_data_s *)match->data;

		HDMITX_INFO("chip_type:%d chip_name:%s\n",
			hdev->tx_hw.chip_data->chip_type,
			hdev->tx_hw.chip_data->chip_name);

		if (hdev->tx_hw.chip_data->chip_type == MESON_CPU_ID_S5)
			hdev->tx_hw.base.hdmi_tx_cap.dsc_capable = true;
		else
			hdev->tx_hw.base.hdmi_tx_cap.dsc_capable = false;
		hdmitx_init_gate_bit_mask(hdev);
		/* Get pxp_mode information */
		ret = of_property_read_u32(pdev->dev.of_node, "pxp_mode",
					   &pxp_mode);
		hdev->pxp_mode = pxp_mode;
		if (!ret)
			HDMITX_INFO("hdev->pxp_mode: %d\n", hdev->pxp_mode);

		/* Get dongle_mode information */
		ret = of_property_read_u32(pdev->dev.of_node, "dongle_mode",
					   &dongle_mode);
		hdev->tx_hw.dongle_mode = !!dongle_mode;
		if (!ret)
			HDMITX_INFO("hdev->dongle_mode: %d\n", hdev->tx_hw.dongle_mode);
		/* Get res_1080p information */
		ret = of_property_read_u32(pdev->dev.of_node, "res_1080p",
			&hdev->tx_comm.res_1080p);
		hdev->tx_comm.res_1080p = !!hdev->tx_comm.res_1080p;
		/*
		 * the max tmds cap is 600Mhz by default,
		 * if soc limit to 1080p maximum, then the
		 * max tmds cap is 225Mhz
		 */
		if (hdev->tx_comm.res_1080p)
			hdev->tx_comm.tx_hw->hdmi_tx_cap.tx_max_tmds_clk = 225;
		else
			hdev->tx_comm.tx_hw->hdmi_tx_cap.tx_max_tmds_clk = 600;

		ret = of_property_read_u32(pdev->dev.of_node, "max_refreshrate",
					   &refreshrate_limit);
		if (ret == 0 && refreshrate_limit > 0)
			hdev->tx_comm.max_refreshrate = refreshrate_limit;

		/* Get repeater_tx information */
		ret = of_property_read_u32(pdev->dev.of_node,
					   "repeater_tx", &val);
		if (!ret)
			hdev->tx_hw.base.hdcp_repeater_en = val;
		else
			hdev->tx_hw.base.hdcp_repeater_en = 0;

		/* Get repeater_tx information */
		ret = of_property_read_u32(pdev->dev.of_node,
					   "hdmi_repeater", &val);
		if (!ret)
			hdev->tx_comm.hdmi_repeater = val;
		else
			hdev->tx_comm.hdmi_repeater = 1;
		/* if it's not hdmi repeater, then should not support hdcp repeater */
		if (hdev->tx_comm.hdmi_repeater == 0)
			hdev->tx_hw.base.hdcp_repeater_en = 0;

		ret = of_property_read_u32(pdev->dev.of_node,
					   "cedst_en", &val);
		if (!ret)
			hdev->tx_comm.cedst_en = !!val;
		ret = of_property_read_u32(pdev->dev.of_node, "hdr_8bit_en", &val);
		if (!ret)
			hdev->tx_comm.hdr_8bit_en = !!val;
		/* not support FRL by default, unless enabled in dts */
		hdev->tx_comm.tx_hw->hdmi_tx_cap.tx_max_frl_rate = FRL_NONE;
		ret = of_property_read_u32(pdev->dev.of_node, "tx_max_frl_rate", &val);
		if (!ret) {
			if (val > FRL_12G4L)
				HDMITX_INFO("wrong tx_max_frl_rate %d\n", val);
			else
				hdev->tx_comm.tx_hw->hdmi_tx_cap.tx_max_frl_rate = val;
		}
		ret = of_property_read_u32(pdev->dev.of_node,
					   "hdcp_type_policy", &val);

		hdev->tx_comm.enc_idx = hdmitx_get_connector();
		HDMITX_INFO("enc_idx %d\n", hdev->tx_comm.enc_idx);

		/* hdcp ctrl 0:sysctrl, 1: drv, 2: linux app */
		ret = of_property_read_u32(pdev->dev.of_node,
			   "hdcp_ctl_lvl", &hdev->tx_comm.hdcp_ctl_lvl);
		HDMITX_INFO("hdcp_ctl_lvl[%d-%d]\n", hdev->tx_comm.hdcp_ctl_lvl, ret);

		if (ret)
			hdev->tx_comm.hdcp_ctl_lvl = 0;

		/* Get vendor information */
		ret = of_property_read_u32(pdev->dev.of_node,
					   "vend-data", &val);
		if (ret)
			HDMITX_INFO("not find match init-data\n");
		if (ret == 0) {
			phandler = val;
			init_data = of_find_node_by_phandle(phandler);
			if (!init_data)
				HDMITX_INFO("not find device node\n");
			hdev->config_data.vend_data =
			kzalloc(sizeof(struct vendor_info_data), GFP_KERNEL);
			if (!(hdev->config_data.vend_data))
				HDMITX_INFO("not allocate memory\n");
			ret = get_dt_vend_init_data
			(init_data, hdev->config_data.vend_data);
			if (ret)
				HDMITX_INFO("not find vend_init_data\n");
		}
		/* Get power control */
		ret = of_property_read_u32(pdev->dev.of_node,
					   "pwr-ctrl", &val);
		if (ret)
			HDMITX_INFO("not find match pwr-ctl\n");
		if (ret == 0) {
			phandler = val;
			init_data = of_find_node_by_phandle(phandler);
			if (!init_data)
				HDMITX_INFO("not find device node\n");
			hdev->config_data.pwr_ctl = kzalloc((sizeof(struct hdmi_pwr_ctl)) *
				HDMI_TX_PWR_CTRL_NUM, GFP_KERNEL);
			if (!hdev->config_data.pwr_ctl)
				HDMITX_INFO("can not get pwr_ctl mem\n");
			else
				memset(hdev->config_data.pwr_ctl, 0, sizeof(struct hdmi_pwr_ctl));
			if (ret)
				HDMITX_INFO("not find pwr_ctl\n");
		}

		/* Get reg information */
		ret = hdmitx21_init_reg_map(pdev);
		if (ret < 0)
			HDMITX_ERROR("ERROR: hdmitx io_remap fail!\n");
	}

#else
		hdmi_pdata = pdev->dev.platform_data;
		if (!hdmi_pdata) {
			HDMITX_INFO("not get platform data\n");
			r = -ENOENT;
		} else {
			HDMITX_INFO("get hdmi platform data\n");
		}
#endif
	hdev->irq_hpd = platform_get_irq_byname(pdev, "hdmitx_hpd");
	if (hdev->irq_hpd == -ENXIO) {
		HDMITX_ERROR("%s: ERROR: hdmitx hpd irq No not found\n",
		       __func__);
			return -ENXIO;
	}
	HDMITX_INFO("hpd irq = %d\n", hdev->irq_hpd);
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
	tx_vrr_params_init();
	hdev->irq_vrr_vsync = platform_get_irq_byname(pdev, "vrr_vsync");
	if (hdev->irq_vrr_vsync == -ENXIO) {
		HDMITX_ERROR("%s: ERROR: hdmitx vrr_vsync irq No not found\n",
		       __func__);
			return -ENXIO;
	}
	HDMITX_INFO("vrr vsync irq = %d\n", hdev->irq_vrr_vsync);
#endif
	ret = of_property_read_u32(pdev->dev.of_node, "arc_rx_en", &val);
	if (!ret)
		hdev->arc_rx_en = val;
	else
		hdev->arc_rx_en = 0;
	return ret;
}

/*
 * amhdmitx_clktree_probe
 * get clktree info from dts
 */
static void amhdmitx_clktree_probe(struct device *hdmitx_dev, struct hdmitx_dev *hdev)
{
	struct clk *hdmi_clk_vapb, *hdmi_clk_vpu;
	struct clk *venci_top_gate, *venci_0_gate, *venci_1_gate;

	hdmi_clk_vapb = devm_clk_get(hdmitx_dev, "hdmi_vapb_clk");
	if (!IS_ERR(hdmi_clk_vapb)) {
		hdev->hdmitx_clk_tree.hdmi_clk_vapb = hdmi_clk_vapb;
		clk_prepare_enable(hdev->hdmitx_clk_tree.hdmi_clk_vapb);
	}

	hdmi_clk_vpu = devm_clk_get(hdmitx_dev, "hdmi_vpu_clk");
	if (!IS_ERR(hdmi_clk_vpu)) {
		hdev->hdmitx_clk_tree.hdmi_clk_vpu = hdmi_clk_vpu;
		clk_prepare_enable(hdev->hdmitx_clk_tree.hdmi_clk_vpu);
	}

	venci_top_gate = devm_clk_get(hdmitx_dev, "venci_top_gate");
	if (!IS_ERR(venci_top_gate))
		hdev->hdmitx_clk_tree.venci_top_gate = venci_top_gate;

	venci_0_gate = devm_clk_get(hdmitx_dev, "venci_0_gate");
	if (!IS_ERR(venci_0_gate))
		hdev->hdmitx_clk_tree.venci_0_gate = venci_0_gate;

	venci_1_gate = devm_clk_get(hdmitx_dev, "venci_1_gate");
	if (!IS_ERR(venci_1_gate))
		hdev->hdmitx_clk_tree.venci_1_gate = venci_1_gate;
}

void amhdmitx21_vpu_dev_register(struct hdmitx_dev *hdev)
{
	hdev->hdmitx_vpu_clk_gate_dev =
	vpu_dev_register(VPU_VENCI, DEVICE_NAME);
}

static void amhdmitx_infoframe_init(struct hdmitx_dev *hdev)
{
	int ret = 0;

	ret = hdmi_vendor_infoframe_init(&hdev->tx_comm.infoframes.vend.vendor.hdmi);
	if (ret)
		HDMITX_INFO("%s[%d] init vendor infoframe failed %d\n", __func__, __LINE__, ret);
	hdmi_avi_infoframe_init(&hdev->tx_comm.infoframes.avi.avi);

	// TODO, panic
	// hdmi_spd_infoframe_init(&hdev->infoframes.spd.spd,
	//	hdev->config_data.vend_data->vendor_name,
	//	hdev->config_data.vend_data->product_desc);
	hdmi_audio_infoframe_init(&hdev->tx_comm.infoframes.aud.audio);
	hdmi_drm_infoframe_init(&hdev->tx_comm.infoframes.drm.drm);
}

/* used for status check when hdmi output setting done */
static int hdmitx21_status_check(void *data)
{
	int clk[3];
	int idx[3];
	struct hdmitx_dev *hdev = get_hdmitx21_device();

	if (hdev->tx_hw.chip_data->chip_type != MESON_CPU_ID_S5)
		return 0;

	/* for S5, here need check the clk index 89 & 16 */
	/* cts_htx_tmds_clk */
	idx[0] = 92;
	/* vid_pll0_clk */
	idx[1] = 16;
	/* htx_tmds20_clk */
	idx[2] = 89;

	while (1) {
		msleep_interruptible(1000);
		if (!hdev->tx_comm.ready)
			continue;
		/* skip FRL mode */
		if (hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_GET_FRL_MODE, 0))
			continue;
		clk[0] = meson_clk_measure(idx[0]);
		clk[1] = meson_clk_measure(idx[1]);
		if (clk[0] && clk[1])
			continue;

		if (!clk[0]) {
			HDMITX_DEBUG("the clock[%d] is %d\n", idx[0], clk[0]);
			hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_CLK_DIV_RST, idx[0]);
			HDMITX_DEBUG("reset the clock div for %d\n", idx[0]);
			HDMITX_INFO("the clock[%d] is %d\n", idx[0], meson_clk_measure(idx[0]));
		}
		if (!clk[1]) {
			HDMITX_DEBUG("the clock[%d] is %d\n", idx[1], clk[1]);
			hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_CLK_DIV_RST, idx[1]);
			HDMITX_DEBUG("reset the clock div for %d\n", idx[1]);
			HDMITX_INFO("the clock[%d] is %d\n", idx[1], meson_clk_measure(idx[1]));
		}
		/* resend the SCDC/DIV40 config */
		if (!clk[0] || !clk[1]) {
			clk[0] = meson_clk_measure(idx[0]);
			if (clk[0] >= 340000000)
				hdmitx_hw_cntl_ddc(&hdev->tx_hw.base,
					DDC_SCDC_DIV40_SCRAMB, 1);
			else
				hdmitx_hw_cntl_ddc(&hdev->tx_hw.base,
					DDC_SCDC_DIV40_SCRAMB, 0);
		}
	}
	return 0;
}

/* check clk status when plug out in case no vsync */
static void hdmitx21_vid_pll_clk_check(struct hdmitx_dev *hdev)
{
	int clk[3];
	int idx[3];

	if (hdev->tx_hw.chip_data->chip_type != MESON_CPU_ID_S5)
		return;
	/*
	 * frl mode use fpll or gp2 pll, and won't go through
	 * vid_clk0_div_top/tmds20_clk_div_top, no need to check.
	 */
	if (hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_GET_FRL_MODE, 0))
		return;

	/* for S5, here need check the clk index 89 & 16 */
	/* cts_htx_tmds_clk */
	idx[0] = 92;
	/* vid_pll0_clk */
	idx[1] = 16;
	/* htx_tmds20_clk */
	idx[2] = 89;

	clk[0] = meson_clk_measure(idx[0]);
	clk[1] = meson_clk_measure(idx[1]);
	if (clk[0] && clk[1])
		return;

	if (!clk[0]) {
		HDMITX_DEBUG("%s the clock[%d] is %d\n", __func__, idx[0], clk[0]);
		hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_CLK_DIV_RST, idx[0]);
		HDMITX_INFO("after reset the clock[%d] is %d\n", idx[0], meson_clk_measure(idx[0]));
	}
	if (!clk[1]) {
		HDMITX_DEBUG("%s the clock[%d] is %d\n",  __func__, idx[1], clk[1]);
		hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_CLK_DIV_RST, idx[1]);
		HDMITX_INFO("after reset the clock[%d] is %d\n", idx[1], meson_clk_measure(idx[1]));
	}
}

static int amhdmitx_probe(struct platform_device *pdev)
{
	int r, ret = 0;
	struct device *device = &pdev->dev;
	struct device *dev;

	struct hdmitx_dev *hdev;
	struct hdmitx_common *tx_comm;
	struct hdmitx_tracer *tx_tracer;
	struct hdmitx_event_mgr *tx_event_mgr;
	bool hpd_state;

	HDMITX_INFO("amhdmitx_probe_start\n");

	hdev = devm_kzalloc(device, sizeof(*hdev), GFP_KERNEL);
	if (!hdev)
		return -ENOMEM;

	tx21_dev = hdev;
	dev_set_drvdata(device, hdev);
	tx_comm = &hdev->tx_comm;

	amhdmitx21_device_init(hdev);
	amhdmitx_infoframe_init(hdev);

	/*init txcommon*/
	hdmitx_common_init(tx_comm, &hdev->tx_hw.base);

	ret = amhdmitx_get_dt_info(pdev, hdev);
	/* if (ret) */
		/* return ret; */

	amhdmitx_clktree_probe(&pdev->dev, hdev);
	if (0)
		/* TODO */
		amhdmitx21_vpu_dev_register(hdev);

	r = alloc_chrdev_region(&hdev->hdmitx_id, 0, HDMI_TX_COUNT,
				DEVICE_NAME);
	cdev_init(&hdev->cdev, &amhdmitx_fops);
	hdev->cdev.owner = THIS_MODULE;
	r = cdev_add(&hdev->cdev, hdev->hdmitx_id, HDMI_TX_COUNT);

	hdmitx_class = class_create("amhdmitx");
	if (IS_ERR(hdmitx_class)) {
		unregister_chrdev_region(hdev->hdmitx_id, HDMI_TX_COUNT);
		return -1;
	}

	/* kernel>=2.6.27 */
	dev = device_create(hdmitx_class, NULL, hdev->hdmitx_id, hdev,
			    "amhdmitx%d", 0);

	if (!dev) {
		HDMITX_ERROR("device_create create error\n");
		class_destroy(hdmitx_class);
		r = -EEXIST;
		return r;
	}
	hdev->hdtx_dev = dev;
	ret = device_create_file(dev, &dev_attr_config);

	/*platform related functions*/
	tx_event_mgr = hdmitx_event_mgr_create(pdev, hdev->hdtx_dev);
	hdmitx_event_mgr_suspend(tx_event_mgr, false);
	hdmitx_common_attch_platform_data(tx_comm,
		HDMITX_PLATFORM_UEVENT, tx_event_mgr);
	tx_tracer = hdmitx_tracer_create(tx_event_mgr);
	hdmitx_common_attch_platform_data(tx_comm,
		HDMITX_PLATFORM_TRACER, tx_tracer);
	hdmitx_audio_register_ctrl_callback(tx_tracer, hdmitx21_ext_set_audio_output,
		hdmitx21_ext_get_audio_status);
#ifdef CONFIG_AMLOGIC_LEGACY_EARLY_SUSPEND
	hdmitx_early_suspend_handler.param = hdev;
	register_early_suspend(&hdmitx_early_suspend_handler);
#endif
	hdev->nb.notifier_call = hdmitx_reboot_notifier;
	register_reboot_notifier(&hdev->nb);

	/* init HW */
	hdmitx21_meson_init(hdev);
	hdmitx_vout_init(tx_comm, &hdev->tx_hw.base);
	hdmitx_hdr_init(tx_comm);
	hdmitx_packet_init(tx_comm);

#if IS_ENABLED(CONFIG_AMLOGIC_SND_SOC)
	if (!hdev->pxp_mode && hdmitx21_uboot_audio_en()) {
		struct aud_para *audpara = &hdev->tx_comm.cur_audio_param;

		audpara->rate = FS_48K;
		audpara->type = CT_PCM;
		audpara->size = SS_16BITS;
		audpara->chs = 2 - 1;
	}
	/* TODO: to confirm: default audio clock is ON */
	hdmitx21_audio_mute_op(1, 0);
	if (!hdev->pxp_mode)
		aout_register_client(&hdmitx_notifier_nb_a);
#endif

	/* get efuse ctrl state */
	get_hdmi_efuse(tx_comm);
	spin_lock_init(&hdev->tx_comm.edid_spinlock);
	INIT_WORK(&hdev->tx_comm.work_hdr, hdr_work_func);
	INIT_WORK(&hdev->tx_comm.work_hdr_unmute, hdr_unmute_work_func);
	hdev->hdmi_hpd_wq = alloc_ordered_workqueue(DEVICE_NAME,
		WQ_HIGHPRI | __WQ_LEGACY | WQ_MEM_RECLAIM);
	INIT_DELAYED_WORK(&hdev->work_hpd_plugin, hdmitx_hpd_plugin_irq_handler);
	INIT_DELAYED_WORK(&hdev->work_hpd_plugout, hdmitx_hpd_plugout_irq_handler);
	/* hdcp related work scheduled in system workqueue */
	INIT_DELAYED_WORK(&hdev->work_start_hdcp, hdmitx_start_hdcp_handler);
	INIT_DELAYED_WORK(&hdev->work_up_hdcp_timeout, hdmitx_up_hdcp_timeout_handler);
	/* for linux start hdcp */
	INIT_DELAYED_WORK(&hdev->work_drm_start_hdcp, drm_hdmitx_start_hdcp_handler);
	/* interrupt handler need to be scheduled in high priority */
	hdev->hdmi_intr_wq = alloc_workqueue("hdmitx_intr_wq", WQ_HIGHPRI | WQ_CPU_INTENSIVE, 0);
	INIT_DELAYED_WORK(&hdev->work_internal_intr, hdmitx_top_intr_handler);

	/* for rx sense feature */
	hdev->tx_comm.rxsense_wq = alloc_workqueue("hdmitx_rxsense",
					   WQ_SYSFS | WQ_FREEZABLE, 0);
	INIT_DELAYED_WORK(&hdev->tx_comm.work_rxsense, hdmitx_rxsense_process);
	/* for cedst feature */
	hdev->tx_comm.cedst_wq = alloc_workqueue("hdmitx_cedst",
					 WQ_SYSFS | WQ_FREEZABLE, 0);
	INIT_DELAYED_WORK(&hdev->tx_comm.work_cedst, hdmitx_cedst_process);

	set_hdcp_common_instance(&hdev->tx_comm);
	hdmitx21_hdcp_init();
	/* bind drm before hdmi event */
	hdmitx_hook_drm(&pdev->dev);

	/* init power_uevent state */
	hdmitx21_set_uevent(HDMITX_HDCPPWR_EVENT, HDMI_WAKEUP);
	/* reset EDID/vinfo */
	if (!hdev->tx_comm.forced_edid) {
		hdmitx_edid_buffer_clear(hdev->tx_comm.EDID_buf, sizeof(hdev->tx_comm.EDID_buf));
		hdmitx_edid_rxcap_clear(&hdev->tx_comm.rxcap);
	}
	/* hpd process of bootup stage */
	mutex_lock(&hdev->tx_comm.hdmimode_mutex);
	intr_status_init_clear();
	/* enable irq firstly before any hpd handler to prevent missing irq. */
	hdmitx_setupirqs(hdev);

	/* actions in top half of plug intr, do it before enable irq */
	hpd_state = !!hdmitx_hw_cntl_misc(&hdev->tx_hw.base,
		MISC_HPD_GPI_ST, 0);
	hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_HPD_IRQ_TOP_HALF, hpd_state);
	/* actions in bottom half of plug intr */
	if (hpd_state)
		hdmitx_bootup_plugin_handler(hdev);
	else
		hdmitx_bootup_plugout_handler(hdev);

	/*
	 * When Sink-led output, the Color Space read from AVI Packet is RGB,
	 * but the input to HDMITX is YUV444, so it is necessary to judge that
	 * the Color Space is configured to be YUV444 when the current output
	 * is Sink-led, this logic judgment needs to be executed after parsing
	 * the EDID, refer to the SWPL-180597 for details
	 *
	 * load fmt para from hw info
	 */
	hdmitx_common_init_bootup_format_para(tx_comm, &tx_comm->fmt_para);
	/* TODO: not consider VESA mode witch HW VIC = 0 */
	if (tx_comm->fmt_para.vic != HDMI_0_UNKNOWN)
		hdev->tx_comm.ready = 1;
	/*
	 * update fmt_attr string from fmt_para, note that fmt_attr is already
	 * set by hdmitx_common_init() with boot arg, and below is un-necessary,
	 * and it will set attr sysfs node as empty if hdmitx not enabled under
	 * uboot as fmt para is in reset state
	 */
	hdmitx_format_para_rebuild_fmtattr_str(&hdev->tx_comm.fmt_para,
		hdev->tx_comm.fmt_attr, sizeof(hdev->tx_comm.fmt_attr));
	/* load init hdr state for HW info */
	hdmitx_hdr_state_init(&hdev->tx_comm);

#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
	hdmitx_register_vrr(hdev);
	hdmitx_register_drm_vrr_api(hdev);
#endif

	/* after unlock, now can take actions of bottom half of hpd irq */
	mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
	/* notify to drm hdmi */
	hdmitx_fire_drm_hpd_cb_unlocked(&hdev->tx_comm);

	/* create misc device for communication with TEE: hdcp key load ready notify */
	tee_comm_dev_reg(hdev);

	hdev->task = kthread_run(hdmitx21_status_check, (void *)hdev,
				      "kthread_hdmist_check");
	hdev->tx_comm.hdmi_init = HDMITX21;
	/* everything is ready, create sysfs here */
	hdmitx_sysfs_common_create(dev, &hdev->tx_comm, &hdev->tx_hw.base);

	/* enable analog frequency division by default on S6 */
	if (hdev->tx_hw.chip_data->chip_type == MESON_CPU_ID_S6)
		hdev->tx_hw.s7_clk_config = 1;

	HDMITX_INFO("amhdmitx_probe_end\n");
	return r;
}

static void amhdmitx_remove(struct platform_device *pdev)
{
	struct hdmitx_dev *hdev = dev_get_drvdata(&pdev->dev);
	struct device *dev = hdev->hdtx_dev;

	/* remove sysfs before uninit */
	hdmitx_sysfs_common_destroy(dev);

	tee_comm_dev_unreg(hdev);
	/* unbind from drm */
	hdmitx_unhook_drm(&pdev->dev);

	cancel_work_sync(&hdev->tx_comm.work_hdr);
	cancel_work_sync(&hdev->tx_comm.work_hdr_unmute);
	hdmitx21_hdcp_exit();
	cancel_delayed_work(&hdev->work_internal_intr);
	destroy_workqueue(hdev->hdmi_intr_wq);
	cancel_delayed_work(&hdev->work_hpd_plugout);
	cancel_delayed_work(&hdev->work_hpd_plugin);
	destroy_workqueue(hdev->hdmi_hpd_wq);
	cancel_delayed_work(&hdev->tx_comm.work_rxsense);
	destroy_workqueue(hdev->tx_comm.rxsense_wq);
	cancel_delayed_work(&hdev->tx_comm.work_cedst);
	destroy_workqueue(hdev->tx_comm.cedst_wq);

	if (hdev->tx_hw.base.uninit)
		hdev->tx_hw.base.uninit(&hdev->tx_hw.base);
	hdev->hpd_event = 0xff;
	kthread_stop(hdev->task);
	hdmitx_vout_uninit();

#if IS_ENABLED(CONFIG_AMLOGIC_SND_SOC)
	aout_unregister_client(&hdmitx_notifier_nb_a);
#endif

	/* Remove the cdev */
	device_remove_file(dev, &dev_attr_config);
	cdev_del(&hdev->cdev);

	device_destroy(hdmitx_class, hdev->hdmitx_id);

	class_destroy(hdmitx_class);

	unregister_chrdev_region(hdev->hdmitx_id, HDMI_TX_COUNT);
	hdmitx_common_destroy(&hdev->tx_comm);
}

static void hdmitx_clk_ctrl(struct hdmitx_dev *hdev, bool en)
{
	if (!hdev)
		return;

	hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_HDMI_CLKS_CTRL, en);
}

static void amhdmitx_shutdown(struct platform_device *pdev)
{
	struct hdmitx_dev *hdev = dev_get_drvdata(&pdev->dev);

	if (hdev->tx_comm.aon_output) {
		hdmitx21_disable_hdcp(hdev);
		return;
	}
	hdmitx_clk_ctrl(hdev, 0);
}

/*
 * there's corner case:
 * when deep suspend, RTC wakeup kernel-->
 * hdmi plugout/in interrupt-->
 * plugin bottom handle, edid...
 * however it may re-enter RTC suspend and
 * disable hdmitx clk during hdmi register
 * access in plugin bottom handler, cause
 * system hard lock and crash. so need to keep
 * basic hdmitx clk enabled when suspend
 */
#ifdef CONFIG_PM
static int amhdmitx_suspend(struct platform_device *pdev,
			    pm_message_t state)
{
	struct hdmitx_dev *hdev = dev_get_drvdata(&pdev->dev);
	struct hdmitx_common *tx_comm = &hdev->tx_comm;
	struct hdcp_t *p_hdcp = (struct hdcp_t *)hdev->am_hdcp;

	hdmitx_clk_ctrl(hdev, 0);
	/*
	 * after suspend, VPU power domain will be powered off,
	 * so hdcp1.4 key otp/crc need to be loaded again
	 */
	p_hdcp->hdcp14_key_loaded = false;
	hdmitx_event_mgr_suspend(tx_comm->event_mgr, true);
	/*
	 * if HPD is high before suspend, and there were hpd
	 * plugout -> in event happened in deep suspend stage,
	 * now resume and stay in early resume stage, still
	 * need to respond to plugin irq and read/update EDID.
	 * so clear last_hpd_handle_done_stat for re-enter
	 * plugin handle. Note there may be re-enter plugout/in
	 * handler under suspend
	 */
	hdev->tx_comm.last_hpd_handle_done_stat = HDMI_TX_NONE;
	HDMITX_INFO("amhdmitx: suspend\n");
	return 0;
}

static int amhdmitx_resume(struct platform_device *pdev)
{
	struct hdmitx_dev *hdev = dev_get_drvdata(&pdev->dev);
	struct hdmitx_common *tx_comm = &hdev->tx_comm;
	struct hdmitx_hw_common *tx_hw_base = tx_comm->tx_hw;

	HDMITX_INFO("amhdmitx: resume\n");
	hdmitx_clk_ctrl(hdev, 1);
	/*
	 * Since the S7 chip, in order to optimize power consumption, it will turn off and on the
	 * vpu power domain when standby and wakes up.When it is turned off, the reg of the relevant
	 * modules will be reset. When it wakes up,the hdmitx driver needs to reinitialize the
	 * required top register.
	 */
	if (hdev->tx_hw.chip_data->chip_type >= MESON_CPU_ID_S7)
		hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_HW_INIT, 1);
	mutex_lock(&tx_comm->hdmimode_mutex);
	hdmitx_event_mgr_suspend(tx_comm->event_mgr, false);
	/* need to update EDID in case TV changed during suspend */
	tx_comm->hpd_state = !!(hdmitx_hw_cntl_misc(tx_hw_base, MISC_HPD_GPI_ST, 0));
	if (tx_comm->hpd_state)
		hdmitx_plugin_common_work(tx_comm);
	else
		hdmitx_plugout_common_work(tx_comm);
	hdmitx_common_notify_hpd_status(tx_comm, false);
	mutex_unlock(&tx_comm->hdmimode_mutex);

	/* notify to drm hdmi */
	hdmitx_fire_drm_hpd_cb_unlocked(tx_comm);

	return 0;
}

static int amhdmitx_pm_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	HDMITX_DEBUG("%s suspend\n", __func__);
	return amhdmitx_suspend(pdev, PMSG_SUSPEND);
}

static int amhdmitx_pm_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);

	HDMITX_DEBUG("%s resume\n", __func__);
	return amhdmitx_resume(pdev);
}

/*
 * After suspend to disk and resume, only global/static values
 * will be restored, HW registers will be cleared because of
 * power down operations. When restore, it will not do
 * probe if driver is insmod by ko instead of built-in.
 * If hdmi HW reg/clk is not set in uboot, or even HW reg/clk
 * is set in uboot, but powered down after enter kernel somehow,
 * then need to do HW init again when pm restore.
 */
static int amhdmitx_pm_restore(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct hdmitx_dev *hdev = dev_get_drvdata(&pdev->dev);
	struct hdmitx_common *tx_comm = &hdev->tx_comm;
	struct hdmitx_hw_common *tx_hw_base = tx_comm->tx_hw;

	HDMITX_INFO("%s restore\n", __func__);
	mutex_lock(&tx_comm->hdmimode_mutex);
	/*
	 * if hdmitx init and display already, then should not do
	 * HW init again as it may change clk settings, otherwise
	 * need to do hw init as did in driver probe in case HW is
	 * in power down or unknown state
	 */
	hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_HW_INIT, 0);
	/*
	 * after suspend to disk and before resume, it may change TV set,
	 * need to update EDID/HPD/fmt_para by current HW status
	 * as which did in driver probe
	 */
	tx_comm->hpd_state = !!(hdmitx_hw_cntl_misc(tx_hw_base, MISC_HPD_GPI_ST, 0));
	if (tx_comm->hpd_state)
		hdmitx_plugin_common_work(tx_comm);
	else
		hdmitx_plugout_common_work(tx_comm);

	/* load fmt para from hw info */
	hdmitx_common_init_bootup_format_para(tx_comm, &tx_comm->fmt_para);
	if (tx_comm->fmt_para.vic != HDMI_0_UNKNOWN)
		tx_comm->ready = 1;
	else
		tx_comm->ready = 0;
	/* rebuild fmt attr */
	hdmitx_format_para_rebuild_fmtattr_str(&tx_comm->fmt_para,
		tx_comm->fmt_attr, sizeof(tx_comm->fmt_attr));
	/* load init hdr state for HW info */
	hdmitx_hdr_state_init(tx_comm);
	hdmitx_common_notify_hpd_status(tx_comm, false);
	mutex_unlock(&tx_comm->hdmimode_mutex);
	/* in case TV set changed after suspend, need to update vinfo */
	if (tx_comm->ready == 1)
		hdmitx21_init_uboot_mode(VMODE_INIT_BIT_MASK);
	/* notify to drm hdmi */
	hdmitx_fire_drm_hpd_cb_unlocked(tx_comm);
	return 0;
}

const struct dev_pm_ops hdmitx21_pm = {
	.suspend	= amhdmitx_pm_suspend,
	.resume		= amhdmitx_pm_resume,
	.restore	= amhdmitx_pm_restore,
};
#endif

static struct platform_driver amhdmitx_driver = {
	.probe	 = amhdmitx_probe,
	.remove	 = amhdmitx_remove,
#ifdef CONFIG_PM
	.suspend	= amhdmitx_suspend,
	.resume	 = amhdmitx_resume,
#endif
	.shutdown = amhdmitx_shutdown,
	.driver	 = {
		.name = DEVICE_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(meson_amhdmitx_of_match),
#ifdef CONFIG_PM
		.pm = &hdmitx21_pm,
#endif
	}
};

int  __init amhdmitx21_init(void)
{
	struct hdmitx_boot_param *param = get_hdmitx_boot_params();

	if (param->init_state & INIT_FLAG_NOT_LOAD) {
		HDMITX_INFO("INIT_FLAG_NOT_LOAD");
		return 0;
	}

	return platform_driver_register(&amhdmitx_driver);
}

void __exit amhdmitx21_exit(void)
{
	HDMITX_INFO("%s\n", __func__);
	/* TODO stop hdcp */
	platform_driver_unregister(&amhdmitx_driver);
}

//MODULE_DESCRIPTION("AMLOGIC HDMI TX driver");
//MODULE_LICENSE("GPL");
//MODULE_VERSION("1.0.0");

/*************DRM connector API**************/
int hdmitx_hook_drm(struct device *device)
{
	struct hdmitx_dev *hdev = dev_get_drvdata(device);

	return hdmitx_bind_meson_drm(device,
		&hdev->tx_comm,
		&hdev->tx_hw.base);
}

int hdmitx_unhook_drm(struct device *device)
{
	struct hdmitx_dev *hdev = dev_get_drvdata(device);

	return hdmitx_unbind_meson_drm(device,
		&hdev->tx_comm,
		&hdev->tx_hw.base);
}

/*************DRM connector API end**************/
