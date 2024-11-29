/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef VFRAME_DEWARP_COMPOSER_H
#define VFRAME_DEWARP_COMPOSER_H
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/amlogic/media/vout/vout_notify.h>
#include <linux/amlogic/media/vfm/vframe.h>
#include <linux/amlogic/media/gdc/gdc.h>

struct dewarp_vf_para_s {
	int src_vf_format;
	int src_vf_width;
	int src_vf_height;
	int src_vf_plane_count;
	int src_vf_angle;
	ulong src_buf_addr0;
	int src_buf_stride0;
	ulong src_buf_addr1;
	int src_buf_stride1;
	int src_endian;
	int dst_vf_format;
	int dst_vf_width;
	int dst_vf_height;
	int dst_vf_plane_count;
	ulong dst_buf_addr;
	int dst_buf_stride;
	int dst_endian;
	bool is_tvp;
	bool uvswap_enable;
};

struct pic_s {
	int format;
	u32 width;
	u32 height;
	ulong addr[2];
	u32 align_w;
	u32 align_h;
	int plane_count;
	bool is_tvp;
};

struct composer_input_para {
	int call_index;
	struct vframe_s *vframe;
	struct pic_s pic_info;
	int transform;
};

struct composer_output_para {
	struct pic_s pic_info;
};

struct dewarp_common_para {
	struct composer_input_para input_para;
	struct composer_output_para output_para;
};

struct dewarp_composer_para {
	int index;
	struct gdc_context_s *context;
	struct firmware_load_s fw_load;
	struct dewarp_vf_para_s *vf_para;
	struct firmware_rotate_s last_fw_param;
};

extern u32 dewarp_load_flag;
extern int dewarp_com_dump;
extern int dewarp_print;

int v2d_get_dewarp_format(int index, struct vframe_s *vf);
int v2d_load_dewarp_firmware(struct dewarp_composer_para *param);
int v2d_unload_dewarp_firmware(struct dewarp_composer_para *param);
bool check_dewarp_status(struct vframe_s *input_vf, int count, int work_mode, int index);
int v2d_init_dewarp_composer(struct dewarp_composer_para *param);
int v2d_uninit_dewarp_composer(struct dewarp_composer_para *param);
int v2d_config_dewarp_vframe(struct dewarp_vf_para_s *vframe_para,
	struct dewarp_common_para *common_para);
int v2d_dewarp_data_composer(struct dewarp_composer_para *param, bool is_tvp);

#endif
