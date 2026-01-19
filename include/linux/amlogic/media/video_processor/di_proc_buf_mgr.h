/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef _DI_PROC_BUF_MGR_H
#define _DI_PROC_BUF_MGR_H

#include <linux/types.h>
#include <linux/atomic.h>
#include <linux/amlogic/media/vfm/vframe.h>

#define VF_LIST_COUNT 64

#define VF_REF_FLAG_DIPROCESS_CHECKIN             0x1
#define VF_REF_FLAG_DECREASE_SELF                 0x2

enum dec_type_t {
	DEC_TYPE_V4L_DEC = 0,
	DEC_TYPE_V4LVIDEO = 1,
	DEC_TYPE_TVIN = 2,
	DEC_TYPE_VDEC_CORE_I = 3,/*define for vdec core interlace*/
	DEC_TYPE_MAX = 0xff,
};

struct vf_ref_t {
	int index; /*0 1 2....*/
	int frame_index; /*for debug*/
	struct vframe_s *vf_p;
	//struct vframe_s *src_vf;
	struct vframe_s vf;
	int ref_count; /*ref by other frame, */
	int ref_number; /*ref by other frame*/
	int ref_other_number;/*ref other  number*/
	struct file *file;
	struct file *file_ext;/*used for h264 decoder interlace*/
	atomic_t on_use;
	bool di_processed;
	int buf_mgr_reset_id;
	int qbuf_id;
	u32 flag;
};

struct dp_buf_mgr_t {
	int dec_type;
	int instance_id;
	//struct vframe_s *vf_2;   /*last frame n-2*/
	//struct vframe_s *vf_1;   /*last frame n-1*/
	struct vf_ref_t *ref_list_2; /*last frame n-2*/
	struct vf_ref_t *ref_list_1; /*last frame n-1*/
	struct vf_ref_t ref_list[VF_LIST_COUNT];
	int get_count;
	void *caller_data;
	void (*recycle_buffer_cb)(void *caller_data, struct file *file, int instance_id);
	struct mutex ref_count_mutex; /*for ref_count*/
	struct mutex file_mutex; /*for file release mutex*/
	int reset_id;
	int receive_count;
};

struct uvm_di_mgr_t {
	//struct vframe_s *vf;
	//struct vframe_s *vf_1;   //n-1
	//struct vframe_s *vf_2;   //n-2
	//bool di_processed_flag;  /*true: di process done; false: dropped*/
	struct dp_buf_mgr_t *buf_mgr;
	struct file *file;
	struct uvm_hook_mod *uhmod_v4lvideo;
	struct uvm_hook_mod *uhmod_dec;
};

int buf_mgr_free_checkin(struct dp_buf_mgr_t *buf_mgr, struct file *file);
void buf_mgr_file_lock(struct uvm_di_mgr_t *uvm_di_mgr);
void buf_mgr_file_unlock(struct uvm_di_mgr_t *uvm_di_mgr);
int update_di_process_state(struct file *file);

/**
 * @brief  get_buf_mgr  get buf mgr
 *
 * @param[in]  file
 *
 * @return     struct buf_manager_t * for  success, or NULL for fail
 */
struct dp_buf_mgr_t *get_buf_mgr(struct file *file);

/**
 * @brief  buf_mgr_creat  creat buf mgr instance
 *
 * @param[in]  dec_type    dec_type
 * @param[in]  id    decoder instance id, need always ++ after system boot
 * @param[in]  caller_data   decoder instance dev
 * @param[in]  recycle_buffer_cb   recycle vf to dec
 *
 * @return     struct buf_manager_t * for  success, or NULL for fail
 */
struct dp_buf_mgr_t *buf_mgr_creat(int dec_type, int id, void *caller_data,
	void (*recycle_buffer_cb)(void *, struct file *, int));

/**
 * @brief  buf_mgr_release  release buf mgr instance
 *
 * @param[in]  buf_manager_t *    buf_manager_t
 *
 * @return     0 for  success, or fail type if < 0
 */
int buf_mgr_release(struct dp_buf_mgr_t *buf_mgr);

/**
 * @brief  buf_mgr_reset  reset buf mgr instance for seek
 *
 * @param[in]  dp_buf_mgr_t *    dp_buf_mgr_t
 *
 * @return     0 for  success, or fail type if < 0
 */
int buf_mgr_reset(struct dp_buf_mgr_t *buf_mgr);

/**
 * @brief  buf_mgr_dq_checkin  dq buffer from decoder need checkin vf info
 *
 * @param[in]  buf_manager_t *buf_manager
 * @param[in]  file    file
 *
 * @return     0 for  success, or fail type if < 0
 */
int buf_mgr_dq_checkin(struct dp_buf_mgr_t *buf_mgr, struct file *file);

/**
 * @brief  buf_mgr_q_checkin  q buffer from hal need checkin vf info
 *
 * @param[in]  file    file
 *
 * @return     0 for  success, or fail type if < 0
 */
int buf_mgr_q_checkin(struct dp_buf_mgr_t *buf_mgr, struct file *file);

/**
 * @brief  buf_mgr_q_checkin_dec  q buffer from hal need checkin vf info
 *
 * @param[in]  file    file
 * @param[in]  file_ext    used for H264 interlace uvm_dma
 *
 * @return     0 for  success, or fail type if < 0
 */
int buf_mgr_q_checkin_dec(struct dp_buf_mgr_t *buf_mgr, struct file *file,
	struct file *file_ext);

/**
 * @brief  get_di_proc_enable  judge whether the di post is enable
 *
 * @return  1 for enable, 0 for disable
 */
int get_di_proc_enable(void);

/**
 * @brief  set_di_proc_enable  judge whether the di post is enable
 *
 *@param[in]  1 for enable, 0 for disable
 *
 * @return  0 for  success, or fail type if < 0
 */
int set_di_proc_enable(int enable);

/**
 * @brief  get_buf_mgr_print_flag  get buf manager print flag
 *
 *@param[in]
 *
 * @return  current print flag
 */
int get_buf_mgr_print_flag(void);

/**
 * @brief  set_buf_mgr_print_flag: set buf manager print flag
 *
 *@param[in]  print flag
 *
 * @return  0 for  success, or fail type if < 0
 */
int set_buf_mgr_print_flag(int flag);

/**
 * @brief  buf_mgr_set_eos: set eos signal for dpss
 *
 *@param[in]
 *
 * @return
 */
#ifdef CONFIG_AMLOGIC_DPSS_PROCESS
void buf_mgr_set_eos(int print_flag, struct vf_ref_t *vf_ref);
#endif

/**
 * @brief  get_di_backend_mem: get di backend keeped memory
 *
 *@param[in]  width              width of input resolution
 *@param[in]  height             height of input resolution
 *@param[in]  source_type        0 for progressive, 1 for interlace
 *
 * @return  di backend keeped memory
 */
int get_di_backend_need_mem(int width, int height, int source_type);
#endif /* _DI_PROC_BUF_MGR_H */
