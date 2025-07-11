// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/media/vout/meson_tx_connector/hdmitx_common/hdmitx_hw_common.h>

/* param:
 * cmd: specific HW operation;
 * input_argv: input param for specific command, maybe a struct pointer;
 * output_struct: return structure for specific command;
 *
 * return: for simple integer value return; if need to return
 * a structure instead of a simple integer value, use output_struct.
 */
int hdmitx_hw_cntl(struct hdmitx_hw_common *tx_hw,
	u32 cmd, void *input_argv, void *output_struct)
{
	if (!tx_hw || !tx_hw->hw_cntl)
		return -EINVAL;
	return tx_hw->hw_cntl(tx_hw, cmd, input_argv, output_struct);
}

int hdmitx_hw_validate_mode(struct hdmitx_hw_common *tx_hw, u32 vic,
	u32 max_refresh_rate)
{
	if (!tx_hw || !tx_hw->validate_mode)
		return -EINVAL;
	return tx_hw->validate_mode(tx_hw, vic, max_refresh_rate);
}

/* calculate clk_ratio/tmds_scramble/frl/dsc */
int hdmitx_hw_calc_format_para(struct hdmitx_hw_common *tx_hw,
	struct meson_tx_format_para *para)
{
	int ret = -1;

	if (!tx_hw || !para)
		return ret;
	if (tx_hw->calc_format_para)
		ret = tx_hw->calc_format_para(tx_hw, para);
	return ret;
}

int hdmitx_hw_set_phy(struct hdmitx_hw_common *tx_hw, int flag)
{
	u32 cmd = TMDS_PHY_ENABLE;

	if (flag == 0)
		cmd = TMDS_PHY_DISABLE;
	else
		cmd = TMDS_PHY_ENABLE;
	return hdmitx_hw_cntl(tx_hw, PLATFORM_PHY_OP, (void *)&cmd, NULL);
}
EXPORT_SYMBOL(hdmitx_hw_set_phy);

int hdmitx_hw_set_vrr_rate(struct hdmitx_hw_common *tx_hw, int rate, void *vrr_info)
{
	if (tx_hw && tx_hw->set_vrr_rate)
		tx_hw->set_vrr_rate(tx_hw, rate, vrr_info);
	return 0;
}

enum hdmi_tf_type hdmitx_hw_get_hdr_st(struct hdmitx_hw_common *tx_hw)
{
	return hdmitx_hw_cntl(tx_hw, AUX_PKT_GET_HDR_ST, NULL, NULL);
}

enum hdmi_tf_type hdmitx_hw_get_dv_st(struct hdmitx_hw_common *tx_hw)
{
	return hdmitx_hw_cntl(tx_hw, AUX_PKT_GET_AMDV_ST, NULL, NULL);
}

enum hdmi_tf_type hdmitx_hw_get_hdr10p_st(struct hdmitx_hw_common *tx_hw)
{
	return hdmitx_hw_cntl(tx_hw, AUX_PKT_GET_HDR10P_ST, NULL, NULL);
}

void hdmitx_register_hw_event_callback(struct hdmitx_hw_common *hw_comm,
				    struct meson_hw_event_ops *event_ops)
{
	hw_comm->tx_hw_base.event_ops = event_ops;
}
