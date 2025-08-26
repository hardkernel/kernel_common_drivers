/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __AM_MESON_LOGO_H
#define __AM_MESON_LOGO_H
#include <linux/stdarg.h>

#define VMODE_NAME_LEN_MAX    64

struct am_meson_logo {
	struct page *logo_page;
	void *vaddr;
	phys_addr_t start;
	int panel_index;
	int vpp_index;
	u32 size;
	u32 width;
	u32 height;
	u32 bpp;
	/**
	 * @alloc_flag
	 *
	 * if logo buffer have been released.
	 * 1:not released.
	 * 0:released.
	 */
	u32 alloc_flag;
	u32 info_loaded_mask;
	u32 osd_reverse;
	u32 vmode;
	u32 debug;
	u32 loaded;
	char *outputmode_t;
	char outputmode[VMODE_NAME_LEN_MAX];
	bool is_std;
	bool is_cma;
	/**
	 * @plane_has_fb
	 *
	 * if plane attach valid fb to ensure that the
	 * logo is released at the correct time in std mode.
	 * 1:attach valid fb and logo will be released.
	 * 0:not attach valid fb and logo will not be released.
	 */
	bool plane_has_fb;
};

enum osd_dev_e {
	DEV_OSD0 = 0,
	DEV_OSD1,
	DEV_OSD2,
	DEV_OSD3,
	DEV_ALL,
	DEV_MAX
};

#ifndef CONFIG_AMLOGIC_MEDIA_FB
struct osd_info_s {
	u32 index;
	u32 osd_reverse;
};

struct para_osd_info_s {
	char *name;
	u32 info;
	u32 prev_idx;
	u32 next_idx;
	u32 cur_group_start;
	u32 cur_group_end;
};

enum reverse_info_e {
	REVERSE_FALSE = 0,
	REVERSE_TRUE,
	REVERSE_X,
	REVERSE_Y,
	REVERSE_MAX
};

void drm_logo_get_osd_reverse(u32 *index, u32 *reverse_type);
#endif

void am_meson_logo_init(struct drm_device *dev);
void am_meson_free_logo_memory(void);
void am_meson_logo_cma_alloc(struct device *dev, int logo_init);
void am_meson_logo_cma_mem_reset_zero(struct am_meson_logo *logo);
void am_meson_drm_put_logo_fb(struct drm_device *dev,
	int index, int uboot_mode_init);

#endif

