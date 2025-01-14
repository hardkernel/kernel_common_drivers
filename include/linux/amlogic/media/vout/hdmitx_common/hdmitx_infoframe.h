/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMITX_INFOFRAME_H
#define __HDMITX_INFOFRAME_H

#include <linux/hdmi.h>
#include <uapi/drm/drm_mode.h>

#define HDMI_INFOFRAME_TYPE_SBTM 0xA //SBTM-EM PKT use GEN5
void hdmitx_infoframe_send(u16 info_type, u8 *body);
int hdmitx_infoframe_rawget(u16 info_type, u8 *body);
void hdmitx_hdr_init(struct hdmitx_common *tx_comm);
int hdmi_avi_infoframe_unpack_renew(struct hdmi_avi_infoframe *frame,
		const void *buffer, size_t size);
void hdmi_avi_infoframe_set(struct hdmi_avi_infoframe *info);
void hdmi_avi_infoframe_sync_from_uboot(struct hdmi_avi_infoframe *frame);

int hdmi_avi_infoframe_config(enum avi_component_conf conf, u8 val);
void hdr_unmute_work_func(struct work_struct *work);
void hdr_work_func(struct work_struct *work);
/* hdmitx hdr packet api*/
void hdmitx_set_drm_pkt(struct master_display_info_s *data);
void hdmitx_set_hdr10plus_pkt(unsigned int flag, struct hdr10plus_para *data);
void hdmitx_set_vsif_pkt(enum eotf_type type, enum mode_type
	tunnel_mode, struct dv_vsif_para *data, bool signal_sdr);
void hdmitx_set_cuva_hdr_vsif(struct cuva_hdr_vsif_para *data);

void hdmitx_clear_all_infoframe_pkt(struct hdmitx_common *tx_comm);
/* CONF_AVI_BT2020 */
#define CLR_AVI_BT2020      0
#define SET_AVI_BT2020      1
/* CONF_AVI_Q01 */
#define RGB_RANGE_DEFAULT   0
#define RGB_RANGE_LIM       1
#define RGB_RANGE_FUL       2
#define RGB_RANGE_RSVD      3
/* CONF_AVI_YQ01 */
#define YCC_RANGE_LIM       0
#define YCC_RANGE_FUL       1
#define YCC_RANGE_RSVD      2

#endif
