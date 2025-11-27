// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "define.h"
#include "fgrain.h"

#ifdef FUNC_EN_FGRAIN
void film_grain_init(struct FGRAIN_t *fgrain)
{
	fgrain->id  = 0;
	fgrain->path_sel = FG_NR0;
	fgrain->fmt = FG_YUV420;
	//fgrain->hsize = ;
	//fgrain->vsize = ;
	fgrain->dma_table_bigend = 0;
	fgrain->src_8bit_mst  = 0;
	fgrain->src_8bit      = 0;
	fgrain->dma_mem_base0 = 0x01000000;
	fgrain->dma_mem_base1 = 0x01010000;
	fgrain->dma_mem_base2 = 0x01020000;
	fgrain->dma_mem_base3 = 0x01030000;
	fgrain->dma_ini[0] = 1;
	fgrain->dma_ini[1] = 1;
	fgrain->reg_mode   = 0;
	fgrain->slc_num    = 1;
	fgrain->slc0_win_hbgn = 0;
	fgrain->slc0_win_hend = 3812;
	fgrain->slc1_win_hbgn = 0;
	fgrain->slc1_win_hend = 3812;
	fgrain->slc2_win_hbgn = 0;
	fgrain->slc2_win_hend = 3812;
	fgrain->slc3_win_hbgn = 0;
	fgrain->slc3_win_hend = 3812;
	fgrain->slc_win_vbgn = 0;
	fgrain->slc_win_vend = 2156;
	fgrain->rev_mode = 0;
	w_reg_bit_vc(VPU_DMA_RDMIF_SEL, 0, 0, 1);
}

void film_grain_uint(void)
{
	wr_vc(VPU_DMA_RDMIF1_CTRL, 0x00000200);
	w_reg_bit_vc(VPU_AXIRD_FG_CTRL, 0xf, 0, 4);
	w_reg_bit_vc(VPU_DMA_RDMIF_SEL, 1, 0, 1);
}

void film_grain_cfg(struct FGRAIN_t *fgrain, struct vframe_s *vf, uint cnt)
{
	u32 FGRAIN_TBL_SIZE = 498;
	u32 reg_VPU_DMA_RDMIF_BADR0;
	u32 reg_VPU_DMA_RDMIF_BADR1;
	u32 reg_VPU_DMA_RDMIF_BADR2;
	u32 reg_VPU_DMA_RDMIF_BADR3;
	u32 reg_VPU_DMA_RDMIF_CTRL;
	u32 fmt = 0;
	u32 id = 0;
	u32 vfcd_src_sel = 0;
	u32 reg_ext_imode = 0;//1 src 8bit in lsb
	u32 reg_comp_bits = 0;//
	u32 reg_use_input_size;
	u32 slice_num;
	u32 slc0_hsize_info;
	u32 slc1_hsize_info;
	u32 slc2_hsize_info;
	u32 slc3_hsize_info;
	u32 slc_vsize_info;
	unsigned int i;

	w_reg_bit_vc(VPU_DMA_RDMIF_SEL, 0, 0, 1);

	if (vf->bitdepth & BITDEPTH_Y10)
		fgrain->src_8bit = false;
	else
		fgrain->src_8bit = true;

	if (vf->flag & VFRAME_FLAG_VIDEO_SECURE) {
		w_reg_bit_vc(VPU_LUT_DMA_SEC_IN, 1, 0, 1);
		w_reg_bit_vc(VPU_LUT_DMA_SEC_IN, 1, 1, 1);
		w_reg_bit_vc(VPU_LUT_DMA_SEC_IN, 1, 2, 1);
	} else {
		w_reg_bit_vc(VPU_LUT_DMA_SEC_IN, 0, 0, 1);
		w_reg_bit_vc(VPU_LUT_DMA_SEC_IN, 0, 1, 1);
		w_reg_bit_vc(VPU_LUT_DMA_SEC_IN, 0, 2, 1);
	}

	id  = fgrain->id;
	reg_ext_imode = fgrain->src_8bit_mst ? 0 : 1;
	reg_comp_bits = fgrain->src_8bit == 1 ? 0 : 1;//8 : 10
	reg_use_input_size = fgrain->reg_mode == 1 ? 0 : 1;
	slice_num     = fgrain->slc_num;
	slc0_hsize_info = (fgrain->slc0_win_hend) << 16 | (fgrain->slc0_win_hbgn);
	slc1_hsize_info = (fgrain->slc1_win_hend) << 16 | (fgrain->slc1_win_hbgn);
	slc2_hsize_info = (fgrain->slc2_win_hend) << 16 | (fgrain->slc2_win_hbgn);
	slc3_hsize_info = (fgrain->slc3_win_hend) << 16 | (fgrain->slc3_win_hbgn);
	slc_vsize_info = (fgrain->slc_win_vend) << 16 | (fgrain->slc_win_vbgn);

	switch (fgrain->path_sel) {
		case(FG_NR0):
			vfcd_src_sel = 0; break;
		case(FG_NR1):
			vfcd_src_sel = 1; break;
		case(FG_NR2):
			vfcd_src_sel = 2; break;
		case(FG_NR3):
			vfcd_src_sel = 3; break;
		case(FG_VD1):
			vfcd_src_sel = 4; break;
		case(FG_VD2):
			vfcd_src_sel = 6; break;
		case(FG_MC0):
			vfcd_src_sel = 4; break;
		case(FG_MC1):
			vfcd_src_sel = 5; break;
	}
	switch (fgrain->fmt) {
		case(FG_YUV444):
			dbg_h2("error fgrain fmt444 not support");
			stimulus_finish_fail(0);
			break;
		case(FG_YUV422):
			fmt = 1; break;
		case(FG_YUV420):
			fmt = 2; break;
	}

	fgrain->dma_mem_base0 = vf->fgs_table_adr;
	fgrain->dma_mem_base1 = vf->fgs_table_adr;
	fgrain->dma_mem_base2 = vf->fgs_table_adr;
	fgrain->dma_mem_base3 = vf->fgs_table_adr;
	//configure Fgrain DMA read
	if (fgrain->dma_ini[id]) {
		if (id == 0) {
			reg_VPU_DMA_RDMIF_BADR0 = VPU_DMA_RDMIF1_BADR0;
			reg_VPU_DMA_RDMIF_BADR1 = VPU_DMA_RDMIF1_BADR1;
			reg_VPU_DMA_RDMIF_BADR2 = VPU_DMA_RDMIF1_BADR2;
			reg_VPU_DMA_RDMIF_BADR3 = VPU_DMA_RDMIF1_BADR3;
			reg_VPU_DMA_RDMIF_CTRL  = VPU_DMA_RDMIF1_CTRL;
		} else if (id == 1) {
			reg_VPU_DMA_RDMIF_BADR0 = VPU_DMA_RDMIF2_BADR0;
			reg_VPU_DMA_RDMIF_BADR1 = VPU_DMA_RDMIF2_BADR1;
			reg_VPU_DMA_RDMIF_BADR2 = VPU_DMA_RDMIF2_BADR2;
			reg_VPU_DMA_RDMIF_BADR3 = VPU_DMA_RDMIF2_BADR3;
			reg_VPU_DMA_RDMIF_CTRL  = VPU_DMA_RDMIF2_CTRL;
		} else {
			DBG_WARN("%s:id =%d\n", __func__, id); //ary add for debug
			reg_VPU_DMA_RDMIF_BADR0 = VPU_DMA_RDMIF1_BADR0;
			reg_VPU_DMA_RDMIF_BADR1 = VPU_DMA_RDMIF1_BADR1;
			reg_VPU_DMA_RDMIF_BADR2 = VPU_DMA_RDMIF1_BADR2;
			reg_VPU_DMA_RDMIF_BADR3 = VPU_DMA_RDMIF1_BADR3;
			reg_VPU_DMA_RDMIF_CTRL  = VPU_DMA_RDMIF1_CTRL;
		}
		i = cnt % 4;
		wr_vc(reg_VPU_DMA_RDMIF_BADR0 + i, fgrain->dma_mem_base0 >> 4);
		dbg_h0("%s 0x%x\n", __func__, reg_VPU_DMA_RDMIF_BADR0 + i);
		//wr_vc(reg_VPU_DMA_RDMIF_BADR1, fgrain->dma_mem_base1 >> 4);
		//wr_vc(reg_VPU_DMA_RDMIF_BADR2, fgrain->dma_mem_base2 >> 4);
		//wr_vc(reg_VPU_DMA_RDMIF_BADR3, fgrain->dma_mem_base3 >> 4);

		w_reg_bit_vc(reg_VPU_DMA_RDMIF_CTRL, FGRAIN_TBL_SIZE, 0, 13);
		w_reg_bit_vc(reg_VPU_DMA_RDMIF_CTRL, 0, 30, 1);//reset index to 0
		w_reg_bit_vc(reg_VPU_DMA_RDMIF_CTRL, 1, 29, 1);
		w_reg_bit_vc(reg_VPU_DMA_RDMIF_CTRL, reg_comp_bits, 24, 1);
		w_reg_bit_vc(reg_VPU_DMA_RDMIF_CTRL, fgrain->dma_table_bigend, 13, 1);
		//little_endian
		if (id == 0)
			w_reg_bit_vc(reg_VPU_DMA_RDMIF_CTRL, 1, 20, 1);
		//enable fgrain0 channel
		else if (id == 1)
			w_reg_bit_vc(reg_VPU_DMA_RDMIF_CTRL, 1, 17, 1);
		//enable fgrain1 channel
		fgrain->dma_ini[id] = 0;
	}
	dbg_h0("film_grain\n");
	if (id == 0)
		w_reg_bit_vc(VPU_AXIRD_FG_CTRL, vfcd_src_sel, 0, 4);
	else if (id == 1)
		w_reg_bit_vc(VPU_AXIRD_FG_CTRL, vfcd_src_sel, 8, 4);
	wr_vc((id * FG_REG_OFFSET + FGRAIN_CTRL), (reg_use_input_size << 29) |
		//[29]reg_use_input_size
		(slice_num << 26) | //[28:26]reg_slice_size_num
		(0 << 24) | //[25:24]reg_sync_ctrl
		(0 << 22) | //[22]reg_dma_st_clr
		(0 << 21) | //[21]reg_hold4dma_scale
		(0 << 20) | //[20]reg_hold4dma_tbl
		(0 << 19) | //[19]reg_cin_uv_swap
		(0 << 18) | //[18]reg_cin_rev
		(0 << 17) | //[17]reg_yin_rev
		(0 << 16) | //[16]reg_fgrain_ext_imode
		(0 << 15) | //[15]reg_use_par_apply_fgrain
		(0 << 14) | //[14]reg_fgrain_last_ln_mode
		(0 << 13) | //[13]reg_fgrain_use_sat4bp
		(1 << 12) | //[12]reg_apply_c_mode
		(1 << 11) | //[11]reg_fgrain_tbl_sign_mode
		(1 << 10) | //[10]reg_fgrain_tbl_ext_mode
		((fmt & 0x3) << 8) | //[9:8]  reg_fmt_mode
		((reg_comp_bits & 0x3) << 6) | //[7:6]  reg_comp_bits
		(fgrain->rev_mode << 4)   | //[5:4]  reg_rev_mode
		(0 << 2) | //[2] reg_block_mode
		(1 << 1) | //[1] reg_fgrain_loc_en
		(1 << 0));//[0] reg_fgrain_glb_en
	if (reg_use_input_size == 0) {
		if (slice_num > 0)
			wr_vc((id * FG_REG_OFFSET + FGRAIN_WIN_H0), slc0_hsize_info);
		if (slice_num > 1)
			wr_vc((id * FG_REG_OFFSET + FGRAIN_WIN_H1), slc1_hsize_info);
		if (slice_num > 2)
			wr_vc((id * FG_REG_OFFSET + FGRAIN_WIN_H2), slc2_hsize_info);
		if (slice_num > 3)
			wr_vc((id * FG_REG_OFFSET + FGRAIN_WIN_H3), slc3_hsize_info);
		wr_vc((id * FG_REG_OFFSET + FGRAIN_WIN_V), slc_vsize_info);
	}
	//wr((DPSS_DPE_FGRAIN_WIN_H),(0<<0)|((fgrain->hsize-1)<<16));
	//wr((DPSS_DPE_FGRAIN_WIN_V),(0<<0)|((fgrain->vsize-1)<<16));

	//w_reg_bit(DPSS_DPE_FGRAIN_ALONE_MODE_CTRL,0x10,8,8 );
	//set fg_final gain bit12 0x40
	//w_reg_bit(DPSS_DPE_FGRAIN_ALONE_MODE_CTRL,0x10,16,8);
	//set fg_final gain bit12 0x40
}

#endif /* FUNC_EN_FGRAIN */
