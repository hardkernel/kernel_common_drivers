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

static struct lcd_duration_s lcd_std_fr[] = {
	{330, 330,    1,    0},
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

//default ref mode compatible
static struct lcd_vmode_info_s lcd_vmode_ref[] = {
	{//0
		.name     = "600p",
		.width    = 1024,
		.height   = 600,
		.base_fr  = 60,
		.duration_index = 0,
		.duration = {
			{60,  60,     1,    0},
			{59,  60000,  1001, 1},
			{50,  50,     1,    0},
			{48,  48,     1,    0},
			{47,  48000,  1001, 1},
			{0,   0,      0,    0}
		},
		.dft_timing = NULL,
	},
	{//1
		.name     = "768p",
		.width    = 1366,
		.height   = 768,
		.base_fr  = 60,
		.duration_index = 0,
		.duration = {
			{60,  60,     1,    0},
			{59,  60000,  1001, 1},
			{50,  50,     1,    0},
			{48,  48,     1,    0},
			{47,  48000,  1001, 1},
			{0,   0,      0,    0}
		},
		.dft_timing = NULL,
	},
	{//2
		.name     = "1080p",
		.width    = 1920,
		.height   = 1080,
		.base_fr  = 60,
		.duration_index = 0,
		.duration = {
			{60,  60,     1,    0},
			{59,  60000,  1001, 1},
			{50,  50,     1,    0},
			{48,  48,     1,    0},
			{47,  48000,  1001, 1},
			{0,   0,      0,    0}
		},
		.dft_timing = NULL,
	},
	{//3
		.name     = "2160p",
		.width    = 3840,
		.height   = 2160,
		.base_fr  = 60,
		.duration_index = 0,
		.duration = {
			{60,  60,     1,    0},
			{59,  60000,  1001, 1},
			{50,  50,     1,    0},
			{48,  48,     1,    0},
			{47,  48000,  1001, 1},
			{0,   0,      0,    0}
		},
		.dft_timing = NULL,
	},
	{//4
		.name     = "invalid",
		.width    = 1920,
		.height   = 1080,
		.base_fr  = 60,
		.duration_index = 0,
		.duration = {
			{60,  60,     1,    0},
			{0,   0,      0,    0}
		},
		.dft_timing = NULL,
	},
};

static int lcd_vmode_add_list(struct aml_lcd_drv_s *pdrv, struct lcd_vmode_info_s *vmode_info)
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
	pdrv->vmode_mgr.vmode_cnt += vmode_info->duration_cnt;

	LCD_PR(pdrv, "%s: name:%s, base_fr:%dhz, vmode_cnt: %d", __func__,
		cur_list->info->name, cur_list->info->base_fr, pdrv->vmode_mgr.vmode_cnt);

	return 0;
}

static void lcd_vmode_duration_add(struct lcd_vmode_info_s *vmode_find,
		struct lcd_duration_s *duration_tablet, unsigned int size)
{
	unsigned int n, i;

	memset(vmode_find->duration, 0, sizeof(struct lcd_duration_s) * LCD_DURATION_MAX);
	n = 0;
	for (i = 0; i < size; i++) {
		if (duration_tablet[i].frame_rate == 0)
			break;
		if (duration_tablet[i].frame_rate > vmode_find->dft_timing->frame_rate_max)
			continue;
		if (duration_tablet[i].frame_rate < vmode_find->dft_timing->frame_rate_min)
			break;

		vmode_find->duration[n].frame_rate = duration_tablet[i].frame_rate;
		vmode_find->duration[n].duration_num = duration_tablet[i].duration_num;
		vmode_find->duration[n].duration_den = duration_tablet[i].duration_den;
		vmode_find->duration[n].frac = duration_tablet[i].frac;
		n++;
		if (n >= LCD_DURATION_MAX)
			break;
	}
	vmode_find->duration_cnt = n;
}

//--default ref mode: 1080p60hz, 2160p60hz...
static struct lcd_vmode_info_s *lcd_vmode_default_find(struct lcd_detail_timing_s *ptiming)
{
	struct lcd_vmode_info_s *vmode_find = NULL;
	int i, dft_vmode_size = ARRAY_SIZE(lcd_vmode_ref);

	for (i = 0; i < dft_vmode_size; i++) {
		if (ptiming->h_active == lcd_vmode_ref[i].width &&
		    ptiming->v_active == lcd_vmode_ref[i].height &&
		    ptiming->frame_rate == lcd_vmode_ref[i].base_fr) {
			vmode_find = kzalloc(sizeof(*vmode_find), GFP_KERNEL);
			if (!vmode_find)
				return NULL;
			memcpy(vmode_find, &lcd_vmode_ref[i], sizeof(struct lcd_vmode_info_s));
			vmode_find->dft_timing = ptiming;
			lcd_vmode_duration_add(vmode_find,
				lcd_vmode_ref[i].duration, LCD_DURATION_MAX);
			break;
		}
	}

	return vmode_find;
}

//--general mode: (h)x(v)p(frame_rate)hz
static struct lcd_vmode_info_s *lcd_vmode_general_find(struct lcd_detail_timing_s *ptiming)
{
	struct lcd_vmode_info_s *vmode_find = NULL;
	unsigned int std_fr_size = ARRAY_SIZE(lcd_std_fr);

	vmode_find = kzalloc(sizeof(*vmode_find), GFP_KERNEL);
	if (!vmode_find)
		return NULL;
	vmode_find->width = ptiming->h_active;
	vmode_find->height = ptiming->v_active;
	vmode_find->base_fr = ptiming->frame_rate;
	snprintf(vmode_find->name, 32, "%dx%dp", ptiming->h_active, ptiming->v_active);
	vmode_find->dft_timing = ptiming;
	lcd_vmode_duration_add(vmode_find, lcd_std_fr, std_fr_size);

	return vmode_find;
}

static struct lcd_vmode_info_s *lcd_vmode_find(struct lcd_detail_timing_s *ptiming)
{
	struct lcd_vmode_info_s *vmode_find = lcd_vmode_default_find(ptiming);

	return vmode_find ? vmode_find : lcd_vmode_general_find(ptiming);
}

static void lcd_output_vmode_init(struct aml_lcd_drv_s *pdrv, int config_prev)
{
	struct lcd_vmode_info_s *vmode_find = NULL;
	int i;

	if (!pdrv)
		return;

	mutex_lock(&lcd_vout_mutex);
	// lcd_vmode_remove_all(pdrv); ///! multi device temp

	for (i = 0; i < pdrv->curr_dev->dev_cfg.timing.num_timings; i++) {
		vmode_find = lcd_vmode_find(pdrv->curr_dev->dev_cfg.timing.timings[i]);
		if (vmode_find)
			lcd_vmode_add_list(pdrv, vmode_find);
	}

	//if (config_prev) {//before lcd_get_config, use act_timing
	//	vmode_find = lcd_vmode_find(&pdrv->curr_dev->dev_cfg.timing.act_timing);
	//	if (vmode_find)
	//		lcd_vmode_add_list(pdrv, vmode_find);
	//} else {
	//	for (i = 0; i < pdrv->curr_dev->dev_cfg.timing.num_timings; i++) {
	//		vmode_find = lcd_vmode_find(pdrv->curr_dev->dev_cfg.timing.timings[i]);
	//		if (vmode_find)
	//			lcd_vmode_add_list(pdrv, vmode_find);
	//	}
	//}

	mutex_unlock(&lcd_vout_mutex);
}

void lcd_output_vmode_init_to_device(struct aml_lcd_drv_s *pdrv, struct aml_lcd_device_s *dev_p)
{
	struct lcd_vmode_info_s *vmode_find = NULL;
	int i;

	if (!pdrv)
		return;

	mutex_lock(&lcd_vout_mutex);
	// lcd_vmode_remove_all(pdrv); ///! multi device temp

	for (i = 0; i < dev_p->dev_cfg.timing.num_timings; i++) {
		vmode_find = lcd_vmode_find(dev_p->dev_cfg.timing.timings[i]);
		if (vmode_find)
			lcd_vmode_add_list(pdrv, vmode_find);
	}

	//if (config_prev) {//before lcd_get_config, use act_timing
	//	vmode_find = lcd_vmode_find(&pdrv->curr_dev->dev_cfg.timing.act_timing);
	//	if (vmode_find)
	//		lcd_vmode_add_list(pdrv, vmode_find);
	//} else {
	//	for (i = 0; i < pdrv->curr_dev->dev_cfg.timing.num_timings; i++) {
	//		vmode_find = lcd_vmode_find(pdrv->curr_dev->dev_cfg.timing.timings[i]);
	//		if (vmode_find)
	//			lcd_vmode_add_list(pdrv, vmode_find);
	//	}
	//}

	mutex_unlock(&lcd_vout_mutex);
}

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

static void lcd_vmode_vinfo_update(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_detail_timing_s *ptiming;
	struct lcd_vmode_info_s *info;

	if (!pdrv)
		return;
	if (!pdrv->vmode_mgr.cur_vmode_info)
		return;
	info = pdrv->vmode_mgr.cur_vmode_info;
	memset(pdrv->output_name, 0, sizeof(pdrv->output_name));
	sprintf(pdrv->output_name, "%s%dhz", info->name, info->base_fr);
	LCD_DBG(pdrv, "%s: outputmode = %s",  __func__, pdrv->output_name);

	ptiming = &pdrv->curr_dev->dev_cfg.timing.act_timing;

	/* update vinfo */
	pdrv->vinfo.name = pdrv->output_name;
	pdrv->vinfo.mode = VMODE_LCD;
	pdrv->vinfo.width = info->width;
	pdrv->vinfo.height = info->height;
	pdrv->vinfo.field_height = info->height;
	pdrv->vinfo.aspect_ratio_num = pdrv->curr_dev->dev_cfg.basic.screen_width;
	pdrv->vinfo.aspect_ratio_den = pdrv->curr_dev->dev_cfg.basic.screen_height;
	pdrv->vinfo.screen_real_width = pdrv->curr_dev->dev_cfg.basic.screen_width;
	pdrv->vinfo.screen_real_height = pdrv->curr_dev->dev_cfg.basic.screen_height;
	pdrv->vinfo.sync_duration_num = ptiming->sync_duration_num;
	pdrv->vinfo.sync_duration_den = ptiming->sync_duration_den;
	pdrv->vinfo.frac = ptiming->frac;
	pdrv->vinfo.brr_duration = info->base_fr;
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
	pdrv->vinfo.viu_mux = VIU_MUX_ENCL;
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
	pdrv->vinfo.asf_mode = ptiming->asf_mode;
	pdrv->vinfo.ufr_mode = ptiming->ufr_mode;
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

/* ************************************************** *
 * vout server api
 * **************************************************
 */
static enum vmode_e lcd_validate_vmode(char *mode, unsigned int frac, void *data)
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

static void lcd_vmode_update(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_detail_timing_s *ptiming;
	unsigned int pre_pclk;
	int dur_index;
	unsigned char switch_type, switch_type_pre = pdrv->curr_dev->dev_cfg.timing.switch_type;

	if (!pdrv->curr_dev->dev_cfg.timing.base_timing)
		return;
	/* clear clk_change flag */
	pdrv->curr_dev->dev_cfg.timing.clk_change &= ~(LCD_CLK_PLL_RESET);
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
			pdrv->curr_dev->dev_cfg.timing.clk_change |=
				(LCD_CLK_PLL_RESET | LCD_CLK_PLL_CHANGE);
			lcd_clk_generate_parameter(pdrv);
		}
		pdrv->vmode_switch = 1;
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
		LCDERR("[%d]: %s: cur_vmode_info or std_duration is null\n",
			pdrv->index, __func__);
		return;
	}
	dur_index = pdrv->vmode_mgr.cur_vmode_info->duration_index;

	pdrv->curr_dev->dev_cfg.timing.act_timing.sync_duration_num =
		pdrv->std_duration[dur_index].duration_num;
	pdrv->curr_dev->dev_cfg.timing.act_timing.sync_duration_den =
		pdrv->std_duration[dur_index].duration_den;
	pdrv->curr_dev->dev_cfg.timing.act_timing.frac = pdrv->std_duration[dur_index].frac;
	pdrv->curr_dev->dev_cfg.timing.act_timing.frame_rate =
		pdrv->std_duration[dur_index].frame_rate;
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
	lcd_vmode_update(pdrv);
	lcd_vmode_vinfo_update(pdrv);
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

	mutex_lock(&lcd_vout_mutex);
	pdrv->status &= ~LCD_STATE_VMODE_ACTIVE;
	mutex_unlock(&lcd_vout_mutex);

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

struct lcd_vframe_match_s {
	int fps; /* *100 */
	unsigned int frame_rate;
};

static struct lcd_vframe_match_s lcd_vframe_match_table[] = {
	{33000, 330},
	{28800, 288},
	{24000, 240},
	{23976, 239},
	{20000, 200},
	{19200, 192},
	{19180, 191},
	{16500, 165},
	{14400, 144},
	{12000, 120},
	{11988, 119},
	{10000, 100},
	{9600, 96},
	{9590, 95},
	{6000, 60},
	{5994, 59},
	{5000, 50},
	{4800, 48},
	{4795, 47}
};

static int lcd_framerate_auto_std_duration_index(struct aml_lcd_drv_s *pdrv,
						 struct lcd_vframe_match_s *vtable,
						 int size, int duration)
{
	unsigned int frame_rate;
	int i, j;

	for (i = 0; i < size; i++) {
		if (duration == vtable[i].fps) {
			frame_rate = vtable[i].frame_rate;
			for (j = 0; j < LCD_DURATION_MAX; j++) {
				if (pdrv->std_duration[j].frame_rate == 0)
					break;
				if (frame_rate == pdrv->std_duration[j].frame_rate)
					return j;
			}
		}
	}

	return LCD_DURATION_MAX;
}

static int lcd_set_vframe_rate_hint(int duration, void *data)
{
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)data;
	struct vinfo_s *info;
	unsigned int frame_rate = 60;
	unsigned int duration_num = 60, duration_den = 1, frac = 0;
	struct lcd_vframe_match_s *vtable = NULL;
	int n, find = 0;

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
	vtable = lcd_vframe_match_table;
	n = ARRAY_SIZE(lcd_vframe_match_table);

	if (duration == 0) { /* end hint */
		LCD_PR(pdrv, "%s: return mode: %s, fr_adj_type: %d", __func__, info->name,
			pdrv->curr_dev->dev_cfg.timing.act_timing.fr_adjust_type);

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
		find = lcd_framerate_auto_std_duration_index(pdrv, vtable, n, duration);
		if (find >= LCD_DURATION_MAX) {
			LCDERR("[%d]: %s: can't support duration %d\n, exit\n",
			       pdrv->index, __func__, duration);
			mutex_unlock(&lcd_vout_mutex);
			return -1;
		}

		frame_rate = pdrv->std_duration[find].frame_rate;
		duration_num = pdrv->std_duration[find].duration_num;
		duration_den = pdrv->std_duration[find].duration_den;
		frac = pdrv->std_duration[find].frac;

		LCD_PR(pdrv, "%s: duration: %d, frame_rate: %d, fr_adj_type: %d",
		      __func__, duration, frame_rate,
		      pdrv->curr_dev->dev_cfg.timing.act_timing.fr_adjust_type);

		pdrv->fr_duration = duration;
		/* if the sync_duration is same as current */
		if (duration_num == pdrv->curr_dev->dev_cfg.timing.act_timing.sync_duration_num &&
		    duration_den == pdrv->curr_dev->dev_cfg.timing.act_timing.sync_duration_den) {
			LCD_PR(pdrv, "%s: sync_duration is same, exit", __func__);
			mutex_unlock(&lcd_vout_mutex);
			return 0;
		}

		/* update frame rate */
		pdrv->curr_dev->dev_cfg.timing.act_timing.frame_rate = frame_rate;
		pdrv->curr_dev->dev_cfg.timing.act_timing.sync_duration_num = duration_num;
		pdrv->curr_dev->dev_cfg.timing.act_timing.sync_duration_den = duration_den;
		pdrv->curr_dev->dev_cfg.timing.act_timing.frac = frac;
		pdrv->fr_mode = 1;
	}

	lcd_mode_framerate_automation_set_mode(pdrv);
	mutex_unlock(&lcd_vout_mutex);

	return 0;
}

static void lcd_vinfo_update_default(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_detail_timing_s *ptiming = &pdrv->curr_dev->dev_cfg.timing.act_timing;

	// unsigned int frac;
	enum vmode_e vmode;
	char *mode;

	if ((pdrv->status & LCD_STATUS_ENCL_ON) == 0)
		return;

	mode = kstrdup(get_vout_mode_uboot(), GFP_KERNEL);
	if (!mode)
		return;

	ptiming = &pdrv->curr_dev->dev_cfg.timing.act_timing;
	pdrv->vmode_mgr.cur_vmode_info = &lcd_vmode_ref[4];

	lcd_output_vmode_init(pdrv, 1);
	vmode = lcd_validate_vmode(mode, 0, (void *)pdrv);
	if (vmode == VMODE_LCD) {
		if (pdrv->vmode_mgr.next_vmode_info) {
			pdrv->vmode_mgr.cur_vmode_info = pdrv->vmode_mgr.next_vmode_info;
			pdrv->vmode_mgr.next_vmode_info = NULL;
		}
	}
	pdrv->std_duration = pdrv->vmode_mgr.cur_vmode_info->duration;

	memset(pdrv->output_name, 0, sizeof(pdrv->output_name));
	snprintf(pdrv->output_name, sizeof(pdrv->output_name), "%s", mode);
	pdrv->vinfo.name = pdrv->output_name;
	pdrv->vinfo.mode = VMODE_LCD;
	pdrv->vinfo.width = ptiming->h_active;
	pdrv->vinfo.height = ptiming->v_active;
	pdrv->vinfo.field_height = ptiming->v_active;
	pdrv->vinfo.aspect_ratio_num = ptiming->h_active;
	pdrv->vinfo.aspect_ratio_den = ptiming->v_active;
	pdrv->vinfo.screen_real_width = ptiming->h_active;
	pdrv->vinfo.screen_real_height = ptiming->v_active;
	pdrv->vinfo.sync_duration_num = ptiming->frame_rate;
	pdrv->vinfo.sync_duration_den = 1;
	pdrv->vinfo.frac = ptiming->frac;
	pdrv->vinfo.std_duration = ptiming->frame_rate;
	pdrv->vinfo.vfreq_max = pdrv->vinfo.std_duration;
	pdrv->vinfo.vfreq_min = pdrv->vinfo.std_duration;
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

	kfree(mode);
}

void lcd_tv_vout_server_init(struct aml_lcd_drv_s *pdrv)
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

	lcd_vinfo_update_default(pdrv);

#ifdef CONFIG_AMLOGIC_VOUT_SERVE
	vout_register_server(pdrv->vout_server);
#endif
}

void lcd_tv_vout_server_remove(struct aml_lcd_drv_s *pdrv)
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

	// lcd_output_vmode_init(pdrv, 0); // !temp remove
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

	lcd_vmode_init(pdrv);
	lcd_frame_rate_change(pdrv);

	lcd_clk_generate_parameter(pdrv);
	pdrv->curr_dev->dev_cfg.timing.clk_change = 0; /* clear clk_change flag */
}

int lcd_mode_tv_init(struct aml_lcd_drv_s *pdrv)
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

int lcd_mode_tv_remove(struct aml_lcd_drv_s *pdrv)
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
