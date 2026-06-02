/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
#ifndef __HDMITX_ODROID_MODES_H__
#define __HDMITX_ODROID_MODES_H__

#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_mode.h>

#if IS_ENABLED(CONFIG_ODROID_CUSTOM_DISPLAY_MODES)
bool odroid_hdmitx_mode_is_custom_vic(u32 vic);
const struct hdmi_timing *odroid_hdmitx_mode_vic_to_timing(u32 vic);
const struct hdmi_timing *odroid_hdmitx_mode_set_current_by_name(const char *name);
int odroid_hdmitx_mode_get_timing_by_name(const char *name,
		struct hdmi_timing *timing);
int odroid_hdmitx_mode_set_current_timing(const struct hdmi_timing *timing);
#endif

#endif
