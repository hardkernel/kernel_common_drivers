// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/media/sound/aout_notify.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>
#include "dptx_hw_opcode.h"
#include <dptx_log.h>
#include "dptx_gtc.h"

int dptx_gtc_init(struct dptx_common *tx_comm)
{
	if (!tx_comm)
		return -1;

	dptx_hw_cntl(tx_comm->base.tx_hw_base, DP_GTC_INIT, NULL, NULL);
	return 0;
}

int dptx_gtc_start(struct dptx_common *tx_comm)
{
	enum gtc_working_mode mode = GTC_TIMER_MODE;

	if (!tx_comm)
		return -1;

	return dptx_hw_cntl(tx_comm->base.tx_hw_base, DP_GTC_START, (void *)&mode, NULL);
}

int dptx_gtc_stop(struct dptx_common *tx_comm)
{
	if (!tx_comm)
		return -1;

	return dptx_hw_cntl(tx_comm->base.tx_hw_base, DP_GTC_STOP, NULL, NULL);
}
