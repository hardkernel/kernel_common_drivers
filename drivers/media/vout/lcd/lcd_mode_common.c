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

int lcd_mode_get_vframe_rate_hint(void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;

	if (!pdrv)
		return 0;

	return pdrv->fr_duration;
}

void lcd_mode_vout_debug_test(unsigned int num, void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;

	if (!pdrv)
		return;

	lcd_debug_test(pdrv, num);
}

void lcd_mode_vout_set_bl_brightness(unsigned int brightness, void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;
#ifdef CONFIG_AMLOGIC_BACKLIGHT
	struct aml_bl_drv_s *bdrv;
#endif

	if (!pdrv)
		return;

#ifdef CONFIG_AMLOGIC_BACKLIGHT
	bdrv = aml_bl_get_driver(pdrv->index);
	aml_bl_set_level_brightness(bdrv, brightness);
#endif
}

unsigned int lcd_mode_vout_get_bl_brightness(void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;
#ifdef CONFIG_AMLOGIC_BACKLIGHT
	struct aml_bl_drv_s *bdrv;
#endif
	unsigned int ret = 0;

	if (!pdrv)
		return 0;

#ifdef CONFIG_AMLOGIC_BACKLIGHT
	bdrv = aml_bl_get_driver(pdrv->index);
	ret = aml_bl_get_level_brightness(bdrv);
#endif
	return ret;
}

int lcd_mode_suspend(void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;

	if (!pdrv)
		return -1;

	mutex_lock(&lcd_power_mutex);
	lcd_proc_time_clear(pdrv);
	pdrv->init_flag = 0;
	pdrv->status &= ~LCD_STATE_POWER;
	pdrv->status |= LCD_STATE_DUMMY;
	aml_lcd_notifier_call_chain(LCD_EVENT_POWER_OFF | LCD_EVENT_ENCL_DUMMY, (void *)pdrv);
	LCD_PR(pdrv, "early_suspend finished, status=0x%x", pdrv->status);
	mutex_unlock(&lcd_power_mutex);
	return 0;
}

int lcd_mode_resume(void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;

	if (!pdrv)
		return -1;

	if ((pdrv->status & LCD_STATE_VMODE_ACTIVE) == 0)
		return 0;

	if (pdrv->resume_type & (1 << 0)) {
		lcd_queue_work(&pdrv->late_resume_work);
	} else {
		LCD_PR(pdrv, "directly lcd late_resume, status=0x%x", pdrv->status);
		lcd_late_resume(pdrv);
	}

	return 0;
}

struct vinfo_s *lcd_mode_get_current_info(void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;

	if (!pdrv)
		return NULL;
	return &pdrv->vinfo;
}

int lcd_mode_check_same_vmodeattr(char *mode, void *data)
{
	return 1;
}

int lcd_mode_vmode_is_supported(enum vmode_e mode, void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;

	mode &= VMODE_MODE_BIT_MASK;
	LCD_DBG(pdrv, "%s vmode = %d, lcd_vmode = %d", __func__, mode, VMODE_LCD);

	if (mode == VMODE_LCD)
		return true;

	return false;
}

int lcd_mode_vout_set_state(int index, void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;

	if (!pdrv)
		return -1;

	mutex_lock(&lcd_vout_mutex);
	pdrv->vout_state |= (1 << index);
	LCD_DBG(pdrv, "%s: viu:[%d -> %d]", __func__, pdrv->viu_sel, index);
	pdrv->viu_sel = index;
	mutex_unlock(&lcd_vout_mutex);

	return 0;
}

int lcd_mode_vout_clr_state(int index, void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;

	if (!pdrv)
		return -1;

	mutex_lock(&lcd_vout_mutex);
	pdrv->vout_state &= ~(1 << index);
	LCD_PR(pdrv, "%s: viu=%d, clear_viu=%d", __func__, pdrv->viu_sel, index);
	if (pdrv->viu_sel == index)
		pdrv->viu_sel = LCD_VIU_SEL_NONE;
	mutex_unlock(&lcd_vout_mutex);

	return 0;
}

int lcd_mode_vout_get_state(void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;

	return pdrv->vout_state;
}

int lcd_vmode_remove_all(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_vmode_list_s *cur_list;
	struct lcd_vmode_list_s *next_list;

	cur_list = pdrv->vmode_mgr.vmode_list_header;
	while (cur_list) {
		next_list = cur_list->next;
		kfree(cur_list->info);
		kfree(cur_list);
		cur_list = next_list;
	}
	pdrv->vmode_mgr.vmode_list_header = NULL;
	pdrv->vmode_mgr.cur_vmode_info = NULL;
	pdrv->vmode_mgr.next_vmode_info = NULL;
	pdrv->vmode_mgr.vmode_cnt = 0;

	return 0;
}

int lcd_mode_timing_switch_update(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_detail_timing_s *ptiming;
	unsigned int pre_pclk;
	unsigned char switch_type, switch_type_pre;

	if (!pdrv->curr_dev || !pdrv->curr_dev->dev_cfg.timing.base_timing)
		return -1;
	switch_type_pre = pdrv->curr_dev->dev_cfg.timing.act_timing.switch_type;
	/* clear clk_change flag */
	pdrv->curr_dev->dev_cfg.timing.clk_change &= ~(LCD_CLK_PLL_RESET);
	if (pdrv->vmode_mgr.next_vmode_info) {
		pre_pclk = pdrv->curr_dev->dev_cfg.timing.base_timing->pixel_clk;
		pdrv->vmode_mgr.cur_vmode_info = pdrv->vmode_mgr.next_vmode_info;
		pdrv->vmode_mgr.next_vmode_info = NULL;

		pdrv->std_duration = pdrv->vmode_mgr.cur_vmode_info->duration;
		ptiming = pdrv->vmode_mgr.cur_vmode_info->dft_timing;
		pdrv->curr_dev->dev_cfg.timing.base_timing = ptiming;

		lcd_base_to_act_timing_init_config(pdrv);
		if (pdrv->curr_dev->dev_cfg.timing.base_timing->pixel_clk != pre_pclk) {
			pdrv->curr_dev->dev_cfg.timing.clk_change |=
				(LCD_CLK_PLL_RESET | LCD_CLK_PLL_CHANGE);
			lcd_clk_generate_parameter(pdrv);
		}
		pdrv->vmode_switch = 1;
	}

	switch_type = pdrv->curr_dev->dev_cfg.timing.act_timing.switch_type;
	switch (switch_type_pre) {
	case LCD_VMODE_SWITCH_FULL:
		pdrv->curr_dev->dev_cfg.timing.switch_type = switch_type_pre;
		break;
	case LCD_VMODE_SWITCH_LIMIT:
		if (switch_type == LCD_VMODE_SWITCH_FULL)
			pdrv->curr_dev->dev_cfg.timing.switch_type = LCD_VMODE_SWITCH_FULL;
		else
			pdrv->curr_dev->dev_cfg.timing.switch_type = switch_type_pre;
		break;
	case LCD_VMODE_SWITCH_MIN:
	case LCD_VMODE_SWITCH_MIN_WO_TCON_RST:
		if (switch_type == LCD_VMODE_SWITCH_FULL)
			pdrv->curr_dev->dev_cfg.timing.switch_type = LCD_VMODE_SWITCH_FULL;
		else if (switch_type == LCD_VMODE_SWITCH_LIMIT)
			pdrv->curr_dev->dev_cfg.timing.switch_type = LCD_VMODE_SWITCH_LIMIT;
		else
			pdrv->curr_dev->dev_cfg.timing.switch_type = switch_type_pre;
		break;
	default:
		pdrv->curr_dev->dev_cfg.timing.switch_type = switch_type;
		break;
	}
	LCD_DBG(pdrv, "%s: switch_type: pre: %d, cur: %d, final: %d, lcd_status: 0x%x",
		__func__, switch_type_pre, switch_type,
		pdrv->curr_dev->dev_cfg.timing.switch_type, pdrv->status);

	if (pdrv->curr_dev->dev_cfg.timing.switch_type_dbg) {
		pdrv->curr_dev->dev_cfg.timing.switch_type =
			pdrv->curr_dev->dev_cfg.timing.switch_type_dbg;
		LCD_PR(pdrv, "%s: switch_type: force dbg to final: %d",
			__func__, pdrv->curr_dev->dev_cfg.timing.switch_type);
	}

	switch (pdrv->curr_dev->dev_cfg.timing.switch_type) {
	case LCD_VMODE_SWITCH_MIN:
	case LCD_VMODE_SWITCH_MIN_WO_TCON_RST:
		pdrv->switch_off_event = LCD_EVENT_MDSW_MIN_OFF;
		pdrv->switch_on_event = LCD_EVENT_MDSW_MIN_ON;
		break;
	case LCD_VMODE_SWITCH_LIMIT:
		pdrv->switch_off_event = LCD_EVENT_MDSW_LIMIT_OFF;
		pdrv->switch_on_event = LCD_EVENT_MDSW_LIMIT_ON;
		break;
	case LCD_VMODE_SWITCH_FULL:
	default:
		pdrv->switch_off_event = LCD_EVENT_POWER_OFF;
		pdrv->switch_on_event = LCD_EVENT_POWER_ON;
		break;
	}

	return 0;
}

void lcd_mode_vmode_switch(struct aml_lcd_drv_s *pdrv)
{
	unsigned long long local_time[2];

	if (pdrv->curr_dev->dev_cfg.timing.switch_type == LCD_VMODE_SWITCH_NONE)
		return;

	//step 1: switch off
	local_time[0] = sched_clock();
	if (pdrv->status & LCD_STATE_POWER) {
		/* include lcd_vout_mutex */
		aml_lcd_notifier_call_chain(pdrv->switch_off_event, (void *)pdrv);
	}
	local_time[1] = sched_clock();
	pdrv->proc_time.switch_off_time = local_time[1] - local_time[0];

	//step 2: driver change
	if (pdrv->status & LCD_STATE_PREPARE) {
		mutex_lock(&lcd_vout_mutex);
		lcd_connector_driver_change(pdrv);
		mutex_unlock(&lcd_vout_mutex);
	}

	//step 3: switch on
	lcd_queue_work(&pdrv->mode_switch_on_work);
}

int lcd_mode_framerate_automation_set_mode(struct aml_lcd_drv_s *pdrv)
{
	unsigned int clk_change = 0;

	if (!pdrv)
		return -1;

	LCD_DBG(pdrv, "%s\n", __func__);
	lcd_vout_notify_mode_change_pre(pdrv);

	lcd_frame_rate_change(pdrv);

	clk_change = pdrv->curr_dev->dev_cfg.timing.clk_change;
	if (clk_change & LCD_CLK_CHANGE) {
		if (pdrv->status & LCD_STATUS_IF_ON) {
#ifdef CONFIG_AMLOGIC_LCD_VBYONE
			if (pdrv->curr_dev->dev_cfg.basic.lcd_type == LCD_VBYONE)
				lcd_vbyone_interrupt_enable(pdrv, 0);
#endif
		}
		lcd_clk_change(pdrv);
	}

	lcd_venc_change(pdrv);

	if (pdrv->status & LCD_STATUS_IF_ON) {
#ifdef CONFIG_AMLOGIC_LCD_VBYONE
		if (pdrv->curr_dev->dev_cfg.basic.lcd_type == LCD_VBYONE) {
			if (clk_change & LCD_CLK_PLL_CHANGE)
				lcd_vbyone_wait_stable(pdrv);
			if (clk_change & LCD_CLK_CHANGE)
				lcd_vbyone_interrupt_enable(pdrv, 1);
		}
#endif
	}

	lcd_vout_notify_mode_change(pdrv);

	return 0;
}
