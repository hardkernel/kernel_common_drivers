// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "define.h"
#include "dpss_sw_cfg.h"

void dpss_inp_ini(struct INP_SW_CFG *sw_prm)
{
	sw_prm->hsize_in = 0;
	sw_prm->vsize_in = 0;
	sw_prm->me_hsize = 0;
	sw_prm->me_vsize = 0;
	sw_prm->cur_wr_mif_opt = 0;
	sw_prm->cur_rd_mif_opt = 0;
	sw_prm->pre_rd_mif_opt = 0;
	sw_prm->luma_rdmif_baddr = 0;
	sw_prm->chrm_rdmif_baddr = 0;
	sw_prm->pre_rdmif_baddr = 0;
	sw_prm->cur_wrmif_baddr = 0;
	sw_prm->wrmif_en = 0;
	sw_prm->film_hwfw_sel = 0;
	sw_prm->frc_film_mode_fw = 0;
	sw_prm->film_phase_fw = 0;
	sw_prm->frm_hold = 0;
	DBG_INF("%s:sw_prm->frm_hold:%d\n", __func__, sw_prm->frm_hold);
}

void dpss_dae_ini(struct DAE_SW_CFG *sw_prm)
{
	sw_prm->frm_swth               = 0;
	sw_prm->ip_logo_init_frm       = 0;
	sw_prm->frm_phase              = 0;
	sw_prm->frm_phs                = 0;
	sw_prm->dae_rd_phs             = 0;
	sw_prm->src_idx                = 0;
	sw_prm->src_mode               = 0;
	sw_prm->frc_en                 = 0;
	sw_prm->di_en                  = 0;
	sw_prm->nr_en                  = 0;
	sw_prm->inp_hsize              = 0;
	sw_prm->inp_vsize              = 0;
	sw_prm->frm_hsize              = 0;
	sw_prm->frm_vsize              = 0;
	sw_prm->blk_hsize              = 0;
	sw_prm->blk_vsize              = 0;
	sw_prm->me_mvx_div_mode        = 0;
	sw_prm->me_mvy_div_mode        = 0;
	sw_prm->frc_ofrm_idx           = 0;
	sw_prm->me_pre_rd_mif_opt      = 0;
	sw_prm->me_cur_rd_mif_opt      = 0;
	sw_prm->me_nxt_rd_mif_opt      = 0;
	sw_prm->me_yuv_rd_mif_opt      = 0;
	sw_prm->me_nc_uni_mv_wr_addr   = 0;
	sw_prm->me_cn_uni_mv_wr_addr   = 0;
	sw_prm->me_pc_phs_mv_wr_addr   = 0;
	sw_prm->me_nc_uni_mv_rd_addr   = 0;
	sw_prm->me_cn_uni_mv_rd_addr   = 0;
	sw_prm->me_pc_phs_mv_rd_addr   = 0;
	sw_prm->me_cp_uni_mv_rd_addr   = 0;
	sw_prm->me_pc_uni_mv_rd_addr   = 0;
	sw_prm->me_pb_uni_mv_rd_addr   = 0;
	sw_prm->me_cur_rdmif_baddr     = 0;
	sw_prm->me_pre_rdmif_baddr     = 0;
	sw_prm->me_nxt_luma_baddr      = 0;
	sw_prm->me_nxt_chrm_baddr      = 0;
	sw_prm->me_mix_wrmif_baddr     = 0;
	sw_prm->mevp_mc_mv_wr_baddr    = 0;
	sw_prm->mevp_mc_logo_wr_baddr  = 0;
	sw_prm->mevp_mc_logo_rd_baddr  = 0;
	sw_prm->nrme_info_rdmif_baddr  = 0;
	sw_prm->nrme_info_wrmif_baddr  = 0;
	sw_prm->nrme_mtn_rdmif_baddr   = 0;
	sw_prm->nrme_mtn_wrmif_baddr   = 0;
	sw_prm->nrme_alp_wrmif_baddr   = 0;
	sw_prm->dime_tfbf_baddr        = 0;
	sw_prm->dctgrd_wrmif_baddr     = 0;
	sw_prm->dctdivr_wrmif_baddr    = 0;
	sw_prm->dctdir_wrmif_baddr     = 0;
	sw_prm->logo_pre_iir_baddr     = 0;
	sw_prm->logo_cur_iir_baddr     = 0;
	sw_prm->logo_pre_scc_baddr     = 0;
	sw_prm->logo_cur_scc_baddr     = 0;
	sw_prm->iplogo_wrmif_baddr     = 0;
	sw_prm->mero_wrmif_baddr       = 0;
	sw_prm->fd_phase_err_flg_film  = 0;
	DBG_INF("%s: sw_prm->di_en %d\n", __func__, sw_prm->di_en);
}

void dpss_dpe_ini(struct DPE_SW_CFG *sw_prm)
{
	sw_prm->mv_blk_mode        = 0;
	sw_prm->func_en            = 0;
	sw_prm->di_loop            = 0;
	sw_prm->mc_loop            = 0;
	sw_prm->amdv_loop          = 0;
	sw_prm->src_hsize          = 0;
	sw_prm->src_vsize          = 0;
	sw_prm->frm_hsize          = 0;
	sw_prm->frm_vsize          = 0;
	sw_prm->amdv_src_hsize     = 0;
	sw_prm->amdv_src_vsize     = 0;
	sw_prm->amdv_frm_hsize     = 0;
	sw_prm->amdv_frm_vsize     = 0;
	sw_prm->mc_phase           = 0;
	sw_prm->ofrm_step          = 0;
	sw_prm->mc_gcb_bypass_mc_en = 0;
	sw_prm->mc_mv_baddr0       = 0;
	sw_prm->mc_mv_baddr1       = 0;
	sw_prm->mc_melogo_baddr0   = 0;
	sw_prm->mc_melogo_baddr1   = 0;
	sw_prm->nr_wrmif_baddr0    = 0;
	sw_prm->nr_wrmif_baddr1    = 0;
	sw_prm->nr_wrmif_baddr2    = 0;
	sw_prm->nr_wrmif_baddr3    = 0;
	sw_prm->mc_dwmif_baddr0    = 0;
	sw_prm->mc_dwmif_baddr1    = 0;
	sw_prm->mc_dwmif_baddr2    = 0;
	sw_prm->mc_dwmif_baddr3    = 0;
	sw_prm->di_wmif_baddr0     = 0;
	sw_prm->di_wmif_baddr1     = 0;
	sw_prm->di_wmif_baddr2     = 0;
	sw_prm->di_wmif_baddr3     = 0;

	sw_prm->mc_luma0_baddr0    = 0;
	sw_prm->mc_luma0_baddr1    = 0;
	sw_prm->mc_luma1_baddr0    = 0;
	sw_prm->mc_luma1_baddr1    = 0;
	sw_prm->mc_chrm0_baddr0    = 0;
	sw_prm->mc_chrm0_baddr1    = 0;
	sw_prm->mc_chrm1_baddr0    = 0;
	sw_prm->mc_chrm1_baddr1    = 0;
	sw_prm->mc_logo0_baddr     = 0;
	sw_prm->mc_logo1_baddr     = 0;
	sw_prm->ip_logo_baddr      = 0;
	sw_prm->mc_mv_baddr        = 0;

	sw_prm->logo0_rdmif_baddr  = 0;
	sw_prm->logo1_rdmif_baddr  = 0;
	sw_prm->nr_blk_baddr       = 0;
	sw_prm->di_blk_baddr       = 0;
	sw_prm->nr_mtn_baddr       = 0;
	sw_prm->di_mtn_baddr       = 0;
	sw_prm->rd_dmsq_baddr      = 0;
	sw_prm->hdblk_baddr        = 0;
	sw_prm->vdblk_baddr        = 0;
	sw_prm->wr_dmsq_baddr      = 0;
	sw_prm->dcntr_hist_rbaddr  = 0;
	sw_prm->dcntr_yds_rbaddr   = 0;
	sw_prm->dcntr_cds_rbaddr   = 0;
	sw_prm->amdv_intp_rbaddr   = 0;
	sw_prm->ds_mix_baddr       = 0;
	sw_prm->me_mvx_div_mode    = 0;
	sw_prm->me_mvy_div_mode    = 0;
	sw_prm->src_idx            = 0;
	sw_prm->debug_info0        = 0;
	sw_prm->debug_info1        = 0;
	sw_prm->debug_info2        = 0;
	sw_prm->debug_info3        = 0;
	sw_prm->debug_info4        = 0;
	sw_prm->clr_dpe_flag       = 0;
	sw_prm->fnr_loop           = 0;
	DBG_INF("%s:sw_prm->mv_blk_mode %d\n", __func__, sw_prm->mv_blk_mode);
}

void dpss_inp_sw(struct INP_SW_CFG *sw_prm)
{
	u32 reg_frm_org_hsize        = sw_prm->hsize_in & 0x1fff;
	u32 reg_frm_org_vsize        = sw_prm->vsize_in & 0x1fff;
	u32 reg_frm_skp_hsize        = sw_prm->hsize_in & 0x1fff;
	u32 reg_frm_skp_vsize        = sw_prm->vsize_in & 0x1fff;
	u32 reg_frm_hsize            = sw_prm->me_hsize & 0x1fff;
	u32 reg_frm_vsize            = sw_prm->me_vsize & 0x1fff;
	u32 reg_det_hsize            = sw_prm->me_hsize & 0x1fff;
	u32 reg_det_vsize            = sw_prm->me_vsize & 0x1fff;
	u32 reg_inp_cur_wr_mif_opt   = sw_prm->cur_wr_mif_opt & 0x7;
	u32 reg_inp_cur_rd_mif_opt   = sw_prm->cur_rd_mif_opt & 0x7;
	u32 reg_inp_pre_rd_mif_opt   = sw_prm->pre_rd_mif_opt & 0x7;
	u32 reg_inp_luma_rdmif_baddr = sw_prm->luma_rdmif_baddr;
	u32 reg_inp_chrm_rdmif_baddr = sw_prm->chrm_rdmif_baddr;
	u32 reg_inp_pre_rdmif_baddr  = sw_prm->pre_rdmif_baddr;
	u32 reg_inp_cur_wrmif_baddr  = sw_prm->cur_wrmif_baddr;
	u32 reg_inp_wrmif_en         = sw_prm->wrmif_en & 0xff;
	u32 reg_inp_film_hwfw_sel    = sw_prm->film_hwfw_sel & 0x1;
	u32 reg_inp_frc_film_mode_fw = sw_prm->frc_film_mode_fw & 0xff;
	u32 reg_inp_film_phase_fw    = sw_prm->film_phase_fw & 0xff;
	u32 reg_inp_frm_hold         = sw_prm->frm_hold & 0x1;

	DBG_INF("%s: %d\n", __func__, reg_inp_frm_hold);

	wr(FRC_INP_SW_FRM_ORIGINAL_SIZE, (reg_frm_org_hsize << 16) |
		(reg_frm_org_vsize));
	wr(FRC_INP_SW_FRM_SKIP_SIZE, (reg_frm_skp_hsize << 16) |
		(reg_frm_skp_vsize));
	wr(FRC_INP_SW_FRM_ALIGNED_SIZE, (reg_frm_hsize << 16) |
		(reg_frm_vsize));
	wr(FRC_INP_SW_DETECT_SIZE, (reg_det_hsize << 16) | (reg_det_vsize));

	wr(FRC_INP_SW_FRM_INFO, (reg_inp_cur_wr_mif_opt << 8) |
		(reg_inp_cur_rd_mif_opt << 4) | (reg_inp_pre_rd_mif_opt));
	wr(FRC_INP_SW_LUMA_RMIF_BADDR, reg_inp_luma_rdmif_baddr);
	wr(FRC_INP_SW_CHRM_RMIF_BADDR, reg_inp_chrm_rdmif_baddr);
	wr(FRC_INP_SW_PRE_RMIF_BADDR, reg_inp_pre_rdmif_baddr);
	wr(FRC_INP_SW_CUR_WMIF_BADDR, reg_inp_cur_wrmif_baddr);
	wr(FRC_INP_SW_CTRL, (reg_inp_film_hwfw_sel << 28) |
		(reg_inp_frm_hold << 24) |
		(reg_inp_wrmif_en << 16) |
		(reg_inp_frc_film_mode_fw << 8) |
		(reg_inp_film_phase_fw));

	w_reg_bit(FRC_REG_INP_FRM_CTRL, 1, 0, 1);// pls_inp_frm_start
}

//din_mode: 0:frc only;1:nr only; 2:di only; other:nrdi
// function: count addr for dae and set addr

void dpss_dae_sw(struct DAE_SW_CFG *sw_prm)
{
	u32 reg_frm_swth                = sw_prm->frm_swth & 0xff;
	u32 reg_ip_logo_init_frm        = sw_prm->ip_logo_init_frm & 0x1;
	u32 reg_frm_phase               = sw_prm->frm_phase & 0xff;
	u32 reg_dae_frm_phs             = sw_prm->frm_phs & 0x1;
	u32 reg_dae_rd_phs              = sw_prm->dae_rd_phs & 0x1;
	u32 reg_dae_src_idx             = sw_prm->src_idx & 0xf;
	u32 reg_dae_src_mode            = sw_prm->src_mode & 0x1;
	u32 reg_dae_frc_en              = sw_prm->frc_en & 0x1;
	u32 reg_dae_di_en               = sw_prm->di_en & 0x1;
	u32 reg_dae_nr_en               = sw_prm->nr_en & 0x1;
	u32 reg_sw_mvx_div_mode         = sw_prm->me_mvx_div_mode   & 0x3;
	u32 reg_sw_mvy_div_mode         = sw_prm->me_mvy_div_mode   & 0x3;
	u32 reg_me_pre_rd_mif_opt       = sw_prm->me_pre_rd_mif_opt & 0x3;
	u32 reg_me_cur_rd_mif_opt       = sw_prm->me_cur_rd_mif_opt & 0x3;
	u32 reg_me_nxt_rd_mif_opt       = sw_prm->me_nxt_rd_mif_opt & 0x3;
	u32 reg_me_yuv_rd_mif_opt       = sw_prm->me_yuv_rd_mif_opt & 0x3;
	u32 reg_me_nc_uni_mv_wr_addr    = sw_prm->me_nc_uni_mv_wr_addr >> 4;
	u32 reg_me_cn_uni_mv_wr_addr    = sw_prm->me_cn_uni_mv_wr_addr >> 4;
	u32 reg_me_pc_phs_mv_wr_addr    = sw_prm->me_pc_phs_mv_wr_addr >> 4;
	u32 reg_me_nc_uni_mv_rd_addr    = sw_prm->me_nc_uni_mv_rd_addr >> 4;
	u32 reg_me_cn_uni_mv_rd_addr    = sw_prm->me_cn_uni_mv_rd_addr >> 4;
	u32 reg_me_pc_phs_mv_rd_addr    = sw_prm->me_pc_phs_mv_rd_addr >> 4;
	u32 reg_me_cp_uni_mv_rd_addr    = sw_prm->me_cp_uni_mv_rd_addr >> 4;
	u32 reg_me_pc_uni_mv_rd_addr    = sw_prm->me_pc_uni_mv_rd_addr >> 4;
	u32 reg_me_pb_uni_mv_rd_addr    = sw_prm->me_pb_uni_mv_rd_addr >> 4;
	u32 reg_me_cur_rdmif_baddr      = sw_prm->me_cur_rdmif_baddr >> 4;
	u32 reg_me_pre_rdmif_baddr      = sw_prm->me_pre_rdmif_baddr >> 4;
	u32 reg_me_nxt_luma_baddr       = sw_prm->me_nxt_luma_baddr >> 4;
	u32 reg_me_nxt_chrm_baddr       = sw_prm->me_nxt_chrm_baddr >> 4;
	u32 reg_me_mix_wrmif_baddr      = sw_prm->me_mix_wrmif_baddr >> 4;
	u32 reg_mevp_mc_mv_wr_baddr     = sw_prm->mevp_mc_mv_wr_baddr >> 4;
	u32 reg_mevp_mc_logo_wr_baddr   = sw_prm->mevp_mc_logo_wr_baddr >> 4;
	u32 reg_mevp_mc_logo_rd_baddr   = sw_prm->mevp_mc_logo_rd_baddr >> 4;
	u32 reg_nrme_info_rdmif_baddr   = sw_prm->nrme_info_rdmif_baddr >> 4;
	u32 reg_nrme_info_wrmif_baddr   = sw_prm->nrme_info_wrmif_baddr >> 4;
	u32 reg_nrme_mtn_rdmif_baddr    = sw_prm->nrme_mtn_rdmif_baddr >> 4;
	u32 reg_nrme_mtn_wrmif_baddr    = sw_prm->nrme_mtn_wrmif_baddr >> 4;
	u32 reg_nrme_alp_wrmif_baddr    = sw_prm->nrme_alp_wrmif_baddr >> 4;
	u32 reg_dae_dime_tfbf_baddr     = sw_prm->dime_tfbf_baddr >> 4;
	u32 reg_dae_dctgrd_wrmif_baddr  = sw_prm->dctgrd_wrmif_baddr >> 4;
	u32 dctdivr_wrmif_baddr         = sw_prm->dctdivr_wrmif_baddr >> 4;
	u32 dctdir_wrmif_baddr          = sw_prm->dctdir_wrmif_baddr >> 4;
	u32 reg_logo_pre_iir_baddr      = sw_prm->logo_pre_iir_baddr >> 4;
	u32 reg_logo_cur_iir_baddr      = sw_prm->logo_cur_iir_baddr >> 4;
	u32 reg_logo_pre_scc_baddr      = sw_prm->logo_pre_scc_baddr >> 4;
	u32 reg_logo_cur_scc_baddr      = sw_prm->logo_cur_scc_baddr >> 4;
	u32 reg_iplogo_wrmif_baddr      = sw_prm->iplogo_wrmif_baddr >> 4;
	u32 reg_mero_wrmif_baddr        = sw_prm->mero_wrmif_baddr >> 4;

	DBG_INF("%s:reg_dae_di_en %d\n", __func__, reg_dae_di_en);

	w_reg_bit(VPU_DAE_WRAP_SW_EN, reg_dae_src_mode << 8 |
				reg_dae_src_idx << 4 |
				reg_dae_rd_phs << 1 |
				reg_dae_frm_phs, 12, 9);
	w_reg_bit(VPU_DAE_WRAP_SW_EN, 1, 0, 1);// pls_syn_frm_rst_me
	wr(VPU_DAE_WRAP_SW_EN, (reg_dae_nr_en << 23)		|
				(reg_dae_di_en << 22)		|
				(reg_dae_frc_en << 21)		|
				(reg_dae_src_mode << 20)	|
				(reg_dae_src_idx << 16)		|
				(reg_dae_rd_phs << 13)		|
				(reg_dae_frm_phs << 12)		|
				(reg_frm_phase << 4)		|
				(reg_frm_swth << 2)		|
				(reg_ip_logo_init_frm << 1));

	wr(VPU_DAE_WRAP_SW_INFO, (reg_me_yuv_rd_mif_opt	<< 24) |
				(reg_me_nxt_rd_mif_opt	<< 20) |
				(reg_me_cur_rd_mif_opt	<< 16) |
				(reg_me_pre_rd_mif_opt	<< 12) |
				(reg_sw_mvy_div_mode	<<  2) |
				(reg_sw_mvx_div_mode));
	wr(VPU_DAE_WRAP_SW_ADDR0, reg_me_nc_uni_mv_wr_addr);
	wr(VPU_DAE_WRAP_SW_ADDR1, reg_me_cn_uni_mv_wr_addr);
	wr(VPU_DAE_WRAP_SW_ADDR2, reg_me_pc_phs_mv_wr_addr);
	wr(VPU_DAE_WRAP_SW_ADDR3, reg_me_nc_uni_mv_rd_addr);
	wr(VPU_DAE_WRAP_SW_ADDR4, reg_me_cn_uni_mv_rd_addr);
	wr(VPU_DAE_WRAP_SW_ADDR5, reg_me_pc_phs_mv_rd_addr);
	wr(VPU_DAE_WRAP_SW_ADDR6, reg_me_cp_uni_mv_rd_addr);
	wr(VPU_DAE_WRAP_SW_ADDR7, reg_me_pc_uni_mv_rd_addr);
	wr(VPU_DAE_WRAP_SW_ADDR8, reg_me_pb_uni_mv_rd_addr);
	wr(VPU_DAE_WRAP_SW_ADDR9, reg_me_cur_rdmif_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR10, reg_me_pre_rdmif_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR11, reg_me_nxt_luma_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR12, reg_me_nxt_chrm_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR13, reg_me_mix_wrmif_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR14, reg_mevp_mc_mv_wr_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR15, reg_mevp_mc_logo_wr_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR16, reg_mevp_mc_logo_rd_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR17, reg_nrme_info_rdmif_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR18, reg_nrme_info_wrmif_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR19, reg_nrme_mtn_rdmif_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR20, reg_nrme_mtn_wrmif_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR21, reg_nrme_alp_wrmif_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR22, reg_dae_dime_tfbf_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR23, reg_dae_dctgrd_wrmif_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR24, dctdivr_wrmif_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR25, dctdir_wrmif_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR26, reg_logo_pre_iir_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR27, reg_logo_cur_iir_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR28, reg_logo_pre_scc_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR29, reg_logo_cur_scc_baddr);
	wr(VPU_DAE_WRAP_SW_ADDR30, reg_iplogo_wrmif_baddr);
	wr(VPU_DAE_WRAP_SW_MERO, reg_mero_wrmif_baddr);
}

