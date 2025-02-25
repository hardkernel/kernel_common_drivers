// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/major.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/devinfo.h>
#include <linux/pinctrl/consumer.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/miscdevice.h>
#include <linux/of_gpio.h>
#include <linux/reboot.h>
#include <linux/amlogic/clk_measure.h>
#include <linux/delay.h>

#include <linux/amlogic/media/video_sink/video.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_config.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_platform_linux.h>
#include <linux/amlogic/media/registers/cpu_version.h>
#if IS_ENABLED(CONFIG_AMLOGIC_SND_SOC)
#include <linux/amlogic/media/sound/aout_notify.h>
#endif

#ifdef CONFIG_AMLOGIC_DSC
#include <linux/amlogic/media/vout/dsc.h>
#endif

#include "hdmitx_log.h"
#include "hdmitx_boot_parameters.h"
#include "hdmitx_drm.h"
#include "hdmitx_sysfs_common.h"
#include "hdmitx_module.h"
#include "hdmitx_compliance.h"
#include "hdmitx_dump.h"

#ifdef CONFIG_AMLOGIC_HDMITX

#include "hdmitx_common.h"
#include "hdmitx_hdcp.h"
#endif

#if CONFIG_AMLOGIC_HDMITX21

#include "hdmitx.h"
#include "hdmitx_common.h"
#include "tvin_global.h"
#include "hdmi_rx_repeater.h"

#endif

#define HDMI_TX_COUNT 32
static struct class *hdmitx_class;
static struct hdmitx_dev *hdmitx_device;
static u8 hdmi_allm_passthough_en;
unsigned int rx_hdcp2_ver;

static void hdmitx21_vid_pll_clk_check(struct hdmitx_dev *hdev);
static void hdmitx21_reset_hdcp_param(struct hdmitx_dev *hdev);

struct hdmitx_dev *get_hdmitx_device(void)
{
	return hdmitx_device;
}
EXPORT_SYMBOL(get_hdmitx_device);

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

static struct vout_device_s hdmitx_vdev = {
	.fresh_tx_hdr_pkt = hdmitx_set_drm_pkt,
	.fresh_tx_sbtm_pkt = hdmitx_set_sbtm_pkt,
	.fresh_tx_vsif_pkt = hdmitx_set_vsif_pkt,
	.fresh_tx_hdr10plus_pkt = hdmitx_set_hdr10plus_pkt,
	.fresh_tx_cuva_hdr_vsif = hdmitx_set_cuva_hdr_vsif,
	.fresh_tx_cuva_hdr_vs_emds = hdmitx_set_cuva_hdr_vs_emds,
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

static void amhdmitx_infoframe_init(struct hdmitx_dev *hdev)
{
	int ret = 0;

	ret = hdmi_vendor_infoframe_init(&hdev->tx_comm.infoframes.vend.vendor.hdmi);
	if (ret)
		HDMITX_INFO("%s[%d] init vendor infoframe failed %d\n", __func__, __LINE__, ret);
	hdmi_avi_infoframe_init(&hdev->tx_comm.infoframes.avi.avi);

	// TODO, panic
	// hdmi_spd_infoframe_init(&hdev->infoframes.spd.spd,
	//	hdev->tx_comm.config_data.vend_data->vendor_name,
	//	hdev->tx_comm.config_data.vend_data->product_desc);
	hdmi_audio_infoframe_init(&hdev->tx_comm.infoframes.aud.audio);
	hdmi_drm_infoframe_init(&hdev->tx_comm.infoframes.drm.drm);
}

static int hdmitx_device_init(struct hdmitx_dev *hdev)
{
	const char *hdmi_ver = NULL;

	if (!hdev)
		return 1;

	GET_HDMITX_VER(hdmi_ver);
	HDMITX_INFO("Ver: %s\n", hdmi_ver);

	hdev->tx_comm.hdtx_dev = NULL;
	hdev->tx_comm.vdev = &hdmitx_vdev;

	if (hdev->hw_comm.chip_data->chip_type < MESON_CPU_ID_T7) {
		hdev->tx_comm.topo_info =
			kmalloc(sizeof(struct hdcprp_topo), GFP_KERNEL);
		if (!hdev->tx_comm.topo_info)
			HDMITX_ERROR("failed to alloc hdcp topo info\n");

		/* not capable of DSC/FRL */
		hdev->hw_comm.hdmi_tx_cap.dsc_capable = false;
		hdev->hw_comm.hdmi_tx_cap.tx_max_frl_rate = FRL_NONE;

		/* for hdcp */
		hdev->max_exceed = 200;
		hdev->tx20_hw.base = &hdev->hw_comm;
	} else {
		hdev->tx_comm.def_stream_type = DEFAULT_STREAM_TYPE;

		hdev->tx_comm.need_filter_hdcp_off = false;
		/* default 6S */
		hdev->tx_comm.filter_hdcp_off_period = 6;
		hdev->tx_comm.not_restart_hdcp = false;
		/* wait for upstream start hdcp auth 5S */
		hdev->up_hdcp_timeout_sec = 5;
		/* ll mode init values */
		hdev->tx_comm.ll_enabled_in_auto_mode = false;
		hdev->tx_comm.ll_user_set_mode = HDMI_LL_MODE_AUTO;
		/* vrr function init*/
		hdev->vrr_dbg_vframe = -1;

		hdev->dfm_type = -1;
		hdev->csc_type = 1;
		/* create misc device for communication with TEE: hdcp key load ready notify */
		tee_comm_dev_reg(hdev);

		amhdmitx_infoframe_init(hdev);
		hdev->tx21_hw.base = &hdev->hw_comm;
	}
	set_dummy_dv_info(&hdmitx_vdev);

	return 0;
}

static int amhdmitx_get_dt_info(struct platform_device *pdev, struct hdmitx_dev *hdev)
{
	struct pinctrl *pin;
	int ret;
#ifdef CONFIG_OF
	int val;
	phandle phandler;
	struct device_node *init_data;
	const struct of_device_id *match;
#else
	struct hdmi_config_platform_data *hdmi_pdata;
#endif

	u32 refreshrate_limit = 0;
	struct hdmitx_hw_common *tx_hw_base;

	tx_hw_base = &hdev->hw_comm;
#ifdef CONFIG_OF
	/* Get chip type and name information */
	match = of_match_device(meson_amhdmitx_of_match, &pdev->dev);

	if (!match) {
		HDMITX_INFO("%s: no match table\n", __func__);
		return -1;
	}
	tx_hw_base->chip_data = (struct amhdmitx_data_s *)match->data;
	HDMITX_DEBUG("chip_type:%d chip_name:%s\n",
		tx_hw_base->chip_data->chip_type,
		tx_hw_base->chip_data->chip_name);
	hdmitx_device_init(hdev);
#endif
	/* HDMITX pinctrl config for hdp and ddc */
	if (pdev->dev.of_node) {
		hdev->tx_comm.pdev = &pdev->dev;
		pin = devm_pinctrl_get(&pdev->dev);
		if (!pin) {
			HDMITX_INFO("get pin control fail\n");
			return -1;
		}
		if (tx_hw_base->chip_data->chip_type < MESON_CPU_ID_T7) {
			hdev->tx_comm.pinctrl_default =
				pinctrl_lookup_state(pdev->dev.pins->p, "default");
			if (IS_ERR(hdev->tx_comm.pinctrl_default))
				HDMITX_ERROR("no default of pinctrl state\n");

			hdev->tx_comm.pinctrl_i2c =
				pinctrl_lookup_state(pdev->dev.pins->p, "hdmitx_i2c");
			if (IS_ERR(hdev->tx_comm.pinctrl_i2c))
				HDMITX_DEBUG("no hdmitx_i2c of pinctrl state\n");
			pinctrl_select_state(pdev->dev.pins->p,
					hdev->tx_comm.pinctrl_default);
		} else {
			hdev->tx_comm.pinctrl_default = pinctrl_lookup_state(pin, "hdmitx_hpd");
			if (IS_ERR(hdev->tx_comm.pinctrl_default)) {
				HDMITX_ERROR("no default of pinctrl state\n");
			} else {
				ret = pinctrl_select_state(pin, hdev->tx_comm.pinctrl_default);
				if (ret < 0)
					HDMITX_ERROR("failed to select default pinctrl state\n");
			}

			hdev->tx_comm.pinctrl_i2c = pinctrl_lookup_state(pin, "hdmitx_ddc");
			if (IS_ERR(hdev->tx_comm.pinctrl_i2c)) {
				HDMITX_ERROR("no hdmitx_i2c of pinctrl state\n");
			} else {
				ret = pinctrl_select_state(pin, hdev->tx_comm.pinctrl_i2c);
				if (ret < 0)
					HDMITX_ERROR("failed to select hdmitx_i2c pinctrl state\n");
			}
		}
	} else {
		HDMITX_INFO("node null\n");
	}
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

		memset(&hdev->tx_comm.config_data, 0,
		       sizeof(struct hdmi_config_platform_data));

		if (tx_hw_base->chip_data->chip_type == MESON_CPU_ID_TM2 ||
			tx_hw_base->chip_data->chip_type == MESON_CPU_ID_TM2B) {
			/* diff revA/B of TM2 chip */
			if (is_meson_rev_b()) {
				tx_hw_base->chip_data->chip_type = MESON_CPU_ID_TM2B;
				tx_hw_base->chip_data->chip_name = "tm2b";
			} else {
				tx_hw_base->chip_data->chip_type = MESON_CPU_ID_TM2;
				tx_hw_base->chip_data->chip_name = "tm2";
			}
		}
		if (tx_hw_base->chip_data->chip_type == MESON_CPU_ID_S5)
			tx_hw_base->hdmi_tx_cap.dsc_capable = true;
		else
			tx_hw_base->hdmi_tx_cap.dsc_capable = false;
		/* Get hdmi_rext information */
		ret = of_property_read_u32(pdev->dev.of_node, "hdmi_rext", &val);
		hdev->tx_comm.hdmi_rext = val;
		if (!ret)
			HDMITX_INFO("hdmi_rext: %d\n", val);
		/* Get pxp_mode information */
		ret = of_property_read_u32(pdev->dev.of_node, "pxp_mode",
					   &pxp_mode);
		hdev->tx_comm.pxp_mode = pxp_mode;
		if (!ret)
			HDMITX_INFO("hdev->pxp_mode: %d\n", hdev->tx_comm.pxp_mode);
		/* Get dongle_mode information */
		ret = of_property_read_u32(pdev->dev.of_node, "dongle_mode",
					   &dongle_mode);
		tx_hw_base->dongle_mode = !!dongle_mode;
		if (!ret)
			HDMITX_INFO("hdmitx_device.dongle_mode: %d\n",
				tx_hw_base->dongle_mode);
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
			tx_hw_base->hdmi_tx_cap.tx_max_tmds_clk = 225;
		else
			tx_hw_base->hdmi_tx_cap.tx_max_tmds_clk = 600;
		ret = of_property_read_u32(pdev->dev.of_node, "max_refreshrate",
					   &refreshrate_limit);
		if (ret == 0 && refreshrate_limit > 0)
			hdev->tx_comm.max_refreshrate = refreshrate_limit;
		/* Get repeater_tx information */
		ret = of_property_read_u32(pdev->dev.of_node,
					   "repeater_tx", &val);
		if (!ret)
			tx_hw_base->hdcp_repeater_en = val;
		else
			tx_hw_base->hdcp_repeater_en = 0;
		/* Get repeater_tx information */
		ret = of_property_read_u32(pdev->dev.of_node,
					   "hdmi_repeater", &val);
		if (!ret)
			hdev->tx_comm.hdmi_repeater = val;
		else
			hdev->tx_comm.hdmi_repeater = 1;
		/* if it's not hdmi repeater, then should not support hdcp repeater */
		if (hdev->tx_comm.hdmi_repeater == 0)
			tx_hw_base->hdcp_repeater_en = 0;
		/* Get cedst_en information */
		ret = of_property_read_u32(pdev->dev.of_node,
					   "cedst_en", &val);
		if (!ret)
			hdev->tx_comm.cedst_en = !!val;
		/* Get hdr_8bit_en information */
		ret = of_property_read_u32(pdev->dev.of_node, "hdr_8bit_en", &val);
		if (!ret)
			hdev->tx_comm.hdr_8bit_en = !!val;
		/* Get hdcp_type_policy information */
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
		/* not support FRL by default, unless enabled in dts */
		tx_hw_base->hdmi_tx_cap.tx_max_frl_rate = FRL_NONE;
		ret = of_property_read_u32(pdev->dev.of_node, "tx_max_frl_rate", &val);
		if (!ret) {
			if (val > FRL_12G4L)
				HDMITX_INFO("wrong tx_max_frl_rate %d\n", val);
			else
				tx_hw_base->hdmi_tx_cap.tx_max_frl_rate = val;
		}
		/* hdcp ctrl 0:sysctrl, 1: drv, 2: linux app */
		ret = of_property_read_u32(pdev->dev.of_node,
			   "hdcp_ctl_lvl", &hdev->tx_comm.hdcp_ctl_lvl);
		if (ret)
			hdev->tx_comm.hdcp_ctl_lvl = 0;
		HDMITX_INFO("hdcp_ctl_lvl[%d-%d]\n", hdev->tx_comm.hdcp_ctl_lvl, ret);
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
			hdev->tx_comm.config_data.vend_data =
			kzalloc(sizeof(struct vendor_info_data), GFP_KERNEL);
			if (!(hdev->tx_comm.config_data.vend_data))
				HDMITX_INFO("not allocate memory\n");
			ret = get_dt_vend_init_data
			(init_data, hdev->tx_comm.config_data.vend_data);
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
			hdev->tx_comm.config_data.pwr_ctl = kzalloc((sizeof(struct hdmi_pwr_ctl)) *
				HDMI_TX_PWR_CTRL_NUM, GFP_KERNEL);
			if (!hdev->tx_comm.config_data.pwr_ctl)
				HDMITX_INFO("can not get pwr_ctl mem\n");
			else
				memset(hdev->tx_comm.config_data.pwr_ctl,
					0, sizeof(struct hdmi_pwr_ctl));
			if (ret)
				HDMITX_INFO("not find pwr_ctl\n");
		}
		/* Get reg information */
		if (tx_hw_base->chip_data->chip_type < MESON_CPU_ID_T7)
			ret = hdmitx20_init_reg_map(pdev);
		else
			ret = hdmitx21_init_reg_map(pdev);
		if (ret < 0)
			HDMITX_ERROR("ERROR: hdmitx io_remap fail!\n");

		hdev->tx_comm.enc_idx = hdmitx_get_connector();
		HDMITX_INFO("enc_idx %d\n", hdev->tx_comm.enc_idx);
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
	hdev->tx_comm.irq_hpd = platform_get_irq_byname(pdev, "hdmitx_hpd");
	if (hdev->tx_comm.irq_hpd == -ENXIO) {
		HDMITX_ERROR("%s: ERROR: hdmitx hpd irq No not found\n",
			__func__);
			return -ENXIO;
	}
	HDMITX_DEBUG("hpd irq = %d\n", hdev->tx_comm.irq_hpd);
	if (tx_hw_base->chip_data->chip_type < MESON_CPU_ID_T7) {
		hdev->tx_comm.irq_viu1_vsync =
		platform_get_irq_byname(pdev, "viu1_vsync");
		if (hdev->tx_comm.irq_viu1_vsync == -ENXIO) {
			HDMITX_ERROR("%s: ERROR: viu1_vsync irq No not found\n",
				__func__);
			return -ENXIO;
		}
		HDMITX_DEBUG("viu1_vsync irq = %d\n", hdev->tx_comm.irq_viu1_vsync);
	} else {
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
		hdev->tx_comm.irq_vrr_vsync = platform_get_irq_byname(pdev, "vrr_vsync");
		if (hdev->tx_comm.irq_vrr_vsync == -ENXIO) {
			HDMITX_ERROR("%s: ERROR: hdmitx vrr_vsync irq No not found\n",
				__func__);
				return -ENXIO;
		}
		HDMITX_DEBUG("vrr vsync irq = %d\n", hdev->tx_comm.irq_vrr_vsync);
#endif
	}
	ret = of_property_read_u32(pdev->dev.of_node, "arc_rx_en", &val);
	if (!ret)
		hdev->tx_comm.arc_rx_en = val;
	else
		hdev->tx_comm.arc_rx_en = 0;
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
		hdev->tx_comm.hdmitx_clk_tree.hdmi_clk_vapb = hdmi_clk_vapb;
		clk_prepare_enable(hdev->tx_comm.hdmitx_clk_tree.hdmi_clk_vapb);
	}

	hdmi_clk_vpu = devm_clk_get(hdmitx_dev, "hdmi_vpu_clk");
	if (!IS_ERR(hdmi_clk_vpu)) {
		hdev->tx_comm.hdmitx_clk_tree.hdmi_clk_vpu = hdmi_clk_vpu;
		clk_prepare_enable(hdev->tx_comm.hdmitx_clk_tree.hdmi_clk_vpu);
	}

	hdcp22_tx_skp = devm_clk_get(hdmitx_dev, "hdcp22_tx_skp");
	if (!IS_ERR(hdcp22_tx_skp))
		hdev->tx_comm.hdmitx_clk_tree.hdcp22_tx_skp = hdcp22_tx_skp;

	hdcp22_tx_esm = devm_clk_get(hdmitx_dev, "hdcp22_tx_esm");
	if (!IS_ERR(hdcp22_tx_esm))
		hdev->tx_comm.hdmitx_clk_tree.hdcp22_tx_esm = hdcp22_tx_esm;

	venci_top_gate = devm_clk_get(hdmitx_dev, "venci_top_gate");
	if (!IS_ERR(venci_top_gate))
		hdev->tx_comm.hdmitx_clk_tree.venci_top_gate = venci_top_gate;

	venci_0_gate = devm_clk_get(hdmitx_dev, "venci_0_gate");
	if (!IS_ERR(venci_0_gate))
		hdev->tx_comm.hdmitx_clk_tree.venci_0_gate = venci_0_gate;

	venci_1_gate = devm_clk_get(hdmitx_dev, "venci_1_gate");
	if (!IS_ERR(venci_1_gate))
		hdev->tx_comm.hdmitx_clk_tree.venci_1_gate = venci_1_gate;

	cts_hdmi_axi_clk = devm_clk_get(hdmitx_dev, "cts_hdmi_axi_clk");
	if (!IS_ERR(cts_hdmi_axi_clk))
		hdev->tx_comm.hdmitx_clk_tree.cts_hdmi_axi_clk = cts_hdmi_axi_clk;
}

/*****************************
 *	hdmitx driver file_operations
 *
 ******************************/
static int amhdmitx_open(struct inode *node, struct file *file)
{
	struct hdmitx_common *tx_comm;
	struct hdmitx_dev *hdev;

	/* Get the per-device structure that contains this cdev */
	tx_comm = container_of(node->i_cdev, struct hdmitx_common, cdev);
	hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);
	file->private_data = hdev;

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

#if IS_ENABLED(CONFIG_AMLOGIC_SND_SOC)

static int hdmitx_notify_callback_a(struct notifier_block *block,
				    unsigned long cmd, void *para);
static struct notifier_block hdmitx_notifier_nb_a = {
	.notifier_call	= hdmitx_notify_callback_a,
};

static int hdmitx_notify_callback_a(struct notifier_block *block,
				    unsigned long cmd, void *para)
{
	struct hdmitx_common *tx_comm = container_of(block,
		struct hdmitx_common, hdmitx_notifier_nb_a);

	hdmitx_audio_notify_callback(tx_comm, tx_comm->tx_hw, block, cmd, para);
	return 0;
}

#endif

#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
/* hdmi21 specific functions disable */
void hdmitx_disable_21_work(struct hdmitx_common *tx_comm)
{
	struct hdmitx_dev *hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);

	/* old chip not have this function */
	if (tx_comm->tx_hw->chip_data->chip_type < MESON_CPU_ID_T7)
		return;
	frl_tx_stop();
	hdmitx_set_frl_rate_none(hdev);
	hdmitx_vrr_disable();
#ifdef CONFIG_AMLOGIC_DSC
	if (tx_comm->tx_hw->hdmi_tx_cap.dsc_capable) {
		aml_dsc_enable(false);
		hdmitx_dsc_cvtem_pkt_disable();
		hdev->dsc_en = 0;
	}
#endif
}

void hdmitx_disable_frl_work(struct hdmitx_common *tx_comm)
{
	struct hdmitx_dev *hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);

	if (tx_comm->tx_hw->chip_data->chip_type == MESON_CPU_ID_S5) {
		frl_tx_stop();
		hdmitx_set_frl_rate_none(hdev);
	}
}
#endif

#ifdef CONFIG_AMLOGIC_LEGACY_EARLY_SUSPEND
#include <linux/amlogic/pm.h>
static void hdmitx_early_suspend(struct early_suspend *h)
{
	struct hdmitx_dev *hdev = (struct hdmitx_dev *)h->param;
	struct hdmitx_common *tx_comm = &hdev->tx_comm;
	bool need_rst_ratio;

	need_rst_ratio = hdmitx_find_vendor_ratio(hdev->tx_comm.EDID_buf);
	if (tx_comm->tx_hw->chip_data->chip_type >= MESON_CPU_ID_T7) {
		if (hdev->tx_comm.aon_output) {
			HDMITX_INFO("%s return, HDMI signal enabled\n", __func__);
			return;
		}
	}

	mutex_lock(&hdev->tx_comm.hdmimode_mutex);
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
	tx_comm->qms_log_id = 0;
	HDMITX_INFO(SYS "Early Suspend\n");

	/* step2: clear ready status/disable phy/packets/hdcp HW */
	hdmitx_common_output_disable(&hdev->tx_comm,
		true, true, true, false);
	if (tx_comm->tx_hw->chip_data->chip_type >= MESON_CPU_ID_T7)
		hdmitx21_reset_hdcp_param(hdev);
	/* step3: SW: post uevent to system */
	hdmitx_set_uevent(&hdev->tx_comm, HDMITX_HDCPPWR_EVENT, HDMI_SUSPEND);
	hdmitx_set_uevent(&hdev->tx_comm, HDMITX_AUDIO_EVENT, 0);

	if (need_rst_ratio)
		hdmitx_hw_cntl_ddc(&hdev->hw_comm, DDC_SCDC_DIV40_SCRAMB, 0);

	mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
}

static void hdmitx_late_resume(struct early_suspend *h)
{
	struct hdmitx_dev *hdev = (struct hdmitx_dev *)h->param;
	struct hdmitx_common *tx_comm = &hdev->tx_comm;

	if (tx_comm->tx_hw->chip_data->chip_type >= MESON_CPU_ID_T7) {
		if (hdev->tx_comm.aon_output) {
			HDMITX_INFO("%s return, HDMI signal already enabled\n", __func__);
			return;
		}
	}
	mutex_lock(&hdev->tx_comm.hdmimode_mutex);
	hdmitx_hw_cntl_config(&hdev->hw_comm, CONF_AUDIO_MUTE_OP, AUDIO_MUTE);
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
	struct hdmitx_common *tx_comm = container_of(nb, struct hdmitx_common, reboot_nb);
	struct hdmitx_dev *hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);

	hdmitx_common_avmute_locked(tx_comm, SET_AVMUTE, AVMUTE_PATH_HDMITX);
	hdmitx_hw_cntl_misc(tx_comm->tx_hw, MISC_TMDS_PHY_OP, TMDS_PHY_DISABLE);
	tx_comm->ready = 0;
	if (tx_comm->tx_hw->chip_data->chip_type >= MESON_CPU_ID_T7) {
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
		hdmitx_vrr_disable();
		hdmitx_disable_frl_work(tx_comm);
#ifdef CONFIG_AMLOGIC_DSC
		if (hdev->hw_comm.hdmi_tx_cap.dsc_capable) {
			aml_dsc_enable(false);
			hdmitx_dsc_cvtem_pkt_disable();
			hdev->dsc_en = 0;
		}
#endif
#endif
		hdmitx21_disable_hdcp(hdev);
		hdmitx21_rst_stream_type(hdev->am_hdcp);
	}

	if (hdev->tx_comm.rxsense_policy)
		cancel_delayed_work(&hdev->tx_comm.work_rxsense);
	if (hdev->tx_comm.cedst_en)
		cancel_delayed_work(&hdev->tx_comm.work_cedst);

	return NOTIFY_OK;
}

static struct early_suspend hdmitx_early_suspend_handler = {
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 10,
	.suspend = hdmitx_early_suspend,
	.resume = hdmitx_late_resume,
};
#endif

static void hdmitx_rxsense_process(struct work_struct *work)
{
	int sense;
	struct hdmitx_common *tx_comm = container_of((struct delayed_work *)work,
		struct hdmitx_common, work_rxsense);

	sense = hdmitx_hw_cntl_misc(tx_comm->tx_hw, MISC_TMDS_RXSENSE, 0);
	hdmitx_set_uevent(tx_comm, HDMITX_RXSENSE_EVENT, sense);
	queue_delayed_work(tx_comm->rxsense_wq, &tx_comm->work_rxsense, HZ);
}

static void hdmitx_cedst_process(struct work_struct *work)
{
	int ced;
	struct hdmitx_common *tx_comm = container_of((struct delayed_work *)work,
		struct hdmitx_common, work_cedst);

	ced = hdmitx_hw_cntl_misc(tx_comm->tx_hw, MISC_TMDS_CEDST, 0);
	/* firstly send as 0, then real ced, A trigger signal */
	hdmitx_set_uevent(tx_comm, HDMITX_CEDST_EVENT, 0);
	hdmitx_set_uevent(tx_comm, HDMITX_CEDST_EVENT, ced);
	queue_delayed_work(tx_comm->cedst_wq, &tx_comm->work_cedst, HZ);
}

static void hdmitx_meson_init(struct hdmitx_dev *hdev)
{
	if (hdev->tx_comm.tx_hw->chip_data->chip_type < MESON_CPU_ID_T7)
		hdmitx20_meson_init(hdev);
	else
		hdmitx21_meson_init(hdev);
}

static void hdmitx_audio_init(struct hdmitx_common *tx_comm, struct hdmitx_tracer *tx_tracer)
{
	bool audio_en;

	if (tx_comm->tx_hw->chip_data->chip_type < MESON_CPU_ID_T7)
		audio_en = hdmitx_uboot_audio_en();
	else
		audio_en = hdmitx21_uboot_audio_en();

#if IS_ENABLED(CONFIG_AMLOGIC_SND_SOC)
	if (!tx_comm->pxp_mode && audio_en) {
		struct aud_para *audpara = &tx_comm->cur_audio_param;

		audpara->rate = FS_48K;
		audpara->type = CT_PCM;
		audpara->size = SS_16BITS;
		audpara->chs = 2 - 1;
	}
	/* default audio clock is ON */
	if (tx_comm->tx_hw->chip_data->chip_type < MESON_CPU_ID_T7)
		hdmitx20_audio_mute_op(1);
	else
		hdmitx21_audio_mute_op(1, 0);
#endif
	if (tx_comm->tx_hw->chip_data->chip_type < MESON_CPU_ID_T7)
		hdmitx_audio_register_ctrl_callback(tx_tracer,
			hdmitx20_ext_set_audio_output,
			hdmitx20_ext_get_audio_status);
	else
		hdmitx_audio_register_ctrl_callback(tx_tracer,
			hdmitx21_ext_set_audio_output,
			hdmitx21_ext_get_audio_status);
}

/* action in suspend/plugout handler, should not be done when disable_module */
static void hdmitx21_reset_hdcp_param(struct hdmitx_dev *hdev)
{
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

static void hdmitx_process_plugin(struct hdmitx_dev *hdev)
{
	struct hdmitx_common *tx_comm = &hdev->tx_comm;
	unsigned long flags = 0;

	/* step1: SW: EDID read */
	hdmitx_plugin_common_work(tx_comm);

	/* step2: SW: update cec phy addr and audio data block */
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

	hdmitx_cec_phy_addr_parse(&tx_comm->rxcap, tx_comm->EDID_buf);
	hdmitx_audio_parse(&tx_comm->rxcap, tx_comm->EDID_buf);
	spin_unlock_irqrestore(&tx_comm->edid_spinlock, flags);

	/* step3: SW: notify client modules and update uevent state */
	hdmitx_common_notify_hpd_status(tx_comm, false);
}

static void hdmitx_hpd_plugin_irq_handler(struct work_struct *work)
{
	struct hdmitx_common *tx_comm = container_of((struct delayed_work *)work,
		struct hdmitx_common, work_hpd_plugin);
	struct hdmitx_dev *hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);

	mutex_lock(&hdev->tx_comm.hdmimode_mutex);

	/*
	 * this may happen when just queue plugin work,
	 * but plugout event happen at this time. no need
	 * to continue plugin work.
	 */
	if (hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_HPD_GPI_ST, 0) == 0) {
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
	if (hdev->tx_comm.tx_hw->chip_data->chip_type >= MESON_CPU_ID_T7) {
		hdmitx21_reset_hdcp_param(hdev);
		/* for vsync loss when HPD loss */
		hdmitx21_vid_pll_clk_check(hdev);
	}
	/*
	 * after suspend, hdcp auth state(including topo info) should
	 * keep not changed, thus that encrypted video stream can
	 * recover playing normally after resume, specially for hdcp
	 * repeater case
	 */
	if (!hdev->tx_comm.suspend_flag)
		hdmitx_hw_cntl_ddc(&hdev->hw_comm, DDC_HDCP_SET_TOPO_INFO, 0);

	/*
	 * Reset the ll_enabled_in_auto_mode flag used for auto mode
	 * status. If we are in auto mode, gaming signal should be enabled
	 * when the request arrives again from the input device or playback
	 * and not on hotplug.
	 */
	if (hdev->tx_comm.tx_hw->chip_data->chip_type >= MESON_CPU_ID_T7)
		hdev->tx_comm.ll_enabled_in_auto_mode = false;
	/* SW: notify event to user space and other modules */
	hdmitx_common_notify_hpd_status(&hdev->tx_comm, false);
}

/* plugout handle for hpd irq */
static void hdmitx_hpd_plugout_irq_handler(struct work_struct *work)
{
	struct hdmitx_common *tx_comm = container_of((struct delayed_work *)work,
		struct hdmitx_common, work_hpd_plugout);
	struct hdmitx_dev *hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);

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

/* plugout handle only for bootup stage */
static void hdmitx_bootup_plugout_handler(struct hdmitx_dev *hdev)
{
	hdmitx_process_plugout(hdev);
}

static void hdmitx_bootup_update_vinfo(struct hdmitx_dev *hdev)
{
	struct vinfo_s *vinfo = NULL;
	struct hdmitx_common *tx_comm = &hdev->tx_comm;

	edidinfo_attach_to_vinfo(tx_comm);
	update_vinfo_from_formatpara(tx_comm);

	if (tx_comm->tx_hw->chip_data->chip_type >= MESON_CPU_ID_T7) {
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
	}

	/* Should be started at end of output */
	if (hdev->tx_comm.cedst_en) {
		cancel_delayed_work(&hdev->tx_comm.work_cedst);
		queue_delayed_work(hdev->tx_comm.cedst_wq, &hdev->tx_comm.work_cedst, 0);
	}
}

static bool is_frl_ready(struct hdmitx_dev *hdev)
{
	enum frl_rate_enum tx_frl_rate =
		hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_GET_FRL_MODE, 0);
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

/* check clk status when plug out in case no vsync */
static void hdmitx21_vid_pll_clk_check(struct hdmitx_dev *hdev)
{
	int clk[3];
	int idx[3];

	if (hdev->tx_comm.tx_hw->chip_data->chip_type != MESON_CPU_ID_S5)
		return;
	/*
	 * frl mode use fpll or gp2 pll, and won't go through
	 * vid_clk0_div_top/tmds20_clk_div_top, no need to check.
	 */
	if (hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_GET_FRL_MODE, 0))
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
		hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_CLK_DIV_RST, idx[0]);
		HDMITX_INFO("after reset the clock[%d] is %d\n", idx[0], meson_clk_measure(idx[0]));
	}
	if (!clk[1]) {
		HDMITX_DEBUG("%s the clock[%d] is %d\n",  __func__, idx[1], clk[1]);
		hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_CLK_DIV_RST, idx[1]);
		HDMITX_INFO("after reset the clock[%d] is %d\n", idx[1], meson_clk_measure(idx[1]));
	}
}

/* used for status check when hdmi output setting done */
static int hdmitx21_status_check(void *data)
{
	int clk[3];
	int idx[3];
	struct hdmitx_dev *hdev = get_hdmitx_device();

	if (hdev->tx_comm.tx_hw->chip_data->chip_type != MESON_CPU_ID_S5)
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
		if (hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_GET_FRL_MODE, 0))
			continue;
		clk[0] = meson_clk_measure(idx[0]);
		clk[1] = meson_clk_measure(idx[1]);
		if (clk[0] && clk[1])
			continue;

		if (!clk[0]) {
			HDMITX_DEBUG("the clock[%d] is %d\n", idx[0], clk[0]);
			hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_CLK_DIV_RST, idx[0]);
			HDMITX_DEBUG("reset the clock div for %d\n", idx[0]);
			HDMITX_INFO("the clock[%d] is %d\n", idx[0], meson_clk_measure(idx[0]));
			HDMITX_INFO("the clock[%d] is %d\n", idx[2], meson_clk_measure(idx[2]));
		}
		if (!clk[1]) {
			HDMITX_DEBUG("the clock[%d] is %d\n", idx[1], clk[1]);
			hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_CLK_DIV_RST, idx[1]);
			HDMITX_DEBUG("reset the clock div for %d\n", idx[1]);
			HDMITX_INFO("the clock[%d] is %d\n", idx[1], meson_clk_measure(idx[1]));
		}
		/* resend the SCDC/DIV40 config */
		if (!clk[0] || !clk[1]) {
			clk[0] = meson_clk_measure(idx[0]);
			if (clk[0] >= 340000000)
				hdmitx_hw_cntl_ddc(&hdev->hw_comm,
					DDC_SCDC_DIV40_SCRAMB, 1);
			else
				hdmitx_hw_cntl_ddc(&hdev->hw_comm,
					DDC_SCDC_DIV40_SCRAMB, 0);
		}
	}
	return 0;
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
	hdmitx_bootup_update_vinfo(hdev);

	/* TODO: need remove/optimised, keep it temporarily */
	if (set_audio) {
		info = hdmitx_get_current_vinfo(NULL);
		if (info && info->mode == VMODE_HDMI) {
			if (hdev->tx_comm.tx_hw->chip_data->chip_type < MESON_CPU_ID_T7)
				hdmitx20_set_audio(hdev, &hdev->tx_comm.cur_audio_param);
			else
				hdmitx21_set_audio(hdev, &hdev->tx_comm.cur_audio_param);
		}
	}

	/* step2: SW: notify client modules and update uevent state */
	hdmitx_common_notify_hpd_status(&hdev->tx_comm, false);
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
	int frl_rate = FRL_NONE;

	if (hdev->tx_comm.tx_hw->chip_data->chip_type == MESON_CPU_ID_S5)
		frl_rate = hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_GET_FRL_MODE, 0);
	/* if current mode is TMDS/nonFRL, then resend_div40 */
	if (frl_rate == FRL_NONE) {
		if (hdev->tx_comm.fmt_para.tmds_clk_div40)
			hdmitx_hw_cntl_ddc(&hdev->hw_comm, DDC_SCDC_DIV40_SCRAMB, 1);
	} else {
		if (!is_frl_ready(hdev))
			hdev->tx_comm.ready = 0;
	}
	hdmitx_bootup_process_plugin(hdev, hdev->tx_comm.ready);
}

extern unsigned int __hdmitx_debug;
static void hdmitx_internal_intr_handler(struct work_struct *work)
{
	struct hdmitx_common *tx_comm = container_of((struct delayed_work *)work,
		struct hdmitx_common, work_internal_intr);
	struct hdmitx_dev *hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);

	if (__hdmitx_debug & REG_LOG)
		hdev->hw_comm.debugfunc(&hdev->hw_comm, "dumpintr");
}

static void hdmitx_start_hdcp_handler(struct work_struct *work)
{
	struct hdmitx_dev *hdev = container_of((struct delayed_work *)work,
		struct hdmitx_dev, work_start_hdcp);
	struct hdmitx_common *tx_comm = &hdev->tx_comm;
	unsigned long timeout_sec;

	if (hdcp_need_control_by_upstream(&hdev->hw_comm)) {
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
			hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_AVMUTE_OP, CLR_AVMUTE);
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
		if (!hdev->tx_comm.ready || !hdev->tx_comm.hpd_state) {
			HDMITX_INFO("signal ready: %d, hpd_state: %d, eixt hdcp2\n",
				hdev->tx_comm.ready, hdev->tx_comm.hpd_state);
			hdev->tx_comm.is_passthrough_switch = 0;
			mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
			return;
		}
		hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_AVMUTE_OP, CLR_AVMUTE);
		hdmitx21_enable_hdcp(hdev);
		mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
	}
	/* clear after start hdcp */
	hdev->tx_comm.is_passthrough_switch = 0;
}

static void hdmitx_up_hdcp_timeout_handler(struct work_struct *work)
{
	struct hdmitx_dev *hdev = container_of((struct delayed_work *)work,
		struct hdmitx_dev, work_up_hdcp_timeout);
	struct hdmitx_common *tx_comm = &hdev->tx_comm;

	if (hdcp_need_control_by_upstream(tx_comm->tx_hw)) {
		HDMITX_INFO("enable hdcp as wait upstream hdcp timeout\n");
		/* note: hdcp should only be started when hdmi signal ready */
		mutex_lock(&tx_comm->hdmimode_mutex);
		if (!tx_comm->ready || !tx_comm->hpd_state) {
			HDMITX_INFO("signal ready: %d, hpd_state: %d, exit hdcp\n",
				tx_comm->ready, tx_comm->hpd_state);
			mutex_unlock(&tx_comm->hdmimode_mutex);
			return;
		}
		hdmitx_hw_cntl_misc(tx_comm->tx_hw, MISC_AVMUTE_OP, CLR_AVMUTE);
		hdmitx21_enable_hdcp(hdev);
		mutex_unlock(&tx_comm->hdmimode_mutex);
	} else {
		HDMITX_INFO("wait upstream hdcp timeout, but now not in hdmirx channel\n");
	}
}

static void hdmitx_init_gate_bit_mask(struct hdmitx_dev *hdev)
{
	if (!hdev)
		return;
	switch (hdev->tx_comm.tx_hw->chip_data->chip_type) {
	case MESON_CPU_ID_S5:
		hdev->tx21_hw.gate_bit_mask = 0xffc7f;
		break;
	case MESON_CPU_ID_S6:
		hdev->tx21_hw.gate_bit_mask = 0x01c7f;
		break;
	case MESON_CPU_ID_S7:
		hdev->tx21_hw.gate_bit_mask = 0x01c7f;
		break;
	case MESON_CPU_ID_S7D:
		hdev->tx21_hw.gate_bit_mask = 0x01c7f;
		break;
	default:
		hdev->tx21_hw.gate_bit_mask = 0;
		break;
	}
}

static void hdmitx_work_init(struct hdmitx_dev *hdev)
{
	struct hdmitx_common *tx_comm = &hdev->tx_comm;

	INIT_WORK(&hdev->tx_comm.work_hdr, hdr_work_func);
	INIT_WORK(&hdev->tx_comm.work_hdr_unmute, hdr_unmute_work_func);
	hdev->tx_comm.hdmi_hpd_wq = alloc_ordered_workqueue(DEVICE_NAME,
					WQ_HIGHPRI | __WQ_LEGACY | WQ_MEM_RECLAIM);
	/* for rx sense feature */
	hdev->tx_comm.rxsense_wq = alloc_workqueue("hdmitx_rxsense",
					   WQ_SYSFS | WQ_FREEZABLE, 0);
	INIT_DELAYED_WORK(&hdev->tx_comm.work_rxsense, hdmitx_rxsense_process);
	/* for cedst feature */
	hdev->tx_comm.cedst_wq = alloc_workqueue("hdmitx_cedst",
					 WQ_SYSFS | WQ_FREEZABLE, 0);
	INIT_DELAYED_WORK(&hdev->tx_comm.work_cedst, hdmitx_cedst_process);
	INIT_DELAYED_WORK(&tx_comm->work_hpd_plugin, hdmitx_hpd_plugin_irq_handler);
	INIT_DELAYED_WORK(&tx_comm->work_hpd_plugout, hdmitx_hpd_plugout_irq_handler);
	if (hdev->tx_comm.tx_hw->chip_data->chip_type < MESON_CPU_ID_T7) {
		INIT_DELAYED_WORK(&tx_comm->work_internal_intr, hdmitx_internal_intr_handler);
	} else {
		/* hdcp related work scheduled in system workqueue */
		INIT_DELAYED_WORK(&hdev->work_start_hdcp, hdmitx_start_hdcp_handler);
		INIT_DELAYED_WORK(&hdev->work_up_hdcp_timeout, hdmitx_up_hdcp_timeout_handler);
		/* for linux start hdcp */
		INIT_DELAYED_WORK(&hdev->work_drm_start_hdcp, drm_hdmitx_start_hdcp_handler);
		/* interrupt handler need to be scheduled in high priority */
		hdev->hdmi_intr_wq = alloc_workqueue("hdmitx_intr_wq",
			WQ_HIGHPRI | WQ_CPU_INTENSIVE, 0);
		INIT_DELAYED_WORK(&hdev->tx_comm.work_internal_intr, hdmitx_top_intr_handler);

		/* s5 init status check task */
		if (tx_comm->tx_hw->chip_data->chip_type == MESON_CPU_ID_S5)
			hdev->tx_comm.task = kthread_run(hdmitx21_status_check, (void *)hdev,
						"kthread_hdmist_check");
		/* enable analog frequency division by default on S6 */
		if (hdev->tx_comm.tx_hw->chip_data->chip_type == MESON_CPU_ID_S6)
			hdev->tx21_hw.clk_config = 1;
		hdmitx_init_gate_bit_mask(hdev);
	}
#ifdef CONFIG_AMLOGIC_LEGACY_EARLY_SUSPEND
	hdmitx_early_suspend_handler.param = hdev;
	register_early_suspend(&hdmitx_early_suspend_handler);
#endif
}

static void hdmitx_hdcp_init(struct hdmitx_dev *hdev)
{
	if (hdev->tx_comm.tx_hw->chip_data->chip_type < MESON_CPU_ID_T7)
		hdmitx20_hdcp_init(hdev);
	else
		hdmitx21_hdcp_init(hdev);
}

static void hdmitx21_vrr_init(struct hdmitx_dev *hdev)
{
	tx_vrr_params_init();
	hdmitx_register_vrr(hdev);
	hdmitx_register_drm_vrr_api(hdev);
}

static void hdmitx_vrr_init(struct hdmitx_dev *hdev)
{
	if (hdev->tx_comm.tx_hw->chip_data->chip_type >= MESON_CPU_ID_T7)
		hdmitx21_vrr_init(hdev);
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
	hdmitx_device = hdev;
	dev_set_drvdata(device, hdev);
	tx_comm = &hdev->tx_comm;
	ret = amhdmitx_get_dt_info(pdev, hdev);
	/* init tx_common */
	hdmitx_common_init(tx_comm, &hdev->hw_comm);
	amhdmitx_clktree_probe(device, hdev);
	r = alloc_chrdev_region(&hdev->tx_comm.hdmitx_id, 0, HDMI_TX_COUNT,
				DEVICE_NAME);
	cdev_init(&hdev->tx_comm.cdev, &amhdmitx_fops);
	hdev->tx_comm.cdev.owner = THIS_MODULE;
	r = cdev_add(&hdev->tx_comm.cdev, hdev->tx_comm.hdmitx_id, HDMI_TX_COUNT);

	hdmitx_class = class_create(DEVICE_NAME);
	if (IS_ERR(hdmitx_class)) {
		unregister_chrdev_region(hdev->tx_comm.hdmitx_id, HDMI_TX_COUNT);
		return -1;
	}
	/* kernel>=2.6.27 */
	dev = device_create(hdmitx_class, NULL, hdev->tx_comm.hdmitx_id, hdev,
			    "amhdmitx%d", MINOR(hdev->tx_comm.hdmitx_id));

	if (!dev) {
		HDMITX_ERROR("device_create create error\n");
		class_destroy(hdmitx_class);
		r = -EEXIST;
		return r;
	}
	hdev->tx_comm.hdtx_dev = dev;

#ifdef CONFIG_AMLOGIC_VPU
	if (tx_comm->tx_hw->chip_data->chip_type <	MESON_CPU_ID_T7) {
		tx_comm->encp_vpu_dev = vpu_dev_register(VPU_VENCP, DEVICE_NAME);
		tx_comm->enci_vpu_dev = vpu_dev_register(VPU_VENCI, DEVICE_NAME);
		/* vpu gate/mem ctrl for hdmitx, since TM2B */
		tx_comm->hdmi_vpu_dev = vpu_dev_register(VPU_HDMI, DEVICE_NAME);
	}
#endif
	/* platform related functions */
	tx_uevent_mgr = hdmitx_event_mgr_create(pdev, hdev->tx_comm.hdtx_dev);
	hdmitx_event_mgr_suspend(tx_uevent_mgr, false);
	hdmitx_common_attch_platform_data(tx_comm,
		HDMITX_PLATFORM_UEVENT, tx_uevent_mgr);
	tx_tracer = hdmitx_tracer_create(tx_uevent_mgr);
	hdmitx_common_attch_platform_data(tx_comm,
		HDMITX_PLATFORM_TRACER, tx_tracer);
	hdev->tx_comm.reboot_nb.notifier_call = hdmitx_reboot_notifier;
	register_reboot_notifier(&hdev->tx_comm.reboot_nb);
	/* init hw */
	hdmitx_meson_init(hdev);
	hdmitx_vout_init(tx_comm, &hdev->hw_comm);
	hdmitx_hdr_init(tx_comm);
	hdmitx_packet_init(tx_comm);
	/* load init audio fmt for HW info */
	hdmitx_audio_init(tx_comm, tx_tracer);
#if IS_ENABLED(CONFIG_AMLOGIC_SND_SOC)
	tx_comm->hdmitx_notifier_nb_a = hdmitx_notifier_nb_a;
	if (!hdev->tx_comm.pxp_mode)
		aout_register_client(&tx_comm->hdmitx_notifier_nb_a);
#endif
	/* get efuse ctrl state */
	get_hdmi_efuse(tx_comm);
	spin_lock_init(&hdev->tx_comm.edid_spinlock);
	/* init work and delay work */
	hdmitx_work_init(hdev);

	set_hdcp_common_instance(&hdev->tx_comm);
	hdmitx_hdcp_init(hdev);
	/* bind drm before hdmi event */
	hdmitx_bind_meson_drm(&pdev->dev, &hdev->tx_comm, &hdev->hw_comm);
	/* init power_uevent state */
	hdmitx_set_uevent(&hdev->tx_comm, HDMITX_HDCPPWR_EVENT, HDMI_WAKEUP);
	/* reset EDID/vinfo */
	if (!hdev->tx_comm.forced_edid) {
		hdmitx_edid_buffer_clear(hdev->tx_comm.EDID_buf, sizeof(hdev->tx_comm.EDID_buf));
		hdmitx_edid_rxcap_clear(&hdev->tx_comm.rxcap);
	}
	/*
	 * hpd process of bootup stage, need to be done in probe
	 * as other client modules may need the hpd/edid info.
	 * use mutex to prevent hpd irq bottom half concurrency.
	 */
	mutex_lock(&hdev->tx_comm.hdmimode_mutex);
	/* enable irq firstly before any hpd handler to prevent missing irq. */
	hdev->hw_comm.setupirq(&hdev->hw_comm);
	/* actions in top half of plug intr */
	hpd_state = !!hdmitx_hw_cntl_misc(&hdev->hw_comm,
		MISC_HPD_GPI_ST, 0);
	hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_HPD_IRQ_TOP_HALF, hpd_state);
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
	hdmitx_hdr_state_init(tx_comm);
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
	/* init vrr */
	hdmitx_vrr_init(hdev);
#endif
	/* after unlock, now can take actions of bottom half of hpd irq */
	mutex_unlock(&hdev->tx_comm.hdmimode_mutex);
	/* notify to drm hdmi */
	hdmitx_fire_drm_hpd_cb_unlocked(&hdev->tx_comm);

	if (tx_comm->tx_hw->chip_data->chip_type >= MESON_CPU_ID_T7)
		tx_comm->hdmi_init = HDMITX21;
	else
		tx_comm->hdmi_init = HDMITX20;
	/* everything is ready, create sysfs here */
	hdmitx_sysfs_common_create(dev, &hdev->tx_comm, &hdev->hw_comm);
	hdmitx_common_debugfs_init(hdev);
	hdmitx_common_profs_init(hdev);
	HDMITX_INFO("amhdmitx_probe_end\n");

	return r;
}

int get_hdmitx_init(void)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	if (hdev)
		return hdev->tx_comm.hdmi_init;
	return 0;
}

static void hdmitx_pre_display_init(struct hdmitx_dev *hdev)
{
	u8 update_flags = 0;

	if (hdev->tx_comm.rxcap.max_frl_rate > FRL_NONE &&
		hdev->tx_comm.rxcap.scdc_present == 1 &&
		/* hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_GET_FRL_MODE, 0)&& */
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
		hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_TMDS_PHY_OP, TMDS_PHY_DISABLE);
	} else if (hdev->tx_comm.rxcap.max_frl_rate > FRL_NONE &&
		hdev->tx_comm.rxcap.scdc_present == 1 &&
		hdev->frl_rate > FRL_NONE &&
		hdev->frl_rate < FRL_RATE_MAX &&
		hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_GET_FRL_MODE, 0) == FRL_NONE) {
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
		hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_TMDS_PHY_OP, TMDS_PHY_DISABLE);
	} else if (hdev->frl_rate == FRL_NONE &&
		hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_GET_FRL_MODE, 0) == FRL_NONE) {
		/*
		 * for cases switch between TMDS modes
		 * per hdmi2.1 spec chapter 6.1.3.2, when source change
		 * tmds_bit_clk_ratio, should follow the steps:
		 * 1.disable tmds clk/data
		 * 2.change tmds_bit_clk_ratio 0 <-> 1
		 * 3.resume tmds clk/data transmission in 1~100 ms
		 */
		hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_TMDS_PHY_OP, TMDS_PHY_DISABLE);
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

int hdmitx_pre_enable_mode(struct hdmitx_common *tx_comm, struct hdmi_format_para *para)
{
	struct hdmitx_dev *hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);
#ifdef CONFIG_AMLOGIC_DSC
	struct dsc_notifier_data_s dsc_notifier_data;
	int ret = -1;
#endif
	enum frl_rate_enum source_test_frl_rate = FRL_NONE;

	/* older chips do not require this step, return pass */
	if (tx_comm->tx_hw->chip_data->chip_type < MESON_CPU_ID_T7)
		return 0;

	/*
	 * disable hdcp before set mode if hdcp enabled.
	 * normally hdcp is disabled before setting mode
	 * when disable phy, but for special case of bootup,
	 * if mode changed as it's different with uboot mode,
	 * hdcp is not stopped firstly, and may hdcp fail
	 */
	if (!hdcp_need_control_by_upstream(&hdev->hw_comm))
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

static void restore_mute(void)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();
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

int hdmitx_enable_mode(struct hdmitx_common *tx_comm, struct hdmi_format_para *para)
{
	int ret;
	struct hdmitx_dev *hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);

	/* if vic is HDMI_UNKNOWN, hdmitx_set_display will disable HDMI */
	if (tx_comm->tx_hw->chip_data->chip_type < MESON_CPU_ID_T7)
		ret = hdmitx_set_display(hdev, para->vic);
	else
		ret = hdmitx21_set_display(hdev, para->vic);

	if (tx_comm->tx_hw->chip_data->chip_type < MESON_CPU_ID_T7) {
		hdmitx20_set_audio(hdev, &hdev->tx_comm.cur_audio_param);
	} else {
		if (ret >= 0)
			restore_mute();
		hdmitx21_set_audio(hdev, &hdev->tx_comm.cur_audio_param);
	}

	return 0;
}

int hdmitx_post_enable_mode(struct hdmitx_common *tx_comm, struct hdmi_format_para *para)
{
	struct hdmitx_dev *hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);
	struct vinfo_s *vinfo = NULL;

	if (tx_comm->tx_hw->chip_data->chip_type < MESON_CPU_ID_T7)
		return 0;
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

int hdmitx_disable_mode(struct hdmitx_common *tx_comm, struct hdmi_format_para *para)
{
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
	struct hdmitx_dev *hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);

	hdmitx_unregister_vrr(hdev);
#endif
	return 0;
}

/*
 * for game console-> hdmirx -> hdmitx -> TV
 * interface for hdmirx module
 * ret: false if not update, true if updated
 */
bool hdmitx_update_latency_info(struct tvin_latency_s *latency_info)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();
	struct hdmitx_common *tx_comm = &hdev->tx_comm;
	bool it_content = false;

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
				hdmitx_hw_cntl_config(&hdev->hw_comm, CONF_VIDEO_MUTE_OP,
					VIDEO_MUTE);
			}
			tx_comm->allm_mode = 1;
			HDMITX_INFO("%s: enabling ALLM\n", __func__);
			hdmitx_common_setup_vsif_packet(tx_comm, VT_ALLM, 1, NULL);
			tx_comm->ct_mode = 0;
			tx_comm->it_content = 0;
			hdmitx_hw_cntl_config(&hdev->hw_comm, CONF_CT_MODE, SET_CT_OFF);
		}
	} else {
		if (!get_rx_active_sts()) {
			video_mute = true;
			/* hdmitx21_video_mute_op(0, VIDEO_MUTE_PATH_4); */
			hdmitx_hw_cntl_config(&hdev->hw_comm, CONF_VIDEO_MUTE_OP, VIDEO_MUTE);
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
			hdmitx_hw_cntl_config(&hdev->hw_comm, CONF_CT_MODE,
				SET_CT_GAME | it_content << 4);
		} else if (tx_comm->rxcap.cnc0 && latency_info->cn_type == GRAPHICS &&
		    latency_info->it_content == 1) {
			tx_comm->ct_mode = 2;
			HDMITX_INFO("%s: enabling GRAPHICS mode\n", __func__);
			hdmitx_hw_cntl_config(&hdev->hw_comm, CONF_CT_MODE,
				SET_CT_GRAPHICS | it_content << 4);
		} else if (tx_comm->rxcap.cnc1 && latency_info->cn_type == PHOTO) {
			tx_comm->ct_mode = 3;
			HDMITX_INFO("%s: enabling PHOTO mode\n", __func__);
			hdmitx_hw_cntl_config(&hdev->hw_comm, CONF_CT_MODE,
				SET_CT_PHOTO | it_content << 4);
		} else if (tx_comm->rxcap.cnc2 && latency_info->cn_type == CINEMA) {
			tx_comm->ct_mode = 4;
			HDMITX_INFO("%s: enabling CINEMA mode\n", __func__);
			hdmitx_hw_cntl_config(&hdev->hw_comm, CONF_CT_MODE,
				SET_CT_CINEMA | it_content << 4);
		} else {
			tx_comm->ct_mode = 0;
			HDMITX_INFO("%s: No GAME or CT mode\n", __func__);
			hdmitx_hw_cntl_config(&hdev->hw_comm, CONF_CT_MODE,
				SET_CT_OFF | it_content << 4);
		}
	}
	return true;
}
EXPORT_SYMBOL(hdmitx_update_latency_info);

void hdmitx_clear_packets(struct hdmitx_common *tx_comm)
{
	hdmitx_clear_all_infoframe_pkt(tx_comm);
}

/* action in suspend/plugout/disable_module(switch mode) */
void hdmitx_disable_hdcp(struct hdmitx_common *tx_comm)
{
	struct hdmitx_hw_common *tx_hw_base = tx_comm->tx_hw;
	struct hdmitx_dev *hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);

	if (tx_comm->tx_hw->chip_data->chip_type < MESON_CPU_ID_T7) {
		/* HW: mux to hdcp14 & hdcp14 off, DDC free */
		hdmitx_hw_cntl_ddc(tx_hw_base, DDC_HDCP_MUX_INIT, 1);
		hdmitx_hw_cntl_ddc(tx_hw_base, DDC_HDCP_OP, HDCP14_OFF);
		tx_comm->hdcp_mode = 0;
		tx_comm->hdcp_bcaps_repeater = 0;
	} else {
		hdmitx21_disable_hdcp(hdev);
	}
}

void hdmitx_hdcp_exit(struct hdmitx_dev *hdev)
{
	if (hdev->tx_comm.tx_hw->chip_data->chip_type < MESON_CPU_ID_T7)
		hdmitx20_hdcp_exit(hdev);
	else
		hdmitx21_hdcp_exit(hdev);
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

void hdmitx_common_sw_debugfunc(struct hdmitx_common *tx_comm, const char *buf)
{
	char tmpbuf[64];
	char *tmpbuf2;
	int i = 0;
	int ret;
	unsigned long value = 0;
	struct ced_cnt *ced = &tx_comm->ced_cnt;
	struct scdc_locked_st *ch_st = &tx_comm->chlocked_st;
	enum frl_rate_enum frl_rate;
	struct hdmitx_dev *hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);
	struct hdmitx_hw_common *tx_hw = tx_comm->tx_hw;
	struct master_display_info_s data = {0};
	struct hdr10plus_para hdr_data = {0x1, 0x2, 0x3};
	struct cuva_hdr_vs_emds_para cuva_data = {0x1, 0x2, 0x3};
	struct dv_vsif_para vsif_para = {0};

	while ((buf[i]) && (buf[i] != ',') && (buf[i] != ' ')) {
		tmpbuf[i] = buf[i];
		i++;
	}
	tmpbuf[i] = 0;

	if (strncmp(tmpbuf, "edid_check", 10) == 0) {
		if (strncmp(tmpbuf + 10, "=0", 2) == 0)
			tx_comm->rxcap.edid_check = 0;
		else if (strncmp(tmpbuf + 10, "=1", 2) == 0)
			tx_comm->rxcap.edid_check = 1;
		else if (strncmp(tmpbuf + 10, "=2", 2) == 0)
			tx_comm->rxcap.edid_check = 2;
		else if (strncmp(tmpbuf + 10, "=3", 2) == 0)
			tx_comm->rxcap.edid_check = 3;
		HDMITX_INFO("edid_check = %d\n", tx_comm->rxcap.edid_check);
		return;
	} else if (strncmp(tmpbuf, "chkfmt", 6) == 0) {
		hdmitx_mode_print_all_mode_table();
		return;
	} else if (strncmp(tmpbuf, "hdcp_mode", 9) == 0) {
		ret = kstrtoul(tmpbuf + 9, 16, &value);
		if (ret == 0 && value <= 2)
			hdev->tx_comm.drm_hdcp.test_hdcp_mode = value - 0;
		HDMITX_INFO("test drm_hdcp_mode: %d\n", hdev->tx_comm.drm_hdcp.test_hdcp_mode);
	} else if (strncmp(tmpbuf, "drm_hdcp_op", 11) == 0) {
		ret = kstrtoul(tmpbuf + 11, 16, &value);
		if (value == 0 && hdev->tx_comm.drm_hdcp.test_hdcp_disable)
			hdev->tx_comm.drm_hdcp.test_hdcp_disable();
		else if (value == 1 && tx_comm->drm_hdcp.test_hdcp_enable)
			tx_comm->drm_hdcp.test_hdcp_enable(tx_comm->drm_hdcp.test_hdcp_mode);
		else if (value == 2 && hdev->tx_comm.drm_hdcp.test_hdcp_disconnect)
			hdev->tx_comm.drm_hdcp.test_hdcp_disconnect();
	} else if (strncmp(tmpbuf, "drm_hdcp_ver", 12) == 0) {
		HDMITX_INFO("test drm_hdcp_ver: %d\n", drm_hdmitx_common_get_rx_hdcp_cap());
	} else if (strncmp(tmpbuf, "hdcp_result", 11) == 0) {
		if (tx_comm->tx_hw->chip_data->chip_type < MESON_CPU_ID_T7) {
			HDMITX_INFO("hdcp result: hdcp22: %d topo: %d, hdcp14: %d\n",
			hdmitx_hdcp_opr(7), hdmitx_hdcp_opr(0xe), hdmitx_hdcp_opr(2));
		} else {
			HDMITX_INFO("hdcp filtered result: hdcp22: %d topo: %d, hdcp14: %d\n",
			get_hdcp2_result(), get_hdcp2_topo(), get_hdcp1_result());
		}
	} else if (strncmp(tmpbuf, "avmute_frame", 12) == 0) {
		ret = kstrtoul(tmpbuf + 12, 10, &value);
		hdev->tx_comm.debug_param.avmute_frame = value;
		HDMITX_INFO("avmute_frame = %lu\n", value);
	} else if (strncmp(tmpbuf, "config_csc_en", 13) == 0) {
		ret = kstrtoul(tmpbuf + 13, 0, &value);
		HDMITX_INFO("config_csc_en to %lu\n", value);
		if (value == 0)
			hdev->tx_comm.config_csc_en = false;
		else if (value == 1)
			hdev->tx_comm.config_csc_en = true;
	} else if (strncmp(tmpbuf, "edid_parse", 10) == 0) {
		if (tmpbuf[10] == '1') {
			/*
			 * Enable edid parse in hdmitx debug function command
			 * echo edid_parse1 > /sys/class/amhdmitx/amhdmitx0/debug
			 */
			hdev->tx_comm.edid_parse_in_hdmitx = true;
			HDMITX_INFO("edid_parse_in_hdmitx = %d\n",
					hdev->tx_comm.edid_parse_in_hdmitx);
		} else if (tmpbuf[10] == '0') {
			/*
			 * Disable edid parse in hdmitx debug function command
			 * echo edid_parse0 > /sys/class/amhdmitx/amhdmitx0/debug
			 */
			hdev->tx_comm.edid_parse_in_hdmitx = false;
			HDMITX_INFO("edid_parse_in_hdmitx = %d\n",
					hdev->tx_comm.edid_parse_in_hdmitx);
		}
	} else if (strncmp(tmpbuf, "vinfo", 5) == 0) {
		ret = kstrtoul(tmpbuf + 5, 10, &value);
		if (value ==  0) {
			edidinfo_detach_to_vinfo(&hdev->tx_comm);
			HDMITX_INFO("detach vinfo\n");
		} else if (value ==  1) {
			edidinfo_attach_to_vinfo(&hdev->tx_comm);
			HDMITX_INFO("attach vinfo\n");
		}
	} else if (strncmp(tmpbuf, "cedst_count", 11) == 0) {
		frl_rate = hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_GET_FRL_MODE, 0);
		if (!frl_rate && !ch_st->clock_detected)
			HDMITX_INFO("clock undetected\n");
		if (!ch_st->ch0_locked)
			HDMITX_INFO("CH0 unlocked\n");
		if (!ch_st->ch1_locked)
			HDMITX_INFO("CH1 unlocked\n");
		if (!ch_st->ch2_locked)
			HDMITX_INFO("CH2 unlocked\n");
		if (frl_rate && !ch_st->ch3_locked)
			HDMITX_INFO("CH3 unlocked\n");
		if (ced->ch0_valid && ced->ch0_cnt)
			HDMITX_INFO("CH0 ErrCnt 0x%x\n", ced->ch0_cnt);
		if (ced->ch1_valid && ced->ch1_cnt)
			HDMITX_INFO("CH1 ErrCnt 0x%x\n", ced->ch1_cnt);
		if (ced->ch2_valid && ced->ch2_cnt)
			HDMITX_INFO("CH2 ErrCnt 0x%x\n", ced->ch2_cnt);
		if (frl_rate >= FRL_6G4L && ced->ch3_valid && ced->ch3_cnt)
			HDMITX_INFO("CH3 ErrCnt 0x%x\n", ced->ch3_cnt);
		if (frl_rate && ced->rs_c_valid && ced->rs_c_cnt)
			HDMITX_INFO("RSCC ErrCnt 0x%x\n", ced->rs_c_cnt);
		memset(ced, 0, sizeof(*ced));
	} else if (strncmp(tmpbuf, "fake_plug", 9) == 0) {
		HDMITX_INFO("fake plug %s\n", tmpbuf);
		if (tmpbuf[9] == '1' || tmpbuf[9] == '0') {
			ret = kstrtoul(tmpbuf + 9, 10, &value);
			tx_comm->hpd_state = value;
			hdmitx_common_notify_hpd_status(tx_comm, false);
			/* notify to drm hdmi */
			hdmitx_fire_drm_hpd_cb_unlocked(tx_comm);
		}
		HDMITX_INFO("hpd_state: %d\n", tx_comm->hpd_state);
	} else if (strncmp(tmpbuf, "hdr_mute_frame", 14) == 0) {
		if (kstrtoul(tmpbuf + 14, 10, &value) == 0)
			tx_comm->hdr_mute_frame = value;
		HDMITX_INFO("hdr_mute_frame: %d\n", tx_comm->hdr_mute_frame);
		return;
	} else if (strncmp(tmpbuf, "is_hdcp_cts_te", 14) == 0) {
		/* is hdcp cts test equipment */
		HDMITX_INFO("%d\n", hdmitx_edid_only_support_sd(&tx_comm->rxcap));
	} else if (strncmp(tmpbuf, "config", 6) == 0) {
		tmpbuf2 = tmpbuf + 6;
		HDMITX_INFO("config: %s\n", tmpbuf2);

		if (strncmp(tmpbuf2, "info", 4) == 0) {
			HDMITX_INFO("%x %x %x %x %x %x\n",
				hdmitx_hw_get_hdr_st(tx_hw),
				hdmitx_hw_get_dv_st(tx_hw),
				hdmitx_hw_get_hdr10p_st(tx_hw),
				hdmitx_hdr_en(tx_hw),
				hdmitx_dv_en(tx_hw),
				hdmitx_hdr10p_en(tx_hw)
				);
		} else if (strncmp(tmpbuf2, "sdr_hdr_dov", 11) == 0) {
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
		} else if (strncmp(tmpbuf2, "sdr", 3) == 0) {
			data.features = 0x00010100;
			hdmitx_set_drm_pkt(&data);
		} else if (strncmp(tmpbuf2, "hdr", 3) == 0) {
			data.features = 0x00091000;
			hdmitx_set_drm_pkt(&data);
		} else if (strncmp(tmpbuf2, "sbtm", 4) == 0) {
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
		} else if (strncmp(tmpbuf2, "hlg", 3) == 0) {
			data.features = 0x00091200;
			hdmitx_set_drm_pkt(&data);
		} else if (strncmp(tmpbuf2, "vsif", 4) == 0) {
			if (tmpbuf2[4] == '1' && tmpbuf2[5] == '1') {
				/* DV STD */
				vsif_para.ver = 0x1;
				vsif_para.length = 0x1b;
				vsif_para.ver2_l11_flag = 0;
				vsif_para.vers.ver2.low_latency = 0;
				vsif_para.vers.ver2.dobly_vision_signal = 1;
				hdmitx_set_vsif_pkt(1, 1, &vsif_para, false);
			} else if (tmpbuf2[4] == '1' && tmpbuf2[5] == '0') {
				/* DV STD packet, but dolby_vision_signal bit cleared */
				vsif_para.ver = 0x1;
				vsif_para.length = 0x1b;
				vsif_para.ver2_l11_flag = 0;
				vsif_para.vers.ver2.low_latency = 0;
				vsif_para.vers.ver2.dobly_vision_signal = 0;
				hdmitx_set_vsif_pkt(1, 1, &vsif_para, false);
			} else if (tmpbuf2[4] == '4' && tmpbuf2[5] == '1') {
				/* DV LL */
				vsif_para.ver = 0x1;
				vsif_para.length = 0x1b;
				vsif_para.ver2_l11_flag = 0;
				vsif_para.vers.ver2.low_latency = 1;
				vsif_para.vers.ver2.dobly_vision_signal = 1;
				hdmitx_set_vsif_pkt(4, 0, &vsif_para, false);
			}  else if (tmpbuf2[4] == '4' && tmpbuf2[5] == '0') {
				/* DV LL packet, but dolby_vision_signal bit cleared */
				vsif_para.ver = 0x1;
				vsif_para.length = 0x1b;
				vsif_para.ver2_l11_flag = 0;
				vsif_para.vers.ver2.low_latency = 1;
				vsif_para.vers.ver2.dobly_vision_signal = 0;
				hdmitx_set_vsif_pkt(4, 0, &vsif_para, false);
			} else if (tmpbuf2[4] == '0') {
				/* exit DV to SDR */
				hdmitx_set_vsif_pkt(0, 0, NULL, true);
			}
		} else if (strncmp(tmpbuf2, "hdr10+", 6) == 0) {
			hdmitx_set_hdr10plus_pkt(1, &hdr_data);
		} else if (strncmp(tmpbuf2, "cuva", 4) == 0) {
			hdmitx_set_cuva_hdr_vs_emds(&cuva_data);
		}
	}

}

void hdmitx20_sw_debugfunc(struct hdmitx_common *tx_comm, const char *buf)
{
	char tmpbuf[128];
	int i = 0;
	int ret = 0;
	unsigned long value = 0;
	struct hdmitx_dev *hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);

	while ((buf[i]) && (buf[i] != ',') && (buf[i] != ' ')) {
		tmpbuf[i] = buf[i];
		i++;
	}
	tmpbuf[i] = 0;

	if (strncmp(tmpbuf, "hpd_stick", 9) == 0) {
		if (tmpbuf[9] == '1')
			hdev->hdcp_hpd_stick = 1;
		else
			hdev->hdcp_hpd_stick = 0;
		HDMITX_DEBUG_HPD("%sstick hpd\n",
			(hdev->hdcp_hpd_stick) ? "" : "un");
	} else if (strncmp(tmpbuf, "drm_set_hdmi", 12) == 0) {
		if (hdev->tx_comm.drm_hdcp.test_set_hdmi_mode)
			hdev->tx_comm.drm_hdcp.test_set_hdmi_mode();
	} else if (strncmp(tmpbuf, "drm_set_out", 11) == 0) {
		if (hdev->tx_comm.drm_hdcp.test_set_out_mode)
			hdev->tx_comm.drm_hdcp.test_set_out_mode();
	} else if (strncmp(tmpbuf, "hdcp_max_exceed", 15) == 0) {
		HDMITX_INFO("%d", hdev->hdcp_max_exceed_state);
	} else if (strncmp(tmpbuf, "max_exceed", 10) == 0) {
		ret = kstrtoul(tmpbuf + 10, 10, &value);
		hdev->max_exceed = value;
		HDMITX_INFO("%d", hdev->max_exceed);
	}
}

void hdmitx21_sw_debugfunc(struct hdmitx_common *tx_comm, const char *buf)
{
	char tmpbuf[128];
	int i = 0;
	int ret;
	unsigned long value = 0;
	struct hdmitx_dev *hdev = container_of(tx_comm, struct hdmitx_dev, tx_comm);
	struct hdcp_t *p_hdcp = (struct hdcp_t *)hdev->am_hdcp;

	while ((buf[i]) && (buf[i] != ',') && (buf[i] != ' ')) {
		tmpbuf[i] = buf[i];
		i++;
	}
	tmpbuf[i] = 0;

	if (strncmp(tmpbuf, "maskqms", 7) == 0) {
		if (tmpbuf[7] == '1')
			hdev->edid_mask_qms = 1;
		if (tmpbuf[7] == '0')
			hdev->edid_mask_qms = 0;
		HDMITX_INFO("mask edid qms %d\n", hdev->edid_mask_qms);
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
	} else if (strncmp(tmpbuf, "frl", 3) == 0) {
		if (strncmp(tmpbuf + 3, "stop", 4) == 0) {
			frl_tx_stop();
			return;
		}
		hdev->frl_rate = tmpbuf[3] - '0';
		frl_tx_training_handler(hdev);
		return;
#endif
	} else if (strncmp(tmpbuf, "reauth_dbg", 10) == 0) {
		ret = kstrtoul(tmpbuf + 10, 10, &value);
		hdcp_reauth_dbg = value;
		HDMITX_INFO("set hdcp_reauth_dbg :%lu\n", hdcp_reauth_dbg);
	} else if (strncmp(tmpbuf, "streamtype_dbg", 14) == 0) {
		ret = kstrtoul(tmpbuf + 14, 10, &value);
		streamtype_dbg = value;
		HDMITX_INFO("set streamtype_dbg :%lu\n", streamtype_dbg);
	} else if (strncmp(tmpbuf, "en_fake_rcv_id", 14) == 0) {
		ret = kstrtoul(tmpbuf + 14, 10, &value);
		en_fake_rcv_id = value;
		HDMITX_INFO("set en_fake_rcv_id :%lu\n", en_fake_rcv_id);
	} else if (strncmp(tmpbuf, "aud_mute", 8) == 0) {
		ret = kstrtoul(tmpbuf + 8, 10, &value);
		hdmitx_ext_set_audio_output(value);
		HDMITX_INFO("aud_mute :%lu\n", value);
	} else if (strncmp(tmpbuf, "hdcp_timeout", 12) == 0) {
		ret = kstrtoul(tmpbuf + 12, 10, &value);
		hdev->up_hdcp_timeout_sec = value;
	} else if (strncmp(tmpbuf, "avmute_ms", 10) == 0) {
		ret = kstrtoul(tmpbuf + 10, 10, &value);
		avmute_ms = value;
		HDMITX_INFO("set avmute_ms :%lu\n", avmute_ms);
	} else if (strncmp(tmpbuf, "vid_mute_ms", 10) == 0) {
		ret = kstrtoul(tmpbuf + 10, 10, &value);
		vid_mute_ms = value;
		HDMITX_INFO("set vid_mute_ms :%lu\n", vid_mute_ms);
	} else if (strncmp(tmpbuf, "get_output_mute", 15) == 0) {
		HDMITX_INFO("VPP output mute :%d\n", get_output_mute());
	} else if (strncmp(tmpbuf, "set_output_mute", 15) == 0) {
		ret = kstrtoul(tmpbuf + 15, 10, &value);
		set_output_mute(!!value);
		HDMITX_INFO("set VPP output mute :%d\n", !!value);
	} else if (strncmp(tmpbuf, "hdcp_reauth", 11) == 0) {
		ret = kstrtoul(tmpbuf + 11, 16, &value);
		if (ret != 0)
			return;
		HDMITX_INFO("hdcp reauth: %lu\n", value);
		hdmitx_reauth_request(value & 0xFF);
	} else if (strncmp(tmpbuf, "propagate_stream_type", 21) == 0) {
		if (p_hdcp->ds_repeater && p_hdcp->hdcp_type == HDCP_VER_HDCP2X) {
			HDMITX_INFO("%d\n", p_hdcp->csm_message.streamid_type & 0xFF);
		} else {
			HDMITX_INFO("no stream type, as ds_repeater: %d, hdcp_type: %d\n",
				p_hdcp->ds_repeater, p_hdcp->hdcp_type);
		}
	} else if (strncmp(tmpbuf, "cont_smng_method", 16) == 0) {
		/*
		 * content stream management update method:
		 * when upstream side received new content stream
		 * management message, there're two method to
		 * update content stream type that propagated
		 * to downstream hdcp2.3 repeater:
		 * 0(default): only send content stream management
		 * message with new stream type to downstream
		 * 1: init new re-auth with downstream
		 */
		ret = kstrtoul(tmpbuf + 16, 10, &value);
		if (value == 0 || value == 1) {
			HDMITX_INFO("set cont_smng_method as %d\n", value);
			p_hdcp->cont_smng_method = value;
		} else {
			HDMITX_INFO("current cont_smng_method: %d\n", p_hdcp->cont_smng_method);
		}
	} else if (strncmp(tmpbuf, "hdcp_delay", 10) == 0) {
		ret = kstrtoul(tmpbuf + 10, 10, &value);
		HDMITX_INFO("hdcp_delay :%d\n", value);
		hdev->hdcp_debug_delay = value;
	} else if (strncmp(tmpbuf, "vrr_mode", 8) == 0) {
		switch (hdev->vrr_mode) {
		case T_VRR_GAME:
			HDMITX_INFO("%s\n", "game-vrr");
			break;
		case T_VRR_QMS:
			HDMITX_INFO("%s\n", "qms-vrr");
			break;
		default:
			HDMITX_INFO("%s\n", "none");
			break;
		}
	} else if (strncmp(tmpbuf, "frl_rate", 8) == 0) {
		/* forced FRL rate setting */
		if (strncmp(&tmpbuf[8], "_w", 2) == 0) {
			ret = kstrtoul(tmpbuf + 10, 10, &value);
			if (!hdev->tx_comm.rxcap.max_frl_rate)
				HDMITX_INFO("rx not support FRL\n");
			if (value > FRL_12G4L) {
				HDMITX_ERROR("set frl_rate in 0 ~ 6\n");
				return;
			}
			hdev->manual_frl_rate = value;
			HDMITX_INFO("set tx frl_rate as %d\n", value);
			if (hdev->manual_frl_rate > hdev->tx_comm.rxcap.max_frl_rate)
				HDMITX_INFO("larger than rx max_frl_rate %d\n",
					hdev->tx_comm.rxcap.max_frl_rate);
		} else if (strncmp(&tmpbuf[8], "_r", 2) == 0) {
			HDMITX_INFO("%d\n", hdev->frl_rate);
			switch (hdev->frl_rate) {
			case FRL_3G3L:
				HDMITX_INFO("FRL_3G3L\n");
				break;
			case FRL_6G3L:
				HDMITX_INFO("FRL_6G3L\n");
				break;
			case FRL_6G4L:
				HDMITX_INFO("FRL_6G4L\n");
				break;
			case FRL_8G4L:
				HDMITX_INFO("FRL_8G4L\n");
				break;
			case FRL_10G4L:
				HDMITX_INFO("FRL_10G4L\n");
				break;
			case FRL_12G4L:
				HDMITX_INFO("FRL_12G4L\n");
				break;
			default:
				break;
			}
		}
	} else if (strncmp(tmpbuf, "dsc_en", 6) == 0) {
		HDMITX_INFO("dsc enable: %d\n", hdev->dsc_en);
	} else if (strncmp(tmpbuf, "dsc_policy", 10) == 0) {
		/*
		 * dsc_policy:
		 * 0 automatically enable dsc if necessary, but not support 8k Y444/rgb,12bit
		 * or 4k100/120hz Y444/rgb,12bit
		 * 1 force enable dsc for mode that can be supported both with dsc/non-dsc
		 * 2 force enable dsc for mode test for new dsc mode(debug only)
		 * 3 forcely filter out dsc mode output by valid_mode_check
		 * 4 automatically enable dsc if necessary, include those mentioned in 0
		 */
		if (strncmp(&tmpbuf[10], "_w", 2) == 0) {
			ret = kstrtoul(tmpbuf + 12, 10, &value);
			if (value != 0 && value != 1 && value != 2 && value != 3 && value != 4) {
				HDMITX_INFO("set dsc_policy in 0~4\n");
				return;
			}
			hdev->hw_comm.hdmi_tx_cap.dsc_policy = value;
			HDMITX_INFO("set dsc_policy as %d\n", value);
		} else if (strncmp(&tmpbuf[10], "_r", 2) == 0) {
			HDMITX_INFO("%d\n", hdev->hw_comm.hdmi_tx_cap.dsc_policy);
		}
	} else if (strncmp(tmpbuf, "vrr_dbg_vframe", 14) == 0) {
		ret = kstrtoul(tmpbuf + 14, 10, &value);
		hdev->vrr_dbg_vframe = value;
		HDMITX_INFO("vrr_dbg_vframe :%d\n", hdev->vrr_dbg_vframe);
	} else if (strncmp(tmpbuf, "emp_no", 6) == 0) {
		ret = kstrtoul(tmpbuf + 6, 10, &value);
		hdev->emp_no = value;
		HDMITX_INFO("emp_no :%d\n", hdev->emp_no);
	} else if (strncmp(tmpbuf, "emp_verbose", 11) == 0) {
		ret = kstrtoul(tmpbuf + 11, 10, &value);
		hdev->emp_verbose = value;
		HDMITX_INFO("emp_verbose :%d\n", hdev->emp_verbose);
	}
}

static void amhdmitx_remove(struct platform_device *pdev)
{
	struct hdmitx_dev *hdev = dev_get_drvdata(&pdev->dev);
	struct device *dev = hdev->tx_comm.hdtx_dev;

	/* remove sysfs before uninit */
	hdmitx_sysfs_common_destroy(dev);

	/* unbind from drm */
	hdmitx_unbind_meson_drm(&pdev->dev, &hdev->tx_comm, &hdev->hw_comm);

	cancel_work_sync(&hdev->tx_comm.work_hdr);
	cancel_work_sync(&hdev->tx_comm.work_hdr_unmute);
	hdmitx_hdcp_exit(hdev);
	cancel_delayed_work(&hdev->tx_comm.work_hpd_plugout);
	cancel_delayed_work(&hdev->tx_comm.work_hpd_plugin);
	cancel_delayed_work(&hdev->tx_comm.work_internal_intr);
	destroy_workqueue(hdev->tx_comm.hdmi_hpd_wq);
	cancel_delayed_work(&hdev->tx_comm.work_rxsense);
	destroy_workqueue(hdev->tx_comm.rxsense_wq);
	cancel_delayed_work(&hdev->tx_comm.work_cedst);
	destroy_workqueue(hdev->tx_comm.cedst_wq);

	if (hdev->tx_comm.tx_hw->chip_data->chip_type >= MESON_CPU_ID_T7) {
		tee_comm_dev_unreg(hdev);
		destroy_workqueue(hdev->hdmi_intr_wq);
	}
	if (hdev->hw_comm.uninit)
		hdev->hw_comm.uninit(&hdev->hw_comm);
	hdev->tx_comm.hpd_event = 0xff;
	if (hdev->tx_comm.tx_hw->chip_data->chip_type == MESON_CPU_ID_S5)
		kthread_stop(hdev->tx_comm.task);
	hdmitx_vout_uninit();

#if IS_ENABLED(CONFIG_AMLOGIC_SND_SOC)
	aout_unregister_client(&hdev->tx_comm.hdmitx_notifier_nb_a);
#endif

	/* Remove the cdev */
	cdev_del(&hdev->tx_comm.cdev);
	device_destroy(hdmitx_class, hdev->tx_comm.hdmitx_id);
	class_destroy(hdmitx_class);

	unregister_chrdev_region(hdev->tx_comm.hdmitx_id, HDMI_TX_COUNT);
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
	if (hdev->tx_comm.tx_hw->chip_data->chip_type < MESON_CPU_ID_T7) {
		/* drm tx22 enters AUTH_STOP, don't do hdcp22 IP reset */
		if (hdev->tx_comm.hdcp_ctl_lvl > 0)
			return;

		hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_DIS_HPLL, 0);
		hdmitx_hw_cntl_ddc(&hdev->hw_comm, DDC_RESET_HDCP, 0);
		hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_ESMCLK_CTRL, 0);
		HDMITX_INFO("amhdmitx: suspend and reset hdcp\n");
	}
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
	struct hdcp_t *p_hdcp = (struct hdcp_t *)hdev->am_hdcp;
	struct hdmitx_common *tx_comm = &hdev->tx_comm;

	hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_HDMI_CLKS_CTRL, 0);

	/*
	 * after suspend, VPU power domain will be powered off,
	 * so hdcp1.4 key otp/crc need to be loaded again
	 */
	if (hdev->tx_comm.tx_hw->chip_data->chip_type >= MESON_CPU_ID_T7)
		p_hdcp->hdcp14_key_loaded = false;
	hdmitx_event_mgr_suspend(tx_comm->event_mgr, true);
	_amhdmitx_suspend(hdev);

	HDMITX_INFO("amhdmitx: suspend\n");
	return 0;
}

static int amhdmitx_resume(struct platform_device *pdev)
{
	struct hdmitx_dev *hdev = dev_get_drvdata(&pdev->dev);
	struct hdmitx_common *tx_comm = &hdev->tx_comm;
	struct hdmitx_hw_common *tx_hw_base = tx_comm->tx_hw;

	HDMITX_INFO("amhdmitx: resume\n");
	hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_HDMI_CLKS_CTRL, 1);
	/*
	 * Since the S7 chip, in order to optimize power consumption, it will turn off and on the
	 * vpu power domain when standby and wakes up.When it is turned off, the reg of the relevant
	 * modules will be reset. When it wakes up,the hdmitx driver needs to reinitialize the
	 * required top register.
	 */
	if (tx_hw_base->chip_data->chip_type >= MESON_CPU_ID_S7)
		hdmitx_hw_cntl_config(&hdev->hw_comm, CONF_HW_INIT, 1);
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
	/*
	 * may resume after start hdcp22, i2c
	 * reactive will force mux to hdcp14
	 */
	if (hdev->tx_comm.hdcp_ctl_lvl > 0)
		return 0;
	hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_ESMCLK_CTRL, 1);

	if (hdev->tx_comm.tx_hw->chip_data->chip_type < MESON_CPU_ID_G12A)
		hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_I2C_RESET, 0);
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

static void hdmitx_init_uboot_mode(enum vmode_e mode)
{
	if (!(mode & VMODE_INIT_BIT_MASK))
		HDMITX_ERROR("warning, echo /sys/class/display/mode is disabled\n");
	else
		HDMITX_INFO("already display in uboot\n");
}

static int amhdmitx_pm_restore(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct hdmitx_dev *hdev = dev_get_drvdata(&pdev->dev);
	struct hdmitx_common *tx_comm = &hdev->tx_comm;
	struct hdmitx_hw_common *tx_hw_base = tx_comm->tx_hw;

	if (tx_hw_base->chip_data->chip_type < MESON_CPU_ID_T7)
		return -1;
	HDMITX_INFO("%s restore\n", __func__);
	mutex_lock(&tx_comm->hdmimode_mutex);
	/*
	 * if hdmitx init and display already, then should not do
	 * HW init again as it may change clk settings, otherwise
	 * need to do hw init as did in driver probe in case HW is
	 * in power down or unknown state
	 */
	hdmitx_hw_cntl_config(&hdev->hw_comm, CONF_HW_INIT, 0);
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
		hdmitx_init_uboot_mode(VMODE_INIT_BIT_MASK);
	/* notify to drm hdmi */
	hdmitx_fire_drm_hpd_cb_unlocked(tx_comm);
	return 0;
}

const struct dev_pm_ops hdmitx_pm = {
	.suspend	= amhdmitx_pm_suspend,
	.resume		= amhdmitx_pm_resume,
	/* no application for now */
	.restore    = amhdmitx_pm_restore,
};

#endif

static void amhdmitx_shutdown(struct platform_device *pdev)
{
	struct hdmitx_dev *hdev = dev_get_drvdata(&pdev->dev);

	_amhdmitx_suspend(hdev);

	if (hdev->tx_comm.tx_hw->chip_data->chip_type >= MESON_CPU_ID_T7) {
		if (hdev->tx_comm.aon_output) {
			hdmitx21_disable_hdcp(hdev);
			return;
		}
		hdmitx_hw_cntl_misc(&hdev->hw_comm, MISC_HDMI_CLKS_CTRL, 0);
	}
}

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
		.pm = &hdmitx_pm,
#endif
	}
};

int  __init amhdmitx_init(void)
{
	struct hdmitx_boot_param *param = get_hdmitx_boot_params();

	if (param->init_state & INIT_FLAG_NOT_LOAD) {
		HDMITX_INFO("INIT_FLAG_NOT_LOAD");
		return 0;
	}

	return platform_driver_register(&amhdmitx_driver);
}

void __exit amhdmitx_exit(void)
{
	HDMITX_INFO("amhdmitx exit\n");
	platform_driver_unregister(&amhdmitx_driver);
}
