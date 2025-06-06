/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2021 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_PRINT_H
#define __MESON_PRINT_H

#include <linux/printk.h>
#include <drm/drm.h>
#include <drm/drm_print.h>

#define MESON_DRM_UT_FENCE		(DRM_UT_DRMRES + 1)  /* fence message */
#define MESON_DRM_UT_REG		(DRM_UT_DRMRES + 2)  /* register message */
#define MESON_DRM_UT_FBDEV		(DRM_UT_DRMRES + 3)  /* fbdev message */
#define MESON_DRM_UT_TRAVERSE		(DRM_UT_DRMRES + 4)  /* pipeline traverse message */
#define MESON_DRM_UT_BLOCK		(DRM_UT_DRMRES + 5)  /* vpu block message */
#define MESON_DRM_UT_STATE		(DRM_UT_DRMRES + 6)  /* commit state check */

#define MESON_DRM_FENCE(fmt, ...)						\
	__drm_dbg(MESON_DRM_UT_FENCE, fmt, ## __VA_ARGS__)

#define MESON_DRM_REG(fmt, ...)						\
	__drm_dbg(MESON_DRM_UT_REG, fmt, ## __VA_ARGS__)

#define MESON_DRM_FBDEV(fmt, ...)						\
	__drm_dbg(MESON_DRM_UT_FBDEV, fmt, ## __VA_ARGS__)

#define MESON_DRM_TRAVERSE(fmt, ...)						\
	__drm_dbg(MESON_DRM_UT_TRAVERSE, fmt, ## __VA_ARGS__)

#define MESON_DRM_BLOCK(fmt, ...)						\
	__drm_dbg(MESON_DRM_UT_BLOCK, fmt, ## __VA_ARGS__)

#define MESON_DRM_STATE(fmt, ...)						\
	__drm_dbg(MESON_DRM_UT_STATE, fmt, ## __VA_ARGS__)

#endif /* __AM_MESON_DRV_H */
