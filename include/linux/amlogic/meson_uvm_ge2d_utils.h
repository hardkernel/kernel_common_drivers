/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_UVM_GE2D_UTILS_H__
#define __MESON_UVM_GE2D_UTILS_H__

#include <linux/amlogic/media/vfm/vframe.h>

/* define ge2d work mode. */
#define UVM_GE2D_MODE_CONVERT_NV12		BIT(0)
#define UVM_GE2D_MODE_CONVERT_NV21		BIT(1)
#define UVM_GE2D_MODE_CONVERT_LE		BIT(2)
#define UVM_GE2D_MODE_CONVERT_BE		BIT(3)
#define UVM_GE2D_MODE_SEPARATE_FIELD		BIT(4)
#define UVM_GE2D_MODE_422_TO_420		BIT(5)

struct uvm_canvas_res {
	int cid;
	u8 name[32];
};

struct uvm_canvas_cache {
	int ref;
	struct uvm_canvas_res res[6];
	struct mutex lock; /* cache mutex */
};

struct uvm_ge2d {
	u32 work_mode; /* enum ge2d_work_mode */
	struct ge2d_context_s *ge2d_context; /* handle of GE2D */
	struct uvm_canvas_cache cache;
};

struct uvm_ge2d_info {
	struct vframe_s *dst_vf;
	u32 src_canvasAddr;
	u32 src_yuv_width;
	u32 src_yuv_height;
	u32 dst_yuv_width;
	u32 dst_yuv_height;
	struct canvas_config_s src_canvas_config[3];
	struct canvas_config_s dst_canvas_config[3];
};

int uvm_ge2d_init(struct uvm_ge2d **ge2d_handle, int mode);
int uvm_ge2d_copy_data(struct uvm_ge2d *ge2d, struct uvm_ge2d_info *ge2d_info);
void uvm_ge2d_destroy(struct uvm_ge2d *ge2d);

#endif

