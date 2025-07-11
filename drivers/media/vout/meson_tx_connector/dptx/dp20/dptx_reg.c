// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>
#include "dptx_log.h"
#include "dptx_hw.h"
#include "dptx20_reg.h"

int dptx20_reg_write(struct dptx_hw_common *tx_hw, u32 io_type, u32 offset, u32 value)
{
	DPTX_INFO("TODO %s\n", __func__);
	return 0;
}

u32 dptx20_reg_read(struct dptx_hw_common *tx_hw, u32 io_type, u32 offset)
{
	DPTX_INFO("TODO %s\n", __func__);
	return 0;
}
