// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "sys_def.h"
#ifdef RUN_ON_ARM
#include <linux/module.h>
#include <linux/kfifo.h>
#include <linux/amlogic/media/vfm/vframe.h>
#include <linux/amlogic/media/vfm/vframe_provider.h>
#include <linux/amlogic/media/vfm/vframe_receiver.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>

#include <linux/amlogic/media/vpu/vpu.h>
/*dma_get_cma_size_int_byte*/
#include <linux/amlogic/media/codec_mm/codec_mm.h>
#include <linux/amlogic/media/di/dpss_interface.h>
#endif //RUN_ON_ARM
#include "dpss_base.h"
#include "dpss_s.h"
#include "dpss_sys.h"
#include "dpss_hw.h"
#include "dpss_func.h"
#include "dpss_rdma_frc.h"

static struct rdma_irq_reg_s irq_status;
static unsigned int rdma_ctrl;

static unsigned int dpss_rdma_en;
module_param_named(dpss_rdma_en, dpss_rdma_en, uint, 0664);
static unsigned int buf_alloced;

static struct dpss_rdma_info rdma_info[RDMA_CHANNEL]; // 0: man 1:pre vsync 2:vsync

static unsigned int rdma_handle_irq_pre_vs = 0x2; // pre vs tri bit:2
static unsigned int rdma_handle_irq_vs = 0x10000000; //vs tri bit:28

int rdma_enable(void)
{
	if (dpss_rdma_en)
		return 1;
	else
		return 0;
}

static struct rdma_regadr_s rdma_regadr_t6w[RDMA_CHANNEL] = {
	{
		DPSS_RDMA_AHB_START_ADDR_MAN,
		DPSS_RDMA_AHB_START_ADDR_MAN_MSB,
		DPSS_RDMA_AHB_END_ADDR_MAN,
		DPSS_RDMA_AHB_END_ADDR_MAN_MSB,
		DPSS_RDMA_ACCESS_MAN, 0,
		DPSS_RDMA_ACCESS_MAN, 1,
		DPSS_RDMA_ACCESS_MAN, 2,
		16, 4
	},
	{
		DPSS_RDMA_AHB_START_ADDR_1,
		DPSS_RDMA_AHB_START_ADDR_1_MSB,
		DPSS_RDMA_AHB_END_ADDR_1,
		DPSS_RDMA_AHB_END_ADDR_1_MSB,
		DPSS_RDMA_AUTO_SRC1_SEL,  0,
		DPSS_RDMA_ACCESS_AUTO,  1,
		DPSS_RDMA_ACCESS_AUTO,  5,
		17, 5
	},
	{
		DPSS_RDMA_AHB_START_ADDR_2,
		DPSS_RDMA_AHB_START_ADDR_2_MSB,
		DPSS_RDMA_AHB_END_ADDR_2,
		DPSS_RDMA_AHB_END_ADDR_2_MSB,
		DPSS_RDMA_AUTO_SRC2_SEL,  0,
		DPSS_RDMA_ACCESS_AUTO,  2,
		DPSS_RDMA_ACCESS_AUTO,  6,
		18, 6
	}
};

void dpss_rdma_man_wr_clear(void)
{
	rdma_info[0].rdma_item_count = 0;
}

void dpss_rdma_man_wr_test(unsigned int addr, unsigned int val)
{
	unsigned int i;

	dbg_a1("%s addr:%#x, val:%#x\n", __func__, addr, val);

	i = rdma_info[0].rdma_item_count;

	if ((i + 1) * 8 > rdma_info->rdma_table_size) {
		dbg_a1("dpss rdma in buffer overflow\n");
		return;
	}

	rdma_info[0].rdma_table_addr[i * 2] = addr & 0xffffffff;
	rdma_info[0].rdma_table_addr[i * 2 + 1] = val & 0xffffffff;

	rdma_info[0].rdma_item_count++;
}

void dpss_rdma_man_wr_tri(void)
{
	// unsigned int i;
	struct rdma_regadr_s *rdma_regadr;

	dbg_a1("%s man count:%d\n", __func__, rdma_info[0].rdma_item_count);

	rdma_regadr = (struct rdma_regadr_s *)rdma_info[0].rdma_regadr;

	if (1) {// (handle == 0) { //man
		// manual RDMA
		wr(rdma_regadr->trigger_mask_reg,
			rd(rdma_regadr->trigger_mask_reg) & (~1));
		#ifdef CONFIG_ARM64
		wr(rdma_regadr->rdma_ahb_start_addr,
			rdma_info[0].rdma_table_phy_addr & 0xffffffff);
		wr(rdma_regadr->rdma_ahb_start_addr_msb,
			(rdma_info[0].rdma_table_phy_addr >> 32) & 0xf);
		wr(rdma_regadr->rdma_ahb_end_addr,
			(rdma_info[0].rdma_table_phy_addr +
			rdma_info[0].rdma_item_count * 8 - 1) & 0xffffffff);
		wr(rdma_regadr->rdma_ahb_end_addr_msb,
			((rdma_info[0].rdma_table_phy_addr +
			rdma_info[0].rdma_item_count * 8 - 1) >> 32) & 0xf);
		#else
		wr(rdma_regadr->rdma_ahb_start_addr,
			rdma_info[0].rdma_table_phy_addr & 0xffffffff);
		wr(rdma_regadr->rdma_ahb_end_addr,
			(rdma_info[0].rdma_table_phy_addr +
			rdma_info[0].rdma_item_count * 8 - 1) & 0xffffffff);
		#endif

		w_reg_bit(rdma_regadr->addr_inc_reg, 0,
			rdma_regadr->addr_inc_reg_bitpos, 1);
		w_reg_bit(rdma_regadr->rw_flag_reg, 1,   //1:write
			rdma_regadr->rw_flag_reg_bitpos, 1);

		wr(rdma_regadr->trigger_mask_reg,
			rd(rdma_regadr->trigger_mask_reg) | 1);

		dbg_a1("config manual write done\n");
		rdma_info[0].rdma_item_count = 0;
	}
}

// pre_vs
void dpss_rdma_pre_vs_table_config(unsigned int addr, unsigned int val)
{
	unsigned int i;

	// dbg_a1("%s addr:%#x, val:%#x\n", __func__, addr, val);

	i = rdma_info[1].rdma_item_count; // pre_vs

	if ((i + 1) * 8 > rdma_info[1].rdma_table_size) {
		dbg_a1("dpss rdma in buffer overflow\n");
		return;
	}

	rdma_info[1].rdma_table_addr[i * 2] = addr & 0xffffffff;
	rdma_info[1].rdma_table_addr[i * 2 + 1] = val & 0xffffffff;

	rdma_info[1].rdma_item_count++;
}

// vsync
void dpss_rdma_vs_table_config(unsigned int addr, unsigned int val)
{
	unsigned int i;

	// dbg_a1("%s addr:%#x, val:%#x\n", __func__, addr, val);

	i = rdma_info[2].rdma_item_count; // vs

	if ((i + 1) * 8 > rdma_info[2].rdma_table_size) {
		dbg_a1("dpss rdma in buffer overflow\n");
		return;
	}

	rdma_info[2].rdma_table_addr[i * 2] = addr & 0xffffffff;
	rdma_info[2].rdma_table_addr[i * 2 + 1] = val & 0xffffffff;

	rdma_info[2].rdma_item_count++;
}

int _dpss_rdma_wr_reg_pre_vs(u32 addr, u32 val)
{
	// if (get_frc_devp()->power_on_flag == 0)
	// return 0;

	if (rdma_enable()) {
		dpss_rdma_pre_vs_table_config(addr, val);
		dbg_a2("rdma on, channel in addr:0x%04x, val:0x%08x\n",
			addr, val);
	} else {
		wr(addr, val);
		dbg_a2("rdma off, channel in addr:0x%04x, val:0x%08x\n",
			addr, val);
	}

	return 0;
}

int _dpss_rdma_wr_reg_vs(u32 addr, u32 val)
{
	// if (get_frc_devp()->power_on_flag == 0)
	// return 0;

	if (rdma_enable()) {
		dpss_rdma_vs_table_config(addr, val);
		dbg_a2("rdma on, channel in addr:0x%04x, val:0x%08x\n",
			addr, val);
	} else {
		wr(addr, val);
		dbg_a2("rdma off, channel in addr:0x%04x, val:0x%08x\n",
			addr, val);
	}

	return 0;
}

void DPSS_RDMA_WR_PRE_VS(u32 addr, u32 val)
{
	if (unlikely(addr < 0x8000 || addr > 0xffff)) {
		DBG_ERR("rdma not support addr:%x\n", addr);
		return;
	}

	_dpss_rdma_wr_reg_pre_vs(addr, val);
}
EXPORT_SYMBOL(DPSS_RDMA_WR_PRE_VS);

void DPSS_RDMA_WR_VS(u32 addr, u32 val)
{
	if (unlikely(addr < 0x8000 || addr > 0xffff)) {
		DBG_ERR("rdma not support addr:%x\n", addr);
		return;
	}

	_dpss_rdma_wr_reg_vs(addr, val);
}
EXPORT_SYMBOL(DPSS_RDMA_WR_VS);

void irq_rdma(void)
{
	unsigned int i, rdma_status;
	struct rdma_regadr_s *info_regadr;
	//struct frc_rdma_s *frc_rdma = get_frc_rdma();

	rdma_status = rd(irq_status.reg);

	dbg_a1("%s dpss rdma status[0x%x]\n", __func__, rdma_status);

	for (i = 0; i < RDMA_CHANNEL; i++) {
		info_regadr = (struct rdma_regadr_s *)rdma_info[i].rdma_regadr;
		if (rdma_status & (0x1 << (i + irq_status.start))) {
			wr(rdma_ctrl,
				0x1 << info_regadr->clear_irq_bitpos);
			dbg_a1("channel:%d rdma irq done\n", i);
			break;
		}
	}
}

void dpss_rdma_auto_wr_tri(u32 handle)
{
	unsigned int handle_irq = 0x2;
	struct rdma_regadr_s *rdma_regadr;
	struct dpss_rdma_info *rdma_info_p;

	rdma_info_p = &rdma_info[handle];
	rdma_regadr = rdma_info_p->rdma_regadr;

	if (!rdma_enable())
		return;

	if (handle == 1)
		handle_irq = rdma_handle_irq_pre_vs;
	else if (handle == 2)
		handle_irq = rdma_handle_irq_vs;

	dbg_a2("%s handle:%d auto count:%d\n", __func__,
		handle, rdma_info[handle].rdma_item_count);

	if (1) {
		//auto RDMA
		// w_reg_bit(rdma_regadr->trigger_mask_reg, 0,
		// rdma_regadr->trigger_mask_reg_bitpos, 1);
		wr(rdma_regadr->trigger_mask_reg, 0);

		#ifdef CONFIG_ARM64
		wr(rdma_regadr->rdma_ahb_start_addr,
			rdma_info_p->rdma_table_phy_addr & 0xffffffff);
		wr(rdma_regadr->rdma_ahb_start_addr_msb,
			(rdma_info_p->rdma_table_phy_addr >> 32) & 0xf);
		wr(rdma_regadr->rdma_ahb_end_addr,
			(rdma_info_p->rdma_table_phy_addr +
			rdma_info_p->rdma_item_count * 8 - 1) & 0xffffffff);
		wr(rdma_regadr->rdma_ahb_end_addr_msb,
			((rdma_info_p->rdma_table_phy_addr +
			rdma_info_p->rdma_item_count * 8 - 1) >> 32) & 0xf);
		#else
		wr(rdma_regadr->rdma_ahb_start_addr,
			rdma_info_p->rdma_table_phy_addr & 0xffffffff);
		wr(rdma_regadr->rdma_ahb_end_addr,
			(rdma_info_p->rdma_table_phy_addr +
			rdma_info_p->rdma_item_count * 8 - 1) & 0xffffffff);
		#endif

		w_reg_bit(rdma_regadr->addr_inc_reg, 0,
			rdma_regadr->addr_inc_reg_bitpos, 1);
		w_reg_bit(rdma_regadr->rw_flag_reg, 1,  // 1:write
			rdma_regadr->rw_flag_reg_bitpos, 1);

		// w_reg_bit(rdma_regadr->trigger_mask_reg, rdma_handle_irq,
		// rdma_regadr->trigger_mask_reg_bitpos, 31);
		wr(rdma_regadr->trigger_mask_reg, handle_irq);

		dbg_a2("config auto done rdma_handle_irq:%x count:%d\n",
			handle_irq, rdma_info_p->rdma_item_count);
		rdma_info_p->rdma_item_count = 0;
	}
}

void rdma_alloc_buf(struct dpss_dev_s *de_devp)
{
	int i;
	phys_addr_t dma_handle;
	// struct platform_device *pdev;
	// struct frc_rdma_info *rdma_info;

	if (!de_devp) {
		DBG_ERR("%s rdma alloc buf failed\n", __func__);
		return;
	}

	// pdev = (struct platform_device *)de_devp->pdev;
	// pdev->dev.coherent_dma_mask = DMA_BIT_MASK(64);
	// pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;

	for (i = 0; i < RDMA_CHANNEL; i++) {
		if (!rdma_info[i].buf_status) {
			rdma_info[i].rdma_table_addr =
				dma_alloc_coherent(&de_devp->pdev->dev,
				rdma_info[i].rdma_table_size, &dma_handle, 0);
			rdma_info[i].rdma_table_phy_addr = dma_handle;
			rdma_info[i].buf_status = 1;
		}
		if (sizeof(rdma_info[i].rdma_table_phy_addr) > 4)
			rdma_info[i].is_64bit_addr = 1; // 64bit addr
		else
			rdma_info[i].is_64bit_addr = 0;

		dbg_a2("%s channel:%d rdma_table_addr: %lx phy:%lx size:%x\n",
			__func__, i, (ulong)rdma_info[i].rdma_table_addr,
			(ulong)rdma_info[i].rdma_table_phy_addr,
			rdma_info[i].rdma_table_size);
	}
	buf_alloced = 1;
}

void rdma_release_buf(void)
{
	int i;
	struct dpss_dev_s *de_devp = dpss_get_devp();

	if (!de_devp)
		return;

	if (!buf_alloced) {
		DBG_ERR("%s buf alread released\n", __func__);
		return;
	}

	for (i = 0; i < RDMA_CHANNEL; i++) {
		if (rdma_info[i].buf_status) {
			dma_free_coherent(&de_devp->pdev->dev,
			rdma_info[i].rdma_table_size, rdma_info[i].rdma_table_addr,
			(dma_addr_t)rdma_info[i].rdma_table_phy_addr);
			rdma_info[i].buf_status = 0;
		}
	}
	buf_alloced = 0;

	dbg_a2("%s rdma buf released done, buf_alloced:%d\n", __func__, buf_alloced);
}

void dpss_rdma_info_init(void)
{
	int i;

	for (i = 0; i < RDMA_CHANNEL; i++) {
		rdma_info[i].rdma_table_addr = NULL;
		rdma_info[i].rdma_table_size = DPSS_RDMA_SIZE / 16;
		//if (i > 2) // dbg, rdma read
			//rdma_info[i].rdma_table_size = DPSS_RDMA_SIZE / 16;
		rdma_info[i].buf_status = 0;
		rdma_info[i].is_64bit_addr = 0;
		rdma_info[i].rdma_item_count = 0;
		rdma_info[i].rdma_write_count = 0;
		rdma_info[i].rdma_regadr = NULL;
	}
}

int dpss_rdma_init(void)
{
	u32 i, data32;
	// enum chip_id chip;
	// struct frc_dev_s *devp;
	// struct frc_fw_data_s *fw_data;
	struct dpss_dev_s *de_devp = dpss_get_devp();
	// struct frc_rdma_s *frc_rdma;

	// devp = get_frc_devp();
	// chip = get_chip_type();
	// frc_rdma = get_frc_rdma();
	//fw_data = (struct frc_fw_data_s *)devp->fw_data;

	dpss_rdma_info_init();
	// init frc rdma
	// for (i = 0; i < RDMA_CHANNEL; i++)
		// rdma_info[i] = &rdma_info[i];

	// alloc buf
	rdma_alloc_buf(de_devp);

	if (buf_alloced) {
		dpss_rdma_en = 1;			  // drv rdma en
		//frc_rdma->rdma_alg_en = 0;		  // alg rdma default off
		// fw_data->frc_top_type.dpss_rdma_en = 0;
	} else {
		DBG_ERR("alloc dpss rdma buffer failed\n");
		return 0;
	}

	for (i = 0; i < RDMA_CHANNEL; i++)
		rdma_info[i].rdma_regadr = &rdma_regadr_t6w[i];
	irq_status.reg = DPSS_RDMA_STATUS1;
	irq_status.start = 4;
	irq_status.len = 16;
	rdma_ctrl = DPSS_RDMA_CTRL;

	// rdma read init
	// frc_rdma_rd_table_init(devp);

	data32  = 0;
	data32 |= 0 << 7; /* write ddr urgent */
	data32 |= 1 << 6; /* read ddr urgent */
	// data32 |= 0 << 4;
	// data32 |= 0 << 2;
	// data32 |= 0 << 1;
	// data32 |= 0 << 0;

	// init clk
	// w_reg_bit(FRC_RDMA_SYNC_CTRL, 1, 2, 1);
	// w_reg_bit(FRC_RDMA_SYNC_CTRL, 1, 6, 1);
	// rdma config
	wr(rdma_ctrl, data32);
	dbg_a2("%s dpss rdma init done\n", __func__);
	return 1;
}

void dpss_rdma_exit(void)
{
	// struct frc_rdma_s *frc_rdma = get_frc_rdma();

	// close auto
	// w_reg_bit(FRC_RDMA_SYNC_CTRL, 0, 2, 1);
	// w_reg_bit(FRC_RDMA_SYNC_CTRL, 0, 6, 1);
	//wr(FRC_RDMA_AUTO_SRC1_SEL_T3X, 0);
	//wr(FRC_RDMA_AUTO_SRC2_SEL_T3X, 0);
	wr(rdma_ctrl, 0);
	rdma_release_buf();
	dpss_rdma_en = 0;

	dbg_a2("%s dpss rdma exit\n", __func__);
}
