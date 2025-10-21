/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _DPSS_SW_CFG_H
#define _DPSS_SW_CFG_H
#ifdef RUN_ON_PC
#include <stdbool.h>
#endif
#include "dpss.h"

struct INP_SW_CFG {
	u32 hsize_in;
	u32 vsize_in;
	u32 me_hsize;
	u32 me_vsize;
	//u32 me_hsize;
	//u32 me_vsize;
	u32 cur_wr_mif_opt;
	u32 cur_rd_mif_opt;
	u32 pre_rd_mif_opt;
	u32 luma_rdmif_baddr;
	u32 chrm_rdmif_baddr;
	u32 pre_rdmif_baddr;
	u32 cur_wrmif_baddr;
	u32 wrmif_en;
	u32 film_hwfw_sel;
	u32 frc_film_mode_fw;
	u32 film_phase_fw;
	u32 frm_hold;
};

struct DAE_SW_CFG {
	u32 frm_swth;
	u32 ip_logo_init_frm;
	u32 frm_phase;
	u32 frm_phs;
	u32 dae_rd_phs;
	u32 src_idx;
	u32 src_mode;
	u32 frc_en;
	u32 di_en;
	u32 nr_en;
	u32 inp_hsize;
	u32 inp_vsize;
	u32 frm_hsize;
	u32 frm_vsize;
	u32 blk_hsize;
	u32 blk_vsize;
	u32 me_mvx_div_mode;
	u32 me_mvy_div_mode;
	u32 frc_ofrm_idx;
	u32 me_pre_rd_mif_opt;
	u32 me_cur_rd_mif_opt;
	u32 me_nxt_rd_mif_opt;
	u32 me_yuv_rd_mif_opt;
	u32 me_nc_uni_mv_wr_addr;
	u32 me_cn_uni_mv_wr_addr;
	u32 me_pc_phs_mv_wr_addr;
	u32 me_nc_uni_mv_rd_addr;
	u32 me_cn_uni_mv_rd_addr;
	u32 me_pc_phs_mv_rd_addr;
	u32 me_cp_uni_mv_rd_addr;
	u32 me_pc_uni_mv_rd_addr;
	u32 me_pb_uni_mv_rd_addr;
	u32 me_cur_rdmif_baddr;
	u32 me_pre_rdmif_baddr;
	u32 me_nxt_luma_baddr;
	u32 me_nxt_chrm_baddr;
	u32 me_mix_wrmif_baddr;
	u32 mevp_mc_mv_wr_baddr;
	u32 mevp_mc_logo_wr_baddr;
	u32 mevp_mc_logo_rd_baddr;
	u32 nrme_info_rdmif_baddr;
	u32 nrme_info_wrmif_baddr;
	u32 nrme_mtn_rdmif_baddr;
	u32 nrme_mtn_wrmif_baddr;
	u32 nrme_alp_wrmif_baddr;
	u32 dime_tfbf_baddr;
	u32 dctgrd_wrmif_baddr;
	u32 dctdivr_wrmif_baddr;
	u32 dctdir_wrmif_baddr;
	u32 logo_pre_iir_baddr;
	u32 logo_cur_iir_baddr;
	u32 logo_pre_scc_baddr;
	u32 logo_cur_scc_baddr;
	u32 iplogo_wrmif_baddr;
	u32 mero_wrmif_baddr;
	u32 fd_phase_err_flg_film;
};

struct DPE_SW_CFG {
	// u32 frm_phs_hsync;
	u32 mv_blk_mode;
	// u32 dv_go_field;
	// u32 dv_go_line ;
	u32 func_en;
	u32 di_loop;
	u32 mc_loop;
	u32 amdv_loop;
	u32 src_hsize;
	u32 src_vsize;
	u32 frm_hsize;
	u32 frm_vsize;
	//  u32 me_hsize;
	//  u32 me_vsize;
	//  u32 blk_hsize;
	//  u32 blk_vsize;
	//  u32 logo_hsize;
	//  u32 logo_vsize;
	u32 amdv_src_hsize;
	u32 amdv_src_vsize;
	u32 amdv_frm_hsize;
	u32 amdv_frm_vsize;
	u32 mc_phase;
	u32 ofrm_step;
	u32 mc_gcb_bypass_mc_en;
	// u32 inp0_rd_mif_opt;
	// u32 inp1_rd_mif_opt;
	// u32 inp2_rd_mif_opt;
	// u32 out_wr_mif_opt;
	u32 mc_mv_baddr0;	//ary pre2
	u32 mc_mv_baddr1;
	u32 mc_melogo_baddr0;
	u32 mc_melogo_baddr1;
	u32 nr_wrmif_baddr0;	//ary wr y
	u32 nr_wrmif_baddr1;	//ary wr c
	u32 nr_wrmif_baddr2;	//ary wr hd y
	u32 nr_wrmif_baddr3;	//	wr hd c
	u32 mc_dwmif_baddr0;
	u32 mc_dwmif_baddr1;
	u32 mc_dwmif_baddr2;
	u32 mc_dwmif_baddr3;
	u32 di_wmif_baddr0;
	u32 di_wmif_baddr1;
	u32 di_wmif_baddr2;
	u32 di_wmif_baddr3;

	u32 nr_luma0_baddr;	//ary:pre 1
	u32 nr_luma1_baddr;	//ary: current input
	u32 nr_luma2_baddr;	//ary: pre 2
	u32 nr_chrm0_baddr;	//
	u32 nr_chrm1_baddr;
	u32 nr_chrm2_baddr;

	u32 nr_luma0_hd_baddr;	//ary: header pre 1
	u32 nr_luma1_hd_baddr;
	u32 nr_luma2_hd_baddr;
	u32 nr_chrm0_hd_baddr;
	u32 nr_chrm1_hd_baddr;
	u32 nr_chrm2_hd_baddr;

	u32 mc_luma0_baddr0;
	u32 mc_luma0_baddr1;
	u32 mc_luma1_baddr0;
	u32 mc_luma1_baddr1;
	u32 mc_chrm0_baddr0;
	u32 mc_chrm0_baddr1;
	u32 mc_chrm1_baddr0;
	u32 mc_chrm1_baddr1;
	u32 mc_logo0_baddr;
	u32 mc_logo1_baddr;
	u32 ip_logo_baddr;
	u32 mc_mv_baddr;

	u32 logo0_rdmif_baddr;
	u32 logo1_rdmif_baddr;
	u32 nr_blk_baddr;
	u32 di_blk_baddr;
	u32 nr_mtn_baddr;
	u32 di_mtn_baddr;
	u32 rd_dmsq_baddr;
	u32 hdblk_baddr;
	u32 vdblk_baddr;
	u32 wr_dmsq_baddr;
	u32 dcntr_hist_rbaddr;
	u32 dcntr_yds_rbaddr;
	u32 dcntr_cds_rbaddr;
	u32 dcntr_divr_rbaddr;
	u32 dcntr_dir_rbaddr;
	u32 amdv_intp_rbaddr;
	u32 ds_mix_baddr;
	u32 me_mvx_div_mode;
	u32 me_mvy_div_mode;
	u32 src_idx;
	u32 debug_info0;
	u32 debug_info1;
	u32 debug_info2;
	u32 debug_info3;
	u32 debug_info4;
	u32 clr_dpe_flag;
	u32 fnr_loop;
};

struct DPSS_SW_CFG {
	struct INP_SW_CFG inp_sw_prm[100];
	struct DAE_SW_CFG dae_sw_prm[3][100];
	struct DPE_SW_CFG dpe_sw_prm[3][100];
	u32 sw_mode;
};

//typedef struct DPSS_SW_FRM_CFG{
// struct DPSS_SW_CFG  dpss_sw_prm;
//} DPSS_SW_FRM_CFG_t;

void dpss_inp_sw(struct INP_SW_CFG *sw_prm);
void dpss_dae_sw(struct DAE_SW_CFG *sw_prm);
void dpss_inp_ini(struct INP_SW_CFG *sw_prm);
void dpss_dae_ini(struct DAE_SW_CFG *sw_prm);
void dpss_dpe_ini(struct DPE_SW_CFG *sw_prm);
#endif
