/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __SM1_MBOX_H__
#define __SM1_MBOX_H__

#include "amlogic,mbox.h"

/* MAILBOX DRIVER ID */
/* ARMREE to AOCPU driver ID */
#define SM1_REE2AO0       64
#define SM1_REE2AO1       65
#define SM1_REE2AO2       66
#define SM1_REE2AO3       67
#define SM1_REE2AO4       68
#define SM1_REE2AO5       69
#define SM1_REE2AO6       70
#define SM1_REE2AO7       71
#define SM1_REE2AO8       72
#define SM1_REE2AO9       73
#define SM1_REE2AO10      74

/* ARMREE to M4 driver ID */
#define SM1_REE2MF0       209
#define SM1_REE2MF1       210
#define SM1_REE2MF2       211
#define SM1_REE2MF3       212
#define SM1_REE2MF4       213

/* MBOX CLIENT ID */
/* ARMREE to AOCPU client ID */
#define SM1_REE2AO_DRV       SM1_REE2AO0
#define SM1_REE2AO_DEV       SM1_REE2AO1
#define SM1_REE2AO_VRTC      SM1_REE2AO2
#define SM1_REE2AO_MF        SM1_REE2AO3
#define SM1_REE2AO_KEYPAD    SM1_REE2AO4
#define SM1_REE2AO_AOCEC     SM1_REE2AO5
#define SM1_REE2AO_LED       SM1_REE2AO6
#define SM1_REE2AO_ETH       SM1_REE2AO7

/* ARMREE to M4 client ID */
#define SM1_REE2MF_MF        SM1_REE2MF0

/* MBOX CHANNEL ID */
#define SM1_MBOX_MF2REE      0
#define SM1_MBOX_REE2MF      1
#define SM1_MBOX_MF_NUMS     2

#define SM1_MBOX_REE2AO      0
#define SM1_MBOX_AO_NUMS     1

#endif /* __SM1_MBOX_H__ */
