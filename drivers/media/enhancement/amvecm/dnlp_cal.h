/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * drivers/amlogic/media/enhancement/amvecm/dnlp_cal.h
 *
 * Copyright (C) 2017 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
#ifndef __AM_DNLP_CAL_H
#define __AM_DNLP_CAL_H
#include <linux/amlogic/media/amvecm/dnlp_alg.h>

#define DNLP_VER	"v2025.02.26.1(minimum ko version: v2025.02.26.1)"

struct dnlp_parse_cmd_s {
	char *parse_string;
	unsigned int *value;
};

extern struct ve_ble_whe_param_s ble_whe_param_load;
extern struct ve_dnlp_curve_param_s dnlp_curve_param_load;
extern ulong ve_dnlp_reg[16];
extern ulong ve_dnlp_reg_v2[32];
extern ulong ve_dnlp_reg_def[16];
extern bool ve_en;
extern bool dnlp_insmod_ok; /*0:fail, 1:ok*/
extern unsigned int dnlp_dbg_flag;
extern int dnlp_rd_param;
extern char dnlp_rd_curve[400];
extern int hist_sel;
extern unsigned int dnlp_dbg_print;

int ve_dnlp_calculate_tgtx(struct vframe_s *vf, int vpp_index, struct vpp_hist_param_s *vp);
void ve_set_v3_dnlp(struct ve_dnlp_curve_param_s *p);
void ve_dnlp_calculate_lpf(void);
void ve_dnlp_calculate_reg(void);
void dnlp_alg_param_init(void);
void ai_dnlp_param_update(int value);
void ble_whe_param_update(struct ve_ble_whe_param_s *p);
void ve_dnlp_tgt_init(bool slt_en);
int dnlp_debug_store(char **parm);
#endif
#endif
