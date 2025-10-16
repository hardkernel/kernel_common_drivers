// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/io.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/amlogic/usbtype.h>
#include <linux/amlogic/cpu_version.h>
#include "phy-meson-usb.h"

static inline bool  meson_aml_u3phy_is_sw_on(struct aml_usb3_phy *phy)
{
	return !phy->off ? true : false;
}

static inline bool  meson_aml_u3phy_is_hw_on(struct aml_usb3_phy *phy)
{
	return phy->suspend ? false : true;
}

static inline void  meson_aml_u3phy_set_hw_on(struct aml_usb3_phy *phy, bool on)
{
	phy->suspend = !on;
}

static inline phys_addr_t  meson_aml_u3phy_virt_to_phy(struct aml_usb3_phy *phy,
										void __iomem *reg)
{
	if (reg >= phy->ctrl_reg && reg < phy->ctrl_reg + phy->ctrl_reg_size)
		return phy->ctrl_reg_phy + (phys_addr_t)reg - (phys_addr_t)phy->ctrl_reg;

	if (reg >= phy->cfg_reg && reg < phy->cfg_reg + phy->cfg_reg_size)
		return phy->cfg_reg_phy + (phys_addr_t)reg - (phys_addr_t)phy->cfg_reg;

	if (reg >= phy->reset_reg && reg < phy->reset_reg + phy->reset_reg_size)
		return phy->reset_reg_phy + (phys_addr_t)reg - (phys_addr_t)phy->reset_reg;

	if (reg >= phy->trim_reg && reg < phy->trim_reg + phy->trim_reg_size)
		return phy->trim_reg_phy + (phys_addr_t)reg - (phys_addr_t)phy->trim_reg;

	return (phys_addr_t)(-1);
}

inline unsigned int  meson_aml_u3phy_read_reg32(struct aml_usb3_phy *phy, void __iomem *reg)
{
	phys_addr_t pa = meson_aml_u3phy_virt_to_phy(phy, reg);
	u32 val = readl(reg);

	dev_dbg(phy->dev, "read: %pa, 0x%x.\n", &pa, val);
	pa = 0;

	return val;
}

inline void  meson_aml_u3phy_modify_reg32(struct aml_usb3_phy *phy,
			void __iomem *reg, u32 clear_mask, u32 set_mask)
{
	phys_addr_t pa = meson_aml_u3phy_virt_to_phy(phy, reg);

	dev_dbg(phy->dev, "initial: %pa, 0x%x.\n", &pa, readl(reg));
	writel((readl(reg) & ~clear_mask) | set_mask, reg);
	dev_dbg(phy->dev, "modified: %pa, 0x%x.\n", &pa, readl(reg));
	pa = 0;
}

inline void  meson_aml_u3phy_write_reg32(struct aml_usb3_phy *phy,
								u32 val, void __iomem *reg)
{
	phys_addr_t pa = meson_aml_u3phy_virt_to_phy(phy, reg);

	dev_dbg(phy->dev, "initial: %pa, 0x%x.\n", &pa, readl(reg));
	writel(val, reg);
	dev_dbg(phy->dev, "written: %pa, 0x%x.\n", &pa, readl(reg));
	pa = 0;
}

static int  meson_aml_u3phy_set_clk(struct aml_usb3_phy *phy, bool on)
{
	int ret = 0;

	if (on) {
		ret = clk_bulk_prepare_enable(phy->clk_num, phy->clks);
		if (ret) {
			dev_err(phy->dev, "Failed to enable usb phy bus clock at %d\n",
								__LINE__);
			return ret;
		}
	} else {
		clk_bulk_disable_unprepare(phy->clk_num, phy->clks);
	}

	return ret;
}

static bool meson_aml_u3phy_wait_pll_locked(struct aml_usb3_phy *phy)
{
	bool ret = false;
	int plldone_i;
	u32 pll_locked_mask = 0;

#define U3P2PLL1_PLL_LOCK_FLAG	24
#define U3P2PLL0_PLL_DONE 25
#define PLL_STAT_REG_OFF	0X154

	pll_locked_mask = BIT(U3P2PLL1_PLL_LOCK_FLAG);
	for (plldone_i = 10; plldone_i > 0; plldone_i--) {
		usleep_range(100, 200);
		if ((readl(phy->ctrl_reg + PLL_STAT_REG_OFF) & pll_locked_mask)
			== pll_locked_mask)
			ret = true;
	}

	if (!ret)
		dev_warn(phy->dev, "0x%x PLL init failed. Checkout timeout val?\n",
				readl(phy->ctrl_reg + PLL_STAT_REG_OFF));
	else
		dev_dbg(phy->dev, "0x%x PLL init done.\n",
			readl(phy->ctrl_reg + PLL_STAT_REG_OFF));

	return ret;
}

/* RESET_reg, write each bit to 1 can reset related module, the reset will auto-cover to 0 by HW.
 * RESETn_module(low active) = (~RESET_reg & RESET_LEVEL) | RESET_MASK.
 */
static inline void  meson_aml_u3phy_pre_reset(struct aml_usb3_phy *phy)
{
	meson_aml_u3phy_write_reg32(phy, BIT(phy->usb3_apb_reset_bit),
			phy->reset_reg + phy->apb_reset_offset);
	meson_aml_u3phy_write_reg32(phy, BIT(phy->usb3_phy_reset_bit),
			phy->reset_reg);

	/* Reset usb3_phy_reset_bit trigger HWs pll cfg. Wait for it. */
	if (phy->priv->wait_pll_locked) {
		if (!phy->priv->wait_pll_locked(phy))
			dev_err(phy->dev, "Pre Wait HW PLL init failed.\n");
	} else {
		if (!meson_aml_u3phy_wait_pll_locked(phy))
			dev_err(phy->dev, "Pre Wait HW PLL init failed.\n");
	}
}

static void  meson_aml_u3phy_set_power(struct aml_usb3_phy *phy, bool on)
{
	/* The vbus power is controlled by the companion usb2 phy. */
	if (on) {
		/* Make sure reset is not held. */
		meson_aml_u3phy_modify_reg32(phy, phy->reset_reg + phy->reset_level_shift,
				BIT(phy->usb3_phy_reset_bit), BIT(phy->usb3_phy_reset_bit));
		meson_aml_u3phy_modify_reg32(phy, phy->reset_reg + phy->reset_level_shift +
				phy->apb_reset_offset, BIT(phy->usb3_apb_reset_bit),
				BIT(phy->usb3_apb_reset_bit));
		/* The u3phy power is comprised of dynamic part & static part
		 * and controlled by the reg values. Reset phy to default.
		 * Make sure reset is done here even if previous step have already
		 * done it.
		 */
		meson_aml_u3phy_pre_reset(phy);
	} else {
		/* Hold reset to power off. */
		meson_aml_u3phy_modify_reg32(phy, phy->reset_reg + phy->reset_level_shift,
				BIT(phy->usb3_phy_reset_bit), 0);
		meson_aml_u3phy_modify_reg32(phy, phy->reset_reg + phy->reset_level_shift +
				phy->apb_reset_offset, BIT(phy->usb3_apb_reset_bit), 0);
		usleep_range(100, 200);
	}
}

static inline void  meson_aml_u3phy_post_reset(struct aml_usb3_phy *phy)
{
	/* The usb3_phy_reset_bit triggers HW pll cfg. */
	meson_aml_u3phy_write_reg32(phy, BIT(phy->usb3_controller_reset_bit),
				phy->reset_reg);
	usleep_range(100, 200);
}

static int meson_aml_u3phy_pll_init(struct aml_usb3_phy *phy)
{
	int ret = 0;

	ret = phy->priv->pll_init(phy);
	if (ret)
		return ret;

	if (phy->priv->wait_pll_locked) {
		if (!phy->priv->wait_pll_locked(phy))
			ret = -ETIMEDOUT;
	} else {
		if (!meson_aml_u3phy_wait_pll_locked(phy))
			ret = -ETIMEDOUT;
	}
	return ret;
}

static void meson_aml_u3phy_trim(struct aml_usb3_phy *phy)
{
	u32 raw = meson_aml_u3phy_read_reg32(phy, phy->trim_reg);
	u32 val = 0;

	if (!(raw & BIT(6))) {
		dev_err(phy->dev, "No TX termination impedance trim? Skip.\n");
	} else {
		/* software force value, need to set upctx_pdio_main en in tx_cfg0. */
		meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0xbc,
				BIT(14), BIT(14));
		/* TX termination impedance trim: upctx_pdio_main. */
		val = raw & (u32)GENMASK(2, 0) >> 0;
		meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0xc0,
			(u32)GENMASK(28, 26), val << 26);

		/* software force value, need to set upctx_puio_main en in tx_cfg0. */
		meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0xbc,
			BIT(17), BIT(17));
		/* TX termination impedance trim: upctx_puio_main */
		val = (raw & (u32)GENMASK(5, 3)) >> 3;
		meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0xc0,
			(u32)GENMASK(25, 23), val << 23);
	}

	if (!(raw & BIT(11))) {
		dev_err(phy->dev, "No TX amplitude trim? Skip.\n");
	} else {
		/* software force analog value, need to set upctx_boost_en en in tx_cfg0. */
		meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0xbc,
				BIT(0), BIT(0));
		/* TX amplitude trim: upctx_boost_en. */
		meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0xc0,
			BIT(0), BIT(0));

		/* software force value, need to set upctx_boost_ctrl en in tx_cfg0. */
		meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0xbc,
			BIT(1), BIT(1));
		/* TX amplitude trim: upctx_boost_ctrl. */
		val = (raw & (u32)GENMASK(10, 7)) >> 7;
		meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0xc0,
			(u32)GENMASK(4, 1), val << 1);
	}

	if (!(raw & BIT(16))) {
		dev_err(phy->dev, "No RX termination impedance trim? Skip.\n");
	} else {
		/* RX impedance trim: rterm_cal_cn field of reg_dcha_afe   */
		val = (raw & (u32)GENMASK(15, 12)) >> 12;
		meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x4,
			(u32)GENMASK(7, 4), val << 4);
	}
}


static void  meson_aml_u3phy_cali(struct aml_usb3_phy *phy)
{
	if (phy->priv->trim)
		phy->priv->trim(phy);
	else
		meson_aml_u3phy_trim(phy);

	phy->priv->txrx_cali(phy);
}

int meson_aml_u3phy_init(struct aml_usb3_phy *phy)
{
	int ret = 0;

	if (!meson_aml_u3phy_is_sw_on(phy))
		return 0;

	if (meson_aml_u3phy_is_hw_on(phy))
		return 0;

	ret = meson_aml_u3phy_set_clk(phy, true);
	if (ret)
		goto exit;

	if (phy->priv->set_power)
		phy->priv->set_power(phy, true);
	else
		meson_aml_u3phy_set_power(phy, true);

	ret = meson_aml_u3phy_pll_init(phy);
	if (ret)
		goto cleanup;

	if (phy->priv->ssc_set)
		phy->priv->ssc_set(phy, true);

	meson_aml_u3phy_cali(phy);
	meson_aml_u3phy_post_reset(phy);

	meson_aml_u3phy_set_hw_on(phy, true);

	return ret;

cleanup:
	if (phy->priv->set_power)
		phy->priv->set_power(phy, false);
	else
		meson_aml_u3phy_set_power(phy, false);
	meson_aml_u3phy_set_clk(phy, false);

exit:
	return ret;
}

int meson_aml_u3phy_exit(struct aml_usb3_phy *phy)
{
	if (!meson_aml_u3phy_is_hw_on(phy))
		return -ENODEV;

	if (phy->priv->set_power)
		phy->priv->set_power(phy, false);
	else
		meson_aml_u3phy_set_power(phy, false);
	meson_aml_u3phy_set_clk(phy, false);
	meson_aml_u3phy_set_hw_on(phy, false);

	dev_dbg(phy->dev, "reset level final: 0x%x.\n",
				readl(phy->reset_reg + phy->reset_level_shift));

	return 0;
}

int meson_aml_u3phy_parse(struct device *dev, struct meson_uphy_instance *instance,
				struct meson_aml_u3phy_priv *priv)
{
	struct aml_usb3_phy *phy;
	int i, cnt, addr_i = 0, ret = 0;
	bool phy_off = true;
	bool pll_sw_cfg = true;
	void __iomem *cfg_reg = NULL;
	void __iomem *reset_reg = NULL;
	void __iomem *ctrl_reg = NULL;
	void __iomem *trim_reg = NULL;
	u32 reset_level_shift = 0;
	u32 apb_reset_offset = 0;
	u8 usb3_apb_reset_bit = 0;
	u8 usb3_phy_reset_bit = 0;
	u8 usb3_controller_reset_bit = 0;
	u32 ic_ver = get_cpu_type();

	phy = kzalloc(sizeof(*phy), GFP_KERNEL);
	if (!phy)
		return -ENOMEM;

	phy->phy_id = instance->index;
	phy->dev = dev;
	dev_dbg(dev, "phy_id %d.\n", phy->phy_id);
	instance->meson_uphy = phy;
	get_device(dev);

	phy->priv = priv;
	if (!phy->priv || !phy->priv->pll_init || !phy->priv->txrx_cali) {
		dev_err(phy->dev, "No necessary callback(s).\n");
		return -EINVAL;
	}

	ret = of_property_read_reg(dev->of_node, addr_i++, &phy->cfg_reg_phy, &phy->cfg_reg_size);
	if (ret) {
		dev_err(dev, "failed to get address %d(id-%d)\n",
			addr_i, phy->phy_id);
		return ret;
	}

	cfg_reg = devm_ioremap(dev, (resource_size_t)phy->cfg_reg_phy,
						(resource_size_t)phy->cfg_reg_size);
	if (IS_ERR(cfg_reg))
		return PTR_ERR(cfg_reg);

	dev_dbg(dev, "USB3 phy probe:cfg_reg_phy:%llx, iomap cfg_reg:%px, s:%llx\n",
			phy->cfg_reg_phy, cfg_reg, phy->cfg_reg_size);

	ret = of_property_read_reg(dev->of_node, addr_i++, &phy->reset_reg_phy,
						&phy->reset_reg_size);
	if (ret) {
		dev_err(dev, "failed to get address %d(id-%d)\n",
			addr_i, phy->phy_id);
		return ret;
	}

	reset_reg = devm_ioremap(dev, (resource_size_t)phy->reset_reg_phy,
						(resource_size_t)phy->reset_reg_size);
	if (IS_ERR(reset_reg))
		return PTR_ERR(reset_reg);

	dev_dbg(dev, "USB3 phy probe:reset_phy:%llx, iomap reset_reg:%px, s:%llx\n",
			phy->reset_reg_phy, reset_reg, phy->reset_reg_size);

	ret = of_property_read_reg(dev->of_node, addr_i++, &phy->ctrl_reg_phy, &phy->ctrl_reg_size);
	if (ret) {
		dev_err(dev, "failed to get address %d(id-%d)\n",
			addr_i, phy->phy_id);
		return ret;
	}

	ctrl_reg = devm_ioremap(dev, (resource_size_t)phy->ctrl_reg_phy,
							(resource_size_t)phy->ctrl_reg_size);
	if (IS_ERR(ctrl_reg))
		return PTR_ERR(ctrl_reg);

	dev_dbg(dev, "USB3 phy probe:ctrl_phy:%llx, iomap ctrl_reg:%px, s:%llx\n",
			phy->ctrl_reg_phy, ctrl_reg, phy->ctrl_reg_size);

	ret = of_property_read_reg(dev->of_node, addr_i++, &phy->trim_reg_phy, &phy->trim_reg_size);
	if (ret) {
		dev_err(dev, "failed to get address %d(id-%d), This usb phy has no trim reg?\n",
			addr_i, phy->phy_id);
		return ret;
	}
	trim_reg = devm_ioremap(dev, (resource_size_t)phy->trim_reg_phy,
						(resource_size_t)phy->trim_reg_size);
	if (IS_ERR(trim_reg))
		return PTR_ERR(trim_reg);

	dev_dbg(dev, "USB3 phy probe:trim_phy:%llx, iomap trim_reg:%px, s:%llx\n",
			phy->trim_reg_phy, trim_reg, phy->trim_reg_size);

	phy_off = of_property_read_bool(dev->of_node, "off");

	pll_sw_cfg = of_property_read_bool(dev->of_node, "pll-sw-cfg");

	ret = of_property_read_u32(dev->of_node, "reset-level-shift", &reset_level_shift);
	if (ret)
		return ret;

	ret = of_property_read_u32(dev->of_node, "apb-reset-offset", &apb_reset_offset);
	if (ret)
		apb_reset_offset = 0;

	ret = of_property_read_u8(dev->of_node, "usb3-apb-reset-bit", &usb3_apb_reset_bit);
	if (ret)
		return ret;

	ret = of_property_read_u8(dev->of_node, "usb3-phy-reset-bit", &usb3_phy_reset_bit);
	if (ret)
		return ret;

	ret = of_property_read_u8(dev->of_node, "usb3-controller-reset-bit",
							&usb3_controller_reset_bit);
	if (ret)
		return ret;

	cnt = of_property_count_strings(dev->of_node, "clock-names");
	if (cnt < 0) {
		dev_err(dev, "no clks? exit.");
		return -EINVAL;
	} else if (cnt > AML_USB_PHY_MAX_CLK_NUMBER) {
		dev_err(dev, "too many clks. exit.");
		return -EOVERFLOW;
	}
	phy->clk_num = cnt;
	dev_dbg(dev, "clk num: %d\n", cnt);
	for (i = 0; i < cnt; i++) {
		ret = of_property_read_string_index(dev->of_node, "clock-names",
									i, &phy->clks[i].id);
		if (ret < 0) {
			dev_err(dev, "read clk-names idx:%d err", i);
			return -EINVAL;
		}
	}

	ret = devm_clk_bulk_get(dev, phy->clk_num, phy->clks);
	if (ret) {
		dev_dbg(dev, "Failed to get usb phy bus clocks\n");
		return ret;
	}

	for (i = 0; i < phy->clk_num; i++)
		dev_dbg(dev, "%s %px.\n", phy->clks[i].id, (void *)phy->clks[i].clk);

	dev_dbg(dev, "USB3 phy_off:%d, pll_sw_cfg:%d, phy_id:%d, clk_num:%d\n"
						 "reset_level_shift:0x%x, usb3-apb-reset-bit:%d\n"
						 "usb3-phy-reset-bit:%d, usb3-controller-reset-bit:%d\n",
						phy_off, pll_sw_cfg, phy->phy_id, phy->clk_num,
						reset_level_shift, usb3_apb_reset_bit,
						usb3_phy_reset_bit, usb3_controller_reset_bit);

	phy->cfg_reg = cfg_reg;
	phy->ctrl_reg = ctrl_reg;
	phy->trim_reg = trim_reg;
	phy->reset_reg = reset_reg;
	phy->reset_level_shift = reset_level_shift;
	phy->usb3_apb_reset_bit = usb3_apb_reset_bit;
	phy->apb_reset_offset = apb_reset_offset;
	phy->usb3_phy_reset_bit = usb3_phy_reset_bit;
	phy->usb3_controller_reset_bit = usb3_controller_reset_bit;
	phy->off = phy_off;
	phy->pll_sw_cfg = pll_sw_cfg;
	phy->ic_ver = ic_ver;

	/* Sync state to hw off. */
	if (meson_aml_u3phy_is_sw_on(phy)) {
		meson_aml_u3phy_set_power(phy, false);
		meson_aml_u3phy_set_hw_on(phy, false);
	}

	return 0;
}

