/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __G12A_MBOX_H__
#define __G12A_MBOX_H__

#include "amlogic,mbox.h"

/* MAILBOX DRIVER ID */
/* ARMREE to AOCPU driver ID */
#define G12A_REE2AO0      64
#define G12A_REE2AO1      65
#define G12A_REE2AO2      66
#define G12A_REE2AO3      67
#define G12A_REE2AO4      68
#define G12A_REE2AO5      69
#define G12A_REE2AO6      70
#define G12A_REE2AO7      71
#define G12A_REE2AO8      72
#define G12A_REE2AO9      73
#define G12A_REE2AO10     74

/* ARMREE to M4 driver ID */
#define G12A_REE2MF0      209
#define G12A_REE2MF1      210
#define G12A_REE2MF2      211
#define G12A_REE2MF3      212
#define G12A_REE2MF4      213

/* M4 to ARMREE driver ID */
#define G12A_MF2REE0      193
#define G12A_MF2REE1      194
#define G12A_MF2REE2      195
#define G12A_MF2REE3      196
#define G12A_MF2REE4      197

/* MBOX CLIENT ID */
/* ARMREE to AOCPU client ID */
#define G12A_REE2AO_DEV    G12A_REE2AO0
#define G12A_REE2AO_VRTC   G12A_REE2AO1
#define G12A_REE2AO_MF     G12A_REE2AO2
#define G12A_REE2AO_AOCEC  G12A_REE2AO3
#define G12A_REE2AO_LED    G12A_REE2AO4
#define G12A_REE2AO_ETH    G12A_REE2AO5

/* ARMREE to M4 client ID */
#define G12A_REE2MF_MF     G12A_REE2MF0

/* ARMREE to M4 client ID */
#define G12A_MF2REE_DEV    G12A_MF2REE0

/* MBOX CHANNEL ID */
#define G12A_MBOX_MF2REE   0
#define G12A_MBOX_REE2MF   1
#define G12A_MBOX_MF_NUMS  2

#define G12A_MBOX_REE2AO   0
#define G12A_MBOX_AO_NUMS  1

#endif /* __G12A_MBOX_H__ */
