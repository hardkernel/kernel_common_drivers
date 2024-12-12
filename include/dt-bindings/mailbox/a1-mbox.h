/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __A1_MBOX_H__
#define __A1_MBOX_H__

#include "amlogic,mbox.h"

/* MAILBOX DRIVER ID */
/* DSPA to ARMREE driver ID */
#define A1_DSPA2REE0      129
#define A1_DSPA2REE1      130
#define A1_DSPA2REE2      131
#define A1_DSPA2REE3      132
#define A1_DSPA2REE4      133

/* ARMREE to DSPA driver ID */
#define A1_REE2DSPA0      161
#define A1_REE2DSPA1      162
#define A1_REE2DSPA2      163
#define A1_REE2DSPA3      164
#define A1_REE2DSPA4      165

/* DSPB to ARMREE driver ID */
#define A1_DSPB2REE0      145
#define A1_DSPB2REE1      146
#define A1_DSPB2REE2      147
#define A1_DSPB2REE3      148
#define A1_DSPB2REE4      149

/* ARMREE to DSPA driver ID */
#define A1_REE2DSPB0      177
#define A1_REE2DSPB1      178
#define A1_REE2DSPB2      179
#define A1_REE2DSPB3      180
#define A1_REE2DSPB4      181

/* MBOX CLIENT ID */
/* ARMREE to DSPA client ID */
#define A1_REE2DSPA_DEV   A1_REE2DSPA0
#define A1_REE2DSPA_DSP   A1_REE2DSPA1
#define A1_REE2DSPA_AUDIO A1_REE2DSPA2

/* DSPA to ARMREE client ID */
#define A1_DSPA2REE_DEV   A1_DSPA2REE0

/* ARMREE to DSPB client ID */
#define A1_REE2DSPB_DEV   A1_REE2DSPB0
#define A1_REE2DSPB_DSP   A1_REE2DSPB1
#define A1_REE2DSPB_AUDIO A1_REE2DSPB2

/* DSPB to ARMREE client ID */
#define A1_DSPB2REE_DEV   A1_DSPB2REE0

/* MBOX CHANNEL ID */
#define A1_MBOX_DSPA2REE  0
#define A1_MBOX_REE2DSPA  1
#define A1_MBOX_DSPB2REE  2
#define A1_MBOX_REE2DSPB  3
#define A1_MBOX_NUMS      4

#endif /* __A1_MBOX_H__ */
