// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/of_fdt.h>
#include <linux/irqreturn.h>
#include <linux/module.h>
#include <linux/mm.h>

#include <linux/cpu.h>
#include <linux/smp.h>
#include <linux/kallsyms.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/sched/clock.h>
#include <linux/amlogic/page_trace.h>
#include "ddr_port.h"
#include "dmc_monitor.h"

#define DMC_PROT0_STA		((0x00e0  << 2))
#define DMC_PROT0_EDA		((0x00e1  << 2))
#define DMC_PROT0_CTRL		((0x00e2  << 2))
#define DMC_PROT0_CTRL1		((0x00e3  << 2))

#define DMC_PROT1_STA		((0x00e4  << 2))
#define DMC_PROT1_EDA		((0x00e5  << 2))
#define DMC_PROT1_CTRL		((0x00e6  << 2))
#define DMC_PROT1_CTRL1		((0x00e7  << 2))

#define DMC_PROT_VIO_0		((0x00e8  << 2))
#define DMC_PROT_VIO_1		((0x00e9  << 2))

#define DMC_PROT_VIO_2		((0x00ea  << 2))
#define DMC_PROT_VIO_3		((0x00eb  << 2))

#define DMC_PROT_IRQ_CTRL_STS	((0x00ec  << 2))

#define DMC_SEC_STATUS		(0x051a << 2)
#define DMC_VIO_ADDR0		(0x051b << 2)
#define DMC_VIO_ADDR1		(0x051c << 2)
#define DMC_VIO_ADDR2		(0x051d << 2)
#define DMC_VIO_ADDR3		(0x051e << 2)

#define DMC_VIO_PROT0		BIT(22)
#define DMC_VIO_PROT1		BIT(23)

static size_t a4_dmc_dump_reg(char *buf)
{
	size_t sz = 0, i;
	unsigned long val;
	void *io = dmc_mon->mon_comm[0].io_mem;

	for (i = 0; i < 2; i++) {
		val = dmc_prot_rw(io, DMC_PROT0_STA + (i * 16), 0, DMC_READ);
		sz += sprintf(buf + sz, "DMC_PROT%zu_STA:%lx\n", i, val);
		val = dmc_prot_rw(io, DMC_PROT0_EDA + (i * 16), 0, DMC_READ);
		sz += sprintf(buf + sz, "DMC_PROT%zu_EDA:%lx\n", i, val);
		val = dmc_prot_rw(io, DMC_PROT0_CTRL + (i * 16), 0, DMC_READ);
		sz += sprintf(buf + sz, "DMC_PROT%zu_CTRL:%lx\n", i, val);
		val = dmc_prot_rw(io, DMC_PROT0_CTRL1 + (i * 16), 0, DMC_READ);
		sz += sprintf(buf + sz, "DMC_PROT%zu_CTRL1:%lx\n", i, val);
	}
	for (i = 0; i < 4; i++) {
		val = dmc_prot_rw(io, DMC_PROT_VIO_0 + (i << 2), 0, DMC_READ);
		sz += sprintf(buf + sz, "DMC_PROT_VIO_%zu:%lx\n", i, val);
	}
	val = dmc_prot_rw(io, DMC_PROT_IRQ_CTRL_STS, 0, DMC_READ);
	sz += sprintf(buf + sz, "DMC_PROT_IRQ_CTRL:%lx\n", val);

	return sz;
}

static int check_violation(struct dmc_monitor *mon, void *data)
{
	int ret = -1;
	unsigned long irqreg;
	struct dmc_mon_comm *mon_comm = (struct dmc_mon_comm *)data;

	irqreg = dmc_prot_rw(mon_comm->io_mem, DMC_PROT_IRQ_CTRL_STS, 0, DMC_READ);
	if (irqreg & DMC_WRITE_VIOLATION) {
		mon_comm->time = sched_clock();
		mon_comm->status = dmc_prot_rw(mon_comm->io_mem, DMC_PROT_VIO_1, 0, DMC_READ);
		/* combine address */
		mon_comm->addr = dmc_prot_rw(mon_comm->io_mem, DMC_PROT_VIO_0, 0, DMC_READ);
		mon_comm->rw = 'w';
		ret = 0;
	} else if (irqreg & DMC_READ_VIOLATION) {
		mon_comm->time = sched_clock();
		mon_comm->status = dmc_prot_rw(mon_comm->io_mem, DMC_PROT_VIO_3, 0, DMC_READ);
		mon_comm->addr = dmc_prot_rw(mon_comm->io_mem, DMC_PROT_VIO_2, 0, DMC_READ);
		mon_comm->rw = 'r';
		ret = 0;
	}

	if (!ret) {
		mon_comm->port.number = mon_comm->status & 0xff;
		mon_comm->sub.number = (mon_comm->status >> 8) & 0x7f;
		dmc_vio_check_page(data);
	}

	return ret;
}

static int a4_dmc_mon_irq(struct dmc_monitor *mon, void *data, char clear)
{
	unsigned long irqreg;
	struct dmc_mon_comm *mon_comm = (struct dmc_mon_comm *)data;

	if (clear) {
		/* clear irq */
		irqreg = dmc_prot_rw(mon_comm->io_mem, DMC_PROT_IRQ_CTRL_STS, 0, DMC_READ);
		irqreg |= 0x04;		/* en */
		dmc_prot_rw(mon_comm->io_mem, DMC_PROT_IRQ_CTRL_STS, irqreg, DMC_WRITE);
	} else {
		return check_violation(mon, data);
	}

	return 0;
}

static void a4_dmc_vio_to_port(void *data, unsigned long *vio_bit)
{
	struct dmc_mon_comm *mon_comm = (struct dmc_mon_comm *)data;

	*vio_bit = DMC_VIO_PROT1 | DMC_VIO_PROT0;
	set_port_to_mon_comm(data, mon_comm->port.number, mon_comm->sub.number);
}

static int a4_dmc_mon_set(struct dmc_monitor *mon)
{
	unsigned long start, end, val;
	void *io;
	unsigned int dev1, dev2;

	if (!dmc_mon->device)
		return 0;

	start = mon->addr_start >> PAGE_SHIFT;
	end   = ALIGN(mon->addr_end, PAGE_SIZE);
	end   = end >> PAGE_SHIFT;
	dev1  = mon->device & 0xffffffff;
	dev2  = mon->device >> 32;

	io = dmc_mon->mon_comm[0].io_mem;
	dmc_prot_rw(io, DMC_PROT_IRQ_CTRL_STS, 0x3, DMC_WRITE);

	if (dev1) {
		dmc_prot_rw(io, DMC_PROT0_STA, start, DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT0_EDA, end, DMC_WRITE);
	} else {
		dmc_prot_rw(io, DMC_PROT0_STA, 0, DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT0_EDA, 0, DMC_WRITE);
	}

	/* when set exclude, PROT1 can not be used */
	if (dev2 && (dmc_mon->debug & DMC_DEBUG_INCLUDE)) {
		dmc_prot_rw(io, DMC_PROT1_STA, start, DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT1_EDA, end, DMC_WRITE);
	} else {
		dmc_prot_rw(io, DMC_PROT1_STA, 0UL, DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT1_EDA, 0UL, DMC_WRITE);
	}

	val = 0xf;
	if (dmc_mon->debug & DMC_DEBUG_WRITE)
		val |= (1 << 27);

	if (dmc_mon->debug & DMC_DEBUG_READ)
		val |= (1 << 26);

	if (dmc_mon->debug & DMC_DEBUG_INCLUDE)
		val |= (1 << 8);
	else
		val &= ~(1 << 8);

	if (dev1) {
		dmc_prot_rw(io, DMC_PROT0_CTRL,  val,  DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT0_CTRL1, dev1, DMC_WRITE);
	} else {
		dmc_prot_rw(io, DMC_PROT0_CTRL,  0UL,  DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT0_CTRL1, 0UL, DMC_WRITE);
	}

	/* when set exclude, PROT1 can not be used */
	if (dev2 && (dmc_mon->debug & DMC_DEBUG_INCLUDE)) {
		dmc_prot_rw(io, DMC_PROT1_CTRL,  val,  DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT1_CTRL1, dev2, DMC_WRITE);
	} else {
		dmc_prot_rw(io, DMC_PROT1_CTRL,  0UL,  DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT1_CTRL1, 0UL, DMC_WRITE);
	}

	val = 0x7;
	dmc_prot_rw(io, DMC_PROT_IRQ_CTRL_STS, val, DMC_WRITE);

	pr_emerg("range:%08lx - %08lx, device:%16llx\n",
		 mon->addr_start, mon->addr_end, mon->device);
	return 0;
}

void a4_dmc_mon_disable(struct dmc_monitor *mon)
{
	void *io = dmc_mon->mon_comm[0].io_mem;

	dmc_prot_rw(io, DMC_PROT0_STA,   0, DMC_WRITE);
	dmc_prot_rw(io, DMC_PROT0_EDA,   0, DMC_WRITE);
	dmc_prot_rw(io, DMC_PROT1_STA,   0, DMC_WRITE);
	dmc_prot_rw(io, DMC_PROT1_EDA,   0, DMC_WRITE);
	dmc_prot_rw(io, DMC_PROT0_CTRL,  0, DMC_WRITE);
	dmc_prot_rw(io, DMC_PROT0_CTRL1, 0, DMC_WRITE);
	dmc_prot_rw(io, DMC_PROT1_CTRL,  0, DMC_WRITE);
	dmc_prot_rw(io, DMC_PROT1_CTRL1, 0, DMC_WRITE);
	mon->device     = 0;
	mon->addr_start = 0;
	mon->addr_end   = 0;
}

static int a4_reg_analysis(char *input, char *output)
{
	unsigned long status, vio_reg0, vio_reg1, vio_reg2, vio_reg3;
	int port, subport, count = 0;
	unsigned long addr;
	char rw = 'n';

	if (sscanf(input, "%lx %lx %lx %lx %lx",
			 &status, &vio_reg0, &vio_reg1,
			 &vio_reg2, &vio_reg3) != 5) {
		pr_emerg("%s param input error, buf:%s\n", __func__, input);
		return 0;
	}

	if (status & 0x1) { /* read */
		addr = vio_reg2;
		port = vio_reg3 & 0xff;
		subport = (vio_reg3 >> 8) & 0x7f;
		rw = 'r';
		count += sprintf(output + count, "DMC READ:");
		count += sprintf(output + count, "addr=%09lx port=%s sub=%s\n",
				 addr, to_ports(port), to_sub_ports_name(port, subport, rw));
	}

	if (status & 0x2) { /* write */
		addr = vio_reg0;
		port = vio_reg1 & 0xff;
		subport = (vio_reg1 >> 8) & 0x7f;
		rw = 'w';
		count += sprintf(output + count, "DMC WRITE:");
		count += sprintf(output + count, "addr=%09lx port=%s sub=%s\n",
				 addr, to_ports(port), to_sub_ports_name(port, subport, rw));
	}
	return count;
}

static int dmc_sec_check(char *output)
{
	struct dmc_mon_comm tmp;
	unsigned long dmc_vio_status, dmc_vio_reg[4];
	int count = 0, i;

	tmp.rw = 'n';
	dmc_vio_status = dmc_prot_rw(NULL, DMC_SEC_STATUS, 0, DMC_READ);
	for (i = 0; i < 4; i++)
		dmc_vio_reg[i] = dmc_prot_rw(NULL, DMC_VIO_ADDR0 + (i << 2), 0, DMC_READ);

	if (dmc_vio_status & 0x1) {
		tmp.rw = 'r';
		tmp.addr =  dmc_vio_reg[2];
		tmp.port.number =  dmc_vio_reg[3] & 0xff;
		tmp.sub.number = (dmc_vio_reg[3] >> 8) & 0x7f;
		count += dmc_sec_save_info(output + count, 0, &tmp);
	}
	if (dmc_vio_status & 0x2) {
		tmp.rw = 'w';
		tmp.addr =  dmc_vio_reg[0];
		tmp.port.number =  dmc_vio_reg[1] & 0xff;
		tmp.sub.number = (dmc_vio_reg[1] >> 8) & 0x7f;
		count += dmc_sec_save_info(output + count, 0, &tmp);
	}

	count += dmc_sec_save_reg(output + count, 0, dmc_vio_status, dmc_vio_reg, 4);

	return count;
}

static int a4_dmc_reg_control(char *input, char control, char *output)
{
	int count = 0;

	switch (control) {
	case 'a':	/* analysis vio reg */
		count = a4_reg_analysis(input, output);
		break;
	case 'c':	/* clear sec statue reg */
		dmc_prot_rw(NULL, DMC_SEC_STATUS, 0x3, DMC_WRITE);
		break;
	case 'd':	/* dump sec vio reg */
		count = dmc_sec_check(output);
		break;
	default:
		break;
	}

	return count;
}

struct dmc_mon_ops a4_dmc_mon_ops = {
	.handle_irq  = a4_dmc_mon_irq,
	.vio_to_port = a4_dmc_vio_to_port,
	.set_monitor = a4_dmc_mon_set,
	.disable     = a4_dmc_mon_disable,
	.dump_reg    = a4_dmc_dump_reg,
	.reg_control = a4_dmc_reg_control,
};
