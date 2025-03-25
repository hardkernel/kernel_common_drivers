// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "../phy-meson-usb.h"

static int meson_usb2phy_s6_set_pll(struct amlogic_usb_v2 *phy)
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
	void __iomem *pll_cfg = phy->phy_cfg[0] + 0x40;

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
		dev_dbg(phy->dev, "usb2 pll not locked, retry. val: 0x%08x\n", readl(pll_cfg));
	}
	dev_err(phy->dev, "%s set pll failed, exit, val:0x%08x.\n", __func__, readl(pll_cfg));
	return -ETIMEDOUT;
okay:
	dev_dbg(phy->dev, "usb2 pll init done, val: 0x%08x\n", readl(pll_cfg));
	return 0;
}

static int meson_usb2phy_s6_cali_disc_squelch
			(struct amlogic_usb_v2 *mphy)
{
/* S6
 * usb2_squelch_trim: reg32_03[3:0] (MSB->LSB) default 0b0111
 * usb2_disc_trim: reg32_03[6:4] (MSB->LSB) default 0b000
 */
#define PHY_CRG_DRD_TUNING_DISCONNECT_THRESHOLD_BIT6_0 0x39
#define USB2_REG_CFG_DIS	27

	/* The s7d default value 0b000 of usb2_disc_trim leads to hs handshake err.
	 * S6 shares the params with s7d.
	 */
	writel(PHY_CRG_DRD_TUNING_DISCONNECT_THRESHOLD_BIT6_0, mphy->phy_cfg[0] + 0xC);
	/* The USB2_REG_CFG_DIS is not used but default set. Clear it.*/
	writel(readl(mphy->phy_cfg[0] + 0x38) & ~BIT(USB2_REG_CFG_DIS),
			mphy->phy_cfg[0] + 0x38);

	return 0;
}

static int meson_usb2phy_s6_set_misc(struct amlogic_usb_v2 *mphy)
{
	/* edgedrv cali for signal quality. */
	writel(mphy->pll_setting[3], mphy->phy_cfg[0] + 0x50);

	return 0;
}

static void meson_usb2phy_s6_cali(struct amlogic_usb_v2 *phy)
{
	meson_usb2phy_set_calibration_trim(phy);
	meson_usb2phy_s6_cali_disc_squelch(phy);
	meson_usb2phy_s6_set_misc(phy);
}

static struct meson_u2phy_priv meson_u2phy_s6_priv = {
	.set_mode = meson_u2phy_set_mode,
	.cali = meson_usb2phy_s6_cali,
	.set_pll = meson_usb2phy_s6_set_pll,
};

static int meson_u2phy_s6_set_mode(void *phy, enum meson_uphy_mode mode)
{
	return meson_u2phy_set_mode((struct amlogic_usb_v2 *)phy, mode);
}

static int meson_u2phy_s6_init(void *phy)
{
	return meson_u2phy_aml_init((struct amlogic_usb_v2 *)phy, &meson_u2phy_s6_priv);
}

static int meson_u2phy_s6_exit(void *phy)
{
	return meson_u2phy_exit((struct amlogic_usb_v2 *)phy);
}

static int meson_u2phy_s6_power_on(void *phy)
{
	return meson_u2phy_power_on((struct amlogic_usb_v2 *)phy);
}

static int meson_u2phy_s6_power_off(void *phy)
{
	return meson_u2phy_power_off((struct amlogic_usb_v2 *)phy);
}

static int meson_u3phy_s6_init(void *phy)
{
	return meson_aml_u3phy_init((struct aml_usb3_phy *)phy);
}

static int meson_u3phy_s6_exit(void *phy)
{
	return meson_aml_u3phy_exit((struct aml_usb3_phy *)phy);
}

static struct meson_uphy_ops meson_u2phy_s6_ops = {
	.init = meson_u2phy_s6_init,
	.exit = meson_u2phy_s6_exit,
	.power_on = meson_u2phy_s6_power_on,
	.power_off = meson_u2phy_s6_power_off,
	.set_mode = meson_u2phy_s6_set_mode,
};

static struct meson_uphy_ops meson_u3phy_s6_ops = {
	.init = meson_u3phy_s6_init,
	.exit = meson_u3phy_s6_exit,
};

struct meson_uphy_pdata meson_uphy_s6_pdata = {
	.u2phy_ops = &meson_u2phy_s6_ops,
	.u3phy_ops = &meson_u3phy_s6_ops,
	.u2phy_parse = meson_aml_u2phy_parse,
	.u3phy_parse = meson_aml_u3phy_parse,
	.otg_parse = meson_u2phy_crg_otg_parse,
	.otg_remove = meson_u2phy_crg_otg_remove,
};
