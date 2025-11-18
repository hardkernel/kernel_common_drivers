/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _CODEC_MM_PREALLOC_H
#define _CODEC_MM_PREALLOC_H

enum prealloc_mem_type {
	PREALLOC_YUV_TYPE = 0,
	PREALLOC_AVBC_HEADER_TYPE = 1,
	PREALLOC_WK_TYPE = 2,
	PREALLOC_MV_TYPE = 3,
	PREALLOC_SC_TYPE = 4,
	PREALLOC_DPSS_SML_TYPE = 5,
	PREALLOC_DPSS_YUV_TYPE = 6,
};

void release_all_prealloc_job_direct(void);
void release_prealloc_job(int inst_identify);
void release_prealloc_job_with_type(int inst_identify, u32 type);
void submit_prealloc_job(u32 type, u32 num, u32 size, int align_2n, int memflags, int inst_id);

#endif //_CODEC_MM_PREALLOC_H
