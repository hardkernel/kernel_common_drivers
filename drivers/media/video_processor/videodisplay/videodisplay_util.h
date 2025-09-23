/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef VD_UTIL_H
#define VD_UTIL_H

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/amlogic/media/vfm/vframe.h>

#define PRINT_ERROR		0X0
#define PRINT_QUEUE_STATUS	0X0001
#define PRINT_FENCE		0X0002
#define PRINT_PERFORMANCE	0X0004
#define PRINT_AXIS		0X0008
#define PRINT_INDEX_DISP	0X0010
#define PRINT_PATTERN	        0X0020
#define PRINT_OTHER		0X0040
#define PRINT_NN		0X0080
#define PRINT_DEWARP		0X0100
#define PRINT_VICP		0X0200
#define PRINT_DALTON		0X0400
#define PRINT_AIFACE		0X0800

#define MAX_VIDEODISPLAY_INSTANCE_NUM 3

enum videocom_source_type {
	DECODER_8BIT_NORMAL = 0,
	DECODER_8BIT_BOTTOM,
	DECODER_8BIT_TOP,
	DECODER_10BIT_NORMAL,
	DECODER_10BIT_BOTTOM,
	DECODER_10BIT_TOP,
	VDIN_8BIT_NORMAL,
	VDIN_10BIT_NORMAL,
};

enum com_buffer_used {
	USED_UNINITIAL = 0,
	USED_BY_GE2D,
	USED_BY_DEWARP,
};

struct dst_buf_t {
	int index;
	struct vframe_s frame;
	struct composer_info_t composer_info;
	enum com_buffer_used buf_used;
	bool dirty;
	ulong phy_addr;
	u32 buf_w;
	u32 buf_h;
	u32 buf_size;
	bool is_tvp;
	u32 dw_size;
	ulong afbc_head_addr;
	u32 afbc_head_size;
	ulong afbc_body_addr;
	u32 afbc_body_size;
	ulong afbc_table_addr;
	ulong afbc_table_handle;
	u32 afbc_table_size;
};

struct composer_vf_para {
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

struct pic_struct_t {
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
	struct pic_struct_t pic_info;
	int transform;
};

struct composer_output_para {
	struct pic_struct_t pic_info;
};

struct composer_common_para {
	struct composer_input_para input_para;
	struct composer_output_para output_para;
};

int set_print_flag(int index, u32 value);
u32 get_print_flag(int index);
int vd_print(int index, int debug_flag, const char *fmt, ...);

#endif
