// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/amlogic/media/amvecm/amvecm.h>
#include <linux/amlogic/media/amvecm/hdr10p_adaptive_alg.h>

unsigned int pr_hdr10p_adp;
#define pr_hdr10p_adap_dbg(fmt, args...)\
	do {\
		if (pr_hdr10p_adp)\
			pr_info("HDR10P_ADAPTIVE: " fmt, ##args);\
	} while (0)

#define VER "hdr10p_adaptive_20250604_v0\n"

unsigned int def_min_rate[4] = {10, 10, 6, 6};//standard: {5, 5, 1, 1}
unsigned int def_max_rate[4] = {80, 80, 70, 50};//standard: {85, 85, 75, 55}

struct hdr10p_adap_param_s hdr10p_adp_par = {
	.en = 0,
	.al = 250,
	.r = 410, //10% << 12
	.k2 = 600, //15% << 16
	.b = 600, //15% << 16
	.peak_nit = 350,
	.ogain_shift = 6,
	.scale = 1 << 10, // 10bit as 1.0
	.min_rate = def_min_rate,
	.max_rate = def_max_rate,
	.adap_hdr10p_alg = NULL,
};

struct hdr10p_adap_param_s *get_hdr10p_par(void)
{
	return &hdr10p_adp_par;
}
EXPORT_SYMBOL(get_hdr10p_par);

void hdr10p_adp_par_show(struct hdr10p_adap_param_s *p)
{
	int i;

	pr_info("en = %d\n", p->en);
	pr_info("al = %d\n", p->al);
	pr_info("r = %d\n", p->r);
	pr_info("k2 = %d\n", p->k2);
	pr_info("b = %d\n", p->b);
	pr_info("peak_nit = %d\n", p->peak_nit);
	pr_info("ogain_shift = %d\n", p->ogain_shift);
	pr_info("adap_hdr10p_alg = %p\n", p->adap_hdr10p_alg);
	for (i = 0; i < 4; i++)
		pr_info("min_rate[%d] = %d\n", i, p->min_rate[i]);
	for (i = 0; i < 4; i++)
		pr_info("max_rate[%d] = %d\n", i, p->max_rate[i]);
}

int hdr10p_adp_dbg(char **parm)
{
	long val = 0;
	struct hdr10p_adap_param_s *p = get_hdr10p_par();
	unsigned int idx;

	if (!strcmp(parm[1], "ver"))  {
		pr_hdr10p_adap_dbg(VER);
	} else if (!strcmp(parm[1], "debug")) {
		if (kstrtoul(parm[2], 10, &val) < 0)
			goto error;
		pr_hdr10p_adp = val;
		pr_hdr10p_adap_dbg("pr_hdr10p_adp = %d\n", (int)val);
	} else if (!strcmp(parm[1], "en"))  {
		if (kstrtoul(parm[2], 10, &val) < 0)
			goto error;
		p->en = val;
		pr_hdr10p_adap_dbg("en = %d\n", (int)val);
	} else if (!strcmp(parm[1], "al")) {
		if (kstrtoul(parm[2], 10, &val) < 0)
			goto error;
		p->al = (int)val;
		pr_hdr10p_adap_dbg("al = %d\n", (int)val);
	} else if (!strcmp(parm[1], "r")) {
		if (kstrtoul(parm[2], 10, &val) < 0)
			goto error;
		p->r = (int)val;
		pr_hdr10p_adap_dbg("r = %d\n", (int)val);
	} else if (!strcmp(parm[1], "k2")) {
		if (kstrtoul(parm[2], 10, &val) < 0)
			goto error;
		p->k2 = (int)val;
		pr_hdr10p_adap_dbg("k2 = %d\n", (int)val);
	} else if (!strcmp(parm[1], "b")) {
		if (kstrtoul(parm[2], 10, &val) < 0)
			goto error;
		p->b = (int)val;
		pr_hdr10p_adap_dbg("b = %d\n", (int)val);
	} else if (!strcmp(parm[1], "peak_nit")) {
		if (kstrtoul(parm[2], 10, &val) < 0)
			goto error;
		p->peak_nit = (int)val;
		pr_hdr10p_adap_dbg("peak_nit = %d\n", (int)val);
	} else if (!strcmp(parm[1], "ogain_shift")) {
		if (kstrtoul(parm[2], 10, &val) < 0)
			goto error;
		p->ogain_shift = (int)val;
		pr_hdr10p_adap_dbg("ogain_shift = %d\n", (int)val);
	} else if (!strcmp(parm[1], "min_rate")) {
		if (kstrtoul(parm[2], 10, &val) < 0)
			goto error;
		idx = (unsigned int)val;
		if (idx > 3) {
			pr_info("min_rate index error\n");
			goto error;
		}
		if (kstrtoul(parm[3], 10, &val) < 0)
			goto error;
		p->min_rate[idx] = (unsigned int)val;
		pr_hdr10p_adap_dbg("min_rate[%d] = %d\n", idx, (unsigned int)val);
	} else if (!strcmp(parm[1], "max_rate")) {
		if (kstrtoul(parm[2], 10, &val) < 0)
			goto error;
		idx = (unsigned int)val;
		if (idx > 3) {
			pr_info("max_rate index error\n");
			goto error;
		}
		if (kstrtoul(parm[3], 10, &val) < 0)
			goto error;
		p->max_rate[idx] = (unsigned int)val;
		pr_hdr10p_adap_dbg("max_rate[%d] = %d\n", idx, (unsigned int)val);
	} else if (!strcmp(parm[1], "read_param")) {
		hdr10p_adp_par_show(p);
	}

	return 0;

error:
	return -1;
}
#endif
