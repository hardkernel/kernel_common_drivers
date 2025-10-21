/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _DPSS_DAE_H_
#define _DPSS_DAE_H_

#include "dpss_param.h"
//#include "dpss_lib.h"
void dpss_me_width(u32 data);
void dpss_me_height(u32 data);
extern u32 ini_cfg_frc_dae;
extern int frc_fst_frm;
extern bool ini_cfg_dae_in_nr_frc;
extern int frc_fst_frm_in_nr;

void cfg_dpss_dae_me_rand_seed(u32 frm_cnt_inp,
	u32 frm_cnt_src0, u32 frm_cnt_src1, u32 frm_cnt_src2);
void cfg_dae_me_is_top(u32 frm_cnt);
void cfg_dpss_dae_di_sw_seed(u32 frm_di_cnt,
	u32 pre_dae_src, u32 cur_dae_src);
void cfg_dpss_me_bbd(u32 me_hsize, u32 me_vsize);
void cfg_dae_game_mode_sw(s32 *base_addr,
	s32 *base_oft, s32 buf_depth, s32 in_addr_en,
		struct PRM_DPSS_TOP *prm_top, s32 c_match);
void cfg_tfbf(u32 hsize, u32 vsize);
void dpss_dae_update_cfg(enum DPSS_WORK_MODE dae_mode,
	struct PRM_DPSS_TOP *prm_top,
	struct PRM_DPSS_DAE *prm_dae);
void dpss_dae_hpps(u32 ihsize, u32 ohsize);
void dae_glb_mot(int glb_mot);
//ary no extern void cfg_dae_hvds();
#endif
