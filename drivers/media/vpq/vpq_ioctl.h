/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __VPQ_IOCTL_H__
#define __VPQ_IOCTL_H__

#include <linux/fs.h>

#define RELOAD_PQ_FOR_SAME_TIMING    (0) // ex: unplug->plug cable
#define VF_BY_HWC                    (0)

int vpq_ioctl_process(struct file *file, unsigned int cmd, unsigned long arg);

#endif // __VPQ_IOCTL_H__
