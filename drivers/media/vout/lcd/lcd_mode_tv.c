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
#include <linux/amlogic/media/vrr/vrr.h>
#include <linux/amlogic/media/vout/lcd/lcd_vout.h>
#include <linux/amlogic/media/vout/lcd/lcd_notify.h>
#include <drm/amlogic/meson_connector_dev.h>
#ifdef CONFIG_AMLOGIC_BACKLIGHT
#include <linux/amlogic/media/vout/lcd/aml_bl.h>
#endif
#include "./lcd_reg.h"
#include "./lcd_common.h"
#include "./connectors/lcd_connector.h"

static int lcd_outputmode_is_matched(struct aml_lcd_drv_s *pdrv, const char *mode)
{
	struct lcd_vmode_list_s *temp_list;
	char mode_name[48];
	int i;

	LCD_DBG(pdrv, "%s: outputmode=%s", __func__, mode);

	temp_list = pdrv->vmode_mgr.vmode_list_header;
	while (temp_list) {
		for (i = 0; i < LCD_DURATION_MAX; i++) {
			if (temp_list->info->duration[i].frame_rate == 0)
				break;
			memset(mode_name, 0, 48);
			sprintf(mode_name, "%s%dhz", temp_list->info->name,
					temp_list->info->duration[i].frame_rate);
			LCD_DBG(pdrv, "%s: %s", __func__, mode_name);
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

/* ************************************************** *
 * vout server api
 * **************************************************
 */
enum vmode_e lcd_mode_tv_validate_vmode(char *mode, unsigned int frac, void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;
	int ret;

	if (!pdrv)
		return -1;

	if (lcd_vout_serve_bypass) {
		LCD_PR(pdrv, "vout_serve bypass");
		return VMODE_MAX;
	}
	if (!mode)
		return VMODE_MAX;

	mutex_lock(&lcd_vout_mutex);
	ret = lcd_outputmode_is_matched(pdrv, mode);
	if (ret == 0) {
		mutex_unlock(&lcd_vout_mutex);
		return VMODE_LCD;
	}

	mutex_unlock(&lcd_vout_mutex);
	return VMODE_MAX;
}

int lcd_mode_tv_set_current_vmode(enum vmode_e mode, void *data)
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

	if (lcd_fr_is_fixed(pdrv)) {
		if ((pdrv->status & LCD_STATUS_ENCL_ON) == 0) {
			//workaround for drm resume
			aml_lcd_notifier_call_chain(LCD_EVENT_PREPARE, (void *)pdrv);
			pdrv->status |= LCD_STATE_PREPARE;

			lcd_power_if_early_on(pdrv);
		}
		pdrv->status |= LCD_STATE_VMODE_ACTIVE;
		LCD_PR(pdrv, "fixed timing, exit vmode change");
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
	lcd_act_timing_update_to_vinfo(pdrv);
	mutex_unlock(&lcd_vout_mutex);

	if (mode & VMODE_INIT_BIT_MASK) {
		lcd_clk_gate_switch(pdrv, 1);
	} else {
		if ((pdrv->status & LCD_STATUS_ENCL_ON) == 0) {
			//workaround for drm resume
			aml_lcd_notifier_call_chain(LCD_EVENT_PREPARE, (void *)pdrv);
			pdrv->status |= LCD_STATE_PREPARE;
		} else {
			if (pdrv->vmode_switch) {
				lcd_mode_vmode_switch(pdrv);
			} else {
				mutex_lock(&lcd_vout_mutex);
				lcd_connector_driver_change(pdrv);
				mutex_unlock(&lcd_vout_mutex);
			}
		}
	}

	if (pdrv->vmode_switch == 0) {
		local_time[1] = sched_clock();
		pdrv->proc_time.switch_full_time = lcd_do_div(local_time[1] - local_time[0], 1000);
	}

	/* must update vrr dev after driver change for panel parameters update */
	lcd_vrr_dev_update(pdrv);

	pdrv->status |= LCD_STATE_VMODE_ACTIVE;

	mutex_unlock(&lcd_power_mutex);

	return ret;
}

int lcd_mode_tv_vout_disable(enum vmode_e cur_vmod, void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;

	if (!pdrv)
		return -1;

	mutex_lock(&lcd_vout_mutex);
	pdrv->status &= ~LCD_STATE_VMODE_ACTIVE;
	mutex_unlock(&lcd_vout_mutex);

	return 0;
}

int lcd_mode_tv_vout_get_disp_cap(char *buf, void *data)
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
		for (i = 0; i < LCD_DURATION_MAX; i++) {
			frame_rate = temp_list->info->duration[i].frame_rate;
			if (frame_rate == 0)
				break;
			ret += sprintf(buf + ret, "%s%dhz\n",
				temp_list->info->name, frame_rate);
		}
		temp_list = temp_list->next;
	}
	mutex_unlock(&lcd_vout_mutex);

	return ret;
}

static void lcd_vmode_init(struct aml_lcd_drv_s *pdrv)
{
	char *mode, *init_mode;
	enum vmode_e vmode;

	init_mode = get_vout_mode_uboot();
	mode = kstrdup(init_mode, GFP_KERNEL);
	if (!mode)
		return;

	LCD_PR(pdrv, "%s: mode: %s", __func__, mode);
	vmode = lcd_mode_tv_validate_vmode(mode, 0, (void *)pdrv);
	mutex_lock(&lcd_vout_mutex);
	if (vmode == VMODE_LCD)
		lcd_timing_update_vmode(pdrv);
	lcd_act_timing_update_to_vinfo(pdrv);
	pdrv->vmode_switch = 0;
	mutex_unlock(&lcd_vout_mutex);

	kfree(mode);
}

void lcd_tv_config_init(struct aml_lcd_drv_s *pdrv)
{
	lcd_base_to_act_timing_init_config(pdrv);
	lcd_clk_config_parameter_init(pdrv);

	lcd_vmode_init(pdrv);
	lcd_frame_rate_change(pdrv);

	lcd_clk_generate_parameter(pdrv);
	pdrv->curr_dev->dev_cfg.timing.clk_change = 0; /* clear clk_change flag */
}
