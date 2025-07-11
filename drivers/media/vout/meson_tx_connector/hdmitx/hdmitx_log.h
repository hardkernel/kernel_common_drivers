/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMITX_LOG_H
#define __HDMITX_LOG_H

#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_log.h>

#define HDMITX_ERROR(fmt, ...)							\
	__hdmitx_err(fmt, ##__VA_ARGS__)

#define HDMITX_WARNING(fmt, ...)						\
	__hdmitx_warn(fmt, ##__VA_ARGS__)

#define HDMITX_DEBUG(fmt, ...)							\
	__hdmitx_dbg(CORE_LOG, fmt, ##__VA_ARGS__)

#define HDMITX_DEBUG_VIDEO(fmt, ...)					\
	__hdmitx_dbg(VIDEO_LOG, fmt, ##__VA_ARGS__)

#define HDMITX_DEBUG_AUDIO(fmt, ...)					\
	__hdmitx_dbg(AUDIO_LOG, fmt, ##__VA_ARGS__)

#define HDMITX_DEBUG_HDCP(fmt, ...)						\
	__hdmitx_dbg(HDCP_LOG, "hdcptx: " fmt, ##__VA_ARGS__)

#define HDMITX_DEBUG_PACKET(fmt, ...)					\
	__hdmitx_dbg(PACKET_LOG, fmt, ##__VA_ARGS__)

#define HDMITX_DEBUG_EDID(fmt, ...)						\
	__hdmitx_dbg(EDID_LOG, fmt, ##__VA_ARGS__)

#define HDMITX_DEBUG_PHY(fmt, ...)						\
	__hdmitx_dbg(PHY_LOG, fmt, ##__VA_ARGS__)

#define HDMITX_DEBUG_REG(fmt, ...)						\
	__hdmitx_dbg(REG_LOG, fmt, ##__VA_ARGS__)

#define HDMITX_DEBUG_SCDC(fmt, ...)						\
	__hdmitx_dbg(SCDC_LOG, fmt, ##__VA_ARGS__)

#define HDMITX_DEBUG_VINFO(fmt, ...)					\
	__hdmitx_dbg(VINFO_LOG, fmt, ##__VA_ARGS__)

#define HDMITX_DEBUG_EVENT(fmt, ...)					\
	__hdmitx_dbg(EVENT_LOG, fmt, ##__VA_ARGS__)

#define HDMITX_DEBUG_HPD(fmt, ...)						\
	__hdmitx_dbg(HPD_LOG, fmt, ##__VA_ARGS__)

#define HDMITX_DEBUG_DSC(fmt, ...)						\
	__hdmitx_dbg(DSC_LOG, fmt, ##__VA_ARGS__)

#define HDMITX_DEBUG_QMS(fmt, ...)						\
	__hdmitx_dbg(QMS_LOG, "QMS: " fmt, ##__VA_ARGS__)

#define HDMITX_INFO(fmt, ...)							\
	__hdmitx_info(fmt, ##__VA_ARGS__)

#define HDMITX_HDCP_INFO(fmt, ...)							\
	__hdmitx_info("hdcptx: " fmt, ##__VA_ARGS__)

/*DONT USE API directly, use macro define instead.*/
void __hdmitx_info(const char *format, ...);
void __hdmitx_err(const char *format, ...);
void __hdmitx_warn(const char *format, ...);
void __hdmitx_dbg(enum tx_debug_category category, const char *format, ...);

#endif
