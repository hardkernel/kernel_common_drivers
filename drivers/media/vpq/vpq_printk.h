/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __VPQ_PRINTK_H__
#define __VPQ_PRINTK_H__

#include <linux/kernel.h>

extern unsigned int log_lev;

#define ENABLE_USAGE_STR (0) // 1:printk vpq_debug.c xxx_usage_str
#define OPEN_PRINTK      (0) // 1:printk by pr_inf

#define lev_dbg          (0) // log level of debug layer
#define lev_ioc          (1) // log level of ioctl layer
#define lev_tab          (2) // log level of table logic layer
#define lev_mod          (3) // log level of modules layer
#define lev_vfm          (4) // log level of vfm layer
#define lev_proc         (5) // log level of processor layer

#if OPEN_PRINTK
#define pr_inf(lev, fmt, args...)\
	do {\
		if ((lev) <= log_lev)\
			pr_info("vpq: %s " fmt, __func__, ## args);\
	} while (0)

#else
#define pr_inf(lev, fmt, args...)
#endif

#define pr_error(fmt, args ...)   pr_err("vpq: %s " fmt, __func__, ##args)
#define pr_pri(fmt, args ...)     pr_info("vpq: %s " fmt, __func__, ##args)

#endif // __VPQ_PRINTK_H__
