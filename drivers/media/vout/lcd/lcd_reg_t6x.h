/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __LCD_REG_T6X_H__
#define __LCD_REG_T6X_H__

#define VPU_INTF_CTRL_T6X                              0x270a

// Reading file:  ./encl_top_reg.h
//
#define ENCL_TCON_INVERT_CTL_T6X                       0x72b0
//Bit 31:30 reg_gclk_ctrl                       // unsigned, RW, default = 2,
//Bit 29:16 reserved
//Bit 15: 0 reg_tcon_invert_ctl                 // unsigned, RW, default = 0,
#define ENCL_VFIFO2VD_CTL_T6X                          0x72b1
//Bit 31:28 reserved
//Bit 27    reg_encl_lcd_scaler_bypass          // unsigned, RW, default = 1,
//Bit 26    reg_encl_vadj_scaler_bypass         // unsigned, RW, default = 1,
//Bit 25    reg_vfifo2vd_out_scaler_bypass      // unsigned, RW, default = 1,
//Bit 24    reg_vfifo_din_full_range            // unsigned, RW, default = 0,
//Bit 23:16 reserved
//Bit 15: 8 reg_vfifo2vd_vd_sel                 // unsigned, RW, default = 8'h1b,
//Bit  7    reg_vfifo2vd_drop                   // unsigned, RW, default = 0,
//Bit  6: 1 reg_vfifo2vd_delay                  // unsigned, RW, default = 0,
//Bit  0    reg_vfifo2vd_en                     // unsigned, RW, default = 0,
#define ENCL_VFIFO2VD_PIXEL_RANGE_T6X                  0x72b2
//Bit 31    reserved
//Bit 30    reg_vdata_yc_use_first_vfifo_req    // unsigned, RW, default = 0,
//Bit 29    reserved
//Bit 28:16 reg_vfifo2vd_pixel_start            // unsigned, RW, default = 0,
//Bit 15:13 reserved
//Bit 12: 0 reg_vfifo2vd_pixel_end              // unsigned, RW, default = 0,
#define ENCL_VFIFO2VD_LINE_TOP_RANGE_T6X               0x72b3
//Bit 31:16 reg_vfifo2vd_line_top_start         // unsigned, RW, default = 0,
//Bit 15: 0 reg_vfifo2vd_line_top_end           // unsigned, RW, default = 0,
#define ENCL_VFIFO2VD_LINE_BOT_RANGE_T6X               0x72b4
//Bit 31:16 reg_vfifo2vd_line_bot_start         // unsigned, RW, default = 0,
//Bit 15: 0 reg_vfifo2vd_line_bot_end           // unsigned, RW, default = 0,
#define ENCL_TST_EN_T6X                                0x72b5
//Bit 31:30 reg_encl_tst_latch                  // unsigned, RW, default = 3,
//Bit 29:14 reserved
//Bit 13:12 reg_encl_tst_vdcnt_strtset          // unsigned, RW, default = 0,
//Bit 11: 9 reserved
//Bit  8    reg_encl_tst_en                     // unsigned, RW, default = 0,
//Bit  7: 0 reg_encl_tst_mode_sel               // unsigned, RW, default = 1,
#define ENCL_TST_DATA_T6X                              0x72b6
//Bit 31:30 reserved
//Bit 29:20 reg_encl_tst_Y                      // unsigned, RW, default = 10'h200,
//Bit 19:10 reg_encl_tst_Cb                     // unsigned, RW, default = 10'h200,
//Bit  9: 0 reg_encl_tst_Cr                     // unsigned, RW, default = 10'h200,
#define ENCL_TST_CLRBAR_CNTL_T6X                       0x72b7
//Bit 31:29 reserved
//Bit 28:16 reg_encl_tst_clrbar_width           // unsigned, RW, default = 13'h168,
//Bit 15:13 reserved
//Bit 12: 0 reg_encl_tst_clrbar_strt            // unsigned, RW, default = 13'h113,
#define ENCL_TST_CRSBAR_CNTL_T6X                       0x72b8
//Bit 31:29 reserved
//Bit 28:16 reg_encl_tst_clrbar_high            // unsigned, RW, default = 13'h100,
//Bit 15:13 reserved
//Bit 12: 0 reg_encl_tst_clrbar_vstrt           // unsigned, RW, default = 13'h80,
#define ENCL_DACSEL_0_T6X                              0x72b9
//Bit 31:16 reg_venc_t_dacsel_0                 // unsigned, RW, default = 16'h3210,
//Bit 15: 0 reg_venc_t_dacsel_1                 // unsigned, RW, default = 16'h0054,
#define ENCL_INBUF_CNTL0_T6X                           0x72ba
//Bit 31:30 reserved
//Bit 29:16 reg_inbuf_size                      // unsigned, RW, default = 14'h200,
//Bit 15    reg_inbuf_rst                       // unsigned, RW, default = 1,
//Bit 14    reg_inbuf_soft_rst_edge             // unsigned, RW, default = 0,
//Bit 13    reg_inbuf_sel_vpp_go_field          // unsigned, RW, default = 0,
//Bit 12: 0 reg_inbuf_line_lenm1                // unsigned, RW, default = 13'heff,
#define ENCL_INBUF_CNT_T6X                             0x72bb
//Bit 31:13 reserved
//Bit 12: 0 ro_inbuf_cnt                        // unsigned, RO
#define ENCL_INBUF_FIX_PIX_NUM_T6X                     0x72bc
//Bit 31:16 reg_inbuf_hold_cnt                  // unsigned, RW, default = 0,
//Bit 15    reg_inbuf_fix_pix_num_en            // unsigned, RW, default = 0,
//Bit 14    reserved
//Bit 13:0  reg_inbuf_vact_num_m1               // unsigned, RW, default = 14'h86f,
//
// Closing file:  ./encl_top_reg.h
//
// 0xc0 - 0xdf  0xf0-0xf8
//
// Reading file:  ./encl_reg.h
//
#define ENCL_VIDEO_EN_T6X                              0x72c0
//Bit 31: 3 reserved
//Bit  2    reg_en_hs_pbpr              // unsigned, RW, default = 0,
//Bit  1    reg_en_gain_hdtv            // unsigned, RW, default = 0,
//Bit  0    reg_video_en                // unsigned, RW, default = 0,
#define ENCL_VIDEO_GAIN_BLANK_T6X                      0x72c1
//Bit 31:30 reserved
//Bit 29:20 reg_video_blankY_val        // unsigned, RW, default = 240,
//Bit 19:10 reg_video_blankPb_val       // unsigned, RW, default = 512,
//Bit  9: 0 reg_video_blankPr_val       // unsigned, RW, default = 512,
#define ENCL_VIDEO_GAIN_RGB_CTRL_T6X                   0x72c2
//Bit 31:16 reg_video_YC_dly            // unsigned, RW, default = 0,
//Bit 15: 0 reg_video_RGB_ctrl          // unsigned, RW, default = 16'h302,
#define ENCL_VIDEO_VSRC_CTRL_T6X                       0x72c3
//Bit 31: 8 reserved
//Bit  7    reg_video_rgbin_zblk        // unsigned, RW, default = 0,
//Bit  6    reg_en_hue_matrix           // unsigned, RW, default = 0,
//Bit  5    reg_swap_pbpr               // unsigned, RW, default = 0,
//Bit  4    reg_vfifo_en                // unsigned, RW, default = 0,
//Bit  3    reserved
//Bit  2: 0 reg_vfifo_upmode            // unsigned, RW, default = 1,
#define ENCL_VIDEO_SCALE_YP_T6X                        0x72c4
//Bit 31:28 reserved
//Bit 27:20 reg_video_Y_scale           // unsigned, RW, default = 81,
//Bit 19:18 reserved
//Bit 17:10 reg_video_Pb_scale          // unsigned, RW, default = 79,
//Bit  9: 8 reserved
//Bit  7: 0 reg_video_Pr_scale          // unsigned, RW, default = 79,
#define ENCL_VIDEO_OFFST_YP_T6X                        0x72c5
//Bit 31:30 reserved
//Bit 29:20 reg_video_Y_offst           // unsigned, RW, default = 0,
//Bit 19:10 reg_video_Pb_offst          // unsigned, RW, default = 0,
//Bit  9: 0 reg_video_Pr_offst          // unsigned, RW, default = 0,
#define ENCL_VIDEO_MATRIX_T6X                          0x72c6
//Bit 31:16 reg_video_matrix_cb         // unsigned, RW, default = 0,
//Bit 15: 0 reg_video_matrix_cr         // unsigned, RW, default = 0,
#define ENCL_VIDEO_MODE_T6X                            0x72c7
//Bit 31:30 reg_gclk_ctrl               // unsigned, RW, default = 2,
//Bit 29:21 reserved
//Bit 20:19 reg_sp_timing_ctrl          // unsigned, RW, default = 0,
//Bit 18    reg_sel_gamma_rgb_in        // unsigned, RW, default = 0,
//Bit 17    reg_vid_lock_adj_en         // unsigned, RW, default = 0,
//Bit 16    reg_video_out_sel           // unsigned, RW, default = 0,
//Bit 15: 0 reg_video_mode              // unsigned, RW, default = 0,
#define ENCL_VIDEO_MAX_CNT_T6X                         0x72c8
//Bit 31:29 reserved
//Bit 28:16 reg_video_max_pxcnt         // unsigned, RW, default = 1715,
//Bit 15: 0 reg_video_max_lncnt         // unsigned, RW, default = 524,
#define ENCL_VIDEO_HAVON_PX_RNG_T6X                    0x72c9
//Bit 31:29 reserved
//Bit 28:16 reg_video_havon_begin       // unsigned, RW, default = 217,
//Bit 15:13 reserved
//Bit 12: 0 reg_video_havon_end         // unsigned, RW, default = 1656,
#define ENCL_VIDEO_VAVON_LN_RNG_T6X                    0x72ca
//Bit 31:16 reg_video_vavon_bline       // unsigned, RW, default = 42,
//Bit 15: 0 reg_video_vavon_eline       // unsigned, RW, default = 519,
#define ENCL_VIDEO_HSO_PX_RNG_T6X                      0x72cb
//Bit 31:29 reserved
//Bit 28:16 reg_video_hso_begin         // unsigned, RW, default = 16,
//Bit 15:13 reserved
//Bit 12: 0 reg_video_hso_end           // unsigned, RW, default = 32,
#define ENCL_VIDEO_VSO_PX_RNG_T6X                      0x72cc
//Bit 31:29 reserved
//Bit 28:16 reg_video_vso_begin         // unsigned, RW, default = 16,
//Bit 15:13 reserved
//Bit 12: 0 reg_video_vso_end           // unsigned, RW, default = 32,
#define ENCL_VIDEO_VSO_LN_RNG_T6X                      0x72cd
//Bit 31:16 reg_video_vso_bline         // unsigned, RW, default = 37,
//Bit 15: 0 reg_video_vso_eline         // unsigned, RW, default = 39,
#define ENCL_VIDEO_OFFST_HV_T6X                        0x72ce
//Bit 31:29 reserved
//Bit 28:16 reg_video_hoffst            // unsigned, RW, default = 2,
//Bit 15: 0 reg_video_voffst            // unsigned, RW, default = 0,
#define ENCL_VIDEO_OFFST_OFLD_T6X                      0x72cf
//Bit 31:16 reg_video_ofld_vpeq_ofst    // unsigned, RW, default = 16'h100,
//Bit 15: 0 reg_video_ofld_voav_ofst    // unsigned, RW, default = 16'h111,
#define ENCL_DBG_PX_LN_RST_T6X                         0x72d0
//Bit 31:16 reg_dbg_px_rst              // unsigned, RW, default = 0,
//Bit 15: 0 reg_dbg_ln_rst              // unsigned, RW, default = 0,
#define ENCL_DBG_PX_LN_INT_T6X                         0x72d1
//Bit 31:16 reg_dbg_px_int              // unsigned, RW, default = 0,
//Bit 15: 0 reg_dbg_ln_int              // unsigned, RW, default = 0,
#define ENCL_MAX_LINE_SWITCH_POINT_T6X                 0x72d2
//Bit 31:13 reserved
//Bit 12: 0 reg_max_line_switch_point   // unsigned, RW, default = 13'h1fff,
#define ENCL_TIM_SYNC_CTRL_T6X                         0x72d3
//Bit 31:13 reserved
//Bit 12: 8 reg_sync_pulse_length       // unsigned, RW, default = 0,
//Bit  7    reserved
//Bit  6    reg_sync_pulse_enable       // unsigned, RW, default = 0,
//Bit  5    reg_short_fussy_sync        // unsigned, RW, default = 0,
//Bit  4    reg_fussy_sync_enable       // unsigned, RW, default = 0,
//Bit  3    reg_enci_sync_enable        // unsigned, RW, default = 0,
//Bit  2    reg_encp_sync_enable        // unsigned, RW, default = 0,
//Bit  1    reg_enct_sync_enable        // unsigned, RW, default = 0,
//Bit  0    reg_encl_sync_enable        // unsigned, RW, default = 0,
#define ENCL_TIM_SYNC_START_T6X                        0x72d4
//Bit 31:29 reserved
//Bit 28:16 reg_sync_pulse_start_pixel  // unsigned, RW, default = 0,
//Bit 15: 0 reg_sync_pulse_start_line   // unsigned, RW, default = 0,
#define ENCL_TIM_SYNC_TARGET_T6X                       0x72d5
//Bit 31:29 reserved
//Bit 28:16 reg_sync_pulse_target_pixel // unsigned, RW, default = 0,
//Bit 15: 0 reg_sync_pulse_target_line  // unsigned, RW, default = 0,
#define VENC_VRR_CTRL_T6X                              0x72d6
//Bit   31   pls_vsp_din      // W1T, pulse
//Bit   30   pls_vrr_clr      // W1T, pulse
//Bit 27:24  reg_vrr_frm_num  // R/W, unsigned, default 1
//Bit 23:8   reg_vsp_dly_num  // R/W, unsigned, default 0
//Bit  7:4   reg_vrr_frm_ths  // R/W, unsigned, default 0
//Bit  3:2   reg_vrr_vsp_en   // R/W, unsigned, default 0
//Bit    1   reg_vrr_mode     // R/W, unsigned, default 0
//Bit    0   reg_vrr_vsp_sel  // R/W, unsigned, default 0
#define VENC_VRR_ADJ_LMT_T6X                           0x72d8
//Bit 31:16  reg_vrr_min_vnum  //R/W, unsigned,
//Bit 15:0   reg_vrr_max_vnum  //R/W, unsigned,
#define VENC_VRR_CTRL1_T6X                             0x72d9
//Bit 31     pls_vsp_cnt_rst   //R/W, unsigned, default 0
//Bit 29:28  reserved
//Bit 27:12  reg_pre_dly_num   //R/W, unsigned, default 16'h40
//Bit 11:4   reg_iofrm_num     //R/W, unsigned, default 8'h10
//Bit 3:0    reg_vsp_rst_num   //R/W, unsigned, default 1
#define VENC_RO_VRR_T6X                                0x72da
//Bit 31:24  ro_vrr_st         // R,
//Bit 23:20  ro_vrr_vsp_cnt    // R,
//Bit 19:16  ro_vrr_max_err    // R,
//Bit 15:0   ro_last_lcnt      // R,
#define ENCL_FRC_CTRL_T6X                              0x72dd
//Bit 31:16  reg_vrr_vtotal    //R/W, unsigned, default 2250
//Bit 15:0   reg_frc_st_ln     //R/W, unsigned, default 176
#define ENCL_SYNC_CTRL_T6X                             0x72de
//Bit 31:0   reg_sync_ctrl     //RW  unsigned, default 0,
//reg_sync_clr      //  W, unsigned, default 0, cfg_sync_ctrl[31];//0; // pulse
//reg_sync_mode     //R/W  unsigned, default 0, cfg_sync_ctrl[24];//0;
//reg_ref_loop_num  //R/W  unsigned, default 0, cfg_sync_ctrl[23:20];//0;
//reg_din_loop_num  //R/W  unsigned, default 0, cfg_sync_ctrl[19:16];//0;
//reg_syn_min_vnum  //R/W  unsigned, default 0, cfg_sync_ctrl[15:0];//1920;
#define ENCL_VIDEO_HSO_PRE_PX_RNG_T6X                  0x72f0
//Bit 31:29 reserved
//Bit 28:16 reg_video_hso_pre_begin         // unsigned, RW, default = 16,
//Bit 15:13 reserved
//Bit 12: 0 reg_video_hso_pre_end           // unsigned, RW, default = 32,
#define ENCL_VIDEO_VSO_PRE_PX_RNG_T6X                  0x72f1
//Bit 31:29 reserved
//Bit 28:16 reg_video_vso_pre_begin         // unsigned, RW, default = 16,
//Bit 15:13 reserved
//Bit 12: 0 reg_video_vso_pre_end           // unsigned, RW, default = 32,
#define ENCL_VIDEO_VSO_PRE_LN_RNG_T6X                  0x72f2
//Bit 31:16 reg_video_vso_pre_bline         // unsigned, RW, default = 37,
//Bit 15: 0 reg_video_vso_pre_eline         // unsigned, RW, default = 39,
#define ENCL_VIDEO_H_PRE_DE_PX_RNG_T6X                 0x72f3
//Bit 31:29 reserved
//Bit 28:16 reg_video_h_pre_de_begin         // unsigned, RW, default = 162,
//Bit 15:13 reserved
//Bit 12: 0 reg_video_h_pre_de_end           // unsigned, RW, default = 2082,
#define ENCL_VIDEO_V_PRE_DE_LN_RNG_T6X                 0x72f4
//Bit 31:16 reg_video_v_pre_de_bline         // unsigned, RW, default = 33,
//Bit 15: 0 reg_video_v_pre_de_eline         // unsigned, RW, default = 40,
//
// Closing file:  ./encl_reg.h
//
// 0xe0 - 0xef
//
// Reading file:  ./vencl_lcd_reg.h
//
#define LCD_LCD_IF_CTRL_T6X                            0x72e0
//Bit 31    reserved
//Bit 30:28 reg_rgb_pola                // unsigned, RW, default = 0,
//Bit 27:23 reserved
//Bit 22:12 reg_rgb_coeff               // unsigned, RW, default = 1024,
//Bit 11:10 reserved
//Bit  9: 0 reg_rgb_base                // unsigned, RW, default = 0,
#define LCD_LCD_TOP_CTRL_T6X                           0x72e1
//Bit 31:30 reg_gclk_ctrl               // unsigned, RW, default = 2,
//Bit 29:13 reserved
//Bit 12: 0 reg_pol_cntl                // unsigned, RW, default = 0,
#define LCD_DITH_CTRL_T6X                              0x72e2
//Bit 31:14 reserved
//Bit 13: 0 reg_dith_cntl               // unsigned, RW, default = 0,
#define LCD_GAMMA_10B_CTRL_T6X                         0x72e3
//Bit 31: 1 reserved
//Bit  0    reg_disable_gamma_10b       // unsigned, RW, default = 0,
//`define     LCD_GAMMA_PROBE_CTRL        8'he4
//Bit 31:16 reg_gamma_probe_hl_color    // unsigned, RW, default = 0,
//Bit 15: 2 reserved
//Bit  1: 0 reg_gamma_probe_ctrl        // unsigned, RW, default = 0,
#define LCD_GAMMA_PROBE_COORD_T6X                      0x72e5
//Bit 31:29 reserved
//Bit 28:16 reg_gamma_probe_pos_y       // unsigned, RW, default = 0,
//Bit 15:13 reserved
//Bit 12: 0 reg_gamma_probe_pos_x       // unsigned, RW, default = 0,
#define LCD_GAMMA_PROBE_COLOR_T6X                      0x72e6
//Bit 31:16 ro_gamma_probe_color_h      // unsigned, RO
//Bit 15: 0 ro_gamma_probe_color_l      // unsigned, RO
//`define     LCD_LDC_AXI_UGT_T6X             8'he7
//Bit 31:16 reserved
//Bit 15: 0 reg_ldc_ctrl                // unsigned, RW, default = 0,
//`define     LCD_LCD_PORT_SWAP_T6X           8'he8
//Bit 31:13 reserved
//Bit 12    ro_lcd_port_swap            // unsigned, RO
//Bit 11    reserved
//Bit 10: 0 ro_vbo_lane_swap            // unsigned, RO
#define LCD_GAMMA_CNTL_PORT0_T6X                       0x72e9
#define LCD_GAMMA_DATA_PORT0_T6X                       0x72ea
#define LCD_GAMMA_ADDR_PORT0_T6X                       0x72eb
#define LCD_OLED_SIZE_T6X                              0x72ec

#endif
