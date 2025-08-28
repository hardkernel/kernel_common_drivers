/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_TX_FORMAT_PARA_H__
#define __MESON_TX_FORMAT_PARA_H__

#include <linux/hdmi.h>

#include <linux/amlogic/media/vout/vinfo.h>
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

enum dp_link_rate_e {
	DPTX_LINK_RATE_UNKNOWN = -1,
	DPTX_LINK_RATE_1P62GHZ = 0x06,
	DPTX_LINK_RATE_2P70GHZ = 0x0a,
	DPTX_LINK_RATE_5P40GHZ = 0x14,
	DPTX_LINK_RATE_8P10GHZ = 0x1e,
	DPTX_LINK_RATE_MAX,
};

enum dp_lane_count_e {
	DPTX_LANE_COUNT_UNKNOWN = -1,
	DPTX_LANE_COUNT_1 = 1,
	DPTX_LANE_COUNT_2 = 2,
	DPTX_LANE_COUNT_4 = 4,
	DPTX_LANE_COUNT_MAX,
};

/* HW format param calculated from primary SW format param */
struct dptx_hw_fmt_para {
	u32 total_bandwidth;
	enum dp_link_rate_e link_rate;
	enum dp_lane_count_e lane_count;
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
	bool cta_range; /* 1: cta range; 0: vesa range */
	bool bt709; /* 1: bt-709; 0: bt-601 */
	u32 frac_mode;
	/* video source virtual channel, used for MST index */
	u8 vc_id;
	/* enc->vpu_hdmi_if->hdmi/dp_core
	 * bit[3:0]: venc_idx, bit[7:4]: hdmi_if_idx, other bits for future
	 */
	u32 vid_clk_path;

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

	u32 flag_3dfp:1;
	u32 flag_3dtb:1;
	u32 flag_3dss:1;

	union {
		struct dptx_hw_fmt_para dptx_hw_para;
		struct hdmitx_hw_fmt_para hdmitx_hw_para;
	} tx_hw_para;
};

enum venc_bist_type {
	VENC_BIST_PTTN_OFF = 0,
	VENC_BIST_PTTN_BLACK,
	VENC_BIST_PTTN_WHITE,
	VENC_BIST_PTTN_RED,
	VENC_BIST_PTTN_GREEN,
	VENC_BIST_PTTN_BLUE,
	VENC_BIST_PTTN_LINE,
	VENC_BIST_PTTN_DOT,
	VENC_BIST_PTTN_COLORBAR,
	VENC_BIST_PTTN_CROSSING, /* new adding in S5 */
	VENC_BIST_PTTN_GRAY, /* new adding in A9 */
};

/*
 * BIST: built-in self test video pattern
 *   bist_name: colorbar, red, green, blue, black, white, line, dot, x, gray
 */
struct video_bist_format_para {
	enum venc_bist_type bist_type;
	u8 enc_sel; /* 0: encp; 1: encl */
};

#endif
