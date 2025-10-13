/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef AMDV_REGS_HW5_T6W_HEADER_
#define AMDV_REGS_HW5_T6W_HEADER_

//===========================================================================
// -----------------------------------------------
// REG_BASE:  DV_TV_VCBUS_BASE = 0x45
// -----------------------------------------------
//===========================================================================
//
// Reading file:  ./dolby5_inc/vpu_dolby_hdr_wrap_reg.h
//
#define T6W_VPU_DOLBY_WRAP_GCLK                        0x4500
//Bit 31:28         reserved                          //
//Bit 27:16         reg_sw_rst                        // unsigned ,    RW, default = 0
//Bit 15:0          reg_gclk_ctrl                     // unsigned ,    RW, default = 0
#define T6W_VPU_DOLBY_WRAP_CTRL                        0x4501
//Bit 31            reg_dv_core2_byp                  // unsigned ,    RW, default = 0
//Bit 30            reg_dv_sec_ctrl                   // unsigned ,    RW, default = 0
//Bit 29:23         reserved
//Bit 22            reg_crc_en                        // unsigned ,    RW, default = 0
//Bit 21:20         reg_dv_core1_sync_sel             // unsigned ,    RW, default = 0
//Bit 19:17         reserved
//Bit 16:4          reg_eol_hsizem1                   // unsigned ,    RW, default = 1279
//Bit 3:2           reg_eol_sel                       // unsigned ,    RW, default = 0
//Bit 1             reg_hdr2_latch_en                 // unsigned ,    RW, default = 0
//Bit 0             reg_dv_latch_en                   // unsigned ,    RW, default = 0
#define T6W_VPU_DOLBY_WRAP_DTNL_CTRL                   0x4502
//Bit 31:5          reserved                          // unsigned
//Bit 4             reg_detunnel_byps                 // unsigned ,  RW,  default = 0
//Bit 3:2           reg_detunnel_en                   // unsigned ,  RW,  default = 0
//Bit 1:0           reg_detunnel_u_start              // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_IRQ_CTRL0                        0x4503 //top1
//Bit 31:21         reserved
//Bit 20            reg_irq0_flg_sel                  // unsigned ,  RW,  default = 0
//Bit 19:18         reserved
//Bit 17:0          reg_irq0_en                       // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_IRQ_CTRL1                        0x4504  //top1
//Bit 31:18         reserved
//Bit 17:0          reg_irq0_clr                      // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_IRQ_CTRL2                        0x4505 //top2
//Bit 31:21         reserved
//Bit 20            reg_irq1_flg_sel                  // unsigned ,  RW,  default = 0
//Bit 19:8          reserved
//Bit 7:0           reg_irq1_en                       // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_IRQ_CTRL3                        0x4506 //top2
//Bit 31:8          reserved
//Bit 7:0           reg_irq1_clr                      // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_WRAP_DTNL                        0x4507
//Bit 31            reserved                          // unsigned
//Bit 30:18         reg_detunnel_hsize                // unsigned ,  RW,  default = 1920
//Bit 17:0          reg_detunnel_sel                  // unsigned ,  RW,  default = 34658
#define T6W_VPU_DOLBY_WRAP_PROB_CTRL                   0x4508
//Bit 31:8          reserved                        // unsigned
//Bit 7:2           reg_prob_sel                    // unsigned ,  RW,  default = 0
//Bit 1:1           reg_prob_clr                    // unsigned ,  RW,  default = 0
//Bit 0:0           reg_prob_en                     // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_WRAP_PROB_SIZE                   0x4509
//Bit 31:16         reg_prob_vsize                  // unsigned ,  RW,  default = 720
//Bit 15:0          reg_prob_hsize                  // unsigned ,  RW,  default = 1280
#define T6W_VPU_DOLBY_WRAP_PROB_WIN                    0x450a
//Bit 31:16         reg_prob_ypos                   // unsigned ,  RW,  default = 0
//Bit 15:0          reg_prob_xpos                   // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_WRAP_IDAT_SWAP                   0x450b
//Bit 31:18         reserved                        // unsigned
//Bit 17:0          reg_dv_idat_swap                // unsigned ,  RW,  default = 181896
#define T6W_VPU_DOLBY_WRAP_ODAT_SWAP                   0x450c
//Bit 31:18         reserved                        // unsigned
//Bit 17:0          reg_dv_odat_swap                // unsigned ,  RW,  default = 181896
#define T6W_VPU_DOLBY_WRAP_RD_ARB_CTRL                 0x450d
//Bit  31:26        reserved
//Bit  25:8         reg_dv_rd_arb_weigh             // unsigned ,  RW,  default = 18'hf
//Bit  7            reserved
//Bit  6:4          reg_dv_rd_arb_req_en            // unsigned ,  RW,  default = 7
//Bit  3:2          reserved
//Bit  1:0          reg_dv_rd_arb_gclk_ctrl         // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_WRAP_WR_ARB_CTRL                 0x450e
//Bit  31:26        reserved
//Bit  25:8         reg_dv_wr_arb_weigh              // unsigned ,  RW,  default = 18'hf
//Bit  7            reserved
//Bit  6:4          reg_dv_wr_arb_req_en             // unsigned ,  RW,  default = 7
//Bit  3:2          reserved
//Bit  1:0          reg_dv_wr_arb_gclk_ctrl          // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_WRAP_HSIZE_0                     0x450f
//Bit 31:29         reserved
//Bit 28:16         reg_dolby_hsize_slc1             // unsigned ,  RW,  default = 320
//Bit 15:13         reserved
//Bit 12:0          reg_dolby_hsize_slc0             // unsigned ,  RW,  default = 320
#define T6W_VPU_DOLBY_WRAP_HSIZE_1                     0x4510
//Bit 31:29         reserved
//Bit 28:16         reg_dolby_hsize_slc3             // unsigned ,  RW,  default = 320
//Bit 15:13         reserved
//Bit 12:0          reg_dolby_hsize_slc2             // unsigned ,  RW,  default = 320
#define T6W_VPU_DOLBY_WRAP_CTRL1                       0x4511
//Bit 31            reg_win_cut_en                   // unsigned ,  RW   default = 0
//Bit 30:29         reserved
//Bit 28:16         reg_dolby_vsize                  // unsigned ,  RW,  default = 320
//Bit 15:13         reserved
//Bit 12:0          reserved                         // unsigned ,  RW,  default = 320
#define T6W_VPU_DOLBY_IPATH_SEL                        0x4512
//Bit 31:25         reserved
//Bit 24:22         reg_iloop_gofield_sel            // unsigned ,  RW,  default = 0
//Bit 21:20         reg_dvc1_gofield_sel             // unsigned ,  RW,  default = 0
//Bit 19:16         reserved
//Bit 15:0          reg_dpath_inmux_sel              // unsigned ,  RW,  default = 16'hffff
#define T6W_VPU_DOLBY_OPATH_SEL                        0x4513
//Bit 23:0          reg_dpath_outmux_sel             // unsigned ,  RW,  default = 24'hffffff
#define T6W_VPU_HDR2_SIZE_IN                           0x4514
//Bit 31:29         reserved
//Bit 28:16         reg_hdr2_vsize_in                // unsigned ,  RW,  default = 320
//Bit 15:13         reserved
//Bit 12:0          reg_hdr2_hsize_in                // unsigned ,  RW,  default = 320
#define T6W_VPU_HDR2_SLC_HSIZE_0                       0x4515
//Bit 31:29         reserved
//Bit 28:16         reg_hdr2_hsize_slc1              // unsigned ,  RW,  default = 320
//Bit 15:13         reserved
//Bit 12:0          reg_hdr2_hsize_slc0              // unsigned ,  RW,  default = 320
#define T6W_VPU_HDR2_SLC_HSIZE_1                       0x4516
//Bit 31:29         reserved
//Bit 28:16         reg_hdr2_hsize_slc3              // unsigned ,  RW,  default = 320
//Bit 15:13         reserved
//Bit 12:0          reg_hdr2_hsize_slc2              // unsigned ,  RW,  default = 320
#define T6W_VPU_HDR2_TOP_CTRL                          0x4517
//Bit 31            pls_hdr2_frm_start               // unsigned ,  W1T, default = 0
//Bit 30:23         reserved
//Bit 22:21         reg_hdr2_gclk_ctrl               // unsigned ,  RW,  default = 0
//Bit 20            reg_hdr2_dma_mode                // unsigned ,  RW,  default = 0
//Bit 19:17         reserved
//Bit 16:4          reg_hdr2_hold_line_num           // unsigned ,  RW,  default = 4
//Bit 3:2           reserved
//Bit 1             reg_hdr2_go_line_sel             // unsigned ,  RW,  default = 0
//Bit 0             reg_hdr2_frm_start_sel           // unsigned ,  RW,  default = 0
#define T6W_VPU_PRIMESL_TOP_CTRL                       0x4518
//Bit 31:3          reserved
//Bit 2             reg_primesl_secure               // unsigned ,  RW,  default = 0
//Bit 1:0           reg_primesl_gclk_ctrl            // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_CRC_ACT_WIN_0                    0x4520
//Bit 31:29         reserved
//Bit 28:16         reg_crc_act_hbgn_slc1            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_crc_act_hbgn_slc0            // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_CRC_ACT_WIN_1                    0x4521
//Bit 31:29         reserved
//Bit 28:16         reg_crc_act_hbgn_slc3            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_crc_act_hbgn_slc2            // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_CRC_ACT_WIN_2                    0x4522
//Bit 31:29         reserved
//Bit 28:16         reg_crc_act_hend_slc1            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_crc_act_hend_slc0            // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_CRC_ACT_WIN_3                    0x4523
//Bit 31:29         reserved
//Bit 28:16         reg_crc_act_hend_slc3            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_crc_act_hend_slc2            // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_WIN_CUT_WIN_0                    0x4524
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut_hbgn_slc1            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut_hbgn_slc0            // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_WIN_CUT_WIN_1                    0x4525
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut_hbgn_slc3            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut_hbgn_slc2            // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_WIN_CUT_WIN_2                    0x4526
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut_hend_slc1            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut_hend_slc0            // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_WIN_CUT_WIN_3                    0x4527
//Bit 31:29         reserved
//Bit 28:16         reg_win_cut_hend_slc3            // unsigned ,  RW,  default = 0
//Bit 15:13         reserved
//Bit 12:0          reg_win_cut_hend_slc2            // unsigned ,  RW,  default = 0
#define T6W_VPU_DOLBY_RDMA_TRIG_CTRL                   0x4528
//Bit 31:20         reserved
//Bit 19            reg_dv_slc_force                  //unsigned, RW, default = 0
//Bit 18:16         reg_dv_slcnum_force               //unsigned, RW, default = 4
//Bit 15            reg_hdr_slc_force                 //unsigned, RW, default = 0
//Bit 14:12         reg_hdr_slcnum_force              //unsigned, RW, default = 4
//Bit 11            reg_prim_slc_force                //unsigned, RW, default = 0
//Bit 10:8          reg_prim_slcnum_force             //unsigned, RW, default = 4
//Bit 7:1           reserved
//Bit 0             reg_hdr2_rdma_trig_sel            //unsigned, RW, default = 0
#define T6W_VPU_DOLBY_WRAP_RO_IRQ0                     0x4530
//Bit 31:18        reserved
//Bit 17:0         ro_irq0_status                    // unsigned ,  RO,  default = 0
#define T6W_VPU_DOLBY_WRAP_RO_IRQ1                     0x4531
//Bit 31:8         reserved
//Bit 7:0          ro_irq1_status                    // unsigned ,  RO,  default = 0
#define T6W_VPU_DOLBY_WRAP_RO_DV0                      0x4532
//Bit 31:0         ro_dolby_debug0                  // unsigned ,  RO,  default = 0
#define T6W_VPU_DOLBY_WRAP_RO_DV1                      0x4533
//Bit 31:0         ro_dolby_debug1                  // unsigned ,  RO,  default = 0
#define T6W_VPU_DOLBY_WRAP_RO_DV2                      0x4534
//Bit 31:0         ro_dolby_debug2                  // unsigned ,  RO,  default = 0
#define T6W_VPU_DOLBY_WRAP_RO_DAT0                     0x4535
//Bit 31:0         ro_prob_dat0                     // unsigned ,  RO,  default = 0
#define T6W_VPU_DOLBY_WRAP_RO_DAT1                     0x4536
//Bit 31:0         ro_prob_dat1                     // unsigned ,  RO,  default = 0
#define T6W_VPU_DOLBY_WRAP_RO_DAT2                     0x4537
//Bit 31:0         ro_prob_dat2                     // unsigned ,  RO,  default = 0
#define T6W_VPU_DOLBY_WRAP_RO_CRC                      0x4538
//Bit 31:0         ro_dolby_act_crc                 // unsigned ,  RO,  default = 0
//
// Closing file:  ./dolby5_inc/vpu_dolby_hdr_wrap_reg.h
//
//===========================================================================
// -----------------------------------------------
// REG_BASE:  DV_TOP1_TOP_VCBUS_BASE = 0x46
// -----------------------------------------------
//===========================================================================
//
// Reading file:  ./dolby5_inc/dolby_top1_reg.h
//
#define T6W_DOLBY_TOP1_PIC_SIZE                        0x4600
//Bit 31:16 reg_pic_vsize       //unsigned, RW, default=1080, dos src pic vsize
//Bit 15:0  reg_pic_hsize       //unsigned, RW, default=1920, dos src pic hsize
#define T6W_DOLBY_TOP1_RDMA_CTRL                       0x4601
//Bit 31    reg_rdma_sw_rst     //unsigned, W1T, default=0, sw rst for rdma
//Bit 30    reg_rdma_on         //unsigned, RW, default=1, top1 rdma on
//Bit 29    reg_rdma_shdw_rst   //unsigned, W1T, default=0, shadow rst for rdma
//Bit 28:19 reserved
//Bit 18:16 reg_rdma_num        //unsigned, RW, default=1, rdma num
//Bit 15:0  reg_rdma_size       //unsigned, RW, default=149, rdma transaction size
#define T6W_DOLBY_TOP1_PYWR_CTRL                       0x4602
//Bit 31    reg_pywrmif_sw_rst  //unsigned, W1T, default=0, sw rst for pyramid wrmif
//Bit 30:10 reg_pyid_mux_map //unsigned, RW, default=0
//Bit 9     reg_pyid_mux_en  //unsigned, RW, default=0
//Bit 8:1   reg_gclk_ctrl    //unsigned, RW, default=0, clk gating ctrl
//Bit 0     reg_py_level     //unsigned, RW, default=1, pymid level, src pic>512x288?1:0
#define T6W_DOLBY_TOP1_PYWR_BADDR1                     0x4603
//Bit 31:0 reg_py_baddr1        //unsigned, RW, default=0, py1 baddr
#define T6W_DOLBY_TOP1_PYWR_BADDR2                     0x4604
//Bit 31:0 reg_py_baddr2        //unsigned, RW, default=0, py1 baddr
#define T6W_DOLBY_TOP1_PYWR_BADDR3                     0x4605
//Bit 31:0 reg_py_baddr3        //unsigned, RW, default=0, py1 baddr
#define T6W_DOLBY_TOP1_PYWR_BADDR4                     0x4606
//Bit 31:0 reg_py_baddr4        //unsigned, RW, default=0, py1 baddr
#define T6W_DOLBY_TOP1_PYWR_BADDR5                     0x4607
//Bit 31:0 reg_py_baddr5        //unsigned, RW, default=0, py1 baddr
#define T6W_DOLBY_TOP1_PYWR_BADDR6                     0x4608
//Bit 31:0 reg_py_baddr6        //unsigned, RW, default=0, py1 baddr
#define T6W_DOLBY_TOP1_PYWR_BADDR7                     0x4609
//Bit 31:0 reg_py_baddr7        //unsigned, RW, default=0, py1 baddr
#define T6W_DOLBY_TOP1_PYWR_STRIDE12                   0x460a
//Bit 31:29 reserved
//Bit 28:16 reg_py_stride2      //unsigned, RW, default=512
//Bit 15:13 reserved
//Bit 12:0  reg_py_stride1      //unsigned, RW, default=1024
#define T6W_DOLBY_TOP1_PYWR_STRIDE34                   0x460b
//Bit 31:29 reserved
//Bit 28:16 reg_py_stride4      //unsigned, RW, default=128
//Bit 15:13 reserved
//Bit 12:0  reg_py_stride3      //unsigned, RW, default=256
#define T6W_DOLBY_TOP1_PYWR_STRIDE56                   0x460c
//Bit 31:29 reserved
//Bit 28:16 reg_py_stride6      //unsigned, RW, default=32
//Bit 15:13 reserved
//Bit 12:0  reg_py_stride5      //unsigned, RW, default=64
#define T6W_DOLBY_TOP1_PYWR_STRIDE7                    0x460d
//Bit 31:15 reg_py_urgent       //unsigned, RW, default=0, pyramid wrmif urgent
//Bit 14:13 reserved
//Bit 12:0  reg_py_stride7      //unsigned, RW, default=16
#define T6W_DOLBY_TOP1_WDMA_CTRL                       0x460e
//Bit 31    reg_wdma_sw_rst     //unsigned ,W1T, default=0, sw rst for wdma
//Bit 30:20 reserved
//Bit 19:17 reg_wdma_slc_num    //unsigned, RW, default=1, histogram && metadata
//Bit 16:0  reg_wdma_urgent     //unsigned, RW, default=0, wdma wrmif urgent
#define T6W_DOLBY_TOP1_WDMA_BADDR0                     0x460f
//Bit 31:0  reg_wdma_baddr0     //unsigned, RW, default=0, histogram baddr
//
#define T6W_DOLBY_TOP1_WDMA_BADDR1                     0x4610
//Bit 31    reg_wdma_frm_cnt_clr //unsigned, W1T, default=0
//Bit 30:24 reserved
//Bit 23:12 reg_wdma_baddr_step //unsigned, RW, default=0
//Bit 11:9  reserved
//Bit 8     reg_wdma_frm_cnt_froce_en  //unsigned, RW, default=0
//Bit 7:4   reg_wdma_frm_cnt_force     //unsigned, RW, default=0;
//Bit 3:0   reserved
#define T6W_DOLBY_TOP1_CTRL0                           0x4611
//Bit 31    reg_sw_reset     //unsigned, W1T, default=0, sw rst for the whole module, used as rst_n
//Bit 30    reg_frm_rst      //unsigned, W1T, default=0, sw-triggered frm_rst
//Bit 29:28 reserved
//Bit 27    reg_core1_byps   //unsigned, RW, default=0, internal bypass
//Bit 26    reg_rdmif_arsec  //unsigned, RW, default=0, pix rdmif security
//Bit 25:24 reg_din_sel      //unsigned, RW, default=2, 1=reg_frm_rst 2=vsync as frm rst, 0/3=idle
//Bit 23    reg_hs_sel       //unsigned, RW, default=0, 0=hsync, 1=hold hend as hsync
//Bit 22:13 reserved
//Bit 12:0  reg_stdly_num  //unsigned, RW, default=2, hold line cnt after frm rst to generate frm_en
#define T6W_DOLBY_TOP1_CTRL1                           0x4612
//Bit 31:29 reserved
//Bit 28:16 reg_hold_vnum       //unsigned, RW, default=4, hold vnum
//Bit 15:13 reserved
//Bit 12:0  reg_hold_hnum       //unsigned, RW, default=2100, hold hnum
#define T6W_DOLBY_TOP1_CTRL2                           0x4613
//Bit 31:30 reg_conv_mode //unsigned, RW, default=0, 0:10bit 1:12bit detunnel,2:12bit depack by 8bit
//Bit 29:12 reg_tunnel_sel //unsigned, RW, default=0, tunnel_sel
//Bit 11    reserved
//Bit 10    reg_pywr_en    //unsigned, RW, default=1
//Bit 9:0   reg_int_sel//unsigned, RW, default=0, output interrupt select, 1:shutdown 0:open
//bit5:0=[dos din, rdma, core1, core1b, pyra, hist]
#define T6W_DOLBY_TOP1_CTRL3                           0x4614
//Bit 31    reg_mmu_reset       //unsigned, W1T, default=0
//Bit 30:4  reserved
//Bit 3     reg_latch_en        //unsigned, RW, default=0
//Bit 2     reg_yuvmif_reg_mode //unsigned, RW, default=0
//Bit 1:0   reg_wdma_reg_mode   //unsigned, RW, default=0
#define T6W_DOLBY_TOP1_HOLD_CTRL0                      0x4615
//Bit 31:0  reg_hs_hold_num0    //unsigned, RW, default=256, delay between rdma done && pix hs
#define T6W_DOLBY_TOP1_LDMA_TRIG_CTRL                  0x4616
//Bit 31     pls_ldma_trig     // unsigned ,  RW,  default = 0
//Bit 30:24  reserved
//Bit 23:12  reg_ldma_trig_clr // unsigned ,  RW,  default = 0
//Bit 11:0   reg_ldma_trig_en  // unsigned ,  RW,  default = 0
#define T6W_DOLBY_TOP1_RDMA_TRIG_CTRL                  0x4617
//Bit 31     pls_rdma_trig     // unsigned ,  RW,  default = 0
//Bit 30:24  reserved
//Bit 23:12  reg_rdma_trig_clr // unsigned ,  RW,  default = 0
//Bit 11:0   reg_rdma_trig_en  // unsigned ,  RW,  default = 0
#define T6W_DOLBY_TOP1_TRIG_RO                         0x4618
//Bit 31:28  reserved
//Bit 27:16  ro_rdma_trig_status // unsigned ,  RO,  default = 0
//Bit 15:12  reserved
//Bit 11:0   ro_ldma_trig_status // unsigned ,  RO,  default = 0
#define T6W_DOLBY_TOP1_RO_0                            0x4619
//Bit 31:0  ro_dbg0         //unsigned, RO, default=0
//
#define T6W_DOLBY_TOP1_RO_1                            0x461a
//Bit 31:0  ro_dbg1         //unsigned, RO, default=0
//
#define T6W_DOLBY_TOP1_RO_2                            0x461b
//Bit 31:0  ro_dbg2         //unsigned, RO, default=0
//
#define T6W_DOLBY_TOP1_RO_3                            0x461c
//Bit 31:0  ro_dbg3         //unsigned, RO, default=0
//
#define T6W_DOLBY_TOP1_RO_4                            0x461d
//Bit 31:0  ro_dbg4         //unsigned, RO, default=0
//
#define T6W_DOLBY_TOP1_RO_5                            0x461e
//Bit 31:0  ro_dbg5         //unsigned, RO, default=0
//
#define T6W_DOLBY_TOP1_RO_6                            0x461f
//Bit 31:0  ro_dbg6         //unsigned, RO, default=0
//
// synopsys translate_off
// synopsys translate_on
//
#define T6W_DOLBY_INP_RMIF_TOP_CTRL    0x4620
//Bit 31    reg_luma_only     // unsigned , RW ,default = 0
//Bit 30    reserved
//Bit 29    reg_last_line_end // unsigned , RW ,default = 1
//Bit 28    reg_last_line_mode// unsigned , RW ,default = 0
//Bit 27:15 reg_luma_last_line_cnt // unsigned , RW ,default = 0
//Bit 14    reg_line_hs_mode //unsigned, RW ,default = 0
//Bit 13    reg_hz_avg      //unsigned,RW ,default = 0
//Bit 12    reg_rgba_en     //unsigned,RW ,default = 0
//Bit 11    reg_uv_swap     //unsigned,RW ,default = 0
//Bit 10    reg_acc_mode    //unsigned,RW ,default = 1
//Bit 9:8   reg_bits_mode   //unsigned,RW, default = 0, 0:8 1:10in10 2:10in12 3:10in16
//Bit 7:6   reg_fmt_mode    //unsigned,RW, default = 0, 0:444 1:422 2:420
//Bit 5:4   reg_plane_mode  //unsigned,RW, default = 0, 0:1plane 1:2plane 2: 3plane
//Bit 3     reg_swap_64bit  //unsigned,RW, default = 0, 64bits of 128bit swap enable
//Bit 2     reg_little_endian //unsigned,RW,default=1,1:little endian 0:big endian enable
//Bit 1     reg_y_rev       //unsigned, RW, default=0, vertical reverse enable
//Bit 0     reg_x_rev       //unsigned, RW, default=0, horizontal reverse enable
#define T6W_DOLBY_INP_MIF0_RMIF_CTRL1   0x4621
//Bit 31    reserved
//Bit 30    reg_field_sel       // unsigned, RW, default = 0
//Bit 29    reg_field_rev       // unsigned, RW, default = 0
//Bit 28:27 reg_mode_sel        // unsigned, RW ,default = 0
//Bit 26:25 reserved
//Bit 24    reg_cav_rst         // unsigned, RW, default = 0
//Bit 23    reg_arb_rst         // unsigned, RW, default = 0
//Bit 22    reg_mif0_32b_align  //RW, default=0, 1:stride32aligned
//Bit 21:20 reg_mif0_block_mode //RW, default=0, 0:linear 1:32x32burst2  2:64x32 burst
//Bit 19    reg_mif0_en         //RW, default=0,
//Bit 18:17 reg_mif0_sync_sel   //RW, default=0, axi canvas id sync with frm rst
//Bit 16:9  reg_mif0_canvas_id  //RW, default=0, axi canvas id num
//Bit 8:6   reg_mif0_cmd_intr_len //RW, default=1,interrupt send cmd when how many series axi cmd,
//0=12 1=16 2=24 3=32 4=40 5=48 6=56 7=64
//Bit 5:3   reg_mif0_cmd_req_size //RW, default=1,how many room fifo have,then axi send series req
//0=16 1=32 2=24 3=64
//Bit 2:0   reg_mif0_burst_len    //RW, default=2,
#define T6W_DOLBY_INP_MIF0_RMIF_CTRL2   0x4622
//Bit 31:30 reg_mif0_sw_rst        // unsigned , RW, default = 0,
//Bit 29:28 reserved
//Bit 27:26 reg_mif0_gclk_ctrl1    // unsigned , RW, default = 0,
//Bit 25:24 reg_f0_field_mode_y    // unsigned , RW, default = 0,
//Bit 23:22 reg_f0_field_mode_c    // unsigned , RW, default = 0,
//Bit 21:20 reg_mif0_int_clr       // unsigned , RW, default = 0
//Bit 19:18 reg_mif0_gclk_ctrl0    // unsigned , RW, default = 0,
//Bit 17    reserved
//Bit 16:0  reg_mif0_urgent_ctrl   // unsigned , RW, default = 0, urgent control reg
#define T6W_DOLBY_INP_MIF0_RMIF_CTRL3           0x4623
//Bit 31    reserved
//Bit 30    reg_mif0_hold_en       // unsigned , RW, default = 0
//Bit 29:24 reg_mif0_pass_num      // unsigned , RW, default = 1
//Bit 23:18 reg_mif0_hold_num      // unsigned , RW, default = 0
//Bit 17    reserved
//Bit 16:13 reg_luma_vstep0       // unsigned , RW, default = 1
//Bit 12:0  reg_mif0_f0_stride     // unsigned , RW, default = 4096,
#define T6W_DOLBY_INP_MIF0_RMIF_CTRL4           0x4624
//Bit 31:0  reg_mif0_f0_baddr      // unsigned , RW, default = 0,
#define T6W_DOLBY_INP_MIF0_RMIF_CTRL5           0x4625
//Bit 31:30  reg_mif0_lath_vld_sel //RW,default=0,latch use 0:frm_start 1:frm_end 2:1'b1 3: 1'b0
//Bit 29:28  reg_mif0_lath_out_sel //RW,default=1,
//[0]:0:use latch 1:use reg,[1]: 0:baddr use baddr_sel 1:baddr use reg
//Bit 27:20  reg_luma_psel_loop    //RW,default=0,
//Bit 19:7   reserved
//Bit 6:3 reg_mif0_pix_bits_mode//RW,default=1,0:4b 1:8b 2:16b 3:32b 4:64b 5:128b 6:12b 7:10b 8:14b
//Bit 2:0 reg_mif0_out_pack_mode//RW,default=0,0:1x 1:2x 2:4x 3:8x 4:16x 5:32x 6:64x
#define T6W_DOLBY_INP_MIF1_RMIF_CTRL1           0x4626
//Bit 31:27 reserved
//Bit 26:23 reg_chrm_vstep0        // RW, default = 1
//Bit 22    reg_mif1_32b_align     //RW, default = 0, 1: stride32aligned
//Bit 21:20 reg_mif1_block_mode    //RW, default = 0, 0: linear 1:32x32burst2  2:64x32 burst4
//Bit 19    reg_mif1_en            //RW, default = 0,
//Bit 18:17 reg_mif1_sync_sel      //RW, default = 0, axi canvas id sync with frm rst
//Bit 16:9  reg_mif1_canvas_id     //RW, default = 0, axi canvas id num
//Bit 8:6   reg_mif1_cmd_intr_len  //RW, default=1,interrupt send cmd when how many series axi cmd,
// 0=12 1=16 2=24 3=32 4=40 5=48 6=56 7=64
//Bit 5:3   reg_mif1_cmd_req_size//RW, default=1,how many room fifo have, then axi send series req,
//0=16 1=32 2=24 3=64
//Bit 2:0   reg_mif1_burst_len    // RW, default = 2,
#define T6W_DOLBY_INP_MIF1_RMIF_CTRL2            0x4627
//Bit 31:30 reg_mif1_sw_rst        //RW, default = 0,
//Bit 29:28 reserved
//Bit 27:26 reg_mif1_gclk_ctrl1    //RW, default = 0,
//Bit 25:22 reserved
//Bit 21:20 reg_mif1_int_clr       //RW, default = 0
//Bit 19:18 reg_mif1_gclk_ctrl0    //RW, default = 0,
//Bit 17    reserved
//Bit 16:0  reg_mif1_urgent_ctrl   //RW, default = 0, urgent control reg
#define T6W_DOLBY_INP_MIF1_RMIF_CTRL3            0x4628
//Bit 31    reserved
//Bit 30    reg_mif1_hold_en       // unsigned , RW, default = 0
//Bit 29:24 reg_mif1_pass_num      // unsigned , RW, default = 1
//Bit 23:18 reg_mif1_hold_num      // unsigned , RW, default = 0
//Bit 17:13 reserved
//Bit 12:0  reg_mif1_f0_stride        // unsigned , RW, default = 4096,
#define T6W_DOLBY_INP_MIF1_RMIF_CTRL4            0x4629
//Bit 31:0  reg_mif1_f0_baddr        // unsigned , RW, default = 0,
#define T6W_DOLBY_INP_MIF1_RMIF_CTRL5            0x462a
//Bit 31:30  reg_mif1_lath_vld_sel //RW, default=0,latch use 0:frm_start 1:frm_end 2:1'b1 3: 1'b0
//Bit 29:28  reg_mif1_lath_out_sel
//RW, default=1,[0]:0:use latch 1:use reg,[1]:0:baddr use baddr_sel 1:baddr use reg
//Bit 27:20  reserved
//Bit 19:7   reg_chrm_last_line_cnt // RW, default = 0,
//Bit 6:3    reg_mif1_pix_bits_mode
//RW, default=1, 0:4b 1:8b 2:16b 3:32b 4:64b 5:128b 6:12b 7:10b 8:14b
//Bit 2:0    reg_mif1_out_pack_mode//RW, default=0, 0:1x 1:2x 2:4x 3:8x 4:16x 5:32x 6:64x
#define T6W_DOLBY_INP_LUMA_RMIF_SCOPE_X          0x462b
//Bit 31:29 reserved
//Bit 28:16 reg_f0_luma_x_end   // unsigned , RW, default = 4095, the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_f0_luma_x_start // unsigned , RW, default = 0, the canvas hor start pixel position
#define T6W_DOLBY_INP_LUMA_RMIF_SCOPE_Y          0x462c
//Bit 31:29 reserved
//Bit 28:16 reg_f0_luma_y_end   // unsigned , RW, default = 0, the canvas ver end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_f0_luma_y_start // unsigned , RW, default = 0, the canvas ver start pixel position
#define T6W_DOLBY_INP_CHRM_RMIF_SCOPE_X          0x462d
//Bit 31:29 reserved
//Bit 28:16 reg_f0_chrm_x_end  // unsigned , RW, default = 4095, the canvas hor end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_f0_chrm_x_start // unsigned , RW, default = 0, the canvas hor start pixel position
#define T6W_DOLBY_INP_CHRM_RMIF_SCOPE_Y          0x462e
//Bit 31:29 reserved
//Bit 28:16 reg_f0_chrm_y_end  // unsigned , RW, default = 0, the canvas ver end pixel position
//Bit 15:13 reserved
//Bit 12: 0 reg_f0_chrm_y_start// unsigned , RW, default = 0, the canvas ver start pixel position
#define T6W_DOLBY_INP_RMIF_RO_STAT               0x462f
//Bit 31    reserved
//Bit 30:0  ro_status        // unsigned , RO, default = 0 ,
#define T6W_DOLBY_INP_RMIF_DUMMY_PIXEL           0x4630
//Bit 31:0     reg_dummy_pixel_val          //unsigned, RW, default = 8421376
#define T6W_DOLBY_INP_RMIF_RPT_LOOP              0x4631
//Bit 31:24     reg_f1_chrm_rpt_loop        //unsigned, RW, default =0
//Bit 23:16     reg_f1_luma_rpt_loop        //unsigned, RW, default =0
//Bit 15:8      reg_f0_chrm_rpt_loop        //unsigned, RW, default =0
//Bit  7:0      reg_f0_luma_rpt_loop        //unsigned, RW, default =0
#define T6W_DOLBY_INP_RMIF_F0_LUMA_RPT_PAT       0x4632
//Bit 31:0     reg_f0_luma_rpt_pat          //unsigned, RW, default = 0
#define T6W_DOLBY_INP_RMIF_F0_CHRM_RPT_PAT       0x4633
//Bit 31:0     reg_f0_chrm_rpt_pat          //unsigned, RW, default = 0
#define T6W_DOLBY_INP_RMIF_FIFO_SIZE             0x4634
//Bit 31:29 reserved
//Bit 28:16 reg_chrm_fifo_size         // unsigned , RW, default = 32,
//Bit 15:13 reserved
//Bit 12: 0 reg_luma_fifo_size         // unsigned , RW, default = 64,
//
// Closing file:  ./dpss_inc/../ip_inc/mif/inp_pix_rdmif_reg.h
//
//
// Reading file:  ./dpss_inc/../ip_inc/mif/inp_fmtup_reg.h
//
#define T6W_DOLBY_INP_VD_CFMT_CTRL               0x4636
//Bit 31    cfmt_gclk_bit_dis //RW, default=0;//if true, disable clock,otherwise enable clock
//Bit 30    cfmt_soft_rst_bit //RW, default=0;//soft rst bit
//Bit 29    reserved
//Bit 28    chfmt_rpt_pix //RW, default =0;
//if true,horizontal formatter use repeating to generate pixel,otherwise use bilinear interpolation
//Bit 27:24 chfmt_ini_phase //RW, default= 0;//horizontal formatter initial phase
//Bit 23    chfmt_rpt_p0_en //RW, default= 0;//horizontal formatter repeat pixel 0 enable
//Bit 22:21 chfmt_yc_ratio// RW, default= 0;//horizontal Y/C ratio, 00: 1:1, 01: 2:1, 10: 4:1
//Bit 20    chfmt_en      //RW, default= 0; //horizontal formatter enable
//Bit 19    cvfmt_phase0_always_en //RW, default = 0;
//if true, always use phase0 while vertical formater, meaning always //repeat data, no interpolation
//Bit 18    cvfmt_rpt_last_dis //RW,default=0;
//if true, disable vertical formatter chroma repeat last line
//Bit 17    cvfmt_phase0_nrpt_en//RW,default=0;
//vertical formatter don't need repeat line on phase0, 1: enable, 0: disable
//Bit 16    cvfmt_rpt_line0_en //RW,default=0;//vertical formatter repeat line 0 enable
//Bit 15:12 cvfmt_skip_line_num //RW, default=0;//vertical formatter skip line num at the beginning
//Bit 11:8  cvfmt_ini_phase     //RW, default=0;//vertical formatter initial phase
//Bit 7:1   cvfmt_phase_step    //RW, default=0;//vertical formatter phase step (3.4)
//Bit 0     cvfmt_en            //RW, default=0;//vertical formatter enable
#define T6W_DOLBY_INP_VD_CFMT_W                   0x4637
//Bit 31:29 reserved
//Bit 28:16 chfmt_w    //unsigned, RW, default = 0 ;horizontal formatter width
//Bit 15:13 reserved
//Bit 12:0  cvfmt_w    //unsigned, RW, default = 0 ;vertical formatter width
#define T6W_DOLBY_INP_VD_CFMT_H                   0x4638
//Bit 31        ram_arb_rst //unsigned, RW, default=0
//Bit 30        reg_win_sel //unsigned, RW, default=0;//0:use mif size 1:use reg
//Bit 29        reg_win_vsel//unsigned, RW, default=0;//0:use mif size 1:use reg
//Bit 28:13     reserved
//Bit 12:0      cfmt_h     //unsigned, RW, default = 0;//vertical formatter height
#define T6W_DOLBY_INP_WIN_LUMA_SIZE               0x4639
//Bit 31:30     reserved
//Bit 29        bypass_en_y            //unsigned, RW, default = 0 ;
//Bit 28:16     post_win_hsize         //unsigned, RW, default = 0 ;
//Bit 15:13     reserved
//Bit 12:0      post_win_vsize         //unsigned, RW, default = 0 ;
#define T6W_DOLBY_INP_WIN_LUMA_X                  0x463a
//Bit 31:29     reserved
//Bit 28:16     post_win_bgn_h         //unsigned, RW, default = 0 ;
//Bit 15:13     reserved
//Bit 12:0      post_win_end_h         //unsigned, RW, default = 0
#define T6W_DOLBY_INP_WIN_LUMA_Y                  0x463b
//Bit 31:29     reserved
//Bit 28:16     post_win_bgn_v         //unsigned, RW, default = 0 ;
//Bit 15:13     reserved
//Bit 12:0      post_win_end_v         //unsigned, RW, default = 0 ;
#define T6W_DOLBY_INP_WIN_CHRM_SIZE               0x463c
//Bit 31:30     reserved
//Bit 29        bypass_en_c            //unsigned, RW, default = 0 ;
//Bit 28:16     post_win_uv_hsize      //unsigned, RW, default = 0 ;
//Bit 15:13     reserved
//Bit 12:0      post_win_uv_vsize      //unsigned, RW, default = 0 ;
#define T6W_DOLBY_INP_WIN_CHRM_X                  0x463d
//Bit 31:29     reserved
//Bit 28:16     post_win_uv_bgn_h         //unsigned, RW, default = 0 ;
//Bit 15:13     reserved
//Bit 12:0      post_win_uv_end_h         //unsigned, RW, default = 0 ;
#define T6W_DOLBY_INP_WIN_CHRM_Y                  0x463e
//Bit 31:29     reserved
//Bit 28:16     post_win_uv_bgn_v         //unsigned, RW, default = 0 ;
//Bit 15:13     reserved
//Bit 12:0      post_win_uv_end_v         //unsigned, RW, default = 0 ;
#define T6W_DOLBY_INP_RMIF_MMU_CTRL               0x463f
//Bit 31:7      reserved
//Bit 6:2       reg_mmu_pre_num         //unsigned, RW, default = 4;
//Bit 1         reg_mmu_pg_mode         //unsigned, RW ,default = 0;
//Bit 0         reg_mmu_mode_en         //unsigned, RW, default = 0;

#define T6W_DOLBY_TOP2_PYRD_CTRL                       0x4640
//Bit 31    pls_py_sw_rst  //unsigned, W1T, default=0, sw rst for pyup as vsync trigger
//Bit 30    pls_mmu_sw_rst //unsigned, W1T, default=0, sw rst for pyramid rdmif mmu
//Bit 29    pls_dv_sw_rst  //unsigned, W1T, default=0, sw rst for dolby core ip
//Bit 28    pls_corep_pyup_sw_rst   //unsigned, W1T, default=0, sw rst for corep all
//Bit 27:26 pls_wrmif_sw_rst        //unsigned, W1T, default=0, sw rst for corep wrmif
//Bit 25:17 reserved
//Bit 16    reg_py_arsec        //unsigned, RW, default=0, security for pyramid rdmif
//Bit 15:12 reg_py_int_sel      //unsigned, RW, default=0, int_sel for pyup
//Bit 11:4  reg_gclk_ctrl       //unsigned, RW, default=0, clk gating ctrl
//Bit 3     reserved
//Bit 2     reg_py_int_tri
//unsigned, RW, default=1, last frame dolby done int to trigger next frame dolby pyramid read
//Bit 1:0   reg_py_level
//unsigned, RW, default=0, pymid level, src pic>512x288 ? 1 : 0, bit1=1 no pyramid
#define T6W_DOLBY_TOP2_PYRD_BADDR1                     0x4641
//Bit 31:0 reg_py_baddr1        //unsigned, RW, default=0, py1 baddr
#define T6W_DOLBY_TOP2_PYRD_BADDR2                     0x4642
//Bit 31:0 reg_py_baddr2        //unsigned, RW, default=0, py1 baddr
#define T6W_DOLBY_TOP2_PYRD_BADDR3                     0x4643
//Bit 31:0 reg_py_baddr3        //unsigned, RW, default=0, py1 baddr
#define T6W_DOLBY_TOP2_PYRD_BADDR4                     0x4644
//Bit 31:0 reg_py_baddr4        //unsigned, RW, default=0, py1 baddr
#define T6W_DOLBY_TOP2_PYRD_BADDR5                     0x4645
//Bit 31:0 reg_py_baddr5        //unsigned, RW, default=0, py1 baddr
#define T6W_DOLBY_TOP2_PYRD_BADDR6                     0x4646
//Bit 31:0 reg_py_baddr6        //unsigned, RW, default=0, py1 baddr
#define T6W_DOLBY_TOP2_PYRD_BADDR7                     0x4647
//Bit 31:0 reg_py_baddr7        //unsigned, RW, default=0, py1 baddr
#define T6W_DOLBY_TOP2_PYRD_STRIDE12                   0x4648
//Bit 31:29 reserved
//Bit 28:16 reg_py_stride2      //unsigned, RW, default=512
//Bit 15:13 reserved
//Bit 12:0  reg_py_stride1      //unsigned, RW, default=1024
#define T6W_DOLBY_TOP2_PYRD_STRIDE34                   0x4649
//Bit 31:29 reserved
//Bit 28:16 reg_py_stride4      //unsigned, RW, default=128
//Bit 15:13 reserved
//Bit 12:0  reg_py_stride3      //unsigned, RW, default=256
#define T6W_DOLBY_TOP2_PYRD_STRIDE56                   0x464a
//Bit 31:29 reserved
//Bit 28:16 reg_py_stride6      //unsigned, RW, default=32
//Bit 15:13 reserved
//Bit 12:0  reg_py_stride5      //unsigned, RW, default=64
#define T6W_DOLBY_TOP2_PYRD_STRIDE7                    0x464b
//Bit 31:15 reg_py_urgent       //unsigned, RW, default=0, pyramid rdmif urgent
//Bit 14:13 reserved
//Bit 12:0  reg_py_stride7      //unsigned, RW, default=16
#define T6W_DOLBY_TOP2_PYUP_CTRL0                      0x464c
//Bit 31    reg_pyup_byps             //unsigned, RW, default=0,internal bypass
//Bit 30:25 reserved
//Bit 24    reg_latch_en              //unsigned, RW, default=0
//Bit 23    reg_pyup_hs_mask_en       //unsigned, RW, default=1
//Bit 22:20 reg_int_pyid_sel          //unsigned, RW, default=0
//Bit 19:8  reg_int_line_num          //unsigned, RW, default=0
//Bit 7:4   reserved
//Bit 3:2   reserved
//Bit 1     reg_corep_md_pgm_over     //unsigned, RW, default=0
//Bit 0     reg_corep_cfg_pyramid_en  //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_PYUP_CTRL1                      0x464d
//Bit 31:26 reserved
//Bit 25:24 reg_din_sel
//unsigned,RW, default=1,use core1_done 1=reg_frm_rst 2=vsync as frm rst,0/3=idle
//Bit 23    reg_hs_sel   //unsigned, RW, default=0, 0=hsync, 1=hold hend as hsync
//Bit 22:13 reserved
//Bit 12:0  reg_stdly_num  //unsigned, RW, default=2, hold line cnt after frm rst to generate frm_en
#define T6W_DOLBY_TOP2_PYUP_CTRL2                      0x464e
//Bit 31:29 reserved
//Bit 28:16 reg_hold_vnum       //unsigned, RW, default=4, hold vnum
//Bit 15:13 reserved
//Bit 12:0  reg_hold_hnum       //unsigned, RW, default=2100, hold hnum
#define T6W_DOLBY_TOP2_PYR1_WGT                        0x464f
//Bit 31:16 reg_corep_pyr1_weight1    //unsigned, RW, default=0
//Bit 15: 0 reg_corep_pyr1_weight0    //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_PYR2_WGT                        0x4650
//Bit 31:16 reg_corep_pyr2_weight1    //unsigned, RW, default=0
//Bit 15: 0 reg_corep_pyr2_weight0    //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_PYR3_WGT                        0x4651
//Bit 31:16 reg_corep_pyr3_weight1    //unsigned, RW, default=0
//Bit 15: 0 reg_corep_pyr3_weight0    //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_PYR4_WGT                        0x4652
//Bit 31:16 reg_corep_pyr4_weight1    //unsigned, RW, default=0
//Bit 15: 0 reg_corep_pyr4_weight0    //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_PYR5_WGT                        0x4653
//Bit 31:16 reg_corep_pyr5_weight1    //unsigned, RW, default=0
//Bit 15: 0 reg_corep_pyr5_weight0    //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_PYR6_WGT                        0x4654
//Bit 31:16 reg_corep_pyr6_weight1    //unsigned, RW, default=0
//Bit 15: 0 reg_corep_pyr6_weight0    //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_PYR7_WGT                        0x4655
//Bit 31:16 reg_corep_pyr7_weight1    //unsigned, RW, default=0
//Bit 15: 0 reg_corep_pyr7_weight0    //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_PYRES                           0x4656
//Bit 31:28 reserved
//Bit 27:16 reg_corep_pyramid_yres    //unsigned, RW, default=576
//Bit 15:12 reserved
//Bit 11: 0 reg_corep_pyramid_xres    //unsigned, RW, default=1024
#define T6W_DOLBY_TOP2_VDRES                           0x4657
//Bit 31:16 reg_corep_vdr_yres        //unsigned, RW, default=1080
//Bit 15:0  reg_corep_vdr_xres        //unsigned, RW, default=1920
#define T6W_DOLBY_TOP2_PYUP_RO                         0x4658
//Bit 31:16 reserved
//Bit 15:12 ro_corep_pyramid_in_progress //unsigned, RO, default=0
//Bit 11:9  reserved
//Bit 8     ro_corep_pyramid_read_done //unsigned, RO, default=0
//Bit 7:0   ro_corep_rdseq_state       //unsigned, RO, default=0
#define T6W_DOLBY_TOP2_PYUP_WMIF_CTRL1                 0x4659
//Bit 31:26     reserved
//Bit 25:24     reg_sync_sel            //unsigned, RW, default=0
//Bit 23:16     reg_canvas_id           //unsigned, RW, default=0
//Bit 15        reserved
//Bit 14:12     reg_cmd_intr_len        //unsigned, RW, default=1
//Bit 11:10     reg_cmd_req_size        //unsigned, RW, default=1
//Bit 9:8       reg_burst_len           //unsigned, RW, default=2
//Bit 7         reg_swap_64bit          //unsigned, RW, default=0
//Bit 6         reg_little_endian       //unsigned, RW, default=1
//Bit 5         reg_y_rev               //unsigned, RW, default=0
//Bit 4         reg_x_rev               //unsigned, RW, default=0
//Bit 3         reserved
//Bit 2:0       reg_pack_mode           //unsigned, RW, default=1
#define T6W_DOLBY_TOP2_PYUP_WMIF_CTRL2                 0x465a
//Bit 31:30     reserved
//Bit 29:26     reg_wrmif_reg_mode    //unsigned, RW, default=0
//Bit 25:22     reserved
//Bit 21:20     reg_int_clr           //unsigned, RW, default=0
//Bit 19:18     reg_gclk_ctrl         //unsigned, RW, default=0
//Bit 17        reserved
//Bit 16:0      reg_urgent_ctrl       //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_PYUP_WMIF_CTRL3                 0x465b
//Bit 31        reserved
//Bit 30        reg_hold_en          //unsigned, RW, default=0
//Bit 29:24     reg_pass_num         //unsigned, RW, default=1
//Bit 23:18     reg_hold_num         //unsigned, RW, default=0
//Bit 17        reserved
//Bit 16        reg_acc_mode         //unsigned, RW, default=1
//Bit 15:13     reserved
//Bit 12:0      reg_stride           //unsigned, RW, default=4096
#define T6W_DOLBY_TOP2_PYUP_WMIF_CTRL4                 0x465c
//Bit 31:0      reg_baddr           //unsigned, RW, default=0

#define T6W_DOLBY_TOP2_PYUP_WMIF_RO_STAT               0x465d
//Bit 31:16     reserved
//Bit 15:0      reg_status           //unsigned, RO, default=0
#define T6W_DOLBY_TOP2_PYUP_RO_0                       0x4660
//Bit 31:0  ro_dbg0         //unsigned, RO, default=0
#define T6W_DOLBY_TOP2_PYUP_RO_1                       0x4661
//Bit 31:0  ro_dbg1         //unsigned, RO, default=0
#define T6W_DOLBY_TOP2_PYUP_RO_2                       0x4662
//Bit 31:0  ro_dbg2         //unsigned, RO, default=0
#define T6W_DOLBY_TOP2_PYUP_RO_3                       0x4663
//Bit 31:0  ro_dbg3         //unsigned, RO, default=0
#define T6W_DOLBY_TOP2_PYUP_RO_4                       0x4664
//Bit 31:0  ro_dbg4         //unsigned, RO, default=0
#define T6W_DOLBY_TOP2_PYRMIF_MMU_ADDR                 0x4665
//Bit 31:0 reg_pyrmif_mmu_addr  //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_PYRMIF_MMU_DATA                 0x4666
//Bit 31:0 reg_pyrmif_mmu_data  //unsigned, RW, default=0
// synopsys translate_off
// synopsys translate_on
//
// Closing file:  ./dolby5_inc/dolby_pyup_reg.h
//
//===========================================================================
// -----------------------------------------------
// REG_BASE:  DV_TOP1_CORE_VCBUS_BASE = 0x47
// -----------------------------------------------
//===========================================================================
//===========================================================================
// -----------------------------------------------
// REG_BASE:  DV_TOP2_TOP_VCBUS_BASE = 0x48
// -----------------------------------------------
//===========================================================================
//
// Reading file:  ./dolby5_inc/dolby_top2_reg.h
//
// synopsys translate_off
// synopsys translate_on
//
#define T6W_DOLBY_TOP2_PIC_SIZE                        0x4800
//Bit 31:16 reg_pic_vsize       //unsigned, RW, default=1080, dos src pic vsize
//Bit 15:0  reserved
#define T6W_DOLBY_TOP2_PIC_HSIZE_S0                    0x4801
//Bit 31:16 reg_pic_hsize_s1    //unsigned, RW, default=1920, dos src pic hsize_s1
//Bit 15:0  reg_pic_hsize_s0    //unsigned, RW, default=1920, dos src pic hsize_s0
#define T6W_DOLBY_TOP2_PIC_HSIZE_S1                    0x4802
//Bit 31:16 reg_pic_hsize_s3    //unsigned, RW, default=1920, dos src pic hsize_s3
//Bit 15:0  reg_pic_hsize_s2    //unsigned, RW, default=1920, dos src pic hsize_s2
#define T6W_DOLBY_TOP2_RDMA_CTRL                       0x4803
//Bit 31    reg_rdma_sw_rst   //unsigned, W1T, default=0, sw rst for rdma
//Bit 30    reserved
//Bit 29    reg_rdma_shdw_rst //unsigned, W1T, default=0, used in rdma shadow, to config 2nd time
//Bit 28    reg_rdma_shdw_en  //unsigned, RW, default=0, shadow_en, used for change rdma_num/size
//Bit 27:14 reserved
//Bit 13:12 reg_rdma_on       //unsigned, RW, default=3, top2_rdma_on
//Bit 11:8  reg_rdma1_num     //unsigned, RW, default=1, rdma1 lut num total 10
//Bit 7:2   reserved
//Bit 1:0   reg_rdma0_num        //unsigned, RW, default=1, rdma0 lut num total 2
#define T6W_DOLBY_TOP2_RDMA_SIZE0                      0x4804
//Bit 31:16 reg_rdma_size0      //unsigned, RW, default=166, rdma lut0 size
//Bit 15:0  reg_rdma_size1      //unsigned, RW, default=258, rdma lut1 size
#define T6W_DOLBY_TOP2_RDMA_SIZE1                      0x4805
//Bit 31:16 reg_rdma_size2      //unsigned, RW, default=1032, rdma lut2 size
//Bit 15:0  reg_rdma_size3      //unsigned, RW, default=128, rdma lut3 size
#define T6W_DOLBY_TOP2_RDMA_SIZE2                      0x4806
//Bit 31:16 reg_rdma_size4      //unsigned, RW, default=128, rdma lut4 size
//Bit 15:0  reg_rdma_size5      //unsigned, RW, default=128, rdma lut5 size
#define T6W_DOLBY_TOP2_RDMA_SIZE3                      0x4807
//Bit 31:16 reg_rdma_size6      //unsigned, RW, default=128, rdma lut6 size
//Bit 15:0  reg_rdma_size7      //unsigned, RW, default=128, rdma lut7 size
#define T6W_DOLBY_TOP2_RDMA_SIZE4                      0x4808
//Bit 31:16 reg_rdma_size8      //unsigned, RW, default=128, rdma lut8 size
//Bit 15:0  reg_rdma_size9      //unsigned, RW, default=128, rdma lut9 size
#define T6W_DOLBY_TOP2_RDMA_SIZE5                      0x4809
//Bit 31:16 reg_rdma_size10      //unsigned, RW, default=128, rdma lut10 size
//Bit 15:0  reg_rdma_size11      //unsigned, RW, default=128, rdma lut11 size
#define T6W_DOLBY_TOP2_CTRL0                           0x480a
//Bit 31    reg_sw_reset //unsigned, W1T,default=0,sw rst for whole module,used as rst_n
//Bit 30    reg_frm_rst      //unsigned, W1T, default=0, sw-triggered frm_rst
//Bit 29    reg_core2_byps   //unsigned, RW, default=0, internal bypass
//Bit 28    reg_eol_sel      //unsigned, RW, default=0
//Bit 27:24 reg_top2_gclk_ctrl  //unsigned, RW, default=0
//Bit 23:20 reg_int_sel//unsigned, RW, default=0, 1=close,0=open,bit0: err_int(din_hend),
//bit1: err_int(rdma end), bit2:int(rdma_done), bit3:int(core2_done)
//Bit 19:8  reg_int_line_num
//unsigned, RW, default=0, reg_int_sel=0, err int trigger when din_hcnt=line_num
//Bit 7:6   reserved
//Bit 5     reg_pyrd_en          //unsigned, RW, default=1
//Bit 4     reg_rdmif_arsec      //unsigned, RW, default=0, pyrdmif arsec
//Bit 3:1   reserved
//Bit 0     reg_latch_en         //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_CTRL1                           0x480b
//Bit 31:26 reserved
//Bit 25:24 reg_din_sel  //unsigned,RW, default=2, 1=reg_frm_rst 2=vsync as frm rst, 0/3=idle
//Bit 23    reg_hs_sel   //unsigned,RW, default=0, 0=hsync, 1=hold hend as hsync
//Bit 22:13 reserved
//Bit 12:0  reg_stdly_num
//unsigned,RW,default=2,hold line cnt after frm rst to generate frm_en
#define T6W_DOLBY_TOP2_CTRL2                           0x480c
//Bit 31:29 reserved
//Bit 28:16 reg_hold_vnum       //unsigned, RW, default=4, hold vnum
//Bit 15:13 reserved
//Bit 12:0  reg_hold_hnum       //unsigned, RW, default=2100, hold hnum
#define T6W_DOLBY_TOP2_CTRL3                           0x480d
//Bit 31    reg_slc_hold_en     //unsigned, RW, default=1, hold enable for slice case
//Bit 30:29 reserved
//Bit 28:16 reg_slc_hold_vnum   //unsigned, RW, default=4, hold vnum
//Bit 15:13 reserved
//Bit 12:0  reg_slc_hold_hnum   //unsigned, RW, default=12, hold hnum
#define T6W_DOLBY_TOP2_RD_PYRES                        0x480e
//Bit 31:28 reserved
//Bit 27:16 reg_corep_pyramid_yres    //unsigned, RW, default=576
//Bit 15:0  reserved
#define T6W_DOLBY_TOP2_PYRES_SLC_0                     0x480f
//Bit 31:28 reserved
//Bit 27:16 reg_corep_pyramid_xres_s1   //unsigned, RW, default=1024
//Bit 15:12 reserved
//Bit 11:0  reg_corep_pyramid_xres_s0   //unsigned, RW, default=1024
#define T6W_DOLBY_TOP2_PYRES_SLC_1                     0x4810
//Bit 31:28 reserved
//Bit 27:16 reg_corep_pyramid_xres_s3   //unsigned, RW, default=1024
//Bit 15:12 reserved
//Bit 11:0  reg_corep_pyramid_xres_s2   //unsigned, RW, default=1024
#define T6W_DOLBY_TOP2_PYUP_RMIF_CTRL1                 0x4811
//Bit 31:28     reg_pix_bits             //unsigned, RW, default=2
//Bit 27:26     reserved
//Bit 25:24     reg_sync_sel             //unsigned, RW, default=0
//Bit 23:16     reg_canvas_id            //unsigned, RW, default=0
//Bit 15        reserved
//Bit 14:12     reg_cmd_intr_len         //unsigned, RW, default=1
//Bit 11:10     reg_cmd_req_size         //unsigned, RW, default=1
//Bit 9:8       reg_burst_len            //unsigned, RW, default=2
//Bit 7         reg_swap_64bit           //unsigned, RW, default=0
//Bit 6         reg_little_endian        //unsigned, RW, default=1
//Bit 5         reg_y_rev                //unsigned, RW, default=0
//Bit 4         reg_x_rev                //unsigned, RW, default=0
//Bit 3         reserved
//Bit 2:0       reg_pack_mode            //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_PYUP_RMIF_CTRL2                 0x4812
//Bit 31:30       reg_sw_rst            //unsigned, RW, default=0
//Bit 29:26       reg_rdmif_reg_mode    //unsigned, RW, default=0
//Bit 25:22       reg_vstep             //unsigned, RW, default=1
//Bit 21:20       reg_int_clr           //unsigned, RW, default=0
//Bit 19:18       reg_gclk_ctrl         //unsigned, RW, default=0
//Bit 17          reg_rpt_line          //unsigned, RW, default=0
//Bit 16:0        reg_urgent_ctrl       //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_PYUP_RMIF_CTRL3                 0x4813
//Bit 31          reserved
//Bit 30          reg_hold_en          //unsigned, RW, default=0
//Bit 29:24       reg_pass_num         //unsigned, RW, default=1
//Bit 23:18       reg_hold_num         //unsigned, RW, default=0
//Bit 17          reserved
//Bit 16          reg_acc_mode         //unsigned, RW, default=1
//Bit 15:14       reg_block_mode       //unsigned, RW, default=0
//Bit 13          reg_32b_align        //unsigned, RW, default=0
//Bit 12:0        reg_stride           //unsigned, RW, default=4096
#define T6W_DOLBY_TOP2_PYUP_RMIF_CTRL4                 0x4814
//Bit 31:0        reg_baddr           //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_PYUP_RMIF_RO_STAT               0x4815
//Bit 31:16       reserved
//Bit 15:0        ro_status             //unsigned, RO, default=0
#define T6W_DOLBY_TOP2_PYUP_SLC_HSTART_0               0x4816
//Bit 31:29       reserved
//Bit 28:16       reg_slice_hstart_s1  //unsigned, RW, default=0
//Bit 15:13       reserved
//Bit 12:0        reg_slice_hstart_s0  //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_PYUP_SLC_HSTART_1               0x4817
//Bit 31:29       reserved
//Bit 28:16       reg_slice_hstart_s3  //unsigned, RW, default=0
//Bit 15:13       reserved
//Bit 12:0        reg_slice_hstart_s2  //unsigned, RW, default=0
#define T6W_DOLBY_TOP2_PYUP_SLC_HEND_0                 0x4818
//Bit 31:29       reserved
//Bit 28:16       reg_slice_hend_s1    //unsigned, RW, default=1024
//Bit 15:13       reserved
//Bit 12:0        reg_slice_hend_s0    //unsigned, RW, default=1024
#define T6W_DOLBY_TOP2_PYUP_SLC_HEND_1                 0x4819
//Bit 31:29       reserved
//Bit 28:16       reg_slice_hend_s3    //unsigned, RW, default=1024
//Bit 15:13       reserved
//Bit 12:0        reg_slice_hend_s2    //unsigned, RW, default=1024
#define T6W_DOLBY_TOP2_LDMA_TRIG_CTRL                  0x481a
//Bit 31            pls_ldma_trig       //unsigned,  W1T, default = 0
//Bit 30:24         reserved
//Bit 23:16         ro_ldma_trig_status //unsigned ,  RO,  default = 0
//Bit 15:8          reg_ldma_trig_clr   //unsigned ,  RW,  default = 0
//Bit 7:0           reg_ldma_trig_en    //unsigned ,  RW,  default = 0
#define T6W_DOLBY_TOP2_RDMA_TRIG_CTRL                  0x481b
//Bit 31            pls_rdma_trig       //unsigned , W1T, default = 0
//Bit 30:24         reserved
//Bit 23:16         ro_rdma_trig_status //unsigned ,  RO,  default = 0
//Bit 15:8          reg_rdma_trig_clr   //unsigned ,  RW,  default = 0
//Bit 7:0           reg_rdma_trig_en    //unsigned ,  RW,  default = 0
#define T6W_DOLBY_TOP2_BYPS_MIF_ADDR                   0x481c
//Bit 31:0          reg_byps_baddr      //unsigned ,  RW,  default = 0
#define T6W_DOLBY_TOP2_RO_0                            0x4820
//Bit 31:0  ro_dbg0         //unsigned, RO, default=0
#define T6W_DOLBY_TOP2_RO_1                            0x4821
//Bit 31:0  ro_dbg1         //unsigned, RO, default=0
#define T6W_DOLBY_TOP2_RO_2                            0x4822
//Bit 31:0  ro_dbg2         //unsigned, RO, default=0
#define T6W_DOLBY_TOP2_RO_3                            0x4823
//Bit 31:0  ro_dbg3         //unsigned, RO, default=0

/***************DV TOP2 REGS END **********************/

#define T6W_DOLBY5_CORE1_REG_BASE       0x4700
#define T6W_DOLBY5_CORE1B_REG_BASE      0x47d1
#define T6W_DOLBY5_CORE2_REG_BASE0      0x4900
#define T6W_DOLBY5_CORE2_REG_BASE1      0x4a00
#define T6W_DOLBY5_CORE2_REG_BASE2      0x4b00

#define T6W_DOLBY5_CORE1_CRC_CNTRL      0x47c7 /*top1 CRC control*/
//Bit 1      crc_rst      //RW, default = 0
//Bit 0      CRC enable   //RW, default = 0
#define T6W_DOLBY5_CORE1_CRC_IN_FRM     0x47c8 /*Input frame CRC*/
#define T6W_DOLBY5_CORE1_CRC_OUT_COMP   0x47c9 /*Composer output frame CRC*/
#define T6W_DOLBY5_CORE1_CRC_OUT_FRM    0x47ca /*Output frame CRC*/
#define T6W_DOLBY5_CORE1_CRC_ICSC       0x47cb /*Input CSC LUT CRC*/
#define T6W_DOLBY5_CORE1_L1_MINMAX      0x47cc
#define T6W_DOLBY5_CORE1_L1_MID_L4      0x47cd

#define T6W_DOLBY5_CORE1_ICSCLUT_DBG_RD 0x47ce/*addr for lut*/
#define T6W_DOLBY5_CORE1_ICSCLUT_RDADDR 0x47cf/*addr for lut*/
#define T6W_DOLBY5_CORE1_ICSCLUT_RDDATA 0x47d0/*addr for lut*/

#define T6W_DOLBY5_CORE1B_CRC_CNTRL     0x47da /*top1b CRC control*/
//Bit 1      crc_rst      //RW, default = 0
//Bit 0      CRC enable   //RW, default = 0
#define T6W_DOLBY5_CORE1B_CRC_IN_FRM    0x47db /*Input frame CRC*/

#define T6W_DOLBY5_CORE2_INP_FRM_ST     0x4b39/*input frame v-count and h-count*/
#define T6W_DOLBY5_CORE2_OP_FRM_ST      0x4b3a/*output frame v-count and h-count*/
#define T6W_DOLBY5_CORE2_L1_MINMAX      0x4b3b/*bit0-15 l1_min, bit16-31 l1_max*/
#define T6W_DOLBY5_CORE2_L1_MID_L4_STD  0x4b3c/*bit0-15 l1_mid, bit16-31 l4_std*/

#define T6W_DOLBY5_CORE2_CRC_CNTRL      0x4b3d /*top2 CRC control*/
#define T6W_DOLBY5_CORE2_CRC_IN_FRM     0x4b3e /*Input frame CRC*/
#define T6W_DOLBY5_CORE2_CRC_OUT_COMP   0x4b3f /*Composer output frame CRC*/
#define T6W_DOLBY5_CORE2_CRC_OUT_FRM    0x4b40 /*Output frame CRC*/
#define T6W_DOLBY5_CORE2_CRC_UPSCL_OUT_FRM    0x4b41 /*Upscaler Output frame CRC*/
#define T6W_DOLBY5_CORE2_CRC_ICSC             0x4b42 /*iCSC LUT CRC*/
#define T6W_DOLBY5_CORE2_CRC_OCSC             0x4b43 /*OCSC LUT CRC*/
#define T6W_DOLBY5_CORE2_CRC_CVM_TAILUT       0x4b44 /*CVM TAI LUT CRC*/
#define T6W_DOLBY5_CORE2_CRC_CVM_SMILUT       0x4b45 /*CVM SMI LUT CRC*/
#define T6W_DOLBY5_CORE2_CRC_CVM_LITE_TAILUT  0x4b46 /*CVM Lite TAI LUT CRC*/
#define T6W_DOLBY5_CORE2_CRC_CVM_LITE_SMILUT  0x4b47 /*CVM Lite SMI LUT CRC*/
#define T6W_DOLBY5_CORE2_CRC_CVM_LITE_TMSLUT  0x4b48 /*CVM Lite TMS LUT CRC*/
#define T6W_DOLBY5_CORE2_CRC_CVM_LITE_SMSLUT  0x4b49 /*CVM Lite SMS LUT CRC*/
#define T6W_DOLBY5_CORE2_CRC_CVM_LITE_TMI2LUT 0x4b4a /*CVM Lite TMI2 LUT CRC*/
#define T6W_DOLBY5_CORE2_CRC_CVM_LITE_SMI2LUT 0x4b4b /*CVM Lite SMI2 LUT CRC*/
#define T6W_DOLBY5_CORE2_CRC_CVM_LITE_TMS2LUT 0x4b4c /*CVM Lite TMS2 LUT CRC*/
#define T6W_DOLBY5_CORE2_CRC_CVM_LITE_SMS2LUT 0x4b4d /*CVM Lite SMS2 LUT CRC*/

#define T6W_DOLBY5_CORE2_DEBUG_LUT_CNTRL      0x4b4e/*lut debug control*/
#define T6W_DOLBY5_CORE2_CVM_TAILUT_RDADDR    0x4b4f/*addr for lut*/
#define T6W_DOLBY5_CORE2_CVM_SMILUT_RDADDR    0x4b50/*addr for lut*/
#define T6W_DOLBY5_CORE2_ICSCLUT_RDADDR       0x4b51/*addr for lut*/
#define T6W_DOLBY5_CORE2_OCSCLUT_RDADDR       0x4b52/*addr for lut*/
#define T6W_DOLBY5_CORE2_CVM_TAILUT_RDDATA    0x4b5b/*data for lut*/
#define T6W_DOLBY5_CORE2_CVM_SMILUT_RDDATA    0x4b5c/*data for lut*/
#define T6W_DOLBY5_CORE2_ICSCLUT_RDDATA       0x4b5d/*data for lut*/
#define T6W_DOLBY5_CORE2_OCSCLUT_RDDATA       0x4b5e/*data for lut*/

#define T6W_VPU_AXIRD_PATH_CTRL               0x6d02

#define VPU_LUT_DMA_INTR_SEL  0x2717
#define VPU_DMA_RDMIF3_CTRL   0x2753
#define VPU_DMA_RDMIF4_CTRL   0x2754
#define VPU_DMA_RDMIF3_BADR0  0x2764
#define VPU_DMA_RDMIF3_BADR1  0x2765
#define VPU_DMA_RDMIF3_BADR2  0x2766
#define VPU_DMA_RDMIF3_BADR3  0x2767
#define VPU_DMA_RDMIF3_BADR3  0x2767
#define VPU_DMA_RDMIF4_BADR0  0x2768
#define VPU_DMA_RDMIF4_BADR1  0x2769
#define VPU_DMA_RDMIF4_BADR2  0x276a
#define VPU_DMA_RDMIF4_BADR3  0x276b
#define VPU_DMA_RDMIF_SEL     0x2778

#define VIU_FRM_CTRL         0x1a51
#define VPU_RDARB_MODE_L2C1  0x279d

#endif
