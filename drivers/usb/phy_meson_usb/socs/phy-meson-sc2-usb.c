// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "../phy-meson-usb.h"

static int meson_usb2phy_sc2_cali(struct amlogic_usb_v2 *mphy)
{
	meson_usb2phy_legacy_cali(mphy);
	meson_usb2phy_legacy_cali_disc_squelch(mphy);
	return 0;
}

static int meson_u2phy_sc2_set_mode(struct phy *phy, enum phy_mode mode, int submode)
{
	int ret = 0;
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	ret = meson_u2phy_legacy_set_mode(mphy, mode);

	return ret;
}

static int meson_u2phy_sc2_init(struct phy *phy)
{
	int ret;
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	if (!mphy->suspend_flag) {
		mup_err(mphy->dev, "%s excessive init\n", __func__);
		return -EBUSY;
	}

	ret = clk_bulk_prepare_enable(mphy->clk_num, mphy->clks);
	if (ret) {
		mup_err(mphy->dev, "Failed to enable usb2 phy bus clock at %d\n",
							__LINE__);
	}

	meson_u2phy_hold_reset(mphy, true);
	meson_u2phy_usb_reset(mphy);
	usleep_range(49, 50);

	mup_dbg(mphy->dev, "init r0~r2 0x%x 0x%x 0x%x.\n",
			readl(&mphy->u2p_aml_regs[0]->r0),
			readl(&mphy->u2p_aml_regs[0]->r1),
			readl(&mphy->u2p_aml_regs[0]->r2));

	if (mphy->suspend_flag && mphy->current_mode != 0)
		meson_u2phy_sc2_set_mode(phy, mphy->current_mode, 0);

	usleep_range(50, 100);
	meson_u2phy_reset_phycfg(mphy);
	usleep_range(50, 100);

	meson_usb2phy_sc2_cali(mphy);

	ret = meson_usb2phy_wait_ready(mphy, 200);
	if (ret)
		mup_err(mphy->dev, " wait for ready timeout.\n");

	meson_usb2phy_legacy_set_pll(mphy);
	//set_trim_initvalue(phy, mphy->phy_cfg[i], i);

	mup_dbg(mphy->dev, "end r0~r2 0x%x 0x%x 0x%x.\n",
			readl(&mphy->u2p_aml_regs[0]->r0),
			readl(&mphy->u2p_aml_regs[0]->r1),
			readl(&mphy->u2p_aml_regs[0]->r2));

	if (mphy->suspend_flag)
		mphy->suspend_flag = 0;

	return ret;
}

static int meson_u2phy_sc2_exit(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	return meson_u2phy_exit(mphy);
}

/* For runtime port mux, the phy_power_on & phy_power_off cannot
 * be called during role switch by the controller driver. The otg
 * port power is handled by the otg driver.
 */
static int meson_u2phy_sc2_power_on(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	if (mphy->otg_helper.otg_port)
		return 0;

	return meson_u2phy_power_on(mphy);
}

static int meson_u2phy_sc2_power_off(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	if (mphy->otg_helper.otg_port)
		return 0;

	return meson_u2phy_power_off(mphy);
}

int meson_u2phy_sc2_configure(struct phy *phy, struct meson_uphy_configure_opts *opts)
{
	int ret = 0;

	if (opts->otg_init) {
		ret = meson_uphy_rtmux_otg_init(gphy_to_amlusbv2phy(phy));
		if (ret)
			return ret;
	}

	return 0;
}

static int meson_u2phy_sc2_validate(struct phy *phy, enum phy_mode mode, int submode,
				struct meson_uphy_configure_opts *opts)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	if (mode == mphy->current_mode)
		return 1;
	else
		return 0;
}

static int meson_u3phy_sc2_init(struct phy *phy)
{
	return meson_synopsis_u3phy_init(gphy_to_amlusbv2phy(phy));
}

static int meson_u3phy_sc2_exit(struct phy *phy)
{
	return meson_synopsis_u3phy_exit(gphy_to_amlusbv2phy(phy));
}

static struct meson_uphy_ops meson_u2phy_sc2_ops = {
	.init = meson_u2phy_sc2_init,
	.exit = meson_u2phy_sc2_exit,
	.power_on = meson_u2phy_sc2_power_on,
	.power_off = meson_u2phy_sc2_power_off,
	.set_mode = meson_u2phy_sc2_set_mode,
	.configure = meson_u2phy_sc2_configure,
	.validate = meson_u2phy_sc2_validate,
};

static struct meson_uphy_ops meson_u3phy_sc2_ops = {
	.init = meson_u3phy_sc2_init,
	.exit = meson_u3phy_sc2_exit,
};

struct meson_uphy_pdata meson_uphy_sc2_pdata = {
	.u2phy_ops = &meson_u2phy_sc2_ops,
	.u3phy_ops = &meson_u3phy_sc2_ops,
	.u2phy_parse = meson_aml_u2phy_parse,
	.u3phy_parse = meson_synopsis_u3phy_parse,
	.otg_parse = meson_uphy_rtmux_otg_parse,
	.otg_remove = meson_uphy_rtmux_otg_remove,
};
