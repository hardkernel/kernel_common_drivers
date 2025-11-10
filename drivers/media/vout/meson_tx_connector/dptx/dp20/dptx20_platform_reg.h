/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPTX_PLATFORM_REG_H__
#define __DPTX_PLATFORM_REG_H__

#define BASE_REG_OFFSET		24
#define ANACTRL_REG_ADDR(reg) \
	((ANACTRL_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))
#define PWRCTRL_REG_ADDR(reg) \
	((PWRCTRL_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))
#define RESETCTRL_REG_ADDR(reg) \
	((RESETCTRL_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))
#define SYSCTRL_REG_ADDR(reg) \
	((SYSCTRL_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))
#define CLKCTRL_REG_ADDR(reg) \
	((CLKCTRL_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))
#define VPUCTRL_REG_ADDR(reg) \
	((VPUCTRL_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))
#define PADCTRL_REG_ADDR(reg) \
	((PADCTRL_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))

#define PWRCTRL_MEM_PD13             PWRCTRL_REG_ADDR(0x001d)
#define PWRCTRL_MEM_PD21             PWRCTRL_REG_ADDR(0x0025)

/* CLK_CTRL */
#define CLKCTRL_VID_CLK_CTRL         CLKCTRL_REG_ADDR(0x0030)
#define CLKCTRL_VID_CLK_CTRL2        CLKCTRL_REG_ADDR(0x0031)
#define CLKCTRL_VID_CLK_DIV          CLKCTRL_REG_ADDR(0x0032)
#define CLKCTRL_VIID_CLK_DIV         CLKCTRL_REG_ADDR(0x0033)
#define CLKCTRL_VIID_CLK_CTRL        CLKCTRL_REG_ADDR(0x0034)
#define CLKCTRL_HDMI_CLK_CTRL        CLKCTRL_REG_ADDR(0x0038)
#define CLKCTRL_DPTX_CLK_CTRL        CLKCTRL_REG_ADDR(0x00de)

/* VPU_CTRL */
#define VPU_VIU_VENC_MUX_CTRL         VPUCTRL_REG_ADDR(0x271a)

//Bit 31    lvds_out_enable[0]
//Bit 30    vbo_out_enable[0]
//Bit 29    hdmi_tx_enable[0]
//Bit 28    dsi_edp_enable[0]
//Bit 27:7  reserved
//Bit 6     viu0_disable_rst_afifo
//Bit 5:4   viu0_vs_hs_ctrl
//Bit 3:2   viu0_force_field_ctrl
//Bit 1     viu0_force_go_line
//Bit 0     viu0_force_go_field
#define VPU_DISP_VIU0_CTRL VPUCTRL_REG_ADDR(0x2786)
//Bit 31    lvds_out_enable[1]
//Bit 30    vbo_out_enable[1]
//Bit 29    hdmi_tx_enable[1]
//Bit 28    dsi_edp_enable[1]
//Bit 27:7  reserved
//Bit 6     viu1_disable_rst_afifo
//Bit 5:4   viu1_vs_hs_ctrl
//Bit 3:2   viu1_force_field_ctrl
//Bit 1     viu1_force_go_line
//Bit 0     viu1_force_go_field
#define VPU_DISP_VIU1_CTRL VPUCTRL_REG_ADDR(0x2787)

/* bit0: 0: disp_if_unit0 data from venc0
 * bit0: 1: disp_if_unit0 data from venc0->split0 decoup
 * bit1: 0: disp_if_unit1 data from venc1
 * bit1: 1: disp_if_unit1 data from venc0->split1 decoup
 */
#define VPU_DISP_WRAP_CTRL VPUCTRL_REG_ADDR(0x278b)

/* bit[1:0], 2'b00 mux to disp_if_unit0->edp0
 * 2'b11 mux to disp_if_unit1->edp1
 */
#define VPU_DSI_EDP_INFO VPUCTRL_REG_ADDR(0x27fe)

/*
 * there are two VPU_HDMI_IF modules, so there are also
 * two groups of below registers
 */
#define VPU_HDMI_SETTING              VPUCTRL_REG_ADDR(0x2800)
#define VPU_HDMI_FMT_CTRL             VPUCTRL_REG_ADDR(0x2802)
#define VPU_HDMI_DITH_CNTL            VPUCTRL_REG_ADDR(0x281c)

#define VPU_HDMI_MATRIX_COEF00_01     VPUCTRL_REG_ADDR(0x2820)
#define VPU_HDMI_MATRIX_COEF02_10     VPUCTRL_REG_ADDR(0x2821)
#define VPU_HDMI_MATRIX_COEF11_12     VPUCTRL_REG_ADDR(0x2822)
#define VPU_HDMI_MATRIX_COEF20_21     VPUCTRL_REG_ADDR(0x2823)
#define VPU_HDMI_MATRIX_COEF22        VPUCTRL_REG_ADDR(0x2824)
#define VPU_HDMI_MATRIX_COEF13_14     VPUCTRL_REG_ADDR(0x2825)
#define VPU_HDMI_MATRIX_COEF23_24     VPUCTRL_REG_ADDR(0x2826)
#define VPU_HDMI_MATRIX_COEF15_25     VPUCTRL_REG_ADDR(0x2827)
#define VPU_HDMI_MATRIX_CLIP          VPUCTRL_REG_ADDR(0x2828)
#define VPU_HDMI_MATRIX_OFFSET0_1     VPUCTRL_REG_ADDR(0x2829)
#define VPU_HDMI_MATRIX_OFFSET2       VPUCTRL_REG_ADDR(0x282a)
#define VPU_HDMI_MATRIX_PRE_OFFSET0_1 VPUCTRL_REG_ADDR(0x282b)
#define VPU_HDMI_MATRIX_PRE_OFFSET2   VPUCTRL_REG_ADDR(0x282c)
#define VPU_HDMI_MATRIX_EN_CTRL       VPUCTRL_REG_ADDR(0x282d)

/* PAD_CTRL */
#define PADCTRL_PIN_MUX_REGB          PADCTRL_REG_ADDR(0x000b)

#endif

