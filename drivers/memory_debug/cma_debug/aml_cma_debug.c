// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/debugfs.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/kasan.h>
#include <linux/highmem.h>
#include <linux/uaccess.h>
#include <linux/amlogic/gki_module.h>
#include <linux/cma.h>
#include <linux/mm.h>
#include <cma.h>

static LIST_HEAD(cma_list);

struct cma_mem_list {
	struct hlist_head mem_head;
	struct list_head node;
	/* spinlock for protecting struct cma_mem */
	spinlock_t mem_head_lock;
	struct cma *cma;
};

struct cma_mem {
	struct hlist_node node;
	struct page *p;
	unsigned long n;
};

struct class_compat {
	struct kobject *kobj;
};

static struct class_compat *cma_debug_class;

static void cma_add_to_cma_mem_list(struct cma *cma, struct cma_mem *mem)
{
	struct cma_mem_list *list;

	list_for_each_entry(list, &cma_list, node) {
		if (list->cma == cma)
			break;
	}

	spin_lock(&list->mem_head_lock);
	hlist_add_head(&mem->node, &list->mem_head);
	spin_unlock(&list->mem_head_lock);
}

static struct cma_mem *cma_get_entry_from_list(struct cma *cma)
{
	struct cma_mem *mem = NULL;
	struct cma_mem_list *list;

	list_for_each_entry(list, &cma_list, node) {
		if (list->cma == cma)
			break;
	}
	spin_lock(&list->mem_head_lock);
	if (!hlist_empty(&list->mem_head)) {
		mem = hlist_entry(list->mem_head.first, struct cma_mem, node);
		hlist_del_init(&mem->node);
	}
	spin_unlock(&list->mem_head_lock);

	return mem;
}

static ssize_t alloc_store(struct kobject *kobj,
					struct kobj_attribute *attr, const char *buf, size_t count)
{
	struct cma *cma =  container_of(kobj, struct cma_kobject, kobj)->cma;
	struct cma_mem *mem;
	struct page *p;
	unsigned long nr_pages;

	int ret = kstrtoul(buf, 0, &nr_pages);

	if (ret)
		return ret;
	mem = kzalloc(sizeof(*mem), GFP_KERNEL);
	if (!mem)
		return -ENOMEM;
	pr_debug("alloc count = %lu\n", nr_pages);
	p = cma_alloc(cma, nr_pages, 0, false);
	if (!p) {
		kfree(mem);
		return -ENOMEM;
	}

	mem->p = p;
	mem->n = nr_pages;

	cma_add_to_cma_mem_list(cma, mem);

	return count;
}

static struct kobj_attribute alloc_attr = __ATTR_WO(alloc);

static ssize_t free_store(struct kobject *kobj,
					struct kobj_attribute *attr, const char *buf, size_t count)
{
	struct cma *cma =  container_of(kobj, struct cma_kobject, kobj)->cma;
	struct cma_mem *mem = NULL;
	unsigned long nr_pages;

	int ret = kstrtoul(buf, 0, &nr_pages);

	if (ret)
		return ret;
	pr_debug("free count = %lu", nr_pages);
	while (nr_pages) {
		mem = cma_get_entry_from_list(cma);
		if (!mem)
			return count;
		if (mem->n <= nr_pages) {
			cma_release(cma, mem->p, mem->n);
			nr_pages -= mem->n;
			kfree(mem);
		} else if (cma->order_per_bit == 0) {
			cma_release(cma, mem->p, nr_pages);
			mem->p += nr_pages;
			mem->n -= nr_pages;
			nr_pages = 0;
			cma_add_to_cma_mem_list(cma, mem);
		} else {
			pr_debug("cma: cannot release partial block when order_per_bit != 0\n");
			cma_add_to_cma_mem_list(cma, mem);
			break;
		}
	}

	return count;
}

static struct kobj_attribute free_attr = __ATTR_WO(free);

static ssize_t base_pfn_show(struct kobject *kobj,
					struct kobj_attribute *attr, char *buf)
{
	struct cma *cma =  container_of(kobj, struct cma_kobject, kobj)->cma;

	return sprintf(buf, "%lu\n", cma->base_pfn);
}

static struct kobj_attribute base_pfn_attr = __ATTR_RO(base_pfn);

static ssize_t count_show(struct kobject *kobj,
					struct kobj_attribute *attr, char *buf)
{
	struct cma *cma =  container_of(kobj, struct cma_kobject, kobj)->cma;

	return sprintf(buf, "%lu\n", cma->count);
}

static struct kobj_attribute count_attr = __ATTR_RO(count);

static ssize_t order_per_bit_show(struct kobject *kobj,
					struct kobj_attribute *attr, char *buf)
{
	struct cma *cma =  container_of(kobj, struct cma_kobject, kobj)->cma;

	return sprintf(buf, "%u\n", cma->order_per_bit);
}

static struct kobj_attribute order_per_bit_attr = __ATTR_RO(order_per_bit);

static ssize_t used_show(struct kobject *kobj,
					struct kobj_attribute *attr, char *buf)
{
	struct cma *cma =  container_of(kobj, struct cma_kobject, kobj)->cma;
	unsigned long used;

	spin_lock_irq(&cma->lock);
	used = bitmap_weight(cma->bitmap, (int)cma_bitmap_maxno(cma));
	spin_unlock_irq(&cma->lock);

	return sprintf(buf, "%llu\n", (u64)used << cma->order_per_bit);
}

static struct kobj_attribute used_attr = __ATTR_RO(used);

static ssize_t maxchunk_show(struct kobject *kobj,
					struct kobj_attribute *attr, char *buf)
{
	struct cma *cma =  container_of(kobj, struct cma_kobject, kobj)->cma;
	unsigned long maxchunk = 0;
	unsigned long start, end = 0;
	unsigned long bitmap_maxno = cma_bitmap_maxno(cma);

	spin_lock_irq(&cma->lock);
	for (;;) {
		start = find_next_zero_bit(cma->bitmap, bitmap_maxno, end);
		if (start >= bitmap_maxno)
			break;
		end = find_next_bit(cma->bitmap, bitmap_maxno, start);
		maxchunk = max(end - start, maxchunk);
	}
	spin_unlock_irq(&cma->lock);

	return sprintf(buf, "%llu\n", (u64)maxchunk << cma->order_per_bit);
}

static struct kobj_attribute maxchunk_attr = __ATTR_RO(maxchunk);

static ssize_t bitmap_show(struct kobject *kobj,
					struct kobj_attribute *attr, char *buf)
{
	struct cma *cma =  container_of(kobj, struct cma_kobject, kobj)->cma;
	size_t size = BITS_TO_LONGS(cma->count) * sizeof(unsigned long);
	unsigned long *bitmap = kmalloc(size, GFP_KERNEL);
	ssize_t len = 0;

	if (!bitmap)
		return -ENOMEM;

	spin_lock_irq(&cma->lock);
	memcpy(bitmap, cma->bitmap, size);
	spin_unlock_irq(&cma->lock);

	for (size_t i = 0; i < size / sizeof(unsigned long); i++) {
		if (i > 0)
			len += sprintf(buf + len, " ");
		len += sprintf(buf + len, "%016lx", bitmap[i]);
	}
	kfree(bitmap);

	return len;
}

static struct kobj_attribute bitmap_attr = __ATTR_RO(bitmap);

static void create_cma_mem_list(struct cma *cma)
{
	struct cma_mem_list *list = kzalloc(sizeof(*list), GFP_KERNEL);

	if (!list)
		return;

	INIT_HLIST_HEAD(&list->mem_head);
	spin_lock_init(&list->mem_head_lock);
	list->cma = cma;
	INIT_LIST_HEAD(&list->node);
	list_add_tail(&list->node, &cma_list);
}

static struct attribute *cma_attrs[] = {
	&alloc_attr.attr,
	&free_attr.attr,
	&base_pfn_attr.attr,
	&count_attr.attr,
	&order_per_bit_attr.attr,
	&used_attr.attr,
	&maxchunk_attr.attr,
	&bitmap_attr.attr,
	NULL,
};
ATTRIBUTE_GROUPS(cma);

static void cma_kobj_release(struct kobject *kobj)
{
	struct cma *cma = container_of(kobj, struct cma_kobject, kobj)->cma;
	struct cma_kobject *cma_kobj = cma->cma_kobj;

	kfree(cma_kobj);
	cma->cma_kobj = NULL;
}

static const struct kobj_type cma_ktype = {
	.release = cma_kobj_release,
	.sysfs_ops = &kobj_sysfs_ops,
	.default_groups = cma_groups,
};

static int aml_cma_debug_create(struct cma *cma, void *data)
{
	struct cma_kobject *cma_kobj;
	int err = 0;

	create_cma_mem_list(cma);
	cma_kobj = kzalloc(sizeof(*cma_kobj), GFP_KERNEL);
	if (!cma_kobj)
		return -ENOMEM;

	cma->cma_kobj = cma_kobj;
	cma_kobj->cma = cma;
	err = kobject_init_and_add(&cma_kobj->kobj, &cma_ktype,
					   cma_debug_class->kobj, "%s", cma->name);
	if (err)
		kobject_put(&cma_kobj->kobj);

	return err;
}

int __init cma_debug_init(void)
{
	int ret = 0;

	cma_debug_class = class_compat_register("cma_debug");
	if (!cma_debug_class) {
		pr_err("Failed to create cma debug class\n");
		return 0;
	}

	ret = cma_for_each_area(aml_cma_debug_create, NULL);
	if (ret) {
		pr_err("Failed to cma_for_each_area\n");
		return 0;
	}

	return 0;
}

void __exit cma_debug_exit(void)
{
}
