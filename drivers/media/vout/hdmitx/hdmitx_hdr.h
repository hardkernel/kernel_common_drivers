/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMITX_HDR_H
#define __HDMITX_HDR_H

#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include <linux/amlogic/media/vout/vinfo.h>

void hdmitx_hdr_init(struct hdmitx_common *tx_comm);
void hdr_unmute_work_func(struct work_struct *work);
void hdr_work_func(struct work_struct *work);
/* hdmitx hdr packet api*/
void hdmitx_set_drm_pkt(void *tx_instance, struct master_display_info_s *data);
void hdmitx_set_hdr10plus_pkt(void *tx_instance, unsigned int flag, struct hdr10plus_para *data);
void hdmitx_set_vsif_pkt(void *tx_instance, enum eotf_type type, enum mode_type
	tunnel_mode, struct dv_vsif_para *data, bool signal_sdr);
void hdmitx_set_cuva_hdr_vsif(void *tx_instance, struct cuva_hdr_vsif_para *data);
void hdmitx_set_cuva_hdr_vs_emds(void *tx_instance, struct cuva_hdr_vs_emds_para *data);
void hdmitx_set_sbtm_pkt(void *tx_instance, struct vtem_sbtm_st *data);

void hdmitx_clear_all_infoframe_pkt(struct hdmitx_common *tx_comm);

#endif
