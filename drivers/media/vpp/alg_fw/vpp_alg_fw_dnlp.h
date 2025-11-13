/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#ifndef __VPP_ALG_FW_DNLP_H__
#define __VPP_ALG_FW_DNLP_H__

enum dnlp_alg_param_ai_e {
	EN_DNLP_ALG_AI_FINAL_GAIN = 0,
	EN_DNLP_ALG_AI_PARAM_MAX,
};

struct dnlp_alg_param_ai_s {
	int param[EN_DNLP_ALG_AI_PARAM_MAX];
};

struct dnlp_alg_dbg_parse_cmd_s {
	char *parse_string;
	unsigned int *value;
	unsigned int data_len;
};

int vpp_alg_fw_dnlp_init(void);
void vpp_alg_fw_dnlp_en(bool enable);
void vpp_alg_fw_dnlp_set_final_gain(unsigned int data);
void vpp_alg_fw_dnlp_calculate_tgtx(int hist_luma_sum,
	unsigned short *hist_data);
void vpp_alg_fw_dnlp_cal_sat_compensation(bool *sat_comp, int *sat_val);
void vpp_alg_fw_dnlp_set_param(struct vpp_dnlp_curve_param_s *data);
void vpp_alg_fw_dnlp_set_ble_whe(struct vpp_ble_whe_param_s *data);
void vpp_alg_fw_dnlp_set_ai_param(struct dnlp_alg_param_ai_s *data);
bool vpp_alg_fw_dnlp_get_insmod_status(void);
void vpp_alg_fw_dnlp_get_final_curve(unsigned int *data);
struct dnlp_alg_dbg_parse_cmd_s *vpp_alg_fw_dnlp_get_dbg_info(void);
struct dnlp_alg_dbg_parse_cmd_s *vpp_alg_fw_dnlp_get_dbg_ro_info(void);
struct dnlp_alg_dbg_parse_cmd_s *vpp_alg_fw_dnlp_get_dbg_cv_info(void);

#endif
#endif

