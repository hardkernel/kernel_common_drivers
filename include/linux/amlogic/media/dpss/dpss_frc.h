/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPSS_FRC_H__
#define __DPSS_FRC_H__

enum EPVPP_DISPLAY_FRC_MODE {
	EPVPP_DISPLAY_MODE_FRC_BYPASS = 0,
	EPVPP_DISPLAY_MODE_FRC,
};

enum EPVPP_FRC_API_MODE {
	EPVPP_API_MODE_FRC_NONE,
	EPVPP_API_MODE_FRC,
	EPVPP_MEM_MODE_FRC_MAX
};

struct frc_win_s {
	unsigned int x_size;
	unsigned int y_size;
	unsigned int x_st;
	unsigned int y_st;
	unsigned int x_end;
	unsigned int y_end;
	unsigned int orig_w;
	unsigned int orig_h;
	unsigned int x_check_sum;
	unsigned int y_check_sum;
};

struct frc_screen_vinfo {
	unsigned int vtotal;
	unsigned int htotal;
	unsigned int width;
	unsigned int height;
	unsigned int frequency;
	unsigned int x_d_st;
	unsigned int y_d_st;
	unsigned int x_d_end;
	unsigned int y_d_end;
	unsigned int x_d_size;
	unsigned int y_d_size;
};

struct pvpp_dis_frc_para_in_s {
	enum EPVPP_DISPLAY_FRC_MODE dmode;
	bool unreg_bypass; //for unreg bypass: set 1; other, set 0;
	struct frc_win_s win;
	struct frc_screen_vinfo vinfo;
	unsigned int follow_hold_line;
	bool plink_reverse;//reverse mirror flag at pre/post vpp link
	unsigned int plink_hv_mirror;//0x1,H-mirror;0x2,V-mirror;
	enum EPVPP_FRC_API_MODE link_mode;
};

int pvpp_display_frc(struct vframe_s *vfm,
				struct pvpp_dis_frc_para_in_s *in_para,
				void *out_para);
int frclink_vpp_check_vf(struct vframe_s *vfm);
bool frclink_vpp_check_act(void);
int pvpp_sw_frc(bool on);
void frc_disable_plink_notify(bool async);
void frc_plink_state_changed_notify(void);
void irq_display(void);

int dpss_frc_get_video_latency(void);
int dpss_frc_get_video_latency_for_gd(void);
int dpss_frc_get_video_latency_for_gd1(void);

//vpp rdma
u32 VSYNC_RD_VIDEO_TABLE_REG(u32 adr);
int VSYNC_WR_VIDEO_TABLE_REG(u32 adr, u32 val);
int VSYNC_WR_VIDEO_TABLE_REG_BITS(u32 adr, u32 val, u32 start, u32 len);

#endif	/*__DPSS_FRC_H__*/
