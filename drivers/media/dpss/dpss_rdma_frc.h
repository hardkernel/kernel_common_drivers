/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPSS_RDMA_FRC_H__
#define __DPSS_RDMA_FRC_H__

#define RDMA_CHANNEL 3

#ifndef PAGE_SIZE
# define PAGE_SIZE 4096
#endif

/*RDMA total memory size is 1M*/
#define DPSS_RDMA_SIZE (256 * (PAGE_SIZE))

struct rdma_irq_reg_s {
	u32 reg;
	u32 start;
	u32 len;
};

struct rdma_regadr_s {
	u32 rdma_ahb_start_addr;
	u32 rdma_ahb_start_addr_msb;
	u32 rdma_ahb_end_addr;
	u32 rdma_ahb_end_addr_msb;
	u32 trigger_mask_reg;
	u32 trigger_mask_reg_bitpos;
	u32 addr_inc_reg;
	u32 addr_inc_reg_bitpos;
	u32 rw_flag_reg;
	u32 rw_flag_reg_bitpos;
	u32 clear_irq_bitpos;
	u32 irq_status_bitpos;
};

struct dpss_rdma_info {
	ulong rdma_table_phy_addr;
	u32 *rdma_table_addr;
	int rdma_table_size;
	u8 buf_status;
	u8 is_64bit_addr;
	int rdma_item_count;
	int rdma_write_count;
	struct rdma_regadr_s *rdma_regadr;
};

void irq_rdma(void);
int  dpss_rdma_init(void);
void dpss_rdma_exit(void);
void dpss_rdma_man_wr_test(unsigned int addr, unsigned int val);
void dpss_rdma_man_wr_clear(void);
void dpss_rdma_man_wr_tri(void);

int rdma_enable(void);
// void dpss_rdma_auto_wr_reg(unsigned int addr, unsigned int val);
void dpss_rdma_auto_wr_tri(u32 handle);
void DPSS_RDMA_WR_PRE_VS(u32 addr, u32 val);
void DPSS_RDMA_WR_VS(u32 addr, u32 val);

#endif //__DPSS_RDMA_FRC_H__
