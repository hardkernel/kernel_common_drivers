// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel_read_file.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "display_internal.h"

#define DISPLAY_TIMING_FILENAME "display-timings.txt"
#define DISPLAY_TIMING_FILE_MAX_SIZE 4095

static const char * const odroid_display_files[] = {
	"/fat/" DISPLAY_TIMING_FILENAME,
	"/boot/" DISPLAY_TIMING_FILENAME,
	"/usr/lib/firmware/" DISPLAY_TIMING_FILENAME,
	"/lib/firmware/" DISPLAY_TIMING_FILENAME,
};

int odroid_display_load_files(void)
{
	char *buf, *cursor, *line;
	int count = 0;
	ssize_t size;
	int ret;
	int i;

	buf = kzalloc(DISPLAY_TIMING_FILE_MAX_SIZE + 1, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	for (i = 0; i < ARRAY_SIZE(odroid_display_files); i++) {
		void *file_buf = buf;

		memset(buf, 0, DISPLAY_TIMING_FILE_MAX_SIZE + 1);
		size = kernel_read_file_from_path(odroid_display_files[i], 0,
			&file_buf, DISPLAY_TIMING_FILE_MAX_SIZE, NULL,
			READING_UNKNOWN);
		if (size <= 0)
			continue;
		buf[size] = '\0';

		cursor = buf;
		while ((line = strsep(&cursor, "\n")) != NULL) {
			struct drm_display_mode mode;

			ret = odroid_display_parse_modeline(line, &mode);
			if (ret)
				continue;
			if (!odroid_display_add_mode(&mode, false, false))
				count++;
		}
	}

	kfree(buf);
	return count;
}
