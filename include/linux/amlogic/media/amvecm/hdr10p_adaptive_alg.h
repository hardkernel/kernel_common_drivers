/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>

#ifndef HDR10_ADAPTIVE
#define HDR10_ADAPTIVE

#ifndef MAX
#define MAX(a, b) ({ \
			const typeof(a) _a = a; \
			const typeof(b) _b = b; \
			_a > _b ? _a : _b; \
		})
#endif // MAX

#ifndef MIN
#define MIN(a, b) ({ \
			const typeof(a) _a = a; \
			const typeof(b) _b = b; \
			_a < _b ? _a : _b; \
		})
#endif // MIN

#ifndef ABS
#define ABS(x) ({ \
			const typeof(x) _x = x; \
			_x > 0 ? _x : -_x; \
		})
#endif

struct hdr10p_adap_param_s {
	unsigned int en;
	unsigned int al;
	unsigned int r;
	unsigned int k2;
	int b;
	unsigned int peak_nit;
	int ogain_shift;
	unsigned int scale;
	unsigned int *min_rate;
	unsigned int *max_rate;

	void (*adap_hdr10p_alg)(unsigned int *hdr10p_ogain, unsigned int *hdr10p_adp_ogain,
		struct hdr10p_adap_param_s *adap_par);
};

struct hdr10p_adap_param_s *get_hdr10p_par(void);
#endif
