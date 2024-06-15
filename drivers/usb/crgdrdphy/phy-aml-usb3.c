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

static inline void aml_usb3_phy_reset(struct aml_usb3_phy *phy)
{
	/* Reset u3phy only. U2phy driver will reset the controller. */
	writel((0x1 << phy->usb3_apb_reset_bit) | (0x1 << phy->usb3_phy_reset_bit),
		(void __iomem *)phy->reset_reg);
	usleep_range(100, 200);
}

static void aml_usb3_phy_set_power(struct aml_usb3_phy *phy, bool on)
{
	u32 val;

	if (on) {
		val = readl(phy->reset_reg + phy->reset_level_shift);
		writel(val | (0x1 << phy->usb3_apb_reset_bit) | (0x1 << phy->usb3_phy_reset_bit),
			(void __iomem *)
			((unsigned long)phy->reset_reg + phy->reset_level_shift));
	} else {
		val = readl(phy->reset_reg + phy->reset_level_shift);
		writel(val & ~(0x1 << phy->usb3_phy_reset_bit),
			(void __iomem *)
			((unsigned long)phy->reset_reg + phy->reset_level_shift));
	}
	usleep_range(100, 200);
}

static int aml_usb3_phy_pll_init_v0(struct aml_usb3_phy *phy)
{
#define U3P2PLL0_BIAS_EN 9
#define U3P2PLL0_RSTN 10
#define	U3P2PLL0_LOCK_EN 11
#define	U3P2PLL1_BIAS_EN 20
#define	U3P2PLL1_RSTN 21
#define	U3P2PLL1_RSTN_LOCK 22
#define	U3P2PLL1_LOCK_START 23
#define	U3P2PLL1_PLL_LOCK_FLAG  24
#define U3P2PLL0_PLL_DONE 25
#define PLL_LOCKED_MASK (BIT(U3P2PLL1_PLL_LOCK_FLAG) | BIT(U3P2PLL0_PLL_DONE))
#define IS_PLL_LOCKED(x) (((x) & PLL_LOCKED_MASK) == PLL_LOCKED_MASK)

	u32 val = 0;
	int rty = 5;
	int plldone_i;

	//0xFE35A020 [15:14]=2'b11
	val = readl(phy->ctrl_reg + 0x20);
	pr_debug("FE35A020: 0x%x.\n", val);
	val |= 0x3 << 14;
	pr_debug("wr: 0x%x.\n", val);
	writel(val, phy->ctrl_reg + 0x20);
	pr_debug("FE35A020: 0x%x.\n", readl(phy->ctrl_reg + 0x20));
	usleep_range(160, 170);

	//	0xFE35A028 [1:0]=2'b11
	val = readl(phy->ctrl_reg + 0x28);
	pr_debug("FE35A028: 0x%x.\n", val);
	val |= 0x3;
	val |= 0x1 << 10;
	pr_debug("wr: 0x%x.\n", val);
	writel(val, phy->ctrl_reg + 0x28);
	pr_debug("FE35A028: 0x%x.\n", readl(phy->ctrl_reg + 0x28));

	while (rty--) {
		/*enable sw force pll*/
		writel(0xFE000000, phy->ctrl_reg + 0x50);
		usleep_range(20, 30);

		writel(0x1C0422, phy->ctrl_reg + 0x38);

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

		for (plldone_i = 5; plldone_i > 0; plldone_i--) {
			usleep_range(20, 30);
			if (IS_PLL_LOCKED(readl(phy->ctrl_reg + 0x154)))
				goto done;
		}
		dev_dbg(phy->dev, "usb2 pll not locked. val: 0x%08x\n",
								readl(phy->ctrl_reg + 0x154));
	}

	dev_err(phy->dev, "%s set pll failed, exit, val:0x%08x.\n",
			__func__, readl(phy->ctrl_reg + 0x154));
	return -1;
done:
	return 0;
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

	aml_usb3_phy_reset(phy);
	aml_usb3_phy_set_power(phy, true);

	ret = aml_usb3_phy_pll_init_v0(phy);
	if (ret)
		goto cleanup;

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
	struct resource *reg_res = NULL;
	void __iomem *cfg_reg = NULL;
	void __iomem *reset_reg = NULL;
	void __iomem *ctrl_reg = NULL;
	u8 phy_id = 0;
	u32 reset_level_shift = 0;
	u8 usb3_apb_reset_bit = 0;
	u8 usb3_phy_reset_bit = 0;
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

	cfg_reg = devm_ioremap(dev, reg_res->start, resource_size(reg_res));
	if (IS_ERR(cfg_reg))
		return PTR_ERR(cfg_reg);

	reg_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!reg_res)
		return -EINVAL;

	phy->reset_reg_phy = reg_res->start;

	reset_reg = devm_ioremap(dev, reg_res->start, resource_size(reg_res));
	if (IS_ERR(reset_reg))
		return PTR_ERR(reset_reg);

	reg_res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	if (!reg_res)
		return -EINVAL;

	phy->ctrl_reg_phy = reg_res->start;

	ctrl_reg = devm_ioremap(dev, reg_res->start, resource_size(reg_res));
	if (IS_ERR(ctrl_reg))
		return PTR_ERR(ctrl_reg);

	phy_off = of_property_read_bool(dev->of_node, "off");

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

	dev_dbg(&pdev->dev, "USB3 phy probe:cfg_reg_phy:0x%lx, iomap cfg_reg:%px\n",
			(unsigned long)phy->cfg_reg_phy, cfg_reg);
	dev_dbg(&pdev->dev, "USB3 phy probe:reset_phy:0x%lx, iomap reset_reg:%px\n",
			(unsigned long)phy->reset_reg_phy, reset_reg);
	dev_dbg(&pdev->dev, "USB3 phy probe:ctrl_phy:0x%lx, iomap ctrl_reg:%px\n",
			(unsigned long)phy->ctrl_reg_phy, ctrl_reg);
	dev_dbg(&pdev->dev, "USB3 phy_off:%d, phy_id:%d, num_clk:%d\n"
						 "reset_level_shift:0x%x, usb3-apb-reset-bit:%d\n"
						 "usb3-phy-reset-bit:%d\n",
						phy_off, phy_id, num_clk, reset_level_shift,
						usb3_apb_reset_bit, usb3_phy_reset_bit);

no_port:
	phy->dev = dev;
	phy->cfg_reg = cfg_reg;
	phy->ctrl_reg = ctrl_reg;
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
	phy->off = phy_off;
	phy->num_clk = num_clk;

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
