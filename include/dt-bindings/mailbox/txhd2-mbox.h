/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __TXHD2_MBOX_H__
#define __TXHD2_MBOX_H__

#include "amlogic,mbox.h"

/* MAILBOX DRIVER ID */
/* AOCPU to ARMREE driver ID */
#define TXHD2_AO2REE0       0
#define TXHD2_AO2REE1       1
#define TXHD2_AO2REE2       2
#define TXHD2_AO2REE3       3
#define TXHD2_AO2REE4       4
#define TXHD2_AO2REE5       5
#define TXHD2_AO2REE6       6
#define TXHD2_AO2REE7       7
#define TXHD2_AO2REE8       8
#define TXHD2_AO2REE9       9
#define TXHD2_AO2REE10      10

/* ARMREE to AOCPU driver ID */
#define TXHD2_REE2AO0       64
#define TXHD2_REE2AO1       65
#define TXHD2_REE2AO2       66
#define TXHD2_REE2AO3       67
#define TXHD2_REE2AO4       68
#define TXHD2_REE2AO5       69
#define TXHD2_REE2AO6       70
#define TXHD2_REE2AO7       71
#define TXHD2_REE2AO8       72
#define TXHD2_REE2AO9       73
#define TXHD2_REE2AO10      74

/* MBOX CLIENT ID */
/* AOCPU to ARMREE client ID */
#define TXHD2_AO2REE_DEV       TXHD2_AO2REE0
#define TXHD2_AO2REE_VRTC      TXHD2_AO2REE1
#define TXHD2_AO2REE_RTC       TXHD2_AO2REE2
#define TXHD2_AO2REE_KEYPAD    TXHD2_AO2REE3
#define TXHD2_AO2REE_AOCEC     TXHD2_AO2REE4
#define TXHD2_AO2REE_LED       TXHD2_AO2REE5
#define TXHD2_AO2REE_ETH       TXHD2_AO2REE6
#define TXHD2_AO2REE_SPINLOCK  TXHD2_AO2REE7

/* ARMREE to AOCPU client ID */
#define TXHD2_REE2AO_DRV       TXHD2_REE2AO0
#define TXHD2_REE2AO_DEV       TXHD2_REE2AO1
#define TXHD2_REE2AO_VRTC      TXHD2_REE2AO2
#define TXHD2_REE2AO_RTC       TXHD2_REE2AO3
#define TXHD2_REE2AO_KEYPAD    TXHD2_REE2AO4
#define TXHD2_REE2AO_AOCEC     TXHD2_REE2AO5
#define TXHD2_REE2AO_LED       TXHD2_REE2AO6
#define TXHD2_REE2AO_ETH       TXHD2_REE2AO7
#define TXHD2_REE2AO_SPINLOCK  TXHD2_REE2AO8
#define TXHD2_REE2AO_IR        TXHD2_REE2AO9

/* MBOX CHANNEL ID */
#define TXHD2_MBOX_AO2REE    2
#define TXHD2_MBOX_REE2AO    3
#define TXHD2_MBOX_NUMS      2

#endif /* __TXHD2_MBOX_H__ */
