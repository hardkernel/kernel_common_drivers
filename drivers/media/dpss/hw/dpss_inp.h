/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _DPSS_INP_H_
#define _DPSS_INP_H_

#include "dpss_param.h"
#include "dpss_intf.h"

void cfg_dpss_inp(struct PRM_DPSS_TOP *prm_top, struct PRM_DPSS_INP prm_inp);
void inp_chg2done_triggle(void);
void cfg_inp_triggle(bool triggle_sel);
void cfg_inp_raddr(void);
#endif
