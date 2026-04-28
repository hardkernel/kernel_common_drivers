// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */


#include <linux/stacktrace.h>
#include <linux/export.h>
#include <linux/types.h>
#include <linux/smp.h>
#include <linux/irqflags.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/sched/clock.h>
#include <linux/sched/debug.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/amlogic/arm-smccc.h>
#include <linux/kprobes.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/sched/cputime.h>
#include <linux/sched/sysctl.h>
#include <sched.h>
#include <linux/sched/topology.h>
#include <linux/amlogic/gki_module.h>
#include <trace/hooks/sched.h>
#include <trace/hooks/dtask.h>
#include <trace/events/sched.h>
#include <trace/events/meson_atrace.h>
#include "lockup.h"

#ifdef CONFIG_AMLOGIC_DEBUG_BGKI_SCHED
//for noGKI, use struct task_struct save schedule information
#define SCHED_LAST_WAKEUP_TIME(p)    p->sched_last_wakeup_time
#define SCHED_LAST_IN_CPU_TIME(p)    p->sched_last_in_cpu_time
#define SCHED_LAST_OUT_CPU_TIME(p)   p->sched_last_out_cpu_time
#define SCHED_LAST_SLEEP_TIME(p)     p->sched_last_sleep_time
#define SCHED_LAST_TICK_TIME(p)      p->sched_last_tick_time
#else
//for GKI both GKI10 and GKI20, use stack save schedule information
//skip stack[0] because it's used for STACK_END_MAGIC
#define SCHED_LAST_WAKEUP_TIME(p)    ((unsigned long long *)(p->stack))[1]
#define SCHED_LAST_IN_CPU_TIME(p)    ((unsigned long long *)(p->stack))[2]
#define SCHED_LAST_OUT_CPU_TIME(p)   ((unsigned long long *)(p->stack))[3]
#define SCHED_LAST_SLEEP_TIME(p)     ((unsigned long long *)(p->stack))[4]
#define SCHED_LAST_TICK_TIME(p)      ((unsigned long long *)(p->stack))[5]
#endif

#if defined(CONFIG_ANDROID_VENDOR_HOOKS) && defined(CONFIG_FAIR_GROUP_SCHED)
static int sched_big_weight = 10; // * NICE_0_LOAD
static int sched_interactive_task_util = 150;
static int sched_task_low_prio = 125;
static int sched_task_high_prio = 118;
static int sched_rt_nice_enable;
static int sched_rt_nice_debug;
static int sched_rt_nice_prio = 110;
static unsigned long sched_rt_nice_gran = 4000000; //4ms
static int sched_check_preempt_wakeup_enable;
static int sched_check_preempt_wakeup_debug;
/* default 3ms, same with wakeup_granularity_ns(4*core smp) */
static unsigned long sched_check_preempt_wakeup_gran = 3000000;
static int sched_pick_next_task_enable;
static int sched_pick_next_task_debug;
static int sched_pick_next_task_wait_socre = 10; //1ms+
static int sched_pick_next_task_ignore_wait_prio = 120;

#if IS_ENABLED(CONFIG_AMLOGIC_GKI_TOOL)
static struct param_entry sched_params[] = {
	PARAM_INT(sched_big_weight),
	PARAM_INT(sched_interactive_task_util),
	PARAM_INT(sched_task_low_prio),
	PARAM_INT(sched_task_high_prio),
	PARAM_INT(sched_rt_nice_enable),
	PARAM_INT(sched_rt_nice_debug),
	PARAM_INT(sched_rt_nice_prio),
	PARAM_ULONG(sched_rt_nice_gran),
	PARAM_INT(sched_check_preempt_wakeup_enable),
	PARAM_INT(sched_check_preempt_wakeup_debug),
	PARAM_ULONG(sched_check_preempt_wakeup_gran),
	PARAM_INT(sched_pick_next_task_enable),
	PARAM_INT(sched_pick_next_task_debug),
	PARAM_INT(sched_pick_next_task_wait_socre),
	PARAM_INT(sched_pick_next_task_ignore_wait_prio),
	{ /* sentinel */ }
};

module_param_cb(debug_sched, &key_value_param_ops, &sched_params, 0644);
#endif

#ifdef CONFIG_SMP
static inline bool should_honor_rt_sync(struct rq *rq, struct task_struct *p,
					bool sync)
{
	/*
	 * If the waker is CFS, then an RT sync wakeup would preempt the waker
	 * and force it to run for a likely small time after the RT wakee is
	 * done. So, only honor RT sync wakeups from RT wakers.
	 */
	return sync && task_has_rt_policy(rq->curr) &&
		p->prio <= rq->rt.highest_prio.next &&
		rq->rt.rt_nr_running <= 2;
}
#else
static inline bool should_honor_rt_sync(struct rq *rq, struct task_struct *p,
					bool sync)
{
	return 0;
}
#endif

static void aml_select_rt_nice(void *data, struct task_struct *p,
				int prev_cpu, int sd_flag,
				int wake_flags, int *new_cpu)
{
	int test = 0;
	struct rq *rq;
	struct task_struct *curr;
	int this_cpu;
	struct rq *this_cpu_rq;
	unsigned long rtime = 0;
	int lowest_prio_cpu = -1;
	int lowest_prio = -1;
	int tmp_cpu;
	bool sync = !!(wake_flags & WF_SYNC);

	if (!sched_rt_nice_enable)
		return;

	rcu_read_lock();
	rq = cpu_rq(prev_cpu);
	/* coverity[overrun-local] prev_cpu is safe */
	curr = READ_ONCE(rq->curr);
	this_cpu = smp_processor_id();
	this_cpu_rq = cpu_rq(this_cpu);

	if (should_honor_rt_sync(this_cpu_rq, p, sync) &&
	    cpumask_test_cpu(this_cpu, p->cpus_ptr)) {
		*new_cpu = this_cpu;
		goto out_unlock;
	}

	if (!curr)
		goto out_unlock;

	if (rt_task(curr)) {
		test = 1;
	} else if (curr->prio <= sched_rt_nice_prio) {
		if (curr->se.depth == 1 &&
		    curr->se.parent->my_q->tg->shares < sched_big_weight * NICE_0_LOAD)
			goto out_unlock;

		//high prio normal interactive task
		if (curr->se.avg.util_avg >= sched_interactive_task_util)
			goto out_unlock;

		update_rq_clock(rq);

		rtime = curr->se.sum_exec_runtime - curr->se.prev_sum_exec_runtime;
		rtime += (rq_clock_task(rq) - curr->se.exec_start);
		if (rtime >= sched_rt_nice_gran)
			goto out_unlock;

		test = 1;
	}

	if (!test)
		goto out_unlock;

	for_each_cpu(tmp_cpu, p->cpus_ptr) {
		/* coverity[overrun-local] for_each_cpu() is safe */
		struct task_struct *task = READ_ONCE(cpu_rq(tmp_cpu)->curr);

		if (task && task->pid == 0) {
			if (sched_rt_nice_debug)
				aml_trace_printk("wake:%s/%d curr:%s/%d prio=%d util=%lu rtime=%lu idle_cpu:%d\n",
					     p->comm, p->pid, curr->comm, curr->pid,
					     curr->prio, curr->se.avg.util_avg, rtime,
					     tmp_cpu);

			*new_cpu = tmp_cpu;
			goto out_unlock;
		}

		if (task && task->se.depth == 1 &&
		    task->se.parent->my_q->tg->shares < sched_big_weight * NICE_0_LOAD) {
			if (sched_rt_nice_debug)
				aml_trace_printk("wake:%s/%d curr:%s/%d prio=%d util=%lu rtime=%lu low_share_group_cpu:%d\n",
					     p->comm, p->pid, curr->comm, curr->pid,
					     curr->prio, curr->se.avg.util_avg, rtime,
					     tmp_cpu);

			*new_cpu = tmp_cpu;
			goto out_unlock;
		}

		if (task && task->prio > lowest_prio) {
			lowest_prio = task->prio;
			lowest_prio_cpu = tmp_cpu;
		}
	}

	if (lowest_prio_cpu != -1) {
		if (sched_rt_nice_debug)
			aml_trace_printk("wake:%s/%d curr:%s/%d prio=%d util=%lu rtime=%lu lowest_prio_cpu:%d\n",
				     p->comm, p->pid, curr->comm, curr->pid,
				     curr->prio, curr->se.avg.util_avg, rtime,
				     lowest_prio_cpu);
		*new_cpu = lowest_prio_cpu;
	}

out_unlock:
	rcu_read_unlock();
}

static void aml_check_preempt_wakeup(void *data, struct rq *rq, struct task_struct *p,
					bool *preempt, bool *nopreempt, int wake_flags,
					struct sched_entity *se, struct sched_entity *pse)
{
	struct task_struct *curr = rq->curr;
	unsigned long delta_exec = curr->se.sum_exec_runtime - curr->se.prev_sum_exec_runtime;
	int cpu = cpu_of(rq);

	if (!sched_check_preempt_wakeup_enable)
		return;

	if (p->se.depth == 1 &&
	    p->se.parent->my_q->tg->shares < sched_big_weight * NICE_0_LOAD) {
		if (sched_check_preempt_wakeup_debug)
			aml_trace_printk("ignore:%d low-share group:%s share=%lu\n",
				     cpu, p->sched_task_group->css.cgroup->kn->name,
				     p->se.parent->my_q->tg->shares);
		 *nopreempt = 1;
		return;
	}

	if (curr->se.depth == 1 &&
	    curr->se.parent->my_q->tg->shares < sched_big_weight * NICE_0_LOAD) {
		if (sched_check_preempt_wakeup_debug)
			aml_trace_printk("resched:%d current low-share group:%s share=%lu\n",
				     cpu, curr->sched_task_group->css.cgroup->kn->name,
				     curr->se.parent->my_q->tg->shares);
		 *preempt = 1;
		return;
	}

	if (p->prio >= sched_task_low_prio) {
		if (sched_check_preempt_wakeup_debug)
			aml_trace_printk("ignore:%d low-prio task: prio=%d\n", cpu, p->prio);
		 *nopreempt = 1;
		return;
	}

	if (curr->prio >= sched_task_low_prio) {
		if (sched_check_preempt_wakeup_debug)
			aml_trace_printk("resched:%d low-prio current task: prio=%d\n", cpu, p->prio);
		 *preempt = 1;
		return;
	}

	if (curr->prio <= sched_task_high_prio && curr->se.avg.util_avg < sched_interactive_task_util &&
	    delta_exec <= sched_check_preempt_wakeup_gran) {
		if (sched_check_preempt_wakeup_debug)
			aml_trace_printk("ignore:%d current interactive min_gran: delta_exec=%lu\n", cpu, delta_exec);
		 *nopreempt = 1;
		return;
	}

	if (p->prio <= sched_task_high_prio && p->se.avg.util_avg < sched_interactive_task_util) {
		if (sched_check_preempt_wakeup_debug)
			aml_trace_printk("resched:%d new interactive\n", cpu);
		 *preempt = 1;
		return;
	}
}

void set_next_entity(struct cfs_rq *cfs_rq, struct sched_entity *se);

#define __node_2_se(node) \
	rb_entry((node), struct sched_entity, run_node)

static struct sched_entity *___pick_first_entity(struct cfs_rq *cfs_rq)
{
	struct rb_node *left = rb_first_cached(&cfs_rq->tasks_timeline);

	if (!left)
		return NULL;

	return __node_2_se(left);
}

static struct sched_entity *__pick_next_entity(struct sched_entity *se)
{
	struct rb_node *next = rb_next(&se->run_node);

	if (!next)
		return NULL;

	return __node_2_se(next);
}

static inline struct sched_entity *parent_entity(struct sched_entity *se)
{
		return se->parent;
}

static int task_interactive_score(struct task_struct *p, unsigned long weight, int ignore_wait)
{
	int score, weight_score, prio_score, wait_score, util_score;
	unsigned long delta;

	wait_score = 0;

	if (weight < sched_big_weight * NICE_0_LOAD ||
	    p->prio > sched_task_high_prio ||
	    p->se.avg.util_avg >= sched_interactive_task_util)
		return 0;

	weight_score = (weight / NICE_0_LOAD - 10) * 5;  //share 10240 = 0, 20480 = 50, 40960+ = 100;
	if (weight_score > 100)
		weight_score = 100;

	prio_score = (sched_task_high_prio - p->prio) * 10;

	if (!ignore_wait) {
		delta = rq_clock(rq_of(p->se.cfs_rq)) - SCHED_LAST_WAKEUP_TIME(p);
		delta = delta >> 20;
		wait_score = delta * 10; //wait 1ms = 10, 10ms = 100, 20ms = 200;

		if (wait_score < sched_pick_next_task_wait_socre)
			return 0;
	}

	util_score = sched_interactive_task_util - p->se.avg.util_avg;

	score = weight_score + prio_score + wait_score + util_score;
	if (sched_pick_next_task_debug)
		aml_trace_printk("interactive_task: %s/%d score:%d/%d,%d,%d,%d, wait:%llu util=%lu\n",
			     p->comm, p->pid, score, weight_score, prio_score, wait_score, util_score,
			     SCHED_LAST_WAKEUP_TIME(p), p->se.avg.util_avg);

	return score;
}

static struct sched_entity *__aml_pick_next_task(struct cfs_rq *cfs_rq, unsigned long weight, int *score, int ignore_wait)
{
	struct sched_entity *se, *ret;
	int max_score = 0;
	int tmp_score;

	*score = 0;
	ret = NULL;

	se = ___pick_first_entity(cfs_rq);

	while (se) {
		if (!entity_is_task(se))
			WARN(1, "not support 2+ level cgroups");

		tmp_score = task_interactive_score(task_of(se), weight, ignore_wait);
		if (tmp_score > max_score) {
			ret = se;
			max_score = tmp_score;
			*score = max_score;
		}

		se = __pick_next_entity(se);
	}

	return ret;
}

static void aml_pick_next_task(void *data, struct rq *rq, struct task_struct **p_new, struct task_struct *prev)
{
	struct sched_entity *ret, *p;
	struct sched_entity *se;
	int score, max_score;
	struct task_struct *aml_p = NULL;
	struct sched_entity *aml_se = NULL;
	struct task_struct *curr = rq->curr;
	int ignore_wait = 0;

	if (!sched_pick_next_task_enable)
		return;

	ret = NULL;
	max_score = 0;

	//if current task is big-group interactive task, select it again
	if (prev->on_rq && prev->se.depth == 1 && prev->se.parent->my_q->tg->shares >= sched_big_weight * NICE_0_LOAD &&
	    task_interactive_score(prev, prev->se.parent->my_q->tg->shares, 1)) {
		if (sched_pick_next_task_debug)
			aml_trace_printk("try_again:%s/%d -> %s/%d\n", (*p_new)->comm, (*p_new)->pid, prev->comm, prev->pid);

		*p_new = prev;
		return;
	}

	if (task_has_dl_policy(curr) || task_has_rt_policy(curr)) {
		ignore_wait = 1;
	} else if (fair_policy(curr->policy)) {
		if ((curr->se.depth == 1 && curr->se.parent->my_q->tg->shares < sched_big_weight * NICE_0_LOAD) ||
		    curr->prio >= sched_pick_next_task_ignore_wait_prio)
			ignore_wait = 1;
	}

	se = ___pick_first_entity(&rq->cfs);

	while (se) {
		if (!entity_is_task(se) && se->my_q->tg->shares >= sched_big_weight * NICE_0_LOAD) {
			p = __aml_pick_next_task(group_cfs_rq(se), se->my_q->tg->shares, &score, ignore_wait);
			if (p && score > max_score) {
				ret = p;
				max_score = score;
			}
		}

		se = __pick_next_entity(se);
	}

	if (!ret)
		return;

	aml_se = ret;
	aml_p = task_of(aml_se);

	if (sched_pick_next_task_debug)
		aml_trace_printk("select: %s/%d\n", aml_p->comm, aml_p->pid);

	*p_new = aml_p;
}
#endif

#ifdef CONFIG_ANDROID_VENDOR_HOOKS

static void __maybe_unused sched_show_task_hook(void *data, struct task_struct *p)
{
	pr_info("thread:%s[%d] task:%s[%d] on_cpu=%d prio=%d sum_exec_runtime=%llu runnable_avg=%lu util_avg=%lu wake=%llu in_cpu=%llu off_cpu=%llu sleep=%llu tick=%llu rcu_neset=%d\n",
		p->comm, p->pid,
		p->group_leader->comm, p->group_leader->pid,
		p->on_cpu, p->prio,
		p->se.sum_exec_runtime,
		p->se.avg.runnable_avg,
		p->se.avg.util_avg,
		SCHED_LAST_WAKEUP_TIME(p),
		SCHED_LAST_IN_CPU_TIME(p),
		SCHED_LAST_OUT_CPU_TIME(p),
		SCHED_LAST_SLEEP_TIME(p),
		SCHED_LAST_TICK_TIME(p),
		p->rcu_read_lock_nesting);
}

static void sched_switch_hook(void *data, bool mode, struct task_struct *prev,
			      struct task_struct *next, unsigned int prev_stat)
{
	unsigned long long now;

	now = sched_clock();
	SCHED_LAST_IN_CPU_TIME(next) = now; //last in cpu time
	SCHED_LAST_OUT_CPU_TIME(prev) = now; //last off cpu time
	if (prev->__state & TASK_INTERRUPTIBLE || prev->__state & TASK_UNINTERRUPTIBLE)
		SCHED_LAST_SLEEP_TIME(prev) = now; //last sleep time
}

static void enqueue_task_hook(void *data, struct rq *rq, struct task_struct *p, int flags)
{
	if (p->__state == TASK_WAKING)
		SCHED_LAST_WAKEUP_TIME(p) = sched_clock(); //last wakeup time
}

static void tick_entry_hook(void *data, struct rq *rq)
{
	struct task_struct *p = current;

	SCHED_LAST_TICK_TIME(p) = sched_clock(); //last tick time
}
#endif

void rebuild_sched_flag(void)
{
	int cpu, old_flags;
	struct sched_domain *sd;

	for_each_possible_cpu(cpu) {
		rcu_read_lock();
		for_each_domain(cpu, sd) {
			old_flags = sd->flags;
			sd->flags &= ~SD_WAKE_AFFINE;
			sd->flags |= (SD_BALANCE_WAKE | SD_BALANCE_NEWIDLE);
		}
		rcu_read_unlock();
	}
}
EXPORT_SYMBOL(rebuild_sched_flag);

int aml_sched_init(void)
{
#if defined(CONFIG_ANDROID_VENDOR_HOOKS) && defined(CONFIG_FAIR_GROUP_SCHED)
	register_trace_android_rvh_select_task_rq_rt(aml_select_rt_nice, NULL);
	register_trace_android_rvh_check_preempt_wakeup_fair(aml_check_preempt_wakeup, NULL);
	register_trace_android_rvh_replace_next_task_fair(aml_pick_next_task, NULL);
#endif

#ifdef CONFIG_ANDROID_VENDOR_HOOKS
	register_trace_sched_switch(sched_switch_hook, NULL);
	register_trace_android_rvh_enqueue_task(enqueue_task_hook, NULL);
	register_trace_android_rvh_tick_entry(tick_entry_hook, NULL);
	register_trace_android_vh_sched_show_task(sched_show_task_hook, NULL);
#endif
	rebuild_sched_flag();

	return 0;
}
