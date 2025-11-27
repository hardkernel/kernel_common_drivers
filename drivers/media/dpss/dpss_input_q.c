// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include "sys_def.h" //ary add for sim
#ifdef RUN_ON_ARM
#include <linux/kfifo.h>
#include <linux/types.h>
#include <linux/kernel.h>
//#include <linux/sched/clock.h>
#include <linux/delay.h> //usleep_range

#include <linux/amlogic/media/di/dpss_interface.h>

#endif /* RUN_ON_ARM */

#include "dpss_base.h"
#include "dpss_s.h"
#include "dpss_sys.h"
#include "dpss_func.h"

void dpss_input_q_init(struct dpss_input_q *q)
{
	int i;

	mutex_init(&q->input_q_mutex);
	spin_lock_init(&q->kfifo_lock);

	mutex_lock(&q->input_q_mutex);
	for (i = 0; i < DPSS_INP_LIST_COUNT; i++) {
		q->input[i].index = i;
		q->input[i].input_in_index = 0;
		q->input[i].input_done_index = 0;
		q->input[i].dropped_count = 0;
		q->input[i].used = false;
		q->input[i].input_done = false;
		q->input[i].is_key_frame = false;
		q->input[i].dpe_done = false;
		q->input[i].is_i = false;
		q->input[i].buf_index = 0;
	}

	INIT_KFIFO(q->input_q);
	kfifo_reset(&q->input_q);

	q->input_frame_count = 0;
	q->dpe_done_count = 0;
	q->output_idx_last = 0;
	q->total_drop_count  = 0;
	q->wait_hw_finish    = false;

	mutex_unlock(&q->input_q_mutex);

	dbg_ct0("input_q: reset\n");
}

u32 dpss_input_q_free_count(struct dpss_input_q *q)
{
	int free_count = 0;
	int i;

	mutex_lock(&q->input_q_mutex);
	for (i = 0; i < DPSS_INP_LIST_COUNT; i++) {
		if (!q->input[i].used)
			free_count++;
	}
	mutex_unlock(&q->input_q_mutex);

	dbg_ct0("input_q: free_count=%d\n", free_count);
	return free_count;
}

void dpss_input_q_in(struct dpss_input_q *q, struct vframe_s *vfm)
{
	int i = 0;

	mutex_lock(&q->input_q_mutex);
	for (i = 0; i < DPSS_INP_LIST_COUNT; i++) {
		if (!q->input[i].used)
			break;
	}

	if (i >= DPSS_INP_LIST_COUNT) {
		DBG_ERR("input_q_in fail\n");
		mutex_unlock(&q->input_q_mutex);
		return;
	}

	q->input[i].input_in_index = q->input_frame_count;
	q->input[i].input_done_index = 0;
	q->input[i].used = true;
	q->input[i].input_done = false;
	q->input[i].is_key_frame = false;
	q->input[i].dpe_done = false;
	q->input[i].is_i = vfm->type & VIDTYPE_INTERLACE;

	if (!kfifo_put(&q->input_q, &q->input[i]))
		DBG_ERR("%s: input_q is full!, i=%d\n", __func__, i);
	dbg_ct0("input_q: i=%d,index=%d, in_index=%d,list_len=%d\n",
		i,
		q->input[i].index,
		q->input[i].input_in_index,
		kfifo_len(&q->input_q));

	q->input_frame_count++;
	mutex_unlock(&q->input_q_mutex);
}

void dpss_input_q_update_status(struct dpss_input_q *q, u32 inp_ofrm_vld, bool is_i)
{
	int i;
	struct prm_dpss_input *input = NULL;
	struct prm_dpss_input *input_tmp[DPSS_INP_LIST_COUNT];
	int len;

	if (is_i) {
		dbg_ct0("irq: input_q: is_i key_frame=%d\n", inp_ofrm_vld);
		return;
	}

	len = kfifo_len(&q->input_q);
	if (len == 0) {
		DBG_ERR("irq: input_q: task: peek failed\n");
	} else {
		if (kfifo_out_peek(&q->input_q, (void *)&input_tmp, len)) {
			for (i = 0; i < len; i++) {
				input = input_tmp[i];
				dbg_ct0("irq: input_q: debug: list i=%d\n", input->index);
				if (!input->input_done)
					break;
			}
			if (i == len)
				DBG_ERR("irq: input_q: no input undone frame\n");
		}
		if (!input)
			return;
		dbg_ct0("irq: input_q: current index=%d, in_index=%d\n",
			input->index, input->input_in_index);

		if (inp_ofrm_vld == 0) {
			input->is_key_frame = false;
			q->total_drop_count++;
		} else {
			input->is_key_frame = true;
			input->dropped_count = q->total_drop_count;
		}
		input->input_done_index = q->input_frame_count;
		input->input_done = true;
		dbg_ct0("irq: input_q: index=%d, key_frame=%d,len=%d",
			input->index, input->is_key_frame,
			kfifo_len(&q->input_q));
	}
}

void dpss_input_q_update_dpe_status(struct PRM_DPSS_TOP *top, u32 addr_y)
{
	int i;
	struct dpss_input_q *queue = NULL;
	struct prm_dpss_input *input = NULL;
	struct prm_dpss_input *input_tmp[DPSS_INP_LIST_COUNT];
	int len;

	if (!top) {
		dbg_ct0("%s: top is NULL.\n", __func__);
		return;
	}

	queue = top->in_q;
	len = kfifo_len(&queue->input_q);
	dbg_ct0("%s: input_q len is %d, input_frame_count=%d.\n",
		__func__,
		len,
		queue->input_frame_count);

	if (len == 0) {
		DBG_ERR("%s: input_q is empty.\n", __func__);
	} else {
		if (top->is_i && queue->input_frame_count < 2) {
			dbg_ct0("%s: no need update first frame for interlace.\n", __func__);
			return;
		}

		if (kfifo_out_peek(&queue->input_q, (void *)&input_tmp, len)) {
			for (i = 0; i < len; i++) {
				input = input_tmp[i];
				if (!input->dpe_done)
					break;
			}
			if (i == len)
				DBG_ERR("%s: no dpe undone frame\n", __func__);
		}
		if (!input) {
			DBG_ERR("%s: input is NULL.\n", __func__);
			return;
		}

		for (i = 0; i < top->num_dpe_o; i++) { //out ?
			if (addr_y == top->src0_dio_fbuf_yaddr[i])
				break;
		}
		if (i == top->num_dpe_o) {
			DBG_ERR("%s: No match buffer found for 0x%x\n", __func__, addr_y);
			return;
		}

		input->dpe_done = true;
		input->buf_index = i;
		dbg_ct0("%s: index:%d, input_index:%d,is_i:%d, buf_index:%d, addr_y=0x%x.\n",
			__func__,
			input->index,
			input->input_in_index,
			input->is_i,
			input->buf_index,
			addr_y);
	}
}

int dpss_input_get_buf_index(struct dpss_input_q *q)
{
	struct prm_dpss_input *input = NULL;
	int ret = 0;

	mutex_lock(&q->input_q_mutex);
	if (!kfifo_peek(&q->input_q, &input)) {
		DBG_WARN("%s: empty q\n", __func__);
		mutex_unlock(&q->input_q_mutex);
		return -1;
	}

	dbg_ct0("%s:index:%d, input_index:%d, is_i:%d, buf_index:%d.\n",
		__func__,
		input->index,
		input->input_in_index,
		input->is_i,
		input->buf_index);
	ret = input->buf_index;
	mutex_unlock(&q->input_q_mutex);

	return ret;
}

u32 dpss_input_get_drop_count(struct dpss_input_q *q)
{
	int i;
	struct prm_dpss_input *input = NULL;
	struct prm_dpss_input *input_tmp[DPSS_INP_LIST_COUNT];
	int len;
	u32 drop_count = 0;
	unsigned long flags;

	len = kfifo_len(&q->input_q);
	if (len == 0) {
		DBG_ERR("irq: input_q: task: peek failed\n");
	} else {
		spin_lock_irqsave(&q->kfifo_lock, flags);
		if (kfifo_out_peek(&q->input_q, (void *)&input_tmp, len)) {
			for (i = 0; i < len; i++) {
				input = input_tmp[i];
				dbg_ct0("irq: input_q: debug: list i=%d\n", input->index);
				if (input->is_key_frame)
					break;
			}
			if (i == len)
				dbg_ct0("irq: input_q: no input undone frame\n");
		}
		spin_unlock_irqrestore(&q->kfifo_lock, flags);
		if (!input)
			return 0;

		drop_count = input->dropped_count;

		dbg_ct0("irq: input_q: current index=%d, drop_count=%d, in_index=%d\n",
			input->index, drop_count, input->input_in_index);
	}

	return drop_count;
}

/*input_done  but no key_frame*/
u32 dpss_input_q_out_1(struct dpss_input_q *q)
{
	u32 count = 0;
	struct prm_dpss_input *input;
	unsigned long flags;

	mutex_lock(&q->input_q_mutex);
	while (1) {
		if (!kfifo_peek(&q->input_q, &input))
			break;

		if (input->input_done && !input->is_key_frame) {
			count++;
			spin_lock_irqsave(&q->kfifo_lock, flags);
			if (!kfifo_get(&q->input_q, &input))
				DBG_ERR("%s: task: get failed\n", __func__);
			else
				input->used = false;
			spin_unlock_irqrestore(&q->kfifo_lock, flags);
			dbg_ct0("%s:input_q: output vf_1: no_key index=%d, in_index=%d\n",
				__func__, input->index, input->input_in_index);
		} else {
			break;
		}
	}
	mutex_unlock(&q->input_q_mutex);

	return count;
}

/*input_done  but key_frame*/
void dpss_input_q_out_head_key(struct dpss_input_q *q)
{
	struct prm_dpss_input *input;
	unsigned long flags;

	mutex_lock(&q->input_q_mutex);
	if (!kfifo_peek(&q->input_q, &input)) {
		DBG_WARN("%s:input_q: should not here!!! empty q\n", __func__);
		mutex_unlock(&q->input_q_mutex);
		return;
	}

	if ((input->input_done && input->is_key_frame && !input->is_i) || input->is_i) {
		dbg_ct0("%s:input_q:output vf_2:key index=%d,len=%d,in_index=%d\n",
			__func__, input->index, kfifo_len(&q->input_q),
			input->input_in_index);
		spin_lock_irqsave(&q->kfifo_lock, flags);
		if (!kfifo_get(&q->input_q, &input))
			DBG_ERR("%s:input_q:  task: get failed\n", __func__);
		else
			input->used = false;
		spin_unlock_irqrestore(&q->kfifo_lock, flags);
	} else {
		DBG_WARN("%s:input_q: should not here!!!\n", __func__);
	}
	mutex_unlock(&q->input_q_mutex);
}

/*output head for no frc case*/
void dpss_input_q_out_head(struct dpss_input_q *q)
{
	struct prm_dpss_input *input;
	unsigned long flags;

	mutex_lock(&q->input_q_mutex);
	spin_lock_irqsave(&q->kfifo_lock, flags);
	if (!kfifo_get(&q->input_q, &input))
		DBG_ERR("%s:input_q:  task: get failed\n", __func__);
	else
		input->used = false;
	spin_unlock_irqrestore(&q->kfifo_lock, flags);
	mutex_unlock(&q->input_q_mutex);
}
