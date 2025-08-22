/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _CODEC_MM_MEM_INFO_H_
#define _CODEC_MM_MEM_INFO_H_

#include <linux/atomic.h>
#include <linux/spinlock.h>
#include <linux/list.h>

enum codec_mm_buf_type {
	CODEC_MM_TYPE_INVALID = 0,
	CODEC_MM_TYPE_ES = 1,
	CODEC_MM_TYPE_MV = 2,
	CODEC_MM_TYPE_YUV = 3,
	CODEC_MM_TYPE_WK = 4,
	CODEC_MM_TYPE_COHERENT = 5,
	CODEC_MM_TYPE_SCATTER = 6,
	CODEC_MM_TYPE_MAX,
};

enum codec_mm_module_type {
	CODEC_MM_MODULE_INVALID = 0,
	CODEC_MM_MODULE_DECODER = 1,
	CODEC_MM_MODULE_DI = 2,
	CODEC_MM_MODULE_VDIN = 3,
	CODEC_MM_MODULE_MAX,
};

struct codec_mm_dec_mem_info {
	int es_buf_size;
	int mv_buf_size;
	int yuv_buf_size;
	int wk_buf_size;
	int coherent_buf_size;
	int sc_buf_size;
};

struct codec_mm_mem_info {
	int inst_id;
	int module_id;
	bool tvp_flag;

	union {
		struct codec_mm_dec_mem_info dec_mem_info;
	};
};

int query_decoder_mem(int inst_id, bool tvp_flag);

void codec_mm_update_info(void *handle, int inst_id, enum codec_mm_module_type module_type,
	enum codec_mm_buf_type mem_type);

void codec_mm_set_info_by_phy_addr(int inst_id, enum codec_mm_module_type module_type,
	enum codec_mm_buf_type mem_type, unsigned long phy_addr);

void codec_mm_sc_update_info(void *handle, int inst_id,
	enum codec_mm_module_type module_type);

#endif
