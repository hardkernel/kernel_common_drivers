/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPTX_HW_H__
#define __DPTX_HW_H__

enum hw_io_type {
	TOP_LEVEL,
	CORE_LEVEL,
};

/*
 * There are 4 MST0/1/2/3 group registers, 4 groups are the same function
 * but the base address are different.
 * DPTX20_MST_SRC0: 0x800 ~ 0x9bc
 * DPTX20_MST_SRC1: 0xa00 ~ 0xbbc
 * DPTX20_MST_SRC2: 0xc00 ~ 0xdbc
 * DPTX20_MST_SRC3: 0xe00 ~ 0xfbc
 * Function: calculate_mst_reg
 *   calculate the MST register with video_num under MST mode
 */
u32 dptx20_hw_calc_mst_reg(u8 video_num, u32 reg);

/* aux transmitter status */
enum dptx_aux_status_t {
	/* no status reported (this shouldn't happen) */
	DPTX_AUX_STATUS_NONE,
	/* status is good, operation success */
	DPTX_AUX_STATUS_OK,
	/* status is good, operation in progress */
	DPTX_AUX_STATUS_IN_PROCESS,
	/* general error condition */
	DPTX_AUX_STATUS_ERROR,
	/* reply error condition */
	DPTX_AUX_STATUS_REPLY_TIMEOUT,
	/* ACK reply status condition */
	DPTX_AUX_STATUS_ACK,
	/* NACK reply status condition */
	DPTX_AUX_STATUS_NACK,
	/* DEFER reply status condition */
	DPTX_AUX_STATUS_DEFER,
	/* I2C ACK reply status condition */
	DPTX_AUX_STATUS_ACK_I2C,
	/* I2C NACK reply status condition */
	DPTX_AUX_STATUS_NACK_I2C,
	/* I2C DEFER reply status condition */
	DPTX_AUX_STATUS_DEFER_I2C,
	/* UNKNOWN reply status condition */
	DPTX_AUX_STATUS_UNKNOWN,
	/* operation resulted in a timeout */
	DPTX_AUX_STATUS_TIMEOUT,
	/* never found an appropriate interrupt */
	DPTX_AUX_STATUS_TIMEOUT_INTERRUPT,
	/* hpd is not asserted */
	DPTX_AUX_STATUS_NOT_CONNECTED,
	/* state machine not idle */
	DPTX_AUX_STATUS_NOT_IDLE,
	/* abort transaction, back to idle */
	DPTX_AUX_STATUS_ABORT,
	/* reset state machine, back to idle */
	DPTX_AUX_STATUS_RESET,
	/* unsupported command */
	DPTX_AUX_STATUS_UNSUPPORTED_CMD,
	/* unknown command */
	DPTX_AUX_STATUS_RESERVED_CMD,
};

int dptx20_reg_write(struct dptx_hw_common *tx_hw, u32 io_type, u32 offset, u32 value);
u32 dptx20_reg_read(struct dptx_hw_common *tx_hw, u32 io_type, u32 offset);

ssize_t dptx_aux_xfer(struct dptx_hw_common *tx_comm, struct dptx_aux_msg *msg);
void dptx_isr_disable(struct dptx_hw_common *tx_comm, u32 flags);

void dptx_hw_get_mnvid_clock(struct dptx_hw_common *hw_comm, u8 video_num,
	u32 *pixel_clock, u32 *link_clock);
void dptx_set_data_control(struct dptx_hw_common *hw_comm, u8 video_num,
	struct meson_tx_format_para *para);

#endif
