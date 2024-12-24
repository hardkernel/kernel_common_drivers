// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "../phy-meson-usb.h"

static void meson_usb2phy_s905x5m_set_pll(struct amlogic_usb_v2 *mphy, int port)
{
#define USB2_MPPLL_EN_CTRL_BIT	27
#define USBPLL_BIAS_EN_BIT	26
#define USB2_PLL_RSTN_BIT	25
#define USB2_PLL_LOCK_EN_BIT	24
#define USB2_PLL_DONE		31
	u32 retry = 5;
	int plldone_i;
	u32 pll_val;
	/* default value
	 * USB2_MPPLL_EN_CTRL	0
	 * USBPLL_BIAS_EN	0
	 * USB2_PLL_RSTN	0
	 * USB2_PLL_LOCK_EN	0
	 */
	u32 def_val = 0x549540;
	void __iomem *pll_cfg = mphy->phy_cfg[port] + 0x40;

	pll_val = readl(pll_cfg);
	/*Enable PLL Control*/
	pll_val |= BIT(USB2_MPPLL_EN_CTRL_BIT);
	writel(pll_val, pll_cfg);
	usleep_range(10, 20);

	while (retry--) {
		/* Reset Register value */
		pll_val = def_val | BIT(USB2_MPPLL_EN_CTRL_BIT);
		writel(pll_val, pll_cfg);
		usleep_range(10, 20);
		/* assert usb_pll_bias_en */
		pll_val |= BIT(USBPLL_BIAS_EN_BIT);
		writel(pll_val, pll_cfg);
		/* delay 20μs */
		usleep_range(20, 30);
		/* assert usb_pll_rstn */
		pll_val |= BIT(USB2_PLL_RSTN_BIT);
		writel(pll_val, pll_cfg);
		/* delay 20μs */
		usleep_range(20, 30);
		/* assert usb_pll_lock_en */
		pll_val |= BIT(USB2_PLL_LOCK_EN_BIT);
		writel(pll_val, pll_cfg);
		/* check lock bit */
		for (plldone_i = 5; plldone_i > 0; plldone_i--) {
			usleep_range(20, 30);
			if (readl(pll_cfg) & BIT(USB2_PLL_DONE))
				goto okay;
		}
		mu2p_dbg(mphy->dev, "usb2 pll not locked, retry. val: 0x%08x\n", readl(pll_cfg));
	}
	mu2p_err(mphy->dev, "%s set pll failed, exit, val:0x%08x.\n", __func__, readl(pll_cfg));
	return;
okay:
	mu2p_dbg(mphy->dev, "usb2 pll init done, val: 0x%08x\n", readl(pll_cfg));
}

static void meson_usb2phy_s905x5m_set_calibration_trim
	(struct amlogic_usb_v2 *mphy, int port)
{
	u32 value = 0;
	u32 cali, i;
	u8 cali_en;

	if (!mphy->usb_phy_trim_reg) {
		mu2p_err(mphy->dev, "No usb-phy-trim-reg\n");
		return;
	}

	cali = readl(mphy->usb_phy_trim_reg);
	cali_en = (cali >> 12) & 0x1;
	cali = cali >> 8;
	if (cali_en) {
		cali = cali & 0xf;
		/* s7d modify. */
		cali = cali + 2;
		if (cali > 12)
			cali = 12;
	} else {
		cali = mphy->pll_setting[4];
	}
	value = readl(mphy->phy_cfg[port] + 0x10);
	value &= (~0xfff);
	for (i = 0; i < cali; i++)
		value |= (1 << i);

	writel(value, mphy->phy_cfg[port] + 0x10);

	mu2p_info(mphy->dev, "phy trim value= 0x%08x\n", value);
}

static int meson_usb2phy_s905x5m_cali_disc_squelch
			(struct amlogic_usb_v2 *mphy, int port)
{
/* S7D/S6
 * usb2_squelch_trim: reg32_03[3:0] (MSB->LSB) default 0b0111
 * usb2_disc_trim: reg32_03[6:4] (MSB->LSB) default 0b000
 */
#define PHY_CRG_DRD_TUNING_DISCONNECT_THRESHOLD_BIT6_0 0x39
#define USB2_REG_CFG_DIS	27

	/* The s7d default value 0b000 of usb2_disc_trim leads to hs handshake err.
	 * S6 shares the params with s7d.
	 */
	writel(PHY_CRG_DRD_TUNING_DISCONNECT_THRESHOLD_BIT6_0, mphy->phy_cfg[port] + 0xC);
	/* The USB2_REG_CFG_DIS is not used but default set. Clear it.*/
	writel(readl(mphy->phy_cfg[port] + 0x38) & ~BIT(USB2_REG_CFG_DIS),
			mphy->phy_cfg[port] + 0x38);

	return 0;
}

static int meson_usb2phy_s905x5m_set_misc(struct amlogic_usb_v2 *mphy, int port)
{
	/* edgedrv cali for signal quality. */
	writel(mphy->pll_setting[3], mphy->phy_cfg[port] + 0x50);

	return 0;
}

static int meson_usb2phy_s905x5m_cali(struct amlogic_usb_v2 *mphy, int port)
{
	int ret;

	meson_usb2phy_s905x5m_set_calibration_trim(mphy, port);
	ret = meson_usb2phy_s905x5m_cali_disc_squelch(mphy, port);
	if (ret)
		goto done;
	ret = meson_usb2phy_s905x5m_set_misc(mphy, port);
	if (ret)
		goto err;

done:
	return ret;
err:
	return ret;
}

int meson_u2phy_s905x5m_set_mode(struct phy *phy, enum phy_mode mode, int submode)
{
	int ret = 0;
	struct meson_uphy_instance *ist =
		(struct meson_uphy_instance *)phy_get_drvdata(phy);
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)ist->meson_uphy;

	ret = meson_u2phy_set_mode(mphy, ist->index, mode);

	return ret;
}

int meson_u2phy_s905x5m_init(struct phy *phy)
{
	int ret;
	struct meson_uphy_instance *ist =
		(struct meson_uphy_instance *)phy_get_drvdata(phy);
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)ist->meson_uphy;
	int port = ist->index;

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
		meson_u2phy_s905x5m_set_mode(phy, mphy->last_mode, 0);

	usleep_range(50, 100);
	meson_u2phy_reset_phycfg(mphy, port);

	meson_usb2phy_s905x5m_cali(mphy, port);

	ret = meson_usb2phy_wait_ready(mphy, port, 200);
	if (ret)
		mu2p_err(mphy->dev, " wait for ready timeout.\n");

	meson_usb2phy_s905x5m_set_pll(mphy, port);

	mu2p_dbg(mphy->dev, "end r0~r2 0x%x 0x%x 0x%x.\n",
			readl(&mphy->u2p_aml_regs[port]->r0),
			readl(&mphy->u2p_aml_regs[port]->r1),
			readl(&mphy->u2p_aml_regs[port]->r2));

	if (mphy->suspend_flag)
		mphy->suspend_flag = 0;

	return ret;
}

int meson_u2phy_s905x5m_exit(struct phy *phy)
{
	struct meson_uphy_instance *ist =
		(struct meson_uphy_instance *)phy_get_drvdata(phy);
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)ist->meson_uphy;

	return meson_u2phy_exit(mphy, ist->index);
}

int meson_u2phy_s905x5m_power_on(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)
				((struct meson_uphy_instance *)phy_get_drvdata(phy))->meson_uphy;

	return meson_u2phy_power_on(mphy);
}

int meson_u2phy_s905x5m_power_off(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)
				((struct meson_uphy_instance *)phy_get_drvdata(phy))->meson_uphy;

	return meson_u2phy_power_off(mphy);
}

int meson_u2phy_s905x5m_configure(struct phy *phy, struct meson_uphy_configure_opts *opts)
{
	return 0;
}

int meson_u2phy_s905x5m_calibrate(struct phy *phy)
{
	return 0;
}

int meson_u2phy_s905x5m_connect(struct phy *phy, int port)
{
	return 0;
}

int meson_u2phy_s905x5m_disconnect(struct phy *phy, int port)
{
	return 0;
}

static struct meson_uphy_ops meson_u2phy_s905x5m_pdata = {
	.init = meson_u2phy_s905x5m_init,
	.exit = meson_u2phy_s905x5m_exit,
	.power_on = meson_u2phy_s905x5m_power_on,
	.power_off = meson_u2phy_s905x5m_power_off,
	.set_mode = meson_u2phy_s905x5m_set_mode,
	.configure = meson_u2phy_s905x5m_configure,
	.calibrate = meson_u2phy_s905x5m_calibrate,
	.connect = meson_u2phy_s905x5m_connect,
	.disconnect = meson_u2phy_s905x5m_disconnect,
};

struct meson_uphy_pdata meson_uphy_s905x5m_pdata = {
	.u2phy_ops = &meson_u2phy_s905x5m_pdata,
	.u3phy_ops = NULL,
	.u2phy_parse = meson_aml_u2phy_parse,
	.u3phy_parse = meson_aml_u3phy_parse,
	.otg_parse = meson_u2phy_crg_otg_parse,
};
