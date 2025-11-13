// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#include "../vpp_common.h"
#include "../vpp_alg_fw_inc.h"
#include "vpp_module_dnlp.h"

#define DNLP_REG_SIZE 32

enum _dnlp_mode_e {
	EN_MODE_DNLP_VPP = 0,
	EN_MODE_DNLP_SR0,
	EN_MODE_DNLP_SR1,
	EN_MODE_DNLP_VSR,
	EN_MODE_DNLP_MAX,
};

struct _dnlp_bit_cfg_s {
	struct _bit_s bit_dnlp_ctrl_en;
};

struct _dnlp_reg_cfg_s {
	unsigned char page;
	unsigned char reg_dnlp_ctrl;
	unsigned char reg_dnlp_p00;
};

struct _dnlp_reg_cal_s {
	int len;
	int rate;
	int shift;
	int bit_shift;
	int range_up;
};

/*Default table from T3*/
static struct _dnlp_reg_cfg_s dnlp_reg_cfg[EN_MODE_DNLP_MAX] = {
	{0x1d, 0xa1, 0x81},
	{0x50, 0x45, 0x90},
	{0x52, 0x45, 0x90},
	{0x53, 0x03, 0x04}
};

static struct _dnlp_bit_cfg_s dnlp_bit_cfg = {
	{0, 1}
};

static unsigned int dnlp_ctrl_pnt_max[EN_MODE_DNLP_MAX] = {
	16,
	32,
	32,
	32
};

static enum _dnlp_mode_e cur_mode;
static struct _dnlp_reg_cal_s reg_cal;

static bool dnlp_bypass;
static bool pre_dnlp_en;
static bool cur_dnlp_en;

static bool dnlp_alg_insmod_status;
static int dnlp_data[DNLP_REG_SIZE << 1] = {0};
static int dnlp_reg[DNLP_REG_SIZE] = {0};
static int dnlp_reg_def_16p[DNLP_REG_SIZE >> 1] = {
	0x0b070400, 0x1915120e, 0x2723201c, 0x35312e2a,
	0x47423d38, 0x5b56514c, 0x6f6a6560, 0x837e7974,
	0x97928d88, 0xaba6a19c, 0xbfbab5b0, 0xcfccc9c4,
	0xdad7d5d2, 0xe6e3e0dd, 0xf2efece9, 0xfdfaf7f4
};

/*For ai pq*/
static struct dnlp_ai_pq_param_s dnlp_ai_pq_offset;
static struct dnlp_ai_pq_param_s dnlp_ai_pq_base;

/*Internal functions*/
static void _set_dnlp_ctrl(enum _dnlp_mode_e mode, int val,
	unsigned char start, unsigned char len, enum io_mode_e io_mode)
{
	unsigned int addr = 0;

	if (mode == EN_MODE_DNLP_MAX)
		return;

	addr = ADDR_PARAM(dnlp_reg_cfg[mode].page,
		dnlp_reg_cfg[mode].reg_dnlp_ctrl);

	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val, start, len);
}

static void _set_dnlp_data(enum _dnlp_mode_e mode, unsigned int *data,
	enum io_mode_e io_mode)
{
	int i = 0;
	unsigned int val = 0;
	unsigned int addr = 0;

	if (mode == EN_MODE_DNLP_MAX || !data)
		return;

	addr = ADDR_PARAM(dnlp_reg_cfg[mode].page,
		dnlp_reg_cfg[mode].reg_dnlp_p00);

	for (i = 0; i < dnlp_ctrl_pnt_max[mode]; i++) {
		val = data[i];
		WRITE_VPP_REG_BY_MODE(io_mode, addr + i, val);
	}
}

static void _dump_dnlp_reg_info(void)
{
	int i = 0;

	PR_DRV("dnlp_reg_cfg info:\n");
	for (i = 0; i < EN_MODE_DNLP_MAX; i++) {
		PR_DRV("dnlp type = %d\n", i);
		PR_DRV("page = 0x%x\n", dnlp_reg_cfg[i].page);
		PR_DRV("reg_dnlp_ctrl = 0x%x\n", dnlp_reg_cfg[i].reg_dnlp_ctrl);
		PR_DRV("reg_dnlp_p00 = 0x%x\n", dnlp_reg_cfg[i].reg_dnlp_p00);
	}
}

static void _dump_dnlp_data_info(void)
{
	int i = 0;

	PR_DRV("cur_mode = %d\n", cur_mode);
	PR_DRV("dnlp_bypass = %d\n", dnlp_bypass);

	PR_DRV("reg_cal.len = %d\n", reg_cal.len);
	PR_DRV("reg_cal.rate = %d\n", reg_cal.rate);
	PR_DRV("reg_cal.shift = %d\n", reg_cal.shift);
	PR_DRV("reg_cal.bit_shift = %d\n", reg_cal.bit_shift);
	PR_DRV("reg_cal.range_up = %d\n", reg_cal.range_up);

	PR_DRV("dnlp_data info:\n");
	for (i = 0; i < (DNLP_REG_SIZE << 1); i++)
		PR_DRV("[%d]0x%08x\n", i, dnlp_data[i]);

	PR_DRV("dnlp_reg data info:\n");
	for (i = 0; i < DNLP_REG_SIZE; i++)
		PR_DRV("[%d]0x%08x\n", i, dnlp_reg[i]);
}

static void _calculate_dnlp_reg(unsigned int *data_in,
	unsigned int *data_out)
{
	int i = 0;
	int j = 0;
	int k = 0;
	unsigned int val = 0;

	if (!data_in || !data_out)
		return;

	for (i = 0; i < reg_cal.len; i++) {
		data_out[i] = 0;
		k = i * reg_cal.rate;

		for (j = 0; j < reg_cal.rate; j++) {
			val = data_in[k + j] << reg_cal.shift;
			val = vpp_check_range(val, 0, reg_cal.range_up);
			data_out[i] |= val << (j * reg_cal.bit_shift);
		}
	}
}

/*External functions*/
int vpp_module_dnlp_init(struct vpp_dev_s *dev)
{
	enum vpp_chip_type_e chip_id;
	enum dnlp_hw_e dnlp_hw;

	dnlp_bypass = false;
	dnlp_alg_insmod_status = false;

	chip_id = dev->m_data->chip_id;
	dnlp_hw = dev->vpp_cfg_data.dnlp_hw;

	reg_cal.len = 32;
	reg_cal.rate = 2;
	reg_cal.shift = 2;
	reg_cal.range_up = 1023;
	reg_cal.bit_shift = 16;

	if (chip_id >= CHIP_T6D)
		reg_cal.bit_shift = 12;

	switch (dnlp_hw) {
	case VPP_DNLP:
		cur_mode = EN_MODE_DNLP_VPP;
		reg_cal.len = 16;
		reg_cal.rate = 4;
		reg_cal.shift = 0;
		reg_cal.bit_shift = 8;
		reg_cal.range_up = 255;
		break;
	case SR0_DNLP:
		cur_mode = EN_MODE_DNLP_SR0;
		dnlp_reg_cfg[cur_mode].page = 0x50;
		break;
	case SR1_DNLP:
		cur_mode = EN_MODE_DNLP_SR1;
		dnlp_reg_cfg[cur_mode].page = 0x52;
		break;
	case VSR_DNLP:
	default:
		cur_mode = EN_MODE_DNLP_VSR;
		dnlp_reg_cfg[cur_mode].page = 0x53;
		break;
	}

	dnlp_ctrl_pnt_max[cur_mode] = 32;

	memset(&dnlp_ai_pq_offset, 0, sizeof(dnlp_ai_pq_offset));
	dnlp_ai_pq_base.final_gain = 8;

	return 0;
}

void vpp_module_dnlp_en(bool enable, enum io_mode_e io_mode)
{
	if (!enable)
		dnlp_bypass = true;
	else
		dnlp_bypass = false;

	cur_dnlp_en = enable;

	if (io_mode == EN_MODE_DIR)
		_set_dnlp_ctrl(cur_mode, enable,
			dnlp_bit_cfg.bit_dnlp_ctrl_en.start,
			dnlp_bit_cfg.bit_dnlp_ctrl_en.len,
			EN_MODE_DIR);
}

void vpp_module_dnlp_set_default(void)
{
	if (!dnlp_alg_insmod_status || dnlp_bypass)
		return;

	_set_dnlp_data(EN_MODE_DNLP_VPP, &dnlp_reg_def_16p[0], EN_MODE_DIR);
}

void vpp_module_dnlp_on_vs(int hist_luma_sum,
	unsigned short *hist_data)
{
	if (pre_dnlp_en != cur_dnlp_en) {
//		pr_vpp(PR_DEBUG_DNLP,
//			"[dnlp_on_vs] dnlp_en = %d -> %d\n",
//			pre_dnlp_en, cur_dnlp_en);

		pre_dnlp_en = cur_dnlp_en;
		_set_dnlp_ctrl(cur_mode, cur_dnlp_en,
			dnlp_bit_cfg.bit_dnlp_ctrl_en.start,
			dnlp_bit_cfg.bit_dnlp_ctrl_en.len,
			EN_MODE_RDMA);
	}

	if (!dnlp_alg_insmod_status) {
		vpp_alg_fw_dnlp_init();
		dnlp_alg_insmod_status =
			vpp_alg_fw_dnlp_get_insmod_status();
	}

	if (!dnlp_alg_insmod_status || !hist_data)
		return;

	vpp_alg_fw_dnlp_en(!dnlp_bypass);
	vpp_alg_fw_dnlp_calculate_tgtx(hist_luma_sum, hist_data);

	if (!dnlp_bypass) {
		vpp_alg_fw_dnlp_get_final_curve(&dnlp_data[0]);
		_calculate_dnlp_reg(&dnlp_data[0], &dnlp_reg[0]);
		_set_dnlp_data(cur_mode, &dnlp_reg[0], EN_MODE_RDMA);
	}
}

void vpp_module_dnlp_get_sat_compensation(bool *sat_comp,
	int *sat_val)
{
	if (!sat_comp || !sat_val)
		return;

	vpp_alg_fw_dnlp_cal_sat_compensation(sat_comp, sat_val);
}

void vpp_module_dnlp_set_param(struct vpp_dnlp_curve_param_s *data)
{
	if (dnlp_bypass || !data)
		return;

	vpp_alg_fw_dnlp_set_param(data);

	dnlp_ai_pq_base.final_gain =
		data->param[EN_DNLP_FINAL_GAIN];
}

void vpp_module_dnlp_set_ble_whe(struct vpp_ble_whe_param_s *data)
{
	if (dnlp_bypass || !data)
		return;

	vpp_alg_fw_dnlp_set_ble_whe(data);
}

/*For ai pq*/
void vpp_module_dnlp_get_ai_pq_base(struct dnlp_ai_pq_param_s *param)
{
	if (!param)
		return;

	param->final_gain = dnlp_ai_pq_base.final_gain;
}

void vpp_module_dnlp_set_ai_pq_offset(struct dnlp_ai_pq_param_s *param)
{
	int offset = 0;
	struct dnlp_alg_param_ai_s data;

	if (!param)
		return;

	offset = param->final_gain;
	if (dnlp_ai_pq_offset.final_gain != offset) {
		dnlp_ai_pq_offset.final_gain = offset;
		data.param[EN_DNLP_ALG_AI_FINAL_GAIN] = offset;
		vpp_alg_fw_dnlp_set_ai_param(&data);
	}
}

void vpp_module_dnlp_dump_info(enum vpp_dump_module_info_e info_type)
{
	if (info_type == EN_DUMP_INFO_REG)
		_dump_dnlp_reg_info();
	else
		_dump_dnlp_data_info();
}

#endif

