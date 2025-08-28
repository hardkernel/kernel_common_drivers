// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/printk.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/amlogic/media/vout/meson_tx_connector/clk/meson_tx_clk.h>

#include "meson_tx_clk_reg.h"
#include "meson_tx_clk_internal.h"

/*
 * set path and divider between analog pll output(hdmitx_clk_out2
 * or pll_pix_clk) and CRT_VIDEO
 */
static void tx_vid_pll_pix_div_set(u8 div)
{
	pr_info("%s TODO\n", __func__);
}

/*
 * VCO for video clk, and phy clk which is coherent with video clk.
 *
 * 1.connector_type: differ pll source for different connector type
 * 2.vco_clk: VCO frequency before divide to connector phy/digital
 * 3.clk_to_crt_video is clk for digital video, such as pixel/venc clk
 * and may be for tmds_clk in special case.
 * clk_to_crt_video is divided from vco_clk
 */

int tx_pixel_pll_set_a9(struct meson_tx_clk *tx_clk,
	enum tx_clk_type clk_type, u32 vco_clk, u32 clk_to_crt_video)
{
	u8 div = 5;

	if (!tx_clk) {
		pr_err("%s NULL tx_clk\n", __func__);
		return -EINVAL;
	}

	pr_info("%s TODO for VCO\n", __func__);
	tx_vid_pll_pix_div_set(div);
	return 0;
}

int tx_pixel_pll_disable_a9(struct meson_tx_clk *tx_clk, enum tx_clk_type clk_type)
{
	if (!tx_clk) {
		pr_err("%s NULL tx_clk\n", __func__);
		return -EINVAL;
	}

	pr_info("%s TODO for VCO\n", __func__);
	return 0;
}

/* clk_type: differ phy clk source by connector type
 *
 * if phy link clk is not coherent with video clk(such
 * as FRL link or dp link), phy clk is divided from PLL independent
 * from pixel pll;
 * if pixel clk and phy clk are coherent, phy clk is divided
 * from pixel pll which is set by pixel_pll_set;
 */

int tx_phy_clk_set_a9(struct meson_tx_clk *tx_clk, enum tx_clk_type clk_type, u32 clk_todig_phy)
{
	if (!tx_clk) {
		pr_err("%s NULL tx_clk\n", __func__);
		return -EINVAL;
	}
	pr_info("%s TODO\n", __func__);
	return 0;
}

int tx_phy_clk_disable_a9(struct meson_tx_clk *tx_clk, enum tx_clk_type clk_type)
{
	if (!tx_clk) {
		pr_err("%s NULL tx_clk\n", __func__);
		return -EINVAL;
	}

	pr_info("%s TODO\n", __func__);
	return 0;
}

/*
 * Function: tx_crt_video_clk_set_a9
 *
 * set venc/fe/pixel clk path/divider
 * clk_to_crt_video: input clk to crt_video
 * vclk_sel 0: V1, 1:V2. assume V1 connect to VENC0 and V2 connect to VENC1
 * crt_video_clk_out: output clk from crt_video
 *
 * there have 2 parallel set clock generator: V1 and V2,
 * TO confirm: V1 connect to VENC0, and V2 connect to VENC1
 * by default and can't be changed
 *
 * vc_id(0) refers to the primary display device, vc_id(1)
 * refers to the secondary display device, the display device
 * may be one of dptx/hdmitx/edp/dsi/vbo/lvds
 *
 * vc_id0: pixel0/1 pll-->V1_VENC0/V2_VENC1-->HDMI_IF_unit0
 * vc_id1: pixel0/1 pll-->V1_VENC0/V2_VENC1-->HDMI_IF_unit1
 *
 * Note: need to use different VENC between connectors
 * hdmitx/DP/eDP/dsi/vbo, this is decided by drm encoder.
 */
static int tx_crt_video_clk_set_a9(struct meson_tx_clk *tx_clk,
	u32 clk_to_crt_video, u8 vclk_sel, u32 crt_video_clk_out)
{
	/* 0:vid_pll_clk_pre; 1:pixel0_pll_clkout; 2:vid_pll0_clk;
	 * 3:pixel1_pll_clkout;
	 * the clk rate from clk_src equals to clk_to_crt_video
	 */
	u8 clk_src = 0;
	u8 clk_div = 1;

	if (!tx_clk) {
		pr_err("%s NULL tx_clk\n", __func__);
		return -EINVAL;
	}
	if (crt_video_clk_out == 0) {
		pr_err("%s crt_video_clk_out invalid\n", __func__);
		return -EINVAL;
	}

	clk_div = clk_to_crt_video / crt_video_clk_out;

	if (vclk_sel == 0) {
		/* step1: set crt_video_clk source and divider */
		/* clk source: V1 always use pixel0_pll as source by default */
		clk_src = 1;
		/* path/div set for V1 */
		/* bit[19]: disable clk_div0 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL, 0, 19, 1);
		/* bit[18:16]: cntl_clk_in_sel */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL, clk_src, 16, 3);
		/* bit[7:0]: cntl_xd0 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_DIV, clk_div - 1, 0, 8);
		/* bit[19]: enable clk_div0 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL, 1, 19, 1);
		/* bit[0]: enable v1 div1 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL, 1, 0, 1);
	} else if (vclk_sel == 1) {
		/* step1: set crt_video_clk source and divider */
		/* clk source: V2 always use pixel1_pll as source by default */
		clk_src = 3;
		/* path/div set for V2 */
		/* bit[19]: disable clk_div0 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VIID_CLK_CTRL, 0, 19, 1);
		/* bit[18:16]: cntl_clk_in_sel */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VIID_CLK_CTRL, clk_src, 16, 3);
		/* bit[7:0]: cntl_xd0 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VIID_CLK_DIV, clk_div - 1, 0, 8);
		/* bit[19]: enable clk_div0 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VIID_CLK_CTRL, 1, 19, 1);
		/* v2 div1 enable */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VIID_CLK_CTRL, 1, 0, 1);
	} else {
		pr_err("%s invalid vclk_sel(%d)\n", __func__, vclk_sel);
		return -EINVAL;
	}
	return 0;
}

/* vclk_sel 0: V1, 1:V2 */
static int tx_crt_video_clk_disable_a9(struct meson_tx_clk *tx_clk, u8 vclk_sel)
{
	if (!tx_clk) {
		pr_err("%s NULL tx_clk\n", __func__);
		return -EINVAL;
	}

	if (vclk_sel == 0) {
		/* path/div set for V1 */
		/* bit[19]: disable clk_div0 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL, 0, 19, 1);
		/* bit[0]: v1 div1 disable */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL, 0, 0, 1);
	} else if (vclk_sel == 1) {
		/* path/div set for V2 */
		/* bit[19]: disable clk_div0 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VIID_CLK_CTRL, 0, 19, 1);
		/* bit[0]: v2 div1 disable */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VIID_CLK_CTRL, 0, 0, 1);
	} else {
		pr_err("%s invalid vclk_sel(%d)\n", __func__, vclk_sel);
		return -EINVAL;
	}
	return 0;
}

/*
 * crt_video_clk_out: clk set crt_video_clk_set_a9()
 * vclk_sel: 0: V1, 1:V2. assume V1 connect to VENC0 and V2 connect to VENC1
 */
static int tx_venc_clk_set_a9(struct meson_tx_clk *tx_clk,
	u32 crt_video_clk_out, u32 clk_div_path, u32 venc_clk)
{
	u8 venc_clk_div = 1;
	u8 venc_clk_div_offset = 0;
	/* 0: V1, 1:V2. assume V1 connect to VENC0 and V2 connect to VENC1 */
	u8 vclk_sel = clk_div_path & BIT(0);

	if (!tx_clk) {
		pr_err("%s NULL tx_clk\n", __func__);
		return -EINVAL;
	}

	if (venc_clk == 0) {
		pr_err("%s venc_clk invalid\n", __func__);
		return -EINVAL;
	}

	venc_clk_div = crt_video_clk_out / venc_clk;
	switch (venc_clk_div) {
	case 2:
		/* extra div 2 */
		venc_clk_div_offset = 1;
		break;
	case 1:
		/* no extra div */
		venc_clk_div_offset = 0;
		break;
	default:
		pr_err("%s crt_video_clk_out(%d)/venc_clk(%d) ratio invalid\n",
			__func__, crt_video_clk_out, venc_clk);
		return -EINVAL;
	}

	if (vclk_sel == 0) {
		/* V1 enc0 sel div1 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VIID_CLK_DIV,
			venc_clk_div_offset, 12, 4);
		/* bit[10] enable enc0 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL2, 1, 10, 1);
	} else if (vclk_sel == 1) {
		/* V2 enc1 sel div1 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VIID_CLK_DIV,
			8 + venc_clk_div_offset, 8, 4);
		/* [11] enable enc1 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL2, 1, 11, 1);
	}

	return 0;
}

/*
 * crt_video_clk_out: clk set crt_video_clk_set_a9()
 */
static int tx_pixel_clk_set_a9(struct meson_tx_clk *tx_clk,
	u32 crt_video_clk_out, u32 clk_div_path, u32 pixel_clk)
{
	/* 0: V1, 1:V2. assume V1 connect to VENC0 and V2 connect to VENC1 */
	u8 vclk_sel = clk_div_path & BIT(0);
	/* 0: HDMI_IF0, 1:HDMI_IF1 */
	u8 hdmi_if_idx = (clk_div_path >> 4) & 0x1;
	u8 pixel_clk_div = 1;
	u8 pixel_clk_div_offset = 0;

	if (!tx_clk) {
		pr_err("%s NULL tx_clk\n", __func__);
		return -EINVAL;
	}

	if (pixel_clk == 0) {
		pr_err("%s pixel clk invalid\n", __func__);
		return -EINVAL;
	}

	pixel_clk_div = crt_video_clk_out / pixel_clk;
	switch (pixel_clk_div) {
	case 2:
		/* extra div 2 */
		pixel_clk_div_offset = 1;
		break;
	case 1:
		/* no extra div */
		pixel_clk_div_offset = 0;
		break;
	default:
		pr_err("%s crt_video_clk_out(%d)/pixel_clk(%d) ratio invalid\n",
			__func__, crt_video_clk_out, pixel_clk);
		return -EINVAL;
	}

	/* crt_video_clk to hdmi_if */
	if (hdmi_if_idx == 0) {
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_HDMI_CLK_CTRL,
			vclk_sel == 0 ? 0 + pixel_clk_div_offset : 8 + pixel_clk_div_offset, 16, 4);
		/* fe0 sel div0 always */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_HDMI_CLK_CTRL,
			vclk_sel == 0 ? 0 : 8, 20, 4);
		/* bit[5] enable pix0 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL2, 1, 5, 1);
		/* [9] enable fe0 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL2, 1, 9, 1);
	} else if (hdmi_if_idx == 1) {
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_HDMI_CLK_CTRL,
			vclk_sel == 0 ? 0 + pixel_clk_div_offset : 8 + pixel_clk_div_offset, 24, 4);
		/* fe0 sel div0 always */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_HDMI_CLK_CTRL,
			vclk_sel == 0 ? 0 : 8, 28, 4);
		/* bit[5] enable pix0 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL2, 1, 12, 1);
		/* [9] enable fe0 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL2, 1, 13, 1);
	}

	return 0;
}

/* assume venc_idx = clk_divider_idx */
static int tx_venc_clk_gate_a9(struct meson_tx_clk *tx_clk, u8 venc_idx, bool en)
{
	if (!tx_clk) {
		pr_err("%s NULL tx_clk\n", __func__);
		return -EINVAL;
	}

	if (venc_idx == 0)
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL2, en, 10, 1);
	else if (venc_idx == 1)
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL2, en, 11, 1);
	else
		pr_err("%s invalid venc_idx: %d\n", __func__, venc_idx);

	return 0;
}

/* for fe_clk and pixel_clk gate */
static int tx_pixel_clk_gate_a9(struct meson_tx_clk *tx_clk, u8 hdmi_if_idx, bool en)
{
	if (!tx_clk) {
		pr_err("%s NULL tx_clk\n", __func__);
		return -EINVAL;
	}

	if (hdmi_if_idx == 0) {
		/* bit[5] enable pix0 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL2, en, 5, 1);
		/* bit[9] enable fe0 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL2, en, 9, 1);
	} else if (hdmi_if_idx == 1) {
		/* bit[12] enable pix1 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL2, en, 12, 1);
		/* bit[13] enable fe1 */
		tx_clk_set_reg_bits(tx_clk->reg_io_base, CLKCTRL_VID_CLK_CTRL2, en, 13, 1);
	} else {
		pr_err("%s invalid hdmi_if_idx: %d\n", __func__, hdmi_if_idx);
	}

	return 0;
}

int tx_bulk_clk_set_a9(struct meson_tx_clk *tx_clk, u32 clk_mask)
{
	u32 crt_video_clk_out;
	struct meson_tx_clk_data *clk_cfg = NULL;

	if (!tx_clk) {
		pr_err("%s NULL tx_clk\n", __func__);
		return -EINVAL;
	}

	clk_cfg = &tx_clk->tx_clk_cfg;
	crt_video_clk_out = clk_cfg->venc_clk;

	if (clk_mask & PIXEL_PLL_MASK)
		tx_pixel_pll_set_a9(tx_clk, clk_cfg->clk_type, clk_cfg->pixel_vco_clk,
			clk_cfg->clk_to_crt_video);

	/* crt_video_clk: set clk source for venc and pixel clk,
	 * here set the crt_video divider to output clk at venc clk rate
	 */
	if (clk_mask & (PIXEL_CLK_MASK | VENC_CLK_MASK))
		tx_crt_video_clk_set_a9(tx_clk, clk_cfg->clk_to_crt_video,
			clk_cfg->clk_div_path & BIT(0), crt_video_clk_out);

	/* clk div1 for VENC */
	if (clk_mask & VENC_CLK_MASK)
		tx_venc_clk_set_a9(tx_clk, crt_video_clk_out,
			clk_cfg->clk_div_path, clk_cfg->venc_clk);

	if (clk_mask & PIXEL_CLK_MASK)
		tx_pixel_clk_set_a9(tx_clk, crt_video_clk_out,
			clk_cfg->clk_div_path, clk_cfg->pixel_clk);

	if (clk_mask & PHY_CLK_MASK)
		tx_phy_clk_set_a9(tx_clk, clk_cfg->clk_type, clk_cfg->phy_clk);

	return 0;
}

int tx_bulk_clk_disable_a9(struct meson_tx_clk *tx_clk, u32 clk_mask)
{
	struct meson_tx_clk_data *clk_cfg = NULL;

	if (!tx_clk) {
		pr_err("%s NULL tx_clk\n", __func__);
		return -EINVAL;
	}

	clk_cfg = &tx_clk->tx_clk_cfg;
	if (clk_mask & PIXEL_PLL_MASK)
		tx_pixel_pll_disable_a9(tx_clk, clk_cfg->clk_type);

	/*
	 * crt_video_clk: clk source for venc and pixel clk,
	 * disable crt_video only if both pixel clk and venc clk are disabled
	 */
	if ((clk_mask & (PIXEL_CLK_MASK | VENC_CLK_MASK)) == (PIXEL_CLK_MASK | VENC_CLK_MASK))
		tx_crt_video_clk_disable_a9(tx_clk, clk_cfg->clk_div_path & BIT(0));

	if (clk_mask & VENC_CLK_MASK)
		tx_venc_clk_gate_a9(tx_clk, clk_cfg->clk_div_path & BIT(0), false);

	if (clk_mask & PIXEL_CLK_MASK)
		tx_pixel_clk_gate_a9(tx_clk, (clk_cfg->clk_div_path >> 4) & 0x1, false);

	if (clk_mask & PHY_CLK_MASK)
		tx_phy_clk_disable_a9(tx_clk, clk_cfg->clk_type);

	return 0;
}

