/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __VDIN_T6X_REGS_H
#define __VDIN_T6X_REGS_H

#include "vdin_regs_t3x.h"

#define VDIN0_WRMIF_REG_OFFSET_T6X          0x40
#define VDIN0_AA_DS_REG_OFFSET_T6X          0x40

/* t6x vdin_preproc begin */
#define VPU_VDIN_HDMI0_CTRL0_T6X            0x2738
#define VPU_VDIN_HDMI0_CTRL1_T6X            0x274c
#define VPU_VDIN_HDMI1_CTRL0_T6X            0x274d
#define VPU_VDIN_HDMI1_CTRL1_T6X            0x274e
#define VPU_VDIN_HDMI0_TUNNEL_T6X           0x274f
/* t6x vdin_preproc end */

#define VDIN_DPSS_CTRL_MISC                 0x1222

/* t6x hist begin */
#define VDIN_HIST_CTRL_T6X                  0x1209
#define VDIN_HIST_H_START_END_T6X           0x1292
#define VDIN_HIST_V_START_END_T6X           0x1293
#define VDIN_HIST_SPL_VAL_T6X               0x1297
#define VDIN_HIST_CHROMA_SUM_T6X            0x1299
/* t6x hist end */

#define VDIN0_META_REG_OFFSET	(VDIN0_META_DSC_CTRL0_T6X - VDIN0_META_DSC_CTRL0) //0x4180
#define VDIN0_META_DSC_CTRL0_T6X                    0x43f0
//Bit 31           reg_meta_dolby_check_en          // unsigned ,    RW, default = 1
//Bit 30           reg_meta_tunnel_swap_en          // unsigned ,    RW, default = 0
//Bit 29:24        reg_meta_soft_rst                // unsigned ,    RW, default = 0
//Bit 23:18        reserved
//Bit 17           reg_meta_frame_rst               // unsigned ,    RW, default = 1
//Bit 16           reg_meta_lsb                     // unsigned ,    RW, default = 1
//Bit 15: 0        reg_meta_data_bytes              // unsigned ,    RW, default = 128
#define VDIN0_META_DSC_CTRL1_T6X                    0x43f1
//Bit 31:16        reg_meta_data_start              // unsigned ,    RW, default = 0
//Bit 15: 8        reserved
//Bit  7: 0        reg_meta_crc_ctrl                // unsigned ,    RW, default = 3
#define VDIN0_META_DSC_CTRL2_T6X                    0x43f2
//Bit 31           reserved
//Bit 30           reg_meta_rd_mode                 // unsigned ,    RW, default = 0
//Bit 29:20        reg_meta_wr_sum                  // unsigned ,    RW, default = 280
//Bit 19:18        reserved
//Bit 17: 0        reg_meta_tunnel_sel              // unsigned ,    RW, default = 18'h2c2d0
#define VDIN0_META_DSC_CTRL3_T6X                    0x43f3
//Bit 31:16        reg_meta_start_addr              // unsigned ,    RW, default = 0
//Bit 15: 0        reg_meta_byte_select             // unsigned ,    RW, default = 0
#define VDIN0_META_AXI_CTRL0_T6X                    0x43f4
//Bit 31: 0        reg_meta_axi_ctrl0               // unsigned ,    RW, default = 32'h97181000
#define VDIN0_META_AXI_CTRL1_T6X                    0x43f5
//Bit 31: 0        reg_meta_axi_ctrl1               // unsigned ,    RW, default = 32'h8000000
#define VDIN0_META_AXI_CTRL2_T6X                    0x43f6
//Bit 31: 0        reg_meta_axi_ctrl2               // unsigned ,    RW, default = 32'h10001000
#define VDIN0_META_AXI_CTRL3_T6X                    0x43f7
//Bit 31: 0        reg_meta_axi_ctrl3               // unsigned ,    RW, default = 32'h10
#define VDIN0_META_DSC_STATUS0_T6X                  0x43f8
//Bit 31: 8        reg_meta_axi_status0             // unsigned ,    RO, default = 0
//Bit  7: 0        reg_meta_byte                    // unsigned ,    RO, default = 0
#define VDIN0_META_DSC_STATUS1_T6X                  0x43f9
//Bit 31:0         reg_meta_sram                    // unsigned ,    RO, default = 0
#define VDIN0_META_DSC_STATUS2_T6X                  0x43fa
//Bit 31:28        reg_meta_axi_status1             // unsigned ,    RO, default = 0
//Bit 27           reg_meta_valid                   // unsigned ,    RO, default = 0
//Bit 26:21        reserved
//Bit 20:19        reg_meta_crc_status              // unsigned ,    RO, default = 0
//Bit 18:11        reg_meta_crc_count               // unsigned ,    RO, default = 0
//Bit 10           reg_meta_crc_pass                // unsigned ,    RO, default = 0
//Bit  9: 0        reg_meta_addr                    // unsigned ,    RO, default = 0
#define VDIN0_META_DSC_STATUS3_T6X                  0x43fb
//Bit 31: 0        reg_meta_axi_status2             // unsigned ,    RO, default = 0

#endif /* __VDIN_T6X_REGS_H */
