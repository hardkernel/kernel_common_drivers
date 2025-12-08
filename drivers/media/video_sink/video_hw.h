/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef VIDEO_HW_HH
#define VIDEO_HW_HH

#define FGRAIN_TBL_SIZE  (498 * 16)

#ifdef CONFIG_AMLOGIC_MEDIA_SECURITY
void vpp_secure_cb(u32 arg);
#endif
void int_vpu_delay_work(void);
void vd1_set_go_field(void);
void get_video_mute_info(void);
int set_video_mute_info(u32 owner, bool on, bool use_vcbus);
void check_video_mute(void);
bool get_video_mute_val(u32 owner);
void vppx_vdx_mux_set(void);
int get_video_reg_table(u32 *check_item);
#ifdef CONFIG_AMLOGIC_ZAPPER_CUT
int get_video_reg_table_cut(u32 *check_item);
#endif
void clear_vpu_venc_error(void);
u32 get_vpu_venc_error_status(void);
#endif
