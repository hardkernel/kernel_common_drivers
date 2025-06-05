/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __VDIN_T6W_REGS_H
#define __VDIN_T6W_REGS_H

/* t6w newly added registers start */

#define VFCE_CHNL0_ENABLE                          0x4200
//Bit 31:18       reg_gclk_ctrl                   // unsigned ,   RW,  default = 0
//14'hfff:always open 14'h000:gating clock
//Bit 17:13       reserved
//Bit 12          pls_chnl_prot_sw_rst            // unsigned ,   W1T, default = 0  active high
//Bit 11: 8       reserved
//Bit  7          pls_force_frm_en                // unsigned ,   W1T, default = 0
//reg_pls_enc_frm_start pulse
//Bit  6          pls_force_frm_rst               // unsigned ,   W1T, default = 0
//reg_pls_enc_frm_reset pulse
//Bit  5          reg_frm_en_mode                 // unsigned ,   RW,  default = 0
//1:start by write a pulse into VFCE_FORCE_CTRL[3]  0:auto start several lines after vsync
//Bit  4          reg_frm_rst_mode                // unsigned ,   RW,  default = 0
//1:soft reset by write pulse into registers VFCE_FORCE_CTRL[2]  0:auto reset by vsync
//Bit  3: 1       reserved
//Bit  0          reg_chnl_enable                 // unsigned ,   RW,  default = 0
//1:enable 0:disable
#define VFCE_CHNL0_MODE                            0x4201
//Bit 31:23       reserved
//Bit 22:20       reg_444to422_mode               // unsigned ,   RW,  default = 0
//Bit 19:17       reg_422to420_mode               // unsigned ,   RW,  default = 0
//Bit 16          reg_core_sel                    // unsigned ,   RW,  default = 0
//0: afbc, 1: afrc
//Bit 15:14       reg_bayer_input_plane           // unsigned ,   RW,  default = 0
//0: raw input form y, 1: raw input from c
//Bit 13          reg_enc_align_en                // unsigned ,   RW,  default = 1
//Bit 12:10       reg_enc_bits_luma               // unsigned ,   RW,  default = 0
//0: 8bit, 1:10bit, 2:12bit, 3:14it, 4:16bit
//Bit  9: 7       reg_enc_bits_chrm               // unsigned ,   RW,  default = 0
//0: 8bit, 1:10bit, 2:12bit, 3:14it, 4:16bit
//Bit  6: 4       reg_enc_format                  // unsigned ,   RW,  default = 0
//0: YUV444, 1:YUV422, 2:YUV420
//Bit  3: 1       reg_input_format                // unsigned ,   RW,  default = 0
//0: 444, 1: 422, 2: 420
//Bit  0          reg_fmt444_comb                 // unsigned ,   RW,  default = 0
//0: 444 8bit comb mode
#define VFCE_CHNL0_CTRL                            0x4202
//Bit 31:28       reserved
//Bit 27:24       reg_top_force_en                // unsigned ,   RW,  default = 4'hf top force en
//Bit 23          reg_v_slice_mode                // unsigned ,   RW,  default = 0
//1: enable v slice window store to reg
//Bit 22          reg_h_slice_mode                // unsigned ,   RW,  default = 0
//1: enable h slice window store to reg
//Bit 21:18       reg_v_slice_num                 // unsigned ,   RW,  default = 3  slice num - 1
//Bit 17:14       reg_h_slice_num                 // unsigned ,   RW,  default = 3  slice num - 1
//Bit 13          pls_v_slice_clr                 // unsigned ,   W1T, default = 0
//clear slice config count
//Bit 12          pls_h_slice_clr                 // unsigned ,   W1T, default = 0
//clear slice config count
//Bit 11          reserved
//Bit 10          reg_pip_ini_ctrl                // unsigned ,   RW,  default = 0
//Bit  9          reg_pip_mode                    // unsigned ,   RW,  default = 0
//Bit  8: 1       reg_trans_thr_num               // unsigned ,   RW,
//default = 111 stop trans thr num
//Bit  0          reg_trans_ctrl_en               // unsigned ,   RW,
//default = 1  1:enable 0:disable, ctrl write mif full stop trans to core
#define VFCE_CHNL0_DIMM_CTRL                       0x4203
//Bit 31          reg_dimm_layer_en               // unsigned ,   RW,
//default = 0  dimm_layer enable signal
//Bit 30          reserved
//Bit 29: 0       reg_dimm_data                   // unsigned ,   RW,
//default = 29'h00080200  dimm_layer data
#define VFCE_CHNL0_DUMMY_DATA                      0x4204
//Bit 31:30       reserved
//Bit 29: 0       reg_dummy_data                  // unsigned ,   RW,  default = 0
#define VFCE_CHNL0_SIZE_IN                         0x4205
//Bit 31:16       reg_hsize_bg                    // unsigned ,   RW,  default = 1920
//pic horz background size in  unit pixel
//Bit 15: 0       reg_vsize_bg                    // unsigned ,   RW,  default = 1080
//pic vert background size in  unit pixel
#define VFCE_CHNL0_PIXEL_IN_HOR_SCOPE              0x4206
//Bit 31:16       reg_enc_end_h                   // unsigned ,   RW,  default = 1919
//Bit 15: 0       reg_enc_bgn_h                   // unsigned ,   RW,  default = 0
#define VFCE_CHNL0_PIXEL_IN_VER_SCOPE              0x4207
//Bit 31:16       reg_enc_end_v                   // unsigned ,   RW,  default = 1079
//Bit 15: 0       reg_enc_bgn_v                   // unsigned ,   RW,  default = 0
#define VFCE_CHNL0_HEAD1_BADDR                     0x4208
//Bit 31: 0       reg_head1_baddr                 // unsigned ,   RW,  default = 32'h00
//head address
#define VFCE_CHNL0_HEAD0_BADDR                     0x4209
//Bit 31: 0       reg_head0_baddr                 // unsigned ,   RW,  default = 32'h00
//head address
#define VFCE_CHNL0_BODY1_BADDR                     0x420a
//Bit 31: 0       reg_body1_baddr                 // unsigned ,   RW,  default = 32'h00
//body address
#define VFCE_CHNL0_BODY0_BADDR                     0x420b
//Bit 31: 0       reg_body0_baddr                 // unsigned ,   RW,  default = 32'h00
//body address
#define VFCE_CHNL0_CONV_BUF_OFST                   0x420c
//Bit 31:30       reserved
//Bit 29:16       reg_cbuf_1_offset               // unsigned ,   RW,  default = 0
//Bit 15:14       reserved
//Bit 13: 0       reg_cbuf_0_offset               // unsigned ,   RW,  default = 0
#define VFCE_CHNL0_DS_BUF_OFST                     0x420d
//Bit 31:14       reserved
//Bit 13: 0       reg_ds_buf_offset               // unsigned ,   RW,  default = 0
#define VFCE_CHNL0_HEAD_BUF_OFST                   0x420e
//Bit 31:30       reserved
//Bit 29:16       reg_head_1_offset               // unsigned ,   RW,  default = 0
//Bit 15:14       reserved
//Bit 13: 0       reg_head_0_offset               // unsigned ,   RW,  default = 0
#define VFCE_CHNL0_BODY_BUF_OFST                   0x420f
//Bit 31:27       reserved
//Bit 26:16       reg_body_1_offset               // unsigned ,   RW,  default = 0
//Bit 15:11       reserved
//Bit 10: 0       reg_body_0_offset               // unsigned ,   RW,  default = 0
#define VFCE_CHNL0_STAT1                           0x4210
//Bit 31: 0       ro_dbg_top_info1                // unsigned ,   RO   default = 0
#define VFCE_CHNL0_STAT2                           0x4211
//Bit 31: 0       ro_dbg_top_info2                // unsigned ,   RO   default = 0
#define VFCE_CHNL0_CLR_FLAG                        0x4212
//Bit 31: 0       pls_clr_flag                    // unsigned ,   W1T, default = 0
#define VFCE_CHNL0_STA0_FLAG                       0x4213
//Bit 31: 0       ro_sta0_flag                    // unsigned ,   RO   default = 0
#define VFCE_CHNL0_STA1_FLAG                       0x4214
//Bit 31: 0       ro_sta1_flag                    // unsigned ,   RO   default = 0
#define VFCE_CHNL0_WR_ARB_MISC                     0x4215
//Bit 31          pls_arb_sw_rst                  // unsigned ,   W1T, default = 0
//Bit 30:10       reserved
//Bit  9          reg_arb_awblk_last1             // unsigned ,   RW,  default = 0
//Bit  8          reg_arb_awblk_last0             // unsigned ,   RW,  default = 0
//Bit  7: 4       reg_arb_weight_ch1              // unsigned ,   RW,  default = 15
//Bit  3: 0       reg_arb_weight_ch0              // unsigned ,   RW,  default = 15
#define VFCE_CHNL0_AFBC_MODE_0                     0x4216
//Bit 31:13       reserved
//Bit 12: 5       reg_uncmp_size                  // unsigned ,   RW,  default = 20
//Bit  4: 2       reg_burst_length_add_value      // unsigned ,   RW,  default = 2
//Bit  1          reg_ofset_burst4_en             // unsigned ,   RW,  default = 0
//Bit  0          reg_burst_length_add_en         // unsigned ,   RW,  default = 0
#define VFCE_CHNL0_AFRC_MODE_0                     0x4217
//Bit 31          reg_afrc_head_mode              // unsigned ,   RW,  default = 1
//1 afrc with head, 0 afrc no head
//Bit 30:28       reg_input_format_mode           // unsigned ,   RW,  default = 1
//0:rgb  1:yuv   2:bayer
//Bit 27:24       reg_pixel_format                // unsigned ,   RW,  default = 5
//0:R  1:RG  2:RGB  3:RGBA  4:Y/U/V  5: Y_UV  6:bayer 4plane  7:bayer 2plane  8:bayer 2plane 8x16
//Bit 23:22       reg_input_bayer_mode            // unsigned ,   RW,  default = 1
//0:mono, 1:bayer2x2  2:bayer4x4
//Bit 21:15       reg_comp_target_byte_0          // unsigned ,   RW,  default = 32
//Bit 14: 8       reg_comp_target_byte_1          // unsigned ,   RW,  default = 32
//Bit  7: 4       reg_pixel_type_0                // unsigned ,   RW,  default = 10
//0:RGB, 1:RGBA, 2:R8x8, 3:R16x4, 4:RG
//Bit  3: 0       reg_pixel_type_1                // unsigned ,   RW,  default = 10
//0:RGB, 1:RGBA, 2:R8x8, 3:R16x4, 4:RG
#define VFCE_CHNL1_ENABLE                          0x4220
//Bit 31:18       reg_gclk_ctrl                   // unsigned ,   RW,  default = 0
//14'hfff:always open 14'h000:gating clock
//Bit 17:13       reserved
//Bit 12          pls_chnl_prot_sw_rst            // unsigned ,   W1T, default = 0  active high
//Bit 11: 8       reserved
//Bit  7          pls_force_frm_en                // unsigned ,   W1T, default = 0
//reg_pls_enc_frm_start pulse
//Bit  6          pls_force_frm_rst               // unsigned ,   W1T, default = 0
//reg_pls_enc_frm_reset pulse
//Bit  5          reg_frm_en_mode                 // unsigned ,   RW,  default = 0
//1:start by write a pulse into VFCE_FORCE_CTRL[3]  0:auto start several lines after vsync
//Bit  4          reg_frm_rst_mode                // unsigned ,   RW,  default = 0
//1:soft reset by write pulse into registers VFCE_FORCE_CTRL[2]  0:auto reset by vsync
//Bit  3: 1       reserved
//Bit  0          reg_chnl_enable                 // unsigned ,   RW,  default = 0
//1:enable 0:disable
#define VFCE_CHNL1_MODE                            0x4221
//Bit 31:23       reserved
//Bit 22:20       reg_444to422_mode               // unsigned ,   RW,  default = 0
//Bit 19:17       reg_422to420_mode               // unsigned ,   RW,  default = 0
//Bit 16          reg_core_sel                    // unsigned ,   RW,  default = 0  0: afbc, 1: afrc
//Bit 15:14       reg_bayer_input_plane           // unsigned ,   RW,  default = 0
//0: raw input form y, 1: raw input from c
//Bit 13          reg_enc_align_en                // unsigned ,   RW,  default = 1
//Bit 12:10       reg_enc_bits_luma               // unsigned ,   RW,  default = 0
//0: 8bit, 1:10bit, 2:12bit, 3:14it, 4:16bit
//Bit  9: 7       reg_enc_bits_chrm               // unsigned ,   RW,  default = 0
//0: 8bit, 1:10bit, 2:12bit, 3:14it, 4:16bit
//Bit  6: 4       reg_enc_format                  // unsigned ,   RW,  default = 0
//0: YUV444, 1:YUV422, 2:YUV420
//Bit  3: 1       reg_input_format                // unsigned ,   RW,  default = 0
//0: 444, 1: 422, 2: 420
//Bit  0          reg_fmt444_comb                 // unsigned ,   RW,  default = 0
//0: 444 8bit comb mode
#define VFCE_CHNL1_CTRL                            0x4222
//Bit 31:28       reserved
//Bit 27:24       reg_top_force_en                // unsigned ,   RW,  default = 4'hf top force en
//Bit 23          reg_v_slice_mode                // unsigned ,   RW,  default = 0
//1: enable v slice window store to reg
//Bit 22          reg_h_slice_mode                // unsigned ,   RW,  default = 0
//1: enable h slice window store to reg
//Bit 21:18       reg_v_slice_num                 // unsigned ,   RW,  default = 3  slice num - 1
//Bit 17:14       reg_h_slice_num                 // unsigned ,   RW,  default = 3  slice num - 1
//Bit 13          pls_v_slice_clr                 // unsigned ,   W1T, default = 0
//clear slice config count
//Bit 12          pls_h_slice_clr                 // unsigned ,   W1T, default = 0
//clear slice config count
//Bit 11          reserved
//Bit 10          reg_pip_ini_ctrl                // unsigned ,   RW,  default = 0
//Bit  9          reg_pip_mode                    // unsigned ,   RW,  default = 0
//Bit  8: 1       reg_trans_thr_num               // unsigned ,   RW,  default = 111
//stop trans thr num
//Bit  0          reg_trans_ctrl_en               // unsigned ,   RW,  default = 1
//1:enable 0:disable, ctrl write mif full stop trans to core
#define VFCE_CHNL1_DIMM_CTRL                       0x4223
//Bit 31          reg_dimm_layer_en               // unsigned ,   RW,  default = 0
//dimm_layer enable signal
//Bit 30          reserved
//Bit 29: 0       reg_dimm_data                   // unsigned ,   RW,  default = 29'h00080200
//dimm_layer data
#define VFCE_CHNL1_DUMMY_DATA                      0x4224
//Bit 31:30       reserved
//Bit 29: 0       reg_dummy_data                  // unsigned ,   RW,  default = 0
#define VFCE_CHNL1_SIZE_IN                         0x4225
//Bit 31:16       reg_hsize_bg                    // unsigned ,   RW,  default = 1920
//pic horz background size in  unit pixel
//Bit 15: 0       reg_vsize_bg                    // unsigned ,   RW,  default = 1080
//pic vert background size in  unit pixel
#define VFCE_CHNL1_PIXEL_IN_HOR_SCOPE              0x4226
//Bit 31:16       reg_enc_end_h                   // unsigned ,   RW,  default = 1919
//Bit 15: 0       reg_enc_bgn_h                   // unsigned ,   RW,  default = 0
#define VFCE_CHNL1_PIXEL_IN_VER_SCOPE              0x4227
//Bit 31:16       reg_enc_end_v                   // unsigned ,   RW,  default = 1079
//Bit 15: 0       reg_enc_bgn_v                   // unsigned ,   RW,  default = 0
#define VFCE_CHNL1_HEAD1_BADDR                     0x4228
//Bit 31: 0       reg_head1_baddr                 // unsigned ,   RW,  default = 32'h00
//head address
#define VFCE_CHNL1_HEAD0_BADDR                     0x4229
//Bit 31: 0       reg_head0_baddr                 // unsigned ,   RW,  default = 32'h00
//head address
#define VFCE_CHNL1_BODY1_BADDR                     0x422a
//Bit 31: 0       reg_body1_baddr                 // unsigned ,   RW,  default = 32'h00
//body address
#define VFCE_CHNL1_BODY0_BADDR                     0x422b
//Bit 31: 0       reg_body0_baddr                 // unsigned ,   RW,  default = 32'h00
//body address
#define VFCE_CHNL1_CONV_BUF_OFST                   0x422c
//Bit 31:30       reserved
//Bit 29:16       reg_cbuf_1_offset               // unsigned ,   RW,  default = 0
//Bit 15:14       reserved
//Bit 13: 0       reg_cbuf_0_offset               // unsigned ,   RW,  default = 0
#define VFCE_CHNL1_DS_BUF_OFST                     0x422d
//Bit 31:14       reserved
//Bit 13: 0       reg_ds_buf_offset               // unsigned ,   RW,  default = 0
#define VFCE_CHNL1_HEAD_BUF_OFST                   0x422e
//Bit 31:30       reserved
//Bit 29:16       reg_head_1_offset               // unsigned ,   RW,  default = 0
//Bit 15:14       reserved
//Bit 13: 0       reg_head_0_offset               // unsigned ,   RW,  default = 0
#define VFCE_CHNL1_BODY_BUF_OFST                   0x422f
//Bit 31:27       reserved
//Bit 26:16       reg_body_1_offset               // unsigned ,   RW,  default = 0
//Bit 15:11       reserved
//Bit 10: 0       reg_body_0_offset               // unsigned ,   RW,  default = 0
#define VFCE_CHNL1_STAT1                           0x4230
//Bit 31: 0       ro_dbg_top_info1                // unsigned ,   RO   default = 0
#define VFCE_CHNL1_STAT2                           0x4231
//Bit 31: 0       ro_dbg_top_info2                // unsigned ,   RO   default = 0
#define VFCE_CHNL1_CLR_FLAG                        0x4232
//Bit 31: 0       pls_clr_flag                    // unsigned ,   W1T, default = 0
#define VFCE_CHNL1_STA0_FLAG                       0x4233
//Bit 31: 0       ro_sta0_flag                    // unsigned ,   RO   default = 0
#define VFCE_CHNL1_STA1_FLAG                       0x4234
//Bit 31: 0       ro_sta1_flag                    // unsigned ,   RO   default = 0
#define VFCE_CHNL1_WR_ARB_MISC                     0x4235
//Bit 31          pls_arb_sw_rst                  // unsigned ,   W1T, default = 0
//Bit 30:10       reserved
//Bit  9          reg_arb_awblk_last1             // unsigned ,   RW,  default = 0
//Bit  8          reg_arb_awblk_last0             // unsigned ,   RW,  default = 0
//Bit  7: 4       reg_arb_weight_ch1              // unsigned ,   RW,  default = 15
//Bit  3: 0       reg_arb_weight_ch0              // unsigned ,   RW,  default = 15
#define VFCE_CHNL1_AFBC_MODE_0                     0x4236
//Bit 31:13       reserved
//Bit 12: 5       reg_uncmp_size                  // unsigned ,   RW,  default = 20
//Bit  4: 2       reg_burst_length_add_value      // unsigned ,   RW,  default = 2
//Bit  1          reg_ofset_burst4_en             // unsigned ,   RW,  default = 0
//Bit  0          reg_burst_length_add_en         // unsigned ,   RW,  default = 0
#define VFCE_CHNL1_AFRC_MODE_0                     0x4237
//Bit 31          reg_afrc_head_mode              // unsigned ,   RW,  default = 1
//1 afrc with head, 0 afrc no head
//Bit 30:28       reg_input_format_mode           // unsigned ,   RW,  default = 1
//0:rgb  1:yuv   2:bayer
//Bit 27:24       reg_pixel_format                // unsigned ,   RW,  default = 5
//0:R  1:RG  2:RGB  3:RGBA  4:Y/U/V  5: Y_UV  6:bayer 4plane  7:bayer 2plane  8:bayer 2plane 8x16
//Bit 23:22       reg_input_bayer_mode            // unsigned ,   RW,  default = 1
//0:mono, 1:bayer2x2  2:bayer4x4
//Bit 21:15       reg_comp_target_byte_0          // unsigned ,   RW,  default = 32
//Bit 14: 8       reg_comp_target_byte_1          // unsigned ,   RW,  default = 32
//Bit  7: 4       reg_pixel_type_0                // unsigned ,   RW,  default = 10
//0:RGB, 1:RGBA, 2:R8x8, 3:R16x4, 4:RG
//Bit  3: 0       reg_pixel_type_1                // unsigned ,   RW,  default = 10
//0:RGB, 1:RGBA, 2:R8x8, 3:R16x4, 4:RG
#define VFCE_AFBCE_FORMAT                          0x4240
//Bit 31:15       reserved
//Bit 14:12       reg_burst_length_add_value      // unsigned ,   RW, default = 2
//Bit 11          reg_ofset_burst4_en             // unsigned ,   RW, default = 0
//Bit 10          reg_burst_length_add_en         // unsigned ,   RW, default = 0
//Bit  9: 8       reg_format_mode                 // unsigned ,   RW, default = 2
//data format 0 : YUV444  1:YUV422  2:YUV420  3:RGB
//Bit  7: 4       reg_comp_bits_c                  // unsigned ,   RW, default = 10  chroma bitwidth
//Bit  3: 0       reg_comp_bits_y                  // unsigned ,   RW, default = 10  luma bitwidth
#define VFCE_AFBCE_MODE_EN                         0x4241
//Bit 31:28       reserved
//Bit 27:26       reserved
//Bit 25          reg_adpt_interleave_ymode       // unsigned ,   RW, default = 0
//force 0 to disable it: no  HW implementation
//Bit 24          reg_adpt_interleave_cmode       // unsigned ,   RW, default = 0
//force 0 to disable it: not HW implementation
//Bit 23          reg_adpt_yinterleave_luma_ride  // unsigned ,   RW, default = 1
//vertical interleave piece luma reorder ride    0: no reorder ride  1: w/4 as ride
//Bit 22          reg_adpt_yinterleave_chrm_ride  // unsigned ,   RW, default = 1
//vertical interleave piece chroma reorder ride  0: no reorder ride  1: w/2 as ride
//Bit 21          reg_adpt_xinterleave_luma_ride  // unsigned ,   RW, default = 1
//vertical interleave piece luma reorder ride    0: no reorder ride  1: w/4 as ride
//Bit 20          reg_adpt_xinterleave_chrm_ride  // unsigned ,   RW, default = 1
//vertical interleave piece chroma reorder ride  0: no reorder ride  1: w/2 as ride
//Bit 19          reserved
//Bit 18          reg_disable_order_mode_i_6      // unsigned ,   RW, default = 0
//disable order mode0~6: each mode with one  disable bit: 0: no disable  1: disable
//Bit 17          reg_disable_order_mode_i_5      // unsigned ,   RW, default = 0
//disable order mode0~6: each mode with one  disable bit: 0: no disable  1: disable
//Bit 16          reg_disable_order_mode_i_4      // unsigned ,   RW, default = 0
//disable order mode0~6: each mode with one  disable bit: 0: no disable  1: disable
//Bit 15          reg_disable_order_mode_i_3      // unsigned ,   RW, default = 0
//disable order mode0~6: each mode with one  disable bit: 0: no disable  1: disable
//Bit 14          reg_disable_order_mode_i_2      // unsigned ,   RW, default = 0
//disable order mode0~6: each mode with one  disable bit: 0: no disable  1: disable
//Bit 13          reg_disable_order_mode_i_1      // unsigned ,   RW, default = 0
//disable order mode0~6: each mode with one  disable bit: 0: no disable  1: disable
//Bit 12          reg_disable_order_mode_i_0      // unsigned ,   RW, default = 0
//disable order mode0~6: each mode with one  disable bit: 0: no disable  1: disable
//Bit 11          reserved
//Bit 10          reg_minval_yenc_en              // unsigned ,   RW, default = 0
//force disable  final decision to remove this ws 1% performance loss
//Bit  9          reg_16x4block_enable            // unsigned ,   RW, default = 0
//block as mission  but permit 16x4 block
//Bit  8          reg_uncompress_split_mode       // unsigned ,   RW, default = 0
//0: no split  1: split
//Bit  7: 6       reserved
//Bit  5          reg_input_padding_uv128         // unsigned ,   RW, default = 0
//input picture 32x4 block gap mode: 0:  pad uv=0  1: pad uv=128
//Bit  4          reg_dwds_padding_uv128          // unsigned ,   RW, default = 0
//downsampled image for double write 32x gap mode: 0:  pad uv=0  1: pad uv=128
//Bit  3: 1       reg_force_order_mode_value      // unsigned ,   RW, default = 0
//force order mode 0~7
//Bit  0          reg_force_order_mode_en         // unsigned ,   RW, default = 0
//force order mode enable: 0: no force  1: forced to force_value
#define VFCE_AFBCE_DWSCALAR                        0x4242
//Bit 31: 8       reserved
//Bit  7: 6       reg_dwscalar_w0                 // unsigned ,   RW, default = 3
//horizontal 1st step scalar mode: 0: 1:1 no scalar  1: 2:1 data drop (0 2 4  6) pixel kept  2: 2:1
//data drop (1  3  5 7..) pixels kept  3: avg
//Bit  5: 4       reg_dwscalar_w1                 // unsigned ,   RW, default = 0
//horizontal 2nd step scalar mode: 0: 1:1 no scalar  1: 2:1 data drop (0 2 4  6) pixel kept  2: 2:1
//data drop (1  3  5 7..) pixels kept  3: avg
//Bit  3: 2       reg_dwscalar_h0                 // unsigned ,   RW, default = 2
//vertical 1st step scalar mode: 0: 1:1 no scalar  1: 2:1 data drop (0 2 4  6) pixel kept  2: 2:1
//data drop (1  3  5 7..) pixels kept  3: avg
//Bit  1: 0       reg_dwscalar_h1                 // unsigned ,   RW, default = 3
//vertical 2nd step scalar mode: 0: 1:1 no scalar  1: 2:1 data drop (0 2 4  6) pixel kept  2: 2:1
//data drop (1  3  5 7..) pixels kept  3: avg
#define VFCE_AFBCE_DEF_COLOR_1                      0x4243
//Bit 31:24       reserved
//Bit 23:12       reg_enc_default_color_3          // unsigned ,   RW, default = 4095
//Picture wise default color value in [Y Cb Cr]
//Bit 11: 0       reg_enc_default_color_0          // unsigned ,   RW, default = 4095
//Picture wise default color value in [Y Cb Cr]
#define VFCE_AFBCE_DEF_COLOR_2                      0x4244
//Bit 31:24       reserved
//Bit 23:12       reg_enc_default_color_2          // unsigned ,   RW, default = 4095
//wise default color value in [Y Cb Cr]
//Bit 11: 0       reg_enc_default_color_1          // unsigned ,   RW, default = 4095
//wise default color value in [Y Cb Cr]
#define VFCE_AFBCE_QUANT_ENABLE                    0x4245
//Bit 31:12       reserved
//Bit 11          reg_quant_expand_en_1           // unsigned ,   RW, default = 0
//enable for quantization value expansion
//Bit 10          reg_quant_expand_en_0           // unsigned ,   RW, default = 0
//enable for quantization value expansion
//Bit  9: 8       reg_bcleav_ofst                 //   signed ,   RW, default = 0
//bcleave ofset to get lower range  especially under lossy  for v1/v2  x=0 is equivalent
//Bit  7: 5       reserved
//Bit  4          reg_quant_enable_1              // unsigned ,   RW, default = 0
//enable for quant to get some lossy
//Bit  3: 1       reserved
//Bit  0          reg_quant_enable_0              // unsigned ,   RW, default = 0
//enable for quant to get some lossy
#define VFCE_AFBCE_IQUANT_LUT_1                    0x4246
//Bit 31          reserved
//Bit 30:28       reg_iquant_yclut_0_11           // unsigned ,   RW, default = 0
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit 27          reserved
//Bit 26:24       reg_iquant_yclut_0_10           // unsigned ,   RW, default = 1
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit 23          reserved
//Bit 22:20       reg_iquant_yclut_0_9            // unsigned ,   RW, default = 2
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit 19          reserved
//Bit 18:16       reg_iquant_yclut_0_8            // unsigned ,   RW, default = 3
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit 15          reserved
//Bit 14:12       reg_iquant_yclut_0_7            // unsigned ,   RW, default = 4
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit 11          reserved
//Bit 10: 8       reg_iquant_yclut_0_6            // unsigned ,   RW, default = 5
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit  7          reserved
//Bit  6: 4       reg_iquant_yclut_0_5            // unsigned ,   RW, default = 5
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit  3          reserved
//Bit  2: 0       reg_iquant_yclut_0_4            // unsigned ,   RW, default = 4
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
#define VFCE_AFBCE_IQUANT_LUT_2                    0x4247
//Bit 31:16       reserved
//Bit 15          reserved
//Bit 14:12       reg_iquant_yclut_0_3            // unsigned ,   RW, default = 3
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit 11          reserved
//Bit 10: 8       reg_iquant_yclut_0_2            // unsigned ,   RW, default = 2
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit  7          reserved
//Bit  6: 4       reg_iquant_yclut_0_1            // unsigned ,   RW, default = 1
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit  3          reserved
//Bit  2: 0       reg_iquant_yclut_0_0            // unsigned ,   RW, default = 0
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
#define VFCE_AFBCE_IQUANT_LUT_3                    0x4248
//Bit 31          reserved
//Bit 30:28       reg_iquant_yclut_1_11           // unsigned ,   RW, default = 0
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit 27          reserved
//Bit 26:24       reg_iquant_yclut_1_10           // unsigned ,   RW, default = 1
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit 23          reserved
//Bit 22:20       reg_iquant_yclut_1_9            // unsigned ,   RW, default = 2
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit 19          reserved
//Bit 18:16       reg_iquant_yclut_1_8            // unsigned ,   RW, default = 3
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit 15          reserved
//Bit 14:12       reg_iquant_yclut_1_7            // unsigned ,   RW, default = 4
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit 11          reserved
//Bit 10: 8       reg_iquant_yclut_1_6            // unsigned ,   RW, default = 5
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit  7          reserved
//Bit  6: 4       reg_iquant_yclut_1_5            // unsigned ,   RW, default = 5
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit  3          reserved
//Bit  2: 0       reg_iquant_yclut_1_4            // unsigned ,   RW, default = 4
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
#define VFCE_AFBCE_IQUANT_LUT_4                    0x4249
//Bit 31:16       reserved
//Bit 15          reserved
//Bit 14:12       reg_iquant_yclut_1_3            // unsigned ,   RW, default = 3
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit 11          reserved
//Bit 10: 8       reg_iquant_yclut_1_2            // unsigned ,   RW, default = 2
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit  7          reserved
//Bit  6: 4       reg_iquant_yclut_1_1            // unsigned ,   RW, default = 1
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
//Bit  3          reserved
//Bit  2: 0       reg_iquant_yclut_1_0            // unsigned ,   RW, default = 0
//quantization lut for min_tree leavs  iquant=2^lut(bc_leav_q+1)
#define VFCE_AFBCE_RQUANT_LUT_1                    0x424a
//Bit 31          reserved
//Bit 30:28       reg_rquant_yclut_0_11           // unsigned ,   RW, default = 5
//quantization lut for bctree leavs  quant=2^lut(bc_leav_r+1)  can be calculated from
//iquant_yclut(fw_setting)
//Bit 27          reserved
//Bit 26:24       reg_rquant_yclut_0_10           // unsigned ,   RW, default = 5
//Bit 23          reserved
//Bit 22:20       reg_rquant_yclut_0_9            // unsigned ,   RW, default = 4
//Bit 19          reserved
//Bit 18:16       reg_rquant_yclut_0_8            // unsigned ,   RW, default = 4
//Bit 15          reserved
//Bit 14:12       reg_rquant_yclut_0_7            // unsigned ,   RW, default = 3
//Bit 11          reserved
//Bit 10: 8       reg_rquant_yclut_0_6            // unsigned ,   RW, default = 3
//Bit  7          reserved
//Bit  6: 4       reg_rquant_yclut_0_5            // unsigned ,   RW, default = 2
//Bit  3          reserved
//Bit  2: 0       reg_rquant_yclut_0_4            // unsigned ,   RW, default = 2
#define VFCE_AFBCE_RQUANT_LUT_2                    0x424b
//Bit 31:16       reserved
//Bit 15          reserved
//Bit 14:12       reg_rquant_yclut_0_3            // unsigned ,   RW, default = 1
//Bit 11          reserved
//Bit 10: 8       reg_rquant_yclut_0_2            // unsigned ,   RW, default = 1
//Bit  7          reserved
//Bit  6: 4       reg_rquant_yclut_0_1            // unsigned ,   RW, default = 0
//Bit  3          reserved
//Bit  2: 0       reg_rquant_yclut_0_0            // unsigned ,   RW, default = 0
#define VFCE_AFBCE_RQUANT_LUT_3                    0x424c
//Bit 31          reserved
//Bit 30:28       reg_rquant_yclut_1_11           // unsigned ,   RW, default = 5
//quantization lut for bctree leavs  quant=2^lut(bc_leav_r+1)  can be calculated from
//iquant_yclut(fw_setting)
//Bit 27          reserved
//Bit 26:24       reg_rquant_yclut_1_10           // unsigned ,   RW, default = 5
//Bit 23          reserved
//Bit 22:20       reg_rquant_yclut_1_9            // unsigned ,   RW, default = 4
//Bit 19          reserved
//Bit 18:16       reg_rquant_yclut_1_8            // unsigned ,   RW, default = 4
//Bit 15          reserved
//Bit 14:12       reg_rquant_yclut_1_7            // unsigned ,   RW, default = 3
//Bit 11          reserved
//Bit 10: 8       reg_rquant_yclut_1_6            // unsigned ,   RW, default = 3
//Bit  7          reserved
//Bit  6: 4       reg_rquant_yclut_1_5            // unsigned ,   RW, default = 2
//Bit  3          reserved
//Bit  2: 0       reg_rquant_yclut_1_4            // unsigned ,   RW, default = 2
#define VFCE_AFBCE_RQUANT_LUT_4                    0x424d
//Bit 31:16       reserved
//Bit 15          reserved
//Bit 14:12       reg_rquant_yclut_1_3            // unsigned ,   RW, default = 1
//Bit 11          reserved
//Bit 10: 8       reg_rquant_yclut_1_2            // unsigned ,   RW, default = 1
//Bit  7          reserved
//Bit  6: 4       reg_rquant_yclut_1_1            // unsigned ,   RW, default = 0
//Bit  3          reserved
//Bit  2: 0       reg_rquant_yclut_1_0            // unsigned ,   RW, default = 0
#define VFCE_AFRCE_CORE_PIXEL_TYPE                 0x4250
//Bit 31:16        reserved
//Bit 15:12        reg_pixel_type_0               // unsigned ,    RW, default = 10
//0:RGB, 1:RGBA, 2:R8x8, 3:R16x4, 4:RG
//Bit 11: 8        reg_pixel_type_1               // unsigned ,    RW, default = 10
//0:RGB, 1:RGBA, 2:R8x8, 3:R16x4, 4:RG
//Bit  7: 4        reg_pixel_type_2               // unsigned ,    RW, default = 10
//0:RGB, 1:RGBA, 2:R8x8, 3:R16x4, 4:RG
//Bit  3: 0        reg_pixel_type_3               // unsigned ,    RW, default = 10
//0:RGB, 1:RGBA, 2:R8x8, 3:R16x4, 4:RG
#define VFCE_AFRCE_CORE_PIXEL_DEPTH                0x4251
//Bit 31:16        reserved
//Bit 15:12        reg_input_bit_depth_0          // unsigned ,    RW, default = 10
//Bit 11: 8        reg_input_bit_depth_1          // unsigned ,    RW, default = 10
//Bit  7: 4        reg_input_bit_depth_2          // unsigned ,    RW, default = 10
//Bit  3: 0        reg_input_bit_depth_3          // unsigned ,    RW, default = 10
#define VFCE_AFRCE_CORE_TARGET_SIZE                0x4252
//Bit 31:24        reg_comp_target_byte_0         // unsigned ,    RW, default = 32
//Bit 23:16        reg_comp_target_byte_1         // unsigned ,    RW, default = 32
//Bit 15: 8        reg_comp_target_byte_2         // unsigned ,    RW, default = 32
//Bit  7: 0        reg_comp_target_byte_3         // unsigned ,    RW, default = 32
#define VFCE_AFRCE_CORE_LAST_MAX_AC                0x4253
//Bit 31:28        reg_raw_last_th_0              // unsigned ,    RW, default = 9
//enter emit_raw last min threshold
//Bit 27:24        reg_raw_last_th_1              // unsigned ,    RW, default = 9
//enter emit_raw last min threshold
//Bit 23:20        reg_raw_last_th_2              // unsigned ,    RW, default = 9
//enter emit_raw last min threshold
//Bit 19:16        reg_raw_last_th_3              // unsigned ,    RW, default = 9
//enter emit_raw last min threshold
//Bit 15:12        reg_max_ac_depth_0             // unsigned ,    RW, default = 9
//Bit 11: 8        reg_max_ac_depth_1             // unsigned ,    RW, default = 9
//Bit  7: 4        reg_max_ac_depth_2             // unsigned ,    RW, default = 9
//Bit  3: 0        reg_max_ac_depth_3             // unsigned ,    RW, default = 9
#define VFCE_AFRCE_CORE_HEADER_CTRL                0x4254
//Bit 31:20        reserved
//Bit 19           reg_comp_raw_mode_en_0         // unsigned ,    RW, default = 1
//Bit 18           reg_comp_raw_mode_en_1         // unsigned ,    RW, default = 1
//Bit 17           reg_comp_raw_mode_en_2         // unsigned ,    RW, default = 1
//Bit 16           reg_comp_raw_mode_en_3         // unsigned ,    RW, default = 1
//Bit 15           reg_comp_force_raw_mode_en_0   // unsigned ,    RW, default = 0
//Bit 14           reg_comp_force_raw_mode_en_1   // unsigned ,    RW, default = 0
//Bit 13           reg_comp_force_raw_mode_en_2   // unsigned ,    RW, default = 0
//Bit 12           reg_comp_force_raw_mode_en_3   // unsigned ,    RW, default = 0
//Bit 11           reg_bitstream_pack_mode_0      // unsigned ,    RW, default = 1
//Bit 10           reg_bitstream_pack_mode_1      // unsigned ,    RW, default = 1
//Bit  9           reg_bitstream_pack_mode_2      // unsigned ,    RW, default = 1
//Bit  8           reg_bitstream_pack_mode_3      // unsigned ,    RW, default = 1
//Bit  7           reg_header_last_extend_en_0    // unsigned ,    RW, default = 0  0:
//8/10bit  1:new 12bit 14bit 16bit
//Bit  6           reg_header_last_extend_en_1    // unsigned ,    RW, default = 0  0:
//8/10bit  1:new 12bit 14bit 16bit
//Bit  5           reg_header_last_extend_en_2    // unsigned ,    RW, default = 0  0:
//8/10bit  1:new 12bit 14bit 16bit
//Bit  4           reg_header_last_extend_en_3    // unsigned ,    RW, default = 0  0:
//8/10bit  1:new 12bit 14bit 16bit
//Bit  3           reg_header_mode_0              // unsigned ,    RW, default = 0
//Bit  2           reg_header_mode_1              // unsigned ,    RW, default = 0
//Bit  1           reg_header_mode_2              // unsigned ,    RW, default = 0
//Bit  0           reg_header_mode_3              // unsigned ,    RW, default = 0
#define VFCE_AFRCE_CORE_PACK_MODE_MIN_BYTE         0x4255
//Bit 31:30        reserved
//Bit 29:24        reg_bitstream_pack_min_byte_0  // unsigned ,    RW, default = 4
//Bit 23:22        reserved
//Bit 21:16        reg_bitstream_pack_min_byte_1  // unsigned ,    RW, default = 4
//Bit 15:14        reserved
//Bit 13: 8        reg_bitstream_pack_min_byte_2  // unsigned ,    RW, default = 4
//Bit  7: 6        reserved
//Bit  5: 0        reg_bitstream_pack_min_byte_3  // unsigned ,    RW, default = 4
#define VFCE_AFRCE_CORE_DICT_ERROR_TH              0x4256
//Bit 31:24        reg_dict_error_th_0            // unsigned ,    RW, default = 25
//dictionary threshold
//Bit 23:16        reg_dict_error_th_1            // unsigned ,    RW, default = 25
//dictionary threshold
//Bit 15: 8        reg_dict_error_th_2            // unsigned ,    RW, default = 25
//dictionary threshold
//Bit  7: 0        reg_dict_error_th_3            // unsigned ,    RW, default = 25
//dictionary threshold
#define VFCE_AFRCE_CORE_DICT_ENTER_TH              0x4257
//Bit 31:24        reg_dict_diff_enter_th_0       // unsigned ,    RW, default = 0
//enter dictionary threshold
//Bit 23:16        reg_dict_diff_enter_th_1       // unsigned ,    RW, default = 0
//enter dictionary threshold
//Bit 15: 8        reg_dict_diff_enter_th_2       // unsigned ,    RW, default = 0
//enter dictionary threshold
//Bit  7: 0        reg_dict_diff_enter_th_3       // unsigned ,    RW, default = 0
//enter dictionary threshold
#define VFCE_AFRCE_CORE_DICT_CTRL                  0x4258
//Bit 31:16        reserved
//Bit 15:13        reg_dict_blk_diff_cont_th_0    // unsigned ,    RW, default = 1  diff
//Bit 12:10        reg_dict_blk_diff_cont_th_1    // unsigned ,    RW, default = 1  diff
//Bit  9: 7        reg_dict_blk_diff_cont_th_2    // unsigned ,    RW, default = 1  diff
//Bit  6: 4        reg_dict_blk_diff_cont_th_3    // unsigned ,    RW, default = 1  diff
//Bit  3           reg_dict_en_0                  // unsigned ,    RW, default = 0
//dictionary enable
//Bit  2           reg_dict_en_1                  // unsigned ,    RW, default = 0
//dictionary enable
//Bit  1           reg_dict_en_2                  // unsigned ,    RW, default = 0
//dictionary enable
//Bit  0           reg_dict_en_3                  // unsigned ,    RW, default = 0
//dictionary enable
#define VFCE_AFRCE_INPUT_CTRL                      0x4259
//Bit 31:27        reserved
//Bit 26           reg_rgb2yuv_en                 // unsigned ,    RW, default = 0
//0:non rgb2yuv, 1:rgb2yuv
//Bit 25           reg_pixel_is_diff_chn          // unsigned ,    RW, default = 0
//0:enable lvl 2 dct, 1:disable lvl 2 dct
//Bit 24           reg_input_is_rgba              // unsigned ,    RW, default = 0
//0:yuv header, 1:rgba header
//Bit 23           reserved
//Bit 22:20        reg_input_format_mode          // unsigned ,    RW, default = 1
//0:rgb  1:yuv   2:bayer
//Bit 19:16        reg_pixel_format               // unsigned ,    RW, default = 5
//0:R  1:RG  2:RGB  3:RGBA  4:Y/U/V  5: Y_UV  6:bayer 4plane  7:bayer 2plane  8:bayer 2plane 8x16
//Bit 15:14        reserved
//Bit 13:12        reg_input_yuv_format           // unsigned ,    RW, default = 1
//0:YUV444  1:yuv422  2:yuv420
//Bit 11:10        reserved
//Bit  9: 8        reg_input_bayer_mode           // unsigned ,    RW, default = 1
//0:mono, 1:bayer2x2  2:bayer4x4
//Bit  7: 6        reg_bayer_y_ofst               // unsigned ,    RW, default = 0
//threshold of value diff to calculate block num
//Bit  5: 4        reg_bayer_x_ofst               // unsigned ,    RW, default = 0
//the rsh olf of block num which value diff > th
//Bit  3           reserved
//Bit  2: 0        reg_block_mode                 // unsigned ,    RW, default = 1
//1:8x2(16x4)  2:32x2  3:64x1
#define VFCE_AFRCE_BAYER_PHASE_LUT_0               0x425a
//Bit 31           reserved
//Bit 30:28        reg_raw_phslut_7               // unsigned ,    RW, default = 3  raw phase lut
//Bit 27           reserved
//Bit 26:24        reg_raw_phslut_6               // unsigned ,    RW, default = 4
//global raw phase lut
//Bit 23           reserved
//Bit 22:20        reg_raw_phslut_5               // unsigned ,    RW, default = 3
//global raw phase lut
//Bit 19           reserved
//Bit 18:16        reg_raw_phslut_4               // unsigned ,    RW, default = 4
//global raw phase lut
//Bit 15           reserved
//Bit 14:12        reg_raw_phslut_3               // unsigned ,    RW, default = 2  raw phase lut
//Bit 11           reserved
//Bit 10: 8        reg_raw_phslut_2               // unsigned ,    RW, default = 0
//global raw phase lut
//Bit  7           reserved
//Bit  6: 4        reg_raw_phslut_1               // unsigned ,    RW, default = 1
//global raw phase lut
//Bit  3           reserved
//Bit  2: 0        reg_raw_phslut_0               // unsigned ,    RW, default = 0  raw phase lut
#define VFCE_AFRCE_BAYER_PHASE_LUT_1               0x425b
//Bit 31           reserved
//Bit 30:28        reg_raw_phslut_15              // unsigned ,    RW, default = 3  raw phase lut
//Bit 27           reserved
//Bit 26:24        reg_raw_phslut_14              // unsigned ,    RW, default = 4
//global raw phase lut
//Bit 23           reserved
//Bit 22:20        reg_raw_phslut_13              // unsigned ,    RW, default = 3
//global raw phase lut
//Bit 19           reserved
//Bit 18:16        reg_raw_phslut_12              // unsigned ,    RW, default = 4
//global raw phase lut
//Bit 15           reserved
//Bit 14:12        reg_raw_phslut_11              // unsigned ,    RW, default = 2  raw phase lut
//Bit 11           reserved
//Bit 10: 8        reg_raw_phslut_10              // unsigned ,    RW, default = 0
//global raw phase lut
//Bit  7           reserved
//Bit  6: 4        reg_raw_phslut_9               // unsigned ,    RW, default = 1
//global raw phase lut
//Bit  3           reserved
//Bit  2: 0        reg_raw_phslut_8               // unsigned ,    RW, default = 0  raw phase lut
#define VFCE_MIF_PLANE0_MODE                       0x4260
//Bit 31:28       reserved
//Bit 27:26       reg_rev_mode                    // unsigned ,   RW,  default = 0
//reverse mode bit0:x reverse 1:y reverse
//Bit 25:24       reg_mif_urgent                  // unsigned ,   RW,  default = 3
//info mif and data mif urgent
//Bit 23          reg_awblk_last1                 // unsigned ,   RW,  default = 0
//Bit 22          reg_awblk_last0                 // unsigned ,   RW,  default = 0
//Bit 21:18       reg_weight_ch1                  // unsigned ,   RW,  default = 4
//Bit 17:14       reg_weight_ch0                  // unsigned ,   RW,  default = 8
//Bit 13          pls_arb_sw_rst                  // unsigned ,   W1T, default = 0
//Bit 12: 6       reserved
//Bit  5          reg_mmu_page_mode               // unsigned ,   RW,  default = 0
//0: 4k page, 1: 8k page
//Bit  4          reg_mmu_mode_en                 // unsigned ,   RW,  default = 0
//Bit  3          reserved
//Bit  2: 0       reg_burst_mode                  // unsigned ,   RW,  default = 3,
//0: burst1 1:burst2 2:burst4 3:burst8 4:burst16
#define VFCE_MIF_PLANE0_SIZE                       0x4261
//Bit 31:30       reserved
//Bit 29:28       reg_ddr_blk_size                // unsigned ,   RW,  default = 1
//Bit 27:25       reg_cmd_blk_size                // unsigned ,   RW,  default = 3
//Bit 24:17       reserved
//Bit 16: 0       reg_body_urgent                 // unsigned ,   RW,  default = 0
#define VFCE_MIF_PLANE0_LOOP                       0x4262
//Bit 31:14       reserved
//Bit 13: 2       reg_loop_num                    // unsigned ,   RW,  default = 128
//Bit  1          pls_loop_rst                    // unsigned ,   W1T, default = 0
//Bit  0          reg_loop_mode                   // unsigned ,   RW,  default = 0
#define VFCE_MIF_PLANE0_AXI_STA                    0x4264
//Bit 31: 0       ro_axi_sta                      // unsigned ,   RO   default = 0
#define VFCE_MIF_PLANE0_HEAD_STA                   0x4265
//Bit 31: 0       ro_head_sta                     // unsigned ,   RO   default = 0
#define VFCE_MIF_PLANE0_BODY_STA                   0x4266
//Bit 31: 0       ro_body_sta                     // unsigned ,   RO   default = 0
#define VFCE_MIF_PLANE0_MMU_STA                    0x4267
//Bit 31: 0       ro_mmu_sta                      // unsigned ,   RO   default = 0
#define VFCE_MIF_PLANE1_MODE                       0x4268
//Bit 31:28       reserved
//Bit 27:26       reg_rev_mode                    // unsigned ,   RW,  default = 0
//reverse mode bit0:x reverse 1:y reverse
//Bit 25:24       reg_mif_urgent                  // unsigned ,   RW,  default = 3
//info mif and data mif urgent
//Bit 23          reg_awblk_last1                 // unsigned ,   RW,  default = 0
//Bit 22          reg_awblk_last0                 // unsigned ,   RW,  default = 0
//Bit 21:18       reg_weight_ch1                  // unsigned ,   RW,  default = 4
//Bit 17:14       reg_weight_ch0                  // unsigned ,   RW,  default = 8
//Bit 13          pls_arb_sw_rst                  // unsigned ,   W1T, default = 0
//Bit 12: 6       reserved
//Bit  5          reg_mmu_page_mode               // unsigned ,   RW,  default = 0
//0: 4k page, 1: 8k page
//Bit  4          reg_mmu_mode_en                 // unsigned ,   RW,  default = 0
//Bit  3          reserved
//Bit  2: 0       reg_burst_mode                  // unsigned ,   RW,  default = 3,
//0: burst1 1:burst2 2:burst4 3:burst8 4:burst16
#define VFCE_MIF_PLANE1_SIZE                       0x4269
//Bit 31:30       reserved
//Bit 29:28       reg_ddr_blk_size                // unsigned ,   RW,  default = 1
//Bit 27:25       reg_cmd_blk_size                // unsigned ,   RW,  default = 3
//Bit 24:17       reserved
//Bit 16: 0       reg_body_urgent                 // unsigned ,   RW,  default = 0
#define VFCE_MIF_PLANE1_LOOP                       0x426a
//Bit 31:14       reserved
//Bit 13: 2       reg_loop_num                    // unsigned ,   RW,  default = 128
//Bit  1          pls_loop_rst                    // unsigned ,   W1T, default = 0
//Bit  0          reg_loop_mode                   // unsigned ,   RW,  default = 0
#define VFCE_MIF_PLANE1_AXI_STA                    0x426c
//Bit 31: 0       ro_axi_sta                      // unsigned ,   RO   default = 0
#define VFCE_MIF_PLANE1_HEAD_STA                   0x426d
//Bit 31: 0       ro_head_sta                     // unsigned ,   RO   default = 0
#define VFCE_MIF_PLANE1_BODY_STA                   0x426e
//Bit 31: 0       ro_body_sta                     // unsigned ,   RO   default = 0
#define VFCE_MIF_PLANE1_MMU_STA                    0x426f
//Bit 31: 0       ro_mmu_sta                      // unsigned ,   RO   default = 0
#define VFCE_TOP0_CTRL_0                           0x4270
//Bit 31:13       reserved
//Bit 12: 5       reg_soft_rst_chnl               // unsigned ,   RW,  default = 0  active high
//Bit  4: 2       reg_soft_rst_ram                // unsigned ,   RW,  default = 0  active high
//Bit  1          reg_soft_rst_core               // unsigned ,   RW,  default = 0  active high
//Bit  0          reg_soft_rst_top                // unsigned ,   RW,  default = 0  active high
#define VFCE_TOP0_CTRL_1                           0x4271
//Bit 31:26       reserved
//Bit 25:10       reg_gclk_ctrl_chnl              // unsigned ,   RW,  default = 0
//2'b11:always open, 2'b00:gating clock
//Bit  9: 4       reg_gclk_ctrl_ram               // unsigned ,   RW,  default = 0
//2'b11:always open, 2'b00:gating clock
//Bit  3: 2       reg_gclk_ctrl_core              // unsigned ,   RW,  default = 0
//2'b11:always open, 2'b00:gating clock
//Bit  1: 0       reg_gclk_ctrl_top               // unsigned ,   RW,  default = 0
//2'b11:always open, 2'b00:gating clock
#define VFCE_TOP0_CTRL_2                           0x4272
//Bit 31: 4       reserved
//Bit  3: 1       pls_ram_sw_rst                  // unsigned ,   W1T, default = 0
//active high sync reset afrce by write a pulse
//Bit  0          reg_ram_offset_mode             // unsigned ,   RW,  default = 0
//0:ram offset use default, 1:ram offset use reg
#define VFCE_TOP0_CTRL_3                           0x4273
//Bit 31:20       reserved
//Bit 19:16       reg_gclk_ctrl_afxc              // unsigned ,   RW,  default = 0
//4'hf:always open 4'h0:gating clock
//Bit 15: 2       reserved
//Bit  1          pls_afrc_sw_rst                 // unsigned ,   W1T, default = 0  active high
//Bit  0          pls_afbc_sw_rst                 // unsigned ,   W1T, default = 0  active high
#define VFCE_TOP0_CTRL_4                           0x4274
//Bit 31: 8       reserved
//Bit  7: 0       pls_clr_flag                    // unsigned ,   W1T, default = 0
#define VFCE_TOP0_STA_0                            0x4275
//Bit 31: 0       ro_sta_0                        // unsigned ,   RO,  default = 0
#define VFCE_TOP1_CTRL_0                           0x4278
//Bit 31:13       reserved
//Bit 12: 5       reg_soft_rst_chnl               // unsigned ,   RW,  default = 0  active high
//Bit  4: 2       reg_soft_rst_ram                // unsigned ,   RW,  default = 0  active high
//Bit  1          reg_soft_rst_core               // unsigned ,   RW,  default = 0  active high
//Bit  0          reg_soft_rst_top                // unsigned ,   RW,  default = 0  active high
#define VFCE_TOP1_CTRL_1                           0x4279
//Bit 31:26       reserved
//Bit 25:10       reg_gclk_ctrl_chnl              // unsigned ,   RW,  default = 0
//2'b11:always open, 2'b00:gating clock
//Bit  9: 4       reg_gclk_ctrl_ram               // unsigned ,   RW,  default = 0
//2'b11:always open, 2'b00:gating clock
//Bit  3: 2       reg_gclk_ctrl_core              // unsigned ,   RW,  default = 0
//2'b11:always open, 2'b00:gating clock
//Bit  1: 0       reg_gclk_ctrl_top               // unsigned ,   RW,  default = 0
//2'b11:always open, 2'b00:gating clock
#define VFCE_TOP1_CTRL_2                           0x427a
//Bit 31: 4       reserved
//Bit  3: 1       pls_ram_sw_rst                  // unsigned ,   W1T, default = 0
//active high sync reset afrce by write a pulse
//Bit  0          reg_ram_offset_mode             // unsigned ,   RW,  default = 0
//0:ram offset use default, 1:ram offset use reg
#define VFCE_TOP1_CTRL_3                           0x427b
//Bit 31:20       reserved
//Bit 19:16       reg_gclk_ctrl_afxc              // unsigned ,   RW,  default = 0
//4'hf:always open 4'h0:gating clock
//Bit 15: 2       reserved
//Bit  1          pls_afrc_sw_rst                 // unsigned ,   W1T, default = 0  active high
//Bit  0          pls_afbc_sw_rst                 // unsigned ,   W1T, default = 0  active high
#define VFCE_TOP1_CTRL_4                           0x427c
//Bit 31: 8       reserved
//Bit  7: 0       pls_clr_flag                    // unsigned ,   W1T, default = 0
#define VFCE_TOP1_STA_0                            0x427d
//Bit 31: 0       ro_sta_0                        // unsigned ,   RO,  default = 0
#define VDIN0_IP_PAT_CTRL                          0x4300
//Bit 31: 9        reserved
//Bit  8           reg_ip_pat_en             // unsigned ,    RW, default = 0
//enable ip pattern generation
//Bit  7            reserved
//Bit  6            reserved
//Bit  5: 4        reg_ip_pat_sel            // unsigned ,    RW, default = 2
//selection ip pattern generation, 0: static dot-by-dot check pattern, 1: ramp or purely color,
//2or3: moving blocks  with cadences;
//Bit  3           reg_ip_pat_x_rmp_mode      // unsigned ,    RW, default = 0
//0: ramp-up; 1: 256 gain for this direction
//Bit  2           reg_ip_pat_y_rmp_mode      // unsigned ,    RW, default = 1
//0: ramp-up; 1: 256 gain for this direction
//Bit  1           reg_ip_pat_xinvt          // unsigned ,    RW, default = 0  enalbe x index invert
//Bit  0           reg_ip_pat_yinvt          // unsigned ,    RW, default = 0  enable y index invert
#define VDIN0_IP_PAT_XY_SCL                        0x4301
//Bit 31:29        reserved
//Bit 28           reg_frame_hold            // unsigned ,    RW, default = 0
//Bit 27:16        reg_ip_pat_xidx_scale     // unsigned ,    RW, default = 34  index scale
//Bit 15:12        reserved
//Bit 11: 0        reg_ip_pat_yidx_scale     // unsigned ,    RW, default = 30  index scale
#define VDIN0_IP_PAT_XY_OF_SHIFT                    0x4302
//Bit 31:28        reserved
//Bit 27:16        reg_ip_pat_xidx_ofset     // unsigned ,    RW, default = 0  index offset
//Bit 15: 4        reg_ip_pat_yidx_ofset     // unsigned ,    RW, default = 0  index offset
//Bit  3: 2        reg_ip_pat_xidx_rshft     // unsigned ,    RW, default = 0  0~3
//Bit  1: 0        reg_ip_pat_yidx_rshft     // unsigned ,    RW, default = 0  0~3
#define VDIN0_IP_PAT_FG_X_RMP_RGB                   0x4303
//Bit 31:24        reserved
//Bit 23:16        reg_ip_pat_fg_x_rmp_rgb_0  // unsigned ,    RW, default = 255
//Bit 15: 8        reg_ip_pat_fg_x_rmp_rgb_1  // unsigned ,    RW, default = 255
//Bit  7: 0        reg_ip_pat_fg_x_rmp_rgb_2  // unsigned ,    RW, default = 255  ramp scale
#define VDIN0_IP_PAT_BG_Y_RMP_RGB                   0x4304
//Bit 31:24        reserved
//Bit 23:16        reg_ip_pat_bg_y_rmp_rgb_0  // unsigned ,    RW, default = 255
//Bit 15: 8        reg_ip_pat_bg_y_rmp_rgb_1  // unsigned ,    RW, default = 255
//Bit  7: 0        reg_ip_pat_bg_y_rmp_rgb_2  // unsigned ,    RW, default = 255  ramp scale
#define VDIN0_IP_PAT_RECT_LOCATION                 0x4305
//Bit 31            reserved
//Bit 30:16        reg_ip_pat_rect_xy_0      // unsigned ,    RW, default = 128
//rectangle size x_st;
//Bit 15            reserved
//Bit 14: 0        reg_ip_pat_rect_xy_1      // unsigned ,    RW, default = 128
//rectangle size y_st;
#define VDIN0_IP_PAT_RECT_SIZE                     0x4306
//Bit 31            reserved
//Bit 30:16        reg_ip_pat_rect_xsize     // unsigned ,    RW, default = 128
//rectangle xsize width;
//Bit 15            reserved
//Bit 14: 0        reg_ip_pat_rect_ysize     // unsigned ,    RW, default = 128
//rectangle ysize height;
#define VDIN0_IP_PAT_RECT_MV                       0x4307
//Bit 31:29        reserved
//Bit 28:16        reg_ip_pat_rect_mvx       // signed ,    RW, default = 10  rectangle mvx;
//Bit 15:12        reserved
//Bit 11: 0        reg_ip_pat_rect_mvy       // signed ,    RW, default = 5  rectangle mvy;
#define VDIN0_IP_PAT_RECT_CYCLE                    0x4308
//Bit 31: 9        reserved
//Bit  8           reg_ip_pat_move_mode      // unsigned ,    RW, default = 0
//rectangle move mode, 0:beyond rectangle no display; 1:beyond rectangle return
//Bit  7: 5        reg_ip_pat_mode           // unsigned ,    RW, default = 0
//0: video; 1:22; 2:32; 3: 3223; 4: 2224; 5:32322; 6:44; refer to reg_ip_pat_rect_cycle
//and reg_ip_pat_rect_flg.
//Bit  4: 0        reg_ip_pat_rect_cycle     // unsigned ,    RW, default = 12
//cycle of rectangle, work with rectangle flags, 0 or 1: video, 2: 22, 5: 32,
//10: 4222, 12: 32322 ....
#define VDIN0_IP_PAT_RECT_FLG                      0x4309
//Bit 31: 0        reg_ip_pat_rect_flg       // unsigned ,    RW, default = 2378  0x3e,0 or 1:
//video; 0x2: 22; 0x12: 32; 0xa94: 22323; 0x2a8: 2224 ...
#define VDIN0_IP_PAT_GCLK_CTRL                     0x430a
//Bit 31: 4        reserved
//Bit  3: 2        reg_reg_gclk_ctrl         // unsigned ,    RW, default = 0  reg clk gate ctrl;
//Bit  1: 0        reg_ip_pat_gclk_ctrl      // unsigned ,    RW, default = 0
//pat gen clk gate ctrl;
#define VDIN0_HW_IP_PAT_TEST_BG                    0x430b
//Bit  31:25       reserved
//Bit  24          reg_hw_test_pat_en        // unsigned ,    RW, default = 0  reg_test_bg3_mv;
//Bit  23:16       reg_test_bg3_mv           // unsigned ,    RW, default = 5  reg_test_bg3_mv;
//Bit  15: 8       reg_test_bg2_mv           // unsigned ,    RW, default = 5  reg_test_bg2_mv;
//Bit  7 : 0       reg_test_bg1_mv           // unsigned ,    RW, default = 5  reg_test_bg1_mv;
#define VDIN0_HW_IP_PAT_TEST_GRID                  0x430c
//Bit  31:24       reg_test_grid_w           // unsigned ,    RW, default = 5   reg_test_grid_w;
//Bit  23:16       reg_test_grid_r           // unsigned ,    RW, default = 60  reg_test_grid_r;
//Bit  15:8        reg_test_grid_g           // unsigned ,    RW, default = 70  reg_test_grid_g;
//Bit  7: 0        reg_test_grid_b           // unsigned ,    RW, default = 80  reg_test_grid_b;
#define VDIN0_HW_IP_PAT_TEST_BB0                   0x430d
//Bit  31:26       reserved
//Bit  25:13       reg_test_bb0              // unsigned ,    RW, default = 0  reg_test_bb0;
//Bit  12:0        reg_test_bb1              // unsigned ,    RW, default = 0  reg_test_bb1;
#define VDIN0_HW_IP_PAT_TEST_BB1                   0x430e
//Bit  31:26       reserved
//Bit  25:13       reg_test_bb2              // unsigned ,    RW, default = 4095  reg_test_bb2;
//Bit  12:0        reg_test_bb3              // unsigned ,    RW, default = 4095  reg_test_bb3;
#define VDIN0_HW_IP_PAT_TEST_LOGO0                 0x430f
//Bit  31:26       reserved
//Bit  25:13       reg_test_logo0            // unsigned ,    RW, default = 0  reg_test_logo0;
//Bit  12:0        reg_test_logo1            // unsigned ,    RW, default = 0  reg_test_logo1;
#define VDIN0_HW_IP_PAT_TEST_LOGO1                 0x4310
//Bit  31:26       reserved
//Bit  25:13       reg_test_logo2            // unsigned ,    RW, default = 0  reg_test_logo2;
//Bit  12:0        reg_test_logo3            // unsigned ,    RW, default = 0  reg_test_logo3;
#define VDIN0_HW_IP_PAT_START_CTRL                 0x4311
//Bit  31:17       reserved
//Bit  16          reg_patg_start_sel         // unsigned ,    RW, default = 0  reg_inp_start_sel;
//Bit  15:13       reserved
//Bit  12:0        reg_patg_hold_num          // unsigned ,    RW, default = 5  reg_inp_hold_num;
#define VDIN0_HW_IP_PAT_TRIGGLE                    0x4312
//Bit  31:1        reserved
//Bit  0           pls_patg_frm_start         // unsigned ,    RW, default = 5
//pls_inp_frm_start,need config when reg_inp_start_sel high;
#ifdef CONFIG_AMLOGIC_T6W_VDIN_HDR2
#define VDIN0_HDR2_CTRL                            0x4320
//Bit 31:25        reserved
//Bit 24           reg_ergb_sel_mode         // unsigned ,    RW, default = 0
//hist input source select, 0: input rgb 1: output rgb
//Bit 23:21        reserved
//Bit 20:18        reg_din_swap              // unsigned ,    RW, default = 0  reg_din_swap, hw reg
//Bit 17           reg_out_rgb               // unsigned ,    RW, default = 1  reg_out_fmt
//Bit 16           reg_only_mat              // unsigned ,    RW, default = 0  reg_only_mat, hw reg
//Bit 15           reg_matrix_o_en              // unsigned ,    RW, default = 1  matrix_o_en
//Bit 14           reg_matrix_i_en              // unsigned ,    RW, default = 1  matrix_i_en
//Bit 13           reg_hdr2_top_en           // unsigned ,    RW, default = 0
//hdr2 top enable, hw reg
//Bit 12           reg_c_gain_mode           // unsigned ,    RW, default = 1
//Bit 11: 8        reserved
//Bit  7: 6        reg_gmut_mode             // unsigned ,    RW, default = 1  gmut mode
//Bit  5           reg_in_shift              // unsigned ,    RW, default = 0
//0: use input U10 process,IE_BW=10; 1: use input u12 process
//Bit  4           reg_in_fmt                // unsigned ,    RW, default = 1
//20180719,input already RGB(2020),no need trans
//Bit  3           reg_eo_enable             // unsigned ,    RW, default = 1
//Bit  2           reg_oe_enable             // unsigned ,    RW, default = 1
//Bit  1           reg_ogain_enable          // unsigned ,    RW, default = 1
//Bit  0           reg_cgain_enable          // unsigned ,    RW, default = 0
#define VDIN0_HDR2_CLK_GATE                        0x4321
//Bit 31: 0        reg_gclk_ctrl             // unsigned ,    RW, default = 0
#define VDIN0_HDR2_MATRIXI_COEF00_01               0x4322
//Bit 31:29        reserved
//Bit 28:16        reg_mtrxi_coef00        // signed ,    RW, default = 1023  reg_mtrxi_coef_00
//Bit 15:13        reserved
//Bit 12: 0        reg_mtrxi_coef01        // signed ,    RW, default = 0  reg_mtrxi_coef_01
#define VDIN0_HDR2_MATRIXI_COEF02_10               0x4323
//Bit 31:29        reserved
//Bit 28:16        reg_mtrxi_coef02        // signed ,    RW, default = 1510  reg_mtrxi_coef_02
//Bit 15:13        reserved
//Bit 12: 0        reg_mtrxi_coef10        // signed ,    RW, default = 1023  reg_mtrxi_coef_10
#define VDIN0_HDR2_MATRIXI_COEF11_12               0x4324
//Bit 31:29        reserved
//Bit 28:16        reg_mtrxi_coef11        // signed ,    RW, default = -168  reg_mtrxi_coef_11
//Bit 15:13        reserved
//Bit 12: 0        reg_mtrxi_coef12        // signed ,    RW, default = -585  reg_mtrxi_coef_12
#define VDIN0_HDR2_MATRIXI_COEF20_21               0x4325
//Bit 31:29        reserved
//Bit 28:16        reg_mtrxi_coef20        // signed ,    RW, default = 1023  reg_mtrxi_coef_20
//Bit 15:13        reserved
//Bit 12: 0        reg_mtrxi_coef21        // signed ,    RW, default = 1926  reg_mtrxi_coef_21
#define VDIN0_HDR2_MATRIXI_COEF22                  0x4326
//Bit 31:13        reserved
//Bit 12: 0        reg_mtrxi_coef22        // signed ,    RW, default = 0  reg_mtrxi_coef_22
#define VDIN0_HDR2_MATRIXI_COEF30_31               0x4327
//Bit 31:29        reserved
//Bit 28:16        reg_mtrxi_coef30        // signed ,    RW, default = 0  reg_mtrxi_coef_30
//Bit 15:13        reserved
//Bit 12: 0        reg_mtrxi_coef31        // signed ,    RW, default = 0  reg_mtrxi_coef_31
#define VDIN0_HDR2_MATRIXI_COEF32_40               0x4328
//Bit 31:29        reserved
//Bit 28:16        reg_mtrxi_coef32        // signed ,    RW, default = 0  reg_mtrxi_coef_32
//Bit 15:13        reserved
//Bit 12: 0        reg_mtrxi_coef40        // signed ,    RW, default = 0  reg_mtrxi_coef_40
#define VDIN0_HDR2_MATRIXI_COEF41_42               0x4329
//Bit 31:29        reserved
//Bit 28:16        reg_mtrxi_coef41        // signed ,    RW, default = 0  reg_mtrxi_coef_41
//Bit 15:13        reserved
//Bit 12: 0        reg_mtrxi_coef42        // signed ,    RW, default = 0  reg_mtrxi_coef_42
#define VDIN0_HDR2_MATRIXI_OFFSET0_1               0x432a
//Bit 31:27        reserved
//Bit 26:16        reg_mtrxi_offst_oup0     // signed ,    RW, default = 0  reg_mtrxi_offst_oup_0
//Bit 15:11        reserved
//Bit 10: 0        reg_mtrxi_offst_oup1     // signed ,    RW, default = 0  reg_mtrxi_offst_oup_1
#define VDIN0_HDR2_MATRIXI_OFFSET2                 0x432b
//Bit 31:11        reserved
//Bit 10: 0        reg_mtrxi_offst_oup2     // signed ,    RW, default = 0  reg_mtrxi_offst_oup_2
#define VDIN0_HDR2_MATRIXI_PRE_OFFSET0_1           0x432c
//Bit 31:27        reserved
//Bit 26:16        reg_mtrxi_offst_inp0     // signed ,    RW, default = 0
//Bit 15:11        reserved
//Bit 10: 0        reg_mtrxi_offst_inp1     // signed ,    RW, default = -512
#define VDIN0_HDR2_MATRIXI_PRE_OFFSET2             0x432d
//Bit 31:11        reserved
//Bit 10: 0        reg_mtrxi_offst_inp2     // signed ,    RW, default = -512
#define VDIN0_HDR2_MATRIXO_COEF00_01               0x432e
//Bit 31:29        reserved
//Bit 28:16        reg_mtrxo_coef00        // signed ,    RW, default = 218
//Bit 15:13        reserved
//Bit 12: 0        reg_mtrxo_coef01        // signed ,    RW, default = 732
#define VDIN0_HDR2_MATRIXO_COEF02_10               0x432f
//Bit 31:29        reserved
//Bit 28:16        reg_mtrxo_coef02        // signed ,    RW, default = 74
//Bit 15:13        reserved
//Bit 12: 0        reg_mtrxo_coef10        // signed ,    RW, default = -117
#define VDIN0_HDR2_MATRIXO_COEF11_12               0x4330
//Bit 31:29        reserved
//Bit 28:16        reg_mtrxo_coef11        // signed ,    RW, default = -395
//Bit 15:13        reserved
//Bit 12: 0        reg_mtrxo_coef12        // signed ,    RW, default = 512
#define VDIN0_HDR2_MATRIXO_COEF20_21               0x4331
//Bit 31:29        reserved
//Bit 28:16        reg_mtrxo_coef20        // signed ,    RW, default = 512
//Bit 15:13        reserved
//Bit 12: 0        reg_mtrxo_coef21        // signed ,    RW, default = -465
#define VDIN0_HDR2_MATRIXO_COEF22                  0x4332
//Bit 31:13        reserved
//Bit 12: 0        reg_mtrxo_coef22        // signed ,    RW, default = -47
#define VDIN0_HDR2_MATRIXO_COEF30_31               0x4333
//Bit 31:29        reserved
//Bit 28:16        reg_mtrxo_coef30        // signed ,    RW, default = 0
//Bit 15:13        reserved
//Bit 12: 0        reg_mtrxo_coef31        // signed ,    RW, default = 0
#define VDIN0_HDR2_MATRIXO_COEF32_40               0x4334
//Bit 31:29        reserved
//Bit 28:16        reg_mtrxo_coef32        // signed ,    RW, default = 0
//Bit 15:13        reserved
//Bit 12: 0        reg_mtrxo_coef40        // signed ,    RW, default = 0
#define VDIN0_HDR2_MATRIXO_COEF41_42               0x4335
//Bit 31:29        reserved
//Bit 28:16        reg_mtrxo_coef41        // signed ,    RW, default = 0
//Bit 15:13        reserved
//Bit 12: 0        reg_mtrxo_coef42        // signed ,    RW, default = 0
#define VDIN0_HDR2_MATRIXO_OFFSET0_1               0x4336
//Bit 31:27        reserved
//Bit 26:16        reg_mtrxo_offst_oup0     // signed ,    RW, default = 0  reg_mtrxo_offst_oup_0
//Bit 15:11        reserved
//Bit 10: 0        reg_mtrxo_offst_oup1     // signed ,    RW, default = 512  reg_mtrxo_offst_oup_1
#define VDIN0_HDR2_MATRIXO_OFFSET2                 0x4337
//Bit 31:11        reserved
//Bit 10: 0        reg_mtrxo_offst_oup2     // signed ,    RW, default = 512  reg_mtrxo_offst_oup_2
#define VDIN0_HDR2_MATRIXO_PRE_OFFSET0_1           0x4338
//Bit 31:27        reserved
//Bit 26:16        reg_mtrxo_offst_inp0     // signed ,    RW, default = 0  reg_mtrxo_offst_inp_0
//Bit 15:11        reserved
//Bit 10: 0        reg_mtrxo_offst_inp1     // signed ,    RW, default = 0  reg_mtrxo_offst_inp_1
#define VDIN0_HDR2_MATRIXO_PRE_OFFSET2             0x4339
//Bit 31:11        reserved
//Bit 10: 0        reg_mtrxo_offst_inp2     // signed ,    RW, default = 0  reg_mtrxo_offst_inp_2
#define VDIN0_HDR2_MATRIXI_CLIP                    0x433a
//Bit 31:20        reserved
//Bit 19: 8        reg_mtrxi_comp_thrd       // unsigned ,    RW, default = 0
//Bit  7: 5        reg_mtrxi_rs              // signed ,    RW, default = 0
//of the coef: -2: norm to 256; -1: norm to 512; 0: normalized to 1024 as 1;
//1: norm to 2048; 2: norm to 4096;
//Bit  4: 3        reg_mtrxi_clmod           // unsigned ,    RW, default = 0
//luma mode of BT2020: 0: non-constant luma or none BT2020;
//1: BT2020 CL Y'R'B'=>Y'Cb'Cr' (B'/R'-Y') sign; 2: BT2020 CL: Y'Cb'Cr' =>Y;R'B', Cb'Cr' sign;
//3: BT2020 CL: Y'Cb'Cr' =>Y;R'B', Cb'Cr'- 2^(BL-1) sign;
//Bit  2: 0        reserved
#define VDIN0_HDR2_MATRIXO_CLIP                    0x433b
//Bit 31:20        reserved
//Bit 19: 8        reg_mtrxo_comp_thrd       // unsigned ,    RW, default = 0
//Bit  7: 5        reg_mtrxo_rs              // signed ,    RW, default = 0  of the coef: -2:
//norm to 256; -1: norm to 512; 0: normalized to 1024 as 1; 1: norm to 2048; 2: norm to 4096;
//Bit  4: 3        reg_mtrxo_clmod           // unsigned ,    RW, default = 0  luma mode of BT2020:
//0: non-constant luma or none BT2020;  1: BT2020 CL Y'R'B'=>Y'Cb'Cr' (B'/R'-Y') sign;
//2: BT2020 CL: Y'Cb'Cr' =>Y;R'B', Cb'Cr' sign; 3: BT2020 CL: Y'Cb'Cr' =>Y;R'B',
//Cb'Cr'- 2^(BL-1) sign;
//Bit  2: 0        reserved
#define VDIN0_HDR2_CGAIN_OFFT                      0x433c
//Bit 31:27        reserved
//Bit 26:16        reg_cgain_oft2            // unsigned ,    RW, default = 512  hw reg
//Bit 15:11        reserved
//Bit 10: 0        reg_cgain_oft1            // unsigned ,    RW, default = 512  hw reg
#define VDIN0_HDR2_HIST_RD                         0x433d
//Bit 31:24        reserved
//Bit 23: 0        reg_hist_status           // unsigned ,    RW, default = 0
#define VDIN0_EOTF_LUT_ADDR_PORT                   0x433e
#define VDIN0_EOTF_LUT_DATA_PORT                   0x433f
#define VDIN0_OETF_LUT_ADDR_PORT                   0x4340
#define VDIN0_OETF_LUT_DATA_PORT                   0x4341
#define VDIN0_CGAIN_LUT_ADDR_PORT                  0x4342
#define VDIN0_CGAIN_LUT_DATA_PORT                  0x4343
#define VDIN0_HDR2_CGAIN_COEF0                     0x4344
//Bit 31:28        reserved
//Bit 27:16        reg_coef1                 // unsigned ,    RW, default = 2376
//reg_c_gain_lim_coef1
//Bit 15:12        reserved
//Bit 11: 0        reg_coef0                 // unsigned ,    RW, default = 920
//reg_c_gain_lim_coef0
#define VDIN0_HDR2_CGAIN_COEF1                     0x4345
//Bit 31           reg_sel_opt               // unsigned ,    RW, default = 1
//Bit 30:28        reserved
//Bit 27:16        reg_maxrgb                // unsigned ,    RW, default = 1023
//max rgb in no-linear domain
//Bit 15:12        reserved
//Bit 11: 0        reg_c_gain_lim_coef_2     // unsigned ,    RW, default = 208
//reg_c_gain_lim_coef2
#define VDIN0_OGAIN_LUT_ADDR_PORT                  0x4346
#define VDIN0_OGAIN_LUT_DATA_PORT                  0x4347
#define VDIN0_HDR2_ADPS_CTRL                       0x4348
//Bit 31:24        reserved
//Bit 23:20        reg_adpscl1_sft           // unsigned ,    RW, default = 5  adpscl shift
//Bit 19:18        reserved
//Bit 17           reg_ogain_blend_mode      // unsigned ,    RW, default = 0
//adpscl and adpscl1 result blending or not
//Bit 16           reg_adpscl_sel_opt        // unsigned ,    RW, default = 1  hw reg
//Bit 15:14        reserved
//Bit 13: 8        reg_adpscl_max            // unsigned ,    RW, default = 24
//Bit  7           reg_adpscl_clip_en        // unsigned ,    RW, default = 0  clip enable
//Bit  6           reg_adpscl_bypass2        // unsigned ,    RW, default = 0
//enable of adaptive scaling on linear RGB based on Ys, [0] for R, [1] for G and [2] for B;
//Bit  5           reg_adpscl_bypass1        // unsigned ,    RW, default = 0
//enable of adaptive scaling on linear RGB based on Ys, [0] for R, [1] for G and [2] for B;
//Bit  4           reg_adpscl_bypass0        // unsigned ,    RW, default = 0
//enable of adaptive scaling on linear RGB based on Ys, [0] for R, [1] for G and [2] for B;
//Bit  3: 2        reg_adpscl1_mode          // unsigned ,    RW, default = 1
//0, no-linear input, 1, max linear, 2, adpscl mode
//Bit  1: 0        reg_adpscl_mode           // unsigned ,    RW, default = 1
//0, no-linear input, 1, max linear, 2, adpscl mode
#define VDIN0_HDR2_ADPS_ALPHA0                     0x4349
//Bit 31:30        reserved
//Bit 29:16        reg_adpscl_alpha1        // unsigned ,    RW, default = 1024
//gain(contrast) to linear RGB channel with 1/12 factor, normalized to 1024 as "1" ,
//[0] for R, [1] for G and [2] for B;
//Bit 15:14        reserved
//Bit 13: 0        reg_adpscl_alpha0        // unsigned ,    RW, default = 1024
//gain(contrast) to linear RGB channel with 1/12 factor, normalized to 1024 as "1" ,
//[0] for R, [1] for G and [2] for B;
#define VDIN0_HDR2_ADPS_ALPHA1                     0x434a
//Bit 31:28        reg_adpscl_shift0         // unsigned ,    RW, default = 12
//Bit 27:25        reserved
//Bit 24:20        reg_adpscl_shift1         // unsigned ,    RW, default = 14
//Bit 19:16        reg_adpscl_shift2         // unsigned ,    RW, default = 12  hw no use
//Bit 15:14        reserved
//Bit 13: 0        reg_adpscl_alpha2        // unsigned ,    RW, default = 1024
//gain(contrast) to linear RGB channel with 1/12 factor, normalized to 1024 as "1" ,
//[0] for R, [1] for G and [2] for B;
#define VDIN0_HDR2_ADPS_BETA0                      0x434b
//Bit 31:21        reserved
//Bit 20: 0        reg_adpscl_beta0         // unsigned ,    RW, default = 0
//offset(brightness) to linear RGB channel,  [0] for R, [1] for G and [2] for B;
#define VDIN0_HDR2_ADPS_BETA1                      0x434c
//Bit 31:21        reserved
//Bit 20: 0        reg_adpscl_beta1         // unsigned ,    RW, default = 0
//offset(brightness) to linear RGB channel,  [0] for R, [1] for G and [2] for B;
#define VDIN0_HDR2_ADPS_BETA2                      0x434d
//Bit 31:21        reserved
//Bit 20: 0        reg_adpscl_beta2         // unsigned ,    RW, default = 0
//offset(brightness) to linear RGB channel,  [0] for R, [1] for G and [2] for B;
#define VDIN0_HDR2_ADPS_COEF0                      0x434e
//Bit 31:28        reserved
//Bit 27:16        reg_adpscl_ys_coef1      // unsigned ,    RW, default = 1024
//coef to calculate the Ys, normalized to 2048 as "1", leave one bit margin;
//Bit 15:12        reserved
//Bit 11: 0        reg_adpscl_ys_coef0      // unsigned ,    RW, default = 1024
//coef to calculate the Ys, normalized to 2048 as "1", leave one bit margin;
#define VDIN0_HDR2_ADPS_COEF1                      0x434f
//Bit 31:12        reserved
//Bit 11: 0        reg_adpscl_ys_coef2      // unsigned ,    RW, default = 1024
//coef to calculate the Ys, normalized to 2048 as "1", leave one bit margin;
#define VDIN0_HDR2_GMUT_CTRL                       0x4350
//Bit 31: 5        reserved
//Bit  4           reg_new_mode              // unsigned ,    RW, default = 0
//Bit  3: 0        reg_gmut_shift            // unsigned ,    RW, default = 8
//mult shift  means, if shift = 14, coef = 0x4000; gain = 1
#define VDIN0_HDR2_GMUT_COEF0                      0x4351
//Bit 31:16        reg_gmut_coef01         // signed ,    RW, default = -150
//Bit 15: 0        reg_gmut_coef00         // signed ,    RW, default = 425
#define VDIN0_HDR2_GMUT_COEF1                      0x4352
//Bit 31:16        reg_gmut_coef10         // signed ,    RW, default = -31
//Bit 15: 0        reg_gmut_coef02         // signed ,    RW, default = -18
#define VDIN0_HDR2_GMUT_COEF2                      0x4353
//Bit 31:16        reg_gmut_coef12         // signed ,    RW, default = -2
//Bit 15: 0        reg_gmut_coef11         // signed ,    RW, default = 290
#define VDIN0_HDR2_GMUT_COEF3                      0x4354
//Bit 31:16        reg_gmut_coef21         // signed ,    RW, default = -25
//Bit 15: 0        reg_gmut_coef20         // signed ,    RW, default = -5
#define VDIN0_HDR2_GMUT_COEF4                      0x4355
//Bit 31:16        reserved
//Bit 15: 0        reg_gmut_coef_2_2         // signed ,    RW, default = 286
#define VDIN0_HDR2_PIPE_CTRL1                      0x4356
//Bit 31:24        reg_vblank_num_oetf       // unsigned ,    RW, default = 4
//Bit 23:16        reg_hblank_num_oetf       // unsigned ,    RW, default = 4
//Bit 15: 8        reg_vblank_num_eotf       // unsigned ,    RW, default = 10
//Bit  7: 0        reg_hblank_num_eotf       // unsigned ,    RW, default = 10
#define VDIN0_HDR2_PIPE_CTRL2                      0x4357
//Bit 31:24        reg_vblank_num_cgain      // unsigned ,    RW, default = 10
//Bit 23:16        reg_hblank_num_cgain      // unsigned ,    RW, default = 10
//Bit 15: 8        reg_vblank_num_gmut       // unsigned ,    RW, default = 17
//Bit  7: 0        reg_hblank_num_gmut       // unsigned ,    RW, default = 17
#define VDIN0_HDR2_PIPE_CTRL3                      0x4358
//Bit 31:24        reg_vblank_num_adps       // unsigned ,    RW, default = 22
//Bit 23:16        reg_hblank_num_adps       // unsigned ,    RW, default = 22
//Bit 15: 8        reg_vblank_num_uv         // unsigned ,    RW, default = 4
//Bit  7: 0        reg_hblank_num_uv         // unsigned ,    RW, default = 4
#define VDIN0_HDR2_PROC_WIN1                       0x4359
//Bit 31           reg_proc_win_gmut_en      // unsigned ,    RW, default = 0  hw reg
//Bit 30           reg_proc_win_adps_en      // unsigned ,    RW, default = 0  hw reg
//Bit 29           reg_proc_win_cgain_en     // unsigned ,    RW, default = 0  hw reg
//Bit 28:16        reg_proc_x_ed             // unsigned ,    RW, default = 99
//Bit 15:13        reserved
//Bit 12: 0        reg_proc_x_st             // unsigned ,    RW, default = 0
#define VDIN0_HDR2_PROC_WIN2                       0x435a
//Bit 31:30        reserved
//Bit 29           reg_proc_win_aicr_en      // unsigned ,    RW, default = 1  hw reg
//Bit 28:16        reg_proc_y_ed             // unsigned ,    RW, default = 99
//Bit 15:13        reserved
//Bit 12: 0        reg_proc_y_st             // unsigned ,    RW, default = 0
#define VDIN0_HDR2_MATRIXI_EN_CTRL                 0x435b
//Bit 31: 8        reserved
//Bit  7: 0        reg_matrixi_en_ctrl       // unsigned ,    RW, default = 0
#define VDIN0_HDR2_MATRIXO_EN_CTRL                 0x435c
//Bit 31: 8        reserved
//Bit  7: 0        reg_matrixo_en_ctrl       // unsigned ,    RW, default = 0
#define VDIN0_HDR2_HIST_CTRL                       0x435d
//Bit 31:25        reserved
//Bit 24:17        reg_vcbus_rd_idx          // unsigned ,    RW, default = 0  hw reg
//Bit 16           reg_hist_enable           // unsigned ,    RW, default = 0  hw reg
//Bit 15: 8        reg_gclk_ctrl0            // unsigned ,    RW, default = 20  hw reg
//Bit  7: 6        reserved
//Bit  5           reg_piecewise_mode        // unsigned ,    RW, default = 0
//hist statistic mode select; 1 for piecewise,0 for equal step
//Bit  4           reg_hist_win_mode         // unsigned ,    RW, default = 1
//1 for doing hist statistic processing inside window; 0 for outside window
//Bit  3           reg_maxRGB_rshift         // unsigned ,    RW, default = 0
//o domain data (mainly sdr) shift
//Bit  2: 0        reg_maxRGB_sel            // unsigned ,    RW, default = 0
//hist statistic data select 0: non-linear MAX(R',G',B') before EOTF 2: non-linear Y' before EOTF
//3:non-linear sat before EOTF; 4:   linear MAX(R,G,B) after EOTF, default 5~7:  linear Y after EOTF
#define VDIN0_HDR2_HIST_H_START_END                0x435e
//Bit 31:29        reserved
//Bit 28:16        reg_hist_proc_x_st        // unsigned ,    RW, default = 0  hist proc start x
//Bit 15:13        reserved
//Bit 12: 0        reg_hist_proc_x_ed        // unsigned ,    RW, default = 99  hist proc end x
#define VDIN0_HDR2_HIST_V_START_END                0x435f
//Bit 31:29        reserved
//Bit 28:16        reg_hist_proc_y_st        // unsigned ,    RW, default = 0  hist proc start y
//Bit 15:13        reserved
//Bit 12: 0        reg_hist_proc_y_ed        // unsigned ,    RW, default = 99  hist proc end y
#define VDIN0_OGAIN_LUT1_ADDR_PORT                 0x4360
#define VDIN0_OGAIN_LUT1_DATA_PORT                 0x4361
#define VDIN0_HDR2_CGAIN_VIVID                     0x4362
//Bit 31:30        reserved
//Bit 29:26        reg_omax_sync_gain_sft    // unsigned ,    RW, default = 0
//Bit 25:16        reg_omax_sync_gain_sft    // unsigned ,    RW, default = 1
//Bit 15: 4        reserved
//Bit 3:2          reg_cgain_pos             // unsigned ,    RW, default = 0
//Bit 1            reg_ogain_inser           // unsigned ,    RW, default = 0
//Bit 0            reserved
#define VDIN0_HDR2_GMUT_COMP0                      0x4363
//Bit 31           reg_hdr2_gm_comp_en       // unsigned ,    RW, default = 1
//Bit 30:28        reserved
//Bit 27: 8        reg_hdr_comp_ofst_r       // unsigned ,    RW, default = 85900
//Bit  7: 0        reserved
#define VDIN0_HDR2_GMUT_COMP1                      0x4364
//Bit 31:28        reserved
//Bit 27: 8        reg_hdr_comp_ofst_g       // unsigned ,    RW, default = 85900
//Bit  7: 0        reserved
#define VDIN0_HDR2_GMUT_COMP2                      0x4365
//Bit 31:28        reserved
//Bit 27: 8        reg_hdr_comp_ofst_b       // unsigned ,    RW, default = 85900
//Bit  7: 0        reserved
#define VDIN0_HDR2_GMUT_COMP3                      0x4366
//Bit 31:28        reserved
//Bit 27: 8        reg_hdr_comp_min_r        // unsigned ,    RW, default = 510025
//Bit  7: 0        reserved
#define VDIN0_HDR2_GMUT_COMP4                      0x4367
//Bit 31:28        reserved
//Bit 27: 8        reg_hdr_comp_min_g        // unsigned ,    RW, default = 472965
//Bit  7: 0        reserved
#define VDIN0_HDR2_GMUT_COMP5                      0x4368
//Bit 31:28        reserved
//Bit 27: 8        reg_hdr_comp_min_b        // unsigned ,    RW, default = 467019
//Bit  7: 0        reserved
#define VDIN0_HDR2_GMUT_COMP6                      0x4369
//Bit 31:30        reserved
//Bit 29: 8        reg_hdr_comp_rat_r        // unsigned ,    RW, default = 152108
//Bit  7: 0        reserved
#define VDIN0_HDR2_GMUT_COMP7                      0x436a
//Bit 31:30        reserved
//Bit 29: 8        reg_hdr_comp_rat_g        // unsigned ,    RW, default = 472965
//Bit  7: 0        reserved
#define VDIN0_HDR2_GMUT_COMP8                      0x436b
//Bit 31:30        reserved
//Bit 29: 8        reg_hdr_comp_rat_b        // unsigned ,    RW, default = 467019
//Bit  7: 0        reserved
#endif
#define VDIN0_AA_DS_CTRL                           0x4380
//Bit 31:22        reserved
//Bit 21           reg_aa_ds_buf_mode           // unsigned,   RW, default = 0  wbuf_mode
//Bit 20           reg_aa_ds_ue_clr             // unsigned,   RW, default = 0  clr undone status
//Bit 19:18        reserved
//Bit 17:16        reg_gclk_ctrl                // unsigned,   RW, default = 0  gate clk ctrl
//Bit 15:12        reg_aa_ds_dsx_ofst           // signed ,    RW, default = 0
//horizontal pixel offset for the input pixel to downsample filter
//Bit 11:8         reg_aa_ds_dsy_ofst           // signed ,    RW, default = 0
//vertical pixel offset for the input pixel to downsample filter
//Bit 7 :6         reg_aa_ds_h_step             // unsigned,   RW, default = 1
//downscale mode of x direction for me input data; 0: no downscale; 1:1/2 downscale; 2:1/4 downscale
//Bit 5 :4         reg_aa_ds_v_step             // unsigned,   RW, default = 1
//downscale mode of y direction for me input data; 0: no downscale; 1:1/2 downscale; 2:1/4 downscale
//Bit 3 :2         reserved
//Bit 1            reg_aa_ds_hdsc_en            // unsigned,   RW, default = 1
//downscale of x direction enable
//Bit 0            reg_aa_ds_vdsc_en            // unsigned,   RW, default = 1
//downscale of y direction enable
#define VDIN0_AA_DS_COEF_0                         0x4381
//Bit 31:16        reserved
//Bit 15: 8        reg_aa_ds_dsx_coef_0         // signed ,    RW, default = 24
//coef of AA filter for horizontal downsampling of blended data, normalized to 128 as 1
//Bit  7: 0        reg_aa_ds_dsy_coef_0         // signed ,    RW, default = 24
//coef of AA filter for vertical downsampling of blended data, normalized to 128 as 1
#define VDIN0_AA_DS_COEF_1                         0x4382
//Bit 31:16        reserved
//Bit 15: 8        reg_aa_ds_dsx_coef_1         // signed ,    RW, default = 20
//coef of AA filter for horizontal downsampling of blended data, normalized to 128 as 1
//Bit  7: 0        reg_aa_ds_dsy_coef_1         // signed ,    RW, default = 20
//coef of AA filter for vertical downsampling of blended data, normalized to 128 as 1
#define VDIN0_AA_DS_COEF_2                         0x4383
//Bit 31:16        reserved
//Bit 15: 8        reg_aa_ds_dsx_coef_2         // signed ,    RW, default = 16
//coef of AA filter for horizontal downsampling of blended data, normalized to 128 as 1
//Bit  7: 0        reg_aa_ds_dsy_coef_2         // signed ,    RW, default = 16
//coef of AA filter for vertical downsampling of blended data, normalized to 128 as 1
#define VDIN0_AA_DS_COEF_3                         0x4384
//Bit 31:16        reserved
//Bit 15: 8        reg_aa_ds_dsx_coef_3         // signed ,    RW, default = 16
//coef of AA filter for horizontal downsampling of blended data, normalized to 128 as 1
//Bit  7: 0        reg_aa_ds_dsy_coef_3         // signed ,    RW, default = 16
//coef of AA filter for vertical downsampling of blended data, normalized to 128 as 1
#define VDIN0_AA_DS_BB_SCP_H                       0x4385
//Bit 31:16        reg_aa_ds_bb_hscp0           // unsigned ,  RW, default = 0     ds lft posi, dft0
//Bit 15: 0        reg_aa_ds_bb_hscp1           // unsigned ,  RW, default = 1919  ds rit posi, dft0
#define VDIN0_AA_DS_BB_SCP_V                       0x4386
//Bit 31:16        reg_aa_ds_bb_vscp0           // unsigned ,  RW, default = 0
//ds top posi, dft xsize-1
//Bit 15: 0        reg_aa_ds_bb_vscp1           // unsigned ,  RW, default = 2159
//ds bot posi, dft ysize-1
#define VDIN0_AA_DS_RO_DBG                         0x4387
//Bit 31:1         reserved
//Bit 0            ro_undone_err                // unsigned ,  RO, default = 0

#define VDIN0_WRMIF_FRM_EN_CTRL                    0x4390
#define VDIN0_WRMIF_CTRL0                          0x4391
//Bit 31:18 reserved
//Bit 17    reg_req_ctrl                //unsigned, RW, default=1
//Bit 16    reg_420_p010                //unsigned, RW, default=0
//Bit 15    reserved
//Bit 14:13 reg_field_mode              //unsigned, RW, default=0
//Bit 12    reg_eol_sel                 //unsigned, RW, default=0
//Bit 11    reserved
//Bit 10:8  reg_cmd_intr_len            //unsigned, RW, default=1
//Bit 7:6   reg_cmd_req_size            //unsigned, RW, default=1
//Bit 5:4   reg_burst_len               //unsigned, RW, default=1
//Bit 3:2   reg_int_clr                 //unsigned, RW, default=0
//Bit 1:0   ro_wrmif_abort              //unsigned, RO, default=0
#define VDIN0_WRMIF_CTRL1                          0x4392
//Bit 31    reg_bubble                  //unsigned, RW, default=0
//Bit 30    reg_swap_y                  //unsigned, RW, default=0
//Bit 29:28 reg_pix_bits                //unsigned, RW, default=1
//0:8bit 1:10bit 2:12bit 3:16bit
//Bit 27:26 reg_pix_fmt                 //unsigned, RW, default=0
//0:444 1:422 2:420 3:422(no bubble)
//Bit 25    reg_pix_separate            //unsigned, RW, default=0
//Bit 24    reg_uvswap                  //unsigned, RW, default=0
//Bit 23:22 reg_vmode                   //unsigned, RW, default=2
//Bit 21:20 reg_hmode                   //unsigned, RW, default=0
//Bit 19    reserved
//Bit 18:0  reg_descramble_ctrl         //unsigned, RW, default=0
#define VDIN0_WRMIF_LUMA_CTRL0                     0x4393
//Bit 31:30  reg_luma_sw_rst            //unsigned, RW, default=0
//Bit 29:26  reserved
//Bit 25     reg_luma_swap_64bit        //unsigned, RW, default=0
//Bit 24     reg_luma_little_endian     //unsigned, RW, default=1
//Bit 23     reg_luma_y_rev             //unsigned, RW, default=0
//Bit 22     reg_luma_x_rev             //unsigned, RW, default=0
//Bit 21:20  reg_luma_gclk_ctrl         //unsigned, RW, default=0
//Bit 19:17  reserved
//Bit 16:0   reg_luma_urgent_ctrl       //unsigned, RW, default=0
#define VDIN0_WRMIF_LUMA_BADDR                     0x4394
//Bit 31:0   reg_luma_baddr             //unsigned, RW, default=0
#define VDIN0_WRMIF_LUMA_X                         0x4395
//Bit 31:29 reserved
//Bit 28:16 reg_luma_x_end0              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_luma_x_start0            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_LUMA_Y                         0x4396
//Bit 31:29 reserved
//Bit 28:16 reg_luma_y_end0              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_luma_y_start0            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_LUMA_CTRL1                     0x4397
//Bit 31:16 ro_luma_status              //unsigned, RO, default=0
//Bit 15:14 ro_luma_abort               //unsigned, RO, default=0
//Bit 13    reserved
//Bit 12:0  reg_luma_stride             //unsigned, RW, default=4096
#define VDIN0_WRMIF_CHRM_CTRL0                     0x4398
//Bit 31:30  reg_chrm_sw_rst            //unsigned, RW, default=0
//Bit 29:26  reserved
//Bit 25     reg_chrm_swap_64bit        //unsigned, RW, default=0
//Bit 24     reg_chrm_little_endian     //unsigned, RW, default=1
//Bit 23     reg_chrm_y_rev             //unsigned, RW, default=0
//Bit 22     reg_chrm_x_rev             //unsigned, RW, default=0
//Bit 21:20  reg_chrm_gclk_ctrl         //unsigned, RW, default=0
//Bit 19:17  reserved
//Bit 16:0   reg_chrm_urgent_ctrl       //unsigned, RW, default=0
#define VDIN0_WRMIF_CHRM_BADDR                     0x4399
//Bit 31:0   reg_chrm_baddr             //unsigned, RW, default=0
#define VDIN0_WRMIF_CHRM_X                         0x439a
//Bit 31:29 reserved
//Bit 28:16 reg_chrm_x_end0              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_chrm_x_start0            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_CHRM_Y                         0x439b
//Bit 31:29 reserved
//Bit 28:16 reg_chrm_y_end0              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_chrm_y_start0            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_CHRM_CTRL1                     0x439c
//Bit 31:16 ro_chrm_status              //unsigned, RO, default=0
//Bit 15:14 ro_chrm_abort               //unsigned, RO, default=0
//Bit 13    reserved
//Bit 12:0  reg_chrm_stride             //unsigned, RW, default=4096
#define VDIN0_WRMIF_RGBA_CTRL                      0x439d
//Bit 31:12   reserved
//Bit 11:4    reg_rgba_alp             //unsigned, RW, default=0
//Bit 3       reserved
//Bit 2:1     reg_rgba_mode            //unsigned, RW, default=0
//Bit 0       reg_rgba_en              //unsigned, RW, default=0
#define VDIN0_WRMIF_LUMA_X_SLC1                    0x439e
//Bit 31:29 reserved
//Bit 28:16 reg_luma_x_end1              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_luma_x_start1            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_LUMA_Y_SLC1                    0x439f
//Bit 31:29 reserved
//Bit 28:16 reg_luma_y_end1              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_luma_y_start1            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_CHRM_X_SLC1                    0x43a0
//Bit 31:29 reserved
//Bit 28:16 reg_chrm_x_end1              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_chrm_x_start1            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_CHRM_Y_SLC1                    0x43a1
//Bit 31:29 reserved
//Bit 28:16 reg_chrm_y_end1              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_chrm_y_start1            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_LUMA_X_SLC2                    0x43a2
//Bit 31:29 reserved
//Bit 28:16 reg_luma_x_end2              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_luma_x_start2            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_LUMA_Y_SLC2                    0x43a3
//Bit 31:29 reserved
//Bit 28:16 reg_luma_y_end2              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_luma_y_start2            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_CHRM_X_SLC2                    0x43a4
//Bit 31:29 reserved
//Bit 28:16 reg_chrm_x_end2              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_chrm_x_start2            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_CHRM_Y_SLC2                    0x43a5
//Bit 31:29 reserved
//Bit 28:16 reg_chrm_y_end2              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_chrm_y_start2            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_LUMA_X_SLC3                    0x43a6
//Bit 31:29 reserved
//Bit 28:16 reg_luma_x_end3              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_luma_x_start3            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_LUMA_Y_SLC3                    0x43a7
//Bit 31:29 reserved
//Bit 28:16 reg_luma_y_end3              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_luma_y_start3            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_CHRM_X_SLC3                    0x43a8
//Bit 31:29 reserved
//Bit 28:16 reg_chrm_x_end3              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_chrm_x_start3            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_CHRM_Y_SLC3                    0x43a9
//Bit 31:29 reserved
//Bit 28:16 reg_chrm_y_end3              //unsigned , RW, default = 4095,
//the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_chrm_y_start3            //unsigned , RW, default = 0,
//the canvas hor start pixel position
#define VDIN0_WRMIF_MMU_CTRL                       0x43aa
//Bit 31:7  reserved
//Bit 6:2   reg_mmu_pre_num         // unsigned , RW, default = 4,
//Bit 1     reg_mmu_pg_mode         // unsigned , RW, default = 0,
//Bit 0     reg_mmu_mode_en         // unsigned , RW, default = 0,

/* t6w newly added registers end */

#endif /* __VDIN_T6W_REGS_H */
