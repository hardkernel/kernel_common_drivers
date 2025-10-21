/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPSS_D_PQ_DRV_H__
#define __DPSS_D_PQ_DRV_H__
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/amlogic/iomap.h>
#include "d_pq_mgr.h"
#define DI_ADDR_PARAM(page, reg)  (((page) << 8) | (reg))
extern bool sw_dct;
int di_set_dblk_reg(struct di_dblk_param_s *pdata);
int di_set_demosquito_reg(struct di_demosquito_param_s *pdata);
int di_set_dnr_reg(struct di_dnr_param_s *pdata);
int di_set_dct_reg(struct di_dct_param_s *pdata);
int di_set_xlr_reg(struct di_xlr_param_s *pdata);
void di_write_data_table(enum di_page_module_e module, int index);
unsigned int di_read_from_db_table(enum di_page_module_e module,
	int index, int nub);
#endif	/*__DPSS_D_PQ_DRV_H__*/
