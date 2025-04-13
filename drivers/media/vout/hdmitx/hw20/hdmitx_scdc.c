// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/delay.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include "hdmitx_ddc.h"
#include "hdmitx_hw.h"

void scdc_config(struct hdmitx_common *tx_comm)
{
	/*
	 * from hdmi2.1/2.0 spec chapter 10.4, prior to accessing
	 * the SCDC, source devices shall verify that the attached
	 * sink Device incorporates a valid HF-VSDB in the E-EDID
	 * in which the SCDC Present bit is set (=1). Source
	 * devices shall not attempt to access the SCDC unless the
	 * SCDC Present bit is set (=1).
	 * For some special TV(bug#164688), it support 6G 4k60hz,
	 * but not declare scdc_present in EDID, so still force to
	 * send 1:40 tmds bit clk ratio when output >3.4Gbps signal
	 * to cover such non-standard TV.
	 */
	/*
	 * if change to > 3.4Gbps mode, or change from > 3.4Gbps
	 * to < 3.4Gbps mode, need to forcely update clk ratio
	 */
	if (tx_comm->fmt_para.tmds_clk_div40)
		scdc_wr_sink(TMDS_CFG, 3);
	else if (tx_comm->rxcap.scdc_present ||
		tx_comm->tx_hw->pre_tmds_clk_div40)
		scdc_wr_sink(TMDS_CFG, 0);
	else
		HDMITX_INFO("ERR: SCDC not present, should not send 1:10\n");
}

/* update CED, 10.4.1.8 */
static int scdc_ced_cnt(struct hdmitx_common *tx_comm)
{
	struct ced_cnt *ced = &tx_comm->ced_cnt;
	u8 raw[7];
	u8 chksum;
	int i;

	memset(raw, 0, sizeof(raw));
	memset(ced, 0, sizeof(struct ced_cnt));

	if (!tx_comm->rxcap.scdc_present)
		HDMITX_DEBUG("ERR: SCDC not present, should not access SCDC\n");
	chksum = 0;
	for (i = 0; i < 7; i++) {
		scdc_rd_sink(ERR_DET_0_L + i, &raw[i]);
		chksum += raw[i];
	}

	ced->ch0_cnt = raw[0] + ((raw[1] & 0x7f) << 8);
	ced->ch0_valid = (raw[1] >> 7) & 0x1;
	ced->ch1_cnt = raw[2] + ((raw[3] & 0x7f) << 8);
	ced->ch1_valid = (raw[3] >> 7) & 0x1;
	ced->ch2_cnt = raw[4] + ((raw[5] & 0x7f) << 8);
	ced->ch2_valid = (raw[5] >> 7) & 0x1;

	/* Do checksum */
	if (chksum != 0)
		HDMITX_ERROR("ced check sum error\n");
	if (ced->ch0_cnt)
		HDMITX_INFO("ced: ch0_cnt = %d %s\n", ced->ch0_cnt,
			ced->ch0_valid ? "" : "invalid");
	if (ced->ch1_cnt)
		HDMITX_INFO("ced: ch1_cnt = %d %s\n", ced->ch1_cnt,
			ced->ch1_valid ? "" : "invalid");
	if (ced->ch2_cnt)
		HDMITX_INFO("ced: ch2_cnt = %d %s\n", ced->ch2_cnt,
			ced->ch2_valid ? "" : "invalid");

	return chksum != 0;
}

/* update scdc status flags, 10.4.1.7 */
/* ignore STATUS_FLAGS_1, all bits are RSVD */
int scdc_status_flags(struct hdmitx_common *tx_comm)
{
	u8 st = 0;
	u8 locked_st = 0;

	if (!tx_comm->rxcap.scdc_present)
		HDMITX_DEBUG("ERR: SCDC not present, should not access SCDC\n");

	scdc_rd_sink(UPDATE_0, &st);
	if (st & STATUS_UPDATE) {
		scdc_rd_sink(STATUS_FLAGS_0, &locked_st);
		tx_comm->chlocked_st.clock_detected = locked_st & (1 << 0);
		tx_comm->chlocked_st.ch0_locked = !!(locked_st & (1 << 1));
		tx_comm->chlocked_st.ch1_locked = !!(locked_st & (1 << 2));
		tx_comm->chlocked_st.ch2_locked = !!(locked_st & (1 << 3));
	}
	if (st & CED_UPDATE)
		scdc_ced_cnt(tx_comm);
	if (st & (STATUS_UPDATE | CED_UPDATE))
		scdc_wr_sink(UPDATE_0, st & (STATUS_UPDATE | CED_UPDATE));
	if (st & STATUS_UPDATE) {
		if (!tx_comm->chlocked_st.clock_detected)
			HDMITX_INFO("ced: clock undetected\n");
		if (!tx_comm->chlocked_st.ch0_locked)
			HDMITX_INFO("ced: ch0 unlocked\n");
		if (!tx_comm->chlocked_st.ch1_locked)
			HDMITX_INFO("ced: ch1 unlocked\n");
		if (!tx_comm->chlocked_st.ch2_locked)
			HDMITX_INFO("ced: ch2 unlocked\n");
	}

	return st & (STATUS_UPDATE | CED_UPDATE);
}
