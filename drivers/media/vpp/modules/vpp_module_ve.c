// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#include "../vpp_common.h"
#include "vpp_module_ve.h"

struct _ve_bit_cfg_s {
	struct _bit_s bit_blkext_en;
	struct _bit_s bit_blkext_slope2;
	struct _bit_s bit_blkext_midpt;
	struct _bit_s bit_blkext_slope1;
	struct _bit_s bit_blkext_start;
	struct _bit_s bit_ccoring_en;
	struct _bit_s bit_ccoring_slope;
	struct _bit_s bit_ccoring_th;
	struct _bit_s bit_ccoring_bypass_yth;
	struct _bit_s bit_demo_center_bar_en;
	struct _bit_s bit_bluestretch_en;
	struct _bit_s bit_bluestretch_en_sel;
	struct _bit_s bit_bluestretch_cb_inc;
	struct _bit_s bit_bluestretch_cr_inc;
	struct _bit_s bit_bluestretch_err_sign_3;
	struct _bit_s bit_bluestretch_err_sign_2;
	struct _bit_s bit_bluestretch_err_sign_1;
	struct _bit_s bit_bluestretch_err_sign_0;
	struct _bit_s bit_bluestretch_gain;
	struct _bit_s bit_bluestretch_gain_cb4cr;
	struct _bit_s bit_bluestretch_luma_high;
	struct _bit_s bit_bluestretch_err_crp;
	struct _bit_s bit_bluestretch_err_crp_inv;
	struct _bit_s bit_bluestretch_err_crn;
	struct _bit_s bit_bluestretch_err_crn_inv;
	struct _bit_s bit_bluestretch_err_cbp;
	struct _bit_s bit_bluestretch_err_cbp_inv;
	struct _bit_s bit_bluestretch_err_cbn;
	struct _bit_s bit_bluestretch_err_cbn_inv;
	struct _bit_s bit_ccoring_gclk_ctrl;
	struct _bit_s bit_blkext_gclk_ctrl;
	struct _bit_s bit_bluestretch_gclk_ctrl;
	struct _bit_s bit_cm_gclk_ctrl;
};

struct _ve_reg_cfg_s {
	unsigned char page;
	unsigned char reg_ve_enable_ctrl;
	unsigned char reg_ve_blkext_ctrl;
	unsigned char reg_ve_ccoring_ctrl;
	unsigned char reg_ve_demo_center_bar;
	unsigned char reg_ve_demo_left_top_screen_width;
	unsigned char reg_ve_blue_stretch_ctrl;
	unsigned char reg_ve_blue_stretch_err_cr;
	unsigned char reg_ve_blue_stretch_err_cb;
	unsigned char reg_ve_clip_misc0;
	unsigned char reg_ve_clip_misc1;
	unsigned char reg_ve_gclk_ctrl0;
	unsigned char reg_ve_gclk_ctrl1;
};

/*Default table from T3*/
static struct _ve_reg_cfg_s ve_reg_cfg = {
	0x1d,
	0xa1,
	0x80,
	0xa0,
	0xa3,
	0xa2,
	0x9c,
	0x9d,
	0x9e,
	0xd9,
	0xda,
	0x72,
	0x73,
};

static struct _ve_bit_cfg_s ve_bit_cfg = {
	{3, 1},
	{0, 8},
	{8, 8},
	{16, 8},
	{24, 8},
	{4, 1},
	{0, 4},
	{8, 8},
	{16, 12},
	{31, 1},
	{31, 1},
	{30, 1},
	{29, 1},
	{28, 1},
	{27, 1},
	{26, 1},
	{25, 1},
	{24, 1},
	{16, 8},
	{8, 8},
	{0, 8},
	{27, 5},
	{16, 11},
	{11, 5},
	{0, 11},
	{27, 5},
	{16, 11},
	{11, 5},
	{0, 11},
	{24, 2},
	{26, 2},
	{6, 2},
	{0, 4},
};

enum ve_update_sel_e {
	EN_UPDATE_BLE = BIT(0),
	EN_UPDATE_CC = BIT(1),
	EN_UPDATE_BS = BIT(2),
	EN_UPDATE_CLK_CTRL = BIT(3),
};

static bool pre_ble_en;
static bool pre_cc_en;
static bool pre_bs_en;
static int pre_ble_data[EN_BLKEXT_MAX];
static int pre_cc_data[EN_CCORING_MAX];
static int pre_bs_data[EN_BLUE_STRETCH_MAX];
static int pre_clk_ctrl_data[EN_CLK_MAX];

static bool cur_ble_en;
static bool cur_cc_en;
static bool cur_bs_en;
static int cur_ble_data[EN_BLKEXT_MAX];
static int cur_cc_data[EN_CCORING_MAX];
static int cur_bs_data[EN_BLUE_STRETCH_MAX];
static int cur_clk_ctrl_data[EN_CLK_MAX];

/*Internal functions*/
static void _set_ve_enable_ctrl(int val,
	unsigned char start, unsigned char len, enum io_mode_e io_mode)
{
	unsigned int addr = 0;

	addr = ADDR_PARAM(ve_reg_cfg.page, ve_reg_cfg.reg_ve_enable_ctrl);
	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val, start, len);
}

static void _set_ve_blkext_ctrl(int val,
	unsigned char start, unsigned char len, enum io_mode_e io_mode)
{
	unsigned int addr = 0;

	addr = ADDR_PARAM(ve_reg_cfg.page, ve_reg_cfg.reg_ve_blkext_ctrl);
	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val, start, len);
}

static void _set_ve_ccoring_ctrl(int val,
	unsigned char start, unsigned char len, enum io_mode_e io_mode)
{
	unsigned int addr = 0;

	addr = ADDR_PARAM(ve_reg_cfg.page, ve_reg_cfg.reg_ve_ccoring_ctrl);
	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val, start, len);
}

static void _set_ve_blue_stretch_ctrl(int val,
	unsigned char start, unsigned char len, enum io_mode_e io_mode)
{
	unsigned int addr = 0;

	addr = ADDR_PARAM(ve_reg_cfg.page, ve_reg_cfg.reg_ve_blue_stretch_ctrl);
	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val, start, len);
}

static void _set_ve_demo_center_bar(int val,
	unsigned char start, unsigned char len)
{
	unsigned int addr = 0;
	enum io_mode_e io_mode = EN_MODE_DIR;

	addr = ADDR_PARAM(ve_reg_cfg.page, ve_reg_cfg.reg_ve_demo_center_bar);
	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val, start, len);
}

static void _set_ve_blkext_params(unsigned int *data,
	enum io_mode_e io_mode)
{
	unsigned int addr = 0;
	int val = 0;

	if (!data)
		return;

	addr = ADDR_PARAM(ve_reg_cfg.page, ve_reg_cfg.reg_ve_blkext_ctrl);

	val = vpp_insert_int(val, data[EN_BLKEXT_START],
		ve_bit_cfg.bit_blkext_start.start,
		ve_bit_cfg.bit_blkext_start.len);

	val = vpp_insert_int(val, data[EN_BLKEXT_SLOPE1],
		ve_bit_cfg.bit_blkext_slope1.start,
		ve_bit_cfg.bit_blkext_slope1.len);

	val = vpp_insert_int(val, data[EN_BLKEXT_MIDPT],
		ve_bit_cfg.bit_blkext_midpt.start,
		ve_bit_cfg.bit_blkext_midpt.len);

	val = vpp_insert_int(val, data[EN_BLKEXT_SLOPE2],
		ve_bit_cfg.bit_blkext_slope2.start,
		ve_bit_cfg.bit_blkext_slope2.len);

	WRITE_VPP_REG_BY_MODE(io_mode, addr, val);
}

static void _set_ve_ccoring_params(unsigned int *data,
	enum io_mode_e io_mode)
{
	unsigned int addr = 0;
	int val = 0;

	if (!data)
		return;

	addr = ADDR_PARAM(ve_reg_cfg.page, ve_reg_cfg.reg_ve_ccoring_ctrl);

	val = vpp_insert_int(val, data[EN_CCORING_SLOPE],
		ve_bit_cfg.bit_ccoring_slope.start,
		ve_bit_cfg.bit_ccoring_slope.len);

	val = vpp_insert_int(val, data[EN_CCORING_TH],
		ve_bit_cfg.bit_ccoring_th.start,
		ve_bit_cfg.bit_ccoring_th.len);

	val = vpp_insert_int(val, data[EN_CCORING_BYPASS_YTH],
		ve_bit_cfg.bit_ccoring_bypass_yth.start,
		ve_bit_cfg.bit_ccoring_bypass_yth.len);

	WRITE_VPP_REG_BY_MODE(io_mode, addr, val);
}

static void _set_ve_blue_stretch_params(unsigned int *data,
	enum io_mode_e io_mode)
{
	unsigned int addr = 0;
	int val = 0;

	if (!data)
		return;

	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_blue_stretch_ctrl);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_EN_SEL],
		ve_bit_cfg.bit_bluestretch_en_sel.start,
		ve_bit_cfg.bit_bluestretch_en_sel.len);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_CB_INC],
		ve_bit_cfg.bit_bluestretch_cb_inc.start,
		ve_bit_cfg.bit_bluestretch_cb_inc.len);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_CR_INC],
		ve_bit_cfg.bit_bluestretch_cr_inc.start,
		ve_bit_cfg.bit_bluestretch_cr_inc.len);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_GAIN],
		ve_bit_cfg.bit_bluestretch_gain.start,
		ve_bit_cfg.bit_bluestretch_gain.len);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_GAIN_CB4CR],
		ve_bit_cfg.bit_bluestretch_gain_cb4cr.start,
		ve_bit_cfg.bit_bluestretch_gain_cb4cr.len);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_LUMA_HIGH],
		ve_bit_cfg.bit_bluestretch_luma_high.start,
		ve_bit_cfg.bit_bluestretch_luma_high.len);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_ERR_SIGN3],
		ve_bit_cfg.bit_bluestretch_err_sign_3.start,
		ve_bit_cfg.bit_bluestretch_err_sign_3.len);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_ERR_SIGN2],
		ve_bit_cfg.bit_bluestretch_err_sign_2.start,
		ve_bit_cfg.bit_bluestretch_err_sign_2.len);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_ERR_SIGN1],
		ve_bit_cfg.bit_bluestretch_err_sign_1.start,
		ve_bit_cfg.bit_bluestretch_err_sign_1.len);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_ERR_SIGN0],
		ve_bit_cfg.bit_bluestretch_err_sign_0.start,
		ve_bit_cfg.bit_bluestretch_err_sign_0.len);

	WRITE_VPP_REG_BY_MODE(io_mode, addr, val);

	val = 0;
	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_blue_stretch_err_cr);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_ERR_CRP],
		ve_bit_cfg.bit_bluestretch_err_crp.start,
		ve_bit_cfg.bit_bluestretch_err_crp.len);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_ERR_CRP_INV],
		ve_bit_cfg.bit_bluestretch_err_crp_inv.start,
		ve_bit_cfg.bit_bluestretch_err_crp_inv.len);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_ERR_CRN],
		ve_bit_cfg.bit_bluestretch_err_crn.start,
		ve_bit_cfg.bit_bluestretch_err_crn.len);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_ERR_CRN_INV],
		ve_bit_cfg.bit_bluestretch_err_crn_inv.start,
		ve_bit_cfg.bit_bluestretch_err_crn_inv.len);

	WRITE_VPP_REG_BY_MODE(io_mode, addr, val);

	val = 0;
	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_blue_stretch_err_cb);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_ERR_CBP],
		ve_bit_cfg.bit_bluestretch_err_cbp.start,
		ve_bit_cfg.bit_bluestretch_err_cbp.len);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_ERR_CBP_INV],
		ve_bit_cfg.bit_bluestretch_err_cbp_inv.start,
		ve_bit_cfg.bit_bluestretch_err_cbp_inv.len);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_ERR_CBN],
		ve_bit_cfg.bit_bluestretch_err_cbn.start,
		ve_bit_cfg.bit_bluestretch_err_cbn.len);

	val = vpp_insert_int(val, data[EN_BLUE_STRETCH_ERR_CBN_INV],
		ve_bit_cfg.bit_bluestretch_err_cbn_inv.start,
		ve_bit_cfg.bit_bluestretch_err_cbn_inv.len);

	WRITE_VPP_REG_BY_MODE(io_mode, addr, val);
}

static void _dump_ve_reg_info(void)
{
	PR_DRV("ve_reg_cfg info:\n");
	PR_DRV("page = 0x%x\n", ve_reg_cfg.page);
	PR_DRV("reg_ve_enable_ctrl = 0x%x\n",
		ve_reg_cfg.reg_ve_enable_ctrl);
	PR_DRV("reg_ve_blkext_ctrl = 0x%x\n",
		ve_reg_cfg.reg_ve_blkext_ctrl);
	PR_DRV("reg_ve_ccoring_ctrl = 0x%x\n",
		ve_reg_cfg.reg_ve_ccoring_ctrl);
	PR_DRV("reg_ve_demo_center_bar = 0x%x\n",
		ve_reg_cfg.reg_ve_demo_center_bar);
	PR_DRV("reg_ve_demo_left_top_screen_width = 0x%x\n",
		ve_reg_cfg.reg_ve_demo_left_top_screen_width);
	PR_DRV("reg_ve_blue_stretch_ctrl = 0x%x\n",
		ve_reg_cfg.reg_ve_blue_stretch_ctrl);
	PR_DRV("reg_ve_blue_stretch_err_cr = 0x%x\n",
		ve_reg_cfg.reg_ve_blue_stretch_err_cr);
	PR_DRV("reg_ve_blue_stretch_err_cb = 0x%x\n",
		ve_reg_cfg.reg_ve_blue_stretch_err_cb);
	PR_DRV("reg_ve_clip_misc0 = 0x%x\n", ve_reg_cfg.reg_ve_clip_misc0);
	PR_DRV("reg_ve_clip_misc1 = 0x%x\n", ve_reg_cfg.reg_ve_clip_misc1);
	PR_DRV("reg_ve_gclk_ctrl0 = 0x%x\n", ve_reg_cfg.reg_ve_gclk_ctrl0);
	PR_DRV("reg_ve_gclk_ctrl1 = 0x%x\n", ve_reg_cfg.reg_ve_gclk_ctrl1);
}

static void _dump_ve_data_info(void)
{
	int i = 0;
	unsigned int addr = 0;
	unsigned int data32 = 0;

	PR_DRV("cur_data info:\n");
	PR_DRV("cur_ble_en = %d\n", cur_ble_en);
	PR_DRV("cur_cc_en = %d\n", cur_cc_en);
	PR_DRV("cur_bs_en = %d\n", cur_bs_en);

	for (i = 0; i < EN_BLKEXT_MAX; i++)
		PR_DRV("cur_ble_data[%d] = %d\n", i, cur_ble_data[i]);

	for (i = 0; i < EN_CCORING_MAX; i++)
		PR_DRV("cur_cc_data[%d] = %d\n", i, cur_cc_data[i]);

	for (i = 0; i < EN_BLUE_STRETCH_MAX; i++)
		PR_DRV("cur_bs_data[%d] = %d\n", i, cur_bs_data[i]);

	for (i = 0; i < EN_CLK_MAX; i++)
		PR_DRV("cur_clk_ctrl_data[%d] = %d\n", i, cur_clk_ctrl_data[i]);

	PR_DRV("cur_go_reg_val info:\n");
	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_enable_ctrl);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_blkext_ctrl);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_ccoring_ctrl);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_demo_center_bar);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_demo_left_top_screen_width);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_blue_stretch_ctrl);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_blue_stretch_err_cr);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_blue_stretch_err_cb);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_clip_misc0);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_clip_misc1);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_gclk_ctrl0);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_gclk_ctrl1);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);
}

/*External functions*/
int vpp_module_ve_init(struct vpp_dev_s *dev)
{
	return 0;
}

void vpp_module_ve_blkext_en(bool enable,
	enum io_mode_e io_mode)
{
	cur_ble_en = enable;

	if (io_mode == EN_MODE_DIR)
		_set_ve_enable_ctrl(enable,
			ve_bit_cfg.bit_blkext_en.start,
			ve_bit_cfg.bit_blkext_en.len,
			EN_MODE_DIR);
}

void vpp_module_ve_set_blkext_params(unsigned int *data,
	enum io_mode_e io_mode)
{
	if (!data)
		return;

	memcpy(&cur_ble_data, data, sizeof(cur_ble_data));

	if (io_mode == EN_MODE_DIR)
		_set_ve_blkext_params(data, EN_MODE_DIR);
}

void vpp_module_ve_set_blkext_param(enum vpp_blkext_param_e type,
	int val, enum io_mode_e io_mode)
{
	unsigned char start = 0;
	unsigned char len = 0;

	if (type == EN_BLKEXT_MAX)
		return;

	cur_ble_data[type] = val;

	if (io_mode == EN_MODE_DIR) {
		switch (type) {
		case EN_BLKEXT_START:
			start = ve_bit_cfg.bit_blkext_start.start;
			len = ve_bit_cfg.bit_blkext_start.len;
			break;
		case EN_BLKEXT_SLOPE1:
			start = ve_bit_cfg.bit_blkext_slope1.start;
			len = ve_bit_cfg.bit_blkext_slope1.len;
			break;
		case EN_BLKEXT_MIDPT:
			start = ve_bit_cfg.bit_blkext_midpt.start;
			len = ve_bit_cfg.bit_blkext_midpt.len;
			break;
		case EN_BLKEXT_SLOPE2:
			start = ve_bit_cfg.bit_blkext_slope2.start;
			len = ve_bit_cfg.bit_blkext_slope2.len;
			break;
		default:
			return;
		}

		_set_ve_blkext_ctrl(val, start, len, EN_MODE_DIR);
	}
}

void vpp_module_ve_ccoring_en(bool enable,
	enum io_mode_e io_mode)
{
	cur_cc_en = enable;

	if (io_mode == EN_MODE_DIR)
		_set_ve_enable_ctrl(enable,
			ve_bit_cfg.bit_ccoring_en.start,
			ve_bit_cfg.bit_ccoring_en.len,
			EN_MODE_DIR);
}

void vpp_module_ve_set_ccoring_params(unsigned int *data,
	enum io_mode_e io_mode)
{
	if (!data)
		return;

	memcpy(&cur_cc_data, data, sizeof(cur_cc_data));

	if (io_mode == EN_MODE_DIR)
		_set_ve_ccoring_params(data, EN_MODE_DIR);
}

void vpp_module_ve_set_ccoring_param(enum vpp_ccoring_params_e type,
	int val, enum io_mode_e io_mode)
{
	unsigned char start = 0;
	unsigned char len = 0;

	if (type == EN_CCORING_MAX)
		return;

	cur_cc_data[type] = val;

	if (io_mode == EN_MODE_DIR) {
		switch (type) {
		case EN_CCORING_SLOPE:
			start = ve_bit_cfg.bit_ccoring_slope.start;
			len = ve_bit_cfg.bit_ccoring_slope.len;
			break;
		case EN_CCORING_TH:
			start = ve_bit_cfg.bit_ccoring_th.start;
			len = ve_bit_cfg.bit_ccoring_th.len;
			break;
		case EN_CCORING_BYPASS_YTH:
			start = ve_bit_cfg.bit_ccoring_bypass_yth.start;
			len = ve_bit_cfg.bit_ccoring_bypass_yth.len;
			break;
		default:
			return;
		}

		_set_ve_ccoring_ctrl(val, start, len, EN_MODE_DIR);
	}
}

void vpp_module_ve_blue_stretch_en(bool enable,
	enum io_mode_e io_mode)
{
	cur_bs_en = enable;

	if (io_mode == EN_MODE_DIR)
		_set_ve_blue_stretch_ctrl(enable,
			ve_bit_cfg.bit_bluestretch_en.start,
			ve_bit_cfg.bit_bluestretch_en.len,
			EN_MODE_DIR);
}

void vpp_module_ve_set_blue_stretch_params(unsigned int *data,
	enum io_mode_e io_mode)
{
	if (!data)
		return;

	memcpy(&cur_bs_data, data, sizeof(cur_bs_data));

	if (io_mode == EN_MODE_DIR)
		_set_ve_blue_stretch_params(data, EN_MODE_DIR);
}

void vpp_module_ve_set_blue_stretch_param(enum vpp_blue_stretch_param_e type,
	int val, enum io_mode_e io_mode)
{
	unsigned int addr = 0;
	unsigned char start = 0;
	unsigned char len = 0;

	if (type == EN_BLUE_STRETCH_MAX)
		return;

	cur_bs_data[type] = val;

	if (io_mode == EN_MODE_DIR) {
		addr = ADDR_PARAM(ve_reg_cfg.page,
			ve_reg_cfg.reg_ve_blue_stretch_ctrl);

		switch (type) {
		case EN_BLUE_STRETCH_EN_SEL:
			start = ve_bit_cfg.bit_bluestretch_en_sel.start;
			len = ve_bit_cfg.bit_bluestretch_en_sel.len;
			break;
		case EN_BLUE_STRETCH_CB_INC:
			start = ve_bit_cfg.bit_bluestretch_cb_inc.start;
			len = ve_bit_cfg.bit_bluestretch_cb_inc.len;
			break;
		case EN_BLUE_STRETCH_CR_INC:
			start = ve_bit_cfg.bit_bluestretch_cr_inc.start;
			len = ve_bit_cfg.bit_bluestretch_cr_inc.len;
			break;
		case EN_BLUE_STRETCH_GAIN:
			start = ve_bit_cfg.bit_bluestretch_gain.start;
			len = ve_bit_cfg.bit_bluestretch_gain.len;
			break;
		case EN_BLUE_STRETCH_GAIN_CB4CR:
			start = ve_bit_cfg.bit_bluestretch_gain_cb4cr.start;
			len = ve_bit_cfg.bit_bluestretch_gain_cb4cr.len;
			break;
		case EN_BLUE_STRETCH_LUMA_HIGH:
			start = ve_bit_cfg.bit_bluestretch_luma_high.start;
			len = ve_bit_cfg.bit_bluestretch_luma_high.len;
			break;
		case EN_BLUE_STRETCH_ERR_CRP:
			addr = ADDR_PARAM(ve_reg_cfg.page,
				ve_reg_cfg.reg_ve_blue_stretch_err_cr);
			start = ve_bit_cfg.bit_bluestretch_err_crp.start;
			len = ve_bit_cfg.bit_bluestretch_err_crp.len;
			break;
		case EN_BLUE_STRETCH_ERR_SIGN3:
			start = ve_bit_cfg.bit_bluestretch_err_sign_3.start;
			len = ve_bit_cfg.bit_bluestretch_err_sign_3.len;
			break;
		case EN_BLUE_STRETCH_ERR_CRP_INV:
			addr = ADDR_PARAM(ve_reg_cfg.page,
				ve_reg_cfg.reg_ve_blue_stretch_err_cr);
			start = ve_bit_cfg.bit_bluestretch_err_crp_inv.start;
			len = ve_bit_cfg.bit_bluestretch_err_crp_inv.len;
			break;
		case EN_BLUE_STRETCH_ERR_CRN:
			addr = ADDR_PARAM(ve_reg_cfg.page,
				ve_reg_cfg.reg_ve_blue_stretch_err_cr);
			start = ve_bit_cfg.bit_bluestretch_err_crn.start;
			len = ve_bit_cfg.bit_bluestretch_err_crn.len;
			break;
		case EN_BLUE_STRETCH_ERR_SIGN2:
			start = ve_bit_cfg.bit_bluestretch_err_sign_2.start;
			len = ve_bit_cfg.bit_bluestretch_err_sign_2.len;
			break;
		case EN_BLUE_STRETCH_ERR_CRN_INV:
			addr = ADDR_PARAM(ve_reg_cfg.page,
				ve_reg_cfg.reg_ve_blue_stretch_err_cr);
			start = ve_bit_cfg.bit_bluestretch_err_crn_inv.start;
			len = ve_bit_cfg.bit_bluestretch_err_crn_inv.len;
			break;
		case EN_BLUE_STRETCH_ERR_CBP:
			addr = ADDR_PARAM(ve_reg_cfg.page,
				ve_reg_cfg.reg_ve_blue_stretch_err_cb);
			start = ve_bit_cfg.bit_bluestretch_err_cbp.start;
			len = ve_bit_cfg.bit_bluestretch_err_cbp.len;
			break;
		case EN_BLUE_STRETCH_ERR_SIGN1:
			start = ve_bit_cfg.bit_bluestretch_err_sign_1.start;
			len = ve_bit_cfg.bit_bluestretch_err_sign_1.len;
			break;
		case EN_BLUE_STRETCH_ERR_CBP_INV:
			addr = ADDR_PARAM(ve_reg_cfg.page,
				ve_reg_cfg.reg_ve_blue_stretch_err_cb);
			start = ve_bit_cfg.bit_bluestretch_err_cbp_inv.start;
			len = ve_bit_cfg.bit_bluestretch_err_cbp_inv.len;
			break;
		case EN_BLUE_STRETCH_ERR_CBN:
			addr = ADDR_PARAM(ve_reg_cfg.page,
				ve_reg_cfg.reg_ve_blue_stretch_err_cb);
			start = ve_bit_cfg.bit_bluestretch_err_cbn.start;
			len = ve_bit_cfg.bit_bluestretch_err_cbn.len;
			break;
		case EN_BLUE_STRETCH_ERR_SIGN0:
			start = ve_bit_cfg.bit_bluestretch_err_sign_0.start;
			len = ve_bit_cfg.bit_bluestretch_err_sign_0.len;
			break;
		case EN_BLUE_STRETCH_ERR_CBN_INV:
			addr = ADDR_PARAM(ve_reg_cfg.page,
				ve_reg_cfg.reg_ve_blue_stretch_err_cb);
			start = ve_bit_cfg.bit_bluestretch_err_cbn_inv.start;
			len = ve_bit_cfg.bit_bluestretch_err_cbn_inv.len;
			break;
		default:
			return;
		}

		WRITE_VPP_REG_BITS_BY_MODE(EN_MODE_DIR, addr, val, start, len);
	}
}

void vpp_module_ve_demo_center_bar_en(bool enable)
{
	_set_ve_demo_center_bar(enable,
		ve_bit_cfg.bit_demo_center_bar_en.start,
		ve_bit_cfg.bit_demo_center_bar_en.len);
}

void vpp_module_ve_set_demo_left_top_screen_width(int val)
{
	unsigned int addr = 0;
	enum io_mode_e io_mode = EN_MODE_DIR;

	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_demo_left_top_screen_width);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, val & 0x1fff);
}

void vpp_module_ve_clip_range_en(bool enable)
{
	unsigned int addr[2] = {0};
	unsigned int val[2] = {0};
	enum io_mode_e io_mode = EN_MODE_DIR;

	addr[0] = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_clip_misc0);
	addr[1] = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_clip_misc1);

	if (enable) { /*fix mbox av out flicker black dot*/
		/*cvbs output 16-235 16-240 16-240*/
		val[0] = 0x3acf03c0;
		val[1] = 0x04010040;
	} else { /*restore for other mode*/
		val[0] = 0x3fffffff;
		val[1] = 0x0;
	}

	WRITE_VPP_REG_BY_MODE(io_mode, addr[0], val[0]);
	WRITE_VPP_REG_BY_MODE(io_mode, addr[1], val[1]);
}

void vpp_module_ve_get_clip_range(int *top, int *bottom)
{
	unsigned int addr = 0;
	enum io_mode_e io_mode = EN_MODE_DIR;

	if (!top || !bottom)
		return;

	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_clip_misc0);
	*top = READ_VPP_REG_BY_MODE(io_mode, addr);

	addr = ADDR_PARAM(ve_reg_cfg.page,
		ve_reg_cfg.reg_ve_clip_misc1);
	*bottom = READ_VPP_REG_BY_MODE(io_mode, addr);
}

void vpp_module_ve_set_clock_ctrl(enum ve_clock_type_e type,
	int val, enum io_mode_e io_mode)
{
	unsigned int addr = 0;
	unsigned char start = 0;
	unsigned char len = 0;

	if (type == EN_CLK_MAX)
		return;

	cur_clk_ctrl_data[type] = val;

	if (io_mode == EN_MODE_DIR) {
		addr = ADDR_PARAM(ve_reg_cfg.page,
			ve_reg_cfg.reg_ve_enable_ctrl);

		switch (type) {
		case EN_CLK_BLUE_STRETCH:
			addr = ADDR_PARAM(ve_reg_cfg.page,
				ve_reg_cfg.reg_ve_gclk_ctrl0);
			start = ve_bit_cfg.bit_bluestretch_gclk_ctrl.start;
			len = ve_bit_cfg.bit_bluestretch_gclk_ctrl.len;
			break;
		case EN_CLK_CM:
			addr = ADDR_PARAM(ve_reg_cfg.page,
				ve_reg_cfg.reg_ve_gclk_ctrl1);
			start = ve_bit_cfg.bit_cm_gclk_ctrl.start;
			len = ve_bit_cfg.bit_cm_gclk_ctrl.len;
			break;
		case EN_CLK_CCORING:
			start = ve_bit_cfg.bit_ccoring_gclk_ctrl.start;
			len = ve_bit_cfg.bit_ccoring_gclk_ctrl.len;
			break;
		case EN_CLK_BLKEXT:
			start = ve_bit_cfg.bit_blkext_gclk_ctrl.start;
			len = ve_bit_cfg.bit_blkext_gclk_ctrl.len;
			break;
		default:
			return;
		}

		WRITE_VPP_REG_BITS_BY_MODE(EN_MODE_DIR, addr, val, start, len);
	}
}

void vpp_module_ve_on_vs(void)
{
	int i = 0;
	unsigned int update_flag = 0;
	unsigned int data32 = 0;
	unsigned int addr = 0;
	unsigned char start = 0;
	unsigned char len = 0;

	if (pre_ble_en != cur_ble_en) {
//		pr_vpp(PR_DEBUG_VE,
//			"[ve_on_vs] ble_en = %d -> %d\n",
//			pre_ble_en, cur_ble_en);

		pre_ble_en = cur_ble_en;
		_set_ve_enable_ctrl(cur_ble_en,
			ve_bit_cfg.bit_blkext_en.start,
			ve_bit_cfg.bit_blkext_en.len,
			EN_MODE_RDMA);
	}

	if (pre_cc_en != cur_cc_en) {
//		pr_vpp(PR_DEBUG_VE,
//			"[ve_on_vs] ccoring_en = %d -> %d\n",
//			pre_cc_en, cur_cc_en);

		pre_cc_en = cur_cc_en;
		_set_ve_enable_ctrl(cur_cc_en,
			ve_bit_cfg.bit_ccoring_en.start,
			ve_bit_cfg.bit_ccoring_en.len,
			EN_MODE_RDMA);
	}

	if (pre_bs_en != cur_bs_en) {
//		pr_vpp(PR_DEBUG_VE,
//			"[ve_on_vs] blue_stretch_en = %d -> %d\n",
//			pre_bs_en, cur_bs_en);

		pre_bs_en = cur_bs_en;
		_set_ve_blue_stretch_ctrl(cur_bs_en,
			ve_bit_cfg.bit_bluestretch_en.start,
			ve_bit_cfg.bit_bluestretch_en.len,
			EN_MODE_RDMA);
	}

	for (i = 0; i < EN_BLKEXT_MAX; i++) {
		if (pre_ble_data[i] != cur_ble_data[i]) {
			memcpy(pre_ble_data, cur_ble_data,
				sizeof(pre_ble_data));
			update_flag |= EN_UPDATE_BLE;
			break;
		}
	}

	for (i = 0; i < EN_CCORING_MAX; i++) {
		if (pre_cc_data[i] != cur_cc_data[i]) {
			memcpy(pre_cc_data, cur_cc_data,
				sizeof(pre_cc_data));
			update_flag |= EN_UPDATE_CC;
			break;
		}
	}

	for (i = 0; i < EN_BLUE_STRETCH_MAX; i++) {
		if (pre_bs_data[i] != cur_bs_data[i]) {
			memcpy(pre_bs_data, cur_bs_data,
				sizeof(pre_bs_data));
			update_flag |= EN_UPDATE_BS;
			break;
		}
	}

	for (i = 0; i < EN_CLK_MAX; i++) {
		if (pre_clk_ctrl_data[i] != cur_clk_ctrl_data[i]) {
			memcpy(pre_clk_ctrl_data, cur_clk_ctrl_data,
				sizeof(pre_clk_ctrl_data));
			update_flag |= EN_UPDATE_CLK_CTRL;
			break;
		}
	}

	if (update_flag & EN_UPDATE_BLE) {
		_set_ve_blkext_params(cur_ble_data, EN_MODE_RDMA);
		update_flag &= ~EN_UPDATE_BLE;
	}

	if (update_flag & EN_UPDATE_CC) {
		_set_ve_ccoring_params(cur_cc_data, EN_MODE_RDMA);
		update_flag &= ~EN_UPDATE_CC;
	}

	if (update_flag & EN_UPDATE_BS) {
		_set_ve_blue_stretch_params(cur_bs_data, EN_MODE_RDMA);
		update_flag &= ~EN_UPDATE_BS;
	}

	if (update_flag & EN_UPDATE_CLK_CTRL) {
		addr = ADDR_PARAM(ve_reg_cfg.page, ve_reg_cfg.reg_ve_gclk_ctrl0);
		data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
		start = ve_bit_cfg.bit_bluestretch_gclk_ctrl.start;
		len = ve_bit_cfg.bit_bluestretch_gclk_ctrl.len;
		data32 = vpp_insert_int(data32,
			cur_clk_ctrl_data[EN_CLK_BLUE_STRETCH], start, len);
		WRITE_VPP_REG_BY_MODE(EN_MODE_RDMA, addr, data32);

		addr = ADDR_PARAM(ve_reg_cfg.page, ve_reg_cfg.reg_ve_gclk_ctrl1);
		data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
		start = ve_bit_cfg.bit_cm_gclk_ctrl.start;
		len = ve_bit_cfg.bit_cm_gclk_ctrl.len;
		data32 = vpp_insert_int(data32,
			cur_clk_ctrl_data[EN_CLK_CM], start, len);
		WRITE_VPP_REG_BY_MODE(EN_MODE_RDMA, addr, data32);

		addr = ADDR_PARAM(ve_reg_cfg.page, ve_reg_cfg.reg_ve_enable_ctrl);
		data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
		start = ve_bit_cfg.bit_ccoring_gclk_ctrl.start;
		len = ve_bit_cfg.bit_ccoring_gclk_ctrl.len;
		data32 = vpp_insert_int(data32,
			cur_clk_ctrl_data[EN_CLK_CCORING], start, len);
		start = ve_bit_cfg.bit_blkext_gclk_ctrl.start;
		len = ve_bit_cfg.bit_blkext_gclk_ctrl.len;
		data32 = vpp_insert_int(data32,
			cur_clk_ctrl_data[EN_CLK_BLKEXT], start, len);
		WRITE_VPP_REG_BY_MODE(EN_MODE_RDMA, addr, data32);

		update_flag &= ~EN_UPDATE_CLK_CTRL;
	}
}

void vpp_module_ve_dump_info(enum vpp_dump_module_info_e info_type)
{
	if (info_type == EN_DUMP_INFO_REG)
		_dump_ve_reg_info();
	else
		_dump_ve_data_info();
}

#endif

