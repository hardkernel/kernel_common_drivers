/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __C1_MBOX_H__
#define __C1_MBOX_H__

#include "amlogic,mbox.h"

/* MAILBOX DRIVER ID */
/* DSPA to ARMREE driver ID */
#define C1_DSPA2REE0      129
#define C1_DSPA2REE1      130
#define C1_DSPA2REE2      131
#define C1_DSPA2REE3      132
#define C1_DSPA2REE4      133

/* ARMREE to DSPA driver ID */
#define C1_REE2DSPA0      161
#define C1_REE2DSPA1      162
#define C1_REE2DSPA2      163
#define C1_REE2DSPA3      164
#define C1_REE2DSPA4      165

/* DSPB to ARMREE driver ID */
#define C1_DSPB2REE0      145
#define C1_DSPB2REE1      146
#define C1_DSPB2REE2      147
#define C1_DSPB2REE3      148
#define C1_DSPB2REE4      149

/* ARMREE to DSPA driver ID */
#define C1_REE2DSPB0      177
#define C1_REE2DSPB1      178
#define C1_REE2DSPB2      179
#define C1_REE2DSPB3      180
#define C1_REE2DSPB4      181

/* MBOX CLIENT ID */
/* ARMREE to DSPA client ID */
#define C1_REE2DSPA_DEV   C1_REE2DSPA0
#define C1_REE2DSPA_DSP   C1_REE2DSPA1
#define C1_REE2DSPA_AUDIO C1_REE2DSPA2

/* DSPA to ARMREE client ID */
#define C1_DSPA2REE_DEV   C1_DSPA2REE0

/* ARMREE to DSPB client ID */
#define C1_REE2DSPB_DEV  C1_REE2DSPB0
#define C1_REE2DSPB_DSP  C1_REE2DSPB1
#define C1_REE2DSPB_AUDIO C1_REE2DSPB2

/* DSPB to ARMREE client ID */
#define C1_DSPB2REE_DEV  C1_DSPB2REE0

/* MBOX CHANNEL ID */
#define C1_MBOX_DSPA2REE  0
#define C1_MBOX_REE2DSPA  1
#define C1_MBOX_DSPB2REE  2
#define C1_MBOX_REE2DSPB  3
#define C1_MBOX_NUMS      4

#endif /* __C1_MBOX_H__ */
