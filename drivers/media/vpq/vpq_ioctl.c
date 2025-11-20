// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/io.h>
#include <linux/amlogic/media/vpq/vpq_cmd.h>
#include "vpq_ioctl.h"
#include "vpq_printk.h"
#include "vpq_table_type.h"
#include "vpq_table_logic.h"
#include "vpq_processor.h"

typedef int (*vpq_ioctl_func)(struct file *file, unsigned long arg);
struct vpq_ioctl_func_s {
	int cmd;
	vpq_ioctl_func ioctl_func;
};

int vpq_ioctl_set_table_version_info(struct file *file, unsigned long arg)
{
	int ret = 0;
	struct vpq_table_ver_info_s ver_info = {0};

	if (copy_from_user(&ver_info,
			(void __user *)arg, sizeof(struct vpq_table_ver_info_s))) {
		pr_error("copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_pri("project_ver:%s, chip_ver:%s\n",
			ver_info.project_ver, ver_info.chip_ver);

		ret = vpq_set_pq_table_version(&ver_info);
	}

	return ret;
}

int vpq_ioctl_set_table_param(struct file *file, unsigned long arg)
{
	int ret = 0;
	void __user *argp;
	unsigned char *buf;
	struct vpq_table_bin_param_s bin_param = {0};

	if (copy_from_user(&bin_param,
			(void __user *)arg, sizeof(struct vpq_table_bin_param_s))) {
		pr_error("copy_from_user fail\n");
		return -EFAULT;
	}

	if (bin_param.len != sizeof(struct PQ_TABLE_PARAM)) {
		pr_error("hal bin_param.len 0x%x not same driver struct size 0x%zx\n",
			bin_param.len, sizeof(struct PQ_TABLE_PARAM));
		return -EFAULT;
	}
	//pr_pri("hal struct size same with driver struct, index:0x%x, len:0x%x\n",
	//	bin_param.index, bin_param.len);

	argp = (void __user *)bin_param.ptr;
	buf = vmalloc(bin_param.len);
	if (!buf) {
		pr_error("vmalloc bin_param buf for receive PQ_TABLE_PARAM fail\n");
		return -ENOMEM;
	}

	if (copy_from_user((void *)buf, argp, bin_param.len)) {
		pr_error("cp bin_param to buf fail\n");
		ret = -EFAULT;
	} else {
		bin_param.ptr = buf;
		ret = vpq_set_default_pq_table(&bin_param);
	}
	vfree(buf);

	return ret;
}

int vpq_ioctl_set_nonstandard_timing_map(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char i = 0;
	unsigned int buf_size = 0;
	struct vpq_nonstandard_timing_req_s req;
	struct vpq_nonstandard_timing_map_s *pdata = NULL;

	if (copy_from_user(&req,
			(void __user *)arg, sizeof(struct vpq_nonstandard_timing_req_s))) {
		pr_error("copy_from_user fail\n");
		return -EFAULT;
	}

	buf_size = req.map_count * sizeof(struct vpq_nonstandard_timing_map_s);
	pdata = kmalloc(buf_size, GFP_KERNEL);
	if (!pdata) {
		pr_error("vmalloc pdata buf fail\n");
		return -ENOMEM;
	}

	if (copy_from_user(pdata, (void __user *)arg + sizeof(req), buf_size)) {
		pr_error("copy_from_user pdata fail\n");
		ret = -EFAULT;
	} else {
		pr_pri("req.map_count:%d\n", req.map_count);
		for (i = 0; i < req.map_count; i++) {
			pr_pri("non-standard timing map[%d]:%d,%d,%s,%s,%d\n",
				i, pdata[i].width, pdata[i].height, pdata[i].hdr_string,
				pdata[i].src_string, pdata[i].timing_index);
		}

		ret = vpq_set_nonstandard_timing_map(req.map_count, pdata);
	}
	kfree(pdata);

	return ret;
}

int vpq_ioctl_set_pq_module_cfg(struct file *file, unsigned long arg)
{
	int ret = 0;
	struct vpq_pq_module_cfg_s cfg = {0};

	if (copy_from_user(&cfg, (void __user *)arg, sizeof(struct vpq_pq_module_cfg_s))) {
		pr_error("copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_pri("cfg:\n"
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n"
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n"
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
			cfg.pq_en, cfg.vadj1_en, cfg.vd1_ctrst_en, cfg.vadj2_en,
			cfg.post_ctrst_en, cfg.pregamma_en, cfg.gamma_en, cfg.wb_en,
			cfg.dnlp_en, cfg.lc_en, cfg.black_ext_en, cfg.blue_stretch_en,
			cfg.chroma_cor_en, cfg.sharpness0_en, cfg.sharpness1_en, cfg.cm_en,
			cfg.lut3d_en, cfg.dejaggy_sr0_en, cfg.dejaggy_sr1_en, cfg.dering_sr0_en,
			cfg.dering_sr1_en, cfg.di_en, cfg.mcdi_en, cfg.deblock_en,
			cfg.demosquito_en, cfg.smoothplus_en, cfg.nr_en, cfg.hdrtmo_en,
			cfg.ai_en, cfg.aisr_en);

		ret = vpq_set_pq_module_cfg(&cfg);
	}

	return ret;
}

int vpq_ioctl_set_pq_module_status(struct file *file, unsigned long arg)
{
	int ret = 0;
	struct vpq_pq_module_status_s status = {0};

	if (copy_from_user(&status,
			(void __user *)arg, sizeof(struct vpq_pq_module_status_s))) {
		pr_error("copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "module:%d status:%d\n", status.module, status.status);
		ret = vpq_set_pq_module_status(status.module, status.status);
	}

	return ret;
}

int vpq_ioctl_set_brightness(struct file *file, unsigned long arg)
{
	int ret = 0;
	int value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(int))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_brightness(value);
	}

	return ret;
}

int vpq_ioctl_set_contrast(struct file *file, unsigned long arg)
{
	int ret = 0;
	int value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(int))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_contrast(value);
	}

	return ret;
}

int vpq_ioctl_set_saturation(struct file *file, unsigned long arg)
{
	int ret = 0;
	int value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(int))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_saturation(value);
	}

	return ret;
}

int vpq_ioctl_set_hue(struct file *file, unsigned long arg)
{
	int ret = 0;
	int value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(int))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_hue(value);
	}

	return ret;
}

int vpq_ioctl_set_sharpness(struct file *file, unsigned long arg)
{
	int ret = 0;
	int value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(int))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_sharpness(value);
	}

	return ret;
}

int vpq_ioctl_set_brightness_post(struct file *file, unsigned long arg)
{
	int ret = 0;
	int value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(int))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_brightness_post(value);
	}

	return ret;
}

int vpq_ioctl_set_contrast_post(struct file *file, unsigned long arg)
{
	int ret = 0;
	int value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(int))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_contrast_post(value);
	}

	return ret;
}

int vpq_ioctl_set_saturation_post(struct file *file, unsigned long arg)
{
	int ret = 0;
	int value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(int))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_saturation_post(value);
	}

	return ret;
}

int vpq_ioctl_set_hue_post(struct file *file, unsigned long arg)
{
	int ret = 0;
	int value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(int))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_hue_post(value);
	}

	return ret;
}

int vpq_ioctl_set_overscan_table(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned int buf_size = 0;
	struct vpq_overscan_table_s os_table;
	struct vpq_overscan_data_s *pdata = NULL;

	if (copy_from_user(&os_table, (void __user *)arg,
			sizeof(struct vpq_overscan_table_s))) {
		pr_inf(lev_ioc, "copy_from_user os_table fail\n");
		return -EFAULT;
	}

	if (os_table.length > VPQ_TIMING_MAX ||
		os_table.length <= 0) {
		pr_inf(lev_ioc, "length check fail\n");
		return -EFAULT;
	}

	buf_size = os_table.length * sizeof(struct vpq_overscan_data_s);
	pdata = kmalloc(buf_size, GFP_KERNEL);
	if (!pdata) {
		pr_inf(lev_ioc, "vmalloc pdata buf fail\n");
		return -ENOMEM;
	}

	if (copy_from_user(pdata, (void __user *)os_table.param_ptr, buf_size)) {
		pr_inf(lev_ioc, "copy_from_user pdata fail\n");
		ret = -EFAULT;
	} else {
		ret = vpq_set_overscan_data(os_table.length, pdata);
	}
	kfree(pdata);

	return ret;
}

int vpq_ioctl_set_gamma_table(struct file *file, unsigned long arg)
{
	int ret = 0;
	int i = 0;
	int gma_point = 0;
	unsigned int buf_size = 0;
	struct vpq_gamma_vari_table_s g_tab_tmp;
	struct vpq_gamma_table_s g_table;

	if (copy_from_user(&g_tab_tmp,
			(void __user *)arg, sizeof(struct vpq_gamma_vari_table_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		gma_point = vpq_get_gamma_table_point();
		if (g_tab_tmp.number != gma_point) {
			pr_inf(lev_ioc, "get %d points, driver require %d\n",
				g_tab_tmp.number, gma_point);
		}

		buf_size = sizeof(unsigned int) * gma_point;
		g_table.r_data = kmalloc(buf_size, GFP_KERNEL);
		g_table.g_data = kmalloc(buf_size, GFP_KERNEL);
		g_table.b_data = kmalloc(buf_size, GFP_KERNEL);

		if (!g_table.r_data || !g_table.g_data || !g_table.b_data) {
			pr_inf(lev_ioc, "vmalloc g_table buf fail\n");
			kfree(g_table.r_data);
			kfree(g_table.g_data);
			kfree(g_table.b_data);
			return -ENOMEM;
		}

		if (copy_from_user(g_table.r_data,
				(void __user *)g_tab_tmp.r_data, buf_size)) {
			pr_inf(lev_ioc, "copy_from_user r_data fail\n");
			ret = -EFAULT;
		}

		if (copy_from_user(g_table.g_data,
				(void __user *)g_tab_tmp.g_data, buf_size)) {
			pr_inf(lev_ioc, "copy_from_user g_data fail\n");
			ret = -EFAULT;
		}

		if (copy_from_user(g_table.b_data,
				(void __user *)g_tab_tmp.b_data, buf_size)) {
			pr_inf(lev_ioc, "copy_from_user b_data fail\n");
			ret = -EFAULT;
		}

		for (i = 0; i < gma_point; i++)
			pr_inf(lev_ioc, "r/g/b[%d]:%d %d %d\n",
				i, g_table.r_data[i], g_table.g_data[i], g_table.b_data[i]);

		if (ret != -EFAULT) {
			pr_inf(lev_ioc, "success\n");
			ret = vpq_set_gamma_table(&g_table);
		}

		kfree(g_table.r_data);
		kfree(g_table.g_data);
		kfree(g_table.b_data);
	}

	return ret;
}

int vpq_ioctl_set_rgb_ogo(struct file *file, unsigned long arg)
{
	int ret = 0;
	struct vpq_rgb_ogo_s rgb_ogo = {0};

	if (copy_from_user(&rgb_ogo, (void __user *)arg, sizeof(struct vpq_rgb_ogo_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "rgbogo:%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
			rgb_ogo.r_pre_offset, rgb_ogo.g_pre_offset, rgb_ogo.b_pre_offset,
			rgb_ogo.r_gain, rgb_ogo.g_gain, rgb_ogo.b_gain,
			rgb_ogo.r_post_offset, rgb_ogo.g_post_offset, rgb_ogo.b_post_offset);

		ret = vpq_set_rgb_ogo(&rgb_ogo);
	}

	return ret;
}

int vpq_ioctl_set_matrix_param(struct file *file, unsigned long arg)
{
	int ret = 0;
	int i = 0;
	struct vpq_mtrx_info_s matrix_info = {0};

	if (copy_from_user(&matrix_info, (void __user *)arg, sizeof(struct vpq_mtrx_info_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "matrix_info:%d\n", matrix_info.mtrx_sel);
		for (i = 0; i < VPQ_MTRX_OFFSET_LEN; i++) {
			pr_inf(lev_ioc, "pre_offset/post_offset[%d]:%d, %d\n", i,
				matrix_info.mtrx_param.pre_offset[i],
				matrix_info.mtrx_param.post_offset[i]);
		}
		pr_inf(lev_ioc, "right_shift:%d\n", matrix_info.mtrx_param.right_shift);

		ret = vpq_set_matrix_param(&matrix_info);
	}

	return ret;
}

int vpq_ioctl_set_color_base(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_color_base(value);
	}

	return ret;
}

int vpq_ioctl_set_color_customize(struct file *file, unsigned long arg)
{
	int ret = 0;
	struct vpq_cms_s cms_param = {0};

	if (copy_from_user(&cms_param, (void __user *)arg, sizeof(struct vpq_cms_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "cms_param:%d,%d,%d,%d,%d\n",
			cms_param.color_type, cms_param.color_9, cms_param.color_14,
			cms_param.cms_type, cms_param.value);

		ret = vpq_set_color_customize(&cms_param);
	}

	return ret;
}

int vpq_ioctl_set_black_stretch(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_black_stretch(value);
	}

	return ret;
}

int vpq_ioctl_set_dnlp_mode(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_dnlp_mode(value);
	}

	return ret;
}

int vpq_ioctl_set_lc_mode(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_lc_mode(value);
	}

	return ret;
}

int vpq_ioctl_set_csc_type(struct file *file, unsigned long arg)
{
	int ret = 0;
	int value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(enum vpq_csc_type_e))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_csc_type(value);
	}

	return ret;
}

int vpq_ioctl_set_3dlut_data(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned int buf_size = 0;
	int *pdata = NULL;
	struct vpq_lut3d_table_s lut_data;

	if (copy_from_user(&lut_data,
			(void __user *)arg, sizeof(struct vpq_lut3d_table_s))) {
		pr_inf(lev_ioc, "copy_from_user lut_data fail\n");
		return -EFAULT;
	}

	if (lut_data.data_size == 0) {
		pr_inf(lev_ioc, "data_size check fail\n");
		return -EFAULT;
	}

	buf_size = 17 * 17 * 17 * 3 * sizeof(int);
	pdata = kmalloc(buf_size, GFP_KERNEL);
	if (!pdata) {
		pr_inf(lev_ioc, "vmalloc pdata buf fail\n");
		return -ENOMEM;
	}

	if (copy_from_user(pdata, (void __user *)lut_data.data, buf_size)) {
		pr_inf(lev_ioc, "copy_from_user pdata fail\n");
		ret = -EFAULT;
	} else {
		lut_data.data = pdata;
		ret = vpq_set_3dlut_data(&lut_data);
	}
	kfree(pdata);

	return ret;
}

int vpq_ioctl_set_hdr_tmo_mode(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_hdr_tmo_mode(value);
	}

	return ret;
}

int vpq_ioctl_set_hdr_tmo(struct file *file, unsigned long arg)
{
	int ret = 0;
	int i = 0;
	unsigned int buf_size = 0;
	int *ptmp_data = NULL;
	struct vpq_hdr_lut_s hdr_lut = {0};

	if (copy_from_user(&hdr_lut, (void __user *)arg,
		sizeof(struct vpq_hdr_lut_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		return -EFAULT;
	}

	if (hdr_lut.lut_size != 149)
		return -EFAULT;

	buf_size = hdr_lut.lut_size * sizeof(int);
	ptmp_data = kmalloc(buf_size, GFP_KERNEL);
	if (!ptmp_data) {
		pr_inf(lev_ioc, "vmalloc ptmp_data buf fail\n");
		return -ENOMEM;
	}

	if (copy_from_user(ptmp_data, (void __user *)arg, buf_size)) {
		pr_inf(lev_ioc, "copy_from_user fail 2\n");
		ret = -EFAULT;
	} else {
		hdr_lut.lut_data = ptmp_data;
		for (i = 0; i < hdr_lut.lut_size; i++)
			pr_inf(lev_ioc, "lut_data[%d]:%d\n", i, hdr_lut.lut_data[i]);
		ret = vpq_set_hdr_tmo(&hdr_lut);
	}
	kfree(ptmp_data);

	return ret;
}

int vpq_ioctl_set_hdr_oetf(struct file *file, unsigned long arg)
{
	int ret = 0;
	int i = 0;
	unsigned int buf_size = 0;
	int *ptmp_data = NULL;
	struct vpq_hdr_lut_s hdr_lut = {0};

	if (copy_from_user(&hdr_lut, (void __user *)arg,
		sizeof(struct vpq_hdr_lut_s))) {
		pr_inf(lev_ioc, "copy_from_user hdr_lut fail\n");
		return -EFAULT;
	}

	if (hdr_lut.lut_size != 149)
		return -EFAULT;

	buf_size = hdr_lut.lut_size * sizeof(int);
	ptmp_data = kmalloc(buf_size, GFP_KERNEL);
	if (!ptmp_data) {
		pr_inf(lev_ioc, "vmalloc ptmp_data buf fail\n");
		return -ENOMEM;
	}

	if (copy_from_user(ptmp_data, (void __user *)arg, buf_size)) {
		pr_inf(lev_ioc, "copy_from_user ptmp_data fail\n");
		ret = -EFAULT;
	} else {
		hdr_lut.lut_data = ptmp_data;
		for (i = 0; i < hdr_lut.lut_size; i++)
			pr_inf(lev_ioc, "lut_data[%d]:%d\n", i, hdr_lut.lut_data[i]);
		ret = vpq_set_hdr_oetf(&hdr_lut);
	}
	kfree(ptmp_data);

	return ret;
}

int vpq_ioctl_set_hdr_eotf(struct file *file, unsigned long arg)
{
	int ret = 0;
	int i = 0;
	unsigned int buf_size = 0;
	int *ptmp_data = NULL;
	struct vpq_hdr_lut_s hdr_lut = {0};

	if (copy_from_user(&hdr_lut, (void __user *)arg,
		sizeof(struct vpq_hdr_lut_s))) {
		pr_inf(lev_ioc, "copy_from_user hdr_lut fail\n");
		return -EFAULT;
	}

	if (hdr_lut.lut_size != 143)
		return -EFAULT;

	buf_size = hdr_lut.lut_size * sizeof(int);
	ptmp_data = kmalloc(buf_size, GFP_KERNEL);
	if (!ptmp_data) {
		pr_inf(lev_ioc, "vmalloc ptmp_data buf fail\n");
		return -ENOMEM;
	}

	if (copy_from_user(ptmp_data, (void __user *)arg, buf_size)) {
		pr_inf(lev_ioc, "copy_from_user ptmp_data fail\n");
		ret = -EFAULT;
	} else {
		hdr_lut.lut_data = ptmp_data;
		for (i = 0; i < hdr_lut.lut_size; i++)
			pr_inf(lev_ioc, "lut_data[%d]:%d\n", i, hdr_lut.lut_data[i]);
		ret = vpq_set_hdr_eotf(&hdr_lut);
	}
	kfree(ptmp_data);

	return ret;
}

int vpq_ioctl_set_hdr_cgain(struct file *file, unsigned long arg)
{
	int ret = 0;
	int i = 0;
	unsigned int buf_size = 0;
	int *ptmp_data = NULL;
	struct vpq_hdr_lut_s hdr_lut = {0};

	if (copy_from_user(&hdr_lut, (void __user *)arg,
		sizeof(struct vpq_hdr_lut_s))) {
		pr_inf(lev_ioc, "copy_from_user hdr_lut fail\n");
		return -EFAULT;
	}

	if (hdr_lut.lut_size != 65)
		return -EFAULT;

	buf_size = hdr_lut.lut_size * sizeof(int);
	ptmp_data = kmalloc(buf_size, GFP_KERNEL);
	if (!ptmp_data) {
		pr_inf(lev_ioc, "vmalloc ptmp_data buf fail\n");
		return -ENOMEM;
	}

	if (copy_from_user(ptmp_data, (void __user *)arg, buf_size)) {
		pr_inf(lev_ioc, "copy_from_user ptmp_data fail\n");
		ret = -EFAULT;
	} else {
		hdr_lut.lut_data = ptmp_data;
		for (i = 0; i < hdr_lut.lut_size; i++)
			pr_inf(lev_ioc, "lut_data[%d]:%d\n", i, hdr_lut.lut_data[i]);
		ret = vpq_set_hdr_cgain(&hdr_lut);
	}
	kfree(ptmp_data);

	return ret;
}

int vpq_ioctl_set_aipq_mode(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_aipq_mode(value);
	}

	return ret;
}

int vpq_ioctl_set_aisr_mode(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_aisr_mode(value);
	}

	return ret;
}

int vpq_ioctl_set_blue_stretch(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_blue_stretch(value);
	}

	return ret;
}

int vpq_ioctl_set_chroma_coring(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_chroma_coring(value);
	}

	return ret;
}

int vpq_ioctl_set_eye_protect(struct file *file, unsigned long arg)
{
	int ret = 0;
	int i = 0;
	struct vpq_eye_protect_s protect_param = {0};

	if (copy_from_user(&protect_param, (void __user *)arg, sizeof(struct vpq_eye_protect_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "enable:%d\n", protect_param.enable);
		for (i = 0; i < VPQ_MODE_RGB_MAX; i++)
			pr_inf(lev_ioc, "rgb[%d]:%d\n", i, protect_param.rgb[i]);
		ret = vpq_set_eys_protect(&protect_param);
	}

	return ret;
}

int vpq_ioctl_set_cabc(struct file *file, unsigned long arg)
{
	int ret = 0;

	ret = vpq_set_cabc();

	return ret;
}

int vpq_ioctl_set_add(struct file *file, unsigned long arg)
{
	int ret = 0;

	ret = vpq_set_add();

	return ret;
}

int vpq_ioctl_set_pc_mode(struct file *file, unsigned long arg)
{
	int ret = 0;
	int value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(enum vpq_pc_mode_e))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_pc_mode(value);
	}

	return ret;
}

int vpq_ioctl_set_color_primary_status(struct file *file, unsigned long arg)
{
	int ret = 0;
	int value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(int))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_color_primary_status(value);
	}

	return ret;
}

int vpq_ioctl_set_color_primary(struct file *file, unsigned long arg)
{
	int ret = 0;
	int i = 0;
	unsigned int buf_size = 0;
	struct vpq_color_primary_s *pdata = NULL;

	buf_size =  sizeof(struct vpq_color_primary_s);
	pdata = kmalloc(buf_size, GFP_KERNEL);
	if (!pdata) {
		pr_inf(lev_ioc, "vmalloc pdata buf fail\n");
		return -ENOMEM;
	}

	if (copy_from_user(pdata, (void __user *)arg, buf_size)) {
		pr_inf(lev_ioc, "copy_from_user ptmp_data fail\n");
		ret = -EFAULT;
	} else {
		for (i = 0; i < VPQ_COLOR_PRIMARY_LEN; i++)
			pr_inf(lev_ioc, "data_src/data_dest[%d]:%d, %d\n", i,
				pdata->data_src[i], pdata->data_dest[i]);

		ret = vpq_set_color_primary(pdata);
	}
	kfree(pdata);

	return ret;
}

int vpq_ioctl_set_frame_status(struct file *file, unsigned long arg)
{
	int ret = 0;
	int value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(enum vpq_frame_status_e))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);

		#ifdef RELOAD_PQ_FOR_SAME_TIMING
			ret = vpq_set_frame_status(value);
			ret |= vpq_processor_set_frame_status(value);
		#endif
	}

	return ret;
}

int vpq_ioctl_set_frame(struct file *file, unsigned long arg)
{
	int ret = 0;
	struct vpq_frame_info_s frame_info = {0};

	if (copy_from_user(&frame_info, (void __user *)arg, sizeof(struct vpq_frame_info_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		//pr_inf(lev_ioc, "shared_fd:%d\n", frame_info.shared_fd);
		ret = vpq_process_set_frame(file->private_data, &frame_info);
	}

	return ret;
}

int vpq_ioctl_set_nr(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_nr(value);
	}

	return ret;
}

int vpq_ioctl_set_deblock(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_deblock(value);
	}

	return ret;
}

int vpq_ioctl_set_demosquito(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_demosquito(value);
	}

	return ret;
}

int vpq_ioctl_set_smoothplus_mode(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_smoothplus_mode(value);
	}

	return ret;
}

int vpq_ioctl_set_amdv_pic_mode_id(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_amdv_pic_mode_id(value);
	}

	return ret;
}

int vpq_ioctl_set_amdv_dark_detail(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_amdv_dark_detail(value);
	}

	return ret;
}

int vpq_ioctl_set_amdv_light_sensor(struct file *file, unsigned long arg)
{
	int ret = 0;
	struct vpq_light_sensor_s light_sensor = {0};

	if (copy_from_user(&light_sensor, (void __user *)arg, sizeof(struct vpq_light_sensor_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "flag:%d, t_frontLux:%d, t_rearLum:%d\n",
			light_sensor.flag, light_sensor.t_frontLux, light_sensor.t_rearLum);
		ret = vpq_set_amdv_light_sensor(&light_sensor);
	}

	return ret;
}

int vpq_ioctl_set_amdv_precision_detail(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_amdv_precision_detail(value);
	}

	return ret;
}

int vpq_ioctl_set_memc_on_off(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_memc_on_off(value);
	}

	return ret;
}

int vpq_ioctl_set_memc_deblur_level(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_memc_deblur_level(value);
	}

	return ret;
}

int vpq_ioctl_set_memc_dejudder_level(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_memc_dejudder_level(value);
	}

	return ret;
}

int vpq_ioctl_set_memc_demo_mode(struct file *file, unsigned long arg)
{
	int ret = 0;
	unsigned char value = 0;

	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned char))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		pr_inf(lev_ioc, "value:%d\n", value);
		ret = vpq_set_memc_demo_mode(value);
	}

	return ret;
}

int vpq_ioctl_set_vdin_cvd(struct file *file, unsigned long arg)
{
	int ret = 0;

	ret = vpq_set_vdin_cvd();

	return ret;
}

//GET
int vpq_ioctl_get_chip_type(struct file *file, unsigned long arg)
{
	enum vpq_chip_type_e chip_type = (enum vpq_chip_type_e)vpq_get_chip_type();

	pr_inf(lev_ioc, "chip_type:%d\n", chip_type);

	if (copy_to_user((void __user *)arg, &chip_type, sizeof(enum vpq_chip_type_e))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		return -EFAULT;
	}

	return 0;
}

int vpq_ioctl_get_chip_id(struct file *file, unsigned long arg)
{
	enum vpq_chip_id_e chip_id = (enum vpq_chip_id_e)vpq_get_chip_id();

	pr_inf(lev_ioc, "chip_id:%d\n", chip_id);

	if (copy_to_user((void __user *)arg, &chip_id, sizeof(enum vpq_chip_id_e))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		return -EFAULT;
	}

	return 0;
}

int vpq_ioctl_get_pc_mode(struct file *file, unsigned long arg)
{
	enum vpq_pc_mode_e pc_mode = (enum vpq_pc_mode_e)vpq_get_pc_mode();

	pr_inf(lev_ioc, "pc_mode:%d\n", pc_mode);

	if (copy_to_user((void __user *)arg, &pc_mode, sizeof(enum vpq_pc_mode_e))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		return -EFAULT;
	}

	return 0;
}

int vpq_ioctl_get_hist_avg(struct file *file, unsigned long arg)
{
	int ret = 0;
	struct vpq_hist_ave_s hist_ave = {0};

	ret = vpq_get_hist_avg(&hist_ave);
	//pr_inf(lev_ioc, "hist_ave:%d,%d,%d,%d\n",
	//	hist_ave.sum, hist_ave.width, hist_ave.height, hist_ave.ave);

	if (copy_to_user((void __user *)arg, &hist_ave, sizeof(struct vpq_hist_ave_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	}

	return ret;
}

int vpq_ioctl_get_histogram(struct file *file, unsigned long arg)
{
	int ret = 0;
	int buf_size = 0;
	int i = 0;
	struct vpq_hist_param_s *phist = NULL;

	buf_size = sizeof(struct vpq_hist_param_s);
	phist = kmalloc(buf_size, GFP_KERNEL);
	if (!phist) {
		pr_inf(lev_ioc, "vmalloc buf failed\n");
		return -ENOMEM;
	}

	ret = vpq_get_histogram(phist);
	pr_inf(lev_ioc, "hist_param:%d,%d,%d\n",
		phist->hist_pow, phist->luma_sum, phist->pixel_sum);
	for (i = 0; i < VPQ_HIST_BIN_COUNT; i++) {
		pr_inf(lev_ioc, "hist[%d]:%d\n", i, phist->hist[i]);
		pr_inf(lev_ioc, "dark_hist[%d]:%d\n", i, phist->dark_hist[i]);
	}
	i = 0;
	for (i = 0; i < VPQ_COLOR_HIST_BIN_COUNT; i++) {
		pr_inf(lev_ioc, "hue_hist[%d]:%d\n", i, phist->hue_hist[i]);
		pr_inf(lev_ioc, "sat_hist[%d]:%d\n", i, phist->sat_hist[i]);
	}

	if (copy_to_user((void __user *)arg, phist, buf_size)) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	}
	kfree(phist);

	return ret;
}

int vpq_ioctl_get_hdr_histogram(struct file *file, unsigned long arg)
{
	int ret = 0;
	int buf_size = 0;
	int i = 0;
	struct vpq_hdr_hist_param_s *phist = NULL;

	buf_size = sizeof(struct vpq_hdr_hist_param_s);
	phist = kmalloc(buf_size, GFP_KERNEL);
	if (!phist) {
		pr_inf(lev_ioc, "vmalloc buf failed\n");
		return -ENOMEM;
	}

	ret = vpq_get_hdr_histogram(phist);
	for (i = 0; i < VPQ_HDR_HIST_BIN_COUNT; i++)
		pr_inf(lev_ioc, "data_rgb_max[%d]:%d\n", i, phist->data_rgb_max[i]);

	if (copy_to_user((void __user *)arg, phist, buf_size)) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	}
	kfree(phist);

	return ret;
}

int vpq_ioctl_get_csc_type(struct file *file, unsigned long arg)
{
	int ret = 0;
	int value = 0;

	value = vpq_get_csc_type();
	pr_inf(lev_ioc, "value:%d\n", value);

	if (copy_to_user((void __user *)arg, &value, sizeof(enum vpq_csc_type_e))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	}

	return ret;
}

int vpq_ioctl_get_color_primary(struct file *file, unsigned long arg)
{
	int value = 0;

	value = (int)vpq_get_color_primary();
	pr_inf(lev_ioc, "value:%d\n", value);

	if (copy_to_user((void __user *)arg, &value, sizeof(int))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		return -EFAULT;
	}

	return 0;
}

int vpq_ioctl_get_hdr_metadata(struct file *file, unsigned long arg)
{
	int ret = 0;
	int i = 0, j = 0, z = 0;
	struct vpq_hdr_metadata_s metadata = {0};

	ret = vpq_get_hdr_metadata(&metadata);
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 2; j++)
			pr_inf(lev_ioc, "primaries[%d][%d]:%d\n", i, j, metadata.primaries[i][j]);
	}
	for (z = 0; z < 2; z++) {
		pr_inf(lev_ioc, "white_point[%d]:%d\n", i, metadata.white_point[z]);
		pr_inf(lev_ioc, "luminance[%d]:%d\n", i, metadata.luminance[z]);
	}

	if (copy_to_user((void __user *)arg, &metadata, sizeof(struct vpq_hdr_metadata_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	}

	return ret;
}

int vpq_ioctl_get_event_info(struct file *file, unsigned long arg)
{
	enum vpq_event_info_e event_info = VPQ_EVENT_NONE;

	event_info = (enum vpq_event_info_e)vpq_get_event_info();
	pr_inf(lev_ioc, "event_info:%d\n", event_info);

	if (copy_to_user((void __user *)arg, &event_info, sizeof(enum vpq_event_info_e))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		return -EFAULT;
	}

	return 0;
}

int vpq_ioctl_get_signal_info(struct file *file, unsigned long arg)
{
	struct vpq_signal_info_s sig_info = {0};

	vpq_get_signal_info(&sig_info);

	pr_inf(lev_ioc, "src_type:%d, hdmi_port:%d, sig_mode:%d, scan_mode:%d\n",
		sig_info.src_type, sig_info.hdmi_port, sig_info.sig_mode, sig_info.scan_mode);
	pr_inf(lev_ioc, "hdr_type:%d, is_amdv:%d, is_game:%d, is_pc:%d\n",
		sig_info.hdr_type, sig_info.is_amdv, sig_info.is_game, sig_info.is_pc);
	pr_inf(lev_ioc, "sig_fmt:0x%x, trans_fmt:%d, height:%d, width:%d, fps:%d\n",
		sig_info.sig_fmt, sig_info.trans_fmt, sig_info.height, sig_info.width,
		sig_info.fps);

	if (copy_to_user((void __user *)arg, &sig_info, sizeof(struct vpq_signal_info_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		return -EFAULT;
	}

	return 0;
}

int vpq_ioctl_get_dv_cfg_support(struct file *file, unsigned long arg)
{
	struct vpq_dv_cfg_support_s cfg_support = {0};

	if (copy_from_user(&cfg_support, (void __user *)arg, sizeof(struct vpq_dv_cfg_support_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		return -EFAULT;
	}

	pr_inf(lev_ioc, "pic_mode_id:%d\n", cfg_support.pic_mode_id);
	cfg_support = vpq_get_dv_cfg_support(cfg_support.pic_mode_id);

	pr_inf(lev_ioc, "precision_detail:%d, dark_detail:%d, light_sense:%d\n",
		cfg_support.precision_detail, cfg_support.dark_detail, cfg_support.light_sense);

	if (copy_to_user((void __user *)arg, &cfg_support, sizeof(struct vpq_dv_cfg_support_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		return -EFAULT;
	}

	return 0;
}

/* DPSS Start */
int vpq_ioctl_set_nr_dpss(struct file *file, unsigned long arg)
{
	int ret = 0;
	int i = 0;
	struct vpq_dnr_param_s dnr_param = {0};

	if (copy_from_user(&dnr_param, (void __user *)arg, sizeof(struct vpq_dnr_param_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		for (i = 0; i < DNR_MAX; i++)
			pr_inf(lev_ioc, "param[%d]:%d\n", i, dnr_param.param[i]);
		ret = vpq_set_nr_dpss(&dnr_param);
	}

	return ret;
}

int vpq_ioctl_set_deblock_dpss(struct file *file, unsigned long arg)
{
	int ret = 0;
	int i = 0;
	struct vpq_dblk_param_s dblk_param = {0};

	if (copy_from_user(&dblk_param, (void __user *)arg, sizeof(struct vpq_dblk_param_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		for (i = 0; i < DBLK_MAX; i++)
			pr_inf(lev_ioc, "param[%d]:%d\n", i, dblk_param.param[i]);
		ret = vpq_set_deblock_dpss(&dblk_param);
	}

	return ret;
}

int vpq_ioctl_set_demosquito_dpss(struct file *file, unsigned long arg)
{
	int ret = 0;
	int i = 0;
	struct vpq_demosquito_param_s demo_param = {0};

	if (copy_from_user(&demo_param, (void __user *)arg,
			sizeof(struct vpq_demosquito_param_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		for (i = 0; i < DM_MAX; i++)
			pr_inf(lev_ioc, "param[%d]:%d\n", i, demo_param.param[i]);
		ret = vpq_set_demosquito_dpss(&demo_param);
	}

	return ret;
}

int vpq_ioctl_set_smoothplus_dpss(struct file *file, unsigned long arg)
{
	int ret = 0;
	int i = 0;
	struct vpq_dct_param_s dct_param = {0};

	if (copy_from_user(&dct_param, (void __user *)arg, sizeof(struct vpq_dct_param_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		for (i = 0; i < DCT_MAX; i++)
			pr_inf(lev_ioc, "param[%d]:%d\n", i, dct_param.param[i]);
		ret = vpq_set_smoothplus_dpss(&dct_param);
	}

	return ret;
}

int vpq_ioctl_set_xlr_dpss(struct file *file, unsigned long arg)
{
	int ret = 0;
	int i = 0;
	struct vpq_xlr_param_s xlr_param = {0};

	if (copy_from_user(&xlr_param, (void __user *)arg, sizeof(struct vpq_xlr_param_s))) {
		pr_inf(lev_ioc, "copy_from_user fail\n");
		ret = -EFAULT;
	} else {
		for (i = 0; i < XLR_MAX; i++)
			pr_inf(lev_ioc, "param[%d]:%d\n", i, xlr_param.param[i]);
		ret = vpq_set_xlr_dpss(&xlr_param);
	}

	return ret;
}

/* DPSS End */

static struct vpq_ioctl_func_s st_ioctl_info[] = {
	/* SET CMD */
	{VPQ_IOC_SET_TABLE_VERSION_INFO,     vpq_ioctl_set_table_version_info},
	{VPQ_IOC_SET_TABLE_PARAM,            vpq_ioctl_set_table_param},
	{VPQ_IOC_SET_NONSTANDARD_TIMING_MAP, vpq_ioctl_set_nonstandard_timing_map},
	{VPQ_IOC_SET_PQ_MODULE_CFG,          vpq_ioctl_set_pq_module_cfg},
	{VPQ_IOC_SET_PQ_MODULE_STATUS,       vpq_ioctl_set_pq_module_status},
	{VPQ_IOC_SET_BRIGHTNESS,             vpq_ioctl_set_brightness},
	{VPQ_IOC_SET_CONTRAST,               vpq_ioctl_set_contrast},
	{VPQ_IOC_SET_SATURATION,             vpq_ioctl_set_saturation},
	{VPQ_IOC_SET_HUE,                    vpq_ioctl_set_hue},
	{VPQ_IOC_SET_SHARPNESS,              vpq_ioctl_set_sharpness},
	{VPQ_IOC_SET_BRIGHTNESS_POST,        vpq_ioctl_set_brightness_post},
	{VPQ_IOC_SET_CONTRAST_POST,          vpq_ioctl_set_contrast_post},
	{VPQ_IOC_SET_SATURATION_POST,        vpq_ioctl_set_saturation_post},
	{VPQ_IOC_SET_HUE_POST,               vpq_ioctl_set_hue_post},
	{VPQ_IOC_SET_OVERSCAN_TABLE,         vpq_ioctl_set_overscan_table},
	{VPQ_IOC_SET_GAMMA_TABLE,            vpq_ioctl_set_gamma_table},
	{VPQ_IOC_SET_RGB_OGO,                vpq_ioctl_set_rgb_ogo},
	{VPQ_IOC_SET_MATRIX_PARAM,           vpq_ioctl_set_matrix_param},
	{VPQ_IOC_SET_COLOR_BASE,             vpq_ioctl_set_color_base},
	{VPQ_IOC_SET_COLOR_CUSTOMIZE,        vpq_ioctl_set_color_customize},
	{VPQ_IOC_SET_BLACK_STRETCH,          vpq_ioctl_set_black_stretch},
	{VPQ_IOC_SET_DNLP_MODE,              vpq_ioctl_set_dnlp_mode},
	{VPQ_IOC_SET_LC_MODE,                vpq_ioctl_set_lc_mode},
	{VPQ_IOC_SET_CSC_TYPE,               vpq_ioctl_set_csc_type},
	{VPQ_IOC_SET_3DLUT_DATA,             vpq_ioctl_set_3dlut_data},
	{VPQ_IOC_SET_HDR_TMO_MODE,           vpq_ioctl_set_hdr_tmo_mode},
	{VPQ_IOC_SET_HDR_TMO,                vpq_ioctl_set_hdr_tmo},
	{VPQ_IOC_SET_HDR_OETF,               vpq_ioctl_set_hdr_oetf},
	{VPQ_IOC_SET_HDR_EOTF,               vpq_ioctl_set_hdr_eotf},
	{VPQ_IOC_SET_HDR_CGAIN,              vpq_ioctl_set_hdr_cgain},
	{VPQ_IOC_SET_AIPQ_MODE,              vpq_ioctl_set_aipq_mode},
	{VPQ_IOC_SET_AISR_MODE,              vpq_ioctl_set_aisr_mode},
	{VPQ_IOC_SET_BLUE_STRETCH,           vpq_ioctl_set_blue_stretch},
	{VPQ_IOC_SET_CHROMA_CORING,          vpq_ioctl_set_chroma_coring},
	{VPQ_IOC_SET_EYE_PROTECT,            vpq_ioctl_set_eye_protect},
	{VPQ_IOC_SET_CABC,                   vpq_ioctl_set_cabc},
	{VPQ_IOC_SET_ADD,                    vpq_ioctl_set_add},
	{VPQ_IOC_SET_PC_MODE,                vpq_ioctl_set_pc_mode},
	{VPQ_IOC_SET_COLOR_PRIMARY_STATUS,   vpq_ioctl_set_color_primary_status},
	{VPQ_IOC_SET_COLOR_PRIMARY,          vpq_ioctl_set_color_primary},
	{VPQ_IOC_SET_FRAME_STATUS,           vpq_ioctl_set_frame_status},
	{VPQ_IOC_SET_FRAME,                  vpq_ioctl_set_frame},

	{VPQ_IOC_SET_NR,                     vpq_ioctl_set_nr},
	{VPQ_IOC_SET_DEBLOCK,                vpq_ioctl_set_deblock},
	{VPQ_IOC_SET_DEMOSQUITO,             vpq_ioctl_set_demosquito},
	{VPQ_IOC_SET_SMOOTHPLUS_MODE,        vpq_ioctl_set_smoothplus_mode},

	{VPQ_IOC_SET_AMDV_PIC_MODE_ID,       vpq_ioctl_set_amdv_pic_mode_id},
	{VPQ_IOC_SET_AMDV_DARK_DETAIL,       vpq_ioctl_set_amdv_dark_detail},
	{VPQ_IOC_SET_AMDV_LIGHT_SENSE,       vpq_ioctl_set_amdv_light_sensor},
	{VPQ_IOC_SET_AMDV_PRECISION_DETAIL,  vpq_ioctl_set_amdv_precision_detail},

	{VPQ_IOC_SET_MEMC_ON_OFF,            vpq_ioctl_set_memc_on_off},
	{VPQ_IOC_SET_MEMC_DEBLUR_LEVEL,      vpq_ioctl_set_memc_deblur_level},
	{VPQ_IOC_SET_MEMC_DEJUDDER_LEVEL,    vpq_ioctl_set_memc_dejudder_level},
	{VPQ_IOC_SET_MEMC_DMEO_MODE,         vpq_ioctl_set_memc_demo_mode},

	{VPQ_IOC_SET_VDIN_CVD,               vpq_ioctl_set_vdin_cvd},

	/* GET CMD */
	{VPQ_IOC_GET_CHIP_TYPE,              vpq_ioctl_get_chip_type},
	{VPQ_IOC_GET_CHIP_ID,                vpq_ioctl_get_chip_id},
	{VPQ_IOC_GET_PC_MODE,                vpq_ioctl_get_pc_mode},
	{VPQ_IOC_GET_HIST_AVG,               vpq_ioctl_get_hist_avg},
	{VPQ_IOC_GET_HIST_BIN,               vpq_ioctl_get_histogram},
	{VPQ_IOC_GET_HDR_HIST,               vpq_ioctl_get_hdr_histogram},
	{VPQ_IOC_GET_CSC_TYPE,               vpq_ioctl_get_csc_type},
	{VPQ_IOC_GET_COLOR_PRIM,             vpq_ioctl_get_color_primary},
	{VPQ_IOC_GET_HDR_METADATA,           vpq_ioctl_get_hdr_metadata},
	{VPQ_IOC_GET_EVENT_INFO,             vpq_ioctl_get_event_info},
	{VPQ_IOC_GET_SIGNAL_INFO,            vpq_ioctl_get_signal_info},
	{VPQ_IOC_GET_DV_CFG_SUPPORT,         vpq_ioctl_get_dv_cfg_support},

	/* DPSS Start */
	{VPQ_IOC_SET_NR_DPSS,                vpq_ioctl_set_nr_dpss},
	{VPQ_IOC_SET_DEBLOCK_DPSS,           vpq_ioctl_set_deblock_dpss},
	{VPQ_IOC_SET_DEMOSQUITO_DPSS,        vpq_ioctl_set_demosquito_dpss},
	{VPQ_IOC_SET_SMOOTHPLUS_DPSS,        vpq_ioctl_set_smoothplus_dpss},
	{VPQ_IOC_SET_XLR_DPSS,               vpq_ioctl_set_xlr_dpss}
	/* DPSS End */

};

int vpq_ioctl_process(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int index = 0;
	int max_index = 0;

	max_index = sizeof(st_ioctl_info) / sizeof(struct vpq_ioctl_func_s);
	for (index = 0; index < max_index; index++) {
		if (cmd == st_ioctl_info[index].cmd) {
			if (cmd != VPQ_IOC_GET_HIST_AVG &&
				cmd != VPQ_IOC_SET_FRAME) {
				pr_inf(lev_ioc, "cmd:0x%x\n", _IOC_NR(cmd));
			}

			ret = st_ioctl_info[index].ioctl_func(file, arg);
			break;
		}
	}

	return ret;
}
