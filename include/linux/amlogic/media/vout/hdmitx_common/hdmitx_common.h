/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __HDMITX_COMMON_H
#define __HDMITX_COMMON_H

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/hdmi.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/jiffies.h>

#include <drm/amlogic/meson_connector_dev.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_format_para.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_hw_common.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_edid.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_types.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_tracer.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_event_mgr.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_packet.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_infoframe.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_config.h>
#include <linux/amlogic/media/vout/dsc.h>
#include <linux/amlogic/media/vrr/vrr.h>

#define HDMITX20 20
#define HDMITX21 21

#define HDMI_TX_POOL_NUM  6
#define HDMI_TX_RESOURCE_NUM 4
#define HDMI_TX_PWR_CTRL_NUM	6

struct hdmitx_common_state {
	struct hdmi_format_para para;
	enum vmode_e mode;
	u32 hdr_priority;
	/* save sequence id for drm connecter state */
	u64 state_sequence_id;
};

typedef void (*audio_en_callback)(bool enable);
typedef int (*audio_st_callback)(void);

struct hdcp_ctrl_ops {
	void (*set_hdcp_mode)(struct hdmitx_common *tx_comm, const char *buf);
	int (*get_hdcp_ver)(struct hdmitx_common *tx_comm, char *buf, int len);
};

struct drm_hdcp_ctrl_ops {
	/*hdcp apis*/
	void (*hdcp_init)(void);
	void (*hdcp_exit)(void);
	void (*hdcp_enable)(int hdcp_type);
	void (*hdcp_disable)(void);
	void (*hdcp_disconnect)(void);

	unsigned int (*get_tx_hdcp_cap)(void);
	unsigned int (*get_rx_hdcp_cap)(void);
	void (*register_hdcp_notify)(struct connector_hdcp_cb *cb);
	/* get downstream hdcp topo info:
	 * return 1 if downstream devices are all capable of hdcp2.2/2.3
	 * return 0 if downstream contains hdcp1.4 or hdcp2.0 legacy device
	 */
	unsigned char (*get_dw_hdcp_topo_info)(void);
};

struct drm_vrr_ctrl_ops {
	/* vrr apis */
	u32 (*get_vrr_cap)(void);
	int (*get_vrr_mode_group)(struct hdmitx_vrr_mode_group *groups, int max_group);
	int (*set_vframe_rate_hint)(int duration, void *data);
};

struct hdmitx_clk_tree_s {
	/* hdmitx clk tree */
	struct clk *hdmi_clk_vapb;
	struct clk *hdmi_clk_vpu;
	struct clk *hdcp22_tx_skp;
	struct clk *hdcp22_tx_esm;
	struct clk *cts_hdmi_axi_clk;
	struct clk *venci_top_gate;
	struct clk *venci_0_gate;
	struct clk *venci_1_gate;
};

struct st_debug_param {
	unsigned int avmute_frame;
};

/* 0: VESA DSC 1.2a is not supported
 * 1: up to 1 slice and up to (340 MHz/K SliceAdjust) pixel clock per slice
 * 2: up to 2 slices and up to (340 MHz/K SliceAdjust) pixel clock per slice
 * 3: up to 4 slices and up to (340 MHz/K SliceAdjust) pixel clock per slice
 * 4: up to 8 slices and up to (340 MHz/K SliceAdjust) pixel clock per slice
 * 5: up to 8 slices and up to (400 MHz/K SliceAdjust) pixel clock per slice
 * 6: up to 12 slices and up to (400 MHz/K SliceAdjust) pixel clock per slice
 * 7: up to 16 slices and up to (400 MHz/K SliceAdjust) pixel clock per slice
 * 8-15: Reserved
 */

static const u8 dsc_max_slices_num[] = {
	0,
	1,
	2,
	4,
	8,
	8,
	12,
	16
};

struct drm_hdmitx_hdcp {
	int hdcp_auth_result;
	int hdcp_fail_cnt;
	/* hdcp result callback */
	struct connector_hdcp_cb drm_hdcp_cb;
	/* drm hdcp debug function, not common api.*/
	int test_hdcp_mode;
	void (*test_set_hdcp_mode)(unsigned int user_type);
	void (*test_set_hdmi_mode)(void);
	void (*test_set_out_mode)(void);
	void (*test_hdcp_disable)(void);
	void (*test_hdcp_enable)(int hdcp_mode);
	void (*test_hdcp_disconnect)(void);
};

struct ced_cnt {
	bool ch0_valid;
	u16 ch0_cnt:15;
	bool ch1_valid;
	u16 ch1_cnt:15;
	bool ch2_valid;
	u16 ch2_cnt:15;
	u8 chksum;
	bool ch3_valid;
	u16 ch3_cnt:15;
	bool rs_c_valid;
	u16 rs_c_cnt:15;
};

struct scdc_locked_st {
	u8 clock_detected:1;
	u8 ch0_locked:1;
	u8 ch1_locked:1;
	u8 ch2_locked:1;
	u8 ch3_locked:1;
};

struct hdmitx_common {
	struct hdmitx_hw_common *tx_hw;
	struct hdcp_ctrl_ops *hdcp_ctrl_ops;
	struct drm_hdcp_ctrl_ops *drm_hdcp_ctrl_ops;
	struct drm_vrr_ctrl_ops *drm_vrr_ctrl_ops;
	struct connector_hpd_cb drm_hpd_cb;/*drm hpd notify*/

	struct cdev cdev; /* The cdev structure */
	dev_t hdmitx_id;
	struct device *hdtx_dev;
	/* dedicated for hpd event */
	struct workqueue_struct *hdmi_hpd_wq;

	char fmt_attr[16];
	/* for pxp test */
	char tst_fmt_attr[16];

	/*edid related*/
	/* edid hdr/dv cap lock, hdr/dv handle in irq, need spinlock*/
	spinlock_t edid_spinlock;
	u32 forced_edid; /* for external loading EDID */
	unsigned char EDID_buf[EDID_MAX_BLOCK * 128];
	struct rx_cap rxcap;
	/****** hdmitx state ******/
	/* Normally, after the HPD in or late resume, there will reading EDID, and
	 * notify application to select a hdmi mode output. But during the mode
	 * setting moment, there may be HPD out. It will clear the edid data, ..., etc.
	 * To avoid such case, here adds the hdmimode_mutex to let the HPD in, HPD out
	 * handler and mode setting sequentially.
	 */
	struct mutex hdmimode_mutex;
	struct mutex valid_mutex;	/* check valid mode need mutex */
	atomic_t kref_video_mute;
	unsigned char vid_mute_op;

	/* save the last plug out/in work done state */
	enum hdmi_event_t last_hpd_handle_done_stat;
	/* save the last drm infoframe eotf */
	enum hdmi_eotf last_drm_eotf;
	/* 1, connect; 0, disconnect */
	unsigned char hpd_state;
	/* if HDMI plugin even once time, then set 1
	 * if never hdmi plugin, then keep as 0
	 * for android ott.
	 */
	u32 already_used;

	/* indicate hdmitx output ready, sw/hw mode setting done */
	bool ready;
	/*
	 * 1 :HWC to enable hdcp flow in SNPS chip
	 * 0 :IVCX chip don't need
	 */
	bool hdcp_user;
	/*if hdmitx is in early suspend.*/
	bool suspend_flag;
	/*current hdcp mode, 2.1 or 1.4*/
	u8 hdcp_mode;
	/* hdcp2.2 bcaps */
	int hdcp_bcaps_repeater;
	/* allm_mode: 1/on 0/off */
	u32 allm_mode;
	/* contenttype:0/off 1/game, 2/graphics, 3/photo, 4/cinema */
	u32 ct_mode;
	bool it_content;
	/* When hdr_priority is 1, then dv_info will be all 0;
	 * when hdr_priority is 2, then dv_info/hdr_info will be all 0
	 * App won't get real dv_cap/hdr_cap, but can get real dv_cap2/hdr_cap2
	 */
	u32 hdr_priority;
	u32 hdr_8bit_en;
	/*current format para.*/
	struct hdmi_format_para fmt_para;
	/* HDR format state */
	u32 hdmi_last_hdr_mode;
	u32 hdmi_current_hdr_mode;

	/* 0.1% clock shift, 1080p60hz->59.94hz */
	u32 frac_rate_policy;
	/* Indicates spread spectrum, 1/on 0/off */
	u32 sspll;
	/* Indicates current status, HDMITX20/HDMITX21 */
	int hdmi_init;
	/*
	 * for SONY-KD-55A8F TV, need to mute more frames(default 20 frames)
	 * when switch DV(LL)->HLG
	 */
	int hdr_mute_frame;
	/*DRM related*/
	struct drm_hdmitx_hdcp drm_hdcp;
	/* vrr related*/
	struct vrr_device_s hdmitx_vrr_dev;

	bool pre_tmds_clk_div40;

	u32 flag_3dfp:1;
	u32 flag_3dtb:1;
	u32 flag_3dss:1;
	/* audio */
	/* if switching from 48k pcm to 48k DD, the ACR/N parameter is same,
	 * so there is no need to update ACR/N. but for mode change, different
	 * sample rate, need to update ACR/N.
	 */
	struct aud_para cur_audio_param;
	/*audio end*/

	/****** device config ******/
	/* 0: TV product, 1: stb/soundbar product;
	 * used to check if need to notify edid to
	 * upstream hdmirx.
	 */
	u32 hdmi_repeater;
	/*hdcp control type config*/
	u32 hdcp_ctl_lvl;
	/* enc index: for non-ott product*/
	u32 enc_idx;
	/*soc limitation config*/
	u32 res_1080p;
	/* efuse ctrl state
	 * 1 disable the function
	 * 0 dont disable the function
	 */
	bool efuse_dis_hdmi_4k60;	/* 4k50,60hz */
	bool efuse_dis_output_4k;	/* all 4k resolution*/
	bool efuse_dis_hdcp_tx22;	/* hdcptx22 */
	bool efuse_dis_hdmi_tx3d;	/* 3d */
	bool efuse_dis_hdcp_tx14;	/* s1a hdcptx14 */
	u32 max_refreshrate;
	/*for color space conversion*/
	bool config_csc_en;

	/***** ced/rxsense related *****/
	bool cedst_en; /* configure in DTS */
	u32 cedst_policy;
	struct workqueue_struct *cedst_wq;
	struct delayed_work work_cedst;
	u32 rxsense_policy;
	struct workqueue_struct *rxsense_wq;
	struct delayed_work work_rxsense;
	struct ced_cnt ced_cnt;
	struct scdc_locked_st chlocked_st;

	/***** VOUT related: TO move out *****/
	struct vinfo_s hdmitx_vinfo;
	struct vout_device_s *vdev;

	/****** debug & log ******/
	struct hdmitx_tracer *tx_tracer;
	struct hdmitx_event_mgr *event_mgr;
	struct st_debug_param debug_param;
	struct dentry *hdmitx_file_dbgfs;
	struct proc_dir_entry *hdmitx_proc_dbgfs;
	/*
	 * the qms_log_id is referred from hw_sequence_id
	 * if value is not changed, the skip massive qms log
	 */
	u64 qms_log_id;

	struct vsif_debug_save vsif_debug_info;
	struct master_display_info_s drm_config_data;
	struct hdr10plus_para hdr10p_config_data;
	struct aud_para hdmiaud_config_data;
	/* diff */
#ifdef CONFIG_AMLOGIC_HDMITX
	/* in board dts file, here can add
	 * &amhdmitx {
	 *     hdcp_type_policy = <1>;
	 * };
	 * 0 is default for NTS 0->1, 1 is fixed as 1, and 2 is fixed as 0
	 */
	/* -1, fixed 0; 0, NTS 0->1; 1, fixed 1 */
	int hdcp_type_policy;
	int hdcp_tst_sig;
	struct hdcprp_topo *topo_info;
	bool hdcp22_type;

#endif

	enum hdmi_ll_mode ll_user_set_mode; /* ll setting: 0/AUTOMATIC, 1/Always OFF, 2/ALWAYS ON */
	bool ll_enabled_in_auto_mode; /* ll_mode enabled in auto or not */
#ifdef CONFIG_AMLOGIC_HDMITX21
	u32 aon_output:1; /* always output in bl30 */

	u8 def_stream_type;
	u32 is_passthrough_switch;
	bool need_filter_hdcp_off;
	u32 filter_hdcp_off_period;
	bool not_restart_hdcp;
	/* enable poll rx_status for workaround of special hdcp2.2 TV */
	bool en_poll_rx_status;
	/* poll rx_status workaround method:
	 * 0: continuously poll rx_status(2bytes) for 300ms, default method
	 * 1: read only 1 byte of hdcp msg
	 */
	u8 poll_rx_status_mtd;

#endif
	/* hdr info */
	enum hdmi_hdr_transfer hdr_transfer_feature;
	enum hdmi_hdr_color hdr_color_feature;
	unsigned int colormetry;
	/* hdmitx infoframe */
	struct hdmitx_infoframe infoframes;
	/* hdr work */
	struct work_struct work_hdr;
	struct work_struct work_hdr_unmute;
	/* hdr10plus flag */
	unsigned int hdr10plus_feature;
	/* amdv info */
	enum eotf_type hdmi_current_eotf_type;
	enum mode_type hdmi_current_tunnel_mode;
	bool hdmi_current_signal_sdr;
	/*
	 * edid parse debug flag
	 * true: edid parse in hdmitx
	 * false(default): edid parse in drm
	 */
	bool edid_parse_in_hdmitx;
	/* hdmitx bist */
	unsigned int bist_lock:1;

	u32 hdmi_rext; /* Rext resistor */
	u32 pxp_mode:1;
	struct hdmi_config_platform_data config_data;
	u32 irq_hpd;
	u32 irq_viu1_vsync;
	u32 irq_vrr_vsync;
	u32 arc_rx_en;
	struct hdmitx_clk_tree_s hdmitx_clk_tree;
	/*Platform related.*/
	struct notifier_block reboot_nb;
	/* audio related */
	struct notifier_block hdmitx_notifier_nb_a;
	/* task */
	struct task_struct *task_hdcp;
	struct task_struct *task;
	struct delayed_work work_hpd_plugin;
	struct delayed_work work_hpd_plugout;
	struct delayed_work work_internal_intr;
	u8 hpd_event; /* 1, plugin; 2, plugout */
	struct device *pdev; /* for pinctrl*/
	struct pinctrl_state *pinctrl_i2c;
	struct pinctrl_state *pinctrl_default;

#ifdef CONFIG_AMLOGIC_VPU
	struct vpu_dev_s *encp_vpu_dev;
	struct vpu_dev_s *enci_vpu_dev;
	struct vpu_dev_s *hdmi_vpu_dev;
	struct vpu_dev_s *hdmitx_vpu_clk_gate_dev;
#endif
};

void hdmitx_get_init_state(struct hdmitx_common *tx_common,
			   struct hdmitx_common_state *state);

/*******************************hdmitx common api*******************************/
int hdmitx_common_init(struct hdmitx_common *tx_common, struct hdmitx_hw_common *hw_comm);
int hdmitx_common_destroy(struct hdmitx_common *tx_common);
/* modename policy: get vic from name and check if support by rx;
 * return the vic of mode, if failed return HDMI_0_UNKNOWN;
 */
int hdmitx_common_parse_vic_in_edid(struct hdmitx_common *tx_comm, const char *mode);

/* create hdmi_format_para from config and also calc setting from hw; */
int hdmitx_common_build_format_para(struct hdmitx_common *tx_comm,
		struct hdmi_format_para *para, enum hdmi_vic vic, u32 frac_rate_policy,
		enum hdmi_colorspace cs, enum hdmi_color_depth cd, enum hdmi_quantization_range cr);

/* For bootup init: init hdmi_format_para from hw configs.*/
int hdmitx_common_init_bootup_format_para(struct hdmitx_common *tx_comm,
		struct hdmi_format_para *para);

/*edid valid api*/
bool hdmitx_edid_only_support_sd(struct rx_cap *prxcap);
void edid_set_fallback_mode(struct rx_cap *prxcap);

/* Attach platform related functions to hdmitx_common;
 * Currently hdmitx_tracer, hdmitx_uevent_mgr is platform related;
 */
enum HDMITX_PLATFORM_API_TYPE {
	HDMITX_PLATFORM_TRACER = 0,
	HDMITX_PLATFORM_UEVENT,
};

int hdmitx_common_attch_platform_data(struct hdmitx_common *tx_comm,
	enum HDMITX_PLATFORM_API_TYPE type, void *plt_data);

/*
 * Notify hpd event to all outer modules: vpp by vout, drm, userspace
 * bool force_uevent: force send uevent even the hpd state NOT change
 */
int hdmitx_common_notify_ced_status(struct hdmitx_common *tx_comm);
int hdmitx_common_notify_hpd_status(struct hdmitx_common *tx_comm, bool force_uevent);

int hdmitx_set_uevent(struct hdmitx_common *tx_comm, enum hdmitx_event type, int val);

u32 hdmitx_edid_get_hdmi14_4k_vic(u32 vic);
/* packet api */
/* mode = 0 , disable allm; mode 1: set allm; mode -1: */
int hdmitx_common_set_allm_mode(struct hdmitx_common *tx_comm, int mode);

/*
 * avmute function with lock:
 * do set mute when mute cmd from any path;
 * do clear when all path have cleared avmute;
 */
#define AVMUTE_PATH_1 0x80 //mute by avmute sysfs node
#define AVMUTE_PATH_2 0x40 //mute by upstream side request re-auth
#define AVMUTE_PATH_DRM 0x20 //called by drm;
#define AVMUTE_PATH_HDMITX 0x10 //internal use

int hdmitx_common_avmute_locked(struct hdmitx_common *tx_comm,
		int mute_flag, int mute_path_hint);

/*edid tracer post-processing*/
int hdmitx_common_edid_tracer_post_proc(struct hdmitx_common *tx_comm, struct rx_cap *prxcap);
/*read edid raw data and parse edid to rxcap*/
int hdmitx_common_get_edid(struct hdmitx_common *tx_comm);

/*modesetting function*/
int hdmitx_common_do_mode_setting(struct hdmitx_common *tx_comm,
				  struct hdmitx_common_state *new,
				  struct hdmitx_common_state *old);
int hdmitx_common_validate_mode_locked(struct hdmitx_common *tx_comm,
				       struct hdmitx_common_state *new_state,
				       char *mode, char *attr, bool brr_valid);
int hdmitx_common_disable_mode(struct hdmitx_common *tx_comm,
			       struct hdmitx_common_state *new_state);
int set_disp_mode(struct hdmitx_common *tx_comm, const char *mode);

/*packet api*/
int hdmitx_common_setup_vsif_packet(struct hdmitx_common *tx_comm,
	enum vsif_type type, int on, void *param);

unsigned int hdmitx_get_frame_duration(void);

/* hdcp api*/
void hdmitx_set_hdcp_mode(struct hdmitx_common *tx_comm, const char *buf);
int hdmitx_get_hdcp_ver(struct hdmitx_common *tx_comm, char *buf, int len);

/* drm hdcp api */
int drm_hdmitx_common_hdcp_init(void);
int drm_hdmitx_common_hdcp_exit(void);
int drm_hdmitx_common_hdcp_enable(int hdcp_type);
int drm_hdmitx_common_hdcp_disable(void);
int drm_hdmitx_common_hdcp_disconnect(void);
unsigned int drm_hdmitx_common_get_tx_hdcp_cap(void);
unsigned int drm_hdmitx_common_get_rx_hdcp_cap(void);
int drm_hdmitx_common_register_hdcp_notify(struct connector_hdcp_cb *cb);
int drm_hdmitx_common_get_dw_hdcp_topo_info(void);
void set_hdcp_common_instance(struct hdmitx_common *tx_comm);

int hdmitx_get_connector(void);
struct hdmitx_dev *get_hdmitx_device(void);
void hdmitx_common_sw_debugfunc(struct hdmitx_common *tx_comm, const char *cmd_str);
/*******************************hdmitx common api end*******************************/

int hdmitx_register_hpd_cb(struct hdmitx_common *tx_comm, struct connector_hpd_cb *hpd_cb);
int hdmitx_fire_drm_hpd_cb_unlocked(struct hdmitx_common *tx_comm);
int hdmitx_audio_register_ctrl_callback(struct hdmitx_tracer *tracer,
						audio_en_callback cb1, audio_st_callback cb2);

int hdmitx_get_hpd_state(struct hdmitx_common *tx_comm);
u64 hdmitx_get_hpd_hw_sequence_id(struct hdmitx_common *tx_comm);
unsigned char *hdmitx_get_raw_edid(struct hdmitx_common *tx_comm);
bool hdmitx_common_get_ready_state(struct hdmitx_common *tx_comm);
bool hdmitx_common_get_edid_valid_state(struct hdmitx_common *tx_comm);
bool hdmitx_common_get_hdcp_user_state(struct hdmitx_common *tx_comm);
bool hdmitx_common_get_hdmi_used_state(struct hdmitx_common *tx_comm);

int hdmitx_setup_attr(struct hdmitx_common *tx_comm, const char *buf);
int hdmitx_get_attr(struct hdmitx_common *tx_comm, char attr[16]);

int hdmitx_get_hdrinfo(struct hdmitx_common *tx_comm, struct hdr_info *hdrinfo);
int hdmitx_get_hdrinfo_rx(struct hdmitx_common *tx_comm, struct hdr_info *hdrinfo);

int hdmitx_set_hdr_priority(struct hdmitx_common *tx_comm, u32 hdr_priority,
		struct hdr_info *hdr_info, struct dv_info *dv_info);
int hdmitx_get_hdr_priority(struct hdmitx_common *tx_comm, u32 *hdr_priority);
void hdmitx_hdr_state_init(struct hdmitx_common *tx_comm);
bool hdmitx_hdr_en(struct hdmitx_hw_common *tx_hw);
bool hdmitx_dv_en(struct hdmitx_hw_common *tx_hw);
bool hdmitx_hdr10p_en(struct hdmitx_hw_common *tx_hw);

u32 hdmitx_get_frl_bandwidth(const enum frl_rate_enum rate);
u32 hdmitx_calc_frl_bandwidth(u32 pixel_freq, enum hdmi_colorspace cs,
	enum hdmi_color_depth cd);
u32 hdmitx_calc_tmds_clk(u32 pixel_freq,
	enum hdmi_colorspace cs, enum hdmi_color_depth cd);
enum frl_rate_enum hdmitx_select_frl_rate(u8 *dsc_en, u8 dsc_policy, enum hdmi_vic vic,
	enum hdmi_colorspace cs, enum hdmi_color_depth cd);

/*edid related function.*/
bool is_tv_changed(char *cur_edid_chksum, char *boot_param_edid_chksum);

void hdmitx_vout_init(struct hdmitx_common *tx_comm, struct hdmitx_hw_common *tx_hw);
void hdmitx_vout_uninit(void);
struct vinfo_s *hdmitx_get_current_vinfo(void *data);
void update_vinfo_from_formatpara(struct hdmitx_common *tx_comm);
void edidinfo_detach_to_vinfo(struct hdmitx_common *tx_comm);
void edidinfo_attach_to_vinfo(struct hdmitx_common *tx_comm);
void hdrinfo_to_vinfo(struct hdr_info *hdrinfo, struct hdmitx_common *tx_comm);
void set_dummy_dv_info(struct vout_device_s *vdev);
void hdmitx_build_fmt_attr_str(struct hdmitx_common *tx_comm);
void hdmitx_current_status(enum hdmitx_event_log_bits event);
ssize_t hdcp_lstore_show(struct device *dev, struct device_attribute *attr,
				char *buf);
ssize_t hdcp_mode_show(struct device *dev, struct device_attribute *attr,
			      char *buf);
ssize_t hdcp_ver_show(struct device *dev, struct device_attribute *attr,
			     char *buf);

/* work for bootup when hpd is high */
void hdmitx_bootup_plugin_work(struct hdmitx_common *tx_comm);
/* common work for plugin/resume, which is done in lock */
void hdmitx_plugin_common_work(struct hdmitx_common *tx_comm);
/* common work for plugout */
void hdmitx_plugout_common_work(struct hdmitx_common *tx_comm);
/* common edid clear, which is done in lock */
void hdmitx_common_edid_clear(struct hdmitx_common *tx_comm);
/* common work for late resume, which is done in lock */
void hdmitx_common_late_resume(struct hdmitx_common *tx_comm);
/* common disable hdmitx output api */
void hdmitx_common_output_disable(struct hdmitx_common *tx_comm,
	bool phy_dis, bool hdcp_reset, bool pkt_clear, bool edid_clear);
unsigned int hdmitx_get_frame_duration(void);

/*******************************drm hdmitx api*******************************/

unsigned int hdmitx_common_get_contenttypes(void);
int hdmitx_common_set_contenttype(int content_type);
const struct dv_info *hdmitx_common_get_dv_info(void);
const struct dv_info *hdmitx_common_get_dv_info_rx(void);
const struct hdr_info *hdmitx_common_get_hdr_info(void);
const struct hdr_info *hdmitx_common_get_hdr_info_rx(void);
int hdmitx_common_get_vic_list(int **vics);
bool hdmitx_common_chk_mode_attr_sup(char *mode, char *attr);
int hdmitx_common_get_timing_para(int vic, struct drm_hdmitx_timing_para *para);
void hdmitx_audio_notify_callback(struct hdmitx_common *tx_comm,
	struct hdmitx_hw_common *tx_hw_base,
	struct notifier_block *block,
	unsigned long cmd, void *para);
void get_hdmi_efuse(struct hdmitx_common *tx_comm);
enum hdmi_color_depth get_hdmi_colordepth(const struct vinfo_s *vinfo);
enum hdmi_vic hdmitx_get_prefer_vic(struct hdmitx_common *tx_comm, enum hdmi_vic vic);
enum frl_rate_enum get_dsc_frl_rate(enum dsc_encode_mode dsc_mode);
int hdmitx_common_get_hdr_status(void);
u32 hdmitx_common_get_vrr_cap(void);
int hdmitx_common_get_vrr_mode_group(struct hdmitx_vrr_mode_group *group, int max_group);
int hdmitx_common_set_vframe_rate_hint(int rate, void *data);

/* hdmitx diff */
#ifdef CONFIG_AMLOGIC_HDMITX
#define HDCP_SLAVE	0x3a
#define HDCP2_RD_MSG	0x80

unsigned int get_hdcp22_base(void);
bool is_hdcp22_stop_state(void);
void hdmitx_reset_tv_hdcp(void);
u32 ddc_read_1byte(u8 slave, uint8_t offset_addr, uint8_t *data);
void hdmitx_hdcp_do_work(struct hdmitx_common *tx_comm);
int hdmitx20_device_init(struct hdmitx_common *tx_comm);

#endif
#ifdef CONFIG_AMLOGIC_HDMITX21
ssize_t _vrr_cap_show(struct device *dev, struct device_attribute *attr,
	char *buf);
int hdmitx21_device_init(struct hdmitx_common *tx_comm);
#endif

/*
 * below are extern functions declaration
 * only for hdmitx20/21 module internally
 */
typedef void (*pf_callback)(bool st);

#ifdef CONFIG_AMLOGIC_HDMITX
	void hdmitx_earc_hpdst(pf_callback cb);
#else
	#define hdmitx_earc_hpdst NULL
#endif

#ifdef CONFIG_AMLOGIC_HDMITX21
	void hdmitx21_earc_hpdst(pf_callback cb);
#else
	#define hdmitx21_earc_hpdst NULL
#endif

#ifdef CONFIG_AMLOGIC_HDMITX
/* hdmitx20 external interface */

#endif

#ifdef CONFIG_AMLOGIC_HDMITX21
/* hdmitx21 external interface */
#endif

int get_hpd_state(void);
int hdmitx_event_notifier_regist(struct notifier_block *nb);
int hdmitx_event_notifier_unregist(struct notifier_block *nb);
struct vsdb_phyaddr *get_hdmitx_phy_addr(void);
int register_earcrx_callback(pf_callback callback);
void unregister_earcrx_callback(void);
void hdmitx_disable_frl_work(struct hdmitx_common *tx_comm);
void hdmitx_disable_21_work(struct hdmitx_common *tx_comm);
void hdmitx_clear_packets(struct hdmitx_common *tx_comm);
void hdmitx_disable_hdcp(struct hdmitx_common *tx_comm);
int hdmitx_pre_enable_mode(struct hdmitx_common *tx_comm, struct hdmi_format_para *para);
int hdmitx_enable_mode(struct hdmitx_common *tx_comm, struct hdmi_format_para *para);
int hdmitx_post_enable_mode(struct hdmitx_common *tx_comm, struct hdmi_format_para *para);
int hdmitx_disable_mode(struct hdmitx_common *tx_comm, struct hdmi_format_para *para);
int get_hdmitx_init(void);
/*
 * HDMI TX output enable, such as ACRPacket/AudInfo/AudSample
 * enable: 1, normal output; 0: disable output
 */
void hdmitx_ext_set_audio_output(int enable);

/*
 * return Audio output status
 * 1: normal output status; 0: output disabled
 */
int hdmitx_ext_get_audio_status(void);

/*!!DEPRECATED API, DONT USE!!*/
void get_attr(char attr[16]);
void setup_attr(const char *buf);

/* for eARC audio call */
void hdmitx_ext_plugin_handler(void);

#endif
