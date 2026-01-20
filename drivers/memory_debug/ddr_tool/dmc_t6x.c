// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/of.h>
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

#define DMC_PROT0_STA			(0x00e0 << 2)
#define DMC_PROT0_EDA			(0x00e1 << 2)
#define DMC_PROT0_CTRL			(0x00e2 << 2)
#define DMC_PROT0_CTRL1			(0x00e3 << 2)

#define DMC_PROT1_STA			(0x00e4 << 2)
#define DMC_PROT1_EDA			(0x00e5 << 2)
#define DMC_PROT1_CTRL			(0x00e6 << 2)
#define DMC_PROT1_CTRL1			(0x00e7 << 2)

#define DMC_PROT_VIO_0			(0x00e8 << 2)
#define DMC_PROT_VIO_1			(0x00e9 << 2)
#define DMC_PROT_VIO_2			(0x00ea << 2)
#define DMC_PROT_VIO_3			(0x00eb << 2)
#define DMC_PROT_VIO_4			(0x00ec << 2)
#define DMC_PROT_ADDR_MASK		0x03
#define DMC_PROT_ADDR_OFFSET		11

#define DMC_VIO_PROT0			BIT(7)
#define DMC_VIO_PROT1			BIT(8)

#define DMC_PROT_IRQ_CTRL_STS		(0x00ed << 2)

#define DMC_SEC_STATUS			(0x061a << 2)
#define DMC_VIO_ADDR0			(0x061b << 2)
#define DMC_VIO_ADDR1			(0x061c << 2)
#define DMC_VIO_ADDR2			(0x061d << 2)
#define DMC_VIO_ADDR3			(0x061e << 2)
#define DMC_VIO_ADDR_MASK		0x03
#define DMC_VIO_ADDR_OFFSET		17

/* STS0 save addr, STS1 save prot id, */
#define DMC_PROT_STS0			(0x0280 << 2)
#define DMC_PROT_STS1			(0x0281 << 2)
#define DMC_PROT_STS127			(0x02ff << 2)

static size_t t6x_dmc_dump_reg(char *buf)
{
	size_t sz = 0, i, j;
	unsigned long val;
	void *io;

	for (j = 0; j < dmc_mon->mon_number; j++) {
		io = dmc_mon->mon_comm[j].io_mem;
		io = dmc_mon->mon_comm[j].io_mem;

		sz += sprintf(buf + sz, "\nDMC%zu:\n", j);
		for (i = 0; i < 2; i++) {
			val = dmc_prot_rw(io, DMC_PROT0_STA + (i * 16), 0, DMC_READ);
			sz += sprintf(buf + sz, "DMC_PROT%zu_STA:%lx\n", i, val);
			val = dmc_prot_rw(io, DMC_PROT0_EDA + (i * 16), 0, DMC_READ);
			sz += sprintf(buf + sz, "DMC_PROT%zu_EDA:%lx\n", i, val);
			val = dmc_prot_rw(io, DMC_PROT0_CTRL + (i * 16), 0, DMC_READ);
			sz += sprintf(buf + sz, "DMC_PROT%zu_CTRL:%lx\n", i, val);
			val = dmc_prot_rw(io, DMC_PROT0_CTRL1 + (i * 16), 0, DMC_READ);
			sz += sprintf(buf + sz, "DMC_PROT%zu_CTRL:%lx\n", i, val);
		}
		for (i = 0; i < 5; i++) {
			val = dmc_prot_rw(io, DMC_PROT_VIO_0 + (i << 2), 0, DMC_READ);
			sz += sprintf(buf + sz, "DMC_PROT_VIO_%zu:%lx\n", i, val);
		}

		val = dmc_prot_rw(io, DMC_PROT_IRQ_CTRL_STS, 0, DMC_READ);
		sz += sprintf(buf + sz, "DMC_PROT_IRQ_CTRL_STS:%lx\n", val);
	}

	return sz;
}

static int check_violation(struct dmc_monitor *mon, void *data)
{
	int ret = -1, axuser_offset;
	unsigned long irqreg;
	struct dmc_mon_comm *mon_comm = (struct dmc_mon_comm *)data;

	/* only clear prot cache */
	dmc_prot_rw(mon_comm->io_mem, DMC_PROT_IRQ_CTRL_STS, 0xc, DMC_WRITE);

	/* irq write */
	irqreg = dmc_prot_rw(mon_comm->io_mem, DMC_PROT_IRQ_CTRL_STS, 0, DMC_READ);

	if (irqreg & DMC_WRITE_VIOLATION) {
		/* combine address */
		mon_comm->time = sched_clock();
		mon_comm->status = dmc_prot_rw(mon_comm->io_mem, DMC_PROT_VIO_1, 0, DMC_READ);
		mon_comm->addr  = (mon_comm->status >> DMC_PROT_ADDR_OFFSET) & DMC_PROT_ADDR_MASK;
		mon_comm->addr  = DMC_ADDR_HIGH(mon_comm->addr);
		mon_comm->addr |= dmc_prot_rw(mon_comm->io_mem, DMC_PROT_VIO_0, 0, DMC_READ);
		mon_comm->rw = 'w';
		ret = 0;
	} else if (irqreg & DMC_READ_VIOLATION) {
		/* irq read */
		mon_comm->time = sched_clock();
		mon_comm->status = dmc_prot_rw(mon_comm->io_mem, DMC_PROT_VIO_3, 0, DMC_READ);
		/* combine address */
		mon_comm->addr  = (mon_comm->status >> DMC_PROT_ADDR_OFFSET) & DMC_PROT_ADDR_MASK;
		mon_comm->addr  = DMC_ADDR_HIGH(mon_comm->addr);
		mon_comm->addr |= dmc_prot_rw(mon_comm->io_mem, DMC_PROT_VIO_2, 0, DMC_READ);
		mon_comm->rw = 'r';
		ret = 0;
	}

	if (!ret) {
		if (mon_comm->rw == 'r')
			axuser_offset = 29;
		else
			axuser_offset = 23;
		mon_comm->port.number = (mon_comm->status >> 9) & 0x3;
		mon_comm->port.number = (mon_comm->port.number << 3);
		mon_comm->port.number |= ((mon_comm->status >> axuser_offset) & 0x7);
		mon_comm->sub.number = (mon_comm->status >> 13) & 0x3ff;
		dmc_vio_check_page(data);
	}

	return ret;
}

static void t6x_recheck_violation(void *data)
{
	unsigned int i, count = 0, same = 0, invalid = 0, reg0, reg1, port, subport;
	unsigned long addr;
	struct dmc_mon_comm *mon_comm = (struct dmc_mon_comm *)data;
	void *io = mon_comm->io_mem;
	unsigned long long t;

	count = dmc_prot_rw(io, DMC_PROT_VIO_4, 0, DMC_READ);
	mon_comm->prot_buf.total = count;

	t = sched_clock();
	for (i = 0; i < count; i++) {
		reg0 = dmc_prot_rw(io, DMC_PROT_STS0 + ((2 * i) << 2), 0, DMC_READ);
		reg1 = dmc_prot_rw(io, DMC_PROT_STS1 + ((2 * i) << 2), 0, DMC_READ);

		addr = reg1 & DMC_PROT_ADDR_MASK;
		addr = DMC_ADDR_HIGH(addr) | reg0;
		port = (reg1 >> 2) & 0x3;
		port = (port << 3);
		port |= ((reg1 >> 14) & 0x7);
		subport = (reg1 >> 4) & 0x3ff;
		dmc_recheck_invalid(mon_comm, addr, port, subport, &same, &invalid);

		if (sched_clock() - t >= get_recheck_ns())
			break;
	}
	mon_comm->prot_buf.same = same;
	mon_comm->prot_buf.invalid = invalid;
}

static int t6x_dmc_mon_irq(struct dmc_monitor *mon, void *data, char clear)
{
	unsigned long irqreg;
	struct dmc_mon_comm *mon_comm = (struct dmc_mon_comm *)data;

	if (clear) {
		/* clear irq */
		irqreg = dmc_prot_rw(mon_comm->io_mem, DMC_PROT_IRQ_CTRL_STS, 0, DMC_READ);
		irqreg |= 0x04;		/* en */
		irqreg |= BIT(3);	/* clear prot cache */
		dmc_prot_rw(mon_comm->io_mem, DMC_PROT_IRQ_CTRL_STS, irqreg, DMC_WRITE);
	} else {
		return check_violation(mon, data);
	}

	return 0;
}

static void t6x_dmc_vio_to_port(void *data, unsigned long *vio_bit)
{
	struct dmc_mon_comm *mon_comm = (struct dmc_mon_comm *)data;

	*vio_bit = DMC_VIO_PROT1 | DMC_VIO_PROT0;
	set_port_to_mon_comm(data, mon_comm->port.number, mon_comm->sub.number);
}

static int t6x_dmc_mon_set(struct dmc_monitor *mon)
{
	int i;
	unsigned long start, end, value;
	void *io;

	/* aligned to 4KB */
	start = mon->addr_start >> PAGE_SHIFT;
	end   = ALIGN(mon->addr_end, PAGE_SIZE);
	end   = end >> PAGE_SHIFT;

	for (i = 0; i < dmc_mon->mon_number; i++) {
		io = dmc_mon->mon_comm[i].io_mem;

		dmc_prot_rw(io, DMC_PROT_IRQ_CTRL_STS, 0x3, DMC_WRITE);

		dmc_prot_rw(io, DMC_PROT0_STA, start, DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT0_EDA, end, DMC_WRITE);

		value = mon->device;
		dmc_prot_rw(io, DMC_PROT0_CTRL, value, DMC_WRITE);

		value = 0x0;
		if (dmc_mon->debug & DMC_DEBUG_WRITE)
			value |= 0x2;
		/* if set, will be crash when read access */
		if (dmc_mon->debug & DMC_DEBUG_READ)
			value |= 0x1;
		dmc_prot_rw(io, DMC_PROT0_CTRL1, value, DMC_WRITE);

		value = 0x7;
		dmc_prot_rw(io, DMC_PROT_IRQ_CTRL_STS, value, DMC_WRITE);
	}

	pr_emerg("range:%08lx - %08lx, device:%llx, debug:%x\n",
		 mon->addr_start, mon->addr_end, mon->device, mon->debug);
	return 0;
}

void t6x_dmc_mon_disable(struct dmc_monitor *mon)
{
	void *io;
	int i;

	for (i = 0; i < dmc_mon->mon_number; i++) {
		io = dmc_mon->mon_comm[i].io_mem;
		dmc_prot_rw(io, DMC_PROT0_STA,   0, DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT0_EDA,   0, DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT1_STA,   0, DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT1_EDA,   0, DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT0_CTRL,  0, DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT1_CTRL,  0, DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT0_CTRL1,  0, DMC_WRITE);
		dmc_prot_rw(io, DMC_PROT1_CTRL1,  0, DMC_WRITE);
	}
	mon->device     = 0;
	mon->addr_start = 0;
	mon->addr_end   = 0;
}

static int reg_analysis(char *input, char *output)
{
	unsigned long status, vio_reg0, vio_reg1, vio_reg2, vio_reg3;
	int port, subport, remap_id, count = 0;
	unsigned long addr;
	char rw = 'n';

	if (sscanf(input, "%lx %lx %lx %lx %lx",
			 &status, &vio_reg0, &vio_reg1,
			 &vio_reg2, &vio_reg3) != 5) {
		return 0;
	}

	if (status & DMC_READ_VIOLATION) { /* read */
		addr = (vio_reg3 >> DMC_VIO_ADDR_OFFSET) & DMC_VIO_ADDR_MASK;
		addr = DMC_ADDR_HIGH(addr) | vio_reg2;
		subport = (vio_reg3 >> 19) & 0x3ff;
		port = (vio_reg3 & 0x3) << 3;
		remap_id = (vio_reg3 >> 2) & 0x7f;
		if (port == 0x18) {
			if (remap_id <= 9)
				port |= 0x0;
			else if (remap_id <= 25)
				port |= 0x1;
			else if (remap_id <= 27)
				port |= 0x2;
			else if (remap_id <= 54)
				port |= 0x3;
			else if (remap_id <= 56)
				port |= 0x4;
			else if (remap_id <= 71)
				port |= 0x5;
			else
				port |= 0x6;
		}
		rw = 'r';

		count += sprintf(output + count, "DMC READ:");
		count += sprintf(output + count, "addr=%09lx port=%s sub=%s\n",
				 addr, to_ports(port), to_sub_ports_name(port, subport, rw));
	}

	if (status & DMC_WRITE_VIOLATION) { /* write */
		addr = (vio_reg1 >> DMC_VIO_ADDR_OFFSET) & DMC_VIO_ADDR_MASK;
		addr = DMC_ADDR_HIGH(addr) | vio_reg0;
		subport = (vio_reg1 >> 19) & 0x3ff;
		port = (vio_reg1 & 0x3) << 3;
		remap_id = (vio_reg1 >> 2) & 0x7f;
		if (port == 0x18) {
			if (remap_id <= 9)
				port |= 0x0;
			else if (remap_id <= 25)
				port |= 0x1;
			else if (remap_id <= 27)
				port |= 0x2;
			else if (remap_id <= 54)
				port |= 0x3;
			else if (remap_id <= 56)
				port |= 0x4;
			else if (remap_id <= 71)
				port |= 0x5;
			else
				port |= 0x6;
		}
		rw = 'w';
		count += sprintf(output + count, "DMC WRITE:");
		count += sprintf(output + count, "addr=%09lx port=%s sub=%s\n",
				 addr, to_ports(port), to_sub_ports_name(port, subport, rw));
	}
	return count;
}

static int dmc_sec_check(char *output, unsigned char index)
{
	struct dmc_mon_comm tmp;
	unsigned long dmc_vio_status, dmc_vio_reg[4];
	int count = 0, remap_id, i;

	tmp.rw = 'n';
	tmp.io_base = dmc_mon->mon_comm[index].io_base;
	dmc_vio_status = dmc_rw(tmp.io_base + DMC_SEC_STATUS, 0, DMC_READ);
	for (i = 0; i < 4; i++)
		dmc_vio_reg[i] = dmc_rw(tmp.io_base + DMC_VIO_ADDR0 + (i << 2), 0, DMC_READ);

	if (dmc_vio_status & DMC_READ_VIOLATION) {
		tmp.rw = 'r';
		tmp.addr = (dmc_vio_reg[3] >> DMC_VIO_ADDR_OFFSET) & DMC_VIO_ADDR_MASK;
		tmp.addr = DMC_ADDR_HIGH(tmp.addr) | dmc_vio_reg[2];
		tmp.sub.number = (dmc_vio_reg[3] >> 19) & 0x3ff;
		tmp.port.number = (dmc_vio_reg[3] & 0x3) << 3;
		remap_id = (dmc_vio_reg[3] >> 2) & 0x7f;
		if (tmp.port.number == 0x18) {
			if (remap_id <= 9)
				tmp.port.number |= 0x0;
			else if (remap_id <= 25)
				tmp.port.number |= 0x1;
			else if (remap_id <= 27)
				tmp.port.number |= 0x2;
			else if (remap_id <= 54)
				tmp.port.number |= 0x3;
			else if (remap_id <= 56)
				tmp.port.number |= 0x4;
			else if (remap_id <= 71)
				tmp.port.number |= 0x5;
			else
				tmp.port.number |= 0x6;
		}
		count += dmc_sec_save_info(output + count, index, &tmp);
	}
	if (dmc_vio_status & DMC_WRITE_VIOLATION) {
		tmp.rw = 'w';
		tmp.addr = (dmc_vio_reg[1] >> DMC_VIO_ADDR_OFFSET) & DMC_VIO_ADDR_MASK;
		tmp.addr = DMC_ADDR_HIGH(tmp.addr) | dmc_vio_reg[0];
		tmp.sub.number = (dmc_vio_reg[1] >> 19) & 0x3ff;
		tmp.port.number = (dmc_vio_reg[1] & 0x3) << 3;
		remap_id = (dmc_vio_reg[1] >> 2) & 0x7f;
		if (tmp.port.number == 0x18) {
			if (remap_id <= 9)
				tmp.port.number |= 0x0;
			else if (remap_id <= 25)
				tmp.port.number |= 0x1;
			else if (remap_id <= 27)
				tmp.port.number |= 0x2;
			else if (remap_id <= 54)
				tmp.port.number |= 0x3;
			else if (remap_id <= 56)
				tmp.port.number |= 0x4;
			else if (remap_id <= 71)
				tmp.port.number |= 0x5;
			else
				tmp.port.number |= 0x6;
		}
		count += dmc_sec_save_info(output + count, index, &tmp);
	}

	count += dmc_sec_save_reg(output + count, index, dmc_vio_status, dmc_vio_reg, 4);

	return count;
}

static int t6x_dmc_reg_control(char *input, char control, char *output)
{
	int count = 0;
	unsigned char index = 0;

	switch (control) {
	case 'a':	/* analysis sec vio reg */
		count = reg_analysis(input, output);
		break;
	case 'c':	/* clear sec statue reg */
		index = input ? (unsigned char)*input : index;
		dmc_rw(dmc_mon->mon_comm[index].io_base + DMC_SEC_STATUS, 0x3, DMC_WRITE);
		break;
	case 'd':	/* dump sec vio reg */
		index = input ? (unsigned char)*input : index;
		count = dmc_sec_check(output, index);
		break;
	default:
		break;
	}

	return count;
}

struct dmc_mon_ops t6x_dmc_mon_ops = {
	.handle_irq  = t6x_dmc_mon_irq,
	.vio_to_port = t6x_dmc_vio_to_port,
	.set_monitor = t6x_dmc_mon_set,
	.disable     = t6x_dmc_mon_disable,
	.dump_reg    = t6x_dmc_dump_reg,
	.reg_control = t6x_dmc_reg_control,
	.recheck_violation = t6x_recheck_violation,
};
