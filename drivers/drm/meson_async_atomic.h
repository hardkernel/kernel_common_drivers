/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_ASYNC_ATOMIC_H__
#define __MESON_ASYNC_ATOMIC_H__

#include <drm/drm_atomic.h>

int meson_async_atomic_ioctl(struct drm_device *dev,
			  void *data, struct drm_file *file_priv);
bool is_am_osd_async_commit(struct drm_atomic_state *atomic_state);

#endif
