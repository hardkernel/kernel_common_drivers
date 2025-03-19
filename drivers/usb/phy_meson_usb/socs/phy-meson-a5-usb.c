// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "../phy-meson-usb.h"

/* USB 2 */
static void meson_usb2phy_a5_cali(struct amlogic_usb_v2 *mphy)
{
	meson_usb2phy_legacy_cali_disc_squelch_n(mphy);
	meson_usb2phy_legacy_cali_n(mphy);
	meson_usb2phy_set_calibration_trim(mphy);
}

static struct meson_u2phy_priv meson_u2phy_a5_priv = {
	.cali = meson_usb2phy_a5_cali,
	.set_pll = meson_usb2phy_legacy_set_pll,
};

static int meson_u2phy_a5_set_mode(struct phy *phy, enum phy_mode mode, int submode)
{
	int ret = 0;
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	ret = meson_u2phy_set_mode(mphy, mode);

	return ret;
}

static int meson_u2phy_a5_init(struct phy *phy)
{
	return meson_u2phy_aml_init(gphy_to_amlusbv2phy(phy), &meson_u2phy_a5_priv);
}

static int meson_u2phy_a5_exit(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	return meson_u2phy_exit(mphy);
}

static int meson_u2phy_a5_power_on(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	return meson_u2phy_power_on(mphy);
}

static int meson_u2phy_a5_power_off(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	return meson_u2phy_power_off(mphy);
}

static struct meson_uphy_ops meson_u2phy_a5_ops = {
	.init = meson_u2phy_a5_init,
	.exit = meson_u2phy_a5_exit,
	.power_on = meson_u2phy_a5_power_on,
	.power_off = meson_u2phy_a5_power_off,
	.set_mode = meson_u2phy_a5_set_mode,
};

struct meson_uphy_pdata meson_uphy_a5_pdata = {
	.u2phy_ops = &meson_u2phy_a5_ops,
	.u2phy_parse = meson_aml_u2phy_parse,
	.otg_parse = meson_u2phy_crg_otg_v2_parse,
	.otg_remove = meson_u2phy_crg_otg_v2_remove,
};
