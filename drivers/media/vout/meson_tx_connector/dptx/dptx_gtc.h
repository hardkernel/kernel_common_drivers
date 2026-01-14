/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPTX_GTC_H__
#define __DPTX_GTC_H__

#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>

/*
 * Global Time Code(GTC)
 * There are 2 working modes:
 *   1) use the timer to drive 1ms Lock Acquisition or 10ms Maintenance
 *   2) use the HW GTC Auto mode without the timer
 * Choose 1 as default
 * The HW supports use as a GTC master only. GTC slave operation is not currently supported.
 */
enum gtc_working_mode {
	GTC_TIMER_MODE,
	GTC_HW_AUTO_MODE,
};

/*
 * Init the GTC
 */
int dptx_gtc_init(struct dptx_common *tx_comm);
/*
 * start the GTC in the last step with low priority, after EDID/DPCD/HDCP/MST, ...
 * the GTC may always occupy the AUX CH periodically
 */
int dptx_gtc_start(struct dptx_common *tx_comm);

/*
 * stop the GTC when plug out, suspend
 */
int dptx_gtc_stop(struct dptx_common *tx_comm);

#endif // __DPTX_GTC_H__
