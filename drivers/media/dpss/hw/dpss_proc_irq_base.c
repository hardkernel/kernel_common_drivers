// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "define.h"
#include "dpss.h"
#include "dpss_ro_check.h"
#include "dpss_phslut.h"
//#include "dpss_lib.h"
#include "dpss_mc.h"
#include "dpss_dae.h"
#include "dpss_dpe.h"
#include "dpss_vdi.h"
#include "../sys_def.h"

#include "dpss_proc_irq_base.h"

#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
#include <linux/amlogic/media/amvecm/amvecm.h>
#endif

//TRANS trans; //12-17
u32 g_mc_phase;
u32 g_mc_phase_check; //for mc done lp mode check
unsigned int dpss_ei_sel;

//==============================================================//
// Process_Irq
//==============================================================//
//dpe_out_addr_rd
void hw_dpe_out_addr_rd(enum PRM_SRC_IDX src_idx)
{
	u32 dpss_dpe_out_idx;
	u32 dpss_dblk_idx;
	u32 ro_baddr;

	if (src_idx == SRC_IDX_NR) {
		dpss_dpe_out_idx = rd(DPSS_FBUF_DPE_LOOP_IDX) & 0xff;
		dpss_dblk_idx = (rd(DPSS_FBUF_DPE_LOOP_IDX) >> 28) & 0xf;

	} else if (src_idx == SRC_IDX_FRC) {
		dpss_dpe_out_idx = rd(FRC_DPSS_DPE_OUT_IDX) & 0xff;
		dpss_dblk_idx = 0;

	} else {
		dpss_dpe_out_idx = rd(DPSS_REGOFST_VDICTRL +
			DPSS_FBUF_DPE_LOOP_IDX) & 0xff;
		dpss_dblk_idx = (rd(DPSS_REGOFST_VDICTRL +
			DPSS_FBUF_DPE_LOOP_IDX) >> 28) & 0xf;
	}

	dbg_h2("%s %02d 0x%0x\n", __func__, src_idx, dpss_dpe_out_idx);
	dbg_h2("\t:dblk_out_addr_rd %0x\n", dpss_dblk_idx);
	ro_baddr = rd(DPSS_DPE_RDMA_WR_BADDR);
	dbg_h2("\t:dblk_out_addr_rd 0x%x\n", ro_baddr);
}

void dae_out_addr_rd(enum PRM_SRC_IDX src_idx)
{
	u32 dpss_dae_out_idx;

	if (src_idx == SRC_IDX_NR) {
		dpss_dae_out_idx = rd(DPSS_FBUF_DAE_LOOP_IDX) & 0xff;

	} else if (src_idx == SRC_IDX_FRC) {
		dpss_dae_out_idx = 0;//TODO rd(DPSS_FBUF_DPE_LOOP_IDX)&0xff;

	} else {
		dpss_dae_out_idx = rd(DPSS_REGOFST_VDICTRL +
			DPSS_FBUF_DAE_LOOP_IDX) & 0xff;
	}

	send_event_to_sv(DPSS_DAE_OUT_ADDR,
		(dpss_dae_out_idx | (src_idx << 28)));
	stimulus_wait_event_done(DPSS_DAE_OUT_ADDR);
}

void inp_sec_cfg(int frm_idx)
{
#ifdef DPSS_SEC_TEST
	u32 pre_rd_sec;
	u32 cur_rd_sec;
	u32 cur_wr_sec;
	u32 dpss_inp_idx;
	u32 ro_frc_inp_rpix_fid; //pix mif idx
	bool     cur_wr_idx = frm_idx & 0x1;
	//0 means cur use 4200_0000, 1 means cur use 4240_0000 //TODO

	dpss_inp_idx = rd(DPSS_FRC_INP_BUF_FID);
	ro_frc_inp_rpix_fid = (dpss_inp_idx >> 16) & 0xf;
	cur_rd_sec          = (dpss_sec.inp_sec[0] >>
		ro_frc_inp_rpix_fid) & 0x1;
#ifdef VERIFICATION

	if (sec_rd(DPSS_INP_WR_SEC_SEL) & 0x1) {
#else

	if (rd(DPSS_INP_WR_SEC_SEL) & 0x1) {
#endif
		pre_rd_sec          = (dpss_sec.inp_sec[1] >> (!cur_wr_idx)) & 0x1;
		cur_wr_sec          = (dpss_sec.inp_sec[1] >> cur_wr_idx) & 0x1;
		dbg_h2("INP sec0: cur_rd_sec %x  cur_wr_sec/pre_rd_sec %x\n",
			cur_rd_sec, cur_wr_sec);
		wr(FRC_INP_SW_FRM_INFO,
			pre_rd_sec | (cur_rd_sec << 4) | (cur_wr_sec << 8));

		//update inp pre rd/cur wr sec
		if (cur_wr_idx == 0) {
			send_event_to_sv(DPSS_SEC_EVENT,
				cur_wr_sec | (pre_rd_sec << 1) | (0 << 17) |
				(1 << 22));//22 bit for update flag

		} else {
			send_event_to_sv(DPSS_SEC_EVENT,
				pre_rd_sec | (cur_wr_sec << 1) | (0 << 17) |
				(1 << 22));//22 bit for update flag
		}

	} else {
		pre_rd_sec          = cur_rd_sec;
		cur_wr_sec          = cur_rd_sec;
		dbg_h2("INP sec1: cur_rd_sec %x  cur_wr_sec/pre_rd_sec %x\n",
			cur_rd_sec, cur_wr_sec);
		wr(FRC_INP_SW_FRM_INFO,
			pre_rd_sec | (cur_rd_sec << 4) | (cur_wr_sec << 8));

		//update inp pre rd/cur wr sec
		if (cur_wr_idx == 0) {
			send_event_to_sv(DPSS_SEC_EVENT,
				cur_wr_sec | (pre_rd_sec << 1) | (0 << 17) |
				(1 << 22));//22 bit for update flag

		} else {
			send_event_to_sv(DPSS_SEC_EVENT,
				pre_rd_sec | (cur_wr_sec << 1) | (0 << 17) |
				(1 << 22));//22 bit for update flag
		}
	}

	stimulus_wait_event_done(DPSS_SEC_EVENT);
#endif
}

void dpe_sec_cfg(enum PRM_SRC_IDX src_idx,
	int frm_idx)
{
#ifdef DPSS_SEC_TEST
	u32 dpss_dpe_out_idx;
	u32 ro_dpe_cur0_rd_idx;
	u32 ro_dpe_pre1_rd_idx;
	u32 ro_dpe_pre2_rd_idx;
	u32 ro_dpe_nrmv_rd_idx;
	u32 ro_dpe_dimv_rd_idx;
	u32 ro_dpe_nr_wr_idx;
	u32 ro_dpe_di_wr_idx;
	u32  pre2_rd_sec;
	u32  pre1_rd_sec;
	u32  cur_rd_sec;
	u32  wr0_sec;
	u32  wr1_sec;
	u32  wr2_sec;
	u32  ds_wr_sec;
	u32  dcnt_pix_rd_sec;
	u32  dcnt_rd_sec;   //7 8 9 rdmif
	u32  dcnt_rd_sec_tmp;

	if (src_idx == SRC_IDX_NR) {
		dpss_dpe_out_idx   = rd(DPSS_FBUF_DPE_LOOP_IDX);
		ro_dpe_cur0_rd_idx = (dpss_dpe_out_idx >> 24) & 0xf;
		ro_dpe_pre1_rd_idx = (dpss_dpe_out_idx >> 20) & 0xf;
		ro_dpe_pre2_rd_idx = (dpss_dpe_out_idx >> 16) & 0xf;
		ro_dpe_nrmv_rd_idx = (dpss_dpe_out_idx >> 12) & 0xf;
		ro_dpe_nr_wr_idx   = (dpss_dpe_out_idx >> 4) & 0xf;
		ro_dpe_di_wr_idx   = (dpss_dpe_out_idx >> 0) & 0xf;
		cur_rd_sec  = (dpss_sec.nrdi_rd_sec[0] >>
			ro_dpe_cur0_rd_idx) & 0x1;
		wr2_sec     = (dpss_sec.nrdi_wr_sec[2] >>
			ro_dpe_nr_wr_idx) & 0x1;
		ds_wr_sec   = (dpss_sec.nrdi_wr_sec[3] >>
			ro_dpe_nr_wr_idx) & 0x1;

		if (frm_idx < 1) {
			pre1_rd_sec = (dpss_sec.nrdi_rd_sec[0] >>
				ro_dpe_pre1_rd_idx) & 0x1;

		} else {
			pre1_rd_sec = (dpss_sec.nrdi_rd_sec[1] >>
				ro_dpe_pre1_rd_idx) & 0x1;
		}

		dcnt_pix_rd_sec    = (dpss_sec.dae_sec[0] >>
			ro_dpe_cur0_rd_idx) & 0x1;
		dcnt_rd_sec        = (dpss_sec.dae_sec[2] >>
			ro_dpe_nrmv_rd_idx) & 0x1;
		dbg_h2("DPE_NR:ro_dpe_cur0_rd_idx %x sec %x\n",
			ro_dpe_cur0_rd_idx, cur_rd_sec);
		dbg_h2("DPE_NR:ro_dpe_nr_wr_idx %x sec %x\n",
			ro_dpe_nr_wr_idx, wr2_sec);
		dbg_h2("DPE_NR:ro_dpe_nr_wr_idx %x sec %x\n",
			ro_dpe_nr_wr_idx, ds_wr_sec);
		dbg_h2("DPE_NR:ro_dpe_pre1_rd_idx %x sec %x\n",
			ro_dpe_pre1_rd_idx, pre1_rd_sec);
		//for dcnt
		dbg_h2("DPE_NR:ro_dpe_cur0_rd_idx %x dcn sec %x\n",
			ro_dpe_cur0_rd_idx, dcnt_pix_rd_sec);
		dbg_h2("DPE_NR:ro_dpe_nrmv_rd_idx %x sec %x\n",
			ro_dpe_nrmv_rd_idx, dcnt_rd_sec);
		w_reg_bit(DPSS_DPE_INTF_AFBCD1,
			((dcnt_pix_rd_sec << 3) | (dcnt_rd_sec << 2) |
			(dcnt_rd_sec << 1) | (dcnt_rd_sec << 0)), 7,
			4); // 7 8 9 10
		w_reg_bit(VPU_AXIRD_SECURE_EN, cur_rd_sec, 0,
			1);  //nr cur rd
		w_reg_bit(VPU_AXIRD_SECURE_EN, pre1_rd_sec, 1,
			1); //nr pre rd
#ifdef VERIFICATION
		sec_w_reg_bit(DPSS_DPE_SECURE_REG, wr2_sec,
			28, 1);// wrmif2
		sec_w_reg_bit(DPSS_DPE_SECURE_REG, ds_wr_sec,
			29, 1);// ds
#else
		w_reg_bit(DPSS_DPE_SECURE_REG, wr2_sec,   28,
			1);// wrmif2
		w_reg_bit(DPSS_DPE_SECURE_REG, ds_wr_sec, 29,
			1);// ds
#endif

	} else if (src_idx == SRC_IDX_FRC) {
		dpss_dpe_out_idx     = rd(FRC_DPSS_DISP_BUFF_INFO_0);
		ro_dpe_pre1_rd_idx   = (dpss_dpe_out_idx >> 2) & 0xf;
		ro_dpe_cur0_rd_idx   = (dpss_dpe_out_idx >> 8) & 0xf;
		pre1_rd_sec  = (dpss_sec.mc_rd_sec[0] >>
			ro_dpe_pre1_rd_idx) & 0x1;
		cur_rd_sec   = (dpss_sec.mc_rd_sec[0] >>
			ro_dpe_cur0_rd_idx) & 0x1;
		dbg_h2("DPE_MC: %x sec %x\n", ro_dpe_pre1_rd_idx,
			pre1_rd_sec);
		dbg_h2("DPE_MC: %x sec %x\n", ro_dpe_cur0_rd_idx,
			cur_rd_sec);
		w_reg_bit(VPU_AXIRD_SECURE_EN, cur_rd_sec, 4, 1);
		w_reg_bit(VPU_AXIRD_SECURE_EN, pre1_rd_sec, 5, 1);
#ifdef VERIFICATION
		sec_w_reg_bit(DPSS_DPE_SECURE_REG, 1, 0, 1); //force sec
		sec_w_reg_bit(DPSS_DPE_SECURE_REG,
			(cur_rd_sec | pre1_rd_sec), 8, 1); //for dpss dout sec
#else
		w_reg_bit(DPSS_DPE_SECURE_REG, 1, 0, 1); //force sec
		w_reg_bit(DPSS_DPE_SECURE_REG,
			(cur_rd_sec | pre1_rd_sec), 8, 1); //for dpss dout sec
#endif

	} else {
		dpss_dpe_out_idx   = rd(DPSS_REGOFST_VDICTRL +
			DPSS_FBUF_DPE_LOOP_IDX);
		ro_dpe_cur0_rd_idx = (dpss_dpe_out_idx >> 24) & 0xf;
		ro_dpe_pre1_rd_idx = (dpss_dpe_out_idx >> 20) & 0xf;
		ro_dpe_pre2_rd_idx = (dpss_dpe_out_idx >> 16) & 0xf;
		ro_dpe_nr_wr_idx   = (dpss_dpe_out_idx >> 4) & 0xf;
		ro_dpe_di_wr_idx   = (dpss_dpe_out_idx >> 0) & 0xf;
		cur_rd_sec  = (dpss_sec.nrdi_rd_sec[1] >>
			ro_dpe_cur0_rd_idx) & 0x1;
		wr0_sec     = (dpss_sec.nrdi_wr_sec[0] >>
			ro_dpe_nr_wr_idx) & 0x1;
		wr2_sec     = (dpss_sec.nrdi_wr_sec[2] >>
			ro_dpe_di_wr_idx) & 0x1;
		ds_wr_sec   = (dpss_sec.nrdi_wr_sec[3] >>
			ro_dpe_di_wr_idx) & 0x1;

		if (frm_idx < 1) {
			pre1_rd_sec = (dpss_sec.nrdi_rd_sec[1] >>
				ro_dpe_pre1_rd_idx) & 0x1;

		} else {
			pre1_rd_sec = (dpss_sec.nrdi_rd_sec[2] >>
				ro_dpe_pre1_rd_idx) & 0x1;
		}

		if (frm_idx < 2) {
			pre2_rd_sec = (dpss_sec.nrdi_rd_sec[1] >>
				ro_dpe_pre2_rd_idx) & 0x1;

		} else {
			pre2_rd_sec = (dpss_sec.nrdi_rd_sec[3] >>
				ro_dpe_pre2_rd_idx) & 0x1;
		}

		dbg_h2("DPE_NR:ro_dpe_nr_wr_idx %x sec %x\n",
			ro_dpe_nr_wr_idx, wr0_sec);
		dbg_h2("DPE_NR:ro_dpe_di_wr_idx %x sec %x\n",
			ro_dpe_di_wr_idx, wr2_sec);
		dbg_h2("DPE_NR:ro_dpe_di_wr_idx %x sec %x\n",
			ro_dpe_di_wr_idx, ds_wr_sec);
		dbg_h2("DPE_NR:ro_dpe_pre1_rd_idx %x sec %x\n",
			ro_dpe_pre1_rd_idx, pre1_rd_sec);
		dbg_h2("DPE_NR:ro_dpe_pre2_rd_idx %x sec %x\n",
			ro_dpe_pre2_rd_idx, pre2_rd_sec);
		w_reg_bit(VPU_AXIRD_SECURE_EN, cur_rd_sec, 1, 1); //cur
		w_reg_bit(VPU_AXIRD_SECURE_EN, pre1_rd_sec, 2, 1); //pre1
		w_reg_bit(VPU_AXIRD_SECURE_EN, pre2_rd_sec, 3, 1); //pre2
#ifdef VERIFICATION
		sec_w_reg_bit(DPSS_DPE_SECURE_REG, wr0_sec, 26, 1);// wrmif0 //nr out
		sec_w_reg_bit(DPSS_DPE_SECURE_REG, wr2_sec, 28, 1);// wrmif1 //di out
		sec_w_reg_bit(DPSS_DPE_SECURE_REG, ds_wr_sec, 29, 1);// ds
#else
		w_reg_bit(DPSS_DPE_SECURE_REG, wr0_sec, 26, 1);// wrmif0 //nr out
		w_reg_bit(DPSS_DPE_SECURE_REG, wr2_sec, 28, 1);// wrmif1 //di out
		w_reg_bit(DPSS_DPE_SECURE_REG, ds_wr_sec, 29, 1);// ds
#endif
	}

#endif
}

void dae_sec_cfg(enum PRM_SRC_IDX src_idx, int frm_idx)
{
#ifdef DPSS_SEC_TEST
	u32 ro_dae_loop_idx;
	u32 ro_dae_cur0_rd_idx;
	u32 ro_dae_mixo_wr_idx;
	u32 ro_dae_pre1_rd_idx;
	u32 ro_dae_pre2_rd_idx;
	u32 ro_dae_info_wr_idx;
	u32 ro_dae_info_rd_idx;
	bool pre_sec;
	bool cur_sec;
	bool wrmif_sec_en;
	bool yuv_sec;
	bool dcnt_wr_sec;

	if (src_idx == SRC_IDX_NR) {
		ro_dae_loop_idx    = rd(DPSS_FBUF_DAE_LOOP_IDX);
		ro_dae_cur0_rd_idx = (ro_dae_loop_idx >> 20) & 0xf;
		ro_dae_mixo_wr_idx = (ro_dae_loop_idx >> 16) & 0xf;
		ro_dae_pre1_rd_idx = (ro_dae_loop_idx >> 12) & 0xf;
		ro_dae_pre2_rd_idx = (ro_dae_loop_idx >> 8) & 0xf;
		ro_dae_info_wr_idx = (ro_dae_loop_idx >> 4) & 0xf;
		ro_dae_info_rd_idx = (ro_dae_loop_idx >> 0) & 0xf;
		pre_sec      = (dpss_sec.dae_sec[1] >>
			ro_dae_pre2_rd_idx) & 0x1;
		cur_sec      = (dpss_sec.dae_sec[1] >>
			ro_dae_pre1_rd_idx) & 0x1;
		wrmif_sec_en = (dpss_sec.dae_sec[1] >>
			ro_dae_mixo_wr_idx) & 0x1;
		yuv_sec      = (dpss_sec.dae_sec[0] >>
			ro_dae_cur0_rd_idx) & 0x1;
		dcnt_wr_sec  = (dpss_sec.dae_sec[2] >>
			ro_dae_info_wr_idx) & 0x1;
		dbg_h2("[SRC_IDX_NR] ro_dae_mixo_wr_idx: %x sec %x\n",
			ro_dae_mixo_wr_idx, wrmif_sec_en);
		dbg_h2("[SRC_IDX_NR] ro_dae_pre2_rd_idx: %x sec %x\n",
			ro_dae_pre2_rd_idx, pre_sec);
		dbg_h2("[SRC_IDX_NR] ro_dae_pre1_rd_idx: %x sec %x\n",
			ro_dae_pre1_rd_idx, cur_sec);
		dbg_h2("[SRC_IDX_NR] ro_dae_cur0_rd_idx: %x sec %x\n",
			ro_dae_cur0_rd_idx, yuv_sec);
		w_reg_bit(VPU_DAE_WRAP_SW_INFO, cur_sec, 16, 1);
		w_reg_bit(VPU_DAE_WRAP_SW_INFO, yuv_sec, 24, 1);
		// nxt wrmif sec test
#ifdef VERIFICATION
		Sec_w_reg_bit(VPU_DAE_WRAP_SEC_MODE,
			wrmif_sec_en, 0, 1);//reg_wr_sec_en
		Sec_w_reg_bit(VPU_DAE_WRAP_SEC_MODE, 7, 10, 3); //dcbt wrmf sel
		Sec_w_reg_bit(VPU_DAE_WRAP_SEC_MODE,
			dcnt_wr_sec | (dcnt_wr_sec << 1) |
			(dcnt_wr_sec << 2), 2, 3);
#else
		w_reg_bit(VPU_DAE_WRAP_SEC_MODE, wrmif_sec_en, 0, 1);//reg_wr_sec_en
		w_reg_bit(VPU_DAE_WRAP_SEC_MODE, 7, 10, 3); //dcbt wrmf sel
		w_reg_bit(VPU_DAE_WRAP_SEC_MODE,
			dcnt_wr_sec | (dcnt_wr_sec << 1) |
			(dcnt_wr_sec << 2), 2, 3);
#endif
		//for dolby  pix
		w_reg_bit(VPU_VDIN_SEC_IN, yuv_sec, 16, 1);
		w_reg_bit(VPU_VDIN_SEC_IN, yuv_sec, 17, 1);

	} else if (src_idx == SRC_IDX_FRC) {
		ro_dae_loop_idx    = rd(FRC_REG_OUT_FID);
		u32 ro_frc_opre_fid = (ro_dae_loop_idx >> 16) & 0xf;
		u32 ro_frc_ocur_fid = (ro_dae_loop_idx >> 12) & 0xf;
		u32 ro_frc_onex_fid = (ro_dae_loop_idx >> 8) & 0xf;
		u32 ro_frc_opre_logo_fid = (ro_dae_loop_idx >> 4) & 0xf;
		u32 ro_frc_ocur_logo_fid = (ro_dae_loop_idx >> 0) & 0xf;
		u32 ro_dae_in_frm_idx = rd(FRC_DAE_IN_FRM_IDX) & 0xff;

		pre_sec      = (dpss_sec.dae_sec[1] >>
			ro_frc_opre_fid) & 0x1;
		cur_sec      = (dpss_sec.dae_sec[1] >>
			ro_frc_ocur_fid) & 0x1;
		wrmif_sec_en = (dpss_sec.dae_sec[1] >>
			ro_frc_onex_fid) & 0x1;  //wrmif 6 nxt dae, wrmif4 cur scc
		yuv_sec      = (dpss_sec.dae_sec[0] >>
			ro_dae_in_frm_idx) & 0x1;
		dbg_h2("[FRC]: ro_frc_onex_fid: %x sec %x\n",
			ro_frc_onex_fid, wrmif_sec_en);
		dbg_h2("[FRC]: ro_frc_opre_fid: %x sec %x\n",
			ro_frc_opre_fid, pre_sec);
		dbg_h2("[FRC]: ro_frc_ocur_fid: %x sec %x\n",
			ro_frc_ocur_fid, cur_sec);
		dbg_h2("[FRC]: ro_frc_onex_fid: %x sec %x\n",
			ro_frc_onex_fid, yuv_sec);
		w_reg_bit(VPU_DAE_WRAP_SW_INFO, pre_sec, 12, 1);
		w_reg_bit(VPU_DAE_WRAP_SW_INFO, cur_sec, 16, 1);
		w_reg_bit(VPU_DAE_WRAP_SW_INFO, yuv_sec, 24, 1);
		//nxt wrmif sec test
#ifdef VERIFICATION
		Sec_w_reg_bit(VPU_DAE_WRAP_SEC_MODE,
			wrmif_sec_en, 0, 1);//reg_wr_sec_en
#else
		w_reg_bit(VPU_DAE_WRAP_SEC_MODE,
			wrmif_sec_en, 0, 1);//reg_wr_sec_en
#endif
		dbg_h2("dae sec for frm_idx%d\n", frm_idx);

		u32 ro_logo_iir_buf_pre_idx = rd(FRC_LOGO_IIR_BUF_CUR_IDX) & 0x1;

		dbg_h2("ro_logo_iir_buf_pre_idx %d\n",
			ro_logo_iir_buf_pre_idx);
		w_reg_bit(VPU_DAE_WRAP_MEVP_CTRL, cur_sec, 31, 1);  //iir pre rd

		//update inp pre iir/cur iir
		if (ro_logo_iir_buf_pre_idx == 0) {
			send_event_to_sv(DPSS_SEC_EVENT,
				cur_sec | (wrmif_sec_en << 1) | (2 << 17) |
				(1 << 22));//22 bit for update flag

		} else {
			send_event_to_sv(DPSS_SEC_EVENT,
				wrmif_sec_en | (cur_sec << 1) | (2 << 17) |
				(1 << 22));//22 bit for update flag
		}

	} else if (src_idx == SRC_IDX_DI1) { //for NRDI SRC0
		ro_dae_loop_idx    = rd(DPSS_FBUF_DAE_LOOP_IDX);
		ro_dae_cur0_rd_idx = (ro_dae_loop_idx >> 20) & 0xf;
		ro_dae_mixo_wr_idx = (ro_dae_loop_idx >> 16) & 0xf;
		ro_dae_pre1_rd_idx = (ro_dae_loop_idx >> 12) & 0xf;
		ro_dae_pre2_rd_idx = (ro_dae_loop_idx >> 8) & 0xf;
		ro_dae_info_wr_idx = (ro_dae_loop_idx >> 4) & 0xf;
		ro_dae_info_rd_idx = (ro_dae_loop_idx >> 0) & 0xf;
		pre_sec      = (dpss_sec.dae_sec[1] >>
			ro_dae_pre2_rd_idx) & 0x1;
		cur_sec      = (dpss_sec.dae_sec[1] >>
			ro_dae_pre1_rd_idx) & 0x1;
		wrmif_sec_en = (dpss_sec.dae_sec[1] >>
			ro_dae_mixo_wr_idx) & 0x1;
		yuv_sec      = (dpss_sec.dae_sec[0] >>
			ro_dae_cur0_rd_idx) & 0x1;
		dbg_h2("[SRC_IDX_DI1] ro_dae_mixo_wr_idx: %x sec %x\n",
			ro_dae_mixo_wr_idx, wrmif_sec_en);
		dbg_h2("[SRC_IDX_DI1] ro_dae_pre2_rd_idx: %x sec %x\n",
			ro_dae_pre2_rd_idx, pre_sec);
		dbg_h2("[SRC_IDX_DI1] ro_dae_pre1_rd_idx: %x sec %x\n",
			ro_dae_pre1_rd_idx, cur_sec);
		dbg_h2("[SRC_IDX_DI1] ro_dae_cur0_rd_idx: %x sec %x\n",
			ro_dae_cur0_rd_idx, yuv_sec);
		w_reg_bit(VPU_DAE_WRAP_SW_INFO, cur_sec, 16, 1);
		w_reg_bit(VPU_DAE_WRAP_SW_INFO, yuv_sec, 24, 1);
		// nxt wrmif sec test
#ifdef VERIFICATION
		Sec_w_reg_bit(VPU_DAE_WRAP_SEC_MODE,
			wrmif_sec_en, 0, 1);//reg_wr_sec_en
#else
		w_reg_bit(VPU_DAE_WRAP_SEC_MODE, wrmif_sec_en,
			0, 1);//reg_wr_sec_en
#endif

	} else {
		ro_dae_loop_idx    = rd(DPSS_REGOFST_VDICTRL +
			DPSS_FBUF_DAE_LOOP_IDX);
		ro_dae_cur0_rd_idx = (ro_dae_loop_idx >> 20) & 0xf;
		ro_dae_mixo_wr_idx = (ro_dae_loop_idx >> 16) & 0xf;
		ro_dae_pre1_rd_idx = (ro_dae_loop_idx >> 12) & 0xf;
		ro_dae_pre2_rd_idx = (ro_dae_loop_idx >> 8) & 0xf;
		ro_dae_info_wr_idx = (ro_dae_loop_idx >> 4) & 0xf;
		ro_dae_info_rd_idx = (ro_dae_loop_idx >> 0) & 0xf;
		pre_sec      = (dpss_sec.dae_sec[1] >>
			ro_dae_pre2_rd_idx) & 0x1;
		cur_sec      = (dpss_sec.dae_sec[1] >>
			ro_dae_pre1_rd_idx) & 0x1;
		wrmif_sec_en = (dpss_sec.dae_sec[1] >>
			ro_dae_mixo_wr_idx) & 0x1;
		yuv_sec      = (dpss_sec.nrdi_rd_sec[1] >>
			ro_dae_cur0_rd_idx) & 0x1; //di use pix data
		dbg_h2("ro_dae_mixo_wr_idx: %x sec %x\n",
			ro_dae_mixo_wr_idx, wrmif_sec_en);
		dbg_h2("ro_dae_pre2_rd_idx: %x sec %x\n",
			ro_dae_pre2_rd_idx, pre_sec);
		dbg_h2("ro_dae_pre1_rd_idx: %x sec %x\n",
			ro_dae_pre1_rd_idx, cur_sec);
		dbg_h2("ro_dae_cur0_rd_idx: %x sec %x\n",
			ro_dae_cur0_rd_idx, yuv_sec);
		w_reg_bit(VPU_DAE_WRAP_SW_INFO, pre_sec, 12, 1);
		w_reg_bit(VPU_DAE_WRAP_SW_INFO, cur_sec, 16, 1);
		w_reg_bit(VPU_DAE_WRAP_SW_INFO, yuv_sec, 24, 1);
		// nxt wrmif sec test
#ifdef VERIFICATION
		Sec_w_reg_bit(VPU_DAE_WRAP_SEC_MODE,
			wrmif_sec_en, 0, 1);//reg_wr_sec_en
#else
		w_reg_bit(VPU_DAE_WRAP_SEC_MODE, wrmif_sec_en,
			0, 1);//reg_wr_sec_en
#endif
	}

#endif
}

//ary add
void hw_process_dae1_frm_rst(int cfg_seed,
	int frm_cnt0_adj,
	int frm_cnt1_adj,
	int frm_cnt2_adj,
	enum DPSS_WORK_MODE dae_nr_mode,
	bool src0_nrdi_frc_en,
	struct PRM_DPSS_TOP *prm_top,
	struct PRM_DPSS_DAE *prm_dae)
{
	//if((intVector == GIC_INT_VEC(SPI_DPSS_DAE1_INT)) && (src0_nrdi_frc_en == 0))
	//sec test
	if (src0_nrdi_frc_en != 0)
		return;

#ifdef DPSS_SEC_TEST
	dae_sec_cfg(SRC_IDX_NR, 0);
#endif
	dbg_h2("%s frame %02d\n", __func__,
		dpss_dae_frm_cnt_src1);
	int amdv_prl_mode = prm_top->dolby_mode == DOLBY_DPSS_PRL_MODE;

	if (amdv_prl_mode == 0) {
		if (cfg_seed)
			cfg_dpss_dae_me_rand_seed(1, frm_cnt0_adj,
				frm_cnt1_adj, frm_cnt2_adj);

		if (dae_nr_mode == DAE_BYPS_MODE) {
			/*must cfg in irq when enable dct*/
			prm_top->frame_count = 0;
			hw_cfg_dpss_dae(DAE_BYPS_MODE, prm_top, prm_dae);

		} else {
			if (frm_cnt1_adj > 0) { //For bitmatch start
				check_dae_ro(dpss_dae_frm_cnt_src1 - 1,
					SRC_IDX_NR);
				//ary send_event_to_sv(DPSS_DAE1_EVENT, dpss_dae_frm_cnt_src1);
			}

			//12-05 if(cfg_seed)
			//12-05	cfg_dpss_dae_me_rand_seed(1,frm_cnt0_adj,frm_cnt1_adj,frm_cnt2_adj);
			/*must cfg in irq when enable dct*/
			prm_top->frame_count = dpss_dae_frm_cnt_src1;
			hw_cfg_dpss_dae(dae_nr_mode, prm_top, prm_dae);
		}
		w_reg_bit(FRC_ME_TOP_CTRL2, 1, 16, 1);	//for p source read PD
		dbg_h2("%s:prm_dpss_dae:%x\n", __func__, rd(FRC_ME_TOP_CTRL2));
		if (prm_top->di_interlace) //0501
			hw_dae_update_interlace(prm_top, prm_dae, dpss_dae_frm_cnt_src1);
		hw_dae_update_interlace_cfg(prm_top, dpss_dae_frm_cnt_src1);
		if (frm_cnt0_adj > 0) { //For bitmatch start
			check_dae_ro(dpss_dae_frm_cnt_src0 - 1,
				SRC_IDX_FRC);
		}

		if (frm_cnt2_adj > 0) { //For bitmatch start
			check_dae_ro(dpss_dae_frm_cnt_src2 - 1,
				SRC_IDX_DI0);
		}

		cfg_dae_triggle();
		dpss_dae_frm_cnt_src1 = dpss_dae_frm_cnt_src1 + 1;
	} else {
		dpss_dae_frm_cnt_src1++;
	}
}

//cp process_dae1_nrdi_frm_rst
void hw_process_dae1_nrdi_frm_rst(int cfg_seed,
	int frm_cnt0_adj, int frm_cnt1_adj,
	int frm_cnt2_adj,
	bool src0_nrdi_frc_en, struct PRM_DPSS_TOP *prm_top,
	struct PRM_DPSS_DAE *prm_dae)
{
	int       reg_vdi_src_idx = 8;
	enum PRM_SRC_IDX src_idx = SRC_IDX_DI0;

	if (src0_nrdi_frc_en != 1)
		return;

	dbg_h2("%s:prm_dpss_dae:%p\n", __func__, prm_dae);
	reg_vdi_src_idx = (rd(DPSS_TOP_FUNC_CTRL) >> 24) & 0xf;
	src_idx = (enum PRM_SRC_IDX)reg_vdi_src_idx;
	dbg_h2("%s:  frame %02d check%02d\n", __func__,
		dpss_dae_frm_cnt_src1, frm_cnt1_adj);
	//sec test
	dae_sec_cfg(src_idx, 0);

	//cfg dae seed
	if (cfg_seed)
		cfg_dpss_dae_me_rand_seed(1, frm_cnt0_adj,
			frm_cnt1_adj, frm_cnt2_adj);

	//cfg dae me_is_top
	cfg_dae_me_is_top(frm_cnt1_adj);
	w_reg_bit(FRC_ME_TOP_CTRL2, 1, 16, 1);	//for p source read PD
	dbg_h2("%s:prm_dpss_dae:%x\n", __func__, rd(FRC_ME_TOP_CTRL2));
	if (frm_cnt1_adj > 0) { //For bitmatch start
		check_dae_ro(dpss_dae_frm_cnt_src1 - 1, src_idx);
		dbg_h2("\treg_vdi_src_idx %02d\n",
			reg_vdi_src_idx);
		send_event_to_sv(DPSS_DAE2_EVENT,
			dpss_dae_frm_cnt_src1 | (reg_vdi_src_idx << 28));
	}
	prm_top->frame_count = dpss_dae_frm_cnt_src1;
	hw_cfg_dpss_dae(DAE_VDI_MODE, prm_top, prm_dae);
	if (prm_top->di_interlace) //0501
		hw_dae_update_interlace(prm_top, prm_dae, dpss_dae_frm_cnt_src1);
	hw_dae_update_interlace_cfg(prm_top, dpss_dae_frm_cnt_src1);
	dae_out_addr_rd(SRC_IDX_NR); //use nr tbc loop idx

	if (frm_cnt0_adj > 0) { //For bitmatch start
		check_dae_ro(dpss_dae_frm_cnt_src0 - 1,
			SRC_IDX_FRC);
	}

	if (frm_cnt2_adj > 0) { //For bitmatch start
		check_dae_ro(dpss_dae_frm_cnt_src2 - 1,
			SRC_IDX_DI0);
	}

	cfg_dae_triggle();
	dpss_dae_frm_cnt_src1 = dpss_dae_frm_cnt_src1 + 1;
}

//cp process_dae2_frm_rst
void hw_process_dae2_frm_rst(int cfg_seed,
	int frm_cnt0_adj,
	int frm_cnt1_adj,
	int frm_cnt2_adj,
	bool src0_nrdi_frc_en,
	struct PRM_DPSS_TOP *prm_top,
	struct PRM_DPSS_DAE *prm_dae)
{
	int       reg_vdi_src_idx = 8;
	enum PRM_SRC_IDX src_idx = SRC_IDX_DI0;

	reg_vdi_src_idx = (rd(DPSS_TOP_FUNC_CTRL1) >> 16) & 0xf;
	src_idx = (enum PRM_SRC_IDX)reg_vdi_src_idx;
	dbg_h2("src2 frame %02d  for src %d\n",
		dpss_dae_frm_cnt_src2, src_idx);
	dae_sec_cfg(src_idx, 0); //sec test

	if (cfg_seed)
		cfg_dpss_dae_me_rand_seed(1, frm_cnt0_adj,
			frm_cnt1_adj, frm_cnt2_adj);

	cfg_dae_me_is_top(frm_cnt2_adj);

	if (frm_cnt2_adj > 0) { //For bitmatch start
		check_dae_ro(dpss_dae_frm_cnt_src2, src_idx);
		send_event_to_sv(DPSS_DAE2_EVENT,
			dpss_dae_frm_cnt_src2 | (reg_vdi_src_idx << 28));
	}

#ifdef SRC1_PSRC_NR
	hw_cfg_dpss_dae(DAE_NR_MODE, prm_top, prm_dae);
	w_reg_bit(VPU_DAE_WRAP_SW_EN, 7, 16, 4); //reg_dae_src_idx
	w_reg_bit(VPU_DAE_WRAP_SW_SEL0, 1, 4, 1); //reg_din_sw_en[4]
#else
	/*must cfg in irq when enable dct*/
	prm_top->frame_count = dpss_dae_frm_cnt_src2;
	hw_cfg_dpss_dae(prm_top->dae_nr_mode, prm_top, prm_dae);

#endif

	if (prm_top->di_interlace) //0501
		hw_dae_update_interlace(prm_top, prm_dae, dpss_dae_frm_cnt_src2);
	hw_dae_update_interlace_cfg(prm_top, dpss_dae_frm_cnt_src2);
	dae_out_addr_rd(src_idx);

	if (frm_cnt1_adj > 0) { //For bitmatch start
		if (src0_nrdi_frc_en == 0)
			check_dae_ro(dpss_dae_frm_cnt_src1 - 1,
				SRC_IDX_NR);
		else
			check_dae_ro(dpss_dae_frm_cnt_src1 - 1,
				SRC_IDX_DI1);
	}

	if (frm_cnt0_adj > 0) { //For bitmatch start
		check_dae_ro(dpss_dae_frm_cnt_src0 - 1,
			SRC_IDX_FRC);
	}

	cfg_dae_triggle();
	dpss_dae_frm_cnt_src2++;
}

void hw_process_dpe1_frm_rst(int cfg_slc,
	int slc_num,
	int nr_frm_cnt_adj,
	int di_frm_cnt_adj,
	int dpss_nr_ignore_num,
	enum DPSS_WORK_MODE dpe_nr_mode,
	bool src0_nrdi_frc_en,
	struct PRM_DPSS_TOP *prm_top,
	struct PRM_DPSS_DPE *prm_dpe)
{
	//if((intVector == GIC_INT_VEC(SPI_DPSS_DPE1_INT)) && (src0_nrdi_frc_en == 0))
	struct AA_PPS_TOP_TYPE *pps = &g_nr_pps_cfg;
	if (src0_nrdi_frc_en != 0)
		return;

	dbg_h2("%s frame %02d\n", __func__,
		dpss_dpe_nr_frm_cnt);
#ifdef DPSS_SEC_TEST
	dpe_sec_cfg(SRC_IDX_NR, dpss_dpe_nr_frm_cnt);
#endif
	dbg_h2("\t dolby_mode=%02d\n", prm_top->dolby_mode);
	int amdv_prl_mode = prm_top->dolby_mode == DOLBY_DPSS_PRL_MODE;//src0:dv_on,nrdi_off

#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
	//amvecm_update_hdr_path_for_dpss(NULL);
	//dbg_h2("%s amvecm_update_hdr_path_for_dpss[%d]\n",
	//	__func__, dpss_dpe_nr_frm_cnt);
#endif

	if (nr_frm_cnt_adj > 0 && amdv_prl_mode == 0) { //For bitmatch start
		hw_check_nr_ro(dpss_dpe_nr_frm_cnt, SRC_IDX_NR);
		//ary send_event_to_sv(DPSS_DPE1_EVENT, dpss_dpe_nr_frm_cnt);
	}

	if (prm_top->bbd_parallel == 1)
		dbg_h2("in bbd parallel mode,do nothing\n");
		//in bbd parallel mode, cannot change din_dmsq_init config, otherwise src1 affected
	else if (nr_frm_cnt_adj == 0)
		w_reg_bit(DPSS_DPE_DIN_DMSQ_INIT, 1, 0, 1);//set dms init for first frame
	else
		w_reg_bit(DPSS_DPE_DIN_DMSQ_INIT, 0, 0, 1);

	if (di_frm_cnt_adj > 0) {
		check_di_ro(dpss_dpe_di_frm_cnt, SRC_IDX_DI0);
		hw_check_nr_ro(dpss_dpe_di_frm_cnt, SRC_IDX_DI0);
	}

	if (cfg_slc)
		prm_top->dpe_slc_num   = slc_num;

	int di2pps_idx;
	unsigned long nr_yaddr;
	u32 i;

	nr_yaddr = rd(DPSS_DPE_DIN_WR_BADDR4) >> 5;
	di2pps_idx = -1;
	for (i = 0; i < prm_top->num_dpe_o; i++) {
		if (nr_yaddr == prm_top->src0_dio_fbuf_yaddr[i]) {
			di2pps_idx = i;
			if (pps->pps_en)
				prm_top->fbuf_is_pps[i] = 1;
			else
				prm_top->fbuf_is_pps[i] = 0;
			break;
		}
	}
	dbg_pps0("pps[%d] pps test di 0x%x,%d\n", di2pps_idx,
		rd(DPSS_DPE_DIN_WR_BADDR4), prm_top->fbuf_is_pps[i]);
	if (pps->pps_en) {
		nr_yaddr = rd(DPSS_DPE_DIN_WR_BADDR0) >> 5;
		di2pps_idx = -1;
		for (i = 0; i < prm_top->num_dpe_o; i++) {
			if (nr_yaddr == prm_top->src0_nro_fbuf_yaddr[i]) {
				di2pps_idx = i;
				break;
			}
		}
		dbg_pps0("pps2[%d] pps test nr 0x%x\n", di2pps_idx,
			rd(DPSS_DPE_DIN_WR_BADDR0));
	}

	if (prm_top->bbd_parallel == 1) {
		//no need trig dpe config & wrmif
		//reuse dv trigger for bbd
		w_reg_bit(DPSS_DPE_INTF_DBG, 0x3, 11, 2);
		w_reg_bit(DPSS_BBD_ONLY_CTRL, 1, 0, 1);
		cfg_dpe_dv_triggle();

	} else if (amdv_prl_mode == 0) {
		if (dpss_dpe_nr_frm_cnt < dpss_nr_ignore_num &&
		    dpe_nr_mode != DPE_BBD_ONLY) {
			prm_top->is_current = true;
			hw_cfg_dpss_dpe(DPE_NR_BYPS, prm_top, prm_dpe,
				&g_nr_pps_cfg);

			dpss_last_dpe_mode = DPE_NR_BYPS;
			dpss_last_dpe_src = 0;
		} else {
			if (dpss_last_dpe_mode != dpe_nr_mode ||
			    dpss_last_dpe_src != 0) {
				prm_top->is_current = false;
				hw_cfg_dpss_dpe(dpe_nr_mode, prm_top, prm_dpe,
					&g_nr_pps_cfg);
				dpss_last_dpe_mode = dpe_nr_mode;
				dpss_last_dpe_src = 0;
			}
		}
		//if (!prm_dpe->di_en)
		if (dpss_dpe_nr_frm_cnt >= dpss_nr_ignore_num) {
			prm_top->is_current = false;
			dbg_h2("%s:%d\n", __func__, dpe_nr_mode);
			nr_force_config(prm_top, prm_dpe);
		}
		//cfg_nr_vfcd_mode(prm_top,prm_dpe);
		hw_dpe_out_addr_rd(SRC_IDX_NR);
		if ((dpss_dbg_en_logo & C_BIT0) &&
		    (prm_top->mode_drct2 || prm_top->mode_drct)) { //tmp: for dd
			w_reg_bit_vc(0x4512, 0, 31, 1);
			dbg_h2("force :reg[0x4512]=0x%x\n", rd_vc(4512));
		}

		cfg_dpe_triggle(prm_top);
		//12-05 dpss_dpe_nr_frm_cnt = dpss_dpe_nr_frm_cnt + 1;
	} else {
		//for prl mode:
		if (dpss_dpe_nr_frm_cnt == 0) {
			hw_cfg_dpss_dpe(dpe_nr_mode,
				prm_top, prm_dpe, &g_nr_pps_cfg);
			cfg_dpe_triggle(prm_top);
			dbg_h2("%s:prl mode:dpe:%d\n", __func__, prm_dpe->dpe_mode);
		} else if (dpss_dbg_step & C_BIT0) {
			hw_cfg_dpss_dpe(dpe_nr_mode,
				prm_top, prm_dpe, &g_nr_pps_cfg);
		}
	}

	//12-05	else {
	//12-05		hw_cfg_dpss_dpe(DPE_NR_BYPS,prm_top,prm_dpe, &g_nr_pps_cfg);
	//12-05	}
	dpss_dpe_nr_frm_cnt++;

	if (prm_top->dolby_mode >= DOLBY_DPSS_MODE)
		cfg_dpe_dv_triggle();
}

//process_dpe1_nrdi_frm_rst
void hw_process_dpe1_nrdi_frm_rst(int cfg_slc,
	int slc_num, int nr_frm_cnt_adj,
	int di_frm_cnt_adj, int dpss_di_ignore_num,
	enum DPSS_WORK_MODE dpe_di_mode, bool src0_nrdi_frc_en,
	struct PRM_DPSS_TOP *prm_top,
	struct PRM_DPSS_DPE *prm_dpe)
{
	struct AA_PPS_TOP_TYPE *pps = &g_nr_pps_cfg;
	int di2pps_idx;
	unsigned long di2pps_yaddr;
	unsigned long di2pps_caddr;
	unsigned long nr_yaddr;
	u32 i;
	if (src0_nrdi_frc_en != 1)
		return;

	//if((intVector == GIC_INT_VEC(SPI_DPSS_DPE1_INT))&(src0_nrdi_frc_en == 1)){
#ifdef DPSS_SEC_TEST
	w_reg_bit(DPSS_DPE_INTF_AFBCD1, 0x1e00, 0, 30);
#endif
	int reg_vdi_src_idx;
	//ary no use        u32 ro_baddr;
	enum PRM_SRC_IDX src_idx =  SRC_IDX_DI0;

	dbg_h2("%s: frame %02d\n", __func__, dpss_dpe_nr_frm_cnt);
	if (nr_frm_cnt_adj > 0) {
		reg_vdi_src_idx = (rd(DPSS_TOP_FUNC_CTRL) >> 24) & 0xf;
		src_idx = (enum PRM_SRC_IDX)reg_vdi_src_idx;
		dbg_h2("\t reg_vdi_src_idx %02d frm%0d\n",
			reg_vdi_src_idx, dpss_dpe_nr_frm_cnt);
		hw_check_nr_ro(dpss_dpe_nr_frm_cnt - 1, src_idx);
		check_di_ro((dpss_dpe_nr_frm_cnt - 1), src_idx);
		send_event_to_sv(DPSS_DPE2_EVENT,
			(dpss_dpe_nr_frm_cnt | reg_vdi_src_idx << 28));
	}

	if (di_frm_cnt_adj > 0) {
		reg_vdi_src_idx = (rd(DPSS_TOP_FUNC_CTRL1) >> 16) & 0xf;
		dbg_h2("\t reg_vdi_src_idx %02d frm%0d\n",
			reg_vdi_src_idx, dpss_dpe_di_frm_cnt);
		hw_check_nr_ro(dpss_dpe_di_frm_cnt - 1, SRC_IDX_DI0);
		check_di_ro(dpss_dpe_di_frm_cnt - 1, SRC_IDX_DI0);
		send_event_to_sv(DPSS_DPE2_EVENT,
			(dpss_dpe_di_frm_cnt | reg_vdi_src_idx << 28));
	}

#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
	//amvecm_update_hdr_path_for_dpss(NULL);
	//dbg_h2("%s amvecm_update_hdr_path_for_dpss[%d]\n",
	//	__func__, dpss_dpe_nr_frm_cnt);
#endif

	cfg_dpss_vdi_istop(dpss_dpe_nr_frm_cnt, prm_top->f_top);

	if (nr_frm_cnt_adj == 0) { //(only first di need force 1)
		w_reg_bit(DPSS_DPE_DIN_DMSQ_INIT, 1, 0, 1);
		w_reg_bit(VPU_DI_HW_SLC_INFO, 1, 8, 1); //reg_di_fst_frm;

	} else {
		w_reg_bit(DPSS_DPE_DIN_DMSQ_INIT, 0, 0, 1);
		w_reg_bit(VPU_DI_HW_SLC_INFO, 0, 8, 1); //reg_di_fst_frm;
	}

	prm_dpe->dpss_dpe_di_frm_cnt = dpss_dpe_nr_frm_cnt;	//yu.zong 2024-12-06
	if (dpss_force_nr_debug > 0xf) {
		wr(0x00 + VID_WMIF_LUMA_BADDR, rd(DPSS_DPE_DIN_WR_BADDR0));
		wr(0x00 + VID_WMIF_CHRM_BADDR, rd(DPSS_DPE_DIN_WR_BADDR1));
		wr(0x40 + VID_WMIF_LUMA_BADDR, rd(DPSS_DPE_DIN_WR_BADDR4));
		wr(0x40 + VID_WMIF_CHRM_BADDR, rd(DPSS_DPE_DIN_WR_BADDR5));
		wr(0x60 + VID_WMIF_LUMA_BADDR, rd(DPSS_DPE_DIN_DWR_BADDR2));
		wr(0x60 + VID_WMIF_CHRM_BADDR, rd(DPSS_DPE_DIN_DWR_BADDR3));
	}
	dbg_h2("%s:this force addr: 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n", __func__,
			rd(DPSS_DPE_DIN_WR_BADDR0), rd(DPSS_DPE_DIN_WR_BADDR1),
			rd(DPSS_DPE_DIN_WR_BADDR4), rd(DPSS_DPE_DIN_WR_BADDR5),
			rd(DPSS_DPE_DIN_DWR_BADDR2), rd(DPSS_DPE_DIN_DWR_BADDR3));

	if (dpss_last_dpe_mode != prm_top->dpe_nr_mode ||
	    dpss_last_dpe_src != 0	||
	    dpss_dpe_nr_frm_cnt == dpss_di_ignore_num) {
		prm_top->is_current = false;
		hw_cfg_dpss_dpe(prm_top->dpe_nr_mode, prm_top, prm_dpe, &g_nr_pps_cfg);
		dpss_last_dpe_mode = prm_top->dpe_nr_mode;
		dpss_last_dpe_src = 0;
	}
	//cfg_nr_vfcd_mode(prm_top,prm_dpe);

	if (dpss_dpe_nr_frm_cnt < dpss_di_ignore_num) {
		if (dpss_dpe_nr_frm_cnt == 0) {
			dpss_vbe_proc_byp(1, pps->pps_en);//di
			dpss_last_dpe_src = 3;
		} else {
			if (dpss_slt_mode) {
				dpss_vbe_proc_byp(1, pps->pps_en);
			} else {
	//07-10			hw_cfg_dpss_dpe(dpe_di_mode, prm_top, prm_dpe, &g_nr_pps_cfg);
				w_reg_bit(VPU_DI_BLEND_EI_POST_EN_MODE, 1, 4, 4);
				dbg_h1("%s i src second frame ei\n", __func__);
			}
		}
	} else {
		nr_force_config(prm_top, prm_dpe);
		dbg_h1("%s SLT bypass for first two frames\n", __func__);
		if (!dpss_slt_mode)
			if (dpss_dpe_nr_frm_cnt == dpss_di_ignore_num) {
				dpss_ei_sel = (dpss_ei_sel >> 4) & 0xf;
				w_reg_bit(VPU_DI_BLEND_EI_POST_EN_MODE, dpss_ei_sel, 4, 4);
			}
	}
		hw_dpe_update_interlace(prm_top, prm_dpe);

		hw_dpe_out_addr_rd(SRC_IDX_NR); //use nr tbc loop idx
	if (pps->di2pps_en) {
		nr_yaddr = rd(DPSS_DPE_DIN_WR_BADDR4) >> 5;
		di2pps_idx = -1;
		for (i = 0; i < prm_top->num_dpe_o; i++) {
			if (nr_yaddr == prm_top->src0_dio_fbuf_yaddr[i]) {
				di2pps_idx = i;
				break;
			}
		}
		if (di2pps_idx < 0 || di2pps_idx >= prm_top->num_dpe_o) {
			DBG_ERR("No match buffer found for 0x%lx\n", nr_yaddr);
			return;
		}
		di2pps_yaddr = prm_top->src0_di2pps_buf_yaddr[di2pps_idx];
		di2pps_caddr = prm_top->src0_di2pps_buf_caddr[di2pps_idx];
		wr(DPSS_DPE_PPS_WR_BADDR0, di2pps_yaddr << 5);
		wr(DPSS_DPE_PPS_WR_BADDR1, di2pps_caddr << 5);
		dbg_h1("pps[%d] pps test 1 0x%x\n", di2pps_idx,
			rd(DPSS_DPE_DIN_WR_BADDR4));
	}
		cfg_dpe_triggle(prm_top);
		dpss_dpe_nr_frm_cnt++;
	}

//cp process_dpe2_frm_rst
void hw_process_dpe2_frm_rst(int cfg_slc,
	int slc_num,
	int nr_frm_cnt_adj,
	int di_frm_cnt_adj,
	int dpss_nr_ignore_num,
	int dpss_di_ignore_num,
	enum DPSS_WORK_MODE dpe_di_mode,
	bool src0_nrdi_frc_en,
	struct PRM_DPSS_TOP *prm_top,
	struct PRM_DPSS_DPE *prm_dpe)
{
	dbg_h2("%s:prm_top:%p\n", __func__, prm_top);
#ifdef DPSS_SEC_TEST
	w_reg_bit(DPSS_DPE_INTF_AFBCD1, 0x1e00, 0, 30);
#endif
	int reg_vdi_src_idx;
	enum PRM_SRC_IDX src_idx =  SRC_IDX_DI0;

	reg_vdi_src_idx = (rd(DPSS_TOP_FUNC_CTRL1) >> 16) & 0xf;
	src_idx = (enum PRM_SRC_IDX)reg_vdi_src_idx;
	dbg_h2("%s frame %02d  for src %d\n", __func__,
		dpss_dpe_di_frm_cnt, src_idx);
	dpe_sec_cfg(src_idx, dpss_dpe_di_frm_cnt);

	if (di_frm_cnt_adj > 0) {
		dbg_h2(" %02d frm%0d\n", reg_vdi_src_idx,
			dpss_dpe_di_frm_cnt);
		hw_check_nr_ro(dpss_dpe_di_frm_cnt, src_idx);
		check_di_ro((dpss_dpe_di_frm_cnt), src_idx);
		send_event_to_sv(DPSS_DPE2_EVENT,
			(dpss_dpe_di_frm_cnt | reg_vdi_src_idx << 28));
	}

	if (nr_frm_cnt_adj > 0) { //For bitmatch start
		if (src0_nrdi_frc_en == 0) {
			check_nr_ro(dpss_dpe_nr_frm_cnt - 1, SRC_IDX_NR);

		} else {
			reg_vdi_src_idx = (rd(DPSS_TOP_FUNC_CTRL) >> 24) & 0xf;
			dbg_h2("reg_vdi_src_idx %02d  frm %0d\n",
				reg_vdi_src_idx, dpss_dpe_nr_frm_cnt);
			hw_check_nr_ro(dpss_dpe_nr_frm_cnt, SRC_IDX_DI1);
			check_di_ro((dpss_dpe_nr_frm_cnt), SRC_IDX_DI1);
		}
	}
	afbcd_update_addr(prm_top, dpss_dpe_di_frm_cnt);
	cfg_dpss_vdi_istop(dpss_dpe_di_frm_cnt, prm_top->f_top);

	if (di_frm_cnt_adj == 0) { //(only first di need force 1)
		w_reg_bit(DPSS_DPE_DIN_DMSQ_INIT, 1, 0, 1);
		w_reg_bit(VPU_DI_HW_SLC_INFO, 1, 8, 1); //reg_di_fst_frm;

	} else {
		w_reg_bit(DPSS_DPE_DIN_DMSQ_INIT, 0, 0, 1);
		w_reg_bit(VPU_DI_HW_SLC_INFO, 0, 8, 1); //reg_di_fst_frm;
	}

	prm_dpe->dpss_dpe_di_frm_cnt =
		dpss_dpe_di_frm_cnt;	//yu.zong 2024-12-06

	//reg_src1_nr_di_loop, =0 psrc src1 nr
	//12-06	bool src1_nr_isrc_flag = ((rd(DPSS_DPE_TOP_CTRL0) >> 23) & 0x1);
	//12-06	if((dpe_di_mode==DPE_NR_MODE) && (src1_nr_isrc_flag==0)) {
	//12-06		dpss_di_ignore_num = dpss_nr_ignore_num;
	//12-06	}
	dbg_h2("%s:dpss_di_ignore_num = %d\n", __func__, dpss_di_ignore_num);
	if (dpss_dpe_di_frm_cnt < dpss_di_ignore_num) {
		if (dpe_di_mode == DPE_NR_SRC1_MODE ||
		    dpe_di_mode == DPE_NR_MODE) {
			if (prm_top->dae_dpe_cfg_en == 1  ||
			    dpss_last_dpe_mode != DPE_NR_BYPS ||
			    dpss_last_dpe_src != 1) {
				hw_cfg_dpss_dpe(DPE_NR_BYPS, prm_top, prm_dpe,
					&g_nr_pps_cfg2);
				dpss_last_dpe_mode = DPE_NR_BYPS;
				dpss_last_dpe_src = 1;
			}

			//w_reg_bit(DPSS_DPE_TOP_SW_SEL1,0,4,1);//32+5
			//w_reg_bit(DPSS_DPE_TOP_SW_SEL1,0,7,1);//32+8
			if (dpe_di_mode == DPE_NR_SRC1_MODE)
				w_reg_bit(DPSS_BBD_ONLY_CTRL, 0, 1, 1); //nr_cur_only

		} else if (prm_top->dolby_mode == DOLBY_DPSS_PRL_MODE) {
			dbg_h2("%s:db prl\n", __func__);
			if (dpss_last_dpe_mode != prm_top->dpe_nr_mode ||
			    dpss_last_dpe_src != 1	||
			    dpss_dpe_di_frm_cnt == dpss_di_ignore_num) {
				hw_cfg_dpss_dpe(prm_top->dpe_nr_mode, prm_top, prm_dpe,
					&g_nr_pps_cfg2);
				dpss_last_dpe_mode = prm_top->dpe_nr_mode;
				dpss_last_dpe_src = 1;
			}
			if (dpss_dpe_di_frm_cnt == 0) {
				dpss_vbe_proc_byp(2, 0);//
				dpss_last_dpe_src = 3;
			} else {
				w_reg_bit(VPU_DI_BLEND_EI_POST_EN_MODE, 1, 4, 4);
				dbg_h1("%s second frame set ei mode\n", __func__);
			}
		} else {
			if (dpss_last_dpe_mode != prm_top->dpe_nr_mode ||
			    dpss_last_dpe_src != 1	||
			    dpss_dpe_di_frm_cnt == dpss_di_ignore_num) {
				hw_cfg_dpss_dpe(prm_top->dpe_nr_mode, prm_top, prm_dpe,
					&g_nr_pps_cfg2);
				dpss_last_dpe_mode = prm_top->dpe_nr_mode;
				dpss_last_dpe_src = 1;
			}
			if (dpss_slt_mode) {
				dpss_vbe_proc_byp(1, 0);
			} else {
				if (dpss_dpe_di_frm_cnt == 0) {
					dpss_vbe_proc_byp(1, 0);//di
					dpss_last_dpe_src = 3;
				} else {
	//0710				hw_cfg_dpss_dpe(dpe_di_mode, prm_top, prm_dpe,
	//0710					&g_nr_pps_cfg);
					w_reg_bit(VPU_DI_BLEND_EI_POST_EN_MODE, 1, 4, 4);
					dbg_h1("%s second frame set ei mode\n", __func__);
				}
			}
		}
		prm_top->is_current = true;
	} else if (dpss_last_dpe_mode != prm_top->dpe_nr_mode ||
		   dpss_last_dpe_src != 1	||
		   dpss_dpe_di_frm_cnt == dpss_di_ignore_num) {
		prm_top->is_current = false;
		hw_cfg_dpss_dpe(prm_top->dpe_nr_mode, prm_top, prm_dpe,
			&g_nr_pps_cfg2);
		dpss_last_dpe_mode = prm_top->dpe_nr_mode;
		dpss_last_dpe_src = 1;
	}
	if (dpss_dpe_di_frm_cnt == dpss_di_ignore_num) {
		dpss_ei_sel = (dpss_ei_sel >> 4) & 0xf;
		w_reg_bit(VPU_DI_BLEND_EI_POST_EN_MODE, dpss_ei_sel, 4, 4);
	}
	hw_dpe_update_interlace(prm_top, prm_dpe);
	if (dpss_dpe_di_frm_cnt >=  dpss_di_ignore_num)
		nr_force_config(prm_top, prm_dpe);
	hw_dpe_out_addr_rd(src_idx);
	cfg_dpe_triggle(prm_top);
	dpss_dpe_di_frm_cnt++;
}

#ifdef NO_USED//nr_frc_0113
void hw_process_disp_done_int(int src_mode,
	enum DPSS_WORK_MODE dpe_nr_mode,
	enum DPSS_WORK_MODE dpe_di_mode,
	s32 cfg_slc,
	s32 slc_num,
	u32 dpss_dpe_nr_frm_cnt_adj,
	u32 dpss_dpe_di_frm_cnt_adj,
	u32 dpss_nr_ignore_num,
	u32 dpss_di_ignore_num,
	struct PRM_DPSS_TOP *prm_top,
	struct PRM_DPSS_DPE *prm_dpe)
#else
void hw_process_disp_done_int(char ch)
#endif
{
	//if(intVector==GIC_INT_VEC(SPI_VIU1_VSYNC))
	u32 dpss_frc_vpp_link;
	u32 dpss_fnr_vpp_link;
	u32 tmpseg = 0x0055F0F0;

	if (dpss_inp_frm_cnt == 0)
		return;

	dpss_frc_vpp_link = rd(FRC_DPSS_VPP_LINK);
	dpss_fnr_vpp_link = rd(DPSS_NR_VPP_LINK);
	dbg_h2("irq:vsync %d", ch);

	if (dpe_pre_phs0_flg)
		cfg_dpss_mc_triggle();

	if (dpss_frc_vpp_link == 1 &&
		dpss_fnr_vpp_link == 1) { //For DPSS error
		DBG_ERR("%s:Vpp link config error\n", __func__);
		return;
	}

	if (dpss_frc_vpp_link) { //for frc link
		if (frc_ocnt_status) { //one frame done
			dpss_disp0_frm_cnt++;
		}

		tmpseg |= (dpss_disp0_frm_cnt & 0x7);
		tmpseg |= ((dpss_inp_frm_cnt & 0x7) << 8);
		wr(FRC_MC_SEVEN_FLAG_POSI_AND_NUM11_NUM12, tmpseg);
		tmpseg = 0xA0A00000;
		tmpseg |= ((rd(FRC_REG_FILM_PHS_1) >> 8) & 0xf) << 16;
		wr(FRC_MC_SEVEN_FLAG_NUM13_NUM14_NUM15_NUM16, tmpseg);

		if (frc_vpp_link_obuf_rdy)
			frc_obuf_status = 1;

#ifndef MC_REPEAT_FRAME

		if (frc_vpp_link_obuf_rdy == 0 && mc_need_match) {
			mc_repeat_frm++;
			dbg_h2("[Process_Irq] DAE_OBUF NOT READY,NO NEW MC FRAME");
			dbg_h2("PLEASE CHECK TIMING GEN!!\n");

			if (mc_repeat_frm >= 3) {
				dbg_h2("[Process_Irq] MC FRAME REPEAT 3 TIMES");
				dbg_h2("PLEASE CHECK TIMING!!\n");
				return;
			}
		}

#endif
		dbg_h2("vsync, check mc_need_match= %d, force triggle mc\n", mc_need_match);
		//if(mc_need_match) {
		cfg_dpss_mc_triggle();
		g_mc_phase_check = g_mc_phase;
		//}

	} else if (dpss_fnr_vpp_link) { //for only fnr link
		DBG_WARN("fnr link: not support\n");

	} else {//for only fnr slice
		src0_disp_obuf_rdy = disp_obuf_trigger(FNR_DST_DONE);

		if (src0_disp_obuf_rdy) {
			cfg_dpss_done_trigger(FNR_DST_DONE);//last frame
			dpss_disp0_frm_cnt++;
		}
	}

	dbg_h2("disp 0 frame %02d\n", dpss_disp0_frm_cnt);
	src1_disp_obuf_rdy = disp_obuf_trigger(VDI_DST_DONE);

	if (src1_disp_obuf_rdy) { //for nrdi slice
		cfg_dpss_done_trigger(VDI_DST_DONE);//last frame
		dpss_disp1_frm_cnt++;
	}

	dbg_h2("dps 1 frame %02d\n", dpss_disp1_frm_cnt);
	dpss_disp_timeout_cnt++;

	//For DPSS error
	if (dpss_disp_timeout_cnt > 500) {
		dbg_h2("timeout,please check\n");
		return;
	}
}

void hw_process_inp_frm_rst(int frm_cnt_adj)
{
	static    enum DPSS_FILM_MODE pre_film_mode;
	enum DPSS_FILM_MODE ro_pd_film_mode;
	u32 ro_pd_film_phs;
	u32 data32;
	//inp frm rst irq
	dbg_h2("%s frame %02d\n", __func__, dpss_inp_frm_cnt);
	inp_sec_cfg(dpss_inp_frm_cnt);

	if (dpss_inp_frm_cnt == 0)
		pre_film_mode = DPSS_VIDEO;

	data32 = rd(FRC_REG_FILM_PHS_1);
	ro_pd_film_mode = (enum DPSS_FILM_MODE)((data32 >> 8) & 0xff);
	data32 = rd(FRC_REG_FILM_PHS_2);
	ro_pd_film_phs  = ((data32 >> 16) & 0xff);
	dbg_h2("\tfilm_mode switch from %0d to %0d\n",
		pre_film_mode, ro_pd_film_mode);

	//badedit
	if (rd(DPSS_FRC_FRM_DONE_SEL) & 0x1) { ////write inp frmae valid flag by fw
		config_inp_loop_tab(ro_pd_film_mode,
			ro_pd_film_phs, 1);
		w_reg_bit(DPSS_FRC_INP_FW_DONE, 1, 0, 1);

	} else if (pre_film_mode != ro_pd_film_mode) {
		pre_film_mode = ro_pd_film_mode;
		config_inp_loop_tab(ro_pd_film_mode, ro_pd_film_phs, 0);
	}

	if (frm_cnt_adj > 0) { //For bitmatch start
		send_event_to_sv(DPSS_INP_EVENT, dpss_inp_frm_cnt);
	}

	//cfg_inp_raddr();
	cfg_inp_triggle(1);
	dpss_inp_frm_cnt++;
	dbg_h2("\t frame %02d done\n", dpss_inp_frm_cnt);
}

void hw_process_pre_vsync_int(void)
{
	//static bool frc_vpp_link_obuf_rdy;
	//u32 dpss_frc_vpp_link = rd(FRC_DPSS_VPP_LINK);
	u32 mc_ibuf_stats = 0;

	mc_ibuf_stats = rd(DPSS_FRC_MC_IUFF_STATUS); //nr_frc_0113
	dbg_h2("irq:pre_vs:%s, dpe_pre_phs0_flg=%d, mc_ibuf_stats:%#X\n",
		__func__, dpe_pre_phs0_flg, mc_ibuf_stats);
	if (dpe_pre_phs0_flg) {
		dpss_dpe_info_cfg(0); //dpe mc
		cfg_dpss_mc_pre_triggle();
		frc_vpp_link_obuf_rdy = disp_obuf_trigger(FRC_DST_DONE);
		return;
	}

	if (frc_vpp_link_obuf_rdy)
		cfg_dpss_done_trigger(FRC_DST_DONE);//last frame

	frc_ocnt_status = frc_vpp_link_obuf_rdy;
	dbg_h2("link_obuf_rdy_pre: %d\n", frc_vpp_link_obuf_rdy);
	frc_vpp_link_obuf_rdy = disp_obuf_trigger(FRC_DST_DONE);
	dbg_h2("cur: %d\n", frc_vpp_link_obuf_rdy);

	if (frc_vpp_link_obuf_rdy) {
		dpss_dpe_info_cfg(0); //dpe mc
		dpe_sec_cfg(SRC_IDX_FRC, 0);
		mc_need_match = 1;
		mc_repeat_frm = 0;
	}

	g_mc_phase   = (rd(DPSS_DPE_MC_PHASE) >> 0) & 0xff;
	dbg_h2("-+g_mc_phase = %d\n", g_mc_phase);
	dbg_h2("pre_vs, check mc_need_match= %d, force triggle mc_pre\n",
		mc_need_match);
	//if(mc_need_match)
	cfg_dpss_mc_pre_triggle();
}

void hw_process_dae0_frm_rst(int cfg_seed,
	int frm_cnt0_adj,
	int frm_cnt1_adj,
	int frm_cnt2_adj,
	struct PRM_DPSS_TOP *prm_top,
	struct PRM_DPSS_DAE *prm_dae)
{
	dbg_h2("%s frame %02d\n", __func__,
		dpss_dae_frm_cnt_src0);
	dae_sec_cfg(SRC_IDX_FRC, frm_cnt0_adj); //sec test

	if (dae_rd_less_one) {
		if (dpss_dae_frm_cnt_src0 >= (dpe_start_num - 1)) {
			if (dpss_dae_frm_cnt_src0 == (dpe_start_num - 1))
				dpe_pre_phs0_flg = 1;
			else if (dpss_dae_frm_cnt_src0 == dpe_start_num)
				dpe_pre_phs0_flg = 0;
		}

		w_reg_bit(VPU_DAE_WRAP_GMAE_MODE,
			frm_cnt0_adj > 0, 23, 1); //12-05 add
	}

#ifdef ME_GLB_CLR_BYPASS_MC
	bool m_reg_me_glb_clr_bypass_mc_en = ((frm_cnt0_adj) % 2);

	dbg_h2("[Process_Irq]: m_reg_me_glb_clr_bypass_mc_en %02d\n",
		m_reg_me_glb_clr_bypass_mc_en);
	w_reg_bit(FRC_ME_GCV_EN,
		m_reg_me_glb_clr_bypass_mc_en, 1, 1);
#endif

	if (cfg_seed)
		cfg_dpss_dae_me_rand_seed(1, frm_cnt0_adj,
			frm_cnt1_adj, frm_cnt2_adj);

	if (frm_cnt0_adj > 0) { //For bitmatch start
		check_dae_ro(dpss_dae_frm_cnt_src0 - 1, SRC_IDX_FRC);
#ifndef DPSS_GAME_MODE
		send_event_to_sv(DPSS_DAE0_EVENT, dpss_dae_frm_cnt_src0);
#endif
	}

	if (frm_cnt1_adj > 0)
		check_dae_ro(dpss_dae_frm_cnt_src1 - 1, SRC_IDX_NR);

	if (frm_cnt2_adj > 0) //For bitmatch start
		check_dae_ro(dpss_dae_frm_cnt_src2 - 1, SRC_IDX_DI0);
	prm_top->frame_count = dpss_dae_frm_cnt_src0;
	if (prm_top->dae_dpe_cfg_en == 2)
		hw_cfg_dpss_dae(DAE_FRC_MODE, prm_top, prm_dae);

	cfg_dae_triggle();
	dpss_dae_frm_cnt_src0++;
}

