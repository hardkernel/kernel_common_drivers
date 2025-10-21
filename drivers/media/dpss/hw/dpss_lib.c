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

