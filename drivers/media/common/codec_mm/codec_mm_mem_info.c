// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/amlogic/media/codec_mm/codec_mm_mem_info.h>
#include <linux/amlogic/media/codec_mm/codec_mm.h>
#include <linux/amlogic/media/codec_mm/codec_mm_scatter.h>
#include "codec_mm_priv.h"
#include "codec_mm_scatter_priv.h"

int query_decoder_mem(int inst_id, bool tvp_flag)
{
	struct codec_mm_s *mem = NULL;
	int total_pages = 0;
	unsigned long flags = 0;
	struct list_head *mem_list;
	int wk_size = 0;
	int es_size = 0;
	int yuv_size = 0;
	int coherent_size = 0;
	int sc_size = 0;

	codec_mm_mgt_lock(&flags);
	mem_list = codec_mm_get_list_head();
	if (!list_empty(mem_list)) {
		list_for_each_entry(mem, mem_list, list) {
			if (mem->ins_id != inst_id || mem->module_type != CODEC_MM_MODULE_DECODER)
				continue;

			switch (mem->mem_type) {
			case CODEC_MM_TYPE_WK:
				wk_size += mem->page_count;
				break;
			case CODEC_MM_TYPE_ES:
				es_size += mem->page_count;
				break;
			case CODEC_MM_TYPE_YUV:
				yuv_size += mem->page_count;
				break;
			case CODEC_MM_TYPE_COHERENT:
				coherent_size += mem->page_count;
				break;
			default:
				break;
			}
		}
	}
	codec_mm_mgt_unlock(&flags);
	sc_size = codec_mm_query_sc_buf(inst_id, CODEC_MM_MODULE_DECODER, tvp_flag);
	total_pages = wk_size + es_size + yuv_size + coherent_size + sc_size;

	if (codec_mm_get_debug_mode() & 0x80) {
		pr_err("dec mem details: wk %d, es %d, yuv %d, coherent %d, sc %d\n",
			wk_size, es_size, yuv_size, coherent_size, sc_size);

		pr_err("dec mem details: inst_id %d, total_pages %d\n",
			inst_id, total_pages);
	}

	return total_pages;
}
EXPORT_SYMBOL(query_decoder_mem);

void codec_mm_update_info(void *handle, int inst_id, enum codec_mm_module_type module_type,
	enum codec_mm_buf_type mem_type)
{
	struct codec_mm_s *mem = (struct codec_mm_s *)handle;

	if (!mem) {
		pr_err("mem is NULL\n");
		return;
	}

	mem->ins_id = inst_id;
	mem->module_type = module_type;
	mem->mem_type = mem_type;

	if (codec_mm_get_debug_mode() & 0x80)
		pr_err("update mem info: inst_id %d, module_type %d, mem_type %d\n",
			inst_id, module_type, mem_type);
}
EXPORT_SYMBOL(codec_mm_update_info);

void codec_mm_set_info_by_phy_addr(int inst_id, enum codec_mm_module_type module_type,
	enum codec_mm_buf_type mem_type, unsigned long phy_addr)
{
	struct codec_mm_s *mem = codec_mm_search_mem_by_phy(phy_addr);

	if (!mem) {
		if (codec_mm_get_debug_mode() & 0x80)
			pr_err("no phy_addr %lx in codec_mm\n", phy_addr);
		return;
	}

	codec_mm_update_info(mem, inst_id, module_type, mem_type);
}
EXPORT_SYMBOL(codec_mm_set_info_by_phy_addr);

void codec_mm_sc_update_info(void *handle, int inst_id,
	enum codec_mm_module_type module_type)
{
	struct codec_mm_scatter *mms = (struct codec_mm_scatter *)handle;

	if (!mms) {
		pr_err("mem is NULL\n");
		return;
	}

	mms->inst_id = inst_id;
	mms->module_type = module_type;
}
EXPORT_SYMBOL(codec_mm_sc_update_info);
