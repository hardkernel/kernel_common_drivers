/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _CODEC_MM_COMPACT_H
#define _CODEC_MM_COMPACT_H

void codec_mm_compaction_init(void *data);

void codec_mm_compaction_start(int rounds);
void codec_mm_compaction_stop(void);
int codec_mm_compact_trylock(void);
void codec_mm_compact_lock(void);
void codec_mm_compact_unlock(void);

#endif //_CODEC_MM_COMPACT_H

