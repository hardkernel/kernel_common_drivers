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
static u8 dptx_aux_fifo_read(struct dptx_hw_common *tx_comm, u8 *buf)
{
	u8 reply_cnt = 0;
	u8 i = 0;

	/* get the number of bytes received */
	reply_cnt = dptx20_reg_read(tx_comm, CORE_LEVEL, DPTX20_AUX_REPLY_DATA_COUNT);
	for (i = 0; i < reply_cnt; i++)
		*buf++ = dptx20_reg_read(tx_comm, CORE_LEVEL, DPTX20_AUX_REPLY_DATA);

	return reply_cnt;
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
static void dptx_aux_start_transaction(struct dptx_hw_common *tx_comm, u8 cmd, u16 addr, u8 bytes)
{
	u32 cmd_value = 0;
	u8 byte_num = 0;

	if (bytes == 0)
		byte_num = 1;
	else
		byte_num = bytes;

	cmd_value = (cmd << 0x8) & AUX_CMD_MASK;
	cmd_value |= (byte_num - 1) & AUX_BYTE_CT_MASK;

	/* read address only case */
	if (cmd == DP_AUX_I2C_READ ||
		(cmd == (DP_AUX_I2C_READ | DP_AUX_I2C_MOT))) {
		if (bytes == 0)
			cmd_value |= AUX_COMMAND_ADDRESS_ONLY;
	}
	dptx20_reg_write(tx_comm, CORE_LEVEL, DPTX20_AUX_ADDRESS, addr);
	dptx20_reg_write(tx_comm, CORE_LEVEL, DPTX20_AUX_COMMAND, cmd_value);
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
	unsigned long timeout = jiffies + msecs_to_jiffies(tx_comm->tx_aux->aux_timeout_ms);

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
 * 2.return actual written/read bytes if ACK/NACK, otherwise return error code
 */
static ssize_t dptx_aux_cmd_submit(struct dptx_hw_common *tx_comm, u8 cmd, u16 addr,
					u8 *buf, u8 size, u8 *reply)
{
	u32 isr_mask = INTERRUPT_MASK_REPLY_TIMEOUT_EVENT | INTERRUPT_MASK_REPLY_RECEIVED_EVENT;
	u8 aux_reply;
	enum dptx_aux_status_t aux_status = DPTX_AUX_STATUS_NONE;
	ssize_t ret_size = 0;
	u8 fifo_rd_size = 0;
	struct dptx20_hw *tx20_hw = to_dptx20_hw(tx_comm);
	int stat = 0;
	u8 reply_cnt[16] = {0};

	if (tx20_hw->aux_block_mode) {
		dptx_isr_disable(tx_comm, isr_mask);
	} else {
		dptx_isr_enable(tx_comm, isr_mask);
		/* init completion before aux access */
		reinit_completion(&tx20_hw->aux_completion);
	}

	if (cmd == DP_AUX_NATIVE_WRITE ||
		cmd == DP_AUX_I2C_WRITE ||
		cmd == (DP_AUX_I2C_WRITE | DP_AUX_I2C_MOT))
		dptx_aux_fifo_write(tx_comm, buf, size);
	/* write address, command, and count */
	dptx_aux_start_transaction(tx_comm, cmd, addr, size);

	/* wait for response */
	if (tx20_hw->aux_block_mode) {
		aux_status = dptx_aux_wait_response(tx_comm);
	} else {
		stat = wait_for_completion_timeout(&tx20_hw->aux_completion,
			msecs_to_jiffies(tx_comm->tx_aux->aux_timeout_ms));
		if (stat == 0) {
			DPTX_DEBUG("aux wait timeout\n");
			aux_status = DPTX_AUX_STATUS_REPLY_TIMEOUT;
		} else {
			/*
			 * aux reply received/timeout intr received before
			 * completion timeout, then use aux status updated
			 * in interrupt handler
			 */
			aux_status = tx20_hw->aux_status;
		}
	}

	aux_reply = dptx20_reg_read(tx_comm, CORE_LEVEL, DPTX20_AUX_REPLY_CODE) & AUX_REPLY_MASK;
	if (aux_status == DPTX_AUX_STATUS_OK) {
		switch (aux_reply) {
		case AUX_REPLY_CODE_AUX_ACK:
			/* read the data */
			if (cmd == DP_AUX_NATIVE_READ ||
				cmd == DP_AUX_I2C_READ ||
				cmd == (DP_AUX_I2C_READ | DP_AUX_I2C_MOT)) {
				/* ready to reply with data following */
				fifo_rd_size = dptx_aux_fifo_read(tx_comm, buf);
			} else if (cmd == DP_AUX_NATIVE_WRITE) {
				/* all data bytes have been written */
				fifo_rd_size = size;
			} else if ((cmd == DP_AUX_I2C_WRITE) ||
				(cmd == (DP_AUX_I2C_WRITE | DP_AUX_I2C_MOT))) {
				/* For I2C write transactions, I2C ACK shall be
				 * followed by the data byte "M", where "M" is
				 * the number of bytes the DPRX has written to
				 * its I2C slave without NACK. The data byte "M"
				 * shall be omitted when all the data bytes have
				 * been written.
				 */
				fifo_rd_size = dptx_aux_fifo_read(tx_comm, reply_cnt);
				if (fifo_rd_size == 0) {
					/* The data byte "M" shall be omitted when all
					 * the data bytes have been written.
					 */
					fifo_rd_size = size;
				} else if (fifo_rd_size == 1) {
					fifo_rd_size = reply_cnt[0];
					DPTX_INFO("%s I2C write ACK with reply count: %d\n",
						__func__, fifo_rd_size);
				} else {
					DPTX_ERROR("%s I2C write ACK with error reply count: %d\n",
						__func__, fifo_rd_size);
					/* assume all bytes have been written */
					fifo_rd_size = size;
				}
			}
			break;
		case AUX_REPLY_CODE_AUX_NACK:
		case AUX_REPLY_CODE_I2C_NACK:
			/* DP1.4 Spec Table 2-147 */
			/* 1.For write transactions: AUX NACK shall be followed by a
			 * data byte "M", where "M" indicates the number of data
			 * bytes successfully written. When a DPTX is writing a
			 * DPCD address not supported by the DPRX, the DPRX shall
			 * reply with AUX NACK and "M" equal to zero.
			 *
			 * 2.A DPRX receiving a Native AUX read request for an
			 * unsupported DPCD address shall reply with an AUX ACK
			 * and read data set equal to zero instead of replying
			 * with AUX NACK. so do nothing
			 *
			 * 3.For I2C write transactions: I2C NACK shall be followed
			 * by a data byte "M", except when the I2C slave has
			 * NACKed the I2C address, in which case the reply transaction
			 * shall end with I2C NACK without the data byte "M". Data
			 * byte "M" indicates the number of bytes the DPRX has
			 * successfully written to its I2C slave before receiving the
			 * NACK. The byte on which the NACK occurred is excluded
			 * from the number.
			 *
			 * 4.For I2C read transaction: I2C slave NACKed the I2C address,
			 * return error -1;
			 */
			if (cmd == DP_AUX_NATIVE_WRITE ||
				cmd == DP_AUX_I2C_WRITE ||
				(cmd == (DP_AUX_I2C_WRITE | DP_AUX_I2C_MOT))) {
				fifo_rd_size = dptx_aux_fifo_read(tx_comm, reply_cnt);
				if (fifo_rd_size == 0) {
					DPTX_INFO("%s I2C NACK slave addr?\n", __func__);
				} else if (fifo_rd_size == 1) {
					fifo_rd_size = reply_cnt[0];
					DPTX_INFO("%s write NACK with reply count: %d\n",
						__func__, fifo_rd_size);
				} else {
					DPTX_ERROR("%s write NACK with error reply count: %d\n",
						__func__, fifo_rd_size);
					/* assume none bytes written */
					fifo_rd_size = 0;
				}
			} else if ((cmd == DP_AUX_I2C_READ) ||
				(cmd == (DP_AUX_I2C_READ | DP_AUX_I2C_MOT))) {
				fifo_rd_size = -1;
			} else {
				/* should not happen */
				DPTX_INFO("%s NACK should not happen with Native AUX read\n",
					__func__);
				fifo_rd_size = 0;
			}
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

	/* only return actual read/write size if aux status OK or NACK;
	 * return -1 if aux status other than TIMEOUT
	 */
	if (aux_status == DPTX_AUX_STATUS_OK ||
		aux_status == DPTX_AUX_STATUS_NACK)
		ret_size = fifo_rd_size;
	else if (aux_status == DPTX_AUX_STATUS_TIMEOUT ||
		aux_status == DPTX_AUX_STATUS_REPLY_TIMEOUT)
		ret_size = -ETIMEDOUT;
	else
		ret_size = -1;
	*reply = aux_reply;
	dptx20_reg_write(tx_comm, CORE_LEVEL, TR_DPTX_AUX_APB_WR_RD_DONE, 0x1);

	return ret_size;
}

/* return actual accessed bytes */
ssize_t dptx_aux_xfer(struct dptx_hw_common *tx_comm, struct dptx_aux_msg *msg)
{
	ssize_t ret;
	ssize_t accessed_bytes = 0;
	ssize_t tmp_cnt = 0;

	if (!msg) {
		DPTX_ERROR("%s NULL aux msg!\n", __func__);
		return -1;
	}

	do {
		tmp_cnt = 0;
		ssize_t to_access = MIN(DP_AUX_MAX_PAYLOAD_BYTES, msg->size - accessed_bytes);

I2C_CONTINUE:
		ret = dptx_aux_cmd_submit(tx_comm, msg->request, msg->address + accessed_bytes,
			msg->buffer + accessed_bytes, to_access, &msg->reply);
		DPTX_DEBUG("%s to_access_bytes: 0x%x, cur_accessed_bytes: 0x%x\n",
			__func__, to_access, ret);

		if (msg->request == DP_AUX_I2C_READ ||
			(msg->request == (DP_AUX_I2C_READ | DP_AUX_I2C_MOT)) ||
			msg->request == DP_AUX_I2C_WRITE ||
			(msg->request == (DP_AUX_I2C_WRITE | DP_AUX_I2C_MOT))) {
			if (ret > 0) {
				tmp_cnt += ret;
				if (tmp_cnt < to_access) {
					goto I2C_CONTINUE;
				} else if (tmp_cnt > to_access) {
					DPTX_ERROR("%s cmd 0x%x failed, access exceed(%d > %d)\n",
						__func__, msg->request, tmp_cnt, to_access);
					break;
				}
				/* continue next read if tmp_cnt == to_access */
			} else if (ret < 0) {
				DPTX_ERROR("%s failed to do I2C_AUX cmd 0x%x, ret = %d\n",
					__func__, msg->request, ret);
				break;
			}
		} else {
			/* TODO: currently not consider NACK case */
			if (ret != to_access) {
				DPTX_ERROR("%s failed to do AUX cmd 0x%x, ret = %d\n",
					__func__, msg->request, ret);
				break;
			}
		}
		accessed_bytes += to_access;
	} while (accessed_bytes < msg->size);

	return accessed_bytes;
}
