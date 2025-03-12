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

static DEFINE_MUTEX(aud_mute_mutex);
void hdmitx21_audio_mute_op(u32 flag, unsigned int path)
{
	static unsigned int aud_mute_path;
	struct hdmitx21_dev *hdev = get_hdmitx21_device();

	mutex_lock(&aud_mute_mutex);
	if (flag == 0)
		aud_mute_path |= path;
	else
		aud_mute_path &= ~path;
	hdev->tx_comm.cur_audio_param.aud_output_en = !aud_mute_path;

	if (flag == 0) {
		HDMITX_INFO("audio: AUD_MUTE path=0x%x\n", path);
		hdmitx_hw_cntl_config(&hdev->hw_comm, CONF_AUDIO_MUTE_OP, AUDIO_MUTE);
	} else {
		/* unmute only if none of the paths are muted */
		if (aud_mute_path == 0) {
			HDMITX_INFO("audio: AUD_UNMUTE path=0x%x\n", path);
			hdmitx_hw_cntl_config(&hdev->hw_comm, CONF_AUDIO_MUTE_OP, AUDIO_UNMUTE);
		}
	}
	mutex_unlock(&aud_mute_mutex);
}

void hdmitx21_ext_set_audio_output(bool enable)
{
	hdmitx21_audio_mute_op(enable, AUDIO_MUTE_PATH_1);
	HDMITX_INFO("audio: enable:%d\n", enable);
}

int hdmitx21_ext_get_audio_status(void)
{
	struct hdmitx21_dev *hdev = get_hdmitx21_device();

	return !!hdev->tx_comm.cur_audio_param.aud_output_en;
}

void hdmitx21_audio_init(struct hdmitx_common *tx_comm)
{
	bool audio_en;

	audio_en = hdmitx21_uboot_audio_en();
#if IS_ENABLED(CONFIG_AMLOGIC_SND_SOC)
	if (!tx_comm->pxp_mode && audio_en) {
		struct aud_para *audpara = &tx_comm->cur_audio_param;

		audpara->rate = FS_48K;
		audpara->type = CT_PCM;
		audpara->size = SS_16BITS;
		audpara->chs = 2 - 1;
	}
	/* default audio clock is ON */
	hdmitx21_audio_mute_op(1, 0);
#endif
	hdmitx_audio_register_ctrl_callback(tx_comm->tx_tracer,
			hdmitx21_ext_set_audio_output,
			hdmitx21_ext_get_audio_status);
}
