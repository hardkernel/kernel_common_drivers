/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __VPQ_COMMON_H__
#define __VPQ_COMMON_H__

#include "vpq_table_type.h"

#define RET_NULL_POINT               (-1)
#define RET_INVALID_INPUT            (-2)
#define REF_CFG_DISABLED             (-3)
#define RET_TAB_ERROR                (-4)
#define RET_SAME_SETTING             (-5)

enum vpq_pq_setting_item_e {
	PQ_BRIG = 0,
	PQ_CONT,
	PQ_SAT,
	PQ_HUE,
	PQ_SHARPNESS,
	PQ_DNLP,
	PQ_LC,
	PQ_BLK,
	PQ_BLS,
	PQ_CCORING,
	PQ_HDR_TMO,
	PQ_CM,
	PQ_AIPQ,
	PQ_AISR,

	PQ_NR,
	PQ_DEBLOCK,
	PQ_DEMOSQUITO,
	PQ_SMOOTHPLUS,

	PQ_AMDV_PIC_MODE,
	PQ_AMDV_DARK_DETAIL,
	PQ_AMDV_PRE_DETAIL,

	PQ_SETTING_MAX,
};

extern struct TABLE_VER_PQ pq_table_ver;
extern struct PQ_TABLE_PARAM pq_table_param;
extern struct TABLE_PQ_MODULE_CFG pq_module_cfg;
extern unsigned int pq_setting_val[PQ_SETTING_MAX];
extern unsigned char pq_table_idx[PQ_INDEX_MAX];

#endif // __VPQ_COMMON_H__
