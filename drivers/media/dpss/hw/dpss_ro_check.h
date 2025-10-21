/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef DPSS_RO_CHECK_H
#define DPSS_RO_CHECK_H

void check_di_ro(int frm_idx, enum PRM_SRC_IDX src_idx);
void check_ne_ro(int frm_idx, enum PRM_SRC_IDX src_idx);
void check_hist_ro(int frm_idx, enum PRM_SRC_IDX src_idx);
void check_me_ro(int frm_idx);
void check_me_bbd_ro(int frm_idx);
void check_iplogo_ro(int frm_idx);
void check_vp_ro(int frm_idx);
void check_nrme_ro(int frm_idx, char *path);
void check_fd_ro(int frm_idx, char *path);
void check_dae_ro(int frm_idx, enum PRM_SRC_IDX dump_src_idx);
void check_mc_ro(int frm_idx, int frm_sel);
void check_mc_bbd_ro(int frm_idx, bool bb_frm_switch);
void check_nr_ro(int frm_idx, enum PRM_SRC_IDX dump_src_idx);
#endif//DPSS_RO_CHECK_H
