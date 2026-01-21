// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "define.h"
#include "dpss_lib.h"

void vpu_initqueue(struct Vpu_queue *queue)
{
	// queue->front = 0;
	///queue->rear = 0;
	memset(queue, 0x0, sizeof(struct Vpu_queue));
}
EXPORT_SYMBOL(vpu_initqueue);

bool vpu_is_queue_empty(struct Vpu_queue *queue)
{
	return queue->front == queue->rear;
}
EXPORT_SYMBOL(vpu_is_queue_empty);

bool vpu_enqueue(struct Vpu_queue *queue, int value)
{
	if ((queue->rear + 1) % QUEEN_NUM == queue->front) {
		//stimulus_printf("Queue is full\n");
		return 0;
	}
	queue->data[queue->rear] = value;
	queue->rear = (queue->rear + 1) % QUEEN_NUM;
	//dbg_h2("enqueue : value=%0x,",value);
	//dbg_h2("rear:%x\n",queue->rear);
	return 1;
}
EXPORT_SYMBOL(vpu_enqueue);

bool vpu_dequeue(struct Vpu_queue *queue, int *value, int *empty)
{
	if (vpu_is_queue_empty(queue)) {
		// stimulus_printf("Queue is empty\n");
		*empty = 1;
		return 0;
	}
	*value = queue->data[queue->front];
	queue->front = (queue->front + 1) % QUEEN_NUM;
	//dbg_h2("dequeue : value=%0x,",*value);
	//fpga_fifo_printf("front:%x\n",queue->front);
	return 1;
}
EXPORT_SYMBOL(vpu_dequeue);

void dpss_initqueue(struct dpss_queue *queue)
{
	memset(queue, 0x0, sizeof(struct dpss_queue));
}

bool dpss_is_queue_empty(struct dpss_queue *queue)
{
	return queue->front == queue->rear;
}

bool dpss_enqueue(struct dpss_queue *queue, int value)
{
	if ((queue->rear + 1) % DPSS_QUEEN_NUM == queue->front)
		return 0;
	queue->data[queue->rear] = value;
	queue->rear = (queue->rear + 1) % DPSS_QUEEN_NUM;
	return 1;
}

bool dpss_peek_queue(struct dpss_queue *queue, int *value, bool *empty)
{
	if (dpss_is_queue_empty(queue)) {
		*empty = 1;
		return 0;
	}
	*value = queue->data[queue->front];
	return 1;
}

bool dpss_put_queue(struct dpss_queue *queue, int *value, bool *empty)
{
	if (dpss_is_queue_empty(queue)) {
		*empty = 1;
		return 0;
	}
	*value = queue->data[queue->front];
	queue->front = (queue->front + 1) % DPSS_QUEEN_NUM;
	return 1;
}

void dpss_put_last_queue(struct dpss_queue *queue, bool *empty, u32 *value)
{
	u16 tmp_idx;

	if (dpss_is_queue_empty(queue)) {
		*empty = 1;
		return;
	}

	tmp_idx = queue->rear;

	if (queue->rear != 0)
		tmp_idx--;
	else
		tmp_idx = DPSS_QUEEN_NUM - 1;

	*value = queue->data[tmp_idx];
}

void display_init_queue(struct display_queue *queue)
{
	memset(queue, 0x0, sizeof(struct display_queue));
}

bool display_queue_is_empty(struct display_queue *queue)
{
	return queue->inp_idx == queue->drop_idx;
}

bool display_queue_put(struct display_queue *queue)
{
	u8 drop_index = queue->drop_idx;
	u8 disp_index = queue->disp_idx;

	if (disp_index < drop_index)
		disp_index += DPSS_QUEEN_NUM;

	return disp_index > drop_index ? true : false;
}

u8 get_dst_buf_cnt(struct display_queue *queue)
{
	u8 drop_index = queue->drop_idx;
	u8 inp_index = queue->inp_idx;

	if (inp_index < drop_index)
		inp_index += DPSS_QUEEN_NUM;

	return inp_index - drop_index;
}

//0:drop_index
//1:disp_idx
//2:mc_index
//3:inp_index
void display_queue_use_last(struct display_queue *queue, u8 sel)
{
	u8 *index;

	if (!queue)
		return;

	index = sel == 0 ? &queue->drop_idx : sel == 1 ? &queue->disp_idx : sel == 2 ?
			&queue->mc_idx : &queue->inp_idx;
	if (*index == 0 || *index >= DPSS_QUEEN_NUM)
		*index = DPSS_QUEEN_NUM - 1;
	else
		(*index)--;
}

