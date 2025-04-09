// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>

/* use drm */
int drm_hdmitx_common_hdcp_init(struct hdmitx_common *tx_comm)
{
	if (tx_comm && tx_comm->drm_hdcp_ctrl_ops &&
		tx_comm->drm_hdcp_ctrl_ops->hdcp_init) {
		tx_comm->drm_hdcp_ctrl_ops->hdcp_init();
		return 0;
	}
	return -EINVAL;
}
EXPORT_SYMBOL(drm_hdmitx_common_hdcp_init);

int drm_hdmitx_common_hdcp_exit(struct hdmitx_common *tx_comm)
{
	if (tx_comm && tx_comm->drm_hdcp_ctrl_ops &&
		tx_comm->drm_hdcp_ctrl_ops->hdcp_exit) {
		tx_comm->drm_hdcp_ctrl_ops->hdcp_exit();
		return 0;
	}
	return -EINVAL;
}
EXPORT_SYMBOL(drm_hdmitx_common_hdcp_exit);

int drm_hdmitx_common_hdcp_enable(struct hdmitx_common *tx_comm, int hdcp_type)
{
	if (tx_comm && tx_comm->drm_hdcp_ctrl_ops &&
		tx_comm->drm_hdcp_ctrl_ops->hdcp_enable) {
		tx_comm->drm_hdcp_ctrl_ops->hdcp_enable(hdcp_type);
		return 0;
	}
	return -EINVAL;
}
EXPORT_SYMBOL(drm_hdmitx_common_hdcp_enable);

int drm_hdmitx_common_hdcp_disable(struct hdmitx_common *tx_comm)
{
	if (tx_comm && tx_comm->drm_hdcp_ctrl_ops &&
		tx_comm->drm_hdcp_ctrl_ops->hdcp_disable) {
		tx_comm->drm_hdcp_ctrl_ops->hdcp_disable();
		return 0;
	}
	return -EINVAL;
}
EXPORT_SYMBOL(drm_hdmitx_common_hdcp_disable);

int drm_hdmitx_common_hdcp_disconnect(struct hdmitx_common *tx_comm)
{
	if (tx_comm && tx_comm->drm_hdcp_ctrl_ops &&
		tx_comm->drm_hdcp_ctrl_ops->hdcp_disconnect) {
		tx_comm->drm_hdcp_ctrl_ops->hdcp_disconnect();
		return 0;
	}
	return -EINVAL;
}
EXPORT_SYMBOL(drm_hdmitx_common_hdcp_disconnect);

unsigned int drm_hdmitx_common_get_tx_hdcp_cap(struct hdmitx_common *tx_comm)
{
	if (tx_comm && tx_comm->drm_hdcp_ctrl_ops &&
		tx_comm->drm_hdcp_ctrl_ops->get_tx_hdcp_cap) {
		return tx_comm->drm_hdcp_ctrl_ops->get_tx_hdcp_cap();
	}
	return 0;
}
EXPORT_SYMBOL(drm_hdmitx_common_get_tx_hdcp_cap);

unsigned int drm_hdmitx_common_get_rx_hdcp_cap(struct hdmitx_common *tx_comm)
{
	if (tx_comm && tx_comm->drm_hdcp_ctrl_ops &&
		tx_comm->drm_hdcp_ctrl_ops->get_rx_hdcp_cap) {
		return tx_comm->drm_hdcp_ctrl_ops->get_rx_hdcp_cap();
	}
	return 0;
}
EXPORT_SYMBOL(drm_hdmitx_common_get_rx_hdcp_cap);

int drm_hdmitx_common_register_hdcp_notify(struct hdmitx_common *tx_comm,
					   struct connector_hdcp_cb *cb)
{
	if (tx_comm && tx_comm->drm_hdcp_ctrl_ops &&
		tx_comm->drm_hdcp_ctrl_ops->register_hdcp_notify) {
		tx_comm->drm_hdcp_ctrl_ops->register_hdcp_notify(cb);
		return 0;
	}
	return -EINVAL;
}
EXPORT_SYMBOL(drm_hdmitx_common_register_hdcp_notify);

int drm_hdmitx_common_get_dw_hdcp_topo_info(struct hdmitx_common *tx_comm)
{
	if (tx_comm && tx_comm->drm_hdcp_ctrl_ops &&
		tx_comm->drm_hdcp_ctrl_ops->get_dw_hdcp_topo_info) {
		tx_comm->drm_hdcp_ctrl_ops->get_dw_hdcp_topo_info();
		return 0;
	}
	return -EINVAL;
}
EXPORT_SYMBOL(drm_hdmitx_common_get_dw_hdcp_topo_info);
/* use drm end */

void hdmitx_set_hdcp_mode(struct hdmitx_common *tx_comm, const char *buf)
{
	if (tx_comm && tx_comm->hdcp_ctrl_ops->set_hdcp_mode)
		tx_comm->hdcp_ctrl_ops->set_hdcp_mode(tx_comm, buf);
}

int hdmitx_get_hdcp_ver(struct hdmitx_common *tx_comm, char *buf, int len)
{
	if (tx_comm && tx_comm->hdcp_ctrl_ops->get_hdcp_ver)
		return tx_comm->hdcp_ctrl_ops->get_hdcp_ver(tx_comm, buf, len);
	else
		return -EINVAL;
}
