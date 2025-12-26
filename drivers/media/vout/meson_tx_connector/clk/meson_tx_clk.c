// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/printk.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/of_address.h>
#include <linux/amlogic/media/vout/meson_tx_connector/clk/meson_tx_clk.h>
#include "meson_tx_clk_internal.h"

#define DEVICE_NAME "meson_tx_clk"

static int tx_clk_reg_map(struct platform_device *pdev, struct meson_tx_clk *tx_clk)
{
	u8 i = 0;
	struct resource res;
	struct device_node *np = NULL;

	if (!pdev || !tx_clk) {
		pr_err("%s invalid clk instance\n", __func__);
		return -EINVAL;
	}

	np = pdev->dev.of_node;
	/* get the core, vpu, ana, ... etc address from dts */
	for (i = 0; i < REG_MAX; i++) {
		if (of_address_to_resource(np, i, &res)) {
			pr_err("%s not get regbase index %d\n", __func__, i);
			return -1;
		}
		tx_clk->reg_io_base[i] = devm_ioremap(&pdev->dev, res.start,
						 resource_size(&res));
		if (IS_ERR(tx_clk->reg_io_base[i]))
			return -ENXIO;
	}

	return 0;
}

static void __iomem *paddr_map_vaddr(void __iomem *base, u32 addr_offset)
{
	return (void __iomem *)(base + addr_offset);
}

u32 tx_clk_read_reg(void __iomem *base, u32 addr_offset)
{
	u32 val;

	val = readl(paddr_map_vaddr(base, addr_offset));
	pr_debug("clk_reg Rd32[0x%08x] 0x%08x\n", addr_offset, val);
	return val;
}

void tx_clk_write_reg(void __iomem *base, u32 addr, u32 val)
{
	u32 rval;

	writel(val, paddr_map_vaddr(base, addr));
	rval = readl(paddr_map_vaddr(base, addr));
	if (val != rval)
		pr_err("clk_reg Wr32[0x%08x] 0x%08x != Rd32 0x%08x\n",
			addr, val, rval);
	else
		pr_debug("clk_reg Wr32[0x%08x] 0x%08x\n", addr, val);
}

void tx_clk_set_reg_bits(void __iomem *base, u32 addr, u32 value, u32 offset, u32 len)
{
	u32 data32 = 0;

	data32 = tx_clk_read_reg(base, addr);
	data32 &= ~(((1 << len) - 1) << offset);
	data32 |= (value & ((1 << len) - 1)) << offset;
	tx_clk_write_reg(base, addr, data32);
}

int meson_tx_clk_set(struct meson_tx_clk *tx_clk, u32 clk_mask)
{
	if (!tx_clk || !tx_clk->tx_clk_ops) {
		pr_err("%s invalid clk param\n", __func__);
		return -EINVAL;
	}

	pr_info("%s", __func__);
	mutex_lock(&tx_clk->clk_set_mutex);
	if (tx_clk->tx_clk_ops->tx_clk_set)
		tx_clk->tx_clk_ops->tx_clk_set(tx_clk, clk_mask);
	mutex_unlock(&tx_clk->clk_set_mutex);
	return 0;
}

int meson_tx_clk_disable(struct meson_tx_clk *tx_clk, u32 clk_mask)
{
	if (!tx_clk || !tx_clk->tx_clk_ops) {
		pr_err("%s invalid clk param\n", __func__);
		return -EINVAL;
	}

	pr_info("%s", __func__);
	mutex_lock(&tx_clk->clk_set_mutex);
	if (tx_clk->tx_clk_ops->tx_clk_disable)
		tx_clk->tx_clk_ops->tx_clk_disable(tx_clk, clk_mask);
	mutex_unlock(&tx_clk->clk_set_mutex);
	return 0;
}

static int meson_tx_clk_probe(struct platform_device *pdev)
{
	struct meson_tx_clk *tx_clk;
	const struct meson_tx_clk_ops *plat_data;
	int ret = 0;

	plat_data = of_device_get_match_data(&pdev->dev);
	if (!plat_data) {
		pr_info("%s no matched clk data\n", __func__);
		return -EINVAL;
	}

	tx_clk = kzalloc(sizeof(*tx_clk), GFP_KERNEL);
	if (!tx_clk)
		return -ENOMEM;

	ret = tx_clk_reg_map(pdev, tx_clk);
	if (ret)
		goto reg_map_err;
	tx_clk->tx_clk_ops = plat_data;
	mutex_init(&tx_clk->clk_set_mutex);

	dev_set_drvdata(&pdev->dev, tx_clk);

	pr_info("%s %px\n", __func__, tx_clk);
	return 0;

reg_map_err:
	kfree(tx_clk);
	return ret;
}

static void meson_tx_clk_remove(struct platform_device *pdev)
{
	struct device *device = &pdev->dev;
	struct meson_tx_clk *tx_clk = dev_get_drvdata(device);

	kfree(tx_clk);
	pr_info("%s %s\n", __func__, dev_name(device));
}

static struct meson_tx_clk_ops tx_clk_ops_a9 = {
	.tx_clk_set = tx_bulk_clk_set_a9,
	.tx_clk_disable = tx_bulk_clk_disable_a9,
};

static const struct of_device_id meson_tx_clk_of_match[] = {
	{
		.compatible	 = "amlogic, tx-clk-a9",
		.data = &tx_clk_ops_a9,
	},
	{},
};

static struct platform_driver meson_tx_clk_driver = {
	.probe	 = meson_tx_clk_probe,
	.remove	 = meson_tx_clk_remove,
	.driver	 = {
		.name = DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(meson_tx_clk_of_match),
	}
};

int  __init meson_tx_clk_drv_init(void)
{
	return platform_driver_register(&meson_tx_clk_driver);
}

void __exit meson_tx_clk_drv_exit(void)
{
	pr_info("%s\n", __func__);
	platform_driver_unregister(&meson_tx_clk_driver);
}
