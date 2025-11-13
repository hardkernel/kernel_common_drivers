// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#include "../vpp_common.h"
#include "vpp_module_vadj.h"

enum _vadj_mode_e {
	EN_MODE_VADJ_01 = 0,
	EN_MODE_VADJ_02,
	EN_MODE_VADJ_MAX
};

struct _vadj_bit_cfg_s {
	struct _bit_s bit_vadj1_ctrl_en;
	struct _bit_s bit_vadj2_ctrl_en;
	struct _bit_s bit_vadj_brightness;
	struct _bit_s bit_vadj_contrast;
	struct _bit_s bit_vd1_rgbbst_en;
	struct _bit_s bit_post_rgbbst_en;
};

struct _vadj_reg_cfg_s {
	unsigned char page;
	unsigned char reg_vadj1_ctrl;
	unsigned char reg_vadj1_y;
	unsigned char reg_vadj1_ma_mb;
	unsigned char reg_vadj1_mc_md;
	unsigned char reg_vadj2_ctrl;
	unsigned char reg_vadj2_y;
	unsigned char reg_vadj2_ma_mb;
	unsigned char reg_vadj2_mc_md;
	unsigned char reg_vd1_rgb_ctrst;
	unsigned char reg_post_rgb_ctrst;
};

struct _vadj_data_s {
	unsigned int brightness;
	unsigned int contrast;
	unsigned int ma_mb;
	unsigned int mc_md;
};

/*Default table from T3*/
static struct _vadj_reg_cfg_s vadj_reg_cfg = {
	0x32,
	0x80,
	0x82,
	0x83,
	0x84,
	0xa0,
	0xa2,
	0xa3,
	0xa4,
	0x89,
	0xa9,
};

static struct _vadj_bit_cfg_s vadj_bit_cfg = {
	{0, 1},
	{0, 1},
	{8, 11},
	{0, 8},
	{1, 1},
	{1, 1},
};

static bool pre_vadj_en[EN_MODE_VADJ_MAX];
static bool cur_vadj_en[EN_MODE_VADJ_MAX];
static struct _vadj_data_s pre_vadj_data[EN_MODE_VADJ_MAX];
static struct _vadj_data_s cur_vadj_data[EN_MODE_VADJ_MAX];
static int pre_vadj_param[EN_VADJ_PARAM_MAX];
static int cur_vadj_param[EN_VADJ_PARAM_MAX];

/*For ai pq*/
static bool vadj_ai_pq_update;
static struct vadj_ai_pq_param_s vadj_ai_pq_offset;
static struct vadj_ai_pq_param_s vadj_ai_pq_base;

/*Internal functions*/
static int _set_vadj_ctrl(enum _vadj_mode_e mode, int val,
	unsigned char start, unsigned char len, enum io_mode_e io_mode)
{
	unsigned int addr = 0;

	switch (mode) {
	case EN_MODE_VADJ_01:
		addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj1_ctrl);
		break;
	case EN_MODE_VADJ_02:
		addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj2_ctrl);
		break;
	default:
		break;
	}

	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val, start, len);

	return 0;
}

static int _set_vadj_y(enum _vadj_mode_e mode, int val,
	unsigned char start, unsigned char len, enum io_mode_e io_mode)
{
	unsigned int addr = 0;

	switch (mode) {
	case EN_MODE_VADJ_01:
		addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj1_y);
		break;
	case EN_MODE_VADJ_02:
		addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj2_y);
		break;
	default:
		break;
	}

	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val, start, len);

	return 0;
}

static int _set_vadj_ma_mb(enum _vadj_mode_e mode, int val,
	enum io_mode_e io_mode)
{
	unsigned int addr = 0;

	switch (mode) {
	case EN_MODE_VADJ_01:
		addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj1_ma_mb);
		break;
	case EN_MODE_VADJ_02:
		addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj2_ma_mb);
		break;
	default:
		break;
	}

	WRITE_VPP_REG_BY_MODE(io_mode, addr, val);

	return 0;
}

static int _set_vadj_mc_md(enum _vadj_mode_e mode, int val,
	enum io_mode_e io_mode)
{
	unsigned int addr = 0;

	switch (mode) {
	case EN_MODE_VADJ_01:
		addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj1_mc_md);
		break;
	case EN_MODE_VADJ_02:
		addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj2_mc_md);
		break;
	default:
		break;
	}

	WRITE_VPP_REG_BY_MODE(io_mode, addr, val);

	return 0;
}

static void _dump_vadj_reg_info(void)
{
	PR_DRV("vadj_reg_cfg info:\n");
	PR_DRV("page = 0x%x\n", vadj_reg_cfg.page);
	PR_DRV("reg_vadj1_ctrl = 0x%x\n", vadj_reg_cfg.reg_vadj1_ctrl);
	PR_DRV("reg_vadj1_y = 0x%x\n", vadj_reg_cfg.reg_vadj1_y);
	PR_DRV("reg_vadj1_ma_mb = 0x%x\n", vadj_reg_cfg.reg_vadj1_ma_mb);
	PR_DRV("reg_vadj1_mc_md = 0x%x\n", vadj_reg_cfg.reg_vadj1_mc_md);
	PR_DRV("reg_vadj2_ctrl = 0x%x\n", vadj_reg_cfg.reg_vadj2_ctrl);
	PR_DRV("reg_vadj2_y = 0x%x\n", vadj_reg_cfg.reg_vadj2_y);
	PR_DRV("reg_vadj2_ma_mb = 0x%x\n", vadj_reg_cfg.reg_vadj2_ma_mb);
	PR_DRV("reg_vadj2_mc_md = 0x%x\n", vadj_reg_cfg.reg_vadj2_mc_md);
	PR_DRV("reg_vd1_rgb_ctrst = 0x%x\n", vadj_reg_cfg.reg_vd1_rgb_ctrst);
	PR_DRV("reg_post_rgb_ctrst = 0x%x\n", vadj_reg_cfg.reg_post_rgb_ctrst);
}

static void _dump_vadj_data_info(void)
{
	int i = 0;
	unsigned int addr = 0;
	unsigned int data32 = 0;

	PR_DRV("cur_vadj_data info:\n");

	for (i = EN_MODE_VADJ_01; i < EN_MODE_VADJ_MAX; i++) {
		PR_DRV("cur_vadj_en[%d] = %d\n", i, cur_vadj_en[i]);
		PR_DRV("brightness[%d] = 0x%08x\n", i, cur_vadj_data[i].brightness);
		PR_DRV("contrast[%d] = 0x%08x\n", i, cur_vadj_data[i].contrast);
		PR_DRV("ma_mb[%d] = 0x%08x\n", i, cur_vadj_data[i].ma_mb);
		PR_DRV("mc_md[%d] = 0x%08x\n", i, cur_vadj_data[i].mc_md);
	}

	for (i = EN_VADJ_VD1_RGBBST_EN; i < EN_VADJ_PARAM_MAX; i++)
		PR_DRV("cur_vadj_param[%d] = 0x%08x\n", i, cur_vadj_param[i]);

	PR_DRV("cur_vadj_reg_val info:\n");
	addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj1_ctrl);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj1_y);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj1_ma_mb);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj1_mc_md);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj2_ctrl);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj2_y);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj2_ma_mb);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vadj2_mc_md);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vd1_rgb_ctrst);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_post_rgb_ctrst);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);
}

/*External functions*/
int vpp_module_vadj_init(struct vpp_dev_s *dev)
{
	vadj_ai_pq_update = false;
	memset(&vadj_ai_pq_offset, 0, sizeof(vadj_ai_pq_offset));
	memset(&vadj_ai_pq_base, 0, sizeof(vadj_ai_pq_base));

	return 0;
}

void vpp_module_vadj_en(bool enable, enum io_mode_e io_mode)
{
	cur_vadj_en[EN_MODE_VADJ_01] = enable;

	if (io_mode == EN_MODE_DIR)
		_set_vadj_ctrl(EN_MODE_VADJ_01, enable,
			vadj_bit_cfg.bit_vadj1_ctrl_en.start,
			vadj_bit_cfg.bit_vadj1_ctrl_en.len,
			EN_MODE_DIR);
}

void vpp_module_vadj_post_en(bool enable, enum io_mode_e io_mode)
{
	cur_vadj_en[EN_MODE_VADJ_02] = enable;

	if (io_mode == EN_MODE_DIR)
		_set_vadj_ctrl(EN_MODE_VADJ_02, enable,
			vadj_bit_cfg.bit_vadj2_ctrl_en.start,
			vadj_bit_cfg.bit_vadj2_ctrl_en.len,
			EN_MODE_DIR);
}

void vpp_module_vadj_set_param(enum vadj_param_e index,
	int val, enum io_mode_e io_mode)
{
	unsigned int addr = 0;
	unsigned char start = 0;
	unsigned char len = 0;

	switch (index) {
	case EN_VADJ_VD1_RGBBST_EN:
		addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_vd1_rgb_ctrst);
		start = vadj_bit_cfg.bit_vd1_rgbbst_en.start;
		len = vadj_bit_cfg.bit_vd1_rgbbst_en.len;
		break;
	case EN_VADJ_POST_RGBBST_EN:
		addr = ADDR_PARAM(vadj_reg_cfg.page, vadj_reg_cfg.reg_post_rgb_ctrst);
		start = vadj_bit_cfg.bit_post_rgbbst_en.start;
		len = vadj_bit_cfg.bit_post_rgbbst_en.len;
		break;
	default:
		return;
	}

	cur_vadj_param[index] = val;

	if (io_mode == EN_MODE_DIR)
		WRITE_VPP_REG_BITS_BY_MODE(EN_MODE_DIR, addr, val, start, len);
}

int vpp_module_vadj_set_brightness(int val, enum io_mode_e io_mode)
{
	cur_vadj_data[EN_MODE_VADJ_01].brightness = val;

	if (io_mode == EN_MODE_DIR)
		_set_vadj_y(EN_MODE_VADJ_01, val,
			vadj_bit_cfg.bit_vadj_brightness.start,
			vadj_bit_cfg.bit_vadj_brightness.len,
			EN_MODE_DIR);

	return 0;
}

int vpp_module_vadj_set_brightness_post(int val,
	enum io_mode_e io_mode)
{
	cur_vadj_data[EN_MODE_VADJ_02].brightness = val;

	if (io_mode == EN_MODE_DIR)
		_set_vadj_y(EN_MODE_VADJ_02, val,
			vadj_bit_cfg.bit_vadj_brightness.start,
			vadj_bit_cfg.bit_vadj_brightness.len,
			EN_MODE_DIR);

	return 0;
}

int vpp_module_vadj_set_contrast(int val, enum io_mode_e io_mode)
{
	cur_vadj_data[EN_MODE_VADJ_01].contrast = val;

	if (io_mode == EN_MODE_DIR)
		_set_vadj_y(EN_MODE_VADJ_01, val,
			vadj_bit_cfg.bit_vadj_contrast.start,
			vadj_bit_cfg.bit_vadj_contrast.len,
			EN_MODE_DIR);

	return 0;
}

int vpp_module_vadj_set_contrast_post(int val,
	enum io_mode_e io_mode)
{
	cur_vadj_data[EN_MODE_VADJ_02].contrast = val;

	if (io_mode == EN_MODE_DIR)
		_set_vadj_y(EN_MODE_VADJ_02, val,
			vadj_bit_cfg.bit_vadj_contrast.start,
			vadj_bit_cfg.bit_vadj_contrast.len,
			EN_MODE_DIR);

	return 0;
}

int vpp_module_vadj_set_sat_hue(int val, enum io_mode_e io_mode)
{
	short mc = 0;
	short md = 0;
	int mab = 0;
	int mcd = 0;

	vadj_ai_pq_base.sat_hue_mad = val;

	mab = (val + vadj_ai_pq_offset.sat_hue_mad) & 0x03ff03ff;

	mc = (0 - (short)(val & 0x000003ff));    /* mc = -mb */
	mc = vpp_check_range(mc, (-512), 511);
	md = (short)((val & 0x03ff0000) >> 16);  /* md =  ma; */
	mcd = ((mc & 0x03ff) << 16) | (md & 0x03ff);

	cur_vadj_data[EN_MODE_VADJ_01].ma_mb = mab;
	cur_vadj_data[EN_MODE_VADJ_01].mc_md = mcd;

	if (io_mode == EN_MODE_DIR) {
		_set_vadj_ma_mb(EN_MODE_VADJ_01, mab, EN_MODE_DIR);
		_set_vadj_mc_md(EN_MODE_VADJ_01, mcd, EN_MODE_DIR);
	}

	return 0;
}

int vpp_module_vadj_set_sat_hue_post(int val,
	enum io_mode_e io_mode)
{
	short mc = 0;
	short md = 0;
	int mab = 0;
	int mcd = 0;

	mab = val & 0x03ff03ff;

	mc = (0 - (short)(val & 0x000003ff));    /* mc = -mb */
	mc = vpp_check_range(mc, (-512), 511);
	md = (short)((val & 0x03ff0000) >> 16);  /* md =  ma; */
	mcd = ((mc & 0x03ff) << 16) | (md & 0x03ff);

	cur_vadj_data[EN_MODE_VADJ_02].ma_mb = mab;
	cur_vadj_data[EN_MODE_VADJ_02].mc_md = mcd;

	if (io_mode == EN_MODE_DIR) {
		_set_vadj_ma_mb(EN_MODE_VADJ_02, mab, EN_MODE_DIR);
		_set_vadj_mc_md(EN_MODE_VADJ_02, mcd, EN_MODE_DIR);
	}

	return 0;
}

void vpp_module_vadj_on_vs(void)
{
	unsigned char i = 0;
	unsigned int addr = 0;
	unsigned char start = 0;
	unsigned char len = 0;

	for (i = EN_MODE_VADJ_01; i < EN_MODE_VADJ_MAX; i++) {
		if (pre_vadj_en[i] != cur_vadj_en[i]) {
//			pr_vpp(PR_DEBUG_VADJ,
//				"[vadj_on_vs] vadj_en[%d] = %d -> %d\n",
//				i, pre_vadj_en[i], cur_vadj_en[i]);

			pre_vadj_en[i] = cur_vadj_en[i];

			if (i == EN_MODE_VADJ_01) {
				start = vadj_bit_cfg.bit_vadj1_ctrl_en.start;
				len = vadj_bit_cfg.bit_vadj1_ctrl_en.len;
			} else {
				start = vadj_bit_cfg.bit_vadj2_ctrl_en.start;
				len = vadj_bit_cfg.bit_vadj2_ctrl_en.len;
			}

			_set_vadj_ctrl(i, cur_vadj_en[i],
				start, len, EN_MODE_RDMA);
		}

		if (pre_vadj_data[i].brightness != cur_vadj_data[i].brightness) {
//			pr_vpp(PR_DEBUG_VADJ,
//				"[vadj_on_vs] brightness[%d] = %d -> %d\n",
//				i, pre_vadj_data[i].brightness,
//				cur_vadj_data[i].brightness);

			pre_vadj_data[i].brightness = cur_vadj_data[i].brightness;

			start = vadj_bit_cfg.bit_vadj_brightness.start;
			len = vadj_bit_cfg.bit_vadj_brightness.len;

			_set_vadj_y(i, cur_vadj_data[i].brightness,
				start, len, EN_MODE_RDMA);
		}

		if (pre_vadj_data[i].contrast != cur_vadj_data[i].contrast) {
//			pr_vpp(PR_DEBUG_VADJ,
//				"[vadj_on_vs] contrast[%d] = %d -> %d\n",
//				i, pre_vadj_data[i].contrast,
//				cur_vadj_data[i].contrast);

			pre_vadj_data[i].contrast = cur_vadj_data[i].contrast;

			start = vadj_bit_cfg.bit_vadj_contrast.start;
			len = vadj_bit_cfg.bit_vadj_contrast.len;

			_set_vadj_y(i, cur_vadj_data[i].contrast,
				start, len, EN_MODE_RDMA);
		}

		if (pre_vadj_data[i].ma_mb != cur_vadj_data[i].ma_mb) {
//			pr_vpp(PR_DEBUG_VADJ,
//				"[vadj_on_vs] ma_mb[%d] = %d -> %d\n",
//				i, pre_vadj_data[i].ma_mb,
//				cur_vadj_data[i].ma_mb);

			pre_vadj_data[i].ma_mb = cur_vadj_data[i].ma_mb;

			_set_vadj_ma_mb(i, cur_vadj_data[i].ma_mb, EN_MODE_RDMA);
		}

		if (pre_vadj_data[i].mc_md != cur_vadj_data[i].mc_md) {
//			pr_vpp(PR_DEBUG_VADJ,
//				"[vadj_on_vs] mc_md[%d] = %d -> %d\n",
//				i, pre_vadj_data[i].mc_md,
//				cur_vadj_data[i].mc_md);

			pre_vadj_data[i].mc_md = cur_vadj_data[i].mc_md;

			_set_vadj_mc_md(i, cur_vadj_data[i].mc_md, EN_MODE_RDMA);
		}
	}

	for (i = EN_VADJ_VD1_RGBBST_EN; i < EN_VADJ_PARAM_MAX; i++) {
		if (pre_vadj_param[i] != cur_vadj_param[i]) {
//			pr_vpp(PR_DEBUG_VADJ,
//				"[vadj_on_vs] vadj_param[%d] = %d -> %d\n",
//				i, pre_vadj_param[i], cur_vadj_param[i]);

			pre_vadj_param[i] = cur_vadj_param[i];

			if (i == EN_VADJ_VD1_RGBBST_EN) {
				addr = ADDR_PARAM(vadj_reg_cfg.page,
					vadj_reg_cfg.reg_vd1_rgb_ctrst);
				start = vadj_bit_cfg.bit_vd1_rgbbst_en.start;
				len = vadj_bit_cfg.bit_vd1_rgbbst_en.len;
			} else {
				addr = ADDR_PARAM(vadj_reg_cfg.page,
					vadj_reg_cfg.reg_post_rgb_ctrst);
				start = vadj_bit_cfg.bit_post_rgbbst_en.start;
				len = vadj_bit_cfg.bit_post_rgbbst_en.len;
			}

			WRITE_VPP_REG_BITS_BY_MODE(EN_MODE_RDMA, addr,
				cur_vadj_param[i], start, len);
		}
	}

	if (vadj_ai_pq_update) {
		vpp_module_vadj_set_sat_hue(vadj_ai_pq_base.sat_hue_mad,
			EN_MODE_RDMA);
		vadj_ai_pq_update = false;
	}
}

/*For ai pq*/
void vpp_module_vadj_get_ai_pq_base(struct vadj_ai_pq_param_s *param)
{
	if (!param)
		return;

	param->sat_hue_mad = vadj_ai_pq_base.sat_hue_mad;
}

void vpp_module_vadj_set_ai_pq_offset(struct vadj_ai_pq_param_s *param)
{
	if (!param)
		return;

	if (vadj_ai_pq_offset.sat_hue_mad != param->sat_hue_mad) {
		vadj_ai_pq_offset.sat_hue_mad = param->sat_hue_mad;
		vadj_ai_pq_update = true;
	}
}

void vpp_module_vadj_dump_info(enum vpp_dump_module_info_e info_type)
{
	if (info_type == EN_DUMP_INFO_REG)
		_dump_vadj_reg_info();
	else
		_dump_vadj_data_info();
}

#endif

