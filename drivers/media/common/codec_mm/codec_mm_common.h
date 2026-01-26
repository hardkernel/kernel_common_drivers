/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _CODEC_MM_COMMON_H
#define _CODEC_MM_COMMON_H

#if IS_ENABLED(CONFIG_AMLOGIC_CPU_INFO)
#include <linux/amlogic/media/registers/cpu_version.h>
#else
unsigned char get_meson_cpu_version(int level)
{
	return 0;
}
#endif

static inline bool is_2k_platform(void)
{
	int cpu_type = get_cpu_type();
	int pack_type = get_meson_cpu_version(MESON_CPU_VERSION_LVL_PACK);

	if (cpu_type == MESON_CPU_MAJOR_ID_T5D ||
		cpu_type == MESON_CPU_MAJOR_ID_T6D ||
		cpu_type == MESON_CPU_MAJOR_ID_TXHD2 ||
		cpu_type == MESON_CPU_MAJOR_ID_S1A ||
		(cpu_type == MESON_CPU_MAJOR_ID_S4 && pack_type == 2) ||
		(cpu_type == MESON_CPU_MAJOR_ID_S7 && pack_type == 3))
		return true;

	return false;
}

#endif //_CODEC_MM_COMMON_H
