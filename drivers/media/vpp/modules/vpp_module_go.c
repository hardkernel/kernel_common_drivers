// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#include "../vpp_common.h"
#include "vpp_module_go.h"

struct _go_bit_cfg_s {
	struct _bit_s bit_go_ctrl_en;
	struct _bit_s bit_go_sync_sel_en;
	struct _bit_s bit_go_gain[GO_PARAM_CNT];
	struct _bit_s bit_go_offset[GO_PARAM_CNT];
	struct _bit_s bit_go_pre_offset[GO_PARAM_CNT];
};

struct _go_reg_cfg_s {
	unsigned char page;
	unsigned char reg_go_ctrl0;
	unsigned char reg_go_ctrl1;
	unsigned char reg_go_ctrl2;
	unsigned char reg_go_ctrl3;
	unsigned char reg_go_ctrl4;
};

/*Default table from T3*/
static struct _go_reg_cfg_s go_reg_cfg = {
	0x1d,
	0x6a,
	0x6b,
	0x6c,
	0x6d,
	0x6e,
};

static struct _go_bit_cfg_s go_bit_cfg = {
	{31, 1},
	{30, 1},
	{
		{16, 11},
		{0, 11},
		{16, 11},
	},
	{
		{0, 11},
		{16, 11},
		{0, 11},
	},
	{
		{16, 11},
		{0, 11},
		{0, 11},
	}
};

static bool pre_go_en;
static struct vpp_module_go_s pre_go_data;

static bool cur_go_en;
static struct vpp_module_go_s cur_go_data = {
	{0, 0, 0},
	{1024, 1024, 1024},
	{0, 0, 0},
};

static void _dump_go_reg_info(void)
{
	PR_DRV("go_reg_cfg info:\n");
	PR_DRV("page = 0x%x\n", go_reg_cfg.page);
	PR_DRV("reg_gamma_ctrl = 0x%x\n", go_reg_cfg.reg_go_ctrl0);
	PR_DRV("reg_gamma_addr = 0x%x\n", go_reg_cfg.reg_go_ctrl1);
	PR_DRV("reg_gamma_data = 0x%x\n", go_reg_cfg.reg_go_ctrl2);
	PR_DRV("reg_gamma_ctrl = 0x%x\n", go_reg_cfg.reg_go_ctrl3);
	PR_DRV("reg_gamma_addr = 0x%x\n", go_reg_cfg.reg_go_ctrl4);
}

static void _dump_go_data_info(void)
{
	int i = 0;
	unsigned int addr = 0;
	unsigned int data32 = 0;

	PR_DRV("cur_go_data info:\n");
	PR_DRV("cur_go_en = %d\n", cur_go_en);

	for (i = 0; i < GO_PARAM_CNT; i++) {
		PR_DRV("pre_offset[%d] = %d\n", i, cur_go_data.pre_offset[i]);
		PR_DRV("gain[%d] = %d\n", i, cur_go_data.gain[i]);
		PR_DRV("post_offset[%d] = %d\n", i, cur_go_data.post_offset[i]);
	}

	PR_DRV("cur_go_reg_val info:\n");
	addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl0);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl1);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl2);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl3);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);

	addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl4);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	PR_DRV("[0x%04x] = 0x%08x\n", addr, data32);
}

/*External functions*/
int vpp_module_go_init(struct vpp_dev_s *dev)
{
	return 0;
}

void vpp_module_go_en(bool enable,
	enum io_mode_e io_mode)
{
	unsigned int addr = 0;
	unsigned char start = 0;
	unsigned char len = 0;

	cur_go_en = enable;

	if (io_mode == EN_MODE_DIR) {
		addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl0);
		start = go_bit_cfg.bit_go_ctrl_en.start;
		len = go_bit_cfg.bit_go_ctrl_en.len;

		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, enable, start, len);
	}
}

void vpp_module_go_set_gain(unsigned char idx, int val,
	enum io_mode_e io_mode)
{
	unsigned int addr = 0;
	unsigned char start = 0;
	unsigned char len = 0;

	if (idx > GO_PARAM_CNT - 1)
		return;

	cur_go_data.gain[idx] = val;

	if (io_mode == EN_MODE_DIR) {
		switch (idx) {
		case 0:
		case 1:
			addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl0);
			break;
		case 2:
			addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl1);
			break;
		default:
			return;
		}
		start = go_bit_cfg.bit_go_gain[idx].start;
		len = go_bit_cfg.bit_go_gain[idx].len;

		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val, start, len);
	}
}

void vpp_module_go_set_offset(unsigned char idx, int val,
	enum io_mode_e io_mode)
{
	unsigned int addr = 0;
	unsigned char start = 0;
	unsigned char len = 0;

	if (idx > GO_PARAM_CNT - 1)
		return;

	cur_go_data.post_offset[idx] = val;

	if (io_mode == EN_MODE_DIR) {
		switch (idx) {
		case 0:
			addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl1);
			break;
		case 1:
		case 2:
			addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl2);
			break;
		default:
			return;
		}

		start = go_bit_cfg.bit_go_offset[idx].start;
		len = go_bit_cfg.bit_go_offset[idx].len;

		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val, start, len);
	}
}

void vpp_module_go_set_pre_offset(unsigned char idx, int val,
	enum io_mode_e io_mode)
{
	unsigned int addr = 0;
	unsigned char start = 0;
	unsigned char len = 0;

	if (idx > GO_PARAM_CNT - 1)
		return;

	cur_go_data.pre_offset[idx] = val;

	if (io_mode == EN_MODE_DIR) {
		switch (idx) {
		case 0:
		case 1:
			addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl3);
			break;
		case 2:
			addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl4);
			break;
		default:
			return;
		}

		start = go_bit_cfg.bit_go_pre_offset[idx].start;
		len = go_bit_cfg.bit_go_pre_offset[idx].len;

		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val, start, len);
	}
}

void vpp_module_go_set_data(struct vpp_module_go_s *data,
	enum io_mode_e io_mode)
{
	int i = 0;

	if (!data)
		return;

	memcpy(&cur_go_data, data, sizeof(struct vpp_module_go_s));

	if (io_mode == EN_MODE_DIR) {
		for (i = 0; i < GO_PARAM_CNT; i++) {
			vpp_module_go_set_gain(i, cur_go_data.gain[i], EN_MODE_DIR);
			vpp_module_go_set_offset(i, cur_go_data.post_offset[i], EN_MODE_DIR);
			vpp_module_go_set_pre_offset(i, cur_go_data.pre_offset[i], EN_MODE_DIR);
		}
	}
}

void vpp_module_go_get_en(int *val)
{
	unsigned int addr = 0;
	unsigned int data32 = 0;
	unsigned char start = 0;
	unsigned char len = 0;

	if (!val)
		return;

	addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl0);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	start = go_bit_cfg.bit_go_ctrl_en.start;
	len = go_bit_cfg.bit_go_ctrl_en.len;
	*val = vpp_mask_int(data32, start, len);
}

void vpp_module_go_get_data(struct vpp_module_go_s *data)
{
	unsigned int addr = 0;
	unsigned int data32 = 0;
	unsigned char start = 0;
	unsigned char len = 0;

	if (!data)
		return;

	addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl0);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	start = go_bit_cfg.bit_go_gain[0].start;
	len = go_bit_cfg.bit_go_gain[0].len;
	data->gain[0] = vpp_mask_int(data32, start, len);

	start = go_bit_cfg.bit_go_gain[1].start;
	len = go_bit_cfg.bit_go_gain[1].len;
	data->gain[1] = vpp_mask_int(data32, start, len);

	addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl1);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	start = go_bit_cfg.bit_go_gain[2].start;
	len = go_bit_cfg.bit_go_gain[2].len;
	data->gain[2] = vpp_mask_int(data32, start, len);

	start = go_bit_cfg.bit_go_offset[0].start;
	len = go_bit_cfg.bit_go_offset[0].len;
	data->post_offset[0] = vpp_mask_int(data32, start, len);

	addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl2);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	start = go_bit_cfg.bit_go_offset[1].start;
	len = go_bit_cfg.bit_go_offset[1].len;
	data->post_offset[1] = vpp_mask_int(data32, start, len);

	start = go_bit_cfg.bit_go_offset[2].start;
	len = go_bit_cfg.bit_go_offset[2].len;
	data->post_offset[2] = vpp_mask_int(data32, start, len);

	addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl3);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	start = go_bit_cfg.bit_go_pre_offset[0].start;
	len = go_bit_cfg.bit_go_pre_offset[0].len;
	data->pre_offset[0] = vpp_mask_int(data32, start, len);

	start = go_bit_cfg.bit_go_pre_offset[1].start;
	len = go_bit_cfg.bit_go_pre_offset[1].len;
	data->pre_offset[1] = vpp_mask_int(data32, start, len);

	addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl4);
	data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
	start = go_bit_cfg.bit_go_pre_offset[2].start;
	len = go_bit_cfg.bit_go_pre_offset[2].len;
	data->pre_offset[2] = vpp_mask_int(data32, start, len);
}

void vpp_module_go_on_vs(void)
{
	int i = 0;
	bool update_flag = false;
	unsigned int data32 = 0;
	unsigned int addr = 0;
	unsigned char start = 0;
	unsigned char len = 0;

	if (pre_go_en != cur_go_en) {
//		pr_vpp(PR_DEBUG_GO,
//			"[go_on_vs] go_en = %d -> %d\n",
//			pre_go_en, cur_go_en);

		pre_go_en = cur_go_en;
		update_flag = true;
	} else {
		for (i = 0; i < GO_PARAM_CNT; i++) {
			if (pre_go_data.gain[i] != cur_go_data.gain[i]) {
				memcpy(&pre_go_data, &cur_go_data,
					sizeof(struct vpp_module_go_s));
				update_flag = true;
//				pr_vpp(PR_DEBUG_GO,
//					"[go_on_vs] update gain[%d]\n", i);
				break;
			}

			if (pre_go_data.post_offset[i] != cur_go_data.post_offset[i]) {
				memcpy(&pre_go_data, &cur_go_data,
					sizeof(struct vpp_module_go_s));
				update_flag = true;
//				pr_vpp(PR_DEBUG_GO,
//					"[go_on_vs] update post_offset[%d]\n", i);
				break;
			}

			if (pre_go_data.pre_offset[i] != cur_go_data.pre_offset[i]) {
				memcpy(&pre_go_data, &cur_go_data,
					sizeof(struct vpp_module_go_s));
				update_flag = true;
//				pr_vpp(PR_DEBUG_GO,
//					"[go_on_vs] update pre_offset[%d]\n", i);
				break;
			}
		}
	}

	if (update_flag) {
		addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl0);
		data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
		start = go_bit_cfg.bit_go_ctrl_en.start;
		len = go_bit_cfg.bit_go_ctrl_en.len;
		data32 = vpp_insert_int(data32, cur_go_en, start, len);
		start = go_bit_cfg.bit_go_gain[0].start;
		len = go_bit_cfg.bit_go_gain[0].len;
		data32 = vpp_insert_int(data32, cur_go_data.gain[0], start, len);
		start = go_bit_cfg.bit_go_gain[1].start;
		len = go_bit_cfg.bit_go_gain[1].len;
		data32 = vpp_insert_int(data32, cur_go_data.gain[1], start, len);
		WRITE_VPP_REG_BY_MODE(EN_MODE_RDMA, addr, data32);

		addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl1);
		data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
		start = go_bit_cfg.bit_go_gain[2].start;
		len = go_bit_cfg.bit_go_gain[2].len;
		data32 = vpp_insert_int(data32, cur_go_data.gain[2], start, len);
		start = go_bit_cfg.bit_go_offset[0].start;
		len = go_bit_cfg.bit_go_offset[0].len;
		data32 = vpp_insert_int(data32, cur_go_data.post_offset[0], start, len);
		WRITE_VPP_REG_BY_MODE(EN_MODE_RDMA, addr, data32);

		addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl2);
		data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
		start = go_bit_cfg.bit_go_offset[1].start;
		len = go_bit_cfg.bit_go_offset[1].len;
		data32 = vpp_insert_int(data32, cur_go_data.post_offset[1], start, len);
		start = go_bit_cfg.bit_go_offset[2].start;
		len = go_bit_cfg.bit_go_offset[2].len;
		data32 = vpp_insert_int(data32, cur_go_data.post_offset[2], start, len);
		WRITE_VPP_REG_BY_MODE(EN_MODE_RDMA, addr, data32);

		addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl3);
		start = go_bit_cfg.bit_go_pre_offset[0].start;
		len = go_bit_cfg.bit_go_pre_offset[0].len;
		data32 = vpp_insert_int(data32, cur_go_data.pre_offset[0], start, len);
		start = go_bit_cfg.bit_go_pre_offset[1].start;
		len = go_bit_cfg.bit_go_pre_offset[1].len;
		data32 = vpp_insert_int(data32, cur_go_data.pre_offset[1], start, len);
		WRITE_VPP_REG_BY_MODE(EN_MODE_RDMA, addr, data32);

		addr = ADDR_PARAM(go_reg_cfg.page, go_reg_cfg.reg_go_ctrl4);
		data32 = READ_VPP_REG_BY_MODE(EN_MODE_DIR, addr);
		start = go_bit_cfg.bit_go_pre_offset[2].start;
		len = go_bit_cfg.bit_go_pre_offset[2].len;
		data32 = vpp_insert_int(data32, cur_go_data.pre_offset[2], start, len);
		WRITE_VPP_REG_BY_MODE(EN_MODE_RDMA, addr, data32);
	}
}

void vpp_module_go_dump_info(enum vpp_dump_module_info_e info_type)
{
	if (info_type == EN_DUMP_INFO_REG)
		_dump_go_reg_info();
	else
		_dump_go_data_info();
}

#endif

