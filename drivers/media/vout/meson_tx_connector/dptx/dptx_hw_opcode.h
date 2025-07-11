/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPTX_HW_OPCODE_H
#define __DPTX_HW_OPCODE_H

#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_hw_opcode.h>

/* special offset for DPTX */
#define CMD_DPTX_OFFSET				(0x1 << 20)

/* 0x10 DDC */
enum cmd_dptx_aux {
	/* dptx ddc start */
	DP_AUX_CMD = CMD_DDC_AUX_OFFSET + CMD_DPTX_OFFSET,
	DP_AUX_PIN_MUX_OP,
};

/* 0x11 HDCP */
enum cmd_dptx_hdcp {
	/* dptx hdcp start */
	DP_HDCP_CMD = CMD_HDCP_OFFSET + CMD_DPTX_OFFSET,
};

/* 0x12 packet */
enum cmd_dptx_pkt {
	/* dp aux pkt start */
	DP_AUX_PKT_CMD = CMD_AUX_PKT_OFFSET + CMD_DPTX_OFFSET,
};

/* 0x13 audio */
enum cmd_dptx_aud {
	/* dpts aud start */
	DP_AUDIO_CMD = CMD_AUX_AUD_OFFSET + CMD_DPTX_OFFSET,
};

/* 0x16 DSC */
enum cmd_dptx_dsc {
	/* dptx dsc start */
	DP_DSC_CMD = CMD_DSC_OFFSET + CMD_DPTX_OFFSET,
};

/* 0x18 core misc */
enum cmd_dptx_core_misc {
	/* dptx start */
	DP_CORE_MISC_CMD = CMD_CORE_MISC_OFFSET + CMD_DPTX_OFFSET,
};

/* 0x19 CMD_PLATFORM */
enum cmd_dptx_platform {
	/* dptx start */
	DP_PLATFORM_CNTL_CMD = CMD_PLATFORM_CNTL_OFFSET + CMD_DPTX_OFFSET,
};

/* 0x1b mode flow misc */
enum cmd_dptx_mode_flow {
	DP_MODE_FLOW_CMD = CMD_MODE_FLOW_MISC_OFFSET + CMD_DPTX_OFFSET,
};

/* 0x1c dptx link training */
enum cmd_link_training {
	/* dptx training start */
	DP_TRAINING_CMD = CMD_LINK_TRAINING + CMD_DPTX_OFFSET,
	DP_AUX_LINKCONF_OFFSET,
	LINKCONF_INIT_TPS1,
	LINKCONF_ENABLE_LINK,
	LINKCONF_ENABLE_SRC_VID,
	LINKCONF_ENABLE_ENHANCED_FRAME,
	LINKCONF_SET_LINK_RATE,
	LINKCONF_SET_LANE_COUNT,
	LINKCONF_SET_DOWNSPREAD,
	LINKCONF_SET_8B10B_CODING,
	LINKCONF_SET_TRAIN_PATTERN,
	LINKCONF_SET_VSWING_PREEMP,
};

#endif
