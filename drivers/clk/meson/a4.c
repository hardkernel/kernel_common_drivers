// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/clk-provider.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/module.h>
#ifdef CONFIG_AMLOGIC_CLK_DEBUG
#include <linux/clkdev.h>
#endif
#include <linux/of_address.h>
#include <linux/of_device.h>

#include "meson-clkc-utils.h"
#include "clk-pll.h"
#include "clk-regmap.h"
#include "clk-cpu-dyndiv.h"
#include "clk-dualdiv.h"
#include "a4.h"
#include <dt-bindings/clock/amlogic,a4-clkc.h>

static const struct pll_params_table gp0_pll_table[] = {
	PLL_PARAMS_OD(62, 1, 1), /* DCO = 1488M OD = 1 PLL = 744M */
	PLL_PARAMS_OD(64, 1, 1), /* DCO = 1536M OD = 1 PLL = 768M */
	PLL_PARAMS_OD(66, 1, 1), /* DCO = 1584M OD = 1 PLL = 792M */
	PLL_PARAMS_OD(70, 1, 1), /* DCO = 1680M OD = 1 PLL = 840M */
	PLL_PARAMS_OD(96, 1, 1), /* DCO = 2304M OD = 1 PLL = 1152M */
	{ /* sentinel */ }
};

static const struct reg_sequence gp0_pll_init_regs[] = {
	{ .reg = ANACTRL_GP0PLL_CTRL1, .def = 0x03a00000 },
	{ .reg = ANACTRL_GP0PLL_CTRL2, .def = 0x00040000 },
	{ .reg = ANACTRL_GP0PLL_CTRL3, .def = 0x090da200 }
};

static struct clk_regmap gp0_pll = {
	.data = &(struct meson_clk_pll_data) {
		.en = {
			.reg_off = ANACTRL_GP0PLL_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.m = {
			.reg_off = ANACTRL_GP0PLL_CTRL0,
			.shift   = 0,
			.width   = 9,
		},
		.n = {
			.reg_off = ANACTRL_GP0PLL_CTRL0,
			.shift   = 16,
			.width   = 5,
		},
		.frac = {
			.reg_off = ANACTRL_GP0PLL_CTRL1,
			.shift   = 0,
			.width   = 17,
		},
		.od = {
			.reg_off = ANACTRL_GP0PLL_CTRL0,
			.shift   = 10,
			.width   = 3,
		},
		.l = {
			.reg_off = ANACTRL_GP0PLL_CTRL0,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = ANACTRL_GP0PLL_CTRL0,
			.shift   = 29,
			.width   = 1,
		},
		.l_rst = {
			.reg_off = ANACTRL_GP0PLL_CTRL3,
			.shift   = 9,
			.width   = 1,
		},
		.table = gp0_pll_table,
		.init_regs = gp0_pll_init_regs,
		.init_count = ARRAY_SIZE(gp0_pll_init_regs),
		.od_max = 4,
		.flags = CLK_MESON_PLL_L_RSTN |
			 CLK_MESON_PLL_FIXED_N,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "gp0_pll",
		.ops = &meson_clk_pll_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static const struct pll_mult_range hifi_pll_range = {
	.min = 67,
	.max = 133
};

static const struct reg_sequence hifi_pll_init_regs[] = {
	{ .reg = ANACTRL_HIFIPLL_CTRL1, .def = 0x03a00000 },
	{ .reg = ANACTRL_HIFIPLL_CTRL2, .def = 0x00040000 },
	{ .reg = ANACTRL_HIFIPLL_CTRL3, .def = 0x0a0da200 }
};

static struct clk_regmap hifi_pll = {
	.data = &(struct meson_clk_pll_data) {
		.en = {
			.reg_off = ANACTRL_HIFIPLL_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.m = {
			.reg_off = ANACTRL_HIFIPLL_CTRL0,
			.shift   = 0,
			.width   = 8,
		},
		.n = {
			.reg_off = ANACTRL_HIFIPLL_CTRL0,
			.shift   = 16,
			.width   = 5,
		},
		.frac = {
			.reg_off = ANACTRL_HIFIPLL_CTRL1,
			.shift   = 0,
			.width   = 17,
		},
		.od = {
			.reg_off = ANACTRL_HIFIPLL_CTRL0,
			.shift   = 10,
			.width   = 3,
		},
		.l = {
			.reg_off = ANACTRL_HIFIPLL_CTRL0,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = ANACTRL_HIFIPLL_CTRL0,
			.shift   = 29,
			.width   = 1,
		},
		.l_rst = {
			.reg_off = ANACTRL_HIFIPLL_CTRL3,
			.shift   = 9,
			.width   = 1,
		},
		.range = &hifi_pll_range,
		.init_regs = hifi_pll_init_regs,
		.init_count = ARRAY_SIZE(hifi_pll_init_regs),
		.od_max = 4,
		.flags = CLK_MESON_PLL_ROUND_CLOSEST |
			 CLK_MESON_PLL_FIXED_FRAC_WEIGHT_PRECISION |
			 CLK_MESON_PLL_FIXED_N |
			 CLK_MESON_PLL_L_RSTN,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hifi_pll",
		.ops = &meson_clk_pll_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor fclk50m_div = {
	.mult = 1,
	.div = 40,
	.hw.init = &(struct clk_init_data) {
		.name = "fclk50m_div",
		.ops = &clk_fixed_factor_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "fix_pll"
		},
		.num_parents = 1,
	},
};

static struct clk_regmap fclk50m = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = ANACTRL_FIXPLL_CTRL3,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "fclk50m",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&fclk50m_div.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor fclk_div2_div = {
	.mult = 1,
	.div = 2,
	.hw.init = &(struct clk_init_data) {
		.name = "fclk_div2_div",
		.ops = &clk_fixed_factor_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "fix_pll"
		},
		.num_parents = 1,
	},
};

static struct clk_regmap fclk_div2 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = ANACTRL_FIXPLL_CTRL3,
		.bit_idx = 20,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "fclk_div2",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&fclk_div2_div.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor fclk_div2p5_div = {
	.mult = 2,
	.div = 5,
	.hw.init = &(struct clk_init_data) {
		.name = "fclk_div2p5_div",
		.ops = &clk_fixed_factor_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "fix_pll"
		},
		.num_parents = 1,
	},
};

static struct clk_regmap fclk_div2p5 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = ANACTRL_FIXPLL_CTRL3,
		.bit_idx = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "fclk_div2p5",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&fclk_div2p5_div.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor fclk_div3_div = {
	.mult = 1,
	.div = 3,
	.hw.init = &(struct clk_init_data) {
		.name = "fclk_div3_div",
		.ops = &clk_fixed_factor_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "fix_pll"
		},
		.num_parents = 1,
	},
};

static struct clk_regmap fclk_div3 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = ANACTRL_FIXPLL_CTRL3,
		.bit_idx = 16,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "fclk_div3",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&fclk_div3_div.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor fclk_div4_div = {
	.mult = 1,
	.div = 4,
	.hw.init = &(struct clk_init_data) {
		.name = "fclk_div4_div",
		.ops = &clk_fixed_factor_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "fix_pll"
		},
		.num_parents = 1,
	},
};

static struct clk_regmap fclk_div4 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = ANACTRL_FIXPLL_CTRL3,
		.bit_idx = 17,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "fclk_div4",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&fclk_div4_div.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor fclk_div5_div = {
	.mult = 1,
	.div = 5,
	.hw.init = &(struct clk_init_data) {
		.name = "fclk_div5_div",
		.ops = &clk_fixed_factor_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "fix_pll"
		},
		.num_parents = 1,
	},
};

static struct clk_regmap fclk_div5 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = ANACTRL_FIXPLL_CTRL3,
		.bit_idx = 18,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "fclk_div5",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&fclk_div5_div.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor fclk_div7_div = {
	.mult = 1,
	.div = 7,
	.hw.init = &(struct clk_init_data) {
		.name = "fclk_div7_div",
		.ops = &clk_fixed_factor_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "fix_pll"
		},
		.num_parents = 1,
	},
};

static struct clk_regmap fclk_div7 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = ANACTRL_FIXPLL_CTRL3,
		.bit_idx = 19,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "fclk_div7",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&fclk_div7_div.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_regmap rtc_dual_clkin = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_RTC_BY_OSCIN_CTRL0,
		.bit_idx = 31,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "rtc_dual_clkin",
		.ops = &clk_regmap_gate_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "oscin",
		},
		.num_parents = 1,
	},
};

static const struct meson_clk_dualdiv_param rtc_dual_div_table[] = {
	{ 733, 732, 8, 11, 1 },
	{ }
};

static struct clk_regmap rtc_dual_div = {
	.data = &(struct meson_clk_dualdiv_data) {
		.n1 = {
			.reg_off = CLKCTRL_RTC_BY_OSCIN_CTRL0,
			.shift   = 0,
			.width   = 12,
		},
		.n2 = {
			.reg_off = CLKCTRL_RTC_BY_OSCIN_CTRL0,
			.shift   = 12,
			.width   = 12,
		},
		.m1 = {
			.reg_off = CLKCTRL_RTC_BY_OSCIN_CTRL1,
			.shift   = 0,
			.width   = 12,
		},
		.m2 = {
			.reg_off = CLKCTRL_RTC_BY_OSCIN_CTRL1,
			.shift   = 12,
			.width   = 12,
		},
		.dual = {
			.reg_off = CLKCTRL_RTC_BY_OSCIN_CTRL0,
			.shift   = 28,
			.width   = 2,
		},
		.table = rtc_dual_div_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "rtc_dual_div",
		.ops = &meson_clk_dualdiv_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&rtc_dual_clkin.hw,
		},
		.num_parents = 1,
	},
};

static const struct clk_parent_data rtc_dual_parent_data[] = {
	{ .hw = &rtc_dual_div.hw },
	{ .hw = &rtc_dual_clkin.hw }
};

static struct clk_regmap rtc_dual_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_RTC_BY_OSCIN_CTRL1,
		.mask = 0x1,
		.shift = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "rtc_dual_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = rtc_dual_parent_data,
		.num_parents = ARRAY_SIZE(rtc_dual_parent_data),
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap rtc_dual = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_RTC_BY_OSCIN_CTRL0,
		.bit_idx = 30,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "rtc_dual",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&rtc_dual_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data rtc_clk_parent_data[] = {
	{ .fw_name = "oscin" },
	{ .hw = &rtc_dual.hw }
};

static struct clk_regmap rtc_clk = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_RTC_CTRL,
		.mask = 0x3,
		.shift = 0,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "rtc_clk",
		.ops = &clk_regmap_mux_ops,
		.parent_data = rtc_clk_parent_data,
		.num_parents = ARRAY_SIZE(rtc_clk_parent_data),
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap clk_24m_in = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_CLK12_24_CTRL,
		.bit_idx = 11,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "clk_24m_in",
		.ops = &clk_regmap_gate_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "oscin",
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor clk_12m_div = {
	.mult = 1,
	.div = 2,
	.hw.init = &(struct clk_init_data) {
		.name = "clk_12m_div",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&clk_24m_in.hw,
		},
		.num_parents = 1,
	},
};

static const struct clk_parent_data clk_12m_parent_data[] = {
	{ .hw = &clk_24m_in.hw },
	{ .hw = &clk_12m_div.hw }
};

static struct clk_regmap clk_12m = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_CLK12_24_CTRL,
		.mask = 0x1,
		.shift = 10,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "clk_12m",
		.ops = &clk_regmap_mux_ops,
		.parent_data = clk_12m_parent_data,
		.num_parents = ARRAY_SIZE(clk_12m_parent_data),
	},
};

static struct clk_regmap clk_25m_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_CLK12_24_CTRL,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "clk_25m_div",
		.ops = &clk_regmap_divider_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "fix_pll"
		},
		.num_parents = 1,
	},
};

static struct clk_regmap clk_25m = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_CLK12_24_CTRL,
		.bit_idx = 12,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "clk_25m",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&clk_25m_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data sar_adc_parent_data[] = {
	{ .fw_name = "oscin" },
	{ .fw_name = "sys_clk" }
};

static struct clk_regmap sar_adc_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_SAR_CLK_CTRL0,
		.mask = 0x3,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sar_adc_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = sar_adc_parent_data,
		.num_parents = ARRAY_SIZE(sar_adc_parent_data),
	},
};

static struct clk_regmap sar_adc_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_SAR_CLK_CTRL0,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sar_adc_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&sar_adc_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap sar_adc = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_SAR_CLK_CTRL0,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sar_adc",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&sar_adc_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data pwm_parent_data[] = {
	{ .fw_name = "oscin" },
	{ .hw = &rtc_clk.hw },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div3.hw }
};

static struct clk_regmap pwm_a_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_AB_CTRL,
		.mask = 0x3,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_a_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = pwm_parent_data,
		.num_parents = ARRAY_SIZE(pwm_parent_data),
	},
};

static struct clk_regmap pwm_a_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_AB_CTRL,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_a_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_a_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_a = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_AB_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_a",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_a_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_b_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_AB_CTRL,
		.mask = 0x3,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_b_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = pwm_parent_data,
		.num_parents = ARRAY_SIZE(pwm_parent_data),
	},
};

static struct clk_regmap pwm_b_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_AB_CTRL,
		.shift = 16,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_b_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_b_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_b = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_AB_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_b",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_b_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_c_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_CD_CTRL,
		.mask = 0x3,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_c_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = pwm_parent_data,
		.num_parents = ARRAY_SIZE(pwm_parent_data),
	},
};

static struct clk_regmap pwm_c_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_CD_CTRL,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_c_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_c_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_c = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_CD_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_c",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_c_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_d_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_CD_CTRL,
		.mask = 0x3,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_d_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = pwm_parent_data,
		.num_parents = ARRAY_SIZE(pwm_parent_data),
	},
};

static struct clk_regmap pwm_d_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_CD_CTRL,
		.shift = 16,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_d_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_d_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_d = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_CD_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_d",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_d_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_e_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_EF_CTRL,
		.mask = 0x3,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_e_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = pwm_parent_data,
		.num_parents = ARRAY_SIZE(pwm_parent_data),
	},
};

static struct clk_regmap pwm_e_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_EF_CTRL,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_e_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_e_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_e = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_EF_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_e",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_e_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_f_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_EF_CTRL,
		.mask = 0x3,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_f_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = pwm_parent_data,
		.num_parents = ARRAY_SIZE(pwm_parent_data),
	},
};

static struct clk_regmap pwm_f_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_EF_CTRL,
		.shift = 16,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_f_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_f_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_f = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_EF_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_f",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_f_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_g_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_GH_CTRL,
		.mask = 0x3,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_g_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = pwm_parent_data,
		.num_parents = ARRAY_SIZE(pwm_parent_data),
	},
};

static struct clk_regmap pwm_g_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_GH_CTRL,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_g_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_g_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_g = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_GH_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_g",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_g_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_h_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_GH_CTRL,
		.mask = 0x3,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_h_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = pwm_parent_data,
		.num_parents = ARRAY_SIZE(pwm_parent_data),
	},
};

static struct clk_regmap pwm_h_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_GH_CTRL,
		.shift = 16,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_h_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_h_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_h = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_GH_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_h",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_h_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data spicc_parent_data[] = {
	{ .fw_name = "oscin" },
	{ .fw_name = "sys_clk" },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div2.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &fclk_div7.hw },
	{ .fw_name = "gp1_pll" }
};

static struct clk_regmap spicc0_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_SPICC_CLK_CTRL,
		.mask = 0x7,
		.shift = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "spicc0_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = spicc_parent_data,
		.num_parents = ARRAY_SIZE(spicc_parent_data),
	},
};

static struct clk_regmap spicc0_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_SPICC_CLK_CTRL,
		.shift = 0,
		.width = 6,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "spicc0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&spicc0_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap spicc0 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_SPICC_CLK_CTRL,
		.bit_idx = 6,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "spicc0",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&spicc0_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap spicc1_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_SPICC_CLK_CTRL,
		.mask = 0x7,
		.shift = 23,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "spicc1_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = spicc_parent_data,
		.num_parents = ARRAY_SIZE(spicc_parent_data),
	},
};

static struct clk_regmap spicc1_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_SPICC_CLK_CTRL,
		.shift = 16,
		.width = 6,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "spicc1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&spicc1_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap spicc1 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_SPICC_CLK_CTRL,
		.bit_idx = 22,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "spicc1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&spicc1_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static u32 sd_emmc_parent_table[] = { 0, 1, 2, 3, 4, 7 };
static const struct clk_parent_data sd_emmc_parent_data[] = {
	{ .fw_name = "oscin" },
	{ .hw = &fclk_div2.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &fclk_div2p5.hw },
	{ .hw = &gp0_pll.hw }
};

static struct clk_regmap sd_emmc_a_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_SD_EMMC_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
		.table = sd_emmc_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sd_emmc_a_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = sd_emmc_parent_data,
		.num_parents = ARRAY_SIZE(sd_emmc_parent_data),
	},
};

static struct clk_regmap sd_emmc_a_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_SD_EMMC_CLK_CTRL,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sd_emmc_a_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&sd_emmc_a_mux.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_regmap sd_emmc_a = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_SD_EMMC_CLK_CTRL,
		.bit_idx = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sd_emmc_a",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&sd_emmc_a_div.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_regmap sd_emmc_c_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_NAND_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
		.table = sd_emmc_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sd_emmc_c_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = sd_emmc_parent_data,
		.num_parents = ARRAY_SIZE(sd_emmc_parent_data),
	},
};

static struct clk_regmap sd_emmc_c_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_NAND_CLK_CTRL,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sd_emmc_c_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&sd_emmc_c_mux.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_regmap sd_emmc_c = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_NAND_CLK_CTRL,
		.bit_idx = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sd_emmc_c",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&sd_emmc_c_div.hw,
		},
		.num_parents = 1,
	},
};

static const struct clk_parent_data eth_clk_rmii_parent_data[] = {
	{ .hw = &fclk_div2.hw },
	{ .fw_name = "rmii_pad" }
};

static struct clk_regmap eth_clk_rmii_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_ETH_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "eth_clk_rmii_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = eth_clk_rmii_parent_data,
		.num_parents = ARRAY_SIZE(eth_clk_rmii_parent_data),
	},
};

static struct clk_regmap eth_clk_rmii_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_ETH_CLK_CTRL,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "eth_clk_rmii_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&eth_clk_rmii_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap eth_clk_rmii = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_ETH_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "eth_clk_rmii",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&eth_clk_rmii_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_fixed_factor eth_clk125m_div = {
	.mult = 1,
	.div = 8,
	.hw.init = &(struct clk_init_data) {
		.name = "eth_clk125m_div",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&fclk_div2.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_regmap eth_clk125m = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_ETH_CLK_CTRL,
		.bit_idx = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "eth_clk125m",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&eth_clk125m_div.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_regmap ts_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_TS_CLK_CTRL,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "ts_div",
		.ops = &clk_regmap_divider_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "oscin"
		},
		.num_parents = 1,
	},
};

static struct clk_regmap ts = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_TS_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "ts",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&ts_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data vout_mclk_parent_data[] = {
	{ .hw = &fclk_div2p5.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &gp0_pll.hw },
	{ .hw = &hifi_pll.hw },
	{ .fw_name = "gp1_pll" },
	{ .hw = &fclk_div7.hw }
};

static struct clk_regmap vout_mclk_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VOUTENC_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vout_mclk_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = vout_mclk_parent_data,
		.num_parents = ARRAY_SIZE(vout_mclk_parent_data),
	},
};

static struct clk_regmap vout_mclk_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VOUTENC_CLK_CTRL,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vout_mclk_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vout_mclk_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap vout_mclk = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VOUTENC_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vout_mclk",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vout_mclk_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data vout_venc_mclk_parent_data[] = {
	{ .fw_name = "gp1_pll" },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &gp0_pll.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &fclk_div2p5.hw },
	{ .hw = &fclk_div7.hw }
};

static struct clk_regmap vout_venc_mclk_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VOUTENC_CLK_CTRL,
		.mask = 0x7,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vout_venc_mclk_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = vout_venc_mclk_parent_data,
		.num_parents = ARRAY_SIZE(vout_venc_mclk_parent_data),
	},
};

static struct clk_regmap vout_venc_mclk_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VOUTENC_CLK_CTRL,
		.shift = 16,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vout_venc_mclk_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vout_venc_mclk_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap vout_venc_mclk = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VOUTENC_CLK_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vout_venc_mclk",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vout_venc_mclk_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data audio_core_parent_data[] = {
	{ .hw = &fclk_div2p5.hw },
	{ .fw_name = "gp1_pll" }
};

static struct clk_regmap audio_core_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_AUDIO_CLK_CTRL,
		.mask = 0x7,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "audio_core_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = audio_core_parent_data,
		.num_parents = ARRAY_SIZE(audio_core_parent_data),
	},
};

static struct clk_regmap audio_core_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_AUDIO_CLK_CTRL,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "audio_core_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&audio_core_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap audio_core = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_AUDIO_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "audio_core",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&audio_core_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static u32 gen_parent_table[] = { 0, 1, 2, 6, 7, 17, 19, 20, 21, 22, 23,
				 24 };
static const struct clk_parent_data gen_parent_data[] = {
	{ .fw_name = "oscin" },
	{ .hw = &rtc_clk.hw },
	{ .fw_name = "sys_pll_div16" },
	{ .fw_name = "gp1_pll" },
	{ .hw = &hifi_pll.hw },
	{ .fw_name = "cpu_clk_div16" },
	{ .hw = &fclk_div2.hw },
	{ .hw = &fclk_div2p5.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &fclk_div7.hw }
};

static struct clk_regmap gen_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_GEN_CLK_CTRL,
		.mask = 0x1f,
		.shift = 12,
		.table = gen_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "gen_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gen_parent_data,
		.num_parents = ARRAY_SIZE(gen_parent_data),
	},
};

static struct clk_regmap gen_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_GEN_CLK_CTRL,
		.shift = 0,
		.width = 11,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "gen_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gen_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gen = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_GEN_CLK_CTRL,
		.bit_idx = 11,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "gen",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gen_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

#define MESON_CLK_GATE_SYS_CLK(_name, _reg, _bit)                     \
static struct clk_regmap _name = {                                           \
	.data = &(struct clk_regmap_gate_data) {                      \
		.offset = (_reg),                                     \
		.bit_idx = (_bit),                                    \
	},                                                            \
	.hw.init = &(struct clk_init_data) {                          \
		.name = #_name,                                       \
		.ops = &clk_regmap_gate_ops,                          \
		.parent_data = &(const struct clk_parent_data) {      \
			.fw_name = "sys_clk"                          \
		},                                                    \
		.num_parents = 1,                                     \
		.flags = CLK_IGNORE_UNUSED,                           \
	},                                                            \
}

MESON_CLK_GATE_SYS_CLK(sys_clk_ctrl, CLKCTRL_SYS_CLK_EN0_REG0, 0);
MESON_CLK_GATE_SYS_CLK(sys_reset_ctrl, CLKCTRL_SYS_CLK_EN0_REG0, 1);
MESON_CLK_GATE_SYS_CLK(sys_analog_ctrl, CLKCTRL_SYS_CLK_EN0_REG0, 2);
MESON_CLK_GATE_SYS_CLK(sys_pwr_ctrl, CLKCTRL_SYS_CLK_EN0_REG0, 3);
MESON_CLK_GATE_SYS_CLK(sys_pad_ctrl, CLKCTRL_SYS_CLK_EN0_REG0, 4);
MESON_CLK_GATE_SYS_CLK(sys_sys_ctrl, CLKCTRL_SYS_CLK_EN0_REG0, 5);
MESON_CLK_GATE_SYS_CLK(sys_ts_pll, CLKCTRL_SYS_CLK_EN0_REG0, 6);
MESON_CLK_GATE_SYS_CLK(sys_dev_arb, CLKCTRL_SYS_CLK_EN0_REG0, 7);
MESON_CLK_GATE_SYS_CLK(sys_mmc_pclk, CLKCTRL_SYS_CLK_EN0_REG0, 8);
MESON_CLK_GATE_SYS_CLK(sys_capu, CLKCTRL_SYS_CLK_EN0_REG0, 9);
MESON_CLK_GATE_SYS_CLK(sys_mailbox, CLKCTRL_SYS_CLK_EN0_REG0, 10);
MESON_CLK_GATE_SYS_CLK(sys_cpu_ctrl, CLKCTRL_SYS_CLK_EN0_REG0, 11);
MESON_CLK_GATE_SYS_CLK(sys_jtag_ctrl, CLKCTRL_SYS_CLK_EN0_REG0, 12);
MESON_CLK_GATE_SYS_CLK(sys_ir_ctrl, CLKCTRL_SYS_CLK_EN0_REG0, 13);
MESON_CLK_GATE_SYS_CLK(sys_irq_ctrl, CLKCTRL_SYS_CLK_EN0_REG0, 14);
MESON_CLK_GATE_SYS_CLK(sys_msr_clk, CLKCTRL_SYS_CLK_EN0_REG0, 15);
MESON_CLK_GATE_SYS_CLK(sys_rom, CLKCTRL_SYS_CLK_EN0_REG0, 16);
MESON_CLK_GATE_SYS_CLK(sys_aocpu, CLKCTRL_SYS_CLK_EN0_REG0, 17);
MESON_CLK_GATE_SYS_CLK(sys_cpu_apb, CLKCTRL_SYS_CLK_EN0_REG0, 18);
MESON_CLK_GATE_SYS_CLK(sys_rsa, CLKCTRL_SYS_CLK_EN0_REG0, 19);
MESON_CLK_GATE_SYS_CLK(sys_sar_adc, CLKCTRL_SYS_CLK_EN0_REG0, 20);
MESON_CLK_GATE_SYS_CLK(sys_startup, CLKCTRL_SYS_CLK_EN0_REG0, 21);
MESON_CLK_GATE_SYS_CLK(sys_secure, CLKCTRL_SYS_CLK_EN0_REG0, 22);
MESON_CLK_GATE_SYS_CLK(sys_spifc, CLKCTRL_SYS_CLK_EN0_REG0, 23);
MESON_CLK_GATE_SYS_CLK(sys_led_ctrl, CLKCTRL_SYS_CLK_EN0_REG0, 24);
MESON_CLK_GATE_SYS_CLK(sys_eth_phy, CLKCTRL_SYS_CLK_EN0_REG0, 25);
MESON_CLK_GATE_SYS_CLK(sys_eth_mac, CLKCTRL_SYS_CLK_EN0_REG0, 26);
MESON_CLK_GATE_SYS_CLK(sys_gic, CLKCTRL_SYS_CLK_EN0_REG0, 27);
MESON_CLK_GATE_SYS_CLK(sys_rama, CLKCTRL_SYS_CLK_EN0_REG0, 28);
MESON_CLK_GATE_SYS_CLK(sys_big_nic, CLKCTRL_SYS_CLK_EN0_REG0, 29);
MESON_CLK_GATE_SYS_CLK(sys_ramb, CLKCTRL_SYS_CLK_EN0_REG0, 30);
MESON_CLK_GATE_SYS_CLK(sys_audio_top, CLKCTRL_SYS_CLK_EN0_REG1, 0);
MESON_CLK_GATE_SYS_CLK(sys_audio_vad, CLKCTRL_SYS_CLK_EN0_REG1, 1);
MESON_CLK_GATE_SYS_CLK(sys_usb, CLKCTRL_SYS_CLK_EN0_REG1, 2);
MESON_CLK_GATE_SYS_CLK(sys_sd_emmc_a, CLKCTRL_SYS_CLK_EN0_REG1, 3);
MESON_CLK_GATE_SYS_CLK(sys_sd_emmc_c, CLKCTRL_SYS_CLK_EN0_REG1, 4);
MESON_CLK_GATE_SYS_CLK(sys_pwm_ab, CLKCTRL_SYS_CLK_EN0_REG1, 5);
MESON_CLK_GATE_SYS_CLK(sys_pwm_cd, CLKCTRL_SYS_CLK_EN0_REG1, 6);
MESON_CLK_GATE_SYS_CLK(sys_pwm_ef, CLKCTRL_SYS_CLK_EN0_REG1, 7);
MESON_CLK_GATE_SYS_CLK(sys_pwm_gh, CLKCTRL_SYS_CLK_EN0_REG1, 8);
MESON_CLK_GATE_SYS_CLK(sys_spicc_1, CLKCTRL_SYS_CLK_EN0_REG1, 9);
MESON_CLK_GATE_SYS_CLK(sys_spicc_0, CLKCTRL_SYS_CLK_EN0_REG1, 10);
MESON_CLK_GATE_SYS_CLK(sys_uart_a, CLKCTRL_SYS_CLK_EN0_REG1, 11);
MESON_CLK_GATE_SYS_CLK(sys_uart_b, CLKCTRL_SYS_CLK_EN0_REG1, 12);
MESON_CLK_GATE_SYS_CLK(sys_uart_c, CLKCTRL_SYS_CLK_EN0_REG1, 13);
MESON_CLK_GATE_SYS_CLK(sys_uart_d, CLKCTRL_SYS_CLK_EN0_REG1, 14);
MESON_CLK_GATE_SYS_CLK(sys_uart_e, CLKCTRL_SYS_CLK_EN0_REG1, 15);
MESON_CLK_GATE_SYS_CLK(sys_i2c_m_a, CLKCTRL_SYS_CLK_EN0_REG1, 16);
MESON_CLK_GATE_SYS_CLK(sys_i2c_m_b, CLKCTRL_SYS_CLK_EN0_REG1, 17);
MESON_CLK_GATE_SYS_CLK(sys_i2c_m_c, CLKCTRL_SYS_CLK_EN0_REG1, 18);
MESON_CLK_GATE_SYS_CLK(sys_i2c_m_d, CLKCTRL_SYS_CLK_EN0_REG1, 19);
MESON_CLK_GATE_SYS_CLK(sys_rtc, CLKCTRL_SYS_CLK_EN0_REG1, 21);
MESON_CLK_GATE_SYS_CLK(sys_vout, CLKCTRL_SYS_CLK_EN0_REG1, 22);
MESON_CLK_GATE_SYS_CLK(sys_usb_ctrl, CLKCTRL_SYS_CLK_EN0_REG1, 26);
MESON_CLK_GATE_SYS_CLK(sys_acodec, CLKCTRL_SYS_CLK_EN0_REG1, 27);

#define MESON_CLK_GATE_AXI_CLK(_name, _reg, _bit)                     \
static struct clk_regmap _name = {                                           \
	.data = &(struct clk_regmap_gate_data) {                      \
		.offset = (_reg),                                     \
		.bit_idx = (_bit),                                    \
	},                                                            \
	.hw.init = &(struct clk_init_data) {                          \
		.name = #_name,                                       \
		.ops = &clk_regmap_gate_ops,                          \
		.parent_data = &(const struct clk_parent_data) {      \
			.fw_name = "axi_clk"                          \
		},                                                    \
		.num_parents = 1,                                     \
		.flags = CLK_IGNORE_UNUSED,                           \
	},                                                            \
}

MESON_CLK_GATE_AXI_CLK(axi_audio_vad, CLKCTRL_AXI_CLK_EN0, 0);
MESON_CLK_GATE_AXI_CLK(axi_audio_top, CLKCTRL_AXI_CLK_EN0, 1);
MESON_CLK_GATE_AXI_CLK(axi_sys_nic, CLKCTRL_AXI_CLK_EN0, 2);
MESON_CLK_GATE_AXI_CLK(axi_rama, CLKCTRL_AXI_CLK_EN0, 6);
MESON_CLK_GATE_AXI_CLK(axi_cpu_dmc, CLKCTRL_AXI_CLK_EN0, 7);
MESON_CLK_GATE_AXI_CLK(axi_dev1_dmc, CLKCTRL_AXI_CLK_EN0, 13);
MESON_CLK_GATE_AXI_CLK(axi_dev0_dmc, CLKCTRL_AXI_CLK_EN0, 14);
MESON_CLK_GATE_AXI_CLK(axi_dsp_dmc, CLKCTRL_AXI_CLK_EN0, 15);

static struct clk_regmap *const pll_regmaps[] = {
	&gp0_pll,
	&hifi_pll,
	&fclk50m,
	&fclk_div2,
	&fclk_div2p5,
	&fclk_div3,
	&fclk_div4,
	&fclk_div5,
	&fclk_div7
};

static struct clk_regmap *const clk_regmaps[] = {
	&rtc_dual_clkin,
	&rtc_dual_div,
	&rtc_dual_mux,
	&rtc_dual,
	&rtc_clk,
	&clk_24m_in,
	&clk_12m,
	&clk_25m_div,
	&clk_25m,
	&sar_adc_mux,
	&sar_adc_div,
	&sar_adc,
	&pwm_a_mux,
	&pwm_a_div,
	&pwm_a,
	&pwm_b_mux,
	&pwm_b_div,
	&pwm_b,
	&pwm_c_mux,
	&pwm_c_div,
	&pwm_c,
	&pwm_d_mux,
	&pwm_d_div,
	&pwm_d,
	&pwm_e_mux,
	&pwm_e_div,
	&pwm_e,
	&pwm_f_mux,
	&pwm_f_div,
	&pwm_f,
	&pwm_g_mux,
	&pwm_g_div,
	&pwm_g,
	&pwm_h_mux,
	&pwm_h_div,
	&pwm_h,
	&spicc0_mux,
	&spicc0_div,
	&spicc0,
	&spicc1_mux,
	&spicc1_div,
	&spicc1,
	&sd_emmc_a_mux,
	&sd_emmc_a_div,
	&sd_emmc_a,
	&sd_emmc_c_mux,
	&sd_emmc_c_div,
	&sd_emmc_c,
	&eth_clk_rmii_mux,
	&eth_clk_rmii_div,
	&eth_clk_rmii,
	&eth_clk125m,
	&ts_div,
	&ts,
	&vout_mclk_mux,
	&vout_mclk_div,
	&vout_mclk,
	&vout_venc_mclk_mux,
	&vout_venc_mclk_div,
	&vout_venc_mclk,
	&audio_core_mux,
	&audio_core_div,
	&audio_core,
	&gen_mux,
	&gen_div,
	&gen,
	&sys_clk_ctrl,
	&sys_reset_ctrl,
	&sys_analog_ctrl,
	&sys_pwr_ctrl,
	&sys_pad_ctrl,
	&sys_sys_ctrl,
	&sys_ts_pll,
	&sys_dev_arb,
	&sys_mmc_pclk,
	&sys_capu,
	&sys_mailbox,
	&sys_cpu_ctrl,
	&sys_jtag_ctrl,
	&sys_ir_ctrl,
	&sys_irq_ctrl,
	&sys_msr_clk,
	&sys_rom,
	&sys_aocpu,
	&sys_cpu_apb,
	&sys_rsa,
	&sys_sar_adc,
	&sys_startup,
	&sys_secure,
	&sys_spifc,
	&sys_led_ctrl,
	&sys_eth_phy,
	&sys_eth_mac,
	&sys_gic,
	&sys_rama,
	&sys_big_nic,
	&sys_ramb,
	&sys_audio_top,
	&sys_audio_vad,
	&sys_usb,
	&sys_sd_emmc_a,
	&sys_sd_emmc_c,
	&sys_pwm_ab,
	&sys_pwm_cd,
	&sys_pwm_ef,
	&sys_pwm_gh,
	&sys_spicc_1,
	&sys_spicc_0,
	&sys_uart_a,
	&sys_uart_b,
	&sys_uart_c,
	&sys_uart_d,
	&sys_uart_e,
	&sys_i2c_m_a,
	&sys_i2c_m_b,
	&sys_i2c_m_c,
	&sys_i2c_m_d,
	&sys_rtc,
	&sys_vout,
	&sys_usb_ctrl,
	&sys_acodec,
	&axi_audio_vad,
	&axi_audio_top,
	&axi_sys_nic,
	&axi_rama,
	&axi_cpu_dmc,
	&axi_dev1_dmc,
	&axi_dev0_dmc,
	&axi_dsp_dmc
};

static struct clk_hw *a4_hw_clks[] = {
	[CLKID_GP0_PLL]               = &gp0_pll.hw,
	[CLKID_HIFI_PLL]              = &hifi_pll.hw,
	[CLKID_FCLK50M_DIV]           = &fclk50m_div.hw,
	[CLKID_FCLK50M]               = &fclk50m.hw,
	[CLKID_FCLK_DIV2_DIV]         = &fclk_div2_div.hw,
	[CLKID_FCLK_DIV2]             = &fclk_div2.hw,
	[CLKID_FCLK_DIV2P5_DIV]       = &fclk_div2p5_div.hw,
	[CLKID_FCLK_DIV2P5]           = &fclk_div2p5.hw,
	[CLKID_FCLK_DIV3_DIV]         = &fclk_div3_div.hw,
	[CLKID_FCLK_DIV3]             = &fclk_div3.hw,
	[CLKID_FCLK_DIV4_DIV]         = &fclk_div4_div.hw,
	[CLKID_FCLK_DIV4]             = &fclk_div4.hw,
	[CLKID_FCLK_DIV5_DIV]         = &fclk_div5_div.hw,
	[CLKID_FCLK_DIV5]             = &fclk_div5.hw,
	[CLKID_FCLK_DIV7_DIV]         = &fclk_div7_div.hw,
	[CLKID_FCLK_DIV7]             = &fclk_div7.hw,
	[CLKID_RTC_DUAL_CLKIN]        = &rtc_dual_clkin.hw,
	[CLKID_RTC_DUAL_DIV]          = &rtc_dual_div.hw,
	[CLKID_RTC_DUAL_MUX]          = &rtc_dual_mux.hw,
	[CLKID_RTC_DUAL]              = &rtc_dual.hw,
	[CLKID_RTC_CLK]               = &rtc_clk.hw,
	[CLKID_CLK_24M_IN]            = &clk_24m_in.hw,
	[CLKID_CLK_12M_DIV]           = &clk_12m_div.hw,
	[CLKID_CLK_12M]               = &clk_12m.hw,
	[CLKID_CLK_25M_DIV]           = &clk_25m_div.hw,
	[CLKID_CLK_25M]               = &clk_25m.hw,
	[CLKID_SAR_ADC_MUX]           = &sar_adc_mux.hw,
	[CLKID_SAR_ADC_DIV]           = &sar_adc_div.hw,
	[CLKID_SAR_ADC]               = &sar_adc.hw,
	[CLKID_PWM_A_MUX]             = &pwm_a_mux.hw,
	[CLKID_PWM_A_DIV]             = &pwm_a_div.hw,
	[CLKID_PWM_A]                 = &pwm_a.hw,
	[CLKID_PWM_B_MUX]             = &pwm_b_mux.hw,
	[CLKID_PWM_B_DIV]             = &pwm_b_div.hw,
	[CLKID_PWM_B]                 = &pwm_b.hw,
	[CLKID_PWM_C_MUX]             = &pwm_c_mux.hw,
	[CLKID_PWM_C_DIV]             = &pwm_c_div.hw,
	[CLKID_PWM_C]                 = &pwm_c.hw,
	[CLKID_PWM_D_MUX]             = &pwm_d_mux.hw,
	[CLKID_PWM_D_DIV]             = &pwm_d_div.hw,
	[CLKID_PWM_D]                 = &pwm_d.hw,
	[CLKID_PWM_E_MUX]             = &pwm_e_mux.hw,
	[CLKID_PWM_E_DIV]             = &pwm_e_div.hw,
	[CLKID_PWM_E]                 = &pwm_e.hw,
	[CLKID_PWM_F_MUX]             = &pwm_f_mux.hw,
	[CLKID_PWM_F_DIV]             = &pwm_f_div.hw,
	[CLKID_PWM_F]                 = &pwm_f.hw,
	[CLKID_PWM_G_MUX]             = &pwm_g_mux.hw,
	[CLKID_PWM_G_DIV]             = &pwm_g_div.hw,
	[CLKID_PWM_G]                 = &pwm_g.hw,
	[CLKID_PWM_H_MUX]             = &pwm_h_mux.hw,
	[CLKID_PWM_H_DIV]             = &pwm_h_div.hw,
	[CLKID_PWM_H]                 = &pwm_h.hw,
	[CLKID_SPICC0_MUX]            = &spicc0_mux.hw,
	[CLKID_SPICC0_DIV]            = &spicc0_div.hw,
	[CLKID_SPICC0]                = &spicc0.hw,
	[CLKID_SPICC1_MUX]            = &spicc1_mux.hw,
	[CLKID_SPICC1_DIV]            = &spicc1_div.hw,
	[CLKID_SPICC1]                = &spicc1.hw,
	[CLKID_SD_EMMC_A_MUX]         = &sd_emmc_a_mux.hw,
	[CLKID_SD_EMMC_A_DIV]         = &sd_emmc_a_div.hw,
	[CLKID_SD_EMMC_A]             = &sd_emmc_a.hw,
	[CLKID_SD_EMMC_C_MUX]         = &sd_emmc_c_mux.hw,
	[CLKID_SD_EMMC_C_DIV]         = &sd_emmc_c_div.hw,
	[CLKID_SD_EMMC_C]             = &sd_emmc_c.hw,
	[CLKID_ETH_CLK_RMII_MUX]      = &eth_clk_rmii_mux.hw,
	[CLKID_ETH_CLK_RMII_DIV]      = &eth_clk_rmii_div.hw,
	[CLKID_ETH_CLK_RMII]          = &eth_clk_rmii.hw,
	[CLKID_ETH_CLK125M_DIV]       = &eth_clk125m_div.hw,
	[CLKID_ETH_CLK125M]           = &eth_clk125m.hw,
	[CLKID_TS_DIV]                = &ts_div.hw,
	[CLKID_TS]                    = &ts.hw,
	[CLKID_VOUT_MCLK_MUX]         = &vout_mclk_mux.hw,
	[CLKID_VOUT_MCLK_DIV]         = &vout_mclk_div.hw,
	[CLKID_VOUT_MCLK]             = &vout_mclk.hw,
	[CLKID_VOUT_VENC_MCLK_MUX]    = &vout_venc_mclk_mux.hw,
	[CLKID_VOUT_VENC_MCLK_DIV]    = &vout_venc_mclk_div.hw,
	[CLKID_VOUT_VENC_MCLK]        = &vout_venc_mclk.hw,
	[CLKID_AUDIO_CORE_MUX]        = &audio_core_mux.hw,
	[CLKID_AUDIO_CORE_DIV]        = &audio_core_div.hw,
	[CLKID_AUDIO_CORE]            = &audio_core.hw,
	[CLKID_GEN_MUX]               = &gen_mux.hw,
	[CLKID_GEN_DIV]               = &gen_div.hw,
	[CLKID_GEN]                   = &gen.hw,
	[CLKID_SYS_CLK_CTRL]          = &sys_clk_ctrl.hw,
	[CLKID_SYS_RESET_CTRL]        = &sys_reset_ctrl.hw,
	[CLKID_SYS_ANALOG_CTRL]       = &sys_analog_ctrl.hw,
	[CLKID_SYS_PWR_CTRL]          = &sys_pwr_ctrl.hw,
	[CLKID_SYS_PAD_CTRL]          = &sys_pad_ctrl.hw,
	[CLKID_SYS_SYS_CTRL]          = &sys_sys_ctrl.hw,
	[CLKID_SYS_TS_PLL]            = &sys_ts_pll.hw,
	[CLKID_SYS_DEV_ARB]           = &sys_dev_arb.hw,
	[CLKID_SYS_MMC_PCLK]          = &sys_mmc_pclk.hw,
	[CLKID_SYS_CAPU]              = &sys_capu.hw,
	[CLKID_SYS_MAILBOX]           = &sys_mailbox.hw,
	[CLKID_SYS_CPU_CTRL]          = &sys_cpu_ctrl.hw,
	[CLKID_SYS_JTAG_CTRL]         = &sys_jtag_ctrl.hw,
	[CLKID_SYS_IR_CTRL]           = &sys_ir_ctrl.hw,
	[CLKID_SYS_IRQ_CTRL]          = &sys_irq_ctrl.hw,
	[CLKID_SYS_MSR_CLK]           = &sys_msr_clk.hw,
	[CLKID_SYS_ROM]               = &sys_rom.hw,
	[CLKID_SYS_AOCPU]             = &sys_aocpu.hw,
	[CLKID_SYS_CPU_APB]           = &sys_cpu_apb.hw,
	[CLKID_SYS_RSA]               = &sys_rsa.hw,
	[CLKID_SYS_SAR_ADC]           = &sys_sar_adc.hw,
	[CLKID_SYS_STARTUP]           = &sys_startup.hw,
	[CLKID_SYS_SECURE]            = &sys_secure.hw,
	[CLKID_SYS_SPIFC]             = &sys_spifc.hw,
	[CLKID_SYS_LED_CTRL]          = &sys_led_ctrl.hw,
	[CLKID_SYS_ETH_PHY]           = &sys_eth_phy.hw,
	[CLKID_SYS_ETH_MAC]           = &sys_eth_mac.hw,
	[CLKID_SYS_GIC]               = &sys_gic.hw,
	[CLKID_SYS_RAMA]              = &sys_rama.hw,
	[CLKID_SYS_BIG_NIC]           = &sys_big_nic.hw,
	[CLKID_SYS_RAMB]              = &sys_ramb.hw,
	[CLKID_SYS_AUDIO_TOP]         = &sys_audio_top.hw,
	[CLKID_SYS_AUDIO_VAD]         = &sys_audio_vad.hw,
	[CLKID_SYS_USB]               = &sys_usb.hw,
	[CLKID_SYS_SD_EMMC_A]         = &sys_sd_emmc_a.hw,
	[CLKID_SYS_SD_EMMC_C]         = &sys_sd_emmc_c.hw,
	[CLKID_SYS_PWM_AB]            = &sys_pwm_ab.hw,
	[CLKID_SYS_PWM_CD]            = &sys_pwm_cd.hw,
	[CLKID_SYS_PWM_EF]            = &sys_pwm_ef.hw,
	[CLKID_SYS_PWM_GH]            = &sys_pwm_gh.hw,
	[CLKID_SYS_SPICC_1]           = &sys_spicc_1.hw,
	[CLKID_SYS_SPICC_0]           = &sys_spicc_0.hw,
	[CLKID_SYS_UART_A]            = &sys_uart_a.hw,
	[CLKID_SYS_UART_B]            = &sys_uart_b.hw,
	[CLKID_SYS_UART_C]            = &sys_uart_c.hw,
	[CLKID_SYS_UART_D]            = &sys_uart_d.hw,
	[CLKID_SYS_UART_E]            = &sys_uart_e.hw,
	[CLKID_SYS_I2C_M_A]           = &sys_i2c_m_a.hw,
	[CLKID_SYS_I2C_M_B]           = &sys_i2c_m_b.hw,
	[CLKID_SYS_I2C_M_C]           = &sys_i2c_m_c.hw,
	[CLKID_SYS_I2C_M_D]           = &sys_i2c_m_d.hw,
	[CLKID_SYS_RTC]               = &sys_rtc.hw,
	[CLKID_SYS_VOUT]              = &sys_vout.hw,
	[CLKID_SYS_USB_CTRL]          = &sys_usb_ctrl.hw,
	[CLKID_SYS_ACODEC]            = &sys_acodec.hw,
	[CLKID_AXI_AUDIO_VAD]         = &axi_audio_vad.hw,
	[CLKID_AXI_AUDIO_TOP]         = &axi_audio_top.hw,
	[CLKID_AXI_SYS_NIC]           = &axi_sys_nic.hw,
	[CLKID_AXI_RAMA]              = &axi_rama.hw,
	[CLKID_AXI_CPU_DMC]           = &axi_cpu_dmc.hw,
	[CLKID_AXI_DEV1_DMC]          = &axi_dev1_dmc.hw,
	[CLKID_AXI_DEV0_DMC]          = &axi_dev0_dmc.hw,
	[CLKID_AXI_DSP_DMC]           = &axi_dsp_dmc.hw
};

static const struct meson_clk_hw_data a4_clks = {
	.hws = a4_hw_clks,
	.num = ARRAY_SIZE(a4_hw_clks),
};

static int meson_a4_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct regmap *basic_map;
	struct regmap *pll_map;
	struct meson_clk_hw_data *data;
	struct clk *clk;
	int ret, i;

	data = (struct meson_clk_hw_data *)of_device_get_match_data(dev);
	if (!data)
		return -ENODEV;

#ifdef CONFIG_AMLOGIC_CLK_DEBUG
	clk = devm_clk_get(dev, "xtal");
	if (IS_ERR(clk)) {
		pr_err("%s: clock source xtal not found\n", dev_name(&pdev->dev));
		return PTR_ERR(clk);
	}

	ret = devm_clk_hw_register_clkdev(dev, __clk_get_hw(clk),
					  NULL,
					  __clk_get_name(clk));
	if (ret < 0) {
		dev_err(dev, "Failed to clkdev register: %d\n", ret);
		return ret;
	}
#endif

	basic_map = meson_clk_regmap_resource(pdev, dev, 0);
	if (IS_ERR(basic_map)) {
		dev_err(dev, "basic clk registers not found\n");
		return PTR_ERR(basic_map);
	}

	pll_map = meson_clk_regmap_resource(pdev, dev, 1);
	if (IS_ERR(pll_map)) {
		dev_err(dev, "pll clk registers not found\n");
		return PTR_ERR(pll_map);
	}

	/* Populate regmap for the regmap backed clocks */
	for (i = 0; i < ARRAY_SIZE(clk_regmaps); i++)
		clk_regmaps[i]->map = basic_map;

	for (i = 0; i < ARRAY_SIZE(pll_regmaps); i++)
		pll_regmaps[i]->map = pll_map;

	for (i = 0; i < data->num; i++) {
		/* array might be sparse */
		if (!data->hws[i])
			continue;
		ret = devm_clk_hw_register(dev, data->hws[i]);
		if (ret) {
			dev_err(dev, "Clock registration failed\n");
			return ret;
		}

#ifdef CONFIG_AMLOGIC_CLK_DEBUG
		ret = devm_clk_hw_register_clkdev(dev, data->hws[i],
						  NULL,
						  clk_hw_get_name(data->hws[i]));
		if (ret < 0) {
			dev_err(dev, "Failed to clkdev register: %d\n", ret);
			return ret;
		}
#endif
	}

	return devm_of_clk_add_hw_provider(dev, meson_clk_hw_get,
					   (void *)data);
}

static const struct of_device_id clkc_match_table[] = {
	{
		.compatible = "amlogic,a4-clkc",
		.data = &a4_clks
	},
	{}
};

static struct platform_driver a4_driver = {
	.probe			= meson_a4_probe,
	.driver			= {
		.name		= "a4-clkc",
		.of_match_table	= clkc_match_table,
	},
};

builtin_platform_driver(a4_driver);
MODULE_LICENSE("GPL v2");
