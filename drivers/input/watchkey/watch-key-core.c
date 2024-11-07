// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/amlogic/aml_mbox.h>
#include <linux/amlogic/watch-key.h>
#include "watch-key-private.h"

struct amlogic_watchkey_core {
	struct mbox_chan *mbox;
	struct class class;
	const struct attribute_group **groups;
	struct amlogic_watchkey **wks;
};

struct amlogic_watchkey_attr {
	struct class_attribute attr;
	struct amlogic_watchkey *wk;
};

#define WATCHKEY_CMD_STATE		0x0001
#define WATCHKEY_CMD_DISABLE		0x0002
#define WATCHKEY_CMD_ENABLE		0x0003
#define WATCHKEY_CMD_DUMP		0x0004
#define WATCHKEY_CMD_TEST		0x0005

#pragma pack(push, 1)
struct __mbox_load {
	u16 id;
	u16 cmd;
	u32 data[3];
};

#pragma pack(pop)

static int __watchkey_mbox_read(struct mbox_chan *mbox, struct __mbox_load *load)
{
	int ret;

	ret = aml_mbox_transfer_data(mbox, CMD_WATCHKEY_CTRL, load, sizeof(*load),
				     load, sizeof(*load), MBOX_SYNC);
	if (ret < 0)
		return ret;

	return 0;
}

static int __watchkey_mbox_write(struct mbox_chan *mbox, struct __mbox_load *load)
{
	int ret;

	ret = aml_mbox_transfer_data(mbox, CMD_WATCHKEY_CTRL, load, sizeof(*load),
				     NULL, 0, MBOX_SYNC);
	if (ret < 0)
		return ret;

	return 0;
}

static int __watchkey_bool_get(struct mbox_chan *mbox, u16 id, u16 cmd)
{
	int ret;
	struct __mbox_load load = {
		.id = id,
		.cmd = cmd,
		.data = { 0 },
	};

	ret = __watchkey_mbox_read(mbox, &load);
	if (ret)
		return ret;

	return (int)load.data[0];
}

static struct amlogic_watchkey *
amlogic_watchkey_find_by_id(struct amlogic_watchkey_core *core, int id)
{
	struct amlogic_watchkey *wk = *core->wks;

	while (wk && wk->id != id)
		wk++;

	return wk;
}

int amlogic_watchkey_state(struct amlogic_watchkey *wk)
{
	struct amlogic_watchkey_core *priv;

	if (IS_ERR_OR_NULL(wk))
		return -EINVAL;
	priv = dev_get_drvdata(wk->dev);

	return __watchkey_bool_get(priv->mbox, wk->id, WATCHKEY_CMD_STATE);
}
EXPORT_SYMBOL_GPL(amlogic_watchkey_state);

static int __watchkey_enable(struct mbox_chan *mbox, u16 id)
{
	struct __mbox_load load = {
		.id = id,
		.cmd = WATCHKEY_CMD_ENABLE,
		.data = { 0 },
	};

	return __watchkey_mbox_write(mbox, &load);
}

int amlogic_watchkey_enable(struct amlogic_watchkey *wk)
{
	struct amlogic_watchkey_core *priv;

	if (IS_ERR_OR_NULL(wk))
		return -EINVAL;
	priv = dev_get_drvdata(wk->dev);

	return __watchkey_enable(priv->mbox, wk->id);
}
EXPORT_SYMBOL_GPL(amlogic_watchkey_enable);

static int __watchkey_disable(struct mbox_chan *mbox, u16 id)
{
	struct __mbox_load load = {
		.id = id,
		.cmd = WATCHKEY_CMD_DISABLE,
		.data = { 0 },
	};

	return __watchkey_mbox_write(mbox, &load);
}

int amlogic_watchkey_disable(struct amlogic_watchkey *wk)
{
	struct amlogic_watchkey_core *priv;

	if (IS_ERR_OR_NULL(wk))
		return -EINVAL;
	priv = dev_get_drvdata(wk->dev);

	return __watchkey_disable(priv->mbox, wk->id);
}
EXPORT_SYMBOL_GPL(amlogic_watchkey_disable);

int amlogic_watchkey_dump(struct amlogic_watchkey *wk, u32 *d0, u32 *d1, u32 *d2)
{
	struct amlogic_watchkey_core *priv;
	int ret;
	struct __mbox_load load = {
		.id = wk->id,
		.cmd = WATCHKEY_CMD_DUMP,
		.data = { 0 },
	};

	if (IS_ERR_OR_NULL(wk))
		return -EINVAL;
	priv = dev_get_drvdata(wk->dev);

	ret = __watchkey_mbox_read(priv->mbox, &load);
	if (ret)
		return ret;

	if (d0)
		*d0 = load.data[0];
	if (d1)
		*d1 = load.data[1];
	if (d2)
		*d2 = load.data[2];

	return 0;
}
EXPORT_SYMBOL_GPL(amlogic_watchkey_dump);

int amlogic_watchkey_test(struct amlogic_watchkey *wk)
{
	struct amlogic_watchkey_core *priv;
	struct amlogic_watchkey *priv_wk;

	if (IS_ERR_OR_NULL(wk))
		return -EINVAL;
	priv = dev_get_drvdata(wk->dev);
	priv_wk = amlogic_watchkey_find_by_id(priv, wk->id);
	if (!priv_wk)
		return -EINVAL;

	if (priv_wk->fixed)
		return 1;

	return __watchkey_bool_get(priv->mbox, wk->id, WATCHKEY_CMD_TEST);
}
EXPORT_SYMBOL_GPL(amlogic_watchkey_test);

void amlogic_watchkey_report(struct amlogic_watchkey *wk, unsigned int type, unsigned int code)
{
	struct amlogic_watchkey_core *priv;
	struct amlogic_watchkey *priv_wk;

	if (IS_ERR_OR_NULL(wk))
		return;
	priv = dev_get_drvdata(wk->dev);
	priv_wk = amlogic_watchkey_find_by_id(priv, wk->id);
	if (!priv_wk)
		return;

	if (priv_wk->type == type && priv_wk->code == code)
		priv_wk->fixed = true;
}
EXPORT_SYMBOL_GPL(amlogic_watchkey_report);

void amlogic_watchkey_try_fixed(struct amlogic_watchkey *wk, struct input_dev *input)
{
	struct amlogic_watchkey_core *priv;
	struct amlogic_watchkey *priv_wk;

	if (IS_ERR_OR_NULL(wk) || IS_ERR_OR_NULL(input))
		return;
	priv = dev_get_drvdata(wk->dev);
	priv_wk = amlogic_watchkey_find_by_id(priv, wk->id);
	if (!priv_wk)
		return;

	if (priv_wk->fixed)
		return;

	if (amlogic_watchkey_test(priv_wk) != 1)
		return;

	priv_wk->fixed = true;

	if (!priv_wk->report_event)
		return;

	input_event(input, priv_wk->type, priv_wk->code, 1);
	input_sync(input);
	input_event(input, priv_wk->type, priv_wk->code, 0);
	input_sync(input);
	dev_dbg(wk->dev, "event: %d,%d (%s)\n", priv_wk->type, priv_wk->code, priv_wk->name);
}
EXPORT_SYMBOL_GPL(amlogic_watchkey_try_fixed);

static ssize_t state_show(const struct class *class, const struct class_attribute *attr, char *buf)
{
	struct amlogic_watchkey_attr *wk_attr =
		container_of(attr, struct amlogic_watchkey_attr, attr);
	int ret;

	ret = amlogic_watchkey_state(wk_attr->wk);
	if (ret < 0)
		return ret;

	return sprintf(buf, "%s\n", ret ? "enabled" : "disabled");
}

static ssize_t state_store(const struct class *class, const struct class_attribute *attr,
			   const char *buffer, size_t count)
{
	struct amlogic_watchkey_attr *wk_attr =
		container_of(attr, struct amlogic_watchkey_attr, attr);
	int ret = 0;

	if (count < 1)
		return -EINVAL;

	if ((buffer[0] == 'e') || (buffer[0] == 'E'))
		ret = amlogic_watchkey_enable(wk_attr->wk);
	else if (((buffer[0] == 'd') || (buffer[0] == 'D')))
		ret = amlogic_watchkey_disable(wk_attr->wk);
	else
		ret = -EINVAL;

	if (ret)
		return ret;

	return count;
}

static CLASS_ATTR_RW(state);

static ssize_t dump_show(const struct class *class, const struct class_attribute *attr, char *buf)
{
	struct amlogic_watchkey_attr *wk_attr =
		container_of(attr, struct amlogic_watchkey_attr, attr);
	int ret;
	u32 d0, d1, d2;

	ret = amlogic_watchkey_dump(wk_attr->wk, &d0, &d1, &d2);
	if (ret < 0)
		return ret;

	return sprintf(buf, "%08x %08x %08x\n", d0, d1, d2);
}

static CLASS_ATTR_RO(dump);

static ssize_t test_show(const struct class *class, const struct class_attribute *attr, char *buf)
{
	struct amlogic_watchkey_attr *wk_attr =
		container_of(attr, struct amlogic_watchkey_attr, attr);
	int ret;

	ret = amlogic_watchkey_test(wk_attr->wk);
	if (ret < 0)
		return ret;

	return sprintf(buf, "%s\n", ret ? "yes" : "no");
}

static CLASS_ATTR_RO(test);

static struct class_attribute *watchkey_attrs[] = {
	&class_attr_state,
	&class_attr_dump,
	&class_attr_test,
};

static int amlogic_watchkey_group_init(struct attribute_group *group,
				       struct amlogic_watchkey *wk)
{
	struct attribute **attrs;
	struct amlogic_watchkey_attr *wk_attr;
	int count = ARRAY_SIZE(watchkey_attrs);
	int index;

	/* Ends with NULL */
	attrs = devm_kcalloc(wk->dev, count + 1, sizeof(*attrs), GFP_KERNEL);
	if (!attrs)
		return -ENOMEM;

	for (index = 0; index < count; index++) {
		wk_attr = devm_kzalloc(wk->dev, sizeof(*wk_attr), GFP_KERNEL);
		if (!wk_attr)
			return -ENOMEM;
		wk_attr->attr = *watchkey_attrs[index];
		wk_attr->wk = wk;
		attrs[index] = &wk_attr->attr.attr;
	}

	group->name = wk->name;
	group->attrs = attrs;

	return 0;
}

static int amlogic_watchkey_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct amlogic_watchkey_core *priv;
	struct device_node *child_node;
	struct amlogic_watchkey *wk;
	struct attribute_group *group;
	int count;
	int index;
	int ret;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	/* Mailbox setup */
	priv->mbox = aml_mbox_request_channel_byidx(dev, 0);
	if (IS_ERR_OR_NULL(priv->mbox)) {
		dev_err(dev, "mbox failed\n");
		return -EINVAL;
	}
	priv->mbox->cl->tx_tout = 200;

	/* Parse DT */
	count = of_get_child_count(dev->of_node);
	priv->groups = devm_kcalloc(dev, count + 1, sizeof(*priv->groups), GFP_KERNEL);
	priv->wks = devm_kcalloc(dev, count + 1, sizeof(*priv->wks), GFP_KERNEL);
	if (!priv->groups || !priv->wks) {
		ret = -ENOMEM;
		goto mbox_free_out;
	}
	index = 0;
	for_each_child_of_node(dev->of_node, child_node) {
		wk = devm_kzalloc(dev, sizeof(*wk), GFP_KERNEL);
		if (!wk) {
			ret = -ENOMEM;
			goto mbox_free_out;
		}

		ret = __of_watchkey_parse(wk, child_node);
		if (ret) {
			of_node_put(child_node);
			goto mbox_free_out;
		}
		wk->dev = dev;

		group = devm_kzalloc(dev, sizeof(*group), GFP_KERNEL);
		if (!group) {
			of_node_put(child_node);
			ret = -ENOMEM;
			goto mbox_free_out;
		}

		ret = amlogic_watchkey_group_init(group, wk);
		if (ret) {
			of_node_put(child_node);
			goto mbox_free_out;
		}

		priv->groups[index] = group;
		priv->wks[index] = wk;
		index++;
	}

	/* Create class: /sys/class/watchkey/ */
	priv->class.name = "watchkey";
	priv->class.class_groups = priv->groups;
	ret = class_register(&priv->class);
	if (ret) {
		dev_err(dev, "couldn't register sysfs class\n");
		goto mbox_free_out;
	}

	dev_set_drvdata(dev, priv);

	dev_info(dev, "Watch-Key enabled. Node count: %d\n", count);
	return 0;
mbox_free_out:
	mbox_free_channel(priv->mbox);
	return ret;
}

static void amlogic_watchkey_remove(struct platform_device *pdev)
{
	struct amlogic_watchkey_core *priv = dev_get_drvdata(&pdev->dev);

	class_unregister(&priv->class);
	mbox_free_channel(priv->mbox);
}

static const struct of_device_id amlogic_watchkey_of_match[] = {
	{ .compatible = "amlogic,watch-key", },
	{ /* sentinel */ },
};

static struct platform_driver amlogic_watchkey_driver = {
	.probe = amlogic_watchkey_probe,
	.remove = amlogic_watchkey_remove,
	.driver = {
		.name = "amlogic-watch-key",
		.of_match_table = amlogic_watchkey_of_match,
	},
};

int __init amlogic_watchkey_driver_init(void)
{
	return platform_driver_register(&amlogic_watchkey_driver);
}

void __exit amlogic_watchkey_driver_exit(void)
{
	platform_driver_unregister(&amlogic_watchkey_driver);
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huqiang Qin <huqiang.qin@amlogic.com>");
MODULE_DESCRIPTION("Amlogic Watch-Key driver");
