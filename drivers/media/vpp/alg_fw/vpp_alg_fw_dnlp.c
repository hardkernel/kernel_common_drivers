// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#include "../vpp_common.h"
#include <linux/amlogic/media/amvecm/dnlp_alg.h>
#include "vpp_alg_fw_dnlp.h"

#define DNLP_BIN_SIZE (64)

/*Algorithm related*/
static int pre_sat;
static bool dnlp_alg_insmod_ok;
static struct aml_dnlp_drv_param_s *dnlp_alg_function;
static struct dnlp_alg_input_param_s *dnlp_alg_in;
static struct dnlp_alg_output_param_s *dnlp_alg_out;
static struct dnlp_dbg_ro_param_s *dnlp_dbg_ro;
static struct dnlp_dbg_rw_param_s *dnlp_dbg_rw;
static struct param_for_dnlp_s *dnlp_alg_node_param;
static struct ble_whe_param_s *ble_whe_data;
static struct dnlp_dbg_print_s *dnlp_dbg_info;

/*For debug*/
struct dnlp_alg_dbg_parse_cmd_s dnlp_dbg_parse_cmd[] = {
	{"alg_enable", NULL, 1},
	{"respond", NULL, 1},
	{"sel", NULL, 1},
	{"respond_flag", NULL, 1},
	{"smhist_ck", NULL, 1},
	{"mvreflsh", NULL, 1},/*5*/
	{"pavg_btsft", NULL, 1},
	{"dbg_i2r", NULL, 1},
	{"cuvbld_min", NULL, 1},
	{"cuvbld_max", NULL, 1},
	{"schg_sft", NULL, 1},/*10*/
	{"bbd_ratio_low", NULL, 1},
	{"bbd_ratio_hig", NULL, 1},
	{"limit_rng", NULL, 1},
	{"range_det", NULL, 1},
	{"blk_cctr", NULL, 1},/*15*/
	{"brgt_ctrl", NULL, 1},
	{"brgt_range", NULL, 1},
	{"brght_add", NULL, 1},
	{"brght_max", NULL, 1},
	{"dbg_adjavg", NULL, 1},/*20*/
	{"auto_rng", NULL, 1},
	{"lowrange", NULL, 1},
	{"hghrange", NULL, 1},
	{"satur_rat", NULL, 1},
	{"satur_max", NULL, 1},/*25*/
	{"set_saturtn", NULL, 1},
	{"sbgnbnd", NULL, 1},
	{"sendbnd", NULL, 1},
	{"clashbgn", NULL, 1},
	{"clashend", NULL, 1},/*30*/
	{"var_th", NULL, 1},
	{"clahe_gain_neg", NULL, 1},
	{"clahe_gain_pos", NULL, 1},
	{"clahe_gain_delta", NULL, 1},
	{"mtdbld_rate", NULL, 1},/*35*/
	{"adpmtd_lbnd", NULL, 1},
	{"adpmtd_hbnd", NULL, 1},
	{"blkext_ofst", NULL, 1},
	{"whtext_ofst", NULL, 1},
	{"blkext_rate", NULL, 1},/*40*/
	{"whtext_rate", NULL, 1},
	{"bwext_div4x_min", NULL, 1},
	{"irgnbgn", NULL, 1},
	{"irgnend", NULL, 1},
	{"dbg_map", NULL, 1},/*45*/
	{"final_gain", NULL, 1},
	{"cliprate_v3", NULL, 1},
	{"cliprate_min", NULL, 1},
	{"adpcrat_lbnd", NULL, 1},
	{"adpcrat_hbnd", NULL, 1},/*50*/
	{"scurv_low_th", NULL, 1},
	{"scurv_mid1_th", NULL, 1},
	{"scurv_mid2_th", NULL, 1},
	{"scurv_hgh1_th", NULL, 1},
	{"scurv_hgh2_th", NULL, 1},/*55*/
	{"mtdrate_adp_en", NULL, 1},
	{"clahe_method", NULL, 1},
	{"ble_en", NULL, 1},
	{"norm", NULL, 1},
	{"scn_chg_th", NULL, 1},/*60*/
	{"step_th", NULL, 1},
	{"iir_step_mux", NULL, 1},
	{"single_bin_bw", NULL, 1},
	{"single_bin_method", NULL, 1},
	{"reg_max_slop_1st", NULL, 1},/*65*/
	{"reg_max_slop_mid", NULL, 1},
	{"reg_max_slop_fin", NULL, 1},
	{"reg_min_slop_1st", NULL, 1},
	{"reg_min_slop_mid", NULL, 1},
	{"reg_min_slop_fin", NULL, 1},/*70*/
	{"reg_trend_wht_expand_mode", NULL, 1},
	{"reg_trend_blk_expand_mode", NULL, 1},
	{"ve_hist_cur_gain", NULL, 1},
	{"ve_hist_cur_gain_precise", NULL, 1},
	{"reg_mono_binrang_st", NULL, 1},/*75*/
	{"reg_mono_binrang_ed", NULL, 1},
	{"c_hist_gain_base", NULL, 1},
	{"s_hist_gain_base", NULL, 1},
	{"mvreflsh_offset", NULL, 1},
	{"luma_avg_th", NULL, 1},/*80*/
	{"", NULL, 0}
};

struct dnlp_alg_dbg_parse_cmd_s dnlp_dbg_parse_ro_cmd[] = {
	{"luma_avg4", NULL, 1},
	{"var_d8", NULL, 1},
	{"scurv_gain", NULL, 1},
	{"blk_wht_ext0", NULL, 1},
	{"blk_wht_ext1", NULL, 1},
	{"dnlp_brightness", NULL, 1},/*5*/
	{"", NULL, 0}
};

struct dnlp_alg_dbg_parse_cmd_s dnlp_dbg_parse_cv_cmd[] = {
	{"scurv_low", NULL, VPP_DNLP_SCURV_LEN},
	{"scurv_mid1", NULL, VPP_DNLP_SCURV_LEN},
	{"scurv_mid2", NULL, VPP_DNLP_SCURV_LEN},
	{"scurv_hgh1", NULL, VPP_DNLP_SCURV_LEN},
	{"scurv_hgh2", NULL, VPP_DNLP_SCURV_LEN},
	{"gain_var_lut49", NULL, VPP_DNLP_GAIN_VAR_LUT_LEN},/*5*/
	{"c_hist_gain", NULL, VPP_DNLP_HIST_GAIN_LEN},
	{"s_hist_gain", NULL, VPP_DNLP_HIST_GAIN_LEN},
	{"wext_gain", NULL, VPP_DNLP_WEXT_GAIN_LEN},
	{"adp_thrd", NULL, VPP_DNLP_ADP_THRD_LEN},
	{"reg_blk_boost_12", NULL, VPP_DNLP_REG_BLK_BOOST_LEN},/*10*/
	{"reg_adp_ofset_20", NULL, VPP_DNLP_REG_ADP_OFSET_LEN},
	{"reg_mono_protect", NULL, VPP_DNLP_REG_MONO_PROT_LEN},
	{"reg_trend_wht_expand_lut8", NULL, VPP_DNLP_TREND_WHT_EXP_LUT_LEN},
	{"ve_dnlp_tgt", NULL, VPP_DNLP_SCURV_LEN},
	{"ve_dnlp_tgt_10b", NULL, VPP_DNLP_SCURV_LEN},/*15*/
	{"gmscurve", NULL, VPP_DNLP_SCURV_LEN},
	{"clash_curve", NULL, VPP_DNLP_SCURV_LEN},
	{"clsh_scvbld", NULL, VPP_DNLP_SCURV_LEN},
	{"blkwht_ebld", NULL, VPP_DNLP_SCURV_LEN},
	{"", NULL, 0}
};

struct dnlp_alg_param_ai_s dnlp_alg_ai_offset;

/*Internal functions*/
static void _alg_fw_dnlp_debug_init(void)
{
	int i = 0;

	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_alg_enable;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_respond;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_sel;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_respond_flag;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_smhist_ck;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_mvreflsh;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_pavg_btsft;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_dbg_i2r;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_cuvbld_min;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_cuvbld_max;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_schg_sft;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_bbd_ratio_low;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_bbd_ratio_hig;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_limit_rng;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_range_det;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_blk_cctr;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_brgt_ctrl;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_brgt_range;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_brght_add;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_brght_max;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_dbg_adjavg;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_auto_rng;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_lowrange;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_hghrange;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_satur_rat;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_satur_max;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_set_saturtn;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_sbgnbnd;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_sendbnd;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_clashbgn;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_clashend;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_var_th;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_clahe_gain_neg;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_clahe_gain_pos;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_clahe_gain_delta;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_mtdbld_rate;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_adpmtd_lbnd;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_adpmtd_hbnd;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_blkext_ofst;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_whtext_ofst;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_blkext_rate;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_whtext_rate;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_bwext_div4x_min;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_irgnbgn;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_irgnend;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_dbg_map;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_final_gain;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_cliprate_v3;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_cliprate_min;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_adpcrat_lbnd;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_adpcrat_hbnd;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_scurv_low_th;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_scurv_mid1_th;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_scurv_mid2_th;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_scurv_hgh1_th;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_scurv_hgh2_th;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_mtdrate_adp_en;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_clahe_method;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_ble_en;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_norm;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_scn_chg_th;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_step_th;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_iir_step_mux;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_single_bin_bw;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_single_bin_method;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_reg_max_slop_1st;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_reg_max_slop_mid;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_reg_max_slop_fin;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_reg_min_slop_1st;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_reg_min_slop_mid;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_reg_min_slop_fin;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_reg_trend_wht_expand_mode;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_reg_trend_blk_expand_mode;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_ve_hist_cur_gain;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_ve_hist_cur_gain_precise;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_reg_mono_binrang_st;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_reg_mono_binrang_ed;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_c_hist_gain_base;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_s_hist_gain_base;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_mvreflsh_offset;
	dnlp_dbg_parse_cmd[i++].value = &dnlp_alg_node_param->dnlp_luma_avg_th;

	i = 0;
	dnlp_dbg_parse_ro_cmd[i++].value = dnlp_dbg_ro->ro_luma_avg4;
	dnlp_dbg_parse_ro_cmd[i++].value = dnlp_dbg_ro->ro_var_d8;
	dnlp_dbg_parse_ro_cmd[i++].value = dnlp_dbg_ro->ro_scurv_gain;
	dnlp_dbg_parse_ro_cmd[i++].value = dnlp_dbg_ro->ro_blk_wht_ext0;
	dnlp_dbg_parse_ro_cmd[i++].value = dnlp_dbg_ro->ro_blk_wht_ext1;
	dnlp_dbg_parse_ro_cmd[i++].value = dnlp_dbg_ro->ro_dnlp_brightness;

	i = 0;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_rw->dnlp_scurv_low;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_rw->dnlp_scurv_mid1;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_rw->dnlp_scurv_mid2;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_rw->dnlp_scurv_hgh1;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_rw->dnlp_scurv_hgh2;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_rw->gain_var_lut49;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_rw->c_hist_gain;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_rw->s_hist_gain;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_rw->wext_gain;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_rw->adp_thrd;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_rw->reg_blk_boost_12;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_rw->reg_adp_ofset_20;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_rw->reg_mono_protect;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_rw->reg_trend_wht_expand_lut8;
	dnlp_dbg_parse_cv_cmd[i++].value = (int *)dnlp_alg_out->ve_dnlp_tgt;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_alg_out->ve_dnlp_tgt_10b;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_ro->gmscurve;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_ro->clash_curve;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_ro->clsh_scvbld;
	dnlp_dbg_parse_cv_cmd[i++].value = dnlp_dbg_ro->blkwht_ebld;
}

static void _alg_fw_dnlp_algorithm_init(void)
{
	if (dnlp_alg_insmod_ok)
		return;

	dnlp_alg_function = dnlp_drv_param_get();

	if (dnlp_alg_function) {
		if (!dnlp_alg_function->dnlp_alg_param ||
			!dnlp_alg_function->input_param ||
			!dnlp_alg_function->output_param ||
			!dnlp_alg_function->ro_param ||
			!dnlp_alg_function->rw_param ||
			!dnlp_alg_function->ble_whe_param ||
			!dnlp_alg_function->dbg_alg_print ||
			!dnlp_alg_function->dnlp_algorithm_main) {
			dnlp_alg_insmod_ok = false;
			PR_DRV("%s: is fail.\n", __func__);
			return;
		}

		dnlp_alg_in = dnlp_alg_function->input_param;
		dnlp_alg_out = dnlp_alg_function->output_param;
		dnlp_dbg_ro = dnlp_alg_function->ro_param;
		dnlp_dbg_rw = dnlp_alg_function->rw_param;
		dnlp_dbg_info = dnlp_alg_function->dbg_alg_print;
		dnlp_alg_node_param = dnlp_alg_function->dnlp_alg_param;
		ble_whe_data = dnlp_alg_function->ble_whe_param;

		dnlp_alg_node_param->dnlp_alg_enable = 1;
		dnlp_alg_node_param->dnlp_respond = 0;
		dnlp_alg_node_param->dnlp_sel = 2;
		dnlp_alg_node_param->dnlp_respond_flag = 0;
		dnlp_alg_node_param->dnlp_smhist_ck = 0;
		dnlp_alg_node_param->dnlp_mvreflsh = 6;
		dnlp_alg_node_param->dnlp_pavg_btsft = 5;
		dnlp_alg_node_param->dnlp_dbg_i2r = 255;
		dnlp_alg_node_param->dnlp_cuvbld_min = 3;
		dnlp_alg_node_param->dnlp_cuvbld_max = 17;
		dnlp_alg_node_param->dnlp_schg_sft = 1;
		dnlp_alg_node_param->dnlp_bbd_ratio_low = 16;
		dnlp_alg_node_param->dnlp_bbd_ratio_hig = 128;
		dnlp_alg_node_param->dnlp_limit_rng = 1;
		dnlp_alg_node_param->dnlp_range_det = 0;
		dnlp_alg_node_param->dnlp_blk_cctr = 8;
		dnlp_alg_node_param->dnlp_brgt_ctrl = 48;
		dnlp_alg_node_param->dnlp_brgt_range = 16;
		dnlp_alg_node_param->dnlp_brght_add = 32;
		dnlp_alg_node_param->dnlp_brght_max = 0;
		dnlp_alg_node_param->dnlp_dbg_adjavg = 0;
		dnlp_alg_node_param->dnlp_auto_rng = 0;
		dnlp_alg_node_param->dnlp_lowrange = 18;
		dnlp_alg_node_param->dnlp_hghrange = 18;
		dnlp_alg_node_param->dnlp_satur_rat = 30;
		dnlp_alg_node_param->dnlp_satur_max = 0;
		dnlp_alg_node_param->dnlp_set_saturtn = 0;
		dnlp_alg_node_param->dnlp_sbgnbnd = 4;
		dnlp_alg_node_param->dnlp_sendbnd = 4;
		dnlp_alg_node_param->dnlp_clashbgn = 4;
		dnlp_alg_node_param->dnlp_clashend = 59;
		dnlp_alg_node_param->dnlp_var_th = 16;
		dnlp_alg_node_param->dnlp_clahe_gain_neg = 120;
		dnlp_alg_node_param->dnlp_clahe_gain_pos = 24;
		dnlp_alg_node_param->dnlp_clahe_gain_delta = 32;
		dnlp_alg_node_param->dnlp_mtdbld_rate = 40;
		dnlp_alg_node_param->dnlp_adpmtd_lbnd = 19;
		dnlp_alg_node_param->dnlp_adpmtd_hbnd = 20;
		dnlp_alg_node_param->dnlp_blkext_ofst = 2;
		dnlp_alg_node_param->dnlp_whtext_ofst = 1;
		dnlp_alg_node_param->dnlp_blkext_rate = 32;
		dnlp_alg_node_param->dnlp_whtext_rate = 16;
		dnlp_alg_node_param->dnlp_bwext_div4x_min = 16;
		dnlp_alg_node_param->dnlp_irgnbgn = 0;
		dnlp_alg_node_param->dnlp_irgnend = 64;
		dnlp_alg_node_param->dnlp_dbg_map = 0;
		dnlp_alg_node_param->dnlp_final_gain = 8;
		dnlp_alg_node_param->dnlp_cliprate_v3 = 36;
		dnlp_alg_node_param->dnlp_cliprate_min = 19;
		dnlp_alg_node_param->dnlp_adpcrat_lbnd = 10;
		dnlp_alg_node_param->dnlp_adpcrat_hbnd = 20;
		dnlp_alg_node_param->dnlp_scurv_low_th = 32;
		dnlp_alg_node_param->dnlp_scurv_mid1_th = 48;
		dnlp_alg_node_param->dnlp_scurv_mid2_th = 112;
		dnlp_alg_node_param->dnlp_scurv_hgh1_th = 176;
		dnlp_alg_node_param->dnlp_scurv_hgh2_th = 240;
		dnlp_alg_node_param->dnlp_mtdrate_adp_en = 1;
		dnlp_alg_node_param->dnlp_clahe_method = 1;
		dnlp_alg_node_param->dnlp_ble_en = 1;
		dnlp_alg_node_param->dnlp_norm = 10;
		dnlp_alg_node_param->dnlp_scn_chg_th = 48;
		dnlp_alg_node_param->dnlp_step_th = 1;
		dnlp_alg_node_param->dnlp_iir_step_mux = 1;
		dnlp_alg_node_param->dnlp_single_bin_bw = 2;
		dnlp_alg_node_param->dnlp_single_bin_method = 1;
		dnlp_alg_node_param->dnlp_reg_max_slop_1st = 614;
		dnlp_alg_node_param->dnlp_reg_max_slop_mid = 400;
		dnlp_alg_node_param->dnlp_reg_max_slop_fin = 614;
		dnlp_alg_node_param->dnlp_reg_min_slop_1st = 77;
		dnlp_alg_node_param->dnlp_reg_min_slop_mid = 144;
		dnlp_alg_node_param->dnlp_reg_min_slop_fin = 77;
		dnlp_alg_node_param->dnlp_reg_trend_wht_expand_mode = 2;
		dnlp_alg_node_param->dnlp_reg_trend_blk_expand_mode = 2;
		dnlp_alg_node_param->dnlp_ve_hist_cur_gain = 8;
		dnlp_alg_node_param->dnlp_ve_hist_cur_gain_precise = 8;
		dnlp_alg_node_param->dnlp_reg_mono_binrang_st = 7;
		dnlp_alg_node_param->dnlp_reg_mono_binrang_ed = 26;
		dnlp_alg_node_param->dnlp_c_hist_gain_base = 128;
		dnlp_alg_node_param->dnlp_s_hist_gain_base = 128;
		dnlp_alg_node_param->dnlp_mvreflsh_offset = 0;
		dnlp_alg_node_param->dnlp_luma_avg_th = 50;

		dnlp_alg_insmod_ok = true;
		dnlp_alg_function->dnlp_insmod_ok = &dnlp_alg_insmod_ok;
		PR_DRV("%s: is OK\n", __func__);

		_alg_fw_dnlp_debug_init();
	} else {
		PR_DRV("%s: dnlp_alg_function is NULL\n", __func__);
	}
}

/*External functions*/
int vpp_alg_fw_dnlp_init(void)
{
	pre_sat = 0;
	dnlp_alg_insmod_ok = false;

	_alg_fw_dnlp_algorithm_init();

	memset(&dnlp_alg_ai_offset,
		0, sizeof(struct dnlp_alg_param_ai_s));

	return 0;
}

void vpp_alg_fw_dnlp_en(bool enable)
{
	if (!dnlp_alg_insmod_ok || !dnlp_alg_node_param)
		return;

//	pr_vpp_alg(PR_ALG_DNLP, "[%s] enable = %d\n",
//		__func__, enable);

	dnlp_alg_node_param->dnlp_alg_enable = enable;
}

void vpp_alg_fw_dnlp_set_final_gain(unsigned int data)
{
	if (!dnlp_alg_insmod_ok || !dnlp_alg_node_param)
		return;

//	pr_vpp_alg(PR_ALG_DNLP, "[%s] data = %d\n", __func__, data);

	dnlp_alg_node_param->dnlp_final_gain = data;
}

void vpp_alg_fw_dnlp_calculate_tgtx(int hist_luma_sum,
	unsigned short *hist_data)
{
	int i = 0;
	int same_bin_num = 0;
	unsigned int raw_hist_sum = 0;
	unsigned int *tmp0;
	unsigned int *tmp1;

	if (!dnlp_alg_insmod_ok || !dnlp_alg_function || !hist_data)
		return;

	/*calculate iir-coef params*/
	if (dnlp_alg_node_param->dnlp_mvreflsh < 1)
		dnlp_alg_node_param->dnlp_mvreflsh = 1;

	*dnlp_alg_in->rbase = 1 << dnlp_alg_node_param->dnlp_mvreflsh;

	/*load histogram*/
	*dnlp_alg_in->ve_dnlp_luma_sum = hist_luma_sum;

	tmp0 = dnlp_alg_in->pre_0_gamma;
	tmp1 = dnlp_alg_in->pre_1_gamma;

	for (i = 0; i < VPP_HIST_BIN_COUNT; i++) {
		/*histogram stored for one frame delay*/
		tmp1[i] = tmp0[i];

		tmp0[i] = (unsigned int)hist_data[i];
		raw_hist_sum += (unsigned int)hist_data[i];

		/*counter same histogram*/
		if (tmp1[i] == tmp0[i])
			same_bin_num++;
	}

	/*all same histogram as last frame, freeze DNLP*/
	if (same_bin_num == VPP_HIST_BIN_COUNT &&
		dnlp_alg_node_param->dnlp_smhist_ck &&
		!dnlp_alg_node_param->dnlp_respond_flag)
		return;

	dnlp_alg_function->dnlp_algorithm_main(raw_hist_sum);
}

void vpp_alg_fw_dnlp_cal_sat_compensation(bool *sat_comp, int *sat_val)
{
	bool do_comp = false;
	int val = 0;
	int t0 = 0;
	int t1 = 0;
	int tmp0 = 0;
	int tmp1 = 0;
	int i = 0;
	int tmp = 0;
	int sat_rat = 0;
	int sat_max = 0;
	int set_sat = 0;

	if (!dnlp_alg_insmod_ok || !dnlp_alg_function ||
		!sat_comp || !sat_val)
		return;

	for (i = 1; i < DNLP_BIN_SIZE; i++) {
		tmp = dnlp_alg_out->ve_dnlp_tgt[i];
		if (tmp > 4 * i) {
			t0 += (tmp - 4 * i) * (DNLP_BIN_SIZE + 1 - i);
			t1 += (DNLP_BIN_SIZE + 1 - i);
		}
	}

	sat_rat = dnlp_alg_node_param->dnlp_satur_rat;
	sat_max = dnlp_alg_node_param->dnlp_satur_max;
	set_sat = dnlp_alg_node_param->dnlp_set_saturtn;

	tmp = (t0 * sat_rat + (t1 >> 1)) / (t1 + 1);
	tmp0 = (tmp + 4) >> 3;

	tmp = sat_max << 3;
	tmp1 = MIN(tmp0, tmp);

	if (set_sat == 0) {
		if (tmp1 != pre_sat) {
			do_comp = true;
			val = tmp1 + 512;
			pre_sat = tmp1;
		} else {
			do_comp = false;
		}
	} else {
		if (pre_sat != set_sat) {
			do_comp = true;
			pre_sat = set_sat;
			if (set_sat < 512)
				val = set_sat + 512;
			else
				val = set_sat;
		} else {
			do_comp = false;
		}
	}

	*sat_comp = do_comp;
	*sat_val = val;
}

void vpp_alg_fw_dnlp_set_param(struct vpp_dnlp_curve_param_s *data)
{
	if (!dnlp_alg_insmod_ok || !data)
		return;

//	pr_vpp_alg(PR_ALG_DNLP, "[%s] set data\n", __func__);

	/*load static curve*/
	memcpy(dnlp_dbg_rw->dnlp_scurv_low,
		data->dnlp_scurv_low,
		sizeof(int) * VPP_DNLP_SCURV_LEN);
	memcpy(dnlp_dbg_rw->dnlp_scurv_mid1,
		data->dnlp_scurv_mid1,
		sizeof(int) * VPP_DNLP_SCURV_LEN);
	memcpy(dnlp_dbg_rw->dnlp_scurv_mid2,
		data->dnlp_scurv_mid2,
		sizeof(int) * VPP_DNLP_SCURV_LEN);
	memcpy(dnlp_dbg_rw->dnlp_scurv_hgh1,
		data->dnlp_scurv_hgh1,
		sizeof(int) * VPP_DNLP_SCURV_LEN);
	memcpy(dnlp_dbg_rw->dnlp_scurv_hgh2,
		data->dnlp_scurv_hgh2,
		sizeof(int) * VPP_DNLP_SCURV_LEN);
	/*load gain var*/
	memcpy(dnlp_dbg_rw->gain_var_lut49,
		data->gain_var_lut49,
		sizeof(int) * VPP_DNLP_GAIN_VAR_LUT_LEN);
	/*load wext gain*/
	memcpy(dnlp_dbg_rw->wext_gain,
		data->wext_gain,
		sizeof(int) * VPP_DNLP_WEXT_GAIN_LEN);
	/*load new c curve lut for vlsi-kite.li*/
	memcpy(dnlp_dbg_rw->adp_thrd,
		data->adp_thrd,
		sizeof(int) * VPP_DNLP_ADP_THRD_LEN);
	memcpy(dnlp_dbg_rw->reg_blk_boost_12,
		data->reg_blk_boost_12,
		sizeof(int) * VPP_DNLP_REG_BLK_BOOST_LEN);
	memcpy(dnlp_dbg_rw->reg_adp_ofset_20,
		data->reg_adp_ofset_20,
		sizeof(int) * VPP_DNLP_REG_ADP_OFSET_LEN);
	memcpy(dnlp_dbg_rw->reg_mono_protect,
		data->reg_mono_protect,
		sizeof(int) * VPP_DNLP_REG_MONO_PROT_LEN);
	memcpy(dnlp_dbg_rw->reg_trend_wht_expand_lut8,
		data->reg_trend_wht_expand_lut8,
		sizeof(int) * VPP_DNLP_TREND_WHT_EXP_LUT_LEN);
	memcpy(dnlp_dbg_rw->c_hist_gain,
		data->c_hist_gain,
		sizeof(int) * VPP_DNLP_HIST_GAIN_LEN);
	memcpy(dnlp_dbg_rw->s_hist_gain,
		data->s_hist_gain,
		sizeof(int) * VPP_DNLP_HIST_GAIN_LEN);

	*dnlp_alg_in->menu_chg_en = 1;

	/*calc iir-coef params*/
	dnlp_alg_node_param->dnlp_smhist_ck =
		data->param[EN_DNLP_SMHIST_CK];
	dnlp_alg_node_param->dnlp_mvreflsh =
		data->param[EN_DNLP_MVREFLSH];

	dnlp_alg_node_param->dnlp_cuvbld_min =
		data->param[EN_DNLP_CUVBLD_MIN];
	dnlp_alg_node_param->dnlp_cuvbld_max =
		data->param[EN_DNLP_CUVBLD_MAX];

	/*histogram refine parms (remove bb affects)*/
	dnlp_alg_node_param->dnlp_bbd_ratio_low =
		data->param[EN_DNLP_BBD_RATIO_LOW];
	dnlp_alg_node_param->dnlp_bbd_ratio_hig =
		data->param[EN_DNLP_BBD_RATIO_HIG];

	/*brightness_plus*/
	dnlp_alg_node_param->dnlp_blk_cctr =
		data->param[EN_DNLP_BLK_CCTR];
	dnlp_alg_node_param->dnlp_brgt_ctrl =
		data->param[EN_DNLP_BRGT_CTRL];
	dnlp_alg_node_param->dnlp_brgt_range =
		data->param[EN_DNLP_BRGT_RANGE];
	dnlp_alg_node_param->dnlp_brght_add =
		data->param[EN_DNLP_BRGHT_ADD];
	dnlp_alg_node_param->dnlp_brght_max =
		data->param[EN_DNLP_BRGHT_MAX];

	/*hist auto range parms*/
	dnlp_alg_node_param->dnlp_auto_rng =
		data->param[EN_DNLP_AUTO_RNG];
	dnlp_alg_node_param->dnlp_lowrange =
		data->param[EN_DNLP_LOWRANGE];
	dnlp_alg_node_param->dnlp_hghrange =
		data->param[EN_DNLP_HGHRANGE];

	/*for s curves processing range*/
	dnlp_alg_node_param->dnlp_satur_rat =
		data->param[EN_DNLP_SATUR_RAT];
	dnlp_alg_node_param->dnlp_satur_max =
		data->param[EN_DNLP_SATUR_MAX];
	dnlp_alg_node_param->dnlp_sbgnbnd =
		data->param[EN_DNLP_SBGNBND];
	dnlp_alg_node_param->dnlp_sendbnd =
		data->param[EN_DNLP_SENDBND];

	/*for clahe_curves processing range*/
	dnlp_alg_node_param->dnlp_clashbgn =
		data->param[EN_DNLP_CLASH_BGN];
	dnlp_alg_node_param->dnlp_clashend =
		data->param[EN_DNLP_CLASH_END];

	/*gains to delta of curves (for strength of the DNLP)*/
	dnlp_alg_node_param->dnlp_clahe_gain_neg =
		data->param[EN_DNLP_CLAHE_GAIN_NEG];
	dnlp_alg_node_param->dnlp_clahe_gain_pos =
		data->param[EN_DNLP_CLAHE_GAIN_POS];
	dnlp_alg_node_param->dnlp_final_gain =
		data->param[EN_DNLP_FINAL_GAIN] +
		dnlp_alg_ai_offset.param[EN_DNLP_ALG_AI_FINAL_GAIN];

	/*coef of blending between gma_scurv and clahe curves*/
	dnlp_alg_node_param->dnlp_mtdbld_rate =
		data->param[EN_DNLP_MTDBLD_RATE];
	dnlp_alg_node_param->dnlp_adpmtd_lbnd =
		data->param[EN_DNLP_ADPMTD_LBND];
	dnlp_alg_node_param->dnlp_adpmtd_hbnd =
		data->param[EN_DNLP_ADPMTD_HBND];

	/*black white extension control params*/
	dnlp_alg_node_param->dnlp_blkext_ofst =
		data->param[EN_DNLP_BLKEXT_OFST];
	dnlp_alg_node_param->dnlp_whtext_ofst =
		data->param[EN_DNLP_WHTEXT_OFST];
	dnlp_alg_node_param->dnlp_blkext_rate =
		data->param[EN_DNLP_BLKEXT_RATE];
	dnlp_alg_node_param->dnlp_whtext_rate =
		data->param[EN_DNLP_WHTEXT_RATE];
	dnlp_alg_node_param->dnlp_bwext_div4x_min =
		data->param[EN_DNLP_BWEXT_DIV4X_MIN];
	dnlp_alg_node_param->dnlp_irgnbgn =
		data->param[EN_DNLP_IRGNBGN];
	dnlp_alg_node_param->dnlp_irgnend =
		data->param[EN_DNLP_IRGNEND];

	/*curve- clahe*/
	dnlp_alg_node_param->dnlp_cliprate_v3 =
		data->param[EN_DNLP_CLIPRATE_V3];
	dnlp_alg_node_param->dnlp_cliprate_min =
		data->param[EN_DNLP_CLIPRATE_MIN];
	dnlp_alg_node_param->dnlp_adpcrat_lbnd =
		data->param[EN_DNLP_ADPCRAT_LBND];
	dnlp_alg_node_param->dnlp_adpcrat_hbnd =
		data->param[EN_DNLP_ADPCRAT_HBND];

	/*adaptive saturation compensations*/
	dnlp_alg_node_param->dnlp_scurv_low_th =
		data->param[EN_DNLP_SCURV_LOW_TH];
	dnlp_alg_node_param->dnlp_scurv_mid1_th =
		data->param[EN_DNLP_SCURV_MID1_TH];
	dnlp_alg_node_param->dnlp_scurv_mid2_th =
		data->param[EN_DNLP_SCURV_MID2_TH];
	dnlp_alg_node_param->dnlp_scurv_hgh1_th =
		data->param[EN_DNLP_SCURV_HGH1_TH];
	dnlp_alg_node_param->dnlp_scurv_hgh2_th =
		data->param[EN_DNLP_SCURV_HGH2_TH];

	/*new c curve param add for vlsi-kiteli*/
	dnlp_alg_node_param->dnlp_mtdrate_adp_en =
		data->param[EN_DNLP_MTDRATE_ADP_EN];
	dnlp_alg_node_param->dnlp_clahe_method =
		data->param[EN_DNLP_CLAHE_METHOD];
	dnlp_alg_node_param->dnlp_ble_en =
		data->param[EN_DNLP_BLE_EN];
	dnlp_alg_node_param->dnlp_norm =
		data->param[EN_DNLP_NORM];
	dnlp_alg_node_param->dnlp_scn_chg_th =
		data->param[EN_DNLP_SCN_CHG_TH];
	dnlp_alg_node_param->dnlp_iir_step_mux =
		data->param[EN_DNLP_IIR_STEP_MUX];
	dnlp_alg_node_param->dnlp_single_bin_bw =
		data->param[EN_DNLP_SINGLE_BIN_BW];
	dnlp_alg_node_param->dnlp_single_bin_method =
		data->param[EN_DNLP_SINGLE_BIN_METHOD];
	dnlp_alg_node_param->dnlp_reg_max_slop_1st =
		data->param[EN_DNLP_REG_MAX_SLOP_1ST];
	dnlp_alg_node_param->dnlp_reg_max_slop_mid =
		data->param[EN_DNLP_REG_MAX_SLOP_MID];
	dnlp_alg_node_param->dnlp_reg_max_slop_fin =
		data->param[EN_DNLP_REG_MAX_SLOP_FIN];
	dnlp_alg_node_param->dnlp_reg_min_slop_1st =
		data->param[EN_DNLP_REG_MIN_SLOP_1ST];
	dnlp_alg_node_param->dnlp_reg_min_slop_mid =
		data->param[EN_DNLP_REG_MIN_SLOP_MID];
	dnlp_alg_node_param->dnlp_reg_min_slop_fin =
		data->param[EN_DNLP_REG_MIN_SLOP_FIN];
	dnlp_alg_node_param->dnlp_reg_trend_wht_expand_mode =
		data->param[EN_DNLP_REG_TREND_WHT_EXPAND_MODE];
	dnlp_alg_node_param->dnlp_reg_trend_blk_expand_mode =
		data->param[EN_DNLP_REG_TREND_BLK_EXPAND_MODE];
	dnlp_alg_node_param->dnlp_ve_hist_cur_gain =
		data->param[EN_DNLP_HIST_CUR_GAIN];
	dnlp_alg_node_param->dnlp_ve_hist_cur_gain_precise =
		data->param[EN_DNLP_HIST_CUR_GAIN_PRECISE];
	dnlp_alg_node_param->dnlp_reg_mono_binrang_st =
		data->param[EN_DNLP_REG_MONO_BINRANG_ST];
	dnlp_alg_node_param->dnlp_reg_mono_binrang_ed =
		data->param[EN_DNLP_REG_MONO_BINRANG_ED];
	dnlp_alg_node_param->dnlp_c_hist_gain_base =
		data->param[EN_DNLP_C_HIST_GAIN_BASE];
	dnlp_alg_node_param->dnlp_s_hist_gain_base =
		data->param[EN_DNLP_S_HIST_GAIN_BASE];
	dnlp_alg_node_param->dnlp_mvreflsh_offset =
		data->param[EN_DNLP_MVREFLSH_OFFSET];
	dnlp_alg_node_param->dnlp_luma_avg_th =
		data->param[EN_DNLP_LUMA_AVG_TH];
}

void vpp_alg_fw_dnlp_set_ble_whe(struct vpp_ble_whe_param_s *data)
{
	if (!dnlp_alg_insmod_ok || !data)
		return;

//	pr_vpp_alg(PR_ALG_DNLP, "[%s] set data\n", __func__);

	ble_whe_data->blk_adj_en = data->param[EN_DNLP_BLK_EN];
	ble_whe_data->blk_end = data->param[EN_DNLP_BLK_END];
	ble_whe_data->blk_slp = data->param[EN_DNLP_BLK_SLOPE];

	ble_whe_data->brt_adj_en = data->param[EN_DNLP_BRT_EN];
	ble_whe_data->brt_start = data->param[EN_DNLP_BRT_START];
	ble_whe_data->brt_slp = data->param[EN_DNLP_BRT_SLOPE];
}

void vpp_alg_fw_dnlp_set_ai_param(struct dnlp_alg_param_ai_s *data)
{
	if (!dnlp_alg_insmod_ok || !data)
		return;

//	pr_vpp_alg(PR_ALG_DNLP, "[%s] set data\n", __func__);

	dnlp_alg_node_param->dnlp_final_gain +=
		data->param[EN_DNLP_ALG_AI_FINAL_GAIN];
	dnlp_alg_ai_offset.param[EN_DNLP_ALG_AI_FINAL_GAIN] =
		data->param[EN_DNLP_ALG_AI_FINAL_GAIN];
}

bool vpp_alg_fw_dnlp_get_insmod_status(void)
{
	return dnlp_alg_insmod_ok;
}

void vpp_alg_fw_dnlp_get_final_curve(unsigned int *data)
{
	int i = 0;

	if (!dnlp_alg_insmod_ok || !data)
		return;

	for (i = 0; i < DNLP_BIN_SIZE; i++)
		data[i] = dnlp_alg_out->ve_dnlp_tgt[i];
}

struct dnlp_alg_dbg_parse_cmd_s *vpp_alg_fw_dnlp_get_dbg_info(void)
{
	return &dnlp_dbg_parse_cmd[0];
}

struct dnlp_alg_dbg_parse_cmd_s *vpp_alg_fw_dnlp_get_dbg_ro_info(void)
{
	return &dnlp_dbg_parse_ro_cmd[0];
}

struct dnlp_alg_dbg_parse_cmd_s *vpp_alg_fw_dnlp_get_dbg_cv_info(void)
{
	return &dnlp_dbg_parse_cv_cmd[0];
}

#endif

