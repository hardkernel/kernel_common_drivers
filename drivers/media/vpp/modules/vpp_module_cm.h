/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#ifndef __VPP_MODULE_CM_H__
#define __VPP_MODULE_CM_H__

#define CM2_CURVE_SIZE (32)

enum cm_tuning_param_e {
	EN_PARAM_GLB_HUE = 0,
	EN_PARAM_GLB_SAT,
	EN_PARAM_FINAL_GAIN_LUMA,
	EN_PARAM_FINAL_GAIN_SAT,
	EN_PARAM_FINAL_GAIN_HUE,
};

enum cm_sub_module_e {
	EN_SUB_MD_LUMA_ADJ = 0,
	EN_SUB_MD_SAT_ADJ,
	EN_SUB_MD_HUE_ADJ,
	EN_SUB_MD_FILTER,
};

struct cm_cfg_param_s {
	int frm_height;
	int frm_width;
};

struct cm_ai_pq_param_s {
	int sat[CM2_CURVE_SIZE * 3];
};

int vpp_module_cm_init(struct vpp_dev_s *dev);
int vpp_module_cm_en(bool enable, enum io_mode_e io_mode);
void vpp_module_cm_set_cm2_luma(int *data);
void vpp_module_cm_set_cm2_sat(int *data);
void vpp_module_cm_set_cm2_sat_by_l(int *data);
void vpp_module_cm_set_cm2_sat_by_hl(int *data);
void vpp_module_cm_set_cm2_hue(int *data);
void vpp_module_cm_set_cm2_hue_by_hs(int *data);
void vpp_module_cm_set_cm2_hue_by_hl(int *data);
void vpp_module_cm_set_demo_mode(bool enable, bool left_side);
void vpp_module_cm_set_cfg_param(struct cm_cfg_param_s *param);
void vpp_module_cm_set_tuning_param(enum cm_tuning_param_e type,
	int *data);
void vpp_module_cm_sub_module_en(enum cm_sub_module_e sub_module,
	bool enable);
void vpp_module_cm_set_cm2_offset_luma(char *data);
void vpp_module_cm_set_cm2_offset_sat(char *data);
void vpp_module_cm_set_cm2_offset_hue(char *data);
void vpp_module_cm_set_cm2_offset_hue_by_hs(char *data);
void vpp_module_cm_set_default(void);
int vpp_module_cm_get_cm2_luma(int *data);
int vpp_module_cm_get_cm2_sat(int *data);
int vpp_module_cm_get_cm2_sat_by_l(int *data);
int vpp_module_cm_get_cm2_sat_by_hl(int *data);
int vpp_module_cm_get_cm2_hue(int *data);
int vpp_module_cm_get_cm2_hue_by_hs(int *data);
int vpp_module_cm_get_cm2_hue_by_hl(int *data);
bool vpp_module_cm_get_final_gain_support(void);
void vpp_module_cm_on_vs(void);

void vpp_module_cm_set_reg(unsigned int addr, int val);
int vpp_module_cm_get_reg(unsigned int addr);

void vpp_module_cm_get_ai_pq_base(struct cm_ai_pq_param_s *param);
void vpp_module_cm_set_ai_pq_offset(struct cm_ai_pq_param_s *param);

void vpp_module_cm_dump_info(enum vpp_dump_module_info_e info_type);

#endif
#endif

