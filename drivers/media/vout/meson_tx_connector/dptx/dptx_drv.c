// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/component.h>
#include <linux/vmalloc.h>

#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>
#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_log.h>
#include <drm/amlogic/meson_drm_bind.h>

#include "../meson_tx_internal.h"
#include "dptx_log.h"
#include "dptx_internal.h"

#define DEVICE_NAME "dptx"
#define DPTX_DEV_COUNT 4

static int dptx_bind_drm(struct device *dev, struct device *master,
			   void *data)
{
	int drm_dptx_id;
	struct dptx_common *tx_common = dev_get_drvdata(dev);
	struct meson_drm_bound_data *bound_data = data;

	if (bound_data->connector_component_bind) {
		drm_dptx_id = bound_data->connector_component_bind
			(bound_data->drm,
			DRM_MODE_CONNECTOR_DisplayPort,
			&tx_common->base.conn_dev);
		DPTX_INFO("%s dp [%d]\n", __func__, drm_dptx_id);
		tx_common->drm_dptx_id = drm_dptx_id;
	} else {
		DPTX_INFO("no bind func from drm.\n");
	}

	return 0;
}

static void dptx_unbind_drm(struct device *dev, struct device *master,
			      void *data)
{
	struct dptx_common *tx_common = dev_get_drvdata(dev);
	struct meson_drm_bound_data *bound_data = data;

	if (bound_data->connector_component_unbind) {
		bound_data->connector_component_unbind(bound_data->drm,
			DRM_MODE_CONNECTOR_DisplayPort,
			&tx_common->base.conn_dev);
		DPTX_INFO("%s success\n", __func__);
	} else {
		DPTX_INFO("no unbind func.\n");
	}
}

/*drm component bind ops*/
static const struct component_ops dptx_bind_ops = {
	.bind	= dptx_bind_drm,
	.unbind	= dptx_unbind_drm,
};

static int dptx_get_dt_info(struct platform_device *pdev,
	struct dptx_common *tx_comm, struct dptx_hw_common *hw_comm)
{
	int ret;
	u32 dev_index = 0;
	u32 irq_index;
	struct dptx_cap *tx_cap = kzalloc(sizeof(*tx_cap), GFP_KERNEL);

	if (!tx_comm || !hw_comm)
		return -1;

	if (!pdev->dev.of_node)
		return -1;
	ret = of_property_read_u32(pdev->dev.of_node, "dev_index", &dev_index);
	if (ret) {
		DPTX_INFO("%s: no index exist, default to 0\n", __func__);
		tx_comm->dev_idx = 0;
	} else {
		if (dev_index >= DPTX_DEV_COUNT) {
			DPTX_ERROR("%s: index %d exceed maximum count\n", __func__, dev_index);
			return -1;
		}

		tx_comm->dev_idx = dev_index;
	}
	hw_comm->dev_idx = tx_comm->dev_idx;

	/* TODO: get tx capability from dts */
	tx_comm->base.tx_cap = tx_cap;

	irq_index = platform_get_irq_byname(pdev, "dptx_hpd");
	if (irq_index == -ENXIO) {
		DPTX_ERROR("%s: dptx irq_id not found\n", __func__);
		return -ENXIO;
	}
	hw_comm->irq_id = irq_index;
	return 0;
}

static struct meson_tx_phy *meson_tx_probe_phy(struct device *dev, struct meson_tx_hw *tx_hw)
{
	struct platform_device *phy_pdev;
	struct device_node *phy_node;
	struct meson_tx_phy *tx_phy = NULL;

	phy_node = of_parse_phandle(dev->of_node, "tx_phy", 0);
	if (!phy_node) {
		dev_err(dev, "cannot find phy device\n");
		return NULL;
	}

	phy_pdev = of_find_device_by_node(phy_node);
	if (phy_pdev)
		tx_phy = platform_get_drvdata(phy_pdev);

	of_node_put(phy_node);

	if (!phy_pdev) {
		dev_err(dev, "%s: phy driver is not ready\n", __func__);
		return NULL;
	}
	if (!tx_phy) {
		put_device(&phy_pdev->dev);
		dev_err(dev, "%s: phy driver is not ready\n", __func__);
		return NULL;
	}

	return tx_phy;
}

static int dptx_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct dptx_common *tx_comm;
	struct meson_tx_hw *tx_hw;
	const struct meson_tx_plat_data *plat_data;
	struct device *device = &pdev->dev;

	struct meson_tx_phy *tx_phy;

	tx_comm = kzalloc(sizeof(*tx_comm), GFP_KERNEL);

	if (!tx_comm)
		return -ENOMEM;

	/* get config from device tree match */
	plat_data = of_device_get_match_data(&pdev->dev);

	tx_hw = plat_data->alloc_tx_hw();
	tx_comm->base.pdev = device;

	/* dptx common init */
	ret = dptx_common_init(tx_comm, tx_hw);
	if (ret < 0)
		return -ENOMEM;

	/* get config from dts */
	dptx_get_dt_info(pdev, tx_comm, tx_comm->hw_comm);

	/* dptx probe phy */
	tx_phy = meson_tx_probe_phy(device, tx_hw);

	/* dptx tx hw setup tx_phy& tx_log */
	meson_tx_hw_setup_phy(tx_hw, tx_phy);
	tx_hw->init_tx_hw(tx_hw);

	dev_set_drvdata(device, tx_comm);

	component_add(device, &dptx_bind_ops);

	DPTX_INFO("%s for dev_index[%d]\n", __func__, tx_comm->dev_idx);

	return 0;
}

static void dptx_remove(struct platform_device *pdev)
{
	const struct meson_tx_plat_data *plat_data;
	struct device *device = &pdev->dev;
	struct dptx_common *tx_comm = dev_get_drvdata(device);
	/* get config from device tree match */
	plat_data = of_device_get_match_data(&pdev->dev);

	if (!tx_comm)
		return;

	component_del(device, &dptx_bind_ops);

	dev_set_drvdata(device, NULL);

	plat_data->free_tx_hw(tx_comm->base.tx_hw_base);
	kfree(tx_comm->base.tx_cap);
	meson_tx_dev_release(&tx_comm->base);

	DPTX_INFO("%s for dev_index[%d]\n", __func__, tx_comm->dev_idx);
	kfree(tx_comm);
}

static struct meson_tx_plat_data dptx_plat_data_a9 = {
	.alloc_tx_hw	= dptx20_alloc_tx_hw,
	.free_tx_hw		= dptx20_free_tx_hw,
};

static const struct of_device_id dptx_of_match[] = {
	{
		.compatible	= "amlogic, dptx-t7c",
		.data		= &dptx_plat_data_a9,
	},
	{
		.compatible	= "amlogic, dptx-a9",
		.data		= &dptx_plat_data_a9,
	},
	{},
};

static struct platform_driver dptx_driver = {
	.probe	 = dptx_probe,
	.remove	 = dptx_remove,
	.driver	 = {
		.name = DEVICE_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(dptx_of_match),
	}
};

int  __init dptx_init(void)
{
	return platform_driver_register(&dptx_driver);
}

void __exit dptx_exit(void)
{
	DPTX_INFO("%s\n", __func__);
	platform_driver_unregister(&dptx_driver);
}

