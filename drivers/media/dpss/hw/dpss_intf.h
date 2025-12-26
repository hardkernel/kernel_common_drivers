/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _DPSS_INTF_H_
#define _DPSS_INTF_H_

//#include "dpss.h"

struct DPSS_MIF_TYPE {
	u32 buf_hsize;
	u32 buf_vsize;
	u64 buf_addr[3];
	u64 buf_bits_mode:2;	// 0 : 8 bits     1: 10 bits
	u64 buf_fmt_mode:2;	// 0 : 4:2:0;     1: 4:2:2;   2: 4:4:4
	u64 buf_separate:2;
	// 0 : one plane 1: 2 plane. (NV21). 2: 3 plane(old 4:2:0)
	u64 buf_full_pack:2;	// 0 : old pack (12bits for 10bits 422)1: full pack
	u32 luma_x_start;
	u32 luma_x_end;
	u32 luma_y_start;
	u32 luma_y_end;
	u32 chrm_x_start;
	u32 chrm_x_end;
	u32 chrm_y_start;
	u32 chrm_y_end;
	u32 rev_x;
	u32 rev_y;
};

enum vid_out_fmt {
	OUT_YUV444    = 0,
	OUT_YUV422_1_PLANE    = 1,//10BIT/1p
	OUT_YUV420_NV21    = 2, //8BIT
	OUT_YUV422_2_PLANE    = 3,//10BIT/2p
	OUT_YUV420_NV12    = 4, //8BIT
	OUT_YUV_NULL	= 0XFF
};

enum vid_src_fmt {
	YUV444    = 0,
	YUV422    = 1,
	YUV420    = 2,
	RGB       = 3,
	PLANAR_X1 = 5,
	PLANAR_X2 = 6,
	PLANAR_X3 = 7,
	BIT_008 = 10,
	BIT_010 = 11,
	BIT_012 = 12,
	BIT_014 = 13,
	BIT_016 = 14,
	BIT_P010 = 15,		//10IN16
	BIT_10IN12 = 16,	//10IN12

	CMPR_UN = 20,
	CMPR_AFBC = 21,
	CMPR_AFRC = 22,

	IS_PSRC = 30,		//progressive src
	IS_ISRC = 31		//interlace   src
};

struct PRM_INTF_TYPE {
	enum vid_src_fmt src_fmt;	//YUV444/YUV422/YUV420/RGB
	enum vid_src_fmt src_plan;	//PLANAR_X1/X2/X3
	enum vid_src_fmt src_bit;	//BIT_8/10/12
	enum vid_src_fmt src_cmpr;	//un/afbc/afrc
	enum vid_src_fmt interlace;	//IS_PSRC/IS_ISRC

	u32 src_hsize;
	u32 src_vsize;
	u32 src_baddr[3];	//[0]:y/head [1]:u/c/body [2]:v
	u32 y_crop[4];		//[0]:x_bg   [1]:x_ed     [2]:y_bg  [3]:y_ed
	u32 u_crop[4];		//[0]:x_bg   [1]:x_ed     [2]:y_bg  [3]:y_ed
	u32 v_crop[4];		//[0]:x_bg   [1]:x_ed     [2]:y_bg  [3]:y_ed
	u32 reverse[2];		//[0]:x      [1]:y
	u32 skip[2];		//[0]:x      [1]:y
	u32 bits_mode;
	//0:4b 1:8b 2:16b 3:32b 4:64b 5:128b 6:12b 7:10b 8:14b 9:24b 10:20b
	u32 pack_mode;		//0:1x 1:2x 2:4x 3:8x 4:16x 5:32x 6:64x
	u32 slc_x_st[4];
	u32 slc_x_ed[4];
	u32 slc_y_st[4];
	u32 slc_y_ed[4];
	u32 stride;

	u32 luma_wr_x_st[4];
	u32 luma_wr_x_ed[4];
	u32 luma_wr_y_st[4];
	u32 luma_wr_y_ed[4];
	u32 chrm_wr_x_st[4];
	u32 chrm_wr_x_ed[4];
	u32 chrm_wr_y_st[4];
	u32 chrm_wr_y_ed[4];

	u32 mmu_mode_en;
	u32 skip_line;	//0:no skip 1: skip 1 line 2: skip 2line .. 7:skip 7 line
	u32 pad_en;
	u32 pad_hmode;		//0:8 1:16
	u32 pad_vmode;		//0:8 1:16
	u32 cut_win_en;
	u32       mc_skip_mode;
	u32 burst_len;		//default set 3
	u32 field_mode;	//0:close 1:odd->even 2:even->odd //yu.zong 2024-12-06
	u32 reg_mode;		//default set 0      //yu.zong 2024-12-06

	bool uv_swap;		//ary add
	bool swap_64bit;
	bool little_endian;
	unsigned int h_buf_size;	//use this for stride;
	unsigned int back_1_y;	//0501
	unsigned int back_1_u;	//0501
	unsigned int back_2_y;	//0501
	unsigned int back_2_u;	//0501
	u32 slc_num;
	u32 canvas_hsize;
	u32 block_mode;
	u32 lst_overlap;
};

enum dpss_mif_e {
	DPSS_RMIF_INP = 0,
	DPSS_RMIF_DAE0 = 10,	//pre
	DPSS_RMIF_DAE1 = 11,	//cur
	DPSS_RMIF_DAE2 = 12,	//nxt
	DPSS_RMIF_DPE0 = 30,	//pix_cur
	DPSS_RMIF_DPE1 = 31,	//pix_pre1
	DPSS_RMIF_DPE2 = 32,	//pix_pre2
	DPSS_RMIF_DPE3 = 33,	//

	DPSS_WMIF_DPE0 = 43,	//nr_wrmif
	DPSS_WMIF_DPE1 = 44,	//di_wrmif
	DPSS_WMIF_DPE2 = 45,	//
	DPSS_WMIF_DPE3 = 46,	//nr_ds_wrmif
	DPSS_WMIF_DPE4 = 47,	//di_ds_wrmif
	DPSS_WMIF_DPE5 = 48,

	DPSS_RMIF_MC0 = 60,
	DPSS_RMIF_MC1 = 61,
	DPSS_RMIF_VD2 = 62,
	DPSS_RMIF_NULL = 80
};

void cfg_lcevc_top(u32 lcevc_en,
			//input size
			u32 src1_frm_hsize,
			u32 src1_frm_vsize,
			u32 src1_head_ybaddr,
			u32 src1_body_ybadd,
			u32 src1_head_cbadd,
			u32 src1_body_cbadd,
			u32 src1_is_cmp,
			u32 src1_bit,
			u32 src2_frm_hsize,
			u32 src2_frm_vsiz, u32 src2_ybaddr, u32 src2_cbadd,
			//output size
			u32 frm_hsize_out, u32 frm_vsize_out,
			u32 bld_din0_premult_en,
			u32 bld_din1_premult_en,
			u32 bld_din0_alpha,
			u32 bld_din1_alpha,
			u32 inst_sel,
			u32 apply_mode,
			u32 ds_mode,	//0:aa mode 1:pps mode
			u32 slc_mode,	//0:frame 1:slice
			u32 slc_num,	//2/4 slice
			u32 bld_src0_sel,
			//1:lay1 chose src0  2:lay1 chose src1  else :close blend lay1
			u32 bld_src1_sel,
			u32 dbg_cfg
			);
			//1:lay2 chose src0  2:lay2 chose src1  else :close blend lay2

void ena_vfcd_rdmif(enum dpss_mif_e mif_index, u32 on_off);
void dpss_mif_initial(struct DPSS_MIF_TYPE *dpss_mif, u32 frm_hsize, u32 frm_vsize);
void dpss_mif_stride(struct DPSS_MIF_TYPE *dpss_mif, u32 *stride);
void cfg_dpss_rdmif_3ch(enum dpss_mif_e mif_index, struct DPSS_MIF_TYPE *dpss_mif);
void cfg_vfcd_rdmif_2ch(enum dpss_mif_e mif_index, struct PRM_INTF_TYPE *prm_mif,
			u32 fmt444_out);
void frc_cfg_vfcd_rdmif_2ch(enum dpss_mif_e mif_index, struct PRM_INTF_TYPE *prm_mif,
			u32 fmt444_out);
void cfg_vfcd_rdmif_2ch_reg(enum dpss_mif_e mif_index, struct PRM_INTF_TYPE *prm_mif,
			    u32 fmt444_out);
void dpss_dpe_rdmif_cfg(u32 hsize, u32 baddr, u32 mode, u32 idx, u32 reg_grd,
			u32 xbgn_slc0, u32 xend_slc0, u32 ybgn_slc0,
			u32 yend_slc0, u32 xbgn_slc1, u32 xend_slc1,
			u32 ybgn_slc1, u32 yend_slc1, u32 xbgn_slc2,
			u32 xend_slc2, u32 ybgn_slc2, u32 yend_slc2,
			u32 xbgn_slc3, u32 xend_slc3, u32 ybgn_slc3,
			u32 yend_slc3);
void cfg_dpe_pix_wrmif(enum dpss_mif_e mif_index, struct PRM_INTF_TYPE *prm_mif);
void cfg_dpe_pix_wrmif_reg(enum dpss_mif_e mif_index, struct PRM_INTF_TYPE *prm_mif);
void dpss_dpe_wrmif_cfg(u32 hsize, u32 baddr, u32 idx,
			u32 xbgn_slc0, u32 xend_slc0, u32 ybgn_slc0,
			u32 yend_slc0, u32 xbgn_slc1, u32 xend_slc1,
			u32 ybgn_slc1, u32 yend_slc1, u32 xbgn_slc2,
			u32 xend_slc2, u32 ybgn_slc2, u32 yend_slc2,
			u32 xbgn_slc3, u32 xend_slc3, u32 ybgn_slc3,
			u32 yend_slc3);
void dpss_cfg_viu_pix_rdmif(struct PRM_INTF_TYPE *prm_mif, s32 mif_index);	//
void dpss_cfg_viu_pix_rdmif_di_update(unsigned int y, unsigned int u);
void dpss_dae_rdmif_cfg(u32 hsize, u32 vsize, u32 *baddr, u32 mode);
void dpss_dae_wrmif_cfg(u32 hsize, u32 vsize, u32 tfbf_hsize, u32 tfbf_vsize,
			u32 *baddr, u32 mode);
void cfg_mc_sub_rdmif(struct PRM_INTF_TYPE *prm_mif, s32 mif_index,
		      s32 only_change_addr);
s32 get_pix_rdmif_regs_ofst(s32 mif_index);
s32 get_vfcd_regs_ofst(enum dpss_mif_e mif_index);
void cfg_viu_pix_rdmif_baddr(s32 mif_index, u32 ybaddr, u32 cbbaddr,
			     u32 crbaddr);
void cfg_vfcd_rdmif_ext(enum dpss_mif_e mif_index, struct PRM_INTF_TYPE *prm_mif);
void dpss_dae_wrmif_cfg_brst_len(u32 burst_len);
void dpss_dae_rdmif_cfg_brst_len(u32 burst_len);
void hw_cfg_mc_sub_rdmif(struct PRM_INTF_TYPE *prm_mif, s32 mif_index,
		      s32 only_change_addr);
#endif
