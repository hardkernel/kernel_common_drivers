/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_TX_LOG_H
#define __MESON_TX_LOG_H

#define TX_NAME "meson_tx"

struct va_format;

enum tx_debug_category {
	CORE_LOG = 0x01,
	VIDEO_LOG = 0x02,
	AUDIO_LOG = 0x04,
	HDCP_LOG = 0x08,
	PACKET_LOG = 0x10,
	EDID_LOG = 0x20,
	PHY_LOG = 0x40,
	REG_LOG = 0x80,
	SCDC_LOG = 0x100,
	VINFO_LOG = 0x200,
	EVENT_LOG = 0x400,
	HPD_LOG = 0x800,
	DSC_LOG = 0x1000,
	QMS_LOG = 0x2000,
	DPCD_LOG = 0x4000,
};

enum tx_print_level {
	ERROR_LEVEL,
	WARN_LEVEL,
	DEBUG_LEVEL,
	INFO_LEVEL,
};

struct meson_tx_log {
	const char *driver_tag;
	const char *category_tag;
	unsigned int category;
	unsigned int debug;
	unsigned int print_level;
};

#define TX_ERROR(log, fmt, ...)							\
	meson_tx_err(log, fmt, ##__VA_ARGS__)

#define TX_WARNING(log, fmt, ...)						\
	meson_tx_warn(log, fmt, ##__VA_ARGS__)

#define TX_DEBUG(log, fmt, ...)							\
	meson_tx_dbg(log, CORE_LOG, fmt, ##__VA_ARGS__)

#define TX_DEBUG_EVENT(log, fmt, ...)					\
	meson_tx_dbg(log, EVENT_LOG, fmt, ##__VA_ARGS__)

#define TX_INFO(log, fmt, ...)							\
	meson_tx_info(log, fmt, ##__VA_ARGS__)

/*DONT USE API directly, use macro define instead.*/
void meson_tx_info(struct meson_tx_log *log, const char *format, ...);
void meson_tx_err(struct meson_tx_log *log, const char *format, ...);
void meson_tx_warn(struct meson_tx_log *log, const char *format, ...);
void meson_tx_dbg(struct meson_tx_log *log, int category, const char *format, ...);

void __meson_tx_print(struct meson_tx_log *log, struct va_format *vaf);
void meson_tx_log_init(struct meson_tx_log *log, const char *device_name);

#endif
