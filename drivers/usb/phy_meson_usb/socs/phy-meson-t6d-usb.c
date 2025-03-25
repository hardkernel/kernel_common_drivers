// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "../phy-meson-usb.h"
#include <linux/amlogic/gki_module.h>
#include <linux/clk/clk-conf.h>

static bool meson_u2phy_960m;

static int meson_u2phy_get_speed(char *str)
{
	int ret;

	ret = kstrtobool(str, &meson_u2phy_960m);
	if (ret)
		return ret;

	return 1;
}

__setup("usb2t_mode=", meson_u2phy_get_speed);

static inline bool meson_u2phy_t6d_hsp(struct amlogic_usb_v2 *mphy)
{
	return mphy->sw_hsp && meson_u2phy_960m;
}

static void meson_u2phy_t6d_set_pll(struct amlogic_usb_v2 *mphy)
{
#define USB_PLL_RST	BIT(0)
#define USB_MPLL_EN_CTRL BIT(1)
#define	USB_PLL_LK_RST BIT(28)
#define USB_PLL_EN BIT(11)
#define	USBPLL_LOCK_FLAG BIT(31)
	void __iomem *phy_reg_base = mphy->phy_cfg[0];
	u32 retry = 5;
	int plldone_i;
	u32 val;
	void __iomem *pll_cfg = phy_reg_base + 0x40;

	while (retry--) {
		/* Assert usb_pll_rst and usb_pll_lk_rst. */
		val = readl(pll_cfg);
		val |= USB_PLL_RST | USB_PLL_LK_RST;
		writel(val, pll_cfg);
		usleep_range(20, 30);
		/* Enable sw force control . */
		val = readl(pll_cfg);
		val |= USB_MPLL_EN_CTRL | USB_PLL_EN;
		writel(val, pll_cfg);
		usleep_range(20, 30);
		/* Deassert usb_pll_rst. */
		val = readl(pll_cfg);
		val &= ~USB_PLL_RST;
		writel(val, pll_cfg);
		usleep_range(20, 30);
		/* Deassert usb_pll_lk_rst. */
		val = readl(pll_cfg);
		val &= ~USB_PLL_LK_RST;
		writel(val, pll_cfg);
		/* Check lock bit. */
		for (plldone_i = 5; plldone_i > 0; plldone_i--) {
			usleep_range(20, 30);
			if (readl(pll_cfg) & USBPLL_LOCK_FLAG)
				goto okay;
		}
		dev_dbg(mphy->dev, "usb2 pll not locked, retry. val: 0x%08x\n",
						readl(pll_cfg));
	}

	dev_err(mphy->dev, "%s set pll failed, exit, val:0x%08x.\n", __func__, readl(pll_cfg));
	return;
okay:
	dev_dbg(mphy->dev, "usb2 pll init done, val: 0x%08x\n", readl(pll_cfg));
}

static int meson_u2phy_t6d_cali_disc_squelch
			(struct amlogic_usb_v2 *mphy)
{
/* usb2_squelch_trim:
 *	usb2.0: reg32_14[28]##reg32_03[2:0] (MSB->LSB) default 0b0111.
 *	usb2.2(2t): reg32_23[31:28] (MSB->LSB) default 0b0000.
 * usb2_disc_trim:
 *	usb2.0 & usb2.2: reg32_14[27]##reg32_03[6:4] (MSB->LSB) default 0b1000.
 */
#define SQUELCH_960M ((u32)GENMASK(31, 28))
#define SQUELCH_960M_VAL(x) ((0xf & (x)) << 28)
#define DISCONNECT_MASK ((u32)GENMASK(6, 4))
#define DISCONNECT_VAL(x) ((0x7 & (x)) << 4)
	u32 val = 0;
	void __iomem *cfg = mphy->phy_cfg[0];

	val = readl(cfg + 0xc);
	/* Set to the MAX. */
	val = (val & ~DISCONNECT_MASK) | DISCONNECT_VAL(0x7);
	writel(val, cfg + 0xc);

	if (mphy->portspeed == MESON_USB_SPEED_HIGH_PLUS) {
		/* Trimming for squelch(960M) detect threshold. */
		val = readl(cfg + 0x5c);
		val = (val & ~SQUELCH_960M) | SQUELCH_960M_VAL(0x9);
		writel(val, cfg + 0x5c);
	/* Set to the MAX. */
	} else if (mphy->portspeed == MESON_USB_SPEED_HIGH) {
		val = readl(cfg + 0x38);
		val = val | (u32)BIT(28);
		writel(val, cfg + 0x38);
	}

	return 0;
}

static int meson_u2phy_t6d_set_misc(struct amlogic_usb_v2 *mphy)
{
#define USB2_SEL_STRENGTH ((u32)GENMASK(30, 29))
#define USB2_SEL_STRENGTH_VAL(x) ((0x3 & (x)) << 29)
#define ORW_USB2_EDGEDRV_EN BIT(13)
#define ORW_USB2_EDGEDRV_TRIM ((u32)GENMASK(15, 14))
#define ORW_USB2_EDGEDRV_TRIM_VAL(x) ((0x3 & (x)) << 14)
	u32 val = 0;
	void __iomem *cfg = mphy->phy_cfg[0];

	if (mphy->portspeed == MESON_USB_SPEED_HIGH_PLUS) {
		/* usb2_sel_strength 960M set to 1. */
		val = readl(cfg + 0x38);
		val = (val & ~USB2_SEL_STRENGTH) | USB2_SEL_STRENGTH_VAL(0x1);
		writel(val, cfg + 0x38);
		/* edgedrv cali for signal quality. */
		val = readl(cfg + 0x50);
		val = (val & ~ORW_USB2_EDGEDRV_TRIM) | ORW_USB2_EDGEDRV_TRIM_VAL(0x0) |
				ORW_USB2_EDGEDRV_EN;
		writel(val, cfg + 0x50);
	} else if (mphy->portspeed == MESON_USB_SPEED_HIGH) {
		/* edgedrv cali for signal quality. */
		val = readl(cfg + 0x50);
		val = (val & ~ORW_USB2_EDGEDRV_TRIM) | ORW_USB2_EDGEDRV_TRIM_VAL(0x1) |
				ORW_USB2_EDGEDRV_EN;
		writel(val, cfg + 0x50);
	}

	return 0;
}

static int meson_u2phy_t6d_cali(struct amlogic_usb_v2 *mphy)
{
	meson_usb2phy_set_calibration_trim(mphy);
	meson_u2phy_t6d_cali_disc_squelch(mphy);
	meson_u2phy_t6d_set_misc(mphy);
	return 0;
}

static int meson_u2phy_t6d_set_mode(void *phy, enum meson_uphy_mode mode)
{
	return meson_u2phy_set_mode((struct amlogic_usb_v2 *)phy, mode);
}

static int meson_u2phy_t6d_init(void *phy)
{
	int ret, rty = 3;
	u32 val;
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)phy;
	void __iomem *cfg = mphy->phy_cfg[0];

	if (!mphy->suspend_flag) {
		dev_err(mphy->dev, "%s excessive init\n", __func__);
		return -EBUSY;
	}

	dev_dbg(mphy->dev, "enable clock num: %d.", mphy->clk_num);

	do {
		ret = clk_bulk_prepare_enable(mphy->clk_num, mphy->clks);
		if (ret)
			dev_err(mphy->dev, "retry enable usb2 phy bus.\n");
	} while (ret && rty--);

	if (ret)
		dev_err(mphy->dev, "Failed to enable usb2 phy bus clock %d.\n", ret);

	if (mphy->clk_mux == 2) {
		/* clk_soc_u2drd_48m_pre. */
		if (mphy->clks[2].clk) {
			ret = clk_set_rate(mphy->clks[2].clk, 480000000);
			if (ret)
				dev_err(mphy->dev, "Failed to set clk_soc_u2drd_48m_pre rate\n");
		}
	}

	meson_u2phy_hold_reset(mphy, false);
	usleep_range(49, 50);
	ret = meson_u2phy_reg_hold_reset(mphy, true);
	usleep_range(49, 50);
	meson_u2phy_reg_reset(mphy);
	usleep_range(49, 50);

	dev_dbg(mphy->dev, "init r0~r2 0x%x 0x%x 0x%x.\n",
			readl(&mphy->u2p_aml_regs[0]->r0),
			readl(&mphy->u2p_aml_regs[0]->r1),
			readl(&mphy->u2p_aml_regs[0]->r2));

	if (mphy->suspend_flag && mphy->current_mode != 0)
		meson_u2phy_t6d_set_mode(phy, mphy->current_mode);

	/* T6D specific 960M clk source config. */
	if (mphy->portspeed == MESON_USB_SPEED_HIGH_PLUS) {
		dev_dbg(mphy->dev, "%s setmode HSP.\n", __func__);
		/* usbpll_reve[0]: configure analog and pll to 960m usb2T mode. */
		val = readl(cfg + 0x44);
		val |= BIT(10);
		writel(val, cfg + 0x44);
		usleep_range(20, 30);
		/* Configure controller to 48M. */
		usleep_range(20, 30);
		val = readl(mphy->regs + 0x84);
		if (mphy->clk_mux == 2 || mphy->clk_mux == 3) {
			/* Use SoC 48M ref_clk */
			val |= BIT(1);
		} else if (mphy->clk_mux == 1) {
			/* Use phy PLL hs_clk. */
			val |= BIT(0);
		}
		writel(val, mphy->regs + 0x84);
	}

	meson_u2phy_t6d_cali(mphy);

	meson_u2phy_hold_reset(mphy, true);
	usleep_range(49, 50);

	ret = meson_usb2phy_wait_ready(mphy, 200);
	if (ret)
		dev_err(mphy->dev, " wait for ready timeout.\n");

	meson_u2phy_t6d_set_pll(mphy);

	dev_dbg(mphy->dev, "end r0~r2 0x%x 0x%x 0x%x.\n",
			readl(&mphy->u2p_aml_regs[0]->r0),
			readl(&mphy->u2p_aml_regs[0]->r1),
			readl(&mphy->u2p_aml_regs[0]->r2));

	if (mphy->suspend_flag)
		mphy->suspend_flag = 0;

	return ret;
}

static int meson_u2phy_t6d_exit(void *phy)
{
	return meson_u2phy_exit((struct amlogic_usb_v2 *)phy);
}

static int meson_u2phy_t6d_power_on(void *phy)
{
	return meson_u2phy_power_on((struct amlogic_usb_v2 *)phy);
}

static int meson_u2phy_t6d_power_off(void *phy)
{
	return meson_u2phy_power_off((struct amlogic_usb_v2 *)phy);
}

static struct meson_uphy_ops meson_u2phy_t6d_ops = {
	.init = meson_u2phy_t6d_init,
	.exit = meson_u2phy_t6d_exit,
	.power_on = meson_u2phy_t6d_power_on,
	.power_off = meson_u2phy_t6d_power_off,
	.set_mode = meson_u2phy_t6d_set_mode,
};

int meson_aml_u2phy_t6d_parse(struct device *dev, struct meson_uphy_instance *instance)
{
	int ret;
	struct amlogic_usb_v2 *mphy;

	ret = meson_aml_u2phy_parse(dev, instance);
	if (ret)
		return ret;

	/* meson_uphy is valid now */
	mphy = (struct amlogic_usb_v2 *)instance->meson_uphy;

	if (!mphy)
		return -ENODEV;

	if  (meson_u2phy_t6d_hsp(mphy)) {
		mphy->portspeed = MESON_USB_SPEED_HIGH_PLUS;
		mphy->clk_mux = 0;
		dev_info(mphy->dev, "usb2t_mode %d forces portspeed to %d.",
						meson_u2phy_960m, mphy->portspeed);
	}

	if (mphy->clk_mux != 0 && mphy->portspeed != MESON_USB_SPEED_HIGH_PLUS) {
		dev_err(mphy->dev, "wrong clk-mux=%d in normal HS mode, default to 0.",
						mphy->clk_mux);
		mphy->clk_mux = 0;
	}

	/* Choose 48M soc digital clk src as default.
	 * Note: When use phy PLL hs_clk for USB2T mode, usb clk will be disabled in
	 * suspend mode, software shall enable PLL manually to resume USB!
	 * So it is recommended to use the clock from the SoC by default in USB2T.
	 */
	if (mphy->portspeed == MESON_USB_SPEED_HIGH_PLUS && mphy->clk_mux == 0) {
		mphy->clk_mux = 2;
		dev_info(mphy->dev, "clk-mux=0 in HSP mode, default to %d.", mphy->clk_mux);
	}

	switch (mphy->clk_mux) {
	/* Normal HS mode. */
	case 0:
		mphy->clk_num = 1;
		break;
	/* HSP mode.*/
	/* 48M phy digital clk src. */
	case 1:
		mphy->clk_num = 1;
		break;
	/* 48M soc digital clk src. */
	case 2:
		mphy->clk_num = 3;
		break;
	/* 48M soc analog clk src. */
	case 3:
		mphy->clk_num = 2;
		break;
	}

	ret = of_clk_set_defaults(mphy->dev->of_node, false);
	if (ret < 0)
		dev_err(mphy->dev, "set assigned clock parents and rates failed %d.\n", ret);

	return ret;
}

struct meson_uphy_pdata meson_uphy_t6d_pdata = {
	.u2phy_ops = &meson_u2phy_t6d_ops,
	.u3phy_ops = NULL,
	.u2phy_parse = meson_aml_u2phy_t6d_parse,
	.u3phy_parse = NULL,
	.otg_parse = meson_u2phy_crg_otg_parse,
	.otg_remove = meson_u2phy_crg_otg_remove,
};
