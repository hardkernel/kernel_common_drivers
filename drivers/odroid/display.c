// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "display_internal.h"

static DEFINE_MUTEX(odroid_display_lock);
static DEFINE_MUTEX(odroid_load_lock);
static struct drm_display_mode odroid_modes[ODROID_DISPLAY_MAX_MODES];
static unsigned int odroid_num_modes;
static bool odroid_loaded;
static bool odroid_force_single_mode;
static bool odroid_force_single_explicit;
static char odroid_preferred[DRM_DISPLAY_MODE_LEN];
static bool odroid_preferred_explicit;

static unsigned int parse_fixed_clock_khz(char *str, int digits)
{
	unsigned int ipart;
	unsigned int fpart = 0;
	unsigned int len;
	char buf[8] = { 0, };
	char *p;

	if (!str || !digits)
		return 0;

	p = memscan(str, '.', strlen(str));
	if (*p == '.') {
		*p++ = '\0';
		len = strnlen(p, digits);
		memcpy(buf, p, len);
		fpart = simple_strtoul(buf, NULL, 10);
		while (len++ < digits)
			fpart *= 10;
	}

	ipart = simple_strtoul(str, NULL, 10);
	while (digits-- > 0)
		ipart *= 10;

	return ipart + fpart;
}

static int odroid_display_find_index_locked(const char *name)
{
	unsigned int i;

	if (!name || !name[0])
		return -EINVAL;

	for (i = 0; i < odroid_num_modes; i++) {
		if (!strcmp(odroid_modes[i].name, name))
			return i;
	}

	return -ENOENT;
}

static void odroid_display_set_preferred_locked(const char *name)
{
	if (!name || !name[0])
		return;

	strscpy(odroid_preferred, name, sizeof(odroid_preferred));
}

void odroid_display_set_preferred(const char *name)
{
	mutex_lock(&odroid_display_lock);
	odroid_display_set_preferred_locked(name);
	odroid_preferred_explicit = !!(name && name[0]);
	mutex_unlock(&odroid_display_lock);
}

void odroid_display_set_force_single(bool force, bool explicit)
{
	mutex_lock(&odroid_display_lock);
	if (!odroid_force_single_explicit || explicit) {
		odroid_force_single_mode = force;
		odroid_force_single_explicit = explicit;
	}
	mutex_unlock(&odroid_display_lock);
}

bool odroid_display_force_single_explicit(void)
{
	bool explicit;

	mutex_lock(&odroid_display_lock);
	explicit = odroid_force_single_explicit;
	mutex_unlock(&odroid_display_lock);

	return explicit;
}

int odroid_display_add_mode(const struct drm_display_mode *mode, bool preferred,
		bool fixed)
{
	int index;
	int ret = 0;

	if (!mode || !mode->name[0] || !mode->clock ||
	    !mode->hdisplay || !mode->vdisplay)
		return -EINVAL;

	mutex_lock(&odroid_display_lock);
	index = odroid_display_find_index_locked(mode->name);
	if (index < 0) {
		if (odroid_num_modes >= ODROID_DISPLAY_MAX_MODES) {
			ret = -ENOSPC;
			goto out;
		}
		index = odroid_num_modes++;
	}

	odroid_modes[index] = *mode;
	if (preferred)
		odroid_modes[index].type |= DRM_MODE_TYPE_PREFERRED;
	else
		odroid_modes[index].type &= ~DRM_MODE_TYPE_PREFERRED;

	if (preferred && !odroid_preferred_explicit)
		odroid_display_set_preferred_locked(mode->name);
	if (fixed && !odroid_force_single_explicit)
		odroid_force_single_mode = true;

	pr_info("odroid: display mode %s %ux%u clock=%u%s%s\n",
		mode->name, mode->hdisplay, mode->vdisplay, mode->clock,
		preferred ? " preferred" : "", fixed ? " fixed" : "");

out:
	mutex_unlock(&odroid_display_lock);
	return ret;
}

int odroid_display_parse_modeline(char *modeline,
		struct drm_display_mode *drm_mode)
{
	char *str;
	char *argv[32];
	int argc = 0;
	int pos = 0;

	if (!modeline || !modeline[0] || !drm_mode)
		return -EINVAL;

	while (*modeline == ' ' || *modeline == '\t')
		modeline++;
	if (*modeline == '#')
		return -EINVAL;

	while ((str = strsep(&modeline, " ,\t")) != NULL) {
		if (!str[0])
			continue;
		if (argc >= ARRAY_SIZE(argv))
			return -EINVAL;
		argv[argc++] = str;
	}

	if (!argc)
		return -EINVAL;
	if (!strcasecmp(argv[pos], "modeline"))
		pos++;
	if ((argc - pos) < 10)
		return -EINVAL;

	memset(drm_mode, 0, sizeof(*drm_mode));
	if (argv[pos][0] == '"') {
		char *end;

		strscpy(drm_mode->name, argv[pos] + 1, sizeof(drm_mode->name));
		end = strchr(drm_mode->name, '"');
		if (end)
			*end = '\0';
	} else {
		strscpy(drm_mode->name, argv[pos], sizeof(drm_mode->name));
	}
	pos++;

	drm_mode->clock = parse_fixed_clock_khz(argv[pos++], 3);
	drm_mode->hdisplay = simple_strtoul(argv[pos++], NULL, 10);
	drm_mode->hsync_start = simple_strtoul(argv[pos++], NULL, 10);
	drm_mode->hsync_end = simple_strtoul(argv[pos++], NULL, 10);
	drm_mode->htotal = simple_strtoul(argv[pos++], NULL, 10);
	drm_mode->vdisplay = simple_strtoul(argv[pos++], NULL, 10);
	drm_mode->vsync_start = simple_strtoul(argv[pos++], NULL, 10);
	drm_mode->vsync_end = simple_strtoul(argv[pos++], NULL, 10);
	drm_mode->vtotal = simple_strtoul(argv[pos++], NULL, 10);

	if (drm_mode->hsync_start < drm_mode->hdisplay ||
	    drm_mode->hsync_end < drm_mode->hsync_start ||
	    drm_mode->htotal < drm_mode->hsync_end ||
	    drm_mode->vsync_start < drm_mode->vdisplay ||
	    drm_mode->vsync_end < drm_mode->vsync_start ||
	    drm_mode->vtotal < drm_mode->vsync_end)
		return -EINVAL;

	drm_mode->type = DRM_MODE_TYPE_DRIVER;

	while (pos < argc) {
		str = argv[pos++];
		if (!strcasecmp(str, "+hsync"))
			drm_mode->flags |= DRM_MODE_FLAG_PHSYNC;
		else if (!strcasecmp(str, "-hsync"))
			drm_mode->flags |= DRM_MODE_FLAG_NHSYNC;
		else if (!strcasecmp(str, "+vsync"))
			drm_mode->flags |= DRM_MODE_FLAG_PVSYNC;
		else if (!strcasecmp(str, "-vsync"))
			drm_mode->flags |= DRM_MODE_FLAG_NVSYNC;
		else if (!strcasecmp(str, "interlaced"))
			drm_mode->flags |= DRM_MODE_FLAG_INTERLACE;
		else if (!strcasecmp(str, "dblclk"))
			drm_mode->flags |= DRM_MODE_FLAG_DBLCLK;
	}

	return 0;
}

int odroid_display_ensure_loaded(void)
{
	mutex_lock(&odroid_load_lock);
	if (!odroid_loaded) {
		odroid_display_load_files();
		odroid_display_scan_dt();
		odroid_display_load_cmdline();
		odroid_loaded = true;
	}
	mutex_unlock(&odroid_load_lock);

	return 0;
}
EXPORT_SYMBOL(odroid_display_ensure_loaded);

int odroid_display_count(void)
{
	int count;

	odroid_display_ensure_loaded();
	mutex_lock(&odroid_display_lock);
	count = odroid_num_modes;
	mutex_unlock(&odroid_display_lock);

	return count;
}
EXPORT_SYMBOL(odroid_display_count);

int odroid_display_get_mode(unsigned int index,
		struct drm_display_mode *mode)
{
	int ret = 0;

	if (!mode)
		return -EINVAL;

	odroid_display_ensure_loaded();
	mutex_lock(&odroid_display_lock);
	if (index >= odroid_num_modes)
		ret = -ENOENT;
	else
		*mode = odroid_modes[index];
	mutex_unlock(&odroid_display_lock);

	return ret;
}
EXPORT_SYMBOL(odroid_display_get_mode);

int odroid_display_find_by_name(const char *name,
		struct drm_display_mode *mode)
{
	int index;
	int ret = 0;

	if (!name || !mode)
		return -EINVAL;

	odroid_display_ensure_loaded();
	mutex_lock(&odroid_display_lock);
	index = odroid_display_find_index_locked(name);
	if (index < 0)
		ret = index;
	else
		*mode = odroid_modes[index];
	mutex_unlock(&odroid_display_lock);

	return ret;
}
EXPORT_SYMBOL(odroid_display_find_by_name);

const char *odroid_display_preferred_name(void)
{
	odroid_display_ensure_loaded();
	return odroid_preferred[0] ? odroid_preferred : NULL;
}
EXPORT_SYMBOL(odroid_display_preferred_name);

bool odroid_display_has_mode(const char *name)
{
	bool found;

	odroid_display_ensure_loaded();
	mutex_lock(&odroid_display_lock);
	found = odroid_display_find_index_locked(name) >= 0;
	mutex_unlock(&odroid_display_lock);

	return found;
}
EXPORT_SYMBOL(odroid_display_has_mode);

bool odroid_display_force_single(void)
{
	odroid_display_ensure_loaded();
	return odroid_force_single_mode;
}
EXPORT_SYMBOL(odroid_display_force_single);

static int __init odroid_display_init(void)
{
	int ret;

	ret = odroid_display_register_dt_driver();
	if (ret)
		return ret;

	odroid_display_ensure_loaded();
	return odroid_display_register_debugfs();
}
module_init(odroid_display_init);

static void __exit odroid_display_exit(void)
{
	odroid_display_unregister_debugfs();
	odroid_display_unregister_dt_driver();
}
module_exit(odroid_display_exit);

MODULE_DESCRIPTION("ODROID custom display mode registry");
MODULE_LICENSE("GPL");
