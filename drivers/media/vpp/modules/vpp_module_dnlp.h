/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#ifndef __VPP_MODULE_DNLP_H__
#define __VPP_MODULE_DNLP_H__

struct dnlp_ai_pq_param_s {
	int final_gain;
};

int vpp_module_dnlp_init(struct vpp_dev_s *dev);
void vpp_module_dnlp_en(bool enable, enum io_mode_e io_mode);
void vpp_module_dnlp_on_vs(int hist_luma_sum, unsigned short *hist_data);
void vpp_module_dnlp_get_sat_compensation(bool *sat_comp, int *sat_val);
void vpp_module_dnlp_set_param(struct vpp_dnlp_curve_param_s *data);
void vpp_module_dnlp_set_ble_whe(struct vpp_ble_whe_param_s *data);

void vpp_module_dnlp_get_ai_pq_base(struct dnlp_ai_pq_param_s *param);
void vpp_module_dnlp_set_ai_pq_offset(struct dnlp_ai_pq_param_s *param);

void vpp_module_dnlp_dump_info(enum vpp_dump_module_info_e info_type);

#endif
#endif

