/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef FGRAIN_H
#define FGRAIN_H
//ary #include "c_stimulus.h"

//#include "register.h"

#define FG_REG_OFFSET (0x80)		//ary addr change *4
enum FG_SRC_SEL_t {
	FG_NR0 = 0,
	FG_NR1,
	FG_NR2,
	FG_NR3,
	FG_VD1,
	FG_VD2,
	FG_MC0,
	FG_MC1
};

enum FG_SRC_FMT_t {
	FG_YUV444 = 0,
	FG_YUV422,
	FG_YUV420
};

struct FGRAIN_t {
	u32 id;
	enum FG_SRC_SEL_t path_sel;
	enum FG_SRC_FMT_t fmt;
	//u32 hsize;
	//u32 vsize;
	u32 dma_table_bigend;
	u64 dma_mem_base0;
	u64 dma_mem_base1;
	u64 dma_mem_base2;
	u64 dma_mem_base3;
	u32 dma_ini[2];
	u32 src_8bit_mst;//src 8bit in msb
	u32 src_8bit;//src 8bit
	u32 reg_mode;//1 use cbus set size info
	u32 slc_num;
	u32 slc0_win_hbgn;
	u32 slc0_win_hend;
	u32 slc1_win_hbgn;
	u32 slc1_win_hend;
	u32 slc2_win_hbgn;
	u32 slc2_win_hend;
	u32 slc3_win_hbgn;
	u32 slc3_win_hend;
	u32 slc_win_vbgn;
	u32 slc_win_vend;
	u32 rev_mode;
};

void film_grain_uint(void);
void film_grain_init(struct FGRAIN_t *fgrain);
void film_grain_cfg(struct FGRAIN_t *fgrain, struct vframe_s *vf, uint cnt);

#endif
