/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __VPU_REG_H__
#define __VPU_REG_H__
#include <linux/platform_device.h>

#define VPU_REG_END                  0xffff

/* ********************************
 * register define
 * *********************************
 */

#define AO_RTI_GEN_PWR_SLEEP0        0x03a
#define AO_RTI_GEN_PWR_ISO0          0x03b

/* HIU bus */

#define HHI_MEM_PD_REG0              0x40
#define HHI_VPU_MEM_PD_REG0          0x41
#define HHI_VPU_MEM_PD_REG1          0x42
#define HHI_VPU_MEM_PD_REG2          0x4d
#define HHI_VPU_MEM_PD_REG3          0x4e
#define HHI_VPU_MEM_PD_REG4          0x4c
#define HHI_VPU_MEM_PD_REG3_SM1      0x43
#define HHI_VPU_MEM_PD_REG4_SM1      0x44

/* pwrctrl reg */
#define PWRCTRL_PWR_ACK0             0x0000
#define PWRCTRL_PWR_ACK1             0x0001
#define PWRCTRL_PWR_OFF0             0x0004
#define PWRCTRL_PWR_OFF1             0x0005
#define PWRCTRL_ISO_EN0              0x0008
#define PWRCTRL_ISO_EN1              0x0009
#define PWRCTRL_FOCRST0              0x000c
#define PWRCTRL_FOCRST1              0x000d
#define PWRCTRL_MEM_PD5_SC2          0x0015
#define PWRCTRL_MEM_PD6_SC2          0x0016
#define PWRCTRL_MEM_PD7_SC2          0x0017
#define PWRCTRL_MEM_PD8_SC2          0x0018
#define PWRCTRL_MEM_PD9_SC2          0x0019

#define PWRCTRL_MEM_PD3_T5           0x00a
#define PWRCTRL_MEM_PD4_T5           0x00b
#define PWRCTRL_MEM_PD5_T5           0x00c
#define PWRCTRL_MEM_PD6_T5           0x00d
#define PWRCTRL_MEM_PD7_T5           0x00e

#define PWRCTRL_MEM_PD6_A4            0x0016

#define HHI_VPU_CLK_CNTL             0x6f
#define HHI_VAPBCLK_CNTL             0x7d
#define HHI_VPU_CLK_CTRL             0x6f

#define CLKCTRL_VPU_CLK_CTRL         0x003a
#define CLKCTRL_VAPBCLK_CTRL         0x003f

#define CLKCTRL_VOUTENC_CLK_CTRL     0x0046
#define CLKCTRL_VOUTENC_CLK_CTRL_A4  0x0081

/* cbus */
#define RESET0_LEVEL                 0x0420
#define RESET1_LEVEL                 0x0421
#define RESET2_LEVEL                 0x0422
#define RESET3_LEVEL                 0x0423
#define RESET4_LEVEL                 0x0424
#define RESET5_LEVEL                 0x0425
#define RESET6_LEVEL                 0x0426
#define RESET7_LEVEL                 0x0427

/* vpu clk gate */
/* vcbus */
#define VPU_CLK_GATE                 0x2723

#define VPP_RDARB_MODE               0x3978
#define VPP_RDARB_REQEN_SLV          0x3979
#define VPU_RDARB_MODE_L1C1          0x2790
#define DI_RDARB_MODE_L1C1           0x2050
#define DI_RDARB_REQEN_SLV_L1C1      0x2051
#define DI_WRARB_MODE_L1C1           0x2054
#define DI_WRARB_REQEN_SLV_L1C1      0x2055
#define DI_RDARB_UGT_L1C1            0x205b
#define DI_WRARB_UGT_L1C1            0x205d
#define VPU_RDARB_MODE_L1C2          0x2799
#define VPU_RDARB_MODE_L2C1          0x279d
#define VPP_RDARB_REQEN_SLV_L2C1     0x279e
#define VPU_WRARB_MODE_L2C1          0x27a2
#define VPU_WRARB_REQEN_SLV_L2C1     0x27a3
#define VPU_RDARB_UGT_L2C1           0x27c2
#define VPU_WRARB_UGT_L2C1           0x27c3

#define VENC_VDAC_TST_VAL            0x1b7f
#define VPP_DUMMY_DATA               0x1d00
#define VPU_VPU_PWM_V0               0x1ce0

#define VPU_VOUT_BLEND_DUMDATA       0x0011
#define VPP_VD1_MATRIX_OFFSET0_1     0x0289
#define VPU_VOUT_DTH_DATA            0x0103

#define ENCP_DVI_HSO_BEGIN           0x1c30
#define S5_VPU_VPU_PWM_V0            0x2730

#define VPU_VOUT_BLEND_DUMDATA_A4    0x0041
#define VPP_VD1_MATRIX_OFFSET0_1_A4  0x0189
#define VPU_VOUT_DTH_DATA_A4         0x0403

#define VPU_ARB_URG_CTRL            0x2747
#define VPP_OFIFO_URG_CTRL          0x1dd8

//VPU_ARB_URG_CTRL
#define VPU_ARB_URG_CTRL1_S5        0x2741
#define VPU_ARB_URG_CTRL1_T3X       0x2735
//bit29	super urgent enable
//bit28	sideband enable
//bit13	arb wr0/arb wr2 sideband cntl by vdin1 line fifo
//bit12	arb wr0/arb wr2 sideband cntl by vdin0 line fifo
//bit11	arb rd3  sideband ctrl by vpp2 line fifo
//bit10	arb rd3  sideband ctrl by vpp1 line fifo
//bit9	arb rd3  sideband ctrl by rdma
//bit8	arb rd3  sideband ctrl by vpp0 line fifo
//bit7	arb rd2  sideband ctrl by vpp2 line fifo
//bit6	arb rd2  sideband ctrl by vpp1 line fifo
//bit5	arb rd2  sideband ctrl by rdma
//bit4	arb rd2  sideband ctrl by vpp0 line fifo
//bit3	arb rd0  sideband ctrl by vpp2 line fifo
//bit2	arb rd0  sideband ctrl by vpp1 line fifo
//bit1	arb rd0  sideband ctrl by rdma
//bit0	arb rd0  sideband ctrl by vpp0 line fifo
//simple describe:
//bit0: arb0 rd enable
//bit4: arb2 rd enable
//bit12:arb wr0 enable
//bit28:sideband enable

#define VPU_ARB_URG_CTRL_T3X           0x2747
//not used
#define VPP_OFIFO_URG_CTRL_T3X         0x2505
//bit15:  urgent_ctrl_en
//bit12:  urg_init_value
//bit6-11: unurgent_value
//bit0-5: urgent_vlaue
#define VPP_SLICE1_OFIFO_URG_CTRL_T3X   0x2605
//bit15:  urgent_ctrl_en
//bit12:  urg_init_value
//bit6-11: unurgent_value
//bit0-5: urgent_vlaue

//t3x dmc regs:
   //bit 8.   enable dmc request of axibus chan 7.	NNA Async interface.
   //bit 7.   enable dmc request of axibus chan 7.	VGE Async interface.
   //bit 6.   enable dmc request of axibus chan 6.	GPU async interface.
   //bit 5.   enable dmc request of axibus chan 5.	reserved for dmc_test.
   //bit 4.   enable dmc request of axibus chan 4.	system Async interface.
   //bit 3.   enable dmc request of axibus chan 3.	DOS Async interface.
   //bit 2.   enable dmc request of axibus chan 2.	VPU Async interface.
   //bit 1.   enable dmc request of axibus chan 1.	FRC Async interface.
   //bit 0.   enable dmc request of axibus chan 0.	CPU/A55   async interface.

#define DMC_AXI2_CHAN_CTRL1                        0x008a
//t3x
   //bit 31:29. not used.
   //bit 28:20. when write/read side band signal used to block other request.
   //configure which master we can block. each bit for  one master.
    //Note. don't block vpu itself and  CPU or other urgent request.
   //bit 19. use side band write urgent control signal to control AWQOS.
   //1: enabe. 0: disable.
   //bit 18. use side band read  urgent control signal to control ARQOS.
   //1: enabe. 0: disable.
   //bit 17. use side band write urgent control signal to block other master request.
   //1: enable. 0 disable.
   //bit 16. use side band read urgent control signal to block other master request.
   //1: enable. 0 disable.
   //bit 15:12.  the AWQOS value when side band write urgent
   //control signal = 1 while bit 19 enabled.
   //bit 11:8.   the ARQOS value when side band read  urgent
   //control signal = 1 while bit 18 enabled.
   //bit 7:0.    Not used.

//t7c
//t7c dmc regs:
   //bit 8.   enable dmc request of axibus chan 7.	reserved.
   //bit 7.   enable dmc request of axibus chan 7.	device2/isp/demux/dsp Async interface.
   //bit 6.   enable dmc request of axibus chan 6.	DOS/GE2D interface.
   //bit 5.   enable dmc request of axibus chan 5.	reserved for dmc_test.
   //bit 4.   enable dmc request of axibus chan 4.	device/usb/pcie/hdmi rx Async interface.
   //bit 3.   enable dmc request of axibus chan 3.	NNA Async interface.
   //bit 2.   enable dmc request of axibus chan 2.	VPU Async interface.
   //bit 1.   enable dmc request of axibus chan 1.	reserved.
   //bit 0.   enable dmc request of axibus chan 0.	CPU/A73/A53/GPU async interface.

//bit 31:20. not used.
//bit 19. use side band write urgent control signal to control AWQOS.  1: enabe. 0: disable.
//bit 18. use side band read urgent control signal to control ARQOS.  1: enabe. 0: disable.
//bit 17. use side band write urgent control signal to block other master request.
//1: enable. 0 disable.
//bit 16. use side band read urgent control signal to block other master request.
//1: enable. 0 disable.
//bit 15:12.  the AWQOS value when side band write urgent control signal = 1 while bit 19 enabled.
//bit 11:8.   the ARQOS value when side band read urgent control signal = 1 while bit 18 enabled.
//bit 7:0.	  when write/read side band signal used to block other request.
//configure which master we can block.each bit for  one master.
//Note. don't block VPU and CPU or other urgent request.

int vpu_ioremap(struct platform_device *pdev, int *reg_map_table);

unsigned int vpu_clk_read(unsigned int _reg);
void vpu_clk_write(unsigned int _reg, unsigned int _value);
void vpu_clk_setb(unsigned int _reg, unsigned int _value,
		  unsigned int _start, unsigned int _len);
unsigned int vpu_clk_getb(unsigned int _reg,
			  unsigned int _start, unsigned int _len);
void vpu_clk_set_mask(unsigned int _reg, unsigned int _mask);
void vpu_clk_clr_mask(unsigned int _reg, unsigned int _mask);

unsigned int vpu_pwrctrl_read(unsigned int _reg);
unsigned int vpu_pwrctrl_getb(unsigned int _reg,
			      unsigned int _start, unsigned int _len);

unsigned int vpu_ao_read(unsigned int _reg);
void vpu_ao_write(unsigned int _reg, unsigned int _value);
void vpu_ao_setb(unsigned int _reg, unsigned int _value,
		 unsigned int _start, unsigned int _len);
void vpu_dmc0_write(unsigned int _reg, unsigned int _value);
unsigned int vpu_dmc0_read(unsigned int _reg);
void vpu_dmc0_setb(unsigned int _reg, unsigned int _value,
		 unsigned int _start, unsigned int _len);
void vpu_dmc1_write(unsigned int _reg, unsigned int _value);
unsigned int vpu_dmc1_read(unsigned int _reg);
void vpu_dmc1_setb(unsigned int _reg, unsigned int _value,
		 unsigned int _start, unsigned int _len);
#endif
