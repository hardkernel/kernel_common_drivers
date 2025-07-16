/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __ATV_DEMOD_EXT_H__
#define __ATV_DEMOD_EXT_H__

/* This module is atv demod interacts with other modules */
enum demod_set_mode {
	CH_IN_PLAY = 0,
	CH_IN_SEARCH = 1,
	DEMOD_IS_PAL = 2,
};

bool aml_fe_has_hook_up(void);
bool aml_fe_hook_call_get_fmt(int *fmt);
bool aml_fe_hook_call_set_fmt(int fmt);
bool aml_fe_hook_call_force_fmt(int *fmt);
bool aml_fe_hook_call_set_mode(int mode);
int aml_fe_hook_call_get_atv_status(void);

extern void atvdemod_power_switch(bool on);

#endif /* __ATV_DEMOD_EXT_H__ */
