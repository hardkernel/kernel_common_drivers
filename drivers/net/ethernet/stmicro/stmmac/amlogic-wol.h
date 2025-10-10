/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __AMLOGIC_WOL_H_
#define __AMLOGIC_WOL_H_

#include <linux/types.h>
#include <linux/device.h>
#include <linux/amlogic/aml_mbox.h>

void amlogic_wol_enter(void);
bool amlogic_wol_exit(void);
bool amlogic_wol_wakeup_src_not_empty(void);
void amlogic_wol_wakeup_src_clr_all(void);
void amlogic_wol_wakeup_src_set_all(void);
void amlogic_wol_setup(struct device *dev, struct mbox_chan *mbox);
void amlogic_wol_remove(void);

#endif /* __AMLOGIC_WOL_H_ */
