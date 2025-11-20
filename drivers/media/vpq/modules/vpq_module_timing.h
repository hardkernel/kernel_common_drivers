/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __VPQ_MODULE_TIMING_H__
#define __VPQ_MODULE_TIMING_H__

#include "../vpq_table_type.h"

struct sig_info_t {
	const char *source;
	const char *height;
	const char *scan_mode;
	const char *hdr_type;
};

struct nonstandard_timing_map_s {
	unsigned int height;
	unsigned int width;
	char hdr_string[32];
	char src_string[32];
	unsigned char timing_index;
};

int vpq_module_timing_set_nonstandard_map(unsigned char value,
	struct nonstandard_timing_map_s *pdata);
unsigned char vpq_module_timing_table_index(enum pq_table_module_index_e module);

#endif // __VPQ_MODULE_TIMING_H__

