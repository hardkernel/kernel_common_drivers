// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "define.h"
#include "vfce.h"

int get_vfce_offset(int index)
{
	int reg_offset;

	switch (index) {
	case 0:
		reg_offset = (0x42 - 0x42);
		break;		//vdin   //is 0
#ifndef DPSS_FPGA_DRV

	case 1:
		reg_offset = (0x140 + 0xc8 - 0x42);
		break;		//dpss    // 0x1c6
#else

	case 1:
		reg_offset = (0xc8 - 0x42);
		break;
// is 0x86 dpss,fpga don't nedd base_addr_offset,only page_offset
#endif
	default:
		reg_offset = (0x42 - 0x42);
		break;
	}

	return reg_offset;
}

void init_vfce(int mode, struct VFCE_TYPE *prm)
{
	prm->channel_enable = 1;
	prm->reg_core_sel = 0;	//afbc
	prm->reg_inp_format = 0;	//yuv444
	prm->reg_enc_format = 2;	//yuv420
	prm->reg_compbits_y = 0;	//bits num after compression 8bit
	prm->reg_compbits_c = 0;	//bits num after compression 8bit
	prm->reg_pip_mode = 0;	//pip open bit
	prm->reg_init_ctrl = 0;	//pip init frame flag
	prm->reg_fmt444_comb = 0;	//comb 8bit 444
	prm->reg_444to422_mode = 0;	//444to422 mode
	prm->reg_422to420_mode = 0;	//422to420 mode
	prm->mmu_mode_en = 0;	//mmu mode en
	prm->mmu_page_mode = 0;	//0: 4k page 1: 8k page
	prm->loop_mode_en = 0;	//loop mode en
	prm->loop_num_0 = 0;	//plane 0 loop num
	prm->loop_num_1 = 0;	//plane 1 loop num
	prm->hsize_bgnd = 0;	//hsize of background
	prm->vsize_bgnd = 0;	//hsize of background
	prm->enc_win_bgn_h = 0;	//scope in background buffer
	prm->enc_win_end_h = 0;	//scope in background buffer
	prm->enc_win_bgn_v = 0;	//scope in background buffer
	prm->enc_win_end_v = 0;	//scope in background buffer
	prm->soft_slc_mode_en = 0;	//software slice mode enable
	prm->soft_slc_num_h = 4;	//software h slice num
	prm->soft_slc_num_v = 1;	//software v slice num
	prm->enc_slc_bgn_h[0] = 0;	//scope in background buffer
	prm->enc_slc_end_h[0] = 0;	//scope in background buffer
	prm->enc_slc_bgn_v[0] = 0;	//scope in background buffer
	prm->enc_slc_end_v[0] = 0;	//scope in background buffer
	prm->enc_slc_bgn_h[1] = 0;	//scope in background buffer
	prm->enc_slc_end_h[1] = 0;	//scope in background buffer
	prm->enc_slc_bgn_v[1] = 0;	//scope in background buffer
	prm->enc_slc_end_v[1] = 0;	//scope in background buffer
	prm->enc_slc_bgn_h[2] = 0;	//scope in background buffer
	prm->enc_slc_end_h[2] = 0;	//scope in background buffer
	prm->enc_slc_bgn_v[2] = 0;	//scope in background buffer
	prm->enc_slc_end_v[2] = 0;	//scope in background buffer
	prm->enc_slc_bgn_h[3] = 0;	//scope in background buffer
	prm->enc_slc_end_h[3] = 0;	//scope in background buffer
	prm->enc_slc_bgn_v[3] = 0;	//scope in background buffer
	prm->enc_slc_end_v[3] = 0;	//scope in background buffer
	prm->head_baddr_0 = 0;	//head_addr
	prm->head_baddr_1 = 0;	//head_addr
	prm->body_baddr_0 = 0;	//body_addr or mmu baddr
	prm->body_baddr_1 = 0;	//body_addr or mmu baddr
	prm->reg_ram_offset_mode = 0;	//ram offset mode off
	prm->reg_cbuf_1_offset = 1024;	//ram offset
	prm->reg_cbuf_0_offset = 0;	//ram offset
	prm->reg_ds_buf_offset = 0;	//ram offset
	prm->reg_head_1_offset = 128;	//ram offset
	prm->reg_head_0_offset = 0;	//ram offset
	prm->reg_body_1_offset = 64;	//ram offset
	prm->reg_body_0_offset = 0;	//ram offset
	prm->reg_mif_burst_mode = 3;	//mif_burst_mode
	init_afbce(mode, &prm->afbce);
	init_afrce(mode, &prm->afrce);
}

void init_afbce(int mode, struct VFCE_AFBCE_TYPE *prm)
{
	prm->loosy_mode = 0;
	//0:close 1:luma loosy 2:chrma loosy 3: luma & chrma loosy
	prm->def_color_0 = 4095;	//def_color
	prm->def_color_1 = 4095;
	prm->def_color_2 = 4095;
	prm->def_color_3 = 4095;
	prm->brst_len_add_en = 0;
	prm->brst_len_add_value = 2;
	prm->ofset_brst4_en = 0;
}

void init_afrce(int mode, struct VFCE_AFRCE_TYPE *prm)
{
	prm->reg_input_format_mode = 1;	//0:rgb 1:yuv 2:bayer
	prm->reg_input_bayer_mode = 1;	//0:mono, 1:bayer2x2
	prm->reg_bayer_input_plane = 0;	//0:raw input form y, 1:raw input from c
	prm->reg_bayer_h2w8_format = 0;	//0:bayer2x2 rg 4x32, 1:bayer2x2 rg 8x16
	prm->reg_afrc_head_mode = dpss_afrc_head;	//0:no header 1:with header
	prm->reg_comp_target_byte_0 = dpss_afrc_y;
	prm->reg_comp_target_byte_1 = dpss_afrc_c;
	prm->reg_bitstream_pack_mode_0 = 1;
	prm->reg_bitstream_pack_mode_1 = 1;
	prm->reg_bitstream_pack_min_byte_0 = 4;
	prm->reg_bitstream_pack_min_byte_1 = 4;
	prm->reg_dict_en_0 = afrc_dict_mode_y;
	prm->reg_dict_en_1 = afrc_dict_mode_c;
	prm->reg_dict_error_th_0 = 25;
	prm->reg_dict_error_th_1 = 25;
	prm->reg_dict_diff_enter_th_0 = 0;
	prm->reg_dict_diff_enter_th_1 = 0;
	prm->reg_dict_blk_diff_cont_th_0 = 1;
	prm->reg_dict_blk_diff_cont_th_1 = 1;
	dbg_h2("%s:target:%d,%d\n", __func__, prm->reg_comp_target_byte_0,
			prm->reg_comp_target_byte_1);
}

typedef void (*WR_R)(unsigned int addr, unsigned int val);

static void cfg_afbce(int reg_offset_in, struct VFCE_TYPE *prm, struct VFCE_CALC_TYPE *cal)
{
	WR_R op_wr = wr_vc;
	unsigned int reg_offset = reg_offset_in;

	if (reg_offset_in > 0xffff) {
		reg_offset = reg_offset_in - 0xfe00 - 0x4200;	//ary tmp
		op_wr = wr;
	}

	dbg_h2("%s:offset in= 0x%x, 0x%x\n", __func__,
		reg_offset_in, reg_offset);
	op_wr(reg_offset + VFCE_AFBCE_FORMAT,
		((prm->afbce.brst_len_add_value & 0x7) << 12) |
		// head adj value
		((prm->afbce.ofset_brst4_en & 0x1) << 11) |
		// burst 4
		((prm->afbce.brst_len_add_en & 0x1) << 10) |
		// head adj
		((prm->reg_enc_format & 0x3) << 8) |
		// enc format
		((prm->reg_compbits_c & 0xf) << 4) |
		// chrm bits
		((prm->reg_compbits_y & 0xf) << 0)	// luma bits
	    );
	op_wr(reg_offset + VFCE_AFBCE_QUANT_ENABLE, ((0x0 & 0x1) << 11) |
		// chrm quant expand en
		((0x0 & 0x1) << 10) |
		// luma quant expand en
		((0x0 & 0x3) << 8) |	// bc leave
		((cal->lossy_chrm_en & 0x1) << 4) |
		// chrm lossy
		((cal->lossy_luma_en & 0x1) << 0)	// luma lossy
		);
	op_wr(reg_offset + VFCE_AFBCE_DEFCOLOR_1,
		((prm->afbce.def_color_3 & 0xfff) << 12) |
		// def_color_a
		((prm->afbce.def_color_0 & 0xfff) << 0));
	op_wr(reg_offset + VFCE_AFBCE_DEFCOLOR_2,
		((prm->afbce.def_color_2 & 0xfff) << 12) |
		// def_color_v
		((prm->afbce.def_color_1 & 0xfff) << 0));
}

static void cfg_afrce(int reg_offset_in, struct VFCE_TYPE *prm, struct VFCE_CALC_TYPE *cal)
{
	WR_R op_wr = wr_vc;
	unsigned int reg_offset = reg_offset_in;

	if (reg_offset_in > 0xffff) {
		reg_offset = reg_offset_in - 0xfe00 - 0x4200;	//ary tmp
		op_wr = wr;
	}

	dbg_h2("%s:offset in= 0x%x, 0x%x,0x%x\n", __func__,
		reg_offset_in, reg_offset, prm->afrce.reg_comp_target_byte_0);
	op_wr(reg_offset + VFCE_AFRCE_CORE_PIXEL_TYPE,
		((cal->reg_pixel_type_0 & 0xf) << 12) |
		//plane 0 pixel type
		((cal->reg_pixel_type_1 & 0xf) << 8)	//plane 1 pixel type
		);
	op_wr(reg_offset + VFCE_AFRCE_CORE_PIXEL_DEPTH,
		((prm->reg_compbits_y & 0xf) << 12) |
		//plane 0 bits
		((prm->reg_compbits_c & 0xf) << 8)	//plane 1 bits
		);
	op_wr(reg_offset + VFCE_AFRCE_CORE_TARGET_SIZE,
		((prm->afrce.reg_comp_target_byte_0 & 0xff) << 24) |
		//plane 0 target byte
		((prm->afrce.reg_comp_target_byte_1 & 0xff) << 16)	//plane 1 target byte
		);
	op_wr(reg_offset + VFCE_AFRCE_CORE_LAST_MAX_AC,
		((cal->reg_raw_last_th_0 & 0xf) << 28) |
		//plane 0 raw last th
		((cal->reg_raw_last_th_1 & 0xf) << 24) |
		//plane 1 raw last th
		((cal->reg_max_ac_depth_0 & 0xf) << 12) |
		//plane 0 max ac depth
		((cal->reg_max_ac_depth_1 & 0xf) << 8)	//plane 1 max ac depth
		);
	op_wr(reg_offset + VFCE_AFRCE_CORE_HEADER_CTRL, ((0xf & 0xf) << 16) |
		//plane x raw mode en
		((0x0 & 0xf) << 12) |
		//plane x force raw mode
		((cal->int_bitstream_pack_mode_0 & 0x1) << 11) |
		//plane 0 pack mode
		((cal->int_bitstream_pack_mode_1 & 0x1) << 10) |
		//plane 1 pack mode
		((0x3 & 0x3) << 8) |
		//plane x pack mode
		((cal->reg_header_last_extend_en_0 & 0x1) << 7) |
		//plane 0 header last extend
		((cal->reg_header_last_extend_en_1 & 0x1) << 6) |
		//plane 1 header last extend
		((0x0 & 0x3) << 4) |
		//plane x header last extend
		((0x0 & 0xf) << 0)	//plane x header mode
		);
	op_wr(reg_offset +
		VFCE_AFRCE_CORE_PACK_MODE_MIN_BYTE,
		((prm->afrce.reg_bitstream_pack_min_byte_0 & 0x3f)
		<< 24) |		//plane 0 min byte
		((prm->afrce.reg_bitstream_pack_min_byte_1 & 0x3f)
		<< 16)		//plane 1 min byte
		);
	op_wr(reg_offset + VFCE_AFRCE_CORE_DICT_ERROR_TH,
		((prm->afrce.reg_dict_error_th_0 & 0xff) << 24) |
		//plane 0 dict error
		((prm->afrce.reg_dict_error_th_1 & 0xff) << 16)	//plane 1 dict error
		);
	op_wr(reg_offset + VFCE_AFRCE_CORE_DICT_ENTER_TH,
		((prm->afrce.reg_dict_diff_enter_th_0 & 0xff) << 24) |
		((prm->afrce.reg_dict_diff_enter_th_1 & 0xff) << 16)	//plane 1 enter th
		);
	op_wr(reg_offset + VFCE_AFRCE_CORE_DICT_CTRL,
		((prm->afrce.reg_dict_blk_diff_cont_th_0 & 0x7) << 13) |	//plane 0 diff th
		((prm->afrce.reg_dict_blk_diff_cont_th_1 & 0x7) << 10) |	//plane 1 diff th
		((prm->afrce.reg_dict_en_0 & 0x1) << 3) |	//plane 0 dict en
		((prm->afrce.reg_dict_en_1 & 0x1) << 2)	//plane 1 dict en
		);
	op_wr(reg_offset + VFCE_AFRCE_INPUT_CTRL, ((cal->reg_rgb2yuv_en & 0x1) << 26) |
		((cal->reg_pixel_is_diff_chn & 0x1) << 25) |	// rgb2yuv dct en
		((cal->reg_input_is_rgba & 0x1) << 24) |	// rgb or yuv header
		((prm->afrce.reg_input_format_mode & 0x7) << 20) |	//input format
		((cal->reg_pixel_format & 0xf) << 16) |	// pixel format
		((prm->afrce.reg_input_bayer_mode & 0x3) << 8)	// bayer mode
		);
	dbg_h2("%s:dict_en=%d %d\n", __func__, prm->afrce.reg_dict_en_0, prm->afrce.reg_dict_en_1);
}

static void cfg_vfce_top(int reg_offset_in, struct VFCE_TYPE *prm, struct VFCE_CALC_TYPE *cal)
{
	WR_R op_wr = wr_vc;
	unsigned int reg_offset = reg_offset_in;

	if (reg_offset_in > 0xffff) {
		reg_offset = reg_offset_in - 0xfe00 - 0x4200;	//ary tmp
		op_wr = wr;
	}

	dbg_h2("%s:offset in= 0x%x, 0x%x\n", __func__,
		reg_offset_in, reg_offset);
	op_wr(reg_offset + VFCE_TOP0_CTRL_2, ((0x0 & 0x7) << 1) |	// ram arb rst
		((prm->reg_ram_offset_mode & 0x1) << 0)	// ram offset mode
		);
}

static void cfg_vfce_chnl(int reg_offset_in, struct VFCE_TYPE *prm, struct VFCE_CALC_TYPE *cal)
{
	int i;
	WR_R op_wr = wr_vc;
	unsigned int reg_offset = reg_offset_in;

	if (reg_offset_in > 0xffff) {
		reg_offset = reg_offset_in - 0xfe00 - 0x4200;	//ary tmp
		op_wr = wr;
	}

	dbg_h2("%s:offset in= 0x%x, 0x%x,0x%x, 0x%x\n", __func__,
		reg_offset_in, reg_offset,
		prm->afrce.reg_comp_target_byte_0,
		prm->afrce.reg_afrc_head_mode);
	op_wr(reg_offset + VFCE_CHNL0_MODE,
		((prm->reg_444to422_mode & 0x7) << 20) |
		((prm->reg_422to420_mode & 0x7) << 17) |
		((prm->reg_core_sel & 0x1) << 16) |
		((prm->afrce.reg_bayer_input_plane & 0x3) << 14) |
		((cal->align_en & 0x1) << 13) |
		((cal->comp_y_map & 0x7) << 10) |
		((cal->comp_c_map & 0x7) << 7) |
		((prm->reg_enc_format & 0x7) << 4) |
		((prm->reg_inp_format & 0x7) << 1) |
		((cal->reg_fmt444_comb & 0x1) << 0));
	op_wr(reg_offset + VFCE_CHNL0_CTRL, ((0xf & 0xf) << 24) |
		((cal->v_slc_mode_en & 0x1) << 23) |	//v slice mode en
		((cal->h_slc_mode_en & 0x1) << 22) |	//h slice mode en
		((cal->v_slc_num & 0xf) << 18) |	//v slice mode num
		((cal->h_slc_num & 0xf) << 14) |	//h slice mode num
		((0x0 & 0x1) << 13) |	//v slice cfg cnt clear
		((0x0 & 0x1) << 12) |	//h slice cfg cnt clear
		((prm->reg_init_ctrl & 0x1) << 10) |	//pip init en
		((prm->reg_pip_mode & 0x1) << 9) |	//pip mode en
		((0x6f & 0xff) << 1) |	//trans ctrl thr
		((0x1 & 0x1) << 0)	//trans ctrl en
		);
	op_wr(reg_offset + VFCE_CHNL0_AFBC_MODE_0,
		((cal->uncmp_size & 0xff) << 5)
		|			// uncmp size
		((prm->afbce.brst_len_add_value & 0x7) << 2)
		|			// head adj value
		((prm->afbce.ofset_brst4_en & 0x1) << 1)
		|			// burst 4
		((prm->afbce.brst_len_add_en & 0x1) << 0)	// head adj
		);
	op_wr(reg_offset + VFCE_CHNL0_AFRC_MODE_0,
		((prm->afrce.reg_afrc_head_mode & 0x1) << 31)
		|			// afrc head mode
		((prm->afrce.reg_input_format_mode & 0x7) << 28)
		|			// input format
		((cal->reg_pixel_format & 0xf) << 24)
		|			// pixel format
		((prm->afrce.reg_input_bayer_mode & 0x3) << 22)
		|			// bayer mode
		((prm->afrce.reg_comp_target_byte_0 & 0x7f) << 15)
		|			// plane 0 target byte
		((prm->afrce.reg_comp_target_byte_1 & 0x7f) << 8)
		|			// plane 1 target byte
		((cal->reg_pixel_type_0 & 0xf) << 4)
		|			// plane 0 pixel type
		((cal->reg_pixel_type_1 & 0xf) << 0)	// plane 1 pixel type
		);
	op_wr(reg_offset + VFCE_CHNL0_CONV_BUF_OFST, ((prm->reg_cbuf_1_offset & 0x3fff) << 16) |
		((prm->reg_cbuf_0_offset & 0x3fff) << 0)	// ram offset
		);
	op_wr(reg_offset + VFCE_CHNL0_DS_BUF_OFST, ((prm->reg_ds_buf_offset & 0x3fff) << 0)
		);
	op_wr(reg_offset + VFCE_CHNL0_HEAD_BUF_OFST, ((prm->reg_head_1_offset & 0x3fff) << 16) |
		((prm->reg_head_0_offset & 0x3fff) << 0)	// ram offset
		);
	op_wr(reg_offset + VFCE_CHNL0_BODY_BUF_OFST, ((prm->reg_body_1_offset & 0x7ff) << 16) |
		((prm->reg_body_0_offset & 0x7ff) << 0)	// ram offset
		);
	op_wr(reg_offset + VFCE_CHNL0_SIZE_IN,
		((prm->hsize_bgnd & 0xffff) << 16) |
		((prm->vsize_bgnd & 0xffff) << 0)	// vsize_in of afbc input
		);

	if (cal->h_slc_mode_en) {
		for (i = 0; i <= cal->h_slc_num; i++) {
			op_wr(reg_offset + VFCE_CHNL0_PIXEL_IN_HOR_SCOPE,
				((prm->enc_slc_end_h[i] & 0xffff) << 16) |
				((prm->enc_slc_bgn_h[i] & 0xffff) << 0)	// scope of vsize_in
				);
		}
	} else {
		op_wr(reg_offset + VFCE_CHNL0_PIXEL_IN_HOR_SCOPE,
			((prm->enc_win_end_h & 0xffff) << 16) |
			((prm->enc_win_bgn_h & 0xffff) << 0)	// scope of vsize_in
			);
	}

	if (cal->v_slc_mode_en) {
		for (i = 0; i <= cal->v_slc_num; i++) {
			op_wr(reg_offset + VFCE_CHNL0_PIXEL_IN_VER_SCOPE,
				((prm->enc_slc_end_v[i] & 0xffff) << 16) |
				((prm->enc_slc_bgn_v[i] & 0xffff) << 0)	// scope of vsize_in
				);
		}

	} else {
		op_wr(reg_offset + VFCE_CHNL0_PIXEL_IN_VER_SCOPE,
			((prm->enc_win_end_v & 0xffff) << 16) |
			((prm->enc_win_bgn_v & 0xffff) << 0)	// scope of vsize_in
			);
	}

	op_wr(reg_offset + VFCE_CHNL0_HEAD0_BADDR, prm->head_baddr_0 >> 4);
	op_wr(reg_offset + VFCE_CHNL0_HEAD1_BADDR, prm->head_baddr_1 >> 4);
	op_wr(reg_offset + VFCE_CHNL0_BODY0_BADDR, prm->body_baddr_0 >> 4);
	op_wr(reg_offset + VFCE_CHNL0_BODY1_BADDR, prm->body_baddr_1 >> 4);
	op_wr(reg_offset + VFCE_CHNL0_ENABLE, prm->channel_enable & 0x1);
}

static void cfg_vfce_mif(int reg_offset_in, struct VFCE_TYPE *prm, struct VFCE_CALC_TYPE *cal)
{
	WR_R op_wr = wr_vc;
	unsigned int reg_offset = reg_offset_in;

	if (reg_offset_in > 0xffff) {
		reg_offset = reg_offset_in - 0xfe00 - 0x4200;	//ary tmp
		op_wr = wr;
	}

	dbg_h2("%s:offset in= 0x%x, 0x%x\n", __func__,
		reg_offset_in, reg_offset);
	op_wr(reg_offset + VFCE_MIF_PLANE0_MODE, ((0x0 & 0x3) << 26) |
		((0x3 & 0x3) << 24) |	//mif urgent
		((0x0 & 0x1) << 23)
		|			//awblk_last1
		((0x0 & 0x1) << 22)
		|			//awblk_last0
		((0x4 & 0xf) << 18)
		|			//axi arb weight ch1
		((0x8 & 0xf) << 14)
		|			//axi arb weight ch0
		((0x0 & 0x1) << 13)
		|			//axi arb rst
		((prm->mmu_page_mode & 0x1) << 5)
		|			//mmu page mode
		((prm->mmu_mode_en & 0x1) << 4)
		|			//mmu mode en
		((0x3 & 0x7) << 0)	//burst mode
		);
	op_wr(reg_offset + VFCE_MIF_PLANE0_LOOP, ((prm->loop_num_0 & 0xfff) << 2) |
		((0x0 & 0x1) << 1) |
		((prm->loop_mode_en & 0x1) << 0)	//loop mode en
		);
	op_wr(reg_offset + VFCE_MIF_PLANE1_MODE, ((0x0 & 0x3) << 26) |
		((0x3 & 0x3) << 24) |	//mif urgent
		((0x0 & 0x1) << 23)
		|			//awblk_last1
		((0x0 & 0x1) << 22)
		|			//awblk_last0
		((0x4 & 0xf) << 18)
		|			//axi arb weight ch1
		((0x8 & 0xf) << 14)
		|			//axi arb weight ch0
		((0x0 & 0x1) << 13)
		|			//axi arb rst
		((prm->mmu_page_mode & 0x1) << 5)
		|			//mmu page mode
		((prm->mmu_mode_en & 0x1) << 4)
		|			//mmu mode en
		((0x3 & 0x7) << 0)	//burst mode
		);
	op_wr(reg_offset + VFCE_MIF_PLANE1_LOOP, ((prm->loop_num_1 & 0xfff) << 2) |
		((0x0 & 0x1) << 1) |
		((prm->loop_mode_en & 0x1) << 0)	//loop mode en
		);
}

//ary add:
void hw_cfg_vfce_chnl_addr_only(int reg_offset_in, struct VFCE_TYPE *prm)
{
	WR_R op_wr = wr_vc;
	unsigned int reg_offset = reg_offset_in;

	if (reg_offset_in > 0xffff) {
		reg_offset = reg_offset_in - 0xfe00 - 0x4200;	//ary tmp
		op_wr = wr;
	}

	dbg_h2("%s:offset in= 0x%x, 0x%x\n", __func__,
		reg_offset_in, reg_offset);
	op_wr(reg_offset + VFCE_CHNL0_HEAD0_BADDR, prm->head_baddr_0 >> 4);
	op_wr(reg_offset + VFCE_CHNL0_HEAD1_BADDR, prm->head_baddr_1 >> 4);
	op_wr(reg_offset + VFCE_CHNL0_BODY0_BADDR, prm->body_baddr_0 >> 4);
	op_wr(reg_offset + VFCE_CHNL0_BODY1_BADDR, prm->body_baddr_1 >> 4);
}

void cfg_vfce(int index, int channel, struct VFCE_TYPE *prm)
{
	int reg_offset;
	struct VFCE_CALC_TYPE cal;

	memset(&cal, 0, sizeof(struct VFCE_CALC_TYPE));

	reg_offset = get_vfce_offset(index) * 0x100 + channel * 0x80;
	//dbg_h2("[vfce debug]: %d\n",reg_offset);
	///////////////////////////////////////////////////////////////////////
	//////VFCE configure logic
	///////////////////////////////////////////////////////////////////////
	cal.align_en = 1;	//input align to block
	cal.reg_fmt444_comb = (prm->reg_enc_format == 0) &&
		(prm->reg_fmt444_comb);
	//yuv444 can only support 8bit,and must use comb_mode
	cal.sblk_num = (prm->reg_enc_format == 2) ? 12 :
		(prm->reg_enc_format == 1) ? 16 : 24;
	cal.uncmp_bits = prm->reg_compbits_y >
		prm->reg_compbits_c ? prm->reg_compbits_y : prm->reg_compbits_c;
	cal.uncmp_size = (((((16 * cal.uncmp_bits * cal.sblk_num) + 7) >> 3) + 31) / 32) << 1;
	cal.comp_y_map = prm->reg_compbits_y == 8 ? 0 :
		prm->reg_compbits_y == 10 ? 1 :
		prm->reg_compbits_y == 12 ? 2 : prm->reg_compbits_y == 14 ? 3 : 4;
	cal.comp_c_map = prm->reg_compbits_c == 8 ? 0 :
		prm->reg_compbits_c == 10 ? 1 :
		prm->reg_compbits_c == 12 ? 2 : prm->reg_compbits_c == 14 ? 3 : 4;

	//afbc core quan
	if (prm->afbce.loosy_mode == 0) {	//luma quan off, chrm quan off
		cal.lossy_luma_en = 0;
		cal.lossy_chrm_en = 0;
	} else if (prm->afbce.loosy_mode == 1) {	//luma quan on, chrm quan off
		cal.lossy_luma_en = 1;
		cal.lossy_chrm_en = 0;
	} else if (prm->afbce.loosy_mode == 2) {	//luma quan off, chrm quan on
		cal.lossy_luma_en = 0;
		cal.lossy_chrm_en = 1;
	} else {		//luma quan on, chrm quan on
		cal.lossy_luma_en = 1;
		cal.lossy_chrm_en = 1;
	}

	//afrc core format
	if (prm->afrce.reg_input_format_mode == 0) {	//rgb
		cal.reg_pixel_format = 2;
		cal.reg_pixel_type_0 = 0;
		cal.reg_pixel_type_1 = 0;
		cal.reg_rgb2yuv_en = 1;
		cal.reg_pixel_is_diff_chn = 1;
		cal.reg_input_is_rgba = 1;
	} else if (prm->afrce.reg_input_format_mode == 1) {	//yuv
		cal.reg_pixel_format = 5;
		cal.reg_pixel_type_0 = 3;
		cal.reg_pixel_type_1 = 4;
		cal.reg_rgb2yuv_en = 0;
		cal.reg_pixel_is_diff_chn = 0;
		cal.reg_input_is_rgba = 0;
	} else if (prm->afrce.reg_input_format_mode == 2 &&
		prm->afrce.reg_input_bayer_mode == 0) {
		cal.reg_pixel_format = 0;
		cal.reg_pixel_type_0 = 3;
		cal.reg_pixel_type_1 = 3;
		cal.reg_rgb2yuv_en = 0;
		cal.reg_pixel_is_diff_chn = 0;
		cal.reg_input_is_rgba = 0;
	} else if (prm->afrce.reg_input_format_mode == 2 &&
		prm->afrce.reg_input_bayer_mode == 1) {	//bayer2x2 2plane
		cal.reg_pixel_format = prm->afrce.reg_bayer_h2w8_format ? 8 : 7;
		cal.reg_pixel_type_0 = 4;
		cal.reg_pixel_type_1 = 4;
		cal.reg_rgb2yuv_en = 0;
		cal.reg_pixel_is_diff_chn = 1;
		cal.reg_input_is_rgba = 0;
	} else {
		dbg_h2("rtl not support reg_input_format_mode\n");
	}

	//afrc pack mode
	if ((prm->reg_pip_mode == 1 ||
		prm->loop_mode_en == 1 ||
		prm->afrce.reg_afrc_head_mode == 0) &&
		(prm->afrce.reg_bitstream_pack_mode_0 == 1 ||
		prm->afrce.reg_bitstream_pack_mode_1 == 1)) {
		cal.int_bitstream_pack_mode_0 = 0;
		cal.int_bitstream_pack_mode_1 = 0;
		dbg_h2("pip, loop, no header, only support fix compress\n");
	} else {
		cal.int_bitstream_pack_mode_0 =
			prm->afrce.reg_bitstream_pack_mode_0;
		cal.int_bitstream_pack_mode_1 =
			prm->afrce.reg_bitstream_pack_mode_1;
	}

	//afrc bits config
	if (prm->reg_compbits_y <= 10) {
		cal.reg_raw_last_th_0 = 9;
		cal.reg_max_ac_depth_0 = 9;
		cal.reg_header_last_extend_en_0 = 0;

	} else if (prm->reg_compbits_y <= 12) {
		cal.reg_raw_last_th_0 = 12;
		cal.reg_max_ac_depth_0 = 12;
		cal.reg_header_last_extend_en_0 = 1;

	} else {
		dbg_h2("rtl not support reg_compbits_y\n");
	}

	//afrc bits config
	if (prm->reg_compbits_c <= 10) {
		cal.reg_raw_last_th_1 = 9;
		cal.reg_max_ac_depth_1 = 9;
		cal.reg_header_last_extend_en_1 = 0;
	} else if (prm->reg_compbits_c <= 12) {
		cal.reg_raw_last_th_1 = 12;
		cal.reg_max_ac_depth_1 = 12;
		cal.reg_header_last_extend_en_1 = 1;
	} else {
		dbg_h2("rtl not support reg_compbits_c\n");
	}

	cal.h_slc_mode_en = prm->soft_slc_mode_en;
	cal.v_slc_mode_en = prm->soft_slc_mode_en;
	cal.h_slc_num = prm->soft_slc_num_h - 1;
	cal.v_slc_num = prm->soft_slc_num_v - 1;

	///////////////////////////////////////////////////////////////////////
	//////VFCE configure registers
	///////////////////////////////////////////////////////////////////////

	//config core
	if (prm->reg_core_sel == 0) {
		//afbc core
		cfg_afbce(reg_offset, prm, &cal);

	} else {
		//afrc core
		cfg_afrce(reg_offset, prm, &cal);
	}

	//config wrmif
	cfg_vfce_mif(reg_offset, prm, &cal);
	//config top FE
	cfg_vfce_top(reg_offset, prm, &cal);
	//config top BE
	cfg_vfce_top(reg_offset + 8, prm, &cal);	//ary addr /4
	//config channel FE
	cfg_vfce_chnl(reg_offset, prm, &cal);
	//config channel BE
	cfg_vfce_chnl(reg_offset + 32, prm, &cal);	//ary addr /4
}				/* cfg_vfce */
