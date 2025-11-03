// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

/*#define DEBUG*/

#include <linux/of.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <sound/memalloc.h>
#include <linux/clk.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/suspend.h>
#include <linux/pinctrl/consumer.h>
#include <linux/amlogic/vad_api.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/amlogic/gki_module.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/vmalloc.h>

#include "pdm_hw_coeff.h"
#include "vad_hw_coeff.h"
#include "regs.h"

#define DEV_NAME	"vad_open"
#define DMA_BUFFER_BYTES_MAX (96 * 1024)
#define VAD_READ_FRAME_COUNT    (1024 / 2)

/* #define __VAD_DUMP_DATA__ */
#define VAD_DUMP_FILE_NAME "/data/vaddump.pcm"

static const char * const iomap_name[] = {
	"pdm_bus",
	"audiobus_base",
	"vad_base"
};

enum{
	IO_PDM_BUS = 0,
	IO_AUDIO_BUS,
	IO_VAD,
	IO_MAX,
};

void __iomem *aml_snd_reg_map[3];

struct vad_open {
	struct device *dev;
	struct input_dev *input_device;

	struct pinctrl *pdm_pins;

	struct clk *vad_srcpll;
	/* sel: fclk_div3(666M) */
	struct clk *sysclk_srcpll;
	/* consider same source with tdm, 48k(24576000) */
	struct clk *dclk_srcpll;

	struct work_struct work;
	struct work_struct deinit_work;

	char *buf;
	char *vad_whole_buf;
	struct task_struct *thread;

	struct snd_dma_buffer dma_buffer;
	unsigned int start_last;
	unsigned int end_last;
	unsigned int addr;
	unsigned int threshold;
	int lane_mask_in;

	/* alsa buffer switch to vad buffer */
	bool a2v_buf;

	/* vad flag interrupt */
	int irq_wakeup;
	/* frame sync interrupt */
	int irq_fs;
	/* data source select
	 * Data src sel:
	 * 0: tdmin_a;
	 * 1: tdmin_b;
	 * 2: tdmin_c;
	 * 3: spdifin;
	 * 4: pdmin;
	 * 5: loopback_b;
	 * 6: tdmin_lb;
	 * 7: loopback_a;
	 */
	int src;
	/* Enable */
	int en;

	int (*callback)(char *buf, int frames, int rate,
			int channels, int bitdepth);

	void *mic_src;
	int wakeup_sample_rate;
	unsigned int syssrc_clk_rate;
	bool wake_up_flag;
	int vad_fs_count;
	int wakeup_timeout_fs_count;
	int last_addr;
	int last_cur_addr;
	int curr_addr;

	int ch_num;
	int bit_depth;
	int rate;

#ifdef __VAD_DUMP_DATA__
	struct file *fp;
	loff_t pos;
#endif
};

static int aml_snd_read(u32 base_type, unsigned int reg, unsigned int *val)
{
	if (base_type < IO_MAX) {
		*val = readl((aml_snd_reg_map[base_type] + (reg << 2)));

		return 0;
	}

	return -1;
}

static void aml_snd_write(u32 base_type, unsigned int reg, unsigned int val)
{
	if (base_type < IO_MAX) {
		writel(val, (aml_snd_reg_map[base_type] + (reg << 2)));
#ifdef DEBUG
		register_debug(base_type, reg, val);
#endif
		return;
	}

	pr_err("write snd reg %x error\n", reg);
}

static void aml_snd_update_bits(u32 base_type, unsigned int reg,
				unsigned int mask, unsigned int val)
{
	if (base_type < IO_MAX) {
		unsigned int tmp, orig;

		if (aml_snd_read(base_type, reg, &orig) == 0) {
			tmp = orig & ~mask;
			tmp |= val & mask;
			aml_snd_write(base_type, reg, tmp);

			return;
		}
	}
	pr_err("write snd reg %x error\n", reg);
}

int aml_pdm_read(unsigned int reg)
{
	int ret, val = 0;

	ret = aml_snd_read(IO_PDM_BUS, reg, &val);
	if (ret) {
		pr_err("read pdm reg %x error %d\n", reg, ret);
		return -1;
	}
	return val;
}

void aml_pdm_write(unsigned int reg, unsigned int val)
{
	aml_snd_write(IO_PDM_BUS, reg, val);
}

void aml_pdm_update_bits(unsigned int reg, unsigned int mask,
			 unsigned int val)
{
	aml_snd_update_bits(IO_PDM_BUS, reg, mask, val);
}

int audiobus_read(unsigned int reg)
{
	int ret, val = 0;

	ret = aml_snd_read(IO_AUDIO_BUS, reg, &val);

	if (ret) {
		pr_err("read audio reg %x error %d\n", reg, ret);
		return -1;
	}
	return val;
}

void audiobus_write(unsigned int reg, unsigned int val)
{
	aml_snd_write(IO_AUDIO_BUS, reg, val);
}

void audiobus_update_bits(unsigned int reg, unsigned int mask,
			  unsigned int val)
{
	aml_snd_update_bits(IO_AUDIO_BUS, reg, mask, val);
}

int vad_read(unsigned int reg)
{
	int ret, val = 0;

	ret = aml_snd_read(IO_VAD, reg, &val);

	if (ret) {
		pr_err("read audio reg %x error %d\n", reg, ret);
		return -1;
	}
	return val;
}

void vad_write(unsigned int reg, unsigned int val)
{
	aml_snd_write(IO_VAD, reg, val);
}

void vad_update_bits(unsigned int reg, unsigned int mask,
		     unsigned int val)
{
	aml_snd_update_bits(IO_VAD, reg, mask, val);
}

void vad_set_ram_coeff(int len, int *params)
{
	int i, ctrl_v;

	for (i = 0; i < len; i++) {
		ctrl_v = 0x1 << 31 | (i << 0);
		vad_write(VAD_LUT_WR, params[i]);
		vad_write(VAD_LUT_CTRL, ctrl_v);
	}
}

/* parameters for downsample and emphasis filter */
void vad_set_de_params(int len, int *params)
{
	int i;

	for (i = 0; i < len; i++)
		vad_write(VAD_FIR_CTRL + i, params[i]);
}

/* Power detection */
void vad_set_pwd(void)
{
	/* frame for 32 ms */
	vad_write(VAD_FRAME_CTRL0,
		  0x2 << 30 |
		  0x1 << 24 |
		  0x1 << 16);

	vad_write(VAD_FRAME_CTRL1, 0x00000d65);
	vad_write(VAD_FRAME_CTRL2, 0xd00103ff);
}

void vad_set_cep(void)
{
	vad_write(VAD_CEP_CTRL0, 0x11050000);
	vad_write(VAD_CEP_CTRL1, 0x0000001b);
	vad_write(VAD_CEP_CTRL2, 0xc001fd);
	vad_write(VAD_CEP_CTRL3, 0x137f0000);
	vad_write(VAD_CEP_CTRL4, 0x186d0000);
	vad_write(VAD_CEP_CTRL5, 0xfd00f61);
	vad_write(VAD_DEC_CTRL, 0x10030001);
}

void vad_set_src(int src)
{
	audiobus_update_bits(EE_AUDIO_TOVAD_CTRL0,
		0x1f << 12,
		src << 12);
}

void vad_set_in(void)
{
	/* two channel enable */
	vad_write(VAD_IN_SEL0, 0x00000001);
	vad_write(VAD_IN_SEL1, 0x00000002);

	vad_write(VAD_TO_DDR, 0xa0000719);
}

void vad_set_enable(bool enable)
{
	audiobus_update_bits(EE_AUDIO_TOVAD_CTRL0,
		0x1 << 31 | 0x1 << 30,
		enable << 31 | 0x1 << 30);

	if (enable) {
		vad_write(VAD_TOP_CTRL0, 0x7ff);
		vad_write(VAD_TOP_CTRL0, 0x0);

		vad_write(VAD_TOP_CTRL1, 0xff);
		vad_write(VAD_TOP_CTRL1, 0x0);

		vad_update_bits(VAD_TOP_CTRL0,
			0xfff << 20,
			1 << 31 | /* vad_en */
			1 << 30 | /* dec_fir_en */
			1 << 29 | /* pre_emp_en */
			1 << 28 | /* pre_ram_en */
			1 << 27 | /* frame_his_en */
			1 << 23 | /* ceps_ceps_en */
			1 << 22 | /* ceps_spec_en */
			0 << 20   /* two_channel_en */
		);
	} else {
		vad_write(VAD_TOP_CTRL0, 0x0);

		vad_write(VAD_TOP_CTRL1, 0x0);
	}
}

void vad_set_two_channel_en(bool en)
{
	/* two_channel_en */
	vad_update_bits(VAD_TOP_CTRL0, 0x1 << 20, en << 20);
}

static int vad_wakeup_count;
static irqreturn_t vad_wakeup_isr(int irq, void *data)
{
	struct vad_open *p_vadopen = (struct vad_open *)data;

	if (p_vadopen->a2v_buf &&
		vad_wakeup_count % 20 == 0) {
		vad_wakeup_count++;
		pr_info("%s vad_wakeup_count:%d\n", __func__, vad_wakeup_count);
	}
	p_vadopen->wake_up_flag = true;
	wake_up_process(p_vadopen->thread);

	return IRQ_HANDLED;
}

static irqreturn_t vad_fs_isr(int irq, void *data)
{
	struct vad_open *p_vadopen = (struct vad_open *)data;

	if (p_vadopen->wake_up_flag) {
		p_vadopen->vad_fs_count = 0;
		p_vadopen->wake_up_flag = false;

	} else {
		p_vadopen->vad_fs_count++;
	}

	return IRQ_HANDLED;
}

void aml_toddr_set_fifos(void)
{
	audiobus_update_bits(EE_AUDIO_TODDR_A_CTRL1,
			0xfff << 12 | 0xf << 8,
			0x3f << 12 | 2 << 8);

	audiobus_update_bits(EE_AUDIO_TODDR_A_CTRL0, 0x1, 0x1);
}

int aml_toddr_set_buf(unsigned int start, unsigned int end)
{
	audiobus_write(EE_AUDIO_TODDR_A_START_ADDR, start);
	audiobus_write(EE_AUDIO_TODDR_A_FINISH_ADDR, end);

	audiobus_write(EE_AUDIO_TODDR_A_INIT_ADDR, start);

	return 0;
}

int aml_toddr_set_intrpt(unsigned int intrpt)
{
	audiobus_write(EE_AUDIO_TODDR_A_INT_ADDR, intrpt);
	audiobus_update_bits(EE_AUDIO_TODDR_A_CTRL0, 0xff << 16, 0x4 << 16);

	return 0;
}

enum toddr_src {
	TODDR_INVAL = -1,
	TDMIN_A = 0,
	TDMIN_B = 1,
	TDMIN_C = 2,
	SPDIFIN = 3,
	PDMIN = 4,
	FRATV = 5, /* NONE for axg, g12a, g12b */
	TDMIN_LB = 6,
	LOOPBACK_A = 7,
	FRHDMIRX = 8, /* from tl1 chipset*/
	LOOPBACK_B = 9,
	SPDIFIN_LB = 10,
	EARCRX_DMAC = 11,/* from sm1 chipset */
	FRHDMIRX_PAO = 12, /* tm2 */
	RESAMPLEA = 13,   /* t5 */
	RESAMPLEB = 14,
	VAD = 15,
	PDMIN_B = 16,
	TDMINB_LB = 17,
	TDMIN_D = 18,
	FRHDMIRX_B = 19,
	RESAMPLEC = 20,
	TODDR_SRC_MAX = 21
};

void aml_toddr_select_src(enum toddr_src src)
{
	audiobus_update_bits(EE_AUDIO_TODDR_A_CTRL1,
				 0x1f << 26,
				 4 << 26);
}

void aml_toddr_set_format(int channel)
{
	audiobus_update_bits(EE_AUDIO_TODDR_A_CTRL0,
		0x1 << 27 | 0x7 << 24 | 0x1fff << 3,
		0x1 << 27 | 0 << 24 | 0 << 13 |
		0x1f << 8 | 0x10 << 3);

	audiobus_update_bits(EE_AUDIO_TODDR_A_CHSYNC_CTRL,
			0x1 << 31 | 0xFF, 0x1 << 31 | (channel - 1));
}

void aml_toddr_enable(bool enable, int channel)
{
	audiobus_update_bits(EE_AUDIO_TODDR_A_CTRL0, 1 << 31, enable << 31);
	if (!enable) {
		audiobus_write(EE_AUDIO_TODDR_A_CTRL0, 0x0);
		audiobus_write(EE_AUDIO_TODDR_A_CTRL1, 0x0);
		audiobus_update_bits(EE_AUDIO_TODDR_A_CHSYNC_CTRL, 0x1 << 31 | 0xFF, (channel - 1));
	}
}

unsigned int aml_toddr_get_position(void)
{
	return audiobus_read(EE_AUDIO_TODDR_A_STATUS2);
}

void aml_pdm_ctrl(struct vad_open *p_vadopen)
{
	int ch_mask;

	//pdm open
	aml_pdm_write(PDM_CLKG_CTRL, 1);

	ch_mask = (1 << p_vadopen->ch_num) - 1;

	/* must be sure that clk and pdm is enable */
	aml_pdm_update_bits(PDM_CTRL,
				(0x7 << 28 | 0xff << 8 | 0xff << 0),
				/* invert the PDM_DCLK or not */
				(0 << 30) |
				/* output mode:  1: 24bits. 0: 32 bits */
				(1 << 29) |
				/* bypass mode.
				 * 1: bypass all filter. 0: normal mode.
				 */
				(0 << 28) |
				/* PDM channel reset. */
				(ch_mask << 8) |
				/* PDM channel enable */
				(ch_mask << 0)
				);

	aml_pdm_write(PDM_CHAN_CTRL, ((0xa << 24) |
					(0xa << 16) |
					(0xa << 8) |
					(0xa << 0)
		));
	aml_pdm_write(PDM_CHAN_CTRL1, ((0xa << 24) |
					(0xa << 16) |
					(0xa << 8) |
					(0xa << 0)
		));
}

int pdm_get_ors(int dclk_idx, int sample_rate)
{
	int osr = 0;

	if (dclk_idx == 1) {
		if (sample_rate == 16000)
			osr = 64;
		else if (sample_rate == 8000)
			osr = 128;
		else
			pr_err("%s, Not support rate:%d\n",
				__func__, sample_rate);
	} else if (dclk_idx == 2) {
		if (sample_rate == 16000)
			osr = 48;
		else if (sample_rate == 8000)
			osr = 96;
		else
			pr_err("%s, Not support rate:%d\n",
				__func__, sample_rate);
	} else if (dclk_idx == 3) {
		if (sample_rate == 32000)

			osr = 64;
		else if (sample_rate == 16000)
			osr = 128;
		else if (sample_rate == 8000)
			osr = 256;
		else
			pr_err("%s, Not support rate:%d\n",
				__func__, sample_rate);
	} else {
		if (sample_rate == 96000)
			osr = 32;
		else if (sample_rate == 48000)
			osr = 64;
		else if (sample_rate == 32000)
			osr = 96;
		else if (sample_rate == 24000)
			osr = 128;
		else if (sample_rate == 16000)
			osr = 192;
		else if (sample_rate == 8000)
			osr = 384;
		else
			pr_err("%s, Not support rate:%d\n",
				__func__, sample_rate);
	}

	return osr;
}

static void aml_pdm_HPF_coeff(int pdm_gain_index, int osr,
	int lpf1_len, int lpf2_len, int lpf3_len, int hpf_mode)
{
	s32 hcic_dn_rate;
	s32 hcic_tap_num;
	s32 hcic_gain;
	s32 f1_tap_num;
	s32 f2_tap_num;
	s32 f3_tap_num;
	s32 f1_rnd_mode;
	s32 f2_rnd_mode;
	s32 f3_rnd_mode;
	s32 f1_ds_rate;
	s32 f2_ds_rate;
	s32 f3_ds_rate;
	s32 hpf_en;
	s32 hpf_shift_step;
	s32 hpf_out_factor;
	s32 pdm_out_mode;

	switch (osr) {
	case 32:
		hcic_dn_rate = 0x4;
		hcic_gain	 = pdm_gain_table[pdm_gain_index][PDM_OSR_32];
		break;
	case 40:
		hcic_dn_rate = 0x5;
		hcic_gain	 = pdm_gain_table[pdm_gain_index][PDM_OSR_40];
		break;
	case 48:
		hcic_dn_rate = 0x6;
		hcic_gain	 = pdm_gain_table[pdm_gain_index][PDM_OSR_48];
		break;
	case 56:
		hcic_dn_rate = 0x7;
		hcic_gain	 = pdm_gain_table[pdm_gain_index][PDM_OSR_56];
		break;
	case 64:
		hcic_dn_rate = 0x0008;
		hcic_gain	 = pdm_gain_table[pdm_gain_index][PDM_OSR_64];
		break;
	case 96:
		hcic_dn_rate = 0x000c;
		hcic_gain	 = pdm_gain_table[pdm_gain_index][PDM_OSR_96];
		break;
	case 128:
		hcic_dn_rate = 0x0010;
		hcic_gain	 = pdm_gain_table[pdm_gain_index][PDM_OSR_128];
		break;
	case 192:
		hcic_dn_rate = 0x0018;
		hcic_gain	 = pdm_gain_table[pdm_gain_index][PDM_OSR_192];
		break;
	case 384:
		hcic_dn_rate = 0x0030;
		hcic_gain	 = pdm_gain_table[pdm_gain_index][PDM_OSR_384];
		break;
	default:
		pr_info("Not support osr:%d, translate to :192\n", osr);
		hcic_dn_rate = 0x0018;
		hcic_gain	 = pdm_gain_table[pdm_gain_index][PDM_OSR_192];
		break;
	}

	hcic_tap_num = 0x0007;
	f1_tap_num	 = lpf1_len;
	f2_tap_num	 = lpf2_len;
	f3_tap_num	 = lpf3_len;
	hpf_shift_step = (s32)hpf_mode;
	hpf_en		 = 1;
	pdm_out_mode	 = 0;
	hpf_out_factor = 0x8000;
	f1_rnd_mode  = 1;
	f2_rnd_mode  = 0;
	f3_rnd_mode  = 1;
	f1_ds_rate	 = 2;
	f2_ds_rate	 = 2;
	f3_ds_rate	 = 2;

	/* hcic */
	aml_pdm_write(PDM_HCIC_CTRL1,
		(0x80000000 |
		hcic_tap_num |
		(hcic_dn_rate << 4) |
		(hcic_gain << 16))
		);

	/* lpf */
	aml_pdm_write(PDM_F1_CTRL,
		(0x80000000 |
		f1_tap_num |
		(2 << 12) |
		(f1_rnd_mode << 16))
		);
	aml_pdm_write(PDM_F2_CTRL,
		(0x80000000 |
		f2_tap_num |
		(2 << 12) |
		(f2_rnd_mode << 16))
		);
	aml_pdm_write(PDM_F3_CTRL,
		(0x80000000 |
		f3_tap_num |
		(2 << 12) |
		(f3_rnd_mode << 16))
		);

	/* hpf */
	aml_pdm_write(PDM_HPF_CTRL,
		(hpf_out_factor |
		(hpf_shift_step << 16) |
		(hpf_en << 31))
		);
}

/* coefficient for LPF1,2,3 */
static void aml_pdm_LPF_coeff(int lpf1_len, const int *lpf1_coeff,
	int lpf2_len, const int *lpf2_coeff,
	int lpf3_len, const int *lpf3_coeff)
{
	int i;
	s32 data;
	s32 data_tmp;

	aml_pdm_write(PDM_COEFF_ADDR, 0);
	for (i = 0;
		i < lpf1_len;
		i++)
		aml_pdm_write(PDM_COEFF_DATA, lpf1_coeff[i]);
	for (i = 0;
		i < lpf2_len;
		i++)
		aml_pdm_write(PDM_COEFF_DATA, lpf2_coeff[i]);
	for (i = 0;
		i < lpf3_len;
		i++)
		aml_pdm_write(PDM_COEFF_DATA, lpf3_coeff[i]);

	aml_pdm_write(PDM_COEFF_ADDR, 0);
	for (i = 0; i < lpf1_len; i++) {
		data = aml_pdm_read(PDM_COEFF_DATA);
		data_tmp = lpf1_coeff[i];
		if (data != data_tmp) {
			pr_info("FAILED coeff data verified wrong!\n");
			pr_info("Coeff = %x\n", data);
			pr_info("DDR COEFF = %x\n", data_tmp);
		}
	}
	for (i = 0; i < lpf2_len; i++) {
		data = aml_pdm_read(PDM_COEFF_DATA);
		data_tmp = lpf2_coeff[i];
		if (data != data_tmp) {
			pr_info("FAILED coeff data verified wrong!\n");
			pr_info("Coeff = %x\n", data);
			pr_info("DDR COEFF = %x\n", data_tmp);
		}
	}
	for (i = 0; i < lpf3_len; i++) {
		data = aml_pdm_read(PDM_COEFF_DATA);
		data_tmp = lpf3_coeff[i];
		if (data != data_tmp) {
			pr_info("FAILED coeff data verified wrong!\n");
			pr_info("Coeff = %x\n", data);
			pr_info("DDR COEFF = %x\n", data_tmp);
		}
	}
}

void aml_pdm_filter_ctrl(int pdm_gain_index, int osr, int lpf_mode, int hpf_mode)
{
	int lpf1_len, lpf2_len, lpf3_len;
	const int *lpf1_coeff, *lpf2_coeff, *lpf3_coeff;

	/* select LPF coefficient
	 * For filter 1 and filter 3,
	 * it's only relative with coefficient mode
	 * For filter 2,
	 * it's only relative with osr and hcic stage number
	 */

	/* mode and its latency
	 *	mode	|	latency
	 *	0		|	47.7
	 *	1		|	38.7
	 *	2		|	26
	 *	3		|	20.4
	 *	4		|	15
	 */

	switch (osr) {
	case 32:
	case 64:
		lpf2_coeff = lpf2_osr64;
		break;
	case 96:
		lpf2_coeff = lpf2_osr96;
		break;
	case 128:
		lpf2_coeff = lpf2_osr128;
		break;
	case 192:
		lpf2_coeff = lpf2_osr192;
		break;
	case 256:
		lpf2_coeff = lpf2_osr256;
		break;
	case 384:
		lpf2_coeff = lpf2_osr384;
		break;
	default:
		pr_info("osr :%d , lpf2 uses default parameters with osr64\n",
			osr);
		lpf2_coeff = lpf2_osr64;
		break;
	}

	switch (lpf_mode) {
	case 0:
		lpf1_len = 110;
		lpf2_len = 33;
		lpf3_len = 147;
		lpf1_coeff = lpf1_mode0;
		lpf3_coeff = lpf3_mode0;
		break;
	case 1:
		lpf1_len = 87;
		lpf2_len = 33;
		lpf3_len = 117;
		lpf1_coeff = lpf1_mode1;
		lpf3_coeff = lpf3_mode1;
		break;
	case 2:
		lpf1_len = 87;
		lpf2_len = 33;
		lpf3_len = 66;
		lpf1_coeff = lpf1_mode2;
		lpf3_coeff = lpf3_mode2;
		break;
	case 3:
		lpf1_len = 65;
		lpf2_len = 33;
		lpf3_len = 49;
		lpf1_coeff = lpf1_mode3;
		lpf3_coeff = lpf3_mode3;
		break;
	case 4:
		lpf1_len = 43;
		lpf2_len = 33;
		lpf3_len = 32;
		lpf1_coeff = lpf1_mode4;
		lpf3_coeff = lpf3_mode4;
		break;
	case 5:
		lpf1_len = 65;
		lpf2_len = 33;
		lpf3_len = 76;
		lpf1_coeff = lpf1_mode3;
		lpf3_coeff = lpf3_mode5;
		break;
	default:
		pr_info("default mode 1, osr 64\n");
		lpf1_len = 87;
		lpf2_len = 33;
		lpf3_len = 117;
		lpf1_coeff = lpf1_mode1;
		lpf3_coeff = lpf3_mode1;
		break;
	}

	/* config filter */
	aml_pdm_HPF_coeff(pdm_gain_index,
		osr,
		lpf1_len,
		lpf2_len,
		lpf3_len, hpf_mode);

	aml_pdm_LPF_coeff(lpf1_len, lpf1_coeff,
		lpf2_len, lpf2_coeff,
		lpf3_len, lpf3_coeff);
}

void pdm_enable(int is_enable)
{
	aml_pdm_update_bits(PDM_CTRL,
		0x1 << 31,
		is_enable << 31);
}

void pdm_fifo_reset(void)
{
	/* PDM Asynchronous FIFO soft reset.
	 * write 1 to soft reset AFIFO
	 */
	aml_pdm_update_bits(PDM_CTRL,
		0x1 << 16,
		0 << 16);

	aml_pdm_update_bits(PDM_CTRL,
		0x1 << 16,
		0x1 << 16);
}

static void vad_deinit(struct vad_open *p_vadopen)
{
	/* free irq */
	free_irq(p_vadopen->irq_wakeup, p_vadopen);
	free_irq(p_vadopen->irq_fs, p_vadopen);

#ifdef __VAD_DUMP_DATA__
	if (p_vadopen->fp)
		filp_close(p_vadopen->fp, NULL);
#endif
	if (p_vadopen->thread) {
		kthread_stop(p_vadopen->thread);
		put_task_struct(p_vadopen->thread);
		p_vadopen->thread = NULL;
	}
	memset(p_vadopen->dma_buffer.area, 0x0, p_vadopen->dma_buffer.bytes);
	if (p_vadopen->buf)
		vfree(p_vadopen->buf);
	p_vadopen->buf = NULL;

	pdm_enable(false);
	aml_toddr_enable(false, p_vadopen->ch_num);
	vad_set_enable(false);
	pr_info("vad open module deinit success\n");
}

static void vad_input_device_init(struct vad_open *p_vadopen)
{
	p_vadopen->input_device->name = "vad_keypad";
	p_vadopen->input_device->phys = "vad_keypad/input3";
	p_vadopen->input_device->dev.parent = p_vadopen->dev;
	p_vadopen->input_device->id.bustype = BUS_ISA;
	p_vadopen->input_device->id.vendor  = 0x0001;
	p_vadopen->input_device->id.product = 0x0001;
	p_vadopen->input_device->id.version = 0x0100;
	p_vadopen->input_device->evbit[0] = BIT_MASK(EV_KEY);
	p_vadopen->input_device->keybit[BIT_WORD(BTN_0)] = BIT_MASK(BTN_0);
	set_bit(KEY_POWER, p_vadopen->input_device->keybit);
}

static void vad_key_report(struct vad_open *p_vadopen)
{
	if (!p_vadopen)
		return;

	input_event(p_vadopen->input_device, EV_KEY, KEY_POWER, 1);
	input_sync(p_vadopen->input_device);
	input_event(p_vadopen->input_device, EV_KEY, KEY_POWER, 0);
	input_sync(p_vadopen->input_device);
}

static void vad_notify_user_space(struct vad_open *p_vadopen)
{
	pr_info("Notify to wake up user space\n");

	pm_wakeup_hard_event(p_vadopen->dev);
	p_vadopen->addr = 0;
	vad_key_report(p_vadopen);
}

/* transfer audio data to algorithm
 * 0: no wake word found
 * 1: contained wake word, should wake up system
 */
static int count;
static int vad_transfer_data_to_algorithm(struct vad_open *p_vadopen,
	char *buf,
	int frames,
	int rate,
	int channels,
	int bitdepth)
{
	int ret = 0;

#if (defined CONFIG_VAD_WAKEUP_ASR)
	/* Do check by algorithm */
	if (count % 50 == 0)
		pr_info("jian frames %d\n", frames);
	ret = aml_asr_feed(buf, frames);
#endif
	count++;
	return ret;
}

/* Check buffer in kernel for VAD */
static int vad_engine_check(struct vad_open *p_vadopen, bool init)
{
	unsigned char *hwbuf;
	int bytes = 0, read_bytes = 0;
	int start, end, size;
	int chnum, bitdepth, rate, bytes_per_sample;
	int frame_count = VAD_READ_FRAME_COUNT;
	unsigned int timeout_cnt = 0;
	unsigned int timeout_max_cnt = 200;
	int ret;

	hwbuf = p_vadopen->dma_buffer.area;
	size  = p_vadopen->dma_buffer.bytes;
	start = p_vadopen->dma_buffer.addr;
	end   = start + size;

	chnum    = p_vadopen->ch_num;
	bitdepth = p_vadopen->bit_depth;
	rate     = p_vadopen->rate;

	/* bytes for each sample */
	bytes_per_sample = bitdepth >> 3;

	/* copy vad whole buffer when first detect voice */
	if (init) {
		int frame_count1 = p_vadopen->dma_buffer.bytes / chnum / bytes_per_sample;
		int send_size = frame_count1 * chnum * bytes_per_sample;
		int count = 0;

		for (;;) {
			count++;
			if (count > 200) {
				pr_info("%s:wait vad buffer time out!\n", __func__);
				return 0;
			}
			p_vadopen->curr_addr = aml_toddr_get_position();
			if (p_vadopen->curr_addr < start ||
			    start > end || p_vadopen->curr_addr > end)
				continue;
			break;
		}
		pr_info("%s copy vad whole buffer, start:%x, end:%x, curr_addr:%x, last_addr:%x\n",
			__func__, start, end, p_vadopen->curr_addr, p_vadopen->last_addr);
		memcpy(p_vadopen->vad_whole_buf, hwbuf + p_vadopen->curr_addr - start,
				end - p_vadopen->curr_addr);
		memcpy(p_vadopen->vad_whole_buf + end - p_vadopen->curr_addr, hwbuf,
				p_vadopen->curr_addr - start);

		p_vadopen->addr = p_vadopen->curr_addr;
		p_vadopen->last_addr = p_vadopen->addr;

		#ifdef __VAD_DUMP_DATA__
		p_vadopen->pos = p_vadopen->fp->f_pos;
		kernel_write(p_vadopen->fp,
			p_vadopen->vad_whole_buf + (p_vadopen->dma_buffer.bytes - send_size),
			send_size, &p_vadopen->pos);
		p_vadopen->fp->f_pos = p_vadopen->pos;
		#endif
		ret = vad_transfer_data_to_algorithm(p_vadopen,
			p_vadopen->vad_whole_buf + (p_vadopen->dma_buffer.bytes - send_size),
			frame_count1, rate, chnum, bitdepth);
		memset(p_vadopen->vad_whole_buf + (p_vadopen->dma_buffer.bytes - send_size),
				0x0, send_size);
		return ret;
	}

	read_bytes = frame_count * chnum * bytes_per_sample;

	do {
		if (kthread_should_stop())
			break;

		p_vadopen->curr_addr = aml_toddr_get_position();
		if (p_vadopen->last_cur_addr == p_vadopen->curr_addr)
			continue;
		p_vadopen->last_cur_addr = p_vadopen->curr_addr;
		if (p_vadopen->curr_addr < start || p_vadopen->curr_addr > end ||
			p_vadopen->last_addr < start || p_vadopen->last_addr > end) {
			pr_info("%s line:%d, start:%x,end:%x, addr:%x, curr_addr=%x, last_addr:%x\n",
				__func__, __LINE__,
				start,
				end,
				p_vadopen->addr,
				p_vadopen->curr_addr, p_vadopen->last_addr);
			p_vadopen->addr = p_vadopen->curr_addr;
			return 0;
		}

		bytes = (p_vadopen->curr_addr - p_vadopen->last_addr + size) % size;

		if (bytes < read_bytes) {
			/* can't use sleep as cpd is idle, timer is invalid */
			if (!kthread_should_stop())
				schedule();
			timeout_cnt++;
			if (timeout_cnt >= timeout_max_cnt)
				break;
		}
	} while (bytes < read_bytes);

	if (!p_vadopen->buf) {
		p_vadopen->buf = vmalloc(size);
		if (!p_vadopen->buf)
			return -ENOMEM;
	}
	memset(p_vadopen->buf, 0x0, size);

	pr_debug("start:%x,end:%x, curr_addr:%x, last_addr:%x, offset:%d, read_bytes:%d\n",
		start,
		end,
		p_vadopen->curr_addr,
		p_vadopen->last_addr,
		bytes,
		read_bytes);

	if (bytes > read_bytes) {
		if (p_vadopen->last_addr + bytes <= end) {
			memcpy(p_vadopen->buf,
				hwbuf + p_vadopen->last_addr - start,
				bytes);
			p_vadopen->addr = p_vadopen->last_addr + bytes;
		} else {
			int tmp_bytes = end - p_vadopen->last_addr;

			memcpy(p_vadopen->buf,
				hwbuf + p_vadopen->last_addr - start,
				tmp_bytes);
			memcpy(p_vadopen->buf + tmp_bytes,
				hwbuf,
				bytes - tmp_bytes);
			p_vadopen->addr = start + bytes - tmp_bytes;
		}
		frame_count = bytes / (chnum * bytes_per_sample);
	} else {
		if (p_vadopen->last_addr + read_bytes <= end) {
			memcpy(p_vadopen->buf,
				hwbuf + p_vadopen->last_addr - start,
				read_bytes);
			p_vadopen->addr = p_vadopen->last_addr + read_bytes;
		} else {
			int tmp_bytes = end - p_vadopen->last_addr;

			memcpy(p_vadopen->buf,
				hwbuf + p_vadopen->last_addr - start,
				tmp_bytes);
			memcpy(p_vadopen->buf + tmp_bytes,
				hwbuf,
				read_bytes - tmp_bytes);
			p_vadopen->addr = start + read_bytes - tmp_bytes;
		}
		frame_count = read_bytes / (chnum * bytes_per_sample);
	}

	p_vadopen->last_addr = p_vadopen->addr;
#ifdef __VAD_DUMP_DATA__
	p_vadopen->pos = p_vadopen->fp->f_pos;
	kernel_write(p_vadopen->fp, p_vadopen->buf, read_bytes, &p_vadopen->pos);
	p_vadopen->fp->f_pos = p_vadopen->pos;
#endif

	ret = vad_transfer_data_to_algorithm(p_vadopen,
		p_vadopen->buf, frame_count, rate, chnum, bitdepth);
	memset(p_vadopen->buf, 0x0, frame_count * chnum * bytes_per_sample);
	return ret;
}

static int vad_freeze_thread(void *data)
{
	struct vad_open *p_vadopen = data;
	bool init = true;

	if (!p_vadopen)
		return 0;

	current->flags |= PF_NOFREEZE;

	dev_info(p_vadopen->dev, "vad: freeze thread start\n");

	for (;;) {
		if (p_vadopen->vad_fs_count > p_vadopen->wakeup_timeout_fs_count &&
		    !p_vadopen->wake_up_flag) {
			dev_info(p_vadopen->dev, "vad: thread entry schedule\n");
			set_current_state(TASK_INTERRUPTIBLE);
			if (!kthread_should_stop())
				schedule();
			set_current_state(TASK_RUNNING);
			init = true;
			dev_info(p_vadopen->dev, "vad: thread wake up\n");
		}
		if (kthread_should_stop()) {
			pr_info("%s line:%d\n", __func__, __LINE__);
			break;
		}

		if (!p_vadopen || !p_vadopen->en) {
			schedule();
			continue;
		}

		if (vad_engine_check(p_vadopen, init) > 0) {
			dev_info(p_vadopen->dev, "vad: vad_notify_user_space\n");
			vad_notify_user_space(p_vadopen);
			schedule_work(&p_vadopen->deinit_work);
		}
		init = false;
		/* can't use sleep as cpd is idle, timer is invalid */
		if (!kthread_should_stop())
			schedule();
	}

	dev_info(p_vadopen->dev, "vad: freeze thread exit\n");
	return 0;
}

static void pm_work_func(struct work_struct *p_work)
{
	if (pm_suspend(1))
		pr_err("suspend freeze failed\n");
}

static void deinit_work_func(struct work_struct *p_work)
{
	struct vad_open *p_vadopen = container_of(p_work, struct vad_open, deinit_work);

	vad_deinit(p_vadopen);
}

static int vad_mode;
static int get_vadmode(char *str)
{
	if (strcmp("on", str) == 0) {
		vad_mode = 1;
		return 0;
	}
	pr_err("vad ffv_freeze: bad arg:%s\n", str);
	return -1;
}

early_param("ffv_freeze", get_vadmode);

static int vad_open_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource res;
	struct device_node *np, *child;
	struct vad_open *p_vadopen = NULL;
	int flag = IRQF_SHARED | IRQF_NO_SUSPEND;
	unsigned int osr = 192, lpf_filter_mode = 1, hpf_filter_mode = 6, dclk_idx = 3072000;
	unsigned int samplerate = 16000, pdm_gain_index = 4;
	int *p_de_coeff = vad_de_coeff_16k;
	int len_de = ARRAY_SIZE(vad_de_coeff_16k);
	int len_ram = ARRAY_SIZE(vad_ram_coeff);
	int *p_win_coeff = vad_ram_coeff;

	int i;
	int ret = 0;

	if (!vad_mode) {
		pr_info("vad not set enable\n");
		return 0;
	}

	p_vadopen = devm_kzalloc(&pdev->dev, sizeof(struct vad_open), GFP_KERNEL);
	if (!p_vadopen)
		return -ENOMEM;

	p_vadopen->dev = dev;

	p_vadopen->ch_num = 1;
	p_vadopen->bit_depth = 16;
	p_vadopen->rate = 16000;

	np = pdev->dev.of_node;
	for (i = 0; i < 3; i++) {
		child = of_get_child_by_name(np, iomap_name[i]);
		if (child) {
			if (of_address_to_resource(child, 0, &res)) {
				pr_err("%s could not get resource", __func__);
				break;
			}
			aml_snd_reg_map[i] =
				ioremap(res.start, resource_size(&res));
			pr_info("aml_snd_reg_map[%d], reg:%x, size:%x\n",
				i, (u32)res.start, (u32)resource_size(&res));
		}
	}

	p_vadopen->input_device = input_allocate_device();
	if (!p_vadopen->input_device) {
		kfree(p_vadopen);
		return -ENOMEM;
	}
	vad_input_device_init(p_vadopen);
	if (input_register_device(p_vadopen->input_device)) {
		input_free_device(p_vadopen->input_device);
		kfree(p_vadopen);
		return -EINVAL;
	}

	device_init_wakeup(dev, 1);

	/* pinmux */
	p_vadopen->pdm_pins = devm_pinctrl_get_select(&pdev->dev, "pdm_pins");
	if (IS_ERR(p_vadopen->pdm_pins)) {
		p_vadopen->pdm_pins = NULL;
		dev_warn(&pdev->dev, "Can't get pdm pinmux\n");
	}

	/* open gate */
	audiobus_write(EE_AUDIO_CLK_GATE_EN0, 0xffffffff);
	audiobus_write(EE_AUDIO_CLK_GATE_EN1, 0xffffffff);

	/* vad clock */
	p_vadopen->vad_srcpll = devm_clk_get(&pdev->dev, "vad_srcpll");
	if (IS_ERR(p_vadopen->vad_srcpll)) {
		dev_err(&pdev->dev,
			"Can't retrieve vad vad_srcpll clock\n");
		return PTR_ERR(p_vadopen->vad_srcpll);
	}

	/* enable vad parent clock */
	ret = clk_prepare_enable(p_vadopen->vad_srcpll);
	if (ret) {
		pr_err("Can't enable vad vad_srcpll: %d\n", ret);
		return -EINVAL;
	}

	//set vad clock tree, vad clock = 25000000, fclk_div5(400M)
	audiobus_write(EE_AUDIO_CLK_VAD_CTRL, 0x8700000F);

	p_vadopen->sysclk_srcpll = devm_clk_get(&pdev->dev, "sysclk_srcpll");
	if (IS_ERR(p_vadopen->sysclk_srcpll)) {
		dev_err(&pdev->dev,
			"Can't retrieve pll clock\n");
		ret = PTR_ERR(p_vadopen->sysclk_srcpll);
		return ret;
	}

	/* enable pdm sys parent clock */
	ret = clk_prepare_enable(p_vadopen->sysclk_srcpll);
	if (ret) {
		pr_err("Can't enable vad sysclk_srcpll: %d\n", ret);
		return ret;
	}

	//set pdm sys clock tree 133333351, fclk_div3(666M)
	audiobus_write(EE_AUDIO_CLK_PDMIN_CTRL1, 0x85000004);

	p_vadopen->dclk_srcpll = devm_clk_get(&pdev->dev, "dclk_srcpll");
	if (IS_ERR(p_vadopen->dclk_srcpll)) {
		dev_err(&pdev->dev,
			"Can't retrieve data src clock\n");
		ret = PTR_ERR(p_vadopen->dclk_srcpll);
		return ret;
	}

	clk_set_rate(p_vadopen->dclk_srcpll, 491520000);

	/* enable pdm dclk parent clock */
	ret = clk_prepare_enable(p_vadopen->dclk_srcpll);
	if (ret) {
		pr_err("Can't enable vad dclk_srcpll: %d\n", ret);
		return ret;
	}

	//set pdm dclk clock tree 3.072M, hifi(491.52)
	audiobus_write(EE_AUDIO_CLK_PDMIN_CTRL0, 0x8100009F);

	//vad dma budder
	ret = snd_dma_alloc_pages(SNDRV_DMA_TYPE_DEV,
				p_vadopen->dev,
				DMA_BUFFER_BYTES_MAX,
				&p_vadopen->dma_buffer);
	if (ret) {
		dev_err(p_vadopen->dev, "Cannot allocate buffer(s)\n");
		return ret;
	}

#if (defined CONFIG_VAD_WAKEUP_ASR)
		aml_asr_enable();
#endif

	/* time = 400 * 10ms = 4s */
	p_vadopen->wakeup_timeout_fs_count = 400;
	p_vadopen->vad_whole_buf = vmalloc(p_vadopen->dma_buffer.bytes);
	if (!p_vadopen->vad_whole_buf)
		return -ENOMEM;

	/* irqs */
	p_vadopen->irq_wakeup = platform_get_irq_byname(pdev, "irq_wakeup");
	if (p_vadopen->irq_wakeup < 0) {
		dev_err(dev, "Failed to get irq_wakeup:%d\n",
			p_vadopen->irq_wakeup);
		return -ENXIO;
	}
	p_vadopen->irq_fs = platform_get_irq_byname(pdev, "irq_frame_sync");
	if (p_vadopen->irq_fs < 0) {
		dev_err(dev, "Failed to get irq_frame_sync:%d\n",
			p_vadopen->irq_fs);
		return -ENXIO;
	}

	if (!p_vadopen->thread) {
		p_vadopen->thread =
			kthread_create(vad_freeze_thread, p_vadopen,
					   "vad_freeze_thread");
		if (IS_ERR(p_vadopen->thread)) {
			int err = PTR_ERR(p_vadopen->thread);

			p_vadopen->thread = NULL;
			dev_info(p_vadopen->dev,
					"vad freeze: Creating thread failed\n");
			return err;
		}
		get_task_struct(p_vadopen->thread);
		//vad_wakeup_count = 0;
	}
	p_vadopen->last_addr = 0;
	p_vadopen->last_cur_addr = 0;
	p_vadopen->curr_addr = 0;
#ifdef __VAD_DUMP_DATA__
	p_vadopen->fp = filp_open(VAD_DUMP_FILE_NAME, O_RDWR | O_CREAT, 0666);
	if (IS_ERR(p_vadopen->fp)) {
		pr_err("create file %s error/n", VAD_DUMP_FILE_NAME);
		return -1;
	}
	p_vadopen->pos = 0;
#endif

	ret = request_irq(p_vadopen->irq_wakeup,
			vad_wakeup_isr, flag, "vad_wakeup",
			p_vadopen);
	if (ret) {
		dev_err(p_vadopen->dev, "failed to claim irq_wakeup %u, ret: %d\n",
					p_vadopen->irq_wakeup, ret);
		return -ENXIO;
	}

	ret = request_irq(p_vadopen->irq_fs,
			vad_fs_isr, flag, "vad_fs",
			p_vadopen);
	if (ret) {
		dev_err(p_vadopen->dev, "failed to claim irq_fs %u, ret: %d\n",
					p_vadopen->irq_fs, ret);
		return -ENXIO;
	}

	aml_pdm_ctrl(p_vadopen);

	/* filter for pdm */
	osr = pdm_get_ors(dclk_idx, samplerate);

	aml_pdm_filter_ctrl(pdm_gain_index, osr, lpf_filter_mode, hpf_filter_mode);

	pdm_fifo_reset();

	vad_set_enable(true);
	vad_set_ram_coeff(len_ram, p_win_coeff);
	vad_set_de_params(len_de, p_de_coeff);
	vad_set_pwd();
	vad_set_cep();
	vad_set_src(PDMIN);
	vad_set_in();

	/* reset then enable VAD */
	vad_set_enable(false);
	vad_set_enable(true);

	aml_toddr_set_fifos();
	aml_toddr_set_buf(p_vadopen->dma_buffer.addr,
			p_vadopen->dma_buffer.addr + p_vadopen->dma_buffer.bytes - 8);
	aml_toddr_set_intrpt(0x200);
	aml_toddr_select_src(PDMIN);
	aml_toddr_set_format(p_vadopen->ch_num);
	aml_toddr_enable(true, p_vadopen->ch_num);

	INIT_WORK(&p_vadopen->deinit_work, deinit_work_func);

	pdm_enable(true);
	p_vadopen->en = true;

	//freeze system
	INIT_WORK(&p_vadopen->work, pm_work_func);
	schedule_work(&p_vadopen->work);

	dev_err(dev, "vad open success\n");

	return 0;
}

static const struct of_device_id vad_open_dt_match[] = {
	{ .compatible = "amlogic, vad-open" },
	{},
};

static  struct platform_driver vad_open_platform_driver = {
	.probe		= vad_open_probe,
	.driver		= {
		.owner		    = THIS_MODULE,
		.name		    = DEV_NAME,
		.of_match_table	= vad_open_dt_match,
	},
};

int __init auge_vad_open_init(void)
{
	return platform_driver_register(&vad_open_platform_driver);
}

void __exit auge_vad_open_exit(void)
{
	platform_driver_unregister(&vad_open_platform_driver);
}

#ifdef MODULE
module_init(auge_vad_open_init);
module_exit(auge_vad_open_exit);
MODULE_LICENSE("GPL v2");
#endif
