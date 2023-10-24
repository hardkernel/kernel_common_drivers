/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMITX_TRACER_H
#define __HDMITX_TRACER_H

/* For Debug:
 * write event to fifo, and userspace can read by sysfs.
 */
#define HDMITX_HDMI_ERROR_MASK BIT(16)
enum hdmitx_event_log_bits {
	HDMITX_HPD_PLUGOUT                      = 1,
	HDMITX_HPD_PLUGIN                       = 2,
	/* EDID states */
	HDMITX_EDID_HDMI_DEVICE                 = 3,
	HDMITX_EDID_DVI_DEVICE                  = 4,
	/* HDCP states */
	HDMITX_HDCP_AUTH_SUCCESS                = 5,
	HDMITX_HDCP_AUTH_FAILURE                = 6,
	HDMITX_HDCP_HDCP_1_ENABLED              = 7,
	HDMITX_HDCP_HDCP_2_ENABLED              = 8,
	HDMITX_HDCP_NOT_ENABLED                 = 9,
	/* Output states */
	HDMITX_KMS_DISABLE_OUTPUT               = 10,
	HDMITX_KMS_ENABLE_OUTPUT                = 11,

	/* EDID errors */
	HDMITX_EDID_HEAD_ERROR                  = HDMITX_HDMI_ERROR_MASK | 1,
	HDMITX_EDID_CHECKSUM_ERROR              = HDMITX_HDMI_ERROR_MASK | 2,
	HDMITX_EDID_I2C_ERROR                   = HDMITX_HDMI_ERROR_MASK | 3,
	/* HDCP errors */
	HDMITX_HDCP_DEVICE_NOT_READY_ERROR      = HDMITX_HDMI_ERROR_MASK | 4,
	HDMITX_HDCP_AUTH_NO_14_KEYS_ERROR       = HDMITX_HDMI_ERROR_MASK | 5,
	HDMITX_HDCP_AUTH_NO_22_KEYS_ERROR       = HDMITX_HDMI_ERROR_MASK | 6,
	HDMITX_HDCP_AUTH_READ_BKSV_ERROR        = HDMITX_HDMI_ERROR_MASK | 7,
	HDMITX_HDCP_AUTH_VI_MISMATCH_ERROR      = HDMITX_HDMI_ERROR_MASK | 8,
	HDMITX_HDCP_AUTH_TOPOLOGY_ERROR         = HDMITX_HDMI_ERROR_MASK | 9,
	HDMITX_HDCP_AUTH_R0_MISMATCH_ERROR      = HDMITX_HDMI_ERROR_MASK | 10,
	HDMITX_HDCP_AUTH_REPEATER_DELAY_ERROR   = HDMITX_HDMI_ERROR_MASK | 11,
	HDMITX_HDCP_I2C_ERROR                   = HDMITX_HDMI_ERROR_MASK | 12,
	/* Mode setting erros */
	HDMITX_KMS_ERROR                        = HDMITX_HDMI_ERROR_MASK | 13,
	HDMITX_KMS_SKIP                         = HDMITX_HDMI_ERROR_MASK | 14,
};

struct hdmitx_tracer;

int hdmitx_tracer_destroy(struct hdmitx_tracer *tracer);

/*write event to fifo, return 0 if write success, else return errno.*/
int hdmitx_tracer_write_event(struct hdmitx_tracer *tracer,
	enum hdmitx_event_log_bits event);
/*read events log in fifo, return read buffer len.*/
int hdmitx_tracer_read_event(struct hdmitx_tracer *tracer,
	char *buf, int read_len);

#endif
