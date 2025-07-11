// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_dev.h>

#include "meson_tx_internal.h"

int meson_tx_hw_cntl(struct meson_tx_hw *tx_hw, u32 cmd,
		     void *input_argv, void *output_struct)
{
	if (tx_hw->hw_cntl)
		tx_hw->hw_cntl(tx_hw, cmd, input_argv, output_struct);

	return 0;
}

void meson_tx_hw_setup_phy(struct meson_tx_hw *tx_hw,
			   struct meson_tx_phy *tx_phy)
{
	tx_hw->tx_phy = tx_phy;
}
