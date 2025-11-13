/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#ifndef __VPP_MODULE_VADJ_H__
#define __VPP_MODULE_VADJ_H__

enum vadj_param_e {
	EN_VADJ_VD1_RGBBST_EN = 0,
	EN_VADJ_POST_RGBBST_EN,
	EN_VADJ_PARAM_MAX,
};

struct vadj_ai_pq_param_s {
	int sat_hue_mad;
};

int vpp_module_vadj_init(struct vpp_dev_s *dev);
void vpp_module_vadj_en(bool enable, enum io_mode_e io_mode);
void vpp_module_vadj_post_en(bool enable, enum io_mode_e io_mode);
void vpp_module_vadj_set_param(enum vadj_param_e index,
	int val, enum io_mode_e io_mode);
int vpp_module_vadj_set_brightness(int val,
	enum io_mode_e io_mode);
int vpp_module_vadj_set_brightness_post(int val,
	enum io_mode_e io_mode);
int vpp_module_vadj_set_contrast(int val,
	enum io_mode_e io_mode);
int vpp_module_vadj_set_contrast_post(int val,
	enum io_mode_e io_mode);
int vpp_module_vadj_set_sat_hue(int val,
	enum io_mode_e io_mode);
int vpp_module_vadj_set_sat_hue_post(int val,
	enum io_mode_e io_mode);
void vpp_module_vadj_on_vs(void);

void vpp_module_vadj_get_ai_pq_base(struct vadj_ai_pq_param_s *param);
void vpp_module_vadj_set_ai_pq_offset(struct vadj_ai_pq_param_s *param);

void vpp_module_vadj_dump_info(enum vpp_dump_module_info_e info_type);

#endif
#endif

