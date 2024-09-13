// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/init.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/amlogic/media/vout/lcd/lcd_vout.h>
#include "../lcd_reg.h"
#include "lcd_phy_config.h"

static unsigned int chreg_reg[5] = {
	ANACTRL_DIF_PHY_CNTL1, ANACTRL_DIF_PHY_CNTL2,
	ANACTRL_DIF_PHY_CNTL3, ANACTRL_DIF_PHY_CNTL4,
	ANACTRL_DIF_PHY_CNTL6,
};

static unsigned int chdig_reg[5] = {
	ANACTRL_DIF_PHY_CNTL8, ANACTRL_DIF_PHY_CNTL9,
	ANACTRL_DIF_PHY_CNTL10, ANACTRL_DIF_PHY_CNTL11,
	ANACTRL_DIF_PHY_CNTL12,
};

static int lcd_phy_reg_dump(struct aml_lcd_drv_s *pdrv, char *buf, int offset)
{
	int len = 0;
	struct reg_name_set_s reg_table[] = {
		{ANACTRL_DIF_PHY_CNTL1,  "PHY_CNTL1"},
		{ANACTRL_DIF_PHY_CNTL2,  "PHY_CNTL2"},
		{ANACTRL_DIF_PHY_CNTL3,  "PHY_CNTL3"},
		{ANACTRL_DIF_PHY_CNTL4,  "PHY_CNTL4"},
		{ANACTRL_DIF_PHY_CNTL6,  "PHY_CNTL6"},
		{ANACTRL_DIF_PHY_CNTL8,  "PHY_CNTL8"},
		{ANACTRL_DIF_PHY_CNTL9,  "PHY_CNTL9"},
		{ANACTRL_DIF_PHY_CNTL10, "PHY_CNTL10"},
		{ANACTRL_DIF_PHY_CNTL11, "PHY_CNTL11"},
		{ANACTRL_DIF_PHY_CNTL12, "PHY_CNTL12"},
		{ANACTRL_DIF_PHY_CNTL14, "PHY_CNTL14"},
		{ANACTRL_DIF_PHY_CNTL15, "PHY_CNTL15"}
	};

	len += str_add_reg_sets(pdrv, buf, offset, LCD_REG_DBG_ANA_BUS, 0,
				reg_table, ARRAY_SIZE(reg_table));
	return len;
}

static int lcd_phy_param_get_from_reg(struct aml_lcd_drv_s *pdrv, struct phy_config_s *phy)
{
	//todo
	return 0;
}

static void lcd_phy_cntl14_update(struct phy_config_s *phy, unsigned int cntl14)
{
	/* vswing */
	cntl14 &= ~(0xf << 26);
	cntl14 |= (phy->vswing & 0xf) << 26;

	/* vcm */
	if ((phy->flag & (1 << 1))) {
		cntl14 &= ~(0xf << 12);
		cntl14 |= (phy->vcm & 0xf) << 12;
	}
	lcd_ana_write(ANACTRL_DIF_PHY_CNTL14, cntl14);
}

/*
 *    chreg: channel ctrl
 *    bypass: 1=bypass
 *    mode: 1=normal mode, 0=low common mode
 *    ckdi: clk phase for minilvds
 */
static void lcd_phy_cntl_set(struct aml_lcd_drv_s *pdrv, struct phy_config_s *phy, int status,
			int bypass, unsigned int mode, unsigned int ckdi)
{
	unsigned int cntl15 = 0;
	unsigned int chreg, reg_data = 0, chdig = 0, dig_data = 0;
	unsigned char i, bit;
	unsigned char is_mlvds = pdrv->config.basic.lcd_type == LCD_MLVDS;
	unsigned char clk_phase_sel = 2;  //phase a

	if (lcd_debug_print_flag & LCD_DBG_PR_ADV)
		LCDPR("%s: %d, bypass:%d\n", __func__, status, bypass);

	if (status) {
		reg_data = 1 << 0;
		dig_data = 1 << 15;
		cntl15 = 0x17300000;
		/* odt */
		if ((phy->flag & (1 << 3))) {
			cntl15 &= ~(0x1f << 24);
			cntl15 |= (phy->odt & 0x1f) << 24;
		}
	} else {
		cntl15 = 0x17900000;
		reg_data = 0;
		dig_data = 0;
		lcd_ana_write(ANACTRL_DIF_PHY_CNTL14, 0x1300d100);
	}

	lcd_ana_write(ANACTRL_DIF_PHY_CNTL15, cntl15);

	for (i = 0; i < 10; i++) {
		if (phy->lane_valid & (1 << i)) {
			bit = i & 0x1 ? 16 : 0;
			chreg = reg_data;
			if (ckdi & (1 << i)) {
				chreg |= clk_phase_sel << 1;
				clk_phase_sel++;
			}
			chdig = dig_data | 0x400 |
				((is_mlvds ? 0xf : 0) << 2); //pn swap
			if (status) {
				chreg |= (phy->lane[i].preem & 0xf) << 12;
				chreg |= (phy->lane[i].amp & 0xf) << 8;
			}
			lcd_ana_setb(chreg_reg[i >> 1], chreg, bit, 16);
			lcd_ana_setb(chdig_reg[i >> 1], chdig, bit, 16);
		}
	}
}

static void lcd_lvds_phy_set(struct aml_lcd_drv_s *pdrv, int status)
{
	struct phy_config_s *phy = &pdrv->config.phy_cfg;
	unsigned int cntl14 = 0;

	if (status) {
		cntl14 =  0x1310d107;
		lcd_phy_cntl14_update(phy, cntl14);
		lcd_phy_cntl_set(pdrv, phy, status, 1, 1, 0);
		udelay(1);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL14, 1, 19, 1);
	} else {
		lcd_phy_cntl_set(pdrv, phy, status, 1, 0, 0);
	}
}

static void lcd_mlvds_phy_set(struct aml_lcd_drv_s *pdrv, int status)
{
	struct phy_config_s *phy = &pdrv->config.phy_cfg;
	unsigned int cntl14 = 0;

	if (status) {
		cntl14 =  0x1310d107;
		if (pdrv->config.basic.lcd_bits == 6)
			cntl14 |= (1 << 16);
		else if (pdrv->config.basic.lcd_bits == 8)
			cntl14 |= (2 << 16);
		else
			cntl14 |= (3 << 16);
		lcd_phy_cntl14_update(phy, cntl14);
		lcd_phy_cntl_set(pdrv, phy, status, 1, 1, phy->ckdi);
		udelay(1);
		lcd_ana_setb(ANACTRL_DIF_PHY_CNTL14, 1, 19, 1);
	} else {
		lcd_phy_cntl_set(pdrv, phy, status, 1, 0, 0);
	}
}

static unsigned int lcd_phy_preem_level_to_val_t6d(struct aml_lcd_drv_s *pdrv, unsigned int level)
{
	unsigned int preem_value = 0;

	if (pdrv->config.basic.lcd_type == LCD_LVDS || pdrv->config.basic.lcd_type == LCD_MLVDS)
		preem_value = (level >= 0xf) ? 0xf : level;

	return preem_value;
}

static unsigned int lcd_phy_amp_dft_t6d(struct aml_lcd_drv_s *pdrv)
{
	unsigned int amp_value = 0;

	if (pdrv->config.basic.lcd_type == LCD_LVDS || pdrv->config.basic.lcd_type == LCD_MLVDS)
		amp_value = 0x5;

	return amp_value;
}

static void phy_glb_param_dft_t6d(struct aml_lcd_drv_s *pdrv)
{
	//todo
}

static struct lcd_phy_ctrl_s lcd_phy_ctrl_t6d = {
	.lane_num = 10,

	.phy_vswing_level_to_val = lcd_phy_vswing_level_to_value_dft,
	.phy_preem_level_to_val = lcd_phy_preem_level_to_val_t6d,
	.phy_amp_dft_val = lcd_phy_amp_dft_t6d,
	.phy_glb_param_dft_val = phy_glb_param_dft_t6d,
	.phy_param_get = lcd_phy_param_get_from_reg,
	.phy_reg_dump = lcd_phy_reg_dump,

	.phy_set_lvds = lcd_lvds_phy_set,
	.phy_set_vx1 = NULL,
	.phy_set_mlvds = lcd_mlvds_phy_set,
	.phy_set_p2p = NULL,
	.phy_set_mipi = NULL,
	.phy_set_edp = NULL,
};

struct lcd_phy_ctrl_s *lcd_phy_config_init_t6d(struct aml_lcd_drv_s *pdrv)
{
	return &lcd_phy_ctrl_t6d;
}
