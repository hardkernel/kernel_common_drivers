/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __GKI_MODULE_AMLOGIC_H
#define __GKI_MODULE_AMLOGIC_H

#ifdef CONFIG_AMLOGIC_GKI_TOOL
#define GKI_MODULE_SETUP_MAGIC1 0x014589cd
#define GKI_MODULE_SETUP_MAGIC2 0x2367abef

struct gki_module_setup_struct {
	/* must be first */
	int magic1;
	int magic2;

	char *str;
	void *fn;
	int early;
};

struct cmd_param_val {
	char *param;
	char *val;
};

extern struct cmd_param_val *cpv;
extern int cpv_count;

#define __setup_gki_module(str, fn, early)			\
	struct gki_module_setup_struct __gki_setup_##fn =        \
		   {GKI_MODULE_SETUP_MAGIC1, GKI_MODULE_SETUP_MAGIC2,    \
		   str, fn, early};                                     \
	EXPORT_SYMBOL(__gki_setup_##fn)

#ifdef MODULE

#undef __setup
#undef __setup_param
#undef early_param

#define __setup(str, fn)						\
		__setup_gki_module(str, fn, 0)

#define early_param(str, fn)						\
		__setup_gki_module(str, fn, 1)
void __module_init_hook(struct module *m);

#define module_init_hook(initfn)      \
	int __init init_module(void) \
	{       \
		__module_init_hook(THIS_MODULE); \
		return initfn();     \
	}	\
	___ADDRESSABLE(init_module, __initdata);

#undef early_initcall
#undef core_initcall
#undef core_initcall_sync
#undef postcore_initcall
#undef postcore_initcall_sync
#undef arch_initcall
#undef subsys_initcall
#undef subsys_initcall_sync
#undef fs_initcall
#undef fs_initcall_sync
#undef rootfs_initcall
#undef device_initcall
#undef device_initcall_sync
#undef late_initcall
#undef late_initcall_sync
#undef console_initcall
#undef security_initcall

#define early_initcall(fn)		module_init_hook(fn)
#define core_initcall(fn)		module_init_hook(fn)
#define core_initcall_sync(fn)		module_init_hook(fn)
#define postcore_initcall(fn)		module_init_hook(fn)
#define postcore_initcall_sync(fn)	module_init_hook(fn)
#define arch_initcall(fn)		module_init_hook(fn)
#define subsys_initcall(fn)		module_init_hook(fn)
#define subsys_initcall_sync(fn)	module_init_hook(fn)
#define fs_initcall(fn)			module_init_hook(fn)
#define fs_initcall_sync(fn)		module_init_hook(fn)
#define rootfs_initcall(fn)		module_init_hook(fn)
#define device_initcall(fn)		module_init_hook(fn)
#define device_initcall_sync(fn)	module_init_hook(fn)
#define late_initcall(fn)		module_init_hook(fn)
#define late_initcall_sync(fn)		module_init_hook(fn)
#define console_initcall(fn)		module_init_hook(fn)
#define security_initcall(fn)		module_init_hook(fn)

#undef module_init
#define module_init(fn)			module_init_hook(fn)

#endif //MODULE

enum param_type {
	TYPE_BOOL,
	TYPE_INT,
	TYPE_UINT,
	TYPE_LONG,
	TYPE_ULONG,
	TYPE_LLONG,
	TYPE_ULLONG,
};

#define PARAM_BOOL(name)		{#name "=", &(name), TYPE_BOOL, 1}
#define PARAM_INT(name)			{#name "=", &(name), TYPE_INT, 1}
#define PARAM_UINT(name)		{#name "=", &(name), TYPE_UINT, 1}
#define PARAM_LONG(name)		{#name "=", &(name), TYPE_LONG, 1}
#define PARAM_ULONG(name)		{#name "=", &(name), TYPE_ULONG, 1}
#define PARAM_LLONG(name)		{#name "=", &(name), TYPE_LLONG, 1}
#define PARAM_ULLONG(name)		{#name "=", &(name), TYPE_ULLONG, 1}
#define PARAM_BOOL_ARRAY(name)		{#name "=", (name), TYPE_BOOL, ARRAY_SIZE(name)}
#define PARAM_INT_ARRAY(name)		{#name "=", (name), TYPE_INT, ARRAY_SIZE(name)}
#define PARAM_UINT_ARRAY(name)		{#name "=", (name), TYPE_UINT, ARRAY_SIZE(name)}
#define PARAM_LONG_ARRAY(name)		{#name "=", (name), TYPE_LONG, ARRAY_SIZE(name)}
#define PARAM_ULONG_ARRAY(name)		{#name "=", (name), TYPE_ULONG, ARRAY_SIZE(name)}
#define PARAM_LLONG_ARRAY(name)		{#name "=", (name), TYPE_LLONG, ARRAY_SIZE(name)}
#define PARAM_ULLONG_ARRAY(name)	{#name "=", (name), TYPE_ULLONG, ARRAY_SIZE(name)}

struct param_entry {
	char *name;
	void *value;
	enum param_type type;
	int array_size;
};

extern struct kernel_param_ops key_value_param_ops;

int amlogic_class_debug_create_dir(const struct attribute_group *group, int debug_level);
#endif //CONFIG_AMLOGIC_GKI_TOOL
#endif //__GKI_MODULE_AMLOGIC_H
