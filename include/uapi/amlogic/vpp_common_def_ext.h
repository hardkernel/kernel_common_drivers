/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __VPP_COMMON_DEF_EXT_H__
#define __VPP_COMMON_DEF_EXT_H__

#define _VPP_TYPE  'V'

#define VPP_MTRX_COEF_LEN         (9)
#define VPP_MTRX_OFFSET_LEN       (3)
#define VPP_HIST_BIN_COUNT        (64)
#define VPP_COLOR_HIST_BIN_COUNT  (32)
#define VPP_HDR_HIST_BIN_COUNT    (128)
#define VPP_COLOR_PRIMARY_LEN     (8)

#define VPP_DNLP_SCURV_LEN             (65)
#define VPP_DNLP_GAIN_VAR_LUT_LEN      (49)
#define VPP_DNLP_WEXT_GAIN_LEN         (48)
#define VPP_DNLP_ADP_THRD_LEN          (33)
#define VPP_DNLP_REG_BLK_BOOST_LEN     (13)
#define VPP_DNLP_REG_ADP_OFSET_LEN     (20)
#define VPP_DNLP_REG_MONO_PROT_LEN     (6)
#define VPP_DNLP_TREND_WHT_EXP_LUT_LEN (9)
#define VPP_DNLP_HIST_GAIN_LEN         (65)

enum vpp_level_mode_e {
	EN_LEVEL_MODE_OFF = 0,
	EN_LEVEL_MODE_LOW,
	EN_LEVEL_MODE_MID,
	EN_LEVEL_MODE_HIGH,
	EN_LEVEL_MODE_AUTO,
};

enum vpp_rgb_mode_e {
	EN_MODE_R = 0,
	EN_MODE_G,
	EN_MODE_B,
	EN_MODE_RGB_MAX,
};

enum vpp_mtrx_type_e {
	EN_MTRX_VD1 = 0,
	EN_MTRX_POST,
	EN_MTRX_POST2,
	EN_MTRX_MAX,
};

enum vpp_dnlp_param_e {
	EN_DNLP_SMHIST_CK = 0,
	EN_DNLP_MVREFLSH,
	EN_DNLP_CUVBLD_MIN,
	EN_DNLP_CUVBLD_MAX,
	EN_DNLP_BBD_RATIO_LOW,
	EN_DNLP_BBD_RATIO_HIG,
	EN_DNLP_BLK_CCTR,
	EN_DNLP_BRGT_CTRL,
	EN_DNLP_BRGT_RANGE,
	EN_DNLP_BRGHT_ADD,
	EN_DNLP_BRGHT_MAX,
	EN_DNLP_AUTO_RNG,
	EN_DNLP_LOWRANGE,
	EN_DNLP_HGHRANGE,
	EN_DNLP_SATUR_RAT,
	EN_DNLP_SATUR_MAX,
	EN_DNLP_SBGNBND,
	EN_DNLP_SENDBND,
	EN_DNLP_CLASH_BGN,
	EN_DNLP_CLASH_END,
	EN_DNLP_CLAHE_GAIN_NEG,
	EN_DNLP_CLAHE_GAIN_POS,
	EN_DNLP_MTDBLD_RATE,
	EN_DNLP_ADPMTD_LBND,
	EN_DNLP_ADPMTD_HBND,
	EN_DNLP_BLKEXT_OFST,
	EN_DNLP_WHTEXT_OFST,
	EN_DNLP_BLKEXT_RATE,
	EN_DNLP_WHTEXT_RATE,
	EN_DNLP_BWEXT_DIV4X_MIN,
	EN_DNLP_IRGNBGN,
	EN_DNLP_IRGNEND,
	EN_DNLP_FINAL_GAIN,
	EN_DNLP_CLIPRATE_V3,
	EN_DNLP_CLIPRATE_MIN,
	EN_DNLP_ADPCRAT_LBND,
	EN_DNLP_ADPCRAT_HBND,
	EN_DNLP_SCURV_LOW_TH,
	EN_DNLP_SCURV_MID1_TH,
	EN_DNLP_SCURV_MID2_TH,
	EN_DNLP_SCURV_HGH1_TH,
	EN_DNLP_SCURV_HGH2_TH,
	EN_DNLP_MTDRATE_ADP_EN,
	EN_DNLP_CLAHE_METHOD,
	EN_DNLP_BLE_EN,
	EN_DNLP_NORM,
	EN_DNLP_SCN_CHG_TH,
	EN_DNLP_IIR_STEP_MUX,
	EN_DNLP_SINGLE_BIN_BW,
	EN_DNLP_SINGLE_BIN_METHOD,
	EN_DNLP_REG_MAX_SLOP_1ST,
	EN_DNLP_REG_MAX_SLOP_MID,
	EN_DNLP_REG_MAX_SLOP_FIN,
	EN_DNLP_REG_MIN_SLOP_1ST,
	EN_DNLP_REG_MIN_SLOP_MID,
	EN_DNLP_REG_MIN_SLOP_FIN,
	EN_DNLP_REG_TREND_WHT_EXPAND_MODE,
	EN_DNLP_REG_TREND_BLK_EXPAND_MODE,
	EN_DNLP_HIST_CUR_GAIN,
	EN_DNLP_HIST_CUR_GAIN_PRECISE,
	EN_DNLP_REG_MONO_BINRANG_ST,
	EN_DNLP_REG_MONO_BINRANG_ED,
	EN_DNLP_C_HIST_GAIN_BASE,
	EN_DNLP_S_HIST_GAIN_BASE,
	EN_DNLP_MVREFLSH_OFFSET,
	EN_DNLP_LUMA_AVG_TH,
	EN_DNLP_PARAM_MAX,
};

enum vpp_dnlp_param_ext_e {
	EN_DNLP_BLK_EN = 0,
	EN_DNLP_BLK_END,
	EN_DNLP_BLK_SLOPE,
	EN_DNLP_BRT_EN,
	EN_DNLP_BRT_START,
	EN_DNLP_BRT_SLOPE,
	EN_DNLP_EXT_PARAM_MAX,
};

enum vpp_lc_param_e {
	EN_LC_CURVE_NODES_VLPF = 0,
	EN_LC_CURVE_NODES_HLPF,
	EN_LC_LMT_RAT_VALID,
	EN_LC_LMT_RAT_MIN_MAX,
	EN_LC_CONTRAST_GAIN_HIGH,
	EN_LC_CONTRAST_GAIN_LOW,   /*5*/
	EN_LC_CONTRAST_LMT_HIGH_1,
	EN_LC_CONTRAST_LMT_LOW_1,
	EN_LC_CONTRAST_LMT_HIGH_0,
	EN_LC_CONTRAST_LMT_LOW_0,
	EN_LC_CONTRAST_SCALE_HIGH, /*10*/
	EN_LC_CONTRAST_SCALE_LOW,
	EN_LC_CONTRAST_BVN_HIGH,
	EN_LC_CONTRAST_BVN_LOW,
	EN_LC_SLOPE_MAX_FACE,
	EN_LC_NUM_M_CORING,        /*15*/
	EN_LC_YPKBV_SLOPE_MAX,
	EN_LC_YPKBV_SLOPE_MIN,
	EN_LC_PARAM_MAX,
};

enum vpp_csc_type_e {
	EN_CSC_MATRIX_NULL                = 0,
	EN_CSC_MATRIX_RGB_YUV601          = 0x1,
	EN_CSC_MATRIX_RGB_YUV601F         = 0x2,
	EN_CSC_MATRIX_RGB_YUV709          = 0x3,
	EN_CSC_MATRIX_RGB_YUV709F         = 0x4,
	EN_CSC_MATRIX_YUV601_RGB          = 0x10,
	EN_CSC_MATRIX_YUV601_YUV601F      = 0x11,
	EN_CSC_MATRIX_YUV601_YUV709       = 0x12,
	EN_CSC_MATRIX_YUV601_YUV709F      = 0x13,
	EN_CSC_MATRIX_YUV601F_RGB         = 0x14,
	EN_CSC_MATRIX_YUV601F_YUV601      = 0x15,
	EN_CSC_MATRIX_YUV601F_YUV709      = 0x16,
	EN_CSC_MATRIX_YUV601F_YUV709F     = 0x17,
	EN_CSC_MATRIX_YUV709_RGB          = 0x20,
	EN_CSC_MATRIX_YUV709_YUV601       = 0x21,
	EN_CSC_MATRIX_YUV709_YUV601F      = 0x22,
	EN_CSC_MATRIX_YUV709_YUV709F      = 0x23,
	EN_CSC_MATRIX_YUV709F_RGB         = 0x24,
	EN_CSC_MATRIX_YUV709F_YUV601      = 0x25,
	EN_CSC_MATRIX_YUV709F_YUV709      = 0x26,
	EN_CSC_MATRIX_YUV601L_YUV709L     = 0x27,
	EN_CSC_MATRIX_YUV709L_YUV601L     = 0x28,
	EN_CSC_MATRIX_YUV709F_YUV601F     = 0x29,
	EN_CSC_MATRIX_BT2020YUV_BT2020RGB = 0x40,
	EN_CSC_MATRIX_BT2020RGB_709RGB,
	EN_CSC_MATRIX_BT2020RGB_CUSRGB,
	EN_CSC_MATRIX_BT2020YUV_BT2020RGB_DYNAMIC = 0x50,
	EN_CSC_MATRIX_BT2020YUV_BT2020RGB_CUVA = 0x51,
	EN_CSC_MATRIX_DEFAULT_CSCTYPE = 0xffff,
};

enum vpp_pc_mode_e {
	EN_PC_MODE_OFF = 0,
	EN_PC_MODE_ON,
};

enum vpp_hdr_type_e {
	EN_TYPE_NONE = 0,
	EN_TYPE_SDR,
	EN_TYPE_HDR10,
	EN_TYPE_HLG,
	EN_TYPE_HDR10PLUS,
	EN_TYPE_CUVA_HDR,
	EN_TYPE_CUVA_HLG,
};

enum vpp_color_primary_e {
	EN_COLOR_PRI_NULL = 0,
	EN_COLOR_PRI_BT601,
	EN_COLOR_PRI_BT709,
	EN_COLOR_PRI_BT2020,
	EN_COLOR_PRI_MAX,
};

enum vpp_hdr_lut_type_e {
	EN_HDR_LUT_TYPE_HLG = 0,
	EN_HDR_LUT_TYPE_HDR,
	EN_HDR_LUT_TYPE_MAX,
};

enum vpp_lut3d_data_type_e {
	EN_LUT3D_INPUT_PARAM = 0,
	EN_LUT3D_UNIFY_KEY,
	EN_LUT3D_BIN_FILE,
};

enum vpp_module_e {
	EN_MODULE_VADJ1 = 0,
	EN_MODULE_VADJ2,
	EN_MODULE_PREGAMMA,
	EN_MODULE_GAMMA,
	EN_MODULE_WB,
	EN_MODULE_DNLP,     /*5*/
	EN_MODULE_CCORING,
	EN_MODULE_SR0,
	EN_MODULE_SR1,
	EN_MODULE_LC,
	EN_MODULE_CM,       /*10*/
	EN_MODULE_BLE,
	EN_MODULE_BLS,
	EN_MODULE_LUT3D,
	EN_MODULE_DEJAGGY_SR0,
	EN_MODULE_DEJAGGY_SR1,     /*15*/
	EN_MODULE_DERING_SR0,
	EN_MODULE_DERING_SR1,
	EN_MODULE_ALL,
};

enum vpp_cm_curve_type_e {
	EN_CM_LUMA = 0,
	EN_CM_SAT,
	EN_CM_HUE,
	EN_CM_HUE_BY_HS,
	EN_CM_SAT_BY_L,
	EN_CM_SAT_BY_HL,
	EN_CM_HUE_BY_HL,
};

enum vpp_ccoring_params_e {
	EN_CCORING_SLOPE = 0,
	EN_CCORING_TH,
	EN_CCORING_BYPASS_YTH,
	EN_CCORING_MAX,
};

enum vpp_blkext_param_e {
	EN_BLKEXT_START = 0,
	EN_BLKEXT_SLOPE1,
	EN_BLKEXT_MIDPT,
	EN_BLKEXT_SLOPE2,
	EN_BLKEXT_MAX,
};

enum vpp_blue_stretch_param_e {
	EN_BLUE_STRETCH_EN_SEL = 0,
	EN_BLUE_STRETCH_CB_INC,
	EN_BLUE_STRETCH_CR_INC,
	EN_BLUE_STRETCH_GAIN,
	EN_BLUE_STRETCH_GAIN_CB4CR,
	EN_BLUE_STRETCH_LUMA_HIGH,    /*5*/
	EN_BLUE_STRETCH_ERR_CRP,
	EN_BLUE_STRETCH_ERR_SIGN3,
	EN_BLUE_STRETCH_ERR_CRP_INV,
	EN_BLUE_STRETCH_ERR_CRN,
	EN_BLUE_STRETCH_ERR_SIGN2,    /*10*/
	EN_BLUE_STRETCH_ERR_CRN_INV,
	EN_BLUE_STRETCH_ERR_CBP,
	EN_BLUE_STRETCH_ERR_SIGN1,
	EN_BLUE_STRETCH_ERR_CBP_INV,
	EN_BLUE_STRETCH_ERR_CBN,      /*15*/
	EN_BLUE_STRETCH_ERR_SIGN0,
	EN_BLUE_STRETCH_ERR_CBN_INV,
	EN_BLUE_STRETCH_MAX,
};

enum vpp_sr_type_e {
	EN_SR_0 = 0,
	EN_SR_1,
	EN_SR_VSR,
};

struct vpp_white_balance_s {
	int r_pre_offset;     /*s11.0, range -1024~+1023, default is 0*/
	int g_pre_offset;     /*s11.0, range -1024~+1023, default is 0*/
	int b_pre_offset;     /*s11.0, range -1024~+1023, default is 0*/
	unsigned int r_gain;  /*u1.10, range 0~2047, default is 1024 (1.0x)*/
	unsigned int g_gain;  /*u1.10, range 0~2047, default is 1024 (1.0x)*/
	unsigned int b_gain;  /*u1.10, range 0~2047, default is 1024 (1.0x)*/
	int r_post_offset;    /*s11.0, range -1024~+1023, default is 0*/
	int g_post_offset;    /*s11.0, range -1024~+1023, default is 0*/
	int b_post_offset;    /*s11.0, range -1024~+1023, default is 0*/
};

struct vpp_gamma_table_s {
	unsigned int *r_data;
	unsigned int *g_data;
	unsigned int *b_data;
};

struct vpp_mtrx_param_s {
	unsigned int pre_offset[VPP_MTRX_OFFSET_LEN];
	unsigned int matrix_coef[VPP_MTRX_COEF_LEN];
	unsigned int post_offset[VPP_MTRX_OFFSET_LEN];
	unsigned int right_shift;
};

struct vpp_mtrx_info_s {
	enum vpp_mtrx_type_e mtrx_sel;
	struct vpp_mtrx_param_s mtrx_param;
};

struct vpp_module_ctrl_s {
	enum vpp_module_e module_type;
	int status;
};

struct vpp_pq_en_ctrl_s {
	unsigned char vadj1_en; /*control video brightness contrast saturation hue*/
	unsigned char vd1_ctrst_en;
	unsigned char vadj2_en; /*control video+osd brightness contrast saturation hue*/
	unsigned char post_ctrst_en;
	unsigned char pregamma_en;
	unsigned char gamma_en;
	unsigned char wb_en;
	unsigned char dnlp_en;
	unsigned char lc_en;
	unsigned char black_ext_en;
	unsigned char blue_stretch_en;
	unsigned char chroma_cor_en;
	unsigned char sharpness0_en;
	unsigned char sharpness1_en;
	unsigned char cm_en;
	unsigned char lut3d_en;
	unsigned char dejaggy_sr0_en;
	unsigned char dejaggy_sr1_en;
	unsigned char dering_sr0_en;
	unsigned char dering_sr1_en;
	unsigned char reserved;
};

struct vpp_pq_state_s {
	unsigned char pq_en;
	struct vpp_pq_en_ctrl_s pq_cfg;
};

struct vpp_dnlp_curve_param_s {
	unsigned int dnlp_scurv_low[VPP_DNLP_SCURV_LEN];
	unsigned int dnlp_scurv_mid1[VPP_DNLP_SCURV_LEN];
	unsigned int dnlp_scurv_mid2[VPP_DNLP_SCURV_LEN];
	unsigned int dnlp_scurv_hgh1[VPP_DNLP_SCURV_LEN];
	unsigned int dnlp_scurv_hgh2[VPP_DNLP_SCURV_LEN];
	unsigned int gain_var_lut49[VPP_DNLP_GAIN_VAR_LUT_LEN];
	unsigned int wext_gain[VPP_DNLP_WEXT_GAIN_LEN];
	unsigned int adp_thrd[VPP_DNLP_ADP_THRD_LEN];
	unsigned int reg_blk_boost_12[VPP_DNLP_REG_BLK_BOOST_LEN];
	unsigned int reg_adp_ofset_20[VPP_DNLP_REG_ADP_OFSET_LEN];
	unsigned int reg_mono_protect[VPP_DNLP_REG_MONO_PROT_LEN];
	unsigned int reg_trend_wht_expand_lut8[VPP_DNLP_TREND_WHT_EXP_LUT_LEN];
	unsigned int c_hist_gain[VPP_DNLP_HIST_GAIN_LEN];
	unsigned int s_hist_gain[VPP_DNLP_HIST_GAIN_LEN];
	unsigned int param[EN_DNLP_PARAM_MAX];
};

struct vpp_ble_whe_param_s {
	unsigned int param[EN_DNLP_EXT_PARAM_MAX];
};

struct vpp_lc_curve_s {
	unsigned int lc_saturation[63];
	unsigned int lc_yminval_lmt[16];
	unsigned int lc_ypkbv_ymaxval_lmt[16];
	unsigned int lc_ymaxval_lmt[16];
	unsigned int lc_ypkbv_lmt[16];
	unsigned int lc_ypkbv_ratio[4];
};

struct vpp_lc_param_s {
	unsigned int param[EN_LC_PARAM_MAX];
};

struct vpp_lut3d_table_s {
	enum vpp_lut3d_data_type_e data_type;
	unsigned char data_index;
	unsigned char data_check;
	unsigned int data_size;
	int *data;
};

struct vpp_hdr_lut_s {
	enum vpp_hdr_lut_type_e lut_type;
	unsigned int lut_size;
	union {
		int *lut_data;
		long long lut_len;
	};
};

struct vpp_cm_curve_s {
	enum vpp_cm_curve_type_e curve_type;
	unsigned int data_size;
	void *data;
};

struct vpp_eye_protect_s {
	int enable;
	int rgb[EN_MODE_RGB_MAX];
};

struct vpp_aipq_table_s {
	unsigned int height;
	unsigned int width;
	union {
		void *table_ptr;
		long long table_len;
	};
};

struct vpp_color_primary_s {/*R/G/B/W (x,y)*50000*/
	unsigned int data_src[VPP_COLOR_PRIMARY_LEN];
	unsigned int data_dest[VPP_COLOR_PRIMARY_LEN];
};

struct vpp_overscan_table_s {
	unsigned int src_timing;
	unsigned int value1;
	unsigned int value2;
	unsigned int reserved1;
	unsigned int reserved2;
};

struct vpp_table_s {
	unsigned int length;
	union {
		void *param_ptr;
		long long param_ptr_len;
	};
	union {
		void *reserved;
		long long reserved_len;
	};
};

struct vpp_hist_ave_s {
	unsigned int sum;
	int width;
	int height;
	int ave;
};

struct vpp_hist_bin_s {
	unsigned int hist_pow;
	unsigned int luma_sum;
	unsigned int pixel_sum;
	unsigned int hist[VPP_HIST_BIN_COUNT];
	unsigned int dark_hist[VPP_HIST_BIN_COUNT];
	unsigned int hue_hist[VPP_COLOR_HIST_BIN_COUNT];
	unsigned int sat_hist[VPP_COLOR_HIST_BIN_COUNT];
};

struct vpp_hdr_hist_param_s {
	unsigned int data_rgb_max[VPP_HDR_HIST_BIN_COUNT];
};

/*master_display_info for display device*/
struct vpp_hdr_metadata_s {
	unsigned int primaries[3][2]; /*normalized 50000 in G,B,R order*/
	unsigned int white_point[2];  /*normalized 50000*/
	unsigned int luminance[2];    /*max/min luminance, normalized 10000*/
};

struct vpp_tmo_param_s {
	int tmo_en;
	int reg_highlight;
	int reg_hist_th;
	int reg_light_th;
	int reg_highlight_th1;
	int reg_highlight_th2;
	int reg_display_e;
	int reg_middle_a;
	int reg_middle_a_adj;
	int reg_middle_b;
	int reg_middle_s;
	int reg_max_th1;
	int reg_middle_th;
	int reg_thold1;
	int reg_thold2;
	int reg_thold3;
	int reg_thold4;
	int reg_max_th2;
	int reg_pnum_th;
	int reg_hl0;
	int reg_hl1;
	int reg_hl2;
	int reg_hl3;
	int reg_display_adj;
	int reg_avg_th;
	int reg_avg_adj;
	int reg_low_adj;
	int reg_high_en;
	int reg_high_adj1;
	int reg_high_adj2;
	int reg_high_maxdiff;
	int reg_high_mindiff;
	unsigned int alpha;
};

struct vpp_cc_param_s {
	unsigned int param[EN_CCORING_MAX];
};

struct vpp_blkext_param_s {
	unsigned int param[EN_BLKEXT_MAX];
};

struct vpp_bs_param_s {
	unsigned int param[EN_BLUE_STRETCH_MAX];
};

struct vpp_sr_param_s {
	enum vpp_sr_type_e type;
	unsigned int data_length;
	unsigned int *data;
};

struct vpp_pq_tuning_reg_s {
	unsigned char reg_addr;
	unsigned char bit_start;
	unsigned char bit_end;
	unsigned int val;
};

struct vpp_pq_tuning_page_s {
	unsigned char page_addr;
	int page_idx;
	int reg_count;
	struct vpp_pq_tuning_reg_s *reg_list_ptr;
};

struct vpp_pq_tuning_table_s {
	int page_count;
	struct vpp_pq_tuning_page_s *page_list_ptr;
};

/*IOCtl*/
#define VPP_IOC_SET_BRIGHTNESS      _IOW(_VPP_TYPE, 0x01, int)
#define VPP_IOC_SET_CONTRAST        _IOW(_VPP_TYPE, 0x02, int)
#define VPP_IOC_SET_SATURATION      _IOW(_VPP_TYPE, 0x03, int)
#define VPP_IOC_SET_HUE             _IOW(_VPP_TYPE, 0x04, int)
#define VPP_IOC_SET_SHARPNESS       _IOW(_VPP_TYPE, 0x05, int)
#define VPP_IOC_SET_BRIGHTNESS_POST _IOW(_VPP_TYPE, 0x06, int)
#define VPP_IOC_SET_CONTRAST_POST   _IOW(_VPP_TYPE, 0x07, int)
#define VPP_IOC_SET_SATURATION_POST _IOW(_VPP_TYPE, 0x08, int)
#define VPP_IOC_SET_HUE_POST        _IOW(_VPP_TYPE, 0x09, int)
#define VPP_IOC_SET_WB              _IOW(_VPP_TYPE, 0x0a, struct vpp_white_balance_s)
#define VPP_IOC_SET_PRE_GAMMA_DATA  _IOW(_VPP_TYPE, 0x0b, struct vpp_gamma_table_s)
#define VPP_IOC_SET_GAMMA_DATA      _IOW(_VPP_TYPE, 0x0c, struct vpp_gamma_table_s)
#define VPP_IOC_SET_MATRIX_PARAM    _IOW(_VPP_TYPE, 0x0d, struct vpp_mtrx_info_s)
#define VPP_IOC_SET_MODULE_STATUS   _IOW(_VPP_TYPE, 0x0e, struct vpp_module_ctrl_s)
#define VPP_IOC_SET_PQ_STATE        _IOW(_VPP_TYPE, 0x0f, struct vpp_pq_state_s)
#define VPP_IOC_SET_PC_MODE         _IOW(_VPP_TYPE, 0x10, enum vpp_pc_mode_e)

#define VPP_IOC_SET_DNLP_PARAM      _IOW(_VPP_TYPE, 0x11, struct vpp_dnlp_curve_param_s)
#define VPP_IOC_SET_LC_CURVE        _IOW(_VPP_TYPE, 0x12, struct vpp_lc_curve_s)
#define VPP_IOC_SET_LC_PARAM        _IOW(_VPP_TYPE, 0x13, struct vpp_lc_param_s)
#define VPP_IOC_SET_CSC_TYPE        _IOW(_VPP_TYPE, 0x14, enum vpp_csc_type_e)
#define VPP_IOC_SET_3DLUT_DATA      _IOW(_VPP_TYPE, 0x15, struct vpp_lut3d_table_s)

#define VPP_IOC_SET_HDR_TMO         _IOW(_VPP_TYPE, 0x16, struct vpp_hdr_lut_s)
#define VPP_IOC_SET_HDR_OETF        _IOW(_VPP_TYPE, 0x17, struct vpp_hdr_lut_s)
#define VPP_IOC_SET_HDR_EOTF        _IOW(_VPP_TYPE, 0x18, struct vpp_hdr_lut_s)
#define VPP_IOC_SET_HDR_CGAIN       _IOW(_VPP_TYPE, 0x19, struct vpp_hdr_lut_s)

#define VPP_IOC_SET_CM_CURVE        _IOW(_VPP_TYPE, 0x1a, struct vpp_cm_curve_s)
#define VPP_IOC_SET_CM_OFFSET_CURVE _IOW(_VPP_TYPE, 0x1b, struct vpp_cm_curve_s)

#define VPP_IOC_SET_EYE_PROTECT     _IOW(_VPP_TYPE, 0x1c, struct vpp_eye_protect_s)
#define VPP_IOC_SET_AIPQ_TABLE      _IOW(_VPP_TYPE, 0x1d, struct vpp_aipq_table_s)

#define VPP_IOC_SET_COLOR_PRIMARY_STATUS _IOW(_VPP_TYPE, 0x1e, int)
#define VPP_IOC_SET_COLOR_PRIMARY   _IOW(_VPP_TYPE, 0x1f, struct vpp_color_primary_s)

#define VPP_IOC_SET_OVERSCAN_TABLE  _IOW(_VPP_TYPE, 0x20, struct vpp_overscan_table_s)

#define VPP_IOC_GET_PC_MODE         _IOR(_VPP_TYPE, 0x80, enum vpp_pc_mode_e)
#define VPP_IOC_GET_CSC_TYPE        _IOR(_VPP_TYPE, 0x81, enum vpp_csc_type_e)
#define VPP_IOC_GET_HDR_TYPE        _IOR(_VPP_TYPE, 0x82, enum vpp_hdr_type_e)
#define VPP_IOC_GET_COLOR_PRIM      _IOR(_VPP_TYPE, 0x83, enum vpp_color_primary_e)
#define VPP_IOC_GET_HDR_METADATA    _IOR(_VPP_TYPE, 0x84, struct vpp_hdr_metadata_s)
#define VPP_IOC_GET_HIST_AVG        _IOR(_VPP_TYPE, 0x85, struct vpp_hist_ave_s)
#define VPP_IOC_GET_HIST_BIN        _IOR(_VPP_TYPE, 0x86, struct vpp_hist_bin_s)
#define VPP_IOC_GET_PQ_STATE        _IOR(_VPP_TYPE, 0x87, struct vpp_pq_state_s)
#define VPP_IOC_GET_HDR_HIST        _IOR(_VPP_TYPE, 0x88, struct vpp_hdr_hist_param_s)
#define VPP_IOC_GET_PRE_GAMMA_LEN   _IOR(_VPP_TYPE, 0x89, int)
#define VPP_IOC_GET_GAMMA_LEN       _IOR(_VPP_TYPE, 0x8a, int)

#endif
