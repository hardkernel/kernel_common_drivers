// SPDX-License-Identifier: GPL-2.0
/*
 * Audio locker register controls
 *
 * Copyright (C) 2019 Amlogic,inc
 *
 */

#include <linux/delay.h>

#include "locker_hw.h"
#include "../common/iomapres.h"

void audiolocker_enable(struct regmap *regmap, bool enable)
{
	mmio_write(regmap, AUD_LOCK_EN, !!enable);
}

void audiolocker_sw_reset(struct regmap *regmap)
{
	mmio_write(regmap, AUD_LOCK_SW_RESET, 1);
}

void audiolocker_latch(struct regmap *regmap)
{
	mmio_write(regmap, AUD_LOCK_SW_LATCH, 0xf);
	mmio_write(regmap, AUD_LOCK_HW_LATCH, 0xf);
}

void audiolocker_refclk_downsample_step(struct regmap *regmap, unsigned int step)
{
	mmio_write(regmap, AUD_LOCK_REFCLK_DS_INT, step & 0x3ff);
}

void audiolocker_imclk_downsample_step(struct regmap *regmap, unsigned int step)
{
	mmio_write(regmap, AUD_LOCK_IMCLK_DS_INT, step & 0x3ff);
}

void audiolocker_omclk_downsample_step(struct regmap *regmap, unsigned int step)
{
	mmio_write(regmap, AUD_LOCK_OMCLK_DS_INT, step & 0x3ff);
}

void audiolocker_refclk_latch(struct regmap *regmap, unsigned int num)
{
	mmio_write(regmap, AUD_LOCK_REFCLK_LAT_INT, num);
}

void audiolocker_interrupt_mask(struct regmap *regmap, unsigned int mask)
{
	mmio_write(regmap, AUD_LOCK_INT_CTRL, mask & 0xf);
}

void audiolocker_set_refclk_src(struct regmap *regmap, unsigned int src)
{
	mmio_write(regmap, AUD_LOCK_REFCLK_SRC, src);
}
