/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMI_HDCP_H__
#define __HDMI_HDCP_H__
#include "hdmi_tx_connector/hdmitx/tx21/hdmitx_hw_core.h"

#define DDC_HDCP_DEVICE_ADDR 0x74
#define REG_DDC_HDCP_VERSION 0x50

enum hdcp_ver_t {
	HDCP_VER_NONE,
	HDCP_VER_HDCP1X,
	HDCP_VER_HDCP2X,
};

#define HDCP1X_MAX_DEPTH 7
/* for repeater enable case, as maximum ksv fifo 16*5 bytes */
#define HDCP1X_MAX_TX_DEV_RPT 16 //127
/* for source case(hdcp repeater disabled) */
#define HDCP1X_MAX_TX_DEV_SRC 127
#define HDCP1X_MAX_RPTR_DEV 32

#define HDCP2X_MAX_DEV 31
#define HDCP2X_MAX_DEPTH 4

#define KSV_SIZE 5

struct hdcp_ksv_t {
	u8 b[KSV_SIZE]; /* 40-bit ksv */
};

struct hdcp_topo_t {
	/* hdcp1x compliant device in the topology if true, for hdcp2x */
	bool ds_hdcp1x_dev;
	/* hdcp 2.0 compliant repeater in the topology if true, for hdcp2x */
	bool ds_hdcp2x_dev;
	/* more than 7 level for hdcp1x or 4 level for hdcp2x cascaded together if true */
	bool max_cas_exceed;
	/* more than 127 devices(for hdcp1x) or 31 devices(for hdcp2x) as attached if true */
	bool max_devs_exceed;
	/* total number of attached downstream devices */
	u8 dev_count;
	/* repeater cascade depth */
	u8 rp_depth;
	/* seq_num_v value, for hdcp2x */
	u32 seq_num_v;
};

enum hdcp_content_type_t {
	HDCP_CONTENT_TYPE_0,
	HDCP_CONTENT_TYPE_1,
	HDCP_CONTENT_TYPE_NONE = 0xff,
};

struct hdcp_csm_t {
	u32 seq_num_m; /* seq_num_m value, hdcp2x use only */
	u16 streamid_type; /* streamid_type */
};

enum hdcp_fail_types_t {
	HDCP_FAIL_NONE,
	HDCP_FAIL_DDC_NACK,
	HDCP_FAIL_BKSV_RXID,
	HDCP_FAIL_AUTH_FAIL,
	HDCP_FAIL_READY_TO,
	HDCP_FAIL_V,
	HDCP_FAIL_TOPOLOGY,
	HDCP_FAIL_RI,
	HDCP_FAIL_REAUTH_REQ,
	HDCP_FAIL_CONTENT_TYPE,
	HDCP_FAIL_AUTH_TIME_OUT,
	HDCP_FAIL_HASH,
	HDCP_FAIL_UNKNOWN,
};

enum hdcp_stat_t {
	HDCP_STAT_NONE,
	HDCP_STAT_AUTH,
	HDCP_STAT_CSM,
	HDCP_STAT_SUCCESS,
	HDCP_STAT_FAILED,
};

/* 3. HDCP struct */
struct hdcp_work {
	u32 delay_ms;
	u32 period_ms;
	const char *name;
	struct delayed_work dwork;
};

/* hw21 core private hdcp struct */
struct hdcptx21_core_priv {
	bool hdcptx_enabled;
	bool ds_auth;
	bool encryption_enabled;
	bool ds_repeater;
	bool rpt_ready;
	bool update_topology;
	bool update_topo_state;
	bool reauth_ignored;
	bool csm_msg_sent;
	bool csm_valid;
	enum hdcp_fail_types_t fail_type;
	/* only used for limit print when failure */
	enum hdcp_fail_types_t fail_reason;
	enum hdcp_ver_t req_hdcp_ver;
	enum hdcp_content_type_t content_type;
	enum hdcp_stat_t hdcp_state;
	enum hdcp_ver_t hdcp_type;
	enum hdcp_ver_t hdcp_cap_ds;
	u8 *p_ksv_lists;
	u8 *p_ksv_next;
	struct hdcp_topo_t hdcp_topology;
	struct hdcp_csm_t csm_message;
	struct hdcp_csm_t prev_csm_message;
	union intr_u intr_regs;
	struct workqueue_struct *hdcp_wq;
	struct hdcp_work timer_hdcp_rcv_auth;
	struct hdcp_work timer_hdcp_rpt_auth;
	struct hdcp_work timer_hdcp_auth_fail_retry;
	struct hdcp_work timer_bksv_poll_done;
	struct hdcp_work timer_hdcp_start;
	struct hdcp_work timer_ddc_check_nak;
	struct hdcp_work timer_update_csm;

	/* for ksv/receiver_id list forward */
	struct delayed_work ksv_notify_wk;
	/* for hdcp reauth from upstream */
	struct delayed_work req_reauth_wk;
	/* for mute operation under stream type1 */
	struct delayed_work stream_mute_wk;
	/* for stream type propagate */
	struct delayed_work stream_type_wk;

	/* for upstream side request auth timeout */
	unsigned long up_hdcp_timeout_sec;
	struct delayed_work work_up_hdcp_timeout;
	/* work for hdcp started by hdmitx driver */
	struct delayed_work work_tx_start_hdcp;
	/* work for hdcp started by linux/drm */
	struct delayed_work work_drm_start_hdcp;
	u32 hdcp_debug_delay;

	/* hdcp stop & start should be mutexed */
	struct mutex hdcptx_auth_mutex;
	/*
	 * case1: upstream side update stream type
	 * case2: hdmitx itself update stream type
	 * when hdmirx-hdmitx passthrough enabled,
	 * the sequence of two cases is not certain,
	 * need to use the type of case1 at last.
	 * need to mutex csm_message.streamid_type
	 * and also make stream type management
	 * message sent only once
	 * it may involve hdcptx_auth_mutex inside!
	 */
	struct mutex stream_type_mutex;

	/* audio/video mute if upstream type = 1 */
	bool stream_mute;
	/* hdmirx side request flag, see hdmi_rx_repeater.h
	 * STREAMTYPE_UPDATE 0x10
	 * UPSTREAM_INACTIVE 0x20
	 * UPSTREAM_ACTIVE 0x40
	 * 0: notify reauth
	 */
	u8 rx_update_flag;
	/* the stream type notified by upstream side
	 * bit4: if 1, upstream may send stream type before
	 * hdmitx start hdcp(passthrough enabled), need
	 * to save it, and cover the stream type with bit3:0
	 * when hdmitx hdcp propagate stream type. else
	 * hdmitx is the active source and should decide
	 * the stream type itself
	 */
	u8 saved_upstream_type;
	/* 0: auto hdcp version, 1: hdcp1.4, 2: hdcp2.3 */
	u8 req_reauth_ver;
	u8 cont_smng_method;
	/* flag: csm already updated by single csm message */
	bool csm_updated;
	bool hdcp14_second_part_pass;
	/* hdcp1.4 key only need to be loaded once when bootup/resume,
	 * set this flag true after hdcp1.4 key loaded
	 */
	bool hdcp14_key_loaded;
	struct hdmitx_common *bind_instance;

	u8 def_stream_type;

	/*
	 * hdcp fail event method 2: don't stop-restart hdcp auth
	 * only for special project.
	 * when start play game movie, it will switch from
	 * DV STD to DV LL, hdcp will stop and restart after
	 * mode setting done.
	 * now provide an option: when DV STD <-> DV LL
	 * switch caused by game movie, systemcontrol write 1
	 * to not_restart_hdcp node before do mode switch, it
	 * means that this colorspace(mode) switch will only
	 * switch mode, but won't stop->restart hdcp action to
	 * prevent hdcp fail uevent sent to app.
	 * note:
	 * 1.not sure if it will always work on different TV.
	 * 2.after mode switch done, not_restart_hdcp will be self cleared
	 */
	bool not_restart_hdcp;
	/* switch mode by passthrough hdmirx input format to downstream hdmitx output
	 * if yes, system_control will write this node to 1.
	 * only for special hdmitx21 project.
	 */
	u32 is_passthrough_switch;
};

bool get_hdcp1_lstore(struct hdmitx_common *tx_comm);
bool get_hdcp2_lstore(struct hdmitx_common *tx_comm);
bool get_hdcp1_result(void);
bool get_hdcp2_result(enum amhdmitx_chip_e chip_type);
bool hdcptx1_load_key(void);
bool is_rx_hdcp2ver(struct hdmitx_common *tx_comm);
void hdcptx_mode_set(struct hdcptx21_core_priv *p_hdcp, unsigned int mode);
void hdcp1x_intr_handler(struct intr_t *intr, void *intr_para);
void hdcp2x_intr_handler(struct intr_t *intr, void *intr_para);
void intr_status_save_clr_cp2txs(u8 regs[]);
void hdcptx_init_reg(void);
void hdcptx2_src_auth_start(u8 content_type);
void hdcptx1_auth_start(void);
void hdcptx2_auth_stop(void);
void hdcptx1_auth_stop(void);
void hdcptx1_encryption_update(bool en);
void hdcptx2_encryption_update(bool en);
void hdcptx2_reauth_send(void);
bool hdcptx2_ds_rptr_capability(void);
bool hdcptx1_ds_rptr_capability(void);
void hdcptx1_ds_an_read(u8 *p_an, u8 an_bytes);
void hdcptx1_ds_aksv_read(u8 *p_aksv, u8 ksv_bytes);
void hdcptx1_ds_bksv_read(u8 *p_bksv, u8 ksv_bytes);
u8 hdcptx1_ksv_v_get(void);
void hdcptx1_protection_enable(bool en);
void hdcptx1_intermed_ri_check_enable(bool en);
void hdcptx2_ds_rcv_id_read(u8 *p_rcv_id);
void hdcptx2_ds_rpt_rcvid_list_read(u8 *p_rpt_rcv_id, u8 dev_count, u8 bytes_to_read);
u8 hdcptx1_ds_cap_status_get(void);
u8 hdcptx1_copp_status_get(void);
u16 hdcptx1_get_prime_ri(void);
void hdcptx1_get_ds_ksvlists(u8 **p_ksv, u8 count);
void hdcptx1_bstatus_get(u8 *p_ds_bstatus);
bool hdcptx2_auth_status(void);
u8 hdcptx2_rpt_dev_cnt_get(void);
u8 hdcptx2_rpt_depth_get(void);
u8 hdcptx2_topology_get(void);
void hdcptx2_csm_send(struct hdcp_csm_t *csm_msg);
void hdcptx2_rpt_smng_xfer_start(void);
void hdcptx1_bcaps_get(u8 *p_bcaps_status);
u8 hdcp2x_get_state_st(void);
void hdcptx1_query_aksv(struct hdcp_ksv_t *p_val);
u8 hdmitx_reauth_request(u8 hdcp_version);
void set_hdcp2_topo(enum amhdmitx_chip_e chip_type, u32 topo_type);
bool get_hdcp2_topo(enum amhdmitx_chip_e chip_type);
void hdmitx21_enable_hdcp(struct hdmitx_common *tx_comm);
void hdmitx21_disable_hdcp(struct hdcptx21_core_priv *p_hdcp);
void hdmitx21_rst_stream_type(struct hdcptx21_core_priv *hdcp);
bool hdcptx_need_ctrl_by_upstream(bool hdcp_rpt_en);
u32 hdmitx21_get_hdcp_mode(struct hdmitx_common *tx_comm);
void hdcptx_en_aes_dual_pipe(bool en);
u8 hdcptx_reauth_request(void *hdcptx_priv, u8 hdcp_version);

int hdmitx21_hdcp_init(struct hdmitx_common *tx_comm);
void hdmitx21_hdcp_uninit(struct hdmitx_common *tx_comm);
void ddc_toggle_sw_tpi(void);
bool hdmitx_ddcm_read(u8 seg_index, u8 slave_addr, u8 reg_addr, u8 *p_buf, u16 len, u8 read_cmd);
bool hdmitx_ddcm_write(u8 seg_index, u8 slave_addr, u8 reg_addr, u8 data);
bool hdcptx_ddc_check_busy(void);
/* hdcp clk gate related */
void hdmitx21_ctrl_hdcp_gate(enum amhdmitx_chip_e chip_type, int hdcp_mode, bool en);
u32 hdmitx21_get_gate_status(void);
unsigned int hdcptx_get_key_store(struct hdmitx_common *tx_comm);

#endif /* __HDMI_HDCP_H__ */
