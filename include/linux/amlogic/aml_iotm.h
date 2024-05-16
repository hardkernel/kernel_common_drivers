/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __AML_IOTM_H
#define __AML_IOTM_H

enum iotm_sw_type {
	IOTM_SW_SCHED,
	IOTM_SW_IRQ_IN,
	IOTM_SW_IRQ_OUT,
	IOTM_SW_SMC_IN,
	IOTM_SW_SMC_OUT,
	IOTM_SW_SMC_NORET_IN,
};

#if IS_ENABLED(CONFIG_AMLOGIC_DEBUG_IOTM)
int iotm_init(void);
int get_iotm_en_ddr_size(int *iotm_ddr_size);
int get_iotm_monitor_mode(void);
void iotm_sw_record_write(u32 sw_type, u32 val1, u32 val2);
#else
static inline int iotm_init(void)
{
	return 0;
}

static inline int get_iotm_en_ddr_size(int *iotm_ddr_size)
{
	return 0;
}

static inline int get_iotm_monitor_mode(void)
{
	return 0;
}

static inline void iotm_sw_record_write(u32 sw_type, u32 val1, u32 val2)
{
}
#endif

#endif  /* __AML_IOTM_H */
