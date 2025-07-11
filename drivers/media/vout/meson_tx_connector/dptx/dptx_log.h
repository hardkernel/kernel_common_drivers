/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPTX_LOG_H
#define __DPTX_LOG_H

#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_log.h>

#define DPTX_ERROR(fmt, ...)							\
	__dptx_err(fmt, ##__VA_ARGS__)

#define DPTX_WARNING(fmt, ...)						\
	__dptx_warn(fmt, ##__VA_ARGS__)

#define DPTX_DEBUG(fmt, ...)							\
	__dptx_dbg(CORE_LOG, fmt, ##__VA_ARGS__)

#define DPTX_DEBUG_VIDEO(fmt, ...)					\
	__dptx_dbg(VIDEO_LOG, fmt, ##__VA_ARGS__)

#define DPTX_DEBUG_AUDIO(fmt, ...)					\
	__dptx_dbg(AUDIO_LOG, fmt, ##__VA_ARGS__)

#define DPTX_DEBUG_HDCP(fmt, ...)						\
	__dptx_dbg(HDCP_LOG, "hdcptx: " fmt, ##__VA_ARGS__)

#define DPTX_DEBUG_PACKET(fmt, ...)					\
	__dptx_dbg(PACKET_LOG, fmt, ##__VA_ARGS__)

#define DPTX_DEBUG_EDID(fmt, ...)						\
	__dptx_dbg(EDID_LOG, fmt, ##__VA_ARGS__)

#define DPTX_DEBUG_PHY(fmt, ...)						\
	__dptx_dbg(PHY_LOG, fmt, ##__VA_ARGS__)

#define DPTX_DEBUG_REG(fmt, ...)						\
	__dptx_dbg(REG_LOG, fmt, ##__VA_ARGS__)

#define DPTX_DEBUG_DPCD(fmt, ...)						\
	__dptx_dbg(DPCD_LOG, fmt, ##__VA_ARGS__)

#define DPTX_DEBUG_VINFO(fmt, ...)					\
	__dptx_dbg(VINFO_LOG, fmt, ##__VA_ARGS__)

#define DPTX_DEBUG_EVENT(fmt, ...)					\
	__dptx_dbg(EVENT_LOG, fmt, ##__VA_ARGS__)

#define DPTX_DEBUG_HPD(fmt, ...)						\
	__dptx_dbg(HPD_LOG, fmt, ##__VA_ARGS__)

#define DPTX_DEBUG_DSC(fmt, ...)						\
	__dptx_dbg(DSC_LOG, fmt, ##__VA_ARGS__)

#define DPTX_INFO(fmt, ...)							\
	__dptx_info(fmt, ##__VA_ARGS__)

#define DPTX_HDCP_INFO(fmt, ...)							\
	__dptx_info("hdcptx: " fmt, ##__VA_ARGS__)

/*DONT USE API directly, use macro define instead.*/
void __dptx_info(const char *format, ...);
void __dptx_err(const char *format, ...);
void __dptx_warn(const char *format, ...);
void __dptx_dbg(enum tx_debug_category category, const char *format, ...);

#endif
