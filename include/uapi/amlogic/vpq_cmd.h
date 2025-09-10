/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __VPQ_CMD_H__
#define __VPQ_CMD_H__

#define VPQ_HIST_BIN_COUNT			(64)
#define VPQ_COLOR_HIST_BIN_COUNT	(32)
#define VPQ_MTRX_COEF_LEN           (9)
#define VPQ_MTRX_OFFSET_LEN         (3)
#define VPQ_COLOR_PRIMARY_LEN       (8)
#define VPQ_HDR_HIST_BIN_COUNT      (128)

enum vpq_chip_id_e {
	VPQ_CHIP_ID_NULL = 0,
	VPQ_CHIP_ID_T5W,
	VPQ_CHIP_ID_T3,
	VPQ_CHIP_ID_T6W,
	VPQ_CHIP_ID_T6X,
};

struct vpq_pqtable_bin_param_s {
	unsigned int index;
	unsigned int len;
	union {
		void *ptr;
		long long ptr_length;
	};
};

struct vpq_pqmodule_cfg_s {
	unsigned char pq_en;
	unsigned char vadj1_en;
	unsigned char vd1_ctrst_en;
	unsigned char vadj2_en;
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

	unsigned char di_en;
	unsigned char mcdi_en;
	unsigned char deblock_en;
	unsigned char demosquito_en;
	unsigned char smoothplus_en;
	unsigned char nr_en;
	unsigned char hdrtmo_en;
	unsigned char ai_en;
	unsigned char aisr_en;

	unsigned char reserved;
};

enum vpq_module_e {
	VPQ_MODULE_VADJ1 = 0,
	VPQ_MODULE_VADJ2,
	VPQ_MODULE_PREGAMMA,
	VPQ_MODULE_GAMMA,
	VPQ_MODULE_WB,
	VPQ_MODULE_DNLP,
	VPQ_MODULE_CCORING,
	VPQ_MODULE_SR0,
	VPQ_MODULE_SR1,
	VPQ_MODULE_LC,
	VPQ_MODULE_CM,
	VPQ_MODULE_BLE,
	VPQ_MODULE_BLS,
	VPQ_MODULE_LUT3D,
	VPQ_MODULE_DEJAGGY_SR0,
	VPQ_MODULE_DEJAGGY_SR1,
	VPQ_MODULE_DERING_SR0,
	VPQ_MODULE_DERING_SR1,
	VPQ_MODULE_ALL,
};

struct vpq_pqmodule_status_s {
	enum vpq_module_e module;
	int status;
};

struct vpq_gamma_vari_table_s {
	unsigned short number;
	union {
		unsigned short *r_data;
		long long r_data_len;
	};
	union {
		unsigned short *g_data;
		long long g_data_len;
	};
	union {
		unsigned short *b_data;
		long long b_data_len;
	};
};

struct vpq_rgb_ogo_s {
	int r_pre_offset;  /* range -1024~+1023, default is 0 */
	int g_pre_offset;  /* range -1024~+1023, default is 0 */
	int b_pre_offset;  /* range -1024~+1023, default is 0 */
	unsigned int r_gain; /* range 0~2047, default is 1024 (1.0x) */
	unsigned int g_gain; /* range 0~2047, default is 1024 (1.0x) */
	unsigned int b_gain; /* range 0~2047, default is 1024 (1.0x) */
	int r_post_offset; /* range -1024~+1023, default is 0 */
	int g_post_offset; /* range -1024~+1023, default is 0 */
	int b_post_offset; /* range -1024~+1023, default is 0 */
};

enum vpq_mtrx_type_e {
	VPQ_MTRX_VD1 = 0,
	VPQ_MTRX_POST,
	VPQ_MTRX_POST2,
	VPQ_MTRX_MAX,
};

struct vpq_mtrx_param_s {
	unsigned int pre_offset[VPQ_MTRX_OFFSET_LEN];
	unsigned int matrix_coef[VPQ_MTRX_COEF_LEN];
	unsigned int post_offset[VPQ_MTRX_OFFSET_LEN];
	unsigned int right_shift;
};

struct vpq_mtrx_info_s {
	enum vpq_mtrx_type_e mtrx_sel;
	struct vpq_mtrx_param_s mtrx_param;
};

enum vpq_cms_color_type_e {
	VPQ_CM_9_COLOR = 0,
	VPQ_CM_14_COLOR,
	VPQ_CM_MAX_COLOR,
};

enum vpq_cms_9_color_e {
	VPQ_COLOR_RED = 0,
	VPQ_COLOR_GREEN,
	VPQ_COLOR_BLUE,
	VPQ_COLOR_CYAN,
	VPQ_COLOR_MAGENTA,
	VPQ_COLOR_YELLOW,
	VPQ_COLOR_SKIN,
	VPQ_COLOR_YELLOW_GREEN,
	VPQ_COLOR_BLUE_GREEN,
	VPQ_COLOR_MAX,
};

enum vpq_cms_14_color_e {
	VPQ_COLOR_14_BLUE_PURPLE = 0,
	VPQ_COLOR_14_PURPLE,
	VPQ_COLOR_14_PURPLE_RED,
	VPQ_COLOR_14_RED,
	VPQ_COLOR_14_FLESHTONE_CHEEKS,
	VPQ_COLOR_14_FLESHTONE_HAIR_CHEEKS,
	VPQ_COLOR_14_FLESHTONE_YELLOW,
	VPQ_COLOR_14_YELLOW,
	VPQ_COLOR_14_YELLOW_GREEN,
	VPQ_COLOR_14_GREEN,
	VPQ_COLOR_14_GREEN_CYAN,
	VPQ_COLOR_14_CYAN,
	VPQ_COLOR_14_CYAN_BLUE,
	VPQ_COLOR_14_BLUE,
	VPQ_COLOR_14_MAX,
};

enum vpq_cms_type_e {
	VPQ_CMS_SAT = 0,
	VPQ_CMS_HUE,
	VPQ_CMS_LUMA,
	VPQ_CMS_MAX,
};

struct vpq_cms_s {
	enum vpq_cms_color_type_e color_type;
	enum vpq_cms_9_color_e color_9;
	enum vpq_cms_14_color_e color_14;
	enum vpq_cms_type_e cms_type;
	int value;
};

enum vpq_csc_type_e {
	VPQ_CSC_MATRIX_NULL                        = 0,
	VPQ_CSC_MATRIX_RGB_YUV601                  = 0x1,
	VPQ_CSC_MATRIX_RGB_YUV601F                 = 0x2,
	VPQ_CSC_MATRIX_RGB_YUV709                  = 0x3,
	VPQ_CSC_MATRIX_RGB_YUV709F                 = 0x4,
	VPQ_CSC_MATRIX_YUV601_RGB                  = 0x10,
	VPQ_CSC_MATRIX_YUV601_YUV601F              = 0x11,
	VPQ_CSC_MATRIX_YUV601_YUV709               = 0x12,
	VPQ_CSC_MATRIX_YUV601_YUV709F              = 0x13,
	VPQ_CSC_MATRIX_YUV601F_RGB                 = 0x14,
	VPQ_CSC_MATRIX_YUV601F_YUV601              = 0x15,
	VPQ_CSC_MATRIX_YUV601F_YUV709              = 0x16,
	VPQ_CSC_MATRIX_YUV601F_YUV709F             = 0x17,
	VPQ_CSC_MATRIX_YUV709_RGB                  = 0x20,
	VPQ_CSC_MATRIX_YUV709_YUV601               = 0x21,
	VPQ_CSC_MATRIX_YUV709_YUV601F              = 0x22,
	VPQ_CSC_MATRIX_YUV709_YUV709F              = 0x23,
	VPQ_CSC_MATRIX_YUV709F_RGB                 = 0x24,
	VPQ_CSC_MATRIX_YUV709F_YUV601              = 0x25,
	VPQ_CSC_MATRIX_YUV709F_YUV709              = 0x26,
	VPQ_CSC_MATRIX_YUV601L_YUV709L             = 0x27,
	VPQ_CSC_MATRIX_YUV709L_YUV601L             = 0x28,
	VPQ_CSC_MATRIX_YUV709F_YUV601F             = 0x29,
	VPQ_CSC_MATRIX_BT2020YUV_BT2020RGB         = 0x40,
	VPQ_CSC_MATRIX_BT2020RGB_709RGB,
	VPQ_CSC_MATRIX_BT2020RGB_CUSRGB,
	VPQ_CSC_MATRIX_BT2020YUV_BT2020RGB_DYNAMIC = 0x50,
	VPQ_CSC_MATRIX_BT2020YUV_BT2020RGB_CUVA    = 0x51,
	VPQ_CSC_MATRIX_DEFAULT_CSCTYPE             = 0xffff,
};

enum vpq_lut3d_data_type_e {
	VPQ_LUT3D_INPUT_PARAM = 0,
	VPQ_LUT3D_UNIFY_KEY,
	VPQ_LUT3D_BIN_FILE,
};

struct vpq_lut3d_table_s {
	enum vpq_lut3d_data_type_e data_type;
	unsigned char data_index;
	unsigned char data_check;
	unsigned int data_size;
	int *data;
};

enum vpq_hdr_lut_type_e {
	VPQ_HDR_LUT_TYPE_HLG = 0,
	VPQ_HDR_LUT_TYPE_HDR,
	VPQ_HDR_LUT_TYPE_MAX,
};

struct vpq_hdr_lut_s {
	enum vpq_hdr_lut_type_e lut_type;
	unsigned int lut_size;
	union {
		int *lut_data;
		long long lut_len;
	};
};

enum vpq_rgb_mode_e {
	VPQ_MODE_R = 0,
	VPQ_MODE_G,
	VPQ_MODE_B,
	VPQ_MODE_RGB_MAX,
};

struct vpq_eye_protect_s {
	int enable;
	int rgb[VPQ_MODE_RGB_MAX];
};

struct vpq_overscan_table_s {
	int id;
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

enum vpq_pc_mode_e {
	VPQ_PC_MODE_OFF = 0,
	VPQ_PC_MODE_ON,
};

struct vpq_color_primary_s {
	unsigned int data_src[VPQ_COLOR_PRIMARY_LEN];
	unsigned int data_dest[VPQ_COLOR_PRIMARY_LEN];
};

enum vpq_frame_status_e {
	VPQ_VFRAME_UNSTABLE = 0,
	VPQ_VFRAME_START,
	VPQ_VFRAME_STOP,
};

struct vpq_frame_info_s {
	unsigned int shared_fd;
	unsigned int reserved[8];
};

struct vpq_histgm_ave_s {
	unsigned int sum;
	int width;
	int height;
	int ave;
};

struct vpq_histgm_param_s {
	unsigned int hist_pow;
	unsigned int luma_sum;
	unsigned int pixel_sum;
	unsigned int histgm[VPQ_HIST_BIN_COUNT];
	unsigned int dark_histgm[VPQ_HIST_BIN_COUNT];
	unsigned int hue_histgm[VPQ_COLOR_HIST_BIN_COUNT];
	unsigned int sat_histgm[VPQ_COLOR_HIST_BIN_COUNT];
};

struct vpq_hdr_hist_param_s {
	unsigned int data_rgb_max[VPQ_HDR_HIST_BIN_COUNT];
};

enum vpq_color_primary_e {
	VPQ_COLOR_PRI_NULL = 0,
	VPQ_COLOR_PRI_BT601,
	VPQ_COLOR_PRI_BT709,
	VPQ_COLOR_PRI_BT2020,
	VPQ_COLOR_PRI_MAX,
};

struct vpq_hdr_metadata_s {
	unsigned int primaries[3][2]; /* normalized 50000 in G,B,R order */
	unsigned int white_point[2];  /* normalized 50000 */
	unsigned int luminance[2];    /* max/min luminance, normalized 10000 */
};

enum vpq_event_info_e {
	VPQ_EVENT_NONE = 0,
	VPQ_EVENT_SIG_INFO_CHANGE,
};

enum vpq_source_type_e {
	VPQ_SRC_TYPE_MPEG = 0,
	VPQ_SRC_TYPE_ATV,
	VPQ_SRC_TYPE_CVBS,
	VPQ_SRC_TYPE_HDMI,
};

enum vpq_hdmi_port_e {
	VPQ_HDMI_PORT_NULL = 0,
	VPQ_HDMI_PORT_0,
	VPQ_HDMI_PORT_1,
	VPQ_HDMI_PORT_2,
	VPQ_HDMI_PORT_3,
};

enum vpq_sig_mode_e {
	VPQ_SIG_MODE_OTHERS = 0,
	VPQ_SIG_MODE_PAL,
	VPQ_SIG_MODE_NTSC,
	VPQ_SIG_MODE_SECAM,
};

enum vpq_hdr_type_e {
	VPQ_HDR_TYPE_NONE = 0,
	VPQ_HDR_TYPE_HDR10,
	VPQ_HDR_TYPE_HDR10PLUS,
	VPQ_HDR_TYPE_DOBVI,
	VPQ_HDR_TYPE_PRIMESL,
	VPQ_HDR_TYPE_HLG,
	VPQ_HDR_TYPE_SDR,
	VPQ_HDR_TYPE_MVC,
};

enum vpq_scan_mode_e {
	VPQ_SCAN_MODE_NULL = 0,
	VPQ_SCAN_MODE_PROGRESSIVE,
	VPQ_SCAN_MODE_INTERLACED,
};

struct vpq_signal_info_s {
	enum vpq_source_type_e src_type;
	enum vpq_hdmi_port_e hdmi_port;
	enum vpq_sig_mode_e sig_mode;
	enum vpq_hdr_type_e hdr_type;
	enum vpq_scan_mode_e scan_mode;
	unsigned int sig_fmt; /* refer tvin_sig_fmt_e */
	unsigned int trans_fmt; /* refer tvin_trans_fmt */
	unsigned int height;
	unsigned int width;
	unsigned int fps;
};

/* DPSS Start */
enum vpq_dnr_param_e {
	SNR_LPF_ALPHA_CURV2,
	SNR_LPF_ALPHA_CURV1,
	SNR_LPF_ALPHA_CURV0,
	SNR_LPF_CUR_NOISE_GAIN,
	SNR_LPF_CUR_NOISE_OFST,
	SNR_LPF_PRE_NOISE_GAIN,
	SNR_LPF_PRE_NOISE_OFST,
	SNR_LPF_IIR_NOISE_GAIN,
	SNR_LPF_IIR_NOISE_OFST,
	TNR_NE_FINAL_GAIN,
	TNR_NE_FINAL_OFST,
	TNR_NE_FORCE_Z_SAD_OFST_3,
	TNR_NE_FORCE_Z_SAD_OFST_2,
	TNR_NE_FORCE_Z_SAD_OFST_1,
	TNR_NE_FORCE_Z_SAD_OFST_0,
	TNR_NE_FORCE_Z_MIN_NOISE_LEVEL,
	TNR_MV_VALID_ALPHA_LUT_7,
	TNR_MV_VALID_ALPHA_LUT_6,
	TNR_MV_VALID_ALPHA_LUT_5,
	TNR_MV_VALID_ALPHA_LUT_4,
	TNR_MV_VALID_ALPHA_LUT_3,
	TNR_MV_VALID_ALPHA_LUT_2,
	TNR_MV_VALID_ALPHA_LUT_1,
	TNR_MV_VALID_ALPHA_LUT_0,
	TNR_IIR_GAIN_VSLUMA_7,
	TNR_IIR_GAIN_VSLUMA_6,
	TNR_IIR_GAIN_VSLUMA_5,
	TNR_IIR_GAIN_VSLUMA_4,
	TNR_IIR_GAIN_VSLUMA_3,
	TNR_IIR_GAIN_VSLUMA_2,
	TNR_IIR_GAIN_VSLUMA_1,
	TNR_IIR_GAIN_VSLUMA_0,
	TNR_MV_VALID_CHECK_EN,
	TNR_MV_VALID_EDGE_CHK_EN,
	SNR_LPF_STRENGTH_VSLUMA_7,
	SNR_LPF_STRENGTH_VSLUMA_6,
	SNR_LPF_STRENGTH_VSLUMA_5,
	SNR_LPF_STRENGTH_VSLUMA_4,
	SNR_LPF_STRENGTH_VSLUMA_3,
	SNR_LPF_STRENGTH_VSLUMA_2,
	SNR_LPF_STRENGTH_VSLUMA_1,
	SNR_LPF_STRENGTH_VSLUMA_0,
	DNR_MAX,
};

struct vpq_dnr_param_s {
	int param[DNR_MAX];
};

enum vpq_dblk_param_e {
	DELTA_OUT_ALPHA,
	BLK_H_STRENGTH,
	BLK_V_STRENGTH,
	DBLK_V_DELTA_GAIN_1,
	DBLK_V_DELTA_GAIN_0,
	DBLK_V_DELTA_MAX_1,
	DBLK_V_DELTA_MAX_0,
	DBLK_H_DELTA_GAIN_1,
	DBLK_H_DELTA_GAIN_0,
	DBLK_H_DELTA_MAX_1,
	DBLK_H_DELTA_MAX_0,
	DBLK_MAX,
};

struct vpq_dblk_param_s {
	int param[DBLK_MAX];
};

enum vpq_demosquito_param_e {
	DM_EDGE_BLEND_RATE,
	DM_EDGE_LEVEL_GAIN_15,
	DM_EDGE_LEVEL_GAIN_14,
	DM_EDGE_LEVEL_GAIN_13,
	DM_EDGE_LEVEL_GAIN_12,
	DM_EDGE_LEVEL_GAIN_11,
	DM_EDGE_LEVEL_GAIN_10,
	DM_EDGE_LEVEL_GAIN_9,
	DM_EDGE_LEVEL_GAIN_8,
	DM_EDGE_LEVEL_GAIN_7,
	DM_EDGE_LEVEL_GAIN_6,
	DM_EDGE_LEVEL_GAIN_5,
	DM_EDGE_LEVEL_GAIN_4,
	DM_EDGE_LEVEL_GAIN_3,
	DM_EDGE_LEVEL_GAIN_2,
	DM_EDGE_LEVEL_GAIN_1,
	DM_EDGE_LEVEL_GAIN_0,
	DM_STRENGTH_VS_MOS_LEVEL_7,
	DM_STRENGTH_VS_MOS_LEVEL_6,
	DM_STRENGTH_VS_MOS_LEVEL_5,
	DM_STRENGTH_VS_MOS_LEVEL_4,
	DM_STRENGTH_VS_MOS_LEVEL_3,
	DM_STRENGTH_VS_MOS_LEVEL_2,
	DM_STRENGTH_VS_MOS_LEVEL_1,
	DM_STRENGTH_VS_MOS_LEVEL_0,
	DM_STRENGTH_CHROMA_GAIN,
	DM_ME_SUM_SAD_THD,
	DM_ME_AVG_H_SAD_THD,
	DM_FLAT_THD2,
	DM_FLAT_THD1,
	DM_FLAT_THD0,
	DM_EDGE_TH,
	DM_FLAT_MAX_SAD_GAIN1,
	DM_FLAT_MAX_SAD_GAIN0,
	DM_FLAT_MAX_SAD_THN,
	DM_FLAT_MAX_SAD_TH0,
	DM_EDGE_MIN_SAD_GAIN1,
	DM_EDGE_MIN_SAD_GAIN0,
	DM_EDGE_MIN_SAD_THN,
	DM_EDGE_MIN_SAD_TH0,
	DM_SGM_LEVEL_MIN_THD,
	DM_SGM_LEVEL_MAX_THD2,
	DM_SGM_LEVEL_MAX_THD,
	DM_SGM_LEVEL_EDGE_OFST,
	DM_MAX,
};

struct vpq_demosquito_param_s {
	int param[DM_MAX];
};

enum vpq_dct_param_e {
	DECONTOUR_ENABLE_0,
	DECONTOUR_ENABLE_1,
	DIRMAP_ENABLE,
	DIR_SMGRD_EN,
	DIR_NOISE_TH,
	DIR_GAIN,
	DIR_WIN,
	DIR_MAG,
	AVG_COR_TH,
	BIG_COR_TH,
	FLT_COR_EN,
	MAP_BLD_MODE,
	MAP_BLD_ALPHA,
	BLD1_MODE,
	PMAP_MANUAL_ALP,
	PMAP_DETAIL_ENABLE,
	PMAP_LUMA_SCL_ENABLE,
	PMAP_COLORPRT_ENABLE,
	PMAP_MANUAL_ENABLE,
	PMAP_LUMA_SCL_SEL,
	PMAP_COLORPROTECT_SEL,
	PMAP_LUMA_GAIN,
	PMAP_LUMA_MAG,
	PMAP_COLORPROTECT_GAIN,
	PMAP_COLORPROTECT_MAG,
	PMAP_LUMA_SCL_LUT_0,
	PMAP_LUMA_SCL_LUT_1,
	PMAP_LUMA_SCL_LUT_2,
	PMAP_LUMA_SCL_LUT_3,
	PMAP_LUMA_SCL_LUT_4,
	PMAP_LUMA_SCL_LUT_5,
	PMAP_LUMA_SCL_LUT_6,
	PMAP_LUMA_SCL_LUT_7,
	PMAP_LUMA_SCL_LUT_8,
	DCT_MAX,
};

struct vpq_dct_param_s {
	int param[DCT_MAX];
};

/* DPSS End */

#define VPQ_DEVICE_NAME                  "vpq"
#define VPQ_IOC_MAGIC                    'C'

/* SET CMD */
#define VPQ_IOC_SET_PQTABLE_PARAM         _IOW(VPQ_IOC_MAGIC, 0x00, struct vpq_pqtable_bin_param_s)
#define VPQ_IOC_SET_PQ_MODULE_CFG         _IOW(VPQ_IOC_MAGIC, 0x01, struct vpq_pqmodule_cfg_s)
#define VPQ_IOC_SET_PQ_MODULE_STATUS      _IOW(VPQ_IOC_MAGIC, 0x02, struct vpq_pqmodule_status_s)
#define VPQ_IOC_SET_BRIGHTNESS            _IOW(VPQ_IOC_MAGIC, 0x03, int)
#define VPQ_IOC_SET_CONTRAST              _IOW(VPQ_IOC_MAGIC, 0x04, int)
#define VPQ_IOC_SET_SATURATION            _IOW(VPQ_IOC_MAGIC, 0x05, int)
#define VPQ_IOC_SET_HUE                   _IOW(VPQ_IOC_MAGIC, 0x06, int)
#define VPQ_IOC_SET_SHARPNESS             _IOW(VPQ_IOC_MAGIC, 0x07, int)
#define VPQ_IOC_SET_BRIGHTNESS_POST       _IOW(VPQ_IOC_MAGIC, 0x08, int)
#define VPQ_IOC_SET_CONTRAST_POST         _IOW(VPQ_IOC_MAGIC, 0x09, int)
#define VPQ_IOC_SET_SATURATION_POST       _IOW(VPQ_IOC_MAGIC, 0x0a, int)
#define VPQ_IOC_SET_HUE_POST              _IOW(VPQ_IOC_MAGIC, 0x0b, int)
#define VPQ_IOC_SET_OVERSCAN_TABLE        _IOW(VPQ_IOC_MAGIC, 0x0c, struct vpq_overscan_table_s)
#define VPQ_IOC_SET_BACKLIGHT             _IOW(VPQ_IOC_MAGIC, 0x0d, int)
#define VPQ_IOC_SET_GAMMA_TABLE           _IOW(VPQ_IOC_MAGIC, 0x0e, struct vpq_gamma_vari_table_s)
#define VPQ_IOC_SET_RGB_OGO               _IOW(VPQ_IOC_MAGIC, 0x0f, struct vpq_rgb_ogo_s)
#define VPQ_IOC_SET_MATRIX_PARAM          _IOW(VPQ_IOC_MAGIC, 0x10, struct vpq_mtrx_info_s)
#define VPQ_IOC_SET_COLOR_BASE            _IOW(VPQ_IOC_MAGIC, 0x11, unsigned char)
#define VPQ_IOC_SET_COLOR_CUSTOMIZE       _IOW(VPQ_IOC_MAGIC, 0x12, struct vpq_cms_s)
#define VPQ_IOC_SET_HDMI_COLOR_RANGE_MODE _IOW(VPQ_IOC_MAGIC, 0x13, unsigned char)
#define VPQ_IOC_SET_COLOR_SPACE           _IOW(VPQ_IOC_MAGIC, 0x14, unsigned char)
#define VPQ_IOC_SET_GLOBAL_DIMMING        _IOW(VPQ_IOC_MAGIC, 0x15, unsigned char)
#define VPQ_IOC_SET_LOCAL_DIMMING         _IOW(VPQ_IOC_MAGIC, 0x16, unsigned char)
#define VPQ_IOC_SET_BLACK_STRETCH         _IOW(VPQ_IOC_MAGIC, 0x17, unsigned char)
#define VPQ_IOC_SET_BLUE_STRETCH          _IOW(VPQ_IOC_MAGIC, 0x18, unsigned char)
#define VPQ_IOC_SET_DNLP_MODE             _IOW(VPQ_IOC_MAGIC, 0x19, unsigned char)
#define VPQ_IOC_SET_LC_MODE               _IOW(VPQ_IOC_MAGIC, 0x1a, unsigned char)
#define VPQ_IOC_SET_SR                    _IOW(VPQ_IOC_MAGIC, 0x1b, int)
#define VPQ_IOC_SET_CSC_TYPE              _IOW(VPQ_IOC_MAGIC, 0x1c, enum vpq_csc_type_e)
#define VPQ_IOC_SET_3DLUT_DATA            _IOW(VPQ_IOC_MAGIC, 0x1d, struct vpq_lut3d_table_s)
#define VPQ_IOC_SET_HDR_TMO_MODE          _IOW(VPQ_IOC_MAGIC, 0x1e, unsigned char)
#define VPQ_IOC_SET_HDR_TMO               _IOW(VPQ_IOC_MAGIC, 0x1f, struct vpq_hdr_lut_s)
#define VPQ_IOC_SET_HDR_OETF              _IOW(VPQ_IOC_MAGIC, 0x20, struct vpq_hdr_lut_s)
#define VPQ_IOC_SET_HDR_EOTF              _IOW(VPQ_IOC_MAGIC, 0x21, struct vpq_hdr_lut_s)
#define VPQ_IOC_SET_HDR_CGAIN             _IOW(VPQ_IOC_MAGIC, 0x22, struct vpq_hdr_lut_s)
#define VPQ_IOC_SET_AIPQ_MODE             _IOW(VPQ_IOC_MAGIC, 0x23, unsigned char)
#define VPQ_IOC_SET_AISR_MODE             _IOW(VPQ_IOC_MAGIC, 0x24, unsigned char)
#define VPQ_IOC_SET_CHROMA_CORING         _IOW(VPQ_IOC_MAGIC, 0x25, unsigned char)
#define VPQ_IOC_SET_EYE_PROTECT           _IOW(VPQ_IOC_MAGIC, 0x26, struct vpq_eye_protect_s)
#define VPQ_IOC_SET_CABC                  _IO(VPQ_IOC_MAGIC, 0x27)
#define VPQ_IOC_SET_ADD                   _IO(VPQ_IOC_MAGIC, 0x28)
#define VPQ_IOC_SET_PC_MODE               _IOW(VPQ_IOC_MAGIC, 0x29, enum vpq_pc_mode_e)
#define VPQ_IOC_SET_COLOR_PRIMARY_STATUS  _IOW(VPQ_IOC_MAGIC, 0x2a, int)
#define VPQ_IOC_SET_COLOR_PRIMARY         _IOW(VPQ_IOC_MAGIC, 0x2b, struct vpq_color_primary_s)
#define VPQ_IOC_SET_FRAME_STATUS          _IOW(VPQ_IOC_MAGIC, 0x2c, enum vpq_frame_status_e)
#define VPQ_IOC_SET_FRAME                 _IOW(VPQ_IOC_MAGIC, 0x2d, struct vpq_frame_info_s)

#define VPQ_IOC_SET_NR                    _IOW(VPQ_IOC_MAGIC, 0x40, unsigned char)
#define VPQ_IOC_SET_AUTO_NR               _IOW(VPQ_IOC_MAGIC, 0x41, unsigned char)
#define VPQ_IOC_SET_DEBLOCK               _IOW(VPQ_IOC_MAGIC, 0x42, unsigned char)
#define VPQ_IOC_SET_DEMOSQUITO            _IOW(VPQ_IOC_MAGIC, 0x43, unsigned char)
#define VPQ_IOC_SET_SMOOTHPLUS_MODE       _IOW(VPQ_IOC_MAGIC, 0x44, unsigned char)
#define VPQ_IOC_SET_MCDI_MODE             _IOW(VPQ_IOC_MAGIC, 0x45, unsigned char)

/* GET CMD */
#define VPQ_IOC_GET_CHIP_ID               _IOR(VPQ_IOC_MAGIC, 0x50, enum vpq_chip_id_e)
#define VPQ_IOC_GET_PC_MODE               _IOR(VPQ_IOC_MAGIC, 0x51, enum vpq_pc_mode_e)
#define VPQ_IOC_GET_HIST_AVG              _IOR(VPQ_IOC_MAGIC, 0x52, struct vpq_histgm_ave_s)
#define VPQ_IOC_GET_HIST_BIN              _IOR(VPQ_IOC_MAGIC, 0x53, struct vpq_histgm_param_s)
#define VPQ_IOC_GET_HDR_HIST              _IOR(VPQ_IOC_MAGIC, 0x54, struct vpq_hdr_hist_param_s)
#define VPQ_IOC_GET_CSC_TYPE              _IOR(VPQ_IOC_MAGIC, 0x55, enum vpq_csc_type_e)
#define VPQ_IOC_GET_COLOR_PRIM            _IOR(VPQ_IOC_MAGIC, 0x56, enum vpq_color_primary_e)
#define VPQ_IOC_GET_HDR_METADATA          _IOR(VPQ_IOC_MAGIC, 0x57, struct vpq_hdr_metadata_s)
#define VPQ_IOC_GET_EVENT_INFO	          _IOR(VPQ_IOC_MAGIC, 0x58, enum vpq_event_info_e)
#define VPQ_IOC_GET_SIGNAL_INFO           _IOR(VPQ_IOC_MAGIC, 0x59, struct vpq_signal_info_s)

/* DPSS Start */
#define VPQ_IOC_SET_NR_DPSS               _IOW(VPQ_IOC_MAGIC, 0x100, struct vpq_dnr_param_s)
#define VPQ_IOC_SET_DEBLOCK_DPSS          _IOW(VPQ_IOC_MAGIC, 0x101, struct vpq_dblk_param_s)
#define VPQ_IOC_SET_DEMOSQUITO_DPSS       _IOW(VPQ_IOC_MAGIC, 0x102, struct vpq_demosquito_param_s)
#define VPQ_IOC_SET_SMOOTHPLUS_DPSS       _IOW(VPQ_IOC_MAGIC, 0x103, struct vpq_dct_param_s)
/* DPSS End */

#endif
