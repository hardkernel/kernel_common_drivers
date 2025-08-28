// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>

#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>
#include <linux/amlogic/media/vout/meson_tx_connector/phy/meson_tx_phy_core.h>

#include "meson_tx_task_mgr.h"
#include "dptx_log.h"
#include <dpcd_reg.h>
#include <dptx_aux_helper.h>
#include "dptx_hw.h"
#include "dptx20_reg.h"
#include "dptx_hw_opcode.h"
#include "dptx_infoframe.h"

/* For each MSTn, there are 7 SDP buffers for the Infoframe packets */
#define BUFFER_MAX_NUM             7

static void sdp_info_buffer_write(struct dptx_hw_common *hw_comm, u8 vc_id, u8 sel, u8 *data)
{
	int i;
	unsigned long flags;
	u32 reg;
	u32 val;

	if (!hw_comm || sel >= BUFFER_MAX_NUM)
		return;

	spin_lock_irqsave(&hw_comm->sdp_lock, flags);
	reg = dptx20_hw_calc_mst_reg(vc_id, DPTX20_SECX_INFOFRAME_SELECT);
	dptx20_reg_write(hw_comm, CORE_LEVEL, reg, sel); // buf selection
	reg = dptx20_hw_calc_mst_reg(vc_id, DPTX20_SECX_DATA_PACKET_ID);
	dptx20_reg_write(hw_comm, CORE_LEVEL, reg, vc_id); // vc_id assign to packet id
	if (!data) {
		reg = dptx20_hw_calc_mst_reg(vc_id, DPTX20_SECX_INFOFRAME_RATE);
		val = dptx20_reg_read(hw_comm, CORE_LEVEL, reg);
		val &= ~(1 << sel);
		dptx20_reg_write(hw_comm, CORE_LEVEL, reg, val);
		reg = dptx20_hw_calc_mst_reg(vc_id, DPTX20_SECX_INFOFRAME_ENABLE);
		val = dptx20_reg_read(hw_comm, CORE_LEVEL, reg);
		val &= ~(1 << sel);
		dptx20_reg_write(hw_comm, CORE_LEVEL, reg, val);

		for (i = 0; i < DP_INFOFRAME_TOTAL_SZ; i++) {
			reg = dptx20_hw_calc_mst_reg(vc_id, DPTX20_SECX_INFOFRAME_DATA);
			dptx20_reg_write(hw_comm, CORE_LEVEL, reg, 0);
		}

		spin_unlock_irqrestore(&hw_comm->sdp_lock, flags);
		return;
	}

	for (i = 0; i < DP_INFOFRAME_TOTAL_SZ; i++) {
		reg = dptx20_hw_calc_mst_reg(vc_id, DPTX20_SECX_INFOFRAME_DATA);
		dptx20_reg_write(hw_comm, CORE_LEVEL, reg, data[i]);
	}
	reg = dptx20_hw_calc_mst_reg(vc_id, DPTX20_SECX_INFOFRAME_RATE);
	val = dptx20_reg_read(hw_comm, CORE_LEVEL, reg);
	val |= 1 << sel;
	dptx20_reg_write(hw_comm, CORE_LEVEL, reg, val);
	reg = dptx20_hw_calc_mst_reg(vc_id, DPTX20_SECX_INFOFRAME_ENABLE);
	val = dptx20_reg_read(hw_comm, CORE_LEVEL, reg);
	val |= 1 << sel;
	dptx20_reg_write(hw_comm, CORE_LEVEL, reg, val);
	spin_unlock_irqrestore(&hw_comm->sdp_lock, flags);
}

static int sdp_info_buffer_read(struct dptx_hw_common *hw_comm, u8 vc_id, u8 sel, u8 *data)
{
	int i;
	unsigned long flags;
	u32 reg;

	if (!hw_comm || !data)
		return -1;

	spin_lock_irqsave(&hw_comm->sdp_lock, flags);
	reg = dptx20_hw_calc_mst_reg(vc_id, DPTX20_SECX_INFOFRAME_SELECT);
	dptx20_reg_write(hw_comm, CORE_LEVEL, reg, sel); // buf selection
	for (i = 0; i < DP_INFOFRAME_TOTAL_SZ; i++) {
		reg = dptx20_hw_calc_mst_reg(vc_id, DPTX20_SECX_INFOFRAME_SELECT) + i;
		data[i] = dptx20_reg_read(hw_comm, CORE_LEVEL, reg);
	}
	spin_unlock_irqrestore(&hw_comm->sdp_lock, flags);
	return 31; /* fixed value */
}

static int sdp_infoframe_wrrd(struct dptx_hw_common *hw_comm, u8 vc_id, u8 wr,
	enum infoframe_type info_type, u8 *body)
{
	u8 sel;

	if (info_type < INFOFRAME_TYPE_VENDOR || info_type > INFOFRAME_TYPE_DRM) {
		DPTX_ERROR("wrong infoframe type 0x%x\n", info_type);
		return -1;
	}

	/* the bank index is fixed */
	sel = info_type & 0x7;
	sel--; /* the index is 0 ~ 6, not 1 ~ 7 */

	if (wr) {
		sdp_info_buffer_write(hw_comm, vc_id, sel, body);
		return 0;
	} else {
		return sdp_info_buffer_read(hw_comm, vc_id, sel, body);
	}
}

void dptx_hw_infoframe_send(struct dptx_hw_common *hw_comm, u8 vc_id, u16 info_type, u8 *body)
{
	sdp_infoframe_wrrd(hw_comm, vc_id, 1, info_type, body);
}

int dptx_hw_infoframe_raw_get(struct dptx_hw_common *hw_comm, u8 vc_id, u16 info_type, u8 *body)
{
	return sdp_infoframe_wrrd(hw_comm, vc_id, 0, info_type, body);
}

void dptx_dump_infoframe_packets(struct dptx_hw_common *hw_comm, u8 vc_id, struct seq_file *s)
{
	int i, j;
	u8 body[DP_INFOFRAME_TOTAL_SZ] = {0};

	for (i = 0; i < BUFFER_MAX_NUM; i++) {
		memset(body, 0, sizeof(body));
		sdp_info_buffer_read(hw_comm, vc_id, i, body);
		seq_printf(s, "dump dp[%d] infoframe[%d]\n", vc_id, i);
		for (j = 0; j < DP_INFOFRAME_TOTAL_SZ; j += 9)
			seq_printf(s, "%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
				body[j + 0], body[j + 1], body[j + 2],
				body[j + 3], body[j + 4], body[j + 5],
				body[j + 6], body[j + 7], body[j + 8]);
	}
}
