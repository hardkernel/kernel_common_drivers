/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef AM_PARAM_SET_H
#define AM_PARAM_SET_H

#define RESMAN_ITEM_LEN                (32)

enum RESMAN_PARAM_TYPE {
	PARAM_TYPE_INT,
	PARAM_TYPE_INT64,
	PARAM_TYPE_FLOAT,
	PARAM_TYPE_DOUBLE,
	PARAM_TYPE_STRING,
	PARAM_TYPE_CUSTOM
};

/* Domain and SubDomain enums (fixed set) */
enum RESMAN_DOMAIN_TYPE {
	DOMAIN_NONE = 0,
	DOMAIN_VIDEO,
	DOMAIN_DEVICE,
	DOMAIN_SYSTEM,
    /* add more predefined domains here */
	DOMAIN_COUNT /* sentinel for bounds checking */
};

enum RESMAN_SUB_DOMAIN_TYPE {
	SUB_DOMAIN_NONE = 0,
	SUB_DOMAIN_DECODING,
	SUB_DOMAIN_CHIP,
	SUB_DOMAIN_MEMORY,
	SUB_DOMAIN_PROFILE,
	SUB_DOMAIN_WIDTH,
	SUB_DOMAIN_HEIGHT,
	SUB_DOMAIN_DW,
	SUB_DOMAIN_FORMAT,
	SUB_DOMAIN_MARGIN,
	SUB_DOMAIN_FIELD,
	SUB_DOMAIN_DPB,
	SUB_DOMAIN_SECURE,
	SUB_DOMAIN_PIPELINE,
	SUB_DOMAIN_OUTPUNUM,//output buffer total number
	SUB_DOMAIN_BITDEPTH,
	SUB_DOMAIN_STATUS,
	/* add more predefined subdomains here */
	SUB_DOMAIN_COUNT /* sentinel for bounds checking */
};

enum RESMAN_SET_STATUS {
	RESMAN_STATUS_PREPARING,
	RESMAN_STATUS_PLAYING,
	RESMAN_STATUS_MAX
};

enum RESMAN_SET_PARAM_KEY {
	RESMAN_PARAM_VIDEO_W,
	RESMAN_PARAM_VIDEO_H,
	RESMAN_PARAM_VIDEO_DW,
	RESMAN_PARAM_VIDEO_FMT,
	RESMAN_PARAM_VIDEO_MARGIN,
	RESMAN_PARAM_VIDEO_FD,
	RESMAN_PARAM_VIDEO_DPB,
	RESMAN_PARAM_VIDEO_SECURE,
	RESMAN_PARAM_VIDEO_PIPELINE,
	RESMAN_PARAM_VIDEO_TOTALNUM,//total output number
	RESMAN_PARAM_VIDEO_BITDEPTH,
	RESMAN_PARAM_VIDEO_STATUS,
	RESMAN_PARAM_VIDEO_MAX
};

//TBD
struct resman_custom_media_profile {
	s32   media_profile_id;
	char *profile_name;
};

struct resman_param_item {
	char   name[RESMAN_ITEM_LEN];         /* parameter name */
	__u32  type;         /* parameter type */
	char   module_name[RESMAN_ITEM_LEN];  /* which module filled this item (source); optional */
	__u32  domain;       /* first-level domain (enum) */
	__u32  sub_domain;    /* second-level domain (enum) */
	s32    instance_id; /* parameter instance id */
	s32    process_id;  /* parameter process id */
	s32    thread_id;   /* parameter thread id */
	__u32  reserved[60];

	union {
		s32                   i;
		s64                   i64;
		char                  *s;
		struct resman_param_set               *custom_set;     /* unified name */
		struct resman_custom_media_profile    *custom_profile; /* optional*/
	} value;
	char  desc[RESMAN_ITEM_LEN];            /* description */
};

/* parameter set */
struct resman_param_set {
	struct resman_param_item  *items;
	__u32     size;
	__u32     capacity;
};

struct resman_param_set  *param_set_create(void);
void param_set_destroy(struct resman_param_set  *set);

/* add various types */
int param_set_update_int(struct resman_param_set  *set, const char  *name, int value,
			const char  *desc, const char  *module_name,
			__u32 domain, __u32 sub_domain,
			__u32 instance_id, __u32 process_id, __u32 thread_id);
int param_set_update_int64(struct resman_param_set *set, const char *name, s64 value,
			const char *desc, const char *module_name,
			__u32 domain, __u32 sub_domain,
			__u32 instance_id, __u32 process_id, __u32 thread_id);
int param_set_update_string(struct resman_param_set  *set, const char  *name, char *value,
			const char  *desc, const char  *module_name,
			__u32 domain, __u32 sub_domain,
			__u32 instance_id, __u32 process_id, __u32 thread_id);

/* custom set/profile */
int param_set_update_custom_set(struct resman_param_set  *set, const char  *name,
			struct resman_param_set  *custom,
			const char  *desc, const char  *module_name,
			__u32 domain, __u32 sub_domain,
			__u32 instance_id, __u32 process_id, __u32 thread_id);

int param_set_update_custom_profile(struct resman_param_set  *set, const char  *name,
			struct resman_custom_media_profile  *profile,
			const char  *desc, const char  *module_name,
			__u32 domain, __u32 sub_domain,
			__u32 instance_id, __u32 process_id, __u32 thread_id);

/* retrieval by name/index */
struct resman_param_item  *param_set_get_by_name(struct resman_param_set  *set, const char  *name);
struct resman_param_item  *param_set_get_by_index(struct resman_param_set  *set, size_t idx);

/* retrieval by domain/sub_domain (two-level query)
 * Returns a newly allocated array of struct resman_param_item* on success, or NULL if none.
 * out_count is set to the number of matches (0 if none).
 * Caller must kfree() the returned array, but NOT the struct resman_param_item elements.
 */
struct resman_param_item  **param_set_get_by_domain(struct resman_param_set  *set,
				__u32 domain, __u32 sub_domain, size_t *out_count);

/* debug */
void param_set_trace(struct resman_param_set  *set, int indent);

/* helper for nested custom profile tracing (kept public if needed elsewhere) */
void trace_custom_profile(struct resman_custom_media_profile  *p, int indent);

/* Optional helpers: enum<->string conversions (return NULL if OOR) */
const char  *domain_to_string(__u32 d);
const char  *sub_domain_to_string(__u32 sd);
const char *get_resman_param_key_string(__u32 key);

#endif /* AM_PARAM_SET_H */
