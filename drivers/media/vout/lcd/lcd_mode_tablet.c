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

static struct lcd_duration_s lcd_std_fr[] = {
	{288, 288,    1,    0},
	{240, 240,    1,    0},
	{239, 240000, 1001, 1},
	{200, 200,    1,    0},
	{192, 192,    1,    0},
	{191, 192000, 1001, 1},
	{165, 165,    1,    0},
	{144, 144,    1,    0},
	{120, 120,    1,    0},
	{119, 120000, 1001, 1},
	{100, 100,    1,    0},
	{96,  96,     1,    0},
	{95,  96000,  1001, 1},
	{60,  60,     1,    0},
	{59,  60000,  1001, 1},
	{50,  50,     1,    0},
	{48,  48,     1,    0},
	{47,  48000,  1001, 1},
	{0,   0,      0,    0}
};

// * append to pdrv->vmode_mgr.vmode_list_header
static int lcd_vmode_add_single(struct aml_lcd_drv_s *pdrv, struct lcd_vmode_info_s *vmode_info)
{
	struct lcd_vmode_list_s *temp_list;
	struct lcd_vmode_list_s *cur_list;

	if (!vmode_info)
		return -1;

	/* create list */
	cur_list = kzalloc(sizeof(*cur_list), GFP_KERNEL);
	if (!cur_list)
		return -1;
	cur_list->info = vmode_info;

	if (!pdrv->vmode_mgr.vmode_list_header) {
		pdrv->vmode_mgr.vmode_list_header = cur_list;
	} else {
		temp_list = pdrv->vmode_mgr.vmode_list_header;
		while (temp_list->next)
			temp_list = temp_list->next;
		temp_list->next = cur_list;
	}
	pdrv->vmode_mgr.vmode_cnt += vmode_info->duration_cnt + 1;

	LCD_PR(pdrv, "%s: name:%s, base_fr:%dhz, vmode_cnt: %d", __func__,
		cur_list->info->name, cur_list->info->base_fr, pdrv->vmode_mgr.vmode_cnt);

	return 0;
}

static void lcd_vmode_add_duration(struct lcd_vmode_info_s *vmode, struct lcd_detail_timing_s *dtm)
{
	unsigned int n = 0, i;//, fr, ht, vt;
	unsigned short fr_min, fr_max;

	fr_min = div_around(div_around(dtm->pclk_min, dtm->v_period_max), dtm->h_period_max);
	fr_max = div_around(div_around(dtm->pclk_max, dtm->v_period_min), dtm->h_period_min);

	memset(vmode->duration, 0, sizeof(struct lcd_duration_s) * LCD_DURATION_MAX);

	//if (dtm->fixed_type != 0xff) {
	//	for (i = 0; i < 8; i++) {
	//		if (dtm->fixed_val_set[i] == 0)
	//			break;
	//		if (n >= LCD_DURATION_MAX)
	//			break;
	//		fr = (dtm->fixed_type == 0) ? dtm->fixed_val_set[i] : dtm->pixel_clk;
	//		ht = (dtm->fixed_type == 1) ? dtm->fixed_val_set[i] : dtm->h_period;
	//		vt = (dtm->fixed_type == 2) ? dtm->fixed_val_set[i] : dtm->v_period;
	//		vmode->duration[n].frame_rate = (fr / ht) / vt;
	//		vmode->duration[n].duration_num = fr;
	//		vmode->duration[n].duration_den = ht * vt;
	//		vmode->duration[n].frac = 1;
	//		n++;
	//	}
	//}

	for (i = 0; i < ARRAY_SIZE(lcd_std_fr); i++) {
		if (dtm->fr_adjust_type == 0xff)
			break;
		if (lcd_std_fr[i].frame_rate == 0)
			break;
		if (lcd_std_fr[i].frame_rate > vmode->dft_timing->frame_rate_max)
			continue;
		if (lcd_std_fr[i].frame_rate < vmode->dft_timing->frame_rate_min)
			break;
		if (vmode->base_fr * lcd_std_fr[i].duration_den == lcd_std_fr[i].duration_num)
			continue;

		vmode->duration[n].frame_rate = lcd_std_fr[i].frame_rate;
		vmode->duration[n].duration_num = lcd_std_fr[i].duration_num;
		vmode->duration[n].duration_den = lcd_std_fr[i].duration_den;
		vmode->duration[n].frac = lcd_std_fr[i].frac;
		n++;
		if (n >= LCD_DURATION_MAX)
			break;
	}
	vmode->duration_cnt = n;
}

//--general mode: (h)x(v)p(frame_rate)hz
static struct lcd_vmode_info_s *lcd_detail_timing_to_vmode(struct lcd_detail_timing_s *ptiming)
{
	struct lcd_vmode_info_s *vmode_find = NULL;

	vmode_find = kzalloc(sizeof(*vmode_find), GFP_KERNEL);
	if (!vmode_find)
		return NULL;
	vmode_find->width = ptiming->h_active;
	vmode_find->height = ptiming->v_active;
	vmode_find->base_fr = ptiming->frame_rate;

	str_add_vmode(vmode_find->name, 0, vmode_find->width, vmode_find->height, 0);

	vmode_find->dft_timing = ptiming;
	lcd_vmode_add_duration(vmode_find, ptiming);

	return vmode_find;
}

//clear all vmode (lcd_vmode_remove_all)
//dft_timing to vmode (lcd_detail_timing_to_vmode(append duration)),
//	link vmode to bottom of vmode_mgr(lcd_vmode_add_single)
//cus_ctrl_timing to vmode (lcd_detail_timing_to_vmode(append duration))
void lcd_tablet_add_all_vmode(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_vmode_info_s *vmode_find = NULL;
	int i;

	if (!pdrv)
		return;

	// lcd_vmode_remove_all(pdrv); // ! multi device temp

	for (i = 0; i < pdrv->curr_dev->dev_cfg.timing.num_timings; i++) {
		vmode_find = lcd_detail_timing_to_vmode(pdrv->curr_dev->dev_cfg.timing.timings[i]);
		if (!vmode_find)
			continue;
		lcd_vmode_add_single(pdrv, vmode_find);
	}
}

void lcd_tablet_add_all_device_vmode(struct aml_lcd_drv_s *pdrv, struct aml_lcd_device_s *dev_p)
{
	struct lcd_vmode_info_s *vmode_find = NULL;
	int i;

	if (!pdrv)
		return;

	// lcd_vmode_remove_all(pdrv); // ! multi device temp

	for (i = 0; i < dev_p->dev_cfg.timing.num_timings; i++) {
		vmode_find = lcd_detail_timing_to_vmode(dev_p->dev_cfg.timing.timings[i]);
		if (!vmode_find)
			continue;
		lcd_vmode_add_single(pdrv, vmode_find);
	}
}

// * act_timing to pdrv->vinfo (lcd_config_init)
static void lcd_act_timing_update_vinfo(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_detail_timing_s *ptiming;

	if (!pdrv)
		return;
	ptiming = &pdrv->curr_dev->dev_cfg.timing.act_timing;
	memset(pdrv->output_name, 0, sizeof(pdrv->output_name));

	str_add_vmode(pdrv->output_name, 0,
		ptiming->h_active, ptiming->v_active, ptiming->frame_rate);


	pdrv->vinfo.width = ptiming->h_active;
	pdrv->vinfo.height = ptiming->v_active;
	pdrv->vinfo.field_height = ptiming->v_active;
	pdrv->vinfo.aspect_ratio_num = pdrv->curr_dev->dev_cfg.basic.screen_width;
	pdrv->vinfo.aspect_ratio_den = pdrv->curr_dev->dev_cfg.basic.screen_height;
	pdrv->vinfo.screen_real_width = pdrv->curr_dev->dev_cfg.basic.screen_width;
	pdrv->vinfo.screen_real_height = pdrv->curr_dev->dev_cfg.basic.screen_height;
	pdrv->vinfo.sync_duration_num = ptiming->sync_duration_num;
	pdrv->vinfo.sync_duration_den = ptiming->sync_duration_den;
	pdrv->vinfo.frac = ptiming->frac;
	pdrv->vinfo.brr_duration = pdrv->vmode_mgr.cur_vmode_info->base_fr;
	pdrv->vinfo.std_duration = ptiming->frame_rate;
	pdrv->vinfo.vfreq_max = ptiming->frame_rate_max;
	pdrv->vinfo.vfreq_min = ptiming->frame_rate_min;
	pdrv->vinfo.video_clk = pdrv->curr_dev->dev_cfg.timing.enc_clk;
	pdrv->vinfo.htotal = ptiming->h_period;
	pdrv->vinfo.vtotal = ptiming->v_period;
	pdrv->vinfo.hsw = ptiming->hsync_width;
	pdrv->vinfo.hbp = ptiming->hsync_bp;
	pdrv->vinfo.hfp = ptiming->hsync_fp;
	pdrv->vinfo.vsw = ptiming->vsync_width;
	pdrv->vinfo.vbp = ptiming->vsync_bp;
	pdrv->vinfo.vfp = ptiming->vsync_fp;
	pdrv->vinfo.viu_mux = VIU_MUX_ENCL | (pdrv->index << 4);
	switch (pdrv->curr_dev->dev_cfg.basic.lcd_type) {
	case LCD_LVDS:
	case LCD_MLVDS:
		pdrv->vinfo.connector_type = DRM_MODE_CONNECTOR_MESON_LVDS_A + pdrv->index;
		break;
	case LCD_VBYONE:
	case LCD_P2P:
		pdrv->vinfo.connector_type = DRM_MODE_CONNECTOR_MESON_VBYONE_A + pdrv->index;
		break;
	case LCD_MIPI:
		pdrv->vinfo.connector_type = DRM_MODE_CONNECTOR_MESON_MIPI_A + pdrv->index;
		break;
	default:
		pdrv->vinfo.connector_type = 0 + pdrv->index;
		break;
	}

	pdrv->vinfo.cur_enc_ppc = pdrv->curr_dev->dev_cfg.timing.ppc;
	pdrv->vinfo.asf_mode = 0;
	pdrv->vinfo.ufr_mode = 0;
	switch (ptiming->fr_adjust_type) {
	case 0:
		pdrv->vinfo.fr_adj_type = VOUT_FR_ADJ_CLK;
		break;
	case 1:
		pdrv->vinfo.fr_adj_type = VOUT_FR_ADJ_HTOTAL;
		break;
	case 2:
		pdrv->vinfo.fr_adj_type = VOUT_FR_ADJ_VTOTAL;
		break;
	case 3:
		pdrv->vinfo.fr_adj_type = VOUT_FR_ADJ_COMBO;
		break;
	case 4:
		pdrv->vinfo.fr_adj_type = VOUT_FR_ADJ_HDMI;
		break;
	case 5:
		pdrv->vinfo.fr_adj_type = VOUT_FR_ADJ_FREERUN;
		break;
	default:
		pdrv->vinfo.fr_adj_type = VOUT_FR_ADJ_NONE;
		break;
	}

	lcd_optical_vinfo_update(pdrv);
}

// * dft_timing 2 pdrv->vinfo (lcd_tablet_vout_server_init)
static void lcd_dft_timing_update_vinfo(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_detail_timing_s *ptiming = &pdrv->curr_dev->dev_cfg.timing.act_timing;

	if ((pdrv->status & LCD_STATUS_ENCL_ON) == 0)
		return;

	memset(pdrv->output_name, 0, sizeof(pdrv->output_name));

	str_add_vmode(pdrv->output_name, 0,
		ptiming->h_active, ptiming->v_active, ptiming->frame_rate);

	pdrv->vinfo.name = pdrv->output_name;
	pdrv->vinfo.mode = VMODE_LCD;
	pdrv->vinfo.width =  ptiming->h_active;
	pdrv->vinfo.height = ptiming->v_active;
	pdrv->vinfo.field_height = ptiming->v_active;
	pdrv->vinfo.aspect_ratio_num =  ptiming->h_active;
	pdrv->vinfo.aspect_ratio_den = ptiming->v_active;
	pdrv->vinfo.screen_real_width =  ptiming->h_active;
	pdrv->vinfo.screen_real_height = ptiming->v_active;
	pdrv->vinfo.sync_duration_num = ptiming->frame_rate;
	pdrv->vinfo.sync_duration_den = 1;
	pdrv->vinfo.frac = 0;
	pdrv->vinfo.std_duration = ptiming->frame_rate;
	pdrv->vinfo.vfreq_max = ptiming->frame_rate;
	pdrv->vinfo.vfreq_min = ptiming->frame_rate;
	pdrv->vinfo.video_clk = 0;
	pdrv->vinfo.htotal = ptiming->h_period;
	pdrv->vinfo.vtotal = ptiming->v_period;
	pdrv->vinfo.cur_enc_ppc = pdrv->curr_dev->dev_cfg.timing.ppc;
	pdrv->vinfo.fr_adj_type = VOUT_FR_ADJ_NONE;
	switch (pdrv->curr_dev->dev_cfg.basic.lcd_type) {
	case LCD_LVDS:
	case LCD_MLVDS:
		pdrv->vinfo.connector_type = DRM_MODE_CONNECTOR_MESON_LVDS_A + pdrv->index;
		break;
	case LCD_VBYONE:
	case LCD_P2P:
		pdrv->vinfo.connector_type = DRM_MODE_CONNECTOR_MESON_VBYONE_A + pdrv->index;
		break;
	case LCD_MIPI:
		pdrv->vinfo.connector_type = DRM_MODE_CONNECTOR_MESON_MIPI_A + pdrv->index;
		break;
	default:
		pdrv->vinfo.connector_type = 0 + pdrv->index;
		break;
	}
}

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

	lcd_act_timing_update_vinfo(pdrv);
	lcd_optical_vinfo_update(pdrv);
}

static void lcd_vmode_update(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_detail_timing_s *ptiming;
	unsigned int pre_pclk;
	int dur_index;
	unsigned char switch_type, switch_type_pre = pdrv->curr_dev->dev_cfg.timing.switch_type;

	if (!pdrv->curr_dev->dev_cfg.timing.base_timing)
		return;
	/* clear clk_change flag */
	pdrv->curr_dev->dev_cfg.timing.clk_change &= ~(LCD_CLK_PLL_CHANGE);
	if (pdrv->vmode_mgr.next_vmode_info) {
		pre_pclk = pdrv->curr_dev->dev_cfg.timing.base_timing->pixel_clk;
		pdrv->vmode_mgr.cur_vmode_info = pdrv->vmode_mgr.next_vmode_info;
		pdrv->vmode_mgr.next_vmode_info = NULL;

		pdrv->std_duration = pdrv->vmode_mgr.cur_vmode_info->duration;
		ptiming = pdrv->vmode_mgr.cur_vmode_info->dft_timing;
		pdrv->curr_dev->dev_cfg.timing.base_timing = ptiming;

		//update base_timing to act_timing
		lcd_base_to_enc_timing_init_config(pdrv);
		if (pdrv->curr_dev->dev_cfg.timing.base_timing->pixel_clk != pre_pclk) {
			pdrv->curr_dev->dev_cfg.timing.clk_change |= LCD_CLK_PLL_CHANGE;
			lcd_clk_generate_parameter(pdrv);
		}

		pdrv->vmode_switch = 1; // more than (vfp, hfp, clk) changed
	}

	switch_type = pdrv->curr_dev->dev_cfg.timing.act_timing.switch_type;
	switch (switch_type_pre) {
	case LCD_VMODE_SWITCH_FULL:
		pdrv->curr_dev->dev_cfg.timing.switch_type = LCD_VMODE_SWITCH_FULL;
		break;
	case LCD_VMODE_SWITCH_LIMIT:
		if (switch_type == LCD_VMODE_SWITCH_FULL)
			pdrv->curr_dev->dev_cfg.timing.switch_type = LCD_VMODE_SWITCH_FULL;
		else
			pdrv->curr_dev->dev_cfg.timing.switch_type = LCD_VMODE_SWITCH_LIMIT;
		break;
	case LCD_VMODE_SWITCH_MIN:
	case LCD_VMODE_SWITCH_MIN_WO_TCON_RST:
		if (switch_type == LCD_VMODE_SWITCH_FULL)
			pdrv->curr_dev->dev_cfg.timing.switch_type = LCD_VMODE_SWITCH_FULL;
		else if (switch_type == LCD_VMODE_SWITCH_LIMIT)
			pdrv->curr_dev->dev_cfg.timing.switch_type = LCD_VMODE_SWITCH_LIMIT;
		else
			pdrv->curr_dev->dev_cfg.timing.switch_type = LCD_VMODE_SWITCH_MIN;
		break;
	default:
		pdrv->curr_dev->dev_cfg.timing.switch_type = switch_type;
		break;
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

	LCD_DBG(pdrv, "%s: timing_switch_flag: %d, lcd_status: 0x%x", __func__,
		pdrv->curr_dev->dev_cfg.timing.switch_type, pdrv->status);

	if (pdrv->curr_dev->dev_cfg.timing.switch_type_dbg) {
		pdrv->curr_dev->dev_cfg.timing.switch_type =
			pdrv->curr_dev->dev_cfg.timing.switch_type_dbg;
		LCD_PR(pdrv, "%s: timing_switch_flag pre: %d, cur: %d, force dbg to final: %d",
		      __func__, switch_type_pre,
		      pdrv->curr_dev->dev_cfg.timing.act_timing.switch_type,
		      pdrv->curr_dev->dev_cfg.timing.switch_type);
	}

	if (!pdrv->vmode_mgr.cur_vmode_info || !pdrv->std_duration) {
		LCD_ERR(pdrv, "%s: cur_vmode_info or std_duration is null", __func__);
		return;
	}

	dur_index = pdrv->vmode_mgr.cur_vmode_info->duration_index;

	ptiming = &pdrv->curr_dev->dev_cfg.timing.act_timing;
	if (dur_index != 0xff) {
		ptiming->sync_duration_num = pdrv->std_duration[dur_index].duration_num;
		ptiming->sync_duration_den = pdrv->std_duration[dur_index].duration_den;
		ptiming->frac = pdrv->std_duration[dur_index].frac;
		ptiming->frame_rate = pdrv->std_duration[dur_index].frame_rate;
	} else {
		ptiming->sync_duration_num = pdrv->vmode_mgr.cur_vmode_info->base_fr;
		ptiming->sync_duration_den = 1;
		ptiming->frac = 0;
		ptiming->frame_rate = pdrv->vmode_mgr.cur_vmode_info->base_fr;
	}
	lcd_frame_rate_change(pdrv);

	LCD_DBG(pdrv, "%s: %dx%dp%dhz, duration=%d:%d, dur_index=%d, clk_change=0x%x", __func__,
		pdrv->curr_dev->dev_cfg.timing.act_timing.h_active,
		pdrv->curr_dev->dev_cfg.timing.act_timing.v_active,
		pdrv->curr_dev->dev_cfg.timing.act_timing.frame_rate,
		pdrv->curr_dev->dev_cfg.timing.act_timing.sync_duration_num,
		pdrv->curr_dev->dev_cfg.timing.act_timing.sync_duration_den,
		dur_index, pdrv->curr_dev->dev_cfg.timing.clk_change);
}

static int lcd_set_current_vmode(enum vmode_e mode, void *data)
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
	lcd_vmode_update(pdrv);
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

	pdrv->vmode_switch = 0;
	pdrv->status |= LCD_STATE_VMODE_ACTIVE;

	mutex_unlock(&lcd_power_mutex);
	return ret;
}

static int lcd_vout_disable(enum vmode_e cur_vmod, void *data)
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

static int lcd_vout_get_disp_cap(char *buf, void *data)
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

static enum vmode_e lcd_validate_vmode(char *mode, unsigned int frac, void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;

	if (!pdrv || !mode)
		return VMODE_MAX;

	LCD_DBG(pdrv, "%s: display_mode=%s, drv_mode=%s",  __func__, mode, pdrv->vinfo.name);

	if (lcd_tablet_outputmode_check(pdrv, mode))
		return VMODE_MAX;

	return VMODE_LCD;
}

static int lcd_framerate_support_check(struct aml_lcd_drv_s *pdrv, int duration)
{
	//unsigned int fr, ht, vt;
	//unsigned char i;
	struct lcd_detail_timing_s *dtm = &pdrv->curr_dev->dev_cfg.timing.act_timing;

	//if (dtm->fixed_type != 0xff) {
	//	for (i = 0; i < 8; i++) {
	//		if (dtm->fixed_val_set[i] == 0)
	//			break;
	//		if (n >= LCD_DURATION_MAX)
	//			break;
	//		fr = (dtm->fixed_type == 0) ? dtm->fixed_val_set[i] : dtm->pixel_clk;
	//		ht = (dtm->fixed_type == 1) ? dtm->fixed_val_set[i] : dtm->h_period;
	//		vt = (dtm->fixed_type == 2) ? dtm->fixed_val_set[i] : dtm->v_period;
	//		fr = ((fr / ht) * 100) / vt;
	//		if (fr == duration)
	//			return 1;
	//	}
	//}

	if ((duration <= dtm->frame_rate_max * 100) && (duration >= dtm->frame_rate_min * 100))
		return 1;

	return 0;
}

static int lcd_set_vframe_rate_hint(int duration, void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;
	struct vinfo_s *info;
	unsigned int i, duration_num = 60, duration_den = 1, duration_frac = 0;

	if (!pdrv)
		return -1;
	if (pdrv->probe_done == 0)
		return -1;

	if (lcd_vout_serve_bypass) {
		LCD_PR(pdrv, "vout_serve bypass");
		return 0;
	}
	if ((pdrv->status & LCD_STATUS_ENCL_ON) == 0) {
		LCD_PR(pdrv, "%s: lcd is disabled, exit", __func__);
		return -1;
	}

	if (lcd_fr_is_fixed(pdrv)) {
		LCD_PR(pdrv, "%s: fixed timing, exit", __func__);
		return -1;
	}
	if (pdrv->curr_dev->dev_cfg.fr_auto_flag == 0) {
		LCD_PR(pdrv, "%s: fr_auto_flag = 0, disabled", __func__);
		return 0;
	}

	mutex_lock(&lcd_vout_mutex);

	info = &pdrv->vinfo;
	if (!pdrv->curr_dev->dev_cfg.fr_auto_flag) {
		LCD_PR(pdrv, "%s: fr_auto_flag = 0, disabled", __func__);
		mutex_unlock(&lcd_vout_mutex);
		return 0;
	}

	if (duration == 0) { /* end hint */
		LCD_PR(pdrv, "%s: return mode: %s, fr_adj_type: %d", __func__,
			info->name, pdrv->curr_dev->dev_cfg.timing.act_timing.fr_adjust_type);

		pdrv->fr_duration = 0;
		if (pdrv->fr_mode == 0) {
			LCD_PR(pdrv, "%s: fr_mode is invalid, exit", __func__);
			mutex_unlock(&lcd_vout_mutex);
			return 0;
		}

		/* update frame rate */
		if (pdrv->curr_dev->dev_cfg.timing.base_timing) {
			pdrv->curr_dev->dev_cfg.timing.act_timing.frame_rate =
				pdrv->curr_dev->dev_cfg.timing.base_timing->frame_rate;
			pdrv->curr_dev->dev_cfg.timing.act_timing.sync_duration_num =
				pdrv->curr_dev->dev_cfg.timing.base_timing->sync_duration_num;
			pdrv->curr_dev->dev_cfg.timing.act_timing.sync_duration_den =
				pdrv->curr_dev->dev_cfg.timing.base_timing->sync_duration_den;
			pdrv->curr_dev->dev_cfg.timing.act_timing.frac =
				pdrv->curr_dev->dev_cfg.timing.base_timing->frac;
			pdrv->fr_mode = 0;
		}
	} else {
		if (!lcd_framerate_support_check(pdrv, duration)) {
			LCDERR("[%d]: %s: can't support duration %d\n, exit\n",
			       pdrv->index, __func__, duration);
			mutex_unlock(&lcd_vout_mutex);
			return -1;
		}
		LCD_PR(pdrv, "%s: duration: %d, frame_rate: %d, fr_adj_type: %d",
		      __func__, duration, duration / 100,
		      pdrv->curr_dev->dev_cfg.timing.act_timing.fr_adjust_type);

		for (i = 0; i < ARRAY_SIZE(lcd_std_fr); i++) {
			if (duration ==
				((lcd_std_fr[i].duration_num * 100) / lcd_std_fr[i].duration_den))
				break;
		}
		if (i < ARRAY_SIZE(lcd_std_fr)) {
			duration_num  = lcd_std_fr[i].duration_num;
			duration_den  = lcd_std_fr[i].duration_den;
			duration_frac = lcd_std_fr[i].frac;
		} else {
			duration_num  = duration;
			duration_den  = 100;
			duration_frac = (bool)(duration % 100);
		}

		pdrv->fr_duration = duration;
		/* if the sync_duration is same as current */
		if (duration_num == pdrv->curr_dev->dev_cfg.timing.act_timing.sync_duration_num &&
		    duration_den == pdrv->curr_dev->dev_cfg.timing.act_timing.sync_duration_den) {
			LCD_PR(pdrv, "%s: sync_duration is same, exit", __func__);
			mutex_unlock(&lcd_vout_mutex);
			return 0;
		}

		/* update frame rate */
		pdrv->curr_dev->dev_cfg.timing.act_timing.frame_rate = duration_num / duration_den;
		pdrv->curr_dev->dev_cfg.timing.act_timing.sync_duration_num = duration_num;
		pdrv->curr_dev->dev_cfg.timing.act_timing.sync_duration_den = duration_den;
		pdrv->curr_dev->dev_cfg.timing.act_timing.frac = duration_frac;
		pdrv->fr_mode = duration;
	}

	lcd_mode_framerate_automation_set_mode(pdrv);
	mutex_unlock(&lcd_vout_mutex);

	return 0;
}

// * bind 3 vout op
void lcd_tablet_vout_server_init(struct aml_lcd_drv_s *pdrv)
{
	char *curr_vout_connector, *curr_vout_mode;
	char lcd_connector[10];
	unsigned short h, v, fr, ret = 0;

	pdrv->vout_server = kzalloc(sizeof(*pdrv->vout_server), GFP_KERNEL);
	if (!pdrv->vout_server)
		return;
	pdrv->vout_server->name = kzalloc(32, GFP_KERNEL);
	if (!pdrv->vout_server->name) {
		kfree(pdrv->vout_server);
		return;
	}

	switch (pdrv->curr_dev->dev_cfg.basic.lcd_type) {
	case LCD_LVDS:
	case LCD_MLVDS:
		snprintf(lcd_connector, 10, "LVDS-%c", 'A' + pdrv->index);
		pdrv->vout_server->connector_type = DRM_MODE_CONNECTOR_MESON_LVDS_A + pdrv->index;
		break;
	case LCD_VBYONE:
	case LCD_P2P:
		snprintf(lcd_connector, 10, "VBYONE-%c", 'A' + pdrv->index);
		pdrv->vout_server->connector_type = DRM_MODE_CONNECTOR_MESON_VBYONE_A + pdrv->index;
		break;
	case LCD_MIPI:
		snprintf(lcd_connector, 10, "MIPI-%c", 'A' + pdrv->index);
		pdrv->vout_server->connector_type = DRM_MODE_CONNECTOR_MESON_MIPI_A + pdrv->index;
		break;
	default:
		snprintf(lcd_connector, 10, "LCD-%c", 'A' + pdrv->index);
		pdrv->vout_server->connector_type = 0 + pdrv->index;
		break;
	}

	sprintf(pdrv->vout_server->name, "lcd%d_vout_server", pdrv->index);
	pdrv->vout_server->op.get_vinfo            = lcd_mode_get_current_info;
	pdrv->vout_server->op.set_vmode            = lcd_set_current_vmode;
	pdrv->vout_server->op.validate_vmode       = lcd_validate_vmode;
	pdrv->vout_server->op.check_same_vmodeattr = lcd_mode_check_same_vmodeattr;
	pdrv->vout_server->op.vmode_is_supported   = lcd_mode_vmode_is_supported;
	pdrv->vout_server->op.disable              = lcd_vout_disable;
	pdrv->vout_server->op.set_state            = lcd_mode_vout_set_state;
	pdrv->vout_server->op.clr_state            = lcd_mode_vout_clr_state;
	pdrv->vout_server->op.get_state            = lcd_mode_vout_get_state;
	pdrv->vout_server->op.get_disp_cap         = lcd_vout_get_disp_cap;
	pdrv->vout_server->op.set_vframe_rate_hint = lcd_set_vframe_rate_hint;
	pdrv->vout_server->op.get_vframe_rate_hint = lcd_mode_get_vframe_rate_hint;
	pdrv->vout_server->op.set_bist             = lcd_mode_vout_debug_test;
	pdrv->vout_server->op.set_bl_brightness    = lcd_mode_vout_set_bl_brightness;
	pdrv->vout_server->op.get_bl_brightness    = lcd_mode_vout_get_bl_brightness;
	pdrv->vout_server->op.vout_suspend         = lcd_mode_suspend;
	pdrv->vout_server->op.vout_resume          = lcd_mode_resume;
	pdrv->vout_server->data = (void *)pdrv;

#ifdef CONFIG_AMLOGIC_VOUT_SERVE
	curr_vout_connector = get_uboot_connector0_type();
	curr_vout_mode = get_vout_mode_uboot();
	if (!strcmp(lcd_connector, curr_vout_connector)) {
		pdrv->viu_sel = 1;
		ret = str_parse_vmode(curr_vout_mode, &h, &v, &fr);
		LCD_PR(pdrv, "%s: connector0=%s, mode:%s",
			__func__, curr_vout_connector, curr_vout_mode);
	}
#endif
#ifdef CONFIG_AMLOGIC_VOUT2_SERVE
	curr_vout_connector = get_uboot_connector1_type();
	curr_vout_mode = get_vout2_mode_uboot();
	if (!strcmp(lcd_connector, curr_vout_connector)) {
		pdrv->viu_sel = 2;
		ret = str_parse_vmode(curr_vout_mode, &h, &v, &fr);
		LCD_PR(pdrv, "%s: connector1=%s, mode:%s",
			__func__, curr_vout_connector, curr_vout_mode);
	}
#endif
#ifdef CONFIG_AMLOGIC_VOUT3_SERVE
	curr_vout_connector = get_uboot_connector2_type();
	curr_vout_mode = get_vout3_mode_uboot();
	if (!strcmp(lcd_connector, curr_vout_connector)) {
		pdrv->viu_sel = 3;
		ret = str_parse_vmode(curr_vout_mode, &h, &v, &fr);
		LCD_PR(pdrv, "%s: connector2=%s, mode:%s",
			__func__, curr_vout_connector, curr_vout_mode);
	}
#endif
	if (ret && pdrv->viu_sel) {
		pdrv->curr_dev->dev_cfg.timing.act_timing.frame_rate = fr;
		LCD_PR(pdrv, "%s: update act_timing {%u,%u}%uhz", __func__, h, v, fr);
	}

	LCD_DBG(pdrv, "regist: %s", pdrv->vout_server->name);

	lcd_dft_timing_update_vinfo(pdrv);

#ifdef CONFIG_AMLOGIC_VOUT_SERVE
	vout_register_server(pdrv->vout_server);
#endif
}

void lcd_tablet_vout_server_remove(struct aml_lcd_drv_s *pdrv)
{
#ifdef CONFIG_AMLOGIC_VOUT_SERVE
	vout_unregister_server(pdrv->vout_server);
#endif

	kfree(pdrv->vout_server->name);
	kfree(pdrv->vout_server);
	pdrv->vout_server = NULL;
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
	vmode = lcd_validate_vmode(mode, 0, (void *)pdrv);

	mutex_lock(&lcd_vout_mutex);
	if (vmode == VMODE_LCD)
		lcd_vmode_update(pdrv);
	lcd_vmode_vinfo_update(pdrv);
	mutex_unlock(&lcd_vout_mutex);

	kfree(mode);
}

static void lcd_config_init(struct aml_lcd_drv_s *pdrv)
{
	lcd_base_to_enc_timing_init_config(pdrv);

	lcd_clk_config_parameter_init(pdrv);
	lcd_clk_generate_parameter(pdrv);
	pdrv->curr_dev->dev_cfg.timing.clk_change = 0; /* clear clk_change flag */

	lcd_vmode_init(pdrv);
	lcd_dft_timing_update_vinfo(pdrv);

#ifdef CONFIG_AMLOGIC_LCD_MIPI_DSI
	if (pdrv->curr_dev->dev_cfg.basic.lcd_type == LCD_MIPI) {
		lcd_dsi_if_bind(pdrv);
		lcd_dsi_post_config_load(pdrv);
	}
#endif
}

int lcd_mode_tablet_init(struct aml_lcd_drv_s *pdrv)
{
	if (!pdrv)
		return -1;

	lcd_config_init(pdrv);

	switch (pdrv->curr_dev->dev_cfg.basic.lcd_type) {
#ifdef CONFIG_AMLOGIC_LCD_VBYONE
	case LCD_VBYONE:
		lcd_vbyone_interrupt_up(pdrv);
		break;
#endif
	default:
		break;
	}

	lcd_vrr_dev_register(pdrv);

	return 0;
}

int lcd_mode_tablet_remove(struct aml_lcd_drv_s *pdrv)
{
	lcd_vrr_dev_unregister(pdrv);

	switch (pdrv->curr_dev->dev_cfg.basic.lcd_type) {
#ifdef CONFIG_AMLOGIC_LCD_VBYONE
	case LCD_VBYONE:
		lcd_vbyone_interrupt_down(pdrv);
		break;
#endif
	default:
		break;
	}
	// lcd_vmode_remove_all(pdrv);

	return 0;
}
