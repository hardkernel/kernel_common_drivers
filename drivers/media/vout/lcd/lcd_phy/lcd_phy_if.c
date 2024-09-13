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
#include "lcd_phy_config.h"
#include "../lcd_common.h"

static struct lcd_phy_ctrl_s *lcd_phy_ctrl;

inline unsigned int lcd_phy_vswing_level_to_value(struct aml_lcd_drv_s *pdrv, unsigned int level)
{
	if (!lcd_phy_ctrl)
		return 0;

	if (!lcd_phy_ctrl->phy_vswing_level_to_val)
		return level;

	return lcd_phy_ctrl->phy_vswing_level_to_val(pdrv, level);
}

unsigned int lcd_phy_preem_level_to_value(struct aml_lcd_drv_s *pdrv, unsigned int level)
{
	if (!lcd_phy_ctrl)
		return 0;

	if (!lcd_phy_ctrl->phy_preem_level_to_val)
		return level;

	return lcd_phy_ctrl->phy_preem_level_to_val(pdrv, level);
}

int lcd_phy_param_preset(struct aml_lcd_drv_s *pdrv)
{
	struct phy_config_s *phy = &pdrv->config.phy_cfg;
	unsigned int amp = 0, preem = 0;
	int i;

	if (!lcd_phy_ctrl)
		return -1;

	phy->lane_num = lcd_phy_ctrl->lane_num;
	if (lcd_phy_ctrl->phy_glb_param_dft_val)
		lcd_phy_ctrl->phy_glb_param_dft_val(pdrv);
	if (lcd_phy_ctrl->phy_vswing_level_to_val)
		phy->vswing = lcd_phy_ctrl->phy_vswing_level_to_val(pdrv, phy->vswing_level);
	if (lcd_phy_ctrl->phy_preem_level_to_val)
		preem = lcd_phy_ctrl->phy_preem_level_to_val(pdrv, phy->preem_level);
	if (lcd_phy_ctrl->phy_amp_dft_val)
		amp = lcd_phy_ctrl->phy_amp_dft_val(pdrv);
	for (i = 0; i < phy->lane_num; i++) {
		phy->lane[i].amp = amp;
		phy->lane[i].preem = preem;
		phy->lane[i].sel = i;
		phy->lane[i].en = 1;
	}

	return 0;
}

int lcd_phy_param_get(struct aml_lcd_drv_s *pdrv, struct phy_config_s *phy)
{
	int ret;

	if (!pdrv || !phy)
		return -1;
	if (!lcd_phy_ctrl || !lcd_phy_ctrl->phy_param_get) {
		LCDPR("[%d]: %s: phy_param_get is null\n", pdrv->index, __func__);
		return -1;
	}

	memcpy(phy, &pdrv->config.phy_cfg, sizeof(struct phy_config_s));
	lcd_lane_sel_get(pdrv, phy);
	ret = lcd_phy_ctrl->phy_param_get(pdrv, phy);
	return ret;
}

int lcd_phy_param_print(struct aml_lcd_drv_s *pdrv, char *buf, int offset)
{
	struct phy_config_s local_phy, *phy;
	int i, len = 0, ret;

	if (!pdrv)
		return 0;
	if (pdrv->lcd_pxp)
		return 0;

	phy = &pdrv->config.phy_cfg;
	ret = lcd_phy_param_get(pdrv, &local_phy);
	if (ret)
		return 0;

	len = sprintf(buf,
		"vswing  = 0x%x(0x%x)\n"
		"odt     = 0x%x(0x%x)\n"
		"vcm     = 0x%x(0x%x)\n"
		"cv_mode = %d(%d)\n"
		"ref_bias= %d(%d)\n",
		phy->vswing, local_phy.vswing,
		phy->odt, local_phy.odt,
		phy->vcm, local_phy.vcm,
		phy->cv_mode, local_phy.cv_mode,
		phy->ref_bias, local_phy.ref_bias);
	len += sprintf(buf + len, "  lane  en    sel       amp       preem\n");
	for (i = 0; i < local_phy.lane_num; i++) {
		len += sprintf(buf + len,
			"  [%2d]: %d(%d), 0x%x(0x%x), 0x%x(0x%x), 0x%x(0x%x)\n",
			i, phy->lane[i].en, local_phy.lane[i].en,
			phy->lane[i].sel, local_phy.lane[i].sel,
			phy->lane[i].amp, local_phy.lane[i].amp,
			phy->lane[i].preem, local_phy.lane[i].preem);
	}
	len += sprintf(buf + len,
		"flag=0x%x, state=%d, lane_num=%d, lane valid=0x%x, offset=%d, mask=0x%x\n",
		phy->flag, phy->state, phy->lane_num,
		phy->lane_valid, phy->lane_offset, phy->lane_mask);
	len += sprintf(buf + len,
		"ch_swap0=0x%x, ch_swap1=0x%x, ckdi=0x%x\n",
		phy->ch_swap0, phy->ch_swap1, phy->ckdi);

	return len;
}

int lcd_phy_analog_reg_print(struct aml_lcd_drv_s *pdrv, char *buf, int offset)
{
	int n, len = 0;

	if (!pdrv)
		return 0;
	if (!lcd_phy_ctrl || !lcd_phy_ctrl->phy_reg_dump) {
		n = lcd_debug_info_len(len + offset);
		len += snprintf((buf + len), n, "%s: phy_reg_dump is null\n", __func__);
		return len;
	}

	n = lcd_debug_info_len(len + offset);
	len += snprintf((buf + len), n, "\nphy analog regs:\n");
	len += lcd_phy_ctrl->phy_reg_dump(pdrv, (buf + len), (len + offset));
	return len;
}

void lcd_phy_set(struct aml_lcd_drv_s *pdrv, int status)
{
	struct phy_config_s *phy = &pdrv->config.phy_cfg;
	int i;

	if (pdrv->lcd_pxp)
		return;
	if (!lcd_phy_ctrl || !pdrv->phy_set) {
		LCDPR("[%d]: %s: phy_set is null\n", pdrv->index, __func__);
		return;
	}

	for (i = 0; i < pdrv->data->drv_max; i++) {
		if (pdrv->index == i)
			continue;
		if (phy->lane_valid & lcd_phy_ctrl->lane_lock[i]) {
			LCDERR("[%d]: %s: lane_valid 0x%x conflict with lane_lock[%d] 0x%x\n",
				pdrv->index, __func__, phy->lane_valid,
				i, lcd_phy_ctrl->lane_lock[i]);
			return;
		}
	}

	lcd_phy_ctrl->lane_lock[pdrv->index] = phy->lane_valid;
	if (status)
		lcd_phy_ctrl->lane_lock_total |= phy->lane_valid;
	else
		lcd_phy_ctrl->lane_lock_total &= ~phy->lane_valid;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("[%d]: %s: %d: lane_valid=0x%x, lane_lock_total=0x%x\n",
		      pdrv->index, __func__, status, phy->lane_valid,
		      lcd_phy_ctrl->lane_lock_total);
	}
	if (status != LCD_PHY_LOCK_LANE)
		pdrv->phy_set(pdrv, status);
	phy->state = status ? 1 : 0;
}

int lcd_phy_probe(struct aml_lcd_drv_s *pdrv)
{
	if (pdrv->lcd_pxp) {
		pdrv->phy_set = NULL;
		return 0;
	}

	if (!lcd_phy_ctrl)
		return 0;

	lcd_phy_ctrl->lane_lock[pdrv->index] = 0;
	switch (pdrv->config.basic.lcd_type) {
	case LCD_LVDS:
		pdrv->phy_set = lcd_phy_ctrl->phy_set_lvds;
		break;
	case LCD_VBYONE:
		pdrv->phy_set = lcd_phy_ctrl->phy_set_vx1;
		break;
	case LCD_MLVDS:
		pdrv->phy_set = lcd_phy_ctrl->phy_set_mlvds;
		break;
	case LCD_P2P:
		pdrv->phy_set = lcd_phy_ctrl->phy_set_p2p;
		break;
	case LCD_MIPI:
		pdrv->phy_set = lcd_phy_ctrl->phy_set_mipi;
		break;
	case LCD_EDP:
		pdrv->phy_set = lcd_phy_ctrl->phy_set_edp;
		break;
	default:
		pdrv->phy_set = NULL;
		break;
	}

	if (pdrv->status & LCD_STATUS_IF_ON)
		lcd_phy_set(pdrv, LCD_PHY_LOCK_LANE);

	return 0;
}

int lcd_phy_config_init(struct aml_lcd_drv_s *pdrv)
{
	lcd_phy_ctrl = NULL;
	if (pdrv->lcd_pxp)
		return 0;

	switch (pdrv->data->chip_type) {
	case LCD_CHIP_AXG:
		lcd_phy_ctrl = lcd_phy_config_init_axg(pdrv);
		break;
	case LCD_CHIP_G12A:
	case LCD_CHIP_G12B:
	case LCD_CHIP_SM1:
		lcd_phy_ctrl = lcd_phy_config_init_g12(pdrv);
		break;
	case LCD_CHIP_TL1:
	case LCD_CHIP_TM2:
		lcd_phy_ctrl = lcd_phy_config_init_tl1(pdrv);
		break;
	case LCD_CHIP_T5:
	case LCD_CHIP_T5D:
		lcd_phy_ctrl = lcd_phy_config_init_t5(pdrv);
		break;
	case LCD_CHIP_T5W:
		lcd_phy_ctrl = lcd_phy_config_init_t5w(pdrv);
		break;
	case LCD_CHIP_T7:
		lcd_phy_ctrl = lcd_phy_config_init_t7(pdrv);
		break;
	case LCD_CHIP_T3:
	case LCD_CHIP_T5M:
		lcd_phy_ctrl = lcd_phy_config_init_t3_t5m(pdrv);
		break;
	case LCD_CHIP_C3:
		lcd_phy_ctrl = lcd_phy_config_init_c3(pdrv);
		break;
	case LCD_CHIP_T3X:
		lcd_phy_ctrl = lcd_phy_config_init_t3x(pdrv);
		break;
	case LCD_CHIP_TXHD2:
		lcd_phy_ctrl = lcd_phy_config_init_txhd2(pdrv);
		break;
	case LCD_CHIP_S6:
		lcd_phy_ctrl = lcd_phy_config_init_s6(pdrv);
		break;
	case LCD_CHIP_T6D:
		lcd_phy_ctrl = lcd_phy_config_init_t6d(pdrv);
		break;
	default:
		break;
	}
	if (lcd_phy_ctrl)
		lcd_phy_ctrl->lane_lock_total = 0;

	return 0;
}
