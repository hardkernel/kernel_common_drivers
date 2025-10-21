/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _DPSS_VDI_H_
#define _DPSS_VDI_H_

#include "dpss_param.h"
#include "dpss_dpe.h"
#include "dpss_aa_pps.h"

void nr_force_config(struct PRM_DPSS_TOP *prm_top, struct PRM_DPSS_DPE *prm_dpe);

void cfg_dpss_vdi(struct PRM_DPSS_TOP *prm_top,
		struct PRM_DPSS_DPE *prm_dpe,
	struct AA_PPS_TOP_TYPE *nr_pps_cfg);

void cfg_dpss_vdi_slice(u32 frm_hsize,
	u32 frm_vsize,
	u32 slc_num,
	struct PRM_DPSS_TOP *prm_top,
	struct AA_PPS_TOP_TYPE *nr_pps_cfg);

void cfg_dpss_vdi_istop(u32 di_frm_cnt, bool top);
#endif
