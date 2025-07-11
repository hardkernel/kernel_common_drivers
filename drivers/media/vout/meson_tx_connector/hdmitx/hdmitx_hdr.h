/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMITX_HDR_H
#define __HDMITX_HDR_H

#include <linux/amlogic/media/vout/meson_tx_connector/hdmitx_common/hdmitx_common.h>
#include <linux/amlogic/media/vout/vinfo.h>

void hdmitx_hdr_load_hw_state(struct hdmitx_common *tx_comm);
struct meson_tx_hdr *meson_tx_hdr_create(struct hdmitx_common *tx_comm);
int meson_tx_hdr_destroy(struct meson_tx_hdr *tx_hdr);
int meson_tx_hdr_disable(struct meson_tx_hdr *tx_hdr);
int meson_tx_hdr_dump(struct meson_tx_hdr *tx_hdr, struct seq_file *s);

/* vout server api for vpp */
void hdmitx_set_drm_pkt(void *tx_instance, struct master_display_info_s *data);
void hdmitx_set_hdr10plus_pkt(void *tx_instance, unsigned int flag, struct hdr10plus_para *data);
void hdmitx_set_vsif_pkt(void *tx_instance, enum eotf_type type, enum mode_type
	tunnel_mode, struct dv_vsif_para *data, bool signal_sdr);
void hdmitx_set_cuva_hdr_vsif(void *tx_instance, struct cuva_hdr_vsif_para *data);
void hdmitx_set_cuva_hdr_vs_emds(void *tx_instance, struct cuva_hdr_vs_emds_para *data);
void hdmitx_set_sbtm_pkt(void *tx_instance, struct vtem_sbtm_st *data);
void hdmitx_sync_input_vpp_info(void *tx_instance);

#endif
