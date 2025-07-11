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

#ifndef _DPTX_AUX_HELPER_H_
#define _DPTX_AUX_HELPER_H_

#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>

struct dptx_aux;

#define AUX_RETRY_INTERVAL 500 /* us */

#define EDID_READ_MAX_RETRY 5
#define DPCD_ACCESS_MAX_RETRY 32

struct dptx_hw_common;

struct dptx_aux_msg {
	unsigned int address;
	u8 request;
	u8 reply;
	void *buffer;
	size_t size;
};

struct dptx_aux {
	struct dptx_hw_common *hw_comm;

	/**
	 * @name: user-visible name of this AUX channel and the
	 * I2C-over-AUX adapter.
	 *
	 * It's also used to specify the name of the I2C adapter. If set
	 * to %NULL, dev_name() of @dev will be used.
	 */
	const char *name;

	/**
	 * @hw_mutex: internal mutex used for locking transfers.
	 *
	 * Note that if the underlying hardware is shared among multiple
	 * channels, the driver needs to do additional locking to
	 * prevent concurrent access.
	 */
	struct mutex hw_mutex;

	/**
	 * @i2c_nack_count: Counts I2C NACKs, used for DP validation.
	 */
	unsigned int i2c_nack_count;
	/**
	 * @i2c_defer_count: Counts I2C DEFERs, used for DP validation.
	 */
	unsigned int i2c_defer_count;
};

struct dptx_aux *dptx_aux_init(struct dptx_hw_common *tx_comm);

ssize_t dptx_aux_read_dpcd(struct dptx_aux *aux, unsigned int offset,
			 void *buffer, size_t size);
ssize_t dptx_aux_write_dpcd(struct dptx_aux *aux, unsigned int offset,
			  void *buffer, size_t size);

static inline ssize_t dptx_aux_readb_dpcd(struct dptx_aux *aux,
					unsigned int offset, u8 *valuep)
{
	return dptx_aux_read_dpcd(aux, offset, valuep, 1);
}

static inline ssize_t dptx_aux_writeb_dpcd(struct dptx_aux *aux,
					 unsigned int offset, u8 value)
{
	return dptx_aux_write_dpcd(aux, offset, &value, 1);
}

int dptx_aux_read_dpcd_caps(struct dptx_aux *aux, u8 *dpcd, size_t size);

int dptx_aux_read_edid_data(struct dptx_aux *aux, u8 *edid, int size);

#endif /* _DPTX_AUX_HELPER_H_ */
