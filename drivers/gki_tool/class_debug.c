// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/ctype.h>
#include <linux/kallsyms.h>

#include <linux/amlogic/gki_module.h>
#include "gki_tool.h"

#if IS_ENABLED(CONFIG_AMLOGIC_CLASS_DEBUG)
static int debug_class_level = 1; /* 0 disabled 1 only debug 2 all*/
static int debug_class_setup(char *str)
{
	int val;

	if (kstrtoint(str, 10, &val)) {
		pr_info("invalid debug_class_level value: %s\n", str);
		return 1;
	}

	debug_class_level = val;

	return 1;
}
__setup("debug_class=", debug_class_setup);

struct class_compat {
	struct kobject *kobj;
};

static struct class_compat *aml_debug_class;

int class_debug_init(void)
{
	aml_debug_class = class_compat_register("debug");
	if (!aml_debug_class) {
		pr_err("Failed to create debug class\n");
		return -ENOMEM;
	}

	return 0;
}

/* debug_level - 1 always creat
 *	       - 2 only debug
 */
int amlogic_class_debug_create_dir(const struct attribute_group *group, int debug_level)
{
	int ret;

	if (debug_level > debug_class_level) {
		pr_info("class debug %s group skip\n", group->name);
		return 0;
	}

	if (!group || !aml_debug_class) {
		pr_err("%s param is null\n", __func__);
		return -EINVAL;
	}

	ret = sysfs_create_group(aml_debug_class->kobj, group);
	if (ret) {
		pr_err("Failed to create %s group\n", group->name);
		return -ret;
	}

	return 0;
}
#else
int class_debug_init(void)
{
	return 0;
}

int amlogic_class_debug_create_dir(const struct attribute_group *group, int debug_level)
{
	return 0;
}
#endif
EXPORT_SYMBOL(amlogic_class_debug_create_dir);
