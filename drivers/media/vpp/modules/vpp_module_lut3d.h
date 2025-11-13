/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#ifndef __VPP_MODULE_LUT3D_H__
#define __VPP_MODULE_LUT3D_H__

int vpp_module_lut3d_init(struct vpp_dev_s *dev);
unsigned int vpp_module_lut3d_get_support(void);
unsigned int vpp_module_lut3d_get_lut_point(void);
unsigned int vpp_module_lut3d_get_table_len(void);
void vpp_module_lut3d_mount(void);
void vpp_module_lut3d_demount(void);
void vpp_module_lut3d_en(bool enable, enum io_mode_e io_mode);
void vpp_module_lut3d_set_data(int *data,
	unsigned int data_length, enum io_mode_e io_mode);
void vpp_module_lut3d_pattern_en(bool enable);
void vpp_module_lut3d_pattern_set(unsigned int val_r,
	unsigned int val_g, unsigned int val_b);
void vpp_module_lut3d_on_vs(void);

void vpp_module_lut3d_dump_info(enum vpp_dump_module_info_e info_type);

#endif
#endif

