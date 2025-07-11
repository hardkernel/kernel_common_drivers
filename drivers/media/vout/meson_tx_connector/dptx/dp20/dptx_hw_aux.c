// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>
#include "meson_tx_task_mgr.h"
#include "dptx_log.h"
#include <dpcd_reg.h>
#include <dptx_aux_helper.h>
#include "dptx_hw.h"
#include "dptx20_reg.h"

/*
 * Function: dptx_aux_fifo_read
 *
 *  Reads up to 16-bytes from the aux-channel read-fifo
 */
static void dptx_aux_fifo_read(struct dptx_hw_common *tx_comm, u8 *buf)
{
	u32 reply_cnt = 0;
	u8 i = 0;

	/* get the number of bytes received */
	reply_cnt = dptx20_reg_read(tx_comm, CORE_LEVEL, DPTX20_AUX_REPLY_DATA_COUNT);
	for (i = 0; i < reply_cnt; i++)
		*buf++ = dptx20_reg_read(tx_comm, CORE_LEVEL, DPTX20_AUX_REPLY_DATA);
}

/*
 * Function: dptx_aux_fifo_write
 *
 * Writes up to 16-bytes to the aux-channel write-fifo
 */
static void dptx_aux_fifo_write(struct dptx_hw_common *tx_comm, u8 *buf, u8 size)
{
	u8 i = 0;

	for (i = 0; i < size; i++)
		dptx20_reg_write(tx_comm, CORE_LEVEL, DPTX20_AUX_WRITE_FIFO, *buf++);
}

/*
 * Function: dptx_aux_start_transaction
 *
 * Start aux channel transaction
 */
static void dptx_aux_start_transaction(struct dptx_hw_common *tx_comm, u32 cmd, u16 addr, u8 bytes)
{
	cmd &= AUX_CMD_MASK;
	cmd |= (bytes - 1) & AUX_BYTE_CT_MASK;

	dptx20_reg_write(tx_comm, CORE_LEVEL, DPTX20_AUX_ADDRESS, addr);
	dptx20_reg_write(tx_comm, CORE_LEVEL, DPTX20_AUX_COMMAND, cmd);
}

/*
 * Function: dptx_aux_wait_response
 *
 * Wait for the read transaction to complete or time out.
 */
static enum dptx_aux_status_t dptx_aux_wait_response(struct dptx_hw_common *tx_comm)
{
	u32 int_state = 0;
	u32 aux_status = 0;
	u32 ret_stauts = DPTX_AUX_STATUS_REPLY_TIMEOUT;
	unsigned long timeout = jiffies + msecs_to_jiffies(1);

	/* loop for about 1ms */
	while (time_before(jiffies, timeout)) {
		int_state = dptx20_reg_read(tx_comm, CORE_LEVEL, DPTX20_INTERRUPT_STATE);
		if (int_state & INTERRUPT_STATE_REPLY_TIMEOUT) {
			ret_stauts = DPTX_AUX_STATUS_TIMEOUT;
			break;
		}
		if (int_state & INTERRUPT_STATE_REPLY_RECEIVED) {
			aux_status = dptx20_reg_read(tx_comm, CORE_LEVEL, DPTX20_AUX_STATUS);
			if (aux_status & AUX_STATUS_REPLY_RECEIVED)
				ret_stauts = DPTX_AUX_STATUS_OK;
			if (aux_status & AUX_STATUS_REPLY_ERROR)
				ret_stauts = DPTX_AUX_STATUS_ERROR;
			break;
		}
	}
	/* clear all interrupts */
	dptx20_reg_read(tx_comm, CORE_LEVEL, DPTX20_INTERRUPT_CAUSE);
	return ret_stauts;
}

/*
 * Function: dptx_aux_cmd_submit
 *
 * Read/Write up to 16 bytes in a single transaction.
 * Note:
 * 1.The cmd field must be set before this call.
 * This function knows how to read a block whether it's a native aux
 * channel command or an i2c-over-aux command.
 * 2.it's read/write in block mode with aux interrupts disabled
 */
static ssize_t dptx_aux_cmd_submit(struct dptx_hw_common *tx_comm, u32 cmd, u16 addr,
					u8 *buf, u8 size, u8 *reply)
{
	u32 isr_mask = 0;
	u8 aux_reply;
	enum dptx_aux_status_t aux_status = DPTX_AUX_STATUS_NONE;
	ssize_t ret_size = 0;

	isr_mask |= INTERRUPT_MASK_REPLY_TIMEOUT_MASK;
	isr_mask |= INTERRUPT_MASK_REPLY_RECEIVED_MASK;
	dptx_isr_disable(tx_comm, isr_mask);

	if ((cmd & DP_AUX_NATIVE_WRITE) ||
		(cmd & DP_AUX_I2C_WRITE))
		dptx_aux_fifo_write(tx_comm, buf, size);
	/* write address, command, and count */
	dptx_aux_start_transaction(tx_comm, cmd, addr, size);
	/* wait for response */
	aux_status = dptx_aux_wait_response(tx_comm);
	aux_reply = dptx20_reg_read(tx_comm, CORE_LEVEL, DPTX20_AUX_REPLY_CODE) & AUX_REPLY_MASK;
	if (aux_status == DPTX_AUX_STATUS_OK) {
		switch (aux_reply) {
		case AUX_REPLY_CODE_AUX_ACK:
			/* read the data
			 * TO CONFIRM: whether to return the reply count in FIFO?
			 */
			if ((cmd & DP_AUX_NATIVE_READ) ||
				(cmd & DP_AUX_I2C_READ))
				dptx_aux_fifo_read(tx_comm, buf);
			break;
		case AUX_REPLY_CODE_AUX_NACK:
		case AUX_REPLY_CODE_I2C_NACK:
			aux_status = DPTX_AUX_STATUS_NACK;
			break;
		case AUX_REPLY_CODE_AUX_DEFER:
		case AUX_REPLY_CODE_I2C_DEFER:
			aux_status = DPTX_AUX_STATUS_DEFER;
			break;
		default:
			aux_status = DPTX_AUX_STATUS_UNKNOWN;
			break;
		}
	}

	/* only return actual read/write size if aux status OK;
	 * return -1 if aux status other than TIMEOUT
	 */
	if (aux_status == DPTX_AUX_STATUS_OK)
		ret_size = size;
	else if (aux_status == DPTX_AUX_STATUS_TIMEOUT ||
		aux_status == DPTX_AUX_STATUS_REPLY_TIMEOUT)
		ret_size = -ETIMEDOUT;
	else
		ret_size = -1;
	*reply = aux_reply;

	return ret_size;
}

ssize_t dptx_aux_xfer(struct dptx_hw_common *tx_comm, struct dptx_aux_msg *msg)
{
	ssize_t ret;
	u32 i;
	u32 iter = 1; // TODO

	for (i = 0; i < iter; i++) {
		ret = dptx_aux_cmd_submit(tx_comm, msg->request, msg->address,
						   msg->buffer, msg->size,
						   &msg->reply);
	}
	return ret;
}
