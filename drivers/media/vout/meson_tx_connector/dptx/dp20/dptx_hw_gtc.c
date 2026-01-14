// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/media/sound/aout_notify.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>
#include "dptx_hw_opcode.h"
#include <dptx_log.h>
#include "dptx_hw.h"
#include "dptx_log.h"
#include "dptx20_reg.h"
#include "dptx20_platform_reg.h"
#include "dptx_hw_timer.h"
#include "dptx_hw_gtc.h"
#include "dptx_timer.h"
#include "dptx_aux_helper.h"
#include "dpcd_reg.h"

struct gtc_tx_rx_gtc_val {
	u32 tx_gtc;
	u32 rx_gtc;
};

static void gtc_phase_val_init(void);
static void gtc_phase_val_save(struct gtc_tx_rx_gtc_val *val);
static u16 gtc_phase_val_calc_avg(void);

/* Upon AUX_NACK or retry, the DPTX shall retry up to three times with the GTC value */
#define TEST_RX_GTC_TIMES 3

/* 10 Native AUX write transactions to the TX_GTC_VALUE */
#define LOCK_ACQ_TIMES 10

/*
 * DPTX calculates the average of the signed delta value. The phase offset is half
 * of the average of the delta. A minimum of four reads should be requested
 */
#define PHASE_SKEW_OFF_CALC_TIMES 6

int dptx_hw_gtc_init(struct dptx_hw_common *hw_comm)
{
	if (!hw_comm)
		return -1;

	/* APB clock is fixed 200MHz, so the COUNT_INT is 1/200MHz = 5ns, w/o any COUNT_FRAC */
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_SECX_GTC_COUNT_CONFIG, (0x0005 << 16) | 0);
	return 0;
}

static void gtc_set_tx_rtc_cap(struct dptx_hw_common *hw_comm)
{
	u8 temp = 0;

	if (!hw_comm)
		return;

	/* Setting TX_GTC_CAP in DP_LINK_RATE_SET */
	dptx_aux_read_dpcd(hw_comm->tx_aux, DP_TX_GTC_CAPABILITY, &temp, sizeof(temp));
	temp |= TX_GTC_CAP;
	temp &= ~TX_GTC_SLAVE_CAP;
	dptx_aux_write_dpcd(hw_comm->tx_aux, DP_TX_GTC_CAPABILITY, &temp, sizeof(temp));
}

static int gtc_tx_burst_write_gtc(struct dptx_hw_common *hw_comm)
{
	u32 gtc_val;
	u8 temp[4];

	if (!hw_comm)
		return -1;

	gtc_val = dptx20_reg_read(hw_comm, CORE_LEVEL, DPTX20_SECX_GTC_COMMAND_EDGE);
	temp[0] = (gtc_val >> 0) & 0xff;
	temp[1] = (gtc_val >> 8) & 0xff;
	temp[2] = (gtc_val >> 16) & 0xff;
	temp[3] = (gtc_val >> 24) & 0xff;
	/* burst write tx GTC value */
	/* HW will monitor DP_TX_GTC_VALUE0 address, and automatically load the GTC value */
	return dptx_aux_write_dpcd(hw_comm->tx_aux, DP_TX_GTC_VALUE0, temp, sizeof(temp));
}

static int gtc_read_rx_gtc_and_is_lock_done(struct dptx_hw_common *hw_comm)
{
	u8 temp[6] = {0};

	if (!hw_comm)
		return -1;
	dptx_aux_read_dpcd(hw_comm->tx_aux, DP_RX_GTC_VALUE_0, &temp, sizeof(temp));

	return temp[5] & BIT(0);
}

static u32 gtc_record_read_rx_gtc_and_is_lock_done(struct dptx_hw_common *hw_comm)
{
	u32 ret;
	u8 temp[6] = {0};

	if (!hw_comm)
		return -1;
	dptx_aux_read_dpcd(hw_comm->tx_aux, DP_RX_GTC_VALUE_0, &temp, sizeof(temp));

	ret = temp[0] + (temp[1] << 8) + (temp[2] << 16) + (temp[3] << 24);
	return ret;
}

static void gtc_tx_timer_rpt_write_cb(void *argv)
{
	struct dptx_hw_common *hw_comm = (struct dptx_hw_common *)argv;

	gtc_tx_burst_write_gtc(hw_comm);
}

static void gtc_tx_lock_acquisition_phase(struct dptx_hw_common *hw_comm)
{
	struct timer_config lock_acq_tmr = {
		.timer_type = HW_LPM_TIMER,
		.wait_type = TIMER_REPEAT_ISR,
		.us = 1000,
		.repeater_times = LOCK_ACQ_TIMES,
		.callback = gtc_tx_timer_rpt_write_cb,
		.cb_data = hw_comm,
	};

	if (!hw_comm)
		return;

	dptx_timer_start(hw_comm, &lock_acq_tmr);
}

static int gtc_tx_phase_skew_offset(struct dptx_hw_common *hw_comm)
{
	int i;
	int ret;

	if (!hw_comm)
		return -1;

	for (i = 0; i < TEST_RX_GTC_TIMES; i++) {
		ret = gtc_read_rx_gtc_and_is_lock_done(hw_comm);
		if (ret == 1) /* rx is lock done */
			break;
	}

	return 0;
}

static void wait_us_for_sync(struct dptx_hw_common *hw_comm, u32 us)
{
	struct timer_config lock_acq_tmr = {
		.timer_type = HW_LPM_TIMER,
		.wait_type = TIMER_WAIT,
		.us = us,
	};

	if (!hw_comm)
		return;

	dptx_timer_start(hw_comm, &lock_acq_tmr);
}

static u32 gtc_tx_record_burst_write_gtc(struct dptx_hw_common *hw_comm)
{
	u32 gtc_val;
	u8 temp[4];

	if (!hw_comm)
		return -1;

	gtc_val = dptx20_reg_read(hw_comm, CORE_LEVEL, DPTX20_SECX_GTC_COMMAND_EDGE);
	temp[0] = (gtc_val >> 0) & 0xff;
	temp[1] = (gtc_val >> 8) & 0xff;
	temp[2] = (gtc_val >> 16) & 0xff;
	temp[3] = (gtc_val >> 24) & 0xff;
	/* burst write tx GTC value */
	/* HW will monitor DP_TX_GTC_VALUE0 address, and automatically load the GTC value */
	dptx_aux_write_dpcd(hw_comm->tx_aux, DP_TX_GTC_VALUE0, temp, sizeof(temp));
	return gtc_val;
}

static void gtc_tx_timer_burst_wrrd_cb(void *argv)
{
	struct dptx_hw_common *hw_comm = (struct dptx_hw_common *)argv;
	struct gtc_tx_rx_gtc_val val;

	val.tx_gtc = gtc_tx_record_burst_write_gtc(hw_comm);
	val.rx_gtc = gtc_record_read_rx_gtc_and_is_lock_done(hw_comm);
	gtc_phase_val_save(&val);
}

static void gtc_tx_lock_acquisition_skew_phase(struct dptx_hw_common *hw_comm)
{
	struct timer_config lock_acq_tmr = {
		.timer_type = HW_LPM_TIMER,
		.wait_type = TIMER_REPEAT_ISR,
		.us = 1000,
		.repeater_times = PHASE_SKEW_OFF_CALC_TIMES,
		.callback = gtc_tx_timer_burst_wrrd_cb,
		.cb_data = hw_comm,
	};

	if (!hw_comm)
		return;
	gtc_phase_val_init();
	dptx_timer_start(hw_comm, &lock_acq_tmr);
}

static void gtc_tx_write_skew_adjust(struct dptx_hw_common *hw_comm, u16 skew_val)
{
	u8 temp[4];

	if (!hw_comm)
		return;

	/* write the calculated phase offset value with PHASE_SKEW_EN */
	temp[0] = GTC_VALUE_PHASE_SKEW_EN;
	temp[1] = 0;
	temp[2] = (skew_val >> 0) & 0xff;
	temp[2] = (skew_val >> 8) & 0xff;
	dptx_aux_write_dpcd(hw_comm->tx_aux, DP_RX_GTC_VALUE_PHASE_SKEW_EN, temp, sizeof(temp));
	/* clear the PHASE_SKEW_EN */
	temp[0] = 0;
	dptx_aux_write_dpcd(hw_comm->tx_aux, DP_RX_GTC_VALUE_PHASE_SKEW_EN, &temp[0], 1);
}

static void gtc_tx_lock_maintenance_phase(struct dptx_hw_common *hw_comm)
{
	struct timer_config lock_maint_tmr = {
		.timer_type = HW_LPM_TIMER,
		.wait_type = TIMER_REPEAT_ISR,
		.us = 10000,
		.repeater_times = -1,
		.callback = gtc_tx_timer_rpt_write_cb,
		.cb_data = hw_comm,
	};

	if (!hw_comm)
		return;

	dptx_timer_start(hw_comm, &lock_maint_tmr);
}

static void gtc_start_timer_mode(struct dptx_hw_common *hw_comm)
{
	int i;
	int ret;
	u16 skew_val;

	if (!hw_comm)
		return;

	gtc_set_tx_rtc_cap(hw_comm);

	for (i = 0; i < TEST_RX_GTC_TIMES; i++) {
		ret = gtc_tx_burst_write_gtc(hw_comm);
		if (ret >= 0)
			break;
	}
	if (i == TEST_RX_GTC_TIMES) {
		DPTX_INFO("GTC failed %s\n", __func__);
		return;
	}
	/* TX is GTC master */
	/* Lock Acquisition Phase */
	for (i = 0; i < TEST_RX_GTC_TIMES; i++) {
		gtc_tx_lock_acquisition_phase(hw_comm);
		wait_us_for_sync(hw_comm, 10000);
		ret = gtc_tx_phase_skew_offset(hw_comm);
		if (!ret)
			continue;
		/* Phase Skew Offset at the End of Lock Acquisition Phase */
		gtc_tx_lock_acquisition_skew_phase(hw_comm);
		wait_us_for_sync(hw_comm, PHASE_SKEW_OFF_CALC_TIMES * 1000);
		gtc_tx_burst_write_gtc(hw_comm);
		ret = gtc_read_rx_gtc_and_is_lock_done(hw_comm);
		if (!ret)
			continue;
	}
	if (!ret) {
		DPTX_INFO("Lock Acquisition failed\n");
		return;
	}
	/* write the DP_RX_GTC_VALUE_PHASE_SKEW_EN ~ DP_TX_GTC_PHASE_SKEW_OFFSET1 */
	skew_val = gtc_phase_val_calc_avg();
	gtc_tx_write_skew_adjust(hw_comm, skew_val);
	/* Lock Maintenance Phase */
	gtc_tx_lock_maintenance_phase(hw_comm);
}

int dptx_hw_gtc_start(struct dptx_hw_common *hw_comm, enum gtc_working_mode mode)
{
	ssize_t ret;
	u8 temp = 0;

	if (!hw_comm)
		return -1;

	/*
	 * GTC shall be supported by DP Sink devices with DPCD r1.2 (or higher) that support
	 * audio and all DP Branch devices with DPCD r1.2 (or higher). GTC should be supported
	 * by DP Source devices that support audio.
	 */
	dptx_aux_read_dpcd(hw_comm->tx_aux, DP_DPCD_REV, &temp, sizeof(temp));
	if (temp < DP_DPCD_REV_12)
		DPTX_INFO("rx dpcd version 0x%x\n", temp);
	/* check RX support GTC or not */
	ret = dptx_aux_read_dpcd(hw_comm->tx_aux, DP_DPRX_FEATURE_ENUMERATION_LIST,
		&temp, sizeof(temp));
	if (ret < 0 || !(temp & DP_GTC_CAP)) {
		DPTX_INFO("rx not support gtc\n");
		return -1;
	}

	if (mode == GTC_TIMER_MODE) {
		gtc_start_timer_mode(hw_comm);
		return 0;
	}
	/* TODO, wait for the final HW */
	if (mode == GTC_HW_AUTO_MODE) {
		DPTX_INFO("GTC hw auto mode TBD\n");
		return 0;
	}
	return 0;
}

int dptx_hw_gtc_stop(struct dptx_hw_common *hw_comm)
{
	struct timer_config lock_maint_tmr = {
		.timer_type = HW_LPM_TIMER,
	};

	dptx_timer_stop(hw_comm, &lock_maint_tmr);
	return 0;
}

struct gtc_phase_offset_record {
	int idx;
	struct gtc_tx_rx_gtc_val gtc_values[PHASE_SKEW_OFF_CALC_TIMES];
};

static struct gtc_phase_offset_record gtc_record;

static void gtc_phase_val_init(void)
{
	memset(&gtc_record, 0, sizeof(gtc_record));
}

static void gtc_phase_val_save(struct gtc_tx_rx_gtc_val *val)
{
	if (!val)
		return;

	if (gtc_record.idx >= PHASE_SKEW_OFF_CALC_TIMES)
		return;

	gtc_record.gtc_values[gtc_record.idx] = *val;
	gtc_record.idx++;
}

static u16 gtc_phase_val_calc_avg(void)
{
	int i;
	u32 sum = 0;
	u32 carry = 0;
	u32 temp;
	u32 delta[PHASE_SKEW_OFF_CALC_TIMES] = {0};

	for (i = 0; i < PHASE_SKEW_OFF_CALC_TIMES; i++)
		delta[i] = gtc_record.gtc_values[i].tx_gtc - gtc_record.gtc_values[i].rx_gtc;

	for (i = 0; i < PHASE_SKEW_OFF_CALC_TIMES; i++) {
		temp = sum + delta[i];
		carry += (temp < sum) ? 1 : 0;
		sum = temp;
	}

	/* the average of delta[] should be very small */
	if (carry || sum & 0xffff0000)
		DPTX_INFO("skew delta carry:0x%x sum:0x%x\n", carry, sum);

	temp = (u16)(sum / PHASE_SKEW_OFF_CALC_TIMES);
	return temp;
}
