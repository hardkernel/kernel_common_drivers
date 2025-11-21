/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <linux/dma-buf.h>
#include <linux/amlogic/meson_uvm_allocator.h>
#include <linux/amlogic/meson_uvm_ge2d_utils.h>
#include <linux/amlogic/media/vfm/vframe.h>
#include <linux/amlogic/media/video_sink/v4lvideo_ext.h>

#ifndef _MESON_UVM_INTERFACE_H_
#define _MESON_UVM_INTERFACE_H_

void AMLOGIC_V4LVIDEO_data_copy(struct v4l_data_t *v4l_data,
			       struct dma_buf *dmabuf, u32 align);
typedef void (*AMLOGIC_V4LVIDEO_data_copy_fun_t)(struct v4l_data_t *,
						struct dma_buf *, u32);
void register_amlogic_v4lvideo_data_copy_fun(AMLOGIC_V4LVIDEO_data_copy_fun_t fn);
void unregister_amlogic_v4lvideo_data_copy_fun(void);

int AMLOGIC_ATTACH_uvm_info(struct dma_buf *dmabuf, int fd, int type, char *buf);
typedef int (*AMLOGIC_ATTACH_uvm_info_fun_t)(struct dma_buf *, int, int, char *);
int register_amlogic_attach_uvm_info_fun(AMLOGIC_ATTACH_uvm_info_fun_t fn);
int unregister_amlogic_attach_uvm_info_fun(void);

int AMLOGIC_GET_UVM_video_info(const int fd, struct uvm_fd_info *info);
typedef int (*AMLOGIC_GET_UVM_video_info_fun_t)(const int, struct uvm_fd_info *);
int register_amlogic_get_uvm_video_info_fun(AMLOGIC_GET_UVM_video_info_fun_t fn);
int unregister_amlogic_get_uvm_video_info_fun(void);

int AMLOGIC_UVM_ge2d_init(struct uvm_ge2d **ge2d_handle, int mode);
typedef int(*AMLOGIC_UVM_ge2d_init_fun_t)(struct uvm_ge2d **, int);
int register_amlogic_uvm_ge2d_init_fun(AMLOGIC_UVM_ge2d_init_fun_t fn);
int unregister_amlogic_uvm_ge2d_init_fun(void);

int AMLOGIC_UVM_ge2d_copy_data(struct uvm_ge2d *ge2d, struct uvm_ge2d_info *ge2d_info);
typedef int(*AMLOGIC_UVM_ge2d_copy_data_fun_t)(struct uvm_ge2d *, struct uvm_ge2d_info *);
int register_amlogic_uvm_ge2d_copy_data_fun(AMLOGIC_UVM_ge2d_copy_data_fun_t fn);
int unregister_amlogic_uvm_ge2d_copy_data_fun(void);

void AMLOGIC_UVM_ge2d_destroy(struct uvm_ge2d *ge2d);
typedef void(*AMLOGIC_UVM_ge2d_destroy_fun_t)(struct uvm_ge2d *);
void register_amlogic_uvm_ge2d_destroy_fun(AMLOGIC_UVM_ge2d_destroy_fun_t fn);
void unregister_amlogic_uvm_ge2d_destroy_fun(void);

#endif
