/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __VPQ_TABLE_LOGIC_H__
#define __VPQ_TABLE_LOGIC_H__

#include <linux/amlogic/media/vpq/vpq_cmd.h>
#include "vpq_drv.h"

// single curve length
#define CURVE_LEN_SAT_BY_Y          (9)
#define CURVE_LEN_LUMA_BY_HUE       (32)
#define CURVE_LEN_SAT_BY_HS         (32)
#define CURVE_LEN_HUE_BY_HUE        (32)
#define CURVE_LEN_HUE_BY_HY         (32)
#define CURVE_LEN_HUE_BY_HS         (32)
#define CURVE_LEN_SAT_BY_HY         (32)
#define CURVE_LEN_CM                (32)

// each kind curve number
#define CURVE_NUM_SAT_BY_Y          (1)
#define CURVE_NUM_LUMA_BY_HUE       (1)
#define CURVE_NUM_SAT_BY_HS         (3)
#define CURVE_NUM_HUE_BY_HUE        (1)
#define CURVE_NUM_HUE_BY_HY         (5)
#define CURVE_NUM_HUE_BY_HS         (5)
#define CURVE_NUM_SAT_BY_HY         (5)

struct vpq_pq_en_ctrl_s {
	unsigned char vadj1_en;
	unsigned char vd1_ctrst_en;
	unsigned char vadj2_en;
	unsigned char post_ctrst_en;
	unsigned char pregamma_en;
	unsigned char gamma_en;
	unsigned char wb_en;
	unsigned char dnlp_en;
	unsigned char lc_en;
	unsigned char blue_stretch_en;
	unsigned char black_ext_en;
	unsigned char chroma_cor_en;
	unsigned char sharpness0_en;
	unsigned char sharpness1_en;
	unsigned char cm_en;
	unsigned char lut3d_en;
	unsigned char dejaggy_sr0_en;
	unsigned char dejaggy_sr1_en;
	unsigned char dering_sr0_en;
	unsigned char dering_sr1_en;
};

struct vpq_pq_state_s {
	unsigned char pq_en;
	struct vpq_pq_en_ctrl_s pq_cfg;
};

enum vpq_overscan_timing_e {
	VPQ_TIMING_SD_480 = 0,
	VPQ_TIMING_SD_576,
	VPQ_TIMING_HD,
	VPQ_TIMING_FHD,
	VPQ_TIMING_UHD,
	VPQ_TIMING_NTST_M,
	VPQ_TIMING_NTST_443,
	VPQ_TIMING_PAL_I,
	VPQ_TIMING_PAL_M,
	VPQ_TIMING_PAL_60,
	VPQ_TIMING_PAL_CN,
	VPQ_TIMING_SECAM,
	VPQ_TIMING_NTSC_50,
	VPQ_TIMING_MAX,
};

struct vpq_overscan_data_s {
	unsigned int src_timing;
	unsigned int value1;
	unsigned int value2;
	unsigned int reserved1;
	unsigned int reserved2;
};

struct vpq_gamma_table_s {
	unsigned short *r_data;
	unsigned short *g_data;
	unsigned short *b_data;
};

void vpq_table_init(struct vpq_dev_s *pdev);
void vpq_table_deinit(void);
int vpq_set_pq_table_version(struct vpq_table_ver_info_s *pdata);
int vpq_set_default_pq_table(struct vpq_table_bin_param_s *pdata);
int vpq_set_nonstandard_timing_map(unsigned char value,
	struct vpq_nonstandard_timing_map_s *pdata);
int vpq_set_pq_module_cfg(struct vpq_pq_module_cfg_s *pdata);
void vpq_get_pq_module_status(struct vpq_pq_state_s *pdata);
enum vpq_chip_class_e vpq_get_chip_type(void);
enum vpq_chip_e vpq_get_chip_id(void);
int vpq_set_brightness(int value);
int vpq_set_contrast(int value);
int vpq_set_saturation(int value);
int vpq_set_hue(int value);
int vpq_set_sharpness(int value);
int vpq_set_brightness_post(int value);
int vpq_set_contrast_post(int value);
int vpq_set_saturation_post(int value);
int vpq_set_hue_post(int value);
int vpq_set_pq_module_status(enum vpq_module_e module, int status);
int vpq_get_gamma_table_point(void);
int vpq_set_gamma_table(struct vpq_gamma_table_s *pdata);
int vpq_set_rgb_ogo(struct vpq_rgb_ogo_s *pdata);
int vpq_set_matrix_param(struct vpq_mtrx_info_s *pdata);
int vpq_set_color_base(unsigned char value);
int vpq_set_color_customize(struct vpq_cms_s *pdata);
int vpq_set_dnlp_mode(unsigned char value);
int vpq_set_csc_type(int value);
int vpq_get_csc_type(void);
int vpq_set_3dlut_data(struct vpq_lut3d_table_s *pdata);
int vpq_get_hist_avg(struct vpq_hist_ave_s *pdata);
int vpq_get_histogram(struct vpq_hist_param_s *pdata);
int vpq_get_hdr_histogram(struct vpq_hdr_hist_param_s *pdata);
int vpq_set_lc_mode(unsigned char value);
int vpq_set_hdr_tmo_mode(unsigned char value);
int vpq_set_hdr_tmo(struct vpq_hdr_lut_s *pdata);
int vpq_set_hdr_oetf(struct vpq_hdr_lut_s *pdata);
int vpq_set_hdr_eotf(struct vpq_hdr_lut_s *pdata);
int vpq_set_hdr_cgain(struct vpq_hdr_lut_s *pdata);
int vpq_set_aipq_mode(unsigned char value);
int vpq_set_aisr_mode(unsigned char value);
enum vpq_color_primary_e vpq_get_color_primary(void);
int vpq_get_hdr_metadata(struct vpq_hdr_metadata_s *pdata);
int vpq_set_black_stretch(unsigned char value);
int vpq_set_blue_stretch(unsigned char value);
int vpq_set_chroma_coring(unsigned char value);
int vpq_set_eys_protect(struct vpq_eye_protect_s *pdata);
int vpq_set_cabc(void);
int vpq_set_add(void);
int vpq_set_overscan_data(unsigned int length, struct vpq_overscan_data_s *pdata);
int vpq_set_pc_mode(int value);
int vpq_get_pc_mode(void);
int vpq_set_color_primary_status(int value);
int vpq_set_color_primary(struct vpq_color_primary_s *pdata);

int vpq_set_nr(unsigned char value);
int vpq_set_deblock(unsigned char value);
int vpq_set_demosquito(unsigned char value);
int vpq_set_smoothplus_mode(unsigned char value);
int vpq_set_di_param(void);

/* DPSS Start */
int vpq_set_nr_dpss(struct vpq_dnr_param_s *pdata);
int vpq_set_deblock_dpss(struct vpq_dblk_param_s *pdata);
int vpq_set_demosquito_dpss(struct vpq_demosquito_param_s *pdata);
int vpq_set_smoothplus_dpss(struct vpq_dct_param_s *pdata);
int vpq_set_xlr_dpss(struct vpq_xlr_param_s *pdata);
/* DPSS End */

int vpq_set_amdv_pic_mode_id(unsigned char value);
int vpq_set_amdv_dark_detail(unsigned char value);
int vpq_set_amdv_light_sensor(struct vpq_light_sensor_s *pdata);
int vpq_set_amdv_precision_detail(unsigned char value);
struct vpq_dv_cfg_support_s vpq_get_dv_cfg_support(unsigned char value);

int vpq_set_memc_on_off(unsigned char value);
int vpq_set_memc_deblur_level(unsigned char value);
int vpq_set_memc_dejudder_level(unsigned char value);
int vpq_set_memc_demo_mode(unsigned char value);

int vpq_set_vdin_cvd(void);

void vpq_set_pq_effect(void);
int vpq_set_frame_status(enum vpq_frame_status_e status);
int vpq_get_event_info(void);
void vpq_get_signal_info(struct vpq_signal_info_s *pdata);

#endif // __VPQ_TABLE_LOGIC_H__
