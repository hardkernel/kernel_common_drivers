/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __BL40_MODULE_H__
#define __BL40_MODULE_H__
/*soc:g12a/sm1*/
#define SMC_SUBID_MFH_V1_BOOT	0x20
#define SMC_SUBID_SHIFT		0x8
#define PACK_SMC_SUBID_ID(subid, id) (((subid) << SMC_SUBID_SHIFT) | (id))

#ifdef CONFIG_AMLOGIC_FIRMWARE
void bl40_rx_msg(void *msg, int size);
#else
static inline void bl40_rx_msg(void *msg, int size)
{
}
#endif

#endif /*__BL40_MODULE_H__*/
