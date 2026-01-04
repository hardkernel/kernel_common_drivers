/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef AA_PPS__H
#define AA_PPS__H

#include "dpss_param.h"

// Maximum
#ifndef MAX
#define MAX(x1, x2) ((x1) > (x2) ? (x1) : (x2))
#endif

// Minimum
#ifndef MIN
#define MIN(x1, x2) ((x1) < (x2) ? (x1) : (x2))
#endif

// Absolute Value
#ifndef ABS
#define ABS(X) ((X) > 0 ? (X) : -(X))
#endif

// Absolute Difference
#ifndef ADIF
#define ADIF(x1, x2) ({ \
	typeof(x1) _x1 = (x1); \
	typeof(x2) _x2 = (x2); \
	(_x1 > _x2) ? (_x1 - _x2) : (_x2 - _x1); \
})
#endif

struct AA_PPS_TOP_TYPE {
	int frm_hsize_in; //13 bits source pic hsize
	int frm_vsize_in; //13 bits source pic vsize
	int frm_hsize_out; //13 bits source pic hsize
	int frm_vsize_out; //13 bits source pic vsize

	int inst_sel;//reg cfg---0:aa_pps 1:lcevc
	int apply_mode; //0:zoom up 2path out 1:zoom down 2path out 2:zoom up/down 1path out
	int ds_mode;//0:aa mode 1:pps mode
	int slc_mode;//0:frame 1:slice
	int slc_num;//2/4 slice

	int slc_act_out_bgn[4];//13 bits slice act out bgn
	int slc_act_out_end[4];//13 bits slice act out end

	int slc_in_bgn[4];//13 bits slice in bgn
	int slc_in_end[4];//13 bits slice in end
	int aa_pad;
	//int vbe_slc_in_bgn[4] ; //13 bits slice in bgn
	//int vbe_slc_in_end[4] ; //13 bits slice in end
	//int slc_in_bgn_r16[4] ; //13 bits slice in bgn
	//int slc_in_end_r16[4] ; //13 bits slice in end
	bool pps_en;
	bool di2pps_en;
};

void set_aa_pps_coef(u32 scale_coef_idx,
	u32 scale_coef,
	int hor_tap_num,
	int ver_tap_num);

//void set_aa_pps_hor_slice (int *reg_init_inte,int *reg_init_phase,int
/* *slc_cut_bgn,int *slc_cut_end,*/
/*int *slc_size_out,int slc_num,int slc_bgn,int slc_end,int slc_ovlp_lft,*/
//int slc_ovlp_rgt,int frm_size_out,int reg_sc_ini_integer,
//int reg_sc_ini_phase);

void set_aa_pps_hor_slice(int *slc_in_bgn,
	int *slc_in_end,
	int *reg_init_inte,
	int *reg_init_phase,
	int bord_bgn,
	int bord_end,
	int slc_out_bgn,
	int slc_out_end,
	int frm_size_in,
	int reg_sc_ini_integer,
	int reg_sc_ini_phase,
	int phase_step);

void set_aa_pps_sc_mux_sel(int *reg_sc_mux_sel,
	int *reg_vds_tap_num,
	int slc_hsize_in[4],
	int slc_hsize_out[4],
	int frm_hsize_in,
	int frm_hsize_out,
	int slc_mode,
	int buf_depth);

void aa_pps_top(struct AA_PPS_TOP_TYPE *aa_pps_top);

#endif/*AA_PPS__H*/
