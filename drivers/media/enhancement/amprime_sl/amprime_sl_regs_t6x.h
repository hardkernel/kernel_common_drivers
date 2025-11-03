/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __AMPRIME_SL_REGS_T6X_H__
#define __AMPRIME_SL_REGS_T6X_H__

#include <linux/types.h>
#include <linux/amlogic/media/vpu/vpu.h>
#include <linux/amlogic/media/utils/vdec_reg.h>

#define T6X_PRIMESL_LUTC_ADDR_PORT		(0x4d80)
#define T6X_PRIMESL_LUTC_DATA_PORT		(0x4d81)
#define T6X_PRIMESL_LUTP_ADDR_PORT		(0x4d82)
#define T6X_PRIMESL_LUTP_DATA_PORT		(0x4d83)
#define T6X_PRIMESL_LUTD_ADDR_PORT		(0x4d84)
#define T6X_PRIMESL_LUTD_DATA_PORT		(0x4d85)
#define T6X_PRIMESL_CTRL0				(0x4d90)
#define T6X_PRIMESL_CTRL1				(0x4d91)
#define T6X_PRIMESL_CTRL2				(0x4d92)
#define T6X_PRIMESL_CTRL3				(0x4d93)
#define T6X_PRIMESL_CTRL4				(0x4d94)
#define T6X_PRIMESL_CTRL5				(0x4d95)
#define T6X_PRIMESL_CTRL6				(0x4d96)
#define T6X_PRIMESL_CTRL7				(0x4d97)
#define T6X_PRIMESL_CTRL8				(0x4d98)
#define T6X_PRIMESL_CTRL9				(0x4d99)
#define T6X_PRIMESL_CTRL10				(0x4d9a)
#define T6X_PRIMESL_CTRL11				(0x4d9b)
#define T6X_PRIMESL_CTRL12				(0x4d9c)
#define T6X_PRIMESL_CTRL13				(0x4d9d)
#define T6X_PRIMESL_CTRL14				(0x4d9e)
#define T6X_PRIMESL_CTRL15				(0x4d9f)
#define T6X_PRIMESL_CTRL16				(0x4de0)
#define T6X_PRIMESL_OMAT_OFFSET0		(0x4de1)
#define T6X_PRIMESL_OMAT_OFFSET1		(0x4de2)
#define T6X_PRIMESL_OMAT_OFFSET2		(0x4de3)

#define T6X_PRIMESL_VPU_AXIRD_PATH_CTRL                        0x6d02
//Bit 31:10       reserved
//Bit 9:8         reg_dv_path_mode   // unsigned ,   RW,  default = 0,0:off,1:nr_dv,2:vd1_dv
//Bit 7:3         reserved
//Bit 2           reg_di_vpp_link    // unsigned ,   RW,  default = 0
//Bit 1           reg_nr_vpp_link    // unsigned ,   RW,  default = 0
//Bit 0           reg_vd1_vpp_link   // unsigned ,   RW,  default = 1
#define T6X_PRIMESL_VPU_DOLBY_IPATH_SEL                        0x4513
//Bit 31:25         reserved
//Bit 24:22         reg_iloop_gofield_sel            // unsigned ,  RW,  default = 0
//Bit 21:20         reg_dvc1_gofield_sel             // unsigned ,  RW,  default = 0
//Bit 19:16         reserved
//Bit 15:0          reg_dpath_inmux_sel              // unsigned ,  RW,  default = 16'hffff
#define T6X_PRIMESL_VPU_DOLBY_OPATH_SEL                        0x4514
//Bit 23:0          reg_dpath_outmux_sel             // unsigned ,
#endif	/* __AMPRIME_SL_REGS_T6X_H__ */
