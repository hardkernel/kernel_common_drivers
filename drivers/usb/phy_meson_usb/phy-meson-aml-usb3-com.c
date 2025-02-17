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

static inline unsigned int  meson_aml_u3phy_read_reg32(struct aml_usb3_phy *phy, void __iomem *reg)
{
	phys_addr_t pa = meson_aml_u3phy_virt_to_phy(phy, reg);
	u32 val = readl(reg);

	mup_dbg(phy->dev, "read: %pa, 0x%x.\n", &pa, val);
	pa = 0;

	return val;
}

static inline void  meson_aml_u3phy_modify_reg32(struct aml_usb3_phy *phy,
			void __iomem *reg, u32 clear_mask, u32 set_mask)
{
	phys_addr_t pa = meson_aml_u3phy_virt_to_phy(phy, reg);

	mup_dbg(phy->dev, "initial: %pa, 0x%x.\n", &pa, readl(reg));
	writel((readl(reg) & ~clear_mask) | set_mask, reg);
	mup_dbg(phy->dev, "modified: %pa, 0x%x.\n", &pa, readl(reg));
	pa = 0;
}

static inline void  meson_aml_u3phy_write_reg32(struct aml_usb3_phy *phy,
								u32 val, void __iomem *reg)
{
	phys_addr_t pa = meson_aml_u3phy_virt_to_phy(phy, reg);

	mup_dbg(phy->dev, "initial: %pa, 0x%x.\n", &pa, readl(reg));
	writel(val, reg);
	mup_dbg(phy->dev, "written: %pa, 0x%x.\n", &pa, readl(reg));
	pa = 0;
}

static int  meson_aml_u3phy_set_clk(struct aml_usb3_phy *phy, bool on)
{
	int ret = 0;

	if (on) {
		ret = clk_bulk_prepare_enable(phy->clk_num, phy->clks);
		if (ret) {
			mup_err(phy->dev, "Failed to enable usb phy bus clock at %d\n",
								__LINE__);
			return ret;
		}
	} else {
		clk_bulk_disable_unprepare(phy->clk_num, phy->clks);
	}

	return ret;
}

static void  meson_aml_u3phy_set_power_off_quirks(struct aml_usb3_phy *phy)
{
	switch (phy->ic_ver) {
	case MESON_CPU_MAJOR_ID_S6:
		meson_aml_u3phy_modify_reg32(phy, phy->reset_reg + phy->reset_level_shift,
							BIT(phy->usb3_apb_reset_bit),
							BIT(phy->usb3_apb_reset_bit));
		mup_dbg(phy->dev, "ic_ver:0x%x, set power quirks entered.", phy->ic_ver);
		/*  The ctrl reg default value is power consuming. */
		meson_aml_u3phy_write_reg32(phy, 0x71, phy->ctrl_reg + 0x4);
		meson_aml_u3phy_write_reg32(phy, 0x0, phy->ctrl_reg + 0x8);
		meson_aml_u3phy_write_reg32(phy, 0x0, phy->ctrl_reg + 0x10);
		meson_aml_u3phy_write_reg32(phy, 0x0, phy->ctrl_reg + 0x14);
		break;
	default:
		break;
	}
}

/* RESET_reg, write each bit to 1 can reset related module, the reset will auto-cover to 0 by HW.
 * RESETn_module(low active) = (~RESET_reg & RESET_LEVEL) | RESET_MASK.
 */
static inline void  meson_aml_u3phy_pre_reset(struct aml_usb3_phy *phy)
{
	meson_aml_u3phy_write_reg32(phy, BIT(phy->usb3_apb_reset_bit), phy->reset_reg);
	meson_aml_u3phy_write_reg32(phy, BIT(phy->usb3_phy_reset_bit), phy->reset_reg);

	/* Reset usb3_phy_reset_bit triggers pll cfg. Wait for it. */
	usleep_range(320, 400);
}

static void  meson_aml_u3phy_set_power(struct aml_usb3_phy *phy, bool on)
{
	/* The vbus power is controlled by the companion usb2 phy. */
	if (on) {
		/* Make sure reset is not held. */
		meson_aml_u3phy_modify_reg32(phy, phy->reset_reg + phy->reset_level_shift,
				BIT(phy->usb3_apb_reset_bit) | BIT(phy->usb3_phy_reset_bit),
				BIT(phy->usb3_apb_reset_bit) | BIT(phy->usb3_phy_reset_bit));
		/* The u3phy power is comprised of dynamic part & static part
		 * and controlled by the reg values. Reset phy to default.
		 * Make sure reset is done here even if previous step have already
		 * done it.
		 */
		meson_aml_u3phy_pre_reset(phy);
	} else {
		/* Hold reset to power off. */
		meson_aml_u3phy_modify_reg32(phy, phy->reset_reg + phy->reset_level_shift,
				BIT(phy->usb3_apb_reset_bit) | BIT(phy->usb3_phy_reset_bit), 0);
		meson_aml_u3phy_set_power_off_quirks(phy);
		usleep_range(100, 200);
	}
}

static inline void  meson_aml_u3phy_post_reset(struct aml_usb3_phy *phy)
{
	/* The usb3_phy_reset_bit triggers HW pll cfg. */
	if (phy->pll_sw_cfg) {
		meson_aml_u3phy_write_reg32(phy, BIT(phy->usb3_controller_reset_bit),
					phy->reset_reg);
		usleep_range(100, 200);
	} else {
		meson_aml_u3phy_write_reg32(phy, BIT(phy->usb3_phy_reset_bit),
					phy->reset_reg);
		/* HW do the rest pll settings. */
		usleep_range(320, 400);
		meson_aml_u3phy_write_reg32(phy, BIT(phy->usb3_controller_reset_bit),
			phy->reset_reg);
	}
}

static int meson_aml_u3phy_pll_init(struct aml_usb3_phy *phy)
{
	int plldone_i;

	switch (phy->ic_ver) {
	case MESON_CPU_MAJOR_ID_S6:
#define U3P2PLL0_BIAS_EN 9
#define U3P2PLL0_RSTN 10
#define	U3P2PLL0_LOCK_EN 11
#define U3P2PLL_HCSL_LDO_EN_TM 16
#define U3P2PLL_HCSL_DREN0_TM 17
#define	U3P2PLL1_BIAS_EN 20
#define	U3P2PLL1_RSTN 21
#define	U3P2PLL1_RSTN_LOCK 22
#define	U3P2PLL1_LOCK_START 23
#define	U3P2PLL1_PLL_LOCK_FLAG  24
#define U3P2PLL0_PLL_DONE 25
#define PLL_LOCKED_MASK (BIT(U3P2PLL1_PLL_LOCK_FLAG) | BIT(U3P2PLL0_PLL_DONE))
#define IS_PLL_LOCKED(x) (((x) & PLL_LOCKED_MASK) == PLL_LOCKED_MASK)
		if (phy->pll_sw_cfg) {
			/* The speed-drop usb devices TX maybe unstable at insertion,
			 * leading to CDR KI overload. Delay freq tracking start point
			 * by modifying fr_en delay 1us->300us.
			 * Training timer change fr_en delay 1us->300us (unit: 41.668ns).
			 */
			meson_aml_u3phy_write_reg32(phy, 0x1C20, phy->ctrl_reg + 0x9C);
			/* Enable sw configure upcrx_igen_en & upcrx_ldo_en. */
			meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x20,
									0x3 << 14, 0x3 << 14);
			/* Enable upcrx_igen_en & upcrx_ldo_en. */
			meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x28, 0x3, 0x3);
			/* Enable sw configure pll. */
			meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x50, 0x7f << 25,
										0x7f << 25);
			/* Configure pll_cfg 0~5. */
			meson_aml_u3phy_write_reg32(phy, 0x00FA114D, phy->ctrl_reg + 0x2C);
			meson_aml_u3phy_write_reg32(phy, 0x72007820, phy->ctrl_reg + 0x30);
			meson_aml_u3phy_write_reg32(phy, 0xA0900000, phy->ctrl_reg + 0x34);
			meson_aml_u3phy_write_reg32(phy, 0x081C1C23, phy->ctrl_reg + 0x38);
			meson_aml_u3phy_write_reg32(phy, 0x1435DC92, phy->ctrl_reg + 0x3C);
			meson_aml_u3phy_write_reg32(phy, 0x00000000, phy->ctrl_reg + 0x40);

			/* Configure pll_cfg 6. */
			meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
							BIT(U3P2PLL0_BIAS_EN),
							BIT(U3P2PLL0_BIAS_EN));
			udelay(5);
			meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
							BIT(U3P2PLL0_RSTN), BIT(U3P2PLL0_RSTN));
			usleep_range(40, 140);
			meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
							BIT(U3P2PLL0_LOCK_EN),
							BIT(U3P2PLL0_LOCK_EN));
			usleep_range(160, 260);
			/* Off unused usb3phy digital 100M clk: U3P2PLL_HCSL_LDO_EN_TM -> 0. */
			meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
						BIT(U3P2PLL_HCSL_LDO_EN_TM) |
						BIT(U3P2PLL_HCSL_DREN0_TM),
						0);
			usleep_range(20, 120);
			meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
							BIT(U3P2PLL1_BIAS_EN),
							BIT(U3P2PLL1_BIAS_EN));
			usleep_range(50, 150);
			meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
							BIT(U3P2PLL1_RSTN),
							BIT(U3P2PLL1_RSTN));
			usleep_range(40, 140);
			meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
							BIT(U3P2PLL1_RSTN_LOCK),
							BIT(U3P2PLL1_RSTN_LOCK));
			udelay(1);
			meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
							BIT(U3P2PLL1_LOCK_START),
							BIT(U3P2PLL1_LOCK_START));
		} else {
			/* Training timer change fr_en delay 1us->300us (unit: 41.668ns). */
			meson_aml_u3phy_write_reg32(phy, 0x1C20, phy->ctrl_reg + 0x9C);
			/* Enable sw configure upcrx_igen_en & upcrx_ldo_en. */
			meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x20, 0x3 << 14,
									0x3 << 14);
			/* Enable upcrx_igen_en & upcrx_ldo_en. */
			meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x28, 0x3, 0x3);
			/* Configure pll_cfg 0~5. */
			meson_aml_u3phy_write_reg32(phy, 0x00FA114D, phy->ctrl_reg + 0x2C);
			meson_aml_u3phy_write_reg32(phy, 0x72007820, phy->ctrl_reg + 0x30);
			meson_aml_u3phy_write_reg32(phy, 0xA0900000, phy->ctrl_reg + 0x34);
			meson_aml_u3phy_write_reg32(phy, 0x081C1C23, phy->ctrl_reg + 0x38);
			meson_aml_u3phy_write_reg32(phy, 0x1435DC92, phy->ctrl_reg + 0x3C);
			meson_aml_u3phy_write_reg32(phy, 0x00000000, phy->ctrl_reg + 0x40);
			/* Off unused usb3phy digital 100M clk: U3P2PLL_HCSL_LDO_EN_TM -> 0. */
			meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
						BIT(U3P2PLL_HCSL_LDO_EN_TM) |
						BIT(U3P2PLL_HCSL_DREN0_TM),
						0);
		}
		break;
	default:
		break;
	}

	for (plldone_i = 5; plldone_i > 0; plldone_i--) {
		usleep_range(20, 120);
		if (IS_PLL_LOCKED(readl(phy->ctrl_reg + 0x154)))
			goto done;
	}

	dev_err(phy->dev, "usb3 pll not locked. val: 0x%08x\n",
						readl(phy->ctrl_reg + 0x154));
	return -1;

done:
	return 0;
}

static void  meson_aml_u3phy_trim(struct aml_usb3_phy *phy)
{
	u32 raw = meson_aml_u3phy_read_reg32(phy, phy->trim_reg);
	u32 val = 0;

	/* tx termination impedance. */
	if (raw & BIT(5))
		val = raw & 0x1f;
	else
		val = 0x10;

	meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x16c, 0xff << 24, val << 24);

	/* rx termination impedance. */
	if (raw & BIT(10))
		val = (raw & 0x3c0) >> 6;
	else
		val = 0x7;

	meson_aml_u3phy_modify_reg32(phy, phy->ctrl_reg + 0x4, 0xf << 4, val << 4);
}

static void  meson_aml_u3phy_txrx_cali(struct aml_usb3_phy *phy)
{
	/* rx cali.*/
	meson_aml_u3phy_write_reg32(phy, 0x240014A4, phy->ctrl_reg + 0x10);

	/* tx cali.*/
	meson_aml_u3phy_write_reg32(phy, 0x003154DA, phy->ctrl_reg + 0xcc);
	meson_aml_u3phy_write_reg32(phy, 0x003154DA, phy->ctrl_reg + 0xf0);
	meson_aml_u3phy_write_reg32(phy, 0x00F154DA, phy->ctrl_reg + 0xfc);
}

static void  meson_aml_u3phy_cali(struct aml_usb3_phy *phy)
{
	meson_aml_u3phy_trim(phy);
	meson_aml_u3phy_txrx_cali(phy);
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

	meson_aml_u3phy_set_power(phy, true);

	ret = meson_aml_u3phy_pll_init(phy);
	if (ret)
		goto cleanup;

	meson_aml_u3phy_cali(phy);
	meson_aml_u3phy_post_reset(phy);

	meson_aml_u3phy_set_hw_on(phy, true);

	return ret;

cleanup:
	meson_aml_u3phy_set_power(phy, false);
	meson_aml_u3phy_set_clk(phy, false);

exit:
	return ret;
}

int meson_aml_u3phy_exit(struct aml_usb3_phy *phy)
{
	if (!meson_aml_u3phy_is_hw_on(phy))
		return -ENODEV;

	meson_aml_u3phy_set_power(phy, false);
	meson_aml_u3phy_set_clk(phy, false);
	meson_aml_u3phy_set_hw_on(phy, false);

	mup_dbg(phy->dev, "reset level final: 0x%x.\n",
				readl(phy->reset_reg + phy->reset_level_shift));

	return 0;
}

int meson_aml_u3phy_parse(struct device *dev, struct meson_uphy_instance *instance)
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
	u8 usb3_apb_reset_bit = 0;
	u8 usb3_phy_reset_bit = 0;
	u8 usb3_controller_reset_bit = 0;
	u32 ic_ver = get_cpu_type();

	phy = kzalloc(sizeof(*phy), GFP_KERNEL);
	if (!phy)
		return -ENOMEM;

	phy->phy_id = instance->index;
	phy->dev = dev;
	mup_dbg(dev, "phy_id %d.\n", phy->phy_id);
	instance->meson_uphy = phy;
	get_device(dev);

	ret = of_property_read_reg(dev->of_node, addr_i++, &phy->cfg_reg_phy, &phy->cfg_reg_size);
	if (ret) {
		mup_err(dev, "failed to get address %d(id-%d)\n",
			addr_i, phy->phy_id);
		return ret;
	}

	cfg_reg = devm_ioremap(dev, (resource_size_t)phy->cfg_reg_phy,
						(resource_size_t)phy->cfg_reg_size);
	if (IS_ERR(cfg_reg))
		return PTR_ERR(cfg_reg);

	mup_dbg(dev, "USB3 phy probe:cfg_reg_phy:%llx, iomap cfg_reg:%px, s:%llx\n",
			phy->cfg_reg_phy, cfg_reg, phy->cfg_reg_size);

	ret = of_property_read_reg(dev->of_node, addr_i++, &phy->reset_reg_phy,
						&phy->reset_reg_size);
	if (ret) {
		mup_err(dev, "failed to get address %d(id-%d)\n",
			addr_i, phy->phy_id);
		return ret;
	}

	reset_reg = devm_ioremap(dev, (resource_size_t)phy->reset_reg_phy,
						(resource_size_t)phy->reset_reg_size);
	if (IS_ERR(reset_reg))
		return PTR_ERR(reset_reg);

	mup_dbg(dev, "USB3 phy probe:reset_phy:%llx, iomap reset_reg:%px, s:%llx\n",
			phy->reset_reg_phy, reset_reg, phy->reset_reg_size);

	ret = of_property_read_reg(dev->of_node, addr_i++, &phy->ctrl_reg_phy, &phy->ctrl_reg_size);
	if (ret) {
		mup_err(dev, "failed to get address %d(id-%d)\n",
			addr_i, phy->phy_id);
		return ret;
	}

	ctrl_reg = devm_ioremap(dev, (resource_size_t)phy->ctrl_reg_phy,
							(resource_size_t)phy->ctrl_reg_size);
	if (IS_ERR(ctrl_reg))
		return PTR_ERR(ctrl_reg);

	mup_dbg(dev, "USB3 phy probe:ctrl_phy:%llx, iomap ctrl_reg:%px, s:%llx\n",
			phy->ctrl_reg_phy, ctrl_reg, phy->ctrl_reg_size);

	ret = of_property_read_reg(dev->of_node, addr_i++, &phy->trim_reg_phy, &phy->trim_reg_size);
	if (ret) {
		mup_err(dev, "failed to get address %d(id-%d), This usb phy has no trim reg?\n",
			addr_i, phy->phy_id);
		return ret;
	}
	trim_reg = devm_ioremap(dev, (resource_size_t)phy->trim_reg_phy,
						(resource_size_t)phy->trim_reg_size);
	if (IS_ERR(trim_reg))
		return PTR_ERR(trim_reg);

	mup_dbg(dev, "USB3 phy probe:trim_phy:%llx, iomap trim_reg:%px, s:%llx\n",
			phy->trim_reg_phy, trim_reg, phy->trim_reg_size);

	phy_off = of_property_read_bool(dev->of_node, "off");

	pll_sw_cfg = of_property_read_bool(dev->of_node, "pll-sw-cfg");

	ret = of_property_read_u32(dev->of_node, "reset-level-shift", &reset_level_shift);
	if (ret)
		return ret;

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
		mup_err(dev, "no clks? exit.");
		return -EINVAL;
	} else if (cnt > AML_USB_PHY_MAX_CLK_NUMBER) {
		mup_err(dev, "too many clks. exit.");
		return -EOVERFLOW;
	}
	phy->clk_num = cnt;
	mup_dbg(dev, "clk num: %d\n", cnt);
	for (i = 0; i < cnt; i++) {
		ret = of_property_read_string_index(dev->of_node, "clock-names",
									i, &phy->clks[i].id);
		if (ret < 0) {
			mup_err(dev, "read clk-names idx:%d err", i);
			return -EINVAL;
		}
	}

	ret = devm_clk_bulk_get(dev, phy->clk_num, phy->clks);
	if (ret) {
		mup_dbg(dev, "Failed to get usb phy bus clocks\n");
		return ret;
	}

	for (i = 0; i < phy->clk_num; i++)
		mup_dbg(dev, "%s %px.\n", phy->clks[i].id, (void *)phy->clks[i].clk);

	mup_dbg(dev, "USB3 phy_off:%d, pll_sw_cfg:%d, phy_id:%d, clk_num:%d\n"
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

