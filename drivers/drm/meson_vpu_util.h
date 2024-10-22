/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_VPU_UTIL_H
#define __MESON_VPU_UTIL_H

#include <drm/drmP.h>
#include <linux/types.h>
#include <linux/amlogic/media/canvas/canvas.h>
#include <linux/amlogic/media/canvas/canvas_mgr.h>
#include <linux/amlogic/media/vpu/vpu.h>
#include <linux/amlogic/iomap.h>

#define LINE_THRESHOLD 90
#define WAIT_CNT_MAX 100

/*****drm reg access by rdma*****/
/*one item need two u32 = 8byte,total 512+1536 item is enough for vpu*/
#define RDMA_TABLE_ITEM_TOTAL      2048
#define MESON_VPU_RDMA_TABLE_SIZE (RDMA_TABLE_ITEM_TOTAL * 8)

/*osd internal channel*/
enum din_channel_e {
	DIN0 = 0,
	DIN1,
	DIN2,
	DIN3
};

struct osd_scope_s {
	u32 h_start;
	u32 h_end;
	u32 v_start;
	u32 v_end;
};

u32 meson_drm_read_reg(u32 addr);
void meson_drm_write_reg(u32 addr, u32 val);
void meson_drm_write_reg_bits(u32 addr, u32 val, u32 start, u32 len);
void set_video_enabled(u32 value, u32 index);

#ifdef CONFIG_AMLOGIC_MEDIA_RDMA
void meson_vpu_reg_handle_register(void *arg);
int meson_vpu_reg_vsync_config(u32 vpp_index);
#endif

#endif
