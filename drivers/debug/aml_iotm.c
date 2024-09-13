// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/irqreturn.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/of_address.h>
#include <linux/of_reserved_mem.h>
#include <linux/sched/clock.h>
#include <linux/sysrq.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/amlogic/gki_module.h>
#include <trace/hooks/traps.h>
#include <linux/amlogic/aml_iotm.h>
#include <linux/arm-smccc.h>
#include <linux/panic_notifier.h>
#include <linux/timer.h>
#include <linux/syscore_ops.h>
#include <linux/hardirq.h>
#include "iotm_hw.h"

static int iotm_irq_to_exception;
static struct iotm iotm;
static int iotm_supported;

int get_iotm_monitor_mode(void)
{
	return iotm.monitor_mode;
}
EXPORT_SYMBOL(get_iotm_monitor_mode);

static int iotm_enabled;

int get_iotm_en_ddr_size(int *iotm_ddr_size)
{
	*iotm_ddr_size = iotm.buf_end - iotm.buf_start + 1;
	return iotm_supported;
}
EXPORT_SYMBOL(get_iotm_en_ddr_size);

static unsigned int iotm_disable;

static int iotm_disable_set(const char *val, const struct kernel_param *kp)
{
	unsigned int iotm_ctrl_mode_val;

	if (!iotm_supported) {
		pr_err("IOTM:not support in this board\n");
		return -EINVAL;
	}

	if (kstrtoint(val, 0, &iotm_disable)) {
		pr_err("IOTM:iotm_disable error: %s\n", val);
		return -EINVAL;
	}
	iotm_ctrl_mode_val = readl(iotm.cssys_base + IOTM_CTRL_MODE);

	if (iotm_disable == 1) {
		iotm_ctrl_mode_val |= IOTM_CTRL_MODE_TRACE_DISABLE;
		iotm_enabled = 0;
		pr_info("IOTM:has been disabled\n");
	} else if (iotm_disable == 0) {
		iotm_ctrl_mode_val |= IOTM_CTRL_MODE_TRACE_ENABLE;
		iotm_enabled = 1;
		pr_info("IOTM:has been enabled\n");
	}

	writel(iotm_ctrl_mode_val, iotm.cssys_base + IOTM_CTRL_MODE);

	return 0;
}

static const struct kernel_param_ops iotm_disable_ops = {
	.set = iotm_disable_set,
	.get = param_get_int,
};
module_param_cb(iotm_disable, &iotm_disable_ops, &iotm_disable, 0644);

static int iotm_disable_setup(char *buf)
{
	if (!buf)
		return -EINVAL;

	if (kstrtoint(buf, 0, &iotm_disable)) {
		pr_err("IOTM:iotm_disable error: %s\n", buf);
		return -EINVAL;
	}

	return 0;
}
__setup("iotm_disable=", iotm_disable_setup);

static int iotm_monitor_mode_setup(char *buf)
{
	if (!buf)
		return -EINVAL;

	if (kstrtoint(buf, 0, &iotm.monitor_mode) || iotm.monitor_mode > 2) {
		pr_err("IOTM:iotm_monitor_mode error: %s\n", buf);
		iotm.monitor_mode = 0;
		return -EINVAL;
	}

	return 0;
}
__setup("iotm.monitor_mode=", iotm_monitor_mode_setup);

static int iotm_monitor_range_setup(char *buf)
{
	int ret = 0, i = 0;

	if (!buf)
		return -EINVAL;

	ret = sscanf(buf, "%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",
			&iotm.range[0].start, &iotm.range[0].end,
			&iotm.range[1].start, &iotm.range[1].end,
			&iotm.range[2].start, &iotm.range[2].end,
			&iotm.range[3].start, &iotm.range[3].end,
			&iotm.range[4].start, &iotm.range[4].end);
	if (!ret || ret % 2) {
		pr_err("IOTM:error, iotm.monitor_range=start,end");
		goto err;
	}

	while (iotm.range[i].start && i < 5) {
		if (iotm.range[i].start >= iotm.range[i].end ||
			iotm.range[i].start < IOTM_MONITOR_RANGE_MIN ||
			iotm.range[i].end > IOTM_MONITOR_RANGE_MAX) {
			pr_err("IOTM:error,0x%x <= start < end <= 0x%x",
					IOTM_MONITOR_RANGE_MIN, IOTM_MONITOR_RANGE_MAX);
			goto err;
		} else {
			i++;
		}
	}

	pr_info("iotm.range=[%x,%x],[%x,%x],[%x,%x],[%x,%x],[%x,%x]\n",
			iotm.range[0].start, iotm.range[0].end,
			iotm.range[1].start, iotm.range[1].end,
			iotm.range[2].start, iotm.range[2].end,
			iotm.range[3].start, iotm.range[3].end,
			iotm.range[4].start, iotm.range[4].end);

	return 0;
err:
	for (i = 0; i < 5; i++) {
		iotm.range[i].start = 0;
		iotm.range[i].end = 0;
	}
	return -EINVAL;
}
__setup("iotm.monitor_range=", iotm_monitor_range_setup);

static DEFINE_SPINLOCK(iotm_record_lock);
static void iotm_record_write(struct iotm_record *record)
{
	unsigned long flags;

	spin_lock_irqsave(&iotm_record_lock, flags);
	writel(record->stream2, iotm.cssys_base + SW_DATA_STREAM0);
	writel(record->stream1 | SW_DATA_STREAM1_WRITE, iotm.cssys_base + SW_DATA_STREAM1);
	spin_unlock_irqrestore(&iotm_record_lock, flags);
}

void iotm_sw_record_write(u32 sw_type, u32 val1, u32 val2)
{
	struct iotm_record record;

	if (!iotm_enabled)
		return;

	record.sw_stream1.sw_type = sw_type;
	record.sw_stream1.cpu = smp_processor_id();

	switch (sw_type) {
	case IOTM_SW_IRQ_IN:
	case IOTM_SW_IRQ_OUT:
		record.irq_stream2.irq = val1;
		break;
	case IOTM_SW_SMC_IN:
	case IOTM_SW_SMC_OUT:
	case IOTM_SW_SMC_NORET_IN:
		record.smc_stream2.smcid = val1;
		record.sw_stream1.reserved = val2;
		break;
	case IOTM_SW_SCHED:
		record.sched_stream2.curr_pid = val1;
		record.sched_stream2.next_pid = val2;
		break;
	default:
		break;
	}

	iotm_record_write(&record);
}
EXPORT_SYMBOL(iotm_sw_record_write);

static void print_iotm_trace(void *ptr)
{
	struct iotm_record *record = ptr;

	if (record->sw_stream1.mid == SW_TRACE) {
		/* The source of the data is software */
		switch (record->sw_stream1.sw_type) {
		case IOTM_SW_IRQ_IN:
		case IOTM_SW_IRQ_OUT:
			pr_err("IOTM:ISR %3s [irq:%x] cpu:%x IDX:%u TS:%u\n",
					record->sw_stream1.sw_type == IOTM_SW_IRQ_IN ? "in" : "out",
					record->irq_stream2.irq,
					record->sw_stream1.cpu,
					record->stream0.idx,
					record->stream0.ts);
			break;
		case IOTM_SW_SMC_IN:
		case IOTM_SW_SMC_OUT:
		case IOTM_SW_SMC_NORET_IN:
			pr_err("IOTM:SMC %s cpu:%x [id:%x val:%x] IDX:%u TS:%u\n",
					record->sw_stream1.sw_type == IOTM_SW_SMC_OUT ? "out" :
					(record->sw_stream1.sw_type == IOTM_SW_SMC_IN ? "in" :
					"noret_in"),
					record->sw_stream1.cpu,
					record->smc_stream2.smcid,
					record->sw_stream1.reserved,
					record->stream0.idx,
					record->stream0.ts);
			break;
		case IOTM_SW_SCHED:
			pr_err("IOTM:SCHED_WITCH [prev:%x next:%x] cpu:%d IDX:%u TS:%u\n",
					record->sched_stream2.curr_pid,
					record->sched_stream2.next_pid,
					record->sw_stream1.cpu,
					record->stream0.idx,
					record->stream0.ts);
			break;
		default:
			break;
		}
	} else {
		/* The source of the data is hardware */
		pr_err("IOTM:%s[%08x-%08x] %s %s IDX:%u TS:%u\n",
				record->io_stream1.mid == AOCPU_TRACE ? "AOCPU" : "",
				record->io_stream1.addr + ADDR_OFFSET,
				record->stream2,
				record->stream0.rw ? "wr" : "rd",
				record->io_stream1.fail ? "fail" : "",
				record->stream0.idx,
				record->stream0.ts);
	}
}

/* Ddr buffer can't be confirmed to have been overwritten.
 * So the data has been overwritten defaultly when data is dumped.
 * ddr size needs to be set to a multiple of 48 to confirm corrected.
 */
static void auto_dump_ddr_buf(void)
{
	unsigned int iotm_ddr_size = iotm.buf_end - iotm.buf_start + 1;
	unsigned int iotm_axi_addr_val = readl(iotm.cssys_base + IOTM_AXI_ADDR);
	unsigned int last_seg_size = iotm_axi_addr_val - iotm.buf_start;
	void *ptr, *ptr_end;
	unsigned int bytes_per_iotm_record = sizeof(struct iotm_record);
	unsigned int offset = (last_seg_size % bytes_per_iotm_record);
	void *vaddr;

	if (iotm_axi_addr_val < iotm.buf_start || iotm_axi_addr_val > iotm.buf_end) {
		pr_err("IOTM:error!IOTM_AXI_ADDR=0x%x, ddr_buf=[0x%x, 0x%x]\n",
				iotm_axi_addr_val, iotm.buf_start, iotm.buf_end);
		return;
	}

	vaddr = phys_to_virt(iotm.buf_start);
	if (!vaddr) {
		pr_err("IOTM: NOMEM\n");
		return;
	}

	// modify start addr
	if (offset)
		last_seg_size += (bytes_per_iotm_record - offset);

	ptr = vaddr + last_seg_size;
	ptr_end = ptr + iotm_ddr_size - last_seg_size;

	while (ptr < ptr_end) {
		print_iotm_trace(ptr);
		ptr += bytes_per_iotm_record;
	}

	ptr = vaddr;
	ptr_end = ptr + last_seg_size;

	while (ptr < ptr_end) {
		print_iotm_trace(ptr);
		ptr += bytes_per_iotm_record;
	}
}

void auto_dump_etb_buf(void)
{
	u32 write_ptr, read_data;
	u32 depth;
	u32 iotm_trace[3];
	int size;
	int i;

	// stop etb trace
	writel(0x0, iotm.cssys_base + ETB_CTL);

	depth = readl(iotm.cssys_base + ETB_DEPTH);

	/* -4: T6D chip designs that RWP will take 4 more steps. */
	if (!(readl(iotm.cssys_base + ETB_STATUS) & 0x1)) {
		/* etb buffer is not full, need to read from 0 */
		size = readl(iotm.cssys_base + ETB_RWP) - 4;
		writel(0x0, iotm.cssys_base + ETB_RRP);
	} else {
		/*
		 * etb buffer is full. need to read from ETB_RWP.
		 * (depth % 3): Loop buffer has been overwritten,
		 * and the oldest data needs to be found.
		 * +3: the lastest data is [1,0,0] and useless.
		 */
		write_ptr = readl(iotm.cssys_base + ETB_RWP) - 4 + (depth % 3) + 3;
		size = depth;
		writel(write_ptr, iotm.cssys_base + ETB_RRP);
	}

	/* 8K */
	for (i = 0; i < size; i++) {
		read_data = readl(iotm.cssys_base + ETB_RRD);
		switch (i % 3) {
		case 0:
			iotm_trace[0] = read_data;
			break;
		case 1:
			iotm_trace[1] = read_data;
			break;
		case 2:
			iotm_trace[2] = read_data;
			print_iotm_trace(iotm_trace);
			break;
		default:
			break;
		}
	}
}

static void dump_iotm_trace(void)
{
	int i;

	pr_err("IOTM:dump trace start==========\n");

	/* dump iotm register and trace data*/
	for (i = 0; i < 0x22; i++)
		pr_err("[%x]=0x%x\n", iotm.cssys_base_phy + ADDR_RANGE0_BEGIN + i * 4,
				readl(iotm.cssys_base + ADDR_RANGE0_BEGIN + i * 4));

	if (iotm.monitor_mode == AXI_MODE)
		auto_dump_ddr_buf();
	else if (iotm.monitor_mode == ETB_MODE)
		auto_dump_etb_buf();

	pr_err("IOTM:dump trace end==========\n");
}

static void print_timeout_data(void)
{
	unsigned int val;

	val = readl(iotm.cssys_base + IOTM_IRQ_CTRL);

	/* capu timeout */
	if (val & IOTM_IRQ_CTRL_CAPU_TIMEOUT) {
		pr_err("IOTM:capu timeout addr:%x data:%x monitor_status:%x\n",
			readl(iotm.cssys_base + CAPU_MONITOR_ADDR),
			readl(iotm.cssys_base + CAPU_MONITOR_DATA),
			readl(iotm.cssys_base + MONITOR_STATUS));
	}

	/* vapb4 timeout */
	if (val & IOTM_IRQ_CTRL_VAPB4_TIMEOUT) {
		pr_err("IOTM:vapb timeout addr:%x data:%x monitor_status:%x\n",
			readl(iotm.cssys_base + VAPB4_MONITOR_ADDR),
			readl(iotm.cssys_base + VAPB4_MONITOR_DATA),
			readl(iotm.cssys_base + MONITOR_STATUS));
	}
}

static int iotm_exception_handler(struct notifier_block *nb,
			unsigned long action, void *data)
{
	unsigned int val;

	val = readl(iotm.cssys_base + IOTM_IRQ_CTRL);

	/* panic has nothing with IOTM */
	if (iotm_irq_to_exception == 0 && !(val & (IOTM_IRQ_CTRL_CAPU_TIMEOUT |
		IOTM_IRQ_CTRL_VAPB4_TIMEOUT)))
		return 0;

	print_timeout_data();

	/*
	 * arm64 calls nmi_enter() before do_serror(),
	 * it will cause printk entered deferred mode
	 * and not outputs to console immediately.
	 */
	if (in_nmi())
		__preempt_count_sub(NMI_OFFSET + HARDIRQ_OFFSET);

	dump_iotm_trace();

	return 0;
}

static struct notifier_block iotm_panic_notifier = {
	.notifier_call  = iotm_exception_handler,
};

static void coresight_init(void)
{
	writel(ETB_LOCK_ACCESS_ALL, iotm.cssys_base + FUNNUL_LOCKACCESS);
	writel(FUNNEL_CTRL_REG_VAL, iotm.cssys_base + FUNNEL_CTRL_REG);
	writel(ETB_LOCK_ACCESS_ALL, iotm.cssys_base + REPLICAPTER_LOCKACCESS);
	writel(ETB_LOCK_ACCESS_ALL, iotm.cssys_base + TPIU_LOCKACCESS);
	writel(ETB_LOCK_ACCESS_ALL, iotm.cssys_base + ETB_LOCKACCESS);
	writel(TPIU_ITCTRL_DISABLE, iotm.cssys_base + TPIU_ITCTRL);
	writel(TPIU_ITATBCTR2_ATREADY, iotm.cssys_base + TPIU_ITATBCTR2);
	writel(ETB_FFCR_DISABLE_FORMATTING, iotm.cssys_base + ETB_FFCR);
	writel(ETB_CTL_ENABLE_CAPTURE, iotm.cssys_base + ETB_CTL);
}

static void watchdog_dump_iotm_trace(void)
{
	unsigned int val, iotm_irq_ctrl;

	// if watchdog reset, irq pull up, dump ddr buf
	iotm_irq_ctrl = readl(iotm.cssys_base + IOTM_IRQ_CTRL);

	if (iotm_irq_ctrl & (IOTM_IRQ_CTRL_WATCHDOG_IRQ | IOTM_IRQ_CTRL_SECURITY_WATCHDOG_IRQ)) {
		// stop iotm, munual flush
		val = readl(iotm.cssys_base + IOTM_CTRL_MODE);
		writel(val | IOTM_CTRL_MODE_TRACE_DISABLE, iotm.cssys_base + IOTM_CTRL_MODE);

		/* watchdog reset need clear MONITOR_RANGE_INCLUDE_RST_MASK */
		val = readl(iotm.cssys_base + MONITOR_RANGE_INCLUDE);
		val &= ~MONITOR_RANGE_INCLUDE_RST_MASK;
		writel(val, iotm.cssys_base + MONITOR_RANGE_INCLUDE);

		/* init coresight to dump trace */
		if (iotm.monitor_mode == ETB_MODE)
			coresight_init();

		if (iotm_irq_ctrl & IOTM_IRQ_CTRL_WATCHDOG_IRQ)
			dump_iotm_trace();
	}
}

static void iotm_coresight_init(void)
{
	unsigned int val, i;
	struct arm_smccc_res smccc;

	/* stop iotm. */
	val = readl(iotm.cssys_base + IOTM_CTRL_MODE);
	val |= IOTM_CTRL_MODE_TRACE_DISABLE;
	writel(val, iotm.cssys_base + IOTM_CTRL_MODE);

	/* init iotm and reset coresight */
	__arm_smccc_smc(AML_IOTM_SMC_CMD, AML_IOTM_INIT_SMC_ARG, 0, 0, 0, 0, 0, 0, &smccc, NULL);

	/* exclude a53_dbg CSSYS_BASE_ADDR + iotm & coresight registers */
	iotm.range[6].start = iotm.cssys_base_phy;
	iotm.range[6].end = iotm.range[6].start + iotm.cssys_size;

	/* exclude uart register */
	iotm.range[5].start = UART_REG_START;
	iotm.range[5].end = UART_REG_END;

	/* do exclude registers */
	for (i = 0; i < MAX_EXCLUDE_RANGE; i++) {
		if (iotm.range[i].start != 0)
			__arm_smccc_smc(AML_IOTM_SMC_CMD, AML_IOTM_RANGE_SMC_ARG, i,
				iotm.range[i].start, iotm.range[i].end, 0, 0, 0, &smccc, NULL);
	}

	if (iotm.monitor_mode == AXI_MODE) {
		/* Allocate DDR reserved space to MONITOR_BUF_SIZE_LOW. */
		writel(iotm.buf_start,
			iotm.cssys_base + MONITOR_BUF_BASEADDR_LOW);

		writel((iotm.buf_end - iotm.buf_start + 1) / 16,
			iotm.cssys_base + MONITOR_BUF_SIZE_LOW);
	} else if (iotm.monitor_mode == ETB_MODE) {
		/* init coresight and set iotm_mode*/
		coresight_init();

		val = readl(iotm.cssys_base + IOTM_CTRL_MODE);
		val |= ETB_MODE;
		writel(val, iotm.cssys_base + IOTM_CTRL_MODE);
	}

	/* enable iotm */
	val = readl(iotm.cssys_base + IOTM_CTRL_MODE);

	/* ts1_accurate_sel=5: TS1 = (soc timestamp)/ (2^ts1_accurate_sel) */
	val &= ~IOTM_CTRL_MODE_ACCURATE;
	val |= IOTM_CTRL_MODE_ACCURATE_32;
	/* crtl_iotm_ts=4: TS1:[31:5],TS0: [4:1] */
	val &= ~IOTM_CTRL_MODE_TS;
	val |= IOTM_CTRL_MODE_TS_27;

	/* monitor vapb4 and capu bus then start trace data */
	val |= (IOTM_CTRL_MODE_CAPU_ENABLE | IOTM_CTRL_MODE_VAPB4_ENABLE |
			IOTM_CTRL_MODE_TRACE_ENABLE);
	writel(val, iotm.cssys_base + IOTM_CTRL_MODE);
}

static irqreturn_t iotm_irq_handler(int irq, void *data)
{
	unsigned int val;

	iotm_enabled = 0;
	val = readl(iotm.cssys_base + IOTM_IRQ_CTRL);

	if (val & (IOTM_IRQ_CTRL_CAPU_TIMEOUT | IOTM_IRQ_CTRL_VAPB4_TIMEOUT)) {
		pr_err("IOTM:timeout,IOTM_IRQ_CTRL = 0x%x\n", val);
		print_timeout_data();
		iotm_irq_to_exception = 1;
		mb(); /* make sure we only clear irq after iotm_irq_to_exception is set */
		writel(val | IOTM_IRQ_CTRL_VAPB4_TIMEOUT_CLEAR | IOTM_IRQ_CTRL_CAPU_TIMEOUT_CLEAR,
				iotm.cssys_base + IOTM_IRQ_CTRL);
	}

	if (val & (IOTM_IRQ_CTRL_PACK_IRQ | IOTM_IRQ_CTRL_VAPB4_FULL | IOTM_IRQ_CTRL_CAPU_FULL)) {
		pr_err("IOTM:full irq! IOTM_IRQ_CTRL = 0x%x\n", val);
		writel(val | IOTM_IRQ_CTRL_PACK_IRQ_CLEAR | IOTM_IRQ_CTRL_VAPB4_FULL_CLEAR |
				IOTM_IRQ_CTRL_CAPU_FULL_CLEAR, iotm.cssys_base + IOTM_IRQ_CTRL);

		/* restart trace data */
		val = readl(iotm.cssys_base + IOTM_CTRL_MODE);
		val |= (IOTM_CTRL_MODE_CAPU_ENABLE | IOTM_CTRL_MODE_VAPB4_ENABLE |
			IOTM_CTRL_MODE_TRACE_ENABLE);
		writel(val, iotm.cssys_base + IOTM_CTRL_MODE);

		iotm_enabled = 1;
	}

	return IRQ_HANDLED;
}

static int get_iotm_ddr_range(struct platform_device *pdev)
{
	struct device_node *node;
	struct reserved_mem *rmem;
	unsigned int iotm_ddr_size;
	int ret = 0;

	node = of_parse_phandle(pdev->dev.of_node, "memory-region", 0);
	if (!node) {
		pr_err("IOTM:Failed to find memory-region node\n");
		return -ENODEV;
	}

	rmem = of_reserved_mem_lookup(node);
	if (!rmem) {
		dev_warn(&pdev->dev, "no such reserved mem of node name %s\n",
				pdev->dev.of_node->name);
		return -ENODEV;
	}

	ret = of_property_read_u32(node, "iotm-size", &iotm_ddr_size);
	if (ret) {
		pr_err("IOTM:Failed to read iotm-size property\n");
		return ret;
	}

	iotm.buf_start = (rmem->base + rmem->size - iotm_ddr_size + 1) & PAGE_MASK;
	iotm.buf_end = iotm.buf_start + iotm_ddr_size - 1;

	return 0;
}

static int iotm_syscore_suspend(void)
{
	unsigned int iotm_ctrl_mode_val;

	iotm_enabled = 0;
	iotm_ctrl_mode_val = readl(iotm.cssys_base + IOTM_CTRL_MODE);
	iotm_ctrl_mode_val |= IOTM_CTRL_MODE_TRACE_DISABLE;
	writel(iotm_ctrl_mode_val, iotm.cssys_base + IOTM_CTRL_MODE);

	return 0;
}

static void iotm_syscore_resume(void)
{
	unsigned int iotm_ctrl_mode_val;

	iotm_ctrl_mode_val = readl(iotm.cssys_base + IOTM_CTRL_MODE);
	iotm_ctrl_mode_val |= IOTM_CTRL_MODE_TRACE_ENABLE;
	writel(iotm_ctrl_mode_val, iotm.cssys_base + IOTM_CTRL_MODE);
	iotm_enabled = 1;
}

static void iotm_syscore_shutdown(void)
{
	iotm_syscore_suspend();
}

static struct syscore_ops iotm_syscore_ops = {
	.suspend = iotm_syscore_suspend,
	.resume = iotm_syscore_resume,
	.shutdown = iotm_syscore_shutdown,
};

static int iotm_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret;
	int irq;

	// Checklist: iotm register range, irq;map iotm register;config irq handler
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		pr_err("IOTM:no registers defined\n");
		return -EINVAL;
	}

	iotm.cssys_base_phy = res->start;
	iotm.cssys_size = resource_size(res);
	iotm.cssys_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(iotm.cssys_base)) {
		pr_err("IOTM:fail to ioremap iotm register\n");
		return PTR_ERR(iotm.cssys_base);
	}

	if (iotm.monitor_mode == AXI_MODE) {
		ret = get_iotm_ddr_range(pdev);
		if (ret)
			return ret;
	}

	watchdog_dump_iotm_trace();

	spin_lock_init(&iotm_record_lock);

	iotm_coresight_init();

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		pr_err("IOTM:fail to get iotm irq\n");
		return irq;
	}

	ret = devm_request_irq(&pdev->dev, irq, iotm_irq_handler, 0, "iotm", NULL);
	if (ret) {
		pr_err("IOTM:fail to request irq: %d\n", ret);
		return ret;
	}

	/* print iotm trace */
	atomic_notifier_chain_register(&panic_notifier_list, &iotm_panic_notifier);

	register_syscore_ops(&iotm_syscore_ops);
	iotm_enabled = 1;
	iotm_supported = 1;

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id iotm_match[] = {
	{
		.compatible = "amlogic, iotm",
	},
	{}
};
#endif

static struct platform_driver iotm_driver = {
	.driver = {
		.name  = "iotm",
		.owner = THIS_MODULE,
	#ifdef CONFIG_OF
		.of_match_table = iotm_match,
	#endif
	},
	.probe = iotm_probe,
};

int iotm_init(void)
{
	int ret = 0;

	if (!iotm_disable)
		ret = platform_driver_register(&iotm_driver);

	return ret;
}
