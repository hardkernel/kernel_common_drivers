/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMITX_SYSFS_COMMON_H
#define __HDMITX_SYSFS_COMMON_H

#include <linux/amlogic/media/vout/meson_tx_connector/hdmitx_common/hdmitx_common.h>
#include <linux/amlogic/media/vout/meson_tx_connector/hdmitx_common/hdmitx_hw_common.h>

int hdmitx_sysfs_common_create(struct device *dev,
		struct hdmitx_common *tx_comm,
		struct hdmitx_hw_common *tx_hw);

int hdmitx_sysfs_common_destroy(struct device *dev);
ssize_t _show_aud_cap(struct rx_cap *prxcap, char *buf, int size);
/* dump rx cap information in edid */
int meson_tx_edid_print_rx_cap(const struct rx_cap *prxcap, char *buffer, int buffer_len);
#endif
