#include <linux/slab.h>
#include <linux/module.h>

#include "odroid_helper.h"

static struct drm_display_mode mode;

static int modeline_set(const char *val, const struct kernel_param *kp)
{
	char *param;

	param = kstrdup(val, GFP_KERNEL);
	if (!param)
		return -ENOMEM;

	drm_display_mode_from_modeline(param, &mode);

	kfree(param);

	return 0;
}

static int modeline_get(char *buffer, const struct kernel_param *kp)
{
	return 0;
}

static const struct kernel_param_ops modeline_ops = {
	.set = modeline_set,
	.get = modeline_get,
};

module_param_cb(modeline, &modeline_ops, NULL, 0644);
__MODULE_PARM_TYPE(modeline, "charp");

int load_odroid_modeline_from_commandline(struct drm_connector *connector)
{
	int ret = append_replace_drm_display_mode(connector,
			drm_mode_duplicate(connector->dev, &mode));

	return (ret == 0) ? 1 : 0;
}
