/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __VDIN_VFCE_H__
#define __VDIN_VFCE_H__
#include "vdin_drv.h"
#include "vdin_ctl.h"

#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
void vdin_vfce_write_mif_or_afbce_init(struct vdin_dev_s *devp);
#else
static inline void vdin_vfce_write_mif_or_afbce_init(struct vdin_dev_s *devp)
{
}
#endif
void vdin_vfce_write_mif_or_afbce(struct vdin_dev_s *devp, enum vdin_output_mif_e sel);
void vdin_vfce_afbce_update(struct vdin_dev_s *devp);
void vdin_vfce_afbce_config(struct vdin_dev_s *devp);
void vdin_vfce_afbce_maptable_init(struct vdin_dev_s *devp);
void vdin_vfce_afbce_set_next_frame(struct vdin_dev_s *devp,
			       unsigned int rdma_enable, struct vf_entry *vfe);
void vdin_vfce_clear_write_down_flag(struct vdin_dev_s *devp);
int vdin_vfce_read_write_down_flag(struct vdin_dev_s *devp);
void vdin_vfce_afbce_soft_reset(struct vdin_dev_s *devp);
void vdin_vfce_afbce_mode_init(struct vdin_dev_s *devp);
void vdin_vfce_afbce_mode_update(struct vdin_dev_s *devp);
void vdin_vfce_pause_write(struct vdin_dev_s *devp, unsigned int rdma_enable, bool pause_en);
void vdin_vfce_set_next_frame(struct vdin_dev_s *devp,
	unsigned int rdma_enable, struct vf_entry *vfe);
void vdin_vfce_config(struct vdin_dev_s *devp);
void vdin_vfce_update(struct vdin_dev_s *devp);
#endif
