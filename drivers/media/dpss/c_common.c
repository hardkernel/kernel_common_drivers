// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include "sys_def.h"
#ifdef RUN_ON_ARM
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#endif

unsigned int code_count(void *p)
{
	unsigned long addr, addr_cp;
	unsigned int size_long;
	int i;
	unsigned int pos = 0;
	unsigned int code;

	addr = (unsigned long)p;
	addr_cp = addr;
	size_long = sizeof(unsigned long);
	//find non 0
	for (i = 0; i < (size_long << 1) ; i++) {
		if (addr_cp & 0xf) {
			pos = i;
			break;
		}
		addr_cp = addr_cp >> 4;
	}
	code = (addr_cp & 0xffff) | (pos << 24);
	return code;
}

bool code_check(void *p, unsigned int code)
{
	unsigned long addr;
	unsigned int pos;

	pos = (code >> 24) & 0xf;
	addr = (unsigned long)p;

	if ((addr >> (pos << 2)) == (code & 0xffff))
		return true;

	return false;
}

