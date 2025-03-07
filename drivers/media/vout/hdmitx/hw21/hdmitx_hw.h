/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMI_TX21_HW_H
#define __HDMI_TX21_HW_H

#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_hw_common.h>
#include "hdmitx_audio.h"

struct hdmitx21_hw {
	struct hdmitx_hw_common *base;
	u32 enc_idx;
	struct hdmitx_infoframe *infoframes;
	/*
	 * For use with s7 and later chips, default 0
	 * 1: new clk config, encp/pixel clk is directly configured by the pll simulation part.
	 * through [ 49]hdmi_vx1_pix_clk to encp/pixel clk
	 * CLKCTRL_VID_CLK0_CTRL clk source should select vid_pix_clk.
	 */
	u8 clk_config;
	int gate_bit_mask; /* for ctrl phy/pll/clk */
};

struct hdmitx21_hw *get_hdmitx21_hw_instance(void);
#define to_hdmitx21_hw(x)	container_of(x, struct hdmitx21_hw, base)

#endif
