// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/module.h>

#include <trace/events/meson_atrace.h>
#include <linux/device/class.h>
#include <linux/sysfs.h>
#include <linux/amlogic/gki_module.h>

static u64 atrace_tag;

#define TAG_INFO(name) \
	{ #name,  KERNEL_ATRACE_TAG_ ## name }

struct {
	char *name;
	int tag;
} tag_info[] = {
	TAG_INFO(VIDEO),
	TAG_INFO(CODEC_MM),
	TAG_INFO(VDEC),
	TAG_INFO(TSYNC),
	TAG_INFO(IONVIDEO),
	TAG_INFO(AMLVIDEO),
	TAG_INFO(VIDEO_COMPOSER),
	TAG_INFO(V4L2),
	TAG_INFO(CPUFREQ),
	TAG_INFO(MSYNC),
	TAG_INFO(DEMUX),
	TAG_INFO(MEDIA_SYNC),
	TAG_INFO(DDR_BW),
	TAG_INFO(DIM),
	TAG_INFO(DRM),
	TAG_INFO(MEMORY),
	TAG_INFO(PIPE_KPI),
	{ NULL, 0 }
};

int get_atrace_tag_enabled(unsigned short tag)
{
	if (tag == KERNEL_ATRACE_TAG_ALL)
		return 1;
	return atrace_tag & (1 << tag);
}
EXPORT_SYMBOL(get_atrace_tag_enabled);

void set_atrace_tag_enabled(unsigned short tag, int enable)
{
	if (enable)
		atrace_tag |= (u64)1 << tag;
	else
		atrace_tag &= ~((u64)1 << tag);

	if (enable && tag == KERNEL_ATRACE_TAG_MEMORY)
		atrace_mem_init();
}
EXPORT_SYMBOL(set_atrace_tag_enabled);

#if IS_ENABLED(CONFIG_AMLOGIC_CLASS_DEBUG)
static ssize_t show_atrace_tag(struct kobject *kobj, struct kobj_attribute *attr,
				char *buf)
#else
static ssize_t show_atrace_tag(const struct class *class,
			const struct class_attribute *attr,
			char *buf)
#endif
{
	ssize_t size = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(tag_info) && tag_info[i].name; i++) {
		size += sprintf(buf + size, "%2d %s %s\n",
			tag_info[i].tag, tag_info[i].name,
			get_atrace_tag_enabled(tag_info[i].tag) ? "ON" : "OFF");
	}

	return size;
}

#if IS_ENABLED(CONFIG_AMLOGIC_CLASS_DEBUG)
static ssize_t store_atrace_tag(struct kobject *kobj, struct kobj_attribute *attr,
				const char *buf, size_t size)
#else
static ssize_t store_atrace_tag(const struct class *class,
			const struct class_attribute *attr,
			const char *buf, size_t size)
#endif
{
	unsigned int tag;
	ssize_t r;

	r = kstrtouint(buf, 0, &tag);

	if (r != 0)
		return -EINVAL;
	if (tag < KERNEL_ATRACE_TAG_MAX)
		set_atrace_tag_enabled(tag, !get_atrace_tag_enabled(tag));

	return size;
}

#if IS_ENABLED(CONFIG_AMLOGIC_CLASS_DEBUG)
static struct kobj_attribute atrace_tag_attr = __ATTR(atrace_tag, 0644,
						show_atrace_tag, store_atrace_tag);
static struct attribute *atrace_tag_attrs[] = {
	&atrace_tag_attr.attr,
	NULL,
};

static const struct attribute_group atrace_group = {
	.name = "atrace",
	.attrs = atrace_tag_attrs,
};
#else
static struct class_attribute debug_attrs[] = {
	__ATTR(atrace_tag, 0664, show_atrace_tag, store_atrace_tag),
	__ATTR_NULL
};

static struct attribute *debug_class_attrs[] = {
	&debug_attrs[0].attr,
	NULL
};

ATTRIBUTE_GROUPS(debug_class);

static struct class debug_class = {
		.name = "debug",
		.class_groups = debug_class_groups,
};
#endif

int meson_atrace_init(void)
{
	int r;

#if !IS_ENABLED(CONFIG_AMLOGIC_CLASS_DEBUG)
	r = class_register(&debug_class);

	if (r) {
		pr_err("debug class create fail.\n");
		return r;
	}
#else
	r = amlogic_class_debug_create_dir(&atrace_group, 1);
#endif
	return r;
}

static char *print_flags_type(unsigned int flags)
{
	if (flags & 1UL << KERNEL_ATRACE_COUNTER)
		return "C";
	else if (flags & 1UL << KERNEL_ATRACE_BEGIN)
		return "B";
	else if (flags & 1UL << KERNEL_ATRACE_END)
		return "E";
	else if (flags & 1UL << KERNEL_ATRACE_ASYNC_BEGIN)
		return "S";
	else if (flags & 1UL << KERNEL_ATRACE_ASYNC_END)
		return "F";

	return NULL;
}

static noinline void tracing_mark_write(char *buf)
{
	__trace_puts(_THIS_IP_, buf, strlen(buf));
}

void meson_atrace(int tag, const char *name, unsigned int flags,
	 unsigned long value)
{
	char buf[256];

	if (get_atrace_tag_enabled(tag)) {
		snprintf(buf, sizeof(buf), "%s|1|%s|%lu\n",
			print_flags_type(flags), name, value);
		tracing_mark_write(buf);
	}
}
EXPORT_SYMBOL(meson_atrace);

void meson_atrace_task(int tag, const char *name, unsigned int flags,
	 unsigned long value)
{
	char buf[256];

	if (get_atrace_tag_enabled(tag)) {
		snprintf(buf, sizeof(buf), "%s|%d|%s|%lu\n",
			print_flags_type(flags), current->tgid, name, value);
		tracing_mark_write(buf);
	}
}
EXPORT_SYMBOL(meson_atrace_task);

void __aml_trace_printk(unsigned long ip, const char *fmt, ...)
{
	int len = 0;
	char buf[256];
	va_list ap;

	va_start(ap, fmt);
	len = vscnprintf(buf, sizeof(buf), fmt, ap);
	if (len > 0)
		__trace_puts(ip, buf, strlen(buf));
	va_end(ap);
}
EXPORT_SYMBOL(__aml_trace_printk);
