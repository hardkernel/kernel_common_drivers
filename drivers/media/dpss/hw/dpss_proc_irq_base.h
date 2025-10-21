/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _DPSS_PROC_IRQ_BASE_H_
#define _DPSS_PROC_IRQ_BASE_H_

#define DPSS_INP_EVENT 0//src0:inp frm rst
#define DPSS_DAE0_EVENT1//src0:frc_me, frc_nr_me
#define DPSS_DAE1_EVENT2//src0: nr only me
#define DPSS_DAE2_EVENT3//src1: di only
#define DPSS_DPE0_EVENT4//dpe_only_mc_int
#define DPSS_DPE1_EVENT5//dpe_only_nr_int
#define DPSS_DPE2_EVENT6//dpe_di_nr_int

#define SRC0_DATA_EVENT 7//src0 data load
#define SRC1_DATA_EVENT 8//src1 data load

#define DPSS_WORK_MODE_EVENT 9//work mode  set
#define DPSS_DAE2_DONE_EVENT 11//src1: di done
#define DPSS_DPE2_DONE_EVENT 12//dpe_di_done

#define DPSS_SEC_EVENT 16//for tb to set ddr sec

#define DPSS_VDIN_EVENT 20

extern u32 src0_frm_idx_cnt;
extern u32 src1_frm_idx_cnt;
extern u32 dpss_inp_frm_cnt;
extern u32 dpss_dae_frm_cnt_src0;
extern u32 dpss_dae_frm_cnt_src1;
extern u32 dpss_dae_frm_cnt_src2;
extern u32 dpss_dpe_mc_frm_cnt;
extern u32 dpss_dpe_mcp_frm_cnt;
extern u32 dpss_dpe_nr_frm_cnt;
extern u32 dpss_dpe_di_frm_cnt;
extern u32 dpss_rls0_frm_cnt;
extern u32 dpss_rls1_frm_cnt;
extern u32 dpss_disp0_frm_cnt;
extern u32 dpss_disp1_frm_cnt;

extern bool fnr_disp_obuf_rdy;
extern bool src0_disp_obuf_rdy;
extern bool src1_disp_obuf_rdy;
extern bool frc_vpp_link_obuf_rdy;

extern u32 sec_data;

extern u32 frc_obuf_status, frc_ocnt_status;
extern u32 dpss_disp_timeout_cnt;
extern u32 mc_need_match;

extern u32 src0_pre_idx_cnt;
extern u32 dpe_pre_phs0_flg;
extern u32 dpe_start_num;
extern u32 dae_rd_less_one;
extern u32 vdin_dpss_phs;

extern u32 mc_repeat_frm; //1114
//extern Vpu_queue inp_sec_bufQ;	//1114
//extern Vpu_queue dae_sec_bufQ;	//1114
//extern Vpu_queue dpe_sec_bufQ;	//1114
extern struct DPSS_SEC dpss_sec;	//1114

//==============================================================//
// Process_Irq
//==============================================================//
//void dpe_out_addr_rd(enum PRM_SRC_IDX src_idx);
void dae_out_addr_rd(enum PRM_SRC_IDX src_idx);
void process_irq_load_src0(s32 intvector,
	int src_end_num, bool src0_nrdi_frc_en);
void process_irq_load_src1(s32 intvector,
	int src_end_num);
//void process_inp_frm_rst(int32_t intVector, int frm_cnt_adj);

//void cfg_nr_vfcd_mode(struct PRM_DPSS_TOP prm_top, struct PRM_DPSS_DPE prm_dpe);
//void process_frc_rls_int(int32_t intVector);

//ary add:
void hw_cfg_dpss_dae(enum DPSS_WORK_MODE dae_mode,
	struct PRM_DPSS_TOP *prm_top,
	struct PRM_DPSS_DAE *prm_dae);
void hw_cfg_dpss_dpe(enum DPSS_WORK_MODE dpe_mode,
	struct PRM_DPSS_TOP *prm_top,
	struct PRM_DPSS_DPE *prm_dpe,
	struct AA_PPS_TOP_TYPE *nr_pps_cfg);
void hw_cfg_dpss_dpe_no_buf_update(enum DPSS_WORK_MODE dpe_mode,
	struct PRM_DPSS_TOP *prm_top,
	struct PRM_DPSS_DPE *prm_dpe);

void hw_check_nr_ro(int frm_idx,
	enum PRM_SRC_IDX dump_src_idx);
void hw_dpe_out_addr_rd(enum PRM_SRC_IDX src_idx);
void hw_dpe_update_interlace(struct PRM_DPSS_TOP
	*prm_top, struct PRM_DPSS_DPE *prm_dpe);
void hw_dae_update_interlace(struct PRM_DPSS_TOP *prm_top,
			     struct PRM_DPSS_DAE *prm_dae,
			     unsigned int cnt);
void hw_dae_update_interlace_cfg(struct PRM_DPSS_TOP *prm_top,
				unsigned int cnt);

#endif //_DPSS_PROC_IRQ_BASE_H_
