// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

/*
 * define DEBUG macro to enable pr_debug
 * print to log buffer
 */
/* #define DEBUG */
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
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/reboot.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>

#include <linux/amlogic/media/vout/vinfo.h>
#include <linux/amlogic/media/vout/vout_notify.h>
#if IS_ENABLED(CONFIG_AMLOGIC_SND_SOC)
#include <linux/amlogic/media/sound/aout_notify.h>
#endif
#include <linux/amlogic/media/vout/hdmi_tx_ext.h>
#include <linux/amlogic/media/registers/cpu_version.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_audio.h>
#include <linux/of_gpio.h>

#include "hdmi_tx_module.h"
#include "hw/tvenc_conf.h"
#include "hw/common.h"
#include "hw/hw_clk.h"
#include "hw/reg_ops.h"
#include "hdmi_tx_hdcp.h"
#include "meson_drm_hdmitx.h"

#include <linux/component.h>
#include <uapi/drm/drm_mode.h>
#include <linux/amlogic/gki_module.h>
#include <drm/amlogic/meson_drm_bind.h>
#include <hdmitx_boot_parameters.h>
#include <hdmitx_drm_hook.h>
#include <hdmitx_sysfs_common.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_platform_linux.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_edid.h>
#include "../hdmitx_common/hdmitx_compliance.h"

#define DEVICE_NAME "amhdmitx"
#define HDMI_TX_COUNT 32
#define HDMI_TX_POOL_NUM  6
#define HDMI_TX_RESOURCE_NUM 4
#define HDMI_TX_PWR_CTRL_NUM	6

#define to_hdmitx20_dev(x)	container_of(x, struct hdmitx_dev, tx_comm)

static struct class *hdmitx_class;

static int hdmitx_hook_drm(struct device *device);
static int hdmitx_unhook_drm(struct device *device);
const char *hdmitx_mode_get_timing_name(enum hdmi_vic vic);
const struct hdmi_timing *hdmitx_mode_match_timing_name(const char *name);
static void hdmitx_notify_hpd(int hpd, void *p);

static inline int com_str(const char *buf, const char *str)
{
	return strncmp(buf, str, strlen(str)) == 0;
}

#ifdef CONFIG_OF
static struct amhdmitx_data_s amhdmitx_data_g12a = {
	.chip_type = MESON_CPU_ID_G12A,
	.chip_name = "g12a",
};

static struct amhdmitx_data_s amhdmitx_data_g12b = {
	.chip_type = MESON_CPU_ID_G12B,
	.chip_name = "g12b",
};

static struct amhdmitx_data_s amhdmitx_data_sm1 = {
	.chip_type = MESON_CPU_ID_SM1,
	.chip_name = "sm1",
};

static struct amhdmitx_data_s amhdmitx_data_sc2 = {
	.chip_type = MESON_CPU_ID_SC2,
	.chip_name = "sc2",
};

static struct amhdmitx_data_s amhdmitx_data_tm2 = {
	.chip_type = MESON_CPU_ID_TM2,
	.chip_name = "tm2",
};

static const struct of_device_id meson_amhdmitx_of_match[] = {
	{
		.compatible	 = "amlogic, amhdmitx-g12a",
		.data = &amhdmitx_data_g12a,
	},
	{
		.compatible	 = "amlogic, amhdmitx-g12b",
		.data = &amhdmitx_data_g12b,
	},
	{
		.compatible	 = "amlogic, amhdmitx-sm1",
		.data = &amhdmitx_data_sm1,
	},
	{
		.compatible	 = "amlogic, amhdmitx-sc2",
		.data = &amhdmitx_data_sc2,
	},
	{
		.compatible	 = "amlogic, amhdmitx-tm2",
		.data = &amhdmitx_data_tm2,
	},
	{},
};
#else
#define meson_amhdmitx_dt_match NULL
#endif

static struct hdmitx_dev *tx20_dev;

struct vout_device_s hdmitx_vdev = {
	.fresh_tx_hdr_pkt = hdmitx_set_drm_pkt,
	.fresh_tx_vsif_pkt = hdmitx_set_vsif_pkt,
	.fresh_tx_hdr10plus_pkt = hdmitx_set_hdr10plus_pkt,
	.fresh_tx_cuva_hdr_vsif = hdmitx_set_cuva_hdr_vsif,
	.fresh_tx_cuva_hdr_vs_emds = hdmitx_set_cuva_hdr_vs_emds,
};

int hdmitx_set_uevent_state(enum hdmitx_event type, int state)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	return hdmitx_event_mgr_set_uevent_state(hdev->tx_comm.event_mgr,
				type, state);
}

int hdmitx_set_uevent(enum hdmitx_event type, int val)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	return hdmitx_event_mgr_send_uevent(hdev->tx_comm.event_mgr,
				type, val, false);
}

static void hdmitx20_clear_packets(struct hdmitx_hw_common *tx_hw_base)
{
	/* HW: clear hdr related packets */
	hdmitx_set_drm_pkt(NULL);
	hdmitx_set_vsif_pkt(EOTF_T_NULL, 0, NULL, true);
	hdmitx_set_hdr10plus_pkt(0, NULL);
	hdmitx_hw_cntl_config(tx_hw_base, CONF_CLR_AVI_PACKET, 0);
}

static void hdmitx20_ops_disable_hdcp(struct hdmitx_common *tx_comm)
{
	struct hdmitx_hw_common *tx_hw_base = tx_comm->tx_hw;

	/* HW: mux to hdcp14 & hdcp14 off, DDC free */
	hdmitx_hw_cntl_ddc(tx_hw_base, DDC_HDCP_MUX_INIT, 1);
	hdmitx_hw_cntl_ddc(tx_hw_base, DDC_HDCP_OP, HDCP14_OFF);
	tx_comm->hdcp_mode = 0;
	tx_comm->hdcp_bcaps_repeater = 0;
}

#ifdef CONFIG_AMLOGIC_LEGACY_EARLY_SUSPEND
#include <linux/amlogic/pm.h>
static void hdmitx_early_suspend(struct early_suspend *h)
{
	struct hdmitx_dev *hdev = (struct hdmitx_dev *)h->param;
	struct hdmitx_common *tx_comm = &hdev->tx_comm;
	bool need_rst_ratio;

	mutex_lock(&hdev->tx_comm.hdmimode_mutex);

	need_rst_ratio = hdmitx_find_vendor_ratio(hdev->tx_comm.EDID_buf);

	/* step1: keep hdcp auth state before suspend */
	hdmitx_hw_cntl_misc(tx_comm->tx_hw, MISC_SUSFLAG, 1);
	/*
	 * under suspend, driver should not respond to mode setting,
	 * as it may cause logic abnormal, most importantly,
	 * it will enable hdcp and occupy DDC channel with high
	 * priority, though there's protection in system control,
	 * driver still need protection in case of old android version
	 */
	tx_comm->suspend_flag = true;
	HDMITX_INFO(SYS "Early Suspend\n");

	/* step2: clear ready status/disable phy/packets/hdcp HW */
	hdmitx_common_output_disable(&hdev->tx_comm,
		true, true, true, false);

	/* step3: SW: post uevent to system */
	hdmitx_set_uevent(HDMITX_HDCPPWR_EVENT, HDMI_SUSPEND);
	hdmitx_set_uevent(HDMITX_AUDIO_EVENT, 0);

	if (need_rst_ratio)
		hdmitx_hw_cntl_ddc(&hdev->tx_hw.base, DDC_SCDC_DIV40_SCRAMB, 0);

	mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
}

static void hdmitx_late_resume(struct early_suspend *h)
{
	struct hdmitx_dev *hdev = (struct hdmitx_dev *)h->param;

	mutex_lock(&hdev->tx_comm.hdmimode_mutex);

	hdmitx_common_late_resume(&hdev->tx_comm);
	HDMITX_INFO(SYS "Late Resume\n");
	mutex_unlock(&hdev->tx_comm.hdmimode_mutex);

	/* notify to drm hdmi */
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
	struct hdmitx_dev *hdev = container_of(nb, struct hdmitx_dev, reboot_nb);

	hdmitx_common_avmute_locked(&hdev->tx_comm, SET_AVMUTE, AVMUTE_PATH_HDMITX);

	hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_TMDS_PHY_OP, TMDS_PHY_DISABLE);
	hdev->tx_comm.ready = 0;

	return NOTIFY_OK;
}

static struct early_suspend hdmitx_early_suspend_handler = {
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 10,
	.suspend = hdmitx_early_suspend,
	.resume = hdmitx_late_resume,
};
#endif

void hdmitx20_audio_mute_op(unsigned int flag)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	if (hdev->tx_comm.hdmi_init != HDMITX20)
		return;

	hdev->tx_comm.cur_audio_param.aud_output_en = flag;
	if (flag == 0)
		hdmitx_hw_cntl_config(&hdev->tx_hw.base,
			CONF_AUDIO_MUTE_OP, AUDIO_MUTE);
	else
		hdmitx_hw_cntl_config(&hdev->tx_hw.base,
			CONF_AUDIO_MUTE_OP, AUDIO_UNMUTE);
}

void hdmitx20_video_mute_op(unsigned int flag)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	if (hdev->tx_comm.hdmi_init != HDMITX20)
		return;

	if (flag == 0) {
		/* hdev->tx_hw.base.cntlconfig(&hdev->tx_hw.base, */
			/* CONF_VIDEO_MUTE_OP, VIDEO_MUTE); */
		hdev->tx_comm.vid_mute_op = VIDEO_MUTE;
	} else {
		/* hdev->tx_hw.base.cntlconfig(&hdev->tx_hw.base, */
			/* CONF_VIDEO_MUTE_OP, VIDEO_UNMUTE); */
		hdev->tx_comm.vid_mute_op = VIDEO_UNMUTE;
	}
}

static ssize_t config_store(struct device *dev,
			    struct device_attribute *attr,
			    const char *buf, size_t count)
{
	int ret = 0;
	struct hdmitx_dev *hdev = dev_get_drvdata(dev);
	struct master_display_info_s data = {0};
	struct hdr10plus_para hdr_data = {0x1, 0x2, 0x3};
	struct dv_vsif_para vsif_para = {0};

	HDMITX_INFO("config: %s\n", buf);

	if (strncmp(buf, "3d", 2) == 0) {
		/* Second, set 3D parameters */
		if (strncmp(buf + 2, "tb", 2) == 0) {
			hdev->tx_comm.flag_3dtb = 1;
			hdev->tx_comm.flag_3dss = 0;
			hdev->tx_comm.flag_3dfp = 0;
			hdmi_set_3d(hdev, T3D_TAB, 0);
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
			hdmi_set_3d(hdev, T3D_SBS_HALF,
				    sub_sample_mode);
		} else if (strncmp(buf + 2, "fp", 2) == 0) {
			hdev->tx_comm.flag_3dtb = 0;
			hdev->tx_comm.flag_3dss = 0;
			hdev->tx_comm.flag_3dfp = 1;
			hdmi_set_3d(hdev, T3D_FRAME_PACKING, 0);
		} else if (strncmp(buf + 2, "off", 3) == 0) {
			hdev->tx_comm.flag_3dfp = 0;
			hdev->tx_comm.flag_3dtb = 0;
			hdev->tx_comm.flag_3dss = 0;
			hdmi_set_3d(hdev, T3D_DISABLE, 0);
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
	}
	return count;
}

static void hdmitx20_ext_set_audio_output(bool enable)
{
	HDMITX_INFO("audio: enable = %d\n", enable);
	hdmitx20_audio_mute_op(enable);
}

static int hdmitx20_ext_get_audio_status(void)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();
	int val;
	static int val_st;

	val = !!(hdmitx_hw_cntl_config(&hdev->tx_hw.base, CONF_GET_AUDIO_MUTE_ST, 0));
	if (val_st != val) {
		val_st = val;
		HDMITX_INFO("audio: get val = %d\n", val);
	}
	return val;
}

static int hdmi_hdr_status_to_drm(void)
{
	enum hdmi_tf_type type = HDMI_NONE;
	struct hdmitx_dev *hdev = get_hdmitx_device();

	type = hdmitx_hw_get_state(&hdev->tx_hw.base, STAT_TX_HDR10P, 0);
	if (type) {
		if (type == HDMI_HDR10P_DV_VSIF)
			return HDR10PLUS_VSIF;
	}
	type = hdmitx_hw_get_state(&hdev->tx_hw.base, STAT_TX_DV, 0);
	if (type) {
		if (type == HDMI_DV_VSIF_STD)
			return dolbyvision_std;
		else if (type == HDMI_DV_VSIF_LL)
			return dolbyvision_lowlatency;
	}
	type = hdmitx_hw_get_state(&hdev->tx_hw.base, STAT_TX_HDR, 0);
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

void print_hsty_drm_config_data(void)
{

}

void print_hsty_vsif_config_data(void)
{

}

void print_hsty_hdr10p_config_data(void)
{

}

void print_hsty_hdmiaud_config_data(void)
{
	struct aud_para *data;
	unsigned int arr_cnt, pr_loc;
	unsigned int print_num;

	pr_loc = hsty_hdmiaud_config_loc - 1;
	if (hsty_hdmiaud_config_num > 8)
		print_num = 8;
	else
		print_num = hsty_hdmiaud_config_num;
	HDMITX_INFO("******hdmitx_audpara have trans %d times******\n",
		hsty_hdmiaud_config_num);
	for (arr_cnt = 0; arr_cnt < print_num; arr_cnt++) {
		HDMITX_INFO("***hsty_hdmiaud_config_data[%u]***\n", arr_cnt);
		data = &hsty_hdmiaud_config_data[pr_loc];
		HDMITX_INFO("type: %u, chnum: %u, samrate: %u, samsize: %u\n",
			data->type, data->chs, data->rate, data->size);
		pr_loc = pr_loc > 0 ? pr_loc - 1 : 7;
	}
}

static DEVICE_ATTR_WO(config);

static void hdmitx20_bootup_update_vinfo(struct hdmitx_dev *hdev)
{
	struct hdmitx_common *tx_comm = &hdev->tx_comm;

	edidinfo_attach_to_vinfo(tx_comm);
	update_vinfo_from_formatpara(tx_comm);

	/* Should be started at end of output */
	if (hdev->tx_comm.cedst_en) {
		cancel_delayed_work(&hdev->tx_comm.work_cedst);
		queue_delayed_work(hdev->tx_comm.cedst_wq, &hdev->tx_comm.work_cedst, 0);
	}
}

static int hdmitx20_enable_mode(struct hdmitx_common *tx_comm, struct hdmi_format_para *para)
{
	int ret;
	struct hdmitx_dev *hdev = to_hdmitx20_dev(tx_comm);

	/* if vic is HDMI_UNKNOWN, hdmitx_set_display will disable HDMI */
	ret = hdmitx_set_display(hdev, para->vic);

	hdmitx_set_audio(hdev, &hdev->tx_comm.cur_audio_param);

	return 0;
}

static int hdmitx20_init_uboot_mode(enum vmode_e mode)
{
	if (!(mode & VMODE_INIT_BIT_MASK))
		HDMITX_ERROR("warning, echo /sys/class/display/mode is disabled\n");
	else
		HDMITX_INFO("alread display in uboot\n");

	return 0;
}

static struct hdmitx_ctrl_ops tx20_ctrl_ops = {
	.pre_enable_mode = NULL,
	.enable_mode = hdmitx20_enable_mode,
	.post_enable_mode = NULL,
	.disable_mode = NULL,
	.init_uboot_mode = hdmitx20_init_uboot_mode,
	.disable_hdcp = hdmitx20_ops_disable_hdcp,
	.clear_pkt = hdmitx20_clear_packets,
	.disable_21_work = NULL,
	.disable_frl_work = NULL,
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
	struct hdmitx_dev *hdev = get_hdmitx_device();

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
	hdmitx_set_uevent(HDMITX_RXSENSE_EVENT, sense);
	queue_delayed_work(tx_comm->rxsense_wq, &tx_comm->work_rxsense, HZ);
}

static void hdmitx_cedst_process(struct work_struct *work)
{
	int ced;
	struct hdmitx_common *tx_comm = container_of((struct delayed_work *)work,
		struct hdmitx_common, work_cedst);
	struct hdmitx_dev *hdev = to_hdmitx20_dev(tx_comm);

	ced = hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_TMDS_CEDST, 0);
	/* firstly send as 0, then real ced, A trigger signal */
	hdmitx_set_uevent(HDMITX_CEDST_EVENT, 0);
	hdmitx_set_uevent(HDMITX_CEDST_EVENT, ced);
	queue_delayed_work(tx_comm->cedst_wq, &tx_comm->work_cedst, HZ);
}

static void hdmitx_bootup_process_plugin(struct hdmitx_dev *hdev, bool set_audio)
{
	struct vinfo_s *info = NULL;

	/* step1: SW: EDID read/parse, notify client modules */
	hdmitx_bootup_plugin_work(&hdev->tx_comm);

	/*
	 * During the kernel startup process, the HDR/DV module will use
	 * vinfo information, it needs to attach vinfo after the EDID is
	 * parsed and before the HDR/DV module is enabled.
	 * so do as hdmitx_common_post_enable_mode()
	 */
	hdmitx20_bootup_update_vinfo(hdev);

	/* TODO: need remove/optimised, keep it temporarily */
	if (set_audio) {
		info = hdmitx_get_current_vinfo(NULL);
		if (info && info->mode == VMODE_HDMI)
			hdmitx_set_audio(hdev, &hdev->tx_comm.cur_audio_param);
	}

	/* step2: SW: notify client modules and update uevent state */
	hdmitx_bootup_notify_hpd_status(&hdev->tx_comm, false);
}

static void hdmitx_process_plugin(struct hdmitx_dev *hdev)
{
	struct hdmitx_common *tx_comm = &hdev->tx_comm;
	int i;
	unsigned char cta_block_count;
	u8 *edid_buf = tx_comm->EDID_buf;
	u8 edid_check = 0;
	unsigned long flags = 0;

	/* step1: SW: EDID read */
	hdmitx_plugin_common_work(tx_comm);

	/* step2: SW: update cec phy addr and audio data block */
	spin_lock_irqsave(&tx_comm->edid_spinlock, flags);
	hdmitx_edid_rxcap_clear(&tx_comm->rxcap);
	hdmitx_cec_phy_addr_parse(&tx_comm->rxcap.vsdb_phy_addr, edid_buf);
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
	if (hdev->tx_comm.fmt_para.tmds_clk_div40)
		hdmitx_hw_cntl_ddc(&hdev->tx_hw.base, DDC_SCDC_DIV40_SCRAMB, 1);
	hdmitx_bootup_process_plugin(hdev, hdev->tx_comm.ready);
}

static void hdmitx_hpd_plugin_irq_handler(struct work_struct *work)
{
	struct hdmitx_dev *hdev = container_of((struct delayed_work *)work,
		struct hdmitx_dev, work_hpd_plugin);

	mutex_lock(&hdev->tx_comm.hdmimode_mutex);

	/*
	 * this may happen when just queue plugin work,
	 * but plugout event happen at this time. no need
	 * to continue plugin work.
	 */
	if (hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_HPD_GPI_ST, 0) == 0) {
		HDMITX_INFO("plug out event come when plugin handle, abort handle\n");
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
		HDMITX_INFO("warning: continuous plugin, should not happen!\n");
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
	/*
	 * after suspend, hdcp auth state(including topo info) should
	 * keep not changed, thus that encrypted video stream can
	 * recover playing normally after resume, specially for hdcp
	 * repeater case
	 */
	if (!hdev->tx_comm.suspend_flag)
		hdmitx_hw_cntl_ddc(&hdev->tx_hw.base, DDC_HDCP_SET_TOPO_INFO, 0);

	/* SW: notify event to user space and other modules */
	hdmitx_common_notify_hpd_status(&hdev->tx_comm, false);
}

/* plugout handle only for bootup stage */
static void hdmitx_bootup_plugout_handler(struct hdmitx_dev *hdev)
{
	hdmitx_process_plugout(hdev);
}

/* plugout handle for hpd irq */
static void hdmitx_hpd_plugout_irq_handler(struct work_struct *work)
{
	struct hdmitx_dev *hdev = container_of((struct delayed_work *)work,
		struct hdmitx_dev, work_hpd_plugout);

	mutex_lock(&hdev->tx_comm.hdmimode_mutex);
	if (hdev->tx_comm.last_hpd_handle_done_stat == HDMI_TX_HPD_PLUGOUT) {
		HDMITX_INFO("continuous plugout handler, ignore\n");
		mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
		return;
	}
	hdmitx_process_plugout(hdev);
	mutex_unlock(&hdev->tx_comm.hdmimode_mutex);

	/* notify to drm hdmi */
	hdmitx_fire_drm_hpd_cb_unlocked(&hdev->tx_comm);
}

extern unsigned int __hdmitx_debug;
static void hdmitx_internal_intr_handler(struct work_struct *work)
{
	struct hdmitx_dev *hdev = container_of((struct delayed_work *)work,
		struct hdmitx_dev, work_internal_intr);

	if (__hdmitx_debug & REG_LOG)
		hdev->tx_hw.base.debugfun(&hdev->tx_hw.base, "dumpintr");
}

int get20_hpd_state(void)
{
	int ret;
	struct hdmitx_dev *hdev = get_hdmitx_device();

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
	.release  = amhdmitx_release,
};

struct hdmitx_dev *get_hdmitx_device(void)
{
	return tx20_dev;
}
EXPORT_SYMBOL(get_hdmitx_device);

int get_hdmitx20_init(void)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	if (hdev)
		return hdev->tx_comm.hdmi_init;
	return 0;
}

struct vsdb_phyaddr *get_hdmitx20_phy_addr(void)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	return &hdev->tx_comm.rxcap.vsdb_phy_addr;
}

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
int hdmitx20_event_notifier_regist(struct notifier_block *nb)
{
	int ret = 0;
	struct hdmitx_dev *hdev = get_hdmitx_device();

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
		/*
		 * TODO: actually notify phy_addr is not used by CEC/hdmirx,
		 * just keep for safety
		 */
		/* if (hdev->tx_comm.rxcap.physical_addr != 0xffff) { */
		/* if (hdev->tx_comm.hdmi_repeater == 1) */
		/* hdmitx_event_mgr_notify(hdev->tx_comm.event_mgr, */
		/* HDMITX_PHY_ADDR_VALID, &hdev->tx_comm.rxcap.physical_addr); */
		/* } */
	}

	return ret;
}

int hdmitx20_event_notifier_unregist(struct notifier_block *nb)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	return hdmitx_event_mgr_notifier_unregister(hdev->tx_comm.event_mgr,
		(struct hdmitx_notifier_client *)nb);
}

static void hdmitx_notify_hpd(int hpd, void *p)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	if (hpd)
		hdmitx_event_mgr_notify(hdev->tx_comm.event_mgr,
			HDMITX_PLUG, p);
	else
		hdmitx_event_mgr_notify(hdev->tx_comm.event_mgr,
			HDMITX_UNPLUG, p);
}

void hdmitx_hdcp_status(int hdmi_authenticated)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	hdmitx_set_uevent(HDMITX_HDCP_EVENT, hdmi_authenticated);
	if (hdev->drm_hdcp_cb.callback)
		hdev->drm_hdcp_cb.callback(hdev->drm_hdcp_cb.data,
			hdmi_authenticated);
}

static int amhdmitx_device_init(struct hdmitx_dev *hdmi_dev)
{
	const char *hdmi_ver = NULL;

	GET_HDMITX_VER(hdmi_ver);
	HDMITX_INFO("Ver: %s\n", hdmi_ver);

	hdmi_dev->hdtx_dev = NULL;

	hdmi_dev->tx_comm.hdcp_mode = 0;
	hdmi_dev->tx_comm.ready = 0;
	hdmi_dev->tx_comm.hdcp_user = 1;
	hdmi_dev->tx_comm.hdr_mute_frame = 20;
	/* no RxSense by default */
	hdmi_dev->tx_comm.rxsense_policy = 0;
	/* enable or disable HDMITX SSPLL, enable by default */
	hdmi_dev->tx_comm.sspll = 1;

	hdmi_dev->tx_comm.flag_3dfp = 0;
	hdmi_dev->tx_comm.flag_3dss = 0;
	hdmi_dev->tx_comm.flag_3dtb = 0;

	/* default audio configure is on */
	hdmi_dev->tx_comm.cur_audio_param.aud_output_en = 1;
	hdmi_dev->tx_comm.topo_info =
		kmalloc(sizeof(struct hdcprp_topo), GFP_KERNEL);
	if (!hdmi_dev->tx_comm.topo_info)
		HDMITX_ERROR("failed to alloc hdcp topo info\n");
	hdmi_dev->tx_comm.vid_mute_op = VIDEO_NONE_OP;
	hdmi_dev->tx_comm.ctrl_ops = &tx20_ctrl_ops;
	hdmi_dev->tx_comm.vdev = &hdmitx_vdev;
	set_dummy_dv_info(&hdmitx_vdev);

	/* not capable of DSC/FRL */
	hdmi_dev->tx_hw.base.hdmi_tx_cap.dsc_capable = false;
	hdmi_dev->tx_hw.base.hdmi_tx_cap.tx_max_frl_rate = FRL_NONE;

	return 0;
}

static int amhdmitx_get_dt_info(struct platform_device *pdev, struct hdmitx_dev *hdev)
{
	int ret = 0;

#ifdef CONFIG_OF
	int val;
	phandle handle;
	struct device_node *init_data;
	const struct of_device_id *match;
#else
	struct hdmi_config_platform_data *hdmi_pdata;
#endif
	u32 refreshrate_limit = 0;
	struct hdmitx_hw_common *tx_hw_base;

	/* HDMITX pinctrl config for hdp and ddc */
	if (pdev->dev.pins) {
		hdev->pdev = &pdev->dev;

		hdev->pinctrl_default =
			pinctrl_lookup_state(pdev->dev.pins->p, "default");
		if (IS_ERR(hdev->pinctrl_default))
			HDMITX_ERROR("no default of pinctrl state\n");

		hdev->pinctrl_i2c =
			pinctrl_lookup_state(pdev->dev.pins->p, "hdmitx_i2c");
		if (IS_ERR(hdev->pinctrl_i2c))
			HDMITX_DEBUG("no hdmitx_i2c of pinctrl state\n");

		pinctrl_select_state(pdev->dev.pins->p,
				     hdev->pinctrl_default);
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

		memset(&hdev->config_data, 0,
		       sizeof(struct hdmi_config_platform_data));
		/* Get chip type and name information */
		match = of_match_device(meson_amhdmitx_of_match, &pdev->dev);

		if (!match) {
			HDMITX_INFO("%s: no match table\n", __func__);
			return -1;
		}

		hdev->tx_hw.chip_data = (struct amhdmitx_data_s *)match->data;
		if (hdev->tx_hw.chip_data->chip_type == MESON_CPU_ID_TM2 ||
			hdev->tx_hw.chip_data->chip_type == MESON_CPU_ID_TM2B) {
			/* diff revA/B of TM2 chip */
			if (is_meson_rev_b()) {
				hdev->tx_hw.chip_data->chip_type = MESON_CPU_ID_TM2B;
				hdev->tx_hw.chip_data->chip_name = "tm2b";
			} else {
				hdev->tx_hw.chip_data->chip_type = MESON_CPU_ID_TM2;
				hdev->tx_hw.chip_data->chip_name = "tm2";
			}
		}
		HDMITX_DEBUG("chip_type:%d chip_name:%s\n",
			hdev->tx_hw.chip_data->chip_type,
			hdev->tx_hw.chip_data->chip_name);

		/* Get hdmi_rext information */
		ret = of_property_read_u32(pdev->dev.of_node, "hdmi_rext", &val);
		hdev->hdmi_rext = val;
		if (!ret)
			HDMITX_INFO("hdmi_rext: %d\n", val);

		/* Get dongle_mode information */
		ret = of_property_read_u32(pdev->dev.of_node, "dongle_mode",
					   &dongle_mode);
		dongle_mode = !!hdev->tx_hw.dongle_mode;
		if (!ret)
			HDMITX_INFO("hdmitx_device.dongle_mode: %d\n",
				hdev->tx_hw.dongle_mode);
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
		ret = of_property_read_u32(pdev->dev.of_node,
					   "hdmi_repeater", &val);
		if (!ret)
			hdev->tx_comm.hdmi_repeater = val;
		else
			hdev->tx_comm.hdmi_repeater = 1;
		/* if it's not hdmi repeater, then should not support hdcp repeater */
		if (hdev->tx_comm.hdmi_repeater == 0)
			hdev->tx_hw.base.hdcp_repeater_en = 0;
		if (hdev->tx_hw.base.hdcp_repeater_en)
			hdev->tx_comm.topo_info =
				kzalloc(sizeof(*hdev->tx_comm.topo_info), GFP_KERNEL);

		ret = of_property_read_u32(pdev->dev.of_node,
					   "cedst_en", &val);
		if (!ret)
			hdev->tx_comm.cedst_en = !!val;

		ret = of_property_read_u32(pdev->dev.of_node,
					   "hdcp_type_policy", &val);
		if (!ret) {
			hdev->tx_comm.hdcp_type_policy = 0;
			if (val == 2)
				hdev->tx_comm.hdcp_type_policy = 2;
			if (val == 1)
				hdev->tx_comm.hdcp_type_policy = 1;
			if (val == 3)
				hdev->tx_comm.hdcp_type_policy = -1;
		}

		/* Get vendor information */
		ret = of_property_read_u32(pdev->dev.of_node,
					   "vend-data", &val);
		if (ret)
			HDMITX_INFO("not find match init-data\n");
		if (ret == 0) {
			handle = val;
			init_data = of_find_node_by_phandle(handle);
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
			HDMITX_DEBUG("not find match pwr-ctl\n");
		if (ret == 0) {
			handle = val;
			init_data = of_find_node_by_phandle(handle);
			if (!init_data)
				HDMITX_DEBUG("not find device node\n");
			hdev->config_data.pwr_ctl =
			kzalloc((sizeof(struct hdmi_pwr_ctl)) *
			HDMI_TX_PWR_CTRL_NUM, GFP_KERNEL);
			if (!hdev->config_data.pwr_ctl)
				HDMITX_INFO("can not get pwr_ctl mem\n");
			else
				memset(hdev->config_data.pwr_ctl, 0,
					sizeof(struct hdmi_pwr_ctl));
			if (ret)
				HDMITX_DEBUG("not find pwr_ctl\n");
		}
		/* hdcp ctrl 0:sysctrl, 1: drv, 2: linux app */
		ret = of_property_read_u32(pdev->dev.of_node, "hdcp_ctl_lvl",
					   &hdev->tx_comm.hdcp_ctl_lvl);
		if (ret)
			hdev->tx_comm.hdcp_ctl_lvl = 0;

		/* Get reg information */
		ret = hdmitx_init_reg_map(pdev);
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
	HDMITX_DEBUG("hpd irq = %d\n", hdev->irq_hpd);

	hdev->irq_viu1_vsync =
		platform_get_irq_byname(pdev, "viu1_vsync");
	if (hdev->irq_viu1_vsync == -ENXIO) {
		HDMITX_ERROR("%s: ERROR: viu1_vsync irq No not found\n",
		       __func__);
		return -ENXIO;
	}
	HDMITX_DEBUG("viu1_vsync irq = %d\n", hdev->irq_viu1_vsync);

	return ret;
}

/*
 * amhdmitx_clktree_probe
 * get clktree info from dts
 */
static void amhdmitx_clktree_probe(struct device *hdmitx_dev, struct hdmitx_dev *hdev)
{
	struct clk *hdmi_clk_vapb, *hdmi_clk_vpu;
	struct clk *hdcp22_tx_skp, *hdcp22_tx_esm;
	struct clk *venci_top_gate, *venci_0_gate, *venci_1_gate;
	struct clk *cts_hdmi_axi_clk;

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

	hdcp22_tx_skp = devm_clk_get(hdmitx_dev, "hdcp22_tx_skp");
	if (!IS_ERR(hdcp22_tx_skp))
		hdev->hdmitx_clk_tree.hdcp22_tx_skp = hdcp22_tx_skp;

	hdcp22_tx_esm = devm_clk_get(hdmitx_dev, "hdcp22_tx_esm");
	if (!IS_ERR(hdcp22_tx_esm))
		hdev->hdmitx_clk_tree.hdcp22_tx_esm = hdcp22_tx_esm;

	venci_top_gate = devm_clk_get(hdmitx_dev, "venci_top_gate");
	if (!IS_ERR(venci_top_gate))
		hdev->hdmitx_clk_tree.venci_top_gate = venci_top_gate;

	venci_0_gate = devm_clk_get(hdmitx_dev, "venci_0_gate");
	if (!IS_ERR(venci_0_gate))
		hdev->hdmitx_clk_tree.venci_0_gate = venci_0_gate;

	venci_1_gate = devm_clk_get(hdmitx_dev, "venci_1_gate");
	if (!IS_ERR(venci_1_gate))
		hdev->hdmitx_clk_tree.venci_1_gate = venci_1_gate;

	cts_hdmi_axi_clk = devm_clk_get(hdmitx_dev, "cts_hdmi_axi_clk");
	if (!IS_ERR(cts_hdmi_axi_clk))
		hdev->hdmitx_clk_tree.cts_hdmi_axi_clk = cts_hdmi_axi_clk;
}

static int amhdmitx_probe(struct platform_device *pdev)
{
	int r, ret = 0;
	struct device *device = &pdev->dev;
	struct device *dev;
	struct hdmitx_dev *hdev;
	struct hdmitx_common *tx_comm;
	struct hdmitx_tracer *tx_tracer;
	struct hdmitx_event_mgr *tx_uevent_mgr;
	bool hpd_state;

	HDMITX_INFO("amhdmitx_probe_start\n");

	hdev = devm_kzalloc(device, sizeof(*hdev), GFP_KERNEL);
	if (!hdev)
		return -ENOMEM;

	tx20_dev = hdev;
	dev_set_drvdata(device, hdev);
	tx_comm = &hdev->tx_comm;
	amhdmitx_device_init(hdev);
	/* init txcommon */
	hdmitx_common_init(tx_comm, &hdev->tx_hw.base);
	ret = amhdmitx_get_dt_info(pdev, hdev);
	/* if (ret) */
	/*	return ret; */

	amhdmitx_clktree_probe(device, hdev);

	r = alloc_chrdev_region(&hdev->hdmitx_id, 0, HDMI_TX_COUNT,
				DEVICE_NAME);
	cdev_init(&hdev->cdev, &amhdmitx_fops);
	hdev->cdev.owner = THIS_MODULE;
	r = cdev_add(&hdev->cdev, hdev->hdmitx_id, HDMI_TX_COUNT);

	hdmitx_class = class_create(DEVICE_NAME);
	if (IS_ERR(hdmitx_class)) {
		unregister_chrdev_region(hdev->hdmitx_id, HDMI_TX_COUNT);
		return -1;
	}

	/* kernel>=2.6.27 */
	dev = device_create(hdmitx_class, NULL, hdev->hdmitx_id, hdev,
			    "amhdmitx%d", MINOR(hdev->hdmitx_id));

	if (!dev) {
		HDMITX_ERROR("device_create create error\n");
		class_destroy(hdmitx_class);
		r = -EEXIST;
		return r;
	}
	hdev->hdtx_dev = dev;
	ret = device_create_file(dev, &dev_attr_config);

#ifdef CONFIG_AMLOGIC_VPU
	hdev->encp_vpu_dev = vpu_dev_register(VPU_VENCP, DEVICE_NAME);
	hdev->enci_vpu_dev = vpu_dev_register(VPU_VENCI, DEVICE_NAME);
	/* vpu gate/mem ctrl for hdmitx, since TM2B */
	hdev->hdmi_vpu_dev = vpu_dev_register(VPU_HDMI, DEVICE_NAME);
#endif

	/* platform related functions */
	tx_uevent_mgr = hdmitx_event_mgr_create(pdev, hdev->hdtx_dev);
	hdmitx_event_mgr_suspend(tx_uevent_mgr, false);
	hdmitx_common_attch_platform_data(tx_comm,
		HDMITX_PLATFORM_UEVENT, tx_uevent_mgr);
	tx_tracer = hdmitx_tracer_create(tx_uevent_mgr);
	hdmitx_common_attch_platform_data(tx_comm,
		HDMITX_PLATFORM_TRACER, tx_tracer);
	hdmitx_audio_register_ctrl_callback(tx_tracer, hdmitx20_ext_set_audio_output,
		hdmitx20_ext_get_audio_status);
#ifdef CONFIG_AMLOGIC_LEGACY_EARLY_SUSPEND
	hdmitx_early_suspend_handler.param = hdev;
	register_early_suspend(&hdmitx_early_suspend_handler);
#endif
	hdev->reboot_nb.notifier_call = hdmitx_reboot_notifier;
	register_reboot_notifier(&hdev->reboot_nb);

	/* init hw */
	hdmitx_meson_init(hdev);
	/* load fmt para from hw info */
	hdmitx_common_init_bootup_format_para(tx_comm, &tx_comm->fmt_para);
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
	hdmitx_hdr_init(tx_comm);
	hdmitx_packet_init(tx_comm);
	hdmitx_hdr_state_init(tx_comm);
	hdmitx_vout_init(tx_comm, &hdev->tx_hw.base);
	/* load init audio fmt for HW info */
#if IS_ENABLED(CONFIG_AMLOGIC_SND_SOC)
	if (hdmitx_uboot_audio_en()) {
		struct aud_para *audpara = &hdev->tx_comm.cur_audio_param;

		audpara->rate = FS_48K;
		audpara->type = CT_PCM;
		audpara->size = SS_16BITS;
		audpara->chs = 2 - 1;
	}
	/* default audio clock is ON */
	hdmitx20_audio_mute_op(1);
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
	INIT_DELAYED_WORK(&hdev->work_internal_intr, hdmitx_internal_intr_handler);

	/* for rx sense feature */
	hdev->tx_comm.rxsense_wq = alloc_workqueue("hdmitx_rxsense",
					   WQ_SYSFS | WQ_FREEZABLE, 0);
	INIT_DELAYED_WORK(&hdev->tx_comm.work_rxsense, hdmitx_rxsense_process);
	/* for cedst feature */
	hdev->tx_comm.cedst_wq = alloc_workqueue("hdmitx_cedst",
					 WQ_SYSFS | WQ_FREEZABLE, 0);
	INIT_DELAYED_WORK(&hdev->tx_comm.work_cedst, hdmitx_cedst_process);

	set_hdcp_common_instance(&hdev->tx_comm);
	hdmitx_hdcp_init(hdev);
	/* bind drm before hdmi event */
	hdmitx_hook_drm(&pdev->dev);

	/* init power_uevent state */
	hdmitx_set_uevent(HDMITX_HDCPPWR_EVENT, HDMI_WAKEUP);
	/* reset EDID/vinfo */
	hdmitx_edid_buffer_clear(hdev->tx_comm.EDID_buf, sizeof(hdev->tx_comm.EDID_buf));
	hdmitx_edid_rxcap_clear(&hdev->tx_comm.rxcap);

	/*
	 * hpd process of bootup stage, need to be done in probe
	 * as other client modules may need the hpd/edid info.
	 * use mutex to prevent hpd irq bottom half concurrency.
	 */
	mutex_lock(&hdev->tx_comm.hdmimode_mutex);
	/* enable irq firstly before any hpd handler to prevent missing irq. */
	hdev->tx_hw.base.setupirq(&hdev->tx_hw.base);

	/* actions in top half of plug intr */
	hpd_state = !!hdmitx_hw_cntl_misc(&hdev->tx_hw.base,
		MISC_HPD_GPI_ST, 0);
	hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_HPD_IRQ_TOP_HALF, hpd_state);
	/* actions in bottom half of plug intr */
	if (hpd_state)
		hdmitx_bootup_plugin_handler(hdev);
	else
		hdmitx_bootup_plugout_handler(hdev);

	/* after unlock, now can take actions of bottom half of hpd irq */
	mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
	/* notify to drm hdmi */
	hdmitx_fire_drm_hpd_cb_unlocked(&hdev->tx_comm);

	hdev->tx_comm.hdmi_init = HDMITX20;
	/*everything is ready, create sysfs here.*/
	hdmitx_sysfs_common_create(dev, &hdev->tx_comm, &hdev->tx_hw.base);

	HDMITX_INFO("amhdmitx_probe_end\n");

	return r;
}

static void amhdmitx_remove(struct platform_device *pdev)
{
	struct hdmitx_dev *hdev = dev_get_drvdata(&pdev->dev);
	struct device *dev = hdev->hdtx_dev;

	/* remove sysfs before uninit */
	hdmitx_sysfs_common_destroy(dev);

	/* unbind from drm */
	hdmitx_unhook_drm(&pdev->dev);

	cancel_work_sync(&hdev->tx_comm.work_hdr);
	cancel_work_sync(&hdev->tx_comm.work_hdr_unmute);
	hdmitx_hdcp_exit(hdev);
	cancel_delayed_work(&hdev->work_internal_intr);
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

static void _amhdmitx_suspend(struct hdmitx_dev *hdev)
{
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
	/* drm tx22 enters AUTH_STOP, don't do hdcp22 IP reset */
	if (hdev->tx_comm.hdcp_ctl_lvl > 0)
		return;

	hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_DIS_HPLL, 0);
	hdmitx_hw_cntl_ddc(&hdev->tx_hw.base, DDC_RESET_HDCP, 0);
	hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_ESMCLK_CTRL, 0);
	HDMITX_INFO("amhdmitx: suspend and reset hdcp\n");
}

#ifdef CONFIG_PM
static int amhdmitx_suspend(struct platform_device *pdev,
			    pm_message_t state)
{
	struct hdmitx_dev *hdev = dev_get_drvdata(&pdev->dev);
	struct hdmitx_common *tx_comm = &hdev->tx_comm;

	hdmitx_event_mgr_suspend(tx_comm->event_mgr, true);
	_amhdmitx_suspend(hdev);
	return 0;
}

static int amhdmitx_resume(struct platform_device *pdev)
{
	struct hdmitx_dev *hdev = dev_get_drvdata(&pdev->dev);
	struct hdmitx_common *tx_comm = &hdev->tx_comm;
	struct hdmitx_hw_common *tx_hw_base = tx_comm->tx_hw;

	mutex_lock(&tx_comm->hdmimode_mutex);
	hdmitx_event_mgr_suspend(tx_comm->event_mgr, false);
	/* need to update EDID in case TV changed during suspend */
	tx_comm->hpd_state = !!(hdmitx_hw_cntl_misc(tx_hw_base, MISC_HPD_GPI_ST, 0));
	if (tx_comm->hpd_state)
		hdmitx_plugin_common_work(tx_comm);
	else
		hdmitx_plugout_common_work(tx_comm);
	mutex_unlock(&tx_comm->hdmimode_mutex);
	/* notify to drm hdmi  */
	/* hdmitx_fire_drm_hpd_cb_unlocked(&hdev->tx_comm); */
	/*
	 * may resume after start hdcp22, i2c
	 * reactive will force mux to hdcp14
	 */
	if (hdev->tx_comm.hdcp_ctl_lvl > 0)
		return 0;
	hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_ESMCLK_CTRL, 1);
	HDMITX_INFO("amhdmitx: resume\n");

	if (hdev->tx_hw.chip_data->chip_type < MESON_CPU_ID_G12A)
		hdmitx_hw_cntl_misc(&hdev->tx_hw.base, MISC_I2C_RESET, 0);
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

const struct dev_pm_ops hdmitx20_pm = {
	.suspend	= amhdmitx_pm_suspend,
	.resume		= amhdmitx_pm_resume,
	/* no application for now */
	.restore    = NULL,
};
#endif

static void amhdmitx_shutdown(struct platform_device *pdev)
{
	struct hdmitx_dev *hdev = dev_get_drvdata(&pdev->dev);

	_amhdmitx_suspend(hdev);
}

static struct platform_driver amhdmitx_driver = {
	.probe	  = amhdmitx_probe,
	.remove	 = amhdmitx_remove,
#ifdef CONFIG_PM
	.suspend	= amhdmitx_suspend,
	.resume	 = amhdmitx_resume,
#endif
	.shutdown = amhdmitx_shutdown,
	.driver	 = {
		.name   = DEVICE_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(meson_amhdmitx_of_match),
#ifdef CONFIG_PM
		.pm = &hdmitx20_pm,
#endif
	}
};

int  __init amhdmitx_init(void)
{
	struct hdmitx_boot_param *param = get_hdmitx_boot_params();

	if (param->init_state & INIT_FLAG_NOT_LOAD)
		return 0;

	return platform_driver_register(&amhdmitx_driver);
}

void __exit amhdmitx_exit(void)
{
	HDMITX_INFO("amhdmitx_exit\n");
	platform_driver_unregister(&amhdmitx_driver);
}

//MODULE_DESCRIPTION("AMLOGIC HDMI TX driver");
//MODULE_LICENSE("GPL");
//MODULE_VERSION("1.0.0");

/*************DRM connector API**************/
static struct meson_hdmitx_dev drm_hdmitx_instance = {
	.get_hdmi_hdr_status = hdmi_hdr_status_to_drm,
};

int hdmitx_hook_drm(struct device *device)
{
	struct hdmitx_dev *hdev;

	hdev = dev_get_drvdata(device);
	return hdmitx_bind_meson_drm(device,
		&hdev->tx_comm,
		&hdev->tx_hw.base,
		&drm_hdmitx_instance);
}

int hdmitx_unhook_drm(struct device *device)
{
	struct hdmitx_dev *hdev = dev_get_drvdata(device);

	return hdmitx_unbind_meson_drm(device,
		&hdev->tx_comm,
		&hdev->tx_hw.base,
		&drm_hdmitx_instance);
}

/*************DRM connector API end**************/
