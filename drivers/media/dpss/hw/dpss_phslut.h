/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _PHS_LUT_H_
#define _PHS_LUT_H_

#include "dpss.h"

void dpss_phs_lut_ini(enum DPSS_FRC_RATIO frc_ratio_mode, enum DPSS_FILM_MODE film_mode);
void cfg_dpss_gmd_phs_lut(enum DPSS_FRC_RATIO frc_ratio_mode, u32 config_ini_en);
//void config_auto_mode_base_regs(struct PRM_DPSS_TBC *prm_tbc);
u32 config_inp_loop_tab(enum DPSS_FILM_MODE film_mode,
	u32 film_phs, u32 badedit_en);
void config_dae_loop_tab(enum DPSS_FRC_RATIO frc_ratio,
	enum DPSS_FILM_MODE film_mode, u32 cfg_at_inp_site);

void dpss_film_mode_switch(u32 det_filmmode_chg,
	enum DPSS_FILM_MODE inp_film_mode, u32 inp_film_phs, u32 badedit_en);

void config_inp_ofrm_flg(u32 inp_ofrm_vld);

void config_dae_ofrm_step_at_inp_site(u32 inp_ofrm_step);
void config_dae_ofrm_step_at_dae_site(u32 dae_ofrm_step);

#endif

