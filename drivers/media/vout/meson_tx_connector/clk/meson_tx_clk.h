/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

struct meson_dp_clk {
	struct platform_device *pdev;
	void __iomem *base;
};

void dp_pll_vco_enable(meson_dp_clk *dp_clk, void *para);
void dp_pll_vco_disable(meson_dp_clk *dp_clk);

void dp_vid_enc_clk_enable(meson_dp_clk *dp_clk, void *para);
void dp_vid_enc_clk_disable(meson_dp_clk *dp_clk, void *para);

void dp_phy_clk_enable(meson_dp_clk *dp_clk, void *para);
void dp_phy_clk_disable(meson_dp_clk *dp_clk, void *para);
