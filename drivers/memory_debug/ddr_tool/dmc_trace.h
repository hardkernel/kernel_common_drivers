/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#if !defined(_TRACE_DMC_MONITOR_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_DMC_MONITOR_H

#include <linux/tracepoint.h>

#undef TRACE_SYSTEM
#define TRACE_SYSTEM dmc_monitor

#include <asm/memory.h>
#include <linux/page-flags.h>
struct page;
struct dmc_mon_comm;
unsigned long read_violation_mem(unsigned long addr, char rw);

TRACE_EVENT(dmc_violation,

	TP_PROTO(char *title, struct dmc_mon_comm *mon_comm, char *port, char *sub,
		 unsigned long pagetrace, unsigned long value),

	TP_ARGS(title, mon_comm, port, sub, pagetrace, value),

	TP_STRUCT__entry(
		__string(title, title)
		__field(unsigned long, addr)
		__field(unsigned long, value)
		__field(unsigned long, status)
		__string(port, port)
		__string(sub, sub)
		__field(char, rw)
		__field(unsigned long, page_trace)
		__field(unsigned int, order)
		__field(unsigned long, flags)
		__field(unsigned long long, time)
		__field(unsigned int, invalid)
		__field(unsigned int, same)
		__field(unsigned int, total)
	),

	TP_fast_assign(
		__assign_str(title);
		__entry->addr = mon_comm->addr;
		__entry->value = value;
		__entry->status = mon_comm->status;
		__assign_str(port);
		__assign_str(sub);
		__entry->rw = mon_comm->rw;
		__entry->page_trace = pagetrace;
		__entry->order = (&mon_comm->trace)->order;
		__entry->flags = mon_comm->page_flags;
		__entry->time = mon_comm->time;
		__entry->invalid = mon_comm->prot_buf.invalid;
		__entry->same = mon_comm->prot_buf.same;
		__entry->total = mon_comm->prot_buf.total;
	),

	TP_printk("addr=%09lx val=%016lx s=%08lx port=%s sub=%s f:%08lx lru:%d a:%ps(%d) t:%lld rw:%c%s(%d/%d/%d)",
		  __entry->addr,
		  __entry->value,
		  __entry->status,
		  __get_str(port),
		  __get_str(sub),
		  __entry->flags,
		  test_bit(PG_lru, &__entry->flags),
		  (void *)__entry->page_trace,
		  __entry->order,
		  __entry->time,
		  __entry->rw,
		  __get_str(title),
		  __entry->invalid,
		  __entry->same,
		  __entry->total)
);

#endif /*  _TRACE_DMC_MONITOR_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH ../../drivers/memory_debug/ddr_tool/
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE dmc_trace

/* This part must be outside protection */
#include <trace/define_trace.h>
