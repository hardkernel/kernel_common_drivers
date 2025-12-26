// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/amlogic/media/vout/meson_tx_connector/phy/meson_tx_phy_core.h>

#define DEVICE_NAME "meson_tx_general_phy"

static int meson_tx_general_phy_init(void *data)
{
	pr_info("%s\n", __func__);
	return 0;
}

static int meson_tx_general_phy_exit(void *data)
{
	pr_info("%s\n", __func__);
	return 0;
}

static int meson_tx_general_phy_power_on(void *data)
{
	pr_info("%s\n", __func__);
	return 0;
}

static int meson_tx_general_phy_power_off(void *data)
{
	pr_info("%s\n", __func__);
	return 0;
}

static int meson_tx_general_phy_set_mode(void *data, enum meson_tx_phy_mode mode)
{
	pr_info("%s\n", __func__);
	return 0;
}

static int meson_tx_general_phy_configure(void *phy, struct meson_tx_phy_cfg_opts *opts)
{
	return 0;
}

static int meson_tx_general_phy_validate(void *phy, enum meson_tx_phy_mode mode,
			struct meson_tx_phy_cfg_opts *opts)
{
	pr_info("%s\n", __func__);
	return 0;
}

static int meson_tx_general_phy_calibrate(void *phy)
{
	pr_info("%s\n", __func__);
	return 0;
}

struct meson_tx_phy_ops tx_general_phy_ops = {
	.init = meson_tx_general_phy_init,
	.exit = meson_tx_general_phy_exit,
	.power_on = meson_tx_general_phy_power_on,
	.power_off = meson_tx_general_phy_power_off,
	.set_mode = meson_tx_general_phy_set_mode,
	.configure = meson_tx_general_phy_configure,
	.validate = meson_tx_general_phy_validate,
	.calibrate = meson_tx_general_phy_calibrate,
};

static int meson_tx_phy_probe(struct platform_device *pdev)
{
	struct meson_tx_phy *tx_phy;
	const struct meson_tx_phy_data *data;

	/* 1. Try to get config from device tree match */
	data = of_device_get_match_data(&pdev->dev);
	if (data) {
		pr_info("%s use dt\n", __func__);
	} else {
		/* 2. Fall back to legacy platform_data */
		data = dev_get_platdata(&pdev->dev);
		if (data) {
			pr_info("%s use legacy platform_data\n", __func__);
		} else {
			pr_err("No config found in DT or platform_data\n");
			return -EINVAL;
		}
	}

	tx_phy = kzalloc(sizeof(*tx_phy), GFP_KERNEL);
	if (!tx_phy)
		return -ENOMEM;

	tx_phy->ops = data->ops;
	dev_set_drvdata(&pdev->dev, tx_phy);

	pr_info("%s %px\n", __func__, tx_phy);

	return 0;
}

static void meson_tx_phy_remove(struct platform_device *pdev)
{
	struct device *device = &pdev->dev;

	pr_info("%s %s\n", __func__, dev_name(device));
}

static const struct meson_tx_phy_data normal_phy_data = {
	.ops = &tx_general_phy_ops,
};

static const struct of_device_id meson_tx_phy_of_match[] = {
	{
		.compatible	 = "amlogic, tx-phy-t7c",
		.data = &normal_phy_data,
	},
	{
		.compatible	 = "amlogic, tx-phy-a9",
		.data = &normal_phy_data,
	},
	{},
};

static struct platform_driver meson_tx_phy_driver = {
	.probe	 = meson_tx_phy_probe,
	.remove	 = meson_tx_phy_remove,
	.driver	 = {
		.name = DEVICE_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(meson_tx_phy_of_match),
	}
};

int  __init meson_tx_phy_drv_init(void)
{
	return platform_driver_register(&meson_tx_phy_driver);
}

void __exit meson_tx_phy_drv_exit(void)
{
	pr_info("%s\n", __func__);
	platform_driver_unregister(&meson_tx_phy_driver);
}

