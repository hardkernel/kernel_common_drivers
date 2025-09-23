/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __DI_POST_H__
#define __DI_POST_H__

void dpost_init(unsigned int ch);
void dpost_unreg(unsigned int ch);

void dpst_process(void);

//const char *dpst_state_name_get(enum EDI_PST_ST state);
//void dpst_dbg_f_trig(unsigned int cmd);
bool dpst_can_exit(unsigned int ch);
void dim_pst_poling(void);//

#endif	/*__DI_POST_H__*/
