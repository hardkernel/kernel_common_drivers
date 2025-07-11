/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMITX_INFOFRAME_H
#define __HDMITX_INFOFRAME_H

struct hdmi_packet_t {
	u8 hb[3];
	u8 pb[28];
	/* padding to 32 bytes */
	u8 no_used;
};

/* avi parse/compose */
int hdmi_avi_infoframe_unpack_renew(struct hdmi_avi_infoframe *frame,
	const void *buffer, size_t size);
ssize_t hdmi_avi_infoframe_pack_renew(struct hdmi_avi_infoframe *frame,
				void *buffer, size_t size);
#endif
