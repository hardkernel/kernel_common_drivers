// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "define.h"
#include "dpss_param.h"

#include "dpss.h"
#include "dpss_ro_check.h"
#include "dpss_phslut.h"
//#include "dpss_lib.h"
#include "dpss_mc.h"
#include "dpss_dae.h"
#include "dpss_dpe.h"
//#include "pss_vdi.h"
//#include "../dpss_rdma.h"

#include "dpss_proc_irq_tbc_sw_v1.h"

//static struct VPU_RDMA_TYPE g_vpu_rdma_wbuf;//tmp

//ary no used #include "vpu_rdma.c" //ary tmp

//#include "pkg_event_center.c"
#define ACC(a, b) (((a) == (b)) ? 0 : ((a) + 1))

//ary move from h
u32 fnr_dpe_obuf_wrpt;
u32 fnr_src_ibuf_num;
//ary repeat u32 src1_frm_idx_cnt  = 0;
u32 src1_dpe_obuf_wrpt;

//ary repeat u32 dpss_dae_frm_cnt_src2;
//ary repeat u32 dpss_dpe_di_frm_cnt  ;
//ary repeat u32 dpss_disp1_frm_cnt   ;
//ary repeat u32 dpss_disp_timeout_cnt;

u32 fnr_dae_obuf_wrpt;
u32 fnr_dae_ibuf_wrpt;
u32 fnr_dae_ibuf_rdpt;
u32 fnr_dae_ibuf[16][2];
u32 fnr_dpe_obuf_num = 16;
//u32 fnr_dpe_obuf_wrpt = 0;
u32 fnr_dpe_obuf_free_idx;
//u32 fnr_src_ibuf_num = 0;
//u32 sec_data = 0;
//(src0_nrdi_frc_en == 1) ?
//	((rd(DPSS_FBUF_BUF_CTRL)>>16) & 0x1f) : (rd(FRC_AUTO_MODE_BUFF_NUM) & 0x1f);
u32 reg_inp_ibuf_num = 16;
//ary: for tbc mode, use DPSS_HW_LOOP_IN_OUT_BUF_NUB replace reg_inp_ibuf_num
u32 reg_inp_obuf_num = 6;
u32 reg_fnr_dae_ibuf_num = 6;
u32 reg_fnr_dae_obuf_num = 6; //todo
u32 reg_fnr_dpe_ibuf_num = 6; //todo
u32 reg_fnr_dpe_obuf_num = 6;
u32 reg_frc_dae_ibuf_num = 6;
u32 reg_frc_dae_obuf_num = 4;
u32 reg_frc_dpe_ibuf_num = 6;
u32 reg_frc_dpe_obuf_num = 6;

u32 g_dpss_di_ignore_num; //ary add
u32 g_dpss_nr_ignore_num;//ary add
u32 g_dpss_inp_frm_cnt_adj;	//ary add
u32 g_dpss_dae_frm_cnt_src0_adj;	//ary add
u32 g_dpss_dae_frm_cnt_src1_adj;	//ary add
u32 g_dpss_dae_frm_cnt_src2_adj;	//ary add
u32 g_dpss_dpe_nr_frm_cnt_adj;	//ary add
u32 g_dpss_dpe_di_frm_cnt_adj;	//ary add
u32 dpss_fnr_vpp_link;

u32 inp_ibuf_level;
u32 inp_obuf_level    = 16;//ary tmp reg_inp_ibuf_num    ;
u32 frc_dae_ibuf_level = 0, frc_dae_obuf_level = 4;//ary tmp reg_frc_dae_obuf_num;
//ary no use  u32 frc_dpe_ibuf_level=0, frc_dpe_obuf_level=6; // ary tmp reg_frc_dpe_obuf_num;
u32 fnr_dae_ibuf_level = 0, fnr_dae_obuf_level = 6; //ary tmp reg_fnr_dae_obuf_num;
u32 fnr_dpe_ibuf_level = 0, fnr_dpe_obuf_level = 6; //ary tmp reg_fnr_dpe_obuf_num;

u32 viu_frm_idx = 0, src_frm_idx = 0, inp_frm_idx = 0, frc_dae_frm_idx = 0
/*ary no use,frc_dpe_frm_idx=0*/;
u32 fnr_dae_frm_idx = 0, fnr_dpe_frm_idx = 0;
u32 dae0_frm_rst_cnt = 0, dae1_frm_rst_cnt = 0, dae2_frm_rst_cnt = 0;

u32 dae_cur_phase, dae_pre_phase;
u32 dpe_cur_phase, dpe_pre_phase;
u32 dpe_pre_idx, dpe_cur_idx;
u32 mc_proc_phs;
enum DPSS_FILM_MODE pre_film_mode;

u32 inp_din_idx = 0, inp_proc_idx = 0;
u64 inp_fifo[16];

u32 frc_dae_din_idx = 0, frc_dae_proc_idx = 0;
u64 frc_dae_fifo[8];

u32 fnr_dae_din_idx = 0/*ary no use, fnr_dae_proc_idx=0*/;
u64 fnr_dae_fifo[8];

u32 frc_dpe_din_idx = 0, frc_dpe_proc_idx = 0;
u64 frc_dpe_fifo[8];

u32 fnr_dpe_din_idx = 0,  fnr_dpe_proc_idx = 0;
u64 fnr_dpe_fifo[8];
u64 fnr_dpe_idx_fifo[8];
u32 fnr_dpe_idx_rdpt = 0, fnr_dpe_idx_wrpt = 0;
u32 cfg_dae0_level = 0, cfg_dae1_level = 0; /* ary no use,cfg_dae2_level=0*/

bool dpss_dae_done[10];
unsigned int dpss_wait_cnt1;

//=========================================//
// free func
//=========================================//
void cfg_dpss_inp_free(u32 src_idx)
{
#ifdef NO_USED	//ary note: this is que option only
	dbg_h2("[inp free func] free idx: %d\n", src_idx);
	vpu_enqueue(&inp_bufQ, src_idx); //ibuf_rd_idx
#endif
}

void cfg_dpss_dpe_free(u32 idx)  //free
{
	w_reg_bit(DPSS_FNR_SW_DRV_CTRL1, idx & 0xf, 4, 4);
	//w_reg_bit(DPSS_FNR_SW_DRV_CTRL1,fnr_dpe_idx_wrpt,4,4);//dpe_obuf_free_idx
	cfg_dpss_trigger(FNR_DPE_OBUF_RDY);

	if (fnr_dpe_idx_wrpt == fnr_dpe_idx_rdpt)
		dbg_h2("error! fnr dpe output fifo error! wrpt overwrite rdpt!\n");

	dbg_h2("fnr obuf free idx: %x\n", fnr_dpe_idx_wrpt);
	//fnr_dpe_idx_fifo[fnr_dpe_idx_wrpt%reg_fnr_dpe_obuf_num]=fnr_dpe_idx_wrpt&0xf;//idx&0xf;
	fnr_dpe_idx_fifo[fnr_dpe_idx_wrpt % reg_fnr_dpe_obuf_num] = idx & 0xf;
	fnr_dpe_idx_wrpt = (fnr_dpe_idx_wrpt + 1) % reg_fnr_dpe_obuf_num;
	fnr_dpe_obuf_level++;
}

void cfg_dpss_dpe_src1_free(void)
{
	//reg_vdi_dpe_obuf_wrpt - reg_dpe_obuf_wrpt - dxe_obuf_free_idx
	w_reg_bit(DPSS_VDI_SW_DRV_CTRL1, src1_dpe_obuf_wrpt, 4, 4);
	w_reg_bit(DPSS_VDI_SW_DRV_CTRL0, 1, 1, 1);//pls_vdi_dpe_obuf_rdy - pls_obuf_rdy_en

	dbg_h2("src1 - src1_dpe_obuf_wrpt %02d\n", src1_dpe_obuf_wrpt);
	u32 vdi_obuf_num = (rd(DPSS_REGOFST_VDICTRL + DPSS_FBUF_BUF_CTRL) & 0x1f);

	src1_dpe_obuf_wrpt = ACC(src1_dpe_obuf_wrpt, (vdi_obuf_num - 1));
}

void cfg_dpss_trigger(enum DPSS_TRIGGER_MODE mode)
{
	switch (mode) {
	case FRC_INP_IBUF_RDY:
		{w_reg_bit(FRC_INP_SW_CTRL0, 1, 27, 1); break; }

	case FRC_INP_OBUF_RDY:
		{w_reg_bit(FRC_INP_SW_CTRL0, 1, 26, 1); break; }

	case FRC_DAE_IBUF_RDY:
		{w_reg_bit(FRC_DAE_SW_CTRL0, 1, 27, 1); break; }

	case FRC_DAE_OBUF_RDY:
		{w_reg_bit(FRC_DAE_SW_CTRL0, 1, 25, 1); break; }

//	case FRC_DAE_IBUF_FREE:
//		{w_reg_bit(FRC_DAE_SW_CTRL0, 1, 26, 1);break; }

	case FNR_DAE_IBUF_RDY:
		{w_reg_bit(DPSS_FNR_SW_DRV_CTRL0, 1, 0, 1); break; }

	case FNR_DPE_OBUF_RDY:
		{w_reg_bit(DPSS_FNR_SW_DRV_CTRL0, 1, 1, 1); break; }

	default:
		{break; }
	}
}

//=========================================//
// film mode calc
//=========================================//
u32 dpss_calc_film_step(u32 film_mode, u32 film_phs)
{
	//ary no use    u32  inp_ofrm_vld;//32b
	u32  inp_ofrm_step, inp_ofrm_loop_tab;

	switch (film_mode) {
	case 0:
		inp_ofrm_loop_tab = 0b1; inp_ofrm_step = 128; break; //case DPSS_VIDEO
	case 1:
		inp_ofrm_loop_tab = 0b01; inp_ofrm_step = 64; break;  //case DPSS_FILM22
	case 2:
		inp_ofrm_loop_tab = 0b00101; inp_ofrm_step = 51; break;  //case DPSS_FILM32
	case 3:
		inp_ofrm_loop_tab = 0b0010010101; inp_ofrm_step = 51; break;  //case DPSS_FILM3223
	case 4:
		inp_ofrm_loop_tab = 0b0001010101; inp_ofrm_step = 51; break;  //case DPSS_FILM2224
	case 5:  //case DPSS_FILM32322
		inp_ofrm_loop_tab = 0b001010010101; inp_ofrm_step = 53; break;
	case 6:
		inp_ofrm_loop_tab = 0b0001; inp_ofrm_step = 32; break;  //case DPSS_FILM44
	case 7:
		inp_ofrm_loop_tab = 0b011111; inp_ofrm_step = 106; break; //case DPSS_FILM21111
	case 8:  //case DPSS_FILM23322
		inp_ofrm_loop_tab = 0b001001010101; inp_ofrm_step = 53; break;
	case 9:
		inp_ofrm_loop_tab = 0b01111; inp_ofrm_step = 102; break; //case DPSS_FILM2111
	case 10:  //case DPSS_FILM22224
		inp_ofrm_loop_tab = 0b000101010101; inp_ofrm_step = 53; break;
	case 11:
		inp_ofrm_loop_tab = 0b001; inp_ofrm_step = 42; break;  //case DPSS_FILM33
	case 12:
		inp_ofrm_loop_tab = 0b0001001001; inp_ofrm_step = 38; break;  //case DPSS_FILM334
	case 13:
		inp_ofrm_loop_tab = 0b00001; inp_ofrm_step = 25; break;  //case DPSS_FILM55
	case 14:
		inp_ofrm_loop_tab = 0b0000010001; inp_ofrm_step = 25; break;  //case DPSS_FILM64
	case 15:
		inp_ofrm_loop_tab = 0b000001; inp_ofrm_step = 21; break;  //case DPSS_FILM66
	case 16:  //case DPSS_FILM87
		inp_ofrm_loop_tab = 0b000000010000001; inp_ofrm_step = 17; break;
	case 17:
		inp_ofrm_loop_tab = 0b01011; inp_ofrm_step = 76; break;  //case DPSS_FILM212
	default:
	inp_ofrm_loop_tab = 0b1; inp_ofrm_step = 128; break;
	}

	u32 frm_vld = (inp_ofrm_loop_tab >> film_phs) & 0x1;

	dbg_h2("inp_ofrm_loop_tab: %x\n", inp_ofrm_loop_tab);
	return (frm_vld << 8) | (inp_ofrm_step & 0xff);
}

//=========================================//
// cfg dae info
//=========================================//
void cfg_frc_dae_ibuf_rdy(u64 info)
{
	wr(FRC_DAE_SW_CTRL1, info & 0xffffffff);
	w_reg_bit(FRC_DAE_SW_CTRL2, (info >> 32) & 0xffff, 0, 16);
	cfg_dpss_trigger(FRC_DAE_IBUF_RDY);
}

void cfg_fnr_dae_ibuf_rdy(u64 info)
{
	w_reg_bit(DPSS_FNR_SW_DRV_CTRL1,
		(info >> 32) & 0xffff, 16, 16); //fnr_dae_ibuf_din[47:32]
	wr(DPSS_FNR_SW_DRV_CTRL2, info & 0xffffffff); //fnr_dae_ibuf_din[31:0]
	w_reg_bit(DPSS_FNR_SW_DRV_CTRL0, 1, 0, 1); //fnr_dae_ibuf_rdy,pls
}

void cfg_dpss_dae0_fifo(u32 wren, u32 rden)
{
	cfg_dae0_level = cfg_dae0_level + wren - rden;
}

void cfg_dpss_dae1_fifo(u32 wren, u32 rden)
{
	cfg_dae1_level = cfg_dae1_level + wren - rden;
}

//=========================================//
// cfg dpe info
//=========================================//
void cfg_fnr_dpe_ibuf_rdy(u64 info0, u64 info1)
{
	wr(DPSS_DPE_IBUF_DIN0, info0 & 0xffffffff);
	wr(DPSS_DPE_IBUF_DIN1, info1 & 0xffff);
	w_reg_bit(DPSS_NR_SW_DRV_CTRL1, 1, 0, 1);
}

void cfg_frc_dpe_ibuf_rdy(u64 info0, u64 info1, u32 dae_ibuf_rdy_mode,
	u32 dae_frm_idx)
{
	if (frc_dae_obuf_level > 0) {
		wr(FRC_OBUF_SW_CTRL1, info0 & 0xffffffff);
		w_reg_bit(FRC_OBUF_SW_CTRL2, info1 & 0xffff, 0, 16);

		if (dae_ibuf_rdy_mode == 1) {
			if (dae_frm_idx % 2 == 1) {
				w_reg_bit(FRC_OBUF_SW_CTRL0, 1, 0, 1);
				dbg_h2("frc dae obuf write to buffer every 2 frames\n");
			}

		} else {
			w_reg_bit(FRC_OBUF_SW_CTRL0, 1, 0, 1);
			dbg_h2("frc dae obuf write to buffer every frame\n");
		}

		cfg_dpss_trigger(FRC_DAE_OBUF_RDY); //to release DAE drop frame
	}
}

#ifdef NO_USED
void cfg_dst_ibuf_rdy(u64 info0, u64 info1) //version:0312 add
{
	dbg_h2("%s: dst_info0: %x dst_info1: %x ==\n", __func__, info0, info1);
	wr(FRC_OBUF_SW_CTRL1, info0 & 0xffffffff);
	w_reg_bit(FRC_OBUF_SW_CTRL2, info1 & 0xffff, 0, 16);
	w_reg_bit(FRC_OBUF_SW_CTRL0, 1, 0, 1);
}
#endif

//==============================================================//
// Trans addr to ENV for bitmatch
//==============================================================//
//dpe_out_addr_rd_tbc
void hw_dpe_out_addr_rd_tbc(enum PRM_SRC_IDX src_idx)
{
	u32 dpss_dpe_out_idx;

	if (src_idx == SRC_IDX_NR) {
		dpss_dpe_out_idx = rd(DPSS_FBUF_DPE_LOOP_IDX) & 0xff;
		dbg_h2("%s:nro %d, dio %d\n", __func__,
				(dpss_dpe_out_idx >> 4) & 0xf,
				dpss_dpe_out_idx & 0xf);
	} else if (src_idx == SRC_IDX_FRC) {
		dpss_dpe_out_idx = rd(FRC_DPSS_DPE_OUT_IDX) & 0xff;
	} else {
		dpss_dpe_out_idx = rd(DPSS_REGOFST_VDICTRL + DPSS_FBUF_DPE_LOOP_IDX) & 0xff;
	}
	dbg_h2("[Process_Irq]:dpss_dpe_out_idx %02d %0x\n", src_idx, dpss_dpe_out_idx);
}

void dae_out_addr_rd_tbc(enum PRM_SRC_IDX src_idx)
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

