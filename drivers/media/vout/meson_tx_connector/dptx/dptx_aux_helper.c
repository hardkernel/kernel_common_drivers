// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/component.h>
#include <linux/mutex.h>
#include <linux/delay.h>

#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>
#include "dptx_aux_helper.h"
#include "dpcd_reg.h"
#include "dptx_log.h"
#ifdef __UBOOT__
#include <amlogic/media/vout/meson_tx-connector/meson_tx_edid.h>
#elif __KERNEL__
#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_edid.h>
#endif

static int dptx_dpcd_access(struct dptx_aux *aux, u8 request,
	unsigned int offset, void *buffer, size_t size)
{
	struct dptx_aux_msg msg;
	unsigned int retry, native_reply;
	int err = 0, ret = 0;

	memset(&msg, 0, sizeof(msg));
	msg.address = offset;
	msg.request = request;
	msg.buffer = buffer;
	msg.size = size;

	if (!aux) {
		DPTX_ERROR("%s invalid aux instance\n", __func__);
		return -EINVAL;
	}
	mutex_lock(&aux->hw_mutex);

	for (retry = 0; retry < DPCD_ACCESS_MAX_RETRY; retry++) {
		if (ret != 0 && ret != -ETIMEDOUT) {
			usleep_range(AUX_RETRY_INTERVAL,
				     AUX_RETRY_INTERVAL + 100);
		}

		ret = aux->hw_comm->aux_transfer(aux->hw_comm, &msg);
		if (ret >= 0) {
			native_reply = msg.reply & DP_AUX_NATIVE_REPLY_MASK;
			if (native_reply == DP_AUX_NATIVE_REPLY_ACK) {
				if (ret == size)
					goto unlock;

				ret = -EPROTO;
			} else {
				ret = -EIO;
			}
		}

		/*
		 * We want the error we return to be the error we received on
		 * the first transaction, since we may get a different error the
		 * next time we retry
		 */
		if (!err)
			err = ret;
	}

	ret = err;

unlock:
	mutex_unlock(&aux->hw_mutex);
	return ret;
}

ssize_t dptx_aux_read_dpcd(struct dptx_aux *aux, unsigned int offset,
			 void *buffer, size_t size)
{
	int ret;

	ret = dptx_dpcd_access(aux, DP_AUX_NATIVE_READ, offset,
				   buffer, size);

	return ret;
}

ssize_t dptx_aux_write_dpcd(struct dptx_aux *aux, unsigned int offset,
			  void *buffer, size_t size)
{
	int ret;

	ret = dptx_dpcd_access(aux, DP_AUX_NATIVE_WRITE, offset,
				   buffer, size);

	return ret;
}

static int dptx_read_extended_dpcd_cap(struct dptx_aux *aux,
					  u8 *dpcd, size_t size)
{
	u8 dpcd_ext[DP_RECEIVER_CAP_SIZE];
	int ret;

	if (!(dpcd[DP_TRAINING_AUX_RD_INTERVAL] &
	      DP_EXTENDED_RECEIVER_CAP_FIELD_PRESENT))
		return 0;

	ret = dptx_aux_read_dpcd(aux, DP_DP13_DPCD_REV, &dpcd_ext,
			       sizeof(dpcd_ext));
	if (ret < 0)
		return ret;
	if (ret != sizeof(dpcd_ext))
		return -EIO;

	if (dpcd[DP_DPCD_REV] > dpcd_ext[DP_DPCD_REV])
		return 0;

	if (!memcmp(dpcd, dpcd_ext, sizeof(dpcd_ext)))
		return 0;

	memcpy(dpcd, dpcd_ext, sizeof(dpcd_ext));

	return 0;
}

int dptx_aux_read_dpcd_caps(struct dptx_aux *aux,
			  u8 *dpcd, size_t size)
{
	int ret;

	ret = dptx_aux_read_dpcd(aux, DP_DPCD_REV, dpcd, size);
	if (ret < 0)
		return ret;
	if (ret != DP_RECEIVER_CAP_SIZE || dpcd[DP_DPCD_REV] == 0)
		return -EIO;

	/* The Extended Receiver Capability field  is valid with DPCD r1.4 (and higher).
	 * 02200h ~ 0220Fh match 00000h ~ 0000Fh but except for the following 3 registers
	 * DPCD_REV, MAX_LINK_RATE, DOWN_STREAM_PORT_PRESENT
	 */
	if (dpcd[0] < DP_DPCD_REV_14)
		return ret;
	ret = dptx_read_extended_dpcd_cap(aux, dpcd, DP_RECEIVER_CAP_SIZE);
	if (ret < 0)
		return ret;

	return ret;
}

static int aux_read_edid_single_block(struct dptx_aux *aux, u8 *edid)
{
	int i;
	int ret = 0;

	for (i = 0; i < EDID_LENGTH; i += 16) {
		ret = dptx_dpcd_access(aux, DP_AUX_I2C_MOT | DP_AUX_I2C_READ, DDC_ADDR,
					   edid + i, 16);
		if (ret < 0) {
			DPTX_ERROR("i2c 0x%x 0x%x read mot err\n", DDC_ADDR, i);
			return ret;
		}
	}
	return ret;
}

int dptx_aux_read_edid_data(struct dptx_aux *aux,
			  u8 *_edid, int size)
{
	int ret = 0;
	u8 tmp;
	int blk_no = 1;
	u8 *edid = _edid;
	int i;

	if (!aux || !_edid)
		return -EIO;
	if (size % 128)
		DPTX_INFO("edid buffer length %d should be multiple of 128\n", size);

	ret = dptx_dpcd_access(aux, DP_AUX_I2C_MOT | DP_AUX_I2C_WRITE, DDC_ADDR, &tmp, 1);
	if (ret < 0) {
		DPTX_ERROR("i2c 0x%x write mot err\n", DDC_ADDR);
		return ret;
	}

	// read edid base block
	ret = aux_read_edid_single_block(aux, edid + 0);
	if (ret < 0) {
		DPTX_ERROR("i2c 0x%x read block%d err\n", DDC_ADDR, 0);
		return ret;
	}
	blk_no = edid[126];
	if (blk_no == 0)
		return ret;

	if ((size - EDID_LENGTH) <= 0) {
		DPTX_ERROR("edid buff not enough\n");
		return ret;
	}
	size -= EDID_LENGTH;
	edid += EDID_LENGTH;
	for (i = 0; (i < blk_no) && (size >= ((i + 1) * EDID_LENGTH)); i++) {
		ret = aux_read_edid_single_block(aux, edid + i * 128);
		if (ret < 0) {
			DPTX_ERROR("i2c 0x%x read block%d err\n", DDC_ADDR, i + 1);
			return ret;
		}
	}

	// check the EEODB block numbers
	if (blk_no == 1) {
		if (edid[4] == EXTENSION_EEODB_EXT_TAG && edid[5] == EXTENSION_EEODB_EXT_CODE) {
			blk_no = edid[6];
			if ((size - EDID_LENGTH) <= 0) {
				DPTX_ERROR("edid buff not enough\n");
				return ret;
			}
			size -= EDID_LENGTH;
			edid += EDID_LENGTH;
			for (i = 0; (i < blk_no) && (size >= ((i + 1) * EDID_LENGTH)); i++) {
				ret = aux_read_edid_single_block(aux, edid + i * 128);
				if (ret < 0) {
					DPTX_ERROR("i2c 0x%x read block%d err\n", DDC_ADDR, i + 1);
					return ret;
				}
			}
		}
	}

	// i2c read w/o MOT, dummy read to close the transaction
	ret = dptx_dpcd_access(aux, DP_AUX_I2C_READ, DDC_ADDR, &tmp, 1);
	if (ret < 0) {
		DPTX_ERROR("i2c 0x%x write mot err\n", DDC_ADDR);
		return ret;
	}

	return ret;
}

struct dptx_aux *dptx_aux_init(struct dptx_hw_common *hw_comm)
{
	struct dptx_aux *aux;

	aux = kzalloc(sizeof(*aux), GFP_KERNEL);
	if (!aux) {
		DPTX_ERROR("%s alloc fail\n", __func__);
		return NULL;
	}

	aux->hw_comm = hw_comm;
	mutex_init(&aux->hw_mutex);
	return aux;
}
