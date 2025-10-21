/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _VFCE__H_
#define _VFCE__H_

struct VFCE_AFBCE_TYPE {
	u32 loosy_mode;
	//0:close 1:luma loosy 2:chrma loosy 3: luma & chrma loosy
	u32 def_color_0;
	//def_color
	u32 def_color_1;
	//def_color
	u32 def_color_2;
	//def_color
	u32 def_color_3;
	//def_color
	u32 brst_len_add_en;
	u32 brst_len_add_value;
	u32 ofset_brst4_en;
};

struct VFCE_AFRCE_TYPE {
	u32 reg_input_format_mode;
	//0:rgb  1:yuv   2:bayer
	u32 reg_input_bayer_mode;
	//0:mono, 1:bayer2x2  2:bayer4x4
	u32 reg_bayer_input_plane;
	//0:raw input form y, 1:raw input from c
	u32 reg_bayer_h2w8_format;
	//0:bayer2x2 rg 4x32, 1:bayer2x2 rg 8x16
	u32 reg_afrc_head_mode;
	//0:no header 1:with header
	u32 reg_comp_target_byte_0;
	u32 reg_comp_target_byte_1;
	u32 reg_bitstream_pack_mode_0;
	u32 reg_bitstream_pack_mode_1;
	u32 reg_bitstream_pack_min_byte_0;
	u32 reg_bitstream_pack_min_byte_1;
	u32 reg_dict_en_0;
	u32 reg_dict_en_1;
	u32 reg_dict_error_th_0;
	u32 reg_dict_error_th_1;
	u32 reg_dict_diff_enter_th_0;
	u32 reg_dict_diff_enter_th_1;
	u32 reg_dict_blk_diff_cont_th_0;
	u32 reg_dict_blk_diff_cont_th_1;
};

struct VFCE_TYPE {
	u32 channel_enable;
	u32 reg_core_sel;
	//0 afbc, 1 afrc
	u32 reg_inp_format;
	//0:444 1:422 2:420
	u32 reg_enc_format;
	//0:444 1:422 2:420
	u32 reg_compbits_y;	//bits num after compression
	u32 reg_compbits_c;
	//bits num after compression
	u32 reg_pip_mode;
	//pip open bit
	u32 reg_init_ctrl;	//pip init frame flag
	u32 reg_fmt444_comb;	//comb 8bit 444
	u32 reg_444to422_mode;
	//444to422 mode
	u32 reg_422to420_mode;	//422to420 mode
	u32 mmu_mode_en;	//mmu mode en
	u32 mmu_page_mode;
	//0: 4k page 1: 8k page
	u32 loop_mode_en;
	//loop mode en
	u32 loop_num_0;
	//plane 0 loop num
	u32 loop_num_1;		//plane 1 loop num
	u32 hsize_bgnd;		//hsize of background
	u32 vsize_bgnd;		//hsize of background
	u32 enc_win_bgn_h;	//scope in background buffer
	u32 enc_win_end_h;	//scope in background buffer
	u32 enc_win_bgn_v;	//scope in background buffer
	u32 enc_win_end_v;	//scope in background buffer
	u32 soft_slc_mode_en;	//software slice mode enable
	u32 soft_slc_num_h;	//software h slice num
	u32 soft_slc_num_v;
	//software v slice num
	u32 enc_slc_bgn_h[4];
	//scope in background buffer
	u32 enc_slc_end_h[4];
	//scope in background buffer
	u32 enc_slc_bgn_v[4];
	//scope in background buffer
	u32 enc_slc_end_v[4];
	//scope in background buffer
	u64 head_baddr_0;
	//plane 0 head_addr
	u64 head_baddr_1;
	//plane 1 head_addr
	u64 body_baddr_0;
	//plane 0 body_addr or mmu baddr
	u64 body_baddr_1;
	//plane 1 body_addr or mmu baddr
	u32 reg_ram_offset_mode;
	//ram offset mode
	u32 reg_cbuf_1_offset;
	//ram offset
	u32 reg_cbuf_0_offset;
	//ram offset
	u32 reg_ds_buf_offset;
	//ram offset
	u32 reg_head_1_offset;
	//ram offset
	u32 reg_head_0_offset;
	//ram offset
	u32 reg_body_1_offset;
	//ram offset
	u32 reg_body_0_offset;
	//ram offset
	u32 reg_mif_burst_mode;
	//mif_burst_mode
	struct VFCE_AFBCE_TYPE afbce;
	struct VFCE_AFRCE_TYPE afrce;
};

struct VFCE_CALC_TYPE {
	int lossy_luma_en;
	int lossy_chrm_en;
	int reg_fmt444_comb;	//calculate
	int sblk_num;		//calculate
	int uncmp_size;		//calculate
	int uncmp_bits;		//calculate
	int comp_y_map;
	int comp_c_map;
	int align_en;
	int h_slc_mode_en;
	int v_slc_mode_en;
	int h_slc_num;
	int v_slc_num;

	int reg_pixel_format;
	//0:R 1:RG 2:RGB 3:RGBA 4:Y/U/V 5: Y_UV
	//6:bayer 4 plane  7:bayer rg 4x16  8:bayer2x2 rg 8x16
	int reg_rgb2yuv_en;	//calculate
	int reg_pixel_is_diff_chn;	//calculate
	int reg_input_is_rgba;	//calculate
	int reg_pixel_type_0;	//calculate
	int reg_pixel_type_1;	//calculate
	int reg_raw_last_th_0;	//calculate
	int reg_raw_last_th_1;	//calculate
	int reg_max_ac_depth_0;	//calculate
	int reg_max_ac_depth_1;	//calculate
	int reg_header_last_extend_en_0;	//calculate
	int reg_header_last_extend_en_1;	//calculate
	int int_bitstream_pack_mode_0;
	int int_bitstream_pack_mode_1;
};

void init_vfce(int mode, struct VFCE_TYPE *prm);
void init_afbce(int mode, struct VFCE_AFBCE_TYPE *prm);
void init_afrce(int mode, struct VFCE_AFRCE_TYPE *prm);

void cfg_vfce(int index, int channel, struct VFCE_TYPE *prm);
void hw_cfg_vfce_chnl_addr_only(int reg_offset_in, struct VFCE_TYPE *prm);	//ary add

#endif
