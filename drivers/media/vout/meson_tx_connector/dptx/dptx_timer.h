/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPTX_TIMER_H__
#define __DPTX_TIMER_H__

#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>

void dptx_timer_init(struct dptx_hw_common *tx_hw);
void dptx_timer_uninit(struct dptx_hw_common *tx_hw);
void dptx_timer_start(struct dptx_hw_common *tx_hw, struct timer_config *cfg);
void dptx_timer_stop(struct dptx_hw_common *tx_hw, struct timer_config *cfg);

#endif // __DPTX_TIMER_H__
