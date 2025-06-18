/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __AML_AUDIO_LOCKER_HW_H__
#define __AML_AUDIO_LOCKER_HW_H__
#include <linux/clk.h>

#include "regs.h"
#include "iomap.h"

void audiolocker_enable(struct regmap *regmap, bool enable);

void audiolocker_sw_reset(struct regmap *regmap);

void audiolocker_latch(struct regmap *regmap);

void audiolocker_refclk_downsample_step(struct regmap *regmap, unsigned int step);

void audiolocker_imclk_downsample_step(struct regmap *regmap, unsigned int step);

void audiolocker_omclk_downsample_step(struct regmap *regmap, unsigned int step);

void audiolocker_refclk_latch(struct regmap *regmap, unsigned int num);

void audiolocker_interrupt_mask(struct regmap *regmap, unsigned int mask);

void audiolocker_set_refclk_src(struct regmap *regmap, unsigned int src);

void audiolocker_enable(struct regmap *regmap, bool enable);

#endif
