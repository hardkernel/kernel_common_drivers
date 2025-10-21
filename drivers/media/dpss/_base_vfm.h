/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __BASE_VFM_H__
#define __BASE_VFM_H__

#define _IS_VDIN_SRC(src) (				\
	((src) == VFRAME_SOURCE_TYPE_TUNER)	||	\
	((src) == VFRAME_SOURCE_TYPE_CVBS)	||	\
	((src) == VFRAME_SOURCE_TYPE_COMP)	||	\
	((src) == VFRAME_SOURCE_TYPE_HDMI))

#endif				/* __BASE_VFM_H__ */
