/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPTX_INFOFRAME_H__
#define __DPTX_INFOFRAME_H__

#include <linux/amlogic/media/vout/meson_tx_connector/dptx_common/dptx_common.h>
#include "cea861.h"

struct dp_sdp_header {
	u8 HB0;
	u8 HB1;
	u8 HB2;
	u8 HB3;
} __packed;

struct dp_sdp {
	struct dp_sdp_header sdp_header;
	u8 data[DP_INFOFRAME_DB_MAX_SZ];
} __packed;

/* convert struct aux_pkt_sdp_param to pointer for AUX_PKT_SET_SDP and AUX_PKT_SET_SDP */
struct aux_pkt_sdp_param {
	u8 vc_id;
	enum infoframe_type type;
	u8 *data;
};

#define DP_SDP_AUDIO_INFOFRAME_HB2       0x1b
#define DP_SDP_NON_AUDIO_INFOFRAME_HB2   0x1d

u32 dptx_calc_bpp(enum colorimetry_format color, u8 colordepth);
u8 dptx_get_color_component_cnt(enum colorimetry_format color);
u8 dptx_get_mapped_bpc(enum hdmi_color_depth cd);
void dptx_avi_infoframe_set(struct dptx_hw_common *hw_comm, u8 vc_id, struct avi_infoframe *info);
void dptx_avi_infoframe_rawset(struct dptx_hw_common *hw_comm, u8 vc_id, u8 *hb, u8 *pb);
void dptx_spd_infoframe_set(struct dptx_hw_common *hw_comm, u8 vc_id, struct spd_infoframe *info);
void dptx_spd_infoframe_rawset(struct dptx_hw_common *hw_comm, u8 vc_id, u8 *hb, u8 *pb);
void dptx_audio_infoframe_set(struct dptx_hw_common *hw_comm, u8 vc_id,
	struct audio_infoframe *info);
void dptx_audio_infoframe_rawset(struct dptx_hw_common *hw_comm, u8 vc_id, u8 *hb, u8 *pb);
void dptx_drm_infoframe_set(struct dptx_hw_common *hw_comm, u8 vc_id, struct drm_infoframe *info);
void dptx_drm_infoframe_rawset(struct dptx_hw_common *hw_comm, u8 vc_id, u8 *hb, u8 *pb);

#endif /* __DPTX_INFOFRAME_H__ */
