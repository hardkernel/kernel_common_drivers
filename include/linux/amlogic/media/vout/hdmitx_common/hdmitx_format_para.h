/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMI_FORMAT_PARA_H__
#define __HDMI_FORMAT_PARA_H__

#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_mode.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx.h>

struct hdmi_format_para {
	enum hdmi_vic vic;
	unsigned char *name;
	unsigned char *sname;
	struct hdmi_timing timing;

	enum hdmi_color_depth cd; /* cd8, cd10 or cd12 */
	enum hdmi_colorspace cs; /* 0/1/2/3: rgb/422/444/420 */
	enum hdmi_quantization_range cr; /* limit, full */
	u32 frac_mode;

	/*hw related information, set in calc_format_para() func*/
	u32 scrambler_en:1;
	u32 tmds_clk_div40:1;
	u32 tmds_clk; /* Unit: 1000 */
	u8 dsc_en;
	enum frl_rate_enum frl_rate;
	/*hw related information end*/

	/* for future usage */
	u8 port_type;
	u8 port_id;

	u32 flag_3dfp:1;
	u32 flag_3dtb:1;
	u32 flag_3dss:1;
};

int hdmitx_format_para_reset(struct hdmi_format_para *para);

/* log_buf = null, will print with printk. or will write to log_buf.
 * return is the log len.
 */
int hdmitx_format_para_print(struct hdmi_format_para *para, char *log_buf);

/* fmt_attr is still used by userspace, need rebuild this string from format para.
 * TODO: remove it when we delete fmt_attr sysfs.
 */
int hdmitx_format_para_rebuild_fmtattr_str(struct hdmi_format_para *para, char *attr_str, int len);

/* parse attr string to get enum cs/cd/cr */
void hdmitx_parse_color_attr(char const *attr_str,
	enum hdmi_colorspace *cs, enum hdmi_color_depth *cd,
	enum hdmi_quantization_range *cr);

#endif
