// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/pm_runtime.h>
#include <linux/delay.h>
#include <linux/usb/phy.h>
#include <linux/amlogic/usb-v2.h>
#include <linux/of_gpio.h>
#include <linux/amlogic/usbtype.h>
#include "../phy/phy-aml-new-usb-v2.h"

static unsigned char aml_usb3_phy_clk_name[11];

static inline bool aml_usb3_phy_is_exist(struct aml_usb3_phy *phy)
{
	return phy->portnum ? true : false;
}

static inline bool aml_usb3_phy_is_sw_on(struct aml_usb3_phy *phy)
{
	return aml_usb3_phy_is_exist(phy) && !phy->off ? true : false;
}

static inline bool aml_usb3_phy_is_hw_on(struct aml_usb3_phy *phy)
{
	return phy->phy.flags == AML_USB3_PHY_ENABLE ? true : false;
}

static inline void aml_usb3_phy_set_hw_on(struct aml_usb3_phy *phy, bool on)
{
	phy->phy.flags = on ? AML_USB3_PHY_ENABLE : AML_USB3_PHY_DISABLE;
}

static inline phys_addr_t aml_usb3_phy_virt_to_phy(struct aml_usb3_phy *phy,
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

static inline unsigned int aml_usb3_phy_read_reg32(struct aml_usb3_phy *phy, void __iomem *reg)
{
	phys_addr_t pa = aml_usb3_phy_virt_to_phy(phy, reg);
	u32 val = readl(reg);

	dev_dbg(phy->dev, "read: %pa, 0x%x.\n", &pa, val);

	return val;
}

static inline void aml_usb3_phy_modify_reg32(struct aml_usb3_phy *phy,
			void __iomem *reg, u32 clear_mask, u32 set_mask)
{
	phys_addr_t pa = aml_usb3_phy_virt_to_phy(phy, reg);

	dev_dbg(phy->dev, "initial: %pa, 0x%x.\n", &pa, readl(reg));
	writel((readl(reg) & ~clear_mask) | set_mask, reg);
	dev_dbg(phy->dev, "modified: %pa, 0x%x.\n", &pa, readl(reg));
}

static inline void aml_usb3_phy_write_reg32(struct aml_usb3_phy *phy,
								u32 val, void __iomem *reg)
{
	phys_addr_t pa = aml_usb3_phy_virt_to_phy(phy, reg);

	dev_dbg(phy->dev, "initial: %pa, 0x%x.\n", &pa, readl(reg));
	writel(val, reg);
	dev_dbg(phy->dev, "written: %pa, 0x%x.\n", &pa, readl(reg));
}

static int aml_usb3_phy_set_clk(struct aml_usb3_phy *phy, bool on)
{
	int i, ret = 0;

	if (on) {
		for (i = 0; i < phy->num_clk; i++) {
			ret = clk_prepare_enable(phy->clk[i]);
			if (ret) {
				dev_err(phy->dev, "Failed to enable u3p_%d clock.\n", i);
				ret = PTR_ERR(phy->clk);
				aml_usb3_phy_set_clk(phy, false);
				return ret;
			}
		}
	} else {
		ret = 0;
		for (i = 0; i < phy->num_clk; i++)
			clk_disable_unprepare(phy->clk[i]);
	}

	return ret;
}

/* RESET_reg, write each bit to 1 can reset related module, the reset will auto-cover to 0 by HW.
 * RESETn_module(low active) = (~RESET_reg & RESET_LEVEL) | RESET_MASK.
 */
static inline void aml_usb3_phy_pre_reset(struct aml_usb3_phy *phy)
{
	if (phy->pll_sw_cfg)
		return;

	aml_usb3_phy_write_reg32(phy, BIT(phy->usb3_apb_reset_bit), phy->reset_reg);

	usleep_range(100, 200);
}

static inline void aml_usb3_phy_post_reset(struct aml_usb3_phy *phy)
{
	if (phy->pll_sw_cfg)
		aml_usb3_phy_write_reg32(phy, BIT(phy->usb3_phy_reset_bit), phy->reset_reg);
	else
		aml_usb3_phy_write_reg32(phy, BIT(phy->usb3_phy_reset_bit) |
			BIT(phy->usb3_controller_reset_bit), phy->reset_reg);

	usleep_range(100, 200);
}

static void aml_usb3_phy_set_power(struct aml_usb3_phy *phy, bool on)
{
	/* The vbus power is controlled by the companion usb2 phy. */
	if (on) {
		aml_usb3_phy_modify_reg32(phy, phy->reset_reg + phy->reset_level_shift,
				BIT(phy->usb3_apb_reset_bit) | BIT(phy->usb3_phy_reset_bit),
				BIT(phy->usb3_apb_reset_bit) | BIT(phy->usb3_phy_reset_bit));
	} else {
		/* Don't touch any bit else. */
		aml_usb3_phy_modify_reg32(phy, phy->reset_reg + phy->reset_level_shift,
							BIT(phy->usb3_phy_reset_bit), 0);
	}
	usleep_range(100, 200);
}

static int aml_usb3_phy_pll_init_v0(struct aml_usb3_phy *phy)
{
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
	int plldone_i;

	if (phy->pll_sw_cfg) {
		/* Enable sw configure upcrx_igen_en & upcrx_ldo_en. */
		aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x20, 0x3 << 14, 0x3 << 14);
		/* Enable upcrx_igen_en & upcrx_ldo_en. */
		aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x28, 0x3, 0x3);
		/* Enable sw configure pll. */
		aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x50, 0x7f << 25, 0x7f << 25);
		/* Configure pll_cfg 0~5. */
		aml_usb3_phy_write_reg32(phy, 0x00FA117F, phy->ctrl_reg + 0x2C);
		aml_usb3_phy_write_reg32(phy, 0x72007821, phy->ctrl_reg + 0x30);
		aml_usb3_phy_write_reg32(phy, 0xA0900000, phy->ctrl_reg + 0x34);
		aml_usb3_phy_write_reg32(phy, 0x081C1C23, phy->ctrl_reg + 0x38);
		aml_usb3_phy_write_reg32(phy, 0x1435DC92, phy->ctrl_reg + 0x3C);
		aml_usb3_phy_write_reg32(phy, 0x00000000, phy->ctrl_reg + 0x40);

		/* Configure pll_cfg 6. */
		aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
						BIT(U3P2PLL0_BIAS_EN), BIT(U3P2PLL0_BIAS_EN));
		udelay(5);
		aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
						BIT(U3P2PLL0_RSTN), BIT(U3P2PLL0_RSTN));
		usleep_range(40, 50);
		aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
						BIT(U3P2PLL0_LOCK_EN), BIT(U3P2PLL0_LOCK_EN));
		usleep_range(160, 170);
		aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
					BIT(U3P2PLL_HCSL_LDO_EN_TM) | BIT(U3P2PLL_HCSL_DREN0_TM),
						BIT(U3P2PLL_HCSL_LDO_EN_TM));
		usleep_range(20, 30);
		aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
						BIT(U3P2PLL1_BIAS_EN), BIT(U3P2PLL1_BIAS_EN));
		usleep_range(50, 60);
		aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
						BIT(U3P2PLL1_RSTN), BIT(U3P2PLL1_RSTN));
		usleep_range(40, 50);
		aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
						BIT(U3P2PLL1_RSTN_LOCK), BIT(U3P2PLL1_RSTN_LOCK));
		udelay(1);
		aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
						BIT(U3P2PLL1_LOCK_START), BIT(U3P2PLL1_LOCK_START));
	} else {
		/* Enable sw configure upcrx_igen_en & upcrx_ldo_en. */
		aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x20, 0x3 << 14, 0x3 << 14);
		/* Enable upcrx_igen_en & upcrx_ldo_en. */
		aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x28, 0x3, 0x3);
		/* Configure pll_cfg 0~5. */
		aml_usb3_phy_write_reg32(phy, 0x00FA117F, phy->ctrl_reg + 0x2C);
		aml_usb3_phy_write_reg32(phy, 0x72007821, phy->ctrl_reg + 0x30);
		aml_usb3_phy_write_reg32(phy, 0xA0900000, phy->ctrl_reg + 0x34);
		aml_usb3_phy_write_reg32(phy, 0x081C1C23, phy->ctrl_reg + 0x38);
		aml_usb3_phy_write_reg32(phy, 0x1435DC92, phy->ctrl_reg + 0x3C);
		aml_usb3_phy_write_reg32(phy, 0x00000000, phy->ctrl_reg + 0x40);
		/* enable u3p2pll_hcsl_ldo_en & u3p2pll_hcsl_dren0 */
		aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x44,
					BIT(U3P2PLL_HCSL_LDO_EN_TM) | BIT(U3P2PLL_HCSL_DREN0_TM),
					BIT(U3P2PLL_HCSL_LDO_EN_TM));
		/* HW do the rest. */
		usleep_range(320, 400);
	}

	for (plldone_i = 5; plldone_i > 0; plldone_i--) {
		usleep_range(20, 30);
		if (IS_PLL_LOCKED(readl(phy->ctrl_reg + 0x154)))
			goto done;
	}

	dev_err(phy->dev, "usb3 pll not locked. val: 0x%08x\n",
						readl(phy->ctrl_reg + 0x154));
	return -1;

done:
	return 0;
}

static int aml_usb3_phy_pll_init(struct aml_usb3_phy *phy)
{
	return aml_usb3_phy_pll_init_v0(phy);
}

static void aml_usb3_phy_trim(struct aml_usb3_phy *phy)
{
	u32 raw = aml_usb3_phy_read_reg32(phy, phy->trim_reg);
	u32 val = 0;

	/* tx termination impedance. */
	if (raw & BIT(5))
		val = raw & 0x1f;
	else
		val = 0x10;

	aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x16c, 0xff << 24, val << 24);

	/* rx termination impedance. */
	if (raw & BIT(10))
		val = (raw & 0x3c0) >> 6;
	else
		val = 0x7;

	aml_usb3_phy_modify_reg32(phy, phy->ctrl_reg + 0x4, 0xf << 4, val << 4);
}

static int aml_usb3_phy_init(struct usb_phy *p)
{
	struct aml_usb3_phy *phy = phy_to_amlusb3phy(p);
	int ret = 0;

	if (!aml_usb3_phy_is_sw_on(phy))
		return 0;

	if (aml_usb3_phy_is_hw_on(phy))
		return 0;

	ret = aml_usb3_phy_set_clk(phy, true);
	if (ret)
		goto exit;

	aml_usb3_phy_set_power(phy, true);
	aml_usb3_phy_pre_reset(phy);

	ret = aml_usb3_phy_pll_init(phy);
	if (ret)
		goto cleanup;

	aml_usb3_phy_trim(phy);
	aml_usb3_phy_post_reset(phy);

	aml_usb3_phy_set_hw_on(phy, true);

	return ret;

cleanup:
	aml_usb3_phy_set_power(phy, false);
	aml_usb3_phy_set_clk(phy, false);

exit:
	return ret;
}

static int aml_usb3_phy_suspend(struct usb_phy *p, int suspend)
{
	struct aml_usb3_phy *phy = phy_to_amlusb3phy(p);

	if (!aml_usb3_phy_is_hw_on(phy))
		return 0;

	/* todo */
	return 0;
}

static void aml_usb3_phy_shutdown(struct usb_phy *p)
{
	struct aml_usb3_phy *phy = phy_to_amlusb3phy(p);

	if (!aml_usb3_phy_is_hw_on(phy))
		return;

	aml_usb3_phy_set_clk(phy, false);
	aml_usb3_phy_set_power(phy, false);

	aml_usb3_phy_set_hw_on(phy, false);
}

static int aml_usb3_phy_probe(struct platform_device *pdev)
{
	struct aml_usb3_phy *phy;
	struct device *dev = &pdev->dev;
	u8 portnum;
	bool phy_off = true;
	bool pll_sw_cfg = true;
	struct resource *reg_res = NULL;
	void __iomem *cfg_reg = NULL;
	void __iomem *reset_reg = NULL;
	void __iomem *ctrl_reg = NULL;
	void __iomem *trim_reg = NULL;
	u8 phy_id = 0;
	u32 reset_level_shift = 0;
	u8 usb3_apb_reset_bit = 0;
	u8 usb3_phy_reset_bit = 0;
	u8 usb3_controller_reset_bit = 0;
	u8 num_clk = 0;
	int ret, i;

	phy = devm_kzalloc(dev, sizeof(*phy), GFP_KERNEL);
	if (!phy)
		return -ENOMEM;

	ret = of_property_read_u8(dev->of_node, "portnum", &portnum);
	if (ret)
		return ret;

	if (!portnum) {
		dev_err(dev, "This usb phy has no port.\n");
			goto no_port;
	}

	reg_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!reg_res)
		return -EINVAL;

	phy->cfg_reg_phy = reg_res->start;
	phy->cfg_reg_size = resource_size(reg_res);

	cfg_reg = devm_ioremap(dev, reg_res->start, resource_size(reg_res));
	if (IS_ERR(cfg_reg))
		return PTR_ERR(cfg_reg);

	reg_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!reg_res)
		return -EINVAL;

	phy->reset_reg_phy = reg_res->start;
	phy->reset_reg_size = resource_size(reg_res);

	reset_reg = devm_ioremap(dev, reg_res->start, resource_size(reg_res));
	if (IS_ERR(reset_reg))
		return PTR_ERR(reset_reg);

	reg_res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	if (!reg_res)
		return -EINVAL;

	phy->ctrl_reg_phy = reg_res->start;
	phy->ctrl_reg_size = resource_size(reg_res);

	ctrl_reg = devm_ioremap(dev, reg_res->start, resource_size(reg_res));
	if (IS_ERR(ctrl_reg))
		return PTR_ERR(ctrl_reg);

	reg_res = platform_get_resource(pdev, IORESOURCE_MEM, 3);
	if (!reg_res)
		dev_err(dev, "This usb phy has no trim reg?\n");

	phy->trim_reg_phy = reg_res->start;
	phy->trim_reg_size = resource_size(reg_res);

	trim_reg = devm_ioremap(dev, reg_res->start, resource_size(reg_res));
	if (IS_ERR(ctrl_reg))
		return PTR_ERR(ctrl_reg);

	phy_off = of_property_read_bool(dev->of_node, "off");

	pll_sw_cfg = of_property_read_bool(dev->of_node, "pll-sw-cfg");

	ret = of_property_read_u8(dev->of_node, "phy-id", &phy_id);
	if (ret)
		return ret;

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

	ret = of_property_read_u8(dev->of_node, "num_clk", &num_clk);
	if (ret)
		return ret;

	for (i = 0; i < num_clk; i++) {
		memset(aml_usb3_phy_clk_name, 0, sizeof(aml_usb3_phy_clk_name));
		sprintf(aml_usb3_phy_clk_name, "u3p_clk_%d", i);
		phy->clk[i] = devm_clk_get(dev, aml_usb3_phy_clk_name);
		if (IS_ERR(phy->clk[i]))
			return PTR_ERR(phy->clk[i]);
	}

	dev_dbg(&pdev->dev, "USB3 phy probe:cfg_reg_phy:%pa, iomap cfg_reg:%px, s:%pa\n",
			&phy->cfg_reg_phy, cfg_reg, &phy->cfg_reg_size);
	dev_dbg(&pdev->dev, "USB3 phy probe:reset_phy:%pa, iomap reset_reg:%px, s:%pa\n",
			&phy->reset_reg_phy, reset_reg, &phy->reset_reg_size);
	dev_dbg(&pdev->dev, "USB3 phy probe:ctrl_phy:%pa, iomap ctrl_reg:%px, s:%pa\n",
			&phy->ctrl_reg_phy, ctrl_reg, &phy->ctrl_reg_size);
	dev_dbg(&pdev->dev, "USB3 phy probe:trim_phy:%pa, iomap trim_reg:%px, s:%pa\n",
			&phy->trim_reg_phy, trim_reg, &phy->trim_reg_size);
	dev_dbg(&pdev->dev, "USB3 phy_off:%d, pll_sw_cfg:%d, phy_id:%d, num_clk:%d\n"
						 "reset_level_shift:0x%x, usb3-apb-reset-bit:%d\n"
						 "usb3-phy-reset-bit:%d, usb3-controller-reset-bit:%d\n",
						phy_off, pll_sw_cfg, phy_id, num_clk,
						reset_level_shift, usb3_apb_reset_bit,
						usb3_phy_reset_bit, usb3_controller_reset_bit);

no_port:
	phy->dev = dev;
	phy->cfg_reg = cfg_reg;
	phy->ctrl_reg = ctrl_reg;
	phy->trim_reg = trim_reg;
	phy->portnum = portnum;
	phy->phy.dev = phy->dev;
	phy->phy.label = "aml-usb3phy";
	phy->phy.init = aml_usb3_phy_init;
	phy->phy.set_suspend = aml_usb3_phy_suspend;
	phy->phy.shutdown = aml_usb3_phy_shutdown;
	phy->phy.type = USB_PHY_TYPE_USB3;
	phy->phy.flags = AML_USB3_PHY_DISABLE;
	phy->reset_reg = reset_reg;
	phy->phy_id = phy_id;
	phy->reset_level_shift = reset_level_shift;
	phy->usb3_apb_reset_bit = usb3_apb_reset_bit;
	phy->usb3_phy_reset_bit = usb3_phy_reset_bit;
	phy->usb3_controller_reset_bit = usb3_controller_reset_bit;
	phy->off = phy_off;
	phy->pll_sw_cfg = pll_sw_cfg;
	phy->num_clk = num_clk;

	/* Sync state to hw off. */
	if (aml_usb3_phy_is_exist(phy))
		aml_usb3_phy_set_power(phy, false);

	usb_add_phy_dev(&phy->phy);
	platform_set_drvdata(pdev, phy);
	pm_runtime_enable(phy->dev);

	return 0;
}

static int aml_usb3_phy_remove(struct platform_device *pdev)
{
	/* todo */
	return 0;
}

#ifdef CONFIG_PM_RUNTIME
static int aml_usb3_phy_runtime_suspend(struct device *dev)
{
	struct aml_usb3_phy *phy = (struct aml_usb3_phy *)
				platform_get_drvdata(to_platform_device(dev));

	if (!aml_usb3_phy_is_hw_on(phy))
		return 0;

	/* todo */
	return 0;
}

static int aml_usb3_phy_runtime_resume(struct device *dev)
{
	struct aml_usb3_phy *phy = (struct aml_usb3_phy *)
		platform_get_drvdata(to_platform_device(dev));

	if (!aml_usb3_phy_is_hw_on(phy))
		return 0;

	/* todo */
	return 0;
}

static const struct dev_pm_ops aml_usb3_phy_pm_ops = {
	SET_RUNTIME_PM_OPS(aml_usb3_phy_runtime_suspend,
		aml_usb3_phy_runtime_resume,
		NULL)
};

#define AML_USB3_PHY_DEV_PM_OPS     (&aml_usb3_phy_pm_ops)
#else
#define AML_USB3_PHY_DEV_PM_OPS     NULL
#endif

#ifdef CONFIG_OF
static const struct of_device_id aml_usb3_phy_id_table[] = {
	{ .compatible = "amlogic, amlogic-usb3-phy" },
	{ .compatible = "amlogic, aml-usb3-phy" },
	{}
};
MODULE_DEVICE_TABLE(of, aml_usb3_phy_id_table);
#endif

static struct platform_driver aml_usb3_phy_driver = {
	.probe		= aml_usb3_phy_probe,
	.remove		= aml_usb3_phy_remove,
	.driver		= {
		.name	= "aml-usb3-phy",
		.owner	= THIS_MODULE,
		.pm	= AML_USB3_PHY_DEV_PM_OPS,
		.of_match_table = of_match_ptr(aml_usb3_phy_id_table),
	},
};

#if IS_BUILTIN(CONFIG_AMLOGIC_CRG)

int __init aml_usb3_drv_init(void)
{
	return platform_driver_register(&aml_usb3_phy_driver);
}
#else
module_platform_driver(aml_usb3_phy_driver);

MODULE_ALIAS("platform: aml_usb3");
MODULE_AUTHOR("Amlogic Inc.");
MODULE_DESCRIPTION("amlogic USB3 phy driver");
MODULE_LICENSE("GPL v2");
#endif
