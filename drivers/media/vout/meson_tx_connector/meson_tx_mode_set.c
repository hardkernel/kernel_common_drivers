// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_dev.h>
#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_hw.h>

#include "meson_tx_task_mgr.h"
#include "meson_tx_internal.h"

static int meson_tx_pre_enable_mode(struct meson_tx_dev *tx_dev, struct meson_tx_format_para *para)
{
	struct meson_tx_log *tx_log = meson_get_tx_log(tx_dev);

	if (!tx_dev || !para) {
		TX_ERROR(tx_log, "[%s]: invalid input\n", __func__);
		return -EINVAL;
	}

	if (tx_dev->ready)
		TX_ERROR(tx_log, "Should run disable_mode before enable new mode.\n");

	if (tx_dev->hpd_state == 0 || tx_dev->suspend_flag) {
		TX_ERROR(tx_log, "%s current hpd_state/suspend (%d,%d), exit\n",
			__func__, tx_dev->hpd_state, tx_dev->suspend_flag);
		return -1;
	}

	/* to replace sw_fmt_para with meson_tx_format_para */
	memcpy(&tx_dev->sw_fmt_para, para, sizeof(struct meson_tx_format_para));

	if (tx_dev->ops->pre_mode_enable)
		tx_dev->ops->pre_mode_enable(tx_dev, para);

	return 0;
}

static int meson_tx_enable_mode(struct meson_tx_dev *tx_dev,
				     struct meson_tx_format_para *para)
{
	struct meson_tx_log *tx_log = meson_get_tx_log(tx_dev);

	if (!tx_dev || !tx_dev->tx_hw_base) {
		TX_ERROR(tx_log, "[%s]: invalid input\n", __func__);
		return -EINVAL;
	}

	if (tx_dev->ops->mode_enable)
		tx_dev->ops->mode_enable(tx_dev, para);

	return 0;
}

static int meson_tx_post_enable_mode(struct meson_tx_dev *tx_dev,
					  struct meson_tx_format_para *para)
{
	struct meson_tx_log *tx_log = meson_get_tx_log(tx_dev);
	//struct meson_tx_state *tx_state = to_meson_tx_state(para);

	if (!tx_dev || !tx_dev->tx_hw_base) {
		TX_ERROR(tx_log, "[%s]: invalid input\n", __func__);
		return -EINVAL;
	}

	/*
	 * attach vinfo, if hdr_cap and dv_cap change, the HDR/DV module will
	 * call the packet sending function, need to set ready flag to 1 first
	 */
	tx_dev->ready = 1;

	if (tx_dev->ops->post_mode_enable)
		tx_dev->ops->post_mode_enable(tx_dev, para);

	meson_tx_update_vinfo(tx_dev);

	return 0;
}

int meson_tx_do_mode_setting(struct meson_tx_dev *tx_dev,
				  struct meson_tx_state *new_state,
				  struct meson_tx_state *old_state)
{
	int ret = 0;
	struct meson_tx_format_para *new_para;
	struct meson_tx_log *tx_log = meson_get_tx_log(tx_dev);

	if (!tx_dev || !new_state) {
		TX_ERROR(tx_log, "[%s]: invalid input\n", __func__);
		return -EINVAL;
	}
	new_para = &new_state->para;

	if (new_state->mode & VMODE_INIT_BIT_MASK) {
		TX_INFO(tx_log, "skip real mode setting for uboot init\n");
		/* note that for bootup, post_enable_mode()
		 * action will be done in tx_set_current_vmode()
		 * when vout probe, it's earlier than drm to
		 * call hdmitx_common_do_mode_setting(), and
		 * thus VPP/DV won't miss dv/hdr cap in vinfo
		 */
		return ret;
	}

	mutex_lock(&tx_dev->set_mode_mutex);
	if (new_state->sequence_id != tx_dev->hw_sequence_id) {
		TX_ERROR(tx_log, "sequence_id failed %lld\n", new_state->sequence_id);
		goto fail;
	}

	ret = meson_tx_pre_enable_mode(tx_dev, new_para);
	if (ret < 0) {
		TX_ERROR(tx_log, "pre mode enable fail\n");
		goto fail;
	}

	ret = meson_tx_enable_mode(tx_dev, new_para);
	if (ret < 0) {
		TX_ERROR(tx_log, "mode enable fail\n");
		goto fail;
	}

	ret = meson_tx_post_enable_mode(tx_dev, new_para);
	if (ret < 0) {
		TX_ERROR(tx_log, "post mode enable fail\n");
		goto fail;
	}

fail:
	mutex_unlock(&tx_dev->set_mode_mutex);
	return ret;
}
EXPORT_SYMBOL(meson_tx_do_mode_setting);
