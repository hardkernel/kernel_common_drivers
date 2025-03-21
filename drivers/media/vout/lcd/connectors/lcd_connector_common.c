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
#ifdef CONFIG_AMLOGIC_VPU
#include <linux/amlogic/media/vpu/vpu.h>
#endif
#ifdef CONFIG_AMLOGIC_MEDIA_VIDEO
#include <linux/amlogic/media/video_sink/video.h>
#endif
#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
#include <linux/amlogic/media/amvecm/amvecm.h>
#endif
#include <linux/amlogic/media/vout/vinfo.h>
#include <linux/amlogic/media/vout/lcd/lcd_vout.h>
#include <linux/amlogic/media/vout/lcd/lcd_notify.h>
#include "../lcd_common.h"
#include <linux/sched/clock.h>
#include "./lcd_connector.h"

static int lcd_type_supported(struct aml_lcd_drv_s *pdrv)
{
	int lcd_type = pdrv->config.basic.lcd_type;
	int ret = -1;

	switch (lcd_type) {
	case LCD_LVDS:
	case LCD_RGB:
	case LCD_BT656:
	case LCD_BT1120:
		return 0;
#ifdef CONFIG_AMLOGIC_LCD_VBYONE
	case LCD_VBYONE:
		return 0;
#endif
#ifdef CONFIG_AMLOGIC_LCD_TCON
	case LCD_MLVDS:
	case LCD_P2P:
		if (pdrv->mode == LCD_MODE_TV)
			return 0;
		break;
#endif
#ifdef CONFIG_AMLOGIC_LCD_MIPI_DSI
	case LCD_MIPI:
		if (pdrv->mode == LCD_MODE_TABLET)
			return 0;
		break;
#endif
	default:
		LCDERR("invalid lcd type: %s(%d)\n",
		       lcd_type_type_to_str(lcd_type), lcd_type);
		break;
	}
	return ret;
}

void lcd_connector_driver_init_pre(struct aml_lcd_drv_s *pdrv)
{
	LCDPR("[%d]: connector driver init(ver %s): %s\n", pdrv->index, LCD_DRV_VERSION,
	      lcd_type_type_to_str(pdrv->config.basic.lcd_type));

	if (lcd_type_supported(pdrv))
		return;

	if (pdrv->mode == LCD_MODE_TABLET)
		lcd_frame_rate_change(pdrv);

#ifdef CONFIG_AMLOGIC_VPU
	vpu_dev_mem_power_on(pdrv->lcd_vpu_dev);
#endif
	lcd_clk_gate_switch(pdrv, 1);

	/* init driver */
	switch (pdrv->config.basic.lcd_type) {
#ifdef CONFIG_AMLOGIC_LCD_VBYONE
	case LCD_VBYONE:
		lcd_vbyone_interrupt_enable(pdrv, 0);
		break;
#endif
	default:
		break;
	}

	lcd_set_clk(pdrv);
	lcd_set_venc(pdrv);

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s finished\n", pdrv->index, __func__);
}

void lcd_connector_driver_disable_post(struct aml_lcd_drv_s *pdrv)
{
	lcd_venc_enable(pdrv, 0);

	lcd_disable_clk(pdrv);
	lcd_clk_gate_switch(pdrv, 0);
#ifdef CONFIG_AMLOGIC_VPU
	vpu_dev_mem_power_down(pdrv->lcd_vpu_dev);
	vpu_dev_clk_release(pdrv->lcd_vpu_dev);
#endif

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s finished\n", pdrv->index, __func__);
}

void lcd_connector_driver_change(struct aml_lcd_drv_s *pdrv)
{
	unsigned long long local_time[2];

	if (!pdrv->probe_done)
		LCDPR("[%d]: config not loaded, bypass %s", pdrv->index, __func__);

	local_time[0] = sched_clock();

	LCDPR("[%d]: connector driver change(ver %s): %s\n",
	      pdrv->index, LCD_DRV_VERSION,
	      lcd_type_type_to_str(pdrv->config.basic.lcd_type));

	if (lcd_type_supported(pdrv))
		return;

	if (pdrv->status & LCD_STATUS_IF_ON) {
#ifdef CONFIG_AMLOGIC_LCD_VBYONE
		if (pdrv->config.basic.lcd_type == LCD_VBYONE)
			lcd_vbyone_interrupt_enable(pdrv, 0);
#endif
	}

	if (!(pdrv->status & LCD_STATUS_ENCL_DUMMY)) {
		lcd_clk_change(pdrv);
		lcd_venc_change(pdrv);
	}

	if (pdrv->status & LCD_STATUS_IF_ON) {
#ifdef CONFIG_AMLOGIC_LCD_VBYONE
		if (pdrv->config.basic.lcd_type == LCD_VBYONE) {
			lcd_vbyone_wait_stable(pdrv);
			lcd_vbyone_interrupt_enable(pdrv, 1);
		}
#endif
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s finished\n", pdrv->index, __func__);
	local_time[1] = sched_clock();
	pdrv->proc_time.driver_change_time = local_time[1] - local_time[0];
}

void lcd_connector_driver_init(struct aml_lcd_drv_s *pdrv)
{
	if (lcd_type_supported(pdrv))
		return;

	// lcd_power_screen_black(pdrv);
	// vpp power off when suspend, need cpu write mute
	if (pdrv->viu_sel == 1 && pdrv->data->chip_type == LCD_CHIP_S6) {
		LCDPR("[%d]: %s black\n", pdrv->index, __func__);
#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
		// lcd_post2_matrix();
#endif
#ifdef CONFIG_AMLOGIC_MEDIA_VIDEO
		// mute_output_vcbus();
#endif
	}

	switch (pdrv->config.basic.lcd_type) {
	case LCD_RGB:
		lcd_rgb_control_set(pdrv, 1);
		lcd_rgb_pinmux_set(pdrv, 1);
		break;
	case LCD_BT656:
	case LCD_BT1120:
		lcd_bt_control_set(pdrv, 1);
		lcd_bt_pinmux_set(pdrv, 1);
		break;
	case LCD_LVDS:
		lcd_lvds_dphy_set(pdrv, 1);
		lcd_lvds_enable(pdrv);
		lcd_phy_set(pdrv, LCD_PHY_ON);
		break;
#ifdef CONFIG_AMLOGIC_LCD_VBYONE
	case LCD_VBYONE:
		lcd_vbyone_pinmux_set(pdrv, 1);
		lcd_vbyone_dphy_set(pdrv, 1);
		lcd_vbyone_enable(pdrv);
		lcd_vbyone_wait_hpd(pdrv);
		lcd_phy_set(pdrv, LCD_PHY_ON);
		lcd_vbyone_power_on_wait_stable(pdrv);
		lcd_vbyone_interrupt_enable(pdrv, 1);
		break;
#endif
#ifdef CONFIG_AMLOGIC_LCD_TCON
	case LCD_MLVDS:
		lcd_tcon_top_init(pdrv);
		lcd_mlvds_dphy_set(pdrv, 1);
		lcd_tcon_enable(pdrv);
		lcd_mlvds_pinmux_set(pdrv, 1);
		lcd_phy_set(pdrv, LCD_PHY_ON);
		break;
	case LCD_P2P:
		lcd_tcon_top_init(pdrv);
		lcd_p2p_pinmux_set(pdrv, 1);
		lcd_phy_set(pdrv, LCD_PHY_ON);
		lcd_p2p_dphy_set(pdrv, 1);
		lcd_tcon_enable(pdrv);
		break;
#endif
#ifdef CONFIG_AMLOGIC_LCD_MIPI_DSI
	case LCD_MIPI:
		lcd_phy_set(pdrv, LCD_PHY_ON);
		lcd_mipi_dphy_set(pdrv, 1);
		lcd_dsi_tx_ctrl(pdrv, 1);
		break;
#endif
	default:
		break;
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s finished\n", pdrv->index, __func__);

	if (pdrv->fr_lock)
		pdrv->fr_lock->rst = 1;
}

void lcd_connector_driver_disable(struct aml_lcd_drv_s *pdrv)
{
	LCDPR("[%d]: disable driver\n", pdrv->index);

	if (lcd_type_supported(pdrv))
		return;

	if (pdrv->fr_lock)
		pdrv->fr_lock->rst = 1;

	switch (pdrv->config.basic.lcd_type) {
	case LCD_RGB:
		lcd_rgb_pinmux_set(pdrv, 0);
		break;
	case LCD_BT656:
	case LCD_BT1120:
		lcd_bt_pinmux_set(pdrv, 0);
		break;
	case LCD_LVDS:
		lcd_phy_set(pdrv, LCD_PHY_OFF);
		lcd_lvds_disable(pdrv);
		lcd_lvds_dphy_set(pdrv, 0);
		break;
#ifdef CONFIG_AMLOGIC_LCD_VBYONE
	case LCD_VBYONE:
		lcd_vbyone_interrupt_enable(pdrv, 0);
		lcd_phy_set(pdrv, LCD_PHY_OFF);
		lcd_vbyone_pinmux_set(pdrv, 0);
		lcd_vbyone_disable(pdrv);
		lcd_vbyone_dphy_set(pdrv, 0);
		break;
#endif
#ifdef CONFIG_AMLOGIC_LCD_MIPI_DSI
	case LCD_MIPI:
		lcd_dsi_tx_ctrl(pdrv, 0);
		lcd_phy_set(pdrv, LCD_PHY_OFF);
		lcd_mipi_dphy_set(pdrv, 0);
		break;
#endif
#ifdef CONFIG_AMLOGIC_LCD_TCON
	case LCD_MLVDS:
		lcd_tcon_disable(pdrv);
		lcd_mlvds_dphy_set(pdrv, 0);
		lcd_phy_set(pdrv, LCD_PHY_OFF);
		lcd_mlvds_pinmux_set(pdrv, 0);
		break;
	case LCD_P2P:
		lcd_tcon_disable(pdrv);
		lcd_p2p_dphy_set(pdrv, 0);
		lcd_phy_set(pdrv, LCD_PHY_OFF);
		lcd_p2p_pinmux_set(pdrv, 0);
		break;
#endif
	default:
		break;
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s finished\n", pdrv->index, __func__);
}

/* sync_duration is real_value * 100 */
void lcd_connector_frame_rate_adjust(struct aml_lcd_drv_s *pdrv, int duration)
{
	LCDPR("[%d]: %s: sync_duration=%d\n", pdrv->index, __func__, duration);

	lcd_vout_notify_mode_change_pre(pdrv);

	/* update frame rate */
	pdrv->config.timing.act_timing.frame_rate = duration / 100;
	pdrv->config.timing.act_timing.sync_duration_num = duration;
	pdrv->config.timing.act_timing.sync_duration_den = 100;
	pdrv->config.timing.act_timing.frac =
		lcd_fr_is_frac(pdrv, pdrv->config.timing.act_timing.frame_rate);

	/* update interface timing */
	lcd_frame_rate_change(pdrv);
#ifdef CONFIG_AMLOGIC_LCD_VBYONE
	if (pdrv->config.basic.lcd_type == LCD_VBYONE)
		lcd_vbyone_interrupt_enable(pdrv, 0);
#endif

	/* change clk parameter */
	lcd_clk_change(pdrv);
	lcd_venc_change(pdrv);

#ifdef CONFIG_AMLOGIC_LCD_VBYONE
	if (pdrv->config.basic.lcd_type == LCD_VBYONE) {
		lcd_vbyone_wait_stable(pdrv);
		lcd_vbyone_interrupt_enable(pdrv, 1);
	}
#endif

	lcd_vout_notify_mode_change(pdrv);
}
