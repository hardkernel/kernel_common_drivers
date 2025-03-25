// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "../phy-meson-usb.h"

static int meson_usb2phy_s7_set_pll(struct amlogic_usb_v2 *mphy)
{
#define USBPLL_LK_RST_BIT	28
#define USBPLL_EN_BIT		11
#define USB2_MPLL_EN_CTRL_BIT 1
#define USBPLL_RST_BIT		0
	u32 retry = 5;
	u32 pll_val0 = mphy->pll_setting[0],
		pll_val1 = mphy->pll_setting[1];
	void __iomem *reg = mphy->phy_cfg[0];

__retry:
	writel(pll_val0 | (1 << USBPLL_LK_RST_BIT) |
		(1 << USB2_MPLL_EN_CTRL_BIT) | (1 << USBPLL_RST_BIT),
		(reg + 0x40));
	usleep_range(100, 150);
	writel(pll_val1, (reg + 0x44));
	usleep_range(100, 150);
	writel(pll_val0 | (1 << USBPLL_LK_RST_BIT) |
		(1 << USB2_MPLL_EN_CTRL_BIT) | (1 << USBPLL_RST_BIT) | (1 << USBPLL_EN_BIT),
		(reg + 0x40));
	usleep_range(100, 150);
	writel(pll_val0 | (1 << USBPLL_LK_RST_BIT) |
		(1 << USB2_MPLL_EN_CTRL_BIT) | (1 << USBPLL_EN_BIT), (reg + 0x40));
	usleep_range(100, 150);
	writel(pll_val0 | (1 << USB2_MPLL_EN_CTRL_BIT) | (1 << USBPLL_EN_BIT),
		(reg + 0x40));
	usleep_range(100, 150);
	//check lock bit
	if (readl((reg + 0x40)) >> 31)
		return 0;

	retry--;
	if (!retry)
		return -ETIMEDOUT;

	goto __retry;
}

static int meson_usb2phy_s7_cali_disc_squelch
			(struct amlogic_usb_v2 *mphy)
{
	/* usb2_squelch_trim: usb2_reg_cfg[28]##reg32_03[2:0] (MSB->LSB) default 0b0111
	 * usb2_disc_trim: usb2_reg_cfg[27]##reg32_03[6:4] (MSB->LSB) default 0b1000
	 */
#define PHY_CRG_DRD_TUNING_DISCONNECT_THRESHOLD_BIT6_0_v2 0x7
	void __iomem *reg = mphy->phy_cfg[0];

	writel(PHY_CRG_DRD_TUNING_DISCONNECT_THRESHOLD_BIT6_0_v2, reg + 0xC);
	// wait for 200us
	usleep_range(200, 300);

	return 0;
}

static void meson_usb2phy_s7_cali(struct amlogic_usb_v2 *mphy)
{
	meson_usb2phy_set_calibration_trim(mphy);
	meson_usb2phy_s7_cali_disc_squelch(mphy);
}

static struct meson_u2phy_priv meson_u2phy_s7_priv = {
	.set_mode = meson_u2phy_set_mode,
	.cali = meson_usb2phy_s7_cali,
	.set_pll = meson_usb2phy_s7_set_pll,
};

static int meson_u2phy_s7_set_mode(void *phy, enum meson_uphy_mode mode)
{
	return meson_u2phy_set_mode((struct amlogic_usb_v2 *)phy, mode);
}

static int meson_u2phy_s7_init(void *phy)
{
	return meson_u2phy_aml_init((struct amlogic_usb_v2 *)phy, &meson_u2phy_s7_priv);
}

static int meson_u2phy_s7_exit(void *phy)
{
	return meson_u2phy_exit((struct amlogic_usb_v2 *)phy);
}

static int meson_u2phy_s7_power_on(void *phy)
{
	return meson_u2phy_power_on((struct amlogic_usb_v2 *)phy);
}

static int meson_u2phy_s7_power_off(void *phy)
{
	return meson_u2phy_power_off((struct amlogic_usb_v2 *)phy);
}

static struct meson_uphy_ops meson_u2phy_s7_ops = {
	.init = meson_u2phy_s7_init,
	.exit = meson_u2phy_s7_exit,
	.power_on = meson_u2phy_s7_power_on,
	.power_off = meson_u2phy_s7_power_off,
	.set_mode = meson_u2phy_s7_set_mode,
};

struct meson_uphy_pdata meson_uphy_s7_pdata = {
	.u2phy_ops = &meson_u2phy_s7_ops,
	.u3phy_ops = NULL,
	.u2phy_parse = meson_aml_u2phy_parse,
	.u3phy_parse = NULL,
	.otg_parse = meson_u2phy_crg_otg_parse,
	.otg_remove = meson_u2phy_crg_otg_remove,
};
