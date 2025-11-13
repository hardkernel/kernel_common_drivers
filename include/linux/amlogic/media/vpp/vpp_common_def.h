/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#ifndef __VPP_COMMON_DEF_H__
#define __VPP_COMMON_DEF_H__

#include <linux/amlogic/media/vfm/vframe.h>
#include <uapi/amlogic/vpp_common_def_ext.h>

#define _VPP_TYPE  'V'

enum vpp_vd_path_e {
	EN_VD1_PATH = 0,
	EN_VD2_PATH,
	EN_VD3_PATH,
	EN_VD_PATH_MAX,
};

enum vpp_vf_top_e {
	EN_VF_TOP0 = 0,
	EN_VF_TOP1,
	EN_VF_TOP2,
	EN_VF_TOP_MAX,
};

enum vpp_dnlp_rt_e {
	EN_DNLP_RT_0S = 0,
	EN_DNLP_RT_1S = 6,
	EN_DNLP_RT_2S,
	EN_DNLP_RT_4S,
	EN_DNLP_RT_8S,
	EN_DNLP_RT_16S,
	EN_DNLP_RT_32S,
	EN_DNLP_RT_64S,
	EN_DNLP_RT_FREEZE,
};

enum vpp_dnlp_rl_e {
	EN_DNLP_RL_01 = 1, /*max_contrast = 1.0625x*/
	EN_DNLP_RL_02,     /*max_contrast = 1.1250x*/
	EN_DNLP_RL_03,     /*max_contrast = 1.1875x*/
	EN_DNLP_RL_04,     /*max_contrast = 1.2500x*/
	EN_DNLP_RL_05,     /*max_contrast = 1.3125x*/
	EN_DNLP_RL_06,     /*max_contrast = 1.3750x*/
	EN_DNLP_RL_07,     /*max_contrast = 1.4375x*/
	EN_DNLP_RL_08,     /*max_contrast = 1.5000x*/
	EN_DNLP_RL_09,     /*max_contrast = 1.5625x*/
	EN_DNLP_RL_10,     /*max_contrast = 1.6250x*/
	EN_DNLP_RL_11,     /*max_contrast = 1.6875x*/
	EN_DNLP_RL_12,     /*max_contrast = 1.7500x*/
	EN_DNLP_RL_13,     /*max_contrast = 1.8125x*/
	EN_DNLP_RL_14,     /*max_contrast = 1.8750x*/
	EN_DNLP_RL_15,     /*max_contrast = 1.9375x*/
	EN_DNLP_RL_16,     /*max_contrast = 2.0000x*/
};

enum vpp_dnlp_ext_e {
	EN_DNLP_EXT_00 = 0, /*weak*/
	EN_DNLP_EXT_01,
	EN_DNLP_EXT_02,
	EN_DNLP_EXT_03,
	EN_DNLP_EXT_04,
	EN_DNLP_EXT_05,
	EN_DNLP_EXT_06,
	EN_DNLP_EXT_07,
	EN_DNLP_EXT_08,
	EN_DNLP_EXT_09,
	EN_DNLP_EXT_10,
	EN_DNLP_EXT_11,
	EN_DNLP_EXT_12,
	EN_DNLP_EXT_13,
	EN_DNLP_EXT_14,
	EN_DNLP_EXT_15,
	EN_DNLP_EXT_16,     /*strong*/
};

enum vpp_overscan_input_e {
	EN_INPUT_INVALID = -1,
	EN_INPUT_TV = 0,
	EN_INPUT_AV1,
	EN_INPUT_AV2,
	EN_INPUT_YPBPR1,
	EN_INPUT_YPBPR2,
	EN_INPUT_HDMI1,
	EN_INPUT_HDMI2,
	EN_INPUT_HDMI3,
	EN_INPUT_HDMI4,
	EN_INPUT_VGA,
	EN_INPUT_MPEG,
	EN_INPUT_DTV,
	EN_INPUT_SVIDEO,
	EN_INPUT_IPTV,
	EN_INPUT_DUMMY,
	EN_INPUT_SPDIF,
	EN_INPUT_ADTV,
	EN_INPUT_MAX,
};

enum vpp_overscan_timing_e {
	EN_TIMING_SD_480 = 0,
	EN_TIMING_SD_576,
	EN_TIMING_HD,
	EN_TIMING_FHD,
	EN_TIMING_UHD,
	EN_TIMING_NTST_M,
	EN_TIMING_NTST_443,
	EN_TIMING_PAL_I,
	EN_TIMING_PAL_M,
	EN_TIMING_PAL_60,
	EN_TIMING_PAL_CN,
	EN_TIMING_SECAM,
	EN_TIMING_NTSC_50,
	EN_TIMING_MAX,
};

struct vpp_vf_param_s {
	unsigned int sps_h_en;
	unsigned int sps_v_en;
	unsigned int sps_w_in;
	unsigned int sps_h_in;
	unsigned int cm_in_w;
	unsigned int cm_in_h;
};

/*Common struct*/
struct vpp_cabc_aad_param_s {
	unsigned int length;
	union {
		void *cabc_aad_param_ptr;
		long long cabc_aad_param_ptr_len;
	};
};

struct vpp_aad_param_s {
	int aad_param_cabc_aad_en;
	int aad_param_aad_en;
	int aad_param_tf_en;
	int aad_param_force_gain_en;
	int aad_param_sensor_mode;
	int aad_param_mode;
	int aad_param_dist_mode;
	int aad_param_tf_alpha;
	int aad_param_sensor_input[3];
	struct vpp_cabc_aad_param_s db_LUT_Y_gain;
	struct vpp_cabc_aad_param_s db_LUT_RG_gain;
	struct vpp_cabc_aad_param_s db_LUT_BG_gain;
	struct vpp_cabc_aad_param_s db_gain_lut;
	struct vpp_cabc_aad_param_s db_xy_lut;
};

struct vpp_cabc_param_s {
	int cabc_param_cabc_en;
	int cabc_param_hist_mode;
	int cabc_param_tf_en;
	int cabc_param_sc_flag;
	int cabc_param_bl_map_mode;
	int cabc_param_bl_map_en;
	int cabc_param_temp_proc;
	int cabc_param_max95_ratio;
	int cabc_param_hist_blend_alpha;
	int cabc_param_init_bl_min;
	int cabc_param_init_bl_max;
	int cabc_param_tf_alpha;
	int cabc_param_sc_hist_diff_thd;
	int cabc_param_sc_apl_diff_thd;
	int cabc_param_patch_bl_th;
	int cabc_param_patch_on_alpha;
	int cabc_param_patch_bl_off_th;
	int cabc_param_patch_off_alpha;
	struct vpp_cabc_aad_param_s db_o_bl_cv;
	struct vpp_cabc_aad_param_s db_maxbin_bl_cv;
};

struct vpp_lut3d_path_s {
	enum vpp_lut3d_data_type_e data_type;
	unsigned char data_count;
	unsigned char path_length;
	unsigned char *path;
};

/*Functions*/
void vpp_vf_refresh(struct vframe_s *vf, struct vframe_s *rpt_vf);
void vpp_vf_proc(struct vframe_s *vf,
	struct vframe_s *toggle_vf,
	struct vpp_vf_param_s *vf_param,
	int flags,
	enum vpp_vd_path_e vd_path,
	enum vpp_vf_top_e vpp_top);

#endif
#endif

