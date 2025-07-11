/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_TX_PHY_CORE_H
#define __MESON_TX_PHY_CORE_H

#include <linux/platform_device.h>
#include <linux/phy/phy.h>

enum meson_tx_phy_mode {
	PHY_DP,
	PHY_HDMI,
};

struct meson_tx_phy_cfg_opts {
	u32 phy_clock;
	struct phy_configure_opts_dp dp_ops;
};

struct meson_tx_phy_ops {
	int (*init)(void *phy);
	int (*exit)(void *phy);
	int (*power_on)(void *phy);
	int (*power_off)(void *phy);
	int (*set_mode)(void *phy, enum meson_tx_phy_mode mode);
	int (*configure)(void *phy, struct meson_tx_phy_cfg_opts *opts);
	int (*validate)(void *phy, enum meson_tx_phy_mode mode,
				struct meson_tx_phy_cfg_opts *opts);
	int (*calibrate)(void *phy);
	int (*connect)(void *phy, int port);
	int (*disconnect)(void *phy, int port);
	int (*dump_reg)(void *phy);
};

struct meson_tx_phy {
	struct platform_device *pdev;
	struct meson_tx_phy_ops *ops;
	struct phy *phy;
};

struct meson_tx_phy_data {
	struct meson_tx_phy_ops *ops;
	void *para;
};

int meson_tx_phy_init(struct meson_tx_phy  *phy);
int meson_tx_phy_exit(struct meson_tx_phy  *phy);
int meson_tx_phy_power_on(struct meson_tx_phy  *phy);
int meson_tx_phy_power_off(struct meson_tx_phy  *phy);
int meson_tx_phy_set_mode(struct meson_tx_phy  *phy, enum meson_tx_phy_mode mode);
int meson_tx_phy_configure(struct meson_tx_phy  *phy, struct meson_tx_phy_cfg_opts *opts);
int meson_tx_phy_validate(struct meson_tx_phy  *phy, enum meson_tx_phy_mode mode,
			struct meson_tx_phy_cfg_opts *opts);
int meson_tx_phy_calibrate(struct meson_tx_phy  *phy);

#endif
