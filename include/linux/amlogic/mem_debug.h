/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MEM_DEBUG_H
#define __MEM_DEBUG_H

void dump_mem_layout(char *buf);
void dump_mem_layout_boot_phase(void);

extern unsigned long mlock_fault_size;

struct cma_stat {
	unsigned int driver_c;
	unsigned int anon_c;
	unsigned int file_c;
	unsigned int free_c;
	char *buffer;
	unsigned long buf_size;
};

int get_cma_stat(struct cma *cma, struct cma_stat *stat, bool print_log);
#endif
