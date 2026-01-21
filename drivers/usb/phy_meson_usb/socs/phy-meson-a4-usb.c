// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "../phy-meson-usb.h"

/* USB 2 */
static void meson_usb2phy_a4_cali(struct amlogic_usb_v2 *mphy)
{
	meson_usb2phy_legacy_cali_disc_squelch_n(mphy);
	meson_usb2phy_legacy_cali_n(mphy);
	meson_usb2phy_set_calibration_trim(mphy);
}

static struct meson_u2phy_priv meson_u2phy_a4_priv = {
	.set_mode = meson_u2phy_set_mode,
	.cali = meson_usb2phy_a4_cali,
	.set_pll = meson_usb2phy_legacy_set_pll,
};

static int meson_u2phy_a4_set_mode(void *phy, enum meson_uphy_mode mode)
{
	return meson_u2phy_set_mode((struct amlogic_usb_v2 *)phy, mode);
}

static int meson_u2phy_a4_init(void *phy)
{
	return meson_u2phy_aml_init((struct amlogic_usb_v2 *)phy, &meson_u2phy_a4_priv);
}

static int meson_u2phy_a4_exit(void *phy)
{
	return meson_u2phy_exit((struct amlogic_usb_v2 *)phy);
}

static int meson_u2phy_a4_power_on(void *phy)
{
	return meson_u2phy_power_on((struct amlogic_usb_v2 *)phy);
}

static int meson_u2phy_a4_power_off(void *phy)
{
	return meson_u2phy_power_off((struct amlogic_usb_v2 *)phy);
}

static struct meson_uphy_ops meson_u2phy_a4_ops = {
	.init = meson_u2phy_a4_init,
	.exit = meson_u2phy_a4_exit,
	.power_on = meson_u2phy_a4_power_on,
	.power_off = meson_u2phy_a4_power_off,
	.set_mode = meson_u2phy_a4_set_mode,
};

struct meson_uphy_pdata meson_uphy_a4_pdata = {
	.u2phy_ops = &meson_u2phy_a4_ops,
	.u2phy_parse = meson_aml_u2phy_parse,
	.otg_parse = meson_u2phy_crg_otg_parse,
	.otg_remove = meson_u2phy_crg_otg_remove,
};
