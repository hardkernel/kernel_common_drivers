// SPDX-License-Identifier: GPL-2.0
#include <linux/debugfs.h>
#include <linux/gcd.h>
#include <linux/kernel.h>
#include <linux/math64.h>
#include <linux/seq_file.h>

#include "display_internal.h"

static struct dentry *odroid_debugfs_dir;

static const char *odroid_sync_pol(unsigned int flags, unsigned int pos,
		unsigned int neg)
{
	if (flags & pos)
		return "P";
	if (flags & neg)
		return "N";
	return "N";
}

static const char *odroid_modeline_pol(unsigned int flags, unsigned int pos,
		unsigned int neg, const char *p, const char *n)
{
	if (flags & pos)
		return p;
	if (flags & neg)
		return n;
	return n;
}

static void odroid_custom_display_show_mode(struct seq_file *s,
		const struct drm_display_mode *mode, const char *preferred, bool fixed)
{
	u32 hfront, hsync, hback;
	u32 vfront, vsync, vback;
	u32 htotal, vtotal;
	u32 aspect_div, aspect_w, aspect_h;
	u64 refresh_mhz;
	u64 hfreq_hz;

	htotal = mode->htotal;
	vtotal = mode->vtotal;
	if (!mode->clock || !mode->hdisplay || !mode->vdisplay ||
	    !htotal || !vtotal)
		return;

	hfront = mode->hsync_start - mode->hdisplay;
	hsync = mode->hsync_end - mode->hsync_start;
	hback = mode->htotal - mode->hsync_end;
	vfront = mode->vsync_start - mode->vdisplay;
	vsync = mode->vsync_end - mode->vsync_start;
	vback = mode->vtotal - mode->vsync_end;

	aspect_div = gcd(mode->hdisplay, mode->vdisplay);
	if (!aspect_div)
		aspect_div = 1;
	aspect_w = mode->hdisplay / aspect_div;
	aspect_h = mode->vdisplay / aspect_div;

	refresh_mhz = div_u64((u64)mode->clock * 1000000000ULL,
				    (u64)htotal * vtotal);
	hfreq_hz = div_u64((u64)mode->clock * 1000ULL, htotal);

	seq_printf(s,
		"DTD:  %ux%u    %llu.%03llu Hz %u:%u   %llu.%03llu kHz  %u.%03u MHz\n",
		mode->hdisplay, mode->vdisplay,
		refresh_mhz / 1000, refresh_mhz % 1000,
		aspect_w, aspect_h,
		hfreq_hz / 1000, hfreq_hz % 1000,
		mode->clock / 1000, mode->clock % 1000);
	seq_printf(s,
		"      Hfront %4u Hsync %3u Hback %4u Hpol %s\n",
		hfront, hsync, hback,
		odroid_sync_pol(mode->flags, DRM_MODE_FLAG_PHSYNC,
			DRM_MODE_FLAG_NHSYNC));
	seq_printf(s,
		"      Vfront %4u Vsync %3u Vback %4u Vpol %s\n",
		vfront, vsync, vback,
		odroid_sync_pol(mode->flags, DRM_MODE_FLAG_PVSYNC,
			DRM_MODE_FLAG_NVSYNC));
	seq_printf(s,
		"      Modeline \"%s\" %u.%03u %u %u %u %u %u %u %u %u %s %s%s\n",
		mode->name, mode->clock / 1000, mode->clock % 1000,
		mode->hdisplay, mode->hsync_start, mode->hsync_end,
		mode->htotal, mode->vdisplay, mode->vsync_start,
		mode->vsync_end, mode->vtotal,
		odroid_modeline_pol(mode->flags, DRM_MODE_FLAG_PHSYNC,
			DRM_MODE_FLAG_NHSYNC, "+hsync", "-hsync"),
		odroid_modeline_pol(mode->flags, DRM_MODE_FLAG_PVSYNC,
			DRM_MODE_FLAG_NVSYNC, "+vsync", "-vsync"),
		(mode->flags & DRM_MODE_FLAG_INTERLACE) ? " interlace" : "");
	seq_printf(s, "      Source flags:%s%s\n\n",
		!strcmp(mode->name, preferred ?: "") ? " preferred" : "",
		fixed ? " fixed" : "");
}

static int odroid_custom_display_show(struct seq_file *s, void *unused)
{
	const char *preferred = odroid_display_preferred_name();
	bool fixed = odroid_display_force_single();
	struct drm_display_mode mode;
	int count = odroid_display_count();
	int i;

	seq_puts(s, "ODROID Custom Display Modes\n");
	seq_puts(s, "===========================\n");
	seq_printf(s, "Known modes: %d\n", count);
	seq_printf(s, "Preferred: %s\n", preferred ?: "none");
	seq_printf(s, "Fixed single mode: %s\n\n", fixed ? "yes" : "no");

	for (i = 0; i < count; i++) {
		if (!odroid_display_get_mode(i, &mode))
			odroid_custom_display_show_mode(s, &mode, preferred, fixed);
	}

	return 0;
}
DEFINE_SHOW_ATTRIBUTE(odroid_custom_display);

int odroid_display_register_debugfs(void)
{
	odroid_debugfs_dir = debugfs_create_dir("odroid", NULL);
	if (IS_ERR_OR_NULL(odroid_debugfs_dir))
		return odroid_debugfs_dir ? PTR_ERR(odroid_debugfs_dir) : -ENOMEM;

	debugfs_create_file("custom_display", 0444, odroid_debugfs_dir, NULL,
		&odroid_custom_display_fops);

	return 0;
}

void odroid_display_unregister_debugfs(void)
{
	debugfs_remove_recursive(odroid_debugfs_dir);
	odroid_debugfs_dir = NULL;
}
