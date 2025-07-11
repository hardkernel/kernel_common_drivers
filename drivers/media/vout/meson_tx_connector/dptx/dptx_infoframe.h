/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
/*
 * Copyright © 2008 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifndef __DPTX_INFOFRAME_H__
#define __DPTX_INFOFRAME_H__

#include <linux/types.h>
#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>

/**
 * enum colorimetry_format - drm DP Pixel encoding formats
 *
 * This enum is used to indicate DP VSC SDP Pixel encoding formats.
 * It is based on DP 1.4 spec [Table 2-117: VSC SDP Payload for DB16 through
 * DB18]
 *
 * @COLORIMETRY_FORMAT_RGB: RGB pixel encoding format
 * @COLORIMETRY_FORMAT_YUV444: YCbCr 4:4:4 pixel encoding format
 * @COLORIMETRY_FORMAT_YUV422: YCbCr 4:2:2 pixel encoding format
 * @COLORIMETRY_FORMAT_YUV420: YCbCr 4:2:0 pixel encoding format
 * @COLORIMETRY_FORMAT_Y_ONLY: Y Only pixel encoding format
 * @COLORIMETRY_FORMAT_RAW: RAW pixel encoding format
 * @COLORIMETRY_FORMAT_RESERVED: Reserved pixel encoding format
 */
enum colorimetry_format {
	COLORIMETRY_FORMAT_RGB = 0,
	COLORIMETRY_FORMAT_YUV444 = 0x1,
	COLORIMETRY_FORMAT_YUV422 = 0x2,
	COLORIMETRY_FORMAT_YUV420 = 0x3,
	COLORIMETRY_FORMAT_Y_ONLY = 0x4,
	COLORIMETRY_FORMAT_RAW = 0x5,
	COLORIMETRY_FORMAT_MAX = 0x6,
};

u32 dptx_calc_bpp(enum colorimetry_format color, u8 colordepth);

#endif /* __DPTX_INFOFRAME_H__ */
