/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _MESON_TX_CLK_INTERNAL_H
#define _MESON_TX_CLK_INTERNAL_H

#include <linux/types.h>
#include <linux/amlogic/media/vout/meson_tx_connector/clk/meson_tx_clk.h>

int tx_bulk_clk_set_a9(struct meson_tx_clk *tx_clk, u32 clk_mask);
int tx_bulk_clk_disable_a9(struct meson_tx_clk *tx_clk, u32 clk_mask);
u32 tx_clk_read_reg(void __iomem *base, u32 addr_offset);
void tx_clk_write_reg(void __iomem *base, u32 addr, u32 val);
void tx_clk_set_reg_bits(void __iomem *base, u32 addr, u32 value, u32 offset, u32 len);

#endif

