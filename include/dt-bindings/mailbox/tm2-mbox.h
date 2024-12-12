/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __TM2_MBOX_H__
#define __TM2_MBOX_H__

#include "amlogic,mbox.h"

/* MAILBOX DRIVER ID */
/* ARMREE to AOCPU driver ID */
#define TM2_REE2AO0       64
#define TM2_REE2AO1       65
#define TM2_REE2AO2       66
#define TM2_REE2AO3       67
#define TM2_REE2AO4       68
#define TM2_REE2AO5       69
#define TM2_REE2AO6       70
#define TM2_REE2AO7       71
#define TM2_REE2AO8       72
#define TM2_REE2AO9       73
#define TM2_REE2AO10      74

/* DSPA to ARMREE driver ID */
#define TM2_DSPA2REE0     129
#define TM2_DSPA2REE1     130
#define TM2_DSPA2REE2     131
#define TM2_DSPA2REE3     132
#define TM2_DSPA2REE4     133

/* ARMREE to DSPA driver ID */
#define TM2_REE2DSPA0     161
#define TM2_REE2DSPA1     162
#define TM2_REE2DSPA2     163
#define TM2_REE2DSPA3     164
#define TM2_REE2DSPA4     165

/* DSPB to ARMREE driver ID */
#define TM2_DSPB2REE0     145
#define TM2_DSPB2REE1     146
#define TM2_DSPB2REE2     147
#define TM2_DSPB2REE3     148
#define TM2_DSPB2REE4     149

/* ARMREE to DSPA driver ID */
#define TM2_REE2DSPB0     177
#define TM2_REE2DSPB1     178
#define TM2_REE2DSPB2     179
#define TM2_REE2DSPB3     180
#define TM2_REE2DSPB4     181

/* MBOX CLIENT ID */
/* ARMREE to AOCPU client ID */
#define TM2_REE2AO_DEV        TM2_REE2AO0
#define TM2_REE2AO_VRTC       TM2_REE2AO1
#define TM2_REE2AO_HIFI4DSPA  TM2_REE2AO2
#define TM2_REE2AO_HIFI4DSPB  TM2_REE2AO3
#define TM2_REE2AO_AOCEC      TM2_REE2AO4
#define TM2_REE2AO_LED        TM2_REE2AO5
#define TM2_REE2AO_ETH        TM2_REE2AO6

/* ARMREE to DSPA client ID */
#define TM2_REE2DSPA_DEV      TM2_REE2DSPA0
#define TM2_REE2DSPA_DSP      TM2_REE2DSPA1
#define TM2_REE2DSPA_AUDIO    TM2_REE2DSPA2

/* DSPA to ARMREE client ID */
#define TM2_DSPA2REE_DEV      TM2_DSPA2REE0

/* ARMREE to DSPB client ID */
#define TM2_REE2DSPB_DEV      TM2_REE2DSPB0
#define TM2_REE2DSPB_DSP      TM2_REE2DSPB1
#define TM2_REE2DSPB_AUDIO    TM2_REE2DSPB2

/* DSPB to ARMREE client ID */
#define TM2_DSPB2REE_DEV      TM2_DSPB2REE0

/* MBOX CHANNEL ID */
#define TM2_MBOX_REE2AO_LOW    0
#define TM2_MBOX_REE2AO_HIGH   1
#define TM2_MBOX_PL0_NUMS      2

#define TM2_MBOX_REE2DSPA      0
#define TM2_MBOX_DSPA2REE      1
#define TM2_MBOX_REE2DSPB      2
#define TM2_MBOX_DSPB2REE      3
#define TM2_MBOX_PL1_NUMS      4

#endif /* __TM2_MBOX_H__ */
