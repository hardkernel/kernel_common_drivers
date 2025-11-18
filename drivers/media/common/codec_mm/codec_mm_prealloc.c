// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/cma.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/kthread.h>
#include <linux/syscalls.h>
#include <linux/cpumask.h>
#include <linux/printk.h>
#include <linux/hashtable.h>
#include <linux/amlogic/media/codec_mm/codec_mm.h>

#include "codec_mm_priv.h"

static int codec_prealloc_debug = 1;
module_param(codec_prealloc_debug, int, 0644);

#define dprintk(level, fmt, arg...)						\
	do {									\
		if (codec_prealloc_debug >= (level))						\
			pr_info("" fmt, ## arg);\
	} while (0)

#define pr_dbg(fmt, args ...)  dprintk(6, fmt, ## args)
#define pr_error(fmt, args ...) dprintk(1, fmt, ## args)
#define pr_inf(fmt, args ...) dprintk(8, fmt, ## args)

#define MB_ALIGN(size) (ALIGN(size, SZ_1M) >> 20)
#define INVALID_INST_ID (-1)
#define PRE_MEM "pre_mem"

struct codec_mm_pre_mgt_s {
	struct work_struct release_wrk;
	spinlock_t job_list_lock;            /* Protect job list */
	struct mutex ht_list_mutex;
	struct list_head prealloc_job_list;
};

struct prealloc_job {
	struct list_head list;
	struct hlist_node hnode;
	struct task_struct *host;
	int inst_id;
	u32 type;
	u32 size;
	int align_2n;
	int memflags;
	struct completion done;
	struct codec_mm_s *mem;
};

struct prealloc_pcp {
	struct completion start;
	struct task_struct *task;
	int cpu;
	int ret;
};

#define ALLOC_PAGE_NUM_KTHREAD	3
struct prealloc_pcp prealloc_kthread_array[ALLOC_PAGE_NUM_KTHREAD];
DECLARE_HASHTABLE(ht, 7);

static struct codec_mm_pre_mgt_s *get_mem_pre_mgt(void)
{
	static struct codec_mm_pre_mgt_s pre_mem_mgt;

	return &pre_mem_mgt;
}

void submit_prealloc_job(u32 type, u32 num, u32 size, int align_2n, int memflags, int inst_id)
{
	u32 i, cpu;
	unsigned long flags;
	struct prealloc_pcp *work;
	int cpus = num_online_cpus() - 1;
	struct prealloc_job *job = NULL;
	struct codec_mm_pre_mgt_s *pre_mgt = get_mem_pre_mgt();

	if (!codec_mm_get_secure_mem_ctrl())
		return;

	if (num == 0 || size == 0)
		return;

	pr_dbg("submit job para inst_id %d type is %d num is %d, size is %d, align_2n is %d flags is %d\n",
		inst_id, type, num, size, align_2n, memflags);

	if (align_2n < 0 || align_2n < PAGE_SHIFT)
		align_2n = PAGE_SHIFT;

	for (i = 0; i < num; i++) {
		job = kzalloc(sizeof(*job), GFP_KERNEL | __GFP_HIGH);
		job->type = type;
		job->inst_id = inst_id;
		job->size = size;
		job->align_2n = align_2n;
		job->host = current;
		job->memflags = memflags & (~CODEC_MM_FLAGS_FOR_TRY_PREALLOC);
		INIT_LIST_HEAD(&job->list);
		INIT_HLIST_NODE(&job->hnode);
		init_completion(&job->done);

		spin_lock(&pre_mgt->job_list_lock);
		list_add_tail(&job->list, &pre_mgt->prealloc_job_list);
		spin_unlock(&pre_mgt->job_list_lock);

		mutex_lock(&pre_mgt->ht_list_mutex);
		hash_add(ht, &job->hnode, MB_ALIGN(job->size));
		mutex_unlock(&pre_mgt->ht_list_mutex);

		pr_dbg("submit job size is %d align_2n is %d flags is %d\n",
			job->size, job->align_2n, job->memflags);
	}

	i = 0;
	local_irq_save(flags);
	for (cpu = 0; cpu < ALLOC_PAGE_NUM_KTHREAD; cpu++) {
		work = &prealloc_kthread_array[cpu];
		complete(&work->start);
		i++;
		if (i == cpus) {
			pr_dbg("sched work to %d cpu\n", i);
			break;
		}
	}
	local_irq_restore(flags);
}
EXPORT_SYMBOL(submit_prealloc_job);

static void release_unused_buffer(struct work_struct *work)
{
	int key;
	struct codec_mm_pre_mgt_s *pre_mgt = get_mem_pre_mgt();
	struct prealloc_job *job;
	struct hlist_node *hnode_tmp;
	struct codec_mm_s *mm = NULL;

	for (key = 0; key < HASH_SIZE(ht); key++) {
		mutex_lock(&pre_mgt->ht_list_mutex);
		hash_for_each_possible_safe(ht, job, hnode_tmp, hnode, key) {
			if (job->inst_id == INVALID_INST_ID) {
				hash_del(&job->hnode);
				if (!job->mem) {
					pr_err("enter wait time id %d size %d\n",
						job->inst_id, job->size);
					wait_for_completion(&job->done);
				}
				mm = job->mem;
				if (mm) {
					pr_dbg("mem_id %d prealloc free size is %d type is %d\n",
						mm->mem_id, mm->buffer_size, job->type);
					codec_mm_release(mm, PRE_MEM);
				}
				kfree(job);
			}
		}
		mutex_unlock(&pre_mgt->ht_list_mutex);
	}
}

void release_all_prealloc_job_direct(void)
{
	int key;
	struct codec_mm_pre_mgt_s *pre_mgt = get_mem_pre_mgt();
	struct prealloc_job *job, *tmp;
	struct hlist_node *hnode_tmp;

	spin_lock(&pre_mgt->job_list_lock);
	list_for_each_entry_safe(job, tmp, &pre_mgt->prealloc_job_list, list) {
		list_del_init(&job->list);
		complete(&job->done);
	}
	spin_unlock(&pre_mgt->job_list_lock);

	mutex_lock(&pre_mgt->ht_list_mutex);
	for (key = 0; key < HASH_SIZE(ht); key++) {
		hash_for_each_possible_safe(ht, job, hnode_tmp, hnode, key) {
			job->inst_id = INVALID_INST_ID;
		}
	}
	mutex_unlock(&pre_mgt->ht_list_mutex);
	release_unused_buffer(&pre_mgt->release_wrk);
}
EXPORT_SYMBOL(release_all_prealloc_job_direct);

void release_prealloc_job(int inst_identify)
{
	int key;
	struct codec_mm_pre_mgt_s *pre_mgt = get_mem_pre_mgt();
	struct prealloc_job *job, *tmp;
	struct hlist_node *hnode_tmp;

	spin_lock(&pre_mgt->job_list_lock);
	list_for_each_entry_safe(job, tmp, &pre_mgt->prealloc_job_list, list) {
		if (job->inst_id == inst_identify) {
			list_del_init(&job->list);
			complete(&job->done);
		}
	}
	spin_unlock(&pre_mgt->job_list_lock);

	mutex_lock(&pre_mgt->ht_list_mutex);
	for (key = 0; key < HASH_SIZE(ht); key++) {
		hash_for_each_possible_safe(ht, job, hnode_tmp, hnode, key) {
			if (job->inst_id == inst_identify) {
				job->inst_id = INVALID_INST_ID;
			}
		}
	}
	mutex_unlock(&pre_mgt->ht_list_mutex);
	schedule_work(&pre_mgt->release_wrk);
}
EXPORT_SYMBOL(release_prealloc_job);

void release_prealloc_job_with_type(int inst_identify, u32 type)
{
	int key;
	struct codec_mm_pre_mgt_s *pre_mgt = get_mem_pre_mgt();
	struct prealloc_job *job, *tmp;
	struct hlist_node *hnode_tmp;

	spin_lock(&pre_mgt->job_list_lock);
	list_for_each_entry_safe(job, tmp, &pre_mgt->prealloc_job_list, list) {
		if (job->inst_id == inst_identify && job->type == type) {
			list_del_init(&job->list);
			complete(&job->done);
		}
	}
	spin_unlock(&pre_mgt->job_list_lock);

	mutex_lock(&pre_mgt->ht_list_mutex);
	for (key = 0; key < HASH_SIZE(ht); key++) {
		hash_for_each_possible_safe(ht, job, hnode_tmp, hnode, key) {
			if (job->inst_id == inst_identify && job->type == type)
				job->inst_id = INVALID_INST_ID;
		}
	}
	mutex_unlock(&pre_mgt->ht_list_mutex);
	schedule_work(&pre_mgt->release_wrk);
}
EXPORT_SYMBOL(release_prealloc_job_with_type);

static int prealloc_boost_work_func(void *prealloc_data)
{
	struct prealloc_pcp *c_work = (struct prealloc_pcp *)prealloc_data;
	struct prealloc_job *job;
	struct codec_mm_pre_mgt_s *pre_mgt = get_mem_pre_mgt();
	int ret = -1;

	for (;;) {
		ret = wait_for_completion_interruptible(&c_work->start);
		if (ret < 0) {
			pr_err("%s wait for task %d is %d\n",
			       __func__, c_work->cpu, ret);
			continue;
		}

		while (1) {
			spin_lock(&pre_mgt->job_list_lock);
			if (list_empty(&pre_mgt->prealloc_job_list)) {
				spin_unlock(&pre_mgt->job_list_lock);
				pr_debug("%s,%d, list empty\n", __func__, __LINE__);
				goto next;
			}
			job = list_first_entry(&pre_mgt->prealloc_job_list,
				struct prealloc_job, list);
			list_del_init(&job->list);

			if (fatal_signal_pending(job->host)) {
				spin_unlock(&pre_mgt->job_list_lock);

				mutex_lock(&pre_mgt->ht_list_mutex);
				if (!hlist_unhashed(&job->hnode)) {
					hash_del(&job->hnode);
					kfree(job);
				} else {
					complete(&job->done);
				}
				mutex_unlock(&pre_mgt->ht_list_mutex);

				c_work->ret = -EINTR;
				pr_err("prealloc signal pending\n");
				goto next;
			}
			spin_unlock(&pre_mgt->job_list_lock);
			job->mem = codec_mm_alloc(PRE_MEM, job->size,
				job->align_2n, job->memflags, job->inst_id);
			complete(&job->done);
		}
next:
		if (kthread_should_stop()) {
			pr_err("%s task exit\n", __func__);
			break;
		}
	}

	return 0;
}

static inline bool tvp_flag_equal(int mem_flags, int pre_mem_flags)
{
	bool mem_has_tvp = (mem_flags & CODEC_MM_FLAGS_TVP);
	bool pre_mem_has_tvp = (pre_mem_flags & CODEC_MM_FLAGS_TVP);

	return (mem_has_tvp == pre_mem_has_tvp);
}

struct codec_mm_s *get_mms_from_hashtable(u32 key, int align_2n, int inst_id,
	int memflags, bool no_check_inst_id)
{
	struct prealloc_job *job = NULL;
	struct prealloc_job *pre_select_job = NULL;
	struct codec_mm_s *mm = NULL;
	struct hlist_node *tmp;
	struct codec_mm_pre_mgt_s *pre_mgt = get_mem_pre_mgt();

	if (align_2n < 0 || align_2n < PAGE_SHIFT)
		align_2n = PAGE_SHIFT;

	mutex_lock(&pre_mgt->ht_list_mutex);
	hash_for_each_possible_safe(ht, job, tmp, hnode, MB_ALIGN(key)) {
		if (job->size == key &&
			job->align_2n >= align_2n && tvp_flag_equal(memflags, job->memflags) &&
			(job->inst_id == inst_id || no_check_inst_id)) {
			pre_select_job = job;
			if (job->mem && completion_done(&job->done)) {
				hash_del(&job->hnode);
				mm = job->mem;
				pr_dbg("get inst_id %d job size is %d align_2n is %d id %d\n",
					job->inst_id, job->size, job->align_2n, mm->mem_id);
				kfree(job);
				break;
			}
		}
	}

	if (!mm && pre_select_job)
		hash_del(&pre_select_job->hnode);

	mutex_unlock(&pre_mgt->ht_list_mutex);

	if (!mm && pre_select_job) {
		spin_lock(&pre_mgt->job_list_lock);
		if (!list_empty(&pre_select_job->list)) {
			/* this job still not start, no wait */
			list_del_init(&pre_select_job->list);
			kfree(pre_select_job);
			spin_unlock(&pre_mgt->job_list_lock);
			return NULL;
		}
		spin_unlock(&pre_mgt->job_list_lock);

		/* this job is being allocated, waiting to be completed */
		wait_for_completion(&pre_select_job->done);
		if (!pre_select_job->mem) {
			pr_err("prealloc null, might because signal pending\n");
			list_del_init(&pre_select_job->list);
		}
		mm = pre_select_job->mem;

		kfree(pre_select_job);
	}

	return mm;
}

int init_prealloc_boost_task(void)
{
	int cpu;
	struct codec_mm_pre_mgt_s *pre_mgt = get_mem_pre_mgt();
	struct task_struct *task;
	struct prealloc_pcp *work;
	char task_name[20];

	INIT_WORK(&pre_mgt->release_wrk, release_unused_buffer);
	spin_lock_init(&pre_mgt->job_list_lock);
	mutex_init(&pre_mgt->ht_list_mutex);
	INIT_LIST_HEAD(&pre_mgt->prealloc_job_list);

	hash_init(ht);

	for (cpu = 0; cpu < ALLOC_PAGE_NUM_KTHREAD; cpu++) {
		memset(task_name, 0, sizeof(task_name));
		sprintf(task_name, "prealloc_task%d", cpu);
		work = &prealloc_kthread_array[cpu];
		init_completion(&work->start);
		work->cpu = cpu;
		task = kthread_create(prealloc_boost_work_func, work, task_name);
		if (!IS_ERR(task)) {
			set_user_nice(task, -17);
			work->task = task;
			pr_debug("create prealloc task%p, for cpu %d\n", task, cpu);
			wake_up_process(task);
		} else {
			pr_err("create task for cpu %d fail:%p\n", cpu, task);
			return -1;
		}
	}

	return 0;
}
EXPORT_SYMBOL(init_prealloc_boost_task);
