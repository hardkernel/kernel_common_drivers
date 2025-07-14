/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __VDIN_CANVAS_H
#define __VDIN_CANVAS_H

#include <linux/sizes.h>
#include <linux/amlogic/media/vfm/vframe.h>

#define VDIN_CANVAS_MAX_WIDTH_UHD       4096
#define VDIN_CANVAS_MAX_WIDTH_HD        1920
#define VDIN_CANVAS_MAX_HEIGHT			2228
#define VDIN_YUV422_8BIT_PER_PIXEL_BYTE		2
#define VDIN_YUV422_10BIT_PER_PIXEL_BYTE	3
#define VDIN_YUV444_10BIT_PER_PIXEL_BYTE	4
#define VDIN_YUV444_8BIT_PER_PIXEL_BYTE		3
#define VDIN_RGBA8888_PER_PIXEL_BYTE		4
#define VDIN_MIN_SOURCE_BITDEPTH		8

#define VDIN_YUV444_MAX_CMA_WIDTH        4096
#define VDIN_YUV444_MAX_CMA_HEIGHT       2160

extern const unsigned int vdin_canvas_ids[VDIN_MAX_DEVS][VDIN_CANVAS_MAX_CNT];

void vdin_canvas_init(struct vdin_dev_s *devp);
void vdin_canvas_start_config(struct vdin_dev_s *devp);
void vdin_cal_canvas_w(struct vdin_dev_s *devp);
void vdin_canvas_auto_config(struct vdin_dev_s *devp);
void vdin_mem_memset(struct vdin_dev_s *devp);
unsigned int vdin_cma_alloc(struct vdin_dev_s *devp);
void vdin_cma_release(struct vdin_dev_s *devp);
void vdin_cma_malloc_mode(struct vdin_dev_s *devp);
void vdin_set_mem_protect(struct vdin_dev_s *devp, unsigned int protect);
#endif /* __VDIN_CANVAS_H */

