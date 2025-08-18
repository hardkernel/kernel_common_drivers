/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __GKI_TOOL_AMLOGIC_H
#define __GKI_TOOL_AMLOGIC_H

void gki_module_init(void);
int module_debug_init(void);

#if IS_ENABLED(CONFIG_AMLOGIC_CLASS_DEBUG)
int class_debug_init(void);
#else
static inline int class_debug_init(void)
{
	return 0;
}
#endif

#endif //__GKI_TOOL_AMLOGIC_H
