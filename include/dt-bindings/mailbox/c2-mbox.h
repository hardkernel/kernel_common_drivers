/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __C2_MBOX_H__
#define __C2_MBOX_H__

#include "amlogic,mbox.h"

/* MAILBOX DRIVER ID */
/* SEC to ARMREE driver ID */
#define C2_SE2REE0       0
#define C2_SE2REE1       1
#define C2_SE2REE2       2
#define C2_SE2REE3       3
#define C2_SE2REE4       4
#define C2_SE2REE5       5
#define C2_SE2REE6       6
#define C2_SE2REE7       7
#define C2_SE2REE8       8
#define C2_SE2REE9       9
#define C2_SE2REE10      10

/* ARMREE to SEC driver ID */
#define C2_REE2SE0       64
#define C2_REE2SE1       65
#define C2_REE2SE2       66
#define C2_REE2SE3       67
#define C2_REE2SE4       68
#define C2_REE2SE5       69
#define C2_REE2SE6       70
#define C2_REE2SE7       71
#define C2_REE2SE8       72
#define C2_REE2SE9       73
#define C2_REE2SE10      74

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
/* ARMREE to DSPA client ID */
#define C2_REE2DSPA_DEV  C2_REE2DSPA0
#define C2_REE2DSPA_DSP  C2_REE2DSPA1

/* DSPA to ARMREE client ID */
#define C2_DSPA2REE_DEV  C2_DSPA2REE0

/* ARMREE to SEC client ID */
#define C2_REE2SE_DEV    C2_REE2SE0

/* MBOX CHANNEL ID */
#define C2_MBOX_REE2DSPA  0
#define C2_MBOX_DSPA2REE  1
#define C2_MBOX_REE2SE    2
#define C2_MBOX_SE2REE    3
#define C2_MBOX_NUMS      4

#endif /* __C2_MBOX_H__ */
