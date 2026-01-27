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
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/amlogic/media/vout/vinfo.h>
#include <linux/amlogic/media/vout/vout_notify.h>
#include <linux/amlogic/media/vout/lcd/lcd_vout.h>
#include "lcd_common.h"

#include <linux/component.h>
#include <drm/amlogic/meson_drm_bind.h>

static int meson_lcd_bind(struct device *dev, struct device *master, void *data);
static void meson_lcd_unbind(struct device *dev, struct device *master, void *data);

struct drm_lcd_wrapper {
	struct meson_panel_dev drm_lcd_instance;
	struct aml_lcd_drv_s *lcd_drv;
	int drm_lcd_type;
	int drm_id;
};

#define to_drm_lcd_wrapper(x)	container_of(x, struct drm_lcd_wrapper, drm_lcd_instance)

static struct drm_lcd_wrapper drm_lcd_wrappers[LCD_MAX_DRV];

static void lcd_drm_vmode_update(struct aml_lcd_drv_s *pdrv, struct lcd_detail_timing_s *ptiming,
		struct drm_display_mode *pmode, unsigned int frame_rate)
{
	unsigned int pclk = ptiming->pixel_clk;
	unsigned int htotal = ptiming->h_period;
	unsigned int vtotal = ptiming->v_period;
	unsigned long long temp;

	if (!pmode)
		return;

	switch (ptiming->fr_adjust_type) {
	case 0: /* pixel clk adjust */
		pclk = frame_rate * htotal * vtotal;
		break;
	case 1: /* htotal adjust */
		temp = pclk;
		temp *= 100;
		htotal = vtotal * frame_rate;
		htotal = lcd_do_div(temp, htotal);
		htotal = (htotal + 99) / 100; /* round off */
		pclk = frame_rate * htotal * vtotal;
		break;
	case 2: /* vtotal adjust */
		temp = pclk;
		temp *= 100;
		vtotal = htotal * frame_rate;
		vtotal = lcd_do_div(temp, vtotal);
		vtotal = (vtotal + 99) / 100; /* round off */
		pclk = frame_rate * htotal * vtotal;
		break;
	case 3: /* free adjust, use min/max range to calculate */
		temp = pclk;
		temp *= 100;
		vtotal = htotal * frame_rate;
		vtotal = lcd_do_div(temp, vtotal);
		vtotal = (vtotal + 99) / 100; /* round off */
		if (vtotal > ptiming->v_period_max) {
			vtotal = ptiming->v_period_max;
			htotal = vtotal * frame_rate;
			htotal = lcd_do_div(temp, htotal);
			htotal = (htotal + 99) / 100; /* round off */
			if (htotal > ptiming->h_period_max)
				htotal = ptiming->h_period_max;
		} else if (vtotal < ptiming->v_period_min) {
			vtotal = ptiming->v_period_min;
			htotal = vtotal * frame_rate;
			htotal = lcd_do_div(temp, htotal);
			htotal = (htotal + 99) / 100; /* round off */
			if (htotal < ptiming->h_period_min)
				htotal = ptiming->h_period_min;
		}
		pclk = frame_rate * htotal * vtotal;
		break;
	case 4: /* hdmi mode */
		if (frame_rate == 59 || frame_rate == 119) {
			/* pixel clk adjust */
			pclk = frame_rate * htotal * vtotal;
		} else if (frame_rate == 47) {
			/* htotal adjust */
			temp = pclk;
			temp *= 100;
			htotal = vtotal * 50;
			htotal = lcd_do_div(temp, htotal);
			htotal = (htotal + 99) / 100; /* round off */
			pclk = frame_rate * htotal * vtotal;
		} else if (frame_rate == 95) {
			/* htotal adjust */
			temp = pclk;
			temp *= 100;
			htotal = vtotal * 100;
			htotal = lcd_do_div(temp, htotal);
			htotal = (htotal + 99) / 100; /* round off */
			pclk = frame_rate * htotal * vtotal;
		} else {
			/* htotal adjust */
			temp = pclk;
			temp *= 100;
			htotal = vtotal * frame_rate;
			htotal = lcd_do_div(temp, htotal);
			htotal = (htotal + 99) / 100; /* round off */
			pclk = frame_rate * htotal * vtotal;
		}
		break;
	default:
		LCD_ERR(pdrv, "drm vmode update: invalid fr_adj: %d", ptiming->fr_adjust_type);
		return;
	}

	pmode->clock = pclk / 1000;
	pmode->htotal = htotal;
	pmode->vtotal = vtotal;
}

static void lcd_drm_display_mode_add(struct aml_lcd_drv_s *pdrv,
		struct lcd_detail_timing_s *ptiming,
		struct drm_display_mode *pmode, unsigned short frame_rate)
{
	unsigned short tmp;

	pmode->clock = ptiming->pixel_clk / 1000;
	pmode->hdisplay = ptiming->h_active;
	tmp = ptiming->h_period - ptiming->hsync_width - ptiming->hsync_bp;
	pmode->hsync_start = tmp;
	pmode->hsync_end = ptiming->hsync_width + tmp;
	pmode->htotal = ptiming->h_period;
	pmode->vdisplay = ptiming->v_active;
	tmp = ptiming->v_period - ptiming->vsync_width - ptiming->vsync_bp;
	pmode->vsync_start = tmp;
	pmode->vsync_end = ptiming->vsync_width + tmp;
	pmode->vtotal = ptiming->v_period;
	pmode->width_mm = pdrv->curr_dev->dev_cfg.basic.screen_width;
	pmode->height_mm = pdrv->curr_dev->dev_cfg.basic.screen_height;

	if (frame_rate != ptiming->frame_rate)
		lcd_drm_vmode_update(pdrv, ptiming, pmode, frame_rate);

	LCD_DBG(pdrv, "drm mode add: %s, clock=%dkHz, htotal=%d, vtotal=%d",
		pmode->name, pmode->clock, pmode->htotal, pmode->vtotal);
}

static int get_lcd_drm_modes(struct meson_panel_dev *panel,
		struct drm_display_mode **modes, int *num)
{
	struct drm_lcd_wrapper *wrapper = to_drm_lcd_wrapper(panel);
	struct aml_lcd_drv_s *pdrv = wrapper->lcd_drv;
	struct drm_display_mode *nmodes;
	struct lcd_vmode_list_s *temp_list;
	struct lcd_detail_timing_s *ptiming;
	unsigned int i, frame_rate, mode_cnt, mode_idx = 0;

	if (!pdrv || !pdrv->vmode_mgr.vmode_list_header)
		return 0;

	mode_cnt = pdrv->vmode_mgr.vmode_cnt;

	LCD_DBG(pdrv, "drm mode get: %d", mode_cnt);

	nmodes = kcalloc(mode_cnt, sizeof(struct drm_display_mode), GFP_KERNEL);
	if (!nmodes) {
		*num = 0;
		return -ENOMEM;
	}

	temp_list = pdrv->vmode_mgr.vmode_list_header;
	while (temp_list) {
		if (!temp_list->info || !temp_list->info->dft_timing)
			continue;
		ptiming = temp_list->info->dft_timing;

		if (pdrv->mode == LCD_MODE_TABLET) {
			memset(nmodes[mode_idx].name, 0, DRM_DISPLAY_MODE_LEN);
			str_add_vmode(nmodes[mode_idx].name, 0, temp_list->info->width,
					temp_list->info->height, temp_list->info->base_fr);
			lcd_drm_display_mode_add(pdrv, ptiming, &nmodes[mode_idx],
				temp_list->info->base_fr);
			if (pdrv->vmode_mgr.cur_vmode_info == temp_list->info &&
			    temp_list->info->duration_index == 0xff)
				nmodes[mode_idx].type |= DRM_MODE_TYPE_PREFERRED;
			mode_idx++;
		}

		for (i = 0; i < temp_list->info->duration_cnt; i++) {
			frame_rate = temp_list->info->duration[i].frame_rate;
			if (frame_rate == 0)
				break;

			memset(nmodes[mode_idx].name, 0, DRM_DISPLAY_MODE_LEN);
			str_add_vmode(nmodes[mode_idx].name, 0, temp_list->info->width,
				temp_list->info->height, frame_rate);
			lcd_drm_display_mode_add(pdrv, ptiming, &nmodes[mode_idx], frame_rate);

			if (pdrv->vmode_mgr.cur_vmode_info == temp_list->info &&
					i == temp_list->info->duration_index)
				nmodes[mode_idx].type |= DRM_MODE_TYPE_PREFERRED;
			mode_idx++;
		}
		temp_list = temp_list->next;
	}

	*num = mode_idx;
	*modes = nmodes;

	return 0;
}

static int get_lcd_modes_vrr_range(struct meson_panel_dev *panel, void *range, int max, int *num)
{
	struct drm_vrr_mode_group *group;
	struct drm_lcd_wrapper *wrapper = to_drm_lcd_wrapper(panel);
	struct aml_lcd_drv_s *pdrv;
	struct lcd_detail_timing_s *timing;
	struct lcd_vmode_list_s *temp_list;
	int cnt = 0;

	if (!wrapper)
		return -1;

	pdrv = wrapper->lcd_drv;

	if (!pdrv || !panel || !range || !num)
		return -1;

	temp_list = pdrv->vmode_mgr.vmode_list_header;
	while (temp_list && cnt < max) {
		if (!temp_list->info || !temp_list->info->dft_timing)
			continue;

		timing = temp_list->info->dft_timing;
		group = &((struct drm_vrr_mode_group *)range)[cnt];
		group->width = timing->h_active;
		group->height = timing->v_active;
		group->vrr_min = timing->frame_rate_min * VRR_DIV;
		group->vrr_max = timing->frame_rate_max * VRR_DIV;
		group->brr = timing->frame_rate;
		str_add_vmode(group->modename, 0, timing->h_active, timing->v_active,
								temp_list->info->base_fr);
		LCD_DBG(pdrv, "update vrr range[%d]:%s: w:%d, h:%d, range:[%d-%d], fr:%d",
			cnt, group->modename, group->width, group->height,
			group->vrr_min, group->vrr_max, timing->frame_rate);

		temp_list = temp_list->next;
		cnt++;
	}
	*num = cnt;

	return 0;
}

static u8 lcd_drm_timing_find(struct aml_lcd_drv_s *pdrv, struct drm_display_mode *pmode)
{
	u32 pclk, htotal, vtotal, h_active, v_active;
	struct lcd_vmode_list_s *temp_list = pdrv->vmode_mgr.vmode_list_header;
	u8 vmode_list_idx = 0;
	int i;

	if (!pmode || !temp_list)
		return -1;

	pclk = pmode->clock;
	htotal = pmode->htotal;
	vtotal = pmode->vtotal;
	h_active = pmode->hdisplay;
	v_active = pmode->vdisplay;
	LCD_PR(pdrv, "drm set %u[%u] * %u[%u] %u kHz", h_active, htotal, v_active, vtotal, pclk);

	pclk = pclk * 1000;
	pclk = div_around(pclk, vtotal);
	pclk = pclk * 10;
	pclk = div_around(pclk, htotal);
	// pclk is (frame_rate * 10) now

	while (temp_list) {
		if (temp_list->info->width != h_active || temp_list->info->height != v_active) {
			temp_list = temp_list->next;
			continue;
		}

		if (pdrv->mode == LCD_MODE_TABLET) {
			if (lcd_u32_diff(pclk, temp_list->info->base_fr * 10) < 10) {
				LCD_DBG(pdrv, "timing find: list[%u] %dx%d@%dhz (base)",
					vmode_list_idx,
					temp_list->info->width, temp_list->info->height,
					temp_list->info->base_fr);
				temp_list->info->duration_index = 0xff;
				pdrv->vmode_mgr.next_vmode_info = temp_list->info;
				return 0;
			}
		}

		for (i = 0; i < LCD_DURATION_MAX; i++) {
			if (temp_list->info->duration[i].frame_rate == 0)
				break;

			if (lcd_u32_diff(pclk, temp_list->info->duration[i].frame_rate * 10) < 10) {
				LCD_DBG(pdrv, "timing find: list[%u] %dx%d@%dhz (fr[%u])",
					vmode_list_idx,
					temp_list->info->width, temp_list->info->height,
					temp_list->info->duration[i].frame_rate, i);
				temp_list->info->duration_index = i;
				pdrv->vmode_mgr.next_vmode_info = temp_list->info;
				return 0;
			}
		}

		temp_list = temp_list->next;
		vmode_list_idx++;
	}

	LCD_ERR(pdrv, "timing find: invalid %u[%u] * %u[%u] %u kHz",
			h_active, htotal, v_active, vtotal, pclk);
	return 1;
}

static void lcd_drm_set_mode_timing(struct meson_panel_dev *panel,
				struct drm_display_mode *mode, enum vmode_e vmode)
{
	struct drm_lcd_wrapper *wrapper = to_drm_lcd_wrapper(panel);
	struct aml_lcd_drv_s *pdrv;
	// struct lcd_vmode_list_s *temp_list = pdrv->vmode_mgr.vmode_list_header;
	u8 ret;

	if (!wrapper || !wrapper->lcd_drv)
		return;

	pdrv = wrapper->lcd_drv;

	if (!mode || ((vmode & 0xf) != VMODE_LCD)) {
		LCD_ERR(pdrv, "drm mode set: null mode");
		return;
	}

	ret = lcd_drm_timing_find(pdrv, mode);
	if (ret) {
		LCD_ERR(pdrv, "drm mode set: not matched");
		return;
	}

	lcd_mode_timing_set_next(pdrv, vmode);
}

static int meson_lcd_bind(struct device *dev, struct device *master, void *data)
{
	struct meson_drm_bound_data *bound_data = data;
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)dev_get_drvdata(dev);
	int index = pdrv->index;
	int connector_type = 0;

	/*init drm instance*/
	drm_lcd_wrappers[index].lcd_drv = pdrv;
	drm_lcd_wrappers[index].drm_lcd_instance.base.ver = MESON_DRM_CONNECTOR_V10;
	drm_lcd_wrappers[index].drm_lcd_instance.get_modes = get_lcd_drm_modes;
	drm_lcd_wrappers[index].drm_lcd_instance.get_modes_vrr_range = get_lcd_modes_vrr_range;
	drm_lcd_wrappers[index].drm_lcd_instance.set_mode_timing = lcd_drm_set_mode_timing;

	/*set lcd type.*/
	switch (pdrv->curr_dev->dev_cfg.basic.lcd_type) {
	case LCD_LVDS:
	case LCD_MLVDS:
		if (index == 1)
			connector_type = DRM_MODE_CONNECTOR_MESON_LVDS_B;
		else if (index == 2)
			connector_type = DRM_MODE_CONNECTOR_MESON_LVDS_C;
		else
			connector_type = DRM_MODE_CONNECTOR_MESON_LVDS_A;
		break;
	case LCD_VBYONE:
	case LCD_P2P:
		if (index)
			connector_type = DRM_MODE_CONNECTOR_MESON_VBYONE_B;
		else
			connector_type = DRM_MODE_CONNECTOR_MESON_VBYONE_A;
		break;
	case LCD_MIPI:
		if (index)
			connector_type = DRM_MODE_CONNECTOR_MESON_MIPI_B;
		else
			connector_type = DRM_MODE_CONNECTOR_MESON_MIPI_A;
		break;
	default:
		break;
	}
	/*todo: add more connector_type here for tconless*/
	drm_lcd_wrappers[index].drm_lcd_type = connector_type;

	/*bind instance to drm*/
	if (bound_data->connector_component_bind) {
		drm_lcd_wrappers[index].drm_id =
			bound_data->connector_component_bind(bound_data->drm,
				drm_lcd_wrappers[index].drm_lcd_type,
				&drm_lcd_wrappers[index].drm_lcd_instance.base);
		LCD_PR(pdrv, "drm bind: connector_type: 0x%x, drm_id: %d",
			drm_lcd_wrappers[index].drm_lcd_type, drm_lcd_wrappers[index].drm_id);
	} else {
		LCD_ERR(pdrv, "no bind func from drm");
	}

	drm_lcd_wrappers[index].drm_lcd_instance.base.vout_serv = pdrv->vout_server;
	return 0;
}

static void meson_lcd_unbind(struct device *dev, struct device *master, void *data)
{
	struct meson_drm_bound_data *bound_data = data;
	struct aml_lcd_drv_s *pdrv = (struct aml_lcd_drv_s *)dev_get_drvdata(dev);
	int index = pdrv->index;

	if (bound_data->connector_component_unbind) {
		bound_data->connector_component_unbind(bound_data->drm,
			drm_lcd_wrappers[index].drm_lcd_type,
			&drm_lcd_wrappers[index].drm_lcd_instance.base);
		LCD_PR(pdrv, "drm unbind: connector_type: 0x%x, drm_id: %d",
			drm_lcd_wrappers[index].drm_lcd_type, drm_lcd_wrappers[index].drm_id);
	} else {
		LCD_ERR(pdrv, "no unbind func from drm");
	}

	drm_lcd_wrappers[index].drm_id = 0;
}

static const struct component_ops meson_lcd_bind_ops = {
	.bind	= meson_lcd_bind,
	.unbind	= meson_lcd_unbind,
};

int lcd_drm_add(struct device *dev)
{
	/*bind to drm*/
	component_add(dev, &meson_lcd_bind_ops);

	return 0;
}

void lcd_drm_remove(struct device *dev)
{
	component_del(dev, &meson_lcd_bind_ops);
}
