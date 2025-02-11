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

static void meson_usb2phy_s7_set_calibration_trim(struct amlogic_usb_v2 *phy)
{
	u32 value = 0;
	u32 cali, i;
	u8 cali_en;

	if (!phy->usb_phy_trim_reg) {
		mup_err(phy->dev, "No usb-phy-trim-reg\n");
		return;
	}

	cali = readl(phy->usb_phy_trim_reg);
	cali_en = (cali >> 12) & 0x1;
	cali = cali >> 8;
	if (cali_en) {
		cali = cali & 0xf;
		/* s7 modify. */
		cali = cali + 2;
		if (cali > 12)
			cali = 12;
	} else {
		cali = phy->pll_setting[4];
	}
	value = readl(phy->phy_cfg[0] + 0x10);
	value &= (~0xfff);
	for (i = 0; i < cali; i++)
		value |= (1 << i);

	writel(value, phy->phy_cfg[0] + 0x10);

	mup_dbg(phy->dev, "phy trim value= 0x%08x\n", value);
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

static int meson_usb2phy_s7_cali(struct amlogic_usb_v2 *mphy)
{
	meson_usb2phy_s7_set_calibration_trim(mphy);
	meson_usb2phy_s7_cali_disc_squelch(mphy);
	return 0;
}

static int meson_u2phy_s7_set_mode(struct phy *phy, enum phy_mode mode, int submode)
{
	int ret = 0;
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	ret = meson_u2phy_set_mode(mphy, mode);

	return ret;
}

static int meson_u2phy_s7_init(struct phy *phy)
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
		meson_u2phy_s7_set_mode(phy, mphy->current_mode, 0);

	usleep_range(50, 100);
	meson_u2phy_reset_phycfg(mphy);

	meson_usb2phy_s7_cali(mphy);

	ret = meson_usb2phy_wait_ready(mphy, 200);
	if (ret)
		mup_err(mphy->dev, " wait for ready timeout.\n");

	meson_usb2phy_s7_set_pll(mphy);

	mup_dbg(mphy->dev, "end r0~r2 0x%x 0x%x 0x%x.\n",
			readl(&mphy->u2p_aml_regs[0]->r0),
			readl(&mphy->u2p_aml_regs[0]->r1),
			readl(&mphy->u2p_aml_regs[0]->r2));

	if (mphy->suspend_flag)
		mphy->suspend_flag = 0;

	return ret;
}

static int meson_u2phy_s7_exit(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	return meson_u2phy_exit(mphy);
}

static int meson_u2phy_s7_power_on(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	return meson_u2phy_power_on(mphy);
}

static int meson_u2phy_s7_power_off(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

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

static struct meson_uphy_ops meson_u2phy_s7_ops = {
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
	.u2phy_ops = &meson_u2phy_s7_ops,
	.u3phy_ops = NULL,
	.u2phy_parse = meson_aml_u2phy_parse,
	.u3phy_parse = NULL,
	.otg_parse = meson_u2phy_crg_otg_parse,
};
