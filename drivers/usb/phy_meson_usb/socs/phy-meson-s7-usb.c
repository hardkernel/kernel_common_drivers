// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "../phy-meson-usb.h"

static void meson_usb2phy_s7_set_pll(struct amlogic_usb_v2 *mphy)
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
		return;

	retry--;
	if (!retry)
		return;

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

static int meson_usb2phy_s7_cali(struct amlogic_usb_v2 *mphy, int port)
{
	meson_usb2phy_set_calibration_trim(mphy, port);
	meson_usb2phy_s7_cali_disc_squelch(mphy);
	return 0;
}

static int meson_u2phy_s7_set_mode(struct phy *phy, enum phy_mode mode, int submode)
{
	int ret = 0;
	struct meson_uphy_instance *ist =
		(struct meson_uphy_instance *)phy_get_drvdata(phy);
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)ist->meson_uphy;

	ret = meson_u2phy_set_mode(mphy, 0, mode);

	return ret;
}

static int meson_u2phy_s7_init(struct phy *phy)
{
	int ret;
	struct meson_uphy_instance *ist =
		(struct meson_uphy_instance *)phy_get_drvdata(phy);
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)ist->meson_uphy;
	int port = 0;

	ret = clk_bulk_prepare_enable(mphy->clk_num, mphy->clks);
	if (ret) {
		mu2p_err(mphy->dev, "Failed to enable usb2 phy bus clock at %d\n",
							__LINE__);
	}

	if (port)
		mu2p_err(mphy->dev, "not port 0?.\n");

	meson_u2phy_hold_reset(mphy, port, true);
	meson_u2phy_usb_reset(mphy);
	usleep_range(49, 50);

	mu2p_dbg(mphy->dev, "init r0~r2 0x%x 0x%x 0x%x.\n",
			readl(&mphy->u2p_aml_regs[port]->r0),
			readl(&mphy->u2p_aml_regs[port]->r1),
			readl(&mphy->u2p_aml_regs[port]->r2));

	if (mphy->suspend_flag)
		meson_u2phy_s7_set_mode(phy, mphy->current_mode, 0);

	usleep_range(50, 100);
	meson_u2phy_reset_phycfg(mphy, port);

	meson_usb2phy_s7_cali(mphy, port);

	ret = meson_usb2phy_wait_ready(mphy, port, 200);
	if (ret)
		mu2p_err(mphy->dev, " wait for ready timeout.\n");

	meson_usb2phy_s7_set_pll(mphy);

	mu2p_dbg(mphy->dev, "end r0~r2 0x%x 0x%x 0x%x.\n",
			readl(&mphy->u2p_aml_regs[port]->r0),
			readl(&mphy->u2p_aml_regs[port]->r1),
			readl(&mphy->u2p_aml_regs[port]->r2));

	if (mphy->suspend_flag)
		mphy->suspend_flag = 0;

	return ret;
}

static int meson_u2phy_s7_exit(struct phy *phy)
{
	struct meson_uphy_instance *ist =
		(struct meson_uphy_instance *)phy_get_drvdata(phy);
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)ist->meson_uphy;

	return meson_u2phy_exit(mphy, 0);
}

static int meson_u2phy_s7_power_on(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)
				((struct meson_uphy_instance *)phy_get_drvdata(phy))->meson_uphy;

	return meson_u2phy_power_on(mphy);
}

static int meson_u2phy_s7_power_off(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)
				((struct meson_uphy_instance *)phy_get_drvdata(phy))->meson_uphy;

	return meson_u2phy_power_off(mphy);
}

static int meson_u2phy_s7_configure(struct phy *phy, struct meson_uphy_configure_opts *opts)
{
	return 0;
}

static int meson_u2phy_s7_calibrate(struct phy *phy)
{
	return 0;
}

static int meson_u2phy_s7_connect(struct phy *phy, int port)
{
	return 0;
}

static int meson_u2phy_s7_disconnect(struct phy *phy, int port)
{
	return 0;
}

static struct meson_uphy_ops meson_u2phy_s7_pdata = {
	.init = meson_u2phy_s7_init,
	.exit = meson_u2phy_s7_exit,
	.power_on = meson_u2phy_s7_power_on,
	.power_off = meson_u2phy_s7_power_off,
	.set_mode = meson_u2phy_s7_set_mode,
	.configure = meson_u2phy_s7_configure,
	.calibrate = meson_u2phy_s7_calibrate,
	.connect = meson_u2phy_s7_connect,
	.disconnect = meson_u2phy_s7_disconnect,
};

struct meson_uphy_pdata meson_uphy_s7_pdata = {
	.u2phy_ops = &meson_u2phy_s7_pdata,
	.u3phy_ops = NULL,
	.u2phy_parse = meson_aml_u2phy_parse,
	.u3phy_parse = meson_aml_u3phy_parse,
	.otg_parse = meson_u2phy_crg_otg_parse,
};
