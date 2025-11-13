// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#include "vpp_common.h"

#define LEN_INT    (32)

int vpp_check_range(int val, int down, int up)
{
	int ret = 0;

	if (down < 0 || up < 0)
		return val;

	if (val < down)
		ret = down;
	else if (val > up)
		ret = up;
	else
		ret = val;

	return ret;
}

int vpp_mask_int(int val, int start, int len)
{
	unsigned int ret = 0;
	int tmp = LEN_INT - len;

	if (start < 0 || len < 0)
		return val;

	if (tmp < 0)
		tmp = 0;

	ret = ~((0xffffffff >> len) << len);
	ret &= ((val >> start) << tmp) >> tmp;

	return ret;
}

int vpp_insert_int(int src_val, int insert_val,
	int start, int len)
{
	unsigned int ret = 0;
	int tmp = LEN_INT - len;
	int mask;

	if (start < 0 || len < 0)
		return src_val;

	if (tmp < 0)
		tmp = 0;

	if (!tmp)
		return insert_val;

	if (tmp - start < 0)
		return src_val;

	ret = ~((0xffffffff >> (start + len)) << (start + len));
	mask = ~(ret & (((0xffffffff >> start) << tmp) >> (tmp - start)));
	tmp = insert_val << start;
	ret = (src_val & mask) | tmp;

	return ret;
}

#endif

