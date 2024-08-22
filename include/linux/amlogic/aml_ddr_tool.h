/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __AML_DDR_TOOL_H__
#define __AML_DDR_TOOL_H__

/* ddr_tool: dev_access */
struct dmc_dev_access_data {
	unsigned long addr;
	unsigned long size;
};

#if IS_ENABLED(CONFIG_AMLOGIC_DMC_DEV_ACCESS)
int register_dmc_dev_access_notifier(char *dev_name, struct notifier_block *nb);
int unregister_dmc_dev_access_notifier(char *dev_name, struct notifier_block *nb);
#else
static inline int register_dmc_dev_access_notifier(char *dev_name, struct notifier_block *nb)
{
	return 0;
}

static inline int unregister_dmc_dev_access_notifier(char *dev_name, struct notifier_block *nb)
{
	return 0;
}
#endif

#endif
