// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>
#include "dptx_log.h"
#include "dptx_timer.h"
#include "dptx_hw_opcode.h"

void dptx_timer_init(struct dptx_hw_common *tx_hw)
{
	if (!tx_hw)
		return;

	dptx_hw_cntl(&tx_hw->hw_base, DP_TIMER_INIT, NULL, NULL);
}

void dptx_timer_start(struct dptx_hw_common *tx_hw, struct timer_config *cfg)
{
	int ret = 0;
	struct dptx_timer_handler *handlers = NULL;

	if (!tx_hw || !cfg)
		return;

	ret = dptx_hw_cntl(&tx_hw->hw_base, DP_TIMER_GET, &cfg->timer_type, &handlers);
	if (ret < 0) {
		DPTX_INFO("%s %d failed\n", __func__, cfg->timer_type);
		return;
	}
	handlers->cfg = *cfg;
	mutex_lock(&handlers->timer_mutex);
	dptx_hw_cntl(&tx_hw->hw_base, DP_TIMER_START, handlers, NULL);
	mutex_unlock(&handlers->timer_mutex);
}

void dptx_timer_stop(struct dptx_hw_common *tx_hw, struct timer_config *cfg)
{
	int ret = 0;
	struct dptx_timer_handler *handlers = NULL;

	if (!tx_hw || !cfg)
		return;

	ret = dptx_hw_cntl(&tx_hw->hw_base, DP_TIMER_GET, &cfg->timer_type, &handlers);
	if (ret < 0) {
		DPTX_INFO("%s %d failed\n", __func__, cfg->timer_type);
		return;
	}
	mutex_lock(&handlers->timer_mutex);
	dptx_hw_cntl(&tx_hw->hw_base, DP_TIMER_STOP, handlers, NULL);
	mutex_unlock(&handlers->timer_mutex);
}
