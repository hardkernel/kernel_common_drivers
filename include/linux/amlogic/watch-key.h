/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __WATCH_KEY_
#define __WATCH_KEY_

#include <linux/types.h>
#include <linux/device.h>
#include <linux/input.h>

struct amlogic_watchkey;

#if IS_ENABLED(CONFIG_AMLOGIC_WATCHKEY)
void of_amlogic_watchkey_put(struct amlogic_watchkey *wk);
struct amlogic_watchkey *of_amlogic_watchkey_get(struct device *dev);
struct amlogic_watchkey *devm_of_amlogic_watchkey_get(struct device *dev);
int amlogic_watchkey_state(struct amlogic_watchkey *wk);
int amlogic_watchkey_enable(struct amlogic_watchkey *wk);
int amlogic_watchkey_disable(struct amlogic_watchkey *wk);
int amlogic_watchkey_dump(struct amlogic_watchkey *wk, u32 *d0, u32 *d1, u32 *d2);
int amlogic_watchkey_test(struct amlogic_watchkey *wk);
void amlogic_watchkey_report(struct amlogic_watchkey *wk, unsigned int type, unsigned int code);
void amlogic_watchkey_try_fixed(struct amlogic_watchkey *wk, struct input_dev *input);
#else
static inline void of_amlogic_watchkey_put(struct amlogic_watchkey *wk) { }

static inline struct amlogic_watchkey *of_amlogic_watchkey_get(struct device *dev)
{
	return ERR_PTR(-ENODEV);
}

static inline struct amlogic_watchkey *devm_of_amlogic_watchkey_get(struct device *dev)
{
	return ERR_PTR(-ENODEV);
}

static inline int amlogic_watchkey_state(struct amlogic_watchkey *wk)
{
	return -ENODEV;
}

static inline int amlogic_watchkey_enable(struct amlogic_watchkey *wk)
{
	return -ENODEV;
}

static inline int amlogic_watchkey_disable(struct amlogic_watchkey *wk)
{
	return -ENODEV;
}

static inline int amlogic_watchkey_dump(struct amlogic_watchkey *wk, u32 *d0, u32 *d1, u32 *d2)
{
	return -ENODEV;
}

static inline int amlogic_watchkey_test(struct amlogic_watchkey *wk)
{
	return -ENODEV;
}

static inline void amlogic_watchkey_report(struct amlogic_watchkey *wk, unsigned int type,
					   unsigned int code) { }

static inline void amlogic_watchkey_try_fixed(struct amlogic_watchkey *wk,
					      struct input_dev *input) { }

#endif /* CONFIG_AMLOGIC_WATCHKEY */

#endif /* __WATCH_KEY_ */
