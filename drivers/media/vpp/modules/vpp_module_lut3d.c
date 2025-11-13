// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#include "../vpp_common.h"
#include "vpp_module_lut3d.h"

struct _lut3d_bit_cfg_s {
	struct _bit_s bit_lut3d_en;
	struct _bit_s bit_lut3d_extend_en;
	struct _bit_s bit_lut3d_shadow;
	struct _bit_s bit_gated_clock_en;
};

struct _lut3d_reg_cfg_s {
	unsigned char page;
	unsigned char reg_lut3d_ctrl;
	unsigned char reg_lut3d_cbus2ram_ctrl;
	unsigned char reg_lut3d_ram_addr;
	unsigned char reg_lut3d_ram_data;
};

/*Default table from T3*/
static struct _lut3d_reg_cfg_s lut3d_reg_cfg = {
	0x39,
	0xd0,
	0xd1,
	0xd2,
	0xd3
};

static struct _lut3d_bit_cfg_s lut3d_bit_cfg = {
	{0, 1},
	{2, 1},
	{4, 3},
	{8, 2}
};

static unsigned int lut_support = 1;
static unsigned int lut_point = 17;
static unsigned int lut_bit_depth = 12;
static unsigned int lut_data_max = 4095;
static unsigned int lut_data_size;
static unsigned int lut_update_flag;

static bool pre_lut3d_en;
static bool cur_lut3d_en;
static bool cur_lut3d_pattern_en;

static unsigned int *lut3d_base_data;
static unsigned int *lut3d_pattern;

/*Internal functions*/
static void _lut3d_data_buffer_init(void)
{
	if (!lut_support)
		return;

	if (!lut3d_base_data)
		lut3d_base_data = vmalloc(lut_data_size * sizeof(int));
}

static void _lut3d_data_buffer_free(void)
{
	if (lut3d_base_data) {
		vfree(lut3d_base_data);
		lut3d_base_data = NULL;
	}
}

static void _lut3d_data_init(unsigned int *data,
	unsigned int point_num)
{
	int d0, d1, d2, step;
	unsigned int index;

	if (!lut_support || !data || !point_num)
		return;

	step = (lut_data_max + 1) / (point_num - 1);

	/*initialize lut3d as bypass*/
	for (d0 = 0; d0 < point_num; d0++) {
		for (d1 = 0; d1 < point_num; d1++) {
			for (d2 = 0; d2 < point_num; d2++) {
				index =
					d0 * point_num * point_num * 3 +
					d1 * point_num * 3 + d2 * 3;
				/* bypass data */
				data[index + 0] =
					MIN(lut_data_max, d0 * step);
				data[index + 1] =
					MIN(lut_data_max, d1 * step);
				data[index + 2] =
					MIN(lut_data_max, d2 * step);
			}
		}
	}
}

static void _set_lut3d_enable_ctrl(int val,
	unsigned char start, unsigned char len, enum io_mode_e io_mode)
{
	unsigned int addr = 0;

	addr = ADDR_PARAM(lut3d_reg_cfg.page, lut3d_reg_cfg.reg_lut3d_ctrl);
	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val, start, len);
}

static void _set_lut3d_data(int *data, enum io_mode_e io_mode)
{
	int i = 0;
	unsigned int addr = 0;
	unsigned int tmp = 0;
	unsigned int len = 0;

	if (!lut_support || !data)
		return;

	addr = ADDR_PARAM(lut3d_reg_cfg.page,
		lut3d_reg_cfg.reg_lut3d_ctrl);
	tmp = READ_VPP_REG_BY_MODE(io_mode, addr);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, tmp & 0xfffffffe);

	addr = ADDR_PARAM(lut3d_reg_cfg.page,
		lut3d_reg_cfg.reg_lut3d_cbus2ram_ctrl);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, 1);

	/*LUT3D_RAM_ADDR bit[20:16]=R, [12:8]=G, [4:0]=B*/
	addr = ADDR_PARAM(lut3d_reg_cfg.page,
		lut3d_reg_cfg.reg_lut3d_ram_addr);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, 0);

	addr = ADDR_PARAM(lut3d_reg_cfg.page,
		lut3d_reg_cfg.reg_lut3d_ram_data);

	len = lut_point * lut_point * lut_point;

	for (i = 0; i < len; i++) {/*write data, order RGB*/
		WRITE_VPP_REG_BY_MODE(io_mode, addr,
			((data[i * 3 + 1] & 0xfff) << 16) |
			(data[i * 3 + 2] & 0xfff));
		WRITE_VPP_REG_BY_MODE(io_mode, addr,
			(data[i * 3 + 0] & 0xfff));
	}

	addr = ADDR_PARAM(lut3d_reg_cfg.page,
		lut3d_reg_cfg.reg_lut3d_cbus2ram_ctrl);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, 0);

	addr = ADDR_PARAM(lut3d_reg_cfg.page,
		lut3d_reg_cfg.reg_lut3d_ctrl);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, tmp);
}

static void _dump_lut3d_reg_info(void)
{
	PR_DRV("lut3d_reg_cfg info:\n");
	PR_DRV("page = 0x%x\n", lut3d_reg_cfg.page);
	PR_DRV("reg_lut3d_ctrl = 0x%x\n",
		lut3d_reg_cfg.reg_lut3d_ctrl);
	PR_DRV("reg_lut3d_cbus2ram_ctrl = 0x%x\n",
		lut3d_reg_cfg.reg_lut3d_cbus2ram_ctrl);
	PR_DRV("reg_lut3d_ram_addr = 0x%x\n",
		lut3d_reg_cfg.reg_lut3d_ram_addr);
	PR_DRV("reg_lut3d_ram_data = 0x%x\n",
		lut3d_reg_cfg.reg_lut3d_ram_data);
}

static void _dump_lut3d_data_info(void)
{
	PR_DRV("No more data\n");
}

/*External functions*/
int vpp_module_lut3d_init(struct vpp_dev_s *dev)
{
	enum vpp_chip_type_e chip_id;

	chip_id = dev->m_data->chip_id;

	switch (chip_id) {
	case CHIP_T3:
	case CHIP_T5W:
	case CHIP_T5M:
		lut_bit_depth = 10;
		lut_data_max = 1023;
		break;
	case CHIP_TXHD2:
		lut_support = 0;
		break;
	case CHIP_T6D:
		lut_point = 9;
		break;
	default:
		break;
	}

	lut_data_size = lut_point * lut_point * lut_point * 3;

	return 0;
}

unsigned int vpp_module_lut3d_get_support(void)
{
	return lut_support;
}

unsigned int vpp_module_lut3d_get_lut_point(void)
{
	return lut_point;
}

unsigned int vpp_module_lut3d_get_table_len(void)
{
	return lut_data_size;
}

void vpp_module_lut3d_mount(void)
{
	_lut3d_data_buffer_init();
	_lut3d_data_init(lut3d_base_data, lut_point);
	lut_update_flag = 1;
}

void vpp_module_lut3d_demount(void)
{
	_lut3d_data_buffer_free();
}

void vpp_module_lut3d_en(bool enable, enum io_mode_e io_mode)
{
	unsigned int addr = 0;

	addr = ADDR_PARAM(lut3d_reg_cfg.page,
		lut3d_reg_cfg.reg_lut3d_cbus2ram_ctrl);
	WRITE_VPP_REG_BY_MODE(EN_MODE_DIR, addr, 0);

	_set_lut3d_enable_ctrl(0x7,
		lut3d_bit_cfg.bit_lut3d_extend_en.start,
		lut3d_bit_cfg.bit_lut3d_extend_en.len,
		EN_MODE_DIR);

	cur_lut3d_en = enable;

	if (io_mode == EN_MODE_DIR)
		_set_lut3d_enable_ctrl(enable,
			lut3d_bit_cfg.bit_lut3d_en.start,
			lut3d_bit_cfg.bit_lut3d_en.len,
			EN_MODE_DIR);
}

void vpp_module_lut3d_set_data(int *data,
	unsigned int data_length, enum io_mode_e io_mode)
{
	int i = 0;
	unsigned int length;

	if (!data || !lut3d_base_data)
		return;

	if (data_length > lut_data_size)
		length = lut_data_size;
	else
		length = data_length;

	for (i = 0; i < length; i++)
		lut3d_base_data[i] = data[i];

	if (io_mode == EN_MODE_DIR)
		_set_lut3d_data(data, EN_MODE_DIR);
	else
		lut_update_flag = 1;
}

void vpp_module_lut3d_pattern_en(bool enable)
{
	if (!lut_support)
		return;

	if (enable != cur_lut3d_pattern_en) {
		if (enable) {
			if (!lut3d_pattern)
				lut3d_pattern = vmalloc(lut_data_size * sizeof(int));
			_lut3d_data_init(lut3d_pattern, lut_point);
		} else {
			if (lut3d_pattern) {
				vfree(lut3d_pattern);
				lut3d_pattern = NULL;
			}
		}

		cur_lut3d_pattern_en = enable;
		lut_update_flag = 1;
	}
}

void vpp_module_lut3d_pattern_set(unsigned int val_r,
	unsigned int val_g, unsigned int val_b)
{
	int d0, d1, d2, step;
	unsigned int index;
	unsigned int shift = lut_bit_depth - 8;

	if (!lut_support ||
		!cur_lut3d_pattern_en || !lut3d_pattern)
		return;

	step = (lut_data_max + 1) / (lut_point - 1);

	for (d0 = 0; d0 < lut_point; d0++) {
		for (d1 = 0; d1 < lut_point; d1++) {
			for (d2 = 0; d2 < lut_point; d2++) {
				index =
					d0 * lut_point * lut_point * 3 +
					d1 * lut_point * 3 + d2 * 3;
				/* bypass data */
				lut3d_pattern[index + 0] =
					MIN(lut_data_max, val_r << shift);
				lut3d_pattern[index + 1] =
					MIN(lut_data_max, val_g << shift);
				lut3d_pattern[index + 2] =
					MIN(lut_data_max, val_b << shift);
			}
		}
	}

	lut_update_flag = 1;
}

void vpp_module_lut3d_on_vs(void)
{
	if (!lut_support)
		return;

	if (lut_update_flag) {
		if (!cur_lut3d_pattern_en)
			_set_lut3d_data(lut3d_base_data, EN_MODE_RDMA);
		else
			_set_lut3d_data(lut3d_pattern, EN_MODE_RDMA);

		lut_update_flag = 0;
	}

	if (pre_lut3d_en != cur_lut3d_en) {
//		pr_vpp(PR_DEBUG_LUT3D,
//			"[lut3d_on_vs] lut3d_en = %d -> %d\n",
//			pre_lut3d_en, cur_lut3d_en);

		pre_lut3d_en = cur_lut3d_en;
		_set_lut3d_enable_ctrl(cur_lut3d_en,
			lut3d_bit_cfg.bit_lut3d_en.start,
			lut3d_bit_cfg.bit_lut3d_en.len,
			EN_MODE_RDMA);
	}
}

void vpp_module_lut3d_dump_info(enum vpp_dump_module_info_e info_type)
{
	if (info_type == EN_DUMP_INFO_REG)
		_dump_lut3d_reg_info();
	else
		_dump_lut3d_data_info();
}

#endif

