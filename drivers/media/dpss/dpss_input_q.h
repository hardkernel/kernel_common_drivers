/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPSS_INPUT_Q_H__
#define __DPSS_INPUT_Q_H__

void dpss_input_q_init(struct dpss_input_q *q);
u32 dpss_input_q_free_count(struct dpss_input_q *q);
void dpss_input_q_in(struct dpss_input_q *q, struct vframe_s *vfm);
void dpss_input_q_update_status(struct dpss_input_q *q, u32 inp_ofrm_vld, bool is_i);
void dpss_input_q_update_dpe_status(struct PRM_DPSS_TOP *top, u32 addr_y);
int dpss_input_get_buf_index(struct dpss_input_q *q);
u32 dpss_input_get_drop_count(struct dpss_input_q *q);
u32 dpss_input_q_out_1(struct dpss_input_q *q);
void dpss_input_q_out_head_key(struct dpss_input_q *q);
void dpss_input_q_out_head(struct dpss_input_q *q);
#endif

