/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __WATCH_KEY_PRIVATE_
#define __WATCH_KEY_PRIVATE_

#include <linux/types.h>
#include <linux/device.h>

struct amlogic_watchkey {
	struct device *dev;
	const char *name;
	int id;
	unsigned int type;
	unsigned int code;
	bool report_event;
	bool fixed;
};

int __of_watchkey_parse(struct amlogic_watchkey *wk, struct device_node *np);

#endif /* __WATCH_KEY_PRIVATE_ */
