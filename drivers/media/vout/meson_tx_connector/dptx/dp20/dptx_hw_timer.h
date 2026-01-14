/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPTX_HW_TIMER_H__
#define __DPTX_HW_TIMER_H__

enum timer_hw_type {
	HW_GP_TIMER = 0,
	HW_LPM_TIMER = 1, /* used for GTC */
	HW_HDCP_TIMER = 2,
	HW_MST_TIMER = 3,
	HW_TIMER_MAX = 4,
};

#define TIMER_FLAG_ENABLE      BIT(31)
#define TIMER_FLAG_AUTO_RELOAD BIT(30)
#define TIMER_FLAG_INTERRUPT   BIT(29)
/* The count unit is us, and the MAX time is 2^24us, nearly 16.048s */
#define TIMER_MAX_CNT          GENMASK(23, 0)
#define TIMER_FLAGS_ALL        (TIMER_FLAG_ENABLE | TIMER_FLAG_AUTO_RELOAD | TIMER_FLAG_INTERRUPT)

void hw_timer_init(struct dptx_hw_common *tx_hw, enum timer_hw_type type);
u32 hw_timer_get_value(struct dptx_hw_common *tx_hw, enum timer_hw_type type);
void hw_timer_set_us(struct dptx_hw_common *tx_hw, enum timer_hw_type type, u32 us);
void hw_timer_enable(struct dptx_hw_common *tx_hw, enum timer_hw_type type, bool enable);
void hw_timer_auto_reload(struct dptx_hw_common *tx_hw, enum timer_hw_type type, bool enable);
void hw_timer_interrupt(struct dptx_hw_common *tx_hw, enum timer_hw_type type, bool enable);
void hw_timer_poll_wait_us(struct dptx_hw_common *tx_hw, enum timer_hw_type type, u32 us);

#endif // __DPTX_HW_TIMER_H__
