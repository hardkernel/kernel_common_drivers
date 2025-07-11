/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMI_FORMAT_PARA_H__
#define __HDMI_FORMAT_PARA_H__

#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_mode.h>
#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_edid.h>
#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_format_para.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx.h>

int hdmitx_format_para_reset(struct meson_tx_format_para *para);

/* log_buf = null, will print with printk. or will write to log_buf.
 * return is the log len.
 */
int hdmitx_format_para_print(struct meson_tx_format_para *para, char *log_buf);

/* fmt_attr is still used by userspace, need rebuild this string from format para.
 * TODO: remove it when we delete fmt_attr sysfs.
 */
int hdmitx_format_para_rebuild_fmt_attr_str(struct meson_tx_format_para *para,
					    char *attr_str, int len);

/* parse attr string to get enum cs/cd/cr */
void hdmitx_parse_color_attr(char const *attr_str,
	enum hdmi_colorspace *cs, enum hdmi_color_depth *cd,
	enum hdmi_quantization_range *cr);

#endif
