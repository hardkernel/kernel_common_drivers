// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "../phy-meson-usb.h"

/* aml phy. */
static void meson_usb2phy_t5m_cali(struct amlogic_usb_v2 *mphy)
{
	meson_usb2phy_legacy_cali_disc_squelch_n(mphy);
	meson_usb2phy_legacy_cali_n(mphy);
	meson_usb2phy_set_calibration_trim(mphy);
}

static struct meson_u2phy_priv meson_u2phy_t5m_priv = {
	.set_mode = meson_u2phy_set_mode,
	.cali = meson_usb2phy_t5m_cali,
	.set_pll = meson_usb2phy_legacy_set_pll,
};

static int meson_u2phy_t5m_set_mode(void *phy, enum meson_uphy_mode mode)
{
	return meson_u2phy_set_mode((struct amlogic_usb_v2 *)phy, mode);
}

static int meson_u2phy_t5m_init(void *phy)
{
	return meson_u2phy_aml_init((struct amlogic_usb_v2 *)phy, &meson_u2phy_t5m_priv);
}

static int meson_u2phy_t5m_exit(void *phy)
{
	return meson_u2phy_exit((struct amlogic_usb_v2 *)phy);
}

static int meson_u2phy_t5m_power_on(void *phy)
{
	return meson_u2phy_power_on((struct amlogic_usb_v2 *)phy);
}

static int meson_u2phy_t5m_power_off(void *phy)
{
	return meson_u2phy_power_off((struct amlogic_usb_v2 *)phy);
}

static struct meson_uphy_ops meson_u2phy_t5m_ops = {
	.init = meson_u2phy_t5m_init,
	.exit = meson_u2phy_t5m_exit,
	.power_on = meson_u2phy_t5m_power_on,
	.power_off = meson_u2phy_t5m_power_off,
	.set_mode = meson_u2phy_t5m_set_mode,
};

struct meson_uphy_pdata meson_uphy_t5m_aml_pdata = {
	.u2phy_ops = &meson_u2phy_t5m_ops,
	.u2phy_parse = meson_aml_u2phy_parse,
};

/* m31 phy. */
static int meson_u2phy_m31_t5m_set_mode(void *phy, enum meson_uphy_mode mode)
{
	return meson_u2phy_set_mode((struct amlogic_usb_v2 *)phy, mode);
}

static int meson_u2phy_m31_t5m_configure(void *phy, struct meson_uphy_configure_opts *opts)
{
	if (opts->test_mode)
		return meson_u2phy_m31_set_test_mode((struct amlogic_usb_v2 *)phy, opts->test_mode);
	return 0;
}

static int meson_u3phy_m31_t5m_set_mode(void *phy, enum meson_uphy_mode mode)
{
	return meson_m31_u3phy_set_mode((struct amlogic_usb_m31 *)phy, mode);
}

static int meson_u3phy_m31_t5m_init(void *phy)
{
	return meson_m31_u3phy_init((struct amlogic_usb_m31 *)phy);
}

static int meson_u3phy_m31_t5m_exit(void *phy)
{
	return meson_m31_u3phy_exit((struct amlogic_usb_m31 *)phy);
}

/* m31 phy usb2 does not need configuration. */
static struct meson_uphy_ops meson_u2phy_t5m_m31_ops = {
	.set_mode = meson_u2phy_m31_t5m_set_mode,
	.configure = meson_u2phy_m31_t5m_configure,
};

static struct meson_uphy_ops meson_u3phy_t5m_m31_ops = {
	.init = meson_u3phy_m31_t5m_init,
	.exit = meson_u3phy_m31_t5m_exit,
	.set_mode = meson_u3phy_m31_t5m_set_mode,
};

struct meson_uphy_pdata meson_uphy_t5m_m31_pdata = {
	.u2phy_ops = &meson_u2phy_t5m_m31_ops,
	.u3phy_ops = &meson_u3phy_t5m_m31_ops,
	.u2phy_parse = meson_m31_u2phy_parse,
	.u3phy_parse = meson_m31_u3phy_parse,
	.otg_parse = meson_u2phy_crg_otg_parse,
	.otg_remove = meson_u2phy_crg_otg_remove,
};

