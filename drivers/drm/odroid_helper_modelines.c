#include "odroid_helper.h"

int load_odroid_modelines(struct drm_connector *connector)
{
	struct drm_display_mode mode;
	struct file *file;
	unsigned long len = 4096;
	loff_t pos = 0;
	char *buf;
	char *str;
	int ret;
	int count = 0;

	file = filp_open("/usr/lib/firmware/display-timings.txt", O_RDONLY, 0);
	if (IS_ERR(file))
		return PTR_ERR(file);

	buf = kzalloc(len, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	ret = kernel_read(file, buf, len, &pos);
	if (ret <= 0)
		goto err_load_odroid_modelines;

	while ((str = strsep(&buf, "\n")) != NULL) {
		ret = drm_display_mode_from_modeline(str, &mode);
		if (ret)
			continue;

		ret = append_replace_drm_display_mode(connector, &mode);
		if (!ret)
			count++;
	}

err_load_odroid_modelines:
	filp_close(file, NULL);
	kfree(buf);

	return count;
}

MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
