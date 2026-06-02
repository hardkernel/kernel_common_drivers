// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * ODROID custom HDMI timing bridge.
 *
 * ODROID owns custom modes as DRM display modes.  hdmitx converts only the
 * requested DRM mode into hdmi_timing when the Amlogic path needs it.
 */

#include <linux/gcd.h>
#include <linux/math64.h>
#include <linux/mutex.h>
#include <linux/string.h>

#include "hdmitx_odroid_modes.h"

#if IS_ENABLED(CONFIG_ODROID_CUSTOM_DISPLAY_MODES)
#include <linux/odroid/display.h>

static DEFINE_MUTEX(odroid_hdmitx_current_lock);
static struct hdmi_timing odroid_hdmitx_current_timing;
static char odroid_hdmitx_current_name[DRM_DISPLAY_MODE_LEN];

bool odroid_hdmitx_mode_is_custom_vic(u32 vic)
{
	return vic == HDMI_ODROID_CUSTOM_VIDEO;
}

static void odroid_hdmitx_timing_from_drm_mode(struct hdmi_timing *ht,
		const struct drm_display_mode *mode)
{
	u32 h_total;
	u32 v_total;
	u32 div;

	h_total = mode->htotal;
	v_total = mode->vtotal;
	if (!h_total || !v_total)
		return;

	memset(ht, 0, sizeof(*ht));
	ht->vic = HDMI_ODROID_CUSTOM_VIDEO;
	ht->name = (unsigned char *)mode->name;
	ht->sname = NULL;
	ht->pi_mode = !(mode->flags & DRM_MODE_FLAG_INTERLACE);
	ht->h_freq = div_u64((u64)mode->clock * 1000, h_total);
	ht->v_freq = div_u64((u64)mode->clock * 1000000,
				  (u64)h_total * v_total);
	ht->pixel_freq = mode->clock;

	ht->h_total = h_total;
	ht->h_active = mode->hdisplay;
	ht->h_blank = mode->htotal - mode->hdisplay;
	ht->h_front = mode->hsync_start - mode->hdisplay;
	ht->h_sync = mode->hsync_end - mode->hsync_start;
	ht->h_back = mode->htotal - mode->hsync_end;

	ht->v_total = v_total;
	ht->v_active = mode->vdisplay;
	ht->v_blank = mode->vtotal - mode->vdisplay;
	ht->v_front = mode->vsync_start - mode->vdisplay;
	ht->v_sync = mode->vsync_end - mode->vsync_start;
	ht->v_back = mode->vtotal - mode->vsync_end;

	ht->h_pol = !!(mode->flags & DRM_MODE_FLAG_PHSYNC);
	ht->v_pol = !!(mode->flags & DRM_MODE_FLAG_PVSYNC);

	div = gcd(ht->h_active, ht->v_active);
	if (!div)
		div = 1;
	ht->h_pict = ht->h_active / div;
	ht->v_pict = ht->v_active / div;
	ht->v_sync_ln = 1;
	ht->h_pixel = 1;
	ht->v_pixel = 1;
	ht->pixel_repetition_factor = !!(mode->flags & DRM_MODE_FLAG_DBLCLK);
}

const struct hdmi_timing *odroid_hdmitx_mode_vic_to_timing(u32 vic)
{
	const struct hdmi_timing *timing = NULL;

	if (!odroid_hdmitx_mode_is_custom_vic(vic))
		return NULL;

	mutex_lock(&odroid_hdmitx_current_lock);
	if (odroid_hdmitx_current_timing.name &&
	    odroid_hdmitx_current_timing.pixel_freq)
		timing = &odroid_hdmitx_current_timing;
	mutex_unlock(&odroid_hdmitx_current_lock);

	return timing;
}

int odroid_hdmitx_mode_get_timing_by_name(const char *name,
		struct hdmi_timing *timing)
{
	struct drm_display_mode mode;
	int ret;

	if (!name || !timing)
		return -EINVAL;

	memset(timing, 0, sizeof(*timing));
	ret = odroid_display_find_by_name(name, &mode);
	if (ret)
		return ret;

	odroid_hdmitx_timing_from_drm_mode(timing, &mode);
	if (!timing->pixel_freq)
		return -EINVAL;
	timing->name = (unsigned char *)name;

	return 0;
}

int odroid_hdmitx_mode_set_current_timing(const struct hdmi_timing *timing)
{
	if (!timing || !timing->name || !timing->pixel_freq)
		return -EINVAL;

	mutex_lock(&odroid_hdmitx_current_lock);
	odroid_hdmitx_current_timing = *timing;
	strscpy(odroid_hdmitx_current_name, (const char *)timing->name,
		sizeof(odroid_hdmitx_current_name));
	odroid_hdmitx_current_timing.name =
		(unsigned char *)odroid_hdmitx_current_name;
	odroid_hdmitx_current_timing.vic = HDMI_ODROID_CUSTOM_VIDEO;
	mutex_unlock(&odroid_hdmitx_current_lock);

	return 0;
}

const struct hdmi_timing *odroid_hdmitx_mode_set_current_by_name(const char *name)
{
	struct hdmi_timing timing;

	if (odroid_hdmitx_mode_get_timing_by_name(name, &timing))
		return NULL;
	if (odroid_hdmitx_mode_set_current_timing(&timing))
		return NULL;

	return odroid_hdmitx_mode_vic_to_timing(HDMI_ODROID_CUSTOM_VIDEO);
}

#endif
