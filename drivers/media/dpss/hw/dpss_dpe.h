/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _DPSS_DPE_H_
#define _DPSS_DPE_H_

#include "dpss_param.h"
//#include "dpss_lib.h"
#include "dpss_intf.h"
#include "vfce.h"
#include "vfcd.h"
#include "decontour.h"

//#include "dpss_vdi.h"

struct PRM_DPSS_DPE {
	enum DPSS_WORK_MODE  dpe_mode;
	u32        nr_en;
	bool            di_en;
	bool            dcntr_en;
	bool		dcntr_en_bk; //ary add 09-05
	bool            nr_copy_mode;
	u32        frm_end_mask;
	struct PRM_DPE_NR_SIZE dpe_nr_size;
	//PRM_DPE_TOP_SIZE_t dpe_top_size;
	struct PRM_INTF_TYPE      prm_wmif0_intf;
	struct PRM_INTF_TYPE      prm_wmif1_intf;
	struct PRM_INTF_TYPE      prm_wmif2_intf;
	struct PRM_INTF_TYPE      prm_ds_wmif0_intf;
	struct PRM_INTF_TYPE      prm_ds_wmif1_intf;
	struct PRM_INTF_TYPE      pix_rmif0;
	struct PRM_INTF_TYPE      pix_rmif1;
	struct PRM_INTF_TYPE      pix_rmif2;
	struct PRM_INTF_TYPE      pix_rmif3;
	struct PRM_DPE_REG_MODE dpe_reg_mode;
	struct PRM_DPE_PAD   prm_dpe_pad;
	struct PRM_DPE_SUB_RMIF prm_dpe_sub_rmif;
	struct PRM_DPE_2CH_RMIF prm_dpe_2ch_rmif;
	struct DCNTR_SLC       dcntr_slc;//ary 2025-02-28
	u32        dpss_dpe_di_frm_cnt;	//yu.zong 2024-12-06
	struct PRM_INTF_TYPE pix_rmif_tmp; //07-10
	u32 aa_pad;
	u32 aa_pad_assign_en;
};

#ifdef MOV
void cfg_dpe_size(enum DPSS_WORK_MODE   dpe_mode,
	//u32 slc_num,
	//u32 me_dsx,
	//u32 me_dsy,
	//struct PRM_DPE_SUB_RMIF prm_dpe_sub_rmif,
	struct PRM_DPSS_TOP *prm_top,
	struct PRM_DPSS_DPE *prm_dpe,
	struct PRM_DPE_PAD dpe_pad);
#endif
void dpss_vbe_proc_byp(u32 path_id);
void cfg_dpe_triggle(struct PRM_DPSS_TOP *prm_top);
void cfg_dpe_dv_triggle(void);
void nrdae_to_nrdpe_rx(int nr_info);
void cfg_dpe_hscale(u32 hsc_en, u32 hsize_in,
	u32 hsize_out, u32 hsc_out_lft_pad, u32 hsc_out_rgt_pad);
#endif

