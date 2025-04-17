// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

//#define DEBUG
#include <linux/module.h>
#include <linux/amlogic/module_merge.h>
#include "main.h"

#if IS_ENABLED(CONFIG_AMLOGIC_DEBUG)
#include <linux/amlogic/gki_module.h>
#endif

static int __init i2c_main_init(void)
{
	pr_debug("### %s() start\n", __func__);
	call_sub_init(i2c_meson_init);
	call_sub_init(i2c_meson_t6w_init);
	pr_debug("### %s() end\n", __func__);

	return 0;
}

static void __exit i2c_main_exit(void)
{
	i2c_meson_exit();
	i2c_meson_t6w_exit();
}

module_init(i2c_main_init);
module_exit(i2c_main_exit);
MODULE_LICENSE("GPL v2");
