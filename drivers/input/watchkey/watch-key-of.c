// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include "watch-key-private.h"

int __of_watchkey_parse(struct amlogic_watchkey *wk, struct device_node *np)
{
	int ret;

	ret = of_property_read_string(np, "wk-name", &wk->name);
	if (ret)
		return ret;

	ret = of_property_read_u32(np, "wk-id", &wk->id);
	if (ret)
		return ret;

	ret = of_property_read_u32(np, "wk-type", &wk->type);
	if (ret)
		return ret;

	ret = of_property_read_u32(np, "wk-code", &wk->code);
	if (ret)
		return ret;

	wk->report_event = of_property_read_bool(np, "wk-report-event");

	return 0;
}

static int __of_watchkey_get(struct amlogic_watchkey *wk, struct device_node *np)
{
	struct platform_device *pdev;
	struct device_node *node;
	int ret;

	node = of_parse_phandle(np, "watch-key", 0);
	if (!node)
		return -ENODEV;

	ret = __of_watchkey_parse(wk, node);
	if (ret)
		goto node_put_out;

	pdev = of_find_device_by_node(node->parent);
	if (!pdev) {
		ret = -ENODEV;
		goto node_put_out;
	}

	if (!pdev->dev.driver) {
		ret = -EPROBE_DEFER;
		goto node_put_out;
	}

	wk->dev = &pdev->dev;

	return 0;
node_put_out:
	of_node_put(node);
	return ret;
}

void of_amlogic_watchkey_put(struct amlogic_watchkey *wk)
{
	if (IS_ERR_OR_NULL(wk))
		return;

	of_node_put(wk->dev->of_node);
	put_device(wk->dev);
	kfree(wk);
}
EXPORT_SYMBOL_GPL(of_amlogic_watchkey_put);

struct amlogic_watchkey *of_amlogic_watchkey_get(struct device *dev)
{
	struct amlogic_watchkey *wk;
	int ret;

	wk = kzalloc(sizeof(*wk), GFP_KERNEL);
	if (!wk)
		return ERR_PTR(-ENOMEM);

	ret = __of_watchkey_get(wk, dev->of_node);
	if (ret)
		goto free_out;

	return wk;
free_out:
	kfree(wk);
	return ERR_PTR(ret);
}
EXPORT_SYMBOL_GPL(of_amlogic_watchkey_get);

static void devm_watchkey_get_free(void *wk)
{
	of_amlogic_watchkey_put(wk);
}

struct amlogic_watchkey *devm_of_amlogic_watchkey_get(struct device *dev)
{
	struct amlogic_watchkey *wk;
	int ret;

	wk = of_amlogic_watchkey_get(dev);
	if (IS_ERR(wk))
		return wk;

	ret = devm_add_action_or_reset(dev, devm_watchkey_get_free, wk);
	if (ret)
		return ERR_PTR(ret);

	return wk;
}
EXPORT_SYMBOL_GPL(devm_of_amlogic_watchkey_get);
