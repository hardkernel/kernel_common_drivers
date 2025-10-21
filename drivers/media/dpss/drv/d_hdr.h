/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPSS_D_HDR_H__
#define __DPSS_D_HDR_H__

unsigned int dpss_sub_support_hdr(struct dpss_ch_s *pch, struct vframe_s *vfm);
unsigned int dpss_sub_chg_hdr(struct dpss_ch_s *pch, struct vframe_s *vfm);

void dpss_hdr_init(struct vframe_s *vfm);
void dpss_hdr_sw(bool sw, struct vframe_s *vfm);
void dpss_hdr_proc(struct vframe_s *vfm);

void dpss_hdr_pause(void); //for multi dpss, 2nd ch need disable hdr

void dpss_hdr_disable(void);

#endif	/*__DPSS_D_HDR_H__*/
