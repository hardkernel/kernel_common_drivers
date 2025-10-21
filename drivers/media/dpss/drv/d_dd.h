/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_DOLBYVISION
#include <linux/amlogic/media/amdolbyvision/dolby_vision.h>
#endif

#ifndef __DPSS_D_DD_H__
#define __DPSS_D_DD_H__

unsigned int dpss_sub_support_ddd(struct dpss_ch_s *pch, struct vframe_s *vfm);
unsigned int dpss_sub_chg_ddd(struct dpss_ch_s *pch, struct vframe_s *vfm);

#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_DOLBYVISION
void dpss_dd_init(struct vframe_s *vfm, struct dpss_info_s *dpss_info);
#endif
void dpss_dd_cfg_inp(struct vframe_s *vfm);
void dpss_dd_cfg_dpe(struct vframe_s *vfm);
void dpss_dd_pause(void); //for multi dpss, 2nd ch need disable dd
void dpss_dd_disable(void);
void dpss_dd_dae_update(struct vframe_s *vfm);	//called in irq
void dpss_dd_dpe_update(struct vframe_s *vfm);	//called in irq

#endif	/*__DPSS_D_DD_H__*/
