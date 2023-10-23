/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __AML_IOTRACE_H
#define __AML_IOTRACE_H

extern int ramoops_io_skip;
extern int ramoops_ftrace_en;
extern int ramoops_trace_mask;
DECLARE_PER_CPU(bool, vsync_iotrace_cut);
DECLARE_PER_CPU(bool, frc_iotrace_cut);
DECLARE_PER_CPU(bool, amvecm_iotrace_cut);
DECLARE_PER_CPU(bool, usb_iotrace_cut);

#define BUF_SIZE 1024

void notrace __nocfi pstore_io_save(unsigned long reg, unsigned long val, unsigned int flag,
							unsigned long *irq_flags);

#ifdef CONFIG_SHADOW_CALL_STACK
unsigned long get_prev_lr_val(unsigned long lr, unsigned long offset);
#endif

#define PSTORE_FLAG_IO_R		0x0
#define PSTORE_FLAG_IO_W		0x1
#define PSTORE_FLAG_IO_R_END	0x2
#define PSTORE_FLAG_IO_W_END	0x3

#if IS_ENABLED(CONFIG_AMLOGIC_DEBUG_IOTRACE)
#define pstore_ftrace_io_wr(reg, val, irqflg)	\
pstore_io_save(reg, val, PSTORE_FLAG_IO_W, &(irqflg))

#define pstore_ftrace_io_wr_end(reg, val, irqflg)	\
pstore_io_save(reg, val, PSTORE_FLAG_IO_W_END, &(irqflg))

#define pstore_ftrace_io_rd(reg, irqflg)		\
pstore_io_save(reg, 0, PSTORE_FLAG_IO_R, &(irqflg))

#define pstore_ftrace_io_rd_end(reg, val, irqflg)	\
pstore_io_save(reg, val, PSTORE_FLAG_IO_R_END, &(irqflg))

#else
#define pstore_ftrace_io_wr(reg, val, irqflg)                   do {    } while (0)
#define pstore_ftrace_io_rd(reg, irqflg)                        do {    } while (0)
#define pstore_ftrace_io_wr_end(reg, val, irqflg)               do {    } while (0)
#define pstore_ftrace_io_rd_end(reg, val, irqflg)               do {    } while (0)

#endif /* CONFIG_AMLOGIC_DEBUG_IOTRACE */

enum aml_pstore_type_id {
	AML_PSTORE_TYPE_IO      = 0,
	AML_PSTORE_TYPE_SCHED   = 1,
	AML_PSTORE_TYPE_IRQ     = 2,
	AML_PSTORE_TYPE_SMC     = 3,
	AML_PSTORE_TYPE_MISC    = 4,
	AML_PSTORE_TYPE_MAX
};

struct io_trace_data {
	unsigned int reg;
	unsigned int val;
	unsigned long ip;
	unsigned long parent_ip;
};

void aml_pstore_write(enum aml_pstore_type_id type, char *buf, unsigned long size,
			unsigned int dis_irq, unsigned int io_flag);

int ftrace_ramoops_init(void);
#endif

