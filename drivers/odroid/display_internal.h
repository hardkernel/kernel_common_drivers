/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ODROID_DISPLAY_INTERNAL_H
#define __ODROID_DISPLAY_INTERNAL_H

#include <linux/odroid/display.h>

struct device_node;

int odroid_display_add_mode(const struct drm_display_mode *mode, bool preferred,
		bool fixed);
void odroid_display_set_preferred(const char *name);
void odroid_display_set_force_single(bool force, bool explicit);
bool odroid_display_force_single_explicit(void);
int odroid_display_parse_modeline(char *modeline,
		struct drm_display_mode *mode);

#if IS_ENABLED(CONFIG_ODROID_CUSTOM_DISPLAY_MODES_CMDLINE)
void odroid_display_load_cmdline(void);
#else
static inline void odroid_display_load_cmdline(void) { }
#endif

#if IS_ENABLED(CONFIG_ODROID_CUSTOM_DISPLAY_MODES_DEVICETREE)
int odroid_display_load_dt_node(struct device_node *node);
int odroid_display_scan_dt(void);
int odroid_display_register_dt_driver(void);
void odroid_display_unregister_dt_driver(void);
#else
static inline int odroid_display_load_dt_node(struct device_node *node) { return 0; }
static inline int odroid_display_scan_dt(void) { return 0; }
static inline int odroid_display_register_dt_driver(void) { return 0; }
static inline void odroid_display_unregister_dt_driver(void) { }
#endif

#if IS_ENABLED(CONFIG_ODROID_CUSTOM_DISPLAY_MODES_MODELINE_FILE)
int odroid_display_load_files(void);
#else
static inline int odroid_display_load_files(void) { return 0; }
#endif

#if IS_ENABLED(CONFIG_ODROID_CUSTOM_DISPLAY_MODES_DEBUGFS)
int odroid_display_register_debugfs(void);
void odroid_display_unregister_debugfs(void);
#else
static inline int odroid_display_register_debugfs(void) { return 0; }
static inline void odroid_display_unregister_debugfs(void) { }
#endif

#endif
