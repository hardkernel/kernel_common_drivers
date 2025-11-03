/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __AMPRIME_SL_REGS_T6W_H__
#define __AMPRIME_SL_REGS_T6W_H__

#include <linux/types.h>
#include <linux/amlogic/media/vpu/vpu.h>
#include <linux/amlogic/media/utils/vdec_reg.h>

#define T6W_PRIMESL_LUTC_ADDR_PORT		(0x4c80)
#define T6W_PRIMESL_LUTC_DATA_PORT		(0x4c81)
#define T6W_PRIMESL_LUTP_ADDR_PORT		(0x4c82)
#define T6W_PRIMESL_LUTP_DATA_PORT		(0x4c83)
#define T6W_PRIMESL_LUTD_ADDR_PORT		(0x4c84)
#define T6W_PRIMESL_LUTD_DATA_PORT		(0x4c85)
#define T6W_PRIMESL_CTRL0				(0x4c90)
#define T6W_PRIMESL_CTRL1				(0x4c91)
#define T6W_PRIMESL_CTRL2				(0x4c92)
#define T6W_PRIMESL_CTRL3				(0x4c93)
#define T6W_PRIMESL_CTRL4				(0x4c94)
#define T6W_PRIMESL_CTRL5				(0x4c95)
#define T6W_PRIMESL_CTRL6				(0x4c96)
#define T6W_PRIMESL_CTRL7				(0x4c97)
#define T6W_PRIMESL_CTRL8				(0x4c98)
#define T6W_PRIMESL_CTRL9				(0x4c99)
#define T6W_PRIMESL_CTRL10				(0x4c9a)
#define T6W_PRIMESL_CTRL11				(0x4c9b)
#define T6W_PRIMESL_CTRL12				(0x4c9c)
#define T6W_PRIMESL_CTRL13				(0x4c9d)
#define T6W_PRIMESL_CTRL14				(0x4c9e)
#define T6W_PRIMESL_CTRL15				(0x4c9f)
#define T6W_PRIMESL_CTRL16				(0x4ce0)
#define T6W_PRIMESL_OMAT_OFFSET0		(0x4ce1)
#define T6W_PRIMESL_OMAT_OFFSET1		(0x4ce2)
#define T6W_PRIMESL_OMAT_OFFSET2		(0x4ce3)

#define T6W_PRIMESL_VPU_DOLBY_IPATH_SEL                        0x4512
//Bit 31:25         reserved
//Bit 24:22         reg_iloop_gofield_sel            // unsigned ,  RW,  default = 0
//Bit 21:20         reg_dvc1_gofield_sel             // unsigned ,  RW,  default = 0
//Bit 19:16         reserved
//Bit 15:0          reg_dpath_inmux_sel              // unsigned ,  RW,  default = 16'hffff
#define T6W_PRIMESL_VPU_DOLBY_OPATH_SEL                        0x4513
//Bit 23:0          reg_dpath_outmux_sel             // unsigned ,
#endif	/* __AMPRIME_SL_REGS_T6W_H__ */
