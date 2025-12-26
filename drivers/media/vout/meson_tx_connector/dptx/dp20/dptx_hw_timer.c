// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>
#include <linux/amlogic/media/vout/meson_tx_connector/phy/meson_tx_phy_core.h>
#include "dptx_hw.h"
#include "dptx_hw_timer.h"
#include "dptx20_reg.h"

/*
 * There are 4 HW timers for dptx usage
 * Function: get_hw_timer_addr
 * Returns: the address of 4 type timers
 */
static u32 get_hw_timer_addr(enum timer_hw_type type)
{
	switch (type) {
	case HW_GP_TIMER:
		return DPTX20_HOST_TIMER;
	case HW_LPM_TIMER:
		return DPTX20_LPM_TIMER;
	case HW_HDCP_TIMER:
		return DPTX20_HDCP_HOST_TIMER;
	case HW_MST_TIMER:
		return DPTX20_MST_TIMER;
	default:
		return DPTX20_HOST_TIMER;
	}
}

void hw_timer_init(struct dptx_hw_common *tx_hw, enum timer_hw_type type)
{
	if (!tx_hw)
		return;

	dptx20_reg_write(tx_hw, CORE_LEVEL, get_hw_timer_addr(type), 0);
}

u32 hw_timer_get_value(struct dptx_hw_common *tx_hw, enum timer_hw_type type)
{
	u32 val;

	if (!tx_hw)
		return 0;

	val = dptx20_reg_read(tx_hw, CORE_LEVEL, get_hw_timer_addr(type));
	return val & TIMER_MAX_CNT;
}

void hw_timer_set_us(struct dptx_hw_common *tx_hw, enum timer_hw_type type, u32 us)
{
	u32 val;

	if (!tx_hw)
		return;

	if (us > TIMER_MAX_CNT)
		us = TIMER_MAX_CNT;
	val = dptx20_reg_read(tx_hw, CORE_LEVEL, get_hw_timer_addr(type));
	val = (val & TIMER_FLAGS_ALL) | (us & TIMER_MAX_CNT);
	dptx20_reg_write(tx_hw, CORE_LEVEL, get_hw_timer_addr(type), val);
}

void hw_timer_enable(struct dptx_hw_common *tx_hw, enum timer_hw_type type, bool enable)
{
	u32 val;

	if (!tx_hw)
		return;

	val = dptx20_reg_read(tx_hw, CORE_LEVEL, get_hw_timer_addr(type));
	if (enable)
		val |= TIMER_FLAG_ENABLE;
	else
		val &= ~TIMER_FLAG_ENABLE;
	dptx20_reg_write(tx_hw, CORE_LEVEL, get_hw_timer_addr(type), val);
}

void hw_timer_auto_reload(struct dptx_hw_common *tx_hw, enum timer_hw_type type, bool enable)
{
	u32 val;

	if (!tx_hw)
		return;

	val = dptx20_reg_read(tx_hw, CORE_LEVEL, get_hw_timer_addr(type));
	if (enable)
		val |= TIMER_FLAG_AUTO_RELOAD;
	else
		val &= ~TIMER_FLAG_AUTO_RELOAD;
	dptx20_reg_write(tx_hw, CORE_LEVEL, get_hw_timer_addr(type), val);
}

void hw_timer_interrupt(struct dptx_hw_common *tx_hw, enum timer_hw_type type, bool enable)
{
	u32 val;

	if (!tx_hw)
		return;

	val = dptx20_reg_read(tx_hw, CORE_LEVEL, get_hw_timer_addr(type));
	if (enable)
		val |= TIMER_FLAG_INTERRUPT;
	else
		val &= ~TIMER_FLAG_INTERRUPT;
	dptx20_reg_write(tx_hw, CORE_LEVEL, get_hw_timer_addr(type), val);
}

void hw_timer_poll_wait_us(struct dptx_hw_common *tx_hw, enum timer_hw_type type, u32 us)
{
	if (!tx_hw)
		return;

	if (us > TIMER_MAX_CNT)
		us = TIMER_MAX_CNT;
	hw_timer_enable(tx_hw, type, false);
	hw_timer_auto_reload(tx_hw, type, false);
	hw_timer_set_us(tx_hw, type, us);
	hw_timer_enable(tx_hw, type, true);

	while (hw_timer_get_value(tx_hw, type) != 0)
		; /* poll until to 0 */
	hw_timer_enable(tx_hw, type, false);
}
