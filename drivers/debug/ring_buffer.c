// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

//#include <platform_def.h>
//#include "register.h"
#include <linux/printk.h>
#include <linux/string.h>
#include "ring_buffer.h"

#define LOCK 1

static int blx_ready;
struct m_rb uboot_m_rb;
struct m_rb bl30_m_rb;

#define BL30_LOG_BUF_SIZE   (1024)
static unsigned char bl30_log_buf[BL30_LOG_BUF_SIZE];

/*log message timer*/
#define LOG_MSG_TIMER	SYSCTRL_TIMERE

static void Lock_EnterCritical(void)
{
	//taskENTER_CRITICAL();
}

static void Lock_ExitCritical(void)
{
	//taskEXIT_CRITICAL();
}

static int amp_lock_init(struct amp_lock *lock)
{
	//memset(lock, 0, sizeof(struct amp_lock));
	lock->turn = 0;
	lock->req[0] = 0;
	lock->req[1] = 0;
	return 0;
}

int amp_lock_obtain(struct amp_lock *lock, int self)
{
#if LOCK
	int other = 1 - self;

    // disable interrupt here
	Lock_EnterCritical();

	lock->req[self] = 1;
	if (lock->req[other]) {
		lock->req[self] = 0;
		while (lock->turn != self)
			;
		lock->req[self] = 1;
		while (lock->req[other])
			;
	}
#endif
	return 0;
}

int amp_lock_release(struct amp_lock *lock, int self)
{
#if LOCK
	int other = 1 - self;

	lock->turn = other;
	lock->req[self] = 0;

    // enable interrupt here
	Lock_ExitCritical();
#endif
	return 0;
}

//int lock(void)
//{
//	return 0;
//}

//int unlock(void)
//{
//	return 0;
//}

static struct ring_buffer *init_ring_buffer(unsigned char *buffer, unsigned int size)
{
	struct ring_buffer *b = (struct ring_buffer *)buffer;

	if (!b || size < sizeof(struct ring_buffer))
		return NULL;
	if (b->magic == MAGIC)
		return b;

	b->magic = MAGIC;
	b->inited = UBOOT_LOG_INITFLAG;
	b->ver = UBOOT_LOG_VER;
	/* size is size of payload,
	 * buffer total size = sizeof(struct ring_buffer) + size of payload
	 */
	b->size = size - sizeof(struct ring_buffer);
	b->head = 0;
	b->tail = 0;
	b->len  = 0;
	b->len_b  = 0;
	b->mode = 0;
	amp_lock_init(&b->lock);
	return b;
}

int check_and_get_ring_buffer(unsigned char *buffer, unsigned int size)
{
	struct ring_buffer *b = (struct ring_buffer *)buffer;

	if (!b || size < sizeof(struct ring_buffer))
		return -1;
	if (b->magic == MAGIC) {
		//only check MAGIC
		return 0;
	}
	return -1;
}

/*
 * buffer: entire loop buffer start address
 * size  : loop buffer size
 * return: 0: successful, -1: fail
 */
int initial_ring_buffer(unsigned char *buffer, unsigned int size)
{
	struct ring_buffer *rb;

	rb = init_ring_buffer(buffer, size);

	if (rb) {
		uboot_m_rb.rb = rb;
		uboot_m_rb.data = (char *)rb + sizeof(struct ring_buffer);
		//pr_notice("%s:%d uboot_m_rb.data=0x%lx\n",
		//	__func__, __LINE__, (long)(uboot_m_rb.data));
		return 0;
	}
	return -1;
}

/* __read_ring_buffer
 * timer: get time from loop buffer
 * msg_state: 0: read a message in progress,
 *            bit0 is 1: read a message begin
 *            bit1 is 1: read a message end
 */
static unsigned int __read_ring_buffer(struct m_rb *mrb, unsigned int *timer,
		unsigned char *buffer, unsigned int size, unsigned int *msg_state)
{
	unsigned int count, rlen;
	unsigned int total_len;
	unsigned int msg_header;
	unsigned char *p;
	static unsigned int *r_message_len;
	static unsigned int r_message_len_b;

	if (!mrb || !mrb->rb || mrb->rb->magic != MAGIC || mrb->rb->len == 0)
		return 0;
	if (!buffer || size == 0 || !timer || !msg_state)
		return 0;

	*msg_state = READ_MSG_STATE_IN_PROGRESS;
	count = 0;
	amp_lock_obtain(&mrb->rb->lock, ARM_CPU);
	/*rb->mode LOG_MESSAGE_READ bit is 0 to read header*/
	if ((mrb->rb->mode & LOG_MESSAGE_READ) != LOG_MESSAGE_READ) {
		total_len = mrb->rb->len;
		if (total_len >= MESSAGE_MIN_SIZE) {
			/* a message format:
			 * LOG_MESSAGE_HEADER(4bytes) + time(4bytes) + message len(4bytes) + message
			 * note: message end with '\n'
			 */
			/*find out a message header*/
			msg_header = LOG_MESSAGE_HEADER;
			p = (unsigned char *)&msg_header;
			do {
				if (p[0] != mrb->data[mrb->rb->head] ||
					p[1] != mrb->data[mrb->rb->head + 1] ||
					p[2] != mrb->data[mrb->rb->head + 2] ||
					p[3] != mrb->data[mrb->rb->head + 3]) {
					mrb->rb->head++;
					mrb->rb->head %= mrb->rb->size;
					mrb->rb->len--;
					if (mrb->rb->len == 0)
						goto err;
				} else {
					mrb->rb->head += 4;
					mrb->rb->head %= mrb->rb->size;
					mrb->rb->len -= 4;
					break;
				}
			} while (1);

			/*get time of a message*/
			p = (unsigned char *)timer;
			rlen = 4;
			mrb->rb->len -= rlen;
			while (rlen--) {
				p[count++] = mrb->data[mrb->rb->head++];
				mrb->rb->head %= mrb->rb->size;
			}

			/*get a message len */
			r_message_len = (unsigned int *)&mrb->data[mrb->rb->head];
			mrb->rb->head += 4;
			mrb->rb->head %= mrb->rb->size;
			mrb->rb->len -= 4;
			r_message_len_b = *r_message_len;
			/*start to get a message */
			mrb->rb->mode |= LOG_MESSAGE_READ;
			*msg_state |= READ_MSG_STATE_BEGIN;
		} else if (total_len == 0) {
			goto err;
		} // else {
			/*it should never be here, if come here, there is an error*/
		//}
	}
	if (!r_message_len) {
		/*it should never be here, if come here, there is an error*/
		goto err;
	}
	/*read a message*/
	count = 0;
	rlen = size < mrb->rb->len ? size : mrb->rb->len;
	rlen = r_message_len_b < rlen ? r_message_len_b : rlen;
	mrb->rb->len -= rlen;
	r_message_len_b -= rlen;
	while (rlen--) {
		buffer[count++] = mrb->data[mrb->rb->head++];
		mrb->rb->head %= mrb->rb->size;
	}
	/*read message finished*/
	if (r_message_len_b == 0) {
		unsigned int save_head;

		save_head = mrb->rb->head;
		mrb->rb->head += (MESSAGE_ALIGN_SIZE - 1);
		mrb->rb->head = (mrb->rb->head >> 2) << 2;
		save_head = mrb->rb->head - save_head;
		mrb->rb->head %= mrb->rb->size;
		mrb->rb->len -= save_head;
		mrb->rb->mode &= ~(LOG_MESSAGE_READ);
		r_message_len = NULL;
		*msg_state |= READ_MSG_STATE_END; //one message read finished
	}
	if (mrb->rb->len == 0) {
		//if read finished, clean LOG_MESSAGE_SECOND_READ flag
		mrb->rb->mode &= ~(LOG_MESSAGE_SECOND_READ);
	}
	amp_lock_release(&mrb->rb->lock, ARM_CPU);
	return count;
err:
	amp_lock_release(&mrb->rb->lock, ARM_CPU);
	return 0;
}

unsigned int read_ring_buffer(unsigned int *timer, unsigned char *buffer,
		unsigned int size, unsigned int *msg_state)
{
	return __read_ring_buffer(&uboot_m_rb, timer, buffer, size, msg_state);
}

unsigned int bl30_read_ring_buffer(unsigned int *timer, unsigned char *buffer,
		unsigned int size, unsigned int *msg_state)
{
	if (!bl30_m_rb.rb) {
		pr_notice("Please run \"echo 1 > /proc/uboot_log\" to get bl30 log\n");
		return 0;
	}
	return __read_ring_buffer(&bl30_m_rb, timer, buffer, size, msg_state);
}

static void __second_read_ring_buffer_set(struct ring_buffer *rb)
{
	if (!rb)
		return;

	amp_lock_obtain(&rb->lock, ARM_CPU);

	if (rb->len == 0 && ((rb->mode & LOG_MESSAGE_WRITE) != LOG_MESSAGE_WRITE)) {
		rb->len = rb->len_b;
		rb->head = 0;
		rb->mode |= (LOG_MESSAGE_SECOND_READ);
	}

	amp_lock_release(&rb->lock, ARM_CPU);
}

void second_read_ring_buffer_set(void)
{
	__second_read_ring_buffer_set(uboot_m_rb.rb);
	//__second_read_ring_buffer_set(bl30_m_rb.rb);
}

static unsigned int __write_ring_buffer(struct m_rb *mrb,
		unsigned char *buffer, unsigned int size)
{
	unsigned int count, wlen;
	unsigned int timer;
	unsigned int msg_header;
	unsigned char *p;
	static unsigned int *message_len;

	if (!mrb || !mrb->rb || mrb->rb->magic != MAGIC)
		return 0;
	if (size == 0)
		return 0;

	count = 0;
	amp_lock_obtain(&mrb->rb->lock, RISC_V_CPU);
	/*rb->mode LOG_MESSAGE_WRITE bit is 0 to write header*/
	if ((mrb->rb->mode & LOG_MESSAGE_WRITE) != LOG_MESSAGE_WRITE) {
		/* a message format:
		 * LOG_MESSAGE_HEADER(4bytes) + time(4bytes) + message len(4bytes) + message
		 * note: message end with '\n'
		 */
		/*write message header to loop buffer, it indicate a message start*/
		msg_header = LOG_MESSAGE_HEADER;
		p = (unsigned char *)&msg_header;
		wlen = 4;
		mrb->rb->len += wlen;
		mrb->rb->len_b += wlen;
		while (wlen--) {
			mrb->data[mrb->rb->tail++] = p[count++];
			mrb->rb->tail %= mrb->rb->size;
		}

		/*write time of a message */
		//timer = *(volatile unsigned int *)(unsigned long)LOG_MSG_TIMER;
		timer = get_raw_time_from_timer_e();
		p = (unsigned char *)&timer;
		wlen = 4;
		mrb->rb->len += wlen;
		mrb->rb->len_b += wlen;
		count = 0;
		while (wlen--) {
			mrb->data[mrb->rb->tail++] = p[count++];
			mrb->rb->tail %= mrb->rb->size;
		}

		/*reserver message len space*/
		message_len = (unsigned int *)&mrb->data[mrb->rb->tail];
		*message_len = 0;
		mrb->rb->tail += 4;
		mrb->rb->tail %= mrb->rb->size;
		mrb->rb->len += 4;
		mrb->rb->len_b += 4;
		/*write a message until message ends with '\n' in buffer,
		 *the middle '\n' in the buffer does not act as an end marker
		 */
		mrb->rb->mode |= LOG_MESSAGE_WRITE;
	}
	if (!message_len) {
		/*it should never be here, if come here, it have an error*/
		goto err;
	}
	wlen = size;
	mrb->rb->len += wlen;
	mrb->rb->len_b += wlen;
	(*message_len) += wlen;
	count = 0;
	while (wlen--) {
		mrb->data[mrb->rb->tail++] = buffer[count++];
		mrb->rb->tail %= mrb->rb->size;
	}
	/*'\n' is message end, message size MESSAGE_ALIGN_SIZE size align */
	if (buffer[size - 1] == '\n') {
		unsigned int save_tail;

		save_tail = mrb->rb->tail;
		mrb->rb->tail += (MESSAGE_ALIGN_SIZE - 1);
		mrb->rb->tail = (mrb->rb->tail >> 2) << 2;
		save_tail = mrb->rb->tail - save_tail;
		mrb->rb->tail %= mrb->rb->size;
		mrb->rb->len += save_tail;
		mrb->rb->len_b += save_tail;
		mrb->rb->mode &= ~(LOG_MESSAGE_WRITE);
		message_len = NULL;
	}

	/* New data will overwrite the old data */
	if (mrb->rb->len > mrb->rb->size) {
		mrb->rb->head = mrb->rb->tail;
		mrb->rb->len = mrb->rb->size;
		mrb->rb->len_b = mrb->rb->size;
	}
	amp_lock_release(&mrb->rb->lock, RISC_V_CPU);
	return count;
err:
	amp_lock_release(&mrb->rb->lock, RISC_V_CPU);
	return 0;
}

unsigned int write_ring_buffer(unsigned char *buffer, unsigned int size)
{
	return __write_ring_buffer(&uboot_m_rb, buffer, size);
}

unsigned int bl30_write_ring_buffer(unsigned char *buffer, unsigned int size)
{
	//return __write_ring_buffer(&bl30_m_rb, buffer, size);
	return 0;
}

static void __reset_ring_buffer(struct ring_buffer *rb)
{
	if (!rb || rb->magic != MAGIC)
		return;

	rb->head = 0;
	rb->tail = 0;
	rb->len  = 0;
	rb->len_b  = 0;
	amp_lock_init(&rb->lock);
}

void reset_ring_buffer(void)
{
	__reset_ring_buffer(uboot_m_rb.rb);
	__reset_ring_buffer(bl30_m_rb.rb);
}

void blx_init_uboot_log(unsigned long base_addr, unsigned long size)
{
	blx_ready = 1;
	//initial_ring_buffer((unsigned char *)UBOOT_LOG_BUF_BASE, UBOOT_LOG_BUF_SIZE);
	initial_ring_buffer((unsigned char *)base_addr, size);
	if (!uboot_m_rb.rb) {
		pr_notice("uboot log init fail\n");
		return;
	}
	pr_notice("%s:%d uboot log len:0x%x,len_b:0x%x\n", __func__, __LINE__,
		 uboot_m_rb.rb->len, uboot_m_rb.rb->len_b);
}

static void bl30_copy_to_ddr(char *src, char *dst, int size)
{
	int i;

	for (i = 0; i < size; i++)
		*dst++ = *src++;
}

void bl30_init_log(unsigned long base_addr, unsigned long size)
{
	struct ring_buffer *b = (struct ring_buffer *)base_addr;

	if (!b || size < sizeof(struct ring_buffer))
		return;
	if (b->magic == MAGIC) {
		//memcpy((void *)bl30_log_buf,  (void *)base_addr, BL30_LOG_BUF_SIZE);
		//copy from sram to ddr
		bl30_copy_to_ddr((char *)base_addr, (char *)bl30_log_buf, BL30_LOG_BUF_SIZE);
		bl30_m_rb.rb = (struct ring_buffer *)bl30_log_buf;
		bl30_m_rb.data = ((char *)bl30_m_rb.rb) + sizeof(struct ring_buffer);
		pr_notice("%s:%d bl30 log len:0x%x,len_b:0x%x\n", __func__, __LINE__,
			 bl30_m_rb.rb->len, bl30_m_rb.rb->len_b);
		return;
	}
	pr_notice("bl30 log don't be saved\n");
}

/*
 * when bl30 log will be updated, need confirm bl30 update log to sram finish.
 */
void bl30_log_update(unsigned long base_addr, unsigned long size)
{
	unsigned int save_head;
	struct ring_buffer *b = (struct ring_buffer *)base_addr;

	if (!b || size < sizeof(struct ring_buffer))
		return;
	if (b->magic == MAGIC) {
		if (bl30_m_rb.rb)
			save_head = bl30_m_rb.rb->head;
		//memcpy((void *)bl30_log_buf,  (void *)base_addr, BL30_LOG_BUF_SIZE);
		bl30_copy_to_ddr((char *)base_addr, (char *)bl30_log_buf, BL30_LOG_BUF_SIZE);
		if (bl30_m_rb.rb)
			bl30_m_rb.rb->head = save_head;
		if (!bl30_m_rb.rb) {
			bl30_m_rb.rb = (struct ring_buffer *)bl30_log_buf;
			bl30_m_rb.data = ((char *)bl30_m_rb.rb) + sizeof(struct ring_buffer);
			pr_notice("%s:%d bl30 log len:0x%x,len_b:0x%x\n", __func__, __LINE__,
				 bl30_m_rb.rb->len, bl30_m_rb.rb->len_b);
		}
		pr_notice("bl30 log update to ddr end\n");
	}
}

void blx_put_char(unsigned char c)
{
	if (blx_ready) {
		if (c == '\n') {
			unsigned char c_r = '\r';

			write_ring_buffer(&c_r, 1);
		}
		write_ring_buffer(&c, 1);
	}
}

//#define DUMP_SAVE_LOG_TEST

#ifdef DUMP_SAVE_LOG_TEST
#include <serial.h>
#include <string.h>
void dump_loop_buf_log(void)
{
	unsigned char r_buf[50];
	unsigned int timer = 0;
	int i;
	unsigned int status;
	unsigned int count;

	blx_ready = 0;
	//pr_notice("save log addr: 0x%lx\n", UBOOT_LOG_BUF_BASE);
	pr_notice("*dump start:\n");
	memset(r_buf, 0, sizeof(r_buf));
	for (i = 0; i < 1000; i++) {
		memset(r_buf, 0, sizeof(r_buf));
		timer = 0;
		status = 0;
		count = read_ring_buffer(&timer, r_buf, sizeof(r_buf), &status);
		//pr_notice("dump status: 0x%x, ,count: 0x%x\n", status, count);

		if (count > 0) {
			switch (status & 3) {
			case 1:
			case 3:
				pr_notice("dump timer: 0x%x, %s", timer, r_buf);
			break;
			case 0:
			case 2:
				pr_notice("%s", r_buf);
			break;
			default:
			break;
			}
		}
	}
	pr_notice("*dump end\n");
}
#endif

