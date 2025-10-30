/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/control.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <linux/amlogic/pm.h>
#include <linux/amlogic/clk_measure.h>
#include <linux/amlogic/cpu_version.h>
#include <linux/notifier.h>
#include "ddr_mngr.h"
#include "audio_io.h"
#include "../common/iomapres.h"
#include "regs.h"
#include "iomap.h"
#include "audio_bus_regmap.h"

#define BACKGROUND_RESET_CMD  1

int mixer_format_set(int channel, int sample_bit, int fddr_type, int mixer_lane);
int mixer_fifo_reset(void);
int mixer_source_set(int fifo_id);
int mic_mixer_source(int fifo_id);
int mic_mixer_fifo_reset(void);
int mic_mixer_format_set(int sample_bit, int fddr_type, int channel);

int mixer_eq_enable(struct frddr *fr, int enable);
int mixer_demux_sel(enum frddr_dest dst, int enable);
int mixer_coef_set(struct regmap *coef_reg, unsigned int vol);
int mixer_en(int enable);
int mixer_clip_top_en(int  enable);
int mixer_mic_coef_set(struct regmap *coef_reg, unsigned int vol);
int background_mixer_en(int enable);
int mic_mixer_ch_set(int channel);
int background_mixer_ch_set(int lane);
int mixer_lane_source(struct regmap *coef_reg, int lane_a, int lane_b);
int set_mixer_regmap(struct regmap *mixer_reg);
int mixer_samesource_ctrl_write(unsigned int mask, unsigned int value);
int mixer_samesource_ctrl_read(void);
int mixer_register_client(struct notifier_block *nb);
int mixer_notifier_audioclient(unsigned long val, void *v);
