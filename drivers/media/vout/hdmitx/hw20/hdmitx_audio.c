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

int hdmitx20_set_audio(struct hdmitx_dev *hdev,
		     struct aud_para *audio_param)
{
	int i, ret = -1;
	unsigned char AUD_DB[32];
	unsigned char CHAN_STAT_BUF[24 * 2];

	for (i = 0; i < 32; i++)
		AUD_DB[i] = 0;
	for (i = 0; i < (24 * 2); i++)
		CHAN_STAT_BUF[i] = 0;
	if (hdev->hw_comm.setaudmode(&hdev->hw_comm,
					   audio_param) >= 0) {
		hdev->hwop.setaudioinfoframe(AUD_DB, CHAN_STAT_BUF);
		ret = 0;
	}
	return ret;
}

void hdmitx20_audio_mute_op(unsigned int flag)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	if (hdev->tx_comm.hdmi_init != HDMITX20)
		return;

	hdev->tx_comm.cur_audio_param.aud_output_en = flag;
	if (flag == 0)
		hdmitx_hw_cntl_config(&hdev->hw_comm,
			CONF_AUDIO_MUTE_OP, AUDIO_MUTE);
	else
		hdmitx_hw_cntl_config(&hdev->hw_comm,
			CONF_AUDIO_MUTE_OP, AUDIO_UNMUTE);
}

void hdmitx20_ext_set_audio_output(bool enable)
{
	HDMITX_INFO("audio: enable = %d\n", enable);
	hdmitx20_audio_mute_op(enable);
}

int hdmitx20_ext_get_audio_status(void)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();
	int val;
	static int val_st;

	val = !!(hdmitx_hw_cntl_config(&hdev->hw_comm, CONF_GET_AUDIO_MUTE_ST, 0));
	if (val_st != val) {
		val_st = val;
		HDMITX_INFO("audio: get val = %d\n", val);
	}
	return val;
}
