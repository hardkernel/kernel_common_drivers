/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef AMDV_REGS_HW5_T6X_HEADER_
#define AMDV_REGS_HW5_T6X_HEADER_

//===========================================================================
// -----------------------------------------------
// REG_BASE:  DV_TV_VCBUS_BASE = 0x45
// -----------------------------------------------
//===========================================================================
//
// Reading file:  ./dolby5_inc/vpu_dolby_hdr_wrap_reg.h
//
#define T6X_VPU_DOLBY_WRAP_CTRL                        0x4501
//Bit 31            reg_dv_core2_byp                  // unsigned ,    RW, default = 0
//Bit 30            reg_dv_sec_ctrl                   // unsigned ,    RW, default = 0
//Bit 29            reg_dv_core1_byp                  // unsigned ,    RW, default = 0
//Bit 28            reg_top2_slice_mode               // unsigned ,    RW, default = 0
//Bit 27            reg_dv_core2_ppc_byp              // unsigned ,    RW, default = 0
//Bit 26            reg_dv_rd_arb_sw_sel              // unsigned ,    RW, default = 1
//Bit 25            pls_dv_rd_arb_sw_rst              // unsigned ,    W1T, default = 0
//Bit 24            reg_dv_core1_done_sel             // unsigned ,    RW, default = 0
//Bit 23            pls_dv_core1_sw_done              // unsigned ,    W1T, default = 0
//Bit 22            reg_crc_en                        // unsigned ,    RW, default = 0
//Bit 21:20         reg_dv_core1_sync_sel             // unsigned ,    RW, default = 0
//Bit 19:5          reserved
//Bit 4             reg_dv_core2_cfg_prl              // unsigned ,    RW, default = 0
//Bit 3             reg_dv_eol_sel                    // unsigned ,    RW, default = 0
//Bit 2             reg_hdr2_eol_sel                  // unsigned ,    RW, default = 0
//Bit 1             reg_hdr2_latch_en                 // unsigned ,    RW, default = 0
//Bit 0             reg_dv_latch_en                   // unsigned ,    RW, default = 0
#define T6X_VPU_DOLBY_WRAP_P2S_OVLP                    0x450f
//Bit  31:12        reserved
//Bit  11:10        reg_dv_s2p_gc_ctrl               // unsigned ,  RW,  default = 0
//Bit   9:8         reg_dv_p2s_gc_ctrl               // unsigned ,  RW,  default = 0
//Bit   7:0         reg_dv_p2s_ovlp_size             // unsigned ,  RW,  default = 192
#define T6X_VPU_DOLBY_WRAP_HSIZE_0                     0x4510
//Bit 31:29         reserved
//Bit 28:16         reg_dolby_hsize_slc1             // unsigned ,  RW,  default = 320
//Bit 15:13         reserved
//Bit 12:0          reg_dolby_hsize_slc0             // unsigned ,  RW,  default = 320
#define T6X_VPU_DOLBY_WRAP_HSIZE_1                     0x4511
//Bit 31:29         reserved
//Bit 28:16         reg_dolby_hsize_slc3             // unsigned ,  RW,  default = 320
//Bit 15:13         reserved
//Bit 12:0          reg_dolby_hsize_slc2             // unsigned ,  RW,  default = 320
#define T6X_VPU_DOLBY_WRAP_CTRL1                       0x4512
//Bit 31            reg_win_cut_en                   // unsigned ,  RW   default = 0
//Bit 30:29         reserved
//Bit 28:16         reg_dolby_vsize                  // unsigned ,  RW,  default = 320
//Bit 15:13         reserved
//Bit 12:0          reserved                         // unsigned ,  RW,  default = 320
#define T6X_VPU_DOLBY_IPATH_SEL                        0x4513
//Bit 31:25         reserved
//Bit 24:22         reg_iloop_gofield_sel            // unsigned ,  RW,  default = 0
//Bit 21:20         reg_dvc1_gofield_sel             // unsigned ,  RW,  default = 0
//Bit 19:16         reserved
//Bit 15:0          reg_dpath_inmux_sel              // unsigned ,  RW,  default = 16'hffff
#define T6X_VPU_DOLBY_OPATH_SEL                        0x4514
//Bit 23:0          reg_dpath_outmux_sel             // unsigned ,  RW,  default = 24'hffffff
#define T6X_VPU_HDR2_SIZE_IN                           0x4515
//Bit 31:29         reserved
//Bit 28:16         reg_hdr2_vsize_in                // unsigned ,  RW,  default = 320
//Bit 15:13         reserved
//Bit 12:0          reg_hdr2_hsize_in                // unsigned ,  RW,  default = 320
#define T6X_VPU_HDR2_SLC_HSIZE_0                       0x4516
//Bit 31:29         reserved
//Bit 28:16         reg_hdr2_hsize_slc1              // unsigned ,  RW,  default = 320
//Bit 15:13         reserved
//Bit 12:0          reg_hdr2_hsize_slc0              // unsigned ,  RW,  default = 320
#define T6X_VPU_HDR2_SLC_HSIZE_1                       0x4517
//Bit 31:29         reserved
//Bit 28:16         reg_hdr2_hsize_slc3              // unsigned ,  RW,  default = 320
//Bit 15:13         reserved
//Bit 12:0          reg_hdr2_hsize_slc2              // unsigned ,  RW,  default = 320
#define T6X_VPU_HDR2_FRM2_SIZE                         0x4518
//Bit 31:29         reserved
//Bit 28:16         reg_hdr2_vsize1_in               // unsigned ,  RW,  default = 320
//Bit 15:13         reserved
//Bit 12:0          reg_hdr2_hsize1_in               // unsigned ,  RW,  default = 320
#define T6X_VPU_HDR2_TOP_CTRL                          0x4519
//Bit 31            pls_hdr2_frm_start               // unsigned ,  W1T, default = 0
//Bit 30:23         reserved
//Bit 22:21         reg_hdr2_gclk_ctrl               // unsigned ,  RW,  default = 0
//Bit 20            reg_hdr2_dma_mode                // unsigned ,  RW,  default = 0
//Bit 19:17         reserved
//Bit 16:4          reg_hdr2_hold_line_num           // unsigned ,  RW,  default = 4
//Bit 3:2           reserved
//Bit 1             reg_hdr2_go_line_sel             // unsigned ,  RW,  default = 0
//Bit 0             reg_hdr2_frm_start_sel           // unsigned ,  RW,  default = 0
#define T6X_VPU_PRIMESL_TOP_CTRL                       0x451a
//Bit 31:3          reserved
//Bit 2             reg_primesl_secure               // unsigned ,  RW,  default = 0
//Bit 1:0           reg_primesl_gclk_ctrl            // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_CRC_ACT_WIN_0                    0x451b
//Bit 31:29         reserved
//Bit 28:16         reg_crc_act_hbgn_slc1            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_crc_act_hbgn_slc0            // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_CRC_ACT_WIN_1                    0x451c
//Bit 31:29         reserved
//Bit 28:16         reg_crc_act_hbgn_slc3            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_crc_act_hbgn_slc2            // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_CRC_ACT_WIN_2                    0x451d
//Bit 31:29         reserved
//Bit 28:16         reg_crc_act_hend_slc1            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_crc_act_hend_slc0            // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_CRC_ACT_WIN_3                    0x451e
//Bit 31:29         reserved
//Bit 28:16         reg_crc_act_hend_slc3            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_crc_act_hend_slc2            // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_WIN_CUT_WIN_S0_0                 0x451f
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut0_hbgn_slc1           // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut0_hbgn_slc0           // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_WIN_CUT_WIN_S0_1                 0x4520
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut0_hbgn_slc3           // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut0_hbgn_slc2           // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_WIN_CUT_WIN_S0_2                 0x4521
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut0_hend_slc1           // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut0_hend_slc0           // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_WIN_CUT_WIN_S0_3                 0x4522
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut0_hend_slc3           // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut0_hend_slc2           // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_WIN_CUT_WIN_S1_0                 0x4523
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut1_hbgn_slc1           // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut1_hbgn_slc0           // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_WIN_CUT_WIN_S1_1                 0x4524
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut1_hbgn_slc3           // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut1_hbgn_slc2           // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_WIN_CUT_WIN_S1_2                 0x4525
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut1_hend_slc1           // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut1_hend_slc0           // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_WIN_CUT_WIN_S1_3                 0x4526
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut1_hend_slc3           // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut1_hend_slc2           // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_WIN_CUT_WIN_0                    0x4527
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut_hbgn_slc1            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut_hbgn_slc0            // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_WIN_CUT_WIN_1                    0x4528
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut_hbgn_slc3            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut_hbgn_slc2            // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_WIN_CUT_WIN_2                    0x4529
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut_hend_slc1            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut_hend_slc0            // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_WIN_CUT_WIN_3                    0x452a
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut_hend_slc3            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut_hend_slc2            // unsigned ,  RW,  default = 0
#define T6X_VPU_DOLBY_RDMA_TRIG_CTRL                   0x452b
//Bit 31:20         reserved
//Bit 19            reg_dv_slc_force                  //unsigned, RW, default = 0
//Bit 18:16         reg_dv_slcnum_force               //unsigned, RW, default = 4
//Bit 15            reg_hdr_slc_force                 //unsigned, RW, default = 0
//Bit 14:12         reg_hdr_slcnum_force              //unsigned, RW, default = 4
//Bit 11            reg_prim_slc_force                //unsigned, RW, default = 0
//Bit 10:8          reg_prim_slcnum_force             //unsigned, RW, default = 4
//Bit 7:1           reserved
//Bit 0             reg_hdr2_rdma_trig_sel            //unsigned, RW, default = 0

#define T6X_VPU_DOLBY_WRAP_RO_DV3                      0x4535
//Bit 31:0         ro_dolby_debug3                  // unsigned ,  RO,  default = 0
#define T6X_VPU_DOLBY_WRAP_RO_DAT0                     0x4536
//Bit 31:0         ro_prob_dat0                     // unsigned ,  RO,  default = 0
#define T6X_VPU_DOLBY_WRAP_RO_DAT1                     0x4537
//Bit 31:0         ro_prob_dat1                     // unsigned ,  RO,  default = 0
#define T6X_VPU_DOLBY_WRAP_RO_DAT2                     0x4538
//Bit 31:0         ro_prob_dat2                     // unsigned ,  RO,  default = 0
#define T6X_VPU_DOLBY_WRAP_RO_CRC                      0x4539
//Bit 31:0         ro_dolby_act_crc                 // unsigned ,  RO,  default = 0

#define T6X_CORE2_SLICE_OFF      0x2b00 //0x7300-0x4800
#define T6X_VPU_AXIRD_DV2X2_SIZE0                      0x6d15
//Bit  31:31       reserved
//Bit  30:29       reg_dv_hdr_2x2_mode          // unsigned ,   RW,  default = 0
//Bit  28:16       reg_frm_hsize_0              // unsigned ,   RW,  default = 320
//Bit  15:13       reserved
//Bit  12: 0       reg_frm_vsize_0              // unsigned ,   RW,  default = 240
#define T6X_VPU_AXIRD_DV2X2_SIZE1                      0x6d16
//Bit  31:29       reserved
//Bit  28:16       reg_frm_hsize_1              // unsigned ,   RW,  default = 320
//Bit  15:13       reserved
//Bit  12: 0       reg_frm_vsize_1              // unsigned ,   RW,  default = 240
#define T6X_VPU_AXIRD_DOLBY_PATH_CTRL                  0x6d30
//Bit  31:4      reserved
//Bit  3         reg_dv_mode_timsplt     // unsigned ,   RW,  default = 0
//Bit  2         reg_dv_paral_mode_t     // unsigned ,   RW,  default = 0
//Bit  1:0       reg_dv_path_mode_t      // unsigned ,   RW,  default = 0,0:off,1:nr_dv,2:vd1_dv
#endif

