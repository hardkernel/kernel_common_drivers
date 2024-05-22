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
#include <linux/amlogic/media/vout/lcd/lcd_vout.h>
#include "../lcd_reg.h"
#include "lcd_phy_config.h"

static struct lcd_phy_ctrl_s *phy_ctrl_p;

static void lcd_phy_cntl_set_lane_t7(unsigned int flag, unsigned int base_val,
		unsigned int preem_val)
{
	unsigned int i;
	unsigned int t7_lane_cntl_reg[] = {
		ANACTRL_DIF_PHY_CNTL1,  ANACTRL_DIF_PHY_CNTL3,
		ANACTRL_DIF_PHY_CNTL4,  ANACTRL_DIF_PHY_CNTL5,
		ANACTRL_DIF_PHY_CNTL6,  ANACTRL_DIF_PHY_CNTL7,
		ANACTRL_DIF_PHY_CNTL8,  ANACTRL_DIF_PHY_CNTL9,
		ANACTRL_DIF_PHY_CNTL10, ANACTRL_DIF_PHY_CNTL12,
		ANACTRL_DIF_PHY_CNTL13, ANACTRL_DIF_PHY_CNTL14,
		ANACTRL_DIF_PHY_CNTL15, ANACTRL_DIF_PHY_CNTL16,
		ANACTRL_DIF_PHY_CNTL17, ANACTRL_DIF_PHY_CNTL18,
	};

	for (i = 0; i < ARRAY_SIZE(t7_lane_cntl_reg); i++) {
		if (flag & (1 << i))
			lcd_ana_write(t7_lane_cntl_reg[i], base_val | preem_val << 28);
	}
}

static void lcd_phy_cntl_set_lane_aux_t7(unsigned int flag, unsigned int aux_reg)
{
	if (flag & 0x01)
		lcd_ana_write(ANACTRL_DIF_PHY_CNTL2, aux_reg);
	if (flag & 0x0100)
		lcd_ana_write(ANACTRL_DIF_PHY_CNTL11, aux_reg);
}

static void lcd_lvds_phy_set(struct aml_lcd_drv_s *pdrv, int status)
{
	unsigned int flag;//, data_lane0_aux, data_lane1_aux, data_lane;
	struct lvds_config_s *lvds_conf;
	struct phy_config_s *phy = &pdrv->config.phy_cfg;

	if (!phy_ctrl_p)
		return;
	if (lcd_debug_print_flag & LCD_DBG_PR_ADV)
		LCDPR("%s: %d\n", __func__, status);

	lvds_conf = &pdrv->config.control.lvds_cfg;

	if (pdrv->index == 0) {
		flag = pdrv->config.basic.lcd_bits == 6 ? 0x0f : 0x1f;
	} else if (pdrv->index == 1) {
		flag = pdrv->config.basic.lcd_bits == 6 ? (0x0f << 10) : (0x1f << 10);
	} else if (pdrv->index == 2) {
		flag = pdrv->config.basic.lcd_bits == 6 ? 0x0f : 0x1f;

		if (lvds_conf->dual_port)
			flag = (flag << 5) | (flag << 10);
		else
			flag = flag << (lvds_conf->port_swap ? 10 : 5);
	} else {
		return;
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s: %d, flag=0x%04x\n", pdrv->index, __func__, status, flag);

	if (status) {
		if ((phy_ctrl_p->lane_lock & flag) &&
			((phy_ctrl_p->lane_lock & flag) != flag)) {
			LCDERR("phy lane already locked: 0x%x, invalid 0x%x\n",
				phy_ctrl_p->lane_lock, flag);
			return;
		}
		phy_ctrl_p->lane_lock |= flag;
		if (status == LCD_PHY_LOCK_LANE)
			goto phy_set_done;

		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			LCDPR("vswing_level=0x%x, preem_level=0x%x\n",
			      phy->vswing_level, phy->preem_level);
		}

		lcd_phy_cntl_set_lane_t7(flag & 0x0101, 0x06430028, phy->preem_level);
		lcd_phy_cntl_set_lane_t7(flag & 0xfefe, 0x06530028, phy->preem_level);
		lcd_phy_cntl_set_lane_aux_t7(flag, 0x0100ffff);

		lcd_ana_write(ANACTRL_DIF_PHY_CNTL19, 0x00406240 | phy->vswing_level);
	} else {
		phy_ctrl_p->lane_lock &= ~flag;
		lcd_phy_cntl_set_lane_t7(flag, 0, 0);
		lcd_phy_cntl_set_lane_aux_t7(flag, 0);
		if (phy_ctrl_p->lane_lock == 0)
			lcd_ana_write(ANACTRL_DIF_PHY_CNTL19, 0);
	}

phy_set_done:
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("phy lane_lock: 0x%x\n", phy_ctrl_p->lane_lock);
}

static void lcd_vbyone_phy_set(struct aml_lcd_drv_s *pdrv, int status)
{
	unsigned int flag;//, data_lane0_aux, data_lane1_aux, data_lane;
	struct phy_config_s *phy = &pdrv->config.phy_cfg;

	if (!phy_ctrl_p)
		return;
	if (lcd_debug_print_flag & LCD_DBG_PR_ADV)
		LCDPR("%s: %d\n", __func__, status);

	if (pdrv->index == 0) {
		flag = 0xff;
	} else if (pdrv->index == 1) {
		flag = (0xff << 8);
	} else {
		LCDERR("invalid drv_index %d for vbyone\n", pdrv->index);
		return;
	}

	if (status) {
		if ((phy_ctrl_p->lane_lock & flag) &&
			((phy_ctrl_p->lane_lock & flag) != flag)) {
			LCDERR("phy lane already locked: 0x%x, invalid 0x%x\n",
				phy_ctrl_p->lane_lock, flag);
			return;
		}
		phy_ctrl_p->lane_lock |= flag;
		if (status == LCD_PHY_LOCK_LANE)
			goto phy_set_done;

		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			LCDPR("vswing_level=0x%x, preem_level=0x%x\n",
			      phy->vswing_level, phy->preem_level);
		}

		lcd_phy_cntl_set_lane_t7(flag & 0x0101, 0x06430028, phy->preem_level);
		lcd_phy_cntl_set_lane_t7(flag & 0xfefe, 0x06530028, phy->preem_level);
		lcd_phy_cntl_set_lane_aux_t7(flag, 0x0100ffff);

		lcd_ana_write(ANACTRL_DIF_PHY_CNTL19, 0x00401640 | phy->vswing_level);
	} else {
		phy_ctrl_p->lane_lock &= ~flag;
		lcd_phy_cntl_set_lane_t7(flag, 0, 0);
		lcd_phy_cntl_set_lane_aux_t7(flag, 0);
		if (phy_ctrl_p->lane_lock == 0)
			lcd_ana_write(ANACTRL_DIF_PHY_CNTL19, 0);
	}

phy_set_done:
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("phy lane_lock: 0x%x\n", phy_ctrl_p->lane_lock);
}

static void lcd_mipi_phy_set(struct aml_lcd_drv_s *pdrv, int status)
{
	unsigned int flag;//, data_lane0_aux, data_lane1_aux, data_lane;

	if (!phy_ctrl_p)
		return;
	if (lcd_debug_print_flag & LCD_DBG_PR_ADV)
		LCDPR("%s: %d\n", __func__, status);

	if (pdrv->index == 0) {
		flag = 0x1f;
	} else if (pdrv->index == 1) {
		flag = (0x1f << 8);
	} else {
		LCDERR("invalid drv_index %d for mipi dsi\n", pdrv->index);
		return;
	}

	if (status) {
		if ((phy_ctrl_p->lane_lock & flag) &&
			((phy_ctrl_p->lane_lock & flag) != flag)) {
			LCDERR("phy lane already locked: 0x%x, invalid 0x%x\n",
				phy_ctrl_p->lane_lock, flag);
			return;
		}
		phy_ctrl_p->lane_lock |= flag;
		if (status == LCD_PHY_LOCK_LANE)
			goto phy_set_done;

		lcd_phy_cntl_set_lane_t7(flag & 0x1b1b, 0x022a0028, 0); //data lane
		lcd_phy_cntl_set_lane_t7(flag & 0x0404, 0x822a0028, 0); //clk lane
		lcd_phy_cntl_set_lane_aux_t7(flag, 0x0000ffcf);

		lcd_ana_write(ANACTRL_DIF_PHY_CNTL19, 0x1e406253);
		// if (pdrv->index)
		if (flag & 0xff00)
			lcd_ana_write(ANACTRL_DIF_PHY_CNTL21, 0xffff);
		else
			lcd_ana_write(ANACTRL_DIF_PHY_CNTL20, (0xffff << 16));
	} else {
		phy_ctrl_p->lane_lock &= ~flag;
		lcd_phy_cntl_set_lane_t7(flag & 0x1f1f, 0, 0);
		lcd_phy_cntl_set_lane_aux_t7(flag, 0);

		if (phy_ctrl_p->lane_lock == 0)
			lcd_ana_write(ANACTRL_DIF_PHY_CNTL19, 0);
		if (pdrv->index)
			lcd_ana_write(ANACTRL_DIF_PHY_CNTL21, 0);
		else
			lcd_ana_write(ANACTRL_DIF_PHY_CNTL20, 0);
	}

phy_set_done:
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("phy lane_lock: 0x%x\n", phy_ctrl_p->lane_lock);
}

static void lcd_edp_phy_set(struct aml_lcd_drv_s *pdrv, int status)
{
	unsigned int flag;//, data_lane0_aux, data_lane1_aux, data_lane;
	struct phy_config_s *phy = &pdrv->config.phy_cfg;

	if (!phy_ctrl_p)
		return;
	if (lcd_debug_print_flag & LCD_DBG_PR_ADV)
		LCDPR("%s: %d\n", __func__, status);

	if (pdrv->index == 0) {
		flag = 0x1f;
	} else if (pdrv->index == 1) {
		flag = (0x1f << 8);
	} else {
		LCDERR("invalid drv_index %d for edp\n", pdrv->index);
		return;
	}

	if (status) {
		if ((phy_ctrl_p->lane_lock & flag) &&
			((phy_ctrl_p->lane_lock & flag) != flag)) {
			LCDERR("phy lane already locked: 0x%x, invalid 0x%x\n",
				phy_ctrl_p->lane_lock, flag);
			return;
		}
		phy_ctrl_p->lane_lock |= flag;
		if (status == LCD_PHY_LOCK_LANE)
			goto phy_set_done;

		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			LCDPR("vswing_level=0x%x, preem_level=0x%x\n",
			      phy->vswing_level, phy->preem_level);
		}

		lcd_phy_cntl_set_lane_t7(flag,          0x06530028, phy->preem_level);
		lcd_phy_cntl_set_lane_t7(flag & 0x0101, 0x46770038, 0); //DP aux lane
		lcd_phy_cntl_set_lane_aux_t7(flag, 0x0000ffff);

		lcd_ana_write(ANACTRL_DIF_PHY_CNTL19, 0x00406240 | phy->vswing_level);
	} else {
		phy_ctrl_p->lane_lock &= ~flag;
		lcd_phy_cntl_set_lane_t7(flag, 0, 0);
		lcd_phy_cntl_set_lane_aux_t7(flag, 0);
		if (phy_ctrl_p->lane_lock == 0)
			lcd_ana_write(ANACTRL_DIF_PHY_CNTL19, 0);
	}

phy_set_done:
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("phy lane_lock: 0x%x\n", phy_ctrl_p->lane_lock);
}

static unsigned int lcd_phy_preem_level_to_value_t7(struct aml_lcd_drv_s *pdrv, unsigned int level)
{
	unsigned int preem_value = 0;

	preem_value = (level >= 0xf) ? 0xf : level;

	return preem_value;
}

static struct lcd_phy_ctrl_s lcd_phy_ctrl_t7 = {
	.lane_num = 16,
	.ctrl_bit_on = 1,
	.lane_lock = 0,
	.phy_vswing_level_to_val = lcd_phy_vswing_level_to_value_dft,
	.phy_preem_level_to_val = lcd_phy_preem_level_to_value_t7,
	.phy_amp_dft_val = NULL,
	.phy_glb_param_dft_val = NULL,
	.phy_set_lvds = lcd_lvds_phy_set,
	.phy_set_vx1 = lcd_vbyone_phy_set,
	.phy_set_mlvds = NULL,
	.phy_set_p2p = NULL,
	.phy_set_mipi = lcd_mipi_phy_set,
	.phy_set_edp = lcd_edp_phy_set,
};

struct lcd_phy_ctrl_s *lcd_phy_config_init_t7(struct aml_lcd_drv_s *pdrv)
{
	phy_ctrl_p = &lcd_phy_ctrl_t7;
	return phy_ctrl_p;
}
