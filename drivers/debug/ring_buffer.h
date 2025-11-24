/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

/*
 * Ring Buffer implementation
 *
 *1)
 *head points to first available data byte
 *tail points to first available byte space
 *2)
 *if head==tail && len==0      // empty
 *if head==tail && len==size   // full
 *3)
 *if head>=size   head%=size
 *if tail>=size   tail%=size
 */
#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

//#include "soc.h"

#ifndef BIT
#define BIT(nr)                  (1 << (nr))
#endif

//#define BL30MSG_BUF_BASE (0x40000000 + 0x1000)
//#define BL30MSG_BUF_BASE (0x2e000000 + 0x1000)
//#define BL30MSG_BUF_BASE (0x10000000 + (64*1024))
//#define BL30MSG_BUF_BASE (0xfffc0000 + (24*1024))   /*(0xfffc0000 + (24*1024))=0xfffc6000*/
//#define BL30MSG_BUF_SIZE (2*1024)
//#define BL30MSG_LEN 100

//#define UBOOT_LOG_BUF_BASE   (0x8000000 - 0x100000)
//#define UBOOT_LOG_BUF_SIZE   0X10000
#define UBOOT_LOG_BUF_BASE   (BOOT_LOG_ADDR)
#define UBOOT_LOG_BUF_SIZE   (BOOT_LOG_SIZE)

#define MAGIC                0x11223344
#define UBOOT_LOG_INITFLAG   0x54494e49  /*INIT*/
#define UBOOT_LOG_VER        0X1

#ifndef NULL
#define NULL   ((void *)0)
#endif

#define RISC_V_CPU    0
#define ARM_CPU       1

/*
 *1) req is used for request lock
 *2) turn is flag for who should own the lock when they request at same time.
 */
struct amp_lock {
	int turn;
	int req[2];
};

#define MESSAGE_ALIGN_SIZE   4

struct ring_buffer {
	unsigned int magic;      // magic number
	unsigned int ver;
	unsigned int inited;
#define LOG_MESSAGE_WRITE   BIT(0)
#define LOG_MESSAGE_READ    BIT(1)
#define LOG_MESSAGE_SECOND_READ    BIT(2)
	unsigned int mode;
	struct amp_lock     lock;           // exclusive lock (implement future)
	unsigned int size;       // total size of ring buffer data
	unsigned int head;       // for read data offset, arm(kernel) maintain, kernel move it
	unsigned int tail;       // for write data offset, risc-v maintain it
	unsigned int len;        // available log data in ring buffer
	unsigned int len_b;      // available log data for second read
	/* kernel6.12 have add UBSAN check, when define data[4], only 4bytes can be accessed,
	 * exceed access 4bytes, there are below error:
	 * "Internal error: UBSAN: array index out of bounds: 00000000f2005512 [#1] PREEMPT SMP"
	 * so payload data don't be here. increase struct m_rb
	 */
	//char data[4];            // log buffer payload start
};

struct m_rb {
	struct ring_buffer *rb;
	char *data;
};

#define LOG_MESSAGE_HEADER          0x11121314
#define MESSAGE_MIN_SIZE            (13)

#define READ_MSG_STATE_IN_PROGRESS  0
#define READ_MSG_STATE_BEGIN        BIT(0)
#define READ_MSG_STATE_END          BIT(1)
/*log_message_t format is embedded in msg loop buffer */
struct log_message_t {
	unsigned int message_head; //for find out correct first message after message is loop save
	unsigned int timer;
	unsigned int len;       //message valid len
	unsigned char *message; //message 4byte align
};

//extern int lock(void);
//extern int unlock(void);
int initial_ring_buffer(unsigned char *buffer, unsigned int size);
unsigned int read_ring_buffer(unsigned int *timer, unsigned char *buffer,
		unsigned int size, unsigned int *msg_state);
unsigned int bl30_read_ring_buffer(unsigned int *timer, unsigned char *buffer,
		unsigned int size, unsigned int *msg_state);
void bl30_log_update(unsigned long base_addr, unsigned long size);
unsigned int write_ring_buffer(unsigned char *buffer, unsigned int size);
void reset_ring_buffer(void);
void blx_init_uboot_log(unsigned long base_addr, unsigned long size);
void bl30_init_log(unsigned long base_addr, unsigned long size);
void blx_put_char(unsigned char c);
void second_read_ring_buffer_set(void);
long get_raw_time_from_timer_e(void);
#endif
