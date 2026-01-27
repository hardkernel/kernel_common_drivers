/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __AML_AOCPU_MONITOR_H
#define __AML_AOCPU_MONITOR_H
#include <linux/kthread.h>

#define ARM_ALIVE_CHECK_DIS 0x7500
#define ARM_ALIVE_CHECK_EN 0x7501
#define ARM_ALIVE_CHECK_ONOFF 0x7502
#define ARM_ALIVE_CHECK_TIMESET 0x7503
#define ARM_ALIVE_CHECK_TICK 0x7504
#define ARM_ALIVE_CHECK_DUMP 0x7505

#define AOCPU_TIMEOUT 20

struct aocpu_work_data {
	unsigned int alive_check_cmd[2];
	struct timer_list aocpu_timer;
	struct kthread_worker monitor_worker;
	struct task_struct *monitor_task;
	struct kthread_work monitor_work;
	bool enabled;
};

#if IS_ENABLED(CONFIG_AMLOGIC_DEBUG_AOCPU_MONITOR)
int aocpu_monitor_init(void);
#else
static inline int aocpu_monitor_init(void)
{
	return 0;
}
#endif

#endif  /* __AML_AOCPU_MONITOR_H */
