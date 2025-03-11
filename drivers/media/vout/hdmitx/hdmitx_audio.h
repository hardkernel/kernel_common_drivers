/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMITX_AUDIO_H
#define __HDMITX_AUDIO_H

#include <linux/amlogic/media/vout/hdmitx_common/hdmitx.h>

struct size_map {
	unsigned int sample_bits;
	enum hdmi_audio_sampsize ss;
};

#define AUDIO_PARA_MAX_NUM       14
struct hdmi_audio_fs_ncts {
	struct {
		u32 tmds_clk;
		unsigned int n; /* 24 bit */
		unsigned int cts; /* 24 bit */
		unsigned int n_30bit; /* 30 bit */
		unsigned int cts_30bit; /* 30bit */
		u32 n_36bit;
		u32 cts_36bit;
		u32 n_48bit;
		u32 cts_48bit;
	} array[AUDIO_PARA_MAX_NUM];
	u32 def_n;
};

struct rate_map_fs {
	unsigned int rate;
	enum hdmi_audio_fs fs;
};

u32 hdmitx_hw_get_audio_n_paras(enum hdmi_audio_fs fs, enum hdmi_color_depth cd, u32 tmds_clk);

#endif
