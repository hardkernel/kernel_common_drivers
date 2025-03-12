// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/major.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/cdev.h>

#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include "hdmitx_module.h"
#include "hdmitx_common.h"

void hdmitx20_audio_mute_op(unsigned int flag)
{
	struct hdmitx20_dev *hdev = get_hdmitx20_device();
	struct hdmitx_hw_common *hw_comm = &hdev->hw_comm;

	if (hdev->tx_comm.hdmi_init != HDMITX20)
		return;

	hdev->tx_comm.cur_audio_param.aud_output_en = flag;
	if (flag == 0)
		hdmitx_hw_cntl_config(hw_comm,
			CONF_AUDIO_MUTE_OP, AUDIO_MUTE);
	else
		hdmitx_hw_cntl_config(hw_comm,
			CONF_AUDIO_MUTE_OP, AUDIO_UNMUTE);
}

void hdmitx20_ext_set_audio_output(bool enable)
{
	HDMITX_INFO("audio: enable = %d\n", enable);
	hdmitx20_audio_mute_op(enable);
}

int hdmitx20_ext_get_audio_status(void)
{
	struct hdmitx20_dev *hdev = get_hdmitx20_device();
	struct hdmitx_hw_common *hw_comm = &hdev->hw_comm;
	int val;
	static int val_st;

	val = !!(hdmitx_hw_cntl_config(hw_comm, CONF_GET_AUDIO_MUTE_ST, 0));
	if (val_st != val) {
		val_st = val;
		HDMITX_INFO("audio: get val = %d\n", val);
	}
	return val;
}

void hdmitx20_audio_init(struct hdmitx_common *tx_comm)
{
	bool audio_en;

	audio_en = hdmitx_uboot_audio_en();

#if IS_ENABLED(CONFIG_AMLOGIC_SND_SOC)
	if (!tx_comm->pxp_mode && audio_en) {
		struct aud_para *audpara = &tx_comm->cur_audio_param;

		audpara->rate = FS_48K;
		audpara->type = CT_PCM;
		audpara->size = SS_16BITS;
		audpara->chs = 2 - 1;
	}
	/* default audio clock is ON */
	hdmitx20_audio_mute_op(1);
#endif
	hdmitx_audio_register_ctrl_callback(tx_comm->tx_tracer,
		hdmitx20_ext_set_audio_output,
		hdmitx20_ext_get_audio_status);
}
