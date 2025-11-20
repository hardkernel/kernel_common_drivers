/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _VPQ_TABLE_TYPE_H_
#define _VPQ_TABLE_TYPE_H_

#include <linux/amlogic/media/vpp/vpp_common_def.h>
#include <linux/amlogic/media/di/di.h>
#include "../dpss/drv/d_pq_mgr.h"

#define VPQ_DNLP_CURVE_LEN                  (VPP_DNLP_SCURV_LEN)
#define VPQ_DNLP_GAIN_VAR_LUT_LEN           (VPP_DNLP_GAIN_VAR_LUT_LEN)
#define VPQ_DNLP_WEXT_GAIN_LEN              (VPP_DNLP_WEXT_GAIN_LEN)
#define VPQ_DNLP_ADP_THRD_LEN               (VPP_DNLP_ADP_THRD_LEN)
#define VPQ_DNLP_REG_BLK_BOOST_LEN          (VPP_DNLP_REG_BLK_BOOST_LEN)
#define VPQ_DNLP_REG_ADP_OFSET_LEN          (VPP_DNLP_REG_ADP_OFSET_LEN)
#define VPQ_DNLP_REG_MONO_PROT_LEN          (VPP_DNLP_REG_MONO_PROT_LEN)
#define VPQ_DNLP_TREND_WHT_EXP_LUT_LEN      (VPP_DNLP_TREND_WHT_EXP_LUT_LEN)
#define VPQ_DNLP_HIST_GAIN_LEN              (VPP_DNLP_HIST_GAIN_LEN)
#define VPQ_DNLP_PARAM_MAX                  (EN_DNLP_PARAM_MAX)
#define VPQ_CCORING_MAX                     (EN_CCORING_MAX)
#define VPQ_BLKEXT_MAX                      (EN_BLKEXT_MAX)
#define VPQ_BLUE_STRETCH_MAX                (EN_BLUE_STRETCH_MAX)

#define VPQ_REG_DNR_MAX                     (REG_DNR_MAX)
#define VPQ_REG_DBLK_MAX                    (REG_DBLK_MAX)
#define VPQ_REG_DM_MAX                      (REG_DM_MAX)
#define VPQ_REG_DCT_MAX                     (REG_DCT_MAX)
#define VPQ_REG_XLR_MAX                     (REG_XLR_MAX)

// Reserve for customers to customize non-standard timing
#define RESERVE_NONSTANDARD_TIMING_COUNT    (5)

#define DNLP_TABLE_NUM_MAX                  (6 + RESERVE_NONSTANDARD_TIMING_COUNT)
#define HDR_TMO_TABLE_NUM_MAX               (6 + RESERVE_NONSTANDARD_TIMING_COUNT)
#define LC_TABLE_NUM_MAX                    (6 + RESERVE_NONSTANDARD_TIMING_COUNT)
#define AIPQ_TABLE_NUM_MAX                  (6 + RESERVE_NONSTANDARD_TIMING_COUNT)
#define BLKEXT_TABLE_NUM_MAX                (6 + RESERVE_NONSTANDARD_TIMING_COUNT)
#define BLUE_STR_TABLE_NUM_MAX              (6 + RESERVE_NONSTANDARD_TIMING_COUNT)
#define CCORING_TABLE_NUM_MAX               (6 + RESERVE_NONSTANDARD_TIMING_COUNT)
#define COLOR_BASE_TABLE_NUM_MAX            (6 + RESERVE_NONSTANDARD_TIMING_COUNT)

#define NR_TABLE_NUM_MAX                    (6 + RESERVE_NONSTANDARD_TIMING_COUNT)
#define DEBLOCK_TABLE_NUM_MAX               (6 + RESERVE_NONSTANDARD_TIMING_COUNT)
#define DEMOSQUITO_TABLE_NUM_MAX            (6 + RESERVE_NONSTANDARD_TIMING_COUNT)
#define DECONTOUR_TABLE_NUM_MAX             (6 + RESERVE_NONSTANDARD_TIMING_COUNT)
#define DI_TABLE_NUM_MAX                    (6 + RESERVE_NONSTANDARD_TIMING_COUNT)

#define VPQ_GAMMA_TABLE_POINT_256           (256)
#define VPQ_GAMMA_TABLE_POINT_257           (257)

#define PQ_TABLE_INVALID                    (255)

struct TABLE_VER_PQ {
	char project_version[64];
	char chip_version[64];
	char table_version[64];
	char oem_model[64];
	char panel_index[64];
	char reserved[64];
};

struct TABLE_PQ_MODULE_CFG {
	unsigned char pq_en;          // pq.AllPQModule.en
	unsigned char vadj1_en;       // pq.amvecm.basic.en
	unsigned char vd1_ctrst_en;   // pq.contrast.rgb.en
	unsigned char vadj2_en;       // pq.amvecm.basic.withOSD.en
	unsigned char post_ctrst_en;  // pq.contrast.rgb.withOSD.en
	unsigned char pregamma_en;
	unsigned char gamma_en;       // pq.gamma.en
	unsigned char wb_en;          // pq.whitebalance.en
	unsigned char dnlp_en;        // pq.dnlp.en
	unsigned char lc_en;          // pq.LocalContrast.en
	unsigned char black_ext_en;   // pq.BlackExtension.en
	unsigned char blue_stretch_en;// pq.BlueStretch.en
	unsigned char chroma_cor_en;  // pq.chroma.cor.en
	unsigned char sharpness0_en;  // pq.sharpness0.en
	unsigned char sharpness1_en;  // pq.sharpness1.en
	unsigned char cm_en;          // pq.cm2.en
	unsigned char lut3d_en;
	unsigned char dejaggy_sr0_en;
	unsigned char dejaggy_sr1_en;
	unsigned char dering_sr0_en;
	unsigned char dering_sr1_en;

	unsigned char di_en;          // pq.di.en
	unsigned char mcdi_en;        // pq.mcdi.en
	unsigned char deblock_en;     // pq.deblock.en
	unsigned char demosquito_en;  // pq.DemoSquito.en
	unsigned char smoothplus_en;  // pq.SmoothPlus.en
	unsigned char nr_en;          // pq.NoiseReduction.en
	unsigned char hdrtmo_en;      // pq.hdrtmo.en
	unsigned char ai_en;          // pq.ai.en
	unsigned char aisr_en;        // pq.aisr.en

	unsigned char reserved;
};

enum pq_table_module_index_e {
	PQ_INDEX_DNLP = 0,
	PQ_INDEX_HDR_TMO,
	PQ_INDEX_LC,
	PQ_INDEX_AIPQ,
	PQ_INDEX_BLK,
	PQ_INDEX_BLS,
	PQ_INDEX_CCORING,
	PQ_INDEX_CM2,
	PQ_INDEX_SR,
	PQ_INDEX_SHARPNESS,
	PQ_INDEX_AISR,

	PQ_INDEX_RESOLVED1,
	PQ_INDEX_RESOLVED2,
	PQ_INDEX_RESOLVED3,
	PQ_INDEX_RESOLVED4,

	PQ_INDEX_NR,
	PQ_INDEX_DEBLOCK,
	PQ_INDEX_DEMOSQUITO,
	PQ_INDEX_SMOOTHPLUS,
	PQ_INDEX_DI,

	PQ_INDEX_RESOLVED5,
	PQ_INDEX_RESOLVED6,
	PQ_INDEX_RESOLVED7,
	PQ_INDEX_RESOLVED8,
	PQ_INDEX_RESOLVED9,
	PQ_INDEX_MAX,
};

enum pq_source_timing_e {
	PQ_SRC_INDEX_VGA = 0,

	PQ_SRC_INDEX_SV_NTSC, /*1*/
	PQ_SRC_INDEX_SV_PAL,
	PQ_SRC_INDEX_SV_PAL_M,
	PQ_SRC_INDEX_SV_SECAM,

	PQ_SRC_INDEX_YCBCR_480I, /*5*/
	PQ_SRC_INDEX_YCBCR_576I,
	PQ_SRC_INDEX_YCBCR_480P,
	PQ_SRC_INDEX_YCBCR_576P,
	PQ_SRC_INDEX_YCBCR_720P,
	PQ_SRC_INDEX_YCBCR_1080I,
	PQ_SRC_INDEX_YCBCR_1080P,

	PQ_SRC_INDEX_ATV_NTSC, /*12*/
	PQ_SRC_INDEX_ATV_PAL,
	PQ_SRC_INDEX_ATV_PAL_M,
	PQ_SRC_INDEX_ATV_SECAM,
	PQ_SRC_INDEX_ATV_NTSC443,
	PQ_SRC_INDEX_ATV_PAL60,
	PQ_SRC_INDEX_ATV_NTSC50,
	PQ_SRC_INDEX_ATV_PAL_N,

	PQ_SRC_INDEX_AV_NTSC, /*20*/
	PQ_SRC_INDEX_AV_PAL,
	PQ_SRC_INDEX_AV_PAL_M,
	PQ_SRC_INDEX_AV_SECAM,
	PQ_SRC_INDEX_AV_NTSC443,
	PQ_SRC_INDEX_AV_PAL60,
	PQ_SRC_INDEX_AV_NTSC50,
	PQ_SRC_INDEX_AV_PAL_N,

	PQ_SRC_INDEX_HDMI_480I, /*28*/
	PQ_SRC_INDEX_HDMI_576I,
	PQ_SRC_INDEX_HDMI_480P,
	PQ_SRC_INDEX_HDMI_576P,
	PQ_SRC_INDEX_HDMI_720P,
	PQ_SRC_INDEX_HDMI_1080I,
	PQ_SRC_INDEX_HDMI_1080P,
	PQ_SRC_INDEX_HDMI_4K2KI,
	PQ_SRC_INDEX_HDMI_4K2KP,
	PQ_SRC_INDEX_HDR10_HDMI_480I,
	PQ_SRC_INDEX_HDR10_HDMI_576I,
	PQ_SRC_INDEX_HDR10_HDMI_480P,
	PQ_SRC_INDEX_HDR10_HDMI_576P,
	PQ_SRC_INDEX_HDR10_HDMI_720P,
	PQ_SRC_INDEX_HDR10_HDMI_1080I,
	PQ_SRC_INDEX_HDR10_HDMI_1080P,
	PQ_SRC_INDEX_HDR10_HDMI_4K2KI,
	PQ_SRC_INDEX_HDR10_HDMI_4K2KP,
	PQ_SRC_INDEX_HLG_HDMI_480I,
	PQ_SRC_INDEX_HLG_HDMI_576I,
	PQ_SRC_INDEX_HLG_HDMI_480P,
	PQ_SRC_INDEX_HLG_HDMI_576P,
	PQ_SRC_INDEX_HLG_HDMI_720P,
	PQ_SRC_INDEX_HLG_HDMI_1080I,
	PQ_SRC_INDEX_HLG_HDMI_1080P,
	PQ_SRC_INDEX_HLG_HDMI_4K2KI,
	PQ_SRC_INDEX_HLG_HDMI_4K2KP,
	PQ_SRC_INDEX_AMDV_HDMI_480I,
	PQ_SRC_INDEX_AMDV_HDMI_576I,
	PQ_SRC_INDEX_AMDV_HDMI_480P,
	PQ_SRC_INDEX_AMDV_HDMI_576P,
	PQ_SRC_INDEX_AMDV_HDMI_720P,
	PQ_SRC_INDEX_AMDV_HDMI_1080I,
	PQ_SRC_INDEX_AMDV_HDMI_1080P,
	PQ_SRC_INDEX_AMDV_HDMI_4K2KI,
	PQ_SRC_INDEX_AMDV_HDMI_4K2KP,
	PQ_SRC_INDEX_HDR10p_HDMI_480I,
	PQ_SRC_INDEX_HDR10p_HDMI_576I,
	PQ_SRC_INDEX_HDR10p_HDMI_480P,
	PQ_SRC_INDEX_HDR10p_HDMI_576P,
	PQ_SRC_INDEX_HDR10p_HDMI_720P,
	PQ_SRC_INDEX_HDR10p_HDMI_1080I,
	PQ_SRC_INDEX_HDR10p_HDMI_1080P,
	PQ_SRC_INDEX_HDR10p_HDMI_4K2KI,
	PQ_SRC_INDEX_HDR10p_HDMI_4K2KP,

	PQ_SRC_INDEX_DTV_480I, /*73*/
	PQ_SRC_INDEX_DTV_576I,
	PQ_SRC_INDEX_DTV_480P,
	PQ_SRC_INDEX_DTV_576P,
	PQ_SRC_INDEX_DTV_720P,
	PQ_SRC_INDEX_DTV_1080I,
	PQ_SRC_INDEX_DTV_1080P,
	PQ_SRC_INDEX_DTV_4K2KI,
	PQ_SRC_INDEX_DTV_4K2KP,
	PQ_SRC_INDEX_HDR10_DTV_480I,
	PQ_SRC_INDEX_HDR10_DTV_576I,
	PQ_SRC_INDEX_HDR10_DTV_480P,
	PQ_SRC_INDEX_HDR10_DTV_576P,
	PQ_SRC_INDEX_HDR10_DTV_720P,
	PQ_SRC_INDEX_HDR10_DTV_1080I,
	PQ_SRC_INDEX_HDR10_DTV_1080P,
	PQ_SRC_INDEX_HDR10_DTV_4K2KI,
	PQ_SRC_INDEX_HDR10_DTV_4K2KP,
	PQ_SRC_INDEX_HLG_DTV_480I,
	PQ_SRC_INDEX_HLG_DTV_576I,
	PQ_SRC_INDEX_HLG_DTV_480P,
	PQ_SRC_INDEX_HLG_DTV_576P,
	PQ_SRC_INDEX_HLG_DTV_720P,
	PQ_SRC_INDEX_HLG_DTV_1080I,
	PQ_SRC_INDEX_HLG_DTV_1080P,
	PQ_SRC_INDEX_HLG_DTV_4K2KI,
	PQ_SRC_INDEX_HLG_DTV_4K2KP,
	PQ_SRC_INDEX_AMDV_DTV_480I,
	PQ_SRC_INDEX_AMDV_DTV_576I,
	PQ_SRC_INDEX_AMDV_DTV_480P,
	PQ_SRC_INDEX_AMDV_DTV_576P,
	PQ_SRC_INDEX_AMDV_DTV_720P,
	PQ_SRC_INDEX_AMDV_DTV_1080I,
	PQ_SRC_INDEX_AMDV_DTV_1080P,
	PQ_SRC_INDEX_AMDV_DTV_4K2KI,
	PQ_SRC_INDEX_AMDV_DTV_4K2KP,
	PQ_SRC_INDEX_HDR10p_DTV_480I,
	PQ_SRC_INDEX_HDR10p_DTV_576I,
	PQ_SRC_INDEX_HDR10p_DTV_480P,
	PQ_SRC_INDEX_HDR10p_DTV_576P,
	PQ_SRC_INDEX_HDR10p_DTV_720P,
	PQ_SRC_INDEX_HDR10p_DTV_1080I,
	PQ_SRC_INDEX_HDR10p_DTV_1080P,
	PQ_SRC_INDEX_HDR10p_DTV_4K2KI,
	PQ_SRC_INDEX_HDR10p_DTV_4K2KP,

	PQ_SRC_INDEX_MPEG_480I, /*118*/
	PQ_SRC_INDEX_MPEG_576I,
	PQ_SRC_INDEX_MPEG_480P,
	PQ_SRC_INDEX_MPEG_576P,
	PQ_SRC_INDEX_MPEG_720P,
	PQ_SRC_INDEX_MPEG_1080I,
	PQ_SRC_INDEX_MPEG_1080P,
	PQ_SRC_INDEX_MPEG_4K2KI,
	PQ_SRC_INDEX_MPEG_4K2KP,
	PQ_SRC_INDEX_HDR10_MPEG_480I,
	PQ_SRC_INDEX_HDR10_MPEG_576I,
	PQ_SRC_INDEX_HDR10_MPEG_480P,
	PQ_SRC_INDEX_HDR10_MPEG_576P,
	PQ_SRC_INDEX_HDR10_MPEG_720P,
	PQ_SRC_INDEX_HDR10_MPEG_1080I,
	PQ_SRC_INDEX_HDR10_MPEG_1080P,
	PQ_SRC_INDEX_HDR10_MPEG_4K2KI,
	PQ_SRC_INDEX_HDR10_MPEG_4K2KP,
	PQ_SRC_INDEX_HLG_MPEG_480I,
	PQ_SRC_INDEX_HLG_MPEG_576I,
	PQ_SRC_INDEX_HLG_MPEG_480P,
	PQ_SRC_INDEX_HLG_MPEG_576P,
	PQ_SRC_INDEX_HLG_MPEG_720P,
	PQ_SRC_INDEX_HLG_MPEG_1080I,
	PQ_SRC_INDEX_HLG_MPEG_1080P,
	PQ_SRC_INDEX_HLG_MPEG_4K2KI,
	PQ_SRC_INDEX_HLG_MPEG_4K2KP,
	PQ_SRC_INDEX_AMDV_MPEG_480I,
	PQ_SRC_INDEX_AMDV_MPEG_576I,
	PQ_SRC_INDEX_AMDV_MPEG_480P,
	PQ_SRC_INDEX_AMDV_MPEG_576P,
	PQ_SRC_INDEX_AMDV_MPEG_720P,
	PQ_SRC_INDEX_AMDV_MPEG_1080I,
	PQ_SRC_INDEX_AMDV_MPEG_1080P,
	PQ_SRC_INDEX_AMDV_MPEG_4K2KI,
	PQ_SRC_INDEX_AMDV_MPEG_4K2KP,
	PQ_SRC_INDEX_HDR10p_MPEG_480I,
	PQ_SRC_INDEX_HDR10p_MPEG_576I,
	PQ_SRC_INDEX_HDR10p_MPEG_480P,
	PQ_SRC_INDEX_HDR10p_MPEG_576P,
	PQ_SRC_INDEX_HDR10p_MPEG_720P,
	PQ_SRC_INDEX_HDR10p_MPEG_1080I,
	PQ_SRC_INDEX_HDR10p_MPEG_1080P,
	PQ_SRC_INDEX_HDR10p_MPEG_4K2KI,
	PQ_SRC_INDEX_HDR10p_MPEG_4K2KP,

	// Reserve space for customizing non-standard timing for the customers
	PQ_SRC_INDEX_RESERVE_1, /*163*/
	PQ_SRC_INDEX_RESERVE_2,
	PQ_SRC_INDEX_RESERVE_3,
	PQ_SRC_INDEX_RESERVE_4,
	PQ_SRC_INDEX_RESERVE_5,

	PQ_SRC_INDEX_MAX,
};

enum pq_table_level_e {
	PQ_TABLE_LV_1 = 1,
	PQ_TABLE_LV_2,
	PQ_TABLE_LV_3,
	PQ_TABLE_LV_4,
	PQ_TABLE_LV_5,
	PQ_TABLE_LV_6,
};

struct pq_dnlp_curve_param_s {
	struct vpp_dnlp_curve_param_s param;
};

struct pq_hdr_tmo_param_s {
	struct vpp_tmo_param_s param;
};

struct pq_lc_curve_param_s {
	struct vpp_lc_curve_s curve;
	struct vpp_lc_param_s param;
};

enum aipq_rows_e {
	AIPQ_ROWS_SKIN = 0,
	AIPQ_ROWS_BLUESKY,
	AIPQ_ROWS_FOODS,
	AIPQ_ROWS_ARCHITECTURE,
	AIPQ_ROWS_GRASS,
	AIPQ_ROWS_NIGHT,
	AIPQ_ROWS_DOCUMENT,
	AIPQ_ROWS_MAX,
};

enum aipq_cols_e {
	AIPQ_COLS_BLUE = 0,
	AIPQ_COLS_GREEN,
	AIPQ_COLS_SKIN_TONE,
	AIPQ_COLS_PEAKING,
	AIPQ_COLS_SAT,
	AIPQ_COLS_CONTRAST,
	AIPQ_COLS_MAX,
};

struct aipq_size_s {
	unsigned int height;
	unsigned int width;
};

struct pq_aipq_param_s {
	struct aipq_size_s size;
	unsigned int aipq_table[AIPQ_ROWS_MAX][AIPQ_COLS_MAX];
};

enum sr_type_e {
	SR_TYPE_0 = 0,
	SR_TYPE_1,
	SR_TYPE_MAX,
};

struct pq_ccoring_param_s {
	struct vpp_cc_param_s param;
};

struct pq_blkext_param_s {
	struct vpp_blkext_param_s param;
};

struct pq_blue_stretch_param_s {
	struct vpp_bs_param_s param;
};

struct pq_cm_base_param_s {
	int sat_by_y_0[9];

	int luma_by_hue_0[32];

	int sat_by_hs_0[32];
	int sat_by_hs_1[32];
	int sat_by_hs_2[32];

	int hue_by_hue_0[32];

	int hue_by_hy_0[32];
	int hue_by_hy_1[32];
	int hue_by_hy_2[32];
	int hue_by_hy_3[32];
	int hue_by_hy_4[32];

	int hue_by_hs_0[32];
	int hue_by_hs_1[32];
	int hue_by_hs_2[32];
	int hue_by_hs_3[32];
	int hue_by_hs_4[32];

	int sat_by_hy_0[32];
	int sat_by_hy_1[32];
	int sat_by_hy_2[32];
	int sat_by_hy_3[32];
	int sat_by_hy_4[32];
};

struct aisr_param_s { // refer "vpp_aisr_param_e"
	unsigned int reg_qt_clip_value;
	unsigned int reg_dering_en;
	unsigned int reg_dering_prot_thd;
	unsigned int reg_dering_thd0;
	unsigned int reg_dering_thd1;
	unsigned int reg_dering_slope;
	unsigned int reg_dering_coring_en;
	unsigned int reg_dering_coring_thd;
	unsigned int reg_adj_gain_neg;
	unsigned int reg_adj_gain_pos;
	unsigned int reg_adj_pos_en;
	unsigned int reg_adj_pos_thd0;
	unsigned int reg_adj_pos_thd1;
	unsigned int reg_adj_pos_thd2;
	unsigned int reg_adj_pos_top0;
	unsigned int reg_adj_pos_top1;
	unsigned int reg_adj_pos_top2;
	unsigned int reg_adj_pos_top3;
	unsigned int reg_adj_pos_slope0;
	unsigned int reg_adj_pos_slope1;
	unsigned int reg_adj_pos_slope2;
	unsigned int reg_adj_pos_slope3;
	unsigned int reg_adj_neg_en;
	unsigned int reg_adj_neg_thd0;
	unsigned int reg_adj_neg_thd1;
	unsigned int reg_adj_neg_thd2;
	unsigned int reg_adj_neg_top0;
	unsigned int reg_adj_neg_top1;
	unsigned int reg_adj_neg_top2;
	unsigned int reg_adj_neg_top3;
	unsigned int reg_adj_neg_slope0;
	unsigned int reg_adj_neg_slope1;
	unsigned int reg_adj_neg_slope2;
	unsigned int reg_adj_neg_slope3;
	unsigned int reg_adp_coring_en;
	unsigned int reg_adp_coring_thd;
	unsigned int reg_adp_glb_coring_thd;
};

struct aisr_nn_param_s { // refer "vpp_aisr_nn_param_e"
	unsigned int reg_lrhf_hf_gain_neg;
	unsigned int reg_lrhf_hf_gain_pos;
	unsigned int reg_lrhf_lpf_coeff00;
	unsigned int reg_lrhf_lpf_coeff01;
	unsigned int reg_lrhf_lpf_coeff02;
	unsigned int reg_lrhf_lpf_coeff10;
	unsigned int reg_lrhf_lpf_coeff11;
	unsigned int reg_lrhf_lpf_coeff12;
	unsigned int reg_lrhf_lpf_coeff20;
	unsigned int reg_lrhf_lpf_coeff21;
	unsigned int reg_lrhf_lpf_coeff22;
};

struct pq_ai_sr_param_s {
	// struct vpp_aisr_param_s param;
	struct aisr_param_s param;
	// struct vpp_aisr_nn_param_s nn_param;
	struct aisr_nn_param_s nn_param;
};

struct pq_nr_param_s {
	struct di_dnr_param_s param;
};

struct pq_deblock_param_s {
	struct di_dblk_param_s param;
};

struct pq_demos_param_s {
	struct di_demosquito_param_s param;
};

struct pq_decontour_param_s {
	struct di_dct_param_s param;
};

struct pq_di_param_s {
	struct di_xlr_param_s param;
};

struct PQ_TABLE_PARAM {
	unsigned char pq_index_table0[PQ_SRC_INDEX_MAX][PQ_INDEX_MAX];

	struct pq_dnlp_curve_param_s dnlp_table[DNLP_TABLE_NUM_MAX][PQ_TABLE_LV_4];
	struct pq_hdr_tmo_param_s tmo_table[HDR_TMO_TABLE_NUM_MAX][PQ_TABLE_LV_4];
	struct pq_lc_curve_param_s lc_table[LC_TABLE_NUM_MAX][PQ_TABLE_LV_4];
	struct pq_aipq_param_s aipq_table[AIPQ_TABLE_NUM_MAX];
	struct pq_blkext_param_s blkext_table[BLKEXT_TABLE_NUM_MAX][PQ_TABLE_LV_4];
	struct pq_blue_stretch_param_s bls_table[BLUE_STR_TABLE_NUM_MAX][PQ_TABLE_LV_4];
	struct pq_ccoring_param_s ccoring_table[CCORING_TABLE_NUM_MAX][PQ_TABLE_LV_4];
	struct pq_cm_base_param_s cm_base_table[COLOR_BASE_TABLE_NUM_MAX][PQ_TABLE_LV_4];

	struct pq_nr_param_s nr_table[NR_TABLE_NUM_MAX][PQ_TABLE_LV_5];
	struct pq_deblock_param_s dblk_table[DEBLOCK_TABLE_NUM_MAX][PQ_TABLE_LV_4];
	struct pq_demos_param_s demos_table[DEMOSQUITO_TABLE_NUM_MAX][PQ_TABLE_LV_4];
	struct pq_decontour_param_s dct_table[DECONTOUR_TABLE_NUM_MAX][PQ_TABLE_LV_4];
	struct pq_di_param_s di_table[DI_TABLE_NUM_MAX];
};

#endif // _VPQ_TABLE_TYPE_H_
