/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPTX_HW_GTC_H__
#define __DPTX_HW_GTC_H__

#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>
#include "dptx_gtc.h"

int dptx_hw_gtc_init(struct dptx_hw_common *hw_comm);

int dptx_hw_gtc_start(struct dptx_hw_common *hw_comm, enum gtc_working_mode mode);

int dptx_hw_gtc_stop(struct dptx_hw_common *hw_comm);

#endif // __DPTX_HW_GTC_H__
