/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _DPSS_PROC_IRQ_TBC_V1_H_
#define _DPSS_PROC_IRQ_TBC_V1_H_

//void dpss_proc_irq_sw(int32_t intVector);

#define ACC(a, b) (((a) == (b)) ? 0 : ((a) + 1))

extern u32 fnr_dpe_obuf_wrpt;
extern u32 fnr_src_ibuf_num;

extern u32 src1_dpe_obuf_wrpt;

extern u32 fnr_dae_obuf_wrpt;
extern u32 fnr_dae_ibuf_wrpt;
extern u32 fnr_dae_ibuf_rdpt;
extern u32 fnr_dae_ibuf[16][2];
extern u32 fnr_dpe_obuf_num;

extern u32 fnr_dpe_obuf_free_idx;

//(src0_nrdi_frc_en == 1) ?
//	((rd(DPSS_FBUF_BUF_CTRL) >> 16) & 0x1f) : (rd(FRC_AUTO_MODE_BUFF_NUM) & 0x1f);
extern   u32 reg_inp_ibuf_num;
extern   u32 reg_inp_obuf_num;
extern   u32 reg_fnr_dae_ibuf_num;
extern   u32 reg_fnr_dae_obuf_num; //todo
extern   u32 reg_fnr_dpe_ibuf_num; //todo
extern   u32 reg_fnr_dpe_obuf_num;
extern   u32 reg_frc_dae_ibuf_num;
extern   u32 reg_frc_dae_obuf_num;
extern   u32 reg_frc_dpe_ibuf_num;
extern   u32 reg_frc_dpe_obuf_num;

extern   u32 g_dpss_di_ignore_num;
extern   u32 g_dpss_nr_ignore_num;
extern   u32 g_dpss_inp_frm_cnt_adj;
extern   u32 g_dpss_dae_frm_cnt_src0_adj;
extern   u32 g_dpss_dae_frm_cnt_src1_adj;
extern   u32 g_dpss_dae_frm_cnt_src2_adj;
extern   u32 g_dpss_dpe_nr_frm_cnt_adj;
extern   u32 g_dpss_dpe_di_frm_cnt_adj;
extern   u32 dpss_fnr_vpp_link;

extern  u32 inp_ibuf_level;
extern  u32 inp_obuf_level;
extern  u32 frc_dae_ibuf_level;
extern  u32 frc_dae_obuf_level;

extern  u32 fnr_dae_ibuf_level;
extern  u32 fnr_dae_obuf_level;
extern  u32 fnr_dpe_ibuf_level;
extern  u32 fnr_dpe_obuf_level;

extern  u32 viu_frm_idx;
extern  u32 src_frm_idx;
extern  u32 inp_frm_idx;
extern  u32 frc_dae_frm_idx;
extern  u32 fnr_dae_frm_idx;
extern  u32 fnr_dpe_frm_idx;
extern  u32 dae0_frm_rst_cnt;
extern  u32 dae1_frm_rst_cnt;
extern  u32 dae2_frm_rst_cnt;	//version:0312 add

extern  u32 dae_cur_phase;
extern  u32 dae_pre_phase;
extern  u32 dpe_cur_phase;
extern  u32 dpe_pre_phase;
extern  u32 dpe_pre_idx;
extern  u32 dpe_cur_idx;
extern  u32 mc_proc_phs;
extern  enum DPSS_FILM_MODE pre_film_mode;

extern  u32 inp_din_idx;
extern  u32 inp_proc_idx;
extern  u64 inp_fifo[16];

extern  u32 frc_dae_din_idx;
extern  u32 frc_dae_proc_idx;
extern  u64 frc_dae_fifo[8];

extern  u32 fnr_dae_din_idx;
extern  u64 fnr_dae_fifo[8];

extern  u32 frc_dpe_din_idx;
extern  u32 frc_dpe_proc_idx;
extern  u64 frc_dpe_fifo[8];

extern  u32 fnr_dpe_din_idx;
extern  u32 fnr_dpe_proc_idx;
extern  u64 fnr_dpe_fifo[8];
extern  u64 fnr_dpe_idx_fifo[8];
extern  u32 fnr_dpe_idx_rdpt;
extern  u32 fnr_dpe_idx_wrpt;
extern  u32 cfg_dae0_level;
extern  u32 cfg_dae1_level;

void cfg_dst_ibuf_rdy(u64 info0, u64 info1); //version:0312 add
void dae_out_addr_rd(enum PRM_SRC_IDX src_idx);

void cfg_dpss_fnr_dae_raddr(void);
void cfg_dpss_fnr_dpe_raddr(void);

void cfg_frc_dae_ibuf_rdy(u64 info);
void cfg_fnr_dae_ibuf_rdy(u64 info);

void cfg_dpss_dae0_fifo(u32 wren, u32 rden);
void cfg_dpss_dae1_fifo(u32 wren, u32 rden);

void cfg_fnr_dpe_ibuf_rdy(u64 info0, u64 info1);
void cfg_frc_dpe_ibuf_rdy(u64 info0, u64 info1, u32 dae_ibuf_rdy_mode, u32 dae_frm_idx);

void cfg_dpss_trigger(enum DPSS_TRIGGER_MODE mode);
void cfg_dpss_inp_free(u32 src_idx);
void cfg_dpss_dae_free(u64 info);
void cfg_dpss_dpe_free(u32 idx);

u32 dpss_calc_film_step(u32 film_mode, u32 film_phs);

//==============================================================//
// Process_Irq
//==============================================================//

void hw_dpe_out_addr_rd_tbc(enum PRM_SRC_IDX src_idx);
void dae_out_addr_rd_tbc(enum PRM_SRC_IDX src_idx);

#endif // _DPSS_PROC_IRQ_TBC_V1_H_
