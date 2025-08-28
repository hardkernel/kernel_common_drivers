/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __REG_OPS_H__
#define __REG_OPS_H__

#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/major.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/io.h>
//#include <linux/amlogic/iomap.h>
#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_hw.h>

enum map_addr_idx_e {
	/* VPU */
	VPU_REG_IDX = 0,
	/* HDMITX_DWC */
	HDMITX_SEC_REG_IDX = 1,
	/* HDMITX_TOP */
	HDMITX_REG_IDX = 2,
	/* ESM */
	ELP_ESM_REG_IDX = 3,
	/* ANACTRL for SC2 and later SOCs */
	ANACTRL_REG_IDX = 4,
	/* HHICTRL for G12A and previous SOCs */
	HHI_REG_IDX = 4,
	/* PWRCTRL for SC2 and later SOCs */
	PWRCTRL_REG_IDX = 5,
	/* CBUS for G12A and previous SOCs */
	CBUS_REG_IDX = 5,
	/* RESETCTRL */
	RESETCTRL_REG_IDX = 6,
	/* SYSCTRL for SC2 and later SOCs */
	SYSCTRL_REG_IDX = 7,
	/* PERIPHS for G12A and previous SOCs */
	PERIPHS_REG_IDX = 7,
	/* CLKCTRL for SC2 and later SOCs */
	CLKCTRL_REG_IDX = 8,
	/* AOBUS for G12A and previous SOCs */
	AOBUS_REG_IDX = 8,
	PADCTRL_REG_IDX = 9,
	REG_IDX_END = 10,
};

#define BASE_REG_OFFSET		24

#define CBUS_REG_ADDR(reg) \
	((CBUS_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))
#define VCBUS_REG_ADDR(reg) \
	((VPU_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))
#define HHI_REG_ADDR(reg) \
	((HHI_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))
/* HDMITX_DWC */
#define HDMITX_SEC_REG_ADDR(reg) \
	((HDMITX_SEC_REG_IDX << BASE_REG_OFFSET) + (reg))
/* HDMITX_TOP */
#define HDMITX_REG_ADDR(reg) \
	((HDMITX_REG_IDX << BASE_REG_OFFSET) + (reg))
#define ELP_ESM_REG_ADDR(reg) \
	((ELP_ESM_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))
#define ANACTRL_REG_ADDR(reg) \
	((ANACTRL_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))
#define PWRCTRL_REG_ADDR(reg) \
	((PWRCTRL_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))
#define RESETCTRL_REG_ADDR(reg) \
	((RESETCTRL_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))
#define SYSCTRL_REG_ADDR(reg) \
	((SYSCTRL_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))
#define CLKCTRL_REG_ADDR(reg) \
	((CLKCTRL_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))
#define PADCTRL_REG_ADDR(reg) \
	((PADCTRL_REG_IDX << BASE_REG_OFFSET) + ((reg) << 2))

extern struct reg_map reg_maps[REG_IDX_END];

unsigned int TO_PHY_ADDR(unsigned int addr);
void __iomem *TO_PMAP_ADDR(unsigned int addr);

unsigned int hd_read_reg(unsigned int addr);
void hd_write_reg(unsigned int addr, unsigned int val);
void hd_set_reg_bits(unsigned int addr, unsigned int value,
		unsigned int offset, unsigned int len);

void hdmitx_wr_reg(unsigned int addr, unsigned int data);
void hdmitx_poll_reg(unsigned int addr, unsigned int val,
		     unsigned long timeout);
unsigned int hdmitx_rd_check_reg(unsigned int addr, unsigned int exp_data,
				 unsigned int mask);
bool hdmitx_get_bit(unsigned int addr, unsigned int bit_nr);
void hdmitx_set_reg_bits(unsigned int addr, unsigned int value,
				unsigned int offset, unsigned int len);

int hdmitx20_init_reg_map(struct platform_device *pdev);

#endif
