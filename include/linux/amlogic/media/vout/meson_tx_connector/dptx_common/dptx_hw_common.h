/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPTX_HW_COMMON_H
#define __DPTX_HW_COMMON_H

#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_hw.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>

struct dptx_aux;
struct dptx_aux_msg;

struct dptx_hw_common {
	struct meson_tx_hw hw_base;

	/* irq id of dptx */
	u32 irq_id;
	/* instance index */
	u32 dev_idx;

	/* core ID/revision */
	u32 core_info;
	/* transfers a message representing a single AUX transaction.
	 * Upon success, the implementation should return the number
	 * of payload bytes that were transferred, or a negative
	 * error-code on failure.
	 *
	 * The function must only modify the reply field of
	 * the dptx_aux_msg structure.
	 */
	ssize_t (*aux_transfer)(struct dptx_hw_common *tx_comm, struct dptx_aux_msg *msg);
};

#define to_dptx_hw_common(x)	container_of(x, struct dptx_hw_common, hw_base)

int dptx_hw_cntl(struct meson_tx_hw *tx_hw, u32 cmd,
	void *input_argv, void *output_struct);

/***** dptx20 hw api*****/
struct meson_tx_hw *dptx20_alloc_tx_hw(void);
void dptx20_free_tx_hw(struct meson_tx_hw *tx_hw);

#endif
