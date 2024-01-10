/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef AML_MM_HEADER
#define AML_MM_HEADER
#include <linux/atomic.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include<linux/printk.h>

#ifndef INFO_PREFIX
#define INFO_PREFIX	"[codec_mm]"
#endif

#define CODEC_DBG_ERR_INFO			(0)
#define CODEC_DBG_DISABLE_RESV		(0x1)
#define CODEC_DBG_DISABLE_CMA		(0x2)
#define CODEC_DBG_DISABLE_SYS		(0x4)
#define CODEC_DBG_DISABLE_HALF_MEM	(0x8)
#define CODEC_DBG_DISABLE_MODE		(0xf)
#define CODEC_DBG_DUMP_INFO_WHEN_FAIL	(0x10)
#define CODEC_DBG_TRACE_ALLOC_FREE	(0x20)
#define CODEC_DBG_DUMP_INFO		(0x40)
#define CODEC_DBG_MEM_WATERMARK		(0x80)
#define CODEC_DBG_DBUF_REF_TRACE	(0x100)

extern u32 debug_mode;
#define codec_dbg_level(args) ((args) & debug_mode)

#define codec_pr_dbg(flags, fmt, args...)	\
do {					\
	if (codec_dbg_level(flags))	\
		pr_info(INFO_PREFIX fmt, ##args);	\
	else if (!flags)		\
		pr_crit(INFO_PREFIX fmt, ##args);	\
} while (0)

void dma_clear_buffer(struct page *page, size_t size);

u32 codec_mm_get_sc_debug_mode(void);
u32 codec_mm_get_keep_debug_mode(void);
bool codec_mm_scatter_available_check(int size);
void codec_mm_scatter_level_decrease(int size);
void codec_mm_scatter_level_increase(int size);
void codec_mm_set_min_linear_size(int min_mem_val);
int codec_mm_get_min_linear_size(void);
int codec_mm_get_scatter_watermark(void);
#endif /**/
