// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <linux/device.h>
#include <linux/vmalloc.h>

#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>
#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_log.h>

#include "dptx_log.h"
#include "dptx_internal.h"
#include "../meson_tx_internal.h"

conn_dev_to_txdev dptx_convert;

void dptx_register_dev_to_txdev_xlate(struct meson_tx_dev *base,
		conn_dev_to_txdev convert)
{
	dptx_convert  = convert;
}
EXPORT_SYMBOL(dptx_register_dev_to_txdev_xlate);

static int dptx_load_rx_cap_raw(struct dptx_common *tx_comm, char *raw_data)
{
	size_t str_len;
	int i;
	unsigned char *buf = NULL;
	size_t byte_len;
	unsigned char tmp[3];

	if (!tx_comm || !raw_data)
		return -1;

	str_len = strlen(raw_data);
	byte_len = (str_len - 1) / 2;
	if (byte_len > sizeof(tx_comm->base.edid_buf))
		return -1;
	buf = kzalloc(byte_len, GFP_KERNEL);
	if (!buf)
		return -1;

	/* convert the raw data to hex byte */
	for (i = 0; i < byte_len; i++) {
		tmp[0] = raw_data[i * 2];
		tmp[1] = raw_data[i * 2 + 1];
		tmp[2] = '\0';
		if (kstrtou8(tmp, 16, &buf[i])) {
			DPTX_INFO("%s convert %c%c error\n", __func__,
				raw_data[i * 2], raw_data[i * 2 + 1]);
			return -1;
		}
	}

	memset(tx_comm->base.edid_buf, 0, sizeof(tx_comm->base.edid_buf));
	memcpy(tx_comm->base.edid_buf, buf, byte_len);

	kfree(buf);
	DPTX_INFO("%s: %zu bytes loaded from string\n", __func__, str_len);
	return 0;
}

static ssize_t dptx_debug_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	const char *delim = " ";
	char *token;
	char *cur = (char *)buf;
	unsigned int addr, val;
	struct meson_tx_dev *tx_dev = dptx_convert(dev);
	struct dptx_common *tx_comm = to_dptx_common(tx_dev);
	struct tx_timing *timing_list = NULL;
	int cnt = 0;

	token = strsep(&cur, delim);
	if (token && strncmp(token, "load", 4) == 0) {
		/* load raw EDID/displayID data to buffer */
		token = strsep(&cur, delim);
		if (!token) {
			DPTX_INFO("[%s] cmd format: load raw_data\n",
				__func__);
			return count;
		}
		dptx_load_rx_cap_raw(tx_comm, token);
		meson_tx_clear_rx_cap(&tx_comm->base.rxcap);
		meson_tx_edid_parse(&tx_comm->base.rxcap, tx_comm->base.edid_buf,
			tx_comm->base.edid_parse_mask);
		display_id_cap_print(&tx_comm->base.rxcap.disp_id_cap);
	} else if (token && strncmp(token, "print_info", 10) == 0) {
		DPTX_INFO("drm_dptx_id: %d\n", tx_comm->dev_idx);
	} else if (token && strncmp(token, "r", 1) == 0) {
		token = strsep(&cur, delim);
		if (!token || kstrtouint(token, 16, &addr) < 0)
			return count;
		/* pr_info("read reg:0x%x val:0x%x\n", addr, rd_reg(addr)); */
	} else if (token && strncmp(token, "w", 1) == 0) {
		token = strsep(&cur, delim);
		if (!token || kstrtouint(token, 16, &addr) < 0)
			return count;

		token = strsep(&cur, delim);
		if (!token || kstrtouint(token, 16, &val) < 0)
			return count;

		DPTX_INFO("write reg:0x%x val:0x%x\n", addr, val);
	} else if (token && strncmp(token, "fake_plug", 9) == 0) {
		token = strsep(&cur, delim);
		if (!token || kstrtouint(token, 16, &val) < 0)
			return count;
		/* TODO: for fake plug = 1, should check actual hpd status */
		if (val == 0)
			meson_tx_plugout_handler(&tx_comm->base);
		else
			meson_tx_plugin_handler(&tx_comm->base);
		DPTX_INFO("fake hpd plug %s\n", val ? "in" : "out");
	} else if (token && strncmp(token, "get_modes", 8) == 0) {
		cnt = dptx_get_mode_list(tx_comm, &timing_list);
		if (cnt > 0) {
			/* meson_dptx_mode_probed_add(count, timing_list, connector); */
			vfree(timing_list);
			timing_list = NULL;
		}
		if (tx_comm->base.rxcap.disp_id_cap.version)
			display_id_cap_print(&tx_comm->base.rxcap.disp_id_cap);
		DPTX_INFO("%s get mode list count: %d\n", __func__, cnt);
	}

	return count;
}

static struct device_attribute dptx_attrs[] = {
	__ATTR(debug, 0200, NULL, dptx_debug_store),
};

int dptx_create_sysfs(struct meson_tx_dev *tx_dev)
{
	int i;
	int ret = 0;
	struct dptx_common *tx_comm = to_dptx_common(tx_dev);

	for (i = 0; i < ARRAY_SIZE(dptx_attrs); i++) {
		ret = device_create_file(tx_dev->dev, &dptx_attrs[i]);
		if (ret) {
			DPTX_ERROR("dptx_dev[%d] create attribute %s fail\n",
				tx_comm->dev_idx, dptx_attrs[i].attr.name);
			break;
		}
	}
	return ret;
}

void dptx_remove_sysfs(struct meson_tx_dev *tx_dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(dptx_attrs); i++)
		device_remove_file(tx_dev->dev, &dptx_attrs[i]);
}
