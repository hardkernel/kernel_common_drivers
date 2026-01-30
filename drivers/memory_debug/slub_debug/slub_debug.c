// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/mm.h>
#include <linux/swap.h> /* mm_account_reclaimed_pages() */
#include <linux/module.h>
#include <linux/bit_spinlock.h>
#include <linux/swab.h>
#include <linux/bitops.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/memory.h>
#include <linux/math64.h>
#include <linux/fault-inject.h>
#include <linux/kmemleak.h>
#include <linux/stacktrace.h>
#include <linux/prefetch.h>
#include <linux/memcontrol.h>
#include <linux/random.h>
#include <kunit/test.h>
#include <kunit/test-bug.h>
#include <linux/sort.h>
#include <linux/amlogic/gki_module.h>
#include <trace/events/kmem.h>
#include <trace/hooks/mm.h>
#include <slab.h>

LIST_HEAD(aml_slab_caches);

struct location {
	depot_stack_handle_t handle;
	unsigned long count;
	unsigned long addr;
	unsigned long waste;
	long long sum_time;
	long min_time;
	long max_time;
	long min_pid;
	long max_pid;
	DECLARE_BITMAP(cpus, NR_CPUS);
	nodemask_t nodes;
};

struct loc_track {
	unsigned long max;
	unsigned long count;
	struct location *loc;
	loff_t idx;
};

struct kmem_cache_node {
	spinlock_t list_lock;
	unsigned long nr_partial;
	struct list_head partial;
#ifdef CONFIG_SLUB_DEBUG
	atomic_long_t nr_slabs;
	atomic_long_t total_objects;
	struct list_head full;
#endif
};

static bool aml_debug_enabled;

//static struct dentry *slab_debugfs_root;
static struct proc_dir_entry *slab_debugfs_root;

#define OO_SHIFT	16
#define OO_MASK		((1 << OO_SHIFT) - 1)

static inline struct kmem_cache_node *get_node(struct kmem_cache *s, int node)
{
	return s->node[node];
}

static inline bool aml_kmem_cache_debug_flags(struct kmem_cache *s, slab_flags_t flags)
{
	if (IS_ENABLED(CONFIG_SLUB_DEBUG))
		VM_WARN_ON_ONCE(!(flags & SLAB_DEBUG_FLAGS));

	return s->flags & flags;
}

static void *aml_fixup_red_left(struct kmem_cache *s, void *p)
{
	if (aml_kmem_cache_debug_flags(s, SLAB_RED_ZONE))
		p += s->red_left_pad;

	return p;
}

/*
 * Iterator over all nodes. The body will be executed for each node that has
 * a kmem_cache_node structure allocated (which is true for all online nodes)
 */
#define for_each_kmem_cache_node(__s, __node, __n) \
	for (__node = 0; __node < nr_node_ids; __node++) \
		 if ((__n = get_node(__s, __node)))

/* Loop over all objects in a slab */
#define for_each_object(__p, __s, __addr, __objects) \
	for (__p = aml_fixup_red_left(__s, __addr); \
		__p < (__addr) + (__objects) * (__s)->size; \
		__p += (__s)->size)

static inline freeptr_t freelist_ptr_encode(const struct kmem_cache *s,
					    void *ptr, unsigned long ptr_addr)
{
	unsigned long encoded;

#ifdef CONFIG_SLAB_FREELIST_HARDENED
	encoded = (unsigned long)ptr ^ s->random ^ swab(ptr_addr);
#else
	encoded = (unsigned long)ptr;
#endif
	return (freeptr_t){.v = encoded};
}

static inline void *freelist_ptr_decode(const struct kmem_cache *s,
					freeptr_t ptr, unsigned long ptr_addr)
{
	void *decoded;

#ifdef CONFIG_SLAB_FREELIST_HARDENED
	decoded = (void *)(ptr.v ^ s->random ^ swab(ptr_addr));
#else
	decoded = (void *)ptr.v;
#endif
	return decoded;
}

static inline unsigned int oo_objects(struct kmem_cache_order_objects x)
{
	return x.x & OO_MASK;
}

/*
 * See comment in calculate_sizes().
 */
static inline bool freeptr_outside_object(struct kmem_cache *s)
{
	return s->offset >= s->inuse;
}

/*
 * Return offset of the end of info block which is inuse + free pointer if
 * not overlapping with object.
 */
static inline unsigned int get_info_end(struct kmem_cache *s)
{
	if (freeptr_outside_object(s))
		return s->inuse + sizeof(void *);
	else
		return s->inuse;
}

static inline bool slub_debug_orig_size(struct kmem_cache *s)
{
	return (aml_kmem_cache_debug_flags(s, SLAB_STORE_USER) &&
			(s->flags & SLAB_KMALLOC));
}

static inline unsigned int get_orig_size(struct kmem_cache *s, void *object)
{
	void *p = kasan_reset_tag(object);

	if (!slub_debug_orig_size(s))
		return s->object_size;

	p += get_info_end(s);
	p += sizeof(struct track) * 2;

	return *(unsigned int *)p;
}

static inline void *get_freepointer(struct kmem_cache *s, void *object)
{
	unsigned long ptr_addr;
	freeptr_t p;

	object = kasan_reset_tag(object);
	ptr_addr = (unsigned long)object + s->offset;
	p = *(freeptr_t *)(ptr_addr);
	return freelist_ptr_decode(s, p, ptr_addr);
}

static void __fill_map(unsigned long *obj_map, struct kmem_cache *s,
		       struct slab *slab)
{
	void *addr = slab_address(slab);
	void *p;

	bitmap_zero(obj_map, slab->objects);

	for (p = slab->freelist; p; p = get_freepointer(s, p))
		set_bit(__obj_to_index(s, addr, p), obj_map);
}

static inline unsigned long node_nr_slabs(struct kmem_cache_node *n)
{
	return atomic_long_read(&n->nr_slabs);
}

static void free_loc_track(struct loc_track *t)
{
	if (t->max)
		free_pages((unsigned long)t->loc,
			get_order(sizeof(struct location) * t->max));
}

static int alloc_loc_track(struct loc_track *t, unsigned long max, gfp_t flags)
{
	struct location *l;
	int order;

	order = get_order(sizeof(struct location) * max);

	l = (void *)__get_free_pages(flags, order);
	if (!l)
		return 0;

	if (t->count) {
		memcpy(l, t->loc, sizeof(struct location) * t->count);
		free_loc_track(t);
	}
	t->max = max;
	t->loc = l;
	return 1;
}

static int add_location(struct loc_track *t, struct kmem_cache *s,
				const struct track *track,
				unsigned int orig_size)
{
	long start, end, pos;
	struct location *l;
	unsigned long caddr, aml_handle, cwaste;
	unsigned long age = jiffies - track->when;
	depot_stack_handle_t handle = 0;
	unsigned int waste = s->object_size - orig_size;

#ifdef CONFIG_STACKDEPOT
	handle = READ_ONCE(track->handle);
#endif
	start = -1;
	end = t->count;

	for ( ; ; ) {
		pos = start + (end - start + 1) / 2;

		/*
		 * There is nothing at "end". If we end up there
		 * we need to add something to before end.
		 */
		if (pos == end)
			break;

		l = &t->loc[pos];
		caddr = l->addr;
		aml_handle = l->handle;
		cwaste = l->waste;
		if ((track->addr == caddr) && (handle == aml_handle) &&
			(waste == cwaste)) {

			l->count++;
			if (track->when) {
				l->sum_time += age;
				if (age < l->min_time)
					l->min_time = age;
				if (age > l->max_time)
					l->max_time = age;

				if (track->pid < l->min_pid)
					l->min_pid = track->pid;
				if (track->pid > l->max_pid)
					l->max_pid = track->pid;

				cpumask_set_cpu(track->cpu,
						to_cpumask(l->cpus));
			}
			node_set(page_to_nid(virt_to_page(track)), l->nodes);
			return 1;
		}

		if (track->addr < caddr)
			end = pos;
		else if (track->addr == caddr && handle < aml_handle)
			end = pos;
		else if (track->addr == caddr && handle == aml_handle &&
				waste < cwaste)
			end = pos;
		else
			start = pos;
	}

	/*
	 * Not found. Insert new tracking element.
	 */
	if (t->count >= t->max && !alloc_loc_track(t, 2 * t->max, GFP_ATOMIC))
		return 0;

	l = t->loc + pos;
	if (pos < t->count)
		memmove(l + 1, l,
			(t->count - pos) * sizeof(struct location));
	t->count++;
	l->count = 1;
	l->addr = track->addr;
	l->sum_time = age;
	l->min_time = age;
	l->max_time = age;
	l->min_pid = track->pid;
	l->max_pid = track->pid;
	l->handle = handle;
	l->waste = waste;
	cpumask_clear(to_cpumask(l->cpus));
	cpumask_set_cpu(track->cpu, to_cpumask(l->cpus));
	nodes_clear(l->nodes);
	node_set(page_to_nid(virt_to_page(track)), l->nodes);
	return 1;
}

static void process_slab(struct loc_track *t, struct kmem_cache *s,
		struct slab *slab, enum track_item alloc,
		unsigned long *obj_map)
{
	void *addr = slab_address(slab);
	bool is_alloc = (alloc == TRACK_ALLOC);
	void *p;

	__fill_map(obj_map, s, slab);

	for_each_object(p, s, addr, slab->objects)
		if (!test_bit(__obj_to_index(s, addr, p), obj_map))
			add_location(t, s, get_track(s, p, alloc),
				     is_alloc ? get_orig_size(s, p) :
						s->object_size);
}

static int slab_debugfs_show(struct seq_file *seq, void *v)
{
	struct loc_track *t = seq->private;
	struct location *l;
	unsigned long idx;

	idx = (unsigned long) t->idx;
	if (idx < t->count) {
		l = &t->loc[idx];

		seq_printf(seq, "%7ld ", l->count);

		if (l->addr)
			seq_printf(seq, "%pS", (void *)l->addr);
		else
			seq_puts(seq, "<not-available>");

		if (l->waste)
			seq_printf(seq, " waste=%lu/%lu",
				l->count * l->waste, l->waste);

		if (l->sum_time != l->min_time) {
			seq_printf(seq, " age=%ld/%llu/%ld",
				l->min_time, div_u64(l->sum_time, l->count),
				l->max_time);
		} else
			seq_printf(seq, " age=%ld", l->min_time);

		if (l->min_pid != l->max_pid)
			seq_printf(seq, " pid=%ld-%ld", l->min_pid, l->max_pid);
		else
			seq_printf(seq, " pid=%ld",
				l->min_pid);

		if (num_online_cpus() > 1 && !cpumask_empty(to_cpumask(l->cpus)))
			seq_printf(seq, " cpus=%*pbl",
				 cpumask_pr_args(to_cpumask(l->cpus)));

		if (nr_online_nodes > 1 && !nodes_empty(l->nodes))
			seq_printf(seq, " nodes=%*pbl",
				 nodemask_pr_args(&l->nodes));

#ifdef CONFIG_STACKDEPOT
		{
			depot_stack_handle_t handle;
			unsigned long *entries;
			unsigned int nr_entries, j;

			handle = READ_ONCE(l->handle);
			if (handle) {
				nr_entries = stack_depot_fetch(handle, &entries);
				seq_puts(seq, "\n");
				for (j = 0; j < nr_entries; j++)
					seq_printf(seq, "        %pS\n", (void *)entries[j]);
			}
		}
#endif
		seq_puts(seq, "\n");
	}

	if (!idx && !t->count)
		seq_puts(seq, "No data\n");

	return 0;
}

static void slab_debugfs_stop(struct seq_file *seq, void *v)
{
}

static void *slab_debugfs_next(struct seq_file *seq, void *v, loff_t *ppos)
{
	struct loc_track *t = seq->private;

	t->idx = ++(*ppos);
	if (*ppos <= t->count)
		return ppos;

	return NULL;
}

static int cmp_loc_by_count(const void *a, const void *b)
{
	struct location *loc1 = (struct location *)a;
	struct location *loc2 = (struct location *)b;

	if (loc1->count > loc2->count)
		return -1;
	else
		return 1;
}

static void *slab_debugfs_start(struct seq_file *seq, loff_t *ppos)
{
	struct loc_track *t = seq->private;

	t->idx = *ppos;
	return ppos;
}

static const struct seq_operations slab_debugfs_sops = {
	.start  = slab_debugfs_start,
	.next   = slab_debugfs_next,
	.stop   = slab_debugfs_stop,
	.show   = slab_debugfs_show,
};

static int slab_debug_trace_open(struct inode *inode, struct file *filep)
{

	struct kmem_cache_node *n;
	enum track_item alloc;
	int node;
	struct loc_track *t = __seq_open_private(filep, &slab_debugfs_sops,
						sizeof(struct loc_track));
	struct kmem_cache *s = file_inode(filep)->i_private;
	unsigned long *obj_map;

	if (!t)
		return -ENOMEM;

	obj_map = bitmap_alloc(oo_objects(s->oo), GFP_KERNEL);
	if (!obj_map) {
		seq_release_private(inode, filep);
		return -ENOMEM;
	}

	if (strcmp(filep->f_path.dentry->d_name.name, "alloc_traces") == 0)
		alloc = TRACK_ALLOC;
	else
		alloc = TRACK_FREE;

	if (!alloc_loc_track(t, PAGE_SIZE / sizeof(struct location), GFP_KERNEL)) {
		bitmap_free(obj_map);
		seq_release_private(inode, filep);
		return -ENOMEM;
	}

	for_each_kmem_cache_node(s, node, n) {
		unsigned long flags;
		struct slab *slab;

		if (!node_nr_slabs(n))
			continue;

		spin_lock_irqsave(&n->list_lock, flags);
		list_for_each_entry(slab, &n->partial, slab_list)
			process_slab(t, s, slab, alloc, obj_map);
		list_for_each_entry(slab, &n->full, slab_list)
			process_slab(t, s, slab, alloc, obj_map);
		spin_unlock_irqrestore(&n->list_lock, flags);
	}

	/* Sort locations by count */
	sort(t->loc, t->count, sizeof(struct location),
		cmp_loc_by_count, NULL);

	bitmap_free(obj_map);
	return 0;
}

static int slab_debug_trace_release(struct inode *inode, struct file *file)
{
	struct seq_file *seq = file->private_data;
	struct loc_track *t = seq->private;

	free_loc_track(t);
	return seq_release_private(inode, file);
}

static const struct proc_ops slab_debugfs_fops = {
	.proc_open    = slab_debug_trace_open,
	.proc_read    = seq_read,
	.proc_lseek  = seq_lseek,
	.proc_release = slab_debug_trace_release,
};

static void proc_slab_add(struct kmem_cache *s)
{
	struct proc_dir_entry *slab_cache_dir;

	if (!slab_debugfs_root)
		return;

	slab_cache_dir = proc_mkdir(s->name, slab_debugfs_root);
	if (!slab_cache_dir)
		return;

	proc_create_data("alloc_traces", 0400, slab_cache_dir,
				&slab_debugfs_fops, s);

	proc_create_data("free_traces", 0400, slab_cache_dir,
				&slab_debugfs_fops, s);
}

static int aml_setup_slub_debug(char *buf)
{
	if (!buf)
		aml_debug_enabled = 0;
	else
		aml_debug_enabled = 1;

	return 0;

}

__setup("slub_debug=", aml_setup_slub_debug);

int __init slab_debugfs_init(void)
{
	struct kmem_cache *s;
	struct kmem_cache *aml_slub;
	struct list_head *current_node;

	slab_debugfs_root = proc_mkdir("slub_debug", NULL);
	if (!slab_debugfs_root)
		pr_err("mkdir /proc/slub_debug fail\n");

	if (aml_debug_enabled) {
		aml_slub = kmem_cache_create("aml_slub",
							64,
							0, SLAB_RECLAIM_ACCOUNT,
							NULL);
		current_node = aml_slub->list.prev;
		kmem_cache_destroy(aml_slub);

		list_for_each_entry(s, current_node, list)
			if (s->flags & SLAB_STORE_USER)
				proc_slab_add(s);
	}

	return 0;

}

void __exit slab_debugfs_exit(void)
{
}
