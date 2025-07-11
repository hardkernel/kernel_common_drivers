/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __MESON_TX_EVENT_MGR_H
#define __MESON_TX_EVENT_MGR_H

#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/extcon.h>
#include <linux/extcon-provider.h>

#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_dev.h>
#include <linux/amlogic/media/vout/meson_tx_connector/meson_tx_log.h>

struct meson_tx_tracer;

#define MAX_UEVENT_LEN 64

enum tx_trace_log_bits {
	TX_HPD_PLUGOUT	= 1,
	TX_HPD_PLUGIN,
};

enum meson_tx_event {
	TX_NONE_EVENT = 0,
	TX_HPD_EVENT,
	TX_HDCP_EVENT,
	TX_CUR_ST_EVENT,
	TX_AUDIO_EVENT,
	TX_HDCPPWR_EVENT,
	TX_HDR_EVENT,
	TX_RXSENSE_EVENT,
	TX_CEDST_EVENT,
	TX_SOUNDBAR_EVENT,
	TX_VMODE_AUDIO_EVENT,
	TX_TRACER_EVENT,
};

struct meson_tx_event_mgr {
	/*for uevent*/
	struct kobject *kobj;
	/* can't send uevent after enter suspend */
	bool deep_suspend_flag;
	/*for extcon event*/
	struct extcon_dev *tx_extcon;
	struct device *attached_extcon_dev;
	/*notifier for driver*/
	struct blocking_notifier_head tx_event_notify_list;
	/* if en, not send hpd extcon even to android*/
	bool soundbar_en_flag;
	struct meson_tx_log *log;
	int conn_type;
};

struct tx_notifier_client {
	struct notifier_block base;
};

int meson_tx_tracer_destroy(struct meson_tx_tracer *tracer);
struct meson_tx_tracer *meson_tx_tracer_create(struct meson_tx_event_mgr *event_mgr);
int meson_tx_tracer_read_event(struct meson_tx_tracer *tracer,
	char *buf, int read_len);
int meson_tx_tracer_write_event(struct meson_tx_tracer *tracer,
	enum tx_trace_log_bits event);

struct meson_tx_event_mgr *meson_tx_event_mgr_create(struct meson_tx_dev *tx_dev);
void meson_tx_event_mgr_suspend(struct meson_tx_event_mgr *event_mgr, bool suspend_flag);
int meson_tx_event_mgr_destroy(struct meson_tx_event_mgr *event_mgr);
int meson_tx_event_mgr_set_uevent_state(struct meson_tx_event_mgr *event_mgr,
	enum meson_tx_event type, int state);
int meson_tx_event_mgr_send_uevent(struct meson_tx_event_mgr *uevent_mgr,
	enum meson_tx_event type, int state, bool force);
void meson_tx_event_mgr_set_uevent_dev(struct meson_tx_event_mgr *event_mgr,
					struct device *dev);

int meson_tx_event_mgr_notifier_register(struct meson_tx_event_mgr *event_mgr,
	struct tx_notifier_client *nb);
int meson_tx_event_mgr_notifier_unregister(struct meson_tx_event_mgr *event_mgr,
	struct tx_notifier_client *nb);
int meson_tx_event_mgr_notify(struct meson_tx_event_mgr *event_mgr,
	unsigned long state, void *arg);

#endif
