// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>

/*!!Only one instance supported.*/
static struct hdmitx_common *global_tx_common;

int drm_hdmitx_common_hdcp_init(void)
{
	if (global_tx_common && global_tx_common->drm_hdcp_ctrl_ops &&
		global_tx_common->drm_hdcp_ctrl_ops->hdcp_init) {
		global_tx_common->drm_hdcp_ctrl_ops->hdcp_init();
		return 0;
	}
	return -EINVAL;
}
EXPORT_SYMBOL(drm_hdmitx_common_hdcp_init);

int drm_hdmitx_common_hdcp_exit(void)
{
	if (global_tx_common && global_tx_common->drm_hdcp_ctrl_ops &&
		global_tx_common->drm_hdcp_ctrl_ops->hdcp_init) {
		global_tx_common->drm_hdcp_ctrl_ops->hdcp_exit();
		return 0;
	}
	return -EINVAL;
}
EXPORT_SYMBOL(drm_hdmitx_common_hdcp_exit);

int drm_hdmitx_common_hdcp_enable(int hdcp_type)
{
	if (global_tx_common && global_tx_common->drm_hdcp_ctrl_ops &&
		global_tx_common->drm_hdcp_ctrl_ops->hdcp_init) {
		global_tx_common->drm_hdcp_ctrl_ops->hdcp_enable(hdcp_type);
		return 0;
	}
	return -EINVAL;
}
EXPORT_SYMBOL(drm_hdmitx_common_hdcp_enable);

int drm_hdmitx_common_hdcp_disable(void)
{
	if (global_tx_common && global_tx_common->drm_hdcp_ctrl_ops &&
		global_tx_common->drm_hdcp_ctrl_ops->hdcp_init) {
		global_tx_common->drm_hdcp_ctrl_ops->hdcp_disable();
		return 0;
	}
	return -EINVAL;
}
EXPORT_SYMBOL(drm_hdmitx_common_hdcp_disable);

int drm_hdmitx_common_hdcp_disconnect(void)
{
	if (global_tx_common && global_tx_common->drm_hdcp_ctrl_ops &&
		global_tx_common->drm_hdcp_ctrl_ops->hdcp_init) {
		global_tx_common->drm_hdcp_ctrl_ops->hdcp_disconnect();
		return 0;
	}
	return -EINVAL;
}
EXPORT_SYMBOL(drm_hdmitx_common_hdcp_disconnect);

unsigned int drm_hdmitx_common_get_tx_hdcp_cap(void)
{
	if (global_tx_common && global_tx_common->drm_hdcp_ctrl_ops &&
		global_tx_common->drm_hdcp_ctrl_ops->hdcp_init) {
		return global_tx_common->drm_hdcp_ctrl_ops->get_tx_hdcp_cap();
	}
	return 0;
}
EXPORT_SYMBOL(drm_hdmitx_common_get_tx_hdcp_cap);

unsigned int drm_hdmitx_common_get_rx_hdcp_cap(void)
{
	if (global_tx_common && global_tx_common->drm_hdcp_ctrl_ops &&
		global_tx_common->drm_hdcp_ctrl_ops->hdcp_init) {
		return global_tx_common->drm_hdcp_ctrl_ops->get_rx_hdcp_cap();
	}
	return 0;
}
EXPORT_SYMBOL(drm_hdmitx_common_get_rx_hdcp_cap);

int drm_hdmitx_common_register_hdcp_notify(struct connector_hdcp_cb *cb)
{
	if (global_tx_common && global_tx_common->drm_hdcp_ctrl_ops &&
		global_tx_common->drm_hdcp_ctrl_ops->hdcp_init) {
		global_tx_common->drm_hdcp_ctrl_ops->register_hdcp_notify(cb);
		return 0;
	}
	return -EINVAL;
}
EXPORT_SYMBOL(drm_hdmitx_common_register_hdcp_notify);

int drm_hdmitx_common_get_dw_hdcp_topo_info(void)
{
	if (global_tx_common && global_tx_common->drm_hdcp_ctrl_ops &&
		global_tx_common->drm_hdcp_ctrl_ops->hdcp_init) {
		global_tx_common->drm_hdcp_ctrl_ops->get_dw_hdcp_topo_info();
		return 0;
	}
	return -EINVAL;
}
EXPORT_SYMBOL(drm_hdmitx_common_get_dw_hdcp_topo_info);

void set_hdcp_common_instance(struct hdmitx_common *tx_comm)
{
	if (tx_comm)
		global_tx_common = tx_comm;
}
