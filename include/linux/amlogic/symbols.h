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

/*---------- f2fs symbols -------------*/
typedef s64 (*__percpu_counter_sum_t)(struct percpu_counter *fbc);
typedef bool (*inode_owner_or_capable_t)(struct mnt_idmap *idmap,
			    const struct inode *inode);
typedef struct posix_acl *(*posix_acl_alloc_t)(int count, gfp_t flags);
typedef int (*posix_acl_equiv_mode_t)(const struct posix_acl *acl, umode_t *mode_p);
typedef bool (*in_group_or_capable_t)(struct mnt_idmap *idmap,
			 const struct inode *inode, vfsgid_t vfsgid);
typedef void (*page_cache_ra_unbounded_t)(struct readahead_control *ractl,
		unsigned long nr_to_read, unsigned long lookahead_size);
typedef void *(*mempool_alloc_pages_t)(gfp_t gfp_mask, void *pool_data);
typedef void (*mempool_free_pages_t)(void *element, void *pool_data);
typedef mempool_t *(*mempool_create_node_noprof_t)(int min_nr, mempool_alloc_t *alloc_fn,
				      mempool_free_t *free_fn, void *pool_data,
				      gfp_t gfp_mask, int node_id);
typedef pgoff_t (*__folio_swap_cache_index_t)(struct folio *folio);
typedef void (*fsverity_enqueue_verify_work_t)(struct work_struct *work);
typedef bool (*__fscrypt_inode_uses_inline_crypto_t)(const struct inode *inode);
typedef void (*fscrypt_free_bounce_page_t)(struct page *bounce_page);
typedef bool (*fsverity_verify_blocks_t)(struct folio *folio, size_t len, size_t offset);
typedef void (*fsverity_verify_bio_t)(struct bio *bio);
typedef const char *(*blk_op_str_t)(enum req_op op);
typedef unsigned (*filemap_get_folios_t)(struct address_space *mapping, pgoff_t *start,
		pgoff_t end, struct folio_batch *fbatch);
typedef unsigned (*filemap_get_folios_tag_t)(struct address_space *mapping, pgoff_t *start,
			pgoff_t end, xa_mark_t tag, struct folio_batch *fbatch);
typedef void (*f2fs_destroy_compress_cache_t)(void);
typedef void (*f2fs_destroy_compress_mempool_t)(void);
typedef void (*f2fs_destroy_bioset_t)(void);
typedef void (*f2fs_destroy_bio_entry_cache_t)(void);
typedef void (*f2fs_destroy_iostat_processing_t)(void);
typedef void (*f2fs_destroy_post_read_processing_t)(void);
typedef void (*f2fs_destroy_root_stats_t)(void);
typedef void (*f2fs_exit_sysfs_t)(void);
typedef void (*f2fs_destroy_garbage_collection_cache_t)(void);
typedef void (*f2fs_destroy_extent_cache_t)(void);
typedef void (*f2fs_destroy_recovery_cache_t)(void);
typedef void (*f2fs_destroy_checkpoint_caches_t)(void);
typedef void (*f2fs_destroy_segment_manager_caches_t)(void);
typedef void (*f2fs_destroy_node_manager_caches_t)(void);
typedef int (*utf8_casefold_t)(const struct unicode_map *um, const struct qstr *str,
		  unsigned char *dest, size_t dlen);
typedef int (*fscrypt_setup_filename_t)(struct inode *dir, const struct qstr *iname,
			      int lookup, struct fscrypt_name *fname);
typedef int (*fscrypt_d_revalidate_t)(struct dentry *dentry, unsigned int flags);
typedef int (*__fscrypt_prepare_lookup_t)(struct inode *dir, struct dentry *dentry,
			     struct fscrypt_name *fname);
typedef int (*generic_ci_match_t)(const struct inode *parent,
		     const struct qstr *name,
		     const struct qstr *folded_name,
		     const u8 *de_name, u32 de_name_len);
typedef bool (*fscrypt_match_name_t)(const struct fscrypt_name *fname,
			const u8 *de_name, u32 de_name_len);
typedef unsigned char (*fs_umode_to_ftype_t)(umode_t mode);
typedef int (*fscrypt_set_context_t)(struct inode *inode, void *fs_data);
typedef int (*fscrypt_fname_disk_to_usr_t)(const struct inode *inode,
			      u32 hash, u32 minor_hash,
			      const struct fscrypt_str *iname,
			      struct fscrypt_str *oname);
typedef int (*__fscrypt_prepare_readdir_t)(struct inode *dir);
typedef int (*fscrypt_fname_alloc_buffer_t)(u32 max_encrypted_len,
			       struct fscrypt_str *crypto_str);
typedef void (*page_cache_sync_ra_t)(struct readahead_control *, unsigned long req_count);
typedef void (*fscrypt_fname_free_buffer_t)(struct fscrypt_str *crypto_str);
typedef void (*__dquot_free_space_t)(struct inode *inode, qsize_t number, int flags);
typedef bool (*fscrypt_dio_supported_t)(struct inode *inode);
typedef int (*__fscrypt_prepare_setattr_t)(struct dentry *dentry, struct iattr *attr);
typedef int (*__fsverity_prepare_setattr_t)(struct dentry *dentry, struct iattr *attr);
typedef int (*dquot_transfer_t)(struct mnt_idmap *idmap, struct inode *inode,
		   struct iattr *iattr);
typedef kuid_t (*from_vfsuid_t)(struct mnt_idmap *idmap,
		   struct user_namespace *fs_userns, vfsuid_t vfsuid);
typedef kgid_t (*from_vfsgid_t)(struct mnt_idmap *idmap,
		   struct user_namespace *fs_userns, vfsgid_t vfsgid);
typedef int (*bdev_freeze_t)(struct block_device *bdev);
typedef int (*bdev_thaw_t)(struct block_device *bdev);
typedef struct dquot *(*dqget_t)(struct super_block *sb, struct kqid qid);
typedef int (*__dquot_transfer_t)(struct inode *inode, struct dquot **transfer_to);
typedef void (*dqput_t)(struct dquot *dquot);
typedef int (*fscrypt_ioctl_remove_key_t)(struct file *filp, void __user *arg);
typedef int (*fscrypt_ioctl_remove_key_all_users_t)(struct file *filp, void __user *arg);
typedef int (*fscrypt_ioctl_get_nonce_t)(struct file *filp, void __user *arg);
typedef int (*fscrypt_ioctl_get_key_status_t)(struct file *filp, void __user *arg);
typedef int (*fsverity_ioctl_measure_t)(struct file *filp, void __user *arg);
typedef int (*fsverity_ioctl_read_metadata_t)(struct file *filp, const void __user *uarg);
typedef int (*fscrypt_ioctl_set_policy_t)(struct file *filp, const void __user *arg);
typedef int (*fscrypt_ioctl_add_key_t)(struct file *filp, void __user *arg);
typedef int (*fscrypt_ioctl_get_policy_t)(struct file *filp, void __user *arg);
typedef int (*fscrypt_ioctl_get_policy_ex_t)(struct file *filp, void __user *arg);
typedef int (*fsverity_ioctl_enable_t)(struct file *filp, const void __user *arg);
typedef int (*__dquot_alloc_space_t)(struct inode *inode, qsize_t number, int flags);
typedef void (*dquot_claim_space_nodirty_t)(struct inode *inode, qsize_t number);
typedef loff_t (*generic_file_llseek_size_t)(struct file *file, loff_t offset,
		int whence, loff_t maxsize, loff_t eof);
typedef bool (*xa_get_mark_t)(struct xarray *, unsigned long index, xa_mark_t);
typedef struct iomap_dio *(*__iomap_dio_rw_t)(struct kiocb *iocb, struct iov_iter *iter,
		const struct iomap_ops *ops, const struct iomap_dio_ops *dops,
		unsigned int dio_flags, void *private, size_t done_before);
typedef ssize_t (*iomap_dio_complete_t)(struct iomap_dio *dio);
typedef int (*fscrypt_file_open_t)(struct inode *inode, struct file *filp);
typedef int (*__fsverity_file_open_t)(struct inode *inode, struct file *filp);
typedef int (*dquot_file_open_t)(struct inode *inode, struct file *file);
typedef int (*generic_fadvise_t)(struct file *file, loff_t offset, loff_t len,
			   int advice);
typedef struct backing_dev_info *(*inode_to_bdi_t)(struct inode *inode);
typedef int (*blkdev_issue_secure_erase_t)(struct block_device *bdev, sector_t sector,
		sector_t nr_sects, gfp_t gfp);
typedef int (*fscrypt_zeroout_range_t)(const struct inode *inode, pgoff_t lblk,
			  sector_t pblk, unsigned int len);
typedef int (*iocb_bio_iopoll_t)(struct kiocb *kiocb, struct io_comp_batch *iob,
			unsigned int flags);
typedef bool (*dquot_initialize_needed_t)(struct inode *inode);
typedef void (*dquot_drop_t)(struct inode *inode);
typedef void (*fscrypt_put_encryption_info_t)(struct inode *inode);
typedef void (*__fsverity_cleanup_inode_t)(struct inode *inode);
typedef void (*d_tmpfile_t)(struct file *, struct inode *);
typedef const char *(*fscrypt_get_symlink_t)(struct inode *inode, const void *caddr,
				unsigned int max_size,
				struct delayed_call *done);
typedef int (*fscrypt_symlink_getattr_t)(const struct path *path, struct kstat *stat);
typedef int (*fscrypt_has_permitted_context_t)(struct inode *parent, struct inode *child);
typedef void (*d_instantiate_new_t)(struct dentry *, struct inode *);
typedef int (*__fscrypt_prepare_link_t)(struct inode *inode, struct inode *dir,
			   struct dentry *dentry);
typedef int (*fscrypt_prepare_symlink_t)(struct inode *dir, const char *target,
			    unsigned int len, unsigned int max_len,
			    struct fscrypt_str *disk_link);
typedef int (*__fscrypt_encrypt_symlink_t)(struct inode *inode, const char *target,
			      unsigned int len, struct fscrypt_str *disk_link);
typedef int (*__fscrypt_prepare_rename_t)(struct inode *old_dir, struct dentry *old_dentry,
			     struct inode *new_dir, struct dentry *new_dentry,
			     unsigned int flags);
typedef int (*fscrypt_prepare_new_inode_t)(struct inode *dir, struct inode *inode,
			      bool *encrypt_ret);
typedef u64 (*fscrypt_fname_siphash_t)(const struct inode *dir, const struct qstr *name);
typedef int (*fscrypt_parse_test_dummy_encryption_t)(const struct fs_parameter *param,
				    struct fscrypt_dummy_policy *dummy_policy);
typedef void (*fscrypt_free_inode_t)(struct inode *inode);
typedef int (*fscrypt_drop_inode_t)(struct inode *inode);
typedef void (*fscrypt_show_test_dummy_encryption_t)(struct seq_file *seq, char sep,
					struct super_block *sb);
typedef void (*fscrypt_set_bio_crypt_ctx_t)(struct bio *bio,
			       const struct inode *inode, u64 first_lblk,
			       gfp_t gfp_mask);
typedef bool (*fscrypt_mergeable_bio_t)(struct bio *bio, const struct inode *inode,
			   u64 next_lblk);
typedef struct page *(*fscrypt_encrypt_pagecache_blocks_t)(struct page *page,
					      unsigned int len,
					      unsigned int offs,
					      gfp_t gfp_flags);
typedef u64 (*fscrypt_limit_io_blocks_t)(const struct inode *inode, u64 lblk, u64 nr_blocks);
typedef bool (*fscrypt_decrypt_bio_t)(struct bio *bio);
typedef int (*dquot_initialize_t)(struct inode *inode);
typedef int (*dquot_quota_on_mount_t)(struct super_block *sb, char *qf_name,
 	int format_id, int type);
typedef int (*dquot_load_quota_inode_t)(struct inode *inode, int type, int format_id,
	unsigned int flags);
typedef int (*dquot_quota_off_t)(struct super_block *sb, int type);
typedef int (*dquot_writeback_dquots_t)(struct super_block *sb, int type);
typedef int (*dquot_commit_t)(struct dquot *dquot);
typedef int (*dquot_acquire_t)(struct dquot *dquot);
typedef int (*dquot_release_t)(struct dquot *dquot);
typedef int (*dquot_mark_dquot_dirty_t)(struct dquot *dquot);
typedef int (*dquot_commit_info_t)(struct super_block *sb, int type);
typedef int (*dquot_quota_on_t)(struct super_block *sb, int type, int format_id,
	const struct path *path);
typedef int (*dquot_disable_t)(struct super_block *sb, int type, unsigned int flags);
typedef int (*dquot_resume_t)(struct super_block *sb, int type);
typedef struct dquot *(*dquot_alloc_t)(struct super_block *sb, int type);
typedef void (*dquot_destroy_t)(struct dquot *dquot);
typedef int (*dquot_get_next_id_t)(struct super_block *sb, struct kqid *qid);
typedef int (*dquot_set_dqinfo_t)(struct super_block *sb, int type, struct qc_info *ii);
typedef int (*dquot_get_dqblk_t)(struct super_block *sb, struct kqid id,
		struct qc_dqblk *di);
typedef int (*dquot_get_next_dqblk_t)(struct super_block *sb, struct kqid *id,
		struct qc_dqblk *di);
typedef int (*dquot_set_dqblk_t)(struct super_block *sb, struct kqid id,
		struct qc_dqblk *di);
typedef int (*dquot_get_state_t)(struct super_block *sb, struct qc_state *state);
typedef int (*dquot_alloc_inode_t)(struct inode *inode);
typedef void (*dquot_free_inode_t)(struct inode *inode);
typedef void (*bio_add_folio_nofail_t)(struct bio *bio, struct folio *folio, size_t len,
			  size_t off);
typedef void (*bio_associate_blkg_from_css_t)(struct bio *bio,
				 struct cgroup_subsys_state *css);
typedef void (*utf8_unload_t)(struct unicode_map *um);
typedef struct unicode_map *(*utf8_load_t)(unsigned int version);
typedef int (*blkdev_report_zones_t)(struct block_device *bdev, sector_t sector,
		unsigned int nr_zones, report_zones_cb cb, void *data);
typedef int (*blkdev_zone_mgmt_t)(struct block_device *bdev, enum req_op op,
		sector_t sectors, sector_t nr_sectors);
typedef const char *(*blk_zone_cond_str_t)(enum blk_zone_cond zone_cond);
typedef void (*percpu_counter_set_t)(struct percpu_counter *fbc, s64 amount);
typedef void (*generic_set_sb_d_ops_t)(struct super_block *sb);
typedef void (*evict_inodes_t)(struct super_block *sb);
typedef void (*shrink_dcache_sb_t)(struct super_block *);
typedef void (*__cleancache_init_fs_t)(struct super_block *);
typedef void (*sync_inodes_sb_t)(struct super_block *);
typedef int (*freeze_super_t)(struct super_block *super, enum freeze_holder who);
typedef int (*thaw_super_t)(struct super_block *super, enum freeze_holder who);
typedef void (*wbc_account_cgroup_owner_t)(struct writeback_control *wbc, struct folio *folio,
			      size_t bytes);
typedef void (*__xa_clear_mark_t)(struct xarray *, unsigned long index, xa_mark_t);
typedef int  (*__cleancache_get_page_t)(struct page *);
typedef struct inode *(*find_inode_nowait_t)(struct super_block *,
				       unsigned long,
				       int (*match)(struct inode *,
						    unsigned long, void *),
				       void *data);
typedef void (*folio_wait_stable_t)(struct folio *folio);
typedef int (*LZ4_compress_HC_t)(const char *src, char *dst, int srcSize, int dstCapacity,
	int compressionLevel, void *wrkmem);
typedef int (*LZ4_compress_default_t)(const char *source, char *dest, int inputSize,
	int maxOutputSize, void *wrkmem);
typedef int (*zstd_min_clevel_t)(void);
typedef int (*zstd_max_clevel_t)(void);
typedef void (*__filemap_set_wb_err_t)(struct address_space *mapping, int err);
typedef void (*__folio_batch_release_t)(struct folio_batch *pvec);
typedef void (*__folio_start_writeback_t)(struct folio *folio, bool keep_write);
typedef bool (*folio_clear_dirty_for_io_t)(struct folio *folio);
typedef void (*folio_end_writeback_t)(struct folio *folio);
typedef struct folio *(*__filemap_get_folio_t)(struct address_space *mapping, pgoff_t index,
		fgf_t fgp_flags, gfp_t gfp);
typedef bool (*folio_mark_dirty_t)(struct folio *folio);
typedef bool (*folio_redirty_for_writepage_t)(struct writeback_control *, struct folio *);
typedef void (*folio_unlock_t)(struct folio *folio);
typedef void (*folio_wait_bit_t)(struct folio *folio, int bit_nr);
typedef void (*folio_wait_writeback_t)(struct folio *folio);
typedef int (*__blkdev_issue_discard_t)(struct block_device *bdev, sector_t sector,
		sector_t nr_sects, gfp_t gfp_mask, struct bio **biop);
typedef u32 (*__get_random_u32_below_t)(u32 ceil);
typedef int (*__percpu_counter_init_many_t)(struct percpu_counter *fbc, s64 amount,
			       gfp_t gfp, u32 nr_counters,
			       struct lock_class_key *key);
typedef bool (*__percpu_down_read_t)(struct percpu_rw_semaphore *, bool);
typedef int (*_raw_write_trylock_t)(rwlock_t *lock);
typedef int (*add_swap_extent_t)(struct swap_info_struct *sis, unsigned long start_page,
		unsigned long nr_pages, sector_t start_block);
typedef int (*autoremove_wake_function_t)(struct wait_queue_entry *wq_entry, unsigned mode, int sync, void *key);
typedef struct file *(*bdev_file_open_by_path_t)(const char *path, blk_mode_t mode,
				    void *holder,
				    const struct blk_holder_ops *hops);
typedef void (*bdev_fput_t)(struct file *bdev_file);
typedef void (*bioset_exit_t)(struct bio_set *);
typedef int (*bioset_init_t)(struct bio_set *, unsigned int, unsigned int, int flags);
typedef int (*blk_status_to_errno_t)(blk_status_t status);
typedef int (*blkdev_issue_flush_t)(struct block_device *bdev);
typedef int (*blkdev_issue_zeroout_t)(struct block_device *bdev, sector_t sector,
		sector_t nr_sects, gfp_t gfp_mask, unsigned flags);
typedef bool (*clear_page_dirty_for_io_t)(struct page *page);
typedef void (*d_invalidate_t)(struct dentry *);
typedef ino_t (*d_parent_ino_t)(struct dentry *dentry);
typedef void (*end_page_writeback_t)(struct page *page);
typedef errseq_t (*errseq_set_t)(errseq_t *eseq, int err);
typedef struct fd (*fdget_t)(unsigned int fd);
typedef struct block_device *(*file_bdev_t)(struct file *bdev_file);
typedef int (*file_modified_t)(struct file *file);
typedef int (*file_write_and_wait_range_t)(struct file *file,
						loff_t start, loff_t end);
typedef void (*fileattr_fill_flags_t)(struct fileattr *fa, u32 flags);
typedef int (*filemap_check_errors_t)(struct address_space *mapping);
typedef bool (*filemap_dirty_folio_t)(struct address_space *mapping, struct folio *folio);
typedef vm_fault_t (*filemap_fault_t)(struct vm_fault *vmf);
typedef vm_fault_t (*filemap_map_pages_t)(struct vm_fault *vmf,
		pgoff_t start_pgoff, pgoff_t end_pgoff);
typedef int (*filemap_migrate_folio_t)(struct address_space *mapping, struct folio *dst,
		struct folio *src, enum migrate_mode mode);
typedef ssize_t (*filemap_splice_read_t)(struct file *in, loff_t *ppos,
			    struct pipe_inode_info *pipe,
			    size_t len, unsigned int flags);
typedef int (*finish_open_t)(struct file *file, struct dentry *dentry,
			int (*open)(struct inode *, struct file *));
typedef void (*flush_dcache_folio_t)(struct folio *folio);
typedef void (*folio_unlock_t)(struct folio *folio);
typedef void (*generate_random_uuid_t)(unsigned char uuid[16]);
typedef int (*generic_encode_ino32_fh_t)(struct inode *inode, __u32 *fh, int *max_len,
			    struct inode *parent);
typedef int (*generic_error_remove_folio_t)(struct address_space *mapping,
		struct folio *folio);
typedef ssize_t (*generic_perform_write_t)(struct kiocb *, struct iov_iter *);
typedef struct inode *(*iget_locked_t)(struct super_block *, unsigned long);
typedef struct inode *(*igrab_t)(struct inode *);
typedef int (*in_group_p_t)(kgid_t);
typedef struct timespec64 (*inode_set_ctime_current_t)(struct inode *inode);
typedef void (*inode_set_flags_t)(struct inode *inode, unsigned int flags,
			    unsigned int mask);
typedef int (*invalidate_inode_pages2_range_t)(struct address_space *mapping,
		pgoff_t start, pgoff_t end);
typedef unsigned long (*invalidate_mapping_pages_t)(struct address_space *mapping,
					pgoff_t start, pgoff_t end);
typedef long (*io_schedule_timeout_t)(long timeout);
typedef void *(*kmem_cache_alloc_lru_noprof_t)(struct kmem_cache *s, struct list_lru *lru,
			    gfp_t gfpflags);
typedef bool (*llist_add_batch_t)(struct llist_node *new_first,
			    struct llist_node *new_last,
			    struct llist_head *head);
typedef struct llist_node *(*llist_reverse_order_t)(struct llist_node *head);
typedef vfsgid_t (*make_vfsgid_t)(struct mnt_idmap *idmap,
		     struct user_namespace *fs_userns, kgid_t kgid);
typedef vfsuid_t (*make_vfsuid_t)(struct mnt_idmap *idmap,
		     struct user_namespace *fs_userns, kuid_t kuid);
typedef int (*match_int_t)(substring_t *, int *result);
typedef char *(*match_strdup_t)(const substring_t *);
typedef int (*match_token_t)(char *, const match_table_t table, substring_t args[]);
typedef void *(*mempool_alloc_noprof_t)(mempool_t *pool, gfp_t gfp_mask);
typedef void *(*mempool_alloc_slab_t)(gfp_t gfp_mask, void *pool_data);
typedef void (*mempool_destroy_t)(mempool_t *pool);
typedef void (*mempool_free_slab_t)(void *element, void *pool_data);
typedef void (*mnt_drop_write_file_t)(struct file *file);
typedef int (*mnt_want_write_file_t)(struct file *file);
typedef struct dentry *(*mount_bdev_t)(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data,
	int (*fill_super)(struct super_block *, void *, int));
typedef int (*page_symlink_t)(struct inode *inode, const char *symname, int len);
typedef void (*mempool_free_g)(void *element, mempool_t *pool);
typedef void (*percpu_counter_destroy_many_t)(struct percpu_counter *fbc, u32 nr_counters);
typedef void (*prepare_to_wait_t)(struct wait_queue_head *wq_head, struct wait_queue_entry *wq_entry, int state);
typedef unsigned int (*radix_tree_gang_lookup_t)(const struct radix_tree_root *,
			void **results, unsigned long first_index,
			unsigned int max_items);
typedef int (*radix_tree_preload_t)(gfp_t gfp_mask);
typedef int (*rcuwait_wake_up_t)(struct rcuwait *w);
typedef bool (*redirty_page_for_writepage_t)(struct writeback_control *, struct page *);
typedef void (*release_pages_t)(release_pages_arg, int nr);
typedef int (*remove_proc_subtree_t)(const char *, struct proc_dir_entry *);
typedef int (*security_inode_init_security_t)(struct inode *inode, struct inode *dir,
				 const struct qstr *qstr,
				 initxattrs initxattrs, void *fs_data);
typedef void (*seq_escape_mem_t)(struct seq_file *m, const char *src, size_t len,
		    unsigned int flags, const char *esc);
typedef void (*set_cached_acl_t)(struct inode *inode, int type, struct posix_acl *acl);
typedef void (*set_page_writeback_t)(struct page *page);
typedef int (*set_task_ioprio_t)(struct task_struct *task, int ioprio);
typedef struct timespec64 (*simple_inode_init_ts_t)(struct inode *inode);
typedef char *(*strndup_user_t)(const char __user *, long);
typedef void (*tag_pages_for_writeback_t)(struct address_space *mapping,
			     pgoff_t start, pgoff_t end);
typedef void (*touch_atime_t)(const struct path *);
typedef void (*truncate_inode_pages_t)(struct address_space *, loff_t);
typedef void (*truncate_inode_pages_range_t)(struct address_space *,
				       loff_t lstart, loff_t lend);
typedef void (*truncate_pagecache_range_t)(struct inode *inode, loff_t offset, loff_t end);
typedef int (*utf8s_to_utf16s_t)(const u8 *s, int len,
		enum utf16_endian endian, wchar_t *pwcs, int maxlen);
typedef loff_t (*vfs_setpos_t)(struct file *file, loff_t offset, loff_t maxsize);
typedef void (*wait_for_completion_io_t)(struct completion *);

EXT_FUN_TYPE1(wait_for_completion_io);
EXT_FUN_TYPE1(vfs_setpos);
EXT_FUN_TYPE1(utf8s_to_utf16s);
EXT_FUN_TYPE1(truncate_pagecache_range);
EXT_FUN_TYPE1(truncate_inode_pages_range);
EXT_FUN_TYPE1(truncate_inode_pages);
EXT_FUN_TYPE1(touch_atime);
EXT_FUN_TYPE1(tag_pages_for_writeback);
EXT_FUN_TYPE1(strndup_user);
EXT_FUN_TYPE1(simple_inode_init_ts);
EXT_FUN_TYPE1(set_task_ioprio);
EXT_FUN_TYPE1(set_page_writeback);
EXT_FUN_TYPE1(set_cached_acl);
EXT_FUN_TYPE1(seq_escape_mem);
EXT_FUN_TYPE1(security_inode_init_security);
EXT_FUN_TYPE1(remove_proc_subtree);
EXT_FUN_TYPE1(release_pages);
EXT_FUN_TYPE1(redirty_page_for_writepage);
EXT_FUN_TYPE1(rcuwait_wake_up);
EXT_FUN_TYPE1(radix_tree_preload);
EXT_FUN_TYPE1(radix_tree_gang_lookup);
EXT_FUN_TYPE1(prepare_to_wait);
EXT_FUN_TYPE1(percpu_counter_destroy_many);
EXT_FUN_TYPE1(page_symlink);
EXT_FUN_TYPE1(mount_bdev);
EXT_FUN_TYPE1(mnt_want_write_file);
EXT_FUN_TYPE1(mnt_drop_write_file);
EXT_FUN_TYPE1(mempool_free_slab);
EXT_FUN_TYPEG(mempool_free);
EXT_FUN_TYPE1(mempool_destroy);
EXT_FUN_TYPE1(mempool_alloc_slab);
EXT_FUN_TYPE1(mempool_alloc_noprof);
EXT_FUN_TYPE1(match_token);
EXT_FUN_TYPE1(match_strdup);
EXT_FUN_TYPE1(match_int);
EXT_FUN_TYPE1(make_vfsuid);
EXT_FUN_TYPE1(make_vfsuid);
EXT_FUN_TYPE1(make_vfsgid);
EXT_FUN_TYPE1(llist_reverse_order);
EXT_FUN_TYPE1(llist_add_batch);
EXT_FUN_TYPE1(kmem_cache_alloc_lru_noprof);
EXT_FUN_TYPE1(io_schedule_timeout);
EXT_FUN_TYPE1(invalidate_mapping_pages);
EXT_FUN_TYPE1(invalidate_inode_pages2_range);
EXT_FUN_TYPE1(inode_set_flags);
EXT_FUN_TYPE1(inode_set_ctime_current);
EXT_FUN_TYPE1(in_group_p);
EXT_FUN_TYPE1(igrab);
EXT_FUN_TYPE1(iget_locked);
EXT_FUN_TYPE1(generic_perform_write);
EXT_FUN_TYPE1(generic_error_remove_folio);
EXT_FUN_TYPE1(generic_encode_ino32_fh);
EXT_FUN_TYPE1(generate_random_uuid);
EXT_FUN_TYPE1(folio_unlock);
EXT_FUN_TYPE1(flush_dcache_folio);
EXT_FUN_TYPE1(finish_open);
EXT_FUN_TYPE1(filemap_splice_read);
EXT_FUN_TYPE1(filemap_migrate_folio);
EXT_FUN_TYPE1(filemap_map_pages);
EXT_FUN_TYPE1(filemap_fault);
EXT_FUN_TYPE1(filemap_dirty_folio);
EXT_FUN_TYPE1(filemap_check_errors);
EXT_FUN_TYPE1(fileattr_fill_flags);
EXT_FUN_TYPE1(file_write_and_wait_range);
EXT_FUN_TYPE1(file_modified);
EXT_FUN_TYPE1(file_bdev);
EXT_FUN_TYPE1(fdget);
EXT_FUN_TYPE1(errseq_set);
EXT_FUN_TYPE1(end_page_writeback);
EXT_FUN_TYPE1(d_parent_ino);
EXT_FUN_TYPE1(d_invalidate);
EXT_FUN_TYPE1(clear_page_dirty_for_io);
EXT_FUN_TYPE1(blkdev_issue_zeroout);
EXT_FUN_TYPE1(blkdev_issue_flush);
EXT_FUN_TYPE1(blk_status_to_errno);
EXT_FUN_TYPE1(bioset_init);
EXT_FUN_TYPE1(bioset_exit);
EXT_FUN_TYPE1(bdev_fput);
EXT_FUN_TYPE1(bdev_file_open_by_path);
EXT_FUN_TYPE1(autoremove_wake_function);
EXT_FUN_TYPE1(add_swap_extent);
EXT_FUN_TYPE1(_raw_write_trylock);
EXT_FUN_TYPE1(__percpu_down_read);
EXT_FUN_TYPE1(__percpu_counter_init_many);
EXT_FUN_TYPE1(__get_random_u32_below);
EXT_FUN_TYPE1(__blkdev_issue_discard);
EXT_FUN_TYPE1(folio_wait_writeback);
EXT_FUN_TYPE1(folio_wait_bit);
EXT_FUN_TYPE1(folio_redirty_for_writepage);
EXT_FUN_TYPE1(folio_mark_dirty);
EXT_FUN_TYPE1(folio_end_writeback);
EXT_FUN_TYPE1(folio_clear_dirty_for_io);
EXT_FUN_TYPE1(__folio_start_writeback);
EXT_FUN_TYPE1(__folio_batch_release);
EXT_FUN_TYPE1(__filemap_set_wb_err);
EXT_FUN_TYPE1(__folio_swap_cache_index);
EXT_FUN_TYPE1(__filemap_get_folio);
EXT_FUN_TYPE1(zstd_max_clevel);
EXT_FUN_TYPE1(zstd_min_clevel);
EXT_FUN_TYPE1(LZ4_compress_default);
EXT_FUN_TYPE1(LZ4_compress_HC);
EXT_FUN_TYPE1(folio_wait_stable);
EXT_FUN_TYPE1(find_inode_nowait);
EXT_FUN_TYPE1(__cleancache_get_page);
EXT_FUN_TYPE1(__xa_clear_mark);
EXT_FUN_TYPE1(wbc_account_cgroup_owner);
EXT_FUN_TYPE1(thaw_super);
EXT_FUN_TYPE1(freeze_super);
EXT_FUN_TYPE1(sync_inodes_sb);
EXT_FUN_TYPE1(__cleancache_init_fs);
EXT_FUN_TYPE1(shrink_dcache_sb);
EXT_FUN_TYPE1(evict_inodes);
EXT_FUN_TYPE1(generic_set_sb_d_ops);
EXT_FUN_TYPE1(percpu_counter_set);
EXT_FUN_TYPE1(blk_zone_cond_str);
EXT_FUN_TYPE1(blkdev_zone_mgmt);
EXT_FUN_TYPE1(blkdev_report_zones);
EXT_FUN_TYPE1(utf8_load);
EXT_FUN_TYPE1(utf8_unload);
EXT_FUN_TYPE1(bio_associate_blkg_from_css);
EXT_FUN_TYPE1(bio_add_folio_nofail);
EXT_FUN_TYPE1(dquot_free_inode);
EXT_FUN_TYPE1(dquot_alloc_inode);
EXT_FUN_TYPE1(dquot_get_state);
EXT_FUN_TYPE1(dquot_set_dqblk);
EXT_FUN_TYPE1(dquot_get_next_dqblk);
EXT_FUN_TYPE1(dquot_get_dqblk);
EXT_FUN_TYPE1(dquot_set_dqinfo);
EXT_FUN_TYPE1(dquot_get_next_id);
EXT_FUN_TYPE1(dquot_destroy);
EXT_FUN_TYPE1(dquot_alloc);
EXT_FUN_TYPE1(dquot_resume);
EXT_FUN_TYPE1(dquot_disable);
EXT_FUN_TYPE1(dquot_quota_on);
EXT_FUN_TYPE1(dquot_commit_info);
EXT_FUN_TYPE1(dquot_mark_dquot_dirty);
EXT_FUN_TYPE1(dquot_acquire);
EXT_FUN_TYPE1(dquot_release);
EXT_FUN_TYPE1(dquot_commit);
EXT_FUN_TYPE1(dquot_writeback_dquots);
EXT_FUN_TYPE1(dquot_quota_off);
EXT_FUN_TYPE1(dquot_load_quota_inode);
EXT_FUN_TYPE1(dquot_quota_on_mount);
EXT_FUN_TYPE1(dquot_initialize);
EXT_FUN_TYPE1(fscrypt_decrypt_bio);
EXT_FUN_TYPE1(fscrypt_limit_io_blocks);
EXT_FUN_TYPE1(fscrypt_encrypt_pagecache_blocks);
EXT_FUN_TYPE1(fscrypt_mergeable_bio);
EXT_FUN_TYPE1(fscrypt_set_bio_crypt_ctx);
EXT_FUN_TYPE1(fscrypt_show_test_dummy_encryption);
EXT_FUN_TYPE1(fscrypt_drop_inode);
EXT_FUN_TYPE1(fscrypt_free_inode);
EXT_FUN_TYPE1(fscrypt_parse_test_dummy_encryption);
EXT_FUN_TYPE1(fscrypt_fname_siphash);
EXT_FUN_TYPE1(fscrypt_prepare_new_inode);
EXT_FUN_TYPE1(__fscrypt_prepare_rename);
EXT_FUN_TYPE1(__fscrypt_encrypt_symlink);
EXT_FUN_TYPE1(fscrypt_prepare_symlink);
EXT_FUN_TYPE1(__fscrypt_prepare_link);
EXT_FUN_TYPE1(d_instantiate_new);
EXT_FUN_TYPE1(fscrypt_has_permitted_context);
EXT_FUN_TYPE1(fscrypt_symlink_getattr);
EXT_FUN_TYPE1(fscrypt_get_symlink);
EXT_FUN_TYPE1(d_tmpfile);
EXT_FUN_TYPE1(__fsverity_cleanup_inode);
EXT_FUN_TYPE1(fscrypt_put_encryption_info);
EXT_FUN_TYPE1(dquot_drop);
EXT_FUN_TYPE1(dquot_initialize_needed);
EXT_FUN_TYPE1(iocb_bio_iopoll);
EXT_FUN_TYPE1(fscrypt_zeroout_range);
EXT_FUN_TYPE1(blkdev_issue_secure_erase);
EXT_FUN_TYPE1(inode_to_bdi);
EXT_FUN_TYPE1(generic_fadvise);
EXT_FUN_TYPE1(dquot_file_open);
EXT_FUN_TYPE1(__fsverity_file_open);
EXT_FUN_TYPE1(fscrypt_file_open);
EXT_FUN_TYPE1(iomap_dio_complete);
EXT_FUN_TYPE1(__iomap_dio_rw);
EXT_FUN_TYPE1(xa_get_mark);
EXT_FUN_TYPE1(generic_file_llseek_size);
EXT_FUN_TYPE1(dquot_claim_space_nodirty);
EXT_FUN_TYPE1(__dquot_alloc_space);
EXT_FUN_TYPE1(fsverity_ioctl_enable);
EXT_FUN_TYPE1(fscrypt_ioctl_get_policy_ex);
EXT_FUN_TYPE1(fscrypt_ioctl_get_policy);
EXT_FUN_TYPE1(fscrypt_ioctl_add_key);
EXT_FUN_TYPE1(fscrypt_ioctl_set_policy);
EXT_FUN_TYPE1(fsverity_ioctl_read_metadata);
EXT_FUN_TYPE1(fsverity_ioctl_measure);
EXT_FUN_TYPE1(fscrypt_ioctl_get_key_status);
EXT_FUN_TYPE1(fscrypt_ioctl_get_nonce);
EXT_FUN_TYPE1(fscrypt_ioctl_remove_key);
EXT_FUN_TYPE1(fscrypt_ioctl_remove_key_all_users);
EXT_FUN_TYPE1(dqput);
EXT_FUN_TYPE1(__dquot_transfer);
EXT_FUN_TYPE1(dqget);
EXT_FUN_TYPE1(bdev_thaw);
EXT_FUN_TYPE1(bdev_freeze);
EXT_FUN_TYPE1(from_vfsuid);
EXT_FUN_TYPE1(from_vfsgid);
EXT_FUN_TYPE1(dquot_transfer);
EXT_FUN_TYPE1(__fsverity_prepare_setattr);
EXT_FUN_TYPE1(__fscrypt_prepare_setattr);
EXT_FUN_TYPE1(fscrypt_dio_supported);
EXT_FUN_TYPE1(fscrypt_match_name);
EXT_FUN_TYPE1(__dquot_free_space);
EXT_FUN_TYPE1(fscrypt_fname_free_buffer);
EXT_FUN_TYPE1(page_cache_sync_ra);
EXT_FUN_TYPE1(fscrypt_fname_alloc_buffer);
EXT_FUN_TYPE1(__fscrypt_prepare_readdir);
EXT_FUN_TYPE1(fscrypt_fname_disk_to_usr);
EXT_FUN_TYPE1(fscrypt_set_context);
EXT_FUN_TYPE1(__percpu_counter_sum);
EXT_FUN_TYPE1(inode_owner_or_capable);
EXT_FUN_TYPE1(posix_acl_alloc);
EXT_FUN_TYPE1(posix_acl_equiv_mode);
EXT_FUN_TYPE1(in_group_or_capable);
EXT_FUN_TYPE1(page_cache_ra_unbounded);
EXT_FUN_TYPE1(mempool_alloc_pages);
EXT_FUN_TYPE1(mempool_free_pages);
EXT_FUN_TYPE1(mempool_create_node_noprof);
EXT_FUN_TYPE1(fsverity_enqueue_verify_work);
EXT_FUN_TYPE1(__fscrypt_inode_uses_inline_crypto);
EXT_FUN_TYPE1(fscrypt_free_bounce_page);
EXT_FUN_TYPE1(fsverity_verify_blocks);
EXT_FUN_TYPE1(fsverity_verify_bio);
EXT_FUN_TYPE1(blk_op_str);
EXT_FUN_TYPE1(filemap_get_folios);
EXT_FUN_TYPE1(filemap_get_folios_tag);
EXT_FUN_TYPE1(f2fs_destroy_compress_cache);
EXT_FUN_TYPE1(f2fs_destroy_compress_mempool);
EXT_FUN_TYPE1(f2fs_destroy_bioset);
EXT_FUN_TYPE1(f2fs_destroy_bio_entry_cache);
EXT_FUN_TYPE1(f2fs_destroy_iostat_processing);
EXT_FUN_TYPE1(f2fs_destroy_post_read_processing);
EXT_FUN_TYPE1(f2fs_destroy_root_stats);
EXT_FUN_TYPE1(f2fs_exit_sysfs);
EXT_FUN_TYPE1(f2fs_destroy_garbage_collection_cache);
EXT_FUN_TYPE1(f2fs_destroy_extent_cache);
EXT_FUN_TYPE1(f2fs_destroy_recovery_cache);
EXT_FUN_TYPE1(f2fs_destroy_checkpoint_caches);
EXT_FUN_TYPE1(f2fs_destroy_segment_manager_caches);
EXT_FUN_TYPE1(f2fs_destroy_node_manager_caches);
EXT_FUN_TYPE1(utf8_casefold);
EXT_FUN_TYPE1(fscrypt_setup_filename);
EXT_FUN_TYPE1(fscrypt_d_revalidate);
EXT_FUN_TYPE1(__fscrypt_prepare_lookup);
EXT_FUN_TYPE1(generic_ci_match);
EXT_FUN_TYPE1(fs_umode_to_ftype);
EXT_OBJ_TYPE(struct kmem_cache, f2fs_cf_name_slab);
EXT_OBJ_TYPE(struct shrinker, f2fs_shrinker_info);
EXT_OBJ_TYPE(struct kmem_cache, f2fs_inode_cachep);
EXT_OBJ_TYPE(struct mnt_idmap, nop_mnt_idmap);
EXT_OBJ_TYPE(enum system_states, system_state);

static inline void f_file_accessed(struct file *file)
{
	if (!(file->f_flags & O_NOATIME))
		f_touch_atime(&file->f_path);
}

static inline void f_seq_escape_str(struct seq_file *m, const char *src,
				  unsigned int flags, const char *esc)
{
	f_seq_escape_mem(m, src, strlen(src), flags, esc);
}

static inline void f_seq_escape(struct seq_file *m, const char *s, const char *esc)
{
	f_seq_escape_str(m, s, ESCAPE_OCTAL, esc);
}

static inline void f_seq_show_option(struct seq_file *m, const char *name,
				   const char *value)
{
	seq_putc(m, ',');
	f_seq_escape(m, name, ",= \t\n\\");
	if (value) {
		seq_putc(m, '=');
		f_seq_escape(m, value, ", \t\n\\");
	}
}

static inline void f_percpu_up_read(struct percpu_rw_semaphore *sem)
{
	rwsem_release(&sem->dep_map, _RET_IP_);

	preempt_disable();
	/*
	 * Same as in percpu_down_read().
	 */
	if (likely(rcu_sync_is_idle(&sem->rss))) {
		this_cpu_dec(*sem->read_count);
	} else {
		/*
		 * slowpath; reader will only ever wake a single blocked
		 * writer.
		 */
		smp_mb(); /* B matches C */
		/*
		 * In other words, if they see our decrement (presumably to
		 * aggregate zero, as that is the only time it matters) they
		 * will also see our critical section.
		 */
		this_cpu_dec(*sem->read_count);
		f_rcuwait_wake_up(&sem->writer);
	}
	//_trace_android_vh_record_pcpu_rwsem_starttime(sem, 0);
	preempt_enable();
}

static inline void f___sb_end_write(struct super_block *sb, int level)
{
	f_percpu_up_read(sb->s_writers.rw_sem + level-1);
}

static inline void f_sb_end_write(struct super_block *sb)
{
	f___sb_end_write(sb, SB_FREEZE_WRITE);
}

static inline void f_sb_end_pagefault(struct super_block *sb)
{
	f___sb_end_write(sb, SB_FREEZE_PAGEFAULT);
}

static inline void f_sb_end_intwrite(struct super_block *sb)
{
	f___sb_end_write(sb, SB_FREEZE_FS);
}

static inline void f_percpu_counter_destroy(struct percpu_counter *fbc)
{
	f_percpu_counter_destroy_many(fbc, 1);
}

#define f_mempool_create_node(...)					\
	alloc_hooks(f_mempool_create_node_noprof(__VA_ARGS__))

#define f_mempool_create(_min_nr, _alloc_fn, _free_fn, _pool_data)	\
	f_mempool_create_node(_min_nr, _alloc_fn, _free_fn, _pool_data,	\
			    GFP_KERNEL, NUMA_NO_NODE)

#define f_mempool_create_slab_pool(_min_nr, _kc)			\
	f_mempool_create((_min_nr), f_mempool_alloc_slab, f_mempool_free_slab, (void *)(_kc))

#define f_mempool_alloc(...)						\
	alloc_hooks(f_mempool_alloc_noprof(__VA_ARGS__))

static inline vfsuid_t f_i_uid_into_vfsuid(struct mnt_idmap *idmap,
					 const struct inode *inode)
{
	return f_make_vfsuid(idmap, i_user_ns(inode), inode->i_uid);
}

static inline vfsgid_t f_i_gid_into_vfsgid(struct mnt_idmap *idmap,
					 const struct inode *inode)
{
	return f_make_vfsgid(idmap, i_user_ns(inode), inode->i_gid);
}

static inline bool f_i_gid_needs_update(struct mnt_idmap *idmap,
				      const struct iattr *attr,
				      const struct inode *inode)
{
	return ((attr->ia_valid & ATTR_GID) &&
		!vfsgid_eq(attr->ia_vfsgid,
			   f_i_gid_into_vfsgid(idmap, inode)));
}

static inline bool f_i_uid_needs_update(struct mnt_idmap *idmap,
				      const struct iattr *attr,
				      const struct inode *inode)
{
	return ((attr->ia_valid & ATTR_UID) &&
		!vfsuid_eq(attr->ia_vfsuid,
			   f_i_uid_into_vfsuid(idmap, inode)));
}

static inline bool f_llist_add(struct llist_node *new, struct llist_head *head)
{
	return f_llist_add_batch(new, new, head);
}

static inline void f_memalloc_retry_wait(gfp_t gfp_flags)
{
	/* We use io_schedule_timeout because waiting for memory
	 * typically included waiting for dirty pages to be
	 * written out, which requires IO.
	 */
	__set_current_state(TASK_UNINTERRUPTIBLE);
	gfp_flags = current_gfp_context(gfp_flags);
	if (gfpflags_allow_blocking(gfp_flags) &&
	    !(gfp_flags & __GFP_NORETRY))
		/* Probably waited already, no need for much more */
		f_io_schedule_timeout(1);
	else
		/* Probably didn't wait, and has now released a lock,
		 * so now is a good time to wait
		 */
		f_io_schedule_timeout(HZ/50);
}

static inline int f_finish_open_simple(struct file *file, int error)
{
	if (error)
		return error;

	return f_finish_open(file, file->f_path.dentry, NULL);
}

static inline void f_mapping_set_error(struct address_space *mapping, int error)
{
	if (likely(!error))
		return;

	/* Record in wb_err for checkers using errseq_t based tracking */
	f___filemap_set_wb_err(mapping, error);

	/* Record it in superblock */
	if (mapping->host)
		f_errseq_set(&mapping->host->i_sb->s_wb_err, error);

	/* Record it in flags for now, for legacy callers */
	if (error == -ENOSPC)
		set_bit(AS_ENOSPC, &mapping->flags);
	else
		set_bit(AS_EIO, &mapping->flags);
}

#define f_DEFINE_WAIT(name) DEFINE_WAIT_FUNC(name, f_autoremove_wake_function)

#define f_write_trylock(lock)	__cond_lock(lock, f__raw_write_trylock(lock))

#define f_percpu_counter_init_many(fbc, value, gfp, nr_counters)		\
	({								\
		static struct lock_class_key __key;			\
									\
		f___percpu_counter_init_many(fbc, value, gfp, nr_counters,\
					   &__key);			\
	})

#define f_percpu_counter_init(fbc, value, gfp)				\
	f_percpu_counter_init_many(fbc, value, gfp, 1)

static inline void f_percpu_down_read(struct percpu_rw_semaphore *sem)
{
	might_sleep();

	rwsem_acquire_read(&sem->dep_map, 0, 0, _RET_IP_);

	preempt_disable();
	/*
	 * We are in an RCU-sched read-side critical section, so the writer
	 * cannot both change sem->state from readers_fast and start checking
	 * counters while we are here. So if we see !sem->state, we know that
	 * the writer won't be checking until we're past the preempt_enable()
	 * and that once the synchronize_rcu() is done, the writer will see
	 * anything we did within this RCU-sched read-size critical section.
	 */
	if (likely(rcu_sync_is_idle(&sem->rss)))
		this_cpu_inc(*sem->read_count);
	else
		f___percpu_down_read(sem, false); /* Unconditional memory barrier */
	/*
	 * The preempt_enable() prevents the compiler from
	 * bleeding the critical section out.
	 */
	//_trace_android_vh_record_pcpu_rwsem_starttime(sem, jiffies);
	preempt_enable();
}

static inline bool f_percpu_down_read_trylock(struct percpu_rw_semaphore *sem)
{
	bool ret = true;

	preempt_disable();
	/*
	 * Same as in percpu_down_read().
	 */
	if (likely(rcu_sync_is_idle(&sem->rss)))
		this_cpu_inc(*sem->read_count);
	else
		ret = f___percpu_down_read(sem, true); /* Unconditional memory barrier */
	preempt_enable();
	/*
	 * The barrier() from preempt_enable() prevents the compiler from
	 * bleeding the critical section out.
	 */

	if (ret) {
		//_trace_android_vh_record_pcpu_rwsem_starttime(sem, jiffies);
		rwsem_acquire_read(&sem->dep_map, 0, 1, _RET_IP_);
	}

	return ret;
}

static inline void f___sb_start_write(struct super_block *sb, int level)
{
	f_percpu_down_read(sb->s_writers.rw_sem + level - 1);
}

static inline bool f___sb_start_write_trylock(struct super_block *sb, int level)
{
	return f_percpu_down_read_trylock(sb->s_writers.rw_sem + level - 1);
}

static inline bool f_sb_start_write_trylock(struct super_block *sb)
{
	return f___sb_start_write_trylock(sb, SB_FREEZE_WRITE);
}

static inline void f_sb_start_intwrite(struct super_block *sb)
{
	f___sb_start_write(sb, SB_FREEZE_FS);
}

static inline void f_sb_start_pagefault(struct super_block *sb)
{
	f___sb_start_write(sb, SB_FREEZE_PAGEFAULT);
}

static inline u32 f_get_random_u32_below(u32 ceil)
{
	if (!__builtin_constant_p(ceil))
		return f___get_random_u32_below(ceil);

	/*
	 * For the fast path, below, all operations on ceil are precomputed by
	 * the compiler, so this incurs no overhead for checking pow2, doing
	 * divisions, or branching based on integer size. The resultant
	 * algorithm does traditional reciprocal multiplication (typically
	 * optimized by the compiler into shifts and adds), rejecting samples
	 * whose lower half would indicate a range indivisible by ceil.
	 */
	BUILD_BUG_ON_MSG(!ceil, "get_random_u32_below() must take ceil > 0");
	if (ceil <= 1)
		return 0;
	for (;;) {
		if (ceil <= 1U << 8) {
			u32 mult = ceil * get_random_u8();
			if (likely(is_power_of_2(ceil) || (u8)mult >= (1U << 8) % ceil))
				return mult >> 8;
		} else if (ceil <= 1U << 16) {
			u32 mult = ceil * get_random_u16();
			if (likely(is_power_of_2(ceil) || (u16)mult >= (1U << 16) % ceil))
				return mult >> 16;
		} else {
			u64 mult = (u64)ceil * get_random_u32();
			if (likely(is_power_of_2(ceil) || (u32)mult >= -ceil % ceil))
				return mult >> 32;
		}
	}
}

static inline u32 f_get_random_u32_inclusive(u32 floor, u32 ceil)
{
	BUILD_BUG_ON_MSG(__builtin_constant_p(floor) && __builtin_constant_p(ceil) &&
			 (floor > ceil || ceil - floor == U32_MAX),
			 "get_random_u32_inclusive() must take floor <= ceil");
	return floor + f_get_random_u32_below(ceil - floor + 1);
}

static inline void f_folio_wait_locked(struct folio *folio)
{
	if (folio_test_locked(folio))
		f_folio_wait_bit(folio, PG_locked);
}

#define f_folio_start_writeback(folio)			\
	f___folio_start_writeback(folio, false)

static inline pgoff_t f_folio_index(struct folio *folio)
{
	if (unlikely(folio_test_swapcache(folio)))
		return f___folio_swap_cache_index(folio);
	return folio->index;
}

static inline void f_folio_batch_release(struct folio_batch *fbatch)
{
	if (folio_batch_count(fbatch))
		f___folio_batch_release(fbatch);
}

static inline struct folio *f_filemap_grab_folio(struct address_space *mapping,
					pgoff_t index)
{
	return f___filemap_get_folio(mapping, index,
			FGP_LOCK | FGP_ACCESSED | FGP_CREAT,
			mapping_gfp_mask(mapping));
}

static inline struct folio *f_filemap_lock_folio(struct address_space *mapping,
					pgoff_t index)
{
	return f___filemap_get_folio(mapping, index, FGP_LOCK, 0);
}


static inline int f_cleancache_get_page(struct page *page)
{
	if (cleancache_enabled && cleancache_fs_enabled(page))
		return f___cleancache_get_page(page);
	return -1;
}

static inline void f_cleancache_init_fs(struct super_block *sb)
{
	if (cleancache_enabled)
		f___cleancache_init_fs(sb);
}

static inline void f_wbc_init_bio(struct writeback_control *wbc, struct bio *bio)
{
	/*
	 * pageout() path doesn't attach @wbc to the inode being written
	 * out.  This is intentional as we don't want the function to block
	 * behind a slow cgroup.  Ultimately, we want pageout() to kick off
	 * regular writeback instead of writing things out itself.
	 */
	if (wbc->wb)
		f_bio_associate_blkg_from_css(bio, wbc->wb->blkcg_css);
}

/* Suspend quotas on remount RO */
static inline int f_dquot_suspend(struct super_block *sb, int type)
{
	return f_dquot_disable(sb, type, DQUOT_SUSPENDED);
}

static inline int f_fscrypt_prepare_rename(struct inode *old_dir,
					 struct dentry *old_dentry,
					 struct inode *new_dir,
					 struct dentry *new_dentry,
					 unsigned int flags)
{
	if (IS_ENCRYPTED(old_dir) || IS_ENCRYPTED(new_dir))
		return f___fscrypt_prepare_rename(old_dir, old_dentry,
						new_dir, new_dentry, flags);
	return 0;
}

static inline int f_fscrypt_encrypt_symlink(struct inode *inode,
					  const char *target,
					  unsigned int len,
					  struct fscrypt_str *disk_link)
{
	if (IS_ENCRYPTED(inode))
		return f___fscrypt_encrypt_symlink(inode, target, len, disk_link);
	return 0;
}

static inline int f_fscrypt_prepare_link(struct dentry *old_dentry,
				       struct inode *dir,
				       struct dentry *dentry)
{
	if (IS_ENCRYPTED(dir))
		return f___fscrypt_prepare_link(d_inode(old_dentry), dir, dentry);
	return 0;
}

static inline s64 f_percpu_counter_sum_positive(struct percpu_counter *fbc)
{
	s64 ret = f___percpu_counter_sum(fbc);
	return ret < 0 ? 0 : ret;
}

static inline mempool_t *f_mempool_create_page_pool(int min_nr, int order)
{
	return f_mempool_create_node_noprof(min_nr, f_mempool_alloc_pages, f_mempool_free_pages,
				(void *)(long)order, GFP_KERNEL, NUMA_NO_NODE);
}

static inline bool f_fscrypt_inode_uses_inline_crypto(const struct inode *inode)
{
	return fscrypt_needs_contents_encryption(inode) &&
	       f___fscrypt_inode_uses_inline_crypto(inode);
}

static inline bool f_fscrypt_inode_uses_fs_layer_crypto(const struct inode *inode)
{
	return fscrypt_needs_contents_encryption(inode) &&
	       !f___fscrypt_inode_uses_inline_crypto(inode);
}

static inline void f_fscrypt_finalize_bounce_page(struct page **pagep)
{
	struct page *page = *pagep;

	if (fscrypt_is_bounce_page(page)) {
		*pagep = fscrypt_pagecache_page(page);
		f_fscrypt_free_bounce_page(page);
	}
}

static inline bool f_fsverity_verify_folio(struct folio *folio)
{
	return f_fsverity_verify_blocks(folio, folio_size(folio), 0);
}

static inline bool f_fsverity_verify_page(struct page *page)
{
	return f_fsverity_verify_blocks(page_folio(page), PAGE_SIZE, 0);
}

static inline void f_fscrypt_prepare_dentry(struct dentry *dentry,
					  bool is_nokey_name)
{
	/*
	 * This code tries to only take ->d_lock when necessary to write
	 * to ->d_flags.  We shouldn't be peeking on d_flags for
	 * DCACHE_OP_REVALIDATE unlocked, but in the unlikely case
	 * there is a race, the worst it can happen is that we fail to
	 * unset DCACHE_OP_REVALIDATE and pay the cost of an extra
	 * d_revalidate.
	 */
	if (is_nokey_name) {
		spin_lock(&dentry->d_lock);
		dentry->d_flags |= DCACHE_NOKEY_NAME;
		spin_unlock(&dentry->d_lock);
	} else if (dentry->d_flags & DCACHE_OP_REVALIDATE &&
		   dentry->d_op->d_revalidate == f_fscrypt_d_revalidate) {
		/*
		 * Unencrypted dentries and encrypted dentries where the
		 * key is available are always valid from fscrypt
		 * perspective. Avoid the cost of calling
		 * fscrypt_d_revalidate unnecessarily.
		 */
		spin_lock(&dentry->d_lock);
		dentry->d_flags &= ~DCACHE_OP_REVALIDATE;
		spin_unlock(&dentry->d_lock);
	}
}

static inline int f_fscrypt_prepare_lookup(struct inode *dir,
					 struct dentry *dentry,
					 struct fscrypt_name *fname)
{
	if (IS_ENCRYPTED(dir))
		return f___fscrypt_prepare_lookup(dir, dentry, fname);

	memset(fname, 0, sizeof(*fname));
	fname->usr_fname = &dentry->d_name;
	fname->disk_name.name = (unsigned char *)dentry->d_name.name;
	fname->disk_name.len = dentry->d_name.len;

	f_fscrypt_prepare_dentry(dentry, false);

	return 0;
}

static inline int f_fscrypt_prepare_readdir(struct inode *dir)
{
	if (IS_ENCRYPTED(dir))
		return f___fscrypt_prepare_readdir(dir);
	return 0;
}

static inline
void f_page_cache_sync_readahead(struct address_space *mapping,
		struct file_ra_state *ra, struct file *file, pgoff_t index,
		unsigned long req_count)
{
	DEFINE_READAHEAD(ractl, file, ra, mapping, index);
	f_page_cache_sync_ra(&ractl, req_count);
}

static inline void f_dquot_release_reservation_block(struct inode *inode,
		qsize_t nr)
{
	f___dquot_free_space(inode, nr << inode->i_blkbits, DQUOT_SPACE_RESERVE);
}

static inline int f_fscrypt_prepare_setattr(struct dentry *dentry,
					  struct iattr *attr)
{
	if (IS_ENCRYPTED(d_inode(dentry)))
		return f___fscrypt_prepare_setattr(dentry, attr);
	return 0;
}

static inline int f_fsverity_prepare_setattr(struct dentry *dentry,
					   struct iattr *attr)
{
	if (IS_VERITY(d_inode(dentry)))
		return f___fsverity_prepare_setattr(dentry, attr);
	return 0;
}

static inline void f_i_gid_update(struct mnt_idmap *idmap,
				const struct iattr *attr,
				struct inode *inode)
{
	if (attr->ia_valid & ATTR_GID)
		inode->i_gid = f_from_vfsgid(idmap, i_user_ns(inode),
					   attr->ia_vfsgid);
}

static inline void f_i_uid_update(struct mnt_idmap *idmap,
				const struct iattr *attr,
				struct inode *inode)
{
	if (attr->ia_valid & ATTR_UID)
		inode->i_uid = f_from_vfsuid(idmap, i_user_ns(inode),
					   attr->ia_vfsuid);
}

static inline bool f_is_quota_modification(struct mnt_idmap *idmap,
					 struct inode *inode, struct iattr *ia)
{
	return ((ia->ia_valid & ATTR_SIZE) ||
		f_i_uid_needs_update(idmap, ia, inode) ||
		f_i_gid_needs_update(idmap, ia, inode));
}

static inline void f_dquot_free_space_nodirty(struct inode *inode, qsize_t nr)
{
	f___dquot_free_space(inode, nr, 0);
}

static inline void f_dquot_free_space(struct inode *inode, qsize_t nr)
{
	f_dquot_free_space_nodirty(inode, nr);
	mark_inode_dirty_sync(inode);
}

static inline void f_dquot_free_block(struct inode *inode, qsize_t nr)
{
	f_dquot_free_space(inode, nr << inode->i_blkbits);
}

static inline int f_dquot_reserve_block(struct inode *inode, qsize_t nr)
{
	return f___dquot_alloc_space(inode, nr << inode->i_blkbits,
				DQUOT_SPACE_WARN|DQUOT_SPACE_RESERVE);
}

static inline void f_dquot_alloc_space_nofail(struct inode *inode, qsize_t nr)
{
	f___dquot_alloc_space(inode, nr, DQUOT_SPACE_WARN|DQUOT_SPACE_NOFAIL);
	mark_inode_dirty_sync(inode);
}

static inline void f_dquot_alloc_block_nofail(struct inode *inode, qsize_t nr)
{
	f_dquot_alloc_space_nofail(inode, nr << inode->i_blkbits);
}

static inline void f_dquot_claim_block(struct inode *inode, qsize_t nr)
{
	f_dquot_claim_space_nodirty(inode, nr << inode->i_blkbits);
	mark_inode_dirty_sync(inode);
}

static inline int f_fsverity_file_open(struct inode *inode, struct file *filp)
{
	if (IS_VERITY(inode))
		return f___fsverity_file_open(inode, filp);
	return 0;
}

static inline void f_fsverity_cleanup_inode(struct inode *inode)
{
	if (inode->i_verity_info)
		f___fsverity_cleanup_inode(inode);
}

/*---------- erofs symbols ------------*/
typedef uint32_t (*xxh32_t)(const void *input, const size_t len, const uint32_t seed);
typedef int (*filemap_add_folio_t)(struct address_space *mapping, struct folio *folio,
				pgoff_t index, gfp_t gfp);
typedef void (*psi_memstall_enter_t)(unsigned long *flags);
typedef void (*psi_memstall_leave_t)(unsigned long *flags);
typedef void (*migrate_disable_t)(void);
typedef void (*migrate_enable_t)(void);
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

/*--------- erofs symbols ----------*/
EXT_FUN_TYPE1(xxh32);
EXT_FUN_TYPE1(filemap_add_folio);
EXT_FUN_TYPE1(psi_memstall_leave);
EXT_FUN_TYPE1(psi_memstall_enter);
EXT_FUN_TYPE1(migrate_disable);
EXT_FUN_TYPE1(migrate_enable);
EXT_FUN_TYPE1(alloc_pages_bulk_noprof);
EXT_FUN_TYPE1(lockref_get_not_zero);
EXT_FUN_TYPE1(lockref_put_or_lock);
EXT_FUN_TYPE1(lockref_mark_dead);
EXT_FUN_TYPE1(bio_init);
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
EXT_FUN_TYPE1(copy_highpage);
EXT_FUN_TYPE1(iov_iter_bvec);
EXT_FUN_TYPE1(kmemdup_nul);
EXT_FUN_TYPE1(memchr_inv);
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
