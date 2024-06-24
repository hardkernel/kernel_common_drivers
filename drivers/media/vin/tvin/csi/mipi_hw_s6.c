// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

// functions which are not compatible with sm1 should be implemented here.

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/io.h>

#include "csi_reg.h"
#include "csi.h"

static inline u32 WRITE_CBUS_REG(void __iomem *base, u32 reg_offset, u32 val)
{
	__raw_writel(val, base + reg_offset);
	return 0;
}

static inline u32 READ_CBUS_REG(void __iomem *base, u32 reg_offset)
{
	return __raw_readl(base + reg_offset);
}

static inline u32 WRITE_CBUS_REG_BITS(void __iomem *base, u32 reg_offset,
		u32 val, u32 start, u32 len)
{
	u32 mask = ((1L << len) - 1) << start;
	u32 orig = READ_CBUS_REG(base, reg_offset);
	u32 temp = orig & (~mask);

	temp |= ((val & ((1L << len) - 1)) << start);
	WRITE_CBUS_REG(base, reg_offset, temp);
	return 0;
}

void powerup_csi_analog_s6(struct csi_adapt *adap_dev)
{
	void __iomem *base_addr;

	// no need power domain operation.
	// power domain always on
	//power_domain_switch(PM_CSI, PWR_ON);

	base_addr = ioremap(HHI_CSI_PHY_S6, 0x400);
	if (!base_addr) {
		pr_err("%s: Failed to ioremap addr\n", __func__);
		return;
	}

	WRITE_CBUS_REG(base_addr, ANACTRL_MIPICSI_CTRL0_S6, 0x2377f022);
	if (adap_dev->squlech_mode == 0) // disable squlech bit 4 is 1;
		WRITE_CBUS_REG(base_addr, ANACTRL_MIPICSI_CTRL1_S6, 0x00070010);
	else // enable squlech. default value.
		WRITE_CBUS_REG(base_addr, ANACTRL_MIPICSI_CTRL1_S6, 0x00070000);
	WRITE_CBUS_REG(base_addr, ANACTRL_MIPICSI_CTRL2_S6, 0x00004203);
	WRITE_CBUS_REG(base_addr, ANACTRL_MIPICSI_CTRL3_S6, 0x00000000);
	WRITE_CBUS_REG_BITS(base_addr, ANACTRL_CSIPLL_CTRL3_S6, 1, 30, 1);

	iounmap(base_addr);
}

void powerdown_csi_analog_s6(struct csi_adapt *adap_dev)
{
	// no need power domain operation.
	// power domain always on
	//power_domain_switch(PM_CSI, PWR_OFF);
}
