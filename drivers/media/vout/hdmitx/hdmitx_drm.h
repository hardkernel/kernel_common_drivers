/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMITX_DRM_H
#define __HDMITX_DRM_H

#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_hw_common.h>

int hdmitx_bind_meson_drm(struct device *device,
	struct hdmitx_common *tx_base,
	struct hdmitx_hw_common *tx_hw);
int hdmitx_unbind_meson_drm(struct device *device,
	struct hdmitx_common *tx_base,
	struct hdmitx_hw_common *tx_hw);

#endif
