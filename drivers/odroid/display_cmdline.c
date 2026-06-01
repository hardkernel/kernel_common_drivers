// SPDX-License-Identifier: GPL-2.0
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "display_internal.h"

static char *odroid_display_dup_cmdline(void)
{
	const char *bootargs;

	if (!of_chosen)
		return NULL;
	if (of_property_read_string(of_chosen, "bootargs", &bootargs) ||
	    !bootargs || !bootargs[0])
		return NULL;

	return kstrdup(bootargs, GFP_KERNEL);
}

static void odroid_display_select_mode(char *value)
{
	char *opt;

	if (!value || !value[0])
		return;

	odroid_display_set_force_single(false, true);

	opt = strchr(value, ':');
	if (opt) {
		*opt++ = '\0';
		if (!strcmp(opt, "force"))
			odroid_display_set_force_single(true, true);
	}

	odroid_display_set_preferred(value);
}

void odroid_display_load_cmdline(void)
{
	char *cmdline, *token;
	char *cursor;

	cmdline = odroid_display_dup_cmdline();
	if (!cmdline)
		return;

	cursor = cmdline;
	while ((token = strsep(&cursor, " ")) != NULL) {
		struct drm_display_mode mode;
		char *value;

		if (!token[0])
			continue;
		if (!strncmp(token, "displaymode=", 12)) {
			odroid_display_select_mode(token + 12);
		} else if (!strncmp(token, "odroid.displaymode=", 19)) {
			odroid_display_select_mode(token + 19);
		} else if (!strncmp(token, "aml_drm.displaymode=", 20)) {
			odroid_display_select_mode(token + 20);
		} else if (!strncmp(token, "video-timing=", 13)) {
			value = token + 13;
			if (!odroid_display_parse_modeline(value, &mode))
				odroid_display_add_mode(&mode, false, false);
		}
	}

	kfree(cmdline);
}
