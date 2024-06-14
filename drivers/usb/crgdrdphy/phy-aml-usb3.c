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

static int aml_usb3_phy_suspend(struct usb_phy *p, int suspend)
{
	struct aml_usb3_phy *phy = phy_to_amlusb3phy(p);

	if (!phy->portnum)
		return 0;
	/* todo */
	return 0;
}

static void aml_usb3_phy_shutdown(struct usb_phy *p)
{
	struct aml_usb3_phy *phy = phy_to_amlusb3phy(p);
	u32 val;

	if (phy->suspend_flag || phy->phy.flags != AML_USB3_PHY_ENABLE || !phy->portnum)
		return;

	val = readl(phy->reset_reg + phy->reset_level_shift);
	writel(val & ~(1 << phy->usb3_apb_reset_bit) & ~(1 << phy->usb3_phy_reset_bit),
			(void __iomem *)
			((unsigned long)phy->reset_reg + phy->reset_level_shift));

	clk_disable_unprepare(phy->clk);
	phy->suspend_flag = 1;
}

static int aml_usb3_phy_init(struct usb_phy *p)
{
	struct aml_usb3_phy *phy = phy_to_amlusb3phy(p);
	u32 val = 0;
#define U3P2PLL0_BIAS_EN 9
#define U3P2PLL0_RSTN 10
#define	U3P2PLL0_LOCK_EN 11
#define	U3P2PLL1_BIAS_EN 20
#define	U3P2PLL1_RSTN 21
#define	U3P2PLL1_RSTN_LOCK 22
#define	U3P2PLL1_LOCK_START 23

	if (!phy->portnum)
		return 0;

	if (phy->suspend_flag) {
		phy->suspend_flag = 0;
		return 0;
	}

	/*enable sw force pll*/
	writel(0xFE000000, phy->ctrl_reg + 0x50);
	usleep_range(20, 30);

	val = readl(phy->ctrl_reg + 0x44);
	dev_info(phy->dev, "pll_cfg6 default val 0x%x.", val);

	val |= 1 << U3P2PLL0_BIAS_EN;
	writel(val, phy->ctrl_reg + 0x44);
	udelay(5);
	val |= 1 << U3P2PLL0_RSTN;
	writel(val, phy->ctrl_reg + 0x44);
	usleep_range(40, 50);
	val |= 1 << U3P2PLL0_LOCK_EN;
	writel(val, phy->ctrl_reg + 0x44);
	usleep_range(160, 170);
	val |= 1 << U3P2PLL1_BIAS_EN;
	writel(val, phy->ctrl_reg + 0x44);
	usleep_range(50, 60);
	val |= 1 << U3P2PLL1_RSTN;
	writel(val, phy->ctrl_reg + 0x44);
	usleep_range(40, 50);
	val |= 1 << U3P2PLL1_RSTN_LOCK;
	writel(val, phy->ctrl_reg + 0x44);
	udelay(1);
	val |= 1 << U3P2PLL1_LOCK_START;
	writel(val, phy->ctrl_reg + 0x44);

	/* Hw autocfg for now. */

	return 0;
}

static int aml_usb3_phy_probe(struct platform_device *pdev)
{
		struct aml_usb3_phy *phy;
		struct device *dev = &pdev->dev;
		//const void *prop = NULL;
		u8 portnum = 0;
		struct resource *reg_res = NULL;
		void __iomem *cfg_reg = NULL;
		void __iomem *reset_reg = NULL;
		void __iomem *ctrl_reg = NULL;
		//u32 ctrl_reg_phy = 0;
		//u32 ctrl_reg_size = 0;
		//u32 reset_reg_phy;
		//u32 reset_reg_size = 0;
		u8 phy_id = 0;
		u32 reset_level_shift = 0;
		u8 usb3_apb_reset_bit = 0;
		u8 usb3_phy_reset_bit = 0;
		int ret;
		u32 val;

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
		cfg_reg = ioremap(reg_res->start, resource_size(reg_res));
		if (IS_ERR(cfg_reg))
			return PTR_ERR(cfg_reg);
		dev_info(&pdev->dev, "USB3 phy probe:cfg_reg_phy:0x%lx, iomap cfg_reg:%px\n",
			 (unsigned long)phy->cfg_reg_phy, cfg_reg);

		reg_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
		if (!reg_res)
			return -EINVAL;
		phy->reset_reg_phy = reg_res->start;
		reset_reg = ioremap(reg_res->start, resource_size(reg_res));
		if (IS_ERR(reset_reg))
			return PTR_ERR(reset_reg);
		dev_info(&pdev->dev, "USB3 phy probe:reset_phy:0x%lx, iomap reset_reg:%px\n",
			 (unsigned long)phy->reset_reg_phy, reset_reg);

		reg_res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
		if (!reg_res)
			return -EINVAL;
		phy->ctrl_reg_phy = reg_res->start;
		ctrl_reg = ioremap(reg_res->start, resource_size(reg_res));
		if (IS_ERR(ctrl_reg))
			return PTR_ERR(ctrl_reg);
		dev_info(&pdev->dev, "USB3 phy probe:ctrl_phy:0x%lx, iomap ctrl_reg:%px\n",
			 (unsigned long)phy->ctrl_reg_phy, ctrl_reg);
#ifdef SDA_TEST
		ret = of_property_read_u32(dev->of_node, "reset-reg", &reset_reg_phy);
		if (ret < 0)
			return ret;

		ret = of_property_read_u32(dev->of_node, "reset-reg-size", &reset_reg_size);
		if (ret < 0)
			return ret;

		reset_reg = ioremap((resource_size_t)reset_reg_phy, (unsigned long)reset_reg_size);
		if (!reset_reg)
			return -ENOMEM;

		ret = of_property_read_u32(dev->of_node, "ctrl-reg", &ctrl_reg_phy);
		if (ret < 0)
			return ret;

		ret = of_property_read_u32(dev->of_node, "ctrl-reg-size", &ctrl_reg_size);
		if (ret < 0)
			return ret;

		ctrl_reg = ioremap((resource_size_t)ctrl_reg_phy, (unsigned long)ctrl_reg_size);
		if (!ctrl_reg)
			return -ENOMEM;
#endif
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

no_port:
		phy->dev = dev;
		phy->cfg_reg = cfg_reg;
		phy->ctrl_reg = ctrl_reg;
		phy->portnum = portnum;
		phy->suspend_flag = 0;
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

		if (phy->portnum > 0) {
			phy->clk = devm_clk_get(dev, "u3p_0");
			if (!IS_ERR(phy->clk)) {
				ret = clk_prepare_enable(phy->clk);
				if (ret) {
					dev_err(dev, "Failed to enable u3p_0 clock\n");
					ret = PTR_ERR(phy->clk);
					return ret;
				}
			} else {
				return PTR_ERR(phy->clk);
			}

			phy->clk = devm_clk_get(dev, "u3p_1");
			if (!IS_ERR(phy->clk)) {
				ret = clk_prepare_enable(phy->clk);
				if (ret) {
					dev_err(dev, "Failed to enable u3p_1 clock\n");
					ret = PTR_ERR(phy->clk);
					return ret;
				}
			} else {
				return PTR_ERR(phy->clk);
			}

			/* Reset u3phy only. U2phy driver will reset the controller. */
			writel((0x1 << usb3_apb_reset_bit) | (0x1 << usb3_phy_reset_bit),
				(void __iomem *)phy->reset_reg);

			usleep_range(100, 200);

			/* Enable u3phy. */
			val = readl(phy->reset_reg + phy->reset_level_shift);
			writel(val | (0x1 << usb3_apb_reset_bit) | (0x1 << usb3_phy_reset_bit),
				(void __iomem *)
				((unsigned long)phy->reset_reg + phy->reset_level_shift));

			usleep_range(100, 200);

			phy->phy.flags = AML_USB3_PHY_ENABLE;
		}

		usb_add_phy_dev(&phy->phy);
		platform_set_drvdata(pdev, phy);
		pm_runtime_enable(phy->dev);

		return 0;
}

static int aml_usb3_phy_remove(struct platform_device *pdev)
{
	struct aml_usb3_phy *phy = (struct aml_usb3_phy *)platform_get_drvdata(pdev);

	if (!phy->portnum)
		return 0;
	/* todo */
	return 0;
}

#ifdef CONFIG_PM_RUNTIME
static int aml_usb3_phy_runtime_suspend(struct device *dev)
{
	struct aml_usb3_phy *phy = (struct aml_usb3_phy *)
				platform_get_drvdata(to_platform_device(dev));

	if (!phy->portnum)
		return 0;
	/* todo */
	return 0;
}

static int aml_usb3_phy_runtime_resume(struct device *dev)
{
	struct aml_usb3_phy *phy = (struct aml_usb3_phy *)
		platform_get_drvdata(to_platform_device(dev));

	if (!phy->portnum)
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
