/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPSS_D_PQ_H__
#define __DPSS_D_PQ_H__

void pq_int(struct dpss_ch_s *pch, struct dpss_sub_vf_s *vfs);
void pq_update_ro_dae(u32 buf_id, u8 ch);
void pq_update_ro_dpe(u32 buf_id, u8 ch);
void pq_update_ro_dblk_h(u32 buf_id, u8 ch);
void pq_update_ro_dblk_v(u32 buf_id, u8 ch);
void pq_update_ro_me(u32 buf_id, u8 ch);
void pq_update_ro_input(u32 buf_id, u8 ch);
void pq_update_ro_dae_pd(u32 buf_id, u8 ch);
enum vframe_signal_fmt_e dpss_sfmt(struct vframe_s *vf);

struct di_parm_s *dpss_pq_data(u8 ch);
void dpss_me_size_update(u8 ch);
void dpss_me_ro_data_update(void);
void dpss_input_ro_data_update(unsigned int inp_ofrm_vld);
ssize_t dpss_pq_en_show(const struct class *device,
		const struct class_attribute *attr, char *buf);

ssize_t dpss_pq_en_store(const struct class *class,
		const struct class_attribute *attr,
		const char *buf, size_t count);

extern struct di_parm_s dinr_pq_data[];

#endif	/*__DPSS_D_PQ_H__*/
