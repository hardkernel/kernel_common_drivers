// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#include "../vpp_common.h"
#include "vpp_module_gamma.h"

#define VPP_GAMMA_TABLE_LEN     (257)
#define VPP_PRE_GAMMA_TABLE_LEN (65)

enum _gamma_mode_e {
	EN_MODE_GAMMA_PRE = 0,
	EN_MODE_GAMMA_LCD,
};

enum _gamma_data_type_e {
	EN_TYPE_GAMMA_R = 0,
	EN_TYPE_GAMMA_G,
	EN_TYPE_GAMMA_B
};

struct _gamma_bit_cfg_s {
	struct _bit_s bit_gamma_ctrl_en;
};

struct _gamma_reg_cfg_s {
	unsigned char page;
	unsigned char reg_gamma_ctrl;
	unsigned char reg_gamma_addr;
	unsigned char reg_gamma_data;
};

static int viu_sel;
static unsigned int add_offset;
static int auto_inc;

static unsigned int pre_gamma_tbl_len;
static unsigned int lcd_gamma_tbl_len;

static bool pregm_tp_flag;
static bool lcdgm_tp_flag;
static bool pregm_update_flag;
static bool lcdgm_update_flag;

static unsigned int cur_pregm_tbl[EN_MODE_RGB_MAX][VPP_PRE_GAMMA_TABLE_LEN];
static unsigned int cur_lcdgm_tbl[EN_MODE_RGB_MAX][VPP_GAMMA_TABLE_LEN];
static unsigned int tmp_tbl[EN_MODE_RGB_MAX][VPP_GAMMA_TABLE_LEN];

static bool pre_gamma_en;
static bool cur_gamma_en;
static bool pre_lcd_gamma_en;
static bool cur_lcd_gamma_en;

/*Default table from T3*/
static struct _gamma_reg_cfg_s pre_gamma_reg_cfg = {
	0x39,
	0xd4,
	0xd5,
	0xd6
};

static struct _gamma_reg_cfg_s lcd_gamma_reg_cfg = {
	0x14,
	0xb4,
	0xb6,
	0xb5
};

static struct _gamma_bit_cfg_s gamma_bit_cfg = {
	{0, 1}
};

/*Internal functions*/
static int _set_gamma_ctrl(enum _gamma_mode_e mode, int val,
	unsigned char start, unsigned char len, enum io_mode_e io_mode)
{
	unsigned int addr = 0;

	switch (mode) {
	case EN_MODE_GAMMA_PRE:
		addr = ADDR_PARAM(pre_gamma_reg_cfg.page, pre_gamma_reg_cfg.reg_gamma_ctrl);
		break;
	case EN_MODE_GAMMA_LCD:
		addr = ADDR_PARAM(lcd_gamma_reg_cfg.page, lcd_gamma_reg_cfg.reg_gamma_ctrl);
		addr += add_offset;
		break;
	default:
		break;
	}

	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val, start, len);

	return 0;
}

static void _set_pre_gamma_data(enum io_mode_e io_mode,
	unsigned int addr, int *data)
{
	int i = 0;
	unsigned int val = 0;

	if (!data)
		return;

	for (i = 0; i < (pre_gamma_tbl_len >> 1); i++) {
		val = (((data[i * 2 + 1] << 2) & 0xffff) << 16) | ((data[i * 2] << 2) & 0xffff);
		WRITE_VPP_REG_BY_MODE(io_mode, addr, val);
	}

	val = (data[pre_gamma_tbl_len - 1] << 2) & 0xffff;
	WRITE_VPP_REG_BY_MODE(io_mode, addr, val);
}

static void _set_lcd_gamma_data(enum io_mode_e io_mode,
	int *r_data, int *g_data, int *b_data)
{
	int i = 0;
	unsigned int val = 0;
	unsigned int addr = 0;

	addr = ADDR_PARAM(lcd_gamma_reg_cfg.page, lcd_gamma_reg_cfg.reg_gamma_addr);
	addr += add_offset;

	WRITE_VPP_REG_BY_MODE(io_mode, addr, auto_inc);

	addr = ADDR_PARAM(lcd_gamma_reg_cfg.page, lcd_gamma_reg_cfg.reg_gamma_data);
	addr += add_offset;

	for (i = 0; i < lcd_gamma_tbl_len; i++) {
		val = ((r_data[i] & 0x3ff) << 20)
			| ((g_data[i] & 0x3ff) << 10)
			| (b_data[i] & 0x3ff);
		WRITE_VPP_REG_BY_MODE(io_mode, addr, val);
	}
}

static void _get_lcd_gamma_data(int *r_data, int *g_data, int *b_data)
{
	int i = 0;
	unsigned int val = 0;
	unsigned int addr = 0;
	enum io_mode_e io_mode = EN_MODE_DIR;

	addr = ADDR_PARAM(lcd_gamma_reg_cfg.page, lcd_gamma_reg_cfg.reg_gamma_addr);
	addr += add_offset;

	WRITE_VPP_REG_BY_MODE(io_mode, addr, auto_inc);

	addr = ADDR_PARAM(lcd_gamma_reg_cfg.page, lcd_gamma_reg_cfg.reg_gamma_data);
	addr += add_offset;

	for (i = 0; i < lcd_gamma_tbl_len; i++) {
		val = READ_VPP_REG_BY_MODE(io_mode, addr);
		r_data[i] = (val >> 20) & 0x3ff;
		g_data[i] = (val >> 10) & 0x3ff;
		b_data[i] = val & 0x3ff;
	}
}

static void _init_pre_gamma_table_data(void)
{
	int i = 0;
	int step = 0;
	int val = 0;
	int max_value = 1023;

	if (pre_gamma_tbl_len &&
		pre_gamma_tbl_len <= VPP_PRE_GAMMA_TABLE_LEN) {
		step = max_value / pre_gamma_tbl_len;
		for (i = 0; i < pre_gamma_tbl_len; i++) {
			val = step * i;
			if (val > max_value)
				val = max_value;

			cur_pregm_tbl[EN_MODE_R][i] = val;
			cur_pregm_tbl[EN_MODE_G][i] = val;
			cur_pregm_tbl[EN_MODE_B][i] = val;
		}
	}
}

static void _init_lcd_gamma_table_data(void)
{
	int i = 0;
	int step = 0;
	int val = 0;
	int max_value = 1023;

	if (lcd_gamma_tbl_len &&
		lcd_gamma_tbl_len <= VPP_GAMMA_TABLE_LEN) {
		step = max_value / lcd_gamma_tbl_len;
		for (i = 0; i < lcd_gamma_tbl_len; i++) {
			val = step * i;
			if (val > max_value)
				val = max_value;

			cur_lcdgm_tbl[EN_MODE_R][i] = val;
			cur_lcdgm_tbl[EN_MODE_G][i] = val;
			cur_lcdgm_tbl[EN_MODE_B][i] = val;
		}
	}
}

static void _dump_gamma_reg_info(void)
{
	PR_DRV("pre_gamma_reg_cfg info:\n");
	PR_DRV("page = 0x%x\n", pre_gamma_reg_cfg.page);
	PR_DRV("reg_gamma_ctrl = 0x%x\n", pre_gamma_reg_cfg.reg_gamma_ctrl);
	PR_DRV("reg_gamma_addr = 0x%x\n", pre_gamma_reg_cfg.reg_gamma_addr);
	PR_DRV("reg_gamma_data = 0x%x\n", pre_gamma_reg_cfg.reg_gamma_data);

	PR_DRV("lcd_gamma_reg_cfg info:\n");
	PR_DRV("page = 0x%x\n", lcd_gamma_reg_cfg.page);
	PR_DRV("reg_gamma_ctrl = 0x%x\n", lcd_gamma_reg_cfg.reg_gamma_ctrl);
	PR_DRV("reg_gamma_addr = 0x%x\n", lcd_gamma_reg_cfg.reg_gamma_addr);
	PR_DRV("reg_gamma_data = 0x%x\n", lcd_gamma_reg_cfg.reg_gamma_data);
}

static void _dump_gamma_data_info(void)
{
	int i = 0;
	unsigned int addr = 0;
	unsigned int data32 = 0;

	PR_DRV("viu_sel = %d\n", viu_sel);
	PR_DRV("add_offset = 0x%x\n", add_offset);
	PR_DRV("auto_inc = %d\n", auto_inc);
	PR_DRV("pre_gamma_tbl_len = %d\n", pre_gamma_tbl_len);
	PR_DRV("lcd_gamma_tbl_len = %d\n", lcd_gamma_tbl_len);
	PR_DRV("pregm_tp_flag = %d\n", pregm_tp_flag);
	PR_DRV("lcdgm_tp_flag = %d\n", lcdgm_tp_flag);

	PR_DRV("cur_pregm_tbl data info:\n");
	PR_DRV("cur_gamma_en = %d\n", cur_gamma_en);
	for (i = 0; i < pre_gamma_tbl_len; i++)
		PR_DRV("%d\t%d\t%d\n", cur_pregm_tbl[0][i],
			cur_pregm_tbl[1][i], cur_pregm_tbl[2][i]);

	PR_DRV("cur_lcdgm_tbl data info:\n");
	PR_DRV("cur_lcd_gamma_en = %d\n", cur_lcd_gamma_en);
	for (i = 0; i < lcd_gamma_tbl_len; i++)
		PR_DRV("%d\t%d\t%d\n", cur_lcdgm_tbl[0][i],
			cur_lcdgm_tbl[1][i], cur_lcdgm_tbl[2][i]);

	PR_DRV("cur_lcdgm_reg_val info:\n");
	addr = ADDR_PARAM(pre_gamma_reg_cfg.page,
		pre_gamma_reg_cfg.reg_gamma_ctrl);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(lcd_gamma_reg_cfg.page,
		lcd_gamma_reg_cfg.reg_gamma_ctrl);
	addr += add_offset;
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	_get_lcd_gamma_data(&tmp_tbl[0][0], &tmp_tbl[1][0], &tmp_tbl[2][0]);
	for (i = 0; i < lcd_gamma_tbl_len; i++)
		PR_DRV("%d\t%d\t%d\n", tmp_tbl[0][i],
			tmp_tbl[1][i], tmp_tbl[2][i]);
}

/*External functions*/
int vpp_module_gamma_init(struct vpp_dev_s *dev)
{
	enum vpp_chip_type_e chip_id;

	chip_id = dev->m_data->chip_id;

	if (chip_id < CHIP_T5M) {
		auto_inc = 1 << 8;
		lcd_gamma_tbl_len = VPP_GAMMA_TABLE_LEN - 1;
	} else {
		auto_inc = 1 << 9;
		lcd_gamma_tbl_len = VPP_GAMMA_TABLE_LEN;
	}

	pre_gamma_tbl_len = VPP_PRE_GAMMA_TABLE_LEN;

	pregm_tp_flag = false;
	lcdgm_tp_flag = false;
	pregm_update_flag = false;
	lcdgm_update_flag = false;

	_init_pre_gamma_table_data();
	_init_lcd_gamma_table_data();

	return 0;
}

void vpp_module_gamma_set_viu_sel(int val)
{
	viu_sel = val;

	if (viu_sel == 1) {         /*venc1*/
		add_offset = 0x100;
	} else if (viu_sel == 2) {  /*venc2*/
		add_offset = 0x200;
	} else {                    /*venc0*/
		add_offset = 0x0;
	}
}

void vpp_module_pre_gamma_en(bool enable, enum io_mode_e io_mode)
{
	cur_gamma_en = enable;

	if (io_mode == EN_MODE_DIR)
		_set_gamma_ctrl(EN_MODE_GAMMA_PRE, enable,
			gamma_bit_cfg.bit_gamma_ctrl_en.start,
			gamma_bit_cfg.bit_gamma_ctrl_en.len,
			EN_MODE_DIR);
}

int vpp_module_pre_gamma_write_single(enum vpp_rgb_mode_e mode,
	unsigned int *data)
{
	int i;

	if (!data || mode == EN_MODE_RGB_MAX)
		return 0;

	for (i = 0; i < pre_gamma_tbl_len; i++)
		cur_pregm_tbl[mode][i] = data[i];

	pregm_update_flag = true;

	return 0;
}

int vpp_module_pre_gamma_write(unsigned int *r_data,
	unsigned int *g_data, unsigned int *b_data)
{
	int i;

	if (!r_data || !g_data || !b_data)
		return 0;

	for (i = 0; i < pre_gamma_tbl_len; i++) {
		cur_pregm_tbl[EN_MODE_R][i] = r_data[i];
		cur_pregm_tbl[EN_MODE_G][i] = g_data[i];
		cur_pregm_tbl[EN_MODE_B][i] = b_data[i];
	}

	pregm_update_flag = true;

	return 0;
}

int vpp_module_pre_gamma_read(unsigned int *r_data,
	unsigned int *g_data, unsigned int *b_data)
{
	int i;

	if (!r_data || !g_data || !b_data)
		return 0;

	for (i = 0; i < pre_gamma_tbl_len; i++) {
		r_data[i] = cur_pregm_tbl[EN_MODE_R][i];
		g_data[i] = cur_pregm_tbl[EN_MODE_G][i];
		b_data[i] = cur_pregm_tbl[EN_MODE_B][i];
	}

	return 0;
}

int vpp_module_pre_gamma_set_default(void)
{
	_init_pre_gamma_table_data();

	pregm_update_flag = true;

	return 0;
}

int vpp_module_pre_gamma_pattern(bool enable,
	unsigned int r_val, unsigned int g_val, unsigned int b_val)
{
	int i;
	int *r_data;
	int *g_data;
	int *b_data;
	unsigned int addr;
	enum io_mode_e io_mode = EN_MODE_DIR;

	pregm_tp_flag = enable;

	if (enable) {
		for (i = 0; i < pre_gamma_tbl_len; i++) {
			tmp_tbl[EN_MODE_R][i] = r_val;
			tmp_tbl[EN_MODE_G][i] = g_val;
			tmp_tbl[EN_MODE_B][i] = b_val;
		}

		r_data = &tmp_tbl[EN_MODE_R][0];
		g_data = &tmp_tbl[EN_MODE_G][0];
		b_data = &tmp_tbl[EN_MODE_B][0];
	} else {
		r_data = &cur_pregm_tbl[EN_MODE_R][0];
		g_data = &cur_pregm_tbl[EN_MODE_G][0];
		b_data = &cur_pregm_tbl[EN_MODE_B][0];
	}

	addr = ADDR_PARAM(pre_gamma_reg_cfg.page, pre_gamma_reg_cfg.reg_gamma_addr);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, 0);

	addr = ADDR_PARAM(pre_gamma_reg_cfg.page, pre_gamma_reg_cfg.reg_gamma_data);
	_set_pre_gamma_data(io_mode, addr, r_data);
	_set_pre_gamma_data(io_mode, addr, g_data);
	_set_pre_gamma_data(io_mode, addr, b_data);

	return 0;
}

unsigned int vpp_module_pre_gamma_get_table_len(void)
{
	return pre_gamma_tbl_len;
}

void vpp_module_pre_gamma_on_vs(void)
{
	unsigned int addr;
	enum io_mode_e io_mode = EN_MODE_RDMA;

	if (pre_gamma_en != cur_gamma_en) {
//		pr_vpp(PR_DEBUG_GAMMA,
//			"[pre_gamma_on_vs] pre_gamma_en = %d -> %d\n",
//			pre_gamma_en, cur_gamma_en);

		pre_gamma_en = cur_gamma_en;
		_set_gamma_ctrl(EN_MODE_GAMMA_PRE, cur_gamma_en,
			gamma_bit_cfg.bit_gamma_ctrl_en.start,
			gamma_bit_cfg.bit_gamma_ctrl_en.len,
			io_mode);
	}

	if (pregm_tp_flag || !pregm_update_flag)
		return;

	addr = ADDR_PARAM(pre_gamma_reg_cfg.page, pre_gamma_reg_cfg.reg_gamma_addr);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, 0);

	addr = ADDR_PARAM(pre_gamma_reg_cfg.page, pre_gamma_reg_cfg.reg_gamma_data);
	_set_pre_gamma_data(io_mode, addr, &cur_pregm_tbl[EN_MODE_R][0]);
	_set_pre_gamma_data(io_mode, addr, &cur_pregm_tbl[EN_MODE_G][0]);
	_set_pre_gamma_data(io_mode, addr, &cur_pregm_tbl[EN_MODE_B][0]);

	pregm_update_flag = false;
}

void vpp_module_lcd_gamma_en(bool enable, enum io_mode_e io_mode)
{
	cur_lcd_gamma_en = enable;

	if (io_mode == EN_MODE_DIR)
		_set_gamma_ctrl(EN_MODE_GAMMA_LCD, enable,
			gamma_bit_cfg.bit_gamma_ctrl_en.start,
			gamma_bit_cfg.bit_gamma_ctrl_en.len,
			EN_MODE_DIR);
}

int vpp_module_lcd_gamma_write_single(enum vpp_rgb_mode_e mode,
	unsigned int *data)
{
	int i;

	if (!data || mode == EN_MODE_RGB_MAX)
		return 0;

	for (i = 0; i < lcd_gamma_tbl_len; i++)
		cur_lcdgm_tbl[mode][i] = data[i];

	lcdgm_update_flag = true;

	return 0;
}

int vpp_module_lcd_gamma_write(unsigned int *r_data,
	unsigned int *g_data, unsigned int *b_data)
{
	int i;

	if (!r_data || !g_data || !b_data)
		return 0;

	for (i = 0; i < lcd_gamma_tbl_len; i++) {
		cur_lcdgm_tbl[EN_MODE_R][i] = r_data[i];
		cur_lcdgm_tbl[EN_MODE_G][i] = g_data[i];
		cur_lcdgm_tbl[EN_MODE_B][i] = b_data[i];
	}

	lcdgm_update_flag = true;

	return 0;
}

int vpp_module_lcd_gamma_read(unsigned int *r_data,
	unsigned int *g_data, unsigned int *b_data)
{
	if (!r_data || !g_data || !b_data)
		return 0;

	_get_lcd_gamma_data(r_data, g_data, b_data);

	return 0;
}

int vpp_module_lcd_gamma_set_default(void)
{
	_init_lcd_gamma_table_data();

	lcdgm_update_flag = true;

	return 0;
}

int vpp_module_lcd_gamma_pattern(bool enable,
	unsigned int r_val, unsigned int g_val, unsigned int b_val)
{
	int i;
	int *r_data;
	int *g_data;
	int *b_data;
	enum io_mode_e io_mode = EN_MODE_DIR;

	lcdgm_tp_flag = enable;

	if (enable) {
		for (i = 0; i < lcd_gamma_tbl_len; i++) {
			tmp_tbl[EN_MODE_R][i] = r_val;
			tmp_tbl[EN_MODE_G][i] = g_val;
			tmp_tbl[EN_MODE_B][i] = b_val;
		}

		r_data = &tmp_tbl[EN_MODE_R][0];
		g_data = &tmp_tbl[EN_MODE_G][0];
		b_data = &tmp_tbl[EN_MODE_B][0];
	} else {
		r_data = &cur_lcdgm_tbl[EN_MODE_R][0];
		g_data = &cur_lcdgm_tbl[EN_MODE_G][0];
		b_data = &cur_lcdgm_tbl[EN_MODE_B][0];
	}

	_set_lcd_gamma_data(io_mode, r_data, g_data, b_data);

	return 0;
}

unsigned int vpp_module_lcd_gamma_get_table_len(void)
{
	return lcd_gamma_tbl_len;
}

void vpp_module_lcd_gamma_notify(void)
{
	lcdgm_update_flag = true;
}

void vpp_module_lcd_gamma_on_vs(void)
{
	if (pre_lcd_gamma_en != cur_lcd_gamma_en) {
		pr_vpp(PR_DEBUG_GAMMA,
			"[lcd_gamma_on_vs] lcd_gamma_en = %d -> %d\n",
			pre_lcd_gamma_en, cur_lcd_gamma_en);

		pre_lcd_gamma_en = cur_lcd_gamma_en;
		_set_gamma_ctrl(EN_MODE_GAMMA_LCD, cur_lcd_gamma_en,
			gamma_bit_cfg.bit_gamma_ctrl_en.start,
			gamma_bit_cfg.bit_gamma_ctrl_en.len,
			EN_MODE_RDMA);
	}

	if (lcdgm_tp_flag || !lcdgm_update_flag)
		return;

	_set_lcd_gamma_data(EN_MODE_RDMA,
		&cur_lcdgm_tbl[EN_MODE_R][0],
		&cur_lcdgm_tbl[EN_MODE_G][0],
		&cur_lcdgm_tbl[EN_MODE_B][0]);

	lcdgm_update_flag = false;
}

void vpp_module_gamma_dump_info(enum vpp_dump_module_info_e info_type)
{
	if (info_type == EN_DUMP_INFO_REG)
		_dump_gamma_reg_info();
	else
		_dump_gamma_data_info();
}

#endif

