/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __A5_MBOX_H__
#define __A5_MBOX_H__

#include "amlogic,mbox.h"

// MBOX DRIVER ID
#define A5_DSPA2REE0     0
#define A5_REE2DSPA0     1
#define A5_REE2DSPA1     2
#define A5_REE2DSPA2     3
#define A5_AO2REE        4
#define A5_REE2AO0       (A5_AO2REE + 1)
#define A5_REE2AO1       (A5_AO2REE + 2)
#define A5_REE2AO2       (A5_AO2REE + 3)
#define A5_REE2AO3       (A5_AO2REE + 4)
#define A5_REE2AO4       (A5_AO2REE + 5)
#define A5_REE2AO5       (A5_AO2REE + 6)
#define A5_REE2AO6       (A5_AO2REE + 7)

// DEVICE TREE ID
#define A5_REE2DSPA_DEV  A5_REE2DSPA0
#define A5_REE2DSPA_DSP  A5_REE2DSPA1
#define A5_DSPA2REE_DEV  A5_DSPA2REE0
#define A5_REE2AO_DEV    A5_REE2AO0
#define A5_REE2AO_VRTC   A5_REE2AO1
#define A5_REE2AO_RTC    A5_REE2AO2
#define A5_REE2AO_KEYPAD A5_REE2AO3
#define A5_REE2AO_AOCEC  A5_REE2AO4
#define A5_REE2AO_LED    A5_REE2AO5
#define A5_REE2AO_ETH    A5_REE2AO6

// MBOX CHANNEL ID
#define A5_MBOX_DSPA2REE  0
#define A5_MBOX_REE2DSPA  1
#define A5_MBOX_AO2REE    2
#define A5_MBOX_REE2AO    3
#define A5_MBOX_NUMS      4

#endif /* __A5_MBOX_H__ */
