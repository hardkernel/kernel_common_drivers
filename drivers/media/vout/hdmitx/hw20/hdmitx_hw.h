/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMI_TX20_HW_H
#define __HDMI_TX20_HW_H

#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_hw_common.h>
#include "hdmitx_audio.h"

struct hdmitx20_hw {
	struct hdmitx_hw_common *base;
};

struct hdmitx20_hw *get_hdmitx20_hw_instance(void);
struct hdmitx20_dev *get_hdmitx20_device(void);

unsigned int hdmitx_rd_reg(unsigned int addr);


int hdmitx_hpd_hw_op(enum hpd_op cmd);
int hdmi_set_3d(struct hdmitx20_dev *hdmitx_device, int type,
		unsigned int param);
void hdmitx20_audio_init(struct hdmitx_common *tx_comm);

#endif
