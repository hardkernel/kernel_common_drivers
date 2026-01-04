/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __UAPI_AML_REBOOT_H__
#define __UAPI_AML_REBOOT_H__

#include <linux/types.h>

#define REBOOT_MAGIC 'r'
#define REBOOT_SET _IOR(REBOOT_MAGIC, 1, int)
#define REBOOT_GET _IOW(REBOOT_MAGIC, 2, int)

#endif
