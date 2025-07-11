// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>

/* param:
 * cmd: specific HW operation;
 * input_argv: input param for specific command, maybe a struct pointer;
 * output_struct: return structure for specific command;
 *
 * return: for simple integer value return; if need to return
 * a structure instead of a simple integer value, use output_struct.
 */
int dptx_hw_cntl(struct meson_tx_hw *tx_hw, u32 cmd,
	void *input_argv, void *output_struct)
{
	if (!tx_hw || !tx_hw->hw_cntl)
		return -EINVAL;
	return tx_hw->hw_cntl(tx_hw, cmd, input_argv, output_struct);
}
