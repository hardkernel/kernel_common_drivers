// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "../phy-meson-usb.h"

static int meson_usb2phy_txhd2_set_pll(struct amlogic_usb_v2 *mphy)
{
#define USBPLL_RESET_BIT 18
#define USBPLL_LK_RESET_BIT 28
#define USBPLL_EN_BIT 11
	void __iomem *reg = mphy->phy_cfg[0];
	u32 retry = 5;
	u32 pll_val0 = mphy->pll_setting[0],
		pll_val1 = mphy->pll_setting[1];
__retry:
	writel(pll_val0, reg + 0x40);
	writel(pll_val1, reg + 0x44);
	usleep_range(4, 5);
	writel(pll_val1 | (1 << USBPLL_RESET_BIT), reg + 0x44);
	writel((pll_val0 | (1 << USBPLL_LK_RESET_BIT) | (1 << USBPLL_EN_BIT)), reg + 0x40);
	usleep_range(49, 50);
	writel(pll_val1, reg + 0x44);
	usleep_range(49, 50);
	writel((pll_val0 | (1 << USBPLL_EN_BIT)), reg + 0x40);
	usleep_range(49, 50);
	writel(mphy->pll_setting[3], reg + 0x50);
	// wait for 200us
	usleep_range(200, 300);

	//check lock bit
	if (readl(reg + 0x40) >> 31)
		return 0;

	retry--;
	if (!retry)
		return -ETIMEDOUT;

	goto __retry;
}

static int meson_usb2phy_txhd2_cali_disc_squelch
			(struct amlogic_usb_v2 *mphy)
{
#define PHY_CRG_DRD_TUNING_DISCONNECT_THRESHOLD_BIT6_0 0x7f
	void __iomem *reg = mphy->phy_cfg[0];

	writel(PHY_CRG_DRD_TUNING_DISCONNECT_THRESHOLD_BIT6_0, reg + 0xC);
	usleep_range(200, 300);

	return 0;
}

static void meson_usb2phy_txhd2_cali(struct amlogic_usb_v2 *mphy)
{
	meson_usb2phy_set_calibration_trim(mphy);
	meson_usb2phy_txhd2_cali_disc_squelch(mphy);
}

static struct meson_u2phy_priv meson_u2phy_txhd2_priv = {
	.cali = meson_usb2phy_txhd2_cali,
	.set_pll = meson_usb2phy_txhd2_set_pll,
};

static int meson_u2phy_txhd2_set_mode(struct phy *phy, enum phy_mode mode, int submode)
{
	int ret = 0;
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	ret = meson_u2phy_set_mode(mphy, mode);

	return ret;
}

static int meson_u2phy_txhd2_init(struct phy *phy)
{
	return meson_u2phy_aml_init(gphy_to_amlusbv2phy(phy), &meson_u2phy_txhd2_priv);
}

static int meson_u2phy_txhd2_exit(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	return meson_u2phy_exit(mphy);
}

static int meson_u2phy_txhd2_power_on(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	return meson_u2phy_power_on(mphy);
}

static int meson_u2phy_txhd2_power_off(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	return meson_u2phy_power_off(mphy);
}

static struct meson_uphy_ops meson_u2phy_txhd2_ops = {
	.init = meson_u2phy_txhd2_init,
	.exit = meson_u2phy_txhd2_exit,
	.power_on = meson_u2phy_txhd2_power_on,
	.power_off = meson_u2phy_txhd2_power_off,
	.set_mode = meson_u2phy_txhd2_set_mode,
};

struct meson_uphy_pdata meson_uphy_txhd2_pdata = {
	.u2phy_ops = &meson_u2phy_txhd2_ops,
	.u3phy_ops = NULL,
	.u2phy_parse = meson_aml_u2phy_parse,
	.u3phy_parse = NULL,
	.otg_parse = meson_u2phy_crg_otg_parse,
	.otg_remove = meson_u2phy_crg_otg_remove,
};
