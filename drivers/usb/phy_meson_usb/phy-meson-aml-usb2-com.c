// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "phy-meson-usb.h"

/* Meson aml usb2 phy. */

/* Reset usb controller. */
int meson_u2phy_usb_reset(struct amlogic_usb_v2 *phy)
{
	static int	init_count;

	mup_dbg(phy->dev, "%s initial: 0x%x.\n", __func__,
				readl(phy->reset_regs + phy->reset_level));
	if (!init_count) {
		init_count++;
		if (phy->usb_reset_bit == 2)
			writel((readl(phy->reset_regs) |
				(0x1 << phy->usb_reset_bit)), phy->reset_regs);
		else
			writel((0x1 << phy->usb_reset_bit), phy->reset_regs);
	}
	mup_dbg(phy->dev, "%s after: 0x%x.\n", __func__,
				readl(phy->reset_regs + phy->reset_level));
	return 0;
}

int meson_u2phy_usb_hold_reset(struct amlogic_usb_v2 *phy, bool on)
{
	u32 val = 0;
	size_t mask = 0;
	u32 tmp = 0;

	mup_dbg(phy->dev, "%s initial: 0x%x.\n", __func__,
			readl(phy->reset_regs + phy->reset_level));

	mask = (size_t)phy->reset_regs & 0xf;

	if (phy->usb_reset_bit != -1U)
		tmp |= BIT(phy->usb_reset_bit);

	val = readl((void __iomem *)
		((unsigned long)phy->reset_regs + (phy->reset_level - mask)));

	if (on) {
		writel(val | tmp, (void __iomem	*)
			((unsigned long)phy->reset_regs + (phy->reset_level - mask)));
	} else {
		writel(val & ~tmp, (void __iomem *)
			((unsigned long)phy->reset_regs + (phy->reset_level - mask)));
	}

	mup_dbg(phy->dev, "%s after: 0x%x.\n", __func__,
			readl(phy->reset_regs + phy->reset_level));

	return 0;
}

int meson_u2phy_comb_hold_reset(struct amlogic_usb_v2 *phy, bool on)
{
	u32 val = 0;
	size_t mask = 0;
	u32 tmp = 0;

	mup_dbg(phy->dev, "%s initial: 0x%x.\n", __func__,
			readl(phy->reset_regs + phy->reset_level));

	mask = (size_t)phy->reset_regs & 0xf;

	if (phy->usb_comb_reset_bit != -1U)
		tmp |= BIT(phy->usb_comb_reset_bit);

	val = readl((void __iomem *)
		((unsigned long)phy->reset_regs + (phy->reset_level - mask)));

	if (on) {
		writel(val | tmp, (void __iomem	*)
			((unsigned long)phy->reset_regs + (phy->reset_level - mask)));
	} else {
		writel(val & ~tmp, (void __iomem *)
			((unsigned long)phy->reset_regs + (phy->reset_level - mask)));
	}

	mup_dbg(phy->dev, "%s after: 0x%x.\n", __func__,
			readl(phy->reset_regs + phy->reset_level));

	return 0;
}

int meson_u2phy_reset_comb(struct amlogic_usb_v2 *phy)
{
	meson_u2phy_comb_hold_reset(phy, false);
	usleep_range(50, 100);
	meson_u2phy_comb_hold_reset(phy, true);

	return 0;
}

int meson_u2phy_hold_reset(struct amlogic_usb_v2 *phy, bool on)
{
	u32 val = 0, temp = 0;
	size_t mask = 0;

	mup_dbg(phy->dev, "%s initial: 0x%x.\n", __func__,
				readl(phy->reset_regs + phy->reset_level));
	mask = (size_t)phy->reset_regs & 0xf;

	temp = temp | (1 << phy->phy_reset_level_bit[0]);

	val = readl((void __iomem		*)
		((unsigned long)phy->reset_regs + (phy->reset_level - mask)));

	if (on) {
		writel(val | temp, (void __iomem	*)
			((unsigned long)phy->reset_regs + (phy->reset_level - mask)));
	} else {
		writel(val & ~temp, (void __iomem	*)
			((unsigned long)phy->reset_regs + (phy->reset_level - mask)));
	}
	mup_dbg(phy->dev, "%s after: 0x%x.\n", __func__,
				readl(phy->reset_regs + phy->reset_level));

	return 0;
}

/* Reset phy hw state machine. */
int meson_u2phy_reset_phycfg(struct amlogic_usb_v2 *phy)
{
	meson_u2phy_hold_reset(phy, false);
	usleep_range(50, 100);
	meson_u2phy_hold_reset(phy, true);

	return 0;
}

/* Reg reset: alias for comb reset in some socs.
 * The reg reset in t6d etc. is needed for usb2 960M mode but is known
 * to reset both phy apb and comb apb. So the comb apb is need to be
 * reinit-ed every time the controller driver resume which calls phy init.
 *
 * Normal u2phy reset scheme   T6D u2phy reset scheme
 * ------------					------------
 * |   PHY	  | <---phy reset	|	PHY    | <---phy reset
 * ------------		|			------------
 * | PHY APB  | <----			| PHY APB  | <---
 * ------------					------------	|
 *												|
 * ------------					------------	|
 * |	Xhc   | <---usb reset	|	 Xhc   | <---usb reset
 * ------------					------------	|
 * | COMB APB | <---comb reset	| COMB APB | <--+comb reset
 * ------------					------------
 */
int meson_u2phy_reg_hold_reset(struct amlogic_usb_v2 *phy, bool on)
{
	u32 val = 0, temp = 0;
	size_t mask = 0;

	mup_dbg(phy->dev, "%s initial: 0x%x.\n", __func__,
				readl(phy->reset_regs + phy->reset_level));

	mask = (size_t)phy->reset_regs & 0xf;

	if (phy->phy_reg_reset_bit != -1U)
		temp = temp | (1 << phy->phy_reg_reset_bit);

	val = readl((void __iomem		*)
		((unsigned long)phy->reset_regs + (phy->reset_level - mask)));

	if (on) {
		writel(val | temp, (void __iomem	*)
			((unsigned long)phy->reset_regs + (phy->reset_level - mask)));
	} else {
		writel(val & ~temp, (void __iomem	*)
			((unsigned long)phy->reset_regs + (phy->reset_level - mask)));
	}
	mup_dbg(phy->dev, "%s after: 0x%x.\n", __func__,
				readl(phy->reset_regs + phy->reset_level));

	return 0;
}

int meson_u2phy_reg_reset(struct amlogic_usb_v2 *phy)
{
	int ret = 0;

	ret = meson_u2phy_reg_hold_reset(phy, false);
	if (ret)
		goto done;
	ret = meson_u2phy_reg_hold_reset(phy, true);
	if (ret)
		goto err;
done:
	return ret;
err:
	return ret;
}

void meson_u2phy_set_vbus_power(struct amlogic_usb_v2 *phy, bool is_power_on)
{
	mup_dbg(phy->dev, "%s %d pin:%d.\n", __func__, is_power_on, phy->vbus_power_pin);

	if (phy->vbus_power_pin == -1)
		return;

	if (is_power_on)
		gpiod_direction_output(phy->usb_gpio_desc, 1);
	else
		gpiod_direction_output(phy->usb_gpio_desc, 0);
}

int meson_usb2phy_wait_ready(struct amlogic_usb_v2 *phy, unsigned int timeout)
{
	union u2p_r1_v2 reg1;
	unsigned int cnt = 0;
	int ret = 0;

	reg1.d32 = readl(&phy->u2p_aml_regs[0]->r1);

	while (reg1.b.phy_rdy != 1) {
		reg1.d32 = readl(&phy->u2p_aml_regs[0]->r1);
		/*we wait phy ready max 1ms, common is 100us*/
		if (cnt > timeout) {
			ret = -ETIMEDOUT;
			break;
		}
		cnt++;
		udelay(5);
	}

	return ret;
}

int meson_u2phy_exit(struct amlogic_usb_v2 *phy)
{
	int ret = 0;

	if (phy->suspend_flag) {
		mup_err(phy->dev, "%s excessive exit\n", __func__);
		return -EBUSY;
	}

	ret = meson_u2phy_hold_reset(phy, false);
	ret = meson_u2phy_reg_hold_reset(phy, false);
	//ret = meson_u2phy_comb_hold_reset(phy, false);

	if (phy->suspend_flag == 0)
		clk_bulk_disable_unprepare(phy->clk_num, phy->clks);

	phy->suspend_flag = 1;

	return ret;
}

int meson_u2phy_power_on(struct amlogic_usb_v2 *phy)
{
	meson_u2phy_set_vbus_power(phy, 1);
	return 0;
}

int meson_u2phy_power_off(struct amlogic_usb_v2 *phy)
{
	meson_u2phy_set_vbus_power(phy, 0);
	return 0;
}

int meson_usb2phy_legacy_set_pll(struct amlogic_usb_v2 *phy)
{
	void __iomem	*reg = phy->phy_cfg[0];

	/* set usb  PLL */
	writel((0x30000000 | (phy->pll_setting[0])), reg + 0x40);
	writel(phy->pll_setting[1], reg + 0x44);
	writel(phy->pll_setting[2], reg + 0x48);
	usleep_range(100, 150);
	writel((0x10000000 | (phy->pll_setting[0])), reg + 0x40);

	return 0;
}

/* The set_usb_phy_trim_tuning in the phy driver before is not used since k5.15
 * due to GKI requirements.
 */

/* FIXME: Some socs will enter this with no pll_dis_thred_enhance prop. Is it a BUG? */
void meson_usb2phy_legacy_cali_disc_squelch(struct amlogic_usb_v2 *phy)
{
	void __iomem	*reg = phy->phy_cfg[0];
	u32 val;

	/* BIT_5_0 set to MAX. */
	writel(0x3f, reg + 0xC);

	/* Extended disconnect threshold bit[26-27] */
	val = readl(reg + 0x38);
	val &= ~0xc000000;
	val |= (phy->pll_dis_thred_enhance << 26 & 0xc000000);
	writel(val, reg + 0x38);
}

void meson_usb2phy_legacy_cali_disc_squelch_n(struct amlogic_usb_v2 *phy)
{
	void __iomem	*reg = phy->phy_cfg[0];
	u32 val;

	/* BIT_5_0 set to MAX. */
	writel(0x3f, reg + 0xC);

	val = readl(reg + 0x38);
	val &= ~0x18000000;
	val |= (phy->pll_dis_thred_enhance << 27 & 0x18000000);
	writel(val, reg + 0x38);
}

/* i.e. phy_version != 3 && phy_version != 0 */
void meson_usb2phy_legacy_cali(struct amlogic_usb_v2 *phy)
{
	u32 val;
	void __iomem	*reg = phy->phy_cfg[0];

	val = readl(reg + 0x08);
	val &= 0xfff;
	if (val == 0)
		val = 0x800001f;
	writel(val | readl(reg + 0x10), reg + 0x10);

	writel(phy->pll_setting[3], reg + 0x50);
	writel(0x2a, reg + 0x54);
	writel(0x78000, reg + 0x34);
}

/* i.e. phy_version == 3 */
void meson_usb2phy_legacy_cali_n(struct amlogic_usb_v2 *phy)
{
	void __iomem	*reg = phy->phy_cfg[0];

	/**phy_version == 3, 0x10 set value in usb_set_calibration_trim**/
	/**phy_version == 3, 0x08 no longer contains the default value of 0x10**/

	writel(phy->pll_setting[3], reg + 0x50);
	writel(0x2a, reg + 0x54);
	writel(0x78000, reg + 0x34);
}

/* Legacy multi-port roothub. Typical seen in socs contains the dwc2_a pcd.*/

void meson_u2phy_phy_legacy_device_tuning(struct amlogic_usb_v2 *phy, bool tune)
{
	void __iomem	*phy_reg_base;

	if (phy->phy_version)
		return;

	if (tune == phy->phy_cfg_state[0])
		return;

	phy_reg_base = phy->phy_cfg[0];
	dev_info(phy->dev, "---%s tuning for device cf(%ps)--\n",
		tune ? "Recovery" : "Set", __builtin_return_address(0));
	if (!tune) {
		writel(phy->pll_setting[7], phy_reg_base + 0x38);
		writel(phy->pll_setting[5], phy_reg_base + 0x34);
	} else {
		writel(0, phy_reg_base + 0x38);
		writel(phy->pll_setting[5], phy_reg_base + 0x34);
	}
	phy->phy_cfg_state[0] = tune;
}

static void meson_u2phy_legacy_force_disable_xhci_port_a(struct amlogic_usb_v2 *phy)
{
	union u2p_r2_v2 reg2;
	union usb_r5_v2 r5;

	if (!phy->otg_helper.otg_port)
		return;

	if (phy->xhci_port_a_addr) {
		writel(0xa0, phy->xhci_port_a_addr);
		if (phy->otg_helper.iddig_type == 0) {
			reg2.d32 = readl(&phy->u2p_aml_regs[0]->r2);
			if (reg2.b.iddig_curr != 1)
				mup_err(phy->dev, "%s id state mismatch. expect 1, actual %d.\n",
							__func__, reg2.b.iddig_curr);
		} else {
			r5.d32 = readl(&phy->usb_aml_regs->r5);
			if (r5.b.iddig_curr != 1)
				mup_err(phy->dev, "%s id state mismatch. expect 1, actual %d.\n",
							__func__, r5.b.iddig_curr);
		}
	}
}

static void meson_u2phy_legacy_resume_xhci_port_a(struct amlogic_usb_v2 *phy)
{
	union u2p_r2_v2 reg2;
	union usb_r5_v2 r5;

	if (!phy->otg_helper.otg_port)
		return;

	if (phy->xhci_port_a_addr) {
		writel(0x2a0, phy->xhci_port_a_addr);

		if (phy->otg_helper.iddig_type == 0) {
			reg2.d32 = readl(&phy->u2p_aml_regs[0]->r2);
			if (reg2.b.iddig_curr != 0)
				mup_err(phy->dev, "%s id state mismatch. expect 0, actual %d.\n",
							__func__, reg2.b.iddig_curr);
		} else {
			r5.d32 = readl(&phy->usb_aml_regs->r5);
			if (r5.b.iddig_curr != 0)
				mup_err(phy->dev, "%s id state mismatch. expect 0, actual %d.\n",
							__func__, r5.b.iddig_curr);
		}
	}
}

int meson_u2phy_legacy_set_mode(struct amlogic_usb_v2 *phy, enum phy_mode mode)
{
	union u2p_r0_v2 reg0;
	union u2p_r2_v2 reg2;
	union usb_r0_v2 r0 = {.d32 = 0};
	union usb_r1_v2 r1 = {.d32 = 0};
	union usb_r4_v2 r4 = {.d32 = 0};
	union usb_r5_v2 r5 = {.d32 = 0};
	void __iomem *cfg = phy->phy_cfg[0];
	int ret = 0;

	mup_dbg(phy->dev, "%s: set mode %d.\n", __func__, mode);

	switch (mode) {
	case PHY_MODE_USB_HOST:
		mup_dbg(phy->dev, "%s: reg0:0x%x.\n", __func__,
				readl(&phy->u2p_aml_regs[0]->r0));
		meson_u2phy_legacy_resume_xhci_port_a(phy);
		/* FIXME: SOCs ported yet are observed that only the otg port has the
		 * usb_aml_regs. Are there any violations?
		 */
		if (phy->otg_helper.otg_port && phy->usb_aml_regs) {
			r0.d32 = readl(&phy->usb_aml_regs->r0);
			r1.d32 = readl(&phy->usb_aml_regs->r1);
			r4.d32 = readl(&phy->usb_aml_regs->r4);
			r0.b.u2d_act = 0;
			writel(r0.d32, &phy->usb_aml_regs->r0);
			WARN_ON(phy->phy_id > 3);
			r1.b.u3h_host_u2_port_disable &= ~BIT(phy->phy_id);
			writel(r1.d32, &phy->usb_aml_regs->r1);
			r4.b.p21_SLEEPM0 = 0x0;
			writel(r4.d32, &phy->usb_aml_regs->r4);
		}

		reg0.d32 = readl(&phy->u2p_aml_regs[0]->r0);
		reg0.b.host_device = 1;
		reg0.b.POR = 0;
		writel(reg0.d32, &phy->u2p_aml_regs[0]->r0);
		mup_dbg(phy->dev, "%s: reg0:0x%x.\n", __func__,
				readl(&phy->u2p_aml_regs[0]->r0));
		phy->current_mode = PHY_MODE_USB_HOST;
		break;
	case PHY_MODE_USB_DEVICE:
		mup_dbg(phy->dev, "%s: reg0:0x%x.\n", __func__,
				readl(&phy->u2p_aml_regs[0]->r0));
		meson_u2phy_legacy_force_disable_xhci_port_a(phy);

		if (phy->otg_helper.otg_port && phy->usb_aml_regs) {
			r0.d32 = readl(&phy->usb_aml_regs->r0);
			r1.d32 = readl(&phy->usb_aml_regs->r1);
			r4.d32 = readl(&phy->usb_aml_regs->r4);

			r0.b.u2d_act = 1;
			r0.b.u2d_ss_scaledown_mode = 0;
			writel(r0.d32, &phy->usb_aml_regs->r0);

			r4.b.p21_SLEEPM0 = 0x1;
			writel(r4.d32, &phy->usb_aml_regs->r4);

			/* Bit mapped port to host disable map */
			WARN_ON(phy->phy_id > 3);
			r1.b.u3h_host_u2_port_disable |= BIT(phy->phy_id);
			writel(r1.d32, &phy->usb_aml_regs->r1);
		}
		reg0.d32 = readl(&phy->u2p_aml_regs[0]->r0);
		reg0.b.host_device = 0;
		reg0.b.POR = 0;
		writel(reg0.d32, &phy->u2p_aml_regs[0]->r0);
		mup_dbg(phy->dev, "%s: reg0:0x%x.\n", __func__,
				readl(&phy->u2p_aml_regs[0]->r0));

		phy->current_mode = PHY_MODE_USB_DEVICE;
		break;
	case PHY_MODE_USB_OTG:
		mup_dbg(phy->dev, "%s: reg0:0x%x.\n", __func__,
				readl(&phy->u2p_aml_regs[0]->r0));
		mup_dbg(phy->dev, "%s: r5:0x%x.\n", __func__,
				readl(&phy->usb_aml_regs->r5));
		if (phy->otg_helper.iddig_type == 0) {
			reg2.d32 = readl(&phy->u2p_aml_regs[0]->r2);
			reg2.b.iddig_en0 = 1;
			reg2.b.iddig_en1 = 1;
			reg2.b.iddig_th = 255;
			writel(reg2.d32, &phy->u2p_aml_regs[0]->r2);
		} else if (phy->otg_helper.iddig_type == 1) {
			r5.d32 = readl(&phy->usb_aml_regs->r5);
			r5.b.iddig_en0 = 1;
			r5.b.iddig_en1 = 1;
			r5.b.iddig_th = 255;
			writel(r5.d32, &phy->usb_aml_regs->r5);
			mup_dbg(phy->dev, "%s: r5:0x%x.\n", __func__,
					readl(&phy->usb_aml_regs->r5));
		}

		reg0.d32 = readl(&phy->u2p_aml_regs[0]->r0);
		/* usb2_otg_iddet_en set to 1. */
		reg0.b.IDPULLUP0 = 1;
		/* usb2_otg_vbusdet_en set to 1. */
		reg0.b.DRVVBUS0 = 1;
		writel(reg0.d32, &phy->u2p_aml_regs[0]->r0);
		mup_dbg(phy->dev, "%s: reg0:0x%x.\n", __func__,
				readl(&phy->u2p_aml_regs[0]->r0));
		/* ID DETECT: usb2_otg_aca_en set to 0 */
		writel(readl(cfg + 0x54) & (~(1 << 2)), (cfg + 0x54));
		break;
	default:
		mup_dbg(phy->dev, "ERROR %s unknown mode %d.\n", __func__, mode);
		break;
	}
	return ret;
}

void meson_usb2phy_set_calibration_trim(struct amlogic_usb_v2 *phy)
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

/* Single port roothub. Typical seen in socs contains the crgdrd controller.*/

int meson_u2phy_set_mode(struct amlogic_usb_v2 *phy, enum phy_mode mode)
{
	union u2p_r0_v2 reg0;
	void __iomem *cfg = phy->phy_cfg[0];
	int ret = 0;

	mup_dbg(phy->dev, "%s: set mode %d.\n", __func__, mode);

	switch (mode) {
	case PHY_MODE_USB_HOST:
		reg0.d32 = readl(&phy->u2p_aml_regs[0]->r0);
		reg0.b.host_device = 1;
		reg0.b.POR = 0;
		//reg0.b.IDPULLUP0 = 1;
		//reg0.b.DRVVBUS0 = 1;
		writel(reg0.d32, &phy->u2p_aml_regs[0]->r0);
		phy->current_mode = PHY_MODE_USB_HOST;
		break;
	case PHY_MODE_USB_DEVICE:
		reg0.d32 = readl(&phy->u2p_aml_regs[0]->r0);
		reg0.b.host_device = 0;
		reg0.b.POR = 0;
		//reg0.b.IDPULLUP0 = 1;
		//reg0.b.DRVVBUS0 = 1;
		writel(reg0.d32, &phy->u2p_aml_regs[0]->r0);
		phy->current_mode = PHY_MODE_USB_DEVICE;
		break;
	case PHY_MODE_USB_OTG:
		reg0.d32 = readl(&phy->u2p_aml_regs[0]->r0);
		reg0.b.IDPULLUP0 = 1;
		/* vbus detect feature, not used but set for compatibility. */
		//reg0.b.DRVVBUS0 = 1;
		writel(reg0.d32, &phy->u2p_aml_regs[0]->r0);
		/* ID DETECT: usb2_otg_aca_en set to 0 */
		writel(readl(cfg + 0x54) & (~(1 << 2)), (cfg + 0x54));
		/* reg32_20[7] bypass_otg_det default 0. */

		/* FIXME: The current_mode is Used by phy init when resuming.
		 * Don't override. Is PHY_MODE_USB_OTG really needed?
		 */
		//phy->current_mode = PHY_MODE_USB_OTG;
		break;
	default:
		mup_dbg(phy->dev, "ERROR %s unknown mode %d.\n", __func__, mode);
		break;
	}
	return ret;
}

int meson_u2phy_aml_init(struct amlogic_usb_v2 *phy,  struct meson_u2phy_priv *priv)
{
	int ret;

	if (!phy->suspend_flag) {
		mup_err(phy->dev, "%s excessive init\n", __func__);
		return 0;
	}

	ret = clk_bulk_prepare_enable(phy->clk_num, phy->clks);
	if (ret) {
		mup_err(phy->dev, "Failed to enable usb2 phy bus clock at %d\n",
							__LINE__);
	}

	mup_dbg(phy->dev, "init r0~r2 0x%x 0x%x 0x%x.\n",
			readl(&phy->u2p_aml_regs[0]->r0),
			readl(&phy->u2p_aml_regs[0]->r1),
			readl(&phy->u2p_aml_regs[0]->r2));

	meson_u2phy_hold_reset(phy, true);
	meson_u2phy_usb_reset(phy);
	usleep_range(49, 50);
	meson_u2phy_comb_hold_reset(phy, true);
	usleep_range(50, 100);
	//meson_u2phy_reset_comb(phy);

	mup_dbg(phy->dev, "init r0~r2 0x%x 0x%x 0x%x.\n",
			readl(&phy->u2p_aml_regs[0]->r0),
			readl(&phy->u2p_aml_regs[0]->r1),
			readl(&phy->u2p_aml_regs[0]->r2));

	if (phy->suspend_flag && phy->current_mode != 0)
		meson_u2phy_set_mode(phy, phy->current_mode);

	usleep_range(50, 100);
	meson_u2phy_reset_phycfg(phy);

	priv->cali(phy);

	ret = meson_usb2phy_wait_ready(phy, 200);
	if (ret)
		mup_err(phy->dev, " wait for ready timeout.\n");

	ret = priv->set_pll(phy);
	if (ret)
		return ret;

	mup_dbg(phy->dev, "end r0~r2 0x%x 0x%x 0x%x.\n",
			readl(&phy->u2p_aml_regs[0]->r0),
			readl(&phy->u2p_aml_regs[0]->r1),
			readl(&phy->u2p_aml_regs[0]->r2));

	if (phy->suspend_flag)
		phy->suspend_flag = 0;

	return ret;
}

/* Of node parsing common codes. */
int meson_aml_u2phy_parse(struct device *dev, struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2 *aml_u2phy;
	const char *gpio_name = NULL;
	int i, cnt, addr_i = 0, ret = 0;
	u64 addr, size;

	aml_u2phy = kzalloc(sizeof(*aml_u2phy), GFP_KERNEL);
	if (!aml_u2phy)
		return -ENOMEM;

	aml_u2phy->phy_id = instance->index;
	aml_u2phy->dev = dev;
	mup_info(dev, "phy_id %d.\n", aml_u2phy->phy_id);
	instance->meson_uphy = aml_u2phy;
	get_device(dev);

	aml_u2phy->portspeed = instance->port_speed;

	mup_dbg(dev, "portspeed: %d\n", aml_u2phy->portspeed);

	gpio_name = of_get_property(dev->of_node, "gpio-vbus-power", NULL);
	if (gpio_name) {
		aml_u2phy->vbus_power_pin = 1;
		aml_u2phy->usb_gpio_desc = devm_gpiod_get_index(dev,
					 NULL, 0, GPIOD_OUT_LOW);
		if (IS_ERR(aml_u2phy->usb_gpio_desc))
			return -ENODEV;
	}

	/* Assume that all phy node represent _only_ one hw port.
	 * FIXME: What if the phy controller cfg regs and phy config regs are
	 * not one-to-one maps?
	 */
	aml_u2phy->portnum = 1;

	ret = of_property_read_reg(dev->of_node, addr_i++, &addr, &size);
	if (ret) {
		mup_err(dev, "failed to get address %d (id-%d)\n", addr_i,
			aml_u2phy->phy_id);
		return ret;
	}

	aml_u2phy->regs = devm_ioremap(dev, (resource_size_t)addr, (resource_size_t)size);
	if (IS_ERR(aml_u2phy->regs))
		return PTR_ERR(aml_u2phy->regs);

	mup_dbg(dev, "phy_mem:0x%llx, iomap phy_base:0x%px\n",
						addr, aml_u2phy->regs);

	ret = of_property_read_reg(dev->of_node, addr_i++, &addr, &size);
	if (ret) {
		mup_err(dev, "failed to get address %d(id-%d)\n", addr_i,
			aml_u2phy->phy_id);
		return ret;
	}

	aml_u2phy->reset_regs = devm_ioremap(dev, (resource_size_t)addr, (resource_size_t)size);
	if (IS_ERR(aml_u2phy->reset_regs))
		return PTR_ERR(aml_u2phy->reset_regs);

	mup_dbg(dev, "reset_mem:0x%llx, iomap reset_base:0x%px\n",
						addr, aml_u2phy->reset_regs);

	ret = of_property_read_reg(dev->of_node, addr_i++, &addr, &size);
	if (ret || !addr)
		mup_err(dev, "failed to get address %d(id-%d)\n", addr_i,
			aml_u2phy->phy_id);

	aml_u2phy->usb_phy_trim_reg = devm_ioremap(dev, (resource_size_t)addr,
									(resource_size_t)size);
	if (IS_ERR(aml_u2phy->usb_phy_trim_reg))
		aml_u2phy->usb_phy_trim_reg = NULL;

	mup_dbg(dev, "usb_phy_trim_reg:0x%llx, iomap usb_phy_trim_reg:0x%px\n",
						addr, aml_u2phy->usb_phy_trim_reg);

	for (i = 0; i < aml_u2phy->portnum; i++) {
		ret = of_property_read_reg(dev->of_node, addr_i++, &addr, &size);
		if (ret) {
			mup_err(dev, "failed to get address resource %d (id-%d)\n",
				addr_i, aml_u2phy->phy_id);
			return ret;
		}

		aml_u2phy->phy_cfg[i] = devm_ioremap(dev, (resource_size_t)addr,
									(resource_size_t)size);
		if (IS_ERR(aml_u2phy->phy_cfg[i]))
			return PTR_ERR(aml_u2phy->phy_cfg[i]);

		mup_dbg(dev, "phy_cfg%d_mem:0x%llx, iomap phy_cfg%d_base:0x%px\n",
							i, addr, i,
							aml_u2phy->phy_cfg[i]);

		aml_u2phy->u2p_aml_regs[i] = aml_u2phy->regs + 0x20 * i;
	}

	/* Legacy phy only */
	ret = of_property_read_reg(dev->of_node, addr_i++, &addr, &size);
	if (ret) {
		mup_err(dev, "failed to get address resource %d (id-%d)\n",
			addr_i, aml_u2phy->phy_id);
	} else {
		aml_u2phy->usb_aml_regs = devm_ioremap(dev, (resource_size_t)addr,
									(resource_size_t)size);

		if (IS_ERR(aml_u2phy->usb_aml_regs))
			return PTR_ERR(aml_u2phy->usb_aml_regs);

		mup_dbg(dev, "usb_aml_regs_mem:0x%llx, iomap usb_aml_regs:0x%px\n",
					addr, aml_u2phy->usb_aml_regs);
	}

	/* Legacy phy only */
	ret = of_property_read_u32(dev->of_node, "version", &aml_u2phy->phy_version);
	if (ret < 0)
		aml_u2phy->phy_version = 0;
	mup_dbg(dev, "phy version:0x%x\n", aml_u2phy->phy_version);

	ret = of_property_read_u32(dev->of_node, "reset-level", &aml_u2phy->reset_level);
	if (ret < 0)
		aml_u2phy->reset_level = 0x40;

	for (i = 0; i < aml_u2phy->portnum; i++)
		aml_u2phy->phy_reset_level_bit[i] = -1U;

	ret = of_property_read_u32_array(dev->of_node, "phy-reset-level-bits",
								aml_u2phy->phy_reset_level_bit,
								aml_u2phy->portnum);
	if (ret == -EOVERFLOW) {
		mup_err(dev, "phy-reset-level-bits should contains %d values\n",
								aml_u2phy->portnum);
	} else if (ret < 0) {
		mup_err(dev, "no phy-reset-level-bits? exit.\n");
		return -EINVAL;
	}

	for (i = 0; i < aml_u2phy->portnum; i++)
		mup_dbg(dev, "phy%d_reset_level bits: %d\n", i, aml_u2phy->phy_reset_level_bit[i]);

	ret = of_property_read_u32(dev->of_node, "usb-reset-bit", &aml_u2phy->usb_reset_bit);
	if (ret < 0)
		aml_u2phy->usb_reset_bit = -1U;

	mup_dbg(dev, "usb reset bit: %d\n", aml_u2phy->usb_reset_bit);

	ret = of_property_read_u32(dev->of_node, "usb-comb-reset-bit",
								&aml_u2phy->usb_comb_reset_bit);
	if (ret < 0)
		aml_u2phy->usb_comb_reset_bit = -1U;

	mup_dbg(dev, "usb comb reset bit: %d\n", aml_u2phy->usb_comb_reset_bit);

	ret = of_property_read_u32(dev->of_node, "phy-reg-reset-bit",
							&aml_u2phy->phy_reg_reset_bit);
	if (ret < 0)
		aml_u2phy->phy_reg_reset_bit = -1U;

	mup_dbg(dev, "usb reg reset bit: %d\n", aml_u2phy->phy_reg_reset_bit);

	cnt = of_property_count_u32_elems(dev->of_node, "pll-settings");
	if (cnt < 0) {
		mup_err(dev, "no pll-settings? exit.");
		return -EINVAL;
	}

	ret = of_property_read_u32_array(dev->of_node, "pll-settings",
									aml_u2phy->pll_setting,
									cnt);
	if (ret == -EOVERFLOW) {
		mup_err(dev, "pll-settings should contains %d values\n", cnt);
	} else if (ret < 0) {
		mup_err(dev, "no pll-settings? exit.\n");
		return -EINVAL;
	}

	for (i = 0; i < 8; i++)
		mup_dbg(dev, "pll-settings: 0x%x\n", aml_u2phy->pll_setting[i]);

	ret = of_property_read_u32(dev->of_node,
		"dis-thred-enhance", &aml_u2phy->pll_dis_thred_enhance);

	ret = of_property_read_u32(dev->of_node, "clk-mux", &aml_u2phy->clk_mux);

	aml_u2phy->sw_hsp = of_property_read_bool(dev->of_node, "sw-hsp");

	cnt = of_property_count_strings(dev->of_node, "clock-names");
	if (cnt < 0) {
		mup_err(dev, "no clks? exit.");
		return -EINVAL;
	} else if (cnt > AML_USB_PHY_MAX_CLK_NUMBER) {
		mup_err(dev, "too many clks. exit.");
		return -EOVERFLOW;
	}
	mup_dbg(dev, "clk num: %d\n", cnt);
	for (i = 0; i < cnt; i++) {
		ret = of_property_read_string_index(dev->of_node, "clock-names",
									i, &aml_u2phy->clks[i].id);
		if (ret < 0) {
			mup_err(dev, "read clk-names idx:%d err", i);
			return -EINVAL;
		}
	}
	aml_u2phy->clk_num = cnt;

	ret = devm_clk_bulk_get(dev, aml_u2phy->clk_num, aml_u2phy->clks);
	if (ret) {
		mup_dbg(dev, "Failed to get usb2 phy bus clocks\n");
		return ret;
	}

	for (i = 0; i < aml_u2phy->clk_num; i++)
		mup_dbg(dev, "%s %px.\n", aml_u2phy->clks[i].id, (void *)aml_u2phy->clks[i].clk);

	/* Default OFF. */
	meson_u2phy_hold_reset(aml_u2phy, false);
	/* Don't hold comb reset bit. Otg driver may want it. */
	meson_u2phy_power_off(aml_u2phy);
	aml_u2phy->suspend_flag = 1;

	return ret;
}

