// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2021 Amlogic, Inc. All rights reserved.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/amlogic/arm-smccc.h>
#include <linux/slab.h>
#include "ddr_port.h"
#include "dmc_monitor.h"

#define VPU_PORT_OSD1			"OSD1"
#define VPU_PORT_OSD2			"OSD2"
#define VPU_PORT_OSD3			"OSD3"
#define VPU_PORT_MALI_AFBCD_RD		"MALI_AFBC_RD"
#define VPU_PORT_TCON_P1		"TCON_P1"
#define VPU_PORT_TCON_P2		"TCON_P2"
#define VPU_PORT_RDMA_RD		"RDMA_RD"
#define VPU_PORT_DMA_RD			"DMA_RD"
#define VPU_PORT_DAE_RD0		"DAE_RD0"
#define VPU_PORT_DAE_RD1		"DAE_RD1"
#define VPU_PORT_DAE_RD2		"DAE_RD2"
#define VPU_PORT_DAE_RD3		"DAE_RD3"
#define VPU_PORT_DAE_RD4		"DAE_RD4"
#define VPU_PORT_DAE_RD5		"DAE_RD5"
#define VPU_PORT_DAE_RD6		"DAE_RD6"
#define VPU_PORT_DAE_RD7		"DAE_RD7"
#define VPU_PORT_DAE_RD8		"DAE_RD8"
#define VPU_PORT_DAE_RD9		"DAE_RD9"
#define VPU_PORT_DAE_RD10		"DAE_RD10"
#define VPU_PORT_DAE_RD11		"DAE_RD11"
#define VPU_PORT_INP_RD0		"INP_RD0"
#define VPU_PORT_INP_RD1		"INP_RD1"
#define VPU_PORT_NR_VFCD0		"NR_VFCD0"
#define VPU_PORT_NR_VFCD1		"NR_VFCD1"
#define VPU_PORT_NR_VFCD2		"NR_VFCD2"
#define VPU_PORT_NR_VFCD3		"NR_VFCD3"
#define VPU_PORT_MC_VFCD0		"MC_VFCD0"
#define VPU_PORT_MC_VFCD1		"MC_VFCD1"
#define VPU_PORT_MC_VFCD2		"MC_VFCD2"
#define VPU_PORT_MC_VFCD3		"MC_VFCD3"
#define VPU_PORT_NR_RD0			"NR_RD0"
#define VPU_PORT_NR_RD1			"NR_RD1"
#define VPU_PORT_NR_RD2			"NR_RD2"
#define VPU_PORT_NR_RD3			"NR_RD3"
#define VPU_PORT_NR_RD4			"NR_RD4"
#define VPU_PORT_NR_RD5			"NR_RD5"
#define VPU_PORT_NR_RD6			"NR_RD6"
#define VPU_PORT_NR_RD7			"NR_RD7"
#define VPU_PORT_NR_RD8			"NR_RD8"
#define VPU_PORT_NR_RD9			"NR_RD9"
#define VPU_PORT_NR_RD10		"NR_RD10"
#define VPU_PORT_MC_RD0			"MC_RD0"
#define VPU_PORT_MC_RD1			"MC_RD1"
#define VPU_PORT_MC_RD2			"MC_RD2"
#define VPU_PORT_MC_RD3			"MC_RD3"
#define VPU_PORT_MMU			"MMU"

#define	VPU_PORT_VDIN0_WR		"VDIN0_WR"
#define	VPU_PORT_VDIN1_WR		"VDIN1_WR"
#define	VPU_PORT_VDIN_DOLBY		"VDIN_DOLBY"
#define	VPU_PORT_VDIN_AFBCE		"VDIN_AFBCE"
#define	VPU_PORT_RDMA			"RDMA"
#define	VPU_PORT_LDIM			"LDIM"
#define	VPU_PORT_DMA			"DMA"
#define	VPU_PORT_DAE_WR0		"DAE_WR0"
#define	VPU_PORT_DAE_WR1		"DAE_WR1"
#define	VPU_PORT_DAE_WR2		"DAE_WR2"
#define	VPU_PORT_DAE_WR3		"DAE_WR3"
#define	VPU_PORT_DAE_WR4		"DAE_WR4"
#define	VPU_PORT_DAE_WR5		"DAE_WR5"
#define	VPU_PORT_DAE_WR6		"DAE_WR6"
#define	VPU_PORT_DAE_WR7		"DAE_WR7"
#define	VPU_PORT_DAE_WR8		"DAE_WR8``"
#define	VPU_PORT_INP_WR			"INP_WR"
#define	VPU_PORT_RDMA_WR		"RDMA_WR"
#define	VPU_PORT_VFCE_WRMIF2		"VFCE_WRMIF2"
#define	VPU_PORT_DS_WRMIF		"DS_WRMIF"
#define	VPU_PORT_WRMIF0			"WRMIF0"
#define	VPU_PORT_WRMIF1			"WRMIF1"
#define	VPU_PORT_DMSQ_WR		"DMSQ_WR"
#define	VPU_PORT_NR_WR0			"NR_WR0"
#define	VPU_PORT_NR_WR1			"NR_WR1"

#ifndef CONFIG_AMLOGIC_REMOVE_OLD
static struct vpu_sub_desc vpu_sub_desc_txlx[] __initdata = {
	{ .sub_id = 0x0, .vpu_r0_2 = "OSD1", .vpu_r1 = "DI_IF1",
			.vpu_w0 = "VDIN0", .vpu_w1 = "NR"		},

	{ .sub_id = 0x1, .vpu_r0_2 = "OSD2", .vpu_r1 = "DI_MEM",
			.vpu_w0 = "VDIN1", .vpu_w1 = "DI"		},

	{ .sub_id = 0x2, .vpu_r0_2 = "VD1", .vpu_r1 = "DI_INP",
			.vpu_w0 = "VDIN_DOLBY", .vpu_w1 = "DI_SUB"	},

	{ .sub_id = 0x3, .vpu_r0_2 = "VD2", .vpu_r1 = "DI_CHAN2",
			.vpu_w0 = "AFBC_ENC", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x4, .vpu_r0_2 = "OSD3", .vpu_r1 = "DI_SUB",
			.vpu_w0 = "TVFE", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x5, .vpu_r0_2 = "AFBC_DEC", .vpu_r1 = "DI_IF2",
			.vpu_w0 = "TCON1", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x6, .vpu_r0_2 = "DOLBY0", .vpu_r1 = "DI_IF0",
			.vpu_w0 = "RDMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x7, .vpu_r0_2 = "MALI_AFBCD", .vpu_r1 = "NULL",
			.vpu_w0 = "TCON2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x8, .vpu_r0_2 = "VIU2", .vpu_r1 = "NULL",
			.vpu_w0 = "TCON3", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x9, .vpu_r0_2 = "TCON1", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xA, .vpu_r0_2 = "RDMA", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xB, .vpu_r0_2 = "TVFE", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xC, .vpu_r0_2 = "TCON_P2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xD, .vpu_r0_2 = "TCON_P3", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xE, .vpu_r0_2 = "AFBC_ENC", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xF, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		}
};

//static struct vpu_sub_desc vpu_sub_desc_txl[] __initdata = vpu_sub_desc_txlx[]
//static struct vpu_sub_desc vpu_sub_desc_txhd[] __initdata = vpu_sub_desc_txlx[]

static struct vpu_sub_desc vpu_sub_desc_tl1[] __initdata = {
	{ .sub_id = 0x0, .vpu_r0_2 = "OSD1", .vpu_r1 = "DI_IF1",
			.vpu_w0 = "VDIN0", .vpu_w1 = "NR"		},

	{ .sub_id = 0x1, .vpu_r0_2 = "OSD2", .vpu_r1 = "DI_MEM",
			.vpu_w0 = "VDIN1", .vpu_w1 = "DI"		},

	{ .sub_id = 0x2, .vpu_r0_2 = "VD1", .vpu_r1 = "DI_INP",
			.vpu_w0 = "VDIN_DOLBY", .vpu_w1 = "DI_SUB"	},

	{ .sub_id = 0x3, .vpu_r0_2 = "VD2", .vpu_r1 = "DI_CHAN2",
			.vpu_w0 = "AFBC_ENC", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x4, .vpu_r0_2 = "OSD3", .vpu_r1 = "DI_SUB",
			.vpu_w0 = "TVFE", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x5, .vpu_r0_2 = "AFBC_DEC", .vpu_r1 = "DI_IF2",
			.vpu_w0 = "TCON1", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x6, .vpu_r0_2 = "DOLBY0", .vpu_r1 = "DI_IF0",
			.vpu_w0 = "RDMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x7, .vpu_r0_2 = "MALI_AFBCD", .vpu_r1 = "DI_AFBCD",
			.vpu_w0 = "TCON2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x8, .vpu_r0_2 = "VIU2", .vpu_r1 = "NULL",
			.vpu_w0 = "TCON3", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x9, .vpu_r0_2 = "TCON1", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xA, .vpu_r0_2 = "RDMA", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xB, .vpu_r0_2 = "TVFE", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xC, .vpu_r0_2 = "TCON_P2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xD, .vpu_r0_2 = "TCON_P3", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xE, .vpu_r0_2 = "AFBC_ENC", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xF, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		}
};
#endif

#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
static struct vpu_sub_desc vpu_sub_desc_sm1[] __initdata = {
	{ .sub_id = 0x0, .vpu_r0_2 = "OSD1", .vpu_r1 = "DI_IF1",
			.vpu_w0 = "VDIN0", .vpu_w1 = "NR"		},

	{ .sub_id = 0x1, .vpu_r0_2 = "OSD2", .vpu_r1 = "DI_MEM",
			.vpu_w0 = "VDIN1", .vpu_w1 = "DI"		},

	{ .sub_id = 0x2, .vpu_r0_2 = "VD1", .vpu_r1 = "DI_INP",
			.vpu_w0 = "VDIN_DOLBY", .vpu_w1 = "DI_SUB"	},

	{ .sub_id = 0x3, .vpu_r0_2 = "VD2", .vpu_r1 = "DI_CHAN2",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x4, .vpu_r0_2 = "OSD3", .vpu_r1 = "DI_SUB",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x5, .vpu_r0_2 = "NULL", .vpu_r1 = "DI_IF2",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x6, .vpu_r0_2 = "DOLBY0", .vpu_r1 = "DI_IF0",
			.vpu_w0 = "RDMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x7, .vpu_r0_2 = "MALI_AFBCD", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x8, .vpu_r0_2 = "VIU2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x9, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xA, .vpu_r0_2 = "RDMA", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xB, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xC, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xD, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xE, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xF, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},
};

// static struct vpu_sub_desc vpu_sub_desc_g12b[] __initdata = vpu_sub_desc_sm1[]
// static struct vpu_sub_desc vpu_sub_desc_g12a[] __initdata = vpu_sub_desc_sm1[]

static struct vpu_sub_desc vpu_sub_desc_tm2[] __initdata = {
	{ .sub_id = 0x0, .vpu_r0_2 = "OSD1", .vpu_r1 = "DI_IF1",
			.vpu_w0 = "VDIN0", .vpu_w1 = "NR"		},

	{ .sub_id = 0x1, .vpu_r0_2 = "OSD2", .vpu_r1 = "DI_MEM",
			.vpu_w0 = "VDIN1", .vpu_w1 = "DI"		},

	{ .sub_id = 0x2, .vpu_r0_2 = "VD1", .vpu_r1 = "DI_INP",
			.vpu_w0 = "VDIN_DOLBY", .vpu_w1 = "DI_SUB"	},

	{ .sub_id = 0x3, .vpu_r0_2 = "VD2", .vpu_r1 = "DI_CHAN2",
			.vpu_w0 = "VDIN_AFBCE", .vpu_w1 = "NULL"	},

	{ .sub_id = 0x4, .vpu_r0_2 = "OSD3", .vpu_r1 = "DI_SUB",
			.vpu_w0 = "TVFE", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x5, .vpu_r0_2 = "AFBC_DEC", .vpu_r1 = "DI_IF2",
			.vpu_w0 = "TCON1", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x6, .vpu_r0_2 = "DOLBY0", .vpu_r1 = "DI_IF0",
			.vpu_w0 = "RDMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x7, .vpu_r0_2 = "MALI_AFBCD", .vpu_r1 = "NULL",
			.vpu_w0 = "TCON2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x8, .vpu_r0_2 = "VIU2", .vpu_r1 = "NULL",
			.vpu_w0 = "TCON3", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x9, .vpu_r0_2 = "TCON1", .vpu_r1 = "NULL",
			.vpu_w0 = "VDIN2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xA, .vpu_r0_2 = "RDMA", .vpu_r1 = "NULL",
			.vpu_w0 = "VPU_DMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xB, .vpu_r0_2 = "TVFE", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xC, .vpu_r0_2 = "TCON_P2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xD, .vpu_r0_2 = "TCON_P3", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xE, .vpu_r0_2 = "VDIN_AFBCE", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xF, .vpu_r0_2 = "VPU_DMA", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},
};

static struct vpu_sub_desc vpu_sub_desc_t5[] __initdata = {
	{ .sub_id = 0x0, .vpu_r0_2 = "OSD1", .vpu_r1 = "DI_IF1",
			.vpu_w0 = "VDIN0", .vpu_w1 = "NR"		},

	{ .sub_id = 0x1, .vpu_r0_2 = "OSD2", .vpu_r1 = "DI_MEM",
			.vpu_w0 = "VDIN1", .vpu_w1 = "DI"		},

	{ .sub_id = 0x2, .vpu_r0_2 = "VD1", .vpu_r1 = "DI_INP",
			.vpu_w0 = "VDIN_DOLBY", .vpu_w1 = "DI_SUB"	},

	{ .sub_id = 0x3, .vpu_r0_2 = "VD2", .vpu_r1 = "DI_CHAN2",
			.vpu_w0 = "VDIN_AFBCE", .vpu_w1 = "DI_AFBCE0"	},

	{ .sub_id = 0x4, .vpu_r0_2 = "OSD3", .vpu_r1 = "DI_SUB",
			.vpu_w0 = "DCNTR_GRID", .vpu_w1 = "DI_AFBCE1"	},

	{ .sub_id = 0x5, .vpu_r0_2 = "AFBC_DEC", .vpu_r1 = "DI_IF2",
			.vpu_w0 = "TCON1", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x6, .vpu_r0_2 = "DOLBY0", .vpu_r1 = "DI_IF0",
			.vpu_w0 = "RDMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x7, .vpu_r0_2 = "MALI_AFBCD", .vpu_r1 = "DI_AFBCD",
			.vpu_w0 = "TCON2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x8, .vpu_r0_2 = "VIU2", .vpu_r1 = "NULL",
			.vpu_w0 = "LDIM", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x9, .vpu_r0_2 = "TCON1", .vpu_r1 = "NULL",
			.vpu_w0 = "VDIN2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xA, .vpu_r0_2 = "RDMA", .vpu_r1 = "NULL",
			.vpu_w0 = "VPU_DMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xB, .vpu_r0_2 = "TVFE", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xC, .vpu_r0_2 = "TCON_P2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xD, .vpu_r0_2 = "DCNTR_GRID", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xE, .vpu_r0_2 = "AFBC_ENC", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xF, .vpu_r0_2 = "VPU_DMA", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},
};

static struct vpu_sub_desc vpu_sub_desc_t5d[] __initdata = {
	{ .sub_id = 0x0, .vpu_r0_2 = "OSD1", .vpu_r1 = "DI_IF1",
			.vpu_w0 = "VDIN0", .vpu_w1 = "NR"		},

	{ .sub_id = 0x1, .vpu_r0_2 = "OSD2", .vpu_r1 = "DI_MEM",
			.vpu_w0 = "VDIN1", .vpu_w1 = "DI"		},

	{ .sub_id = 0x2, .vpu_r0_2 = "VD1", .vpu_r1 = "DI_INP",
			.vpu_w0 = "VDIN_DOLBY", .vpu_w1 = "DI_SUB"	},

	{ .sub_id = 0x3, .vpu_r0_2 = "VD2", .vpu_r1 = "DI_CHAN2",
			.vpu_w0 = "AFBC_ENC", .vpu_w1 = "DI_AFBCE"	},

	{ .sub_id = 0x4, .vpu_r0_2 = "OSD3", .vpu_r1 = "DI_SUB",
			.vpu_w0 = "TVFE", .vpu_w1 = "NULL"	},

	{ .sub_id = 0x5, .vpu_r0_2 = "AFBC_DEC", .vpu_r1 = "DI_IF2",
			.vpu_w0 = "TCON1", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x6, .vpu_r0_2 = "DOLBY0", .vpu_r1 = "DI_IF0",
			.vpu_w0 = "RDMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x7, .vpu_r0_2 = "MALI_AFBCD", .vpu_r1 = "DI_AFBCD",
			.vpu_w0 = "TCON2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x8, .vpu_r0_2 = "VIU2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x9, .vpu_r0_2 = "TCON1", .vpu_r1 = "NULL",
			.vpu_w0 = "VDIN2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xA, .vpu_r0_2 = "RDMA", .vpu_r1 = "NULL",
			.vpu_w0 = "VPU_DMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xB, .vpu_r0_2 = "TVFE", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xC, .vpu_r0_2 = "TCON_P2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xD, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xE, .vpu_r0_2 = "AFBC_ENC", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xF, .vpu_r0_2 = "VPU_DMA", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},
};

static struct vpu_sub_desc vpu_sub_desc_t7[] __initdata = {
	{ .sub_id = 0x0, .vpu_r0_2 = "OSD1", .vpu_r1 = "DI_IF1",
			.vpu_w0 = "VDIN0", .vpu_w1 = "NR"		},

	{ .sub_id = 0x1, .vpu_r0_2 = "OSD2", .vpu_r1 = "DI_MEM",
			.vpu_w0 = "VDIN1", .vpu_w1 = "DI"		},

	{ .sub_id = 0x2, .vpu_r0_2 = "VD1", .vpu_r1 = "DI_INP",
			.vpu_w0 = "VDIN_DOLBY", .vpu_w1 = "DI_SUB"	},

	{ .sub_id = 0x3, .vpu_r0_2 = "VD2", .vpu_r1 = "DI_CHAN2",
			.vpu_w0 = "VDIN_AFBCE", .vpu_w1 = "DI_AFBCE0"	},

	{ .sub_id = 0x4, .vpu_r0_2 = "OSD3", .vpu_r1 = "DI_SUB",
			.vpu_w0 = "DCNTR_GRID", .vpu_w1 = "DI_AFBCE1"	},

	{ .sub_id = 0x5, .vpu_r0_2 = "OSD4", .vpu_r1 = "DI_IF2",
			.vpu_w0 = "TCON1", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x6, .vpu_r0_2 = "DOLBY0", .vpu_r1 = "DI_IF0",
			.vpu_w0 = "RDMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x7, .vpu_r0_2 = "MALI_AFBCD", .vpu_r1 = "DI_AFBCD",
			.vpu_w0 = "TCON2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x8, .vpu_r0_2 = "VD3", .vpu_r1 = "NULL",
			.vpu_w0 = "LDIM", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x9, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "VDIN2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xA, .vpu_r0_2 = "RDMA", .vpu_r1 = "NULL",
			.vpu_w0 = "VPU_DMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xB, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xC, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xD, .vpu_r0_2 = "LDIM", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xE, .vpu_r0_2 = "VDIN_AFBCE", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xF, .vpu_r0_2 = "VPU_DMA", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},
};

static struct vpu_sub_desc vpu_sub_desc_t3[] __initdata = {
	{ .sub_id = 0x0, .vpu_r0_2 = "OSD1", .vpu_r1 = "DI_IF1",
			.vpu_w0 = "VDIN0", .vpu_w1 = "NR"		},

	{ .sub_id = 0x1, .vpu_r0_2 = "OSD2", .vpu_r1 = "DI_MEM",
			.vpu_w0 = "VDIN1", .vpu_w1 = "DI"		},

	{ .sub_id = 0x2, .vpu_r0_2 = "VD1", .vpu_r1 = "DI_INP",
			.vpu_w0 = "VDIN_DOLBY", .vpu_w1 = "DI_SUB"	},

	{ .sub_id = 0x3, .vpu_r0_2 = "VD2", .vpu_r1 = "DI_CHAN2",
			.vpu_w0 = "VDIN_AFBCE", .vpu_w1 = "DI_AFBCE0"	},

	{ .sub_id = 0x4, .vpu_r0_2 = "OSD3", .vpu_r1 = "DI_SUB",
			.vpu_w0 = "DCNTR_GRID", .vpu_w1 = "DI_AFBCE1"	},

	{ .sub_id = 0x5, .vpu_r0_2 = "OSD4", .vpu_r1 = "DI_IF2",
			.vpu_w0 = "TCON1", .vpu_w1 = "AISR_HPF"		},

	{ .sub_id = 0x6, .vpu_r0_2 = "DOLBY0", .vpu_r1 = "DI_IF0",
			.vpu_w0 = "RDMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x7, .vpu_r0_2 = "MALI_AFBCD", .vpu_r1 = "DI_AFBCD",
			.vpu_w0 = "TCON2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x8, .vpu_r0_2 = "NPU", .vpu_r1 = "NULL",
			.vpu_w0 = "LDIM", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x9, .vpu_r0_2 = "TCON", .vpu_r1 = "NULL",
			.vpu_w0 = "VDIN2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xA, .vpu_r0_2 = "RDMA", .vpu_r1 = "NULL",
			.vpu_w0 = "VPU_DMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xB, .vpu_r0_2 = "DCNTR_GRID", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xC, .vpu_r0_2 = "TCON_P2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xD, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xE, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xF, .vpu_r0_2 = "VPU_SUBRD", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		}
};
#endif

// static struct vpu_sub_desc vpu_sub_desc_s4d[] __initdata = vpu_sub_desc_s4[]
static struct vpu_sub_desc vpu_sub_desc_s4[] __initdata = {
	{ .sub_id = 0x0, .vpu_r0_2 = "OSD1", .vpu_r1 = "DI_IF1",
			.vpu_w0 = "VDIN0", .vpu_w1 = "NR"		},

	{ .sub_id = 0x1, .vpu_r0_2 = "OSD2", .vpu_r1 = "DI_MEM",
			.vpu_w0 = "VDIN1", .vpu_w1 = "DI"		},

	{ .sub_id = 0x2, .vpu_r0_2 = "VD1", .vpu_r1 = "DI_INP",
			.vpu_w0 = "VDIN_DOLBY", .vpu_w1 = "DI_SUB"	},

	{ .sub_id = 0x3, .vpu_r0_2 = "VD2", .vpu_r1 = "DI_CHAN2",
			.vpu_w0 = "AFBC_ENC", .vpu_w1 = "DI_AFBCE0"		},

	{ .sub_id = 0x4, .vpu_r0_2 = "OSD3", .vpu_r1 = "DI_SUB",
			.vpu_w0 = "TVFE", .vpu_w1 = "DI_AFBCE1"		},

	{ .sub_id = 0x5, .vpu_r0_2 = "AFBC_DEC", .vpu_r1 = "DI_IF2",
			.vpu_w0 = "TCON1", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x6, .vpu_r0_2 = "DOLBY0", .vpu_r1 = "DI_IF0",
			.vpu_w0 = "RDMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x7, .vpu_r0_2 = "MALI_AFBCD", .vpu_r1 = "DI_AFBCD",
			.vpu_w0 = "TCON2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x8, .vpu_r0_2 = "VIU2", .vpu_r1 = "NULL",
			.vpu_w0 = "TCON3", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x9, .vpu_r0_2 = "TCON1", .vpu_r1 = "NULL",
			.vpu_w0 = "VDIN2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xA, .vpu_r0_2 = "RDMA", .vpu_r1 = "NULL",
			.vpu_w0 = "VPU_DMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xB, .vpu_r0_2 = "TVFE", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xC, .vpu_r0_2 = "TCON_P2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xD, .vpu_r0_2 = "TCON_P3", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xE, .vpu_r0_2 = "AFBC_ENC", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xF, .vpu_r0_2 = "VPU_DMA", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		}
};

#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
static struct vpu_sub_desc vpu_sub_desc_sc2[] __initdata = {
	{ .sub_id = 0x0, .vpu_r0_2 = "OSD1", .vpu_r1 = "DI_IF1",
			.vpu_w0 = "VDIN0", .vpu_w1 = "NR"		},

	{ .sub_id = 0x1, .vpu_r0_2 = "OSD2", .vpu_r1 = "DI_MEM",
			.vpu_w0 = "VDIN1", .vpu_w1 = "DI"		},

	{ .sub_id = 0x2, .vpu_r0_2 = "VD1", .vpu_r1 = "DI_INP",
			.vpu_w0 = "VDIN_DOLBY", .vpu_w1 = "DI_SUB"	},

	{ .sub_id = 0x3, .vpu_r0_2 = "VD2", .vpu_r1 = "DI_CHAN2",
			.vpu_w0 = "VDIN_AFBCE", .vpu_w1 = "DI_AFBCE0"	},

	{ .sub_id = 0x4, .vpu_r0_2 = "OSD3", .vpu_r1 = "DI_SUB",
			.vpu_w0 = "TVFE", .vpu_w1 = "DI_AFBCE1"		},

	{ .sub_id = 0x5, .vpu_r0_2 = "OSD4", .vpu_r1 = "DI_IF2",
			.vpu_w0 = "TCON1", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x6, .vpu_r0_2 = "DOLBY0", .vpu_r1 = "DI_IF0",
			.vpu_w0 = "RDMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x7, .vpu_r0_2 = "MALI_AFBCD", .vpu_r1 = "DI_AFBCD",
			.vpu_w0 = "TCON2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x8, .vpu_r0_2 = "VIU2", .vpu_r1 = "NULL",
			.vpu_w0 = "TCON3", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x9, .vpu_r0_2 = "TCON1", .vpu_r1 = "NULL",
			.vpu_w0 = "VDIN2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xA, .vpu_r0_2 = "RDMA", .vpu_r1 = "NULL",
			.vpu_w0 = "VPU_DMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xB, .vpu_r0_2 = "TVFE", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xC, .vpu_r0_2 = "TCON_P2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xD, .vpu_r0_2 = "TCON_P3", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xE, .vpu_r0_2 = "VDIN_AFBCE", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xF, .vpu_r0_2 = "VPU_DMA", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		}
};

static struct vpu_sub_desc vpu_sub_desc_t5w[] __initdata = {
	{ .sub_id = 0x0, .vpu_r0_2 = "OSD1", .vpu_r1 = "DI_IF1",
			.vpu_w0 = "VDIN0", .vpu_w1 = "NR"		},

	{ .sub_id = 0x1, .vpu_r0_2 = "OSD2", .vpu_r1 = "DI_MEM",
			.vpu_w0 = "VDIN1", .vpu_w1 = "DI"		},

	{ .sub_id = 0x2, .vpu_r0_2 = "VD1", .vpu_r1 = "DI_INP",
			.vpu_w0 = "VDIN_DOLBY", .vpu_w1 = "DI_SUB"	},

	{ .sub_id = 0x3, .vpu_r0_2 = "VD2", .vpu_r1 = "DI_CHAN2",
			.vpu_w0 = "AFBC_ENC", .vpu_w1 = "DI_AFBCE0"	},

	{ .sub_id = 0x4, .vpu_r0_2 = "OSD3", .vpu_r1 = "DI_SUB",
			.vpu_w0 = "DCNTR_GRID", .vpu_w1 = "DI_AFBCE1"	},

	{ .sub_id = 0x5, .vpu_r0_2 = "AFBC_DEC", .vpu_r1 = "DI_IF2",
			.vpu_w0 = "TCON1", .vpu_w1 = "ASIR_HPF"		},

	{ .sub_id = 0x6, .vpu_r0_2 = "DOLBY0", .vpu_r1 = "DI_IF0",
			.vpu_w0 = "RDMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x7, .vpu_r0_2 = "MALI_AFBCD", .vpu_r1 = "DI_AFBCD",
			.vpu_w0 = "TCON2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x8, .vpu_r0_2 = "NPU", .vpu_r1 = "NULL",
			.vpu_w0 = "LDIM", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x9, .vpu_r0_2 = "TCON1", .vpu_r1 = "NULL",
			.vpu_w0 = "VDIN2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xA, .vpu_r0_2 = "RDMA", .vpu_r1 = "NULL",
			.vpu_w0 = "VPU_DMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xB, .vpu_r0_2 = "DCNTR_GRID", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xC, .vpu_r0_2 = "TCON_P2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xD, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xE, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xF, .vpu_r0_2 = "VPU_SUBRD", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		}
};

static struct vpu_sub_desc vpu_sub_desc_s5[] __initdata = {
	{ .sub_id = 0x0, .vpu_r0_2 = "OSD1", .vpu_r1 = "DI_IF1",
			.vpu_w0 = "VDIN0", .vpu_w1 = "NR"		},

	{ .sub_id = 0x1, .vpu_r0_2 = "OSD2", .vpu_r1 = "DI_MEM",
			.vpu_w0 = "VDIN1", .vpu_w1 = "DI"		},

	{ .sub_id = 0x2, .vpu_r0_2 = "VD1", .vpu_r1 = "DI_INP",
			.vpu_w0 = "VDIN_DOLBY", .vpu_w1 = "DI_SUB"	},

	{ .sub_id = 0x3, .vpu_r0_2 = "VD2", .vpu_r1 = "DI_CHAN2",
			.vpu_w0 = "VDIN_AFBCE", .vpu_w1 = "DI_AFBCE0"	},

	{ .sub_id = 0x4, .vpu_r0_2 = "OSD3", .vpu_r1 = "DI_SUB",
			.vpu_w0 = "DCNTR_GRID", .vpu_w1 = "DI_AFBCE1"	},

	{ .sub_id = 0x5, .vpu_r0_2 = "OSD4", .vpu_r1 = "DI_IF2",
			.vpu_w0 = "TCON1", .vpu_w1 = "ASIR_HPF"		},

	{ .sub_id = 0x6, .vpu_r0_2 = "VD3", .vpu_r1 = "DI_IF0",
			.vpu_w0 = "RDMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x7, .vpu_r0_2 = "MALI_AFBCD", .vpu_r1 = "DI_AFBCD",
			.vpu_w0 = "TCON2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x8, .vpu_r0_2 = "NPU READ", .vpu_r1 = "NULL",
			.vpu_w0 = "LDIM", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x9, .vpu_r0_2 = "VD4", .vpu_r1 = "NULL",
			.vpu_w0 = "VDIN2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xA, .vpu_r0_2 = "VD5", .vpu_r1 = "NULL",
			.vpu_w0 = "VPU_DMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xB, .vpu_r0_2 = "NOUSE", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xC, .vpu_r0_2 = "DCNTR_GRID", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xD, .vpu_r0_2 = "TCON_P1", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xE, .vpu_r0_2 = "TCON_P2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xF, .vpu_r0_2 = "VPU_SUBRD", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		}
};

static struct vpu_sub_desc vpu_sub_desc_s7d[] __initdata = {
	{ .sub_id = 0x0, .vpu_r0_2 = "OSD1", .vpu_r1 = "DI_IF1",
			.vpu_w0 = "NR_WR", .vpu_w1 = "NR_WR"		},

	{ .sub_id = 0x1, .vpu_r0_2 = "OSD2", .vpu_r1 = "DI_MEM",
			.vpu_w0 = "DI_WR", .vpu_w1 = "DI_WR"		},

	{ .sub_id = 0x2, .vpu_r0_2 = "VD1", .vpu_r1 = "DI_INP",
			.vpu_w0 = "DI_SUBAXI", .vpu_w1 = "DI_SUBAXI"	},

	{ .sub_id = 0x3, .vpu_r0_2 = "VD2", .vpu_r1 = "DI_CHAN2",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x4, .vpu_r0_2 = "VPU_DMA", .vpu_r1 = "DI_SUBAXI",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x5, .vpu_r0_2 = "RDMA", .vpu_r1 = "DI_IF2",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x6, .vpu_r0_2 = "NULL", .vpu_r1 = "DI_IF0",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x7, .vpu_r0_2 = "MALI_AFBCD_RD", .vpu_r1 = "NULL",
			.vpu_w0 = "VDIN1_WR", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x8, .vpu_r0_2 = "DI_IF1", .vpu_r1 = "NULL",
			.vpu_w0 = "RDMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x9, .vpu_r0_2 = "DI_MEM", .vpu_r1 = "NULL",
			.vpu_w0 = "VPU_DMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xA, .vpu_r0_2 = "DI_INP", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xB, .vpu_r0_2 = "DI_CHAN2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xC, .vpu_r0_2 = "DI_SUBAXI", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xD, .vpu_r0_2 = "DI_IF2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xE, .vpu_r0_2 = "DI_IF0", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},
};

static struct vpu_sub_desc vpu_sub_desc_s6[] __initdata = {
	{ .sub_id = 0x0, .vpu_r0_2 = "OSD1", .vpu_r1 = "DI_IF1",
			.vpu_w0 = "NR_WR", .vpu_w1 = "NR_WR"		},

	{ .sub_id = 0x1, .vpu_r0_2 = "OSD2", .vpu_r1 = "DI_MEM",
			.vpu_w0 = "DI_WR", .vpu_w1 = "DI_WR"		},

	{ .sub_id = 0x2, .vpu_r0_2 = "VD1", .vpu_r1 = "DI_INP",
			.vpu_w0 = "DI_SUBAXI", .vpu_w1 = "DI_SUBAXI"	},

	{ .sub_id = 0x3, .vpu_r0_2 = "VD2", .vpu_r1 = "DI_CHAN2",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x4, .vpu_r0_2 = "VPU_DMA", .vpu_r1 = "CONTP_RD",
			.vpu_w0 = "VDIN0_WR", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x5, .vpu_r0_2 = "RDMA", .vpu_r1 = "DI_IF2",
			.vpu_w0 = "VDIN1_WR", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x6, .vpu_r0_2 = "VIU2", .vpu_r1 = "DI_IF0",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x7, .vpu_r0_2 = "MALI_AFBCD_RD", .vpu_r1 = "DI_AFBCE",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x8, .vpu_r0_2 = "DI_IF1", .vpu_r1 = "NULL",
			.vpu_w0 = "RDMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x9, .vpu_r0_2 = "DI_MEM", .vpu_r1 = "NULL",
			.vpu_w0 = "VPU_DMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xA, .vpu_r0_2 = "DI_INP", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xB, .vpu_r0_2 = "DI_CHAN2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xC, .vpu_r0_2 = "CONTP_RD", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xD, .vpu_r0_2 = "DI_IF2", .vpu_r1 = "NULL",
			.vpu_w0 = "VDIN2_WR", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xE, .vpu_r0_2 = "DI_IF0", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xF, .vpu_r0_2 = "DI_AFBCE", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		}
};

static struct vpu_sub_desc vpu_sub_desc_t6d[] __initdata = {
	{ .sub_id = 0x0, .vpu_r0_2 = "OSD1", .vpu_r1 = "DI_IF1",
			.vpu_w0 = "VDIN0_WR", .vpu_w1 = "NR_WR"		},

	{ .sub_id = 0x1, .vpu_r0_2 = "OSD2", .vpu_r1 = "DI_MEM",
			.vpu_w0 = "VDIN1_WR", .vpu_w1 = "DI_WR"		},

	{ .sub_id = 0x2, .vpu_r0_2 = "VD1", .vpu_r1 = "DI_INP",
			.vpu_w0 = "NULL", .vpu_w1 = "DI_SUBAXI"	},

	{ .sub_id = 0x3, .vpu_r0_2 = "VD2", .vpu_r1 = "DI_CHAN2",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x4, .vpu_r0_2 = "NULL", .vpu_r1 = "DI_SUBAXI",
			.vpu_w0 = "DCGRD", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x5, .vpu_r0_2 = "NULL", .vpu_r1 = "DI_IF2",
			.vpu_w0 = "TCON_P1", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x6, .vpu_r0_2 = "NULL", .vpu_r1 = "DI_IF0",
			.vpu_w0 = "RDMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x7, .vpu_r0_2 = "MALI_AFBCD_RD", .vpu_r1 = "NULL",
			.vpu_w0 = "TCON_P2", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x8, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0x9, .vpu_r0_2 = "TCON_P1", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xA, .vpu_r0_2 = "RDMA_RD", .vpu_r1 = "NULL",
			.vpu_w0 = "DMA", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xB, .vpu_r0_2 = "DCGRD", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xC, .vpu_r0_2 = "TCON_P2", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xD, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xE, .vpu_r0_2 = "NULL", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},

	{ .sub_id = 0xF, .vpu_r0_2 = "DMA_RD", .vpu_r1 = "NULL",
			.vpu_w0 = "NULL", .vpu_w1 = "NULL"		},
};

static struct vpu_sub vpu0_r_t6w[] __initdata = {
	{ .id = 0x00, .name = VPU_PORT_OSD1		},
	{ .id = 0x01, .name = VPU_PORT_OSD2		},
	{ .id = 0x04, .name = VPU_PORT_OSD3		},
	{ .id = 0x07, .name = VPU_PORT_MALI_AFBCD_RD	},
	{ .id = 0x09, .name = VPU_PORT_TCON_P1		},
	{ .id = 0x0a, .name = VPU_PORT_RDMA_RD		},
	{ .id = 0x0c, .name = VPU_PORT_TCON_P2		},
	{ .id = 0x0f, .name = VPU_PORT_DMA_RD		},
};

static struct vpu_sub vpu0_w_t6w[] __initdata = {
	{ .id = 0x00, .name = VPU_PORT_VDIN0_WR		},
	{ .id = 0x01, .name = VPU_PORT_VDIN1_WR		},
	{ .id = 0x02, .name = VPU_PORT_VDIN_DOLBY	},
	{ .id = 0x03, .name = VPU_PORT_VDIN_AFBCE	},
	{ .id = 0x05, .name = VPU_PORT_TCON_P1		},
	{ .id = 0x06, .name = VPU_PORT_RDMA		},
	{ .id = 0x08, .name = VPU_PORT_LDIM		},
	{ .id = 0x0a, .name = VPU_PORT_DMA		},
};

static struct vpu_sub vpu1_r_t6w[] __initdata = {
	{ .id = 0x00, .name = VPU_PORT_DAE_RD0		},
	{ .id = 0x10, .name = VPU_PORT_DAE_RD1		},
	{ .id = 0x20, .name = VPU_PORT_DAE_RD2		},
	{ .id = 0x30, .name = VPU_PORT_DAE_RD3		},
	{ .id = 0x40, .name = VPU_PORT_DAE_RD4		},
	{ .id = 0x50, .name = VPU_PORT_DAE_RD5		},
	{ .id = 0x60, .name = VPU_PORT_DAE_RD6		},
	{ .id = 0x70, .name = VPU_PORT_DAE_RD7		},
	{ .id = 0x80, .name = VPU_PORT_DAE_RD8		},
	{ .id = 0x90, .name = VPU_PORT_DAE_RD9		},
	{ .id = 0xa0, .name = VPU_PORT_DAE_RD10		},
	{ .id = 0xb0, .name = VPU_PORT_DAE_RD11		},
	{ .id = 0xc0, .name = VPU_PORT_DAE_RD11		},
	{ .id = 0xd0, .name = VPU_PORT_DAE_RD11		},
	{ .id = 0x01, .name = VPU_PORT_INP_RD0		},
	{ .id = 0x11, .name = VPU_PORT_INP_RD0		},
	{ .id = 0x21, .name = VPU_PORT_INP_RD0		},
	{ .id = 0x41, .name = VPU_PORT_INP_RD1		},
	{ .id = 0x32, .name = VPU_PORT_RDMA_RD		},
};

static struct vpu_sub vpu1_w_t6w[] __initdata = {
	{ .id = 0x00, .name = VPU_PORT_DAE_WR0		},
	{ .id = 0x10, .name = VPU_PORT_DAE_WR1		},
	{ .id = 0x20, .name = VPU_PORT_DAE_WR2		},
	{ .id = 0x30, .name = VPU_PORT_DAE_WR3		},
	{ .id = 0x40, .name = VPU_PORT_DAE_WR4		},
	{ .id = 0x50, .name = VPU_PORT_DAE_WR5		},
	{ .id = 0x60, .name = VPU_PORT_DAE_WR6		},
	{ .id = 0x70, .name = VPU_PORT_DAE_WR7		},
	{ .id = 0x80, .name = VPU_PORT_DAE_WR8		},
	{ .id = 0x01, .name = VPU_PORT_INP_WR		},
	{ .id = 0x42, .name = VPU_PORT_RDMA_WR		},
};

static struct vpu_sub vpu2_r_t6w[] __initdata = {
	{ .id = 0x00, .name = VPU_PORT_NR_VFCD0		},
	{ .id = 0x10, .name = VPU_PORT_NR_VFCD0		},
	{ .id = 0x20, .name = VPU_PORT_NR_VFCD0		},
	{ .id = 0x01, .name = VPU_PORT_NR_VFCD1		},
	{ .id = 0x11, .name = VPU_PORT_NR_VFCD1		},
	{ .id = 0x21, .name = VPU_PORT_NR_VFCD1		},
	{ .id = 0x02, .name = VPU_PORT_NR_VFCD2		},
	{ .id = 0x12, .name = VPU_PORT_NR_VFCD2		},
	{ .id = 0x22, .name = VPU_PORT_NR_VFCD2		},
	{ .id = 0x03, .name = VPU_PORT_NR_VFCD3		},
	{ .id = 0x13, .name = VPU_PORT_NR_VFCD3		},
	{ .id = 0x23, .name = VPU_PORT_NR_VFCD3		},
};

static struct vpu_sub vpu2_w_t6w[] __initdata = {
	{ .id = 0x00, .name = VPU_PORT_VFCE_WRMIF2	},
	{ .id = 0x01, .name = VPU_PORT_VFCE_WRMIF2	},
	{ .id = 0x02, .name = VPU_PORT_VFCE_WRMIF2	},
	{ .id = 0x03, .name = VPU_PORT_VFCE_WRMIF2	},
	{ .id = 0x04, .name = VPU_PORT_DS_WRMIF		},
	{ .id = 0x05, .name = VPU_PORT_DS_WRMIF		},
	{ .id = 0x06, .name = VPU_PORT_WRMIF0		},
	{ .id = 0x07, .name = VPU_PORT_WRMIF0		},
	{ .id = 0x08, .name = VPU_PORT_WRMIF1		},
	{ .id = 0x09, .name = VPU_PORT_WRMIF1		},
	{ .id = 0x0a, .name = VPU_PORT_DMSQ_WR		},
	{ .id = 0x0b, .name = VPU_PORT_NR_WR0		},
	{ .id = 0x0c, .name = VPU_PORT_NR_WR1		},
};

static struct vpu_sub vpu3_r_t6w[] __initdata = {
	{ .id = 0x00, .name = VPU_PORT_MC_VFCD0		},
	{ .id = 0x10, .name = VPU_PORT_MC_VFCD0		},
	{ .id = 0x20, .name = VPU_PORT_MC_VFCD0		},
	{ .id = 0x01, .name = VPU_PORT_MC_VFCD1		},
	{ .id = 0x11, .name = VPU_PORT_MC_VFCD1		},
	{ .id = 0x21, .name = VPU_PORT_MC_VFCD1		},
	{ .id = 0x02, .name = VPU_PORT_MC_VFCD2		},
	{ .id = 0x12, .name = VPU_PORT_MC_VFCD2		},
	{ .id = 0x22, .name = VPU_PORT_MC_VFCD2		},
	{ .id = 0x03, .name = VPU_PORT_NR_RD0		},
	{ .id = 0x23, .name = VPU_PORT_NR_RD2		},
	{ .id = 0x33, .name = VPU_PORT_NR_RD3		},
	{ .id = 0x43, .name = VPU_PORT_NR_RD4		},
	{ .id = 0x53, .name = VPU_PORT_NR_RD5		},
	{ .id = 0x63, .name = VPU_PORT_NR_RD6		},
	{ .id = 0x73, .name = VPU_PORT_NR_RD7		},
	{ .id = 0x83, .name = VPU_PORT_NR_RD8		},
	{ .id = 0x93, .name = VPU_PORT_NR_RD9		},
	{ .id = 0xa3, .name = VPU_PORT_NR_RD10		},
	{ .id = 0xb3, .name = VPU_PORT_NR_RD10		},
	{ .id = 0xc3, .name = VPU_PORT_NR_RD10		},
	{ .id = 0x04, .name = VPU_PORT_MC_RD0		},
	{ .id = 0x14, .name = VPU_PORT_MC_RD1		},
	{ .id = 0x24, .name = VPU_PORT_MC_RD2		},
	{ .id = 0x34, .name = VPU_PORT_MC_RD3		},
	{ .id = 0x05, .name = VPU_PORT_MMU		},
};
#endif

int __init dmc_find_port_sub(struct dmc_monitor *mon, int cpu_type)
{
#define VPU_DATA_RW_CHIP_INIT(id, chip)	\
	{										\
		mon->vpu_port_v2.vpu##id##_r_num = ARRAY_SIZE(vpu##id##_r_##chip);	\
		vpu##id##_r = vpu##id##_r_##chip;					\
		mon->vpu_port_v2.vpu##id##_w_num = ARRAY_SIZE(vpu##id##_w_##chip);	\
		vpu##id##_w = vpu##id##_w_##chip;					\
	}

#define VPU_DATA_R_CHIP_INIT(id, chip)							\
	{										\
		mon->vpu_port_v2.vpu##id##_r_num = ARRAY_SIZE(vpu##id##_r_##chip);	\
		vpu##id##_r = vpu##id##_r_##chip;					\
	}

#define VPU_DATA_W_CHIP_INIT(id, chip)							\
	{										\
		mon->vpu_port_v2.vpu##id##_w_num = ARRAY_SIZE(vpu##id##_w_##chip);	\
		vpu##id##_w = vpu##id##_w_##chip;					\
	}

#define VPU_DATA_INIT(id)									\
	{											\
		if (mon->vpu_port_v2.vpu##id##_r_num) {						\
			mon->vpu_port_v2.vpu##id##_r =						\
						kcalloc(mon->vpu_port_v2.vpu##id##_r_num,	\
								sizeof(struct vpu_sub),		\
								GFP_KERNEL);			\
			if (!mon->vpu_port_v2.vpu##id##_r || !vpu##id##_r)			\
				return -ENOMEM;							\
			memcpy(mon->vpu_port_v2.vpu##id##_r, vpu##id##_r,			\
				sizeof(struct vpu_sub) * mon->vpu_port_v2.vpu##id##_r_num);	\
		}										\
		if (mon->vpu_port_v2.vpu##id##_w_num) {						\
			mon->vpu_port_v2.vpu##id##_w =						\
						kcalloc(mon->vpu_port_v2.vpu##id##_w_num,	\
								sizeof(struct vpu_sub),		\
								GFP_KERNEL);			\
			if (!mon->vpu_port_v2.vpu##id##_w || !vpu##id##_w)			\
				return -ENOMEM;							\
			memcpy(mon->vpu_port_v2.vpu##id##_w, vpu##id##_w,			\
				sizeof(struct vpu_sub) * mon->vpu_port_v2.vpu##id##_w_num);	\
		}										\
	}

	struct vpu_sub_desc *desc = NULL;
	struct vpu_sub *vpu0_r = NULL;
	struct vpu_sub *vpu0_w = NULL;
	struct vpu_sub *vpu1_r = NULL;
	struct vpu_sub *vpu1_w = NULL;
	struct vpu_sub *vpu2_r = NULL;
	struct vpu_sub *vpu2_w = NULL;
	struct vpu_sub *vpu3_r = NULL;
	struct vpu_sub *vpu3_w = NULL;
	int desc_size = -EINVAL;
	static int initialized;

	if (initialized)
		return 0;
	initialized = 1;

	mon->vpu_port_num = 0;
	mon->vpu_port = NULL;

	switch (cpu_type) {
#ifndef CONFIG_AMLOGIC_REMOVE_OLD
	case DMC_TYPE_TXL:
	case DMC_TYPE_TXLX:
	case DMC_TYPE_TXHD:
		desc = vpu_sub_desc_txlx;
		desc_size = ARRAY_SIZE(vpu_sub_desc_txlx);
		break;
	case DMC_TYPE_TL1:
		desc = vpu_sub_desc_tl1;
		desc_size = ARRAY_SIZE(vpu_sub_desc_tl1);
		break;
#endif
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
	case DMC_TYPE_G12A:
	case DMC_TYPE_G12B:
	case DMC_TYPE_SM1:
		desc = vpu_sub_desc_sm1;
		desc_size = ARRAY_SIZE(vpu_sub_desc_sm1);
		break;
	case DMC_TYPE_TM2:
		desc = vpu_sub_desc_tm2;
		desc_size = ARRAY_SIZE(vpu_sub_desc_tm2);
		break;
	case DMC_TYPE_T5:
		desc = vpu_sub_desc_t5;
		desc_size = ARRAY_SIZE(vpu_sub_desc_t5);
		break;
	case DMC_TYPE_T5D:
		desc = vpu_sub_desc_t5d;
		desc_size = ARRAY_SIZE(vpu_sub_desc_t5d);
		break;
	case DMC_TYPE_T7:
		desc = vpu_sub_desc_t7;
		desc_size = ARRAY_SIZE(vpu_sub_desc_t7);
		break;
	case DMC_TYPE_T3:
		desc = vpu_sub_desc_t3;
		desc_size = ARRAY_SIZE(vpu_sub_desc_t3);
		break;
#endif
	case DMC_TYPE_S4:
		desc = vpu_sub_desc_s4;
		desc_size = ARRAY_SIZE(vpu_sub_desc_s4);
		break;
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
	case DMC_TYPE_SC2:
		desc = vpu_sub_desc_sc2;
		desc_size = ARRAY_SIZE(vpu_sub_desc_sc2);
		break;
	case DMC_TYPE_T5W:
	case DMC_TYPE_T5M:
		desc = vpu_sub_desc_t5w;
		desc_size = ARRAY_SIZE(vpu_sub_desc_t5w);
		break;
	case DMC_TYPE_S5:
		desc = vpu_sub_desc_s5;
		desc_size = ARRAY_SIZE(vpu_sub_desc_s5);
		break;
	case DMC_TYPE_S7D:
		desc = vpu_sub_desc_s7d;
		desc_size = ARRAY_SIZE(vpu_sub_desc_s7d);
		break;
	case DMC_TYPE_S6:
		desc = vpu_sub_desc_s6;
		desc_size = ARRAY_SIZE(vpu_sub_desc_s6);
		break;
	case DMC_TYPE_T6D:
		desc = vpu_sub_desc_t6d;
		desc_size = ARRAY_SIZE(vpu_sub_desc_t6d);
		break;
	case DMC_TYPE_T6W:
		VPU_DATA_RW_CHIP_INIT(0, t6w);
		VPU_DATA_RW_CHIP_INIT(1, t6w);
		VPU_DATA_RW_CHIP_INIT(2, t6w);
		VPU_DATA_R_CHIP_INIT(3, t6w);
		break;
#endif
	default:
		return -EINVAL;
	}
	/* using once */
	if (desc) {
		mon->vpu_port = kcalloc(desc_size, sizeof(struct vpu_sub_desc), GFP_KERNEL);
		if (!mon->vpu_port)
			return -ENOMEM;
		memcpy(mon->vpu_port, desc, sizeof(struct vpu_sub_desc) * desc_size);
		mon->vpu_port_num = desc_size;
	} else {
		VPU_DATA_INIT(0);
		VPU_DATA_INIT(1);
		VPU_DATA_INIT(2);
		VPU_DATA_INIT(3);
	}

	return 0;
}

char *vpu_to_sub_port(char *name, char rw, int sid, char *id_str)
{
	int i;
	int size;
#define VPU_TO_SUB(idx)										\
	{											\
		if (!strncmp(name, "VPU"#idx, 4)) {						\
			if (rw == 'r')								\
				for (i = 0; i < dmc_mon->vpu_port_v2.vpu##idx##_r_num; i++) {	\
					if (dmc_mon->vpu_port_v2.vpu##idx##_r[i].id == sid)	\
						return dmc_mon->vpu_port_v2.vpu##idx##_r[i].name;\
				}								\
			else									\
				for (i = 0; i < dmc_mon->vpu_port_v2.vpu##idx##_w_num; i++) {	\
					if (dmc_mon->vpu_port_v2.vpu##idx##_w[i].id == sid)	\
						return dmc_mon->vpu_port_v2.vpu##idx##_w[i].name;\
				}								\
		}										\
	}

	if (!dmc_mon->vpu_port) {
		sid &= 0xff;
		size = dmc_mon->vpu_port_v2.vpu0_r_num + dmc_mon->vpu_port_v2.vpu0_w_num +
		       dmc_mon->vpu_port_v2.vpu1_r_num + dmc_mon->vpu_port_v2.vpu1_w_num +
		       dmc_mon->vpu_port_v2.vpu2_r_num + dmc_mon->vpu_port_v2.vpu2_w_num +
		       dmc_mon->vpu_port_v2.vpu3_r_num + dmc_mon->vpu_port_v2.vpu3_w_num;
		if (!size)
			return id_str;

		VPU_TO_SUB(0);
		VPU_TO_SUB(1);
		VPU_TO_SUB(2);
		VPU_TO_SUB(3);
		return id_str;
	}

	sid &= 0x1f;
	if (!dmc_mon->vpu_port || sid >= dmc_mon->vpu_port_num) {
		return id_str;
	}

	if (dmc_mon->vpu_port[sid].sub_id == sid) {
		if (!strncmp(name, "VPU0", 4)) {
			if (rw == 'r')
				return dmc_mon->vpu_port[sid].vpu_r0_2;
			else
				return dmc_mon->vpu_port[sid].vpu_w0;
		} else if (!strncmp(name, "VPU1", 4)) {
			if (rw == 'r')
				return dmc_mon->vpu_port[sid].vpu_r1;
			else
				return dmc_mon->vpu_port[sid].vpu_w1;
		} else if (!strncmp(name, "VPU2", 4)) {
			if (rw == 'r')
				return dmc_mon->vpu_port[sid].vpu_r0_2;
			else
				return id_str;
		} else if (!strncmp(name, "VPU3", 4)) {
			if (rw == 'r')
				return dmc_mon->vpu_port[sid].vpu_r0_2;
			else
				return id_str;
		}
	}
	return id_str;
}
