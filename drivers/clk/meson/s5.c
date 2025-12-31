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
#include "s5.h"
#include <dt-bindings/clock/amlogic,s5-clkc.h>

static const struct pll_params_table sys2_pll_table[] = {
	PLL_PARAMS_OD(69, 1, 1), /*DCO=1656M OD=828M*/
	{ /* sentinel */ }
};

static const struct reg_sequence sys2_pll_init_regs[] = {
	{ .reg = CLKCTRL_SYS2PLL_CTRL1, .def = 0x3420500f },
	{ .reg = CLKCTRL_SYS2PLL_CTRL2, .def = 0x00023001 },
	{ .reg = CLKCTRL_SYS2PLL_CTRL3, .def = 0x00000000 },
};

static struct clk_regmap sys2_pll = {
	.data = &(struct meson_clk_pll_data) {
		.en = {
			.reg_off = CLKCTRL_SYS2PLL_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.m = {
			.reg_off = CLKCTRL_SYS2PLL_CTRL0,
			.shift   = 0,
			.width   = 8,
		},
		.n = {
			.reg_off = CLKCTRL_SYS2PLL_CTRL0,
			.shift   = 16,
			.width   = 5,
		},
		.od = {
			.reg_off = CLKCTRL_SYS2PLL_CTRL0,
			.shift   = 12,
			.width   = 3,
		},
		.l = {
			.reg_off = CLKCTRL_SYS2PLL_STS,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = CLKCTRL_SYS2PLL_CTRL0,
			.shift   = 29,
			.width   = 1,
		},
		.l_rst = {
			.reg_off = CLKCTRL_SYS2PLL_CTRL2,
			.shift   = 6,
			.width   = 1,
		},
		.table = sys2_pll_table,
		.init_regs = sys2_pll_init_regs,
		.init_count = ARRAY_SIZE(sys2_pll_init_regs),
		.od_max = 4,
		.flags = CLK_MESON_PLL_FIXED_N,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sys2_pll",
		.ops = &meson_clk_pll_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static const struct pll_params_table gp0_pll_table[] = {
	PLL_PARAMS_OD(70, 1, 1), /* DCO = 1680M OD = 2 PLL = 840M */
	PLL_PARAMS_OD(132, 1, 2), /* DCO = 3168M OD = 4 PLL = 792M */
	PLL_PARAMS_OD(128, 1, 2), /* DCO = 3072M OD = 4 PLL = 768M */
	PLL_PARAMS_OD(124, 1, 2), /* DCO = 2976M OD = 4 PLL = 744M */
	PLL_PARAMS_OD(96, 1, 1), /* DCO = 2304M OD = 2 PLL = 1152M */
	{ /* sentinel */ }
};

static const struct reg_sequence gp0_pll_init_regs[] = {
	{ .reg = CLKCTRL_GP0PLL_CTRL1, .def = 0x03a10000 },
	{ .reg = CLKCTRL_GP0PLL_CTRL2, .def = 0x00040000 },
	{ .reg = CLKCTRL_GP0PLL_CTRL3, .def = 0x0b0da200 },
};

static struct clk_regmap gp0_pll = {
	.data = &(struct meson_clk_pll_data) {
		.en = {
			.reg_off = CLKCTRL_GP0PLL_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.m = {
			.reg_off = CLKCTRL_GP0PLL_CTRL0,
			.shift   = 0,
			.width   = 9,
		},
		.n = {
			.reg_off = CLKCTRL_GP0PLL_CTRL0,
			.shift   = 16,
			.width   = 5,
		},
		.frac = {
			.reg_off = CLKCTRL_GP0PLL_CTRL1,
			.shift   = 0,
			.width   = 17,
		},
		.od = {
			.reg_off = CLKCTRL_GP0PLL_CTRL0,
			.shift   = 10,
			.width   = 3,
		},
		.l = {
			.reg_off = CLKCTRL_GP0PLL_STS,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = CLKCTRL_GP0PLL_CTRL0,
			.shift   = 29,
			.width   = 1,
		},
		.l_rst = {
			.reg_off = CLKCTRL_GP0PLL_CTRL3,
			.shift   = 9,
			.width   = 1,
		},
		.table = gp0_pll_table,
		.init_regs = gp0_pll_init_regs,
		.init_count = ARRAY_SIZE(gp0_pll_init_regs),
		.od_max = 4,
		.flags = CLK_MESON_PLL_FIXED_N |
			 CLK_MESON_PLL_L_RSTN,
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

static const struct pll_params_table gp2_pll_table[] = {
	PLL_PARAMS_OD(124, 1, 2), /* DCO = 2976M OD = 4 PLL = 744M */
	PLL_PARAMS_OD(128, 1, 2), /* DCO = 3072M OD = 4 PLL = 768M */
	PLL_PARAMS_OD(132, 1, 2), /* DCO = 3168M OD = 4 PLL = 792M */
	PLL_PARAMS_OD(70,  1, 1), /* DCO = 1680M OD = 2 PLL = 840M */
	PLL_PARAMS_OD(96,  1, 1), /* DCO = 2304M OD = 2 PLL = 1152M */
	{ /* sentinel */ }
};

static const struct reg_sequence gp2_pll_init_regs[] = {
	{ .reg = CLKCTRL_GP2PLL_CTRL1, .def = 0x03a10000 },
	{ .reg = CLKCTRL_GP2PLL_CTRL2, .def = 0x00040000 },
	{ .reg = CLKCTRL_GP2PLL_CTRL3, .def = 0x0b0da200 }
};

static struct clk_regmap gp2_pll = {
	.data = &(struct meson_clk_pll_data) {
		.en = {
			.reg_off = CLKCTRL_GP2PLL_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.m = {
			.reg_off = CLKCTRL_GP2PLL_CTRL0,
			.shift   = 0,
			.width   = 9,
		},
		.n = {
			.reg_off = CLKCTRL_GP2PLL_CTRL0,
			.shift   = 16,
			.width   = 5,
		},
		.frac = {
			.reg_off = CLKCTRL_GP2PLL_CTRL1,
			.shift   = 0,
			.width   = 17,
		},
		.od = {
			.reg_off = CLKCTRL_GP2PLL_CTRL0,
			.shift   = 10,
			.width   = 3,
		},
		.l = {
			.reg_off = CLKCTRL_GP2PLL_STS,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = CLKCTRL_GP2PLL_CTRL0,
			.shift   = 29,
			.width   = 1,
		},
		.l_rst = {
			.reg_off = CLKCTRL_GP2PLL_CTRL3,
			.shift   = 9,
			.width   = 1,
		},
		.table = gp2_pll_table,
		.init_regs = gp2_pll_init_regs,
		.init_count = ARRAY_SIZE(gp2_pll_init_regs),
		.od_max = 4,
		.flags = CLK_MESON_PLL_FIXED_N |
			 CLK_MESON_PLL_L_RSTN,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "gp2_pll",
		.ops = &meson_clk_pll_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
		.flags = CLK_IGNORE_UNUSED,
	},
};

static const struct pll_mult_range hifi_pll_range = {
	.min = 67,
	.max = 133
};

static const struct reg_sequence hifi_pll_init_regs[] = {
	{ .reg = CLKCTRL_HIFIPLL_CTRL1, .def = 0x03a00000 },
	{ .reg = CLKCTRL_HIFIPLL_CTRL2, .def = 0x00040000 },
	{ .reg = CLKCTRL_HIFIPLL_CTRL3, .def = 0x0b0da200 }
};

static struct clk_regmap hifi_pll = {
	.data = &(struct meson_clk_pll_data) {
		.en = {
			.reg_off = CLKCTRL_HIFIPLL_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.m = {
			.reg_off = CLKCTRL_HIFIPLL_CTRL0,
			.shift   = 0,
			.width   = 9,
		},
		.n = {
			.reg_off = CLKCTRL_HIFIPLL_CTRL0,
			.shift   = 16,
			.width   = 5,
		},
		.frac = {
			.reg_off = CLKCTRL_HIFIPLL_CTRL1,
			.shift   = 0,
			.width   = 17,
		},
		.od = {
			.reg_off = CLKCTRL_HIFIPLL_CTRL0,
			.shift   = 10,
			.width   = 3,
		},
		.l = {
			.reg_off = CLKCTRL_HIFIPLL_STS,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = CLKCTRL_HIFIPLL_CTRL0,
			.shift   = 29,
			.width   = 1,
		},
		.l_rst = {
			.reg_off = CLKCTRL_HIFIPLL_CTRL3,
			.shift   = 9,
			.width   = 1,
		},
		.range = &hifi_pll_range,
		.init_regs = hifi_pll_init_regs,
		.init_count = ARRAY_SIZE(hifi_pll_init_regs),
		.od_max = 4,
		.flags = CLK_MESON_PLL_ROUND_CLOSEST |
			 CLK_MESON_PLL_FIXED_N |
			 CLK_MESON_PLL_L_RSTN |
			 CLK_MESON_PLL_NOINIT_ENABLED,
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

static const struct pll_mult_range hifi1_pll_range = {
	.min = 67,
	.max = 133
};

static const struct reg_sequence hifi1_pll_init_regs[] = {
	{ .reg = CLKCTRL_HIFI1PLL_CTRL1, .def = 0x03a00000 },
	{ .reg = CLKCTRL_HIFI1PLL_CTRL2, .def = 0x00040000 },
	{ .reg = CLKCTRL_HIFI1PLL_CTRL3, .def = 0x0b0da200 }
};

static struct clk_regmap hifi1_pll = {
	.data = &(struct meson_clk_pll_data) {
		.en = {
			.reg_off = CLKCTRL_HIFI1PLL_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.m = {
			.reg_off = CLKCTRL_HIFI1PLL_CTRL0,
			.shift   = 0,
			.width   = 9,
		},
		.n = {
			.reg_off = CLKCTRL_HIFI1PLL_CTRL0,
			.shift   = 16,
			.width   = 5,
		},
		.frac = {
			.reg_off = CLKCTRL_HIFI1PLL_CTRL1,
			.shift   = 0,
			.width   = 17,
		},
		.od = {
			.reg_off = CLKCTRL_HIFI1PLL_CTRL0,
			.shift   = 10,
			.width   = 3,
		},
		.l = {
			.reg_off = CLKCTRL_HIFI1PLL_STS,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = CLKCTRL_HIFI1PLL_CTRL0,
			.shift   = 29,
			.width   = 1,
		},
		.l_rst = {
			.reg_off = CLKCTRL_HIFI1PLL_CTRL3,
			.shift   = 9,
			.width   = 1,
		},
		.range = &hifi1_pll_range,
		.init_regs = hifi1_pll_init_regs,
		.init_count = ARRAY_SIZE(hifi1_pll_init_regs),
		.od_max = 4,
		.flags = CLK_MESON_PLL_ROUND_CLOSEST |
			 CLK_MESON_PLL_FIXED_N |
			 CLK_MESON_PLL_L_RSTN,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hifi1_pll",
		.ops = &meson_clk_pll_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static const struct pll_params_table fpll_dco_table[] = {
	PLL_PARAMS(100, 1), /* DCO=2400M */
	PLL_PARAMS(125, 1), /* DCO=3000M */
	PLL_PARAMS(133, 1), /* DCO=3192M */
	{ /* sentinel */ }
};

static const struct reg_sequence fpll_dco_init_regs[] = {
	{ .reg = CLKCTRL_FPLL_CTRL0, .def = 0x00002000 },
	{ .reg = CLKCTRL_FPLL_CTRL1, .def = 0x03a00000 },
	{ .reg = CLKCTRL_FPLL_CTRL2, .def = 0x00040000 },
	{ .reg = CLKCTRL_FPLL_CTRL3, .def = 0x0b0da200 }
};

static struct clk_regmap fpll_dco = {
	.data = &(struct meson_clk_pll_data) {
		.en = {
			.reg_off = CLKCTRL_FPLL_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.m = {
			.reg_off = CLKCTRL_FPLL_CTRL0,
			.shift   = 0,
			.width   = 9,
		},
		.n = {
			.reg_off = CLKCTRL_FPLL_CTRL0,
			.shift   = 16,
			.width   = 5,
		},
		.frac = {
			.reg_off = CLKCTRL_FPLL_CTRL1,
			.shift   = 0,
			.width   = 17,
		},
		.l = {
			.reg_off = CLKCTRL_FPLL_STS,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = CLKCTRL_FPLL_CTRL0,
			.shift   = 29,
			.width   = 1,
		},
		.l_rst = {
			.reg_off = CLKCTRL_FPLL_CTRL3,
			.shift   = 9,
			.width   = 1,
		},
		.table = fpll_dco_table,
		.init_regs = fpll_dco_init_regs,
		.init_count = ARRAY_SIZE(fpll_dco_init_regs),
		.flags = CLK_MESON_PLL_L_RSTN |
			 CLK_MESON_PLL_NOINIT_ENABLED,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "fpll_dco",
		.ops = &meson_clk_pll_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_IGNORE_UNUSED,
	},
};

static const struct pll_mult_range nna_pll_range = {
	.min = 67,
	.max = 133
};

static const struct reg_sequence nna_pll_init_regs[] = {
	{ .reg = CLKCTRL_NNAPLL_CTRL1, .def = 0x03a00000 },
	{ .reg = CLKCTRL_NNAPLL_CTRL2, .def = 0x00040000 },
	{ .reg = CLKCTRL_NNAPLL_CTRL3, .def = 0x0b0da200 }
};

static struct clk_regmap nna_pll = {
	.data = &(struct meson_clk_pll_data) {
		.en = {
			.reg_off = CLKCTRL_NNAPLL_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.m = {
			.reg_off = CLKCTRL_NNAPLL_CTRL0,
			.shift   = 0,
			.width   = 9,
		},
		.n = {
			.reg_off = CLKCTRL_NNAPLL_CTRL0,
			.shift   = 16,
			.width   = 5,
		},
		.frac = {
			.reg_off = CLKCTRL_NNAPLL_CTRL1,
			.shift   = 0,
			.width   = 17,
		},
		.od = {
			.reg_off = CLKCTRL_NNAPLL_CTRL0,
			.shift   = 10,
			.width   = 3,
		},
		.l = {
			.reg_off = CLKCTRL_NNAPLL_STS,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = CLKCTRL_NNAPLL_CTRL0,
			.shift   = 29,
			.width   = 1,
		},
		.l_rst = {
			.reg_off = CLKCTRL_NNAPLL_CTRL3,
			.shift   = 9,
			.width   = 1,
		},
		.range = &nna_pll_range,
		.init_regs = nna_pll_init_regs,
		.init_count = ARRAY_SIZE(nna_pll_init_regs),
		.od_max = 4,
		.flags = CLK_MESON_PLL_FIXED_N |
			 CLK_MESON_PLL_L_RSTN,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "nna_pll",
		.ops = &meson_clk_pll_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static const struct pll_params_table pcie_pll_dco_table[] = {
	PLL_PARAMS(150, 1),
	{ /* sentinel */ }
};

static const struct reg_sequence pcie_pll_dco_init_regs[] = {
	{ .reg = ANACTRL_PCIEPLL_CTRL0, .def = 0x200c04c8 },
	{ .reg = ANACTRL_PCIEPLL_CTRL0, .def = 0x300c04c8 },
	{ .reg = ANACTRL_PCIEPLL_CTRL1, .def = 0x30000000 },
	{ .reg = ANACTRL_PCIEPLL_CTRL2, .def = 0x00001100 },
	{ .reg = ANACTRL_PCIEPLL_CTRL3, .def = 0x10058e00 },
	{ .reg = ANACTRL_PCIEPLL_CTRL4, .def = 0x000100c0 },
	{ .reg = ANACTRL_PCIEPLL_CTRL5, .def = 0x68000048 },
	{ .reg = ANACTRL_PCIEPLL_CTRL5, .def = 0x68000068, .delay_us = 20 },
	{ .reg = ANACTRL_PCIEPLL_CTRL4, .def = 0x008100c0, .delay_us = 20 },
	{ .reg = ANACTRL_PCIEPLL_CTRL0, .def = 0x340c04c8 },
	{ .reg = ANACTRL_PCIEPLL_CTRL0, .def = 0x140c04c8, .delay_us = 20 },
	{ .reg = ANACTRL_PCIEPLL_CTRL2, .def = 0x00001000 }
};

static struct clk_regmap pcie_pll_dco = {
	.data = &(struct meson_clk_pll_data) {
		.en = {
			.reg_off = ANACTRL_PCIEPLL_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.m = {
			.reg_off = ANACTRL_PCIEPLL_CTRL0,
			.shift   = 0,
			.width   = 8,
		},
		.n = {
			.reg_off = ANACTRL_PCIEPLL_CTRL0,
			.shift   = 10,
			.width   = 5,
		},
		.frac = {
			.reg_off = ANACTRL_PCIEPLL_CTRL1,
			.shift   = 0,
			.width   = 12,
		},
		.l = {
			.reg_off = ANACTRL_PCIEPLL_STS0,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = ANACTRL_PCIEPLL_CTRL0,
			.shift   = 29,
			.width   = 1,
		},
		.table = pcie_pll_dco_table,
		.init_regs = pcie_pll_dco_init_regs,
		.init_count = ARRAY_SIZE(pcie_pll_dco_init_regs),
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_pll_dco",
		.ops = &meson_clk_pcie_pll_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE,
	},
};

static const struct pll_params_table pcie1_pll_dco_table[] = {
	PLL_PARAMS(150, 1),
	{ /* sentinel */ }
};

static const struct reg_sequence pcie1_pll_dco_init_regs[] = {
	{ .reg = ANACTRL_PCIEPLL_CTRL6, .def = 0x200c04c8 },
	{ .reg = ANACTRL_PCIEPLL_CTRL6, .def = 0x300c04c8 },
	{ .reg = ANACTRL_PCIEPLL_CTRL7, .def = 0x30000000 },
	{ .reg = ANACTRL_PCIEPLL_CTRL8, .def = 0x00001100 },
	{ .reg = ANACTRL_PCIEPLL_CTRL9, .def = 0x10058e00 },
	{ .reg = ANACTRL_PCIEPLL_CTRL10, .def = 0x000100c0 },
	{ .reg = ANACTRL_PCIEPLL_CTRL11, .def = 0x68000048 },
	{ .reg = ANACTRL_PCIEPLL_CTRL11, .def = 0x68000068, .delay_us = 20 },
	{ .reg = ANACTRL_PCIEPLL_CTRL10, .def = 0x008100c0, .delay_us = 20 },
	{ .reg = ANACTRL_PCIEPLL_CTRL6, .def = 0x340c04c8 },
	{ .reg = ANACTRL_PCIEPLL_CTRL6, .def = 0x140c04c8, .delay_us = 20 },
	{ .reg = ANACTRL_PCIEPLL_CTRL8, .def = 0x00001000 }
};

static struct clk_regmap pcie1_pll_dco = {
	.data = &(struct meson_clk_pll_data) {
		.en = {
			.reg_off = ANACTRL_PCIEPLL_CTRL6,
			.shift   = 28,
			.width   = 1,
		},
		.m = {
			.reg_off = ANACTRL_PCIEPLL_CTRL6,
			.shift   = 0,
			.width   = 8,
		},
		.n = {
			.reg_off = ANACTRL_PCIEPLL_CTRL6,
			.shift   = 10,
			.width   = 5,
		},
		.frac = {
			.reg_off = ANACTRL_PCIEPLL_CTRL7,
			.shift   = 0,
			.width   = 12,
		},
		.l = {
			.reg_off = ANACTRL_PCIEPLL_STS1,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = ANACTRL_PCIEPLL_CTRL6,
			.shift   = 29,
			.width   = 1,
		},
		.table = pcie1_pll_dco_table,
		.init_regs = pcie1_pll_dco_init_regs,
		.init_count = ARRAY_SIZE(pcie1_pll_dco_init_regs),
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie1_pll_dco",
		.ops = &meson_clk_pcie_pll_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap fclk_50m_en = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_FIXPLL_CTRL3,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "fclk_50m_en",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "fixed_pll",
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor fclk_50m = {
	.mult = 1,
	.div = 40,
	.hw.init = &(struct clk_init_data) {
		.name = "fclk_50m",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&fclk_50m_en.hw,
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
			.fw_name = "fixed_pll"
		},
		.num_parents = 1,
	},
};

static struct clk_regmap fclk_div2 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_FIXPLL_CTRL3,
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
			.fw_name = "fixed_pll"
		},
		.num_parents = 1,
	},
};

static struct clk_regmap fclk_div2p5 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_FIXPLL_CTRL3,
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
			.fw_name = "fixed_pll"
		},
		.num_parents = 1,
	},
};

static struct clk_regmap fclk_div3 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_FIXPLL_CTRL3,
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
			.fw_name = "fixed_pll"
		},
		.num_parents = 1,
	},
};

static struct clk_regmap fclk_div4 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_FIXPLL_CTRL3,
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
			.fw_name = "fixed_pll"
		},
		.num_parents = 1,
	},
};

static struct clk_regmap fclk_div5 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_FIXPLL_CTRL3,
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
			.fw_name = "fixed_pll"
		},
		.num_parents = 1,
	},
};

static struct clk_regmap fclk_div7 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_FIXPLL_CTRL3,
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

static struct clk_regmap fpll_tmds_od = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_FPLL_CTRL0,
		.shift = 21,
		.width = 2,
		.flags = CLK_DIVIDER_POWER_OF_TWO |
			 CLK_DIVIDER_ROUND_CLOSEST,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "fpll_tmds_od",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&fpll_dco.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap fpll_tmds_od1 = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_FPLL_CTRL0,
		.shift = 23,
		.width = 2,
		.flags = CLK_DIVIDER_POWER_OF_TWO |
			 CLK_DIVIDER_ROUND_CLOSEST,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "fpll_tmds_od1",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&fpll_tmds_od.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_fixed_factor fpll_tmds = {
	.mult = 1,
	.div = 2,
	.hw.init = &(struct clk_init_data) {
		.name = "fpll_tmds",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&fpll_tmds_od1.hw,
		},
		.num_parents = 1,
	},
};

#define CLK_MULTDIV_ALLOW_ZERO		BIT(0)
#define CLK_MULTDIV_ROUND_CLOSEST	BIT(1)

struct clk_mult_div_table {
	unsigned int	val;
	unsigned int	mult;
	unsigned int	div;
};

struct clk_regmap_mult_div_data {
	unsigned int	offset;
	u8		shift;
	u8		width;
	u8		flags;
	const struct clk_mult_div_table	*table;
};

static inline struct clk_regmap_mult_div_data *
clk_get_regmap_mult_div_data(struct clk_regmap *clk)
{
	return (struct clk_regmap_mult_div_data *)clk->data;
}

static unsigned long clk_regmap_mult_div_recalc_rate(struct clk_hw *hw,
						unsigned long prate)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_mult_div_data *mult_div =
		clk_get_regmap_mult_div_data(clk);
	const struct clk_mult_div_table *table;
	unsigned int val;
	unsigned long rate = 0;
	int ret;

	ret = regmap_read(clk->map, mult_div->offset, &val);
	if (ret)
		/* Gives a hint that something is wrong */
		return 0;

	val >>= mult_div->shift;
	val &= clk_div_mask(mult_div->width);
	if (mult_div->table) {  /* now without defined tables are not supported */
		for (table = mult_div->table; table->div; table++) {
			if (val == table->val) {
				rate = prate * table->mult / table->div;
				break;
			}
		}
	}

	return rate;
}

static bool mult_div_is_best_rate(unsigned long rate, unsigned long new,
			   unsigned long best, unsigned long flags)
{
	if (flags & CLK_MULTDIV_ROUND_CLOSEST)
		return abs(rate - new) < abs(rate - best);

	return new >= rate && new < best;
}

static long clk_regmap_mult_div_round_rate(struct clk_hw *hw,
				unsigned long rate, unsigned long *prate)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_mult_div_data *mult_div =
		clk_get_regmap_mult_div_data(clk);
	const struct clk_mult_div_table *table;
	unsigned long current_rate, best_rate = ~0;

	if (mult_div->table) {  /* now without defined tables are not supported */
		for (table = mult_div->table; table->div; table++) {
			current_rate = *prate * table->mult / table->div;
			if (mult_div_is_best_rate(rate, current_rate, best_rate,
				mult_div->flags))
				best_rate = current_rate;
		}
	}

	return (long)best_rate;
}

#define clk_mult_div_mask(width)	((1 << (width)) - 1)
static int clk_regmap_mult_div_set_rate(struct clk_hw *hw,
				unsigned long rate, unsigned long parent_rate)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_mult_div_data *mult_div =
		clk_get_regmap_mult_div_data(clk);
	const struct clk_mult_div_table *table = NULL;
	unsigned long current_rate, best_rate = ~0;
	unsigned int val, best_val = 0;

	if (mult_div->table) {  /* now without defined tables are not supported */
		for (table = mult_div->table; table->div; table++) {
			current_rate = parent_rate * table->mult / table->div;
			if (mult_div_is_best_rate(rate, current_rate, best_rate,
				mult_div->flags)) {
				best_rate = current_rate;
				best_val = table->val;
			}
		}
	}

	if (best_rate == ~0)
		return -EINVAL;

	val = (unsigned int)best_val << mult_div->shift;
	return regmap_update_bits(clk->map, mult_div->offset,
			clk_div_mask(mult_div->width) << mult_div->shift, val);
};

static const struct clk_ops clk_regmap_mult_div_ops = {
	.recalc_rate = clk_regmap_mult_div_recalc_rate,
	.round_rate = clk_regmap_mult_div_round_rate,
	.set_rate = clk_regmap_mult_div_set_rate,
};

static const struct clk_mult_div_table s5_pixel_mult_div_table[] = {
	{0,	4,	4},  /* val = 0; fact = 4 / 4 = 1 */
	{1,	4,	5},  /* val = 1; fact = 4 / 5 = 0.8 */
	{2,	4,	6},  /* val = 2; fact = 4 / 6 = 0.67 */
	{3,	4,	7},  /* val = 3; fact = 4 / 7 = 0.57 */
	{4,	4,	8},  /* val = 4; fact = 4 / 8 = 0.5 */
	{ /* sentinel */ }
};

static struct clk_regmap fpll_pixel = {
	.data = &(struct clk_regmap_mult_div_data) {
		.offset = CLKCTRL_FPLL_CTRL0,
		.shift = 13,
		.width = 3,
		.table = s5_pixel_mult_div_table,
		.flags = CLK_MULTDIV_ALLOW_ZERO |
			 CLK_MULTDIV_ROUND_CLOSEST,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "fpll_pixel",
		.ops = &clk_regmap_mult_div_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&fpll_tmds.hw,
		},
		.num_parents = 1,
		/*
		 * fdle pll is directly used in other modules, and the
		 * parent rate needs to be modified
		 * Register has the risk of being directly operated.
		 */
		.flags = CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap nna_pll_audio = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_NNAPLL_CTRL0,
		.shift = 14,
		.width = 2,
		.flags = CLK_DIVIDER_POWER_OF_TWO |
			 CLK_DIVIDER_ROUND_CLOSEST,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "nna_pll_audio",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&nna_pll.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor pcie_pll_dco_div2 = {
	.mult = 1,
	.div = 2,
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_pll_dco_div2",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie_pll_dco.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap pcie_pll_od = {
	.data = &(struct clk_regmap_div_data) {
		.offset = ANACTRL_PCIEPLL_CTRL0,
		.shift = 16,
		.width = 5,
		.flags = CLK_DIVIDER_ONE_BASED |
			 CLK_DIVIDER_ALLOW_ZERO |
			 CLK_DIVIDER_ROUND_CLOSEST,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_pll_od",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie_pll_dco_div2.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_fixed_factor pcie_pll = {
	.mult = 1,
	.div = 2,
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_pll",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie_pll_od.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap pcie_bgp = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = ANACTRL_PCIEPLL_CTRL5,
		.bit_idx = 27,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_bgp",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie_pll.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_regmap pcie_hcsl = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = ANACTRL_PCIEPLL_CTRL5,
		.bit_idx = 3,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_hcsl",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie_bgp.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_regmap pcie_hcsl_in_pad = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = ANACTRL_PCIEPLL_CTRL5,
		.bit_idx = 2,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_hcsl_in_pad",
		.ops = &clk_regmap_gate_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "pcie_hcsl_pad",
		},
		.num_parents = 1,
	},
};

static const struct clk_parent_data pcie_clk_in_parent_data[] = {
	{ .hw = &pcie_pll.hw },
	{ .hw = &pcie_hcsl_in_pad.hw }
};

static struct clk_regmap pcie_clk_in = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = ANACTRL_PCIEPLL_CTRL5,
		.mask = 0x1,
		.shift = 2,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_clk_in",
		.ops = &clk_regmap_mux_ops,
		.parent_data = pcie_clk_in_parent_data,
		.num_parents = ARRAY_SIZE(pcie_clk_in_parent_data),
	},
};

static struct clk_fixed_factor pcie1_pll_dco_div2 = {
	.mult = 1,
	.div = 2,
	.hw.init = &(struct clk_init_data) {
		.name = "pcie1_pll_dco_div2",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie1_pll_dco.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap pcie1_pll_od = {
	.data = &(struct clk_regmap_div_data) {
		.offset = ANACTRL_PCIEPLL_CTRL6,
		.shift = 16,
		.width = 5,
		.flags = CLK_DIVIDER_ONE_BASED |
			 CLK_DIVIDER_ALLOW_ZERO |
			 CLK_DIVIDER_ROUND_CLOSEST,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie1_pll_od",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie1_pll_dco_div2.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_fixed_factor pcie1_pll = {
	.mult = 1,
	.div = 2,
	.hw.init = &(struct clk_init_data) {
		.name = "pcie1_pll",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie1_pll_od.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap pcie1_bgp = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = ANACTRL_PCIEPLL_CTRL11,
		.bit_idx = 27,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie1_bgp",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie1_pll.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_regmap pcie1_hcsl = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = ANACTRL_PCIEPLL_CTRL11,
		.bit_idx = 3,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie1_hcsl",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie1_bgp.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_regmap pcie1_hcsl_in_pad = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = ANACTRL_PCIEPLL_CTRL11,
		.bit_idx = 2,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie1_hcsl_in_pad",
		.ops = &clk_regmap_gate_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "pcie1_hcsl_pad",
		},
		.num_parents = 1,
	},
};

static const struct clk_parent_data pcie1_clk_in_parent_data[] = {
	{ .hw = &pcie1_pll.hw },
	{ .hw = &pcie1_hcsl_in_pad.hw }
};

static struct clk_regmap pcie1_clk_in = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = ANACTRL_PCIEPLL_CTRL11,
		.mask = 0x1,
		.shift = 2,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie1_clk_in",
		.ops = &clk_regmap_mux_ops,
		.parent_data = pcie1_clk_in_parent_data,
		.num_parents = ARRAY_SIZE(pcie1_clk_in_parent_data),
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
			.fw_name = "xtal",
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
	{ .fw_name = "xtal" },
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

static struct clk_regmap ceca_dual_clkin = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_CECA_CTRL0,
		.bit_idx = 31,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "ceca_dual_clkin",
		.ops = &clk_regmap_gate_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static const struct meson_clk_dualdiv_param ceca_dual_div_table[] = {
	{ 733, 732, 8, 11, 1 },
	{ }
};

static struct clk_regmap ceca_dual_div = {
	.data = &(struct meson_clk_dualdiv_data) {
		.n1 = {
			.reg_off = CLKCTRL_CECA_CTRL0,
			.shift   = 0,
			.width   = 12,
		},
		.n2 = {
			.reg_off = CLKCTRL_CECA_CTRL0,
			.shift   = 12,
			.width   = 12,
		},
		.m1 = {
			.reg_off = CLKCTRL_CECA_CTRL1,
			.shift   = 0,
			.width   = 12,
		},
		.m2 = {
			.reg_off = CLKCTRL_CECA_CTRL1,
			.shift   = 12,
			.width   = 12,
		},
		.dual = {
			.reg_off = CLKCTRL_CECA_CTRL0,
			.shift   = 28,
			.width   = 2,
		},
		.table = ceca_dual_div_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "ceca_dual_div",
		.ops = &meson_clk_dualdiv_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&ceca_dual_clkin.hw,
		},
		.num_parents = 1,
	},
};

static const struct clk_parent_data ceca_dual_parent_data[] = {
	{ .hw = &ceca_dual_div.hw },
	{ .hw = &ceca_dual_clkin.hw }
};

static struct clk_regmap ceca_dual_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_CECA_CTRL0,
		.mask = 0x1,
		.shift = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "ceca_dual_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = ceca_dual_parent_data,
		.num_parents = ARRAY_SIZE(ceca_dual_parent_data),
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap ceca_dual = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_CECA_CTRL0,
		.bit_idx = 30,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "ceca_dual",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&ceca_dual_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data ceca_clk_parent_data[] = {
	{ .hw = &ceca_dual.hw },
	{ .hw = &rtc_clk.hw }
};

static struct clk_regmap ceca_clk = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_CECA_CTRL1,
		.mask = 0x1,
		.shift = 31,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "ceca_clk",
		.ops = &clk_regmap_mux_ops,
		.parent_data = ceca_clk_parent_data,
		.num_parents = ARRAY_SIZE(ceca_clk_parent_data),
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap cecb_dual_clkin = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_CECB_CTRL0,
		.bit_idx = 31,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cecb_dual_clkin",
		.ops = &clk_regmap_gate_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static const struct meson_clk_dualdiv_param cecb_dual_div_table[] = {
	{ 733, 732, 8, 11, 1 },
	{ }
};

static struct clk_regmap cecb_dual_div = {
	.data = &(struct meson_clk_dualdiv_data) {
		.n1 = {
			.reg_off = CLKCTRL_CECB_CTRL0,
			.shift   = 0,
			.width   = 12,
		},
		.n2 = {
			.reg_off = CLKCTRL_CECB_CTRL0,
			.shift   = 12,
			.width   = 12,
		},
		.m1 = {
			.reg_off = CLKCTRL_CECB_CTRL1,
			.shift   = 0,
			.width   = 12,
		},
		.m2 = {
			.reg_off = CLKCTRL_CECB_CTRL1,
			.shift   = 12,
			.width   = 12,
		},
		.dual = {
			.reg_off = CLKCTRL_CECB_CTRL0,
			.shift   = 28,
			.width   = 2,
		},
		.table = cecb_dual_div_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cecb_dual_div",
		.ops = &meson_clk_dualdiv_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&cecb_dual_clkin.hw,
		},
		.num_parents = 1,
	},
};

static const struct clk_parent_data cecb_dual_parent_data[] = {
	{ .hw = &cecb_dual_div.hw },
	{ .hw = &cecb_dual_clkin.hw }
};

static struct clk_regmap cecb_dual_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_CECB_CTRL0,
		.mask = 0x1,
		.shift = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cecb_dual_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = cecb_dual_parent_data,
		.num_parents = ARRAY_SIZE(cecb_dual_parent_data),
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap cecb_dual = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_CECB_CTRL0,
		.bit_idx = 30,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cecb_dual",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&cecb_dual_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data cecb_clk_parent_data[] = {
	{ .hw = &cecb_dual.hw },
	{ .hw = &rtc_clk.hw }
};

static struct clk_regmap cecb_clk = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_CECB_CTRL1,
		.mask = 0x1,
		.shift = 31,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cecb_clk",
		.ops = &clk_regmap_mux_ops,
		.parent_data = cecb_clk_parent_data,
		.num_parents = ARRAY_SIZE(cecb_clk_parent_data),
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data vapb_parent_data[] = {
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &fclk_div7.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &sys2_pll.hw },
	{ .hw = &nna_pll.hw },
	{ .fw_name = "xtal" }
};

static struct clk_regmap vapb_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VAPBCLK_CTRL,
		.mask = 0x7,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vapb_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = vapb_parent_data,
		.num_parents = ARRAY_SIZE(vapb_parent_data),
	},
};

static struct clk_regmap vapb_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VAPBCLK_CTRL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vapb_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vapb_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap vapb = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VAPBCLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vapb",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vapb_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data ge2d_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &fclk_div2p5.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &nna_pll.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &fclk_div4.hw },
	{ .hw = &sys2_pll.hw },
	{ .hw = &rtc_clk.hw }
};

static struct clk_regmap ge2d_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_GE2DCLK_CTRL,
		.mask = 0x7,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "ge2d_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = ge2d_parent_data,
		.num_parents = ARRAY_SIZE(ge2d_parent_data),
	},
};

static struct clk_regmap ge2d_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_GE2DCLK_CTRL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "ge2d_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&ge2d_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap ge2d = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_GE2DCLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "ge2d",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&ge2d_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data sc_parent_data[] = {
	{ .hw = &fclk_div2.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div5.hw },
	{ .fw_name = "xtal" },
	{ .hw = &nna_pll.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &sys2_pll.hw },
	{ .hw = &fclk_div7.hw }
};

static struct clk_regmap sc_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_SC_CLK_CTRL,
		.mask = 0x3,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sc_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = sc_parent_data,
		.num_parents = ARRAY_SIZE(sc_parent_data),
	},
};

static struct clk_regmap sc_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_SC_CLK_CTRL,
		.shift = 0,
		.width = 8,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sc_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&sc_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap sc = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_SC_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sc",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&sc_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static u32 video_src_01_parent_table[] = { 1, 2, 3, 4, 5, 6, 7 };
static const struct clk_parent_data video_src_01_parent_data[] = {
	{ .hw = &gp2_pll.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &nna_pll.hw },
	{ .hw = &fpll_pixel.hw },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &fclk_div7.hw }
};

static struct clk_regmap video_src0_in_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VID_CLK0_CTRL,
		.mask = 0x7,
		.shift = 16,
		.table = video_src_01_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video_src0_in_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = video_src_01_parent_data,
		.num_parents = ARRAY_SIZE(video_src_01_parent_data),
	},
};

static struct clk_regmap video_src0_in = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_CLK0_DIV,
		.bit_idx = 16,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video_src0_in",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src0_in_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap video_src0_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VID_CLK0_DIV,
		.shift = 0,
		.width = 8,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video_src0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src0_in.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap video_src0 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_CLK0_CTRL,
		.bit_idx = 19,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video_src0",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src0_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap video_src1_in_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VIID_CLK0_CTRL,
		.mask = 0x7,
		.shift = 16,
		.table = video_src_01_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video_src1_in_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = video_src_01_parent_data,
		.num_parents = ARRAY_SIZE(video_src_01_parent_data),
	},
};

static struct clk_regmap video_src1_in = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VIID_CLK0_DIV,
		.bit_idx = 16,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video_src1_in",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src1_in_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap video_src1_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VIID_CLK0_DIV,
		.shift = 0,
		.width = 8,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video_src1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src1_in.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap video_src1 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VIID_CLK0_CTRL,
		.bit_idx = 19,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video_src1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src1_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap video0_div1 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_CLK0_CTRL,
		.bit_idx = 0,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video0_div1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src0.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap video0_div2_gate = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_CLK0_CTRL,
		.bit_idx = 1,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video0_div2_gate",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src0.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_fixed_factor video0_div2 = {
	.mult = 1,
	.div = 2,
	.hw.init = &(struct clk_init_data) {
		.name = "video0_div2",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video0_div2_gate.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap video0_div4_gate = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_CLK0_CTRL,
		.bit_idx = 2,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video0_div4_gate",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src0.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_fixed_factor video0_div4 = {
	.mult = 1,
	.div = 4,
	.hw.init = &(struct clk_init_data) {
		.name = "video0_div4",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video0_div4_gate.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap video0_div6_gate = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_CLK0_CTRL,
		.bit_idx = 3,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video0_div6_gate",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src0.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_fixed_factor video0_div6 = {
	.mult = 1,
	.div = 6,
	.hw.init = &(struct clk_init_data) {
		.name = "video0_div6",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video0_div6_gate.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap video0_div12_gate = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_CLK0_CTRL,
		.bit_idx = 4,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video0_div12_gate",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src0.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_fixed_factor video0_div12 = {
	.mult = 1,
	.div = 12,
	.hw.init = &(struct clk_init_data) {
		.name = "video0_div12",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video0_div12_gate.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap video2_div1 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VIID_CLK0_CTRL,
		.bit_idx = 0,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video2_div1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src1.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap video2_div2_gate = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VIID_CLK0_CTRL,
		.bit_idx = 1,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video2_div2_gate",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src1.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_fixed_factor video2_div2 = {
	.mult = 1,
	.div = 2,
	.hw.init = &(struct clk_init_data) {
		.name = "video2_div2",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video2_div2_gate.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap video2_div4_gate = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VIID_CLK0_CTRL,
		.bit_idx = 2,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video2_div4_gate",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src1.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_fixed_factor video2_div4 = {
	.mult = 1,
	.div = 4,
	.hw.init = &(struct clk_init_data) {
		.name = "video2_div4",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video2_div4_gate.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap video2_div6_gate = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VIID_CLK0_CTRL,
		.bit_idx = 3,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video2_div6_gate",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src1.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_fixed_factor video2_div6 = {
	.mult = 1,
	.div = 6,
	.hw.init = &(struct clk_init_data) {
		.name = "video2_div6",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video2_div6_gate.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap video2_div12_gate = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VIID_CLK0_CTRL,
		.bit_idx = 4,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "video2_div12_gate",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video_src1.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_fixed_factor video2_div12 = {
	.mult = 1,
	.div = 12,
	.hw.init = &(struct clk_init_data) {
		.name = "video2_div12",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&video2_div12_gate.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static u32 enc_hdmitx_parent_table[] = { 0, 1, 2, 3, 4, 8, 9, 10, 11, 12 };
static const struct clk_parent_data enc_hdmitx_parent_data[] = {
	{ .hw = &video0_div1.hw },
	{ .hw = &video0_div2.hw },
	{ .hw = &video0_div4.hw },
	{ .hw = &video0_div6.hw },
	{ .hw = &video0_div12.hw },
	{ .hw = &video2_div1.hw },
	{ .hw = &video2_div2.hw },
	{ .hw = &video2_div4.hw },
	{ .hw = &video2_div6.hw },
	{ .hw = &video2_div12.hw }
};

static struct clk_regmap enci_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VID_CLK0_DIV,
		.mask = 0xf,
		.shift = 28,
		.table = enc_hdmitx_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "enci_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = enc_hdmitx_parent_data,
		.num_parents = ARRAY_SIZE(enc_hdmitx_parent_data),
	},
};

static struct clk_regmap enci = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_CLK0_CTRL2,
		.bit_idx = 0,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "enci",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&enci_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap enct_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VID_CLK0_DIV,
		.mask = 0xf,
		.shift = 20,
		.table = enc_hdmitx_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "enct_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = enc_hdmitx_parent_data,
		.num_parents = ARRAY_SIZE(enc_hdmitx_parent_data),
	},
};

static struct clk_regmap enct = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_CLK0_CTRL2,
		.bit_idx = 1,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "enct",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&enct_mux.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_regmap encp_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VID_CLK0_DIV,
		.mask = 0xf,
		.shift = 24,
		.table = enc_hdmitx_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "encp_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = enc_hdmitx_parent_data,
		.num_parents = ARRAY_SIZE(enc_hdmitx_parent_data),
	},
};

static struct clk_regmap encp = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_CLK0_CTRL2,
		.bit_idx = 2,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "encp",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&encp_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap encl_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VIID_CLK0_DIV,
		.mask = 0xf,
		.shift = 12,
		.table = enc_hdmitx_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "encl_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = enc_hdmitx_parent_data,
		.num_parents = ARRAY_SIZE(enc_hdmitx_parent_data),
	},
};

static struct clk_regmap encl = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_CLK0_CTRL2,
		.bit_idx = 3,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "encl",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&encl_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap vdac_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VIID_CLK0_DIV,
		.mask = 0xf,
		.shift = 28,
		.table = enc_hdmitx_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vdac_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = enc_hdmitx_parent_data,
		.num_parents = ARRAY_SIZE(enc_hdmitx_parent_data),
	},
};

static struct clk_regmap vdac = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_CLK0_CTRL2,
		.bit_idx = 4,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vdac",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vdac_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap enc0_hdmitx_pixel_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_ENC0_HDMI_CLK_CTRL,
		.mask = 0xf,
		.shift = 16,
		.table = enc_hdmitx_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "enc0_hdmitx_pixel_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = enc_hdmitx_parent_data,
		.num_parents = ARRAY_SIZE(enc_hdmitx_parent_data),
	},
};

static struct clk_regmap enc0_hdmitx_pixel = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_CLK0_CTRL2,
		.bit_idx = 5,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "enc0_hdmitx_pixel",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&enc0_hdmitx_pixel_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap enc0_hdmitx_fe_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_ENC0_HDMI_CLK_CTRL,
		.mask = 0xf,
		.shift = 20,
		.table = enc_hdmitx_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "enc0_hdmitx_fe_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = enc_hdmitx_parent_data,
		.num_parents = ARRAY_SIZE(enc_hdmitx_parent_data),
	},
};

static struct clk_regmap enc0_hdmitx_fe = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_CLK0_CTRL2,
		.bit_idx = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "enc0_hdmitx_fe",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&enc0_hdmitx_fe_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap enc0_hdmitx_pnx_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_ENC0_HDMI_CLK_CTRL,
		.mask = 0xf,
		.shift = 24,
		.table = enc_hdmitx_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "enc0_hdmitx_pnx_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = enc_hdmitx_parent_data,
		.num_parents = ARRAY_SIZE(enc_hdmitx_parent_data),
	},
};

static struct clk_regmap enc0_hdmitx_pnx = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_CLK0_CTRL2,
		.bit_idx = 10,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "enc0_hdmitx_pnx",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&enc0_hdmitx_pnx_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static const struct clk_parent_data hdmitx_pixel_parent_data[] = {
	{ .hw = &enc0_hdmitx_pixel.hw },
	{ .hw = &gp2_pll.hw }
};

static struct clk_regmap hdmitx_pixel_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_ENC_HDMI_CLK_CTRL,
		.mask = 0x3,
		.shift = 5,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_pixel_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = hdmitx_pixel_parent_data,
		.num_parents = ARRAY_SIZE(hdmitx_pixel_parent_data),
	},
};

static struct clk_regmap hdmitx_pixel_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_ENC_HDMI_CLK_CTRL,
		.shift = 0,
		.width = 4,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_pixel_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hdmitx_pixel_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap hdmitx_pixel = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_ENC_HDMI_CLK_CTRL,
		.bit_idx = 4,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_pixel",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hdmitx_pixel_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static const struct clk_parent_data hdmitx_fe_parent_data[] = {
	{ .hw = &enc0_hdmitx_fe.hw },
	{ .hw = &gp2_pll.hw }
};

static struct clk_regmap hdmitx_fe_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_ENC_HDMI_CLK_CTRL,
		.mask = 0x3,
		.shift = 13,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_fe_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = hdmitx_fe_parent_data,
		.num_parents = ARRAY_SIZE(hdmitx_fe_parent_data),
	},
};

static struct clk_regmap hdmitx_fe_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_ENC_HDMI_CLK_CTRL,
		.shift = 8,
		.width = 4,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_fe_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hdmitx_fe_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap hdmitx_fe = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_ENC_HDMI_CLK_CTRL,
		.bit_idx = 12,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_fe",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hdmitx_fe_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static const struct clk_parent_data hdmitx_pnx_parent_data[] = {
	{ .hw = &enc0_hdmitx_pnx.hw },
	{ .hw = &gp2_pll.hw }
};

static struct clk_regmap hdmitx_pnx_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_ENC_HDMI_CLK_CTRL,
		.mask = 0x3,
		.shift = 21,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_pnx_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = hdmitx_pnx_parent_data,
		.num_parents = ARRAY_SIZE(hdmitx_pnx_parent_data),
	},
};

static struct clk_regmap hdmitx_pnx_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_ENC_HDMI_CLK_CTRL,
		.shift = 16,
		.width = 4,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_pnx_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hdmitx_pnx_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap hdmitx_pnx = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_ENC_HDMI_CLK_CTRL,
		.bit_idx = 20,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_pnx",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hdmitx_pnx_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static u32 vpu_01_parent_table[] = { 0, 1, 2, 3, 4, 6, 7 };
static const struct clk_parent_data vpu_01_parent_data[] = {
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &fclk_div7.hw },
	{ .hw = &nna_pll.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &hifi1_pll.hw }
};

static struct clk_regmap vpu_0_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VPU_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
		.table = vpu_01_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_0_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = vpu_01_parent_data,
		.num_parents = ARRAY_SIZE(vpu_01_parent_data),
	},
};

static struct clk_regmap vpu_0_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VPU_CLK_CTRL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vpu_0_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap vpu_0 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VPU_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_0",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vpu_0_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap vpu_1_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VPU_CLK_CTRL,
		.mask = 0x7,
		.shift = 25,
		.table = vpu_01_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_1_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = vpu_01_parent_data,
		.num_parents = ARRAY_SIZE(vpu_01_parent_data),
	},
};

static struct clk_regmap vpu_1_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VPU_CLK_CTRL,
		.shift = 16,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vpu_1_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap vpu_1 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VPU_CLK_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vpu_1_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data vpu_parent_data[] = {
	{ .hw = &vpu_0.hw },
	{ .hw = &vpu_1.hw }
};

static struct clk_regmap vpu = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VPU_CLK_CTRL,
		.mask = 0x1,
		.shift = 31,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu",
		.ops = &clk_regmap_mux_ops,
		.parent_data = vpu_parent_data,
		.num_parents = ARRAY_SIZE(vpu_parent_data),
		.flags = CLK_SET_RATE_PARENT |
			 CLK_OPS_PARENT_ENABLE,
	},
};

static const struct clk_parent_data vpu_clkb_tmp_parent_data[] = {
	{ .hw = &vpu.hw },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &fclk_div7.hw }
};

static struct clk_regmap vpu_clkb_tmp_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VPU_CLKB_CTRL,
		.mask = 0x3,
		.shift = 20,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_clkb_tmp_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = vpu_clkb_tmp_parent_data,
		.num_parents = ARRAY_SIZE(vpu_clkb_tmp_parent_data),
	},
};

static struct clk_regmap vpu_clkb_tmp_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VPU_CLKB_CTRL,
		.shift = 16,
		.width = 4,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_clkb_tmp_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vpu_clkb_tmp_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap vpu_clkb_tmp = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VPU_CLKB_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_clkb_tmp",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vpu_clkb_tmp_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap vpu_clkb_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VPU_CLKB_CTRL,
		.shift = 0,
		.width = 8,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_clkb_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vpu_clkb_tmp.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap vpu_clkb = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VPU_CLKB_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_clkb",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vpu_clkb_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static u32 vdin_meas_parent_table[] = { 0, 1, 2, 3, 5, 6, 7 };
static const struct clk_parent_data vdin_meas_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &sys2_pll.hw },
	{ .hw = &nna_pll.hw },
	{ .hw = &hifi1_pll.hw }
};

static struct clk_regmap vdin_meas_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VDIN_MEAS_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
		.table = vdin_meas_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vdin_meas_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = vdin_meas_parent_data,
		.num_parents = ARRAY_SIZE(vdin_meas_parent_data),
	},
};

static struct clk_regmap vdin_meas_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VDIN_MEAS_CLK_CTRL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vdin_meas_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vdin_meas_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap vdin_meas = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VDIN_MEAS_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vdin_meas",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vdin_meas_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static u32 vid_lock_parent_table[] = { 0, 2, 3 };
static const struct clk_parent_data vid_lock_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &sys2_pll.hw },
	{ .hw = &nna_pll.hw }
};

static struct clk_regmap vid_lock_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VID_LOCK_CLK_CTRL,
		.mask = 0x3,
		.shift = 8,
		.table = vid_lock_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vid_lock_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = vid_lock_parent_data,
		.num_parents = ARRAY_SIZE(vid_lock_parent_data),
	},
};

static struct clk_regmap vid_lock_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VID_LOCK_CLK_CTRL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vid_lock_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vid_lock_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap vid_lock = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VID_LOCK_CLK_CTRL,
		.bit_idx = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vid_lock",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vid_lock_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data cmpr_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &fclk_div7.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &gp0_pll.hw }
};

static struct clk_regmap cmpr_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_CMPR_CLK_CTRL,
		.mask = 0x7,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cmpr_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = cmpr_parent_data,
		.num_parents = ARRAY_SIZE(cmpr_parent_data),
	},
};

static struct clk_regmap cmpr_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_CMPR_CLK_CTRL,
		.shift = 16,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cmpr_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&cmpr_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap cmpr = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_CMPR_CLK_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cmpr",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&cmpr_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data mali_01_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &sys2_pll.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &fclk_div2p5.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &nna_pll.hw }
};

static struct clk_regmap mali_0_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_MALI_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "mali_0_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = mali_01_parent_data,
		.num_parents = ARRAY_SIZE(mali_01_parent_data),
	},
};

static struct clk_regmap mali_0_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_MALI_CLK_CTRL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "mali_0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&mali_0_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap mali_0 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_MALI_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "mali_0",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&mali_0_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_SET_RATE_GATE,
	},
};

static struct clk_regmap mali_1_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_MALI_CLK_CTRL,
		.mask = 0x7,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "mali_1_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = mali_01_parent_data,
		.num_parents = ARRAY_SIZE(mali_01_parent_data),
	},
};

static struct clk_regmap mali_1_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_MALI_CLK_CTRL,
		.shift = 16,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "mali_1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&mali_1_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap mali_1 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_MALI_CLK_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "mali_1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&mali_1_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_SET_RATE_GATE,
	},
};

static const struct clk_parent_data mali_parent_data[] = {
	{ .hw = &mali_0.hw },
	{ .hw = &mali_1.hw }
};

static struct clk_regmap mali = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_MALI_CLK_CTRL,
		.mask = 0x1,
		.shift = 31,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "mali",
		.ops = &clk_regmap_mux_ops,
		.parent_data = mali_parent_data,
		.num_parents = ARRAY_SIZE(mali_parent_data),
		.flags = CLK_SET_RATE_PARENT |
			 CLK_OPS_PARENT_ENABLE,
	},
};

static const struct clk_parent_data vdec_01_parent_data[] = {
	{ .hw = &fclk_div2p5.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &fclk_div7.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &nna_pll.hw },
	{ .fw_name = "xtal" }
};

static struct clk_regmap vdec_0_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VDEC_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vdec_0_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = vdec_01_parent_data,
		.num_parents = ARRAY_SIZE(vdec_01_parent_data),
	},
};

static struct clk_regmap vdec_0_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VDEC_CLK_CTRL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vdec_0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vdec_0_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap vdec_0 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VDEC_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vdec_0",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vdec_0_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_SET_RATE_GATE,
	},
};

static struct clk_regmap vdec_1_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VDEC3_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vdec_1_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = vdec_01_parent_data,
		.num_parents = ARRAY_SIZE(vdec_01_parent_data),
	},
};

static struct clk_regmap vdec_1_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VDEC3_CLK_CTRL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vdec_1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vdec_1_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap vdec_1 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VDEC3_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vdec_1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vdec_1_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_SET_RATE_GATE,
	},
};

static const struct clk_parent_data vdec_parent_data[] = {
	{ .hw = &vdec_0.hw },
	{ .hw = &vdec_1.hw }
};

static struct clk_regmap vdec = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VDEC3_CLK_CTRL,
		.mask = 0x1,
		.shift = 15,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vdec",
		.ops = &clk_regmap_mux_ops,
		.parent_data = vdec_parent_data,
		.num_parents = ARRAY_SIZE(vdec_parent_data),
		.flags = CLK_SET_RATE_PARENT |
			 CLK_OPS_PARENT_ENABLE,
	},
};

static const struct clk_parent_data hcodec_01_parent_data[] = {
	{ .hw = &fclk_div2p5.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &fclk_div7.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &nna_pll.hw },
	{ .fw_name = "xtal" }
};

static struct clk_regmap hcodec_0_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VDEC_CLK_CTRL,
		.mask = 0x7,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hcodec_0_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = hcodec_01_parent_data,
		.num_parents = ARRAY_SIZE(hcodec_01_parent_data),
	},
};

static struct clk_regmap hcodec_0_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VDEC_CLK_CTRL,
		.shift = 16,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hcodec_0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hcodec_0_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap hcodec_0 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VDEC_CLK_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hcodec_0",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hcodec_0_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_SET_RATE_GATE,
	},
};

static struct clk_regmap hcodec_1_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VDEC3_CLK_CTRL,
		.mask = 0x7,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hcodec_1_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = hcodec_01_parent_data,
		.num_parents = ARRAY_SIZE(hcodec_01_parent_data),
	},
};

static struct clk_regmap hcodec_1_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VDEC3_CLK_CTRL,
		.shift = 16,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hcodec_1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hcodec_1_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap hcodec_1 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VDEC3_CLK_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hcodec_1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hcodec_1_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_SET_RATE_GATE,
	},
};

static const struct clk_parent_data hcodec_parent_data[] = {
	{ .hw = &hcodec_0.hw },
	{ .hw = &hcodec_1.hw }
};

static struct clk_regmap hcodec = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VDEC3_CLK_CTRL,
		.mask = 0x1,
		.shift = 31,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hcodec",
		.ops = &clk_regmap_mux_ops,
		.parent_data = hcodec_parent_data,
		.num_parents = ARRAY_SIZE(hcodec_parent_data),
		.flags = CLK_SET_RATE_PARENT |
			 CLK_OPS_PARENT_ENABLE,
	},
};

static const struct clk_parent_data hevc_01_parent_data[] = {
	{ .hw = &fclk_div2p5.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &fclk_div7.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &nna_pll.hw },
	{ .fw_name = "xtal" }
};

static struct clk_regmap hevc_0_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VDEC2_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hevc_0_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = hevc_01_parent_data,
		.num_parents = ARRAY_SIZE(hevc_01_parent_data),
	},
};

static struct clk_regmap hevc_0_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VDEC2_CLK_CTRL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hevc_0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hevc_0_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap hevc_0 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VDEC2_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hevc_0",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hevc_0_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_SET_RATE_GATE,
	},
};

static struct clk_regmap hevc_1_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VDEC4_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hevc_1_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = hevc_01_parent_data,
		.num_parents = ARRAY_SIZE(hevc_01_parent_data),
	},
};

static struct clk_regmap hevc_1_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VDEC4_CLK_CTRL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hevc_1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hevc_1_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap hevc_1 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VDEC4_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hevc_1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hevc_1_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_SET_RATE_GATE,
	},
};

static const struct clk_parent_data hevc_parent_data[] = {
	{ .hw = &hevc_0.hw },
	{ .hw = &hevc_1.hw }
};

static struct clk_regmap hevc = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VDEC4_CLK_CTRL,
		.mask = 0x1,
		.shift = 15,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hevc",
		.ops = &clk_regmap_mux_ops,
		.parent_data = hevc_parent_data,
		.num_parents = ARRAY_SIZE(hevc_parent_data),
		.flags = CLK_SET_RATE_PARENT |
			 CLK_OPS_PARENT_ENABLE,
	},
};

static const struct clk_parent_data vc9000e_axi_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &fclk_div7.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &nna_pll.hw }
};

static struct clk_regmap vc9000e_axi_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VC9000E_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vc9000e_axi_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = vc9000e_axi_parent_data,
		.num_parents = ARRAY_SIZE(vc9000e_axi_parent_data),
	},
};

static struct clk_regmap vc9000e_axi_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VC9000E_CLK_CTRL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vc9000e_axi_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vc9000e_axi_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap vc9000e_axi = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VC9000E_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vc9000e_axi",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vc9000e_axi_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data vc9000e_core_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &fclk_div7.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &nna_pll.hw }
};

static struct clk_regmap vc9000e_core_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_VC9000E_CLK_CTRL,
		.mask = 0x7,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vc9000e_core_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = vc9000e_core_parent_data,
		.num_parents = ARRAY_SIZE(vc9000e_core_parent_data),
	},
};

static struct clk_regmap vc9000e_core_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_VC9000E_CLK_CTRL,
		.shift = 16,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vc9000e_core_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vc9000e_core_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap vc9000e_core = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_VC9000E_CLK_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vc9000e_core",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&vc9000e_core_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data hdmitx_sys_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div5.hw }
};

static struct clk_regmap hdmitx_sys_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_HDMI_CLK_CTRL,
		.mask = 0x3,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_sys_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = hdmitx_sys_parent_data,
		.num_parents = ARRAY_SIZE(hdmitx_sys_parent_data),
	},
};

static struct clk_regmap hdmitx_sys_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_HDMI_CLK_CTRL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_sys_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hdmitx_sys_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap hdmitx_sys = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_HDMI_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_sys",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hdmitx_sys_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static const struct clk_parent_data hdmitx_prif_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div5.hw }
};

static struct clk_regmap hdmitx_prif_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_HTX_CLK_CTRL0,
		.mask = 0x3,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_prif_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = hdmitx_prif_parent_data,
		.num_parents = ARRAY_SIZE(hdmitx_prif_parent_data),
	},
};

static struct clk_regmap hdmitx_prif_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_HTX_CLK_CTRL0,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_prif_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hdmitx_prif_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap hdmitx_prif = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_HTX_CLK_CTRL0,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_prif",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hdmitx_prif_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static const struct clk_parent_data hdmitx_200m_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div5.hw }
};

static struct clk_regmap hdmitx_200m_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_HTX_CLK_CTRL0,
		.mask = 0x3,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_200m_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = hdmitx_200m_parent_data,
		.num_parents = ARRAY_SIZE(hdmitx_200m_parent_data),
	},
};

static struct clk_regmap hdmitx_200m_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_HTX_CLK_CTRL0,
		.shift = 16,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_200m_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hdmitx_200m_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap hdmitx_200m = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_HTX_CLK_CTRL0,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_200m",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hdmitx_200m_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static const struct clk_parent_data hdmitx_aud_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div5.hw }
};

static struct clk_regmap hdmitx_aud_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_HTX_CLK_CTRL1,
		.mask = 0x3,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_aud_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = hdmitx_aud_parent_data,
		.num_parents = ARRAY_SIZE(hdmitx_aud_parent_data),
	},
};

static struct clk_regmap hdmitx_aud_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_HTX_CLK_CTRL1,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_aud_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hdmitx_aud_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap hdmitx_aud = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_HTX_CLK_CTRL1,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmitx_aud",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&hdmitx_aud_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static u32 htx_tmds_parent_table[] = { 1, 2 };
static const struct clk_parent_data htx_tmds_parent_data[] = {
	{ .hw = &fpll_tmds.hw },
	{ .hw = &fclk_div3.hw }
};

static struct clk_regmap htx_tmds_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_HTX_CLK_CTRL1,
		.mask = 0x3,
		.shift = 25,
		.table = htx_tmds_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "htx_tmds_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = htx_tmds_parent_data,
		.num_parents = ARRAY_SIZE(htx_tmds_parent_data),
	},
};

static struct clk_regmap htx_tmds_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_HTX_CLK_CTRL1,
		.shift = 16,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "htx_tmds_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&htx_tmds_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap htx_tmds = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_HTX_CLK_CTRL1,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "htx_tmds",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&htx_tmds_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static const struct clk_parent_data nna_01_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &fclk_div2p5.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &nna_pll.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &fclk_div4.hw },
	{ .hw = &sys2_pll.hw },
	{ .hw = &rtc_clk.hw }
};

static struct clk_regmap nna_0_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_NNA_CLK_CTRL0,
		.mask = 0x7,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "nna_0_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = nna_01_parent_data,
		.num_parents = ARRAY_SIZE(nna_01_parent_data),
	},
};

static struct clk_regmap nna_0_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_NNA_CLK_CTRL0,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "nna_0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&nna_0_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap nna_0 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_NNA_CLK_CTRL0,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "nna_0",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&nna_0_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_SET_RATE_GATE,
	},
};

static struct clk_regmap nna_1_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_NNA_CLK_CTRL0,
		.mask = 0x7,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "nna_1_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = nna_01_parent_data,
		.num_parents = ARRAY_SIZE(nna_01_parent_data),
	},
};

static struct clk_regmap nna_1_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_NNA_CLK_CTRL0,
		.shift = 16,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "nna_1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&nna_1_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap nna_1 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_NNA_CLK_CTRL0,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "nna_1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&nna_1_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_SET_RATE_GATE,
	},
};

static const struct clk_parent_data nna_parent_data[] = {
	{ .hw = &nna_0.hw },
	{ .hw = &nna_1.hw }
};

static struct clk_regmap nna = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_NNA_CLK_CTRL0,
		.mask = 0x1,
		.shift = 31,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "nna",
		.ops = &clk_regmap_mux_ops,
		.parent_data = nna_parent_data,
		.num_parents = ARRAY_SIZE(nna_parent_data),
		.flags = CLK_SET_RATE_PARENT |
			 CLK_OPS_PARENT_ENABLE,
	},
};

static u32 pwm_parent_table[] = { 0, 2, 3, 4, 5, 6, 7 };
static const struct clk_parent_data pwm_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &sys2_pll.hw },
	{ .hw = &nna_pll.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &hifi_pll.hw }
};

static struct clk_regmap pwm_a_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_AB_CTRL,
		.mask = 0x7,
		.shift = 9,
		.table = pwm_parent_table,
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
		.flags = CLK_DIVIDER_ALLOW_ZERO,
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
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap pwm_b_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_AB_CTRL,
		.mask = 0x7,
		.shift = 25,
		.table = pwm_parent_table,
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
		.flags = CLK_DIVIDER_ALLOW_ZERO,
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
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap pwm_c_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_CD_CTRL,
		.mask = 0x7,
		.shift = 9,
		.table = pwm_parent_table,
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
		.flags = CLK_DIVIDER_ALLOW_ZERO,
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
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap pwm_d_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_CD_CTRL,
		.mask = 0x7,
		.shift = 25,
		.table = pwm_parent_table,
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
		.flags = CLK_DIVIDER_ALLOW_ZERO,
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
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap pwm_e_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_EF_CTRL,
		.mask = 0x7,
		.shift = 9,
		.table = pwm_parent_table,
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
		.flags = CLK_DIVIDER_ALLOW_ZERO,
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
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap pwm_f_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_EF_CTRL,
		.mask = 0x7,
		.shift = 25,
		.table = pwm_parent_table,
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
		.flags = CLK_DIVIDER_ALLOW_ZERO,
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
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap pwm_g_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_GH_CTRL,
		.mask = 0x7,
		.shift = 9,
		.table = pwm_parent_table,
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
		.flags = CLK_DIVIDER_ALLOW_ZERO,
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
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap pwm_h_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_GH_CTRL,
		.mask = 0x7,
		.shift = 25,
		.table = pwm_parent_table,
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
		.flags = CLK_DIVIDER_ALLOW_ZERO,
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
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap pwm_i_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_IJ_CTRL,
		.mask = 0x7,
		.shift = 9,
		.table = pwm_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_i_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = pwm_parent_data,
		.num_parents = ARRAY_SIZE(pwm_parent_data),
	},
};

static struct clk_regmap pwm_i_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_IJ_CTRL,
		.shift = 0,
		.width = 8,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_i_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_i_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_i = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_IJ_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_i",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_i_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap pwm_j_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_IJ_CTRL,
		.mask = 0x7,
		.shift = 25,
		.table = pwm_parent_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_j_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = pwm_parent_data,
		.num_parents = ARRAY_SIZE(pwm_parent_data),
	},
};

static struct clk_regmap pwm_j_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_IJ_CTRL,
		.shift = 16,
		.width = 8,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_j_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_j_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pwm_j = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_IJ_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pwm_j",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pwm_j_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT |
			 CLK_IGNORE_UNUSED,
	},
};

static const struct clk_parent_data spicc_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .fw_name = "sys_clk" },
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div2.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &fclk_div7.hw },
	{ .hw = &nna_pll.hw }
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
		.flags = CLK_DIVIDER_ALLOW_ZERO,
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
		.flags = CLK_DIVIDER_ALLOW_ZERO,
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

static struct clk_regmap spicc2_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_SPICC_CLK_CTRL1,
		.mask = 0x7,
		.shift = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "spicc2_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = spicc_parent_data,
		.num_parents = ARRAY_SIZE(spicc_parent_data),
	},
};

static struct clk_regmap spicc2_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_SPICC_CLK_CTRL1,
		.shift = 0,
		.width = 6,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "spicc2_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&spicc2_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap spicc2 = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_SPICC_CLK_CTRL1,
		.bit_idx = 6,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "spicc2",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&spicc2_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data emmc_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &fclk_div2.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &fclk_div2p5.hw },
	{ .hw = &nna_pll.hw },
	{ .hw = &hifi1_pll.hw },
	{ .hw = &gp0_pll.hw }
};

static struct clk_regmap sd_emmc_a_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_SD_EMMC_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sd_emmc_a_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = emmc_parent_data,
		.num_parents = ARRAY_SIZE(emmc_parent_data),
	},
};

static struct clk_regmap sd_emmc_a_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_SD_EMMC_CLK_CTRL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
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

static struct clk_regmap sd_emmc_b_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_SD_EMMC_CLK_CTRL,
		.mask = 0x7,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sd_emmc_b_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = emmc_parent_data,
		.num_parents = ARRAY_SIZE(emmc_parent_data),
	},
};

static struct clk_regmap sd_emmc_b_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_SD_EMMC_CLK_CTRL,
		.shift = 16,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sd_emmc_b_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&sd_emmc_b_mux.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_regmap sd_emmc_b = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_SD_EMMC_CLK_CTRL,
		.bit_idx = 23,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sd_emmc_b",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&sd_emmc_b_div.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_regmap sd_emmc_c_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_NAND_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sd_emmc_c_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = emmc_parent_data,
		.num_parents = ARRAY_SIZE(emmc_parent_data),
	},
};

static struct clk_regmap sd_emmc_c_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_NAND_CLK_CTRL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
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
	{ .hw = &fclk_div4.hw },
	{ .hw = &nna_pll.hw },
	{ .hw = &hifi_pll.hw }
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
		.flags = CLK_DIVIDER_ALLOW_ZERO,
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

static struct clk_fixed_factor eth_clk125M_div = {
	.mult = 1,
	.div = 8,
	.hw.init = &(struct clk_init_data) {
		.name = "eth_clk125M_div",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&fclk_div2.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_regmap eth_clk125M = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_ETH_CLK_CTRL,
		.bit_idx = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "eth_clk125M",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&eth_clk125M_div.hw,
		},
		.num_parents = 1,
	},
};

static struct clk_regmap ts_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_TS_CLK_CTRL,
		.shift = 0,
		.width = 8,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "ts_div",
		.ops = &clk_regmap_divider_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal"
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

static const struct clk_parent_data cdac_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &fclk_div5.hw }
};

static struct clk_regmap cdac_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_CDAC_CLK_CTRL,
		.mask = 0x3,
		.shift = 16,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cdac_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = cdac_parent_data,
		.num_parents = ARRAY_SIZE(cdac_parent_data),
	},
};

static struct clk_regmap cdac_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_CDAC_CLK_CTRL,
		.shift = 0,
		.width = 16,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cdac_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&cdac_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap cdac = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_CDAC_CLK_CTRL,
		.bit_idx = 20,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cdac",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&cdac_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data sar_adc_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .fw_name = "sys_clk" },
	{ .hw = &fclk_div5.hw }
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
		.flags = CLK_DIVIDER_ALLOW_ZERO,
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

static const struct clk_parent_data usb_250m_parent_data[] = {
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &fclk_div2.hw },
	{ .hw = &nna_pll.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &sys2_pll.hw },
	{ .hw = &hifi1_pll.hw }
};

static struct clk_regmap usb_250m_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_USB_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "usb_250m_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = usb_250m_parent_data,
		.num_parents = ARRAY_SIZE(usb_250m_parent_data),
	},
};

static struct clk_regmap usb_250m_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_USB_CLK_CTRL,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "usb_250m_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&usb_250m_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap usb_250m = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_USB_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "usb_250m",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&usb_250m_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data pcie_parent_data[] = {
	{ .hw = &fclk_div4.hw },
	{ .hw = &fclk_div3.hw },
	{ .hw = &fclk_div5.hw },
	{ .hw = &fclk_div2.hw },
	{ .hw = &nna_pll.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &sys2_pll.hw },
	{ .hw = &hifi1_pll.hw }
};

static struct clk_regmap pcie_400m_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_USB_CLK_CTRL,
		.mask = 0x7,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_400m_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = pcie_parent_data,
		.num_parents = ARRAY_SIZE(pcie_parent_data),
	},
};

static struct clk_regmap pcie_400m_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_USB_CLK_CTRL,
		.shift = 16,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_400m_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie_400m_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pcie_400m = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_USB_CLK_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_400m",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie_400m_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pcie_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_USB_CLK_CTRL1,
		.mask = 0x7,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = pcie_parent_data,
		.num_parents = ARRAY_SIZE(pcie_parent_data),
	},
};

static struct clk_regmap pcie_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_USB_CLK_CTRL1,
		.shift = 0,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pcie = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_USB_CLK_CTRL1,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie_div.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pcie_tl_mux = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_USB_CLK_CTRL1,
		.mask = 0x7,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_tl_mux",
		.ops = &clk_regmap_mux_ops,
		.parent_data = pcie_parent_data,
		.num_parents = ARRAY_SIZE(pcie_parent_data),
	},
};

static struct clk_regmap pcie_tl_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_USB_CLK_CTRL1,
		.shift = 16,
		.width = 7,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_tl_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie_tl_mux.hw,
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap pcie_tl = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_USB_CLK_CTRL1,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "pcie_tl",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&pcie_tl_div.hw,
		},
		.num_parents = 1,
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
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor clk_24m_div = {
	.mult = 1,
	.div = 2,
	.hw.init = &(struct clk_init_data) {
		.name = "clk_24m_div",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&clk_24m_in.hw,
		},
		.num_parents = 1,
	},
};

static const struct clk_parent_data clk_12_24m_parent_data[] = {
	{ .hw = &clk_24m_in.hw },
	{ .hw = &clk_24m_div.hw }
};

static struct clk_regmap clk_12_24m = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_CLK12_24_CTRL,
		.mask = 0x1,
		.shift = 10,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "clk_12_24m",
		.ops = &clk_regmap_mux_ops,
		.parent_data = clk_12_24m_parent_data,
		.num_parents = ARRAY_SIZE(clk_12_24m_parent_data),
	},
};

static struct clk_regmap clk_25m_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_CLK12_24_CTRL,
		.shift = 0,
		.width = 8,
		.flags = CLK_DIVIDER_ALLOW_ZERO,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "clk_25m_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&fclk_div2.hw,
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

static u32 gen_parent_table[] = { 0, 1, 2, 4, 5, 7, 8, 9, 10, 11, 13,
				 14, 17, 18, 19, 20, 21, 22, 23, 24 };
static const struct clk_parent_data gen_parent_data[] = {
	{ .fw_name = "xtal" },
	{ .hw = &rtc_clk.hw },
	{ .fw_name = "sys_pll_div16" },
	{ .hw = &sys2_pll.hw },
	{ .hw = &gp0_pll.hw },
	{ .hw = &hifi_pll.hw },
	{ .hw = &pcie_clk_in.hw },
	{ .hw = &pcie1_clk_in.hw },
	{ .hw = &gp2_pll.hw },
	{ .hw = &fpll_tmds.hw },
	{ .hw = &nna_pll.hw },
	{ .hw = &hifi1_pll.hw },
	{ .fw_name = "cpu_clk_div16" },
	{ .fw_name = "a76_clk_div16" },
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
		.flags = CLK_DIVIDER_ALLOW_ZERO,
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

MESON_CLK_GATE_SYS_CLK(sys_ddr, CLKCTRL_SYS_CLK_EN0_REG0, 0);
MESON_CLK_GATE_SYS_CLK(sys_ethphy, CLKCTRL_SYS_CLK_EN0_REG0, 4);
MESON_CLK_GATE_SYS_CLK(sys_gpu, CLKCTRL_SYS_CLK_EN0_REG0, 6);
MESON_CLK_GATE_SYS_CLK(sys_aocpu, CLKCTRL_SYS_CLK_EN0_REG0, 13);
MESON_CLK_GATE_SYS_CLK(sys_aucpu, CLKCTRL_SYS_CLK_EN0_REG0, 14);
MESON_CLK_GATE_SYS_CLK(sys_dewarpc, CLKCTRL_SYS_CLK_EN0_REG0, 16);
MESON_CLK_GATE_SYS_CLK(sys_dewarpb, CLKCTRL_SYS_CLK_EN0_REG0, 17);
MESON_CLK_GATE_SYS_CLK(sys_dewarpa, CLKCTRL_SYS_CLK_EN0_REG0, 18);
MESON_CLK_GATE_SYS_CLK(sys_ampipe_nand, CLKCTRL_SYS_CLK_EN0_REG0, 19);
MESON_CLK_GATE_SYS_CLK(sys_Ampipe_eth, CLKCTRL_SYS_CLK_EN0_REG0, 20);
MESON_CLK_GATE_SYS_CLK(sys_Am2axi0, CLKCTRL_SYS_CLK_EN0_REG0, 21);
MESON_CLK_GATE_SYS_CLK(sys_sd_emmc_a, CLKCTRL_SYS_CLK_EN0_REG0, 24);
MESON_CLK_GATE_SYS_CLK(sys_sd_emmc_b, CLKCTRL_SYS_CLK_EN0_REG0, 25);
MESON_CLK_GATE_SYS_CLK(sys_sd_emmc_c, CLKCTRL_SYS_CLK_EN0_REG0, 26);
MESON_CLK_GATE_SYS_CLK(sys_Vc9000e, CLKCTRL_SYS_CLK_EN0_REG0, 27);
MESON_CLK_GATE_SYS_CLK(sys_acodec, CLKCTRL_SYS_CLK_EN0_REG0, 28);
MESON_CLK_GATE_SYS_CLK(sys_spifc, CLKCTRL_SYS_CLK_EN0_REG0, 29);
MESON_CLK_GATE_SYS_CLK(sys_msr_clk, CLKCTRL_SYS_CLK_EN0_REG0, 30);
MESON_CLK_GATE_SYS_CLK(sys_ir_ctrl, CLKCTRL_SYS_CLK_EN0_REG0, 31);
MESON_CLK_GATE_SYS_CLK(sys_audio, CLKCTRL_SYS_CLK_EN0_REG1, 0);
MESON_CLK_GATE_SYS_CLK(sys_dos, CLKCTRL_SYS_CLK_EN0_REG1, 2);
MESON_CLK_GATE_SYS_CLK(sys_eth, CLKCTRL_SYS_CLK_EN0_REG1, 3);
MESON_CLK_GATE_SYS_CLK(sys_uart_a, CLKCTRL_SYS_CLK_EN0_REG1, 5);
MESON_CLK_GATE_SYS_CLK(sys_uart_b, CLKCTRL_SYS_CLK_EN0_REG1, 6);
MESON_CLK_GATE_SYS_CLK(sys_uart_c, CLKCTRL_SYS_CLK_EN0_REG1, 7);
MESON_CLK_GATE_SYS_CLK(sys_uart_d, CLKCTRL_SYS_CLK_EN0_REG1, 8);
MESON_CLK_GATE_SYS_CLK(sys_uart_e, CLKCTRL_SYS_CLK_EN0_REG1, 9);
MESON_CLK_GATE_SYS_CLK(sys_uart_f, CLKCTRL_SYS_CLK_EN0_REG1, 10);
MESON_CLK_GATE_SYS_CLK(sys_spicc2, CLKCTRL_SYS_CLK_EN0_REG1, 12);
MESON_CLK_GATE_SYS_CLK(sys_ts_a55, CLKCTRL_SYS_CLK_EN0_REG1, 16);
MESON_CLK_GATE_SYS_CLK(sys_ge2d, CLKCTRL_SYS_CLK_EN0_REG1, 20);
MESON_CLK_GATE_SYS_CLK(sys_spicc0, CLKCTRL_SYS_CLK_EN0_REG1, 21);
MESON_CLK_GATE_SYS_CLK(sys_spicc1, CLKCTRL_SYS_CLK_EN0_REG1, 22);
MESON_CLK_GATE_SYS_CLK(sys_Smart_card, CLKCTRL_SYS_CLK_EN0_REG1, 23);
MESON_CLK_GATE_SYS_CLK(sys_pcie, CLKCTRL_SYS_CLK_EN0_REG1, 24);
MESON_CLK_GATE_SYS_CLK(sys_pciephy, CLKCTRL_SYS_CLK_EN0_REG1, 25);
MESON_CLK_GATE_SYS_CLK(sys_usbgeneral, CLKCTRL_SYS_CLK_EN0_REG1, 26);
MESON_CLK_GATE_SYS_CLK(sys_Pciephy0, CLKCTRL_SYS_CLK_EN0_REG1, 27);
MESON_CLK_GATE_SYS_CLK(sys_Pciephy1, CLKCTRL_SYS_CLK_EN0_REG1, 28);
MESON_CLK_GATE_SYS_CLK(sys_Pciephy2, CLKCTRL_SYS_CLK_EN0_REG1, 29);
MESON_CLK_GATE_SYS_CLK(sys_i2c_m_a, CLKCTRL_SYS_CLK_EN0_REG1, 30);
MESON_CLK_GATE_SYS_CLK(sys_i2c_m_b, CLKCTRL_SYS_CLK_EN0_REG1, 31);
MESON_CLK_GATE_SYS_CLK(sys_i2c_m_c, CLKCTRL_SYS_CLK_EN0_REG2, 0);
MESON_CLK_GATE_SYS_CLK(sys_i2c_m_d, CLKCTRL_SYS_CLK_EN0_REG2, 1);
MESON_CLK_GATE_SYS_CLK(sys_i2c_m_e, CLKCTRL_SYS_CLK_EN0_REG2, 2);
MESON_CLK_GATE_SYS_CLK(sys_i2c_m_f, CLKCTRL_SYS_CLK_EN0_REG2, 3);
MESON_CLK_GATE_SYS_CLK(sys_i2c_s_a, CLKCTRL_SYS_CLK_EN0_REG2, 8);
MESON_CLK_GATE_SYS_CLK(sys_cmpr, CLKCTRL_SYS_CLK_EN0_REG2, 10);
MESON_CLK_GATE_SYS_CLK(sys_mmc_pclk, CLKCTRL_SYS_CLK_EN0_REG2, 11);
MESON_CLK_GATE_SYS_CLK(sys_Hdmitx_pclk, CLKCTRL_SYS_CLK_EN0_REG2, 16);
MESON_CLK_GATE_SYS_CLK(sys_Hdmi20_aes_clk, CLKCTRL_SYS_CLK_EN0_REG2, 17);
MESON_CLK_GATE_SYS_CLK(sys_Pclk_Sys_cpu_apb, CLKCTRL_SYS_CLK_EN0_REG2, 19);
MESON_CLK_GATE_SYS_CLK(sys_cec, CLKCTRL_SYS_CLK_EN0_REG2, 23);
MESON_CLK_GATE_SYS_CLK(sys_vpu_intr, CLKCTRL_SYS_CLK_EN0_REG2, 25);
MESON_CLK_GATE_SYS_CLK(sys_sar_adc, CLKCTRL_SYS_CLK_EN0_REG2, 28);
MESON_CLK_GATE_SYS_CLK(sys_gic, CLKCTRL_SYS_CLK_EN0_REG2, 30);
MESON_CLK_GATE_SYS_CLK(sys_Ts_gpu, CLKCTRL_SYS_CLK_EN0_REG2, 31);
MESON_CLK_GATE_SYS_CLK(sys_Ts_nna, CLKCTRL_SYS_CLK_EN0_REG3, 0);
MESON_CLK_GATE_SYS_CLK(sys_Ts_vpu, CLKCTRL_SYS_CLK_EN0_REG3, 1);
MESON_CLK_GATE_SYS_CLK(sys_Ts_dos, CLKCTRL_SYS_CLK_EN0_REG3, 2);
MESON_CLK_GATE_SYS_CLK(sys_pwm_ab, CLKCTRL_SYS_CLK_EN0_REG3, 5);
MESON_CLK_GATE_SYS_CLK(sys_pwm_cd, CLKCTRL_SYS_CLK_EN0_REG3, 6);
MESON_CLK_GATE_SYS_CLK(sys_pwm_ef, CLKCTRL_SYS_CLK_EN0_REG3, 7);
MESON_CLK_GATE_SYS_CLK(sys_pwm_gh, CLKCTRL_SYS_CLK_EN0_REG3, 8);
MESON_CLK_GATE_SYS_CLK(sys_pwm_ij, CLKCTRL_SYS_CLK_EN0_REG3, 9);

static struct clk_regmap *const pll_regmaps[] = {
	&pcie_pll_dco,
	&pcie_pll_od,
	&pcie_bgp,
	&pcie_hcsl,
	&pcie_hcsl_in_pad,
	&pcie_clk_in,
	&pcie1_pll_dco,
	&pcie1_pll_od,
	&pcie1_bgp,
	&pcie1_hcsl,
	&pcie1_hcsl_in_pad,
	&pcie1_clk_in,
};

static struct clk_regmap *const clk_regmaps[] = {
	&sys2_pll,
	&gp0_pll,
	&gp2_pll,
	&hifi_pll,
	&hifi1_pll,
	&fpll_dco,
	&nna_pll,
	&fclk_50m_en,
	&fclk_div2,
	&fclk_div2p5,
	&fclk_div3,
	&fclk_div4,
	&fclk_div5,
	&fclk_div7,
	&fpll_tmds_od,
	&fpll_tmds_od1,
	&fpll_pixel,
	&nna_pll_audio,
	&rtc_dual_clkin,
	&rtc_dual_div,
	&rtc_dual_mux,
	&rtc_dual,
	&rtc_clk,
	&ceca_dual_clkin,
	&ceca_dual_div,
	&ceca_dual_mux,
	&ceca_dual,
	&ceca_clk,
	&cecb_dual_clkin,
	&cecb_dual_div,
	&cecb_dual_mux,
	&cecb_dual,
	&cecb_clk,
	&vapb_mux,
	&vapb_div,
	&vapb,
	&ge2d_mux,
	&ge2d_div,
	&ge2d,
	&sc_mux,
	&sc_div,
	&sc,
	&video_src0_in_mux,
	&video_src0_in,
	&video_src0_div,
	&video_src0,
	&video_src1_in_mux,
	&video_src1_in,
	&video_src1_div,
	&video_src1,
	&video0_div1,
	&video0_div2_gate,
	&video0_div4_gate,
	&video0_div6_gate,
	&video0_div12_gate,
	&video2_div1,
	&video2_div2_gate,
	&video2_div4_gate,
	&video2_div6_gate,
	&video2_div12_gate,
	&enci_mux,
	&enci,
	&enct_mux,
	&enct,
	&encp_mux,
	&encp,
	&encl_mux,
	&encl,
	&vdac_mux,
	&vdac,
	&enc0_hdmitx_pixel_mux,
	&enc0_hdmitx_pixel,
	&enc0_hdmitx_fe_mux,
	&enc0_hdmitx_fe,
	&enc0_hdmitx_pnx_mux,
	&enc0_hdmitx_pnx,
	&hdmitx_pixel_mux,
	&hdmitx_pixel_div,
	&hdmitx_pixel,
	&hdmitx_fe_mux,
	&hdmitx_fe_div,
	&hdmitx_fe,
	&hdmitx_pnx_mux,
	&hdmitx_pnx_div,
	&hdmitx_pnx,
	&vpu_0_mux,
	&vpu_0_div,
	&vpu_0,
	&vpu_1_mux,
	&vpu_1_div,
	&vpu_1,
	&vpu,
	&vpu_clkb_tmp_mux,
	&vpu_clkb_tmp_div,
	&vpu_clkb_tmp,
	&vpu_clkb_div,
	&vpu_clkb,
	&vdin_meas_mux,
	&vdin_meas_div,
	&vdin_meas,
	&vid_lock_mux,
	&vid_lock_div,
	&vid_lock,
	&cmpr_mux,
	&cmpr_div,
	&cmpr,
	&mali_0_mux,
	&mali_0_div,
	&mali_0,
	&mali_1_mux,
	&mali_1_div,
	&mali_1,
	&mali,
	&vdec_0_mux,
	&vdec_0_div,
	&vdec_0,
	&vdec_1_mux,
	&vdec_1_div,
	&vdec_1,
	&vdec,
	&hcodec_0_mux,
	&hcodec_0_div,
	&hcodec_0,
	&hcodec_1_mux,
	&hcodec_1_div,
	&hcodec_1,
	&hcodec,
	&hevc_0_mux,
	&hevc_0_div,
	&hevc_0,
	&hevc_1_mux,
	&hevc_1_div,
	&hevc_1,
	&hevc,
	&vc9000e_axi_mux,
	&vc9000e_axi_div,
	&vc9000e_axi,
	&vc9000e_core_mux,
	&vc9000e_core_div,
	&vc9000e_core,
	&hdmitx_sys_mux,
	&hdmitx_sys_div,
	&hdmitx_sys,
	&hdmitx_prif_mux,
	&hdmitx_prif_div,
	&hdmitx_prif,
	&hdmitx_200m_mux,
	&hdmitx_200m_div,
	&hdmitx_200m,
	&hdmitx_aud_mux,
	&hdmitx_aud_div,
	&hdmitx_aud,
	&htx_tmds_mux,
	&htx_tmds_div,
	&htx_tmds,
	&nna_0_mux,
	&nna_0_div,
	&nna_0,
	&nna_1_mux,
	&nna_1_div,
	&nna_1,
	&nna,
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
	&pwm_i_mux,
	&pwm_i_div,
	&pwm_i,
	&pwm_j_mux,
	&pwm_j_div,
	&pwm_j,
	&spicc0_mux,
	&spicc0_div,
	&spicc0,
	&spicc1_mux,
	&spicc1_div,
	&spicc1,
	&spicc2_mux,
	&spicc2_div,
	&spicc2,
	&sd_emmc_a_mux,
	&sd_emmc_a_div,
	&sd_emmc_a,
	&sd_emmc_b_mux,
	&sd_emmc_b_div,
	&sd_emmc_b,
	&sd_emmc_c_mux,
	&sd_emmc_c_div,
	&sd_emmc_c,
	&eth_clk_rmii_mux,
	&eth_clk_rmii_div,
	&eth_clk_rmii,
	&eth_clk125M,
	&ts_div,
	&ts,
	&cdac_mux,
	&cdac_div,
	&cdac,
	&sar_adc_mux,
	&sar_adc_div,
	&sar_adc,
	&usb_250m_mux,
	&usb_250m_div,
	&usb_250m,
	&pcie_400m_mux,
	&pcie_400m_div,
	&pcie_400m,
	&pcie_mux,
	&pcie_div,
	&pcie,
	&pcie_tl_mux,
	&pcie_tl_div,
	&pcie_tl,
	&clk_24m_in,
	&clk_12_24m,
	&clk_25m_div,
	&clk_25m,
	&gen_mux,
	&gen_div,
	&gen,
	&sys_ddr,
	&sys_ethphy,
	&sys_gpu,
	&sys_aocpu,
	&sys_aucpu,
	&sys_dewarpc,
	&sys_dewarpb,
	&sys_dewarpa,
	&sys_ampipe_nand,
	&sys_Ampipe_eth,
	&sys_Am2axi0,
	&sys_sd_emmc_a,
	&sys_sd_emmc_b,
	&sys_sd_emmc_c,
	&sys_Vc9000e,
	&sys_acodec,
	&sys_spifc,
	&sys_msr_clk,
	&sys_ir_ctrl,
	&sys_audio,
	&sys_dos,
	&sys_eth,
	&sys_uart_a,
	&sys_uart_b,
	&sys_uart_c,
	&sys_uart_d,
	&sys_uart_e,
	&sys_uart_f,
	&sys_spicc2,
	&sys_ts_a55,
	&sys_ge2d,
	&sys_spicc0,
	&sys_spicc1,
	&sys_Smart_card,
	&sys_pcie,
	&sys_pciephy,
	&sys_usbgeneral,
	&sys_Pciephy0,
	&sys_Pciephy1,
	&sys_Pciephy2,
	&sys_i2c_m_a,
	&sys_i2c_m_b,
	&sys_i2c_m_c,
	&sys_i2c_m_d,
	&sys_i2c_m_e,
	&sys_i2c_m_f,
	&sys_i2c_s_a,
	&sys_cmpr,
	&sys_mmc_pclk,
	&sys_Hdmitx_pclk,
	&sys_Hdmi20_aes_clk,
	&sys_Pclk_Sys_cpu_apb,
	&sys_cec,
	&sys_vpu_intr,
	&sys_sar_adc,
	&sys_gic,
	&sys_Ts_gpu,
	&sys_Ts_nna,
	&sys_Ts_vpu,
	&sys_Ts_dos,
	&sys_pwm_ab,
	&sys_pwm_cd,
	&sys_pwm_ef,
	&sys_pwm_gh,
	&sys_pwm_ij
};

static struct clk_hw *s5_hw_clks[] = {
	[CLKID_SYS2_PLL]              = &sys2_pll.hw,
	[CLKID_GP0_PLL]               = &gp0_pll.hw,
	[CLKID_GP2_PLL]               = &gp2_pll.hw,
	[CLKID_HIFI_PLL]              = &hifi_pll.hw,
	[CLKID_HIFI1_PLL]             = &hifi1_pll.hw,
	[CLKID_FPLL_DCO]              = &fpll_dco.hw,
	[CLKID_NNA_PLL]               = &nna_pll.hw,
	[CLKID_PCIE_PLL_DCO]          = &pcie_pll_dco.hw,
	[CLKID_PCIE1_PLL_DCO]         = &pcie1_pll_dco.hw,
	[CLKID_FCLK_50M_EN]           = &fclk_50m_en.hw,
	[CLKID_FCLK_50M]              = &fclk_50m.hw,
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
	[CLKID_FPLL_TMDS_OD]          = &fpll_tmds_od.hw,
	[CLKID_FPLL_TMDS_OD1]         = &fpll_tmds_od1.hw,
	[CLKID_FPLL_TMDS]             = &fpll_tmds.hw,
	[CLKID_FPLL_PIXEL]            = &fpll_pixel.hw,
	[CLKID_NNA_PLL_AUDIO]         = &nna_pll_audio.hw,
	[CLKID_PCIE_PLL_DCO_DIV2]     = &pcie_pll_dco_div2.hw,
	[CLKID_PCIE_PLL_OD]           = &pcie_pll_od.hw,
	[CLKID_PCIE_PLL]              = &pcie_pll.hw,
	[CLKID_PCIE_BGP]              = &pcie_bgp.hw,
	[CLKID_PCIE_HCSL]             = &pcie_hcsl.hw,
	[CLKID_PCIE_HCSL_IN_PAD]      = &pcie_hcsl_in_pad.hw,
	[CLKID_PCIE_CLK_IN]           = &pcie_clk_in.hw,
	[CLKID_PCIE1_PLL_DCO_DIV2]    = &pcie1_pll_dco_div2.hw,
	[CLKID_PCIE1_PLL_OD]          = &pcie1_pll_od.hw,
	[CLKID_PCIE1_PLL]             = &pcie1_pll.hw,
	[CLKID_PCIE1_BGP]             = &pcie1_bgp.hw,
	[CLKID_PCIE1_HCSL]            = &pcie1_hcsl.hw,
	[CLKID_PCIE1_HCSL_IN_PAD]     = &pcie1_hcsl_in_pad.hw,
	[CLKID_PCIE1_CLK_IN]          = &pcie1_clk_in.hw,
	[CLKID_RTC_DUAL_CLKIN]        = &rtc_dual_clkin.hw,
	[CLKID_RTC_DUAL_DIV]          = &rtc_dual_div.hw,
	[CLKID_RTC_DUAL_MUX]          = &rtc_dual_mux.hw,
	[CLKID_RTC_DUAL]              = &rtc_dual.hw,
	[CLKID_RTC_CLK]               = &rtc_clk.hw,
	[CLKID_CECA_DUAL_CLKIN]       = &ceca_dual_clkin.hw,
	[CLKID_CECA_DUAL_DIV]         = &ceca_dual_div.hw,
	[CLKID_CECA_DUAL_MUX]         = &ceca_dual_mux.hw,
	[CLKID_CECA_DUAL]             = &ceca_dual.hw,
	[CLKID_CECA_CLK]              = &ceca_clk.hw,
	[CLKID_CECB_DUAL_CLKIN]       = &cecb_dual_clkin.hw,
	[CLKID_CECB_DUAL_DIV]         = &cecb_dual_div.hw,
	[CLKID_CECB_DUAL_MUX]         = &cecb_dual_mux.hw,
	[CLKID_CECB_DUAL]             = &cecb_dual.hw,
	[CLKID_CECB_CLK]              = &cecb_clk.hw,
	[CLKID_VAPB_MUX]              = &vapb_mux.hw,
	[CLKID_VAPB_DIV]              = &vapb_div.hw,
	[CLKID_VAPB]                  = &vapb.hw,
	[CLKID_GE2D_MUX]              = &ge2d_mux.hw,
	[CLKID_GE2D_DIV]              = &ge2d_div.hw,
	[CLKID_GE2D]                  = &ge2d.hw,
	[CLKID_SC_MUX]                = &sc_mux.hw,
	[CLKID_SC_DIV]                = &sc_div.hw,
	[CLKID_SC]                    = &sc.hw,
	[CLKID_VIDEO_SRC0_IN_MUX]     = &video_src0_in_mux.hw,
	[CLKID_VIDEO_SRC0_IN]         = &video_src0_in.hw,
	[CLKID_VIDEO_SRC0_DIV]        = &video_src0_div.hw,
	[CLKID_VIDEO_SRC0]            = &video_src0.hw,
	[CLKID_VIDEO_SRC1_IN_MUX]     = &video_src1_in_mux.hw,
	[CLKID_VIDEO_SRC1_IN]         = &video_src1_in.hw,
	[CLKID_VIDEO_SRC1_DIV]        = &video_src1_div.hw,
	[CLKID_VIDEO_SRC1]            = &video_src1.hw,
	[CLKID_VIDEO0_DIV1]           = &video0_div1.hw,
	[CLKID_VIDEO0_DIV2_GATE]      = &video0_div2_gate.hw,
	[CLKID_VIDEO0_DIV2]           = &video0_div2.hw,
	[CLKID_VIDEO0_DIV4_GATE]      = &video0_div4_gate.hw,
	[CLKID_VIDEO0_DIV4]           = &video0_div4.hw,
	[CLKID_VIDEO0_DIV6_GATE]      = &video0_div6_gate.hw,
	[CLKID_VIDEO0_DIV6]           = &video0_div6.hw,
	[CLKID_VIDEO0_DIV12_GATE]     = &video0_div12_gate.hw,
	[CLKID_VIDEO0_DIV12]          = &video0_div12.hw,
	[CLKID_VIDEO2_DIV1]           = &video2_div1.hw,
	[CLKID_VIDEO2_DIV2_GATE]      = &video2_div2_gate.hw,
	[CLKID_VIDEO2_DIV2]           = &video2_div2.hw,
	[CLKID_VIDEO2_DIV4_GATE]      = &video2_div4_gate.hw,
	[CLKID_VIDEO2_DIV4]           = &video2_div4.hw,
	[CLKID_VIDEO2_DIV6_GATE]      = &video2_div6_gate.hw,
	[CLKID_VIDEO2_DIV6]           = &video2_div6.hw,
	[CLKID_VIDEO2_DIV12_GATE]     = &video2_div12_gate.hw,
	[CLKID_VIDEO2_DIV12]          = &video2_div12.hw,
	[CLKID_ENCI_MUX]              = &enci_mux.hw,
	[CLKID_ENCI]                  = &enci.hw,
	[CLKID_ENCT_MUX]              = &enct_mux.hw,
	[CLKID_ENCT]                  = &enct.hw,
	[CLKID_ENCP_MUX]              = &encp_mux.hw,
	[CLKID_ENCP]                  = &encp.hw,
	[CLKID_ENCL_MUX]              = &encl_mux.hw,
	[CLKID_ENCL]                  = &encl.hw,
	[CLKID_VDAC_MUX]              = &vdac_mux.hw,
	[CLKID_VDAC]                  = &vdac.hw,
	[CLKID_ENC0_HDMITX_PIXEL_MUX] = &enc0_hdmitx_pixel_mux.hw,
	[CLKID_ENC0_HDMITX_PIXEL]     = &enc0_hdmitx_pixel.hw,
	[CLKID_ENC0_HDMITX_FE_MUX]    = &enc0_hdmitx_fe_mux.hw,
	[CLKID_ENC0_HDMITX_FE]        = &enc0_hdmitx_fe.hw,
	[CLKID_ENC0_HDMITX_PNX_MUX]   = &enc0_hdmitx_pnx_mux.hw,
	[CLKID_ENC0_HDMITX_PNX]       = &enc0_hdmitx_pnx.hw,
	[CLKID_HDMITX_PIXEL_MUX]      = &hdmitx_pixel_mux.hw,
	[CLKID_HDMITX_PIXEL_DIV]      = &hdmitx_pixel_div.hw,
	[CLKID_HDMITX_PIXEL]          = &hdmitx_pixel.hw,
	[CLKID_HDMITX_FE_MUX]         = &hdmitx_fe_mux.hw,
	[CLKID_HDMITX_FE_DIV]         = &hdmitx_fe_div.hw,
	[CLKID_HDMITX_FE]             = &hdmitx_fe.hw,
	[CLKID_HDMITX_PNX_MUX]        = &hdmitx_pnx_mux.hw,
	[CLKID_HDMITX_PNX_DIV]        = &hdmitx_pnx_div.hw,
	[CLKID_HDMITX_PNX]            = &hdmitx_pnx.hw,
	[CLKID_VPU_0_MUX]             = &vpu_0_mux.hw,
	[CLKID_VPU_0_DIV]             = &vpu_0_div.hw,
	[CLKID_VPU_0]                 = &vpu_0.hw,
	[CLKID_VPU_1_MUX]             = &vpu_1_mux.hw,
	[CLKID_VPU_1_DIV]             = &vpu_1_div.hw,
	[CLKID_VPU_1]                 = &vpu_1.hw,
	[CLKID_VPU]                   = &vpu.hw,
	[CLKID_VPU_CLKB_TMP_MUX]      = &vpu_clkb_tmp_mux.hw,
	[CLKID_VPU_CLKB_TMP_DIV]      = &vpu_clkb_tmp_div.hw,
	[CLKID_VPU_CLKB_TMP]          = &vpu_clkb_tmp.hw,
	[CLKID_VPU_CLKB_DIV]          = &vpu_clkb_div.hw,
	[CLKID_VPU_CLKB]              = &vpu_clkb.hw,
	[CLKID_VDIN_MEAS_MUX]         = &vdin_meas_mux.hw,
	[CLKID_VDIN_MEAS_DIV]         = &vdin_meas_div.hw,
	[CLKID_VDIN_MEAS]             = &vdin_meas.hw,
	[CLKID_VID_LOCK_MUX]          = &vid_lock_mux.hw,
	[CLKID_VID_LOCK_DIV]          = &vid_lock_div.hw,
	[CLKID_VID_LOCK]              = &vid_lock.hw,
	[CLKID_CMPR_MUX]              = &cmpr_mux.hw,
	[CLKID_CMPR_DIV]              = &cmpr_div.hw,
	[CLKID_CMPR]                  = &cmpr.hw,
	[CLKID_MALI_0_MUX]            = &mali_0_mux.hw,
	[CLKID_MALI_0_DIV]            = &mali_0_div.hw,
	[CLKID_MALI_0]                = &mali_0.hw,
	[CLKID_MALI_1_MUX]            = &mali_1_mux.hw,
	[CLKID_MALI_1_DIV]            = &mali_1_div.hw,
	[CLKID_MALI_1]                = &mali_1.hw,
	[CLKID_MALI]                  = &mali.hw,
	[CLKID_VDEC_0_MUX]            = &vdec_0_mux.hw,
	[CLKID_VDEC_0_DIV]            = &vdec_0_div.hw,
	[CLKID_VDEC_0]                = &vdec_0.hw,
	[CLKID_VDEC_1_MUX]            = &vdec_1_mux.hw,
	[CLKID_VDEC_1_DIV]            = &vdec_1_div.hw,
	[CLKID_VDEC_1]                = &vdec_1.hw,
	[CLKID_VDEC]                  = &vdec.hw,
	[CLKID_HCODEC_0_MUX]          = &hcodec_0_mux.hw,
	[CLKID_HCODEC_0_DIV]          = &hcodec_0_div.hw,
	[CLKID_HCODEC_0]              = &hcodec_0.hw,
	[CLKID_HCODEC_1_MUX]          = &hcodec_1_mux.hw,
	[CLKID_HCODEC_1_DIV]          = &hcodec_1_div.hw,
	[CLKID_HCODEC_1]              = &hcodec_1.hw,
	[CLKID_HCODEC]                = &hcodec.hw,
	[CLKID_HEVC_0_MUX]            = &hevc_0_mux.hw,
	[CLKID_HEVC_0_DIV]            = &hevc_0_div.hw,
	[CLKID_HEVC_0]                = &hevc_0.hw,
	[CLKID_HEVC_1_MUX]            = &hevc_1_mux.hw,
	[CLKID_HEVC_1_DIV]            = &hevc_1_div.hw,
	[CLKID_HEVC_1]                = &hevc_1.hw,
	[CLKID_HEVC]                  = &hevc.hw,
	[CLKID_VC9000E_AXI_MUX]       = &vc9000e_axi_mux.hw,
	[CLKID_VC9000E_AXI_DIV]       = &vc9000e_axi_div.hw,
	[CLKID_VC9000E_AXI]           = &vc9000e_axi.hw,
	[CLKID_VC9000E_CORE_MUX]      = &vc9000e_core_mux.hw,
	[CLKID_VC9000E_CORE_DIV]      = &vc9000e_core_div.hw,
	[CLKID_VC9000E_CORE]          = &vc9000e_core.hw,
	[CLKID_HDMITX_SYS_MUX]        = &hdmitx_sys_mux.hw,
	[CLKID_HDMITX_SYS_DIV]        = &hdmitx_sys_div.hw,
	[CLKID_HDMITX_SYS]            = &hdmitx_sys.hw,
	[CLKID_HDMITX_PRIF_MUX]       = &hdmitx_prif_mux.hw,
	[CLKID_HDMITX_PRIF_DIV]       = &hdmitx_prif_div.hw,
	[CLKID_HDMITX_PRIF]           = &hdmitx_prif.hw,
	[CLKID_HDMITX_200M_MUX]       = &hdmitx_200m_mux.hw,
	[CLKID_HDMITX_200M_DIV]       = &hdmitx_200m_div.hw,
	[CLKID_HDMITX_200M]           = &hdmitx_200m.hw,
	[CLKID_HDMITX_AUD_MUX]        = &hdmitx_aud_mux.hw,
	[CLKID_HDMITX_AUD_DIV]        = &hdmitx_aud_div.hw,
	[CLKID_HDMITX_AUD]            = &hdmitx_aud.hw,
	[CLKID_HTX_TMDS_MUX]          = &htx_tmds_mux.hw,
	[CLKID_HTX_TMDS_DIV]          = &htx_tmds_div.hw,
	[CLKID_HTX_TMDS]              = &htx_tmds.hw,
	[CLKID_NNA_0_MUX]             = &nna_0_mux.hw,
	[CLKID_NNA_0_DIV]             = &nna_0_div.hw,
	[CLKID_NNA_0]                 = &nna_0.hw,
	[CLKID_NNA_1_MUX]             = &nna_1_mux.hw,
	[CLKID_NNA_1_DIV]             = &nna_1_div.hw,
	[CLKID_NNA_1]                 = &nna_1.hw,
	[CLKID_NNA]                   = &nna.hw,
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
	[CLKID_PWM_I_MUX]             = &pwm_i_mux.hw,
	[CLKID_PWM_I_DIV]             = &pwm_i_div.hw,
	[CLKID_PWM_I]                 = &pwm_i.hw,
	[CLKID_PWM_J_MUX]             = &pwm_j_mux.hw,
	[CLKID_PWM_J_DIV]             = &pwm_j_div.hw,
	[CLKID_PWM_J]                 = &pwm_j.hw,
	[CLKID_SPICC0_MUX]            = &spicc0_mux.hw,
	[CLKID_SPICC0_DIV]            = &spicc0_div.hw,
	[CLKID_SPICC0]                = &spicc0.hw,
	[CLKID_SPICC1_MUX]            = &spicc1_mux.hw,
	[CLKID_SPICC1_DIV]            = &spicc1_div.hw,
	[CLKID_SPICC1]                = &spicc1.hw,
	[CLKID_SPICC2_MUX]            = &spicc2_mux.hw,
	[CLKID_SPICC2_DIV]            = &spicc2_div.hw,
	[CLKID_SPICC2]                = &spicc2.hw,
	[CLKID_SD_EMMC_A_MUX]         = &sd_emmc_a_mux.hw,
	[CLKID_SD_EMMC_A_DIV]         = &sd_emmc_a_div.hw,
	[CLKID_SD_EMMC_A]             = &sd_emmc_a.hw,
	[CLKID_SD_EMMC_B_MUX]         = &sd_emmc_b_mux.hw,
	[CLKID_SD_EMMC_B_DIV]         = &sd_emmc_b_div.hw,
	[CLKID_SD_EMMC_B]             = &sd_emmc_b.hw,
	[CLKID_SD_EMMC_C_MUX]         = &sd_emmc_c_mux.hw,
	[CLKID_SD_EMMC_C_DIV]         = &sd_emmc_c_div.hw,
	[CLKID_SD_EMMC_C]             = &sd_emmc_c.hw,
	[CLKID_ETH_CLK_RMII_MUX]      = &eth_clk_rmii_mux.hw,
	[CLKID_ETH_CLK_RMII_DIV]      = &eth_clk_rmii_div.hw,
	[CLKID_ETH_CLK_RMII]          = &eth_clk_rmii.hw,
	[CLKID_ETH_CLK125M_DIV]       = &eth_clk125M_div.hw,
	[CLKID_ETH_CLK125M]           = &eth_clk125M.hw,
	[CLKID_TS_DIV]                = &ts_div.hw,
	[CLKID_TS]                    = &ts.hw,
	[CLKID_CDAC_MUX]              = &cdac_mux.hw,
	[CLKID_CDAC_DIV]              = &cdac_div.hw,
	[CLKID_CDAC]                  = &cdac.hw,
	[CLKID_SAR_ADC_MUX]           = &sar_adc_mux.hw,
	[CLKID_SAR_ADC_DIV]           = &sar_adc_div.hw,
	[CLKID_SAR_ADC]               = &sar_adc.hw,
	[CLKID_USB_250M_MUX]          = &usb_250m_mux.hw,
	[CLKID_USB_250M_DIV]          = &usb_250m_div.hw,
	[CLKID_USB_250M]              = &usb_250m.hw,
	[CLKID_PCIE_400M_MUX]         = &pcie_400m_mux.hw,
	[CLKID_PCIE_400M_DIV]         = &pcie_400m_div.hw,
	[CLKID_PCIE_400M]             = &pcie_400m.hw,
	[CLKID_PCIE_MUX]              = &pcie_mux.hw,
	[CLKID_PCIE_DIV]              = &pcie_div.hw,
	[CLKID_PCIE]                  = &pcie.hw,
	[CLKID_PCIE_TL_MUX]           = &pcie_tl_mux.hw,
	[CLKID_PCIE_TL_DIV]           = &pcie_tl_div.hw,
	[CLKID_PCIE_TL]               = &pcie_tl.hw,
	[CLKID_CLK_24M_IN]            = &clk_24m_in.hw,
	[CLKID_CLK_24M_DIV]           = &clk_24m_div.hw,
	[CLKID_CLK_12_24M]            = &clk_12_24m.hw,
	[CLKID_CLK_25M_DIV]           = &clk_25m_div.hw,
	[CLKID_CLK_25M]               = &clk_25m.hw,
	[CLKID_GEN_MUX]               = &gen_mux.hw,
	[CLKID_GEN_DIV]               = &gen_div.hw,
	[CLKID_GEN]                   = &gen.hw,
	[CLKID_SYS_DDR]               = &sys_ddr.hw,
	[CLKID_SYS_ETHPHY]            = &sys_ethphy.hw,
	[CLKID_SYS_GPU]               = &sys_gpu.hw,
	[CLKID_SYS_AOCPU]             = &sys_aocpu.hw,
	[CLKID_SYS_AUCPU]             = &sys_aucpu.hw,
	[CLKID_SYS_DEWARPC]           = &sys_dewarpc.hw,
	[CLKID_SYS_DEWARPB]           = &sys_dewarpb.hw,
	[CLKID_SYS_DEWARPA]           = &sys_dewarpa.hw,
	[CLKID_SYS_AMPIPE_NAND]       = &sys_ampipe_nand.hw,
	[CLKID_SYS_AMPIPE_ETH]        = &sys_Ampipe_eth.hw,
	[CLKID_SYS_AM2AXI0]           = &sys_Am2axi0.hw,
	[CLKID_SYS_SD_EMMC_A]         = &sys_sd_emmc_a.hw,
	[CLKID_SYS_SD_EMMC_B]         = &sys_sd_emmc_b.hw,
	[CLKID_SYS_SD_EMMC_C]         = &sys_sd_emmc_c.hw,
	[CLKID_SYS_VC9000E]           = &sys_Vc9000e.hw,
	[CLKID_SYS_ACODEC]            = &sys_acodec.hw,
	[CLKID_SYS_SPIFC]             = &sys_spifc.hw,
	[CLKID_SYS_MSR_CLK]           = &sys_msr_clk.hw,
	[CLKID_SYS_IR_CTRL]           = &sys_ir_ctrl.hw,
	[CLKID_SYS_AUDIO]             = &sys_audio.hw,
	[CLKID_SYS_DOS]               = &sys_dos.hw,
	[CLKID_SYS_ETH]               = &sys_eth.hw,
	[CLKID_SYS_UART_A]            = &sys_uart_a.hw,
	[CLKID_SYS_UART_B]            = &sys_uart_b.hw,
	[CLKID_SYS_UART_C]            = &sys_uart_c.hw,
	[CLKID_SYS_UART_D]            = &sys_uart_d.hw,
	[CLKID_SYS_UART_E]            = &sys_uart_e.hw,
	[CLKID_SYS_UART_F]            = &sys_uart_f.hw,
	[CLKID_SYS_SPICC2]            = &sys_spicc2.hw,
	[CLKID_SYS_TS_A55]            = &sys_ts_a55.hw,
	[CLKID_SYS_GE2D]              = &sys_ge2d.hw,
	[CLKID_SYS_SPICC0]            = &sys_spicc0.hw,
	[CLKID_SYS_SPICC1]            = &sys_spicc1.hw,
	[CLKID_SYS_SMART_CARD]        = &sys_Smart_card.hw,
	[CLKID_SYS_PCIE]              = &sys_pcie.hw,
	[CLKID_SYS_PCIEPHY]           = &sys_pciephy.hw,
	[CLKID_SYS_USBGENERAL]        = &sys_usbgeneral.hw,
	[CLKID_SYS_PCIEPHY0]          = &sys_Pciephy0.hw,
	[CLKID_SYS_PCIEPHY1]          = &sys_Pciephy1.hw,
	[CLKID_SYS_PCIEPHY2]          = &sys_Pciephy2.hw,
	[CLKID_SYS_I2C_M_A]           = &sys_i2c_m_a.hw,
	[CLKID_SYS_I2C_M_B]           = &sys_i2c_m_b.hw,
	[CLKID_SYS_I2C_M_C]           = &sys_i2c_m_c.hw,
	[CLKID_SYS_I2C_M_D]           = &sys_i2c_m_d.hw,
	[CLKID_SYS_I2C_M_E]           = &sys_i2c_m_e.hw,
	[CLKID_SYS_I2C_M_F]           = &sys_i2c_m_f.hw,
	[CLKID_SYS_I2C_S_A]           = &sys_i2c_s_a.hw,
	[CLKID_SYS_CMPR]              = &sys_cmpr.hw,
	[CLKID_SYS_MMC_PCLK]          = &sys_mmc_pclk.hw,
	[CLKID_SYS_HDMITX_PCLK]       = &sys_Hdmitx_pclk.hw,
	[CLKID_SYS_HDMI20_AES_CLK]    = &sys_Hdmi20_aes_clk.hw,
	[CLKID_SYS_PCLK_SYS_CPU_APB]  = &sys_Pclk_Sys_cpu_apb.hw,
	[CLKID_SYS_CEC]               = &sys_cec.hw,
	[CLKID_SYS_VPU_INTR]          = &sys_vpu_intr.hw,
	[CLKID_SYS_SAR_ADC]           = &sys_sar_adc.hw,
	[CLKID_SYS_GIC]               = &sys_gic.hw,
	[CLKID_SYS_TS_GPU]            = &sys_Ts_gpu.hw,
	[CLKID_SYS_TS_NNA]            = &sys_Ts_nna.hw,
	[CLKID_SYS_TS_VPU]            = &sys_Ts_vpu.hw,
	[CLKID_SYS_TS_DOS]            = &sys_Ts_dos.hw,
	[CLKID_SYS_PWM_AB]            = &sys_pwm_ab.hw,
	[CLKID_SYS_PWM_CD]            = &sys_pwm_cd.hw,
	[CLKID_SYS_PWM_EF]            = &sys_pwm_ef.hw,
	[CLKID_SYS_PWM_GH]            = &sys_pwm_gh.hw,
	[CLKID_SYS_PWM_IJ]            = &sys_pwm_ij.hw
};

static const struct meson_clk_hw_data s5_clks = {
	.hws = s5_hw_clks,
	.num = ARRAY_SIZE(s5_hw_clks),
};

static int meson_s5_probe(struct platform_device *pdev)
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
		.compatible = "amlogic,s5-clkc",
		.data = &s5_clks
	},
	{}
};

static struct platform_driver s5_driver = {
	.probe			= meson_s5_probe,
	.driver			= {
		.name		= "s5-clkc",
		.of_match_table	= clkc_match_table,
	},
};

builtin_platform_driver(s5_driver);
MODULE_LICENSE("GPL v2");
