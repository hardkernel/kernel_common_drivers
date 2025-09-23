/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef VIDEO_DISPLAY_H
#define VIDEO_DISPLAY_H

#include "video_composer.h"

/* disable video_display mode */
#define VIDEO_DISPLAY_ENABLE_NONE    0
#define VIDEO_DISPLAY_ENABLE_NORMAL  1

#define MAX_VIDEO_COMPOSER_INSTANCE_NUM 3

extern u32 vd_pulldown_level;
extern u32 vd_max_hold_count;
extern u32 vd_set_frame_delay[];
extern struct vframe_s *current_display_vf;
extern u32 vpp_drop_count;
extern u32 use_low_latency;
extern u32 new_afr_pulldown;
extern u32 vd_test_fps[MAX_VD_LAYERS];
extern u64 vd_test_fps_val[MAX_VD_LAYERS];
extern u64 vd_test_vsync_val[MAX_VD_LAYERS];
extern struct video_composer_port_s ports[];
extern u32 next_vsync_wakeup_vpp_to_get;
extern u32 allow_vpp_to_get;
extern struct timespec64 isr_spec_time;
extern u64 next_isr_spec_time;
#ifdef CONFIG_AMLOGIC_MEDIA_PROXY
extern struct mediaproxy_info_t mediaproxy_display_info[];
#endif

struct video_display_frame_info_t {
	struct dma_buf *dmabuf;
	struct dma_fence *input_fence;
	struct dma_fence *release_fence;
	u64 phy_addr[2];
	u32 byte_stride;
	u32 buffer_w;
	u32 buffer_h;
	u32 dst_x;
	u32 dst_y;
	u32 dst_w;
	u32 dst_h;
	u32 crop_x;
	u32 crop_y;
	u32 crop_w;
	u32 crop_h;
	u32 zorder;
	u32 signal_fmt;
	u32 type;
	u32 bitdepth;
	u32 rotation;
	u32 reserved[10];
};

struct mbp_buffer_info_t {
	u32 pool_handle;
	u32 user_id;
	u64 phys_addr[3];
};

struct mbd_video_frame_info_t {
	struct mbp_buffer_info_t *buffer_info;
	u32 type;
	u32 stride[3];
	u32 buffer_w;
	u32 buffer_h;
	u32 dst_x;
	u32 dst_y;
	u32 dst_w;
	u32 dst_h;
	u32 crop_x;
	u32 crop_y;
	u32 crop_w;
	u32 crop_h;
	u32 zorder;
	u32 timeref;
	u64 pts;
	void (*lock_buffer_cb)(void *arg);
	void (*unlock_buffer_cb)(void *arg);
	u32 reserved[10];
};

int video_display_create_path(struct composer_dev *dev);
int video_display_release_path(struct composer_dev *dev);
void vd_prepare_data_q_put(struct composer_dev *dev,
			struct vd_prepare_s *vd_prepare);
struct vd_prepare_s *vd_prepare_data_q_get(struct composer_dev *dev);
int vd_render_index_get(struct composer_dev *dev);
void video_display_para_reset(int layer_index);
int find_nearest_duration(struct composer_dev *dev, int duration_val);
bool check_frc_n2m_status(void);
int is_video_process_in_thread(void);
void vpp_lowlatency_wakeup(void);

#endif
