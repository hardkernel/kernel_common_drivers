// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/export.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/amlogic/iomap.h>
#include <linux/amlogic/secmon.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/upstream_version.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_reserved_mem.h>
#include <linux/mailbox_client.h>
#include <linux/amlogic/aml_mbox.h>
#include <linux/mailbox_controller.h>
#include "ring_buffer.h"

#define LOG_MEM_MAX_REGION_COUNT  2

struct log_operations_t {
	unsigned int (*read)(unsigned int *timer, unsigned char *buf,
			unsigned int size, unsigned int *state);
};

struct log_mem_t {
	unsigned long log_base;
	unsigned long log_size;
};

struct uboot_log_t {
	struct log_mem_t mem[LOG_MEM_MAX_REGION_COUNT];
	struct log_operations_t operations[LOG_MEM_MAX_REGION_COUNT];
	void __iomem  *timer_reg;
	void __iomem *bl30_log_base;
};

struct uboot_log_t *uboot_log;
struct mbox_chan *bl30log_mbox_chan;

static struct proc_dir_entry *proc_uboot_log;

#define LOG_MEM_INDEX_UBOOT   0
#define LOG_MEM_INDEX_BL30    1

#define DATA_BUF_MAX_SIZE   100
#define TIME_BUF_MAX_SIZE	16
#define TOTAL_BUF_MAX_SIZE  (DATA_BUF_MAX_SIZE + TIME_BUF_MAX_SIZE)

static unsigned int get_show_data(unsigned char *buf, int size)
{
	unsigned char r_buf[DATA_BUF_MAX_SIZE];
	unsigned int timer = 0;
	unsigned int status;
	unsigned int count;
	struct log_operations_t *op;
	int i;

	if (!uboot_log)
		return 0;

	if (!buf || size < TOTAL_BUF_MAX_SIZE)
		return 0;

	count = 0;
	memset(buf, 0, size);
	for (i = 0; i < LOG_MEM_MAX_REGION_COUNT; i++) {
		memset(r_buf, 0, sizeof(r_buf));
		timer = 0;
		status = 0;
		op = &uboot_log->operations[i];
		if (op->read) {
			count = op->read(&timer, r_buf, sizeof(r_buf), &status);
			if (count > 0) {
				switch (status & 3) {
				case 1:
				case 3:
					snprintf(buf, size, "[%10d]:%s", timer, r_buf);
					pr_notice("%s", buf);
				break;
				case 0:
				case 2:
					snprintf(buf, size, "%s", r_buf);
					pr_notice("%s", r_buf);
				break;
				}
				break; //exit for loop
			}
		}
	}
	return count;
}

static int uboot_log_show(struct seq_file *m, void *arg)
{
	unsigned char *p = (unsigned char *)arg;

	if (!p)
		return -EINVAL;

//	dump_stack();
	seq_printf(m, "%s", p);
	return 0;
}

static void *uboot_log_start(struct seq_file *m, loff_t *pos)
{
	unsigned int count;
	unsigned char *spos = kmalloc(TOTAL_BUF_MAX_SIZE, GFP_KERNEL);

	if (!spos)
		return NULL;

	count =  get_show_data(spos, TOTAL_BUF_MAX_SIZE);

	if (count == 0) {
		/*when count is 0, there is no data*/
		kfree(spos);
		return NULL;
	}
	return spos;
}

static void *uboot_log_next(struct seq_file *m, void *v, loff_t *pos)
{
	unsigned int count;
	unsigned char *spos = (unsigned char *)v;

	(*pos)++;
	count =  get_show_data(spos, TOTAL_BUF_MAX_SIZE);
	if (count == 0) {
		/*when count is 0, there is no data*/
		return NULL;
	}
	return spos;
}

static void uboot_log_stop(struct seq_file *m, void *v)
{
	kfree(v);
}

static const struct seq_operations uboot_log_seq_ops = {
	.start = uboot_log_start,
	.next  = uboot_log_next,
	.stop  = uboot_log_stop,
	.show  = uboot_log_show,
};

static int uboot_log_open(struct inode *inode, struct file *file)
{
	second_read_ring_buffer_set();
	return seq_open(file, &uboot_log_seq_ops);
}

#define BL30_RECEIVE_STATUS_START       0
#define BL30_RECEIVE_STATUS_FINISHED    1
static void send_refresh_cmd_to_bl30(void)
{
	unsigned long log_base, log_size;
	unsigned int bl30_recive_status = BL30_RECEIVE_STATUS_START;

	if (!uboot_log)
		return;

	if (!IS_ERR_OR_NULL(bl30log_mbox_chan)) {
		aml_mbox_transfer_data(bl30log_mbox_chan,
				      MBOX_CMD_REFRESH_BL30_LOG,
				      NULL, 0,
				      &bl30_recive_status, sizeof(bl30_recive_status),
				      MBOX_SYNC);
		if (bl30_recive_status == BL30_RECEIVE_STATUS_FINISHED) {
			log_base = uboot_log->mem[LOG_MEM_INDEX_BL30].log_base;
			log_size = uboot_log->mem[LOG_MEM_INDEX_BL30].log_size;
			bl30_log_update(log_base, log_size);
		}
	}
}

static ssize_t bl30_log_write(struct file *file, const char __user *buf,
	size_t count, loff_t *ppos)
{
	char cmd;

	if (copy_from_user(&cmd, buf, 1))
		return -EFAULT;

	if (cmd == '1') {
		send_refresh_cmd_to_bl30();
		return count;
	}

	return -EINVAL;
}

static const struct proc_ops uboot_log_ops = {
	.proc_open	= uboot_log_open,
	.proc_read	= seq_read,
	.proc_write	= bl30_log_write,
	.proc_lseek	= seq_lseek,
	.proc_release	= seq_release,
};

long get_raw_time_from_timer_e(void)
{
	long time;

	if (!uboot_log || !uboot_log->timer_reg) {
		pr_err("uboot log get time error\n");
		return 0;
	}
	time = readl(uboot_log->timer_reg);
	return time;
}

static void uboot_log_reserved_memory(struct platform_device *pdev, struct uboot_log_t *log)
{
	struct device_node *node;
	struct reserved_mem *rmem;
	int i;

	for (i = 0; i < LOG_MEM_MAX_REGION_COUNT; i++) {
		node = of_parse_phandle(pdev->dev.of_node, "memory-region", i);

		if (!node) {
			return;
		}
		rmem = of_reserved_mem_lookup(node);
		if (!rmem) {
			return;
		}
		if (!rmem->size) {
			return;
		}
		if (i == LOG_MEM_INDEX_BL30) {
			/*bl30 is sram memory*/
			log->bl30_log_base = ioremap(rmem->base, rmem->size);
			if (!log->bl30_log_base) {
				return;
			}
			log->mem[i].log_base = (unsigned long)log->bl30_log_base;
		} else {
			log->mem[i].log_base = __phys_to_virt(rmem->base);
		}
		log->mem[i].log_size = rmem->size;
		pr_debug("uboot log rmem->base:0x%lx,log->mem[%d].log_base:0x%lx,size: 0x%lx\n",
			 (long)rmem->base, i, log->mem[i].log_base, log->mem[i].log_size);
	}
}

static int uboot_log_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct resource res;
	int ret = 0;
	unsigned long start, size;

	if (of_property_read_bool(np, "mboxes"))
		bl30log_mbox_chan = aml_mbox_request_channel_byidx(&pdev->dev, 0);

	uboot_log = kmalloc(sizeof(*uboot_log), GFP_KERNEL);
	if (!uboot_log)
		return -ENOMEM;

	uboot_log_reserved_memory(pdev, uboot_log);

	ret = of_address_to_resource(np, 0, &res);

	if (ret) {
		pr_err("uboot log get timer E failed\n");
		kfree(uboot_log);
		return -EINVAL;
	}
	//pr_notice("uboot log timer E start:0x%lx, size: 0x%lx\n",
	//	 (long)res.start, (long)resource_size(&res));

	uboot_log->timer_reg = ioremap(res.start, resource_size(&res));
	if (!uboot_log->timer_reg) {
		kfree(uboot_log);
		return -EINVAL;
	}

	start = res.start;
	size  = res.end - res.start + 1;
	pr_debug("uboot log timer E timer_reg:0x%lx,start = %lx, size = %lx\n",
		 (long)uboot_log->timer_reg, start, size);

	blx_init_uboot_log(uboot_log->mem[0].log_base, uboot_log->mem[0].log_size);

	bl30_init_log(uboot_log->mem[1].log_base, uboot_log->mem[1].log_size);

	uboot_log->operations[0].read = read_ring_buffer;
	uboot_log->operations[1].read = bl30_read_ring_buffer;

	proc_uboot_log = proc_create("uboot_log", 0444, NULL, &uboot_log_ops);

	if (IS_ERR_OR_NULL(proc_uboot_log)) {
		iounmap(uboot_log->timer_reg);
		kfree(uboot_log);
		return -EINVAL;
	}

	return ret;
}

static const struct of_device_id uboot_log_dt_match[] = {
	{ .compatible = "amlogic, uboot_log" },
	{ /* sentinel */ }
};

static struct platform_driver uboot_log_platform_driver = {
	.probe		= uboot_log_probe,
	.driver		= {
		.owner		= THIS_MODULE,
		.name		= "uboot_log",
		.of_match_table	= uboot_log_dt_match,
	},
};

int __init meson_uboot_log_init(void)
{
	return  platform_driver_register(&uboot_log_platform_driver);
}

void __exit meson_uboot_log_exit(void)
{
	if (proc_uboot_log)
		proc_remove(proc_uboot_log);

	if (uboot_log && uboot_log->timer_reg) {
		iounmap(uboot_log->timer_reg);
		if (uboot_log->bl30_log_base)
			iounmap(uboot_log->bl30_log_base);
		kfree(uboot_log);
	}

	platform_driver_unregister(&uboot_log_platform_driver);
}
