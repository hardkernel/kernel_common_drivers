// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#include "vpp_common.h"
#include "vpp_pq_mgr.h"
#include "vpp_modules_inc.h"
#include "vpp_vf_proc.h"
#include "vpp_data.h"

#define RET_POINT_FAIL (-1)

static enum vpp_chip_type_e chip_type;
static struct vpp_pq_mgr_settings pq_mgr_settings;

/*For 3D LUT*/
static bool lut3d_db_initial;

/*Internal functions*/
static int _modules_init(struct vpp_dev_s *dev)
{
	vpp_module_vadj_init(dev);
	vpp_module_matrix_init(dev);
	vpp_module_gamma_init(dev);
	vpp_module_go_init(dev);
	vpp_module_meter_init(dev);
	vpp_module_ve_init(dev);
	vpp_module_dnlp_init(dev);
	vpp_module_lc_init(dev);
	vpp_module_cm_init(dev);
	vpp_module_sr_init(dev);
	vpp_module_lut3d_init(dev);
	vpp_vf_init(dev);

	return 0;
}

static void _buffer_init(void)
{
	unsigned int len = 0;
	unsigned int buf_size = 0;

	len = vpp_module_pre_gamma_get_table_len();
	if (len != 0) {
		buf_size = len * sizeof(unsigned int);
		pq_mgr_settings.cur_pre_gamma_tbl.r_data =
			kmalloc(buf_size, GFP_KERNEL);
		pq_mgr_settings.cur_pre_gamma_tbl.g_data =
			kmalloc(buf_size, GFP_KERNEL);
		pq_mgr_settings.cur_pre_gamma_tbl.b_data =
			kmalloc(buf_size, GFP_KERNEL);
	}

	len = vpp_module_lcd_gamma_get_table_len();
	if (len != 0) {
		buf_size = len * sizeof(unsigned int);
		pq_mgr_settings.cur_gamma_tbl.r_data =
			kmalloc(buf_size, GFP_KERNEL);
		pq_mgr_settings.cur_gamma_tbl.g_data =
			kmalloc(buf_size, GFP_KERNEL);
		pq_mgr_settings.cur_gamma_tbl.b_data =
			kmalloc(buf_size, GFP_KERNEL);
	}
}

static void _buffer_free(void)
{
	kfree(pq_mgr_settings.cur_pre_gamma_tbl.r_data);
	kfree(pq_mgr_settings.cur_pre_gamma_tbl.g_data);
	kfree(pq_mgr_settings.cur_pre_gamma_tbl.b_data);
	kfree(pq_mgr_settings.cur_gamma_tbl.r_data);
	kfree(pq_mgr_settings.cur_gamma_tbl.g_data);
	kfree(pq_mgr_settings.cur_gamma_tbl.b_data);
}

static int _set_default_settings(void)
{
	struct vpp_module_go_s go_data = {
		{0, 0, 0},
		{1024, 1024, 1024},
		{0, 0, 0}
	};

	vpp_pq_mgr_set_brightness(0);
	vpp_pq_mgr_set_contrast(0);
	vpp_pq_mgr_set_saturation(0);
	vpp_pq_mgr_set_hue(0);
	vpp_pq_mgr_set_brightness_post(0);
	vpp_pq_mgr_set_contrast_post(0);
	vpp_pq_mgr_set_saturation_post(0);
	vpp_pq_mgr_set_hue_post(0);

	/*WB*/
	vpp_module_go_set_data(&go_data, EN_MODE_DIR);

	/*Gamma*/
	vpp_module_pre_gamma_set_default();
	vpp_module_lcd_gamma_set_default();

	/*CM*/
	vpp_module_cm_set_default();

	return 0;
}

static int _get_mab_from_sat_hue(int sat_val, int hue_val)
{
	int i, ma, mb, mab;

	int hue_cos[] = {
		256, 256, 256, 255, 255, 254, 253, 252, 251, 250, 248, 247, 245, /*0~12*/
		243, 241, 239, 237, 234, 231, 229, 226, 223, 220, 216, 213, 209  /*13~25*/
	};

	int hue_sin[] = {
		-147, -142, -137, -132, -126, -121, -115, -109, -104, -98, /*-25~-16*/
		-92, -86, -80, -74, -68, -62, -56, -50, -44, -38, /*-15~--6*/
		-31, -25, -19, -13, -6, 0, 6, 13, 19, 25, 31, /*-5~5*/
		38, 44, 50, 56, 62, 68, 74, 80, 86, 92, /*6~15*/
		98, 104, 109, 115, 121, 126, 132, 137, 142, 147 /*16~25*/
	};

	sat_val = vpp_check_range(sat_val, (-128), 127);
	hue_val = vpp_check_range(hue_val, (-25), 25);

	i = (hue_val > 0) ? hue_val : (-hue_val);
	ma = (hue_cos[i] * (sat_val + 128)) >> 7;
	mb = (hue_sin[hue_val + 25] * (sat_val + 128)) >> 7;

	ma = vpp_check_range(ma, (-512), 511);
	mb = vpp_check_range(mb, (-512), 511);

	mab = ((ma & 0x03ff) << 16) | (mb & 0x03ff);

	return mab;
}

static void _set_pq_bypass_all(bool enable)
{
	struct vpp_pq_state_s *status = &pq_mgr_settings.pq_status;

//	pr_vpp(PR_DEBUG, "[%s] pq_bypass_all = %d\n", __func__, enable);

	if (enable) {
		vpp_module_vadj_en(false, EN_MODE_RDMA);
		vpp_module_vadj_post_en(false, EN_MODE_RDMA);
		vpp_module_vadj_set_param(EN_VADJ_VD1_RGBBST_EN,
			false, EN_MODE_RDMA);
		vpp_module_vadj_set_param(EN_VADJ_POST_RGBBST_EN,
			false, EN_MODE_RDMA);
		vpp_module_pre_gamma_en(false, EN_MODE_RDMA);
		vpp_module_lcd_gamma_en(false, EN_MODE_RDMA);
		vpp_module_go_en(false, EN_MODE_RDMA);
		vpp_module_dnlp_en(false, EN_MODE_RDMA);
		vpp_module_lc_en(false);
		vpp_module_ve_blkext_en(false, EN_MODE_RDMA);
		vpp_module_ve_ccoring_en(false, EN_MODE_RDMA);
		vpp_module_sr_en(EN_MODE_SR_0, false, EN_MODE_RDMA);
		vpp_module_sr_en(EN_MODE_SR_1, false, EN_MODE_RDMA);
		vpp_module_cm_en(false, EN_MODE_RDMA);
		vpp_module_lut3d_en(false, EN_MODE_RDMA);
	} else {
		vpp_module_vadj_en(status->pq_cfg.vadj1_en, EN_MODE_RDMA);
		vpp_module_vadj_post_en(status->pq_cfg.vadj2_en, EN_MODE_RDMA);
		vpp_module_vadj_set_param(EN_VADJ_VD1_RGBBST_EN,
			status->pq_cfg.vd1_ctrst_en, EN_MODE_RDMA);
		vpp_module_vadj_set_param(EN_VADJ_POST_RGBBST_EN,
			status->pq_cfg.post_ctrst_en, EN_MODE_RDMA);
		vpp_module_pre_gamma_en(status->pq_cfg.pregamma_en, EN_MODE_RDMA);
		vpp_module_lcd_gamma_en(status->pq_cfg.gamma_en, EN_MODE_RDMA);
		vpp_module_go_en(status->pq_cfg.wb_en, EN_MODE_RDMA);
		vpp_module_dnlp_en(status->pq_cfg.dnlp_en, EN_MODE_RDMA);
		vpp_module_lc_en(status->pq_cfg.lc_en);
		vpp_module_ve_blkext_en(status->pq_cfg.black_ext_en, EN_MODE_RDMA);
		vpp_module_ve_ccoring_en(status->pq_cfg.chroma_cor_en, EN_MODE_RDMA);
		vpp_module_sr_en(EN_MODE_SR_0, status->pq_cfg.sharpness0_en, EN_MODE_RDMA);
		vpp_module_sr_en(EN_MODE_SR_1, status->pq_cfg.sharpness1_en, EN_MODE_RDMA);
		vpp_module_cm_en(status->pq_cfg.cm_en, EN_MODE_RDMA);
		vpp_module_lut3d_en(status->pq_cfg.lut3d_en, EN_MODE_RDMA);
	}
}

static void _mtrx_mul_mtrx(int (*mtrx_a)[3], int (*mtrx_b)[3], int (*mtrx_out)[3])
{
	int i, j, k;

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			mtrx_out[i][j] = 0;
			for (k = 0; k < 3; k++)
				mtrx_out[i][j] += mtrx_a[i][k] * mtrx_b[k][j];
		}
	}

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++)
			mtrx_out[i][j] = (mtrx_out[i][j] + (1 << 9)) >> 10;
	}
}

static void _mtrx_multi(int *rgb, int *coef)
{
	int i, j, k;
	int mtrx_rgb[3][3] = {0};
	int mtrx_in[3][3] = {0};
	int mtrx_out[3][3] = {0};

	int mtrx_rgbto709l[3][3] = {
		{187, 629, 63},
		{-103, -346, 450},
		{450, -409, -41},
	};
	int mtrx_709ltorgb[3][3] = {
		{1192, 0, 1836},
		{1192, -218, -547},
		{1192, 2166, 0},
	};

	if (!rgb || !coef)
		return;

	mtrx_in[0][0] = rgb[0];
	mtrx_in[1][1] = rgb[1];
	mtrx_in[2][2] = rgb[2];

	if (mtrx_in[0][0] == 0x400 &&
		mtrx_in[1][1] == 0x400 &&
		mtrx_in[2][2] == 0x400) {
		for (i = 0; i < 3; i++) {
			for (j = 0; j < 3; j++) {
				if (i == j)
					mtrx_out[i][j] = 0x400;
				else
					mtrx_out[i][j] = 0;
			}
		}
	} else {
		_mtrx_mul_mtrx(mtrx_in, mtrx_709ltorgb, mtrx_rgb);
		_mtrx_mul_mtrx(mtrx_rgbto709l, mtrx_rgb, mtrx_out);
	}

	k = 0;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			coef[k] = mtrx_out[i][j];
			k++;
//			pr_vpp(PR_DEBUG, "[%s] mtrx_out[%d][%d] = 0x%x\n",
//				__func__, i, j, mtrx_out[i][j]);
		}
	}
}

static void _str_sapr_to_d(char *s, int *d, int n)
{
	int i, j, count;
	long value;
	char des[9] = {0};

	count = (strlen(s) + n - 2) / (n - 1);
	for (i = 0; i < count; i++) {
		for (j = 0; j < n - 1; j++)
			des[j] = s[j + i * (n - 1)];
		des[n - 1] = '\0';

		if (kstrtol(des, 10, &value) < 0)
			return;

		d[i] = value;
	}
}

#ifdef CONFIG_AMLOGIC_LCD
static int _lcd_gamma_notifier(struct notifier_block *nb,
	unsigned long event, void *data)
{
	if ((event & LCD_EVENT_GAMMA_UPDATE) == 0)
		return NOTIFY_DONE;

	vpp_module_gamma_set_viu_sel(*(int *)data);
	vpp_module_lcd_gamma_notify();

	return NOTIFY_OK;
}

static struct notifier_block nb_lcd_gamma = {
	.notifier_call = _lcd_gamma_notifier,
};
#endif

/*External functions*/
int vpp_pq_mgr_init(struct vpp_dev_s *dev)
{
	int ret = 0;

	chip_type = dev->m_data->chip_id;

	memset(&pq_mgr_settings, 0, sizeof(struct vpp_pq_mgr_settings));

	_buffer_init();
	_modules_init(dev);
	_set_default_settings();

	lut3d_db_initial = false;

#ifdef CONFIG_AMLOGIC_LCD
	ret = aml_lcd_notifier_register(&nb_lcd_gamma);
	if (ret)
		PR_DRV("register aml_lcd_gamma_notifier failed\n");
#endif

	pq_mgr_settings.pq_mgr_init_done = true;

	return ret;
}

void vpp_pq_mgr_deinit(void)
{
#ifdef CONFIG_AMLOGIC_LCD
	aml_lcd_notifier_unregister(&nb_lcd_gamma);
#endif

	_buffer_free();

	pq_mgr_settings.pq_mgr_init_done = false;
}

void vpp_pq_mgr_set_default_settings(void)
{
	_set_default_settings();
}

int vpp_pq_mgr_set_status(struct vpp_pq_state_s *status)
{
	int ret = 0;

	if (pq_mgr_settings.bypass_top_set)
		return ret;

	if (!status)
		return RET_POINT_FAIL;

	memcpy(&pq_mgr_settings.pq_status, status, sizeof(struct vpp_pq_state_s));

//	pr_vpp(PR_DEBUG, "[%s] pq_en = %d\n", __func__, status->pq_en);

	if (!status->pq_en) {
		_set_pq_bypass_all(true);
	} else {
		vpp_module_vadj_en(status->pq_cfg.vadj1_en, EN_MODE_RDMA);
		vpp_module_vadj_post_en(status->pq_cfg.vadj2_en, EN_MODE_RDMA);
		vpp_module_vadj_set_param(EN_VADJ_VD1_RGBBST_EN,
			status->pq_cfg.vd1_ctrst_en, EN_MODE_RDMA);
		vpp_module_vadj_set_param(EN_VADJ_POST_RGBBST_EN,
			status->pq_cfg.post_ctrst_en, EN_MODE_RDMA);
		vpp_module_pre_gamma_en(status->pq_cfg.pregamma_en, EN_MODE_RDMA);
		vpp_module_lcd_gamma_en(status->pq_cfg.gamma_en, EN_MODE_RDMA);
		vpp_module_go_en(status->pq_cfg.wb_en, EN_MODE_RDMA);
		vpp_module_dnlp_en(status->pq_cfg.dnlp_en, EN_MODE_RDMA);
		vpp_module_lc_en(status->pq_cfg.lc_en);
		vpp_module_ve_blkext_en(status->pq_cfg.black_ext_en, EN_MODE_RDMA);
		vpp_module_ve_ccoring_en(status->pq_cfg.chroma_cor_en, EN_MODE_RDMA);
		vpp_module_sr_en(EN_MODE_SR_0,
			status->pq_cfg.sharpness0_en, EN_MODE_RDMA);
		vpp_module_sr_en(EN_MODE_SR_1,
			status->pq_cfg.sharpness1_en, EN_MODE_RDMA);
		vpp_module_cm_en(status->pq_cfg.cm_en, EN_MODE_RDMA);
		vpp_module_lut3d_en(status->pq_cfg.lut3d_en, EN_MODE_RDMA);
	}

	return ret;
}

int vpp_pq_mgr_set_brightness(int val)
{
	int ret = 0;

	if (pq_mgr_settings.bypass_top_set)
		return ret;

	pq_mgr_settings.brightness = val;
	val = vpp_check_range(val, (-1024), 1023);

//	pr_vpp(PR_DEBUG, "[%s] brightness = %d\n", __func__, val);

	ret = vpp_module_vadj_set_brightness(val, EN_MODE_RDMA);

	return ret;
}

int vpp_pq_mgr_set_brightness_post(int val)
{
	int ret = 0;

	if (pq_mgr_settings.bypass_top_set)
		return ret;

	pq_mgr_settings.brightness_post = val;
	val = vpp_check_range(val, (-1024), 1023);

//	pr_vpp(PR_DEBUG, "[%s] brightness_post = %d\n", __func__, val);

	ret = vpp_module_vadj_set_brightness_post(val, EN_MODE_RDMA);

	return ret;
}

int vpp_pq_mgr_set_contrast(int val)
{
	int ret = 0;

	if (pq_mgr_settings.bypass_top_set)
		return ret;

	pq_mgr_settings.contrast = val;
	val = vpp_check_range(val, 0, 255);

//	pr_vpp(PR_DEBUG, "[%s] contrast = %d\n", __func__, val);

	ret = vpp_module_vadj_set_contrast(val, EN_MODE_RDMA);

	return ret;
}

int vpp_pq_mgr_set_contrast_post(int val)
{
	int ret = 0;

	if (pq_mgr_settings.bypass_top_set)
		return ret;

	pq_mgr_settings.contrast_post = val;
	val = vpp_check_range(val, 0, 255);

//	pr_vpp(PR_DEBUG, "[%s] contrast_post = %d\n", __func__, val);

	ret = vpp_module_vadj_set_contrast_post(val, EN_MODE_RDMA);

	return ret;
}

int vpp_pq_mgr_set_saturation(int sat_val)
{
	int ret = 0;
	int val = 0;
	int hue_val = pq_mgr_settings.hue;

	if (pq_mgr_settings.bypass_top_set)
		return ret;

	pq_mgr_settings.saturation = sat_val;

	val = _get_mab_from_sat_hue(sat_val, hue_val);

//	pr_vpp(PR_DEBUG, "[%s] sat_val/hue_val/mab = %d/%d/%d\n",
//		__func__, sat_val, hue_val, val);

	ret = vpp_module_vadj_set_sat_hue(val, EN_MODE_RDMA);

	return ret;
}

int vpp_pq_mgr_set_saturation_post(int sat_val)
{
	int ret = 0;
	int val = 0;
	int hue_val = pq_mgr_settings.hue_post;

	if (pq_mgr_settings.bypass_top_set)
		return ret;

	pq_mgr_settings.saturation_post = sat_val;

	val = _get_mab_from_sat_hue(sat_val, hue_val);

//	pr_vpp(PR_DEBUG, "[%s] sat_val/hue_val/mab_post = %d/%d/%d\n",
//		__func__, sat_val, hue_val, val);

	ret = vpp_module_vadj_set_sat_hue_post(val, EN_MODE_RDMA);

	return ret;
}

int vpp_pq_mgr_set_hue(int hue_val)
{
	int ret = 0;
	int val = 0;
	int sat_val = pq_mgr_settings.saturation;

	if (pq_mgr_settings.bypass_top_set)
		return ret;

	pq_mgr_settings.hue = hue_val;

	val = _get_mab_from_sat_hue(sat_val, hue_val);

//	pr_vpp(PR_DEBUG, "[%s] sat_val/hue_val/mab = %d/%d/%d\n",
//		__func__, sat_val, hue_val, val);

	ret = vpp_module_vadj_set_sat_hue(val, EN_MODE_RDMA);

	return ret;
}

int vpp_pq_mgr_set_hue_post(int hue_val)
{
	int ret = 0;
	int val = 0;
	int sat_val = pq_mgr_settings.saturation_post;

	if (pq_mgr_settings.bypass_top_set)
		return ret;

	pq_mgr_settings.hue_post = hue_val;

	val = _get_mab_from_sat_hue(sat_val, hue_val);

//	pr_vpp(PR_DEBUG, "[%s] sat_val/hue_val/mab_post = %d/%d/%d\n",
//		__func__, sat_val, hue_val, val);

	ret = vpp_module_vadj_set_sat_hue_post(val, EN_MODE_RDMA);

	return ret;
}

int vpp_pq_mgr_set_sharpness(int val)
{/*need to modify as new gain struct or add mapping logic*/
	struct sr_final_gain_param_s data;

	if (pq_mgr_settings.bypass_top_set)
		return 0;

	pq_mgr_settings.sharpness = val;
	val = vpp_check_range(val, 0, 255);

//	pr_vpp(PR_DEBUG, "[%s] sharpness = %d\n", __func__, val);

	data.final_gains[0] = val;
	data.final_gains[1] = val;
	data.final_gains[2] = val;
	data.final_gains[3] = val;

	vpp_module_sr_set_final_gain(&data, EN_MODE_RDMA);

	return 0;
}

int vpp_pq_mgr_set_whitebalance(struct vpp_white_balance_s *data)
{
	int ret = 0;
	struct vpp_module_go_s go_data;

	if (pq_mgr_settings.bypass_top_set)
		return ret;

	if (!data)
		return RET_POINT_FAIL;

//	pr_vpp(PR_DEBUG, "[%s] r_pre_offset = %d\n",
//		__func__, data->r_pre_offset);
//	pr_vpp(PR_DEBUG, "[%s] g_pre_offset = %d\n",
//		__func__, data->g_pre_offset);
//	pr_vpp(PR_DEBUG, "[%s] b_pre_offset = %d\n",
//		__func__, data->b_pre_offset);
//	pr_vpp(PR_DEBUG, "[%s] r_gain = %d\n",
//		__func__, data->r_gain);
//	pr_vpp(PR_DEBUG, "[%s] g_gain = %d\n",
//		__func__, data->g_gain);
//	pr_vpp(PR_DEBUG, "[%s] b_gain = %d\n",
//		__func__, data->b_gain);
//	pr_vpp(PR_DEBUG, "[%s] r_post_offset = %d\n",
//		__func__, data->r_post_offset);
//	pr_vpp(PR_DEBUG, "[%s] g_post_offset = %d\n",
//		__func__, data->g_post_offset);
//	pr_vpp(PR_DEBUG, "[%s] b_post_offset = %d\n",
//		__func__, data->b_post_offset);

	memcpy(&pq_mgr_settings.video_rgb_ogo, data,
		sizeof(struct vpp_white_balance_s));

	go_data.pre_offset[0] =
		vpp_check_range(data->r_pre_offset, (-1024), 1023);
	go_data.pre_offset[1] =
		vpp_check_range(data->g_pre_offset, (-1024), 1023);
	go_data.pre_offset[2] =
		vpp_check_range(data->b_pre_offset, (-1024), 1023);

	go_data.gain[0] =
		vpp_check_range(data->r_gain, 0, 2047);
	go_data.gain[1] =
		vpp_check_range(data->g_gain, 0, 2047);
	go_data.gain[2] =
		vpp_check_range(data->b_gain, 0, 2047);

	go_data.post_offset[0] =
		vpp_check_range(data->r_post_offset, (-1024), 1023);
	go_data.post_offset[1] =
		vpp_check_range(data->g_post_offset, (-1024), 1023);
	go_data.post_offset[2] =
		vpp_check_range(data->b_post_offset, (-1024), 1023);

	vpp_module_go_set_data(&go_data, EN_MODE_RDMA);

	return ret;
}

int vpp_pq_mgr_set_pre_gamma_table(struct vpp_gamma_table_s *data)
{
	int ret = 0;
	unsigned int len = 0;
	unsigned int buf_size = 0;

	if (pq_mgr_settings.bypass_top_set)
		return ret;

	if (!data || !data->r_data || !data->g_data || !data->b_data)
		return RET_POINT_FAIL;

	len = vpp_module_pre_gamma_get_table_len();
	if (len == 0)
		return RET_POINT_FAIL;

	buf_size = len * sizeof(unsigned int);

//	pr_vpp(PR_DEBUG, "[%s] set data\n", __func__);

	memcpy(&pq_mgr_settings.cur_pre_gamma_tbl.r_data,
		data->r_data, buf_size);
	memcpy(&pq_mgr_settings.cur_pre_gamma_tbl.g_data,
		data->g_data, buf_size);
	memcpy(&pq_mgr_settings.cur_pre_gamma_tbl.b_data,
		data->b_data, buf_size);

	ret = vpp_module_pre_gamma_write(data->r_data,
		data->g_data, data->b_data);

	return ret;
}

int vpp_pq_mgr_set_gamma_table(struct vpp_gamma_table_s *data)
{
	int ret = 0;
	unsigned int len = 0;
	unsigned int buf_size = 0;

	if (pq_mgr_settings.bypass_top_set)
		return ret;

	if (!data || !data->r_data || !data->g_data || !data->b_data)
		return RET_POINT_FAIL;

	len = vpp_module_lcd_gamma_get_table_len();
	if (len == 0)
		return RET_POINT_FAIL;

	buf_size = len * sizeof(unsigned int);

//	pr_vpp(PR_DEBUG, "[%s] set data\n", __func__);

	memcpy(&pq_mgr_settings.cur_gamma_tbl.r_data,
		data->r_data, buf_size);
	memcpy(&pq_mgr_settings.cur_gamma_tbl.g_data,
		data->g_data, buf_size);
	memcpy(&pq_mgr_settings.cur_gamma_tbl.b_data,
		data->b_data, buf_size);

	ret = vpp_module_lcd_gamma_write(data->r_data,
		data->g_data, data->b_data);

	return ret;
}

int vpp_pq_mgr_set_cm_curve(enum vpp_cm_curve_type_e type, int *data)
{
	if (pq_mgr_settings.bypass_top_set)
		return 0;

	if (!data)
		return RET_POINT_FAIL;

	pr_vpp(PR_DEBUG, "[%s] set data, type = %d\n", __func__, type);

	switch (type) {
	case EN_CM_LUMA:
		vpp_module_cm_set_cm2_luma(data);
		break;
	case EN_CM_SAT:
		vpp_module_cm_set_cm2_sat(data);
		break;
	case EN_CM_HUE:
		vpp_module_cm_set_cm2_hue(data);
		break;
	case EN_CM_HUE_BY_HS:
		vpp_module_cm_set_cm2_hue_by_hs(data);
		break;
	case EN_CM_SAT_BY_L:
		vpp_module_cm_set_cm2_sat_by_l(data);
		break;
	case EN_CM_SAT_BY_HL:
		vpp_module_cm_set_cm2_sat_by_hl(data);
		break;
	case EN_CM_HUE_BY_HL:
		vpp_module_cm_set_cm2_hue_by_hl(data);
		break;
	}

	return 0;
}

int vpp_pq_mgr_set_cm_offset_curve(enum vpp_cm_curve_type_e type, char *data)
{
	if (pq_mgr_settings.bypass_top_set)
		return 0;

	if (!data)
		return RET_POINT_FAIL;

	pr_vpp(PR_DEBUG, "[%s] set data, type = %d\n", __func__, type);

	switch (type) {
	case EN_CM_LUMA:
		vpp_module_cm_set_cm2_offset_luma(data);
		break;
	case EN_CM_SAT:
		vpp_module_cm_set_cm2_offset_sat(data);
		break;
	case EN_CM_HUE:
		vpp_module_cm_set_cm2_offset_hue(data);
		break;
	case EN_CM_HUE_BY_HS:
		vpp_module_cm_set_cm2_offset_hue_by_hs(data);
		break;
	default:
		break;
	}

	return 0;
}

int vpp_pq_mgr_set_matrix_param(struct vpp_mtrx_info_s *data)
{
	int ret = 0;
	enum matrix_mode_e mode;
	enum vpp_mtrx_type_e mtrx_sel;

	if (pq_mgr_settings.bypass_top_set)
		return ret;

	if (!data)
		return RET_POINT_FAIL;

	pr_vpp(PR_DEBUG, "[%s] mtrx_sel = %d\n",
		__func__, data->mtrx_sel);

	if (data->mtrx_sel < EN_MTRX_MAX)
		mtrx_sel = data->mtrx_sel;
	else
		mtrx_sel = EN_MTRX_VD1;

	switch (mtrx_sel) {
	case EN_MTRX_VD1:
	default:
		mode = EN_MTRX_MODE_VD1;
		break;
	case EN_MTRX_POST:
		mode = EN_MTRX_MODE_POST;
		break;
	case EN_MTRX_POST2:
		mode = EN_MTRX_MODE_POST2;
		break;
	}

	memcpy(&pq_mgr_settings.matrix_param[mtrx_sel],
		&data->mtrx_param, sizeof(struct vpp_mtrx_param_s));

	ret = vpp_module_matrix_rs(mode, data->mtrx_param.right_shift);
	ret |= vpp_module_matrix_set_pre_offset(mode, &data->mtrx_param.pre_offset[0]);
	ret |= vpp_module_matrix_set_offset(mode, &data->mtrx_param.post_offset[0]);
	ret |= vpp_module_matrix_set_coef_3x3(mode, &data->mtrx_param.matrix_coef[0]);

	return ret;
}

int vpp_pq_mgr_set_dnlp_param(struct vpp_dnlp_curve_param_s *data)
{
	if (pq_mgr_settings.bypass_top_set)
		return 0;

	if (!data)
		return RET_POINT_FAIL;

//	pr_vpp(PR_DEBUG, "[%s] set data\n", __func__);

	memcpy(&pq_mgr_settings.dnlp_param, data,
		sizeof(struct vpp_dnlp_curve_param_s));

	vpp_module_dnlp_set_param(data);

	return 0;
}

int vpp_pq_mgr_set_ble_whe_param(struct vpp_ble_whe_param_s *data)
{
	if (pq_mgr_settings.bypass_top_set)
		return 0;

	if (!data)
		return RET_POINT_FAIL;

//	pr_vpp(PR_DEBUG, "[%s] set data\n", __func__);

	vpp_module_dnlp_set_ble_whe(data);

	return 0;
}

int vpp_pq_mgr_set_lc_curve(struct vpp_lc_curve_s *data)
{
	if (pq_mgr_settings.bypass_top_set)
		return 0;

	if (!data)
		return RET_POINT_FAIL;

	if (vpp_module_lc_get_support()) {
//		pr_vpp(PR_DEBUG, "[%s] set data\n", __func__);

		vpp_module_lc_write_lut(EN_LC_SAT, &data->lc_saturation[0]);
		vpp_module_lc_write_lut(EN_LC_YMIN_LMT, &data->lc_yminval_lmt[0]);
		vpp_module_lc_write_lut(EN_LC_YPKBV_YMAX_LMT, &data->lc_ypkbv_ymaxval_lmt[0]);
		vpp_module_lc_write_lut(EN_LC_YMAX_LMT, &data->lc_ymaxval_lmt[0]);
		vpp_module_lc_write_lut(EN_LC_YPKBV_LMT, &data->lc_ypkbv_lmt[0]);
		vpp_module_lc_write_lut(EN_LC_YPKBV_RAT, &data->lc_ypkbv_ratio[0]);
	}

	return 0;
}

int vpp_pq_mgr_set_lc_param(struct vpp_lc_param_s *data)
{
	if (pq_mgr_settings.bypass_top_set)
		return 0;

	if (!data)
		return RET_POINT_FAIL;

	if (vpp_module_lc_get_support()) {
//		pr_vpp(PR_DEBUG, "[%s] set data\n", __func__);
		vpp_data_update_reg_lc(data);
	}

	return 0;
}

int vpp_pq_mgr_set_module_status(enum vpp_module_e module, bool enable)
{
	if (pq_mgr_settings.bypass_top_set)
		return 0;

//	pr_vpp(PR_DEBUG, "[%s] module = %d, enable = %d\n",
//		__func__, module, enable);

	switch (module) {
	case EN_MODULE_VADJ1:
		vpp_module_vadj_en(enable, EN_MODE_RDMA);
		pq_mgr_settings.pq_status.pq_cfg.vadj1_en = enable;
		break;
	case EN_MODULE_VADJ2:
		vpp_module_vadj_post_en(enable, EN_MODE_RDMA);
		pq_mgr_settings.pq_status.pq_cfg.vadj2_en = enable;
		break;
	case EN_MODULE_PREGAMMA:
		vpp_module_pre_gamma_en(enable, EN_MODE_RDMA);
		pq_mgr_settings.pq_status.pq_cfg.pregamma_en = enable;
		break;
	case EN_MODULE_GAMMA:
		vpp_module_lcd_gamma_en(enable, EN_MODE_RDMA);
		pq_mgr_settings.pq_status.pq_cfg.gamma_en = enable;
		break;
	case EN_MODULE_WB:
		vpp_module_go_en(enable, EN_MODE_RDMA);
		pq_mgr_settings.pq_status.pq_cfg.wb_en = enable;
		break;
	case EN_MODULE_DNLP:
		vpp_module_dnlp_en(enable, EN_MODE_RDMA);
		pq_mgr_settings.pq_status.pq_cfg.dnlp_en = enable;
		break;
	case EN_MODULE_CCORING:
		vpp_module_ve_ccoring_en(enable, EN_MODE_RDMA);
		pq_mgr_settings.pq_status.pq_cfg.chroma_cor_en = enable;
		break;
	case EN_MODULE_SR0:
		vpp_module_sr_en(EN_MODE_SR_0, enable, EN_MODE_RDMA);
		pq_mgr_settings.pq_status.pq_cfg.sharpness0_en = enable;
		break;
	case EN_MODULE_SR1:
		vpp_module_sr_en(EN_MODE_SR_1, enable, EN_MODE_RDMA);
		pq_mgr_settings.pq_status.pq_cfg.sharpness1_en = enable;
		break;
	case EN_MODULE_LC:
		vpp_module_lc_en(enable);
		pq_mgr_settings.pq_status.pq_cfg.lc_en = enable;
		break;
	case EN_MODULE_CM:
		vpp_module_cm_en(enable, EN_MODE_RDMA);
		pq_mgr_settings.pq_status.pq_cfg.cm_en = enable;
		break;
	case EN_MODULE_BLE:
		vpp_module_ve_blkext_en(enable, EN_MODE_RDMA);
		pq_mgr_settings.pq_status.pq_cfg.black_ext_en = enable;
		break;
	case EN_MODULE_BLS:
		vpp_module_ve_blue_stretch_en(enable, EN_MODE_RDMA);
		pq_mgr_settings.pq_status.pq_cfg.blue_stretch_en = enable;
		break;
	case EN_MODULE_LUT3D:
		vpp_module_lut3d_en(enable, EN_MODE_RDMA);
		pq_mgr_settings.pq_status.pq_cfg.lut3d_en = enable;
		break;
	case EN_MODULE_DEJAGGY_SR0:
		vpp_module_sr_sub_module_en(EN_MODE_SR_0,
			EN_SUB_MD_DEJAGGY, enable);
		pq_mgr_settings.pq_status.pq_cfg.dejaggy_sr0_en = enable;
		break;
	case EN_MODULE_DEJAGGY_SR1:
		vpp_module_sr_sub_module_en(EN_MODE_SR_1,
			EN_SUB_MD_DEJAGGY, enable);
		pq_mgr_settings.pq_status.pq_cfg.dejaggy_sr1_en = enable;
		break;
	case EN_MODULE_DERING_SR0:
		vpp_module_sr_sub_module_en(EN_MODE_SR_0,
			EN_SUB_MD_DERING, enable);
		pq_mgr_settings.pq_status.pq_cfg.dering_sr0_en = enable;
		break;
	case EN_MODULE_DERING_SR1:
		vpp_module_sr_sub_module_en(EN_MODE_SR_1,
			EN_SUB_MD_DERING, enable);
		pq_mgr_settings.pq_status.pq_cfg.dering_sr1_en = enable;
		break;
	case EN_MODULE_ALL:
		_set_pq_bypass_all(!enable);
		break;
	default:
		break;
	}

	return 0;
}

int vpp_pq_mgr_set_pc_mode(int val)
{
	if (pq_mgr_settings.bypass_top_set)
		return 0;

//	pr_vpp(PR_DEBUG, "[%s] pc_mode = %d\n", __func__, val);

	pq_mgr_settings.pc_mode = val;

	vpp_vf_set_pc_mode(val);

	return 0;
}

int vpp_pq_mgr_set_color_primary_status(int val)
{
	return 0;
}

int vpp_pq_mgr_set_color_primary(struct vpp_color_primary_s *data)
{
	if (!data)
		return 0;

	return 0;
}

int vpp_pq_mgr_set_csc_type(int val)
{
	return 0;
}

int vpp_pq_mgr_load_3dlut_data(struct vpp_lut3d_path_s *data)
{
	return 0;
}

int vpp_pq_mgr_set_3dlut_data(struct vpp_lut3d_table_s *table)
{
	if (pq_mgr_settings.bypass_top_set)
		return 0;

	if (!table)
		return RET_POINT_FAIL;

	pr_vpp(PR_DEBUG, "[%s] data_type/size = %d/%d\n", __func__,
		table->data_type, table->data_size);

	switch (table->data_type) {
	case EN_LUT3D_INPUT_PARAM:
		vpp_module_lut3d_set_data(table->data,
			table->data_size, EN_MODE_RDMA);
		break;
	case EN_LUT3D_UNIFY_KEY:
		break;
	case EN_LUT3D_BIN_FILE:
		break;
	default:
		break;
	}

	return 0;
}

int vpp_pq_mgr_set_hdr_cgain_curve(struct vpp_hdr_lut_s *data)
{
	enum hdr_module_type_e type = EN_MODULE_TYPE_VDIN0;
	enum hdr_sub_module_e sub_module = EN_SUB_MODULE_CGAIN;
	enum hdr_vpp_type_e vpp_sel = EN_TYPE_VPP0;

	if (pq_mgr_settings.bypass_top_set)
		return 0;

	if (!data)
		return RET_POINT_FAIL;

//	pr_vpp(PR_DEBUG, "[%s] set data\n", __func__);

	vpp_module_hdr_set_lut(type, sub_module,
		(int *)data->lut_data, vpp_sel);

	return 0;
}

int vpp_pq_mgr_set_hdr_oetf_curve(struct vpp_hdr_lut_s *data)
{
	enum hdr_module_type_e type = EN_MODULE_TYPE_VDIN0;
	enum hdr_sub_module_e sub_module = EN_SUB_MODULE_OETF;
	enum hdr_vpp_type_e vpp_sel = EN_TYPE_VPP0;

	if (pq_mgr_settings.bypass_top_set)
		return 0;

	if (!data)
		return RET_POINT_FAIL;

//	pr_vpp(PR_DEBUG, "[%s] set data\n", __func__);

	vpp_module_hdr_set_lut(type, sub_module,
		(int *)data->lut_data, vpp_sel);

	return 0;
}

int vpp_pq_mgr_set_hdr_eotf_curve(struct vpp_hdr_lut_s *data)
{
	enum hdr_module_type_e type = EN_MODULE_TYPE_VDIN0;
	enum hdr_sub_module_e sub_module = EN_SUB_MODULE_EOTF;
	enum hdr_vpp_type_e vpp_sel = EN_TYPE_VPP0;

	if (pq_mgr_settings.bypass_top_set)
		return 0;

	if (!data)
		return RET_POINT_FAIL;

//	pr_vpp(PR_DEBUG, "[%s] set data\n", __func__);

	vpp_module_hdr_set_lut(type, sub_module,
		(int *)data->lut_data, vpp_sel);

	return 0;
}

int vpp_pq_mgr_set_hdr_tmo_curve(struct vpp_hdr_lut_s *data)
{
	enum hdr_module_type_e type = EN_MODULE_TYPE_VDIN0;
	enum hdr_sub_module_e sub_module = EN_SUB_MODULE_OGAIN;
	enum hdr_vpp_type_e vpp_sel = EN_TYPE_VPP0;

	if (pq_mgr_settings.bypass_top_set)
		return 0;

	if (!data)
		return RET_POINT_FAIL;

//	pr_vpp(PR_DEBUG, "[%s] set data\n", __func__);

	vpp_module_hdr_set_lut(type, sub_module,
		(int *)data->lut_data, vpp_sel);

	return 0;
}

int vpp_pq_mgr_set_hdr_tmo_param(struct vpp_tmo_param_s *data)
{
	return 0;
}

int vpp_pq_mgr_set_cabc_param(struct vpp_cabc_param_s *data)
{
	return 0;
}

int vpp_pq_mgr_set_aad_param(struct vpp_aad_param_s *data)
{
	return 0;
}

int vpp_pq_mgr_set_eye_protect(struct vpp_eye_protect_s *data)
{
	int ret = 0;
	int coef[9] = {0};
	int pre_offset[3] = {-64, -512, -512};
	int offset[3] = {64, 512, 512};

	if (pq_mgr_settings.bypass_top_set)
		return ret;

	if (!data)/* || vpp_vf_get_vinfo_lcd_support())*/
		return ret;

	pr_vpp(PR_DEBUG, "[%s] enable/r_/g_/b_gain = %d/%d/%d/%d\n",
		__func__, data->enable,
		data->rgb[0], data->rgb[1], data->rgb[2]);

	if (data->enable) {
		_mtrx_multi(&data->rgb[0], &coef[0]);
		ret = vpp_module_matrix_set_pre_offset(EN_MTRX_MODE_POST, &pre_offset[0]);
		ret |= vpp_module_matrix_set_offset(EN_MTRX_MODE_POST, &offset[0]);
		ret |= vpp_module_matrix_set_coef_3x3(EN_MTRX_MODE_POST, &coef[0]);
	}

	ret |= vpp_module_matrix_en(EN_MTRX_MODE_POST, data->enable);

	return ret;
}

int vpp_pq_mgr_set_aipq_offset_table(char *data_str,
	unsigned int height, unsigned int width)
{
	int i = 0;
	int *table = NULL;
	unsigned int table_len = 0;
	unsigned int size = 0;

	if (pq_mgr_settings.bypass_top_set)
		return 0;

	if (!data_str)
		return RET_POINT_FAIL;

	table_len = height * width * sizeof(int);
	table = kmalloc(table_len, GFP_KERNEL);
	if (!table)
		return -EFAULT;

	_str_sapr_to_d(data_str, table, 4);

	size = width * sizeof(int);
	for (i = 0; i < height; i++)
		memcpy(vpp_pq_data[i], table + (i * width), size);

	kfree(table);
	return 0;
}

int vpp_pq_mgr_set_aipq_data(struct vpp_aipq_table_s *data)
{
	int i = 0;
	unsigned int *table = NULL;
	unsigned int size = 0;

	if (pq_mgr_settings.bypass_top_set)
		return 0;

	if (!data)
		return RET_POINT_FAIL;

	table = (unsigned int *)data->table_ptr;
	size = data->width * sizeof(int);
	for (i = 0; i < data->height; i++)
		memcpy(vpp_pq_data[i], table + (i * data->width), size);

	return 0;
}

int vpp_pq_mgr_set_overscan_table(unsigned int length,
	struct vpp_overscan_table_s *load_table)
{
	if (pq_mgr_settings.bypass_top_set)
		return 0;

	if (load_table)
		vpp_vf_set_overscan_table(length, load_table);

	return 0;
}

void vpp_pq_mgr_set_lc_isr(void)
{
	vpp_module_lc_set_isr();
}

void vpp_pq_mgr_set_blkext_params(struct vpp_blkext_param_s *data)
{
	if (!data)
		return;

	vpp_module_ve_set_blkext_params(&data->param[0], EN_MODE_RDMA);
}

void vpp_pq_mgr_set_chroma_coring_params(struct vpp_cc_param_s *data)
{
	if (!data)
		return;

	vpp_module_ve_set_ccoring_params(&data->param[0], EN_MODE_RDMA);
}

void vpp_pq_mgr_set_blue_stretch_params(struct vpp_bs_param_s *data)
{
	if (!data)
		return;

	vpp_module_ve_set_blue_stretch_params(&data->param[0], EN_MODE_RDMA);
}

void vpp_pq_mgr_set_ccoring_params(unsigned int *data, int length)
{
	if (length != EN_CCORING_MAX || !data) {
		pr_vpp(PR_DEBUG, "[%s] length = %d\n", __func__, length);
		return;
	}

	vpp_module_ve_set_ccoring_params(data, EN_MODE_RDMA);
}

void vpp_pq_mgr_set_sr_params(struct vpp_sr_param_s *data)
{
	if (!data)
		return;

	vpp_data_update_reg_sr(data);
}

void vpp_pq_mgr_get_status(struct vpp_pq_state_s *status)
{
	if (status)
		memcpy(status, &pq_mgr_settings.pq_status,
			sizeof(struct vpp_pq_state_s));
}

void vpp_pq_mgr_get_brightness(int *val)
{
	if (val)
		*val = pq_mgr_settings.brightness;
}

void vpp_pq_mgr_get_brightness_post(int *val)
{
	if (val)
		*val = pq_mgr_settings.brightness_post;
}

void vpp_pq_mgr_get_contrast(int *val)
{
	if (val)
		*val = pq_mgr_settings.contrast;
}

void vpp_pq_mgr_get_contrast_post(int *val)
{
	if (val)
		*val = pq_mgr_settings.contrast_post;
}

void vpp_pq_mgr_get_saturation(int *val)
{
	if (val)
		*val = pq_mgr_settings.saturation;
}

void vpp_pq_mgr_get_saturation_post(int *val)
{
	if (val)
		*val = pq_mgr_settings.saturation_post;
}

void vpp_pq_mgr_get_hue(int *val)
{
	if (val)
		*val = pq_mgr_settings.hue;
}

void vpp_pq_mgr_get_hue_post(int *val)
{
	if (val)
		*val = pq_mgr_settings.hue_post;
}

void vpp_pq_mgr_get_sharpness(int *val)
{/*need to modify as new gain struct*/
	if (val)
		*val = pq_mgr_settings.sharpness;
}

void vpp_pq_mgr_get_whitebalance(struct vpp_white_balance_s *data)
{
	if (data)
		memcpy(data, &pq_mgr_settings.video_rgb_ogo,
			sizeof(struct vpp_white_balance_s));
}

struct vpp_gamma_table_s *vpp_pq_mgr_get_pre_gamma_table(void)
{
	return &pq_mgr_settings.cur_pre_gamma_tbl;
}

struct vpp_gamma_table_s *vpp_pq_mgr_get_gamma_table(void)
{
	return &pq_mgr_settings.cur_gamma_tbl;
}

void vpp_pq_mgr_get_matrix_param(struct vpp_mtrx_info_s *data)
{
	enum vpp_mtrx_type_e mtrx_sel;

	if (data->mtrx_sel < EN_MTRX_MAX)
		mtrx_sel = data->mtrx_sel;
	else
		mtrx_sel = EN_MTRX_VD1;

	memcpy(&data->mtrx_param, &pq_mgr_settings.matrix_param[mtrx_sel],
		sizeof(struct vpp_mtrx_param_s));
}

void vpp_pq_mgr_get_dnlp_param(struct vpp_dnlp_curve_param_s *data)
{
	if (data)
		memcpy(data, &pq_mgr_settings.dnlp_param,
			sizeof(struct vpp_dnlp_curve_param_s));
}

void vpp_pq_mgr_get_module_status(enum vpp_module_e module,
	unsigned char *enable_ret)
{
	unsigned char enable;

	if (!enable_ret)
		return;

	switch (module) {
	case EN_MODULE_VADJ1:
		enable = pq_mgr_settings.pq_status.pq_cfg.vadj1_en;
		break;
	case EN_MODULE_VADJ2:
		enable = pq_mgr_settings.pq_status.pq_cfg.vadj2_en;
		break;
	case EN_MODULE_PREGAMMA:
		enable = pq_mgr_settings.pq_status.pq_cfg.pregamma_en;
		break;
	case EN_MODULE_GAMMA:
		enable = pq_mgr_settings.pq_status.pq_cfg.gamma_en;
		break;
	case EN_MODULE_WB:
		enable = pq_mgr_settings.pq_status.pq_cfg.wb_en;
		break;
	case EN_MODULE_DNLP:
		enable = pq_mgr_settings.pq_status.pq_cfg.dnlp_en;
		break;
	case EN_MODULE_CCORING:
		enable = pq_mgr_settings.pq_status.pq_cfg.chroma_cor_en;
		break;
	case EN_MODULE_SR0:
		enable = pq_mgr_settings.pq_status.pq_cfg.sharpness0_en;
		break;
	case EN_MODULE_SR1:
		enable = pq_mgr_settings.pq_status.pq_cfg.sharpness1_en;
		break;
	case EN_MODULE_LC:
		enable = pq_mgr_settings.pq_status.pq_cfg.lc_en;
		break;
	case EN_MODULE_CM:
		enable = pq_mgr_settings.pq_status.pq_cfg.cm_en;
		break;
	case EN_MODULE_BLE:
		enable = pq_mgr_settings.pq_status.pq_cfg.black_ext_en;
		break;
	case EN_MODULE_BLS:
		enable = pq_mgr_settings.pq_status.pq_cfg.blue_stretch_en;
		break;
	case EN_MODULE_LUT3D:
		enable = pq_mgr_settings.pq_status.pq_cfg.lut3d_en;
		break;
	case EN_MODULE_DEJAGGY_SR0:
		enable = pq_mgr_settings.pq_status.pq_cfg.dejaggy_sr0_en;
		break;
	case EN_MODULE_DEJAGGY_SR1:
		enable = pq_mgr_settings.pq_status.pq_cfg.dejaggy_sr1_en;
		break;
	case EN_MODULE_DERING_SR0:
		enable = pq_mgr_settings.pq_status.pq_cfg.dering_sr0_en;
		break;
	case EN_MODULE_DERING_SR1:
		enable = pq_mgr_settings.pq_status.pq_cfg.dering_sr1_en;
		break;
	case EN_MODULE_ALL:
	default:
		enable = pq_mgr_settings.pq_status.pq_en;
		break;
	}

	*enable_ret = enable;
}

void vpp_pq_mgr_get_hdr_tmo_param(struct vpp_tmo_param_s *data)
{
}

void vpp_pq_mgr_get_hdr_metadata(struct vpp_hdr_metadata_s *data)
{
	struct vpp_hdr_metadata_s *metadata;

	if (!data)
		return;

	metadata = vpp_vf_get_hdr_metadata();
	memcpy(data, metadata, sizeof(struct vpp_hdr_metadata_s));
}

void vpp_pq_mgr_get_hdr_histogram(struct vpp_hdr_hist_param_s *data)
{
	struct hdr_hist_report_s *report = NULL;
	int i = 0;

	if (!data)
		return;

	report = vpp_module_hdr_get_hist_report();

	for (i = 0; i < VPP_HDR_HIST_BIN_COUNT; i++)
		data->data_rgb_max[i] =
			report->hist_buff[HDR_HIST_BACKUP_COUNT - 1][i];
}

void vpp_pq_mgr_get_histogram_ave(struct vpp_hist_ave_s *data)
{
	int ave_luma;
	struct vpp_hist_report_s *hist_report;

	if (!data)
		return;

	hist_report = vpp_module_meter_get_hist_report();

	data->sum = hist_report->luma_sum;
	data->width = hist_report->width;
	data->height = hist_report->height;
	ave_luma = hist_report->luma_sum /
		(hist_report->height * hist_report->width);

	ave_luma = (ave_luma - 16) < 0 ? 0 : (ave_luma - 16);
	ave_luma = ave_luma * 255 / (235 - 16);
	if (ave_luma > 255)
		ave_luma = 255;

	data->ave = ave_luma;
}

void vpp_pq_mgr_get_histogram(struct vpp_hist_bin_s *data)
{
	int i = 0;
	struct vpp_hist_report_s *hist_report;

	if (!data)
		return;

	hist_report = vpp_module_meter_get_hist_report();

	data->hist_pow = hist_report->hist_pow;
	data->luma_sum = hist_report->luma_sum;
	data->pixel_sum = hist_report->pixel_sum;

	for (i = 0; i < VPP_HIST_BIN_COUNT; i++)
		data->hist[i] = hist_report->gamma[i];

	for (i = 0; i < VPP_COLOR_HIST_BIN_COUNT; i++) {
		data->hue_hist[i] = hist_report->hue_gamma[i];
		data->sat_hist[i] = hist_report->sat_gamma[i];
	}
}

int vpp_pq_mgr_get_pc_mode(void)
{
	return pq_mgr_settings.pc_mode;
}

enum vpp_csc_type_e vpp_pq_mgr_get_csc_type(void)
{
	return vpp_vf_get_csc_type(EN_VD1_PATH);
}

enum vpp_hdr_type_e vpp_pq_mgr_get_hdr_type(void)
{
	return vpp_vf_get_hdr_type();
}

enum vpp_color_primary_e vpp_pq_mgr_get_color_primary(void)
{
	return vpp_vf_get_color_primary();
}

int vpp_pq_mgr_get_pre_gamma_table_len(void)
{
	return vpp_module_pre_gamma_get_table_len();
}

int vpp_pq_mgr_get_gamma_table_len(void)
{
	return vpp_module_lcd_gamma_get_table_len();
}

int vpp_pq_mgr_get_sr_params_len(enum vpp_sr_type_e type)
{
	int ret = 0;

	ret = vpp_data_get_sr_params_len(type);

	return ret;
}

struct vpp_pq_mgr_settings *vpp_pq_mgr_get_settings(void)
{
	return &pq_mgr_settings;
}

#endif

