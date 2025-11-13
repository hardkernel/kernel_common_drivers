/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#ifndef __VPP_DATA_H__
#define __VPP_DATA_H__

enum vpp_dump_data_info_e {
	EN_DUMP_DATA_LC = 0,
	EN_DUMP_DATA_SR_0,
	EN_DUMP_DATA_SR_1,
	EN_DUMP_DATA_VSR,
	EN_DUMP_DATA_VS_INFO,
};

struct data_vs_param_s {
	unsigned int  src_type;
	unsigned int vf_signal_change;
	unsigned int vf_width;
	unsigned int vf_height;
};

int vpp_data_init(struct vpp_dev_s *dev);
void vpp_data_update_reg_lc(struct vpp_lc_param_s *data);
void vpp_data_update_reg_sr(struct vpp_sr_param_s *data);
int vpp_data_get_sr_params_len(enum vpp_sr_type_e type);
void vpp_data_on_vs(struct data_vs_param_s *vs_param);
void vpp_data_dump_info(enum vpp_dump_data_info_e info_type);

#endif
#endif

