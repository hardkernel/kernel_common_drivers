/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _MESON_TX_CLK_REG_H
#define _MESON_TX_CLK_REG_H

/* for A9 */
#define ANACTRL_TCON_PLL0_CNTL0  (0x0020  << 2)
#define ANACTRL_TCON_PLL0_CNTL1  (0x0021  << 2)
#define ANACTRL_TCON_PLL0_CNTL2  (0x0022  << 2)
#define ANACTRL_TCON_PLL0_CNTL3  (0x0023  << 2)
#define ANACTRL_TCON_PLL0_CNTL4  (0x0024  << 2)
#define ANACTRL_TCON_PLL1_CNTL0  (0x0025  << 2)
#define ANACTRL_TCON_PLL1_CNTL1  (0x0026  << 2)
#define ANACTRL_TCON_PLL1_CNTL2  (0x0027  << 2)
#define ANACTRL_TCON_PLL1_CNTL3  (0x0028  << 2)
#define ANACTRL_TCON_PLL1_CNTL4  (0x0029  << 2)

#define ANACTRL_TCON_PLL2_CNTL0  (0x002a << 2)
#define ANACTRL_TCON_PLL2_CNTL1  (0x002b << 2)
#define ANACTRL_TCON_PLL2_CNTL2  (0x002c << 2)
#define ANACTRL_TCON_PLL2_CNTL3  (0x002d << 2)
#define ANACTRL_TCON_PLL2_CNTL4  (0x002e << 2)
#define ANACTRL_TCON_PLL3_CNTL0  (0x0030 << 2)
#define ANACTRL_TCON_PLL3_CNTL1  (0x0031 << 2)
#define ANACTRL_TCON_PLL3_CNTL2  (0x0032 << 2)
#define ANACTRL_TCON_PLL3_CNTL3  (0x0033 << 2)
#define ANACTRL_TCON_PLL3_CNTL4  (0x0034 << 2)
#define ANACTRL_TCON_PLL0_STS    (0x0035 << 2)
#define ANACTRL_TCON_PLL1_STS    (0x0036 << 2)
#define ANACTRL_TCON_PLL2_STS    (0x0037 << 2)
#define ANACTRL_TCON_PLL3_STS    (0x0038 << 2)

/* CLK_CTRL */
#define CLKCTRL_HDMI_CLK_CTRL        (0x0121 << 2)
#define CLKCTRL_VID_CLK_CTRL         (0x0130 << 2)
#define CLKCTRL_VID_CLK_CTRL2        (0x0131 << 2)
#define CLKCTRL_VID_CLK_DIV          (0x0132 << 2)
#define CLKCTRL_VIID_CLK_DIV         (0x0133 << 2)
#define CLKCTRL_VIID_CLK_CTRL        (0x0134 << 2)

#endif

