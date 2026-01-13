/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _RESOURCE_MANAGE_H_
#define _RESOURCE_MANAGE_H_

#include <linux/ioctl.h>
#include <linux/types.h>
#include <trace/events/meson_atrace.h>

#define RESMAN_ENABLE_PARAMETER_SET   (1)
#ifdef RESMAN_ENABLE_PARAMETER_SET
#include <linux/amlogic/media/resource_mgr/param_set.h>
#endif

#define RESMAN_IOC_MAGIC  'R'

#define RESMAN_IOC_QUERY_RES					_IOR(RESMAN_IOC_MAGIC, 0x01, __u32)
#define RESMAN_IOC_ACQUIRE_RES					_IOW(RESMAN_IOC_MAGIC, 0x02, __u32)
#define RESMAN_IOC_RELEASE_RES					_IOR(RESMAN_IOC_MAGIC, 0x03, __u32)
#define RESMAN_IOC_SETAPPINFO					_IOW(RESMAN_IOC_MAGIC, 0x04, __u32)
#define RESMAN_IOC_SUPPORT_RES					_IOR(RESMAN_IOC_MAGIC, 0x05, __u32)
#define RESMAN_IOC_RELEASE_ALL					_IOR(RESMAN_IOC_MAGIC, 0x06, __u32)
#define RESMAN_IOC_LOAD_RES					_IOR(RESMAN_IOC_MAGIC, 0x07, __u32)
#define RESMAN_IOC_GET_SYS_DEBUG_LEVEL				_IOR(RESMAN_IOC_MAGIC, 0x08, __u32)
#define RESMAN_IOC_SET_SYS_DEBUG_LEVEL				_IOR(RESMAN_IOC_MAGIC, 0x09, __u32)
#define RESMAN_IOC_GET_ERROR_INFO				_IOR(RESMAN_IOC_MAGIC, 0x0A, __u32)
#define RESMAN_IOC_GET_COUNTER_INDEX				_IOR(RESMAN_IOC_MAGIC, 0x0B, __u32)

#define RESMAN_IOC_QUERY_RES_V4			_IOWR(RESMAN_IOC_MAGIC, 0x51, struct resman_para)
#define RESMAN_IOC_ACQUIRE_RES_V4		_IOW(RESMAN_IOC_MAGIC, 0x52, struct resman_para)
#define RESMAN_IOC_SETAPPINFO_V4		_IOW(RESMAN_IOC_MAGIC, 0x54, struct app_info)
#define RESMAN_IOC_SUPPORT_RES_V4		_IOW(RESMAN_IOC_MAGIC, 0x55, struct resman_para)
#define RESMAN_IOC_LOAD_RES_V4			_IOW(RESMAN_IOC_MAGIC, 0x57, struct res_item)
#define RESMAN_IOC_SET_SYS_DEBUG_LEVEL_V4	_IOW(RESMAN_IOC_MAGIC, 0x59, struct debug_level)
#define RESMAN_IOC_GET_VERSION_V4		_IOR(RESMAN_IOC_MAGIC, 0x5C, struct resman_version)
#define RESMAN_IOC_PARAMETER_SET_INFO_CTL_V4 \
				_IOWR(RESMAN_IOC_MAGIC, 0x5D, struct resman_param_set_control)
#define RESMAN_IOC_QUERY_INFO_FROM_CORE_V4	_IOR(RESMAN_IOC_MAGIC, 0x5E, __u32)

#define RESMAN_SUPPORT_PREEMPT		1

#define MAIN_INFO                 0x1
#define FULL_INFO                 0x2
#define MODULE_ONE                0x100
#define MODULE_TWO                0x200
#define RESMAN_LEN                (32)
/*
 * bit0   --- Main Path
 * bit1   --- Full Path
 * bit 8  --- Module one
 * bit 9  --- Module two
 */
extern int pipe_kpi_debug;
#define PR_PIPE_KPI_LOG_INFO(level, fmt, ...)				\
	do {									\
		if (pipe_kpi_debug & (level))						\
			pr_info("%s: " fmt, __func__, ##__VA_ARGS__);\
	} while (0)

#define PR_PIPE_KPI_INFO(name, value, level, fmt, ...)					\
	do {											\
		PR_PIPE_KPI_LOG_INFO(level, fmt, ##__VA_ARGS__);				\
		ATRACE_COUNTER_WITH_TAG(KERNEL_ATRACE_TAG_PIPE_KPI, name, value);	\
	} while (0)

#define RESMAN_DEV_VDEC "vdec"
#define RESMAN_DEV_DI   "di"
#define RESMAN_DEV_PP   "PP"

struct resman_para {
	__u32 k;
	union {
		struct {
			__u32 preempt;
			__u32 timeout;
			char arg[RESMAN_LEN];
		} acquire;
		struct {
			char name[RESMAN_LEN];
			__u32 type;
			__s32 value;
			__s32 avail;
			char app_name[RESMAN_LEN]; // appname that acquired this resource
		} query;
		struct {
			char name[RESMAN_LEN];
		} support;
	} v;
};

struct app_info {
	char app_name[RESMAN_LEN];
	__u32 app_type;
	__s32 prio;
};

struct res_item {
	char name[RESMAN_LEN];
	__u32 type;
	char arg[RESMAN_LEN];
};

struct debug_level {
	char debug_info[2048];
	__u32 len;
};

struct resman_version {
	__u32 ver;
	__u32 sub_ver;
	__u32 submit_number;
	__u32 reserved[8];
};

struct resman_param_set_control {
	__u32 cmd;
	__u32 size;
	char name[RESMAN_LEN];
	char desc[RESMAN_LEN];
	char module_name[RESMAN_LEN];
	__u32 domain;
	__u32 sub_domain;
	__u32 instance_id;
	__u32 process_id;
	__u32 thread_id;
	__u32 reserved[10];
	union {
		__s32 value;
		__s64 value64;
		__u64 ptr;
	};
};

enum RESMAN_PARAM_SET_CTL_CMD {
	GET_INT32_INFO = 0,
	GET_INT64_INFO,
	GET_STRING_INFO,
	GET_CUSTOM_INFO,
	GET_ITEM_BY_NAME,
	GET_ITEM_BY_IDX,
	GET_ITEMS_BY_DOMAIN,
	GET_ITEMS_BY_SUB_DOMAIN,
	GET_CODEC_MM_SIZE_MIN,
	GET_CODEC_MM_SIZE_EXPECTED,
	GET_CODEC_MM_SIZE_ACTUAL,
	GET_INSTANCE_SSID,
	SET_INT32_INFO = 500,
	SET_INT64_INFO,
	SET_STRING_INFO,
	SET_CUSTOM_INFO,
	SET_ITEM_BY_NAME,
	SET_ITEM_BY_IDX,
	SET_ITEMS_BY_DOMAIN,
	SET_ITEMS_BY_SUB_DOMAIN,
	UPDATE_CODEC_MM_SIZE_ACQUIRED = 1000,
};

enum RESMAN_ID {
	RESMAN_ID_VFM_DEFAULT,
	RESMAN_ID_AMVIDEO,
	RESMAN_ID_PIPVIDEO,
	RESMAN_ID_SEC_TVP,
	RESMAN_ID_TSPARSER,
	RESMAN_ID_CODEC_MM,
	RESMAN_ID_ADC_PLL,
	RESMAN_ID_DECODER,
	RESMAN_ID_DMX,
	RESMAN_ID_DI,
	RESMAN_ID_HWC,
	RESMAN_ID_VDETE,
	RESMAN_ID_VDPLANE,
	RESMAN_ID_AUDIO_DECODER,
	RESMAN_ID_MAX,
};

enum RESMAN_PIPELINE {
	RESMAN_PLAYBACK_PIPELINE_NONE = 0,
	RESMAN_PLAYBACK_PIPELINE_V4L2_FRAMEMODE,
	RESMAN_PLAYBACK_PIPELINE_V4L2_STREAMMODE,
	RESMAN_PLAYBACK_PIPELINE_AMPORT_STREAMMODE,
	RESMAN_PLAYBACK_PIPELINE_HDMI_DEFAULT,
	RESMAN_PLAYBACK_PIPELINE_COUNT
};

struct resman_cb_dec_status_t {
	int  ssid;
	bool flag;
};

struct resman_cb_est_param_t {
	int video_format;
	int width;
	int height;
	int res_type;
	int dw;
	int margin_num;
	int dpb_num;
	int pipeline;
	bool is_interlace;
	bool is_tvp;
	bool is_android;
	int bitdepth;
};

enum resman_cb_param_type_t {
	RESMAN_CB_PARAM_DEC_STATUS    = 1,      // resman_cb_dec_status_t
	RESMAN_CB_PARAM_EST           = 2,      // resman_cb_est_param_t
	RESMAN_CB_PARAM_CUSTOM        = 3,
};

/**
 * Queries the minimum memory allocation required for basic video decoder (VDEC) pipeline operation
 * RESMAN_MEM_USAGE_MIN      1
 * Queries the theoretical memory allocation size expected? for the video decoder (VDEC) pipeline
 * RESMAN_MEM_USAGE_EXPECTED 2
 * the real-time memory consumption of the video decoder (VDEC) pipeline
 * RESMAN_MEM_USAGE_ACTUAL   3
 */
enum resman_cb_param_subtype_t {
	RESMAN_MEM_USAGE_SUB_NONE   = 0,
	RESMAN_MEM_USAGE_MIN        = 1,
	RESMAN_MEM_USAGE_EXPECTED   = 2,
	RESMAN_MEM_USAGE_ACTUAL     = 3,
};

typedef u32 resman_custom_type_t;

struct resman_cb_param_any_t {
	u32					size;
	enum resman_cb_param_type_t		type;
	enum resman_cb_param_subtype_t		sub_type; // default 0, 0: none sub-type
	resman_custom_type_t			custom_type;
	union {
		struct resman_cb_dec_status_t		dst;
		struct resman_cb_est_param_t		est;
		void					*custom__cb_ptr;
	} v;
};

struct resman_query_cb_ops {
	char device_name[RESMAN_LEN];
	int (*query_resource_cb)(struct resman_cb_param_any_t q);
	struct list_head list;
};

enum RESMAN_TYPE {
	RESMAN_TYPE_COUNTER = 1,
	RESMAN_TYPE_TOGGLE,
	RESMAN_TYPE_TVP,
	RESMAN_TYPE_CODEC_MM,
	RESMAN_TYPE_CAPACITY_SIZE
};

enum RESMAN_APP {
	RESMAN_APP_NONE	= -1,
	RESMAN_APP_OMX	= 0,
	RESMAN_APP_XDVB,
	RESMAN_APP_HDMI_IN,
	RESMAN_APP_SEC_TVP,
	RESMAN_APP_DVBKIT,
	RESMAN_APP_TSPLAYER,
	RESMAN_APP_CODEC2,
	RESMAN_APP_DEBUG_SERVER,
	RESMAN_APP_OTHER    = 10,
	RESMAN_APP_SYSTEM_RESERVED = 100,
	RESMAN_APP_DIAGNOSTICS = 101,
	RESMAN_APP_GST = 200,
};

enum RESMAN_EVENT {
	RESMAN_EVENT_REGISTER		= 0x1000,
	RESMAN_EVENT_UNREGISTER		= 0x1001,
	RESMAN_EVENT_PREEMPT		= 0x1002,
	RESMAN_EVENT_STOP		= 0x1003,
	RESMAN_EVENT_RESREPORT		= 0x1004,
	RESMAN_EVENT_DEBUGEVENT		= 0x1005,
	RESMAN_EVENT_ERRORINFO		= 0x1007
};

typedef void (*debug_callback)(const char *module, const char *debug, int len);

int resman_init(void);
void resman_exit(void);
int resman_register_debug_callback(const char *module, debug_callback callback);
int resman_remove_debug_callback(const char *module);
int resman_notify_error_info(const char *info);
int resman_register_query_ops_by_name(const char *device_name,
				int (*fn)(struct resman_cb_param_any_t q));


#endif/*_RESOURCE_MANAGE_H_*/
