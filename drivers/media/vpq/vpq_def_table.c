// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "vpq_table_type.h"

struct TABLE_VER_PQ pq_table_ver = {
	"", "", "", "", "", ""
};

struct TABLE_PQ_MODULE_CFG pq_module_cfg = {0};

struct PQ_TABLE_PARAM pq_table_param = {
 /* pq_index_table0 pq module table index
  * line: signel type (0-167: enum pq_source_timing_e)
  * list: pq module (0-24: enum pq_table_module_index_e)
  * 255: means default config
  *
  * 0: Dnlp      1: HDR_TMO  2: LC          3: AIPQ        4: BLK
  * 5: BLS       6: CCoring  7: CM2         8: SR          9: Sharpness
  * 10: AISR    11: Reserve 12: Reserve    13: Reserve    14: Reserve
  * 15: NR      16: Deblock 17: Demosquito 18: SmoothPlus 19: DI
  * 20: Reserve 21: Reserve 22: Reserve    23: Reserve    24: Reserve
  *
  * Ex: PQ_SRC_INDEX_VGA
  *  0    1    2    3    4    5    6    7    8    9    10   11   12
  *	 13   14   15   16   17   18   19   20   21   22   23   24
  *	{255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  *	 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,}, // 0 : VGA
  */
	.pq_index_table0 = {
		[0 ... PQ_SRC_INDEX_MAX - 1] = {
			[0 ... PQ_INDEX_MAX - 1] = 255
		}
	},

/* other array members initialize to 0 automatically
 * pq_dnlp_curve_param_s
 * pq_hdr_tmo_param_s
 * pq_lc_curve_param_s
 * ......
 */
};
