/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _HDMITX_MODULE_H
#define _HDMITX_MODULE_H

#include <linux/wait.h>
#include <linux/clk.h>
#include <linux/cdev.h>
#include <linux/clk-provider.h>
#include <linux/device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/miscdevice.h>

#include <linux/amlogic/media/vout/vout_notify.h>
#include <linux/amlogic/media/vpu/vpu.h>
#include <linux/amlogic/media/vrr/vrr.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_hw_common.h>

#include <drm/amlogic/meson_connector_dev.h>

#define HDMITX_HZ  250

/***** use for hdmitx 20 start *****/
int hdmitx20_init_reg_map(struct platform_device *pdev);
void hdmitx20_sw_debug_func(struct hdmitx_common *tx_comm, const char *cmd_str);
void hdmitx20_meson_init(struct hdmitx_hw_common *hw_comm);
void hdmitx20_meson_uninit(struct hdmitx_hw_common *tx_hw);
struct hdmitx_common *hdmitx20_alloc_instance(struct device *device);
/***** use for hdmitx 20 end *****/

/***** use for hdmitx 21 start *****/
int hdmitx21_init_reg_map(struct platform_device *pdev);
void hdmitx21_sw_debug_func(struct hdmitx_common *tx_comm, const char *cmd_str);
void hdmitx21_meson_init(struct hdmitx_hw_common *hw_comm);
void hdmitx21_meson_uninit(struct hdmitx_hw_common *tx_hw);
struct hdmitx_common *hdmitx21_alloc_instance(struct device *device);
/***** use for hdmitx 21 end *****/

/* common api */
void hdmitx_ext_instance_init(struct hdmitx_common *tx_comm);
int hdmitx_common_setup_vsif_packet(struct hdmitx_common *tx_comm,
	enum vsif_type type, int on, void *param);
u32 reduce_0p1_percent(u32 value);
u32 reduce_1p3_percent(u32 value);
extern const enum hdmi_vic qms_brr_list[9];
extern const enum hdmi_vic game_brr_list[13];

#endif
