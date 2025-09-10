/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__

#include <linux/cleancache.h>
#include <linux/writeback.h>
#include <linux/parser.h>
#include <linux/iomap.h>
#include <linux/nls.h>
#include <linux/pagevec.h>
#include <linux/mempool.h>
#include <linux/fs_context.h>
#include <linux/fs_parser.h>
#include <linux/fscrypt.h>
#include <linux/quotaops.h>

#define EXT_FUN_TYPE1(a)	extern a##_t f_##a
#define EXT_FUN_TYPEG(a)	extern a##_g f_##a
#define EXT_FUN_TYPE2(a, b)	extern a##_t f_##b
#define EXT_OBJ_TYPE(a, b)	extern a * b##_t

#define FUN_TYPE1(a)		a##_t f_##a;	\
				EXPORT_SYMBOL(f_##a)
#define FUN_TYPEG(a)		a##_g f_##a;	\
				EXPORT_SYMBOL(f_##a)
#define FUN_TYPE2(a, b)		a##_t f_##b;	\
				EXPORT_SYMBOL(f_##b)
#define OBJ_TYPE(a, b)		a * b##_t;	\
				EXPORT_SYMBOL(b##_t)
#define FUN_ASSIGN(a)		f_##a = a
#define OBJ_ASSIGN(a)		a##_t = &a

#define KSYM_FUN(sym)			\
	{				\
		.name = #sym,		\
		.data = &f_##sym,	\
	}

#ifdef CONFIG_KASAN_GENERIC
/* When KASAN enabled cfi is disabled */
#define KSYM_CFI(sym)			\
	{				\
		.name = #sym ,		\
		.data = &f_##sym,	\
	}
#else
#define KSYM_CFI(sym)			\
	{				\
		.name = #sym ".cfi_jt",	\
		.data = &f_##sym,	\
	}
#endif

#define KSYM_OBJ(sym)			\
	{				\
		.name = #sym,		\
		.data = &sym##_t,	\
	}
/*---------- erofs symbols ------------*/
typedef uint32_t (*xxh32_t)(const void *input, const size_t len, const uint32_t seed);
typedef int (*filemap_add_folio_t)(struct address_space *mapping, struct folio *folio,
				pgoff_t index, gfp_t gfp);
typedef void (*psi_memstall_enter_t)(unsigned long *flags);
typedef void (*psi_memstall_leave_t)(unsigned long *flags);
typedef void (*migrate_disable_t)(void);
typedef void (*migrate_enable_t)(void);
typedef void (*flush_dcache_folio_t)(struct folio *folio);
typedef void (*folio_unlock_t)(struct folio *folio);
typedef unsigned long (*alloc_pages_bulk_noprof_t)(gfp_t gfp, int preferred_nid,
				nodemask_t *nodemask, int nr_pages,
				struct list_head *page_list,
				struct page **page_array);
typedef int (*lockref_get_not_zero_t)(struct lockref *lockref);
typedef int (*lockref_put_or_lock_t)(struct lockref *lockref);
typedef void (*lockref_mark_dead_t)(struct lockref *lockref);
typedef void (*bio_init_t)(struct bio *bio, struct block_device *bdev, struct bio_vec *table,
	      unsigned short max_vecs, blk_opf_t opf);
typedef ssize_t (*vfs_iocb_iter_read_t)(struct file *file, struct kiocb *iocb,
			   struct iov_iter *iter);
typedef void (*__bio_advance_t)(struct bio *bio, unsigned bytes);
typedef void (*zero_fill_bio_iter_t)(struct bio *bio, struct bvec_iter start);
typedef void (*bio_uninit_t)(struct bio *bio);
typedef bool (*bio_add_folio_t)(struct bio *bio, struct folio *folio, size_t len,
		   size_t off);
typedef int (*get_tree_bdev_flags_t)(struct fs_context *fc,
		int (*fill_super)(struct super_block *sb,
				  struct fs_context *fc), unsigned int flags);
typedef struct file *(*filp_open_t)(const char *filename, int flags, umode_t mode);
typedef int (*get_tree_nodev_t)(struct fs_context *fc,
		  int (*fill_super)(struct super_block *sb,
				    struct fs_context *fc));
typedef int (*super_setup_bdi_t)(struct super_block *sb);
typedef const char *(*bdi_dev_name_t)(struct backing_dev_info *bdi);
typedef struct folio *(*read_cache_folio_t)(struct address_space *mapping, pgoff_t index,
		filler_t filler, struct file *file);
typedef void (*folio_end_read_t)(struct folio *folio, bool success);
typedef int (*iomap_read_folio_t)(struct folio *folio, const struct iomap_ops *ops);
typedef void (*iomap_invalidate_folio_t)(struct folio *folio, size_t offset, size_t len);
typedef bool (*iomap_release_folio_t)(struct folio *folio, gfp_t gfp_flags);
typedef unsigned long (*thp_get_unmapped_area_t)(struct file *filp, unsigned long addr,
		unsigned long len, unsigned long pgoff, unsigned long flags);
typedef void (*__seq_puts_t)(struct seq_file *m, const char *s);
typedef size_t (*_copy_to_iter_t)(const void *addr, size_t bytes, struct iov_iter *i);
typedef void (*copy_highpage_t)(struct page *to, struct page *from);
typedef void (*iov_iter_bvec_t)(struct iov_iter *i, unsigned int direction,
			const struct bio_vec *bvec, unsigned long nr_segs,
			size_t count);
typedef char *(*kmemdup_nul_t)(const char *s, size_t len, gfp_t gfp);
typedef void *(*memchr_inv_t)(const void *start, int c, size_t bytes);
typedef void *(*kmem_cache_alloc_lru_noprof_t)(struct kmem_cache *s, struct list_lru *lru,
			    gfp_t gfpflags);
typedef pgoff_t (*__folio_swap_cache_index_t)(struct folio *folio);
typedef struct file *(*bdev_file_open_by_path_t)(const char *path, blk_mode_t mode,
				    void *holder,
				    const struct blk_holder_ops *hops);
typedef struct block_device *(*file_bdev_t)(struct file *bdev_file);

typedef int (*generic_encode_ino32_fh_t)(struct inode *inode, __u32 *fh, int *max_len,
			    struct inode *parent);
typedef struct folio *(*__filemap_get_folio_t)(struct address_space *mapping, pgoff_t index,
		fgf_t fgp_flags, gfp_t gfp);
typedef ssize_t (*filemap_splice_read_t)(struct file *in, loff_t *ppos,
			    struct pipe_inode_info *pipe,
			    size_t len, unsigned int flags);
typedef void (*wait_for_completion_io_t)(struct completion *);

/*--------- erofs symbols ----------*/
EXT_FUN_TYPE1(wait_for_completion_io);
EXT_FUN_TYPE1(xxh32);
EXT_FUN_TYPE1(__filemap_get_folio);
EXT_FUN_TYPE1(file_bdev);
EXT_FUN_TYPE1(filemap_add_folio);
EXT_FUN_TYPE1(flush_dcache_folio);
EXT_FUN_TYPE1(psi_memstall_leave);
EXT_FUN_TYPE1(psi_memstall_enter);
EXT_FUN_TYPE1(migrate_disable);
EXT_FUN_TYPE1(migrate_enable);
EXT_FUN_TYPE1(alloc_pages_bulk_noprof);
EXT_FUN_TYPE1(generic_encode_ino32_fh);
EXT_FUN_TYPE1(__folio_swap_cache_index);
EXT_FUN_TYPE1(lockref_get_not_zero);
EXT_FUN_TYPE1(lockref_put_or_lock);
EXT_FUN_TYPE1(lockref_mark_dead);
EXT_FUN_TYPE1(bio_init);
EXT_FUN_TYPE1(filemap_splice_read);
EXT_FUN_TYPE1(vfs_iocb_iter_read);
EXT_FUN_TYPE1(__bio_advance);
EXT_FUN_TYPE1(zero_fill_bio_iter);
EXT_FUN_TYPE1(bio_uninit);
EXT_FUN_TYPE1(bio_add_folio);
EXT_FUN_TYPE1(kmem_cache_alloc_lru_noprof);
EXT_FUN_TYPE1(get_tree_bdev_flags);
EXT_FUN_TYPE1(filp_open);
EXT_FUN_TYPE1(get_tree_nodev);
EXT_FUN_TYPE1(super_setup_bdi);
EXT_FUN_TYPE1(bdi_dev_name);
EXT_FUN_TYPE1(read_cache_folio);
EXT_FUN_TYPE1(folio_end_read);
EXT_FUN_TYPE1(iomap_read_folio);
EXT_FUN_TYPE1(iomap_invalidate_folio);
EXT_FUN_TYPE1(iomap_release_folio);
EXT_FUN_TYPE1(thp_get_unmapped_area);
EXT_FUN_TYPE1(__seq_puts);
EXT_FUN_TYPE1(_copy_to_iter);
EXT_FUN_TYPE1(folio_unlock);
EXT_FUN_TYPE1(copy_highpage);
EXT_FUN_TYPE1(iov_iter_bvec);
EXT_FUN_TYPE1(kmemdup_nul);
EXT_FUN_TYPE1(memchr_inv);
EXT_FUN_TYPE1(bdev_file_open_by_path);
EXT_OBJ_TYPE(struct qstr, dotdot_name);
EXT_OBJ_TYPE(struct xattr_handler , nop_posix_acl_access);
EXT_OBJ_TYPE(struct xattr_handler , nop_posix_acl_default);

#define f___alloc_pages_bulk(...)	alloc_hooks(f_alloc_pages_bulk_noprof(__VA_ARGS__))
#define f_alloc_pages_bulk_array(_gfp, _nr_pages, _page_array)		\
	f___alloc_pages_bulk(_gfp, numa_mem_id(), NULL, _nr_pages, NULL, _page_array)

#define f_kmem_cache_alloc_lru(...)	alloc_hooks(f_kmem_cache_alloc_lru_noprof(__VA_ARGS__))
#define f_alloc_inode_sb(_sb, _cache, _gfp) f_kmem_cache_alloc_lru(_cache, &_sb->s_inode_lru, _gfp)

static inline void f_bio_advance(struct bio *bio, unsigned int nbytes)
{
	if (nbytes == bio->bi_iter.bi_size) {
		bio->bi_iter.bi_size = 0;
		return;
	}
	f___bio_advance(bio, nbytes);
}

static inline void f_zero_fill_bio(struct bio *bio)
{
	f_zero_fill_bio_iter(bio, bio->bi_iter);
}

static inline struct folio *f_read_mapping_folio(struct address_space *mapping,
				pgoff_t index, struct file *file)
{
	return f_read_cache_folio(mapping, index, NULL, file);
}

static inline pgoff_t f_folio_index(struct folio *folio)
{
	if (unlikely(folio_test_swapcache(folio)))
		return f___folio_swap_cache_index(folio);
	return folio->index;
}

static inline bool f_folio_contains(struct folio *folio, pgoff_t index)
{
	return index - f_folio_index(folio) < folio_nr_pages(folio);
}

static __always_inline void f_seq_puts(struct seq_file *m, const char *s)
{
	if (!__builtin_constant_p(*s))
		f___seq_puts(m, s);
	else if (s[0] && !s[1])
		seq_putc(m, s[0]);
	else
		seq_write(m, s, __builtin_strlen(s));
}

static __always_inline __must_check
size_t F_copy_to_iter(const void *addr, size_t bytes, struct iov_iter *i)
{
	if (check_copy_size(addr, bytes, true))
		return f__copy_to_iter(addr, bytes, i);
	return 0;
}

static inline void f_memcpy_to_folio(struct folio *folio, size_t offset,
		const char *from, size_t len)
{
	VM_BUG_ON(offset + len > folio_size(folio));

	do {
		char *to = kmap_local_folio(folio, offset);
		size_t chunk = len;

		if (folio_test_highmem(folio) &&
		    chunk > PAGE_SIZE - offset_in_page(offset))
			chunk = PAGE_SIZE - offset_in_page(offset);
		memcpy(to, from, chunk);
		kunmap_local(to);

		from += chunk;
		offset += chunk;
		len -= chunk;
	} while (len > 0);

	f_flush_dcache_folio(folio);
}

int symbol_fix(void);
int symbol_fixed(void);
#endif
