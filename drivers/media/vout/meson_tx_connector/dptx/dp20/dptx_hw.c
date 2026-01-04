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
#include <linux/amlogic/media/vout/meson_tx_connector/clk/meson_tx_clk.h>

#include "meson_tx_task_mgr.h"
#include "dptx_log.h"
#include <dpcd_reg.h>
#include <dptx_aux_helper.h>
#include "dptx_hw.h"
#include "dptx20_reg.h"
#include "dptx20_platform_reg.h"
#include "dptx_hw_opcode.h"
#include "dptx_link.h"
#include "dptx_infoframe.h"
#include "dptx_hw_timer.h"

/*
 * Function: dptx20_hw_calc_mst_reg
 *
 * calculate the MST register with video_num under MST mode
 * There are 4 MST0/1/2/3 group registers, 4 groups are the same function
 * but the base address are different.
 * DPTX20_MST_SRC0: 0x800 ~ 0x9bc
 * DPTX20_MST_SRC1: 0xa00 ~ 0xbbc
 * DPTX20_MST_SRC2: 0xc00 ~ 0xdbc
 * DPTX20_MST_SRC3: 0xe00 ~ 0xfbc
 *
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
static void dptx_set_tu_config(struct dptx_hw_common *hw_comm, u8 vc_id, u32 pixel_clock,
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
static void dptx_set_data_control(struct dptx_hw_common *hw_comm, u8 vc_id,
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

static void dptx20_video_soft_reset(struct dptx_hw_common *hw_comm, u8 vc_id)
{
	if (!hw_comm || vc_id > 3)
		return;
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_SOFT_RESET, 1 << vc_id);
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_SOFT_RESET, 0);
	DPTX_INFO("perform video capture %d reset\n", vc_id);
}

static void dptx20_timer_intr_handler(struct dptx_hw_common *hw_comm, enum timer_hw_type type)
{
	struct dptx_timer_handler *handler;

	if (!hw_comm || type > HW_TIMER_MAX)
		return;

	handler = &hw_comm->timer_manager->handlers[type];

	/* handler the TIMER_ISR and TIMER_REPEAT_ISR case */
	switch (handler->cfg.wait_type) {
	case TIMER_ISR:
		if (handler->cfg.callback)
			handler->cfg.callback(handler->cfg.cb_data);
		hw_timer_init(hw_comm, type);
		break;
	case TIMER_REPEAT_ISR:
		if (handler->cfg.callback)
			handler->cfg.callback(handler->cfg.cb_data);
		if (handler->cfg.repeater_times == 0)
			hw_timer_init(hw_comm, type);
		if (handler->cfg.repeater_times != -1)
			handler->cfg.repeater_times--;
		break;
	/* nothing to do for TIMER_WAIT */
	case TIMER_WAIT:
	default:
		DPTX_INFO("check timer usage, type=%d delay=%d\n", type, handler->cfg.us);
		break;
	}
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

/* aux reply interrupt bottom handler */
static void dptx_aux_irq_process(void *para)
{
	struct dptx_hw_common *hw_comm = (struct dptx_hw_common *)para;
	struct dptx20_hw *tx20_hw = NULL;

	if (!hw_comm) {
		DPTX_ERROR("%s NULL hw_comm pointer\n", __func__);
		return;
	}
	tx20_hw = to_dptx20_hw(hw_comm);
	/* AUX interrupts, below two interrupts should not come together */
	if (tx20_hw->intr_state & INTERRUPT_STATE_REPLY_RECEIVED) {
		tx20_hw->intr_state &= ~INTERRUPT_STATE_REPLY_RECEIVED;
		DPTX_INFO("intr_state=0x%x\n", tx20_hw->intr_state);
	}
	if (tx20_hw->intr_state & INTERRUPT_STATE_REPLY_TIMEOUT) {
		tx20_hw->intr_state &= ~INTERRUPT_STATE_REPLY_TIMEOUT;
		DPTX_INFO("intr_state=0x%x\n", tx20_hw->intr_state);
	}
}

/* aux reply interrupt bottom handler */
static void dptx_timer_irq_process(void *para)
{
	struct dptx_hw_common *hw_comm = (struct dptx_hw_common *)para;
	struct dptx20_hw *tx20_hw = NULL;

	if (!hw_comm) {
		DPTX_ERROR("%s NULL hw_comm pointer\n", __func__);
		return;
	}
	tx20_hw = to_dptx20_hw(hw_comm);
	/* AUX interrupts, below two interrupts should not come together */
	if (tx20_hw->intr_state & INTERRUPT_STATE_MST_TIMER_EVENT) {
		tx20_hw->intr_state &= ~INTERRUPT_STATE_MST_TIMER_EVENT;
		dptx20_timer_intr_handler(hw_comm, HW_MST_TIMER);
	}
	if (tx20_hw->intr_state & INTERRUPT_STATE_HDCP_TIMER_EVENT) {
		tx20_hw->intr_state &= ~INTERRUPT_STATE_HDCP_TIMER_EVENT;
		dptx20_timer_intr_handler(hw_comm, HW_HDCP_TIMER);
	}
	if (tx20_hw->intr_state & INTERRUPT_STATE_GP_TIMER_EVENT) {
		tx20_hw->intr_state &= ~INTERRUPT_STATE_GP_TIMER_EVENT;
		dptx20_timer_intr_handler(hw_comm, HW_GP_TIMER);
	}
	/* STUB for LPM timer */
}

static struct tx_task_info dptx20_task_infos[] = {
	{
		.name = "aux_irq_task",
		.fn = dptx_aux_irq_process,
		.type = AUX_IRQ_TASK,
		.queue_type = TASK_QUEUE_AUX,
		.flag = TASK_FLAG_DELAY_WORK,
		.queue_name = "aux_irq",
		.queue_flag = WQ_HIGHPRI | WQ_CPU_INTENSIVE,
	},
	{
		.name = "timer_irq_task",
		.fn = dptx_timer_irq_process,
		.type = TIMER_IRQ_TASK,
		.queue_type = TASK_QUEUE_TIMER,
		.flag = TASK_FLAG_DELAY_WORK,
		.queue_name = "timer_irq",
		.queue_flag = WQ_HIGHPRI | WQ_CPU_INTENSIVE,
	},
};

/* task init for dptx20 */
static int dptx20_task_init(struct dptx_hw_common *hw_comm)
{
	int i;
	int ret = 0;
	struct dptx20_hw *tx20_hw = NULL;
	char queue_name[QUEUE_NAME_MAX_LEN] = {0};

	if (!hw_comm) {
		DPTX_ERROR("%s NULL hw_comm pointer\n", __func__);
		return -EINVAL;
	}
	tx20_hw = to_dptx20_hw(hw_comm);
	tx20_hw->task_mgr = tx_task_mgr_init();
	if (!tx20_hw->task_mgr)
		return -ENOMEM;

	for (i = 0; i < ARRAY_SIZE(dptx20_task_infos); i++) {
		sprintf(queue_name, "dptx%d_%s", hw_comm->dev_idx, dptx20_task_infos[i].queue_name);
		dptx20_task_infos[i].queue_name = queue_name;
		ret = tx_task_mgr_setup_task(tx20_hw->task_mgr, &dptx20_task_infos[i], hw_comm);
		if (ret)
			DPTX_ERROR("%s setup task[%s] failed\n",
				__func__, dptx20_task_infos[i].name);
	}
	return 0;
}

static int dptx20_task_release(struct dptx_hw_common *hw_comm)
{
	struct dptx20_hw *tx20_hw = NULL;

	if (!hw_comm) {
		DPTX_ERROR("%s NULL hw_comm pointer\n", __func__);
		return -EINVAL;
	}
	tx20_hw = to_dptx20_hw(hw_comm);

	tx_task_mgr_release(tx20_hw->task_mgr);
	return 0;
}

static irqreturn_t dptx_intr_handler(int irq, void *para)
{
	struct dptx_hw_common *hw_comm = (struct dptx_hw_common *)para;
	struct dptx20_hw *tx20_hw = to_dptx20_hw(hw_comm);
	struct meson_hw_event_ops *event_ops = hw_comm->hw_base.event_ops;
	u32 intr_state;
	bool hpd_state;
	u32 intr_mask = dptx20_reg_read(hw_comm, CORE_LEVEL, DPTX20_INTERRUPT_MASK);

	if (!event_ops)
		return IRQ_HANDLED;

	/*
	 * save all the un-handled intr state, clear all interrupts status asap;
	 * clear corresponding saved intr state until it's handled
	 */
	intr_state = dptx20_reg_read(hw_comm, CORE_LEVEL, DPTX20_INTERRUPT_STATE);
	tx20_hw->intr_state |= intr_state;
	/* only handle enabled irq */
	tx20_hw->intr_state &= ~intr_mask;
	/* clear all intr status */
	dptx20_reg_read(hw_comm, CORE_LEVEL, DPTX20_INTERRUPT_CAUSE);

	/* hotplug events in common tx task queue */
	if (tx20_hw->intr_state & INTERRUPT_STATE_HPD_EVENT) {
		tx20_hw->intr_state &= ~INTERRUPT_STATE_HPD_EVENT;
		hpd_state = !!dptx20_reg_read(hw_comm, CORE_LEVEL, DPTX20_HPD_INPUT_STATE);
		if (hpd_state) {
			/* HPD rising */
			event_ops->queue_event(event_ops->data, HPD_PLUGIN, 500);
		} else {
			/* HPD falling */
			event_ops->cancel_event(event_ops->data, HPD_PLUGIN, false);
			event_ops->queue_event(event_ops->data, HPD_PLUGOUT, 0);
		}
	}
	/* hpd_irq in dptx private task queue */
	if (tx20_hw->intr_state & INTERRUPT_STATE_HPD_IRQ) {
		tx20_hw->intr_state &= ~INTERRUPT_STATE_HPD_IRQ;
		event_ops->queue_event(event_ops->data, IRQ_HPD_TASK, 0);
	}
	/* aux irq in dptx private task queue */
	if (tx20_hw->intr_state & (INTERRUPT_STATE_REPLY_RECEIVED | INTERRUPT_STATE_REPLY_TIMEOUT))
		tx_task_mgr_queue_task(tx20_hw->task_mgr, AUX_IRQ_TASK, 0);
	if (tx20_hw->intr_state & (INTERRUPT_STATE_SRC0_OVF_EVENT | INTERRUPT_STATE_SRC0_ERR_EVENT))
		dptx20_video_soft_reset(hw_comm, 0);
	if (tx20_hw->intr_state & (INTERRUPT_STATE_SRC1_OVF_EVENT | INTERRUPT_STATE_SRC1_ERR_EVENT))
		dptx20_video_soft_reset(hw_comm, 1);
	if (tx20_hw->intr_state & (INTERRUPT_STATE_SRC2_OVF_EVENT | INTERRUPT_STATE_SRC2_ERR_EVENT))
		dptx20_video_soft_reset(hw_comm, 2);
	if (tx20_hw->intr_state & (INTERRUPT_STATE_SRC3_OVF_EVENT | INTERRUPT_STATE_SRC3_ERR_EVENT))
		dptx20_video_soft_reset(hw_comm, 3);
	if (tx20_hw->intr_state & (INTERRUPT_STATE_MST_TIMER_EVENT |
		INTERRUPT_STATE_HDCP_TIMER_EVENT | INTERRUPT_STATE_GP_TIMER_EVENT))
		tx_task_mgr_queue_task(tx20_hw->task_mgr, TIMER_IRQ_TASK, 0);
	/* TODO, LPM timer bits undefined */
	return IRQ_HANDLED;
}

static const char *const intr_name[] = {
	"dptx0",
	"dptx1",
	"dptx2",
	"dptx3",
};

/* assume different instance have different irq number */
static int dptx_setup_irq(struct dptx_hw_common *hw_comm)
{
	int ret = 0;

	if (!hw_comm)
		return -1;
	ret = request_irq(hw_comm->irq_id, &dptx_intr_handler,
		IRQF_SHARED, intr_name[hw_comm->dev_idx], (void *)hw_comm);
	return ret;
}

static void dptx_free_irq(struct dptx_hw_common *hw_comm)
{
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
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_DISABLE_SCRAMBLING, scrambler_val);
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_TRAINING_PATTERN_SET, pattern_val);
}

static void dptx_set_phy_link_rate(struct meson_tx_hw *tx_hw, enum dp_link_rate_e link_rate)
{
	if (!tx_hw || !tx_hw->tx_phy)
		return;
	if (link_rate == DPTX_LINK_RATE_UNKNOWN || link_rate == DPTX_LINK_RATE_MAX) {
		DPTX_ERROR("%s invalid link rate: %d\n", __func__, link_rate);
		return;
	}
	/* related to dptx phy implementation, transfer rate to 1620/2700... */
	tx_hw->tx_phy_conf.dp_ops.link_rate = (unsigned int)link_rate;
	meson_tx_phy_configure(tx_hw->tx_phy, &tx_hw->tx_phy_conf);
}

static void dptx_phy_set_lane_count(struct meson_tx_hw *tx_hw, enum dp_lane_count_e lane_ct)
{
	if (!tx_hw || !tx_hw->tx_phy)
		return;
	if (lane_ct == DPTX_LANE_COUNT_UNKNOWN || lane_ct == DPTX_LANE_COUNT_MAX) {
		DPTX_ERROR("%s invalid lane count: %d\n", __func__, lane_ct);
		return;
	}
	/* related to dptx phy implementation */
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
	/* related to dptx phy implementation, replace 4 by lane count */
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
	/* related to dptx phy implementation, replace 4 by lane count */
	for (i = 0; i < 4; i++)
		tx_hw->tx_phy_conf.dp_ops.pre[i] = (unsigned int)pre_emphasis;
	meson_tx_phy_configure(tx_hw->tx_phy, &tx_hw->tx_phy_conf);
}

/* #1.1 init phy instance */
static int dptx_phy_inst_data_init(struct meson_tx_hw *tx_hw)
{
	/* related to dptx phy implementation */
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
	hw_comm->core_info = dptx20_reg_read(hw_comm, CORE_LEVEL, DPTX20_CORE_REVISION);

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
	meson_tx_phy_init(tx_phy);
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
static void dptx_hw_set_link_rate(struct dptx_hw_common *hw_comm, enum dp_link_rate_e link_rate)
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
static void dptx_hw_set_lane_ct(struct dptx_hw_common *hw_comm, enum dp_lane_count_e lane_ct)
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
 * Function: dptx20_hw_init
 *
 * Main function
 */
static int dptx20_hw_init(struct meson_tx_hw *tx_hw)
{
	struct dptx_hw_common *hw_comm = NULL;
	int ret;

	if (!tx_hw)
		return -1;

	DPTX_INFO("%s\n", __func__);
	hw_comm = to_dptx_hw_common(tx_hw);
	hw_comm->aux_transfer = dptx_aux_xfer;
	/* dptx20 device event/task init */
	ret = dptx20_task_init(hw_comm);
	if (ret < 0)
		return ret;
	/* TO CONFIRM: if need to move to dptx_hw_intr_init() */
	dptx_setup_irq(hw_comm);

	dptx_hw_inst_data_init(hw_comm);
	dptx_hw_task_init(hw_comm);
	return 0;
}

static int dptx20_hw_uninit(struct meson_tx_hw *tx_hw)
{
	struct dptx_hw_common *hw_comm = NULL;

	if (!tx_hw)
		return -1;

	DPTX_INFO("%s\n", __func__);
	hw_comm = to_dptx_hw_common(tx_hw);

	dptx_free_irq(hw_comm);
	dptx20_task_release(hw_comm);

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
		DPTX_ERROR("%s cd[%d] not find mapped bit_depth, force 8bit depth\n", __func__, cd);
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
static void dptx20_hw_set_vsc_colorimetry(struct dptx_hw_common *hw_comm,
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
static void dptx20_hw_set_msa(struct dptx_hw_common *hw_comm,
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
	/* Y-only or Raw mode is not default setting, support in the future */
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
	/* 0: VESA (full/limit color)range, 1: CTA range */
	misc |= para->cta_range << 3;
	/* 0: BT.601, 1: BT.709 */
	misc |= para->bt709 << 4;
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
		/* dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_MVID), para->timing->pixel_freq); */

	/*
	 * DP1.4 spec Chapter2.2.3: The Nvid value in this Asynchronous Clock mode shall
	 * be set to 2^15 (= 32768) or higher. A value of power of two should be used.
	 *
	 * In asynchronous mode, this register is read-only and will
	 * return 0x8000. In synchronous mode, this register is read-write
	 * and should be set to the link rate multiplied by 100000
	 */
	/* dptx20_reg_write(hw_comm, CORE_LEVEL, */
		/* dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_NVID), */
		/* para->tx_hw_para.dptx_hw_para.link_rate); */
}

/* config MST_CTRL in DPCD */
static int dptx20_hw_mst_init(struct dptx_hw_common *hw_comm)
{
	u8 dpcd[4] = {0};

	if (!hw_comm)
		return -EINVAL;

	if (hw_comm->mst_en) {
		DPTX_INFO("%s TODO for MST init\n", __func__);
	} else {
		dpcd[0] = 0x0;
		dptx_aux_write_dpcd(hw_comm->tx_aux, DP_MSTM_CTRL, dpcd, 1);
	}
	return 0;
}

/* update VC Payload ID Table, and trigger ACT */
static int dptx20_hw_mst_config(struct dptx_hw_common *hw_comm)
{
	DPTX_INFO("%s TODO for MST update VC payload ID table\n", __func__);
	return 0;
}

static void dptx20_hw_src_vid_enable(struct dptx_hw_common *hw_comm, u8 vc_id, bool enable)
{
	u32 src_en = dptx20_reg_read(hw_comm, CORE_LEVEL, DPTX20_SOURCE_ENABLE);

	if (enable) {
		dptx20_reg_write(hw_comm, CORE_LEVEL,
			dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_USER_CONTROL), 0x8);
		dptx20_reg_write(hw_comm, CORE_LEVEL,
			dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_VIDEO_STREAM_ENABLE), 0x1);
	} else {
		dptx20_reg_write(hw_comm, CORE_LEVEL,
			dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_USER_CONTROL), 0);
		dptx20_reg_write(hw_comm, CORE_LEVEL,
			dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_VIDEO_STREAM_ENABLE), 0);
	}
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_SOURCE_ENABLE, src_en | 0x1 << vc_id);
}

static u32 dptx20_hw_calc_usr_data_ct(u32 hactive, enum colorimetry_format cs,
	u32 bpc, u32 lane_count)
{
	u32 components_per_pixel;
	u32 pixels_per_line;
	u32 pixel_bits_per_line;
	u32 symbols_per_line;

	components_per_pixel = dptx_get_color_component_cnt(cs);
	pixels_per_line = (hactive + lane_count - 1) / lane_count;
	pixel_bits_per_line = pixels_per_line * bpc * components_per_pixel;
	symbols_per_line = (pixel_bits_per_line + 7) / 8;

	return symbols_per_line;
}

/*
 * Program the core for the specified video mode
 */
static int dptx20_hw_set_vid_mode(struct dptx_hw_common *hw_comm,
	struct meson_tx_format_para *para)
{
	/*
	 * set the polarity of the video source sync signals.
	 * Set each bit to a 1 for active high or a 0 for active low.
	 * Note that these polarities are configured in an opposite
	 * state to those carried by the MSA packet
	 */
	u32 sync_polarity, vc_id;
	bool input_odd_even_filed_pol = true;
	bool input_de_pol = true;
	u32 symbols_per_line = 0;

	if (!hw_comm || !para) {
		DPTX_ERROR("%s invalid hw_comm instance or format para\n", __func__);
		return -EINVAL;
	}

	vc_id = para->vc_id;
	sync_polarity = input_odd_even_filed_pol << 3 |
		input_de_pol << 2 |
		!!para->timing.v_pol << 1 |
		!!para->timing.h_pol;

	dptx20_hw_set_msa(hw_comm, vc_id, para);
	dptx20_hw_set_vsc_colorimetry(hw_comm, vc_id, para);
	/* symbols per line */
	symbols_per_line = dptx20_hw_calc_usr_data_ct(para->timing.h_active,
		(enum colorimetry_format)para->cs, dptx_get_mapped_bpc(para->cd),
		para->tx_hw_para.dptx_hw_para.lane_count);
	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_USER_DATA_COUNT),
		symbols_per_line);
	dptx20_reg_write(hw_comm, CORE_LEVEL,
		dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_USER_SYNC_POLARITY), sync_polarity);

	/* Set the configuration of the input interface to the core. */
	/* number of pixels per clock cycle, should be set before the
	 * video source is enabled
	 * 1 for a single pixel wide interface,
	 * 2 for a dual pixel wide interface or
	 * 4 for a quad pixel wide interface.
	 */
	dptx20_reg_write(hw_comm, CORE_LEVEL,
			dptx20_hw_calc_mst_reg(vc_id, DPTX20_SRCX_USER_PIXEL_COUNT), 1);
	if (hw_comm->mst_en) {
		DPTX_ERROR("%s TODO for MST calculate VCPS\n", __func__);
	} else {
		dptx_set_tu_config(hw_comm, vc_id, para->timing.pixel_freq,
			(enum colorimetry_format)para->cs, dptx_get_mapped_bpc(para->cd));
	}
	dptx_set_data_control(hw_comm, vc_id, para);
	dptx20_hw_src_vid_enable(hw_comm, vc_id, true);
	return 0;
}

static int dptx20_hw_core_config(struct dptx_hw_common *hw_comm, struct meson_tx_format_para *para)
{
	dptx20_hw_mst_init(hw_comm);
	dptx20_hw_set_vid_mode(hw_comm, para);
	/* note: set audio mode later from audio source */
	if (hw_comm->mst_en)
		dptx20_hw_mst_config(hw_comm);
	return 0;
}

static int dptx20_hw_cntl_linkconf(struct dptx_hw_common *hw_comm, u32 cmd,
	void *input_argv, void *output_struct)
{
	enum link_pattern_e link_pattern = LINK_PATTERN_OFF;
	int scramble = 0;
	int pattern = 0;
	u8 dpcd[4] = {0};
	u32 val;

	if (!hw_comm)
		return -1;
	if ((cmd & DP_TRAINING_CMD) != DP_TRAINING_CMD) {
		DPTX_ERROR("%s cmd[0x%x] wrong cmd type\n", __func__, cmd);
		return -1;
	}

	switch (cmd) {
	case LINKCONF_INIT_TPS1:
		init_tps1(hw_comm);
		break;
	case LINKCONF_ENABLE_LINK:
		dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_TRANSMITTER_ENABLE,
			*(bool *)input_argv);
		break;
	case LINKCONF_ENABLE_SRC_VID:
		// struct meson_tx_format_para vc_id
		// TODO
		// dptx20_reg_set_bits(NULL, CORE_LEVEL, DPTX20_SRCX_VIDEO_STREAM_ENABLE,
		//	!!(*(int *)input_argv));
		udelay(5);
		dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_FORCE_SCRAMBLER_RESET,
			*(bool *)input_argv);
		break;
	case LINKCONF_ENABLE_ENHANCED_FRAME:
		dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_ENHANCED_FRAMING_ENABLE,
			*(bool *)input_argv);
		break;
	/* Updates DPTX and DPCD with new link rate */
	case LINKCONF_SET_LINK_RATE:
		dptx_hw_set_link_rate(hw_comm, *(enum dp_link_rate_e *)input_argv);
		dpcd[0] = *(enum dp_link_rate_e *)input_argv;
		dptx_aux_write_dpcd(hw_comm->tx_aux, DP_LINK_BW_SET, dpcd, 1);
		break;
	/* Updates DPTX and DPCD with new lane count */
	case LINKCONF_SET_LANE_COUNT:
		val = dptx20_reg_read(hw_comm, CORE_LEVEL, DPTX20_ENHANCED_FRAMING_ENABLE);
		dpcd[0] = 0;
		if (val & 0x1)
			dpcd[0] = DP_LANE_COUNT_ENHANCED_FRAME_EN;
		dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_LANE_COUNT_SET,
			*(enum dp_lane_count_e *)input_argv);
		dptx_phy_set_lane_count(&hw_comm->hw_base, *(enum dp_lane_count_e *)input_argv);
		dpcd[0] |= *(enum dp_lane_count_e *)input_argv;
		dptx_aux_write_dpcd(hw_comm->tx_aux, DP_LANE_COUNT_SET, dpcd, 1);
		break;
	case LINKCONF_SET_DOWNSPREAD:
		dpcd[0] = 0;
		if (*(bool *)input_argv)
			dpcd[0] = DP_SPREAD_AMP_0_5;
		dptx_aux_write_dpcd(hw_comm->tx_aux, DP_DOWNSPREAD_CTRL, dpcd, 1);
		break;
	case LINKCONF_SET_8B10B_CODING:
		dpcd[0] = 0;
		if (*(bool *)input_argv)
			dpcd[0] = DP_SET_ANSI_8B10B;
		dptx_aux_write_dpcd(hw_comm->tx_aux, DP_MAIN_LINK_CHANNEL_CODING_SET, dpcd, 1);
		break;
	case LINKCONF_SET_TRAIN_PATTERN:
		dpcd[0] = 0;
		link_pattern = *(enum link_pattern_e *)input_argv;
		switch (link_pattern) {
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
			pattern = link_pattern & 0x7;
			scramble = 1;
			dpcd[0] = link_pattern;
			break;
		case LINK_PATTERN_4:
			dpcd[0] = 7;
			pattern = 4;
			scramble = 0;
			dpcd[0] = link_pattern;
			break;
		default:
			break;
		}
		dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_DISABLE_SCRAMBLING, scramble);
		dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_TRAINING_PATTERN_SET, pattern);
		dptx_aux_write_dpcd(hw_comm->tx_aux, DP_TRAINING_PATTERN_SET, dpcd, 1);
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
			dptx_aux_write_dpcd(hw_comm->tx_aux, DP_TRAINING_LANE0_SET, dpcd, 4);
		}
		break;
	default:
		break;
	}
	return 0;
}

/* vpu color format convert from YCC to RGB */
static int dptx20_vpu_set_matrix_ycbcr2rgb(struct dptx_hw_common *hw_comm,
	u8 hdmi_if_idx, u8 csc_mode)
{
	if (hdmi_if_idx >= 2) {
		DPTX_ERROR("%s invalid hdmi_if_idx(%d)\n", __func__, hdmi_if_idx);
		return -EINVAL;
	}

	if (csc_mode > 7) {
		DPTX_ERROR("%s not support csc mode(%d)\n", __func__, csc_mode);
		return -EINVAL;
	}

	DPTX_INFO("ycbcr2rgb matrix with mode[%d]\n", csc_mode);
	if (csc_mode == 0) {
		/* 709 conversion; in: yuv/limit; out: rgb/limt---8/255 */
		/* vd1/post */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x1e00 << 16 | 0x1fc0);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET2 + 0x200 * hdmi_if_idx,
			0x1e00);

		/* -512 [1577    1024    0   ]   [Cr] [64]   [R] */
		/* -64  [-469    1024    -188] * [Y ] [64] = [G] */
		/* -512 [0       1024    1858]   [Cb] [64]   [B] */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF00_01 + 0x200 * hdmi_if_idx,
			0x629 << 16 | 0x400);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF02_10 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x1e2b);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF11_12 + 0x200 * hdmi_if_idx,
			0x400 << 16 | 0x1f44);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF20_21 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x400);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF22 + 0x200 * hdmi_if_idx, 0x742);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x40 << 16 | 0x40);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET2 + 0x200 * hdmi_if_idx, 0x40);
	} else if (csc_mode == 1) {
		/* 709 conversion; in: yuv/full; out: rgb/limt---72/255 */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x1e00 << 16 | 0x0);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET2 + 0x200 * hdmi_if_idx,
			0x1e00);

		/* -512 [1385    879     0   ]   [Cr] [64]   [R] */
		/* 0    [-412    879     -165] * [Y ] [64] = [G] */
		/* -512 [0       879     1632]   [Cb] [64]   [B] */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF00_01 + 0x200 * hdmi_if_idx,
			0x569 << 16 | 0x36f);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF02_10 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x1e64);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF11_12 + 0x200 * hdmi_if_idx,
			0x36f << 16 | 0x1f5b);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF20_21 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x36f);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF22 + 0x200 * hdmi_if_idx, 0x660);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x40 << 16 | 0x40);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET2 + 0x200 * hdmi_if_idx, 0x40);
	} else if (csc_mode == 2) {
		/* 709 conversion; in: yuv/limit; out: rgb/full---12/255  */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x1e00 << 16 | 0x1fc0);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET2 + 0x200 * hdmi_if_idx,
			0x1e00);

		/* -512 [1836    1192    0   ]   [Cr] [0]   [R] */
		/* -64  [-546    1192    -218] * [Y ] [0] = [G] */
		/* -512 [0       1192    2163]   [Cb] [0]   [B] */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF00_01 + 0x200 * hdmi_if_idx,
			0x72c << 16 | 0x4a8);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF02_10 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x1dde);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF11_12 + 0x200 * hdmi_if_idx,
			0x4a8 << 16 | 0x1f25);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF20_21 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x4a8);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF22 + 0x200 * hdmi_if_idx, 0x873);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x0);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET2 + 0x200 * hdmi_if_idx, 0x0);
	} else if (csc_mode == 3) {
		/* 709 conversion; in: yuv/full; out: rgb/full---76/255 */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x1e00 << 16 | 0x0);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET2 + 0x200 * hdmi_if_idx,
			0x1e00);

		/* -512 [1613    1024    0   ]   [Cr] [0]   [R] */
		/* 0    [-479    1024    -192] * [Y ] [0] = [G] */
		/* -512 [0       1024    1900]   [Cb] [0]   [B] */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF00_01 + 0x200 * hdmi_if_idx,
			0x64d << 16 | 0x400);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF02_10 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x1e21);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF11_12 + 0x200 * hdmi_if_idx,
			0x400 << 16 | 0x1f40);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF20_21 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x400);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF22 + 0x200 * hdmi_if_idx, 0x76c);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x0);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET2 + 0x200 * hdmi_if_idx, 0x0);
	} else if (csc_mode == 4) {
		/* 601 conversion; in: yuv/limit; out: rgb/limit---25/255 */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x1e00 << 16 | 0x1fc0);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET2 + 0x200 * hdmi_if_idx,
			0x1e00);

		/* -512 [1404    1024    0   ]   [Cr] [64]   [R] */
		/* -64  [-715    1024    -344] * [Y ] [64] = [G] */
		/* -512 [0       1024    1774]   [Cb] [64]   [B] */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF00_01 + 0x200 * hdmi_if_idx,
			0x57c << 16 | 0x400);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF02_10 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x1d35);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF11_12 + 0x200 * hdmi_if_idx,
			0x400 << 16 | 0x1ea8);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF20_21 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x400);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF22 + 0x200 * hdmi_if_idx, 0x6ee);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x40 << 16 | 0x40);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET2 + 0x200 * hdmi_if_idx, 0x40);
	} else if (csc_mode == 5) {
		/* 601 conversion; in: yuv/full; out: rgb/full---93/255  */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x1e00 << 16 | 0x0);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET2 + 0x200 * hdmi_if_idx,
			0x1e00);

		/* -512 [1436    1024    0   ]   [Cr] [0]   [R] */
		/* 0    [-731    1024    -352] * [Y ] [0] = [G] */
		/* -512 [0       1024    1815]   [Cb] [0]   [B] */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF00_01 + 0x200 * hdmi_if_idx,
			0x59c << 16 | 0x400);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF02_10 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x1d25);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF11_12 + 0x200 * hdmi_if_idx,
			0x400 << 16 | 0x1ea0);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF20_21 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x400);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF22 + 0x200 * hdmi_if_idx, 0x717);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x0);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET2 + 0x200 * hdmi_if_idx, 0x0);
	} else if (csc_mode == 6) {
		/* 2020 conversion; in: yuv/limit; out: rgb/limit---42/255 */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x1e00 << 16 | 0x1fc0);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET2 + 0x200 * hdmi_if_idx,
			0x1e00);

		/* -512 [1476    1024    0   ]   [Cr] [64]   [R] */
		/* -64  [-572    1024    -165] * [Y ] [64] = [G] */
		/* -512 [0       1024    1884]   [Cb] [64]   [B] */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF00_01 + 0x200 * hdmi_if_idx,
			0x5c4 << 16 | 0x400);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF02_10 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x1dc4);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF11_12 + 0x200 * hdmi_if_idx,
			0x400 << 16 | 0x1f5b);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF20_21 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x400);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF22 + 0x200 * hdmi_if_idx, 0x75c);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x40 << 16 | 0x40);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET2 + 0x200 * hdmi_if_idx, 0x40);
	} else if (csc_mode == 7) {
		/* 2020 conversion; in: yuv/full; out: rgb/full---110/255 */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x1e00 << 16 | 0x0);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_PRE_OFFSET2 + 0x200 * hdmi_if_idx,
			0x1e00);

		/* -512 [1510    1024    0   ]   [Cr] [0]   [R] */
		/* 0    [-585    1024    -169] * [Y ] [0] = [G] */
		/* -512 [0       1024    1927]   [Cb] [0]   [B] */
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF00_01 + 0x200 * hdmi_if_idx,
			0x5e6 << 16 | 0x400);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF02_10 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x1db7);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF11_12 + 0x200 * hdmi_if_idx,
			0x400 << 16 | 0x1f57);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF20_21 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x400);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_COEF22 + 0x200 * hdmi_if_idx, 0x787);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET0_1 + 0x200 * hdmi_if_idx,
			0x0 << 16 | 0x0);
		dptx20_wr_plt_reg(hw_comm, VPU_HDMI_MATRIX_OFFSET2 + 0x200 * hdmi_if_idx, 0x0);
	}
	return 0;
}

/*
 * Function: dptx20_vpu_fmt_config
 *
 * config VPU_HDMI_IF module including color component
 * remap, csc and dithering
 */
static int dptx20_vpu_fmt_config(struct dptx_hw_common *hw_comm,
	struct meson_tx_format_para *para, u8 venc_idx, u8 hdmi_if_idx)
{
	u32 data32 = 0;
	u8 csc_cov = 0;

	if (!para) {
		DPTX_INFO("%s invalid format para\n", __func__);
		return -EINVAL;
	}

	if (hdmi_if_idx >= 2) {
		DPTX_INFO("%s invalid hdmi_if_idx(%d)\n", __func__, hdmi_if_idx);
		return -EINVAL;
	}

	/* disable venc data to HDMI_IF */
	dptx20_set_plt_reg_bits(hw_comm, VPU_HDMI_SETTING + 0x200 * hdmi_if_idx, 0, venc_idx, 1);

	/* step1: VPU CSC, ycc->rgb convert with limit range by default */
	if (para->cs == HDMI_COLORSPACE_RGB)
		dptx20_vpu_set_matrix_ycbcr2rgb(hw_comm, hdmi_if_idx, 0);
	/*
	 * [ 1: 0] hdmi_vid_fmt. 0=444; 1=convert to 422; 2=convert to 420.3=convert to rgb
	 * [ 3: 2] chroma_dnsmp_h. 0=use pixel 0; 1=use pixel 1; 2=use average.
	 * [    4] dith_en. 1=enable dithering before HDMI TX input.
	 * [    5] hdmi_dith_md: random noise selector.
	 * [ 9: 6] hdmi_dith10_cntl.
	 * [   10] hdmi_round_en. 1= enable 12-b rounded to 10-b.
	 * [   11] tunnel_en
	 * [21:12] hdmi_dith_new
	 * [23:22] chroma_dnsmp_v. 0=use line 0; 1=use line 1; 2=use average.
	 * [27:24] pix_repeat
	 * [31] 1 = enable(yuv2rgb or rgb2yuv);
	 */
	if (para->cs == HDMI_COLORSPACE_YUV420)
		csc_cov = 2;
	else if (para->cs == HDMI_COLORSPACE_YUV422)
		csc_cov = 1;
	else if (para->cs == HDMI_COLORSPACE_RGB)
		csc_cov = 3;
	else
		csc_cov = 0;
	data32 = 0;
	data32 = (csc_cov & 0x3) << 0 |
		  2 << 2 |
		  0 << 4 |
		  0 << 5 |
		  0 << 6 |
		  /* 12 bit to 10 bit round enable */
		  ((para->cd == COLORDEPTH_24B) ? 1 : 0) << 10 |
		  0 << 11 |
		  0 << 12 |
		  2 << 22 |
		  0 << 24 |
		  ((para->cs == HDMI_COLORSPACE_RGB) ? 1 : 0) << 31;
	dptx20_wr_plt_reg(hw_comm, VPU_HDMI_FMT_CTRL + 0x200 * hdmi_if_idx, data32);

	/*
	 * step2: VPU dithering
	 * [    0] dither lut sel:  1 : sel 10b to 8b	0: sel 12b to 10
	 * [    2] inv_hsync_b
	 * [    3] inv_vsync_b
	 * [    4] hdmi_dith_en_b. For 10-b to 8-b.
	 * [    5] hdmi_dith_md_b. For 10-b to 8-b.
	 * [ 9: 6] hdmi_dith10_b. For 10-b to 8-b.
	 * [   10] hdmi_round_en_b. For 10-b to 8-b.
	 * [21:12] hdmi_dith_new_b. For 10-b to 8-b.
	 */
	data32 = 0;
	data32 = 0 << 2 |
		0 << 3 |
		/* 10b to 8b rounding enable */
		(para->cd == COLORDEPTH_24B ? 1 : 0) << 4 |
		0 << 5 |
		0 << 6 |
		/* dither 10-b to 8-b enable */
		((para->cd == COLORDEPTH_24B) ? 1 : 0) << 10 |
		0 << 12;
	dptx20_wr_plt_reg(hw_comm, VPU_HDMI_DITH_CNTL + 0x200 * hdmi_if_idx, data32);

	/*
	 * step3: VPU decouple FIFO data sample(ENCP-->HDMI clk domain)
	 * and color component remap before CSC and before dithering
	 * [    0] src_sel_enc0
	 * [    1] src_sel_enc1
	 * [    2] inv_hsync. 1=Invert Hsync polarity.
	 * [    3] inv_vsync. 1=Invert Vsync polarity.
	 * [    4] inv_dvi_clk. 1=Invert clock to external DVI
	 * [ 7: 5] comp_map_post. Input data is CrYCb(BRG), map the output data to desired format:
	 * 0=output CrYCb(BRG);
	 * 1=output YCbCr(RGB);
	 * 2=output YCrCb(RBG);
	 * 3=output CbCrY(GBR);
	 * 4=output CbYCr(GRB);
	 * 5=output CrCbY(BGR);
	 * 6,7=Rsrv.
	 * [11: 8] wr_rate_pre. 0=A write every clk1; 1=A write every 2 clk1; ...
	 * [15:12] rd_rate_pre. 0=A read every clk2; 1=A read every 2 clk2; ...
	 * [18:16] comp_map_pre
	 * [23:20] wr_rate_post. 0=A write every clk1; 1=A write every 2 clk1; ...
	 * [27:24] rd_rate_post. 0=A read every clk2; 1=A read every 2 clk2
	 */
	data32 = 0;
	/* select ENCP to VPU_HDMI_IF */
	data32 |= 0 << 0;
	data32 |= 0 << 1;
	/* invert H/Vsync polarity */
	data32 |= para->timing.h_pol << 2;
	data32 |= para->timing.v_pol << 3;
	/* comp_map_post */
	data32 |= ((para->cs == HDMI_COLORSPACE_YUV420) ? 4 : 0) << 5;
	/* comp_map_pre, input is YCbCr(RGB), if output colorspace is YCC,
	 * map to CrYCb(BRG) firstly
	 */
	data32 |= ((para->cs == HDMI_COLORSPACE_RGB) ? 0 : 3) << 16;
	/* One write every 2 hdmi_fe_clk if Y420 mode */
	data32 |= ((para->cs == HDMI_COLORSPACE_YUV420) ? 1 : 0) << 20;
	dptx20_wr_plt_reg(hw_comm, VPU_HDMI_SETTING + 0x200 * hdmi_if_idx, data32);

	/* select venc data to HDMI_IF */
	dptx20_set_plt_reg_bits(hw_comm, VPU_HDMI_SETTING + 0x200 * hdmi_if_idx, 1, venc_idx, 1);

	return 0;
}

static int dptx20_pre_enable_mode(struct dptx_hw_common *hw_comm, struct meson_tx_format_para *para)
{
	/*
	 * set clock divider for generating the internal 1MHz
	 * clock from the APB host interface clock
	 * APB clk/divider = 200MHz / 200 = 1Mhz
	 */
	dptx20_reg_write(hw_comm, CORE_LEVEL, DPTX20_AUX_CLOCK_DIVIDER, 0xC8);

	DPTX_INFO("%s %px done\n", __func__, hw_comm);

	return 0;
}

static int dptx20_enable_mode(struct dptx_hw_common *hw_comm, struct meson_tx_format_para *para)
{
	u8 hdmi_if_idx = 0;
	u8 venc_idx = 0;
	/* enable all related CLK by default */
	u32 clk_mask = PIXEL_PLL_MASK | PIXEL_CLK_MASK | VENC_CLK_MASK | PHY_CLK_MASK;
	struct meson_tx_clk *tx_clk = hw_comm->hw_base.tx_clk;

	if (!para) {
		DPTX_INFO("%s invalid format para\n", __func__);
		return -EINVAL;
	}
	if (!tx_clk) {
		DPTX_INFO("%s NULL tx_clk instance\n", __func__);
		return -EINVAL;
	}
	meson_tx_clk_set(tx_clk, clk_mask);
	/* venc config, skip here as it will be done in drm side */
	hdmi_if_idx = (tx_clk->tx_clk_cfg.clk_div_path >> 4) & 0x1;
	venc_idx = tx_clk->tx_clk_cfg.clk_div_path & 0x1;
	dptx20_vpu_fmt_config(hw_comm, para, venc_idx, hdmi_if_idx);
	dptx20_hw_core_config(hw_comm, para);
	DPTX_INFO("%s %px done\n", __func__, hw_comm);

	return 0;
}

static int dptx20_post_enable_mode(struct dptx_hw_common *tx_comm)
{
	DPTX_INFO("%s TODO for hdcp auth\n", __func__);

	return 0;
}

static int dptx20_hw_cntl_flow_misc(struct meson_tx_hw *tx_hw, u32 cmd,
				  void *input_argv, void *output_struct)
{
	struct dptx_hw_common *hw_comm = to_dptx_hw_common(tx_hw);
	int ret = 0;

	switch (cmd) {
	case MODE_FLOW_PRE_ENABLE_MODE:
		return dptx20_pre_enable_mode(hw_comm, (struct meson_tx_format_para *)input_argv);
	case MODE_FLOW_ENABLE_MODE:
		return dptx20_enable_mode(hw_comm, (struct meson_tx_format_para *)input_argv);
	case MODE_FLOW_POST_ENABLE_MODE:
		return dptx20_post_enable_mode(hw_comm);
	default:
		break;
	}

	return ret;
}

static int dptx20_hw_cntl_platform(struct meson_tx_hw *tx_hw, u32 cmd,
				  void *input_argv, void *output_struct)
{
	struct dptx_hw_common *hw_comm = to_dptx_hw_common(tx_hw);
	int ret = 0;

	if (!tx_hw)
		return -1;

	if ((cmd & CMD_TYPE_MASK) != CMD_PLATFORM_CNTL_OFFSET) {
		DPTX_ERROR("%s cmd[0x%x] wrong cmd type\n", __func__, cmd);
		return -1;
	}

	switch (cmd) {
	case PLATFORM_VID_CLK_PARAM_BUILD:
		if (!input_argv) {
			DPTX_ERROR("%s cmd[0x%x] null input arg\n", __func__, cmd);
			ret = -1;
		} else {
			struct meson_tx_format_para *fmt_para =
				(struct meson_tx_format_para *)input_argv;
			struct meson_tx_clk *tx_clk = tx_hw->tx_clk;

			if (!tx_clk) {
				ret = -1;
				break;
			}
			tx_clk->tx_clk_cfg.clk_div_path = fmt_para->vid_clk_path;
			DPTX_INFO("%s TODO build clk param, depends on PLL\n", __func__);
		}
		break;
	case PLATFORM_GET_HPD_GPI_ST:
		if (!output_struct)
			return -1;
		ret = dptx20_reg_read(hw_comm, CORE_LEVEL, DPTX20_HPD_INPUT_STATE);
		*(int *)output_struct = ret;
		break;
	default:
		break;
	}

	return ret;
}

static int dptx20_hw_cntl_aux_pkt(struct meson_tx_hw *tx_hw, u32 cmd,
				  void *input_argv, void *output_struct)
{
	struct dptx_hw_common *hw_comm = to_dptx_hw_common(tx_hw);
	int ret = 0;
	struct aux_pkt_sdp_param *param;

	if (!hw_comm)
		return -1;
	if ((cmd & CMD_AUX_PKT_OFFSET) != CMD_AUX_PKT_OFFSET) {
		DPTX_ERROR("%s cmd[0x%x] wrong cmd type\n", __func__, cmd);
		return -1;
	}

	switch (cmd) {
	case AUX_PKT_SET_SDP:
		param = (struct aux_pkt_sdp_param *)input_argv;
		dptx_hw_infoframe_send(hw_comm, param->vc_id, param->type, param->data);
		break;
	case AUX_PKT_GET_SDP:
		param = (struct aux_pkt_sdp_param *)input_argv;
		ret = dptx_hw_infoframe_raw_get(hw_comm, param->vc_id, param->type,
			(u8 *)output_struct);
		break;
	default:
		break;
	}
	return ret;
}

static void hw_timer_start(struct dptx_hw_common *hw_comm, enum timer_hw_type hw_type,
	struct timer_config *cfg)
{
	if (!hw_comm || !cfg)
		return;

	switch (cfg->wait_type) {
	case TIMER_WAIT:
		hw_timer_init(hw_comm, hw_type);
		hw_timer_poll_wait_us(hw_comm, hw_type, cfg->us);
		break;
	case TIMER_ISR:
		hw_timer_init(hw_comm, hw_type);
		hw_timer_interrupt(hw_comm, hw_type, true);
		hw_timer_set_us(hw_comm, hw_type, cfg->us);
		hw_timer_enable(hw_comm, hw_type, true);
		break;
	case TIMER_REPEAT_ISR:
		hw_timer_init(hw_comm, hw_type);
		hw_timer_auto_reload(hw_comm, hw_type, true);
		hw_timer_interrupt(hw_comm, hw_type, true);
		hw_timer_set_us(hw_comm, hw_type, cfg->us);
		hw_timer_enable(hw_comm, hw_type, true);
		break;
	default:
		break;
	}
}

static int dptx20_hw_cntl_core_misc(struct meson_tx_hw *tx_hw, u32 cmd,
				  void *input_argv, void *output_struct)
{
	struct dptx_hw_common *hw_comm = to_dptx_hw_common(tx_hw);
	enum timer_hw_type hw_type = HW_GP_TIMER;
	struct dptx_timer_manager *timer_manager = NULL;
	struct dptx_timer_handler *handler = NULL;
	enum timer_hw_type type = HW_GP_TIMER;
	u32 size;

	if (!hw_comm)
		return -1;

	timer_manager = hw_comm->timer_manager;

	if ((cmd & CMD_TYPE_MASK) != CMD_CORE_MISC_OFFSET) {
		DPTX_ERROR("%s cmd[0x%x] wrong cmd type\n", __func__, cmd);
		return -1;
	}

	switch (cmd) {
	case DP_TIMER_INIT:
		timer_manager = kzalloc(sizeof(*hw_comm->timer_manager), GFP_KERNEL);
		if (!timer_manager)
			return -1;
		hw_comm->timer_manager = timer_manager;
		timer_manager->timers_num = HW_TIMER_MAX;
		size = sizeof(*timer_manager->handlers) * HW_TIMER_MAX;
		timer_manager->handlers = kzalloc(size, GFP_KERNEL);
		for (type = HW_GP_TIMER; type < HW_TIMER_MAX; type++)
			mutex_init(&timer_manager->handlers[type].timer_mutex);
		break;
	case DP_TIMER_GET:
		if (!input_argv || !output_struct)
			return -1;
		hw_type = *(enum timer_hw_type *)input_argv;
		if (hw_type > HW_TIMER_MAX)
			return -1;
		*(struct dptx_timer_handler *)output_struct = timer_manager->handlers[hw_type];
		break;
	case DP_TIMER_START:
		handler = (struct dptx_timer_handler *)input_argv;
		if (handler->cfg.timer_type >= HW_TIMER_MAX)
			return -1;
		hw_timer_start(hw_comm, hw_type, &handler->cfg);
		break;
	case DP_TIMER_STOP:
		handler = (struct dptx_timer_handler *)input_argv;
		if (handler->cfg.timer_type >= HW_TIMER_MAX)
			return -1;
		hw_timer_init(hw_comm, handler->cfg.timer_type);
		break;
	default:
		break;
	}

	return 0;
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
	case CMD_LINK_TRAINING_OFFSET:
		ret = dptx20_hw_cntl_linkconf(hw_comm, cmd, input_argv, output_struct);
		break;
	case CMD_MODE_FLOW_MISC_OFFSET:
		ret = dptx20_hw_cntl_flow_misc(tx_hw, cmd, input_argv, output_struct);
		break;
	case CMD_PLATFORM_CNTL_OFFSET:
		ret = dptx20_hw_cntl_platform(tx_hw, cmd, input_argv, output_struct);
		break;
	case CMD_AUX_PKT_OFFSET:
		ret = dptx20_hw_cntl_aux_pkt(tx_hw, cmd, input_argv, output_struct);
		break;
	case CMD_CORE_MISC_OFFSET:
		ret = dptx20_hw_cntl_core_misc(tx_hw, cmd, input_argv, output_struct);
		break;
	default:
		break;
	}
	return ret;
}

void dptx20_free_tx_hw(struct meson_tx_hw *tx_hw)
{
	struct dptx_hw_common *hw_comm = NULL;
	struct dptx20_hw *tx20_hw = NULL;

	if (!tx_hw)
		return;

	hw_comm = to_dptx_hw_common(tx_hw);
	tx20_hw = to_dptx20_hw(hw_comm);
	kfree(tx20_hw);
	tx20_hw = NULL;
}

struct meson_tx_hw *dptx20_alloc_tx_hw(void)
{
	struct dptx20_hw *tx20_hw = kzalloc(sizeof(*tx20_hw), GFP_KERNEL);
	struct dptx_hw_common *hw_comm = NULL;

	if (!tx20_hw) {
		DPTX_ERROR("%s failed to alloc tx_hw instance\n", __func__);
		return NULL;
	}
	hw_comm = &tx20_hw->hw_comm;

	hw_comm->hw_base.hw_cntl = dptx20_hw_cntl;
	hw_comm->hw_base.init_tx_hw = dptx20_hw_init;
	hw_comm->hw_base.uninit_tx_hw = dptx20_hw_uninit;

	return &hw_comm->hw_base;
}

