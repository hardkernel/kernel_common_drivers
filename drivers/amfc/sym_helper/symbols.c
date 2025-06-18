// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/fs_context.h>
#include <linux/statfs.h>
#include <linux/buffer_head.h>
#include <linux/backing-dev.h>
#include <linux/kthread.h>
#include <linux/parser.h>
#include <linux/mount.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/random.h>
#include <linux/exportfs.h>
#include <linux/blkdev.h>
#include <linux/quotaops.h>
#include <linux/f2fs_fs.h>
#include <linux/sysfs.h>
#include <linux/quota.h>
#include <linux/unicode.h>
#include <linux/part_stat.h>
#include <linux/cleancache.h>
#include <linux/sched/clock.h>
#include <linux/async.h>
#include <linux/amlogic/amfc.h>
#include <linux/amlogic/gki_module.h>
#include <linux/amlogic/symbols.h>

#include <linux/proc_fs.h>

/*--------- erofs symbols ----------*/
FUN_TYPE1(xxh32);
FUN_TYPE1(filemap_add_folio);
FUN_TYPE1(psi_memstall_leave);
FUN_TYPE1(psi_memstall_enter);
FUN_TYPE1(wait_for_completion_io);
FUN_TYPE1(migrate_disable);
FUN_TYPE1(migrate_enable);
FUN_TYPE1(alloc_pages_bulk_noprof);
FUN_TYPE1(lockref_get_not_zero);
FUN_TYPE1(lockref_put_or_lock);
FUN_TYPE1(lockref_mark_dead);
FUN_TYPE1(bio_init);
FUN_TYPE1(vfs_iocb_iter_read);
FUN_TYPE1(__bio_advance);
FUN_TYPE1(zero_fill_bio_iter);
FUN_TYPE1(bio_uninit);
FUN_TYPE1(bio_add_folio);
FUN_TYPE1(kmem_cache_alloc_lru_noprof);
FUN_TYPE1(get_tree_bdev_flags);
FUN_TYPE1(filp_open);
FUN_TYPE1(get_tree_nodev);
FUN_TYPE1(super_setup_bdi);
FUN_TYPE1(bdi_dev_name);
FUN_TYPE1(generic_encode_ino32_fh);
FUN_TYPE1(read_cache_folio);
FUN_TYPE1(folio_end_read);
FUN_TYPE1(iomap_read_folio);
FUN_TYPE1(iomap_invalidate_folio);
FUN_TYPE1(iomap_release_folio);
FUN_TYPE1(thp_get_unmapped_area);
FUN_TYPE1(filemap_splice_read);
FUN_TYPE1(__filemap_get_folio);
FUN_TYPE1(__folio_swap_cache_index);
FUN_TYPE1(__seq_puts);
FUN_TYPE1(_copy_to_iter);
FUN_TYPE1(bdev_file_open_by_path);
FUN_TYPE1(copy_highpage);
FUN_TYPE1(file_bdev);
FUN_TYPE1(flush_dcache_folio);
FUN_TYPE1(folio_unlock);
FUN_TYPE1(iov_iter_bvec);
FUN_TYPE1(kmemdup_nul);
FUN_TYPE1(memchr_inv);
OBJ_TYPE(struct qstr, dotdot_name);
OBJ_TYPE(struct xattr_handler , nop_posix_acl_access);
OBJ_TYPE(struct xattr_handler , nop_posix_acl_default);

struct ksymbol {
	const char *name;
	void *data;
	unsigned int name_len;
};

int __symbol_fixed;

#ifdef CONFIG_ARM64
static struct ksymbol module_symbols[] = {
	KSYM_FUN(xxh32),
	KSYM_FUN(filemap_add_folio),
	KSYM_FUN(psi_memstall_leave),
	KSYM_FUN(psi_memstall_enter),
	KSYM_FUN(wait_for_completion_io),
	KSYM_FUN(migrate_disable),
	KSYM_FUN(migrate_enable),
	KSYM_FUN(alloc_pages_bulk_noprof),
	KSYM_FUN(lockref_get_not_zero),
	KSYM_FUN(lockref_put_or_lock),
	KSYM_FUN(lockref_mark_dead),
	KSYM_FUN(bio_init),
	KSYM_FUN(vfs_iocb_iter_read),
	KSYM_FUN(__bio_advance),
	KSYM_FUN(zero_fill_bio_iter),
	KSYM_FUN(bio_uninit),
	KSYM_FUN(bio_add_folio),
	KSYM_FUN(kmem_cache_alloc_lru_noprof),
	KSYM_FUN(get_tree_bdev_flags),
	KSYM_FUN(filp_open),
	KSYM_FUN(get_tree_nodev),
	KSYM_FUN(super_setup_bdi),
	KSYM_FUN(bdi_dev_name),
	KSYM_FUN(generic_encode_ino32_fh),
	KSYM_FUN(read_cache_folio),
	KSYM_FUN(folio_end_read),
	KSYM_FUN(iomap_read_folio),
	KSYM_FUN(iomap_invalidate_folio),
	KSYM_FUN(iomap_release_folio),
	KSYM_FUN(thp_get_unmapped_area),
	KSYM_FUN(filemap_splice_read),
	KSYM_FUN(__filemap_get_folio),
	KSYM_FUN(__folio_swap_cache_index),
	KSYM_FUN(__seq_puts),
	KSYM_FUN(_copy_to_iter),
	KSYM_FUN(bdev_file_open_by_path),
	KSYM_FUN(copy_highpage),
	KSYM_FUN(file_bdev),
	KSYM_FUN(flush_dcache_folio),
	KSYM_FUN(folio_unlock),
	KSYM_FUN(iov_iter_bvec),
	KSYM_FUN(kmemdup_nul),
	KSYM_FUN(memchr_inv),
	KSYM_OBJ(dotdot_name),
	KSYM_OBJ(nop_posix_acl_access),
	KSYM_OBJ(nop_posix_acl_default),
	{}
};

/* see struct proc_dir_entry in fs/proc/internal.h */
struct proc_node {
	/*
	 * number of callers into module in progress;
	 * negative -> it's going away RSN
	 */
	atomic_t in_use;
	refcount_t refcnt;
	struct list_head pde_openers;	/* who did ->open, but not ->release */
	/* protects ->pde_openers and all struct pde_opener instances */
	spinlock_t pde_unload_lock;
	struct completion *pde_unload_completion;
	const struct inode_operations *proc_iops;
	union {
		const struct proc_ops *proc_ops;
		const struct file_operations *proc_dir_ops;
	};
	const struct dentry_operations *proc_dops;
	union {
		const struct seq_operations *seq_ops;
		int (*single_show)(struct seq_file *, void *);
	};
	proc_write_t write;
	void *data;
	unsigned int state_size;
	unsigned int low_ino;
	nlink_t nlink;
	kuid_t uid;
	kgid_t gid;
	loff_t size;
	struct proc_node *parent;
	struct rb_root subdir;
	struct rb_node subdir_node;
	char *name;
	umode_t mode;
	u8 flags;
	u8 namelen;
	char inline_name[];
} __randomize_layout;

static struct proc_node *find_subnode(struct proc_node *parent, char *name)
{
	struct proc_node *pd = NULL;

	pd = rb_entry_safe(rb_first(&parent->subdir), struct proc_node, subdir_node);
	while (1) {
		if (!pd)
			break;
		if (!strcmp(pd->name, name))
			break;
		pd = rb_entry_safe(rb_next(&pd->subdir_node), struct proc_node, subdir_node);
	}
	return pd;
}

static int namecmp(const char *name1, int len1, const char *name2, int len2)
{
	int cmp;

	cmp = memcmp(name1, name2, min(len1, len2));
	if (cmp == 0)
		cmp = len1 - len2;
	return cmp;
}

static int proc_handle(const struct ctl_table *ctl, int write, void *buffer,
		size_t *lenp, loff_t *ppos)
{
	return 0;
}

static struct ctl_table *disable_kstr_ptr(int *old)
{
	struct ctl_table tmp[] = {
		{
			.procname = "amfc",
			.proc_handler = proc_handle,
			.mode = 0666,
			.data = NULL,
		}
	};
	struct ctl_table_header *hdr, *head;
	struct rb_node *node;
	struct ctl_table *entry = NULL;
	struct ctl_dir *dir;

	hdr = register_sysctl("kernel", tmp);
	if (!hdr) {
		pr_err("%s, register failed\n", __func__);
		return NULL;
	}

	dir = hdr->parent;
	node = dir->root.rb_node;
	while (node) {
		struct ctl_node *ctl_node;
		const char *procname;
		int cmp;

		ctl_node = rb_entry(node, struct ctl_node, node);
		head = ctl_node->header;
		entry = &head->ctl_table[ctl_node - head->node];
		procname = entry->procname;

		cmp = namecmp("perf_event_paranoid", 13, procname, strlen(procname));
		if (cmp < 0)
			node = node->rb_left;
		else if (cmp > 0)
			node = node->rb_right;
		else {
			break;
		}
	}
	unregister_sysctl_table(hdr);
	if (entry) {
		*old = *(int *)entry->data;
		pr_debug("get entry:%px, %s, value:%d\n",
			entry, entry->procname, *old);
		*(int *)entry->data = 0;
		return entry;
	}
	return NULL;
}

static int fill_symbol(char *line, struct ksymbol *sym)
{
	int finish = 2;
	unsigned long value;
	int ret, find = 0, len;

	while (sym->name) {
		if (*(unsigned long *)sym->data) {
			sym++;
			continue;
		} else
			finish = 0;

		if (!sym->name_len)
			sym->name_len = strlen(sym->name);
		len = sym->name_len;
		if (!strncmp(line + 19, sym->name, len)) {
			if (line[19 + len + 1]) {	// not terminate
				sym++;
				continue;
			}
			line[16] = '\0';
			ret = kstrtoul(line, 16, &value);
			pr_debug("find sym:%lx %s\n", value, sym->name);
			if (ret)
				return ret;
			*(unsigned long *)sym->data = value;
			find = 1;
			break;
		}
		sym++;
	}
	if (finish == 2)
		return 2;
	return find;
}

int check_sym(struct ksymbol *sym)
{
	int find = 0, nfind = 0;

	while (sym->name) {
		if (*(unsigned long *)sym->data) {
			find++;
		} else {
			pr_err("can't find sym:%s\n", sym->name);
			nfind++;
		}
		sym++;
	}
	return nfind;
}

static struct super_block _s = {};

static struct inode _i = {
	.i_sb = &_s,
};

static struct address_space _a = {
	.host = &_i,
};

static struct file f = {
	.f_mapping = &_a,
	.f_inode = &_i,
};

static int fill_module_symbols(struct proc_node *node, struct ksymbol *sym)
{
	struct proc_ops *ops;
	struct seq_file *s;
	char line_buf[256] = {};
	int ret = 0;
	loff_t off = 0;
	void *p;

	ops = (struct proc_ops *)node->proc_ops;
	ret = ops->proc_open(NULL, &f);
	if (ret) {
		pr_info("open fail\n");
		return -1;
	}
	s = f.private_data;
	p = s->op->start(s, &off);
	s->buf = line_buf;
	while (1) {
		s->count = 0;
		s->size  = sizeof(line_buf);
		//memset(line_buf, 0, sizeof(line_buf));
		ret = s->op->show(s, NULL);
		if (ret < 0) {
			pr_info("read fail:%d\n", ret);
			break;
		}
		s->count = 0;
		pr_debug("line:%s", line_buf);
		ret = fill_symbol(line_buf, sym);
		if (ret == 2)
			break;
		p = s->op->next(s, p, &off);
		if (!p || IS_ERR(p))
			break;
	}
	s->buf = NULL;
	ops->proc_release(NULL, &f);
	if (check_sym(sym))
		return -ENOSYS;
	return 0;
}

int symbol_fix(void)
{
	struct proc_dir_entry *entry;
	struct proc_node *parent, *tmp;
	const struct proc_ops temp = {};
	struct ctl_table *kptr;
	int ret = 0, old = -1;
	unsigned long long te;

	te = sched_clock();
	entry = proc_create("amfc", 0444, NULL, &temp);
	if (!entry) {
		pr_err("%s, create proc failed\n", __func__);
		return -1;
	}
	parent = ((struct proc_node *)entry)->parent;
	if (!parent) {
		pr_err("%s, NULL pareret\n", __func__);
		return -1;
	}

	kptr = disable_kstr_ptr(&old);
	if (!kptr) {
		pr_err("get kptr failed\n");
		return -1;
	}
	tmp = find_subnode(parent, "kallsyms");
	if (!tmp) {
		pr_err("get kallsyms failed\n");
		return -1;
	}

	ret = fill_module_symbols(tmp, module_symbols);

	*(int *)kptr->data = old;

	proc_remove(entry);

	if (!ret)
		__symbol_fixed = 1;
	pr_info("%s, prio:%d, ret:%d, time:%lld\n",
		__func__, current->prio, ret, sched_clock() - te);
	return ret;
}
#else
/* for arm32 */
#include <linux/xxhash.h>
#include <linux/psi.h>
#include <linux/posix_acl_xattr.h>

int symbol_fix(void)
{
	/*--------- erofs symbols ----------*/
	FUN_ASSIGN(xxh32);
	FUN_ASSIGN(filemap_add_folio);
	FUN_ASSIGN(psi_memstall_leave);
	FUN_ASSIGN(psi_memstall_enter);
	FUN_ASSIGN(wait_for_completion_io);
	FUN_ASSIGN(migrate_disable);
	FUN_ASSIGN(migrate_enable);
	FUN_ASSIGN(alloc_pages_bulk_noprof);
	FUN_ASSIGN(lockref_get_not_zero);
	FUN_ASSIGN(lockref_put_or_lock);
	FUN_ASSIGN(lockref_mark_dead);
	FUN_ASSIGN(bio_init);
	FUN_ASSIGN(vfs_iocb_iter_read);
	FUN_ASSIGN(__bio_advance);
	FUN_ASSIGN(zero_fill_bio_iter);
	FUN_ASSIGN(bio_uninit);
	FUN_ASSIGN(bio_add_folio);
	FUN_ASSIGN(kmem_cache_alloc_lru_noprof);
	FUN_ASSIGN(get_tree_bdev_flags);
	FUN_ASSIGN(filp_open);
	FUN_ASSIGN(get_tree_nodev);
	FUN_ASSIGN(super_setup_bdi);
	FUN_ASSIGN(bdi_dev_name);
	FUN_ASSIGN(generic_encode_ino32_fh);
	FUN_ASSIGN(read_cache_folio);
	FUN_ASSIGN(folio_end_read);
	FUN_ASSIGN(iomap_read_folio);
	FUN_ASSIGN(iomap_invalidate_folio);
	FUN_ASSIGN(iomap_release_folio);
	FUN_ASSIGN(thp_get_unmapped_area);
	FUN_ASSIGN(filemap_splice_read);
	FUN_ASSIGN(__filemap_get_folio);
	FUN_ASSIGN(__folio_swap_cache_index);
	FUN_ASSIGN(__seq_puts);
	FUN_ASSIGN(_copy_to_iter);
	FUN_ASSIGN(bdev_file_open_by_path);
	FUN_ASSIGN(copy_highpage);
	FUN_ASSIGN(file_bdev);
	FUN_ASSIGN(flush_dcache_folio);
	FUN_ASSIGN(folio_unlock);
	FUN_ASSIGN(iov_iter_bvec);
	FUN_ASSIGN(kmemdup_nul);
	FUN_ASSIGN(memchr_inv);
	dotdot_name_t = (struct qstr *)&dotdot_name;
	nop_posix_acl_access_t = (struct xattr_handler *)&nop_posix_acl_access;
	nop_posix_acl_default_t = (struct xattr_handler *)&nop_posix_acl_default;

	__symbol_fixed = 1;
	return 0;
}
#endif

int symbol_fixed(void)
{
	return __symbol_fixed;
}
EXPORT_SYMBOL(symbol_fixed);

#ifdef CONFIG_ARM64
static void __init do_symbol_fix(void *data, async_cookie_t cookie)
{
	if (symbol_fix()) {
		pr_emerg("%s, %d, symbol fix failed\n", __func__, __LINE__);
		BUG();
		return;
	}
}
#endif

int __init sym_helper_init(void)
{
#ifdef CONFIG_ARM64
	async_schedule(do_symbol_fix, NULL);
#else
	symbol_fix();
#endif
	return 0;
}

void __exit sym_helper_exit(void)
{
	__symbol_fixed = 0;
#ifdef CONFIG_ARM64
	memset(module_symbols, 0, sizeof(module_symbols));
#endif
}
module_init(sym_helper_init);
module_exit(sym_helper_exit);
MODULE_LICENSE("GPL");
