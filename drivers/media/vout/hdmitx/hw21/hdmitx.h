/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMI_TX_H__
#define __HDMI_TX_H__
#include <linux/hdmi.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include "hdmitx_hdcp.h"
#include "hdmitx_module.h"
#include "hdmitx_packet.h"

/* L_0 will always be printed, set log level to L_1/2/3 for detail */
#define L_0 0
#define L_1 1
#define L_2 2
#define L_3 3

#define HDCP_STAGE1_RETRY_TIMER 2000 /* unit: ms */
#define HDCP_BSKV_CHECK_TIMER 100
#define HDCP_FAILED_RETRY_TIMER 200
#define HDCP_DS_KSVLIST_RETRY_TIMER 5000//200
#define HDCP_RCVIDLIST_CHECK_TIMER 3000//200

#define DEFAULT_STREAM_TYPE 0
#define VIDEO_MUTE_PATH_1 0x8000000 //mute by vid_mute sysfs node
#define VIDEO_MUTE_PATH_2 0x4000000 //mute by stream type 1
#define VIDEO_MUTE_PATH_3 0x2000000 //mute by rx request auth

#define AVMUTE_PATH_1 0x80 //mute by avmute sysfs node
#define AVMUTE_PATH_2 0x40 //mute by upstream side request re-auth

struct emp_packet_st;
enum emp_component_conf;

#define HDMI_INFOFRAME_EMP_VRR_GAME ((HDMI_INFOFRAME_TYPE_EMP << 8) | (EMP_TYPE_VRR_GAME))
#define HDMI_INFOFRAME_EMP_VRR_QMS ((HDMI_INFOFRAME_TYPE_EMP << 8) | (EMP_TYPE_VRR_QMS))
#define HDMI_INFOFRAME_EMP_VRR_SBTM ((HDMI_INFOFRAME_TYPE_EMP << 8) | (EMP_TYPE_SBTM))
#define HDMI_INFOFRAME_EMP_VRR_DSC ((HDMI_INFOFRAME_TYPE_EMP << 8) | (EMP_TYPE_DSC))
#define HDMI_INFOFRAME_EMP_VRR_DHDR ((HDMI_INFOFRAME_TYPE_EMP << 8) | (EMP_TYPE_DHDR))

#define HDCPTX_IOOPR		0x820000ab
enum hdcptx_oprcmd {
	HDCP_DEFAULT,
	HDCP14_KEY_READY,
	HDCP14_LOADKEY,
	HDCP14_RESULT,
	HDCP22_KEY_READY,
	HDCP22_LOADKEY,
	HDCP22_RESULT,
	HDCP22_SET_TOPO,
	HDCP22_GET_TOPO,
	CONF_ENC_IDX, /* 0: get idx; 1: set idx */
	HDMITX_GET_RTERM, /* get the rterm value */
	CHECK_SPEC_EDID
};

/* DDC bus error codes */
enum ddc_err_t {
	DDC_ERR_NONE = 0x00,
	DDC_ERR_TIMEOUT = 0x01,
	DDC_ERR_NACK = 0x02,
	DDC_ERR_BUSY = 0x03,
	DDC_ERR_HW = 0x04,
	DDC_ERR_LIM_EXCEED = 0x05,
};

u8 hdmi_ddc_status_check(void);
u8 hdmi_ddc_busy_check(void);
void hdmi_ddc_error_reset(void);

int hdmitx21_hdcp_init(struct hdmitx21_dev *hdev);
void hdmitx21_hdcp_exit(struct hdmitx21_dev *hdev);
int hdmitx21_init_reg_map(struct platform_device *pdev);
void hdmitx21_set_audioclk(bool en);
void hdmitx21_disable_clk(struct hdmitx21_dev *hdev);
u32 hdcp21_rd_hdcp22_ver(void);

struct mvrr_const_val {
	/* unit: 100  6000, 5994, 5000, 3000, 2997, 2500, 2400, 2397 */
	u16 duration;
	u16 vtotal_fixed; /* vtotal_fixed is mutex with bit_len */
	u8 bit_len; /* current max value is 16 * 8, 128 */
	u8 frac_array[16];
};

struct mvrr_const_st {
	enum hdmi_vic brr_vic; /* the vic of brr */
	const struct mvrr_const_val *val[];
};

/* VRR parameters configuration */
struct vrr_conf_para {
	enum vrr_type type;
	u8 vrr_enabled;
	u8 fva_factor; /* should be larger than 0 */
	u8 brr_vic;
	int duration;
	/* EDID capability, refer to 2.1A P380 */
	u8 fapa_end_extended:1;
	u8 qms_sup:1;
	u8 mdelta_bit:1;
	u8 neg_mvrr_bit:1;
	u8 fva_sup:1;
	u8 fapa_start_loc:1;
	u8 qms_tfrmin:1;
	u8 qms_tfrmax:1;
	u8 vrrmin;
	u16 vrrmax;
};

/* below will be used in the vrr sync handler */
struct tx_vrr_params {
	/* the member conf_params is critical and may change at anytime */
	spinlock_t lock;
	struct vrr_conf_para conf_params;
	/* vrr_para_tmp is used for saving conf_params if conf_params is updated */
	struct vrr_conf_para vrr_para_tmp;
	const struct mvrr_const_val *mconst_val; /* for qms */
	struct mvrr_const_val game_val; /* for game */
	struct emp_packet_st emp_vrr_pkt;
	u32 frame_cnt; /* set to 0 when vrr_params changed */
	u32 mdelta_limit; /* for mdelta = 1 case */
	u8 fapa_early_cnt;
};

void hdmi_gcppkt_manual_set(bool en);

struct intr_t;

typedef void(*hdmi_intr_cb)(struct intr_t *);

struct intr_t {
	const u32 intr_mask_reg;
	const u32 intr_st_reg;
	const u32 intr_clr_reg;
	const unsigned int intr_top_bit;
	u32 mask_data;
	u32 st_data;
	hdmi_intr_cb callback;
};

/* intr_array will be used for ISR quickly save status
 * while the entity.NAME will be used for bottom handler
 */
union intr_u {
	struct {
		struct intr_t top_intr;
		struct intr_t tpi_intr;
		struct intr_t cp2tx_intr0;
		struct intr_t cp2tx_intr1;
		struct intr_t cp2tx_intr2;
		struct intr_t cp2tx_intr3;
		struct intr_t scdc_intr;
		struct intr_t intr2;
		struct intr_t intr5;
	} entity;
};

struct hdcp_work {
	u32 delay_ms;
	u32 period_ms;
	const char *name;
	struct delayed_work dwork;
};

struct hdcp_t {
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
	struct delayed_work ksv_notify_wk;
	struct delayed_work req_reauth_wk;
	struct delayed_work stream_mute_wk;
	struct delayed_work stream_type_wk;
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
};

/* hdcp related */
bool get_hdcp1_lstore(void);
bool get_hdcp2_lstore(void);
bool get_hdcp1_result(void);
bool get_hdcp2_result(void);
bool hdcptx1_load_key(void);
bool is_rx_hdcp2ver(void);
void hdcp_mode_set(unsigned int mode);
void hdcp_enable_intrs(bool en);
void hdcp1x_intr_handler(struct intr_t *intr);
void hdcp2x_intr_handler(struct intr_t *intr);
void intr_status_save_clr_cp2txs(u8 regs[]);
void hdcptx_init_reg(void);
void hdcptx2_src_auth_start(u8 content_type);
void hdcptx1_auth_start(void);
void hdcptx2_auth_stop(void);
void hdcptx1_auth_stop(void);
void hdcptx1_encryption_update(bool en);
void hdcptx2_encryption_update(bool en);
void hdcptx2_reauth_send(void);
void ddc_toggle_sw_tpi(void);
bool hdcptx2_ds_rptr_capability(void);
bool hdcptx1_ds_rptr_capability(void);
void hdcptx1_ds_bksv_read(u8 *p_bksv, u8 ksv_bytes);
u8 hdcptx1_ksv_v_get(void);
bool hdcp1x_ksv_valid(u8 *dat);
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
void hdcptx2_smng_auto(bool en);
u8 hdcp2x_get_state_st(void);
void hdcptx1_query_aksv(struct hdcp_ksv_t *p_val);

void tee_comm_dev_reg(struct hdmitx21_dev *hdev);
void tee_comm_dev_unreg(struct hdmitx21_dev *hdev);
u8 hdmitx_reauth_request(u8 hdcp_version);
void set_hdcp2_topo(u32 topo_type);
bool get_hdcp2_topo(void);
void hdmitx21_enable_hdcp(struct hdmitx21_dev *hdev);
void hdmitx21_disable_hdcp(struct hdmitx21_dev *hdev);
void hdmitx21_rst_stream_type(struct hdcp_t *hdcp);
bool hdcp_need_control_by_upstream(struct hdmitx_hw_common *tx_hw);
u32 hdmitx21_get_hdcp_mode(void);
void hdcptx_en_aes_dualpipe(bool en);
void pr_hdcp_info(const char *fmt, ...);
extern unsigned long hdcp_reauth_dbg;
extern unsigned long streamtype_dbg;
extern unsigned long en_fake_rcv_id;
void hdmitx_top_intr_handler(struct work_struct *work);
int hdmitx_setupirqs(struct hdmitx_hw_common *tx_hw);
void intr_status_init_clear(void);
void ddc_toggle_sw_tpi(void);
bool hdmitx_ddcm_read(u8 seg_index, u8 slave_addr, u8 reg_addr, u8 *p_buf, u16 len, u8 read_cmd);
bool hdmitx_ddcm_write(u8 seg_index, u8 slave_addr, u8 reg_addr, u8 data);
bool ddc_bus_wait_free(void);

/* VRR parts */
irqreturn_t hdmitx_vrr_vsync_handler(struct hdmitx21_dev *hdev);
void tx_vrr_params_init(void);
void hdmitx_set_vrr_para(const struct vrr_conf_para *para);
void hdmitx_vrr_set_maxlncnt(u32 max_lcnt);
u32 hdmitx_vrr_get_maxlncnt(void);
int hdmitx_set_vrr_rate(int duration, void *data);
void hdmitx_unregister_vrr(u32 enc_idx);
void hdmitx_register_vrr(struct hdmitx21_dev *hdev);
int hdmitx_dump_vrr_status(struct seq_file *s, void *p);
void hdmitx_vrr_enable(void);
void hdmitx_vrr_disable(void);
void hdmitx_register_drm_vrr_api(struct hdmitx21_dev *hdev);
void hdmitx21_vrr_init(struct hdmitx21_dev *hdev);

/* mute realted */
extern unsigned long avmute_ms;
extern unsigned long vid_mute_ms;
void hdmitx21_av_mute_op(u32 flag, unsigned int path);
void hdmitx21_video_mute_op(u32 flag, unsigned int path);
void hdmitx21_audio_mute_op(u32 flag, unsigned int path);

/* gate realted */
void hdmitx21_clks_gate_ctrl(bool en);
void hdcptx_ctrl_gate(int hdcp_mode, bool en);
void hdmitx21_ctrl_hdcp_gate(int hdcp_mode, bool en);
u32 hdmitx21_get_gate_status(void);

int likely_frac_rate_mode(const char *m);
/* FRL */
struct frl_work {
	u32 delay_ms;
	u32 period_ms;
	const char *name;
	struct delayed_work dwork;
};

struct frl_train_t {
	struct workqueue_struct *frl_wq;
	struct frl_work timer_frl_flt;
	u8 src_test_cfg;
	u8 lane_count;
	u8 update_flags;
	enum flt_tx_states flt_tx_state;
	enum flt_tx_states last_state;
	enum frl_rate_enum max_frl_rate;
	enum frl_rate_enum max_edid_frl_rate;
	enum frl_rate_enum user_max_frl_rate;
	enum frl_rate_enum min_frl_rate;
	enum frl_rate_enum frl_rate;
	enum ffe_levels max_ffe_level;
	enum ffe_levels ffe_level[4];
	bool ds_frl_support;
	bool req_legacy_mode;
	bool frl_rate_no_change;
	bool req_frl_mode;
	bool txffe_pre_shoot_only;
	bool txffe_de_emphasis_only;
	bool txffe_no_ffe;
	bool flt_no_timeout;
	bool flt_timeout;
	bool auto_ffe_update;
	bool auto_pattern_update;
	bool flt_running; /* if flt_running is false, return */
};

/* SCDC related */
#define LT_TX_CMD_TXFFE_UPDATE 0x02
#define LT_TX_CMD_LTP_UPDATE   0x03
void scdc_bus_stall_set(bool en);
u16 scdc_tx_ltp0123_get(void);
bool scdc_tx_frl_cfg1_set(u8 cfg1);
u8 scdc_tx_update_flags_get(void);
bool scdc_tx_update_flags_set(u8 update_flags);
u8 scdc_tx_flt_ready_status_get(void);
u8 scdc_tx_sink_version_get(void);
void scdc_tx_source_version_set(u8 src_ver);
u8 scdc_tx_source_test_cfg_get(void);
void scdc_tx_frl_get_rx_rate(u8 *data);

bool flt_tx_cmd_execute(u8 lt_cmd);
void flt_tx_ltp_req_write(u8 ltp01, u8 ltp23);
void flt_tx_update_set(void);
void frl_tx_start_mod(bool start);
bool flt_tx_update_cleared_wait(void);
bool frl_tx_rate_written(void);
u8 flt_tx_cfg1_get(void);
u8 frl_tx_tx_get_rate(void);
void frl_tx_tx_enable(bool enable);
void frl_tx_av_enable(bool enable);
void frl_tx_sb_enable(bool enable, enum frl_rate_enum frl_rate);
bool frl_tx_pattern_init(u16 patterns);
void frl_tx_pattern_stop(void);
void frl_tx_pin_swap_set(bool en);
bool frl_tx_pattern_set(enum ltp_patterns frl_pat, u8 lane);
bool frl_tx_ffe_set(enum ffe_levels ffe_level, u8 lane);
bool frl_tx_tx_phy_init(bool disable_ffe);
void frl_tx_tx_init(void);
void frl_tx_tx_phy_set(void);
void tmds_tx_phy_set(void);
void frl_tx_lts_1_hdmi21_config(void);
void frl_tx_training_handler(struct hdmitx21_dev *hdev);
void frl_tx_stop(void);
unsigned int drm_hdmitx_get_rx_hdcp_cap(void);
u32 drm_hdmitx_get_vrr_cap(void);
int drm_hdmitx_get_vrr_mode_group(struct hdmitx_vrr_mode_group *group, int max_group);
bool frl_check_full_bw(enum hdmi_colorspace cs, enum hdmi_color_depth cd, u32 pixel_clock,
	u32 h_active, enum frl_rate_enum frl_rate, u32 *tri_bytes);
void fifo_flow_enable_intrs(bool en);
void hdmitx_soft_reset(u32 bits);
void hdmitx_set_frl_rate_none(struct hdmitx_common *tx_comm);

/* dsc related */
#ifdef CONFIG_AMLOGIC_DSC
irqreturn_t hdmitx_emp_vsync_handler(struct hdmitx21_dev *hdev);
void hdmitx_dsc_cvtem_pkt_send(struct dsc_pps_data_s *pps,
	struct hdmi_timing *timing);
void hdmitx_dsc_cvtem_pkt_disable(void);
#endif

unsigned int hdmitx21_get_vendor_infoframe_ieee(void);
bool hdmitx21_edid_only_support_sd(struct hdmitx21_dev *hdev);
/* bool is_4k_sink(struct hdmitx21_dev *hdev); */
void hdmitx_clks_gate_ctrl(bool en);

void hdmitx21_dither_config(struct hdmitx21_dev *hdev);

/* hdcp */
void drm_hdmitx_start_hdcp_handler(struct work_struct *work);
void hdmitx_start_hdcp_handler(struct work_struct *work);
void hdmitx_up_hdcp_timeout_handler(struct work_struct *work);
#endif /* __HDMI_TX_H__ */

