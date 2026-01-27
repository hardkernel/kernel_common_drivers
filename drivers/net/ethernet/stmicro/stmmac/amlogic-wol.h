/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __AMLOGIC_WOL_H_
#define __AMLOGIC_WOL_H_

#include <linux/types.h>
#include <linux/device.h>
#include <net/sock.h>
#include <linux/amlogic/aml_mbox.h>

struct amlogic_wol {
	struct device *dev;
	struct device *c_dev;
	struct mbox_chan *mbox;
	/* provided for sysfs and mbox */
	struct mutex lock;
	u32 wakeup_src;
	struct sock *sk;
	int mdnsoffload_result;
};

void amlogic_wol_enter(struct device *device);
bool amlogic_wol_exit(struct device *device);
bool amlogic_wol_wakeup_src_not_empty(struct device *device);
void amlogic_wol_wakeup_src_clr_all(struct device *device);
void amlogic_wol_wakeup_src_set_all(struct device *device);
int amlogic_wol_setup(struct device *device, struct mbox_chan *mbox);
void amlogic_wol_remove(struct device *device);

#endif /* __AMLOGIC_WOL_H_ */
