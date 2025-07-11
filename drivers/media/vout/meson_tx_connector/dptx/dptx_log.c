// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/amlogic/gki_module.h>

#include "dptx_log.h"

#define DPTX_NAME		"dptx"

unsigned int __dptx_debug;
EXPORT_SYMBOL(__dptx_debug);

MODULE_PARM_DESC(dptx_debug, "Enable debug output, where each bit enables a debug category.\n"
"\t\tBit 0 (0x01)  will enable CORE messages (dptx core code)\n"
"\t\tBit 1 (0x02)  will enable VIDEO messages (dptx video)\n"
"\t\tBit 2 (0x04)  will enable AUDIO messages (dptx audio)\n"
"\t\tBit 3 (0x08)  will enable HDCP messages (dptx hdcp)\n"
"\t\tBit 4 (0x10)  will enable PACKET messages (dptx packet)\n"
"\t\tBit 5 (0x20)  will enable EDID messages (dptx edid parse)\n"
"\t\tBit 6 (0x40)  will enable PHY messages (dptx phy control)\n"
"\t\tBit 7 (0x80)  will enable REG messages (dptx register rd/wr)\n"
"\t\tBit 8 (0x100)  will enable DPCD messages (dptx dpcd)\n"
"\t\tBit 9 (0x200)  will enable VINFO messages (dptx vinfo)\n"
"\t\tBit 10 (0x400)  will enable EVENT messages (dptx event)\n"
"\t\tBit 11 (0x800)  will enable HPD messages (dptx HPD)\n"
"\t\tBit 12 (0x1000)  will enable DSC messages (dptx DSC)");

module_param_named(dptx_debug, __dptx_debug, int, 0600);

void __dptx_info(const char *format, ...)
{
	struct va_format vaf;
	va_list args;
	struct meson_tx_log tx_log = {
		.driver_tag = DPTX_NAME,
		.category = __dptx_debug,
		.debug = __dptx_debug,
		.print_level = INFO_LEVEL,
	};

	va_start(args, format);
	vaf.fmt = format;
	vaf.va = &args;
	__meson_tx_print(&tx_log, &vaf);
	va_end(args);
}

void __dptx_err(const char *format, ...)
{
	struct va_format vaf;
	va_list args;
	struct meson_tx_log tx_log = {
		.driver_tag = DPTX_NAME,
		.category = __dptx_debug,
		.debug = __dptx_debug,
		.print_level = ERROR_LEVEL,
	};

	va_start(args, format);
	vaf.fmt = format;
	vaf.va = &args;
	__meson_tx_print(&tx_log, &vaf);
	va_end(args);
}

void __dptx_warn(const char *format, ...)
{
	struct va_format vaf;
	va_list args;
	struct meson_tx_log tx_log = {
		.driver_tag = DPTX_NAME,
		.category = __dptx_debug,
		.debug = __dptx_debug,
		.print_level = WARN_LEVEL,
	};

	va_start(args, format);
	vaf.fmt = format;
	vaf.va = &args;
	__meson_tx_print(&tx_log, &vaf);
	va_end(args);
}

void __dptx_dbg(enum tx_debug_category category, const char *format, ...)
{
	struct va_format vaf;
	va_list args;
	struct meson_tx_log tx_log = {
		.driver_tag = DPTX_NAME,
		.category = category,
		.debug = __dptx_debug,
		.print_level = DEBUG_LEVEL,
	};

	va_start(args, format);
	vaf.fmt = format;
	vaf.va = &args;
	__meson_tx_print(&tx_log, &vaf);
	va_end(args);
}

static int dptx_config_debug_level(char *str)
{
	int err;
	u32 value;

	if (!str) {
		__dptx_debug = 0;
		return -EINVAL;
	}

	err = kstrtou32(str, 16, &value);
	if (err) {
		DPTX_ERROR("%s fail\n", __func__);
		__dptx_debug = 0;
		return err;
	}
	__dptx_debug = value;

	return 1;
}
__setup("dptx_debug_level=", dptx_config_debug_level);
