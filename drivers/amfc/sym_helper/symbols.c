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
#include <linux/xattr.h>

#include <linux/proc_fs.h>

/*--------- f2fs symbols -----------*/
FUN_TYPE1(__percpu_counter_sum);
FUN_TYPE1(inode_owner_or_capable);
FUN_TYPE1(posix_acl_alloc);
FUN_TYPE1(posix_acl_equiv_mode);
FUN_TYPE1(in_group_or_capable);
FUN_TYPE1(page_cache_ra_unbounded);
FUN_TYPE1(mempool_alloc_pages);
FUN_TYPE1(mempool_free_pages);
FUN_TYPE1(mempool_create_node_noprof);
FUN_TYPE1(fsverity_enqueue_verify_work);
FUN_TYPE1(__fscrypt_inode_uses_inline_crypto);
FUN_TYPE1(fscrypt_free_bounce_page);
FUN_TYPE1(fsverity_verify_blocks);
FUN_TYPE1(fsverity_verify_bio);
FUN_TYPE1(blk_op_str);
FUN_TYPE1(__filemap_get_folio);
FUN_TYPE1(filemap_get_folios);
FUN_TYPE1(filemap_get_folios_tag);
FUN_TYPE1(f2fs_destroy_compress_cache);
FUN_TYPE1(f2fs_destroy_compress_mempool);
FUN_TYPE1(f2fs_destroy_bioset);
FUN_TYPE1(f2fs_destroy_bio_entry_cache);
FUN_TYPE1(f2fs_destroy_iostat_processing);
FUN_TYPE1(f2fs_destroy_post_read_processing);
FUN_TYPE1(f2fs_destroy_root_stats);
FUN_TYPE1(f2fs_exit_sysfs);
FUN_TYPE1(f2fs_destroy_garbage_collection_cache);
FUN_TYPE1(f2fs_destroy_extent_cache);
FUN_TYPE1(f2fs_destroy_recovery_cache);
FUN_TYPE1(f2fs_destroy_checkpoint_caches);
FUN_TYPE1(f2fs_destroy_segment_manager_caches);
FUN_TYPE1(f2fs_destroy_node_manager_caches);
FUN_TYPE1(utf8_casefold);
FUN_TYPE1(fscrypt_setup_filename);
FUN_TYPE1(fscrypt_d_revalidate);
FUN_TYPE1(__fscrypt_prepare_lookup);
FUN_TYPE1(zstd_max_clevel);
FUN_TYPE1(dquot_destroy);
FUN_TYPE1(dquot_resume);
FUN_TYPEG(mempool_free);
FUN_TYPE1(set_cached_acl);
FUN_TYPE1(wait_for_completion_io);
FUN_TYPE1(vfs_setpos);
FUN_TYPE1(utf8s_to_utf16s);
FUN_TYPE1(truncate_pagecache_range);
FUN_TYPE1(truncate_inode_pages_range);
FUN_TYPE1(truncate_inode_pages);
FUN_TYPE1(touch_atime);
FUN_TYPE1(tag_pages_for_writeback);
FUN_TYPE1(strndup_user);
FUN_TYPE1(simple_inode_init_ts);
FUN_TYPE1(set_task_ioprio);
FUN_TYPE1(set_page_writeback);
FUN_TYPE1(seq_escape_mem);
FUN_TYPE1(security_inode_init_security);
FUN_TYPE1(remove_proc_subtree);
FUN_TYPE1(release_pages);
FUN_TYPE1(redirty_page_for_writepage);
FUN_TYPE1(rcuwait_wake_up);
FUN_TYPE1(radix_tree_preload);
FUN_TYPE1(radix_tree_gang_lookup);
FUN_TYPE1(prepare_to_wait);
FUN_TYPE1(percpu_counter_destroy_many);
FUN_TYPE1(page_symlink);
FUN_TYPE1(mount_bdev);
FUN_TYPE1(mnt_want_write_file);
FUN_TYPE1(mnt_drop_write_file);
FUN_TYPE1(mempool_free_slab);
FUN_TYPE1(generic_ci_match);
FUN_TYPE1(mempool_alloc_slab);
FUN_TYPE1(mempool_alloc_noprof);
FUN_TYPE1(match_token);
FUN_TYPE1(match_strdup);
FUN_TYPE1(match_int);
FUN_TYPE1(make_vfsuid);
FUN_TYPE1(make_vfsgid);
FUN_TYPE1(llist_reverse_order);
FUN_TYPE1(mempool_destroy);
FUN_TYPE1(llist_add_batch);
FUN_TYPE1(io_schedule_timeout);
FUN_TYPE1(invalidate_mapping_pages);
FUN_TYPE1(invalidate_inode_pages2_range);
FUN_TYPE1(inode_set_flags);
FUN_TYPE1(inode_set_ctime_current);
FUN_TYPE1(in_group_p);
FUN_TYPE1(igrab);
FUN_TYPE1(iget_locked);
FUN_TYPE1(generic_perform_write);
FUN_TYPE1(generic_error_remove_folio);
FUN_TYPE1(generic_encode_ino32_fh);
FUN_TYPE1(generate_random_uuid);
FUN_TYPE1(folio_unlock);
FUN_TYPE1(flush_dcache_folio);
FUN_TYPE1(finish_open);
FUN_TYPE1(filemap_splice_read);
FUN_TYPE1(filemap_migrate_folio);
FUN_TYPE1(filemap_map_pages);
FUN_TYPE1(filemap_fault);
FUN_TYPE1(filemap_dirty_folio);
FUN_TYPE1(filemap_check_errors);
FUN_TYPE1(fileattr_fill_flags);
FUN_TYPE1(file_write_and_wait_range);
FUN_TYPE1(file_modified);
FUN_TYPE1(file_bdev);
FUN_TYPE1(fdget);
FUN_TYPE1(errseq_set);
FUN_TYPE1(end_page_writeback);
FUN_TYPE1(d_parent_ino);
FUN_TYPE1(d_invalidate);
FUN_TYPE1(clear_page_dirty_for_io);
FUN_TYPE1(blkdev_issue_zeroout);
FUN_TYPE1(blkdev_issue_flush);
FUN_TYPE1(blk_status_to_errno);
FUN_TYPE1(bioset_init);
FUN_TYPE1(bdev_fput);
FUN_TYPE1(bioset_exit);
FUN_TYPE1(autoremove_wake_function);
FUN_TYPE1(add_swap_extent);
FUN_TYPE1(_raw_write_trylock);
FUN_TYPE1(bdev_file_open_by_path);
FUN_TYPE1(__percpu_down_read);
FUN_TYPE1(__percpu_counter_init_many);
FUN_TYPE1(__get_random_u32_below);
FUN_TYPE1(__blkdev_issue_discard);
FUN_TYPE1(folio_wait_writeback);
FUN_TYPE1(folio_wait_bit);
FUN_TYPE1(folio_redirty_for_writepage);
FUN_TYPE1(folio_mark_dirty);
FUN_TYPE1(__folio_swap_cache_index);
FUN_TYPE1(fscrypt_match_name);
FUN_TYPE1(folio_end_writeback);
FUN_TYPE1(folio_clear_dirty_for_io);
FUN_TYPE1(__folio_start_writeback);
FUN_TYPE1(__filemap_set_wb_err);
FUN_TYPE1(dquot_get_next_id);
FUN_TYPE1(zstd_min_clevel);
FUN_TYPE1(LZ4_compress_default);
FUN_TYPE1(LZ4_compress_HC);
FUN_TYPE1(folio_wait_stable);
FUN_TYPE1(find_inode_nowait);
FUN_TYPE1(__folio_batch_release);
FUN_TYPE1(__cleancache_get_page);
FUN_TYPE1(__xa_clear_mark);
FUN_TYPE1(wbc_account_cgroup_owner);
FUN_TYPE1(thaw_super);
FUN_TYPE1(freeze_super);
FUN_TYPE1(sync_inodes_sb);
FUN_TYPE1(__cleancache_init_fs);
FUN_TYPE1(shrink_dcache_sb);
FUN_TYPE1(evict_inodes);
FUN_TYPE1(generic_set_sb_d_ops);
FUN_TYPE1(percpu_counter_set);
FUN_TYPE1(blk_zone_cond_str);
FUN_TYPE1(blkdev_zone_mgmt);
FUN_TYPE1(blkdev_report_zones);
FUN_TYPE1(utf8_load);
FUN_TYPE1(utf8_unload);
FUN_TYPE1(bio_associate_blkg_from_css);
FUN_TYPE1(bio_add_folio_nofail);
FUN_TYPE1(dquot_free_inode);
FUN_TYPE1(dquot_alloc_inode);
FUN_TYPE1(dquot_get_state);
FUN_TYPE1(dquot_set_dqblk);
FUN_TYPE1(dquot_get_next_dqblk);
FUN_TYPE1(dquot_get_dqblk);
FUN_TYPE1(dquot_set_dqinfo);
FUN_TYPE1(fs_umode_to_ftype);
FUN_TYPE1(fscrypt_set_context);
FUN_TYPE1(dquot_alloc);
FUN_TYPE1(fscrypt_fname_disk_to_usr);
FUN_TYPE1(dquot_disable);
FUN_TYPE1(dquot_commit_info);
FUN_TYPE1(dquot_mark_dquot_dirty);
FUN_TYPE1(dquot_acquire);
FUN_TYPE1(dquot_release);
FUN_TYPE1(dquot_commit);
FUN_TYPE1(dquot_writeback_dquots);
FUN_TYPE1(dquot_quota_off);
FUN_TYPE1(dquot_load_quota_inode);
FUN_TYPE1(dquot_quota_on_mount);
FUN_TYPE1(dquot_quota_on);
FUN_TYPE1(dquot_initialize);
FUN_TYPE1(fscrypt_decrypt_bio);
FUN_TYPE1(fscrypt_limit_io_blocks);
FUN_TYPE1(fscrypt_encrypt_pagecache_blocks);
FUN_TYPE1(fscrypt_mergeable_bio);
FUN_TYPE1(fscrypt_set_bio_crypt_ctx);
FUN_TYPE1(fscrypt_parse_test_dummy_encryption);
FUN_TYPE1(fscrypt_show_test_dummy_encryption);
FUN_TYPE1(fscrypt_free_inode);
FUN_TYPE1(fscrypt_drop_inode);
FUN_TYPE1(fscrypt_fname_siphash);
FUN_TYPE1(__fscrypt_encrypt_symlink);
FUN_TYPE1(__fscrypt_prepare_link);
FUN_TYPE1(__fscrypt_prepare_readdir);
FUN_TYPE1(fscrypt_fname_alloc_buffer);
FUN_TYPE1(page_cache_sync_ra);
FUN_TYPE1(fscrypt_fname_free_buffer);
FUN_TYPE1(__dquot_free_space);
FUN_TYPE1(fscrypt_dio_supported);
FUN_TYPE1(__fscrypt_prepare_setattr);
FUN_TYPE1(__fsverity_prepare_setattr);
FUN_TYPE1(dquot_transfer);
FUN_TYPE1(from_vfsuid);
FUN_TYPE1(from_vfsgid);
FUN_TYPE1(bdev_freeze);
FUN_TYPE1(bdev_thaw);
FUN_TYPE1(dqget);
FUN_TYPE1(__dquot_transfer);
FUN_TYPE1(dqput);
FUN_TYPE1(fscrypt_ioctl_remove_key);
FUN_TYPE1(fscrypt_ioctl_remove_key_all_users);
FUN_TYPE1(fscrypt_ioctl_get_nonce);
FUN_TYPE1(fscrypt_ioctl_get_key_status);
FUN_TYPE1(fsverity_ioctl_measure);
FUN_TYPE1(fsverity_ioctl_read_metadata);
FUN_TYPE1(fscrypt_ioctl_set_policy);
FUN_TYPE1(fscrypt_ioctl_add_key);
FUN_TYPE1(fscrypt_ioctl_get_policy);
FUN_TYPE1(fscrypt_ioctl_get_policy_ex);
FUN_TYPE1(fsverity_ioctl_enable);
FUN_TYPE1(__dquot_alloc_space);
FUN_TYPE1(dquot_claim_space_nodirty);
FUN_TYPE1(generic_file_llseek_size);
FUN_TYPE1(__iomap_dio_rw);
FUN_TYPE1(xa_get_mark);
FUN_TYPE1(iomap_dio_complete);
FUN_TYPE1(fscrypt_file_open);
FUN_TYPE1(__fsverity_file_open);
FUN_TYPE1(dquot_file_open);
FUN_TYPE1(generic_fadvise);
FUN_TYPE1(inode_to_bdi);
FUN_TYPE1(blkdev_issue_secure_erase);
FUN_TYPE1(fscrypt_zeroout_range);
FUN_TYPE1(iocb_bio_iopoll);
FUN_TYPE1(dquot_initialize_needed);
FUN_TYPE1(dquot_drop);
FUN_TYPE1(fscrypt_put_encryption_info);
FUN_TYPE1(__fsverity_cleanup_inode);
FUN_TYPE1(d_tmpfile);
FUN_TYPE1(fscrypt_get_symlink);
FUN_TYPE1(fscrypt_symlink_getattr);
FUN_TYPE1(fscrypt_has_permitted_context);
FUN_TYPE1(d_instantiate_new);
FUN_TYPE1(fscrypt_prepare_symlink);
FUN_TYPE1(fscrypt_prepare_new_inode);
FUN_TYPE1(__fscrypt_prepare_rename);
#ifdef CONFIG_KALLSYMS_ALL
OBJ_TYPE(struct kmem_cache, f2fs_cf_name_slab);
OBJ_TYPE(struct kmem_cache, f2fs_inode_cachep);
#endif

/*--------- erofs symbols ----------*/
FUN_TYPE1(xxh32);
FUN_TYPE1(filemap_add_folio);
FUN_TYPE1(psi_memstall_leave);
FUN_TYPE1(psi_memstall_enter);
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
FUN_TYPE1(read_cache_folio);
FUN_TYPE1(folio_end_read);
FUN_TYPE1(iomap_read_folio);
FUN_TYPE1(iomap_invalidate_folio);
FUN_TYPE1(iomap_release_folio);
FUN_TYPE1(thp_get_unmapped_area);
FUN_TYPE1(__seq_puts);
FUN_TYPE1(_copy_to_iter);
FUN_TYPE1(copy_highpage);
FUN_TYPE1(iov_iter_bvec);
FUN_TYPE1(kmemdup_nul);
FUN_TYPE1(memchr_inv);
OBJ_TYPE(struct qstr, dotdot_name);
OBJ_TYPE(struct xattr_handler , nop_posix_acl_access);
OBJ_TYPE(struct xattr_handler , nop_posix_acl_default);
#ifndef CONFIG_KALLSYMS_ALL
static struct qstr _dotdot_name = QSTR_INIT("..", 2);

static bool
f_posix_acl_xattr_list(struct dentry *dentry)
{
	return IS_POSIXACL(d_backing_inode(dentry));
}

/*
 * nop_posix_acl_access - legacy xattr handler for access POSIX ACLs
 *
 * This is the legacy POSIX ACL access xattr handler. It is used by some
 * filesystems to implement their ->listxattr() inode operation. New code
 * should never use them.
 */
static struct xattr_handler _nop_posix_acl_access = {
	.name = XATTR_NAME_POSIX_ACL_ACCESS,
	.list = f_posix_acl_xattr_list,
};

/*
 * nop_posix_acl_default - legacy xattr handler for default POSIX ACLs
 *
 * This is the legacy POSIX ACL default xattr handler. It is used by some
 * filesystems to implement their ->listxattr() inode operation. New code
 * should never use them.
 */
static struct xattr_handler _nop_posix_acl_default = {
	.name = XATTR_NAME_POSIX_ACL_DEFAULT,
	.list = f_posix_acl_xattr_list,
};
#endif

struct ksymbol {
	const char *name;
	void *data;
	unsigned int name_len;
};

static int __symbol_fixed;

#ifdef CONFIG_ARM64
static struct ksymbol module_symbols[] = {
	/* MUST keep sorted */
	KSYM_FUN(LZ4_compress_HC),
	KSYM_FUN(LZ4_compress_default),
	KSYM_FUN(__bio_advance),
	KSYM_FUN(__blkdev_issue_discard),
	KSYM_FUN(__cleancache_get_page),
	KSYM_FUN(__cleancache_init_fs),
	KSYM_FUN(__dquot_alloc_space),
	KSYM_FUN(__dquot_free_space),
	KSYM_FUN(__dquot_transfer),
	KSYM_FUN(__filemap_get_folio),
	KSYM_FUN(__filemap_set_wb_err),
	KSYM_FUN(__folio_batch_release),
	KSYM_FUN(__folio_start_writeback),
	KSYM_FUN(__folio_swap_cache_index),
	KSYM_FUN(__fscrypt_encrypt_symlink),
	KSYM_FUN(__fscrypt_inode_uses_inline_crypto),
	KSYM_FUN(__fscrypt_prepare_link),
	KSYM_FUN(__fscrypt_prepare_lookup),
	KSYM_FUN(__fscrypt_prepare_readdir),
	KSYM_FUN(__fscrypt_prepare_rename),
	KSYM_FUN(__fsverity_cleanup_inode),
	KSYM_FUN(__fsverity_file_open),
	KSYM_FUN(__fsverity_prepare_setattr),
	KSYM_FUN(__get_random_u32_below),
	KSYM_FUN(__iomap_dio_rw),
	KSYM_FUN(__percpu_counter_sum),
	KSYM_FUN(__fscrypt_prepare_setattr),
	KSYM_FUN(__percpu_counter_init_many),
	KSYM_FUN(__percpu_down_read),
	KSYM_FUN(__seq_puts),
	KSYM_FUN(__xa_clear_mark),
	KSYM_FUN(_copy_to_iter),
	KSYM_FUN(_raw_write_trylock),
	KSYM_FUN(add_swap_extent),
	KSYM_FUN(alloc_pages_bulk_noprof),
	KSYM_FUN(autoremove_wake_function),
	KSYM_FUN(bdev_file_open_by_path),
	KSYM_FUN(bdev_fput),
	KSYM_FUN(bdev_freeze),
	KSYM_FUN(bdev_thaw),
	KSYM_FUN(bdi_dev_name),
	KSYM_FUN(bio_add_folio),
	KSYM_FUN(bio_add_folio_nofail),
	KSYM_FUN(bio_associate_blkg_from_css),
	KSYM_FUN(bio_init),
	KSYM_FUN(bio_uninit),
	KSYM_FUN(bioset_exit),
	KSYM_FUN(bioset_init),
	KSYM_FUN(blk_op_str),
	KSYM_FUN(blk_status_to_errno),
	KSYM_FUN(blk_zone_cond_str),
	KSYM_FUN(blkdev_issue_flush),
	KSYM_FUN(blkdev_issue_secure_erase),
	KSYM_FUN(blkdev_issue_zeroout),
	KSYM_FUN(blkdev_report_zones),
	KSYM_FUN(blkdev_zone_mgmt),
	KSYM_FUN(clear_page_dirty_for_io),
	KSYM_FUN(copy_highpage),
	KSYM_FUN(d_instantiate_new),
	KSYM_FUN(d_invalidate),
	KSYM_FUN(d_parent_ino),
	KSYM_FUN(d_tmpfile),
#ifdef CONFIG_KALLSYMS_ALL
	KSYM_OBJ(dotdot_name),
#endif
	KSYM_FUN(dqget),
	KSYM_FUN(dqput),
	KSYM_FUN(dquot_acquire),
	KSYM_FUN(dquot_alloc),
	KSYM_FUN(dquot_alloc_inode),
	KSYM_FUN(dquot_file_open),
	KSYM_FUN(dquot_claim_space_nodirty),
	KSYM_FUN(dquot_commit),
	KSYM_FUN(dquot_commit_info),
	KSYM_FUN(dquot_destroy),
	KSYM_FUN(dquot_disable),
	KSYM_FUN(dquot_drop),
	KSYM_FUN(dquot_free_inode),
	KSYM_FUN(dquot_get_dqblk),
	KSYM_FUN(dquot_get_next_dqblk),
	KSYM_FUN(dquot_get_next_id),
	KSYM_FUN(dquot_get_state),
	KSYM_FUN(dquot_initialize),
	KSYM_FUN(dquot_initialize_needed),
	KSYM_FUN(dquot_load_quota_inode),
	KSYM_FUN(dquot_mark_dquot_dirty),
	KSYM_FUN(dquot_quota_off),
	KSYM_FUN(dquot_quota_on),
	KSYM_FUN(dquot_quota_on_mount),
	KSYM_FUN(dquot_release),
	KSYM_FUN(dquot_resume),
	KSYM_FUN(dquot_set_dqblk),
	KSYM_FUN(dquot_set_dqinfo),
	KSYM_FUN(dquot_transfer),
	KSYM_FUN(dquot_writeback_dquots),
	KSYM_FUN(end_page_writeback),
	KSYM_FUN(errseq_set),
	KSYM_FUN(evict_inodes),
#ifdef CONFIG_KALLSYMS_ALL
	KSYM_OBJ(f2fs_cf_name_slab),
#endif
	KSYM_FUN(f2fs_destroy_bio_entry_cache),
	KSYM_FUN(f2fs_destroy_bioset),
	KSYM_FUN(f2fs_destroy_checkpoint_caches),
	KSYM_FUN(f2fs_destroy_compress_cache),
	KSYM_FUN(f2fs_destroy_compress_mempool),
	KSYM_FUN(f2fs_destroy_extent_cache),
	KSYM_FUN(f2fs_destroy_garbage_collection_cache),
	KSYM_FUN(f2fs_destroy_iostat_processing),
	KSYM_FUN(f2fs_destroy_node_manager_caches),
	KSYM_FUN(f2fs_destroy_post_read_processing),
	KSYM_FUN(f2fs_destroy_recovery_cache),
	KSYM_FUN(f2fs_destroy_root_stats),
	KSYM_FUN(f2fs_destroy_segment_manager_caches),
	KSYM_FUN(f2fs_exit_sysfs),
#ifdef CONFIG_KALLSYMS_ALL
	KSYM_OBJ(f2fs_inode_cachep),
#endif
	KSYM_FUN(fdget),
	KSYM_FUN(file_bdev),
	KSYM_FUN(file_modified),
	KSYM_FUN(file_write_and_wait_range),
	KSYM_FUN(fileattr_fill_flags),
	KSYM_FUN(filemap_add_folio),
	KSYM_FUN(filemap_check_errors),
	KSYM_FUN(filemap_dirty_folio),
	KSYM_FUN(filemap_fault),
	KSYM_FUN(filemap_get_folios),
	KSYM_FUN(filemap_get_folios_tag),
	KSYM_FUN(filemap_map_pages),
	KSYM_FUN(filemap_migrate_folio),
	KSYM_FUN(filemap_splice_read),
	KSYM_FUN(filp_open),
	KSYM_FUN(find_inode_nowait),
	KSYM_FUN(finish_open),
	KSYM_FUN(flush_dcache_folio),
	KSYM_FUN(folio_clear_dirty_for_io),
	KSYM_FUN(folio_end_read),
	KSYM_FUN(folio_end_writeback),
	KSYM_FUN(folio_mark_dirty),
	KSYM_FUN(folio_redirty_for_writepage),
	KSYM_FUN(folio_unlock),
	KSYM_FUN(folio_wait_bit),
	KSYM_FUN(folio_wait_stable),
	KSYM_FUN(folio_wait_writeback),
	KSYM_FUN(freeze_super),
	KSYM_FUN(from_vfsgid),
	KSYM_FUN(from_vfsuid),
	KSYM_FUN(fs_umode_to_ftype),
	KSYM_FUN(fscrypt_d_revalidate),
	KSYM_FUN(fscrypt_decrypt_bio),
	KSYM_FUN(fscrypt_dio_supported),
	KSYM_FUN(fscrypt_drop_inode),
	KSYM_FUN(fscrypt_encrypt_pagecache_blocks),
	KSYM_FUN(fscrypt_get_symlink),
	KSYM_FUN(fscrypt_file_open),
	KSYM_FUN(fscrypt_fname_alloc_buffer),
	KSYM_FUN(fscrypt_fname_disk_to_usr),
	KSYM_FUN(fscrypt_fname_free_buffer),
	KSYM_FUN(fscrypt_fname_siphash),
	KSYM_FUN(fscrypt_free_bounce_page),
	KSYM_FUN(fscrypt_free_inode),
	KSYM_FUN(fscrypt_has_permitted_context),
	KSYM_FUN(fscrypt_limit_io_blocks),
	KSYM_FUN(fscrypt_match_name),
	KSYM_FUN(fscrypt_ioctl_add_key),
	KSYM_FUN(fscrypt_ioctl_get_key_status),
	KSYM_FUN(fscrypt_ioctl_get_nonce),
	KSYM_FUN(fscrypt_ioctl_get_policy),
	KSYM_FUN(fscrypt_ioctl_get_policy_ex),
	KSYM_FUN(fscrypt_ioctl_remove_key),
	KSYM_FUN(fscrypt_ioctl_remove_key_all_users),
	KSYM_FUN(fscrypt_ioctl_set_policy),
	KSYM_FUN(fscrypt_mergeable_bio),
	KSYM_FUN(fscrypt_parse_test_dummy_encryption),
	KSYM_FUN(fscrypt_prepare_new_inode),
	KSYM_FUN(fscrypt_prepare_symlink),
	KSYM_FUN(fscrypt_put_encryption_info),
	KSYM_FUN(fscrypt_set_bio_crypt_ctx),
	KSYM_FUN(fscrypt_set_context),
	KSYM_FUN(fscrypt_setup_filename),
	KSYM_FUN(fscrypt_show_test_dummy_encryption),
	KSYM_FUN(fscrypt_symlink_getattr),
	KSYM_FUN(fscrypt_zeroout_range),
	KSYM_FUN(fsverity_enqueue_verify_work),
	KSYM_FUN(fsverity_ioctl_enable),
	KSYM_FUN(fsverity_ioctl_measure),
	KSYM_FUN(fsverity_ioctl_read_metadata),
	KSYM_FUN(fsverity_verify_bio),
	KSYM_FUN(fsverity_verify_blocks),
	KSYM_FUN(generate_random_uuid),
	KSYM_FUN(generic_ci_match),
	KSYM_FUN(generic_encode_ino32_fh),
	KSYM_FUN(generic_error_remove_folio),
	KSYM_FUN(generic_fadvise),
	KSYM_FUN(generic_file_llseek_size),
	KSYM_FUN(generic_perform_write),
	KSYM_FUN(generic_set_sb_d_ops),
	KSYM_FUN(get_tree_bdev_flags),
	KSYM_FUN(get_tree_nodev),
	KSYM_FUN(iget_locked),
	KSYM_FUN(igrab),
	KSYM_FUN(in_group_or_capable),
	KSYM_FUN(in_group_p),
	KSYM_FUN(inode_set_ctime_current),
	KSYM_FUN(inode_set_flags),
	KSYM_FUN(inode_owner_or_capable),
	KSYM_FUN(inode_to_bdi),
	KSYM_FUN(invalidate_inode_pages2_range),
	KSYM_FUN(invalidate_mapping_pages),
	KSYM_FUN(io_schedule_timeout),
	KSYM_FUN(iocb_bio_iopoll),
	KSYM_FUN(iomap_dio_complete),
	KSYM_FUN(iomap_invalidate_folio),
	KSYM_FUN(iomap_read_folio),
	KSYM_FUN(iomap_release_folio),
	KSYM_FUN(iov_iter_bvec),
	KSYM_FUN(kmem_cache_alloc_lru_noprof),
	KSYM_FUN(kmemdup_nul),
	KSYM_FUN(llist_add_batch),
	KSYM_FUN(llist_reverse_order),
	KSYM_FUN(lockref_get_not_zero),
	KSYM_FUN(lockref_mark_dead),
	KSYM_FUN(lockref_put_or_lock),
	KSYM_FUN(make_vfsgid),
	KSYM_FUN(make_vfsuid),
	KSYM_FUN(match_int),
	KSYM_FUN(match_strdup),
	KSYM_FUN(match_token),
	KSYM_FUN(memchr_inv),
	KSYM_FUN(mempool_alloc_noprof),
	KSYM_FUN(mempool_alloc_pages),
	KSYM_FUN(mempool_alloc_slab),
	KSYM_FUN(mempool_create_node_noprof),
	KSYM_FUN(mempool_destroy),
	KSYM_FUN(mempool_free),
	KSYM_FUN(mempool_free_pages),
	KSYM_FUN(mempool_free_slab),
	KSYM_FUN(migrate_disable),
	KSYM_FUN(migrate_enable),
	KSYM_FUN(mnt_drop_write_file),
	KSYM_FUN(mnt_want_write_file),
	KSYM_FUN(mount_bdev),
#ifdef CONFIG_KALLSYMS_ALL
	KSYM_OBJ(nop_posix_acl_access),
	KSYM_OBJ(nop_posix_acl_default),
#endif
	KSYM_FUN(page_cache_ra_unbounded),
	KSYM_FUN(page_cache_sync_ra),
	KSYM_FUN(page_symlink),
	KSYM_FUN(percpu_counter_destroy_many),
	KSYM_FUN(percpu_counter_set),
	KSYM_FUN(posix_acl_alloc),
	KSYM_FUN(posix_acl_equiv_mode),
	KSYM_FUN(prepare_to_wait),
	KSYM_FUN(psi_memstall_enter),
	KSYM_FUN(psi_memstall_leave),
	KSYM_FUN(radix_tree_gang_lookup),
	KSYM_FUN(radix_tree_preload),
	KSYM_FUN(rcuwait_wake_up),
	KSYM_FUN(read_cache_folio),
	KSYM_FUN(redirty_page_for_writepage),
	KSYM_FUN(release_pages),
	KSYM_FUN(remove_proc_subtree),
	KSYM_FUN(security_inode_init_security),
	KSYM_FUN(seq_escape_mem),
	KSYM_FUN(set_cached_acl),
	KSYM_FUN(set_page_writeback),
	KSYM_FUN(set_task_ioprio),
	KSYM_FUN(simple_inode_init_ts),
	KSYM_FUN(shrink_dcache_sb),
	KSYM_FUN(strndup_user),
	KSYM_FUN(super_setup_bdi),
	KSYM_FUN(sync_inodes_sb),
	KSYM_FUN(tag_pages_for_writeback),
	KSYM_FUN(thaw_super),
	KSYM_FUN(thp_get_unmapped_area),
	KSYM_FUN(touch_atime),
	KSYM_FUN(truncate_inode_pages),
	KSYM_FUN(truncate_inode_pages_range),
	KSYM_FUN(truncate_pagecache_range),
	KSYM_FUN(utf8_casefold),
	KSYM_FUN(utf8_load),
	KSYM_FUN(utf8_unload),
	KSYM_FUN(utf8s_to_utf16s),
	KSYM_FUN(vfs_iocb_iter_read),
	KSYM_FUN(vfs_setpos),
	KSYM_FUN(wait_for_completion_io),
	KSYM_FUN(wbc_account_cgroup_owner),
	KSYM_FUN(xa_get_mark),
	KSYM_FUN(xxh32),
	KSYM_FUN(zero_fill_bio_iter),
	KSYM_FUN(zstd_max_clevel),
	KSYM_FUN(zstd_min_clevel),
	{}
};
#else
/* for arm32 */
static struct ksymbol module_symbols[] = {
	/* these functions are not exported by f2fs */
#ifdef CONFIG_F2FS_FS
	KSYM_FUN(f2fs_destroy_bioset),
	KSYM_FUN(f2fs_destroy_bio_entry_cache),
	KSYM_FUN(f2fs_destroy_checkpoint_caches),
//	KSYM_FUN(f2fs_destroy_compress_cache),
//	KSYM_FUN(f2fs_destroy_compress_mempool),
	KSYM_FUN(f2fs_destroy_extent_cache),
	KSYM_FUN(f2fs_destroy_garbage_collection_cache),
	KSYM_FUN(f2fs_destroy_iostat_processing),
	KSYM_FUN(f2fs_destroy_node_manager_caches),
	KSYM_FUN(f2fs_destroy_post_read_processing),
	KSYM_FUN(f2fs_destroy_recovery_cache),
	KSYM_FUN(f2fs_destroy_root_stats),
	KSYM_FUN(f2fs_destroy_segment_manager_caches),
	KSYM_FUN(f2fs_exit_sysfs),
#endif
	{}
};
#endif

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

static unsigned short ascii_off[128];
static unsigned short ascii_cnt[128];

static void update_ascii_off(struct ksymbol *ksyms)
{
	int i = 0;
	int c;

	memset(ascii_off, 0xff, sizeof(ascii_off));
	while (ksyms->name) {
		if (!ksyms->name_len)
			ksyms->name_len = strlen(ksyms->name);
		c = ksyms->name[0];
		if (ascii_off[c] == 0xffff)
			ascii_off[c] = i;
		ascii_cnt[c]++;
		i++;
		ksyms++;
	}
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
	unsigned long value;
	int ret;
	int off, start, end, i;
	int c;

#ifdef CONFIG_ARM64
	off = 19;
#else
	off = 11;
#endif

	c = line[off];
	if (ascii_off[c] == 0xffff) {	// do not have this acsii
		return 0;
	}

	start = ascii_off[c];
	end = ascii_cnt[c] + start;
	pr_debug("ascii start:%d-%d, sym:%s\n", start, end, &line[off]);

	i = 1;
	while (start < end) {
		c = line[i + off];
		if (!c)	// end of input
			return 0;
		if (*(unsigned long *)sym[start].data) { // already fixed
			start++;
			i = 1;
			continue;
		}
		if (sym[start].name[i] == c) { // keep match in this symbol
			if (i == (sym[start].name_len - 1)) { // sym end
				pr_debug("c:%c-%x, len:%d, start:%d, i:%d, sym:%s, line:%s",
					c, line[i + off + 1], sym[start].name_len,
					start, i, sym[start].name, &line[off]);
				if (line[i + off + 1] != '\n') {	// line not end
					start++;
					i = 1;
					continue;
				}
				line[off - 3] = '\0';
				ret = kstrtoul(line, 16, &value);
				if (ret)
					return ret;
				pr_debug("find sym:%lx %s:%s\n",
					value, sym[start].name, &line[off]);
				*(unsigned long *)sym[start].data = value;
				return 1;
			}
			i++;
			continue;
		}
		/* switch to next symbole */
		start++;
		i = 1;
	}
	pr_debug("not match sym:%s\n", &line[off]);
	return 0;
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
	int fixed_symbols = 0;

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
		if (ret > 0) {
			fixed_symbols += ret;
			if (fixed_symbols >= ARRAY_SIZE(module_symbols) - 1)
			break;
		}
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

#ifdef CONFIG_ARM
static void direct_symbol_assign(void)
{
	/* for arm32, no gki case */
	FUN_ASSIGN(fscrypt_ioctl_add_key);
	FUN_ASSIGN(fsverity_ioctl_enable);
	FUN_ASSIGN(fsverity_ioctl_measure);
	FUN_ASSIGN(fscrypt_ioctl_set_policy);
	FUN_ASSIGN(__blkdev_issue_discard);
	FUN_ASSIGN(__filemap_set_wb_err);
	FUN_ASSIGN(__pagevec_release);
	FUN_ASSIGN(posix_acl_equiv_mode);
	FUN_ASSIGN(set_cached_acl);
	FUN_ASSIGN(posix_acl_alloc);
	FUN_ASSIGN(wait_for_completion_io);
#ifdef CONFIG_BLK_DEV_ZONED
	FUN_ASSIGN(blkdev_zone_mgmt);
#endif
	FUN_ASSIGN(wait_for_stable_page);
	FUN_ASSIGN(wait_on_page_writeback);
	FUN_ASSIGN(find_inode_nowait);
	FUN_ASSIGN(dquot_free_inode);
	FUN_ASSIGN(dquot_alloc_inode);
	FUN_ASSIGN(filemap_check_errors);
	FUN_ASSIGN(fscrypt_limit_io_blocks);
	FUN_ASSIGN(add_swap_extent);
	FUN_ASSIGN(__page_file_index);
	FUN_ASSIGN(find_get_pages_range_tag);
#ifdef CONFIG_CLEANCACHE
	FUN_ASSIGN(__cleancache_get_page);
#endif
	FUN_ASSIGN(fscrypt_free_bounce_page);
	FUN_ASSIGN(__fscrypt_inode_uses_inline_crypto);
	FUN_ASSIGN(fscrypt_mergeable_bio);
	FUN_ASSIGN(fscrypt_set_bio_crypt_ctx);
	FUN_ASSIGN(bio_associate_blkg_from_css);
	FUN_ASSIGN(fsverity_verify_blocks);
	FUN_ASSIGN(fsverity_verify_bio);
	FUN_ASSIGN(fsverity_enqueue_verify_work);
	FUN_ASSIGN(fscrypt_decrypt_bio);
	FUN_ASSIGN(__xa_clear_mark);
	FUN_ASSIGN(migrate_page_states);
	FUN_ASSIGN(migrate_page_copy);
	FUN_ASSIGN(migrate_page_move_mapping);
	FUN_ASSIGN(fscrypt_encrypt_pagecache_blocks);
	FUN_ASSIGN(wbc_account_cgroup_owner);
	FUN_ASSIGN(thaw_super);
	FUN_ASSIGN(freeze_super);
	FUN_ASSIGN(__percpu_counter_sum);
	FUN_ASSIGN(set_task_ioprio);
#ifdef CONFIG_CLEANCACHE
	FUN_ASSIGN(__cleancache_init_fs);
#endif
#ifdef CONFIG_BLK_DEV_ZONED
	FUN_ASSIGN(blkdev_report_zones);
#endif
	FUN_ASSIGN(mempool_free_pages);
	FUN_ASSIGN(mempool_alloc_pages);
	FUN_ASSIGN(percpu_counter_destroy);
	FUN_ASSIGN(__percpu_counter_init);
	FUN_ASSIGN(seq_escape);
	FUN_ASSIGN(fscrypt_show_test_dummy_encryption);
	FUN_ASSIGN(dquot_disable);
	FUN_ASSIGN(sync_inodes_sb);
	FUN_ASSIGN(dquot_resume);
	FUN_ASSIGN(fscrypt_drop_inode);
	FUN_ASSIGN(fscrypt_free_inode);
	FUN_ASSIGN(dquot_quota_on);
	FUN_ASSIGN(dquot_get_state);
	FUN_ASSIGN(dquot_set_dqblk);
	FUN_ASSIGN(dquot_get_next_dqblk);
	FUN_ASSIGN(dquot_get_dqblk);
	FUN_ASSIGN(dquot_set_dqinfo);
	FUN_ASSIGN(dquot_commit_info);
	FUN_ASSIGN(dquot_mark_dquot_dirty);
	FUN_ASSIGN(dquot_release);
	FUN_ASSIGN(dquot_acquire);
	FUN_ASSIGN(dquot_commit);
	FUN_ASSIGN(dquot_get_next_id);
	FUN_ASSIGN(dquot_destroy);
	FUN_ASSIGN(dquot_alloc);
	FUN_ASSIGN(utf8_load);
	FUN_ASSIGN(fscrypt_parse_test_dummy_encryption);
	FUN_ASSIGN(match_int);
	FUN_ASSIGN(match_strdup);
	FUN_ASSIGN(match_token);
	FUN_ASSIGN(shrink_dcache_sb);
	FUN_ASSIGN(utf8_unload);
	FUN_ASSIGN(evict_inodes);
	FUN_ASSIGN(percpu_counter_set);
	FUN_ASSIGN(dquot_writeback_dquots);
	FUN_ASSIGN(dquot_load_quota_inode);
	FUN_ASSIGN(dquot_quota_on_mount);
	FUN_ASSIGN(blk_op_str);
	FUN_ASSIGN(dquot_quota_off);
	FUN_ASSIGN(dquot_initialize);
	FUN_ASSIGN(fscrypt_fname_siphash);
	FUN_ASSIGN(__fscrypt_prepare_rename);
	FUN_ASSIGN(__fscrypt_encrypt_symlink);
	FUN_ASSIGN(page_symlink);
	FUN_ASSIGN(fscrypt_prepare_symlink);
	FUN_ASSIGN(d_invalidate);
	FUN_ASSIGN(__fscrypt_prepare_link);
	FUN_ASSIGN(d_instantiate_new);
	FUN_ASSIGN(fscrypt_has_permitted_context);
	FUN_ASSIGN(generic_set_encrypted_ci_d_ops);
	FUN_ASSIGN(fscrypt_symlink_getattr);
	FUN_ASSIGN(fscrypt_get_symlink);
	FUN_ASSIGN(fscrypt_prepare_new_inode);
	FUN_ASSIGN(d_tmpfile);
	FUN_ASSIGN(__fsverity_cleanup_inode);
	FUN_ASSIGN(fscrypt_put_encryption_info);
	FUN_ASSIGN(dquot_drop);
	FUN_ASSIGN(dquot_initialize_needed);
	FUN_ASSIGN(congestion_wait);
	FUN_ASSIGN(iget_locked);
	OBJ_ASSIGN(percpu_counter_batch);
	FUN_ASSIGN(generic_fadvise);
	FUN_ASSIGN(truncate_pagecache_range);
	FUN_ASSIGN(__fsverity_file_open);
	FUN_ASSIGN(dquot_file_open);
	FUN_ASSIGN(fscrypt_file_open);
	FUN_ASSIGN(filemap_map_pages);
	FUN_ASSIGN(file_modified);
	FUN_ASSIGN(iomap_dio_complete);
	FUN_ASSIGN(__iomap_dio_rw);
	FUN_ASSIGN(fscrypt_dio_supported);
	FUN_ASSIGN(xa_get_mark);
	FUN_ASSIGN(vfs_setpos);
	FUN_ASSIGN(generic_file_llseek_size);
	FUN_ASSIGN(page_cache_ra_unbounded);
	FUN_ASSIGN(blkdev_issue_zeroout);
	FUN_ASSIGN(fscrypt_zeroout_range);
	FUN_ASSIGN(truncate_inode_pages_range);
	FUN_ASSIGN(percpu_counter_add_batch);
	FUN_ASSIGN(utf8s_to_utf16s);
	FUN_ASSIGN(fsverity_ioctl_read_metadata);
	FUN_ASSIGN(fscrypt_ioctl_get_nonce);
	FUN_ASSIGN(fscrypt_ioctl_get_key_status);
	FUN_ASSIGN(fscrypt_ioctl_remove_key_all_users);
	FUN_ASSIGN(fscrypt_ioctl_remove_key);
	FUN_ASSIGN(fscrypt_ioctl_get_policy_ex);
	FUN_ASSIGN(generate_random_uuid);
	FUN_ASSIGN(fscrypt_ioctl_get_policy);
	FUN_ASSIGN(thaw_bdev);
	FUN_ASSIGN(freeze_bdev);
	FUN_ASSIGN(inode_owner_or_capable);
	FUN_ASSIGN(__fsverity_prepare_setattr);
	FUN_ASSIGN(__fscrypt_prepare_setattr);
	FUN_ASSIGN(__dquot_free_space);
	FUN_ASSIGN(__dquot_alloc_space);
	FUN_ASSIGN(dquot_claim_space_nodirty);
	FUN_ASSIGN(iomap_dio_iopoll);
	FUN_ASSIGN(fileattr_fill_flags);
	FUN_ASSIGN(dqput);
	FUN_ASSIGN(__dquot_transfer);
	FUN_ASSIGN(dqget);
	FUN_ASSIGN(dquot_transfer);
	dotdot_name_t = (struct qstr *)&dotdot_name;
	FUN_ASSIGN(page_cache_sync_ra);
	FUN_ASSIGN(__fscrypt_prepare_readdir);
	FUN_ASSIGN(fscrypt_fname_free_buffer);
	FUN_ASSIGN(fscrypt_fname_alloc_buffer);
	FUN_ASSIGN(__page_file_mapping);
	FUN_ASSIGN(utf8_strncasecmp_folded);
	FUN_ASSIGN(fscrypt_match_name);
	FUN_ASSIGN(__fscrypt_prepare_lookup);
	FUN_ASSIGN(fscrypt_fname_disk_to_usr);
	FUN_ASSIGN(fscrypt_set_context);
	FUN_ASSIGN(fs_umode_to_ftype);
	FUN_ASSIGN(fscrypt_setup_filename);
	FUN_ASSIGN(utf8_casefold);
	FUN_ASSIGN(__percpu_down_read);
	FUN_ASSIGN(__set_page_dirty_nobuffers);
	FUN_ASSIGN(__sync_dirty_buffer);
	FUN_ASSIGN(__test_set_page_writeback);
	FUN_ASSIGN(autoremove_wake_function);
	FUN_ASSIGN(bdev_read_only);
	FUN_ASSIGN(bioset_exit);
	FUN_ASSIGN(bioset_init);
	FUN_ASSIGN(blk_status_to_errno);
	FUN_ASSIGN(blkdev_get_by_dev);
	FUN_ASSIGN(blkdev_issue_flush);
	FUN_ASSIGN(capable_wrt_inode_uidgid);
	FUN_ASSIGN(clear_page_dirty_for_io);
	FUN_ASSIGN(down_read_trylock);
	FUN_ASSIGN(_raw_write_trylock);
	FUN_ASSIGN(end_page_writeback);
	FUN_ASSIGN(errseq_set);
	FUN_ASSIGN(file_write_and_wait_range);
	FUN_ASSIGN(filemap_fault);
	FUN_ASSIGN(generic_error_remove_page);
	FUN_ASSIGN(generic_perform_write);
	FUN_ASSIGN(igrab);
	FUN_ASSIGN(in_group_p);
	FUN_ASSIGN(inode_set_flags);
	FUN_ASSIGN(invalidate_mapping_pages);
	FUN_ASSIGN(io_schedule_timeout);
	FUN_ASSIGN(llist_add_batch);
	FUN_ASSIGN(llist_reverse_order);
	FUN_ASSIGN(mempool_alloc);
	FUN_ASSIGN(mempool_alloc_slab);
	FUN_ASSIGN(mempool_free_slab);
	FUN_ASSIGN(mempool_destroy);
	FUN_ASSIGN(mempool_free);
	FUN_ASSIGN(mnt_drop_write_file);
	FUN_ASSIGN(mnt_want_write_file);
	FUN_ASSIGN(mount_bdev);
	FUN_ASSIGN(pagevec_lookup_range);
	FUN_ASSIGN(pagevec_lookup_range_tag);
	FUN_ASSIGN(prepare_to_wait);
	FUN_ASSIGN(mempool_create);
	FUN_ASSIGN(radix_tree_gang_lookup);
	FUN_ASSIGN(radix_tree_preload);
	FUN_ASSIGN(rcuwait_wake_up);
	FUN_ASSIGN(redirty_page_for_writepage);
	FUN_ASSIGN(release_pages);
	FUN_ASSIGN(remove_proc_subtree);
	FUN_ASSIGN(strndup_user);
	FUN_ASSIGN(strreplace);
	FUN_ASSIGN(tag_pages_for_writeback);
	FUN_ASSIGN(touch_atime);
	FUN_ASSIGN(truncate_inode_pages);
	FUN_ASSIGN(security_inode_init_security);
#ifdef CONFIG_LZO_COMPRESS
	FUN_ASSIGN(lzorle1x_1_compress);
	FUN_ASSIGN(lzo1x_1_compress);
#endif
#ifdef CONFIG_LZO_DECOMPRESS
	FUN_ASSIGN(lzo1x_decompress_safe);
#endif
	FUN_ASSIGN(LZ4_compress_HC);
	FUN_ASSIGN(LZ4_compress_default);

	/*--------- erofs symbols ----------*/
	FUN_ASSIGN(__xa_cmpxchg);
	FUN_ASSIGN(fs_ftype_to_dtype);
	FUN_ASSIGN(filemap_read);
	FUN_ASSIGN(iomap_dio_rw);
	FUN_ASSIGN(iomap_bmap);
	FUN_ASSIGN(iomap_readahead);
	FUN_ASSIGN(iomap_readpage);
	FUN_ASSIGN(iomap_fiemap);
	FUN_ASSIGN(read_cache_page_gfp);
	FUN_ASSIGN(crc32c);
	FUN_ASSIGN(add_to_page_cache_lru);
	FUN_ASSIGN(readahead_expand);
	FUN_ASSIGN(kthread_create_worker_on_cpu);
	FUN_ASSIGN(LZ4_decompress_safe);
	FUN_ASSIGN(LZ4_decompress_safe_partial);
	FUN_ASSIGN(out_of_line_wait_on_bit_lock);
	FUN_ASSIGN(fs_param_is_enum);
	FUN_ASSIGN(posix_acl_from_xattr);
	generic_ro_fops_t                 = (struct file_operations *)&generic_ro_fops;
	posix_acl_default_xattr_handler_t = (struct xattr_handler *)&posix_acl_default_xattr_handler;
	posix_acl_access_xattr_handler_t  = (struct xattr_handler *)&posix_acl_access_xattr_handler;

}
#endif
int symbol_fix(void)
{
	struct proc_dir_entry *entry;
	struct proc_node *parent, *tmp;
	const struct proc_ops temp = {};
	struct ctl_table *kptr;
	int ret = 0, old = -1;
	unsigned long long te;

	te = sched_clock();
	update_ascii_off(module_symbols);
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
	/* fake for handle */
#ifndef CONFIG_KALLSYMS_ALL
	dotdot_name_t = &_dotdot_name;
	nop_posix_acl_access_t = &_nop_posix_acl_access;
	nop_posix_acl_default_t = &_nop_posix_acl_default;
#endif

#ifdef CONFIG_ARM
	direct_symbol_assign();
#endif
	if (!ret)
		__symbol_fixed = 1;
	pr_info("%s, prio:%d, ret:%d, time:%lld\n",
		__func__, current->prio, ret, sched_clock() - te);
	return ret;
}

int symbol_fixed(void)
{
	return __symbol_fixed;
}
EXPORT_SYMBOL(symbol_fixed);

static void do_symbol_fix(struct work_struct *work)
{
	if (symbol_fix()) {
		pr_emerg("%s, %d, symbol fix failed\n", __func__, __LINE__);
		BUG();
		return;
	}
}

static struct work_struct symbol_work;

int __init sym_helper_init(void)
{
	int cpu = num_online_cpus() - 1;

	INIT_WORK(&symbol_work, do_symbol_fix);
	queue_work_on(cpu, system_highpri_wq, &symbol_work);
	return 0;
}

void __exit sym_helper_exit(void)
{
	__symbol_fixed = 0;
	memset(module_symbols, 0, sizeof(module_symbols));
}
module_init(sym_helper_init);
module_exit(sym_helper_exit);
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
MODULE_LICENSE("GPL");
