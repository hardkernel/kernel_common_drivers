/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _MMC_COMMON_H_
#define _MMC_COMMON_H_

int mmc_read_internal(struct mmc_card *card, unsigned int dev_addr,
						unsigned int blocks, void *buf);

int mmc_write_internal(struct mmc_card *card, unsigned int dev_addr,
						unsigned int blocks, void *buf);

int get_reserve_partition_off_from_tbl(void);

int aml_disable_mmc_cqe(struct mmc_card *card);

int aml_enable_mmc_cqe(struct mmc_card *card);

void sdio_clk_always_on(bool clk_aws_on);

void sdio_set_max_regs(unsigned int size);

#endif

