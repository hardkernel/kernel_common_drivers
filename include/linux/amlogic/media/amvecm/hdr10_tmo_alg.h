/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <uapi/amlogic/amvecm_ext.h>

#ifndef HDR10_TMO
#define HDR10_TMO

#ifndef MAX
#define MAX(a, b) ({ \
			const typeof(a) _a = a; \
			const typeof(b) _b = b; \
			_a > _b ? _a : _b; \
		})
#endif // MAX

#ifndef MIN
#define MIN(a, b) ({ \
			const typeof(a) _a = a; \
			const typeof(b) _b = b; \
			_a < _b ? _a : _b; \
		})
#endif // MIN

#ifndef ABS
#define ABS(x) ({ \
			const typeof(x) _x = x; \
			_x > 0 ? _x : -_x; \
		})
#endif

#define SOFT_CLIP 0

struct aml_hdr_prm_s {
	u32 hdr_ogain_lut[149];
	u32 hdr_cgain_lut[65];
	u32 hdr_ogain_lut1[149];
};

struct aml_special_pattern_s {
	int special_hist_idx[2];
	int special_pattern_status;
};

struct aml_tmo_reg_sw {
	int tmo_en;                             //u1
	int tmo_display_e;                      //u10
	int tmo_display_e_bc;                   //u10
	int tmo_max_th1;                        //u10
	int tmo_max_th2;                        //u10
	int tmo_max_th3;                        //u10
	int tmo_max_th4;                        //u10
	int tmo_highlight;                      //u10: control overexposure level
	int tmo_highlight_bc;                   //u10
	int tmo_ratio;                          //u10
	int tmo_light_th;                       //u7
	int tmo_hist_th;                        //u8
	int tmo_highlight_th1;                  //u10
	int tmo_highlight_th2;                  //u10
	int tmo_display_adj;                    //u7
	int tmo_avg_th;                         //u10
	int tmo_avg_adj;                        //u8: 0 ~ 256
	int tmo_hl0;                            //u7: 0 ~ 128
	int tmo_hl1;                            //u7: 0 ~ 128
	int tmo_hl2;                            //u7: 0 ~ 128
	int tmo_hl3;                            //u7: 0 ~ 128
	int tmo_hl0_bc;                         //u7: 0 ~ 128
	int tmo_hl1_bc;                         //u7: 0 ~ 128
	int tmo_hl2_bc;                         //u7: 0 ~ 128
	int tmo_hl3_bc;                         //u7: 0 ~ 128
	int tmo_pnum_th;                        //u16
	int tmo_thold1;                         //u10
	int tmo_thold2;                         //u10
	int tmo_thold3;                         //u10
	int tmo_thold4;                         //u10
	int tmo_middle_th;                      //u10
	int tmo_middle_a;                       //u7
	int tmo_middle_a_bc;                    //u7
	int tmo_middle_a_adj;                   //u10
	int tmo_middle_b;                       //u7
	int tmo_middle_s;                       //u7
	int tmo_low_adj;                        //u7
	int tmo_low_adj_bc;                     //u7
	int tmo_high_en;                        //u2
	int tmo_olut_bit_mode;                  //u1, 0: 12 bits, 1: 16 bits
	int tmo_high_adj1;                      //u7
	int tmo_high_adj2;                      //u7
	int tmo_high_maxdiff;                   //u7
	int tmo_high_mindiff;                   //u7
	int tmo_curve_smo_mode;                 //u2
	int tmo_soft_clip_th;                   //u10
	int tmo_r0_32;                          //u10
	int tmo_r1_32;                          //u10
	int tmo_y_highlight_ratio;                  //u10
	int tmo_ratio_sat;                      //u11
	int tmo_avg_lum_ratio;                  //u10
	int tmo_alpha;                          //u8
	int tmo_model;                          //u2
	int tmo_highlight_lut_gain;             //u8
	int tmo_highlight_lut_clip;             //u12
	int tmo_force_xy_point_en;              //u1
	int tmo_highlight_lut_x;                //ogian x index value 0~148
	int tmo_highlight_lut_y;                //u12
	int tmo_highlight_lut_pst_gain_en;      //u1
	int tmo_highlight_lut_pst_gain;         //u8
	int tmo_highlight_y_clip_pos_low;       //u6
	int tmo_highlight_y_clip_pos_high;      //u6
	int tmo_highlight_y_clip_pos_sup_high;  //u6
	int tmo_highlight_y_clip_en;            //u1
	int tmo_fcurv_low_th;                   //u8
	int tmo_fcurv_mid1_th;                  //u8
	int tmo_fcurv_mid2_th;                  //u8
	int tmo_fcurv_high1_th;                 //u8
	int tmo_fcurv_high2_th;                 //u8
	int tmo_fcurv_high3_th;                 //u8
	int tmo_avg_lum_th0;                    //u10
	int tmo_avg_lum_th1;                    //u10
	int tmo_avg_lum_th2;                    //u10
	int tmo_avg_lum_alpha0;                 //u8: 0 ~ 256
	int tmo_avg_lum_alpha1;                 //u8
	int tmo_avg_lum_alpha2;                 //u8
	int tmo_avg_lum_alpha3;                 //u8
	int tmo_fcurve_adj_en;                  //u1
	int *tmo_fcurve_adj_sync_idx;           //ogian x index value 0~148, suggest 90~120
	int *tmo_fcurve_adj_gain;               //u9
	int tmo_avg_lum_alpha_sc_flag;          //u8
	int tmo_sc_hist_th;                     //u10
	int tmo_sc_maxl_th;                     //u10
	int (*tmo_fcurv_clip_val)[4];           //u12
	int tmo_single_bin_th;                  //special bin percentage 0 ~ 100
	int tmo_full_black_th;                  //u10
	int tmo_full_white_th0;                 //u10
	int tmo_full_white_th1;                 //u10
	int tmo_special_pat1_th;                //u10
	int tmo_special_pat2_th;                //u10
	int *tmo_force_ootf1_mode;
	int *tmo_force_ootf1_val;

	unsigned int *tmo_eo_lut;
	int *tmo_oo_init_lut;                   //u10

	/*param for tmo alg*/
	int w;
	int h;
	int hdr_ogain_shift;
	int hdr_hist_sel;
	void (*pre_hdr10_tmo_alg)(struct aml_hdr_prm_s *hdr_reg,
		struct aml_tmo_reg_sw *tmo_reg,
		int *hdr_hist,
		unsigned short *vpp_hist
		);
};

struct aml_tmo_reg_sw *tmo_fw_param_get(void);
#endif
