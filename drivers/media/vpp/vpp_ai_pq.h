/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#ifndef __VPP_AI_PQ_H__
#define __VPP_AI_PQ_H__

int vpp_ai_pq_init(void);
struct vpp_single_scene_s *vpp_ai_pq_get_scene_proc(enum vpp_detect_scene_e index);

#endif
#endif

