// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "videodisplay_util.h"

static u32 print_flag;
int set_print_flag(u32 value)
{
	print_flag = value;
	return 0;
}

u32 get_print_flag(void)
{
	return print_flag;
}

int vd_print(int index, int debug_flag, const char *fmt, ...)
{
	if ((print_flag & debug_flag) ||
	    debug_flag == PRINT_ERROR) {
		unsigned char buf[256];
		int len = 0;
		va_list args;

		va_start(args, fmt);
		len = sprintf(buf, "vd:[%d]", index);
		vsnprintf(buf + len, 256 - len, fmt, args);
		pr_info("%s", buf);
		va_end(args);
	}
	return 0;
}
