// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/regmap.h>

#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_format_para.h>

#include "meson_venc.h"
#include "meson_drm_main.h"
#include "vpu-hw/meson_vpu_reg.h"

#define DEVICE_NAME "meson_tx_venc"
#define MAX_VENC	4

struct meson_tx_venc {
	struct platform_device *pdev;
	struct regmap *regmap[MAX_VENC];
	u32 venc_offset[MAX_VENC];
	int num_regmap;
};

static void config_tv_encp_calc(struct regmap *regmap, struct meson_tx_format_para *para)
{
	const struct tx_timing *tp = NULL;
	struct tx_timing timing = {0};
	/* adjust to align upsample and video enable */
	u32 hsync_st = 5; // hsync start pixel count
	u32 vsync_st = 1; // vsync start line count
	// Latency in pixel clock from ENCP_VFIFO2VD request to data ready to HDMI
	const u32 vfifo2vd_to_hdmi_latency = 2;
	u32 de_h_begin = 0;
	u32 de_h_end = 0;
	u32 de_v_begin = 0;
	u32 de_v_end = 0;
	bool y420_mode = 0;
	int hpara_div = 1;

	if (para->cs == HDMI_COLORSPACE_YUV420)
		y420_mode = 1;

	tp = &para->timing;
	timing = *tp;

	timing.h_total /= hpara_div;
	timing.h_blank /= hpara_div;
	timing.h_front /= hpara_div;
	timing.h_sync /= hpara_div;
	timing.h_back /= hpara_div;
	timing.h_active /= hpara_div;

	de_h_end = tp->h_total - (tp->h_front - hsync_st);
	de_h_begin = de_h_end - tp->h_active;
	de_v_end = tp->v_total - (tp->v_front - vsync_st);
	de_v_begin = de_v_end - tp->v_active;

	// VENC timing gen is disabled
	regmap_write(regmap, ENCP_VIDEO_EN, 0);

	// Enable viu vsync interrupt
	regmap_write(regmap, VPU_VENC_CTRL, 1);

	// set DVI/HDMI transfer timing
	// generate hsync
	regmap_write(regmap, ENCP_DVI_HSO_BEGIN, hsync_st);
	regmap_write(regmap, ENCP_DVI_HSO_END, hsync_st + tp->h_sync);

	// generate vsync
	regmap_write(regmap, ENCP_DVI_VSO_BLINE_EVN, vsync_st + y420_mode);
	regmap_write(regmap, ENCP_DVI_VSO_ELINE_EVN,
		vsync_st + tp->v_sync + y420_mode);
	regmap_write(regmap, ENCP_DVI_VSO_BEGIN_EVN, hsync_st);
	regmap_write(regmap, ENCP_DVI_VSO_END_EVN, hsync_st);

	// generate data valid
	regmap_write(regmap, ENCP_DE_H_BEGIN, de_h_begin);
	regmap_write(regmap, ENCP_DE_H_END, de_h_end);
	regmap_write(regmap, ENCP_DE_V_BEGIN_EVEN, de_v_begin);
	regmap_write(regmap, ENCP_DE_V_END_EVEN, de_v_end);

	// set mode
	// Enable Hsync and equalization pulse switch in center; bit[14] cfg_de_v = 1
	regmap_write(regmap, ENCP_VIDEO_MODE, 0x0040 | (1 << 14));
	regmap_write(regmap, ENCP_VIDEO_MODE_ADV, 0x18); // Sampling rate: 1

	// set active region
	regmap_write(regmap, ENCP_VIDEO_HAVON_BEGIN, de_h_begin - vfifo2vd_to_hdmi_latency);
	regmap_write(regmap, ENCP_VIDEO_HAVON_END, de_h_end - vfifo2vd_to_hdmi_latency - 1);
	regmap_write(regmap, ENCP_VIDEO_VAVON_BLINE, de_v_begin);
	regmap_write(regmap, ENCP_VIDEO_VAVON_ELINE, de_v_end - 1);

	//set hsync
	regmap_write(regmap, ENCP_VIDEO_HSO_BEGIN, hsync_st - vfifo2vd_to_hdmi_latency);
	regmap_write(regmap, ENCP_VIDEO_HSO_END, hsync_st + tp->h_sync - vfifo2vd_to_hdmi_latency);

	//set vsync
	regmap_write(regmap, ENCP_VIDEO_VSO_BEGIN, 0);
	regmap_write(regmap, ENCP_VIDEO_VSO_END, 0);
	regmap_write(regmap, ENCP_VIDEO_VSO_BLINE, vsync_st);
	regmap_write(regmap, ENCP_VIDEO_VSO_ELINE, vsync_st + tp->v_sync);

	//set vtotal & htotal
	regmap_write(regmap, ENCP_VIDEO_MAX_PXCNT, tp->h_total - 1);
	regmap_write(regmap, ENCP_VIDEO_MAX_LNCNT, tp->v_total - 1);

	// VENC timing gen is disabled
	regmap_write(regmap, ENCP_VIDEO_EN, 1);
}

static void config_tv_encl_calc(struct regmap *regmap, struct meson_tx_format_para *para)
{
	const struct tx_timing *tp = NULL;
	u32 de_h_begin = 0;
	u32 de_h_end = 0;
	u32 de_v_begin = 0;
	u32 de_v_end = 0;

	tp = &para->timing;
	de_h_begin = tp->h_total - tp->h_front - tp->h_active;
	de_h_end = tp->h_total - tp->h_front - 1;
	de_v_begin = tp->v_total - tp->v_front - tp->v_active;
	de_v_end = tp->v_total - tp->v_front - 1;

	// VENC timing gen is disabled
	regmap_write(regmap, ENCL_VIDEO_EN, 0);

	// bit[15] shadow en
	regmap_write(regmap, ENCL_VIDEO_MODE, 0x8000);
	// Sampling rate: 1
	regmap_write(regmap, ENCL_VIDEO_MODE_ADV, 0x0418);
	// bypass filter
	regmap_write(regmap, ENCL_VIDEO_FILT_CTRL, 0x1000);

	//set vtotal & htotal
	regmap_write(regmap, ENCL_VIDEO_MAX_PXCNT, tp->h_total - 1);
	regmap_write(regmap, ENCL_VIDEO_MAX_LNCNT, tp->v_total - 1);
	regmap_write(regmap, ENCL_VIDEO_HAVON_BEGIN, de_h_begin);
	regmap_write(regmap, ENCL_VIDEO_HAVON_END, de_h_end);
	regmap_write(regmap, ENCL_VIDEO_VAVON_BLINE, de_v_begin);
	regmap_write(regmap, ENCL_VIDEO_VAVON_ELINE, de_v_end);

	//  set hsync
	regmap_write(regmap, ENCL_VIDEO_HSO_BEGIN, 0x0);
	regmap_write(regmap, ENCL_VIDEO_HSO_END, tp->h_sync);

	// set vsync
	regmap_write(regmap, ENCL_VIDEO_VSO_BEGIN, 0x0);
	regmap_write(regmap, ENCL_VIDEO_VSO_END, 0x0);
	regmap_write(regmap, ENCL_VIDEO_VSO_BLINE, 0x0);
	regmap_write(regmap, ENCL_VIDEO_VSO_ELINE, tp->v_sync - 1);

	// set inbuf
	regmap_write(regmap, ENCL_INBUF_CNTL1, (5 << 13) | (tp->h_sync - 1));
	regmap_write(regmap, ENCL_INBUF_CNTL0, 0x200);

	regmap_write(regmap, LCD_RGB_BASE_ADDR, 0x0);
	regmap_write(regmap, LCD_RGB_COEFF_ADDR, 0x400);
	regmap_write(regmap, LCD_DITH_CNTL_ADDR, 0x0);
	//regmap_write(regmap, LCD_POL_CNTL_ADDR);

	// DE signal
	regmap_write(regmap, DE_HS_ADDR, de_h_begin);
	regmap_write(regmap, DE_HE_ADDR, de_h_end);
	regmap_write(regmap, DE_VS_ADDR, de_v_begin);
	regmap_write(regmap, DE_VE_ADDR, de_v_end);

	// Hsync signal
	regmap_write(regmap, HSYNC_HS_ADDR, 0x0);
	regmap_write(regmap, HSYNC_HE_ADDR, tp->h_sync);
	regmap_write(regmap, HSYNC_VS_ADDR, 0x0);
	regmap_write(regmap, HSYNC_VE_ADDR, tp->v_total - 1);

	// Vsync signal
	regmap_write(regmap, VSYNC_HS_ADDR, 0x0);
	regmap_write(regmap, VSYNC_HE_ADDR, 0x0);
	regmap_write(regmap, VSYNC_VS_ADDR, 0x0);
	regmap_write(regmap, VSYNC_VE_ADDR, tp->v_sync - 1);

	regmap_write(regmap, ENCL_VIDEO_RGBIN_CTRL, 3);
	regmap_write(regmap, ENCL_VIDEO_EN, 1);
	regmap_write(regmap, VPU_VENC_CTRL, 2);
}

int meson_venc_mode_set(struct meson_tx_venc *venc, u32 enc_index, u32 enc_type, void *para)
{
	struct meson_tx_format_para *fmt_para = (struct meson_tx_format_para *)para;
	struct regmap *regmap = venc->regmap[enc_index];

	if (enc_type == VENC_ENCP)
		config_tv_encp_calc(regmap, fmt_para);
	else if (enc_type == VENC_ENCL)
		config_tv_encl_calc(regmap, fmt_para);

	return 0;
}

int meson_venc_mode_check(struct meson_tx_venc *venc, u32 enc_index, void *para)
{
	return 0;
}

int meson_venc_mode_disable(struct meson_tx_venc *venc, u32 enc_index)
{
	struct regmap *regmap = venc->regmap[enc_index];

	// VENC timing gen is disabled
	regmap_write(regmap, ENCP_VIDEO_EN, 0);

	return 0;
}

static const struct regmap_config venc_regmap_config = {
	.reg_bits = 32,
	.val_bits = 32,
	.reg_stride = 4,
	.max_register = 0xffff,
};

static int meson_tx_venc_probe(struct platform_device *pdev)
{
	int i, ret, num_venc;
	struct resource *res;
	void __iomem *base, *venc_base;
	struct regmap **regmap;
	u32 *venc_offset;
	struct meson_tx_venc *tx_venc;
	struct device *device = &pdev->dev;

	tx_venc = devm_kmalloc(device, sizeof(*tx_venc), GFP_KERNEL);
	if (!tx_venc)
		return -ENOMEM;

	num_venc = of_property_count_u32_elems(device->of_node, "venc_offset");
	if (num_venc < 0) {
		pr_err("%s Invalid or missing venc_offset\n", __func__);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(device->of_node, "venc_offset",
					 tx_venc->venc_offset, num_venc);
	if (ret) {
		pr_err("%s Invalid or missing venc_offset\n", __func__);
		return -EINVAL;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -EINVAL;

	base = devm_ioremap(device, res->start, resource_size(res));
	if (!base)
		return -ENOMEM;

	regmap = tx_venc->regmap;
	venc_offset = tx_venc->venc_offset;
	tx_venc->num_regmap = num_venc;

	for (i = 0; i < tx_venc->num_regmap; i++) {
		venc_base = base + (venc_offset[i] << 2);
		regmap[i] = devm_regmap_init_mmio(device, venc_base,
						  &venc_regmap_config);
		if (IS_ERR(regmap[i])) {
			pr_err("%s regmap init mmio fail\n", __func__);
			return -EINVAL;
		}
	}

	dev_set_drvdata(device, tx_venc);

	pr_info("%s %px\n", __func__, tx_venc);

	return 0;
}

static void meson_tx_venc_remove(struct platform_device *pdev)
{
	struct device *device = &pdev->dev;

	pr_info("%s %s\n", __func__, dev_name(device));
}

static const struct of_device_id meson_tx_venc_of_match[] = {
	{
		.compatible	 = "amlogic, tx-venc-t7c",
	},
	{
		.compatible	 = "amlogic, tx-venc-a9",
	},
	{},
};

static struct platform_driver meson_tx_venc_driver = {
	.probe	 = meson_tx_venc_probe,
	.remove	 = meson_tx_venc_remove,
	.driver	 = {
		.name = DEVICE_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(meson_tx_venc_of_match),
	}
};

int  __init meson_tx_venc_init(void)
{
	return platform_driver_register(&meson_tx_venc_driver);
}

void __exit meson_tx_venc_exit(void)
{
	pr_info("%s\n", __func__);
	platform_driver_unregister(&meson_tx_venc_driver);
}

