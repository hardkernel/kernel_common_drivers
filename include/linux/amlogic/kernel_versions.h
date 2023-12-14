/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __KERNEL_VERSIONS_H
#define __KERNEL_VERSIONS_H

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/cpufreq.h>
#include <linux/random.h>
#include <linux/fb.h>
#include <asm/fb.h>
#include <linux/shrinker.h>
#include <linux/mm.h>
#include <drm/drm_mode_config.h>
#include <drm/drm_writeback.h>
#include <linux/dma-resv.h>
#include <linux/leds.h>
#include <linux/i2c.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/power_supply.h>
#include <linux/thermal.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/usb.h>
#include <linux/usb/gadget.h>
#include <sound/soc.h>
#include <sound/soc-dai.h>
#include <drm/drm_device.h>


#if CONFIG_AMLOGIC_KERNEL_VERSION >= 15606
#include <linux/amlogic/kernel_versions_6.6.h>
#else
#include <linux/amlogic/kernel_versions_5.15.h>
#endif

#endif //__KERNEL_VERSIONS_H
