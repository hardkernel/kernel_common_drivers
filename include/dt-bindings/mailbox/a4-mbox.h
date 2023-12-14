/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __A4_MBOX_H__
#define __A4_MBOX_H__

#include "amlogic,mbox.h"

// MBOX DRIVER ID
#define A4_AO2REE        0
#define A4_REE2AO0       (A4_AO2REE + 1)
#define A4_REE2AO1       (A4_AO2REE + 2)
#define A4_REE2AO2       (A4_AO2REE + 3)
#define A4_REE2AO3       (A4_AO2REE + 4)
#define A4_REE2AO4       (A4_AO2REE + 5)
#define A4_REE2AO5       (A4_AO2REE + 6)
#define A4_REE2AO6       (A4_AO2REE + 7)

// MBOX CHANNEL ID
#define A4_MBOX_AO2REE    2
#define A4_MBOX_REE2AO    3
#define A4_MBOX_NUMS      2

// DEVICE TREE ID
#define A4_REE2AO_DEV    A4_REE2AO0
#define A4_REE2AO_VRTC   A4_REE2AO1
#define A4_REE2AO_RTC    A4_REE2AO2
#define A4_REE2AO_KEYPAD A4_REE2AO3
#define A4_REE2AO_AOCEC  A4_REE2AO4
#define A4_REE2AO_ETH    A4_REE2AO5
#define A4_REE2AO_LED    A4_REE2AO6

#endif /* __A4_MBOX_H__ */
