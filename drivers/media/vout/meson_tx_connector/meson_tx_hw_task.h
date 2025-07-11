/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_TX_HW_TASK_H
#define __MESON_TX_HW_TASK_H

enum task_type {
	HPD_PLUGIN,
	HPD_PLUGOUT,
	HPD_IRQ,
	CORE_TASK,
	CEDST_TASK,
	RXSENSE_TASK,
	HDR_TASK,
	HDR_UNMUTE,
	MAX_TASK,
};

#endif
