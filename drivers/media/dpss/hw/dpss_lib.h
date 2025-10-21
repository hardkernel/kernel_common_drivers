/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _DPSS_LIB_H_
#define _DPSS_LIB_H_
#include <linux/amlogic/media/dpss/frc_common_x.h>
#include "dpss_param.h"

#define QUEEN_NUM 60
void vpu_initqueue(struct Vpu_queue *queue);
bool vpu_is_queue_empty(struct Vpu_queue *queue);
bool vpu_enqueue(struct Vpu_queue *queue, int value);
bool vpu_dequeue(struct Vpu_queue *queue, int *value, int *empty);

extern struct Vpu_queue inp_bufQ;
extern unsigned int g_SIM_FRM_NUM;
extern unsigned int g_SIM_FRM_NUM_SRC1;

#endif
