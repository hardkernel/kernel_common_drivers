/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMI_TX_HDCP_H
#define __HDMI_TX_HDCP_H
#include <linux/miscdevice.h>
#include <drm/amlogic/meson_connector_dev.h>
/*
 * hdmi_tx_hdcp.c
 * version 1.0
 */

/* Notic: the HDCP key setting has been moved to uboot
 * On MBX project, it is too late for HDCP get from
 * other devices
 */
int hdcp_ksv_valid(unsigned char *dat);
int hdmitx_hdcp_init(struct hdmitx_dev *hdev);
void hdmitx_hdcp_exit(struct hdmitx_dev *hdev);

/* drm hdmitx hdcp use */

/* ioctl numbers */
enum {
	TEE_HDCP_START,
	TEE_HDCP_END,
	HDCP_DAEMON_LOAD_END,
	HDCP_DAEMON_REPORT,
	HDCP_EXE_VER_SET,
	HDCP_TX_VER_REPORT,
	HDCP_DOWNSTR_VER_REPORT,
	HDCP_EXE_VER_REPORT
};

#define TEE_HDCP_IOC_START    _IOW('P', TEE_HDCP_START, int)
#define TEE_HDCP_IOC_END    _IOW('P', TEE_HDCP_END, int)
#define HDCP_DAEMON_IOC_LOAD_END    _IOW('P', HDCP_DAEMON_LOAD_END, int)
#define HDCP_DAEMON_IOC_REPORT    _IOR('P', HDCP_DAEMON_REPORT, int)
#define HDCP_EXE_VER_IOC_SET    _IOW('P', HDCP_EXE_VER_SET, int)
#define HDCP_TX_VER_IOC_REPORT    _IOR('P', HDCP_TX_VER_REPORT, int)
#define HDCP_DOWNSTR_VER_IOC_REPORT    _IOR('P', HDCP_DOWNSTR_VER_REPORT, int)
#define HDCP_EXE_VER_IOC_REPORT    _IOR('P', HDCP_EXE_VER_REPORT, int)

enum {
	HDCP_TX22_DISCONNECT = 0,
	HDCP_TX22_START,
	HDCP_TX22_STOP
};

enum {
	HDCP22_DAEMON_LOADING = 0,
	HDCP22_DAEMON_DONE,
	HDCP22_DAEMON_TIMEOUT
};

#define HDCP_AUTH_TIMEOUT (40) /*40*200ms = 8s*/
#define HDCP22_LOAD_TIMEOUT (160)
#define TIMER_CHECK	(1 * HZ / 2)
#define TIMER_CHK_CNT 60

struct meson_hdmitx_hdcp {
	struct miscdevice hdcp_comm_device;
	wait_queue_head_t hdcp_comm_queue;
	/* bit0:hdcp14 bit 1:hdcp22 */
	unsigned int hdcp_tx_type;
	/* bit0:hdcp14 bit 1:hdcp22 */
	unsigned int hdcp_downstream_type;
	/* 0: null hdcp 1: hdcp14 2: hdcp22 */
	unsigned int hdcp_execute_type;
	/* 0: null hdcp 1: hdcp14 2: hdcp22 */
	unsigned int hdcp_debug_type;

	unsigned int hdcp_en;
	int hdcp_poll_report;
	int hdcp_auth_result;
	int hdcp_fail_cnt;
	int hdcp_report;
	int hdcp22_daemon_state;

	struct connector_hdcp_cb drm_hdcp_cb;
	struct timer_list daemon_load_timer;
	unsigned int key_chk_cnt;
	struct delayed_work notify_work;
};

void meson_hdcp_init(void);
void meson_hdcp_exit(void);

void meson_hdcp_enable(int hdcp_type);
void meson_hdcp_disable(void);
void meson_hdcp_disconnect(void);

void meson_hdcp_reg_result_notify(struct connector_hdcp_cb *cb);

bool hdcp_tx22_daemon_ready(void);

unsigned int meson_hdcp_get_rx_cap(void);
unsigned int meson_hdcp_get_tx_cap(void);
bool is_hdcp22_stop_state(void);
unsigned char drm_hdmitx_get_hdcp_topo_info(void);

void drm_hdmitx_hdcp22_init(void);

void drm_hdmitx_enable_hdcp_mode(unsigned int content_type);
void drm_hdmitx_disable_hdcp_mode(unsigned int content_type);

#endif

