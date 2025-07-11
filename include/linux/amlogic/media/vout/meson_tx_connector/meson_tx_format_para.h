/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_TX_FORMAT_PARA_H__
#define __MESON_TX_FORMAT_PARA_H__

#include <linux/hdmi.h>

#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_mode.h>

enum frl_rate_enum {
	FRL_NONE = 0,
	FRL_3G3L = 1,
	FRL_6G3L = 2,
	FRL_6G4L = 3,
	FRL_8G4L = 4,
	FRL_10G4L = 5,
	FRL_12G4L = 6,
	FRL_RATE_MAX = 7,
};

/* Table 1-2: Glossary of Terms */
enum dp_link_rate {
	RAR, /* 162MHz for 2.7Gbps/lane */
	HBR, /* 270MHz for 2.7Gbps/lane */
	HBR2, /* 540MHz for 5.4Gbps/lane */
	HBR3, /* 810MHz for 8.1Gbps/lane */
	LINK_RATE_MAX
};

/* HW format param calculated from primary SW format param */
struct dptx_hw_fmt_para {
	enum dp_link_rate link_rate;
	u8 lane_count;
};

/* hw related format param, set in calc_format_para() func */
struct hdmitx_hw_fmt_para {
	u32 scrambler_en:1;
	u32 tmds_clk_div40:1;
	u32 tmds_clk; /* Unit: 1000 */
	u8 dsc_en;
	enum frl_rate_enum frl_rate;
};

struct meson_tx_format_para {
	/* below members for both dptx/hdmitx */
	unsigned char *name;
	unsigned char *sname;
	struct tx_timing timing;

	enum hdmi_color_depth cd; /* cd8, cd10 or cd12 */
	enum hdmi_colorspace cs; /* 0/1/2/3: rgb/422/444/420 */
	enum hdmi_quantization_range cr; /* limit, full */
	u32 frac_mode;

	/* below members only for hdmitx,
	 * TODO: move into struct hdmitx_hw_fmt_para later
	 */
	enum hdmi_vic vic;
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

	union {
		struct dptx_hw_fmt_para dptx_hw_para;
		struct hdmitx_hw_fmt_para hdmitx_hw_para;
	} tx_hw_para;
};

#endif
