// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#include "../vpp_common.h"
#include "vpp_module_sr.h"

#ifndef CAL_FMETER_SCORE
#define CAL_FMETER_SCORE(x, y, z) ({ \
	typeof(x) _x = x; \
	typeof(y) _y = y; \
	typeof(z) _z = z; \
	MAX(MIN(100, (_x * 1000) / (_x + _y + _z)), \
	MIN(100, (_x + _y) * 1000 / \
	(_x + _y + _z) / 3)); \
	})
#endif //CAL_FMETER_SCORE

#define FMETER_TUNING_SIZE_MAX (11)
#define LRHF_LPF_COEF_MASK (0x7ff)

struct _sr_fmeter_size_cfg_s {
	unsigned int sr_w;
	unsigned int sr_h;
};

struct _sr_bit_cfg_s {
	struct _bit_s bit_gclk_ctrl_sr0;
	struct _bit_s bit_gclk_ctrl_sr1;
	struct _bit_s bit_pk_cirhpcon2gain2;
	struct _bit_s bit_pk_cirbpcon2gain2;
	struct _bit_s bit_srsharp0_en;
	struct _bit_s bit_srsharp0_buf_en;
	struct _bit_s bit_srsharp1_en;
	struct _bit_s bit_srsharp1_buf_en;
	struct _bit_s bit_bp_final_gain;
	struct _bit_s bit_hp_final_gain;
	struct _bit_s bit_final_gain_rs;
	struct _bit_s bit_pk_os_up;
	struct _bit_s bit_pk_os_down;
	struct _bit_s bit_pk_nr_en;
	struct _bit_s bit_pk_en;
	struct _bit_s bit_adp_hcti_en;
	struct _bit_s bit_adp_hlti_en;
	struct _bit_s bit_adp_vlti_en;
	struct _bit_s bit_adp_vcti_en;
	struct _bit_s bit_demo_left_top_scrn_width;
	struct _bit_s bit_demo_hsvsharp_en;
	struct _bit_s bit_demo_disp_position;
	struct _bit_s bit_sr3_dejaggy_en;
	struct _bit_s bit_sr3_drtlpf_en;
	struct _bit_s bit_sr3_drtlpf_theta_en;
	struct _bit_s bit_sr3_dering_en;
	struct _bit_s bit_nrdeband_en0;
	struct _bit_s bit_nrdeband_en1;
	struct _bit_s bit_nrdeband_en10;
	struct _bit_s bit_nrdeband_en11;
	struct _bit_s bit_freq_meter_en;
	struct _bit_s bit_fmeter_mode;
	struct _bit_s bit_fmeter_win;
};

struct _sr_reg_cfg_s {
	unsigned char page[4];
	unsigned char reg_srscl_gclk_ctrl;
	unsigned char reg_srsharp0_ctrl;
	unsigned char reg_srsharp1_ctrl;
	unsigned char reg_pk_con_2cirhpgain_lmt;
	unsigned char reg_pk_con_2cirbpgain_lmt;
	unsigned char reg_pk_final_gain;
	unsigned char reg_pk_os_static;
	unsigned char reg_pk_nr_en;
	unsigned char reg_hcti_flt_clp_dc;
	unsigned char reg_hlti_flt_clp_dc;
	unsigned char reg_vlti_flt_con_clp;
	unsigned char reg_vcti_flt_con_clp;
	unsigned char reg_ro_fmeter_hcnt_type0;
	unsigned char reg_demo_ctrl;
	unsigned char reg_dej_ctrl;
	unsigned char reg_sr3_drtlpf_en;
	unsigned char reg_sr3_dering_ctrl;
	unsigned char reg_db_flt_ctrl;
	unsigned char reg_fmeter_ctrl;
	unsigned char reg_fmeter_win_hor;
	unsigned char reg_fmeter_win_ver;
	unsigned char reg_fmeter_coring;
	unsigned char reg_fmeter_ratio_h;
	unsigned char reg_fmeter_ratio_v;
	unsigned char reg_fmeter_ratio_d;
	unsigned char reg_fmeter_h_flt0_9tap_0;
	unsigned char reg_fmeter_h_flt0_9tap_1;
	unsigned char reg_fmeter_h_flt1_9tap_0;
	unsigned char reg_fmeter_h_flt1_9tap_1;
	unsigned char reg_fmeter_h_flt2_9tap_0;
	unsigned char reg_fmeter_h_flt2_9tap_1;
	unsigned char reg_sr7_pk_long_pf_gain;
};

/*Default table from T3*/
static struct _sr_reg_cfg_s sr_reg_cfg = {
	{0x50, 0x51, 0x52, 0x53},
	0x77,
	0x91,
	0x92,
	0x06,
	0x08,
	0x22,
	0x26,
	0x27,
	0x2e,
	0x34,
	0x3a,
	0x3f,
	0x46,
	0x56,
	0x64,
	0x66,
	0x6b,
	0x77,
	0x89,
	0x8a,
	0x8b,
	0x8c,
	0x8d,
	0x8e,
	0x8f,
	0x6b,
	0x6c,
	0x6d,
	0x6e,
	0x6f,
	0x70,
	0x34
};

static struct _sr_bit_cfg_s sr_bit_cfg = {
	{0, 8},
	{8, 8},
	{24, 8},
	{24, 8},
	{0, 1},
	{1, 1},
	{0, 1},
	{1, 1},
	{0, 8},
	{8, 8},
	{16, 2},
	{0, 10},
	{12, 10},
	{0, 1},
	{1, 1},
	{28, 1},
	{28, 1},
	{14, 1},
	{14, 1},
	{0, 13},
	{16, 1},
	{17, 2},
	{0, 1},
	{0, 3},
	{4, 3},
	{28, 3},
	{4, 1},
	{5, 1},
	{22, 1},
	{23, 1},
	{0, 1},
	{4, 4},
	{8, 4}
};

static bool fm_support;
static bool fm_enable;
static int fm_flag;
static int cur_sr_level;
static int pre_fm_level;
static int cur_fm_level;
static unsigned int fm_count;
static struct sr_fmeter_report_s fm_report;
static struct _sr_fmeter_size_cfg_s pre_fm_size_cfg[EN_MODE_SR_MAX];
static bool vsr_support;
static bool pre_sr_en[EN_MODE_SR_MAX];
static bool cur_sr_en[EN_MODE_SR_MAX];

/*For ai pq*/
static bool sr_ai_pq_update;
static struct sr_final_gain_param_s sr_ai_pq_offset;
static struct sr_final_gain_param_s sr_ai_pq_base;

/*Internal functions*/
static void _set_sr_pk_final_gain(enum sr_mode_e mode, int val,
	unsigned char start, unsigned char len, enum io_mode_e io_mode)
{
	unsigned int addr = 0;

	switch (mode) {
	case EN_MODE_SR_0:
	default:
		addr = ADDR_PARAM(sr_reg_cfg.page[0], sr_reg_cfg.reg_pk_final_gain);
		break;
	case EN_MODE_SR_1:
		addr = ADDR_PARAM(sr_reg_cfg.page[2], sr_reg_cfg.reg_pk_final_gain);
		break;
	}

	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val, start, len);
}

static void _set_sr_final_gain(enum io_mode_e io_mode)
{
	int val = 0;

	if (!vsr_support) {
		val = sr_ai_pq_base.final_gains[0] +
			sr_ai_pq_offset.final_gains[0];
		_set_sr_pk_final_gain(EN_MODE_SR_0, val,
			sr_bit_cfg.bit_hp_final_gain.start,
			sr_bit_cfg.bit_hp_final_gain.len,
			io_mode);

		val = sr_ai_pq_base.final_gains[1] +
			sr_ai_pq_offset.final_gains[1];
		_set_sr_pk_final_gain(EN_MODE_SR_1, val,
			sr_bit_cfg.bit_hp_final_gain.start,
			sr_bit_cfg.bit_hp_final_gain.len,
			io_mode);

		val = sr_ai_pq_base.final_gains[2] +
			sr_ai_pq_offset.final_gains[2];
		_set_sr_pk_final_gain(EN_MODE_SR_0, val,
			sr_bit_cfg.bit_bp_final_gain.start,
			sr_bit_cfg.bit_bp_final_gain.len,
			io_mode);

		val = sr_ai_pq_base.final_gains[3] +
			sr_ai_pq_offset.final_gains[3];
		_set_sr_pk_final_gain(EN_MODE_SR_1, val,
			sr_bit_cfg.bit_bp_final_gain.start,
			sr_bit_cfg.bit_bp_final_gain.len,
			io_mode);
	}
}

static void _set_sr_fmeter_size_config(enum io_mode_e io_mode,
	enum sr_mode_e mode, unsigned int sr_w, unsigned int sr_h)
{
	unsigned int addr = 0;
	unsigned int fm_x_st, fm_x_end, fm_y_st, fm_y_end;
	int tmp = 0;

	if (sr_w == pre_fm_size_cfg[mode].sr_w &&
		sr_h == pre_fm_size_cfg[mode].sr_h)
		return;

	switch (mode) {
	case EN_MODE_SR_0:
		tmp = 0;
		break;
	case EN_MODE_SR_1:
		tmp = 2;
		break;
	default:
		return;
	}

	pre_fm_size_cfg[mode].sr_w = sr_w;
	pre_fm_size_cfg[mode].sr_h = sr_h;

	fm_x_st = 0;
	fm_x_end = sr_w & 0x1fff;
	fm_y_st = 0;
	fm_y_end = sr_h & 0x1fff;

	addr = ADDR_PARAM(sr_reg_cfg.page[tmp],
		sr_reg_cfg.reg_fmeter_win_hor);
	WRITE_VPP_REG_BY_MODE(io_mode, addr,
		fm_x_st | fm_x_end << 16);

	addr = ADDR_PARAM(sr_reg_cfg.page[tmp],
		sr_reg_cfg.reg_fmeter_win_ver);
	WRITE_VPP_REG_BY_MODE(io_mode, addr,
		fm_y_st | fm_y_end << 16);
}

static void _set_sr_fmeter_calculate(enum sr_mode_e mode,
	int fmeter_score, unsigned int *fmeter_hcnt)
{
	unsigned char fmeter_score_unit;
	unsigned char fmeter_score_ten;
	unsigned char fmeter_score_hundred;

	fmeter_score_hundred = fmeter_score / 100;
	fmeter_score_ten =
		(fmeter_score - fmeter_score_hundred * 100) / 10;
	fmeter_score_unit =
		fmeter_score - fmeter_score_hundred * 100
		- fmeter_score_ten * 10;

	cur_fm_level = vpp_check_range(fmeter_score / 10, 0, 10);

	if (cur_fm_level == pre_fm_level) {
		fm_flag++;
	} else {
		fm_flag = 0;
		pre_fm_level = cur_fm_level;
	}
}

static void _set_sr_fmeter_tuning_table(enum io_mode_e io_mode)
{
}

static void _get_sr_fmeter_result(void)
{
	int i = 0;
	int j = 0;
	unsigned int index = 0;
	unsigned int addr = 0;
	enum io_mode_e io_mode = EN_MODE_DIR;

	for (i = 0; i < EN_MODE_SR_MAX; i++) {
		index = i * 2;
		addr = ADDR_PARAM(sr_reg_cfg.page[index],
			sr_reg_cfg.reg_ro_fmeter_hcnt_type0);

		for (j = 0; j < FMETER_HCNT_MAX; j++)
			fm_report.hcnt[i][j] = READ_VPP_REG_BY_MODE(io_mode, addr + j);
	}
}

static void _dump_sr_reg_info(void)
{
	int i = 0;

	PR_DRV("sr_reg_cfg info:\n");
	for (i = 0; i < 4; i++)
		PR_DRV("page[%d] = 0x%x\n", i, sr_reg_cfg.page[i]);

	PR_DRV("reg_srscl_gclk_ctrl = 0x%x\n",
		sr_reg_cfg.reg_srscl_gclk_ctrl);
	PR_DRV("reg_srsharp0_ctrl = 0x%x\n",
		sr_reg_cfg.reg_srsharp0_ctrl);
	PR_DRV("reg_srsharp1_ctrl = 0x%x\n",
		sr_reg_cfg.reg_srsharp1_ctrl);
	PR_DRV("reg_pk_con_2cirhpgain_lmt = 0x%x\n",
		sr_reg_cfg.reg_pk_con_2cirhpgain_lmt);
	PR_DRV("reg_pk_con_2cirbpgain_lmt = 0x%x\n",
		sr_reg_cfg.reg_pk_con_2cirbpgain_lmt);
	PR_DRV("reg_pk_final_gain = 0x%x\n",
		sr_reg_cfg.reg_pk_final_gain);
	PR_DRV("reg_pk_os_static = 0x%x\n",
		sr_reg_cfg.reg_pk_os_static);
}

static void _dump_sr_data_info(void)
{
	int i = 0;

	PR_DRV("fm_support = %d\n", fm_support);
	PR_DRV("fm_enable = %d\n", fm_enable);
	PR_DRV("fm_flag = %d\n", fm_flag);
	PR_DRV("cur_sr_level = %d\n", cur_sr_level);
	PR_DRV("pre_fm_level = %d\n", pre_fm_level);
	PR_DRV("cur_fm_level = %d\n", cur_fm_level);
	PR_DRV("fm_count = %d\n", fm_count);

	PR_DRV("pre_fm_size_cfg sr_w/h data info:\n");
	for (i = 0; i < EN_MODE_SR_MAX; i++)
		PR_DRV("%d\t%d\n", pre_fm_size_cfg[i].sr_w,
			pre_fm_size_cfg[i].sr_h);

	PR_DRV("sr ai pq info:\n");
	for (i = 0; i < 4; i++) {
		PR_DRV("base_final_gains[%d] = %d\n", i,
			sr_ai_pq_base.final_gains[i]);
		PR_DRV("offset_final_gain[%d] = %d\n", i,
			sr_ai_pq_offset.final_gains[i]);
	}
}

static void _sr_fmeter_init(enum sr_mode_e mode)
{
	int i = 0;
	unsigned int addr = 0;
	enum io_mode_e io_mode = EN_MODE_DIR;

	switch (mode) {
	case EN_MODE_SR_0:
	default:
		i = 0;
		break;
	case EN_MODE_SR_1:
		i = 2;
		addr = ADDR_PARAM(sr_reg_cfg.page[3],
			sr_reg_cfg.reg_sr7_pk_long_pf_gain);
		WRITE_VPP_REG_BY_MODE(io_mode, addr, 0x40404040);
		addr = ADDR_PARAM(sr_reg_cfg.page[2],
			sr_reg_cfg.reg_pk_os_static);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, 0x14,
			sr_bit_cfg.bit_pk_os_up.start,
			sr_bit_cfg.bit_pk_os_up.len);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, 0x14,
			sr_bit_cfg.bit_pk_os_down.start,
			sr_bit_cfg.bit_pk_os_down.len);
		break;
	}

	addr = ADDR_PARAM(sr_reg_cfg.page[i],
		sr_reg_cfg.reg_fmeter_ctrl);
	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, 0xe,
		sr_bit_cfg.bit_fmeter_mode.start,
		sr_bit_cfg.bit_fmeter_mode.len);
	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, 0xa,
		sr_bit_cfg.bit_fmeter_win.start,
		sr_bit_cfg.bit_fmeter_win.len);

	addr = ADDR_PARAM(sr_reg_cfg.page[i],
		sr_reg_cfg.reg_fmeter_coring);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, 0x04040404);

	addr = ADDR_PARAM(sr_reg_cfg.page[i],
		sr_reg_cfg.reg_fmeter_ratio_h);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, 0x00040404);

	addr = ADDR_PARAM(sr_reg_cfg.page[i],
		sr_reg_cfg.reg_fmeter_ratio_v);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, 0x00040404);

	addr = ADDR_PARAM(sr_reg_cfg.page[i],
		sr_reg_cfg.reg_fmeter_ratio_d);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, 0x00040404);

	addr = ADDR_PARAM(sr_reg_cfg.page[i + 1],
		sr_reg_cfg.reg_fmeter_h_flt0_9tap_0);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, 0xe11ddc24);

	addr = ADDR_PARAM(sr_reg_cfg.page[i + 1],
		sr_reg_cfg.reg_fmeter_h_flt0_9tap_1);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, 0x14);

	addr = ADDR_PARAM(sr_reg_cfg.page[i + 1],
		sr_reg_cfg.reg_fmeter_h_flt1_9tap_0);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, 0x20f0ed26);

	addr = ADDR_PARAM(sr_reg_cfg.page[i + 1],
		sr_reg_cfg.reg_fmeter_h_flt1_9tap_1);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, 0xf0);

	addr = ADDR_PARAM(sr_reg_cfg.page[i + 1],
		sr_reg_cfg.reg_fmeter_h_flt2_9tap_0);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, 0xe2e81932);

	addr = ADDR_PARAM(sr_reg_cfg.page[i + 1],
		sr_reg_cfg.reg_fmeter_h_flt2_9tap_1);
	WRITE_VPP_REG_BY_MODE(io_mode, addr, 0x04);
}

/*External functions*/
int vpp_module_sr_init(struct vpp_dev_s *dev)
{
	enum vpp_chip_type_e chip_id;

	chip_id = dev->m_data->chip_id;

	if (chip_id == CHIP_T3 ||
		chip_id == CHIP_T5W ||
		chip_id == CHIP_T5M) {
		fm_support = true;
		_sr_fmeter_init(EN_MODE_SR_0);
		_sr_fmeter_init(EN_MODE_SR_1);
	} else {
		fm_support = false;
	}

	fm_flag = 0;
	fm_enable = false;
	pre_fm_level = 0;
	cur_fm_level = 0;
	cur_sr_level = 5;

	memset(&fm_report, 0, sizeof(struct sr_fmeter_report_s));
	memset(&pre_fm_size_cfg, 0, sizeof(struct _sr_fmeter_size_cfg_s));

	sr_ai_pq_update = false;
	memset(&sr_ai_pq_offset, 0, sizeof(struct sr_final_gain_param_s));
	memset(&sr_ai_pq_base, 0, sizeof(struct sr_final_gain_param_s));

	return 0;
}

void vpp_module_sr_en(enum sr_mode_e mode, bool enable,
	enum io_mode_e io_mode)
{
	unsigned int addr = 0;
	int i = 0;

	switch (mode) {
	case EN_MODE_SR_0:
		i = 0;
		break;
	case EN_MODE_SR_1:
		i = 2;
		break;
	default:
		return;
	}

	cur_sr_en[mode] = enable;

	if (io_mode == EN_MODE_DIR) {
		addr = ADDR_PARAM(sr_reg_cfg.page[i],
			sr_reg_cfg.reg_pk_nr_en);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, enable,
			sr_bit_cfg.bit_pk_nr_en.start,
			sr_bit_cfg.bit_pk_nr_en.len);
	}
}

void vpp_module_sr_sub_module_en(enum sr_mode_e mode,
	enum sr_sub_module_e sub_module, bool enable)
{
	int i = 0;
	unsigned int addr = 0;
	enum io_mode_e io_mode = EN_MODE_DIR;
	unsigned int val = enable;

	switch (mode) {
	case EN_MODE_SR_0:
	default:
		i = 0;
		break;
	case EN_MODE_SR_1:
		i = 2;
		break;
	}

	switch (sub_module) {
	case EN_SUB_MD_PK_NR:
		addr = ADDR_PARAM(sr_reg_cfg.page[i],
			sr_reg_cfg.reg_pk_nr_en);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val,
			sr_bit_cfg.bit_pk_nr_en.start,
			sr_bit_cfg.bit_pk_nr_en.len);
		break;
	case EN_SUB_MD_PK:
		addr = ADDR_PARAM(sr_reg_cfg.page[i],
			sr_reg_cfg.reg_pk_nr_en);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val,
			sr_bit_cfg.bit_pk_en.start,
			sr_bit_cfg.bit_pk_en.len);
		break;
	case EN_SUB_MD_HCTI:
		addr = ADDR_PARAM(sr_reg_cfg.page[i],
			sr_reg_cfg.reg_hcti_flt_clp_dc);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val,
			sr_bit_cfg.bit_adp_hcti_en.start,
			sr_bit_cfg.bit_adp_hcti_en.len);
		break;
	case EN_SUB_MD_HLTI:
		addr = ADDR_PARAM(sr_reg_cfg.page[i],
			sr_reg_cfg.reg_hlti_flt_clp_dc);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val,
			sr_bit_cfg.bit_adp_hlti_en.start,
			sr_bit_cfg.bit_adp_hlti_en.len);
		break;
	case EN_SUB_MD_VLTI:
		addr = ADDR_PARAM(sr_reg_cfg.page[i],
			sr_reg_cfg.reg_vlti_flt_con_clp);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val,
			sr_bit_cfg.bit_adp_vlti_en.start,
			sr_bit_cfg.bit_adp_vlti_en.len);
		break;
	case EN_SUB_MD_VCTI:
		addr = ADDR_PARAM(sr_reg_cfg.page[i],
			sr_reg_cfg.reg_vcti_flt_con_clp);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val,
			sr_bit_cfg.bit_adp_vcti_en.start,
			sr_bit_cfg.bit_adp_vcti_en.len);
		break;
	case EN_SUB_MD_DEJAGGY:
		addr = ADDR_PARAM(sr_reg_cfg.page[i],
			sr_reg_cfg.reg_dej_ctrl);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val,
			sr_bit_cfg.bit_sr3_dejaggy_en.start,
			sr_bit_cfg.bit_sr3_dejaggy_en.len);
		break;
	case EN_SUB_MD_DRTLPF:
		addr = ADDR_PARAM(sr_reg_cfg.page[i],
			sr_reg_cfg.reg_sr3_drtlpf_en);
		if (enable)
			val = 0x7;
		else
			val = 0;
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val,
			sr_bit_cfg.bit_sr3_drtlpf_en.start,
			sr_bit_cfg.bit_sr3_drtlpf_en.len);
		break;
	case EN_SUB_MD_DRTLPF_THETA:
		addr = ADDR_PARAM(sr_reg_cfg.page[i],
			sr_reg_cfg.reg_sr3_drtlpf_en);
		if (enable)
			val = 0x7;
		else
			val = 0;
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val,
			sr_bit_cfg.bit_sr3_drtlpf_theta_en.start,
			sr_bit_cfg.bit_sr3_drtlpf_theta_en.len);
		break;
	case EN_SUB_MD_DERING:
		addr = ADDR_PARAM(sr_reg_cfg.page[i],
			sr_reg_cfg.reg_sr3_dering_ctrl);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val,
			sr_bit_cfg.bit_sr3_dering_en.start,
			sr_bit_cfg.bit_sr3_dering_en.len);
		break;
	case EN_SUB_MD_DEBAND:
		addr = ADDR_PARAM(sr_reg_cfg.page[i],
			sr_reg_cfg.reg_db_flt_ctrl);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val,
			sr_bit_cfg.bit_nrdeband_en0.start,
			sr_bit_cfg.bit_nrdeband_en0.len);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val,
			sr_bit_cfg.bit_nrdeband_en1.start,
			sr_bit_cfg.bit_nrdeband_en1.len);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val,
			sr_bit_cfg.bit_nrdeband_en10.start,
			sr_bit_cfg.bit_nrdeband_en10.len);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val,
			sr_bit_cfg.bit_nrdeband_en11.start,
			sr_bit_cfg.bit_nrdeband_en11.len);
		break;
	case EN_SUB_MD_FMETER:
		if (val > 0)
			fm_enable = true;
		else
			fm_enable = false;
		addr = ADDR_PARAM(sr_reg_cfg.page[i],
			sr_reg_cfg.reg_fmeter_ctrl);
		WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, val,
			sr_bit_cfg.bit_freq_meter_en.start,
			sr_bit_cfg.bit_freq_meter_en.len);
		break;
	default:
		break;
	}
}

void vpp_module_sr_set_demo_mode(bool enable, bool left_side)
{
	unsigned int addr = 0;
	enum io_mode_e io_mode = EN_MODE_DIR;

	addr = ADDR_PARAM(sr_reg_cfg.page[0],
		sr_reg_cfg.reg_demo_ctrl);
	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, 0x3c0,
		sr_bit_cfg.bit_demo_left_top_scrn_width.start,
		sr_bit_cfg.bit_demo_left_top_scrn_width.len);
	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, 2,
		sr_bit_cfg.bit_demo_disp_position.start,
		sr_bit_cfg.bit_demo_disp_position.len);
	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, enable,
		sr_bit_cfg.bit_demo_hsvsharp_en.start,
		sr_bit_cfg.bit_demo_hsvsharp_en.len);

	addr = ADDR_PARAM(sr_reg_cfg.page[2],
		sr_reg_cfg.reg_demo_ctrl);
	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, 0x780,
		sr_bit_cfg.bit_demo_left_top_scrn_width.start,
		sr_bit_cfg.bit_demo_left_top_scrn_width.len);
	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, 2,
		sr_bit_cfg.bit_demo_disp_position.start,
		sr_bit_cfg.bit_demo_disp_position.len);
	WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, enable,
		sr_bit_cfg.bit_demo_hsvsharp_en.start,
		sr_bit_cfg.bit_demo_hsvsharp_en.len);
}

/*val range is 0~255*/
void vpp_module_sr_set_final_gain(struct sr_final_gain_param_s *param,
	enum io_mode_e io_mode)
{
	if (!param)
		return;

	memcpy(&sr_ai_pq_base, param,
		sizeof(struct sr_final_gain_param_s));

	if (io_mode == EN_MODE_DIR) {
		_set_sr_final_gain(io_mode);
	} else {
		if (!sr_ai_pq_update)
			sr_ai_pq_update = true;
	}
}

bool vpp_module_sr_get_fmeter_support(void)
{
	return fm_support;
}

struct sr_fmeter_report_s *vpp_module_sr_get_fmeter_report(void)
{
	return &fm_report;
}

void vpp_module_sr_on_vs(struct sr_vs_param_s *vs_param)
{
	enum io_mode_e io_mode = EN_MODE_RDMA;
	enum sr_mode_e mode = EN_MODE_SR_0;
	unsigned int *fmeter_hcnt;
	unsigned int index = 0;
	unsigned int addr = 0;

	if (!vs_param)
		return;

	for (mode = EN_MODE_SR_0; mode < EN_MODE_SR_MAX; mode++) {
		if (pre_sr_en[mode] != cur_sr_en[mode]) {
//			pr_vpp(PR_DEBUG_SR,
//				"[sr_on_vs] sr_en[%d] = %d -> %d\n",
//				mode, pre_sr_en[mode], cur_sr_en[mode]);

			pre_sr_en[mode] = cur_sr_en[mode];

			if (mode == EN_MODE_SR_0)
				index = 0;
			else
				index = 2;

			addr = ADDR_PARAM(sr_reg_cfg.page[index],
				sr_reg_cfg.reg_pk_nr_en);
			WRITE_VPP_REG_BITS_BY_MODE(io_mode, addr, cur_sr_en[mode],
				sr_bit_cfg.bit_pk_nr_en.start,
				sr_bit_cfg.bit_pk_nr_en.len);
		}
	}

	if (fm_support && fm_enable) {
		if (vs_param->vf_height <= 1080 &&
			vs_param->vf_width <= 1920)
			mode = EN_MODE_SR_0;
		else
			mode = EN_MODE_SR_1;

		fmeter_hcnt = &fm_report.hcnt[mode][0];

		_set_sr_fmeter_size_config(io_mode, EN_MODE_SR_0,
			vs_param->vf_width, vs_param->vf_height);
		_set_sr_fmeter_size_config(io_mode, EN_MODE_SR_1,
			vs_param->sps_w_in, vs_param->sps_h_in);
		_set_sr_fmeter_calculate(mode, 0, fmeter_hcnt);
		_set_sr_fmeter_tuning_table(io_mode);

		_get_sr_fmeter_result();
	}

	if (sr_ai_pq_update) {
		_set_sr_final_gain(io_mode);
		sr_ai_pq_update = false;
	}
}

/*For ai pq*/
void vpp_module_sr_get_ai_pq_base(struct sr_final_gain_param_s *param)
{
	int i = 0;

	if (!param)
		return;

	for (i = 0; i < 4; i++)
		param->final_gains[i] = sr_ai_pq_base.final_gains[i];
}

void vpp_module_sr_set_ai_pq_offset(struct sr_final_gain_param_s *param)
{
	int i = 0;

	if (!param)
		return;

	for (i = 0; i < 4; i++) {
		if (sr_ai_pq_offset.final_gains[i] != param->final_gains[i]) {
			sr_ai_pq_update = true;
			break;
		}
	}

	memcpy(&sr_ai_pq_offset, param,
		sizeof(struct sr_final_gain_param_s));
}

void vpp_module_sr_dump_info(enum vpp_dump_module_info_e info_type)
{
	if (info_type == EN_DUMP_INFO_REG)
		_dump_sr_reg_info();
	else
		_dump_sr_data_info();
}

#endif

