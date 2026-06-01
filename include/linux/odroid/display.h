/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_ODROID_DISPLAY_H
#define __LINUX_ODROID_DISPLAY_H

#include <linux/types.h>
#include <drm/drm_modes.h>

#define ODROID_DISPLAY_MAX_MODES    64
#define ODROID_DISPLAY_CUSTOM_VIC   0x400

int odroid_display_ensure_loaded(void);
int odroid_display_count(void);
int odroid_display_get_mode(unsigned int index,
		struct drm_display_mode *mode);
int odroid_display_find_by_name(const char *name,
		struct drm_display_mode *mode);
const char *odroid_display_preferred_name(void);
bool odroid_display_force_single(void);
bool odroid_display_has_mode(const char *name);

#endif /* __LINUX_ODROID_DISPLAY_H */
