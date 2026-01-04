/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _DPSS_MC_H_
#define _DPSS_MC_H_

#include "dpss_param.h"

void cfg_dpss_mc_ini(struct PRM_DPSS_TOP *prm_top, u32 src_from_nr);
void cfg_dpss_mc_intf(struct PRM_DPSS_TOP *prm_top, struct DPSS_MC0_TYPE *prm_mc);
void cfg_dpss_mc0(struct PRM_DPSS_TOP *prm_top, u32 src_from_nr, struct DPSS_MC0_TYPE *prm_mc);
void cfg_dpss_mc_pre_triggle(void);//cfg @_pre_go_filed
void cfg_dpss_mc_triggle(void);//cfg @go_filed

void cfg_dpss_mc_slice(u32 frm_hsize, u32 frm_vsize, u32 slc_num);
void cfg_dpss_mc_bbd(u32 frm_hsize, u32 frm_vsize,
				  u32 bbd_en, u32 bbd_sel,
				  u32 src_hsize, u32 src_vsize);

void dpss_dpe_info_cfg(u32 dpe_phs);
extern struct PRM_INTF_TYPE pix_rmif;//ary add.

#endif
