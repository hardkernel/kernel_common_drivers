/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#ifndef __VPP_MODULE_GO_H__
#define __VPP_MODULE_GO_H__

#define GO_PARAM_CNT (3)

struct vpp_module_go_s {
	int pre_offset[GO_PARAM_CNT];     /*s11.0, range -1024~+1023, default is 0*/
	unsigned int gain[GO_PARAM_CNT];  /*u1.10, range 0~2047, default is 1024 (1.0x)*/
	int post_offset[GO_PARAM_CNT];    /*s11.0, range -1024~+1023, default is 0*/
};

int vpp_module_go_init(struct vpp_dev_s *dev);
void vpp_module_go_en(bool enable,
	enum io_mode_e io_mode);
void vpp_module_go_set_gain(unsigned char idx, int val,
	enum io_mode_e io_mode);
void vpp_module_go_set_offset(unsigned char idx, int val,
	enum io_mode_e io_mode);
void vpp_module_go_set_pre_offset(unsigned char idx, int val,
	enum io_mode_e io_mode);
void vpp_module_go_set_data(struct vpp_module_go_s *data,
	enum io_mode_e io_mode);
void vpp_module_go_get_en(int *val);
void vpp_module_go_get_data(struct vpp_module_go_s *data);
void vpp_module_go_on_vs(void);

void vpp_module_go_dump_info(enum vpp_dump_module_info_e info_type);

#endif
#endif

