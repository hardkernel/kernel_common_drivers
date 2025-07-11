// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_hw_common.h>
#include <linux/amlogic/media/vout/meson_tx_connector/phy/meson_tx_phy_core.h>

#include "../../meson_tx_task_mgr.h"
#include "dptx_log.h"
#include <dpcd_reg.h>
#include <dptx_aux_helper.h>
#include "dptx_hw.h"
#include "dptx20_reg.h"
#include "dptx_hw_opcode.h"
#include "dptx_link.h"
#include "dptx_infoframe.h"

/*
 * Function: dptx20_hw_calc_mst_reg
 *   calculate the MST register groups address
 * @vc_id: virtual channel id
 * @reg: MST register address
 * Return: the calculated MST address
 */
u32 dptx20_hw_calc_mst_reg(u8 vc_id, u32 reg)
{
	if (reg < DPTX20_SRCX_VIDEO_STREAM_ENABLE ||
		reg > (DPTX20_SRCX_VIDEO_STREAM_ENABLE + DPTX20_MST_SRC1))
		return reg;
	if (vc_id >= 4)
		return reg;
	return reg + vc_id * DPTX20_MST_SRC1;
}

/*
 * Function: dptx_set_tu_config
 * Parameter:
 *   pixel_clock: kHz
 *   colordepth: 8/10/12
 * Effective under SST, and this HW will be bypassed under MST
 * A transfer unit is a DisplayPort packet and represents valid data symbols
 * and stuffing symbols.  This register value sets the size of a transfer unit
 * in the transmitter framing logic.  Due to the design of the core, only even
 * values are supported by this register.
 * 2.2.14
 * of valid data symbols per lane = packed data rate/link symbol rate * TU size
 */
void dptx_set_tu_config(struct dptx_hw_common *hw_comm, u8 vc_id, u32 pixel_clock,
	enum colorimetry_format color, u8 colordepth)
{
	u32 bandwidth;
	u32 data_per_tu;
	u32 tu_size;
	u32 bits_per_pixel;

	/* default value */
	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_TU_CONFIG), 64);

	if (!hw_comm || !pixel_clock || color >= COLORIMETRY_FORMAT_MAX)
		return;

	bits_per_pixel = dptx_calc_bpp(color, colordepth);
	bandwidth = dptx20_reg_read(hw_comm, CORE_LEVEL, DPTX20_LINK_BW_SET) * 27;
	bandwidth *= dptx20_reg_read(hw_comm, CORE_LEVEL, DPTX20_LANE_COUNT_SET);
	if (!bandwidth)
		return;
	data_per_tu = (pixel_clock * 8 * bits_per_pixel) / bandwidth;
	tu_size = ((data_per_tu % 1000) * 16) / 1000;
	tu_size = (tu_size << 8) + (data_per_tu / 1000);
	tu_size = (tu_size << 16) | 0x0040;
	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_TU_CONFIG), tu_size);
}

static inline u32 choose_index(u32 val)
{
	u32 idx = 0ul;

	if (val >  1ul && val <  6ul)
		idx = 0ul;
	if (val >  7ul && val < 16ul)
		idx = 1ul;
	if (val > 17ul && val < 48ul)
		idx = 2ul;
	if (val > 49ul && val < 57ul)
		idx = 3ul;
	if (val > 58ul && val < 64ul)
		idx = 4ul;

	return idx;
}

/*
 * Function: dptx_set_data_control
 * Set the internal buffer delays based on the video mode and the TU size setting.
 * For high utilization modes, the FIFO timing is adjusted for minimum latency.
 * Parameters:
 *   para: the timing of current video resolution
 */
void dptx_set_data_control(struct dptx_hw_common *hw_comm, u8 vc_id,
	struct meson_tx_format_para *para)
{
	u32 tu_size;
	u32 data_per_tu;
	u32 fifo_settings;
	u32 data_control;
	u32 h_active;
	u32 h_blank;
	struct tx_timing *timing;

	if (!hw_comm || !para)
		return;

	timing = &para->timing;
	h_active = timing->h_active;
	h_blank = timing->h_blank;

	tu_size = dptx20_reg_read(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_TU_CONFIG));
	tu_size &= 0xff;

	/*
	 * link domain first TU delay after input data available
	 * large frames: delay depth = 4, accum delay = 1/2 TU
	 * small frames: delay depth = 3, accum delay = 1/4 TU
	 *   always for test only
	 */
	if (h_blank < (h_active / 20)) {
		fifo_settings = 0;
	} else {
		if (h_active > 128)
			fifo_settings = tu_size / 2;
		else
			fifo_settings = tu_size / 4;
	}
	fifo_settings <<= 8ul;

	/* video input data accumulation delay, high utilizated adjustment */
	data_per_tu = dptx20_reg_read(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_TU_CONFIG));
	switch (choose_index(data_per_tu >> 16)) {
	case 0:
		fifo_settings |= 8; /* 7% */
		break;
	case 1:
		fifo_settings |= 7; /* 8~25% */
		break;
	case 2:
		fifo_settings |= 6; /* 26~74% */
		break;
	case 3:
		fifo_settings |= 5; /* 75~92% */
		break;
	case 4:
	default:
		fifo_settings |= 4; /* 93~100% */
		break;
	}
	data_control = dptx20_reg_read(hw_comm, CORE_LEVEL, DPTX20_SRCX_DATA_CONTROL);
	data_control &= ~0xffff;
	data_control |= fifo_settings;
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_SRCX_DATA_CONTROL, data_control);
}

/*
 * Function: dptx_hw_get_mnvid_clock
 * For most cases, the stream clock and main link clock are coming from different VCO
 * sources. So the M/N_VID won't need be set and can be read for debug purpose.
 * Parameters:
 *     pixel_clock: represent the current stream/pixel clock, unit kHz
 *     link_clock:  represent the current main link clock, unit kHz
 */
void dptx_hw_get_mnvid_clock(struct dptx_hw_common *hw_comm, u8 vc_id,
	u32 *pixel_clock, u32 *link_clock)
{
	if (!pixel_clock || !link_clock)
		return;

	*pixel_clock = dptx20_reg_read(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_MVID));
	*link_clock = dptx20_reg_read(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_NVID));
}

/* TODO */
static u32 dptx_hw_get_intr_st(struct dptx_hw_common *hw_comm)
{
	return 0;
}

static irqreturn_t dptx_intr_handler(int irq, void *para)
{
	struct dptx_hw_common *hw_comm = (struct dptx_hw_common *)para;
	struct meson_hw_event_ops *event_ops = hw_comm->hw_base.event_ops;
	u32 intr_state = dptx_hw_get_intr_st(hw_comm);

	if (!event_ops)
		return IRQ_HANDLED;

	if (intr_state == HPD_PLUGIN) {
		/* HPD rising */
		event_ops->queue_event(event_ops->data, HPD_PLUGIN, 500);
	} else if (intr_state == HPD_PLUGOUT) {
		/* HPD falling */
		event_ops->cancel_event(event_ops->data, HPD_PLUGIN, false);
		event_ops->queue_event(event_ops->data, HPD_PLUGOUT, 0);
	}
	return IRQ_HANDLED;
}

static const char *const intr_name[] = {
	"dptx0",
	"dptx1",
	"dptx2",
	"dptx3",
};

/* assume different instance have different irq number */
static int dptx_setup_irq(struct meson_tx_hw *tx_hw)
{
	int ret = 0;
	struct dptx_hw_common *hw_comm = to_dptx_hw_common(tx_hw);

	if (!hw_comm)
		return -1;
	ret = request_irq(hw_comm->irq_id, &dptx_intr_handler,
		IRQF_SHARED, intr_name[hw_comm->dev_idx], (void *)hw_comm);
	return ret;
}

static void dptx_free_irq(struct meson_tx_hw *tx_hw)
{
	struct dptx_hw_common *hw_comm = to_dptx_hw_common(tx_hw);

	if (!hw_comm)
		return;

	free_irq(hw_comm->irq_id, (void *)hw_comm);
}

/*
 * initialiase the train pattern sequence
 *
 * Function: Sets test pattern on to the hardware
 * Sets test pattern on to the hardware
 */
static void init_tps1(struct dptx_hw_common *hw_comm)
{
	u32 pattern_val = 0;
	u32 scrambler_val = 0;

	pattern_val = DP_TRAINING_PATTERN_1 & 0x07;             /* set pattern register */
	scrambler_val = 1;                                      /* disable the scrambler */

	/* start sending test pattern */
	/* TODO */
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_DISABLE_SCRAMBLING, scrambler_val);
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_TRAINING_PATTERN_SET, pattern_val);
}

static void dptx_set_phy_link_rate(struct meson_tx_hw *tx_hw, enum dptx_link_rate_e link_rate)
{
	if (!tx_hw || !tx_hw->tx_phy)
		return;
	if (link_rate == DPTX_LINK_RATE_UNKNOWN || link_rate == DPTX_LINK_RATE_MAX) {
		DPTX_ERROR("%s invalid link rate: %d\n", __func__, link_rate);
		return;
	}
	/* TODO: related to dptx phy implementation, transfer enum to 1620/2700... */
	tx_hw->tx_phy_conf.dp_ops.link_rate = (unsigned int)link_rate;
	meson_tx_phy_configure(tx_hw->tx_phy, &tx_hw->tx_phy_conf);
}

static void dptx_phy_set_lane_count(struct meson_tx_hw *tx_hw, enum dptx_lane_count_e lane_ct)
{
	if (!tx_hw || !tx_hw->tx_phy)
		return;
	if (lane_ct == DPTX_LANE_COUNT_UNKNOWN || lane_ct == DPTX_LANE_COUNT_MAX) {
		DPTX_ERROR("%s invalid lane count: %d\n", __func__, lane_ct);
		return;
	}
	/* TODO: related to dptx phy implementation */
	tx_hw->tx_phy_conf.dp_ops.lanes = (unsigned int)lane_ct;
	meson_tx_phy_configure(tx_hw->tx_phy, &tx_hw->tx_phy_conf);
}

/*
 * Function: dptx_hw_set_phy_vswing
 *
 * Set the voltage swing value for each lane to the PHY.
 * Actual values programmed into the hardware are specific to each PHY.
 * Some PHYs do not require hardware programming.
 * There is no DPTX core voltage swing level. This is a PHY only value.
 *
 * #2.1.2.4
 */
static void dptx_hw_set_phy_vswing(struct meson_tx_hw *tx_hw, enum dptx_phy_vswing_e vswing)
{
	u8 i = 0;

	if (!tx_hw || !tx_hw->tx_phy)
		return;
	if (vswing == DPTX_PHY_VSWING_UNKNOWN || vswing == DPTX_PHY_VSWING_MAX) {
		DPTX_ERROR("%s invalid vswing: %d\n", __func__, vswing);
		return;
	}
	/* TODO: related to dptx phy implementation, replace 4 by lane count */
	for (i = 0; i < 4; i++)
		tx_hw->tx_phy_conf.dp_ops.voltage[i] = (unsigned int)vswing;
	meson_tx_phy_configure(tx_hw->tx_phy, &tx_hw->tx_phy_conf);
}

/*
 * Function: dptx_hw_set_phy_preemph
 *
 * sets the pre-emphasis levels for the PHY.
 * The PHY driver sets the registers as appropriate for the selected
 * pre-emphasis levels.
 *
 * #2.1.2.5
 */
static void dptx_hw_set_phy_preemph(struct meson_tx_hw *tx_hw,
	enum dptx_phy_preemphasis_e pre_emphasis)
{
	u8 i = 0;

	if (!tx_hw || !tx_hw->tx_phy)
		return;
	if (pre_emphasis == DPTX_PHY_PREEMP_UNKNOWN || pre_emphasis == DPTX_PHY_PREEMP_MAX) {
		DPTX_ERROR("%s invalid pre_emphasis: %d\n", __func__, pre_emphasis);
		return;
	}
	/* TODO: related to dptx phy implementation, replace 4 by lane count */
	for (i = 0; i < 4; i++)
		tx_hw->tx_phy_conf.dp_ops.pre[i] = (unsigned int)pre_emphasis;
	meson_tx_phy_configure(tx_hw->tx_phy, &tx_hw->tx_phy_conf);
}

/* #1.1 init phy instance */
static int dptx_phy_inst_data_init(struct meson_tx_hw *tx_hw)
{
	/* TODO: related to dptx phy implementation */
	DPTX_INFO("TODO %s\n", __func__);
	return 0;
}

/*
 * Function: dptx_aux_nb_init
 *
 * initialize non-blocking data structure
 *
 * #1.2.1
 */
static int dptx_aux_nb_init(struct dptx_hw_common *hw_comm)
{
	/* TODO: */
	/* step1: init aux transaction queue */
	/* step2: state machine init for aux transaction */
	/* step3: register aux interrupt handler */
	/* step4: add virtual timing handler for aux */

	return 0;
}

/*
 * Function: dptx_tmr_gp_init
 *
 * Initialize general purpose timer block in the DisplayPort TX core.
 *
 * #1.2.2
 */
static int dptx_tmr_gp_init(struct dptx_hw_common *hw_comm)
{
	/* TODO: add timer for general purpose */
	return 0;
}

/*
 * Function: dptx_tmr_hdcp_init
 *
 * Initialize HDCP timer block in the DisplayPort TX core.
 *
 * #1.2.3
 */
static int dptx_tmr_hdcp_init(struct dptx_hw_common *hw_comm)
{
	/* TODO: add timer for hdcp */
	return 0;
}

/*
 * Function: dptx_tmr_mst_init
 *
 * Initialize MST timer block in the DisplayPort TX core.
 *
 * #1.2.4
 */
static int dptx_tmr_mst_init(struct dptx_hw_common *hw_comm)
{
	/* TODO: add timer for mst */
	return 0;
}

/* #1.2 */
static void dptx_func_blk_data_init(struct dptx_hw_common *hw_comm)
{
	if (!hw_comm)
		return;
	/* get core revision */
	hw_comm->core_info = dptx20_reg_read(NULL, CORE_LEVEL, DPTX20_CORE_REVISION);

	dptx_aux_nb_init(hw_comm);
	dptx_tmr_gp_init(hw_comm);
	dptx_tmr_hdcp_init(hw_comm);
	dptx_tmr_mst_init(hw_comm);
}

/* #1.3 lpm data structure init */
static void dptx_lpm_data_init(struct dptx_hw_common *hw_comm)
{
}

/* hw_init#1 init hw realted driver data structure */
static void dptx_hw_inst_data_init(struct dptx_hw_common *hw_comm)
{
	if (!hw_comm)
		return;
	dptx_phy_inst_data_init(&hw_comm->hw_base);
	dptx_func_blk_data_init(hw_comm);
	dptx_lpm_data_init(hw_comm);
}

/* #2.1.1 phy init operation:
 * such as reset phy->wait for pll lock->config link rate,
 * and set phy reference clk.
 */
static void dptx_hw_phy_init(struct meson_tx_phy *tx_phy)
{
	if (!tx_phy)
		return;
	/* TODO: related to dptx phy implementation */
}

/*
 * Function: dptx_hw_set_clk_div
 *
 * Set the clock divider in the TX core.
 * set integer value in MHz equal to the clock speed of the connected
 * APB clock signal. must be called before the TX core can be initialized.
 * param:
 * apb_clk_mhz - the value to set the clock divider register to in Mhz.
 * This must match the value of the APB clock signal connected to the core
 *
 * #2.1.2.1
 */
static void dptx_hw_set_clk_div(struct dptx_hw_common *hw_comm, u32 apb_clk_mhz)
{
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_AUX_CLOCK_DIVIDER, apb_clk_mhz);
}

/*
 * Function: dptx_hw_set_link_rate
 *
 * Sets the current link rate for the TX core.
 * This programs the LINK_BW register and calls the PHY link rate update
 * function to set the clock speed in the main link.
 * PHY reset is sent to allow the internal PLLs to lock to the new clock rate.
 *
 * Parameter values correspond to the valid programmable link rates for
 * DisplayPort, in the range of 0x06 to 0x1E, and produces a link rate
 * equal to value * 270Mhz. E.g. 0x06 * 270 = 1.62Ghz.
 *
 * Note that the LINK_BW register is not used by the digital portion of the
 * core and is included here for PHY implementations that require it.
 *
 * The four standard link-rates are:
 * Link Rate       program value hex / decimal
 * 1.62 Gb/s       0x06 / 6
 * 2.7  Gb/s       0x0a / 10
 * 5.4  Gb/s       0x14 / 20
 * 8.1  Gb/s       0x1e / 30
 *
 * Parameters:
 * link_rate - must be within the range 0x06 - 0x1E
 *
 * #2.1.2.2
 */
static void dptx_hw_set_link_rate(struct dptx_hw_common *hw_comm, enum dptx_link_rate_e link_rate)
{
	/* Set link rate: TO CONFIRM if needed */
    /* cfg.link_rate = link_rate; */

	/* Disable the core before continuing */
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_TRANSMITTER_ENABLE, 0);
	/* Set the link rate in the core (for completeness) */
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_LINK_BW_SET, link_rate);
	/* Set the link rate in the PHY */
	dptx_set_phy_link_rate(&hw_comm->hw_base, link_rate);
	/* Reset the link */
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_SOFT_RESET, 0x01);
	/* Small delay */
	udelay(2);
	/* Re-enable the core for use */
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_TRANSMITTER_ENABLE, 1);
}

/*
 * Function: dptx_hw_set_lane_ct
 *
 * Set the number of lanes for the transmitter link.
 *
 * #2.1.2.3
 */
static void dptx_hw_set_lane_ct(struct dptx_hw_common *hw_comm, enum dptx_lane_count_e lane_ct)
{
	/* Set link rate: TO CONFIRM if needed */
    /* cfg.lane_ct = lane_ct; */

	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_LANE_COUNT_SET, lane_ct);
	dptx_phy_set_lane_count(&hw_comm->hw_base, lane_ct);
}

#define APB_BUS_FREQ_MHZ 150

/*
 * Function: dptx_hw_core_init
 *
 * Initialize the DisplayPort transmitter hardware
 *
 * #2.1.2
 */
void dptx_hw_core_init(struct dptx_hw_common *hw_comm)
{
	/* Set the clock divider for the TX core to the APB clock frequency */
	dptx_hw_set_clk_div(hw_comm, APB_BUS_FREQ_MHZ);
	/* Disable transmitter */
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_TRANSMITTER_ENABLE, 0);

	/* transmitter initialization */
	/* Enable transmitter */
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_TRANSMITTER_ENABLE, 1);
	/* Enable MST source 0 (SST mode from init) */
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_SOURCE_ENABLE, 0x1);
	/* Clear LQ pattern, training pattern and enable scrambling */
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_LINK_QUAL_PATTERN_SET, 0);
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_TRAINING_PATTERN_SET, 0);
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_DISABLE_SCRAMBLING, 0);
	/* Enable enhanced framing mode and spread spectrum clocking */
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_ENHANCED_FRAMING_ENABLE, 1);
	/* Set aux channel reply timeout to 400uS */
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_AUX_REPLY_TIMEOUT_INTERVAL, 400);

	dptx_hw_set_link_rate(hw_comm, DPTX_LINK_RATE_1P62GHZ);
	dptx_hw_set_lane_ct(hw_comm, DPTX_LANE_COUNT_4);
	dptx_hw_set_phy_vswing(&hw_comm->hw_base, DPTX_PHY_VSWING_400);
	dptx_hw_set_phy_preemph(&hw_comm->hw_base, DPTX_PHY_PREEMP_0DB);
}

/*
 * Function: dptx_hw_lpm_stack_init
 *
 * LPM stack initialization function
 * This function initializes LPM data structures.
 *
 * #2.1.3
 */
static void dptx_hw_lpm_stack_init(struct dptx_hw_common *hw_comm)
{
	/* TODO */
	/* step1: lpm state machine init */
	/* step2: lpm event queue init, 8 events */
	/* step3: register lpm interrupt handler */
}

/*
 * Function: dptx_hw_intr_init
 *
 * Initialize DisplayPort transmitter interrupts.
 *
 * #2.1.4
 */
void dptx_hw_intr_init(struct dptx_hw_common *hw_comm)
{
	/* TODO: interrupt handler register */

	/* core level intr unmask: enable core intr */
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_INTERRUPT_MASK, 0);
}

/*
 * Function: dptx_isr_disable
 *
 * Disable selected interrupts.
 */
void dptx_isr_disable(struct dptx_hw_common *tx_comm, u32 flags)
{
	u32 isr_mask = dptx20_reg_read(tx_comm, CORE_LEVEL, DPTX20_INTERRUPT_MASK);

	isr_mask |= flags;
	dptx20_reg_write(tx_comm, CORE_LEVEL, DPTX20_INTERRUPT_MASK, isr_mask);
}

/* #2.1 dptx task */
static void dptx_hw_dptx_task_init(struct dptx_hw_common *hw_comm)
{
	dptx_hw_phy_init(hw_comm->hw_base.tx_phy);
	dptx_hw_core_init(hw_comm);
	dptx_hw_lpm_stack_init(hw_comm);
	dptx_hw_intr_init(hw_comm);
}

/* #2.2 Initialize HDCP task */
static void dptx_hw_hdcp2_task_init(struct dptx_hw_common *hw_comm)
{
	/* TODO */
}

/* #2.3 Initialize the timer task */
static void dptx_hw_timer_task_init(struct dptx_hw_common *hw_comm)
{
	/* TODO */
}

/* #2.4 mst task */
static void dptx_hw_mst_task_init(struct dptx_hw_common *hw_comm)
{
	/* TODO */
}

/*
 * Function: dptx_hw_task_init
 *
 * Initialize fixed list of tasks for use
 *
 * hw_init#2 init each hw function block, including hw init/state machine init
 * and interrupt register for each sub function block
 */
static void dptx_hw_task_init(struct dptx_hw_common *hw_comm)
{
	dptx_hw_dptx_task_init(hw_comm);
	dptx_hw_hdcp2_task_init(hw_comm);
	dptx_hw_timer_task_init(hw_comm);
	dptx_hw_mst_task_init(hw_comm);
}

/*
 * Function: dptx_hw_top_intr_enable
 *
 * Enable interrupts.
 * If interrupts are enabled, then interrupts are processed.
 *
 * hw_init#3 enable intr of top level and then each sub function block
 * will be trigger by interrupt->handler interrupt->state machine run
 */
static void dptx_hw_top_intr_enable(struct dptx_hw_common *hw_comm)
{
	/* TODO: system top level intr enable */
}

/*
 * Function: dptx20_hw_init
 *
 * Main function
 */
static int dptx20_hw_init(struct meson_tx_hw *tx_hw)
{
	struct dptx_hw_common *hw_comm = NULL;

	if (!tx_hw)
		return -1;

	DPTX_INFO("%s\n", __func__);
	hw_comm = to_dptx_hw_common(tx_hw);
	hw_comm->aux_transfer = dptx_aux_xfer;
	/* TO CONFIRM: if need to move to dptx_hw_intr_init() */
	dptx_setup_irq(tx_hw);

	dptx_hw_inst_data_init(hw_comm);
	dptx_hw_task_init(hw_comm);
	dptx_hw_top_intr_enable(hw_comm);
	return 0;
}

/* map color depth to configuration value of core register */
static u8 dptx20_get_mapped_cd_conf(enum hdmi_color_depth cd)
{
	u8 bit_depth = 0;

	switch (cd) {
	case COLORDEPTH_18B:
		bit_depth = 0x0;
		break;
	case COLORDEPTH_24B:
		bit_depth = 0x1;
		break;
	case COLORDEPTH_30B:
		bit_depth = 0x2;
		break;
	case COLORDEPTH_36B:
		bit_depth = 0x3;
		break;
	case COLORDEPTH_48B:
		bit_depth = 0x4;
		break;
	/* note: 7/14 for colorimetry Format of RAW(dp1.4a table 2-96) is not set */
	default:
		bit_depth = 1;
		DPTX_ERROR("%s cd[%d] not find mapped bit_depth, for 8bit depth\n", __func__, cd);
		break;
	}
	return bit_depth;
}

/*
 * Function: dptx20_hw_set_vsc_colorimetry
 *
 * override MSA colorimetry/color_depth with VSC
 * for Y420 and BT2O20, etc.
 *
 * refer to DP1.4a chapter2.2.5.7 and table 2-96 for detail
 */
static void __maybe_unused dptx20_hw_set_vsc_colorimetry(struct dptx_hw_common *hw_comm,
	u8 vc_id, struct meson_tx_format_para *para)
{
	u32 data = 0;

	if (para->cs != HDMI_COLORSPACE_YUV420 && !para->dsc_en) {
		/* disable override of MSA */
		dptx20_reg_write(hw_comm, CORE_LEVEL,
			dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_COLORIMETRY_OVERRIDE), 0);
		return;
	}

	/* bit depth override */
	data |= dptx20_get_mapped_cd_conf(para->cd) & 0x7;
	/* override colorimetry */
	if (para->cs == HDMI_COLORSPACE_YUV420)
		data |= 0x4 << 3;
	else
		data |= (para->cs & 0x3) << 3;
	if (para->dsc_en)
		data |= 0x5 << 3;
	/* override enable */
	data |= 1 << 6;

	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_COLORIMETRY_OVERRIDE), data);
}

/*
 * Function: dptx20_hw_set_msa
 *
 * set main stream attribute.
 * DP1.4 spec Chapter2.2.4.3: For y420 and ITU-R BT.2020
 * colorimetry formats, VSC SDP shall be used.
 * refer to dp1.4 spec table2-94 for detail
 */
static void __maybe_unused dptx20_hw_set_msa(struct dptx_hw_common *hw_comm,
	u8 vc_id, struct meson_tx_format_para *para)
{
	u32 misc = 0;
	/*
	 * When bit 6 is set to 1, a Source device uses a VSC SDP to indicate the Pixel
	 * Encoding/Colorimetry format and that a Sink device shall ignore bit 7, and MISC0,
	 * bits 7:1 (i.e., MISC1, bit 7, and MISC0, bits 7:1, become "don’t care").
	 */
	bool vsc_colorimetry = false;
	/*
	 * 0: Link clock and main video stream clock are asynchronous.
	 * 1: Link clock and main video stream clock are synchronous.
	 */
	bool clk_synchronous = false;
	/* TODO: not Y-only or Raw mode by default, support in the future? */
	bool y_only_or_raw_mode = false;
	bool even_line_cnt = false;

	if (!para)
		return;

	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_MAIN_STREAM_HTOTAL),
		para->timing.h_total);
	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_MAIN_STREAM_VTOTAL),
		para->timing.v_total);
	/* 0 = Active high pulse. 1 = Active low pulse */
	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_MAIN_STREAM_POLARITY),
		(!para->timing.h_pol) | (!para->timing.v_pol << 1));
	/* Hsync width, measured in pixel count. */
	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_MAIN_STREAM_HSWIDTH),
		para->timing.h_sync);
	/* Vsync width, measured in line count. */
	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_MAIN_STREAM_VSWIDTH),
		para->timing.v_sync);
	/* Active video width, measured in pixel count. */
	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_MAIN_STREAM_HRES),
		para->timing.h_active);
	/* Active video height, measured in line count. */
	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_MAIN_STREAM_VRES),
		para->timing.v_active);
	/* Horizontal active start from leading edge of Hsync, measured in pixel count. */
	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_MAIN_STREAM_HSTART),
		para->timing.h_sync + para->timing.h_back);
	/* Vertical active start from leading edge of Vsync, measured in line count. */
	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_MAIN_STREAM_VSTART),
		para->timing.v_sync + para->timing.v_back);

	misc = 0;
	misc = clk_synchronous << 0;
	/* color format; Note: 0x3 means reserved */
	misc |= (para->cs & 0x3) << 1;
	/* 0: VESA (full/limit color)range, 1: CTA range; TODO: add color range in para? */
	misc |= 0 << 3;
	/* 0: BT.601, 1: BT.709; TODO: add colorimetry in para? */
	misc |= 1 << 4;
	misc |= (dptx20_get_mapped_cd_conf(para->cd) & 0x7) << 5;
	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_MAIN_STREAM_MISC0), misc);

	/* check the line count is even or odd for interlace mode */
	if (para->timing.pi_mode == 0)
		even_line_cnt = para->timing.v_total % 2 ? false : true;
	misc = 0;
	/* set to 1 when the number of lines per frame of
	 * the interlaced video stream is an even number.
	 */
	misc |= even_line_cnt << 0;
	/* no stereo video(3D) transported */
	misc |= 0x0 << 1;
	/* bit5:3 must be set to 0 as reserved */
	misc |= 0x0 << 3;
	misc |= vsc_colorimetry << 6;
	misc |= y_only_or_raw_mode << 7;
	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_MAIN_STREAM_MISC1), misc);

	/*
	 * In asynchronous mode, this register is read-only and will return the
	 * value computed by the internal logic based on the input video clock. In
	 * synchronous mode, this register is read-write and should be set to the
	 * pixel clock rate multiplied by 100000
	 */
	/* dptx20_reg_write(hw_comm, CORE_LEVEL, */
		/* dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_MVID), 0); */

	/*
	 * DP1.4 spec Chapter2.2.3: The Nvid value in this Asynchronous Clock mode shall
	 * be set to 2^15 (= 32768) or higher. A value of power of two should be used.
	 *
	 * In asynchronous mode, this register is read-only and will
	 * return 0x8000. In synchronous mode, this register is read-write
	 * and should be set to the link rate multiplied by 100000
	 */
	/* dptx20_reg_write(hw_comm, CORE_LEVEL, */
		/* dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_NVID), 0); */
}

static int dptx20_hw_cntl_linkconf(struct dptx_hw_common *hw_comm, u32 cmd,
	void *input_argv, void *output_struct)
{
	int val = 0;
	int scramble = 0;
	int pattern = 0;
	u8 dpcd[4] = {0};

	if (!hw_comm)
		return -1;
	if ((cmd & DP_AUX_LINKCONF_OFFSET) != DP_AUX_LINKCONF_OFFSET) {
		DPTX_ERROR("%s cmd[0x%x] wrong cmd type\n", __func__, cmd);
		return -1;
	}

	switch (cmd) {
	case LINKCONF_INIT_TPS1:
		init_tps1(hw_comm);
		break;
	case LINKCONF_ENABLE_LINK:
		dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_TRANSMITTER_ENABLE,
			!!(*(int *)input_argv));
		break;
	case LINKCONF_ENABLE_SRC_VID:
		// TODO
		// dptx20_reg_set_bits(NULL, CORE_LEVEL, DPTX20_SRCX_VIDEO_STREAM_ENABLE,
		//	!!(*(int *)input_argv));
		udelay(5);
		dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_FORCE_SCRAMBLER_RESET,
			!!(*(int *)input_argv));
		break;
	case LINKCONF_ENABLE_ENHANCED_FRAME:
		dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_ENHANCED_FRAMING_ENABLE,
			!!(*(int *)input_argv));
		break;
	/* Updates DPTX and DPCD with new link rate */
	case LINKCONF_SET_LINK_RATE:
		dptx_hw_set_link_rate(hw_comm, *(enum dptx_link_rate_e *)input_argv);
		dpcd[0] = *(int *)input_argv;
		// TODO
		dptx_aux_write_dpcd(NULL, DP_LINK_BW_SET, dpcd, 1);
		break;
	/* Updates DPTX and DPCD with new lane count */
	case LINKCONF_SET_LANE_COUNT:
		val = dptx20_reg_read(hw_comm, CORE_LEVEL, DPTX20_ENHANCED_FRAMING_ENABLE);
		dpcd[0] = 0;
		if (val & 0x1)
			dpcd[0] = DP_LANE_COUNT_ENHANCED_FRAME_EN;
		dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_LANE_COUNT_SET, *(int *)input_argv);
		dptx_phy_set_lane_count(&hw_comm->hw_base, *(enum dptx_lane_count_e *)input_argv);
		dpcd[0] |= *(int *)input_argv;
		// TODO
		dptx_aux_write_dpcd(NULL, DP_LANE_COUNT_SET, dpcd, 1);
		break;
	case LINKCONF_SET_DOWNSPREAD:
		dpcd[0] = 0;
		if (*(int *)input_argv)
			dpcd[0] = DP_SPREAD_AMP_0_5;
		// TODO
		dptx_aux_write_dpcd(NULL, DP_DOWNSPREAD_CTRL, dpcd, 1);
		break;
	case LINKCONF_SET_8B10B_CODING:
		dpcd[0] = 0;
		if (*(int *)input_argv)
			dpcd[0] = DP_SET_ANSI_8B10B;
		// TODO
		dptx_aux_write_dpcd(NULL, DP_MAIN_LINK_CHANNEL_CODING_SET, dpcd, 1);
		break;
	case LINKCONF_SET_TRAIN_PATTERN:
		dpcd[0] = 0;
		val = *(int *)input_argv;
		switch (val) {
		case LINK_PATTERN_OFF:
		case LINK_PATTERN_D10_2:
		case LINK_PATTERN_SYM_ERR:
		case LINK_PATTERN_PRBS7:
			dpcd[0] = 0;
			pattern = 0;
			scramble = 0;
			break;
		case LINK_PATTERN_1:
		case LINK_PATTERN_2:
		case LINK_PATTERN_3:
			dpcd[0] = 0;
			pattern = val & 0x7;
			scramble = 1;
			dpcd[0] = val;
			break;
		case LINK_PATTERN_4:
			dpcd[0] = 7;
			pattern = 4;
			scramble = 0;
			dpcd[0] = val;
			break;
		default:
			break;
		}
		dptx20_reg_write(NULL, CORE_LEVEL, DPTX20_DISABLE_SCRAMBLING, scramble);
		dptx20_reg_write(NULL, CORE_LEVEL, DPTX20_TRAINING_PATTERN_SET, pattern);
		// TODO
		dptx_aux_write_dpcd(NULL, DP_TRAINING_PATTERN_SET, dpcd, 1);
		break;
	case LINKCONF_SET_VSWING_PREEMP:
		{
			struct phy_vswing_preemp *phy;
			u8 vs;
			u8 pe;

			phy = (struct phy_vswing_preemp *)input_argv;
			dptx_hw_set_phy_vswing(&hw_comm->hw_base, phy->vswing);
			dptx_hw_set_phy_preemph(&hw_comm->hw_base, phy->preemp);
			vs = phy->vswing;
			pe = phy->preemp;
			if (phy->vswing == DPTX_PHY_VSWING_1200)
				vs |= DP_TRAIN_MAX_SWING_REACHED;
			if (phy->preemp == DPTX_PHY_PREEMP_9DB)
				pe |= DP_TRAIN_MAX_PRE_EMPHASIS_REACHED;
			dpcd[0] = (pe << 3) | vs;
			dpcd[1] = dpcd[0];
			dpcd[2] = dpcd[0];
			dpcd[3] = dpcd[0];
			dptx_aux_write_dpcd(NULL, DP_TRAINING_LANE0_SET, dpcd, 4);
		}
		break;
	default:
		break;
	}
	return 0;
}

static int dptx20_pre_enable_mode(struct dptx_hw_common *tx_comm)
{
	DPTX_INFO("%s %px done\n", __func__, tx_comm);

	return 0;
}

static int dptx20_enable_mode(struct dptx_hw_common *tx_comm)
{
	DPTX_INFO("%s %px done\n", __func__, tx_comm);

	return 0;
}

static int dptx20_post_enable_mode(struct dptx_hw_common *tx_comm)
{
	DPTX_INFO("%s %px done\n", __func__, tx_comm);

	return 0;
}

static int dptx20_hw_cntl_flow_misc(struct meson_tx_hw *tx_hw, u32 cmd,
				  void *input_argv, void *output_struct)
{
	struct dptx_hw_common *hw_comm = to_dptx_hw_common(tx_hw);
	int ret = 0;

	switch (cmd) {
	case MODE_FLOW_PRE_ENABLE_MODE:
		return dptx20_pre_enable_mode(hw_comm);
	case MODE_FLOW_ENABLE_MODE:
		return dptx20_enable_mode(hw_comm);
	case MODE_FLOW_POST_ENABLE_MODE:
		return dptx20_post_enable_mode(hw_comm);
	default:
		break;
	}

	return ret;
}

static int dptx20_hw_cntl(struct meson_tx_hw *tx_hw, u32 cmd,
	void *input_argv, void *output_struct)
{
	u32 cmd_type = cmd & CMD_TYPE_MASK;
	int ret = 0;
	struct dptx_hw_common *hw_comm = NULL;

	if (!tx_hw)
		return -1;
	hw_comm = to_dptx_hw_common(tx_hw);

	switch (cmd_type) {
	case CMD_LINK_TRAINING:
		ret = dptx20_hw_cntl_linkconf(hw_comm, cmd, input_argv, output_struct);
		break;
	case CMD_MODE_FLOW_MISC_OFFSET:
		ret = dptx20_hw_cntl_flow_misc(tx_hw, cmd, input_argv, output_struct);
		break;
	default:
		break;
	}
	return ret;
}

void dptx20_free_tx_hw(struct meson_tx_hw *tx_hw)
{
	struct dptx_hw_common *hw_comm = to_dptx_hw_common(tx_hw);

	if (!hw_comm)
		return;

	dptx_free_irq(&hw_comm->hw_base);
	kfree(hw_comm);
}

struct meson_tx_hw *dptx20_alloc_tx_hw(void)
{
	struct dptx_hw_common *hw_comm = kzalloc(sizeof(*hw_comm), GFP_KERNEL);

	if (!hw_comm)
		return NULL;

	hw_comm->hw_base.hw_cntl = dptx20_hw_cntl;
	hw_comm->hw_base.init_tx_hw = dptx20_hw_init;
	return &hw_comm->hw_base;
}

