// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <video/of_display_timing.h>
#include <video/videomode.h>

#include "display_internal.h"

static void odroid_display_from_videomode(struct drm_display_mode *drm_mode,
		const struct videomode *vm, const char *name)
{
	memset(drm_mode, 0, sizeof(*drm_mode));
	strscpy(drm_mode->name, name, sizeof(drm_mode->name));
	drm_mode->type = DRM_MODE_TYPE_DRIVER;
	drm_mode->clock = DIV_ROUND_CLOSEST_ULL(vm->pixelclock, 1000);
	drm_mode->hdisplay = vm->hactive;
	drm_mode->hsync_start = vm->hactive + vm->hfront_porch;
	drm_mode->hsync_end = drm_mode->hsync_start + vm->hsync_len;
	drm_mode->htotal = drm_mode->hsync_end + vm->hback_porch;
	drm_mode->vdisplay = vm->vactive;
	drm_mode->vsync_start = vm->vactive + vm->vfront_porch;
	drm_mode->vsync_end = drm_mode->vsync_start + vm->vsync_len;
	drm_mode->vtotal = drm_mode->vsync_end + vm->vback_porch;

	if (vm->flags & DISPLAY_FLAGS_HSYNC_HIGH)
		drm_mode->flags |= DRM_MODE_FLAG_PHSYNC;
	else
		drm_mode->flags |= DRM_MODE_FLAG_NHSYNC;
	if (vm->flags & DISPLAY_FLAGS_VSYNC_HIGH)
		drm_mode->flags |= DRM_MODE_FLAG_PVSYNC;
	else
		drm_mode->flags |= DRM_MODE_FLAG_NVSYNC;
	if (vm->flags & DISPLAY_FLAGS_INTERLACED)
		drm_mode->flags |= DRM_MODE_FLAG_INTERLACE;
	if (vm->flags & DISPLAY_FLAGS_DOUBLECLK)
		drm_mode->flags |= DRM_MODE_FLAG_DBLCLK;
}

static const char *odroid_display_child_name(struct device_node *timings_np,
		unsigned int index, char *name, size_t size)
{
	struct device_node *child;
	unsigned int i = 0;

	for_each_child_of_node(timings_np, child) {
		if (i++ == index) {
			strscpy(name, child->name, size);
			of_node_put(child);
			return name;
		}
	}

	return NULL;
}

static int odroid_display_add_dt_timing(struct display_timings *disp,
		struct device_node *timings_np, unsigned int index, bool preferred,
		bool fixed)
{
	struct drm_display_mode mode;
	struct videomode vm;
	char name[DRM_DISPLAY_MODE_LEN];
	const char *timing_name;
	int ret;

	ret = videomode_from_timings(disp, &vm, index);
	if (ret)
		return 0;

	timing_name = odroid_display_child_name(timings_np, index, name,
			sizeof(name));
	if (!timing_name || !timing_name[0])
		return 0;

	odroid_display_from_videomode(&mode, &vm, timing_name);
	return !odroid_display_add_mode(&mode, preferred, fixed);
}

int odroid_display_load_dt_node(struct device_node *node)
{
	struct display_timings *disp;
	struct device_node *timings_np;
	unsigned int native;
	bool fixed;
	int count = 0;
	int i;

	if (!node)
		return 0;

	timings_np = of_get_child_by_name(node, "display-timings");
	if (!timings_np)
		return 0;

	disp = of_get_display_timings(node);
	if (!disp) {
		of_node_put(timings_np);
		return 0;
	}

	fixed = of_property_read_bool(timings_np, "fixed-display") ||
		of_property_read_bool(timings_np, "fixed-timing");
	if (fixed && !odroid_display_force_single_explicit())
		odroid_display_set_force_single(true, false);

	native = disp->native_mode;
	if (native >= disp->num_timings)
		native = 0;

	for (i = 0; i < disp->num_timings; i++)
		count += odroid_display_add_dt_timing(disp, timings_np, i,
				i == native, fixed && i == native);

	display_timings_release(disp);
	of_node_put(timings_np);

	return count;
}

static int odroid_display_load_compatible_dt(const char *compatible)
{
	struct device_node *node = NULL;
	int count = 0;

	while ((node = of_find_compatible_node(node, NULL, compatible))) {
		count += odroid_display_load_dt_node(node);
	}

	return count;
}

int odroid_display_scan_dt(void)
{
	struct device_node *node;
	int count = 0;

	/* Preferred path: ODROID-owned display timing node. */
	count += odroid_display_load_compatible_dt("hardkernel,odroid-display");
	count += odroid_display_load_compatible_dt("hardkernel, odroid-display");
	if (count)
		return count;

	/* Fallback path: accept any node carrying a display-timings child. */
	for (node = of_find_all_nodes(NULL); node; node = of_find_all_nodes(node)) {
		struct device_node *timings_np;

		timings_np = of_get_child_by_name(node, "display-timings");
		if (!timings_np)
			continue;
		of_node_put(timings_np);
		count += odroid_display_load_dt_node(node);
	}

	return count;
}

static int odroid_display_probe(struct platform_device *pdev)
{
	odroid_display_load_dt_node(pdev->dev.of_node);
	return 0;
}

static const struct of_device_id odroid_display_of_match[] = {
	{ .compatible = "hardkernel,odroid-display" },
	{ .compatible = "hardkernel, odroid-display" },
	{ }
};
MODULE_DEVICE_TABLE(of, odroid_display_of_match);

static struct platform_driver odroid_display_driver = {
	.probe = odroid_display_probe,
	.driver = {
		.name = "odroid-display",
		.of_match_table = odroid_display_of_match,
	},
};

int odroid_display_register_dt_driver(void)
{
	return platform_driver_register(&odroid_display_driver);
}

void odroid_display_unregister_dt_driver(void)
{
	platform_driver_unregister(&odroid_display_driver);
}
