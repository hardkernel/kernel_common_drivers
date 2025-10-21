/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _DPSS_H_
#define _DPSS_H_

#include "dpss_param.h"

//#include "dpss_tv_enc.h"
//#include "dpss_baddr.h"
#include "decontour.h"
//#include "dpss_lib.h"
#include "dpss_inp.h"
#include "dpss_dae.h"
#include "dpss_dpe.h"
#include "dpss_mc.h"
#include "dpss_aa_pps.h"
//#include "dolby5.h"

#ifdef EMUL_PXP
    #define PXP_ZERO 0
#else
    #define PXP_ZERO
#endif
//==============================================================//
// EXTERN
//==============================================================//
void dpss_prm_init(struct PRM_DPSS_TOP *prm_top, int path_id);
void dpss_gic_init(void);
void cfg_dpss_top(struct PRM_DPSS_TOP *prm_top);
void cfg_dpss_size_frc(struct PRM_DPSS_TOP *prm_top);
void cfg_dpss_size_nr(struct PRM_DPSS_TOP *prm_top);
void cfg_dpss_done_trigger(enum DONE_TRIGGER done_mode);
int  src0_data_trigger(void);
int  src1_data_trigger(void);
//void dst1_data_trigger(u32 dst1_done);
unsigned int disp_obuf_trigger(enum DONE_TRIGGER done_mode);
void cfg_tfbf(u32 hsize, u32 vsize);
void dpss_dpe_update_cfg(enum DPSS_WORK_MODE dpe_mode,
	struct PRM_DPSS_TOP *prm_top, struct PRM_DPSS_DPE *prm_dpe);
void cfg_dpss_dpe_nr_link_out(u32 nr_link_en);
void dpss_disp_func(void);
void cfg_vd1_rdmif(void);
void cfg_inp_triggle(bool triggle_sel);
void cfg_dae_triggle(void);

struct VPP_SET {
	u32 postbld_en;
	u32 postbld_vd1_en;
	u32 postbld_h_size;
	u32 postbld_v_size;
	u32 postbld_vd1_h_start;// 13bit
	u32 postbld_vd1_h_end;// 13bit
	u32 postbld_vd1_v_start;// 13bit
	u32 postbld_vd1_v_end;// 13bit
	u32 postbld_vd1_alpha;// 9bit
};

void cfg_dpss_film_phase(void);
#ifdef MOV
void dpss_process_irq(s32 invector, enum DPSS_WORK_MODE dae_nr_mode,
	enum DPSS_WORK_MODE dpe_nr_mode, enum DPSS_WORK_MODE dpe_di_mode,
	s32 src_mode, s32 cfg_seed, s32 cfg_slc, s32 slc_num);
#endif
//extern void cfg_dpe_top_size(PRM_DPE_TOP_SIZE_t prm_dpe_top_size);

void dpss_patch_for_bitmatch(u32 dpss_inp_frm_cnt,
	u32 dpss_dae_frm_cnt_src0,
	u32 dpss_dae_frm_cnt_src1,
	u32 dpss_dae_frm_cnt_src2,
	u32 dpss_dpe_nr_frm_cnt,
	u32 dpss_dpe_di_frm_cnt,

	u32 *dpss_di_ignore_num_out,
	u32 *dpss_nr_ignore_num_out,
	u32 *dpss_inp_frm_cnt_adj_out,
	u32 *dpss_dae_frm_cnt_src0_adj_out,
	u32 *dpss_dae_frm_cnt_src1_adj_out,
	u32 *dpss_dae_frm_cnt_src2_adj_out,
	u32 *dpss_dpe_nr_frm_cnt_adj_out,
	u32 *dpss_dpe_di_frm_cnt_adj_out);

struct PRM_SRC_FMT fmt_cfg(enum vid_src_fmt src_fmt,
	enum vid_src_fmt src_plan,
	enum vid_src_fmt src_bit,
	enum vid_src_fmt src_cmpr,
	enum vid_src_fmt interlace);
u64 buff_addr_cal(u64 buff_addr, u32 buff_step, u32 buff_idx, u32 buff_num);
#endif
