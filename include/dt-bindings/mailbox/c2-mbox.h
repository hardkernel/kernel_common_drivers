/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __C2_MBOX_H__
#define __C2_MBOX_H__

#include "amlogic,mbox.h"

/* MAILBOX DRIVER ID */
/* AOCPU to ARMREE driver ID */
#define C2_AO2REE0       0
#define C2_AO2REE1       1
#define C2_AO2REE2       2
#define C2_AO2REE3       3
#define C2_AO2REE4       4
#define C2_AO2REE5       5
#define C2_AO2REE6       6
#define C2_AO2REE7       7
#define C2_AO2REE8       8
#define C2_AO2REE9       9
#define C2_AO2REE10      10

/* ARMREE to AOCPU driver ID */
#define C2_REE2AO0       64
#define C2_REE2AO1       65
#define C2_REE2AO2       66
#define C2_REE2AO3       67
#define C2_REE2AO4       68
#define C2_REE2AO5       69
#define C2_REE2AO6       70
#define C2_REE2AO7       71
#define C2_REE2AO8       72
#define C2_REE2AO9       73
#define C2_REE2AO10      74

/* DSPA to ARMREE driver ID */
#define C2_DSPA2REE0     129
#define C2_DSPA2REE1     130
#define C2_DSPA2REE2     131
#define C2_DSPA2REE3     132
#define C2_DSPA2REE4     133

/* ARMREE to DSPA driver ID */
#define C2_REE2DSPA0     161
#define C2_REE2DSPA1     162
#define C2_REE2DSPA2     163
#define C2_REE2DSPA3     164
#define C2_REE2DSPA4     165

/* MBOX CLIENT ID */
/* AOCPU to ARMREE client ID */
#define C2_AO2REE_DEV    C2_AO2REE0

/* ARMREE to AOCPU client ID */
#define C2_REE2AO_DEV    C2_REE2AO0
#define C2_REE2AO_VRTC   C2_REE2AO1
#define C2_REE2AO_KEYPAD C2_REE2AO2
#define C2_REE2AO_AOCEC  C2_REE2AO3

/* ARMREE to DSPA client ID */
#define C2_REE2DSPA_DEV  C2_REE2DSPA0
#define C2_REE2DSPA_DSP  C2_REE2DSPA1

/* DSPA to ARMREE client ID */
#define C2_DSPA2REE_DEV  C2_DSPA2REE0

/* MBOX CHANNEL ID */
#define C2_MBOX_REE2DSPA  0
#define C2_MBOX_DSPA2REE  1
#define C2_MBOX_REE2AO    2
#define C2_MBOX_AO2REE    3
#define C2_MBOX_NUMS      4

#endif /* __C2_MBOX_H__ */
