// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/init.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/of.h>
#include <linux/reset.h>
#include <linux/clk.h>
#include <linux/sched/clock.h>
#ifdef CONFIG_AMLOGIC_VPU
#include <linux/amlogic/media/vpu/vpu.h>
#endif
#include <linux/amlogic/media/vout/vinfo.h>
#include <linux/amlogic/media/vout/vout_notify.h>
#include <linux/amlogic/media/vout/lcd/lcd_vout.h>
#include <linux/amlogic/media/vout/lcd/lcd_notify.h>
#include <drm/amlogic/meson_connector_dev.h>
#ifdef CONFIG_AMLOGIC_BACKLIGHT
#include <linux/amlogic/media/vout/lcd/aml_bl.h>
#endif
#include "./lcd_reg.h"
#include "./lcd_common.h"
#include <linux/sched/clock.h>
#include "./connectors/lcd_connector.h"

static int lcd_tablet_outputmode_check(struct aml_lcd_drv_s *pdrv, char *mode)
{
	struct lcd_vmode_list_s *temp_list = pdrv->vmode_mgr.vmode_list_header;
	char mode_name[48];
	int i;

	if (!mode || !temp_list)
		return -1;

	while (temp_list) {
		memset(mode_name, 0, 48);
		str_add_vmode(mode_name, 0,
			temp_list->info->width, temp_list->info->height, temp_list->info->base_fr);
		if (strcmp(mode, mode_name) == 0) {
			temp_list->info->duration_index = 0xff;
			if (pdrv->vmode_mgr.cur_vmode_info != temp_list->info)
				pdrv->vmode_mgr.next_vmode_info = temp_list->info;

			LCD_DBG(pdrv, "%s: match %s, %dx%d@%dhz", __func__,
				temp_list->info->name,
				temp_list->info->width, temp_list->info->height,
				temp_list->info->base_fr);
			return 0;
		}

		for (i = 0; i < LCD_DURATION_MAX; i++) {
			if (temp_list->info->duration[i].frame_rate == 0)
				break;
			memset(mode_name, 0, 48);
			str_add_vmode(mode_name, 0, temp_list->info->width,
				temp_list->info->height, temp_list->info->duration[i].frame_rate);

			if (strcmp(mode, mode_name))
				continue;

			LCD_DBG(pdrv, "%s: match %s, %dx%d@%dhz", __func__,
				temp_list->info->name,
				temp_list->info->width, temp_list->info->height,
				temp_list->info->duration[i].frame_rate);
			temp_list->info->duration_index = i;
			if (pdrv->vmode_mgr.cur_vmode_info != temp_list->info)
				pdrv->vmode_mgr.next_vmode_info = temp_list->info;
			return 0;
		}
		temp_list = temp_list->next;
	}

	LCD_ERR(pdrv, "%s: invalid mode: %s", __func__, mode);
	return -1;
}

// * update pdrv->output_name, *cur_vmode_info, act_timing==>vinfo
static void lcd_vmode_vinfo_update(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_vmode_info_s *info;
	unsigned short frame_rate;

	if (!pdrv)
		return;

	info = pdrv->vmode_mgr.cur_vmode_info;
	if (!info)
		return;

	memset(pdrv->output_name, 0, sizeof(pdrv->output_name));

	frame_rate = info->duration_index == 0xff ?
		info->base_fr : info->duration[info->duration_index].frame_rate;

	str_add_vmode(pdrv->output_name, 0, info->width, info->height, frame_rate);

	LCD_DBG(pdrv, "%s: outputmode = %s", __func__, pdrv->output_name);

	// lcd_act_timing_update_vinfo(pdrv);
	// lcd_optical_vinfo_update(pdrv);
	lcd_act_timing_update_to_vinfo(pdrv);
}

int lcd_mode_tablet_set_current_vmode(enum vmode_e mode, void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;
	unsigned long long local_time[2];
	int ret = 0;

	if (!pdrv)
		return -1;

	LCD_DBG(pdrv, "%s: mode=0x%x", __func__, mode);
	if (lcd_vout_serve_bypass) {
		LCD_PR(pdrv, "vout_serve bypass");
		return 0;
	}

	if ((mode & VMODE_MODE_BIT_MASK) != VMODE_LCD) {
		LCD_PR(pdrv, "unsupport mode 0x%x", mode & VMODE_MODE_BIT_MASK);
		return -1;
	}

	mutex_lock(&lcd_power_mutex);

	lcd_proc_time_clear(pdrv);
	local_time[0] = sched_clock();
	pdrv->proc_time.switch_start_time = local_time[0];
	/* clear fr*/
	pdrv->fr_duration = 0;
	pdrv->fr_mode = 0;

	mutex_lock(&lcd_vout_mutex);
	lcd_timing_update_vmode(pdrv);
	lcd_vmode_vinfo_update(pdrv);
	mutex_unlock(&lcd_vout_mutex);

	lcd_vrr_dev_register(pdrv);
	if (mode & VMODE_INIT_BIT_MASK) { // first boot: set clktree
		lcd_clk_gate_switch(pdrv, 1);
	} else if (pdrv->status & LCD_STATUS_ENCL_ON) { // mode change by vout
		if (pdrv->vmode_switch) {
			lcd_mode_vmode_switch(pdrv);
		} else {
			mutex_lock(&lcd_vout_mutex);
			lcd_connector_driver_change(pdrv);
			mutex_unlock(&lcd_vout_mutex);
		}
	} else if (!(pdrv->status & LCD_STATUS_ENCL_ON)) {
		// mode change by drm (disable + mode_change)
		aml_lcd_notifier_call_chain(LCD_EVENT_ENABLE, (void *)pdrv);
		pdrv->status |= LCD_EVENT_ENABLE;
	}

	if (pdrv->vmode_switch == 0) {
		local_time[1] = sched_clock();
		pdrv->proc_time.switch_full_time = local_time[1] - local_time[0];
	}

	/* must update vrr dev after driver change for panel parameters update */
	lcd_vrr_dev_update(pdrv);

	pdrv->status |= LCD_STATE_VMODE_ACTIVE;

	mutex_unlock(&lcd_power_mutex);
	return ret;
}

int lcd_mode_tablet_vout_disable(enum vmode_e cur_vmod, void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;

	if (!pdrv)
		return -1;

	mutex_lock(&lcd_power_mutex);
	lcd_vrr_dev_unregister(pdrv);

	pdrv->init_flag = 0;
	pdrv->status &= ~(LCD_STATE_VMODE_ACTIVE | LCD_STATE_PREPARE | LCD_STATE_POWER);
	aml_lcd_notifier_call_chain(LCD_EVENT_DISABLE, (void *)pdrv);
	LCD_PR(pdrv, "%s finished", __func__);
	mutex_unlock(&lcd_power_mutex);

	return 0;
}

int lcd_mode_tablet_vout_get_disp_cap(char *buf, void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;
	struct lcd_vmode_list_s *temp_list;
	unsigned int frame_rate;
	int i, ret = 0;

	if (!pdrv)
		return 0;
	mutex_lock(&lcd_vout_mutex);
	temp_list = pdrv->vmode_mgr.vmode_list_header;
	while (temp_list) {
		if (!temp_list->info)
			continue;
		ret += str_add_vmode(buf + ret, 1, temp_list->info->width,
			temp_list->info->height, temp_list->info->base_fr);

		for (i = 0; i < LCD_DURATION_MAX; i++) {
			frame_rate = temp_list->info->duration[i].frame_rate;
			if (frame_rate == 0)
				break;
			ret += str_add_vmode(buf + ret, 1, temp_list->info->width,
				temp_list->info->height, frame_rate);
		}
		temp_list = temp_list->next;
	}
	mutex_unlock(&lcd_vout_mutex);

	return ret;
}

enum vmode_e lcd_mode_tablet_validate_vmode(char *mode, unsigned int frac, void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;

	if (!pdrv || !mode)
		return VMODE_MAX;

	LCD_DBG(pdrv, "%s: display_mode=%s, drv_mode=%s",  __func__, mode, pdrv->vinfo.name);

	if (lcd_tablet_outputmode_check(pdrv, mode))
		return VMODE_MAX;

	return VMODE_LCD;
}

static void lcd_vmode_init(struct aml_lcd_drv_s *pdrv)
{
	char *mode, *init_mode;
	enum vmode_e vmode;

	init_mode = get_vout_mode_uboot();
	mode = kstrdup(init_mode, GFP_KERNEL);
	if (!mode)
		return;

	// lcd_tablet_add_all_vmode(pdrv); // !temp remove
	LCD_PR(pdrv, "%s: mode: %s", __func__, mode);
	vmode = lcd_mode_tablet_validate_vmode(mode, 0, (void *)pdrv);

	mutex_lock(&lcd_vout_mutex);
	if (vmode == VMODE_LCD)
		lcd_timing_update_vmode(pdrv);
	pdrv->vmode_switch = 0;
	lcd_vmode_vinfo_update(pdrv);
	mutex_unlock(&lcd_vout_mutex);

	kfree(mode);
}

void lcd_tablet_config_init(struct aml_lcd_drv_s *pdrv)
{
	lcd_base_to_act_timing_init_config(pdrv);
	lcd_clk_config_parameter_init(pdrv);
	lcd_clk_generate_parameter(pdrv);
	pdrv->curr_dev->dev_cfg.timing.clk_change = 0; /* clear clk_change flag */

	lcd_vmode_init(pdrv);
	lcd_mode_common_dft_timing_update_vinfo(pdrv);

#ifdef CONFIG_AMLOGIC_LCD_MIPI_DSI
	if (pdrv->curr_dev->dev_cfg.basic.lcd_type == LCD_MIPI) {
		lcd_dsi_if_bind(pdrv);
		lcd_dsi_post_config_load(pdrv);
	}
#endif
}
