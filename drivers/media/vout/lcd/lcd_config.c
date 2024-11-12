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
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_reserved_mem.h>
#include <linux/cma.h>
#include <linux/dma-map-ops.h>
#include <linux/dma-mapping.h>
#include <linux/reset.h>
#include <linux/clk.h>
#include <linux/amlogic/media/vout/lcd/aml_lcd.h>
#include <linux/amlogic/media/vout/lcd/lcd_vout.h>
#include <linux/amlogic/media/vout/lcd/lcd_extern.h>
#include <linux/amlogic/media/vout/lcd/lcd_notify.h>
#include <linux/amlogic/media/vout/lcd/lcd_unifykey.h>
#include <linux/amlogic/media/vout/lcd/aml_bl.h>
#include <linux/amlogic/media/vout/vinfo.h>
#include <linux/amlogic/media/vout/lcd/lcd_resman.h>
#include <linux/page-flags.h>
#include <linux/mm.h>
#include "lcd_common.h"
#include "lcd_reg.h"
#include "lcd_clk/lcd_clk_config.h"

/* **********************************
 * lcd type
 * **********************************
 */
struct lcd_type_match_s {
	char *name;
	enum lcd_type_e type;
};

static struct lcd_type_match_s lcd_type_match_table[] = {
	{"rgb",      LCD_RGB},
	{"lvds",     LCD_LVDS},
	{"vbyone",   LCD_VBYONE},
	{"mipi",     LCD_MIPI},
	{"minilvds", LCD_MLVDS},
	{"p2p",      LCD_P2P},
	{"edp",      LCD_EDP},
	{"bt656",    LCD_BT656},
	{"bt1120",   LCD_BT1120},
	{"invalid",  LCD_TYPE_MAX},
};

int lcd_type_str_to_type(const char *str)
{
	int type = LCD_TYPE_MAX;
	int i;

	for (i = 0; i < ARRAY_SIZE(lcd_type_match_table); i++) {
		if (!strcmp(str, lcd_type_match_table[i].name)) {
			type = lcd_type_match_table[i].type;
			break;
		}
	}
	return type;
}

char *lcd_type_type_to_str(int type)
{
	char *name = lcd_type_match_table[LCD_TYPE_MAX].name;
	int i;

	for (i = 0; i < ARRAY_SIZE(lcd_type_match_table); i++) {
		if (type == lcd_type_match_table[i].type) {
			name = lcd_type_match_table[i].name;
			break;
		}
	}
	return name;
}

static char *lcd_mode_table[] = {
	"tv",
	"tablet",
	"invalid",
};

unsigned char lcd_mode_str_to_mode(const char *str)
{
	unsigned char mode;

	for (mode = 0; mode < ARRAY_SIZE(lcd_mode_table); mode++) {
		if (!strcmp(str, lcd_mode_table[mode]))
			break;
	}
	return mode;
}

char *lcd_mode_mode_to_str(int mode)
{
	return lcd_mode_table[mode];
}

static void lcd_ss_config_fix(struct aml_lcd_drv_s *pdrv)
{
	int i = 0;

	//fix ss in detail timing and phy_attr if not config
	for (i = 0; i < pdrv->config.phy_cfg.group_num; i++) {
		if (pdrv->config.phy_cfg.phys[i]->ss.freq == 255)
			pdrv->config.phy_cfg.phys[i]->ss.freq = pdrv->config.timing.ss_freq;
		if (pdrv->config.phy_cfg.phys[i]->ss.level == 255)
			pdrv->config.phy_cfg.phys[i]->ss.level = pdrv->config.timing.ss_level;
		if (pdrv->config.phy_cfg.phys[i]->ss.mode == 255)
			pdrv->config.phy_cfg.phys[i]->ss.mode = pdrv->config.timing.ss_mode;
	}

	for (i = 0; i < pdrv->config.timing.num_timings; i++) {
		if (pdrv->config.timing.timings[i]->ss_level == 255)
			pdrv->config.timing.timings[i]->ss_level = pdrv->config.timing.ss_level;
		if (pdrv->config.timing.timings[i]->ss_freq == 255)
			pdrv->config.timing.timings[i]->ss_freq = pdrv->config.timing.ss_freq;
		if (pdrv->config.timing.timings[i]->ss_mode == 255)
			pdrv->config.timing.timings[i]->ss_mode = pdrv->config.timing.ss_mode;
	}
}

/* ************************************************** *
 * lcd config
 * **************************************************
 */
static void lcd_config_load_print(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_config_s *pconf = &pdrv->config;
	struct lcd_detail_timing_s *ptiming = pdrv->config.timing.dft_timing;
	struct phy_attr_s *phy;
	struct phy_config_s *phy_cfg = &pdrv->config.phy_cfg;
	union lcd_ctrl_config_u *pctrl;
	int i = 0, pr_len = 4 * 1024;
	char *pr_buf;

	if (!ptiming)
		return;

	if ((lcd_debug_print_flag & LCD_DBG_PR_NORMAL) == 0)
		return;

	LCDPR("[%d]: %s, %s\n",
	      pdrv->index, pconf->basic.model_name,
	      lcd_type_type_to_str(pconf->basic.lcd_type));
	LCDPR("pll_flag = %d\n", pconf->timing.pll_flag);
	LCDPR("clk_mode = %d\n", pconf->timing.clk_mode);
	LCDPR("custom_pinmux = %d\n", pconf->custom_pinmux);
	LCDPR("fr_auto_cus = 0x%x\n", pconf->fr_auto_cus);

	pr_buf = kzalloc(pr_len, GFP_KERNEL);
	if (!pr_buf)
		return;

	LCDPR("\nconfig timings:\n");
	for (i = 0; i < pdrv->config.timing.num_timings; i++) {
		ptiming = pdrv->config.timing.timings[i];
		if (!ptiming)
			continue;
		LCDPR("timing[%d]:\n", i);
		lcd_detail_timing_print(ptiming, pr_buf, 0, pr_len);
		lcd_debug_info_print(pr_buf);
	}

	LCDPR("phy config:\n");
	lcd_phy_cfg_print(phy_cfg, pr_buf, 0, pr_len);
	lcd_debug_info_print(pr_buf);

	for (i = 0; i < phy_cfg->group_num; i++) {
		phy = pdrv->config.phy_cfg.phys[i];
		if (!phy)
			continue;
		LCDPR("phy group[%d]:\n", i);
		lcd_phy_attr_print(phy, phy_cfg->lane_num, pr_buf, 0, pr_len);
		lcd_debug_info_print(pr_buf);
	}

	pctrl = &pconf->control;
	if (pconf->basic.lcd_type == LCD_RGB) {
		LCDPR("type = %d\n", pctrl->rgb_cfg.type);
		LCDPR("clk_pol = %d\n", pctrl->rgb_cfg.clk_pol);
		LCDPR("de_valid = %d\n", pctrl->rgb_cfg.de_valid);
		LCDPR("sync_valid = %d\n", pctrl->rgb_cfg.sync_valid);
		LCDPR("rb_swap = %d\n", pctrl->rgb_cfg.rb_swap);
		LCDPR("bit_swap = %d\n", pctrl->rgb_cfg.bit_swap);
	} else if ((pconf->basic.lcd_type == LCD_BT656) ||
		   (pconf->basic.lcd_type == LCD_BT1120)) {
		LCDPR("clk_phase = 0x%x\n", pctrl->bt_cfg.clk_phase);
		LCDPR("field_type = %d\n", pctrl->bt_cfg.field_type);
		LCDPR("mode_422 = %d\n", pctrl->bt_cfg.mode_422);
		LCDPR("yc_swap = %d\n", pctrl->bt_cfg.yc_swap);
		LCDPR("cbcr_swap = %d\n", pctrl->bt_cfg.cbcr_swap);
	} else if (pconf->basic.lcd_type == LCD_LVDS) {
		LCDPR("lvds_repack = %d\n", pctrl->lvds_cfg.lvds_repack);
		LCDPR("pn_swap = %d\n", pctrl->lvds_cfg.pn_swap);
		LCDPR("dual_port = %d\n", pctrl->lvds_cfg.dual_port);
		LCDPR("port_swap = %d\n", pctrl->lvds_cfg.port_swap);
		LCDPR("lane_reverse = %d\n", pctrl->lvds_cfg.lane_reverse);
		LCDPR("phy_vswing = 0x%x\n", pctrl->lvds_cfg.phy_vswing);
		LCDPR("phy_preem = 0x%x\n", pctrl->lvds_cfg.phy_preem);
	} else if (pconf->basic.lcd_type == LCD_VBYONE) {
		LCDPR("lane_count = %d\n", pctrl->vbyone_cfg.lane_count);
		LCDPR("byte_mode = %d\n", pctrl->vbyone_cfg.byte_mode);
		LCDPR("region_num = %d\n", pctrl->vbyone_cfg.region_num);
		LCDPR("color_fmt = %d\n", pctrl->vbyone_cfg.color_fmt);
		LCDPR("phy_vswing = 0x%x\n", pctrl->vbyone_cfg.phy_vswing);
		LCDPR("phy_preem = 0x%x\n", pctrl->vbyone_cfg.phy_preem);
	} else if (pconf->basic.lcd_type == LCD_MLVDS) {
		LCDPR("channel_num = %d\n", pctrl->mlvds_cfg.channel_num);
		LCDPR("channel_sel0 = 0x%x\n", pctrl->mlvds_cfg.channel_sel0);
		LCDPR("channel_sel1 = 0x%x\n", pctrl->mlvds_cfg.channel_sel1);
		LCDPR("clk_phase = 0x%x\n", pctrl->mlvds_cfg.clk_phase);
		LCDPR("phy_vswing = 0x%x\n", pctrl->mlvds_cfg.phy_vswing);
		LCDPR("phy_preem = 0x%x\n", pctrl->mlvds_cfg.phy_preem);
	} else if (pconf->basic.lcd_type == LCD_P2P) {
		LCDPR("p2p_type = 0x%x\n", pctrl->p2p_cfg.p2p_type);
		LCDPR("lane_num = %d\n", pctrl->p2p_cfg.lane_num);
		LCDPR("channel_sel0 = 0x%x\n", pctrl->p2p_cfg.channel_sel0);
		LCDPR("channel_sel1 = 0x%x\n", pctrl->p2p_cfg.channel_sel1);
		LCDPR("phy_vswing = 0x%x\n", pctrl->p2p_cfg.phy_vswing);
		LCDPR("phy_preem = 0x%x\n", pctrl->p2p_cfg.phy_preem);
	} else if (pconf->basic.lcd_type == LCD_MIPI) {
		if (pctrl->mipi_cfg.check_en) {
			LCDPR("check_reg = 0x%02x\n", pctrl->mipi_cfg.check_reg);
			LCDPR("check_cnt = %d\n", pctrl->mipi_cfg.check_cnt);
		}
		LCDPR("lane_num = %d\n", pctrl->mipi_cfg.lane_num);
		LCDPR("bit_rate_max = %d\n", pctrl->mipi_cfg.bit_rate_max);
		LCDPR("operation_mode_init = %d\n", pctrl->mipi_cfg.operation_mode_init);
		LCDPR("operation_mode_disp = %d\n", pctrl->mipi_cfg.operation_mode_display);
		LCDPR("video_mode_type = %d\n", pctrl->mipi_cfg.video_mode_type);
		LCDPR("clk_always_hs = %d\n", pctrl->mipi_cfg.clk_always_hs);
		LCDPR("extern_init = %d\n", pctrl->mipi_cfg.extern_init);
	} else if (pconf->basic.lcd_type == LCD_EDP) {
		LCDPR("max_lane_count = %d\n", pctrl->edp_cfg.max_lane_count);
		LCDPR("max_link_rate  = %d\n", pctrl->edp_cfg.max_link_rate);
		LCDPR("training_mode  = %d\n", pctrl->edp_cfg.training_mode);
		LCDPR("edid_en        = %d\n", pctrl->edp_cfg.edid_en);
		LCDPR("sync_clk_mode  = %d\n", pctrl->edp_cfg.sync_clk_mode);
		LCDPR("lane_count     = %d\n", pctrl->edp_cfg.lane_count);
		LCDPR("link_rate      = %d\n", pctrl->edp_cfg.link_rate);
		LCDPR("phy_vswing = 0x%x\n", pctrl->edp_cfg.phy_vswing_preset);
		LCDPR("phy_preem  = 0x%x\n", pctrl->edp_cfg.phy_preem_preset);
	}
}

int lcd_base_config_load_from_dts(struct aml_lcd_drv_s *pdrv)
{
	const struct device_node *np;
	const char *mode_str, *str;
	unsigned int val, para[8];
	char str_info[128];
	int str_info_len = 0, ret = 0;

	if (!pdrv->dev->of_node) {
		LCDERR("dev of_node is null\n");
		pdrv->mode = LCD_MODE_MAX;
		return -1;
	}
	np = pdrv->dev->of_node;

	/* lcd driver assign */
	switch (pdrv->debug_ctrl->debug_lcd_mode) {
	case 1:
		LCDPR("[%d]: debug_lcd_mode: 1,tv mode\n", pdrv->index);
		pdrv->mode = LCD_MODE_TV;
		break;
	case 2:
		LCDPR("[%d]: debug_lcd_mode: 2,tablet mode\n", pdrv->index);
		pdrv->mode = LCD_MODE_TABLET;
		break;
	default:
		ret = of_property_read_string(np, "mode", &mode_str);
		if (ret) {
			LCDERR("[%d]: failed to get mode\n", pdrv->index);
			return -1;
		}
		pdrv->mode = lcd_mode_str_to_mode(mode_str);
		break;
	}

	ret = of_property_read_u32(np, "pxp", &val);
	if (ret == 0)
		pdrv->lcd_pxp = (unsigned char)val;

	ret = of_property_read_u32(np, "fr_auto_policy", &val);
	if (ret == 0)
		pdrv->fr_auto_policy = (unsigned char)val;

	ret = of_property_read_u32(np, "vout_regist_on", &val);
	if (ret) {
		pdrv->vout_regist_on_ctrl = 0;
	} else {
		pdrv->vout_regist_on_ctrl = (unsigned char)val;
		LCDPR("set lcd drv regist:%s%s%s\n",
			pdrv->vout_regist_on_ctrl & 0x1 ? " vout"  : "",
			pdrv->vout_regist_on_ctrl & 0x2 ? " vout2" : "",
			pdrv->vout_regist_on_ctrl & 0x4 ? " vout3" : "");
	}

	switch (pdrv->debug_ctrl->debug_para_source) {
	case 1:
		LCDPR("[%d]: debug_para_source: 1,dts\n", pdrv->index);
		pdrv->key_valid = 0;
		break;
	case 2:
		LCDPR("[%d]: debug_para_source: 2,unifykey\n", pdrv->index);
		pdrv->key_valid = 1;
		break;
	default:
		ret = of_property_read_u32(np, "key_valid", &val);
		if (ret) {
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
				LCDPR("failed to get key_valid\n");
			pdrv->key_valid = 0;
		} else {
			pdrv->key_valid = (unsigned char)val;
		}
		break;
	}

	ret = of_property_read_u32(np, "clk_path", &val);
	if (ret) {
		pdrv->clk_path = 0;
	} else {
		pdrv->clk_path = (unsigned char)val;
		LCDPR("[%d]: detect clk_path: %d\n",
		      pdrv->index, pdrv->clk_path);
	}

	ret = of_property_read_u32(np, "auto_test", &val);
	if (ret) {
		pdrv->auto_test = 0;
	} else {
		pdrv->auto_test = (unsigned char)val;
		LCDPR("[%d]: detect auto_test: %d\n",
		      pdrv->index, pdrv->auto_test);
	}

	ret = of_property_read_u32(np, "resume_type", &val);
	if (ret)
		pdrv->resume_type = 1; /* default workqueue */
	else
		pdrv->resume_type = (unsigned char)val;

	ret = of_property_read_u32(np, "config_check_glb", &val);
	if (ret)
		pdrv->config_check_glb = 0;
	else
		pdrv->config_check_glb = (unsigned char)val;

	str_info_len += sprintf(str_info + str_info_len, "fr_auto: %d, ",
			pdrv->fr_auto_policy);
	str_info_len += sprintf(str_info + str_info_len, "resume_type: 0x%x, ",
			pdrv->resume_type);
	sprintf(str_info + str_info_len, "cfg_chk_glb: %d, ", pdrv->config_check_glb);
	LCDPR("[%d]: drv_ver: %s(%d-%s), lcd_mode: %s, key_valid: %d, %s\n",
	      pdrv->index, LCD_DRV_VERSION, pdrv->data->chip_type, pdrv->data->chip_name,
	      mode_str, pdrv->key_valid, str_info);

	ret = of_property_read_u32_array(np, "display_timing_req_min", &para[0], 5);
	if (ret) {
		pdrv->disp_req.alert_level = 0;
		pdrv->disp_req.hswbp_vid = 0;
		pdrv->disp_req.hfp_vid = 0;
		pdrv->disp_req.vswbp_vid = 0;
		pdrv->disp_req.vfp_vid = 0;
	} else {
		pdrv->disp_req.alert_level = para[0];
		pdrv->disp_req.hswbp_vid = para[1];
		pdrv->disp_req.hfp_vid = para[2];
		pdrv->disp_req.vswbp_vid = para[3];
		pdrv->disp_req.vfp_vid = para[4];
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			LCDPR("[%d]: find display_timing_req_min: alert_level:%d\n"
				"hswbp:%d, hfp:%d, vswbp:%d, vfp:%d\n",
				pdrv->index, pdrv->disp_req.alert_level,
				pdrv->disp_req.hswbp_vid, pdrv->disp_req.hfp_vid,
				pdrv->disp_req.vswbp_vid, pdrv->disp_req.vfp_vid);
		}
	}

	/* only for test */
	ret = of_property_read_string(np, "lcd_propname_sel", &str);
	if (ret == 0) {
		strcpy(pdrv->config.propname, str);
		LCDPR("[%d]: find lcd_propname_sel: %s\n",
			pdrv->index, pdrv->config.propname);
	}

	ret = of_property_read_u32_array(np, "sw_vlock", &para[0], 7);
	if (ret == 0) {
		pdrv->fr_lock_en = para[0];
		pdrv->fr_lock = kzalloc(sizeof(*pdrv->fr_lock), GFP_KERNEL);
		if (pdrv->fr_lock) {
			pdrv->fr_lock->en = para[0];
			pdrv->fr_lock->mode = para[1];
			pdrv->fr_lock->kp = para[2];
			pdrv->fr_lock->ki = para[3];
			pdrv->fr_lock->kd = para[4];
			pdrv->fr_lock->line_limit = para[5];
			pdrv->fr_lock->freq_limit = para[6];
			LCDPR("fr_lock:%d, mode:%d, kp:%d, ki:%d, kd:%d,\n"
				  "line_limit:%d, freq_limit:%d\n",
				pdrv->fr_lock_en, pdrv->fr_lock->mode, pdrv->fr_lock->kp,
				pdrv->fr_lock->ki, pdrv->fr_lock->kd,
				pdrv->fr_lock->line_limit, pdrv->fr_lock->freq_limit);
		}
	}

	return 0;
}

static int lcd_power_load_from_dts(struct aml_lcd_drv_s *pdrv, struct device_node *child)
{
	struct lcd_power_ctrl_s *power_step = &pdrv->config.power;
	int ret = 0, append_more = 1;
	unsigned int para[5];
	unsigned int val;
	int i, j, temp;
	unsigned int index;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s\n", pdrv->index, __func__);

	if (!child) {
		LCDERR("[%d]: error: failed to get %s\n",
		      pdrv->index, pdrv->config.propname);
		return -1;
	}

	ret = of_property_read_u32_array(child, "power_on_step", &para[0], 4);
	if (ret) {
		LCDPR("[%d]: failed to get power_on_step\n", pdrv->index);
		power_step->power_on_step[0].type = LCD_POWER_TYPE_MAX;
	} else {
		i = 0;
		while (i < LCD_PWR_STEP_MAX) {
			power_step->power_on_step_max = i;
			j = 4 * i;
			ret = of_property_read_u32_index(child, "power_on_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_on_step %d\n",
				      pdrv->index, i);
				power_step->power_on_step[i].type = 0xff;
				break;
			}
			power_step->power_on_step[i].type = (unsigned char)val;
			if (val == 0xff) /* ending */
				break;
			j = 4 * i + 1;
			ret = of_property_read_u32_index(child, "power_on_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_on_step %d\n",
				      pdrv->index, i);
				power_step->power_on_step[i].type = 0xff;
				break;
			}
			power_step->power_on_step[i].index = val;
			j = 4 * i + 2;
			ret = of_property_read_u32_index(child, "power_on_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_on_step %d\n",
				      pdrv->index, i);
				power_step->power_on_step[i].type = 0xff;
				break;
			}
			power_step->power_on_step[i].value = val;
			j = 4 * i + 3;
			ret = of_property_read_u32_index(child, "power_on_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_on_step %d\n",
				      pdrv->index, i);
				power_step->power_on_step[i].type = 0xff;
				break;
			}
			power_step->power_on_step[i].delay = val;

			/* gpio/extern probe */
			index = power_step->power_on_step[i].index;
			switch (power_step->power_on_step[i].type) {
			case LCD_POWER_TYPE_CPU:
			case LCD_POWER_TYPE_WAIT_GPIO:
				if (index < LCD_CPU_GPIO_NUM_MAX)
					lcd_cpu_gpio_probe(pdrv, index);
				break;
#ifdef CONFIG_AMLOGIC_LCD_EXTERN
			case LCD_POWER_TYPE_EXTERN:
				lcd_resource_add(pdrv, LCD_RES_EXTERN, index);
				lcd_extern_dev_index_add(pdrv->index, index);
				break;
#endif
			case LCD_POWER_TYPE_CLK_SS:
				temp = power_step->power_on_step[i].value;
				pdrv->config.timing.ss_freq = temp & 0xf;
				pdrv->config.timing.ss_mode = (temp >> 4) & 0xf;
				if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
					LCDPR("[%d]: clk_ss value=0x%x: ss_freq=%d, ss_mode=%d\n",
					      pdrv->index, temp,
					      pdrv->config.timing.ss_freq,
					      pdrv->config.timing.ss_mode);
				}
				break;
			case LCD_POWER_TYPE_BACKLIGHT:
			case LCD_POWER_TYPE_MUTE:
				append_more = 0;
				break;
			default:
				break;
			}
			i++;
		}
		if (append_more && i + 2 < LCD_PWR_STEP_MAX) {
			power_step->power_on_step[i].type = LCD_POWER_TYPE_BACKLIGHT;
			power_step->power_on_step[i].index = 0;
			power_step->power_on_step[i].value = 1; //bl on
			power_step->power_on_step[i].delay = 0;
			i++;

			power_step->power_on_step[i].type = LCD_POWER_TYPE_MUTE;
			power_step->power_on_step[i].index = 0;
			power_step->power_on_step[i].value = 0;//unmute
			power_step->power_on_step[i].delay = pdrv->unmute_cnt;
			i++;
			power_step->power_on_step[i].type = LCD_POWER_TYPE_MAX;
			power_step->power_on_step_max = i;
		}
	}

	ret = of_property_read_u32_array(child, "power_off_step", &para[0], 4);
	if (ret) {
		LCDPR("[%d]: failed to get power_off_step\n", pdrv->index);
		power_step->power_off_step[0].type = LCD_POWER_TYPE_MAX;
	} else {
		append_more = 1;
		i = 0;
		while (i < LCD_PWR_STEP_MAX) {
			power_step->power_off_step_max = i;

			j = 4 * i;
			ret = of_property_read_u32_index(child, "power_off_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_off_step %d\n", pdrv->index, i);
				power_step->power_off_step[i].type = 0xff;
				break;
			}
			power_step->power_off_step[i].type = (unsigned char)val;
			if (val == 0xff) /* ending */
				break;
			j = 4 * i + 1;
			ret = of_property_read_u32_index(child, "power_off_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_off_step %d\n", pdrv->index, i);
				power_step->power_off_step[i].type = 0xff;
				break;
			}
			power_step->power_off_step[i].index = val;
			j = 4 * i + 2;
			ret = of_property_read_u32_index(child, "power_off_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_off_step %d\n", pdrv->index, i);
				power_step->power_off_step[i].type = 0xff;
				break;
			}
			power_step->power_off_step[i].value = val;
			j = 4 * i + 3;
			ret = of_property_read_u32_index(child, "power_off_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_off_step %d\n", pdrv->index, i);
				power_step->power_off_step[i].type = 0xff;
				break;
			}
			power_step->power_off_step[i].delay = val;

			/* gpio/extern probe */
			index = power_step->power_off_step[i].index;
			switch (power_step->power_off_step[i].type) {
			case LCD_POWER_TYPE_CPU:
			case LCD_POWER_TYPE_WAIT_GPIO:
				if (index < LCD_CPU_GPIO_NUM_MAX)
					lcd_cpu_gpio_probe(pdrv, index);
				break;
#ifdef CONFIG_AMLOGIC_LCD_EXTERN
			case LCD_POWER_TYPE_EXTERN:
				lcd_resource_add(pdrv, LCD_RES_EXTERN, index);
				lcd_extern_dev_index_add(pdrv->index, index);
				break;
#endif
			case LCD_POWER_TYPE_BACKLIGHT:
			case LCD_POWER_TYPE_MUTE:
				append_more = 0;
				break;
			default:
				break;
			}
			i++;
		}
		if (append_more && i + 2 < LCD_POWER_TYPE_MAX) {
			for (i = i + 2; i >= 2; i--)
				memcpy(&power_step->power_off_step[i],
				       &power_step->power_off_step[i - 2],
				       sizeof(struct lcd_power_step_s));
			power_step->power_off_step_max += 2;
			power_step->power_off_step[0].type  = LCD_POWER_TYPE_MUTE;
			power_step->power_off_step[0].index = 0;
			power_step->power_off_step[0].value = 1; //mute
			power_step->power_off_step[0].delay = pdrv->mute_cnt;

			power_step->power_off_step[1].type = LCD_POWER_TYPE_BACKLIGHT;
			power_step->power_off_step[1].index = 0;
			power_step->power_off_step[1].value = 0;//bl off
			power_step->power_off_step[1].delay = 0;
		}
	}

	return ret;
}

static int lcd_vlock_param_load_from_dts(struct aml_lcd_drv_s *pdrv, struct device_node *child)
{
	unsigned int para[4];
	int ret;

	pdrv->config.vlock_param[0] = LCD_VLOCK_PARAM_BIT_UPDATE;

	ret = of_property_read_u32_array(child, "vlock_attr", &para[0], 4);
	if (ret == 0) {
		LCDPR("[%d]: find vlock_attr\n", pdrv->index);
		pdrv->config.vlock_param[0] |= LCD_VLOCK_PARAM_BIT_VALID;
		pdrv->config.vlock_param[1] = para[0];
		pdrv->config.vlock_param[2] = para[1];
		pdrv->config.vlock_param[3] = para[2];
		pdrv->config.vlock_param[4] = para[3];
	}

	return 0;
}

static int lcd_optical_load_from_dts(struct aml_lcd_drv_s *pdrv, struct device_node *child)
{
	unsigned int para[13];
	int ret;

	ret = of_property_read_u32_array(child, "optical_attr", &para[0], 13);
	if (ret == 0) {
		LCDPR("[%d]: find optical_attr\n", pdrv->index);
		pdrv->config.optical.hdr_support = para[0];
		pdrv->config.optical.features = para[1];
		pdrv->config.optical.primaries_r_x = para[2];
		pdrv->config.optical.primaries_r_y = para[3];
		pdrv->config.optical.primaries_g_x = para[4];
		pdrv->config.optical.primaries_g_y = para[5];
		pdrv->config.optical.primaries_b_x = para[6];
		pdrv->config.optical.primaries_b_y = para[7];
		pdrv->config.optical.white_point_x = para[8];
		pdrv->config.optical.white_point_y = para[9];
		pdrv->config.optical.luma_max = para[10];
		pdrv->config.optical.luma_min = para[11];
		pdrv->config.optical.luma_avg = para[12];
	}
	ret = of_property_read_u32_array(child, "optical_adv_val", &para[0], 13);
	if (ret == 0) {
		LCDPR("[%d]: find optical_adv_val\n", pdrv->index);
		pdrv->config.optical.ldim_support = para[0];
		pdrv->config.optical.luma_peak = para[4];
	}

	lcd_optical_vinfo_update(pdrv);

	return 0;
}

static int lcd_config_load_from_dts(struct aml_lcd_drv_s *pdrv)
{
	struct device_node *child;
	struct lcd_config_s *pconf = &pdrv->config;
	struct lcd_detail_timing_s *ptiming;
	struct lcd_timing_s *tims = &pdrv->config.timing;
	union lcd_ctrl_config_u *pctrl = &pdrv->config.control;
	struct phy_config_s *phy_cfg = &pdrv->config.phy_cfg;
	struct phy_attr_s *phy = NULL;
	unsigned int para[10], val;
	const char *str;
	char str_info[128];
	int i, str_info_len = 0, ret = 0;

	if (!pdrv->dev->of_node) {
		LCDERR("[%d]: dev of_node is null\n", pdrv->index);
		return -1;
	}

	child = of_get_child_by_name(pdrv->dev->of_node, pconf->propname);
	if (!child) {
		LCDERR("[%d]: failed to get %s\n", pdrv->index, pconf->propname);
		return -1;
	}

	ret = of_property_read_string(child, "model_name", &str);
	if (ret) {
		LCDERR("[%d]: failed to get model_name\n", pdrv->index);
		strncpy(pconf->basic.model_name, pconf->propname, MOD_LEN_MAX);
	} else {
		strncpy(pconf->basic.model_name, str, MOD_LEN_MAX);
	}
	/* ensure string ending */
	pconf->basic.model_name[MOD_LEN_MAX - 1] = '\0';

	ret = of_property_read_string(child, "interface", &str);
	if (ret) {
		LCDERR("[%d]: failed to get interface\n", pdrv->index);
		str = "invalid";
	}
	pconf->basic.lcd_type = lcd_type_str_to_type(str);

	ret = of_property_read_u32(child, "config_check", &val);
	if (ret == 0)
		pconf->basic.config_check = val ? 0x3 : 0x2;

	ret = of_property_read_u32_array(child, "basic_setting", &para[0], 7);
	if (ret) {
		LCDERR("[%d]: failed to get basic_setting\n", pdrv->index);
		return -1;
	}

	ptiming = lcd_timing_alloc(pdrv);
	if (!ptiming) {
		LCDERR("[%d] dft_timing alloc fail\n", pdrv->index);
		return -1;
	}
	memset(ptiming, 0, sizeof(*ptiming));
	tims->dft_timing = tims->timings[0];

	ptiming->h_active = para[0];
	ptiming->v_active = para[1];
	ptiming->h_period = para[2];
	ptiming->v_period = para[3];
	ptiming->lcd_bits = para[4] * 3;
	pconf->basic.screen_width = para[5];
	pconf->basic.screen_height = para[6];

	ret = of_property_read_u32_array(child, "range_setting", &para[0], 6);
	if (ret) {
		LCDPR("[%d]: no range_setting\n", pdrv->index);
		ptiming->h_period_min = ptiming->h_period;
		ptiming->h_period_max = ptiming->h_period;
		ptiming->v_period_min = ptiming->v_period;
		ptiming->v_period_max = ptiming->v_period;
		ptiming->pclk_min = 0;
		ptiming->pclk_max = 0;
	} else {
		ptiming->h_period_min = para[0];
		ptiming->h_period_max = para[1];
		ptiming->v_period_min = para[2];
		ptiming->v_period_max = para[3];
		ptiming->pclk_min = para[4];
		ptiming->pclk_max = para[5];
	}

	ret = of_property_read_u32_array(child, "range_frame_rate", &para[0], 6);
	if (ret == 0) {
		ptiming->frame_rate_min = para[0];
		ptiming->frame_rate_max = para[1];
	}

	ret = of_property_read_u32_array(child, "lcd_timing", &para[0], 6);
	if (ret) {
		LCDERR("[%d]: failed to get lcd_timing\n", pdrv->index);
		lcd_timing_free_last(pdrv);
		return -1;
	}
	ptiming->hsync_width = (unsigned short)(para[0]);
	ptiming->hsync_bp = (unsigned short)(para[1]);
	ptiming->hsync_fp = ptiming->h_period - ptiming->h_active -
			ptiming->hsync_width - ptiming->hsync_bp;
	ptiming->hsync_pol = (unsigned short)(para[2]);
	ptiming->vsync_width = (unsigned short)(para[3]);
	ptiming->vsync_bp = (unsigned short)(para[4]);
	ptiming->vsync_fp = ptiming->v_period - ptiming->v_active -
			ptiming->vsync_width - ptiming->vsync_bp;
	ptiming->vsync_pol = (unsigned short)(para[5]);
	ret = of_property_read_u32(child, "ppc_mode", &val);
	if (ret)
		pconf->timing.ppc = 1;
	else
		pconf->timing.ppc = val;

	ret = of_property_read_u32(child, "clk_mode", &val);
	if (ret)
		pconf->timing.clk_mode = LCD_CLK_MODE_DEPENDENCE;
	else
		pconf->timing.clk_mode = val;

	ret = of_property_read_u32_array(child, "pre_de", &para[0], 2);
	if (ret) {
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
			LCDERR("failed to get pre_de\n");
		pconf->timing.pre_de_h = 0;
		pconf->timing.pre_de_v = 0;
	} else {
		pconf->timing.pre_de_h = (unsigned short)(para[0]);
		pconf->timing.pre_de_v = (unsigned short)(para[1]);
	}

	ret = of_property_read_u32_array(child, "clk_attr", &para[0], 4);
	if (ret) {
		LCDERR("[%d]: failed to get clk_attr\n", pdrv->index);
		ptiming->fr_adjust_type = 0;
		pconf->timing.ss_level = 0;
		pconf->timing.ss_freq = 0;
		pconf->timing.ss_mode = 0;
		pconf->timing.pll_flag = 1;
		ptiming->pixel_clk = 60;
	} else {
		ptiming->fr_adjust_type = (unsigned char)(para[0]);
		pconf->timing.ss_level = para[1] & 0xff;
		pconf->timing.ss_freq = (para[1] >> 8) & 0xf;
		pconf->timing.ss_mode = (para[1] >> 12) & 0xf;
		pconf->timing.pll_flag = para[2] & 0xf;
		ptiming->pixel_clk = para[3];
	}
	ptiming->switch_type = LCD_VMODE_SWITCH_NONE;
	ptiming->ss_force = 0;
	ptiming->ss_freq = pconf->timing.ss_freq;
	ptiming->ss_level = pconf->timing.ss_level;
	ptiming->ss_mode = pconf->timing.ss_mode;

	lcd_clk_frame_rate_init(ptiming);
	lcd_default_to_basic_timing_init_config(pdrv);
	lcd_config_timing_check(pdrv, ptiming);

	ret = of_property_read_u32(child, "custom_pinmux", &val);
	if (ret) {
		ret = of_property_read_u32(child, "customer_pinmux", &val);
		if (ret)
			pconf->custom_pinmux = 0;
		else
			pconf->custom_pinmux = val;
	} else {
		pconf->custom_pinmux = val;
	}

	ret = of_property_read_u32(child, "fr_auto_disable", &val);
	if (ret) {
		ret = of_property_read_u32(child, "fr_auto_custom", &val);
		if (ret)
			pconf->fr_auto_cus = 0;
		else
			pconf->fr_auto_cus = val;
	} else {
		if (val)
			pconf->fr_auto_cus = 0xff;
		else
			pconf->fr_auto_cus = 0;
	}

	str_info_len += sprintf(str_info + str_info_len, "ppc:%d, ",
			pconf->timing.ppc);
	str_info_len += sprintf(str_info + str_info_len, "clk_mode:%d, ",
			pconf->timing.clk_mode);
	if (pconf->timing.pre_de_h || pconf->timing.pre_de_v) {
		str_info_len += sprintf(str_info + str_info_len, "pre_de:%d,%d, ",
				pconf->timing.pre_de_h, pconf->timing.pre_de_v);
	}
	str_info_len += sprintf(str_info + str_info_len, "cfg_chk:0x%x, ",
			pconf->basic.config_check);
	str_info_len += sprintf(str_info + str_info_len, "cus_pinmux:%d, ",
			pconf->custom_pinmux);
	sprintf(str_info + str_info_len, "afr_cus:0x%x", pconf->fr_auto_cus);

	LCDPR("[%d]: load dts config: %s, %s, %dbit, %dx%d, %s\n",
	      pdrv->index, pconf->basic.model_name,
	      lcd_type_type_to_str(pconf->basic.lcd_type),
	      ptiming->lcd_bits, ptiming->h_active, ptiming->v_active,
	      str_info);

	switch (pconf->basic.lcd_type) {
	case LCD_RGB:
		ret = of_property_read_u32_array(child, "rgb_attr", &para[0], 6);
		if (ret) {
			LCDERR("[%d]: failed to get rgb_attr\n", pdrv->index);
			return -1;
		}
		pctrl->rgb_cfg.type = para[0];
		pctrl->rgb_cfg.clk_pol = para[1];
		pctrl->rgb_cfg.de_valid = para[2];
		pctrl->rgb_cfg.sync_valid = para[3];
		pctrl->rgb_cfg.rb_swap = para[4];
		pctrl->rgb_cfg.bit_swap = para[5];
		break;
	case LCD_BT656:
	case LCD_BT1120:
		ret = of_property_read_u32_array(child, "bt_attr", &para[0], 8);
		if (ret) {
			LCDERR("[%d]: failed to get bt_attr\n", pdrv->index);
			return -1;
		}
		pctrl->bt_cfg.clk_phase = para[0];
		pctrl->bt_cfg.field_type = para[1];
		pctrl->bt_cfg.mode_422 = para[2];
		pctrl->bt_cfg.yc_swap = para[3];
		pctrl->bt_cfg.cbcr_swap = para[4];
		break;
	case LCD_LVDS:
		ret = of_property_read_u32_array(child, "lvds_attr", &para[0], 5);
		if (ret) {
			LCDPR("[%d]: failed to get lvds_attr\n", pdrv->index);
			return -1;
		}
		pctrl->lvds_cfg.lvds_repack = para[0];
		pctrl->lvds_cfg.dual_port = para[1];
		pctrl->lvds_cfg.pn_swap = para[2];
		pctrl->lvds_cfg.port_swap = para[3];
		pctrl->lvds_cfg.lane_reverse = para[4];

		ret = of_property_read_u32_array(child, "phy_attr", &para[0], 2);
		if (ret) {
			LCDPR("[%d]: failed to get phy_attr\n", pdrv->index);
			pctrl->lvds_cfg.phy_vswing = 0x5;
			pctrl->lvds_cfg.phy_preem  = 0x1;
		} else {
			pctrl->lvds_cfg.phy_vswing = para[0];
			pctrl->lvds_cfg.phy_preem = para[1];
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: phy vswing_level=0x%x, preem_level=0x%x\n",
				      pdrv->index,
				      pctrl->lvds_cfg.phy_vswing,
				      pctrl->lvds_cfg.phy_preem);
			}
		}

		phy_cfg->vswing_level = pctrl->lvds_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->lvds_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->lvds_cfg.phy_preem;
		break;
	case LCD_VBYONE:
		ret = of_property_read_u32_array(child, "vbyone_attr", &para[0], 4);
		if (ret) {
			LCDERR("[%d]: failed to get vbyone_attr\n", pdrv->index);
			return -1;
		}
		pctrl->vbyone_cfg.lane_count = para[0];
		pctrl->vbyone_cfg.region_num = para[1];
		pctrl->vbyone_cfg.byte_mode = para[2];
		pctrl->vbyone_cfg.color_fmt = para[3];

		pctrl->vbyone_cfg.slice = pdrv->config.timing.ppc ? pdrv->config.timing.ppc : 1;

		ret = of_property_read_u32_array(child, "vbyone_intr_enable", &para[0], 2);
		if (ret) {
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
				LCDERR("[%d]: failed to get vbyone_intr_enable\n", pdrv->index);
		} else {
			pctrl->vbyone_cfg.intr_en = para[0];
			pctrl->vbyone_cfg.vsync_intr_en = para[1];
		}
		ret = of_property_read_u32_array(child, "phy_attr", &para[0], 2);
		if (ret) {
			LCDPR("[%d]: failed to get phy_attr\n", pdrv->index);
			pctrl->vbyone_cfg.phy_vswing = 0x5;
			pctrl->vbyone_cfg.phy_preem  = 0x1;
		} else {
			pctrl->vbyone_cfg.phy_vswing = para[0];
			pctrl->vbyone_cfg.phy_preem = para[1];
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: phy vswing_level=0x%x, preem_level=0x%x\n",
				      pdrv->index,
				      pctrl->vbyone_cfg.phy_vswing,
				      pctrl->vbyone_cfg.phy_preem);
			}
		}

		phy_cfg->vswing_level = pctrl->vbyone_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->vbyone_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->vbyone_cfg.phy_preem;

		ret = of_property_read_u32(child, "vbyone_ctrl_flag", &val);
		if (ret) {
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
				LCDPR("[%d]: failed to get vbyone_ctrl_flag\n", pdrv->index);
		} else {
			pctrl->vbyone_cfg.ctrl_flag = val;
			LCDPR("[%d]: vbyone ctrl_flag=0x%x\n",
			      pdrv->index, pctrl->vbyone_cfg.ctrl_flag);
		}
		if (pctrl->vbyone_cfg.ctrl_flag & 0x7) {
			ret = of_property_read_u32_array(child, "vbyone_ctrl_timing", &para[0], 3);
			if (ret) {
				LCDPR("[%d]: failed to get vbyone_ctrl_timing\n", pdrv->index);
			} else {
				pctrl->vbyone_cfg.power_on_reset_delay = para[0];
				pctrl->vbyone_cfg.hpd_data_delay = para[1];
				pctrl->vbyone_cfg.cdr_training_hold = para[2];
			}
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: power_on_reset_delay: %d\n",
				      pdrv->index,
				      pctrl->vbyone_cfg.power_on_reset_delay);
				LCDPR("[%d]: hpd_data_delay: %d\n",
				      pdrv->index,
				      pctrl->vbyone_cfg.hpd_data_delay);
				LCDPR("[%d]: cdr_training_hold: %d\n",
				      pdrv->index,
				      pctrl->vbyone_cfg.cdr_training_hold);
			}
		}
		ret = of_property_read_u32_array(child, "hw_filter", &para[0], 2);
		if (ret) {
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
				LCDPR("[%d]: failed to get hw_filter\n", pdrv->index);
		} else {
			pctrl->vbyone_cfg.hw_filter_time = para[0];
			pctrl->vbyone_cfg.hw_filter_cnt = para[1];
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: vbyone hw_filter=0x%x 0x%x\n",
				      pdrv->index,
				      pctrl->vbyone_cfg.hw_filter_time,
				      pctrl->vbyone_cfg.hw_filter_cnt);
			}
		}
		break;
	case LCD_MLVDS:
		ret = of_property_read_u32_array(child, "minilvds_attr", &para[0], 6);
		if (ret) {
			LCDERR("[%d]: failed to get minilvds_attr\n", pdrv->index);
			return -1;
		}
		pctrl->mlvds_cfg.channel_num = para[0];
		pctrl->mlvds_cfg.channel_sel0 = para[1];
		pctrl->mlvds_cfg.channel_sel1 = para[2];
		pctrl->mlvds_cfg.clk_phase = para[3];
		pctrl->mlvds_cfg.pn_swap = para[4];
		pctrl->mlvds_cfg.bit_swap = para[5];

		ret = of_property_read_u32_array(child, "phy_attr", &para[0], 2);
		if (ret) {
			LCDPR("[%d]: failed to get phy_attr\n", pdrv->index);
			pctrl->mlvds_cfg.phy_vswing = 0x5;
			pctrl->mlvds_cfg.phy_preem  = 0x1;
		} else {
			pctrl->mlvds_cfg.phy_vswing = para[0];
			pctrl->mlvds_cfg.phy_preem = para[1];
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: phy vswing_level=0x%x, preem_level=0x%x\n",
				      pdrv->index,
				      pctrl->mlvds_cfg.phy_vswing,
				      pctrl->mlvds_cfg.phy_preem);
			}
		}

		phy_cfg->vswing_level = pctrl->mlvds_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->mlvds_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->mlvds_cfg.phy_preem;
		break;
	case LCD_P2P:
		ret = of_property_read_u32_array(child, "p2p_attr", &para[0], 6);
		if (ret) {
			LCDERR("[%d]: failed to get p2p_attr\n", pdrv->index);
			return -1;
		}
		pctrl->p2p_cfg.p2p_type = para[0];
		pctrl->p2p_cfg.lane_num = para[1];
		pctrl->p2p_cfg.channel_sel0 = para[2];
		pctrl->p2p_cfg.channel_sel1 = para[3];
		pctrl->p2p_cfg.pn_swap = para[4];
		pctrl->p2p_cfg.bit_swap = para[5];

		ret = of_property_read_u32_array(child, "phy_attr", &para[0], 2);
		if (ret) {
			LCDPR("[%d]: failed to get phy_attr\n", pdrv->index);
			pctrl->p2p_cfg.phy_vswing = 0x5;
			pctrl->p2p_cfg.phy_preem  = 0x1;
		} else {
			pctrl->p2p_cfg.phy_vswing = para[0];
			pctrl->p2p_cfg.phy_preem = para[1];
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: phy vswing_level=0x%x, preem_level=0x%x\n",
				      pdrv->index,
				      pctrl->p2p_cfg.phy_vswing,
				      pctrl->p2p_cfg.phy_preem);
			}
		}

		phy_cfg->vswing_level = pctrl->p2p_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->p2p_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->p2p_cfg.phy_preem;
		break;
	case LCD_MIPI:
		ret = of_property_read_u32_array(child, "mipi_attr", &para[0], 8);
		if (ret) {
			LCDERR("[%d]: failed to get mipi_attr\n", pdrv->index);
			return -1;
		}
		pctrl->mipi_cfg.lane_num = para[0];
		pctrl->mipi_cfg.bit_rate_max = para[1];
		pctrl->mipi_cfg.operation_mode_init = para[3];
		pctrl->mipi_cfg.operation_mode_display = para[4];
		pctrl->mipi_cfg.video_mode_type = para[5];
		pctrl->mipi_cfg.clk_always_hs = para[6];

#ifdef CONFIG_AMLOGIC_LCD_TABLET
		lcd_mipi_dsi_init_table_detect(pdrv, child);
#endif
#ifdef CONFIG_AMLOGIC_LCD_EXTERN
		ret = of_property_read_u32_array(child, "extern_init", &para[0], 1);
		if (ret) {
			LCDPR("[%d]: failed to get extern_init\n", pdrv->index);
		} else {
			pctrl->mipi_cfg.extern_init = para[0];
			lcd_resource_add(pdrv, LCD_RES_EXTERN, para[0]);
			lcd_extern_dev_index_add(pdrv->index, para[0]);
		}
#endif

		phy_cfg->vswing_level = 0;
		phy_cfg->preem_level = 0;
		break;
	case LCD_EDP:
		ret = of_property_read_u32_array(child, "edp_attr", &para[0], 9);
		if (ret) {
			LCDERR("[%d]: failed to get edp_attr\n", pdrv->index);
			return -1;
		}
		pctrl->edp_cfg.max_lane_count = (unsigned char)para[0];
		pctrl->edp_cfg.max_link_rate = (unsigned char)(para[1] < 0x6 ? 0 : para[1]);
		pctrl->edp_cfg.training_mode = (unsigned char)para[2];
		pctrl->edp_cfg.edid_en = (unsigned char)para[3];

		ret = of_property_read_u32_array(child, "phy_attr", &para[0], 2);
		if (ret) {
			LCDPR("[%d]: failed to get phy_attr\n", pdrv->index);
			pctrl->edp_cfg.phy_vswing_preset = 0x5;
			pctrl->edp_cfg.phy_preem_preset  = 0x1;
		} else {
			pctrl->edp_cfg.phy_vswing_preset = para[0];
			pctrl->edp_cfg.phy_preem_preset = para[1];
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: phy vswing_level=0x%x, preem_level=0x%x\n",
				      pdrv->index,
				      pctrl->edp_cfg.phy_vswing_preset,
				      pctrl->edp_cfg.phy_preem_preset);
			}
		}

		phy_cfg->vswing_level = pctrl->edp_cfg.phy_vswing_preset & 0xf;
		phy_cfg->preem_level = pctrl->edp_cfg.phy_preem_preset;
		break;
	default:
		LCDERR("[%d]: invalid lcd type\n", pdrv->index);
		break;
	}

	phy = lcd_phy_alloc(pdrv);// first parse, it is only be phy_cfg->phys[0];
	if (!phy) {
		LCDERR("%s phy alloc fail\n", __func__);
		return -1;
	}
	memset(phy, 0, sizeof(*phy));
	phy_cfg->act_phy = phy_cfg->phys[0];
	lcd_phy_param_preset(pdrv);
	lcd_lane_map_preset(pdrv);
	phy->ss.freq = 255;
	phy->ss.level = 255;
	phy->ss.mode = 255;

	ret = of_property_read_u32_array(child, "phy_adv_attr", &para[0], 5);
	if (ret)
		goto config_phy_adv_attr_done;

	phy_cfg->flag = para[0];
	phy->vswing   = para[1];
	phy->vcm      = para[2];
	phy->ref_bias = para[3];
	phy->odt      = para[4];
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("%s: ctrl_flag=0x%x vsw=0x%08x vcm=0x%x, ref_bias=0x%x, odt=0x%x\n",
			__func__, phy_cfg->flag, phy->vswing, phy->vcm,
			phy->ref_bias, phy->odt);
	}
	ret = of_property_read_variable_u32_array(child, "phy_lane_ctrl", &para[0], 1, 32);
	if (ret <= 0 || ((phy_cfg->flag & (0x3 << 12)) == 0))
		goto config_phy_adv_attr_done;

	for (i = 0; i < phy_cfg->lane_num; i++) {
		if (i >= ret)
			break;

		if (phy_cfg->flag & (1 << 12))
			phy->lane[i].preem = para[i] & 0xffff;

		if (phy_cfg->flag & (1 << 13))
			phy->lane[i].amp   = para[i] >> 16;

		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
			LCDPR("%s: lane[%d]: preem=0x%x amp=0x%x\n",
				__func__, i, phy->lane[i].preem, phy->lane[i].amp);
	}
config_phy_adv_attr_done:

	lcd_vlock_param_load_from_dts(pdrv, child);
	ret = lcd_power_load_from_dts(pdrv, child);

	lcd_optical_load_from_dts(pdrv, child);

	lcd_cus_ctrl_load_from_dts(pdrv, child);

	//fix ss in detail timing and phy_attr if not config
	lcd_ss_config_fix(pdrv);

	ret = of_property_read_u32(child, "backlight_index", &para[0]);
	if (ret) {
		LCDPR("[%d]: failed to get backlight_index\n", pdrv->index);
		pconf->backlight_index = 0xff;
	} else {
		pconf->backlight_index = para[0];
#ifdef CONFIG_AMLOGIC_BACKLIGHT
		lcd_resource_add(pdrv, LCD_RES_BACKLIGHT, pconf->backlight_index);
		aml_bl_index_add(pdrv->index, pconf->backlight_index);
#endif
	}

	return ret;
}

static int lcd_power_load_from_unifykey(struct aml_lcd_drv_s *pdrv,
					unsigned char *buf, int key_len, int len)
{
	struct lcd_power_ctrl_s *power_step = &pdrv->config.power;
	int i, j, temp;
	unsigned char *p;
	unsigned int index;
	int ret, append_more = 1;

	/* power: (5byte * n) */
	p = buf + len;
	i = 0;
	while (i < LCD_PWR_STEP_MAX) {
		power_step->power_on_step_max = i;
		len += 5;
		ret = lcd_unifykey_len_check(key_len, len);
		if (ret < 0) {
			power_step->power_on_step[i].type = 0xff;
			power_step->power_on_step[i].index = 0;
			power_step->power_on_step[i].value = 0;
			power_step->power_on_step[i].delay = 0;
			LCDERR("[%d]: unifykey power_on length is incorrect\n", pdrv->index);
			return -1;
		}
		power_step->power_on_step[i].type = *(p + LCD_UKEY_PWR_TYPE + 5 * i);
		power_step->power_on_step[i].index = *(p + LCD_UKEY_PWR_INDEX + 5 * i);
		power_step->power_on_step[i].value = *(p + LCD_UKEY_PWR_VAL + 5 * i);
		power_step->power_on_step[i].delay =
			(*(p + LCD_UKEY_PWR_DELAY + 5 * i) |
			((*(p + LCD_UKEY_PWR_DELAY + 5 * i + 1)) << 8));

		/* gpio/extern probe */
		index = power_step->power_on_step[i].index;
		switch (power_step->power_on_step[i].type) {
		case LCD_POWER_TYPE_CPU:
		case LCD_POWER_TYPE_WAIT_GPIO:
			if (index < LCD_CPU_GPIO_NUM_MAX)
				lcd_cpu_gpio_probe(pdrv, index);
			break;
#ifdef CONFIG_AMLOGIC_LCD_EXTERN
		case LCD_POWER_TYPE_EXTERN:
			lcd_resource_add(pdrv, LCD_RES_EXTERN, index);
			lcd_extern_dev_index_add(pdrv->index, index);
			break;
#endif
		case LCD_POWER_TYPE_CLK_SS:
			temp = power_step->power_on_step[i].value;
			pdrv->config.timing.ss_freq = temp & 0xf;
			pdrv->config.timing.ss_mode = (temp >> 4) & 0xf;
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: clk_ss value=0x%x: ss_freq=%d, ss_mode=%d\n",
					      pdrv->index, temp,
					      pdrv->config.timing.ss_freq,
					      pdrv->config.timing.ss_mode);
			}
			break;
		case LCD_POWER_TYPE_BACKLIGHT:
		case LCD_POWER_TYPE_MUTE:
			append_more = 0;
			break;
		default:
			break;
		}
		if (power_step->power_on_step[i].type >= LCD_POWER_TYPE_MAX)
			break;
		i++;
	}
	p += (5 * (i + 1));
	if (append_more && i + 2 < LCD_PWR_STEP_MAX) {
		power_step->power_on_step[i].type = LCD_POWER_TYPE_BACKLIGHT;
		power_step->power_on_step[i].index = 0;
		power_step->power_on_step[i].value = 1; //bl on
		power_step->power_on_step[i].delay = 0;
		i++;
		power_step->power_on_step[i].type = LCD_POWER_TYPE_MUTE;
		power_step->power_on_step[i].index = 0;
		power_step->power_on_step[i].value = 0;//unmute
		power_step->power_on_step[i].delay = pdrv->unmute_cnt;
		i++;
		power_step->power_on_step[i].type = LCD_POWER_TYPE_MAX;
		power_step->power_on_step_max = i;
	}

	append_more = 1;
	j = 0;
	while (j < LCD_PWR_STEP_MAX) {
		power_step->power_off_step_max = j;
		len += 5;
		ret = lcd_unifykey_len_check(key_len, len);
		if (ret < 0) {
			power_step->power_off_step[j].type = 0xff;
			power_step->power_off_step[j].index = 0;
			power_step->power_off_step[j].value = 0;
			power_step->power_off_step[j].delay = 0;
			LCDERR("[%d]: unifykey power_off length is incorrect\n", pdrv->index);
			return -1;
		}
		power_step->power_off_step[j].type = *(p + LCD_UKEY_PWR_TYPE + 5 * j);
		power_step->power_off_step[j].index = *(p + LCD_UKEY_PWR_INDEX + 5 * j);
		power_step->power_off_step[j].value = *(p + LCD_UKEY_PWR_VAL + 5 * j);
		power_step->power_off_step[j].delay =
				(*(p + LCD_UKEY_PWR_DELAY + 5 * j) |
				((*(p + LCD_UKEY_PWR_DELAY + 5 * j + 1)) << 8));

		/* gpio/extern probe */
		index = power_step->power_off_step[j].index;
		switch (power_step->power_off_step[j].type) {
		case LCD_POWER_TYPE_CPU:
		case LCD_POWER_TYPE_WAIT_GPIO:
			if (index < LCD_CPU_GPIO_NUM_MAX)
				lcd_cpu_gpio_probe(pdrv, index);
			break;
#ifdef CONFIG_AMLOGIC_LCD_EXTERN
		case LCD_POWER_TYPE_EXTERN:
			lcd_resource_add(pdrv, LCD_RES_EXTERN, index);
			lcd_extern_dev_index_add(pdrv->index, index);
			break;
#endif
		case LCD_POWER_TYPE_BACKLIGHT:
		case LCD_POWER_TYPE_MUTE:
			append_more = 0;
			break;
		default:
			break;
		}
		if (power_step->power_off_step[j].type >= LCD_POWER_TYPE_MAX)
			break;
		j++;
	}

	if (append_more && j + 2 < LCD_POWER_TYPE_MAX) {
		for (i = j + 2; i >= 2; i--)
			memcpy(&power_step->power_off_step[i],
			       &power_step->power_off_step[i - 2],
			       sizeof(struct lcd_power_step_s));
		power_step->power_off_step_max += 2;
		power_step->power_off_step[0].type  = LCD_POWER_TYPE_MUTE;
		power_step->power_off_step[0].index = 0;
		power_step->power_off_step[0].value = 1; //mute
		power_step->power_off_step[0].delay = pdrv->mute_cnt;

		power_step->power_off_step[1].type = LCD_POWER_TYPE_BACKLIGHT;
		power_step->power_off_step[1].index = 0;
		power_step->power_off_step[1].value = 0;//bl off
		power_step->power_off_step[1].delay = 0;
	}

	return 0;
}

static int lcd_vlock_param_load_from_unifykey(struct aml_lcd_drv_s *pdrv, unsigned char *buf)
{
	unsigned char *p;

	p = buf;

	pdrv->config.vlock_param[0] = LCD_VLOCK_PARAM_BIT_UPDATE;
	pdrv->config.vlock_param[1] = *(p + LCD_UKEY_VLOCK_VAL_0);
	pdrv->config.vlock_param[2] = *(p + LCD_UKEY_VLOCK_VAL_1);
	pdrv->config.vlock_param[3] = *(p + LCD_UKEY_VLOCK_VAL_2);
	pdrv->config.vlock_param[4] = *(p + LCD_UKEY_VLOCK_VAL_3);
	if (pdrv->config.vlock_param[1] ||
	    pdrv->config.vlock_param[2] ||
	    pdrv->config.vlock_param[3] ||
	    pdrv->config.vlock_param[4]) {
		LCDPR("[%d]: find vlock_attr\n", pdrv->index);
		pdrv->config.vlock_param[0] |= LCD_VLOCK_PARAM_BIT_VALID;
	}

	return 0;
}

static int lcd_optical_load_from_unifykey(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_optical_info_s *opt_info = &pdrv->config.optical;
	char key_str[15];
	unsigned char *para, *p;
	int key_len, len;
	int ret;

	memset(key_str, 0, 15);
	if (pdrv->index == 0)
		sprintf(key_str, "lcd_optical");
	else
		sprintf(key_str, "lcd%d_optical", pdrv->index);

	ret = lcd_unifykey_get_size(key_str, &key_len);
	if (ret < 0)
		return -1;
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s: find ukey: %s\n", pdrv->index, __func__, key_str);

	para = kzalloc(key_len, GFP_KERNEL);
	if (!para)
		return -1;

	ret = lcd_unifykey_get(key_str, para, key_len);
	if (ret < 0) {
		kfree(para);
		return -1;
	}

	/* check parameters */
	len = LCD_UKEY_OPTICAL_SIZE;
	ret = lcd_unifykey_len_check(key_len, len);
	if (ret < 0) {
		LCDERR("[%d]: %s: unifykey parameters length is incorrect\n",
		       pdrv->index, key_str);
		kfree(para);
		return -1;
	}

	/* attr (52Byte) */
	p = para;

	opt_info->hdr_support = (*(p + LCD_UKEY_OPT_HDR_SUPPORT) |
		((*(p + LCD_UKEY_OPT_HDR_SUPPORT + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_HDR_SUPPORT + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_HDR_SUPPORT + 3)) << 24));
	opt_info->features = (*(p + LCD_UKEY_OPT_FEATURES) |
		((*(p + LCD_UKEY_OPT_FEATURES + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_FEATURES + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_FEATURES + 3)) << 24));
	opt_info->primaries_r_x = (*(p + LCD_UKEY_OPT_PRI_R_X) |
		((*(p + LCD_UKEY_OPT_PRI_R_X + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_PRI_R_X + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_PRI_R_X + 3)) << 24));
	opt_info->primaries_r_y = (*(p + LCD_UKEY_OPT_PRI_R_Y) |
		((*(p + LCD_UKEY_OPT_PRI_R_Y + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_PRI_R_Y + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_PRI_R_Y + 3)) << 24));
	opt_info->primaries_g_x = (*(p + LCD_UKEY_OPT_PRI_G_X) |
		((*(p + LCD_UKEY_OPT_PRI_G_X + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_PRI_G_X + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_PRI_G_X + 3)) << 24));
	opt_info->primaries_g_y = (*(p + LCD_UKEY_OPT_PRI_G_Y) |
		((*(p + LCD_UKEY_OPT_PRI_G_Y + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_PRI_G_Y + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_PRI_G_Y + 3)) << 24));
	opt_info->primaries_b_x = (*(p + LCD_UKEY_OPT_PRI_B_X) |
		((*(p + LCD_UKEY_OPT_PRI_B_X + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_PRI_B_X + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_PRI_B_X + 3)) << 24));
	opt_info->primaries_b_y = (*(p + LCD_UKEY_OPT_PRI_B_Y) |
		((*(p + LCD_UKEY_OPT_PRI_B_Y + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_PRI_B_Y + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_PRI_B_Y + 3)) << 24));
	opt_info->white_point_x = (*(p + LCD_UKEY_OPT_WHITE_X) |
		((*(p + LCD_UKEY_OPT_WHITE_X + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_WHITE_X + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_WHITE_X + 3)) << 24));
	opt_info->white_point_y = (*(p + LCD_UKEY_OPT_WHITE_Y) |
		((*(p + LCD_UKEY_OPT_WHITE_Y + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_WHITE_Y + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_WHITE_Y + 3)) << 24));
	opt_info->luma_max = (*(p + LCD_UKEY_OPT_LUMA_MAX) |
		((*(p + LCD_UKEY_OPT_LUMA_MAX + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_LUMA_MAX + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_LUMA_MAX + 3)) << 24));
	opt_info->luma_min = (*(p + LCD_UKEY_OPT_LUMA_MIN) |
		((*(p + LCD_UKEY_OPT_LUMA_MIN + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_LUMA_MIN + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_LUMA_MIN + 3)) << 24));
	opt_info->luma_avg = (*(p + LCD_UKEY_OPT_LUMA_AVG) |
		((*(p + LCD_UKEY_OPT_LUMA_AVG + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_LUMA_AVG + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_LUMA_AVG + 3)) << 24));

	opt_info->ldim_support = *(p + LCD_UKEY_OPT_ADV_FLAG0);
	opt_info->luma_peak = *(unsigned int *)(p + LCD_UKEY_OPT_ADV_VAL1);

	kfree(para);

	lcd_optical_vinfo_update(pdrv);

	return 0;
}

static int lcd_config_load_from_unifykey_v2(struct aml_lcd_drv_s *pdrv,
					    unsigned char *p,
					    unsigned int key_len,
					    unsigned int offset)
{
	struct aml_lcd_unifykey_header_s *lcd_header;
	struct phy_config_s *phy_cfg = &pdrv->config.phy_cfg;
	struct phy_attr_s *phy = NULL;
	unsigned int len, size;
	unsigned char version;
	int i, ret;

	if (!phy_cfg->phys[0])
		return -1;

	lcd_header = (struct aml_lcd_unifykey_header_s *)p;
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		lcd_unifykey_header_print(p);

	/* step 2: check lcd parameters */
	len = offset + lcd_header->block_cur_size;
	ret = lcd_unifykey_len_check(key_len, len);
	if (ret < 0) {
		LCDERR("unifykey parameters length is incorrect\n");
		return -1;
	}

	/*phy 356byte*/
	phy_cfg->flag = (*(p + LCD_UKEY_PHY_ATTR_FLAG) |
		((*(p + LCD_UKEY_PHY_ATTR_FLAG + 1)) << 8) |
		((*(p + LCD_UKEY_PHY_ATTR_FLAG + 2)) << 16) |
		((*(p + LCD_UKEY_PHY_ATTR_FLAG + 3)) << 24));
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("%s: ctrl_flag=0x%x\n", __func__, phy_cfg->flag);

	phy = phy_cfg->phys[0];
	if (phy_cfg->flag & PHY_BIT_VSWING) {
		phy->vswing = (*(p + LCD_UKEY_PHY_ATTR_0) |
				*(p + LCD_UKEY_PHY_ATTR_0 + 1) << 8);
	}
	if (phy_cfg->flag & PHY_BIT_VCM) {
		phy->vcm = (*(p + LCD_UKEY_PHY_ATTR_1) |
				*(p + LCD_UKEY_PHY_ATTR_1 + 1) << 8);
	}
	if (phy_cfg->flag & PHY_BIT_REF_BIAS) {
		phy->ref_bias = (*(p + LCD_UKEY_PHY_ATTR_2) |
				*(p + LCD_UKEY_PHY_ATTR_2 + 1) << 8);
	}
	if (phy_cfg->flag & PHY_BIT_ODT) {
		phy->odt = (*(p + LCD_UKEY_PHY_ATTR_3) |
				*(p + LCD_UKEY_PHY_ATTR_3 + 1) << 8);
	}
	if (phy_cfg->flag & PHY_BIT_CV_MODE) {
		phy->cv_mode = (*(p + LCD_UKEY_PHY_ATTR_4) |
				*(p + LCD_UKEY_PHY_ATTR_4 + 1) << 8);
	}
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("%s: vswing=0x%x, vcm=0x%x, ref_bias=0x%x, odt=0x%x, cv_mode=%d\n",
		      __func__, phy->vswing, phy->vcm, phy->ref_bias,
		      phy->cv_mode, phy->odt);
	}

	if (phy_cfg->flag & PHY_BIT_LANE_PREEM) {
		for (i = 0; i < phy_cfg->lane_num; i++) {
			phy->lane[i].preem =
				*(p + LCD_UKEY_PHY_LANE_CTRL + 4 * i) |
				(*(p + LCD_UKEY_PHY_LANE_CTRL + 4 * i + 1) << 8);
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("%s: lane[%d]: preem=0x%x\n",
				      __func__, i,
				      phy->lane[i].preem);
			}
		}
	}

	if (phy_cfg->flag & PHY_BIT_LANE_AMP) {
		for (i = 0; i < phy_cfg->lane_num; i++) {
			phy->lane[i].amp =
				*(p + LCD_UKEY_PHY_LANE_CTRL + 4 * i + 2) |
				(*(p + LCD_UKEY_PHY_LANE_CTRL + 4 * i + 3) << 8);
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("%s: lane[%d]: amp=0x%x\n",
				      __func__, i,
				      phy->lane[i].amp);
			}
		}
	}

	if (phy_cfg->flag & PHY_BIT_LANE_SEL) {
		for (i = 0; i < phy_cfg->lane_num; i++) {
			phy_cfg->ch_ctrl[i].sel = *(p + LCD_UKEY_PHY_LANE_SEL + i);
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("%s: lane[%d]: sel=0x%x\n",
					__func__, i, phy_cfg->ch_ctrl[i].sel);
			}
		}
	}

	size = LCD_UKEY_CUS_CTRL_ATTR_FLAG;
	version = lcd_header->version;
	lcd_cus_ctrl_load_from_unifykey(pdrv, (p + size), (key_len - size), version);

	return 0;
}

static int lcd_config_load_from_unifykey_v3(struct aml_lcd_drv_s *pdrv,
					    unsigned char *p,
					    unsigned int key_len,
					    unsigned int offset)
{
	struct aml_lcd_unifykey_header_s *lcd_header;
	unsigned int len, size;
	unsigned char version;
	int ret;

	lcd_header = (struct aml_lcd_unifykey_header_s *)p;
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		lcd_unifykey_header_print(p);

	/* step 2: check lcd parameters */
	len = offset + lcd_header->block_cur_size;
	ret = lcd_unifykey_len_check(key_len, len);
	if (ret < 0) {
		LCDERR("unifykey parameters length is incorrect\n");
		return -1;
	}

	size = LCD_UKEY_CUS_CTRL_ATTR_FLAG_V3;
	version = lcd_header->version;
	lcd_cus_ctrl_load_from_unifykey(pdrv, (p + size), (key_len - size), version);

	return 0;
}

static int lcd_config_load_from_unifykey(struct aml_lcd_drv_s *pdrv, char *key_str)
{
	unsigned char *para;
	int key_len, len;
	unsigned char *p, val;
	const char *str;
	struct aml_lcd_unifykey_header_s *lcd_header;
	struct lcd_config_s *pconf = &pdrv->config;
	struct lcd_detail_timing_s *ptiming = NULL;
	struct phy_config_s *phy_cfg = &pdrv->config.phy_cfg;
	struct phy_attr_s *phy = NULL;
	union lcd_ctrl_config_u *pctrl = &pdrv->config.control;
	unsigned int temp, lcd_bits = 0;
	char str_info[128];
	int str_info_len = 0, ret;

	ret = lcd_unifykey_get_size(key_str, &key_len);
	if (ret)
		return -1;

	para = kzalloc(key_len, GFP_KERNEL);
	if (!para)
		return -1;

	ret = lcd_unifykey_get(key_str, para, key_len);
	if (ret < 0) {
		kfree(para);
		return -1;
	}

	/* step 1: check header size */
	lcd_header = (struct aml_lcd_unifykey_header_s *)para;
	len = LCD_UKEY_DATA_LEN_V1; /*10+36+18+31+20*/
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		lcd_unifykey_header_print(para);

	/* step 2: check lcd parameters */
	ret = lcd_unifykey_len_check(key_len, len);
	if (ret < 0) {
		LCDERR("[%d]: unifykey parameters length is incorrect\n", pdrv->index);
		goto load_from_unifykey_exit;
	}

	/* panel_type update */
	sprintf(pconf->propname, "%s", "unifykey");

	/* basic: 36byte */
	p = para;
	str = (const char *)(p + LCD_UKEY_HEAD_SIZE);
	strncpy(pconf->basic.model_name, str, MOD_LEN_MAX);
	/* ensure string ending */
	pconf->basic.model_name[MOD_LEN_MAX - 1] = '\0';
	temp = *(p + LCD_UKEY_INTERFACE);
	pconf->basic.lcd_type = temp & 0x3f;
	pconf->basic.config_check = (temp >> 6) & 0x3;
	temp = *(p + LCD_UKEY_LCD_BITS_CFMT);
	lcd_bits = (temp & 0x3f) * 3;
	pconf->basic.screen_width = (*(p + LCD_UKEY_SCREEN_WIDTH) |
		((*(p + LCD_UKEY_SCREEN_WIDTH + 1)) << 8));
	pconf->basic.screen_height = (*(p + LCD_UKEY_SCREEN_HEIGHT) |
		((*(p + LCD_UKEY_SCREEN_HEIGHT + 1)) << 8));

	/* timing: 18byte */
	ptiming = lcd_timing_alloc(pdrv);
	if (!ptiming) {
		ret = -1;
		LCDERR("%s timing alloc fail\n", __func__);
		goto load_from_unifykey_exit;
	}
	memset(ptiming, 0, sizeof(*ptiming));
	ptiming->h_active = (*(p + LCD_UKEY_H_ACTIVE) |
		((*(p + LCD_UKEY_H_ACTIVE + 1)) << 8));
	ptiming->v_active = (*(p + LCD_UKEY_V_ACTIVE)) |
		((*(p + LCD_UKEY_V_ACTIVE + 1)) << 8);
	ptiming->h_period = (*(p + LCD_UKEY_H_PERIOD)) |
		((*(p + LCD_UKEY_H_PERIOD + 1)) << 8);
	ptiming->v_period = (*(p + LCD_UKEY_V_PERIOD)) |
		((*(p + LCD_UKEY_V_PERIOD + 1)) << 8);
	temp = *(unsigned short *)(p + LCD_UKEY_HS_WIDTH_POL);
	ptiming->hsync_width = temp & 0xfff;
	ptiming->hsync_pol = (temp >> 12) & 0xf;
	ptiming->hsync_bp = (*(p + LCD_UKEY_HS_BP) |
		((*(p + LCD_UKEY_HS_BP + 1)) << 8));
	ptiming->hsync_fp = ptiming->h_period - ptiming->h_active -
			ptiming->hsync_width - ptiming->hsync_bp;
	temp = *(unsigned short *)(p + LCD_UKEY_VS_WIDTH_POL);
	ptiming->vsync_width = temp & 0xfff;
	ptiming->vsync_pol = (temp >> 12) & 0xf;
	ptiming->vsync_bp = (*(p + LCD_UKEY_VS_BP) |
		((*(p + LCD_UKEY_VS_BP + 1)) << 8));
	ptiming->vsync_fp = ptiming->v_period - ptiming->v_active -
			ptiming->vsync_width - ptiming->vsync_bp;
	pconf->timing.pre_de_h = *(p + LCD_UKEY_PRE_DE_H);
	pconf->timing.pre_de_v = *(p + LCD_UKEY_PRE_DE_V);

	/* customer: 31byte */
	ptiming->fr_adjust_type = *(p + LCD_UKEY_FR_ADJ_TYPE);
	pconf->timing.ss_level = *(p + LCD_UKEY_SS_LEVEL);
	val = *(p + LCD_UKEY_CUST_VAL0);
	pconf->timing.clk_mode = (val >> 4) & 0xf;
	pconf->timing.pll_flag = val & 0xf;
	ptiming->pixel_clk = (*(p + LCD_UKEY_PCLK) |
		((*(p + LCD_UKEY_PCLK + 1)) << 8) |
		((*(p + LCD_UKEY_PCLK + 2)) << 16) |
		((*(p + LCD_UKEY_PCLK + 3)) << 24));
	ptiming->h_period_min = (*(p + LCD_UKEY_H_PERIOD_MIN) |
		((*(p + LCD_UKEY_H_PERIOD_MIN + 1)) << 8));
	ptiming->h_period_max = (*(p + LCD_UKEY_H_PERIOD_MAX) |
		((*(p + LCD_UKEY_H_PERIOD_MAX + 1)) << 8));
	ptiming->v_period_min = (*(p + LCD_UKEY_V_PERIOD_MIN) |
		((*(p + LCD_UKEY_V_PERIOD_MIN + 1)) << 8));
	ptiming->v_period_max = (*(p + LCD_UKEY_V_PERIOD_MAX) |
		((*(p + LCD_UKEY_V_PERIOD_MAX + 1)) << 8));
	ptiming->pclk_min = (*(p + LCD_UKEY_PCLK_MIN) |
		((*(p + LCD_UKEY_PCLK_MIN + 1)) << 8) |
		((*(p + LCD_UKEY_PCLK_MIN + 2)) << 16) |
		((*(p + LCD_UKEY_PCLK_MIN + 3)) << 24));
	ptiming->pclk_max = (*(p + LCD_UKEY_PCLK_MAX) |
		((*(p + LCD_UKEY_PCLK_MAX + 1)) << 8) |
		((*(p + LCD_UKEY_PCLK_MAX + 2)) << 16) |
		((*(p + LCD_UKEY_PCLK_MAX + 3)) << 24));
	ptiming->frame_rate_min = *(p + LCD_UKEY_FRAME_RATE_MIN);
	ptiming->frame_rate_max = *(p + LCD_UKEY_FRAME_RATE_MAX);
	ptiming->lcd_bits = lcd_bits;

	val = *(p + LCD_UKEY_CUST_VAL1);
	pconf->timing.ppc = (val >> 4) & 0xf;
	pconf->custom_pinmux = val & 0xf;

	pconf->fr_auto_cus = *(p + LCD_UKEY_FR_AUTO_CUS);
	ptiming->switch_type = LCD_VMODE_SWITCH_NONE;
	ptiming->ss_force = 0;
	ptiming->ss_freq = 255;
	ptiming->ss_level = pconf->timing.ss_level;
	ptiming->ss_mode = 255;

	pconf->timing.dft_timing = pconf->timing.timings[0];
	lcd_clk_frame_rate_init(ptiming);
	lcd_config_timing_check(pdrv, ptiming);
	lcd_default_to_basic_timing_init_config(pdrv);

	str_info_len += sprintf(str_info + str_info_len, "ppc:%d, ",
			pconf->timing.ppc);
	str_info_len += sprintf(str_info + str_info_len, "clk_mode:%d, ",
			pconf->timing.clk_mode);
	if (pconf->timing.pre_de_h || pconf->timing.pre_de_v) {
		str_info_len += sprintf(str_info + str_info_len, "pre_de:%d,%d, ",
				pconf->timing.pre_de_h, pconf->timing.pre_de_v);
	}
	str_info_len += sprintf(str_info + str_info_len, "cfg_chk:0x%x, ",
			pconf->basic.config_check);
	str_info_len += sprintf(str_info + str_info_len, "cus_pinmux:%d, ",
			pconf->custom_pinmux);
	sprintf(str_info + str_info_len, "afr_cus:0x%x", pconf->fr_auto_cus);

	LCDPR("[%d]: load ukey config: %s, %s, %dbit, %dx%d, %s\n",
	      pdrv->index, pconf->basic.model_name,
	      lcd_type_type_to_str(pconf->basic.lcd_type),
	      ptiming->lcd_bits, ptiming->h_active, ptiming->v_active,
	      str_info);

	/* interface: 20byte */
	switch (pconf->basic.lcd_type) {
	case LCD_LVDS:
		pctrl->lvds_cfg.lvds_repack = *(p + LCD_UKEY_IF_ATTR_0) |
			((*(p + LCD_UKEY_IF_ATTR_0 + 1)) << 8);
		pctrl->lvds_cfg.dual_port = *(p + LCD_UKEY_IF_ATTR_1) |
			((*(p + LCD_UKEY_IF_ATTR_1 + 1)) << 8);
		pctrl->lvds_cfg.pn_swap = *(p + LCD_UKEY_IF_ATTR_2) |
			((*(p + LCD_UKEY_IF_ATTR_2 + 1)) << 8);
		pctrl->lvds_cfg.port_swap = *(p + LCD_UKEY_IF_ATTR_3) |
			((*(p + LCD_UKEY_IF_ATTR_3 + 1)) << 8);
		pctrl->lvds_cfg.phy_vswing = *(p + LCD_UKEY_IF_ATTR_4) |
			((*(p + LCD_UKEY_IF_ATTR_4 + 1)) << 8);
		pctrl->lvds_cfg.phy_preem = *(p + LCD_UKEY_IF_ATTR_5) |
			((*(p + LCD_UKEY_IF_ATTR_5 + 1)) << 8);
		pctrl->lvds_cfg.lane_reverse = *(p + LCD_UKEY_IF_ATTR_8) |
			((*(p + LCD_UKEY_IF_ATTR_8 + 1)) << 8);

		phy_cfg->vswing_level = pctrl->lvds_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->lvds_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->lvds_cfg.phy_preem;
		break;
	case LCD_VBYONE:
		pctrl->vbyone_cfg.lane_count = *(p + LCD_UKEY_IF_ATTR_0) |
			((*(p + LCD_UKEY_IF_ATTR_0 + 1)) << 8);
		pctrl->vbyone_cfg.region_num = *(p + LCD_UKEY_IF_ATTR_1) |
			((*(p + LCD_UKEY_IF_ATTR_1 + 1)) << 8);
		pctrl->vbyone_cfg.byte_mode = *(p + LCD_UKEY_IF_ATTR_2) |
			((*(p + LCD_UKEY_IF_ATTR_2 + 1)) << 8);
		pctrl->vbyone_cfg.color_fmt = *(p + LCD_UKEY_IF_ATTR_3) |
			((*(p + LCD_UKEY_IF_ATTR_3 + 1)) << 8);
		pctrl->vbyone_cfg.phy_vswing = *(p + LCD_UKEY_IF_ATTR_4) |
			((*(p + LCD_UKEY_IF_ATTR_4 + 1)) << 8);
		pctrl->vbyone_cfg.phy_preem = *(p + LCD_UKEY_IF_ATTR_5) |
			((*(p + LCD_UKEY_IF_ATTR_5 + 1)) << 8);
		pctrl->vbyone_cfg.hw_filter_time =
			*(p + LCD_UKEY_IF_ATTR_8) |
			((*(p + LCD_UKEY_IF_ATTR_8 + 1)) << 8);
		pctrl->vbyone_cfg.hw_filter_cnt =
			*(p + LCD_UKEY_IF_ATTR_9) |
			((*(p + LCD_UKEY_IF_ATTR_9 + 1)) << 8);
		pctrl->vbyone_cfg.ctrl_flag = 0;
		pctrl->vbyone_cfg.power_on_reset_delay = VX1_PWR_ON_RESET_DLY_DFT;
		pctrl->vbyone_cfg.hpd_data_delay = VX1_HPD_DATA_DELAY_DFT;
		pctrl->vbyone_cfg.cdr_training_hold = VX1_CDR_TRAINING_HOLD_DFT;
		pctrl->vbyone_cfg.slice = pdrv->config.timing.ppc ? pdrv->config.timing.ppc : 1;

		phy_cfg->vswing_level = pctrl->vbyone_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->vbyone_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->vbyone_cfg.phy_preem;
		break;
	case LCD_MLVDS:
		pctrl->mlvds_cfg.channel_num = *(p + LCD_UKEY_IF_ATTR_0) |
			((*(p + LCD_UKEY_IF_ATTR_0 + 1)) << 8);
		pctrl->mlvds_cfg.channel_sel0 = *(p + LCD_UKEY_IF_ATTR_1) |
			((*(p + LCD_UKEY_IF_ATTR_1 + 1)) << 8) |
			(*(p + LCD_UKEY_IF_ATTR_2) << 16) |
			((*(p + LCD_UKEY_IF_ATTR_2 + 1)) << 24);
		pctrl->mlvds_cfg.channel_sel1 = *(p + LCD_UKEY_IF_ATTR_3) |
			((*(p + LCD_UKEY_IF_ATTR_3 + 1)) << 8) |
			(*(p + LCD_UKEY_IF_ATTR_4) << 16) |
			((*(p + LCD_UKEY_IF_ATTR_4 + 1)) << 24);
		pctrl->mlvds_cfg.clk_phase = *(p + LCD_UKEY_IF_ATTR_5) |
			((*(p + LCD_UKEY_IF_ATTR_5 + 1)) << 8);
		pctrl->mlvds_cfg.pn_swap = *(p + LCD_UKEY_IF_ATTR_6) |
			((*(p + LCD_UKEY_IF_ATTR_6 + 1)) << 8);
		pctrl->mlvds_cfg.bit_swap = *(p + LCD_UKEY_IF_ATTR_7) |
			((*(p + LCD_UKEY_IF_ATTR_7 + 1)) << 8);
		pctrl->mlvds_cfg.phy_vswing = *(p + LCD_UKEY_IF_ATTR_8) |
			((*(p + LCD_UKEY_IF_ATTR_8 + 1)) << 8);
		pctrl->mlvds_cfg.phy_preem = *(p + LCD_UKEY_IF_ATTR_9) |
			((*(p + LCD_UKEY_IF_ATTR_9 + 1)) << 8);

		phy_cfg->vswing_level = pctrl->mlvds_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->mlvds_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->mlvds_cfg.phy_preem;
		break;
	case LCD_P2P:
		pctrl->p2p_cfg.p2p_type = *(p + LCD_UKEY_IF_ATTR_0) |
			((*(p + LCD_UKEY_IF_ATTR_0 + 1)) << 8);
		pctrl->p2p_cfg.lane_num = *(p + LCD_UKEY_IF_ATTR_1) |
			((*(p + LCD_UKEY_IF_ATTR_1 + 1)) << 8);
		pctrl->p2p_cfg.channel_sel0 = *(p + LCD_UKEY_IF_ATTR_2) |
			((*(p + LCD_UKEY_IF_ATTR_2 + 1)) << 8) |
			(*(p + LCD_UKEY_IF_ATTR_3) << 16) |
			((*(p + LCD_UKEY_IF_ATTR_3 + 1)) << 24);
		pctrl->p2p_cfg.channel_sel1 = *(p + LCD_UKEY_IF_ATTR_4) |
			((*(p + LCD_UKEY_IF_ATTR_4 + 1)) << 8) |
			(*(p + LCD_UKEY_IF_ATTR_5) << 16) |
			((*(p + LCD_UKEY_IF_ATTR_5 + 1)) << 24);
		pctrl->p2p_cfg.pn_swap = *(p + LCD_UKEY_IF_ATTR_6) |
			((*(p + LCD_UKEY_IF_ATTR_6 + 1)) << 8);
		pctrl->p2p_cfg.bit_swap = *(p + LCD_UKEY_IF_ATTR_7) |
			((*(p + LCD_UKEY_IF_ATTR_7 + 1)) << 8);
		pctrl->p2p_cfg.phy_vswing = *(p + LCD_UKEY_IF_ATTR_8) |
			((*(p + LCD_UKEY_IF_ATTR_8 + 1)) << 8);
		pctrl->p2p_cfg.phy_preem = *(p + LCD_UKEY_IF_ATTR_9) |
			((*(p + LCD_UKEY_IF_ATTR_9 + 1)) << 8);

		phy_cfg->vswing_level = pctrl->p2p_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->p2p_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->p2p_cfg.phy_preem;
		break;
	default:
		LCDERR("[%d]: unsupport lcd_type: %d\n",
		       pdrv->index, pconf->basic.lcd_type);
		break;
	}
	phy = lcd_phy_alloc(pdrv);// first parse, it is only be phy_cfg->phys[0];
	if (!phy) {
		ret = -1;
		LCDERR("%s phy alloc fail\n", __func__);
		goto load_from_unifykey_exit;
	}
	memset(phy, 0, sizeof(*phy));
	phy_cfg->act_phy = phy_cfg->phys[0];
	lcd_phy_param_preset(pdrv);
	lcd_lane_map_preset(pdrv);
	phy->ss.freq = 255;
	phy->ss.level = 255;
	phy->ss.mode = 255;

	lcd_vlock_param_load_from_unifykey(pdrv, para);

	/* step 3: check power sequence */
	ret = lcd_power_load_from_unifykey(pdrv, para, key_len, len);
	if (ret < 0) {
		LCDERR("%s power parse fail\n", __func__);
		goto load_from_unifykey_exit;
	}

	p = para + lcd_header->block_cur_size;
	switch (lcd_header->version) {
	case 2:
		lcd_config_load_from_unifykey_v2(pdrv, p, key_len, lcd_header->block_cur_size);
		break;
	case 3:
		lcd_config_load_from_unifykey_v3(pdrv, p, key_len, lcd_header->block_cur_size);
		break;
	default:
		break;
	}

	//fix ss in detail timing and phy_attr if not config
	lcd_ss_config_fix(pdrv);

	lcd_optical_load_from_unifykey(pdrv);

#ifdef CONFIG_AMLOGIC_BACKLIGHT
	lcd_resource_add(pdrv, LCD_RES_BACKLIGHT, 0);
	aml_bl_index_add(pdrv->index, 0);
#endif

load_from_unifykey_exit:
	kfree(para);

	return ret;
}

static int lcd_config_load_init(struct aml_lcd_drv_s *pdrv)
{
	if (pdrv->config.basic.config_check & 0x2)
		pdrv->config_check_en = pdrv->config.basic.config_check & 0x1;
	else
		pdrv->config_check_en = pdrv->config_check_glb;

	switch (pdrv->config.fr_auto_cus) {
	case 0: //follow global fr_auto_policy
		pdrv->config.fr_auto_flag = pdrv->fr_auto_policy;
		break;
	case 0xff: //disable fr_auto
		pdrv->config.fr_auto_flag = 0xff;
		break;
	default: //custom fr_auto
		pdrv->config.fr_auto_flag = pdrv->config.fr_auto_cus;
		break;
	}

	if (pdrv->index)
		pdrv->config.timing.ppc = 1;

	if (pdrv->status & LCD_STATUS_ENCL_ON)
		lcd_clk_gate_switch(pdrv, 1);

	return 0;
}

int lcd_get_config(struct aml_lcd_drv_s *pdrv)
{
	char key_str[10];
	int load_id = 0;
	int ret;

	memset(key_str, 0, 10);
	if (pdrv->index == 0)
		sprintf(key_str, "lcd");
	else
		sprintf(key_str, "lcd%d", pdrv->index);

	if (pdrv->key_valid) {
		ret = lcd_unifykey_check(key_str);
		if (ret < 0) {
			load_id = 0;
			LCDERR("[%d]: %s: can't find key %s\n",
			       pdrv->index, __func__, key_str);
		} else {
			load_id = 1;
		}
	}
	pdrv->config_load = load_id;
	if (load_id)
		ret = lcd_config_load_from_unifykey(pdrv, key_str);
	else
		ret = lcd_config_load_from_dts(pdrv);
	if (ret)
		return -1;

	lcd_lane_map_update(pdrv);

	lcd_config_load_init(pdrv);
	lcd_config_load_print(pdrv);

	lcd_tcon_probe(pdrv);

	return 0;
}
