// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include "d_define.h"

#ifdef RUN_ON_ARM
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/kfifo.h>

#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
#include <linux/amlogic/media/amvecm/amvecm.h>
#endif
#endif

#include "../dpss/dpss_base.h"
#include "../dpss/dpss_s.h"
#include "../dpss/dpss_sys.h"
#include "../dpss/dpss_func.h"

unsigned int dpss_sub_support_hdr(struct dpss_ch_s *pch, struct vframe_s *vfm)
{
	//bool support = false;

	if (!(pch->c.work_mode_cfg & D_W_B(HDR)))
		return 0;
#ifdef FUNC_EN_HDR
	//to-do
#endif /* FUNC_EN_HDR */
	//if (support)
	//	return D_W_B(HDR);
	return 0;
}

unsigned int dpss_sub_chg_hdr(struct dpss_ch_s *pch, struct vframe_s *vfm)
{
	//unsigned int chg = 0;

#ifdef FUNC_EN_HDR
		//to-do
#endif /* FUNC_EN_HDR */

	//if (chg)
	//	return D_W_B(HDR);
	return 0;
}

void dpss_hdr_init(struct vframe_s *vfm)
{
#ifdef FUNC_EN_HDR
#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
	amvecm_hdr_init_for_dpss(vfm);
#endif
#endif /* FUNC_EN_HDR */
}

void dpss_hdr_sw(bool sw, struct vframe_s *vfm)
{
#ifdef FUNC_EN_HDR
#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
	if (sw) {
		//amvecm_set_dpss_mode(1);
		amvecm_update_hdr_path_for_dpss(vfm);
	}
#endif
#endif /* FUNC_EN_HDR */
}

void dpss_hdr_proc(struct vframe_s *vfm)
{
#ifdef FUNC_EN_HDR
	//dpss_v_rdma_config_b();
#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
	amvecm_hdr_process_for_dpss(vfm);
#endif
#endif /* FUNC_EN_HDR */
}

void dpss_hdr_pause(void)
{			//for multi dpss, 2nd ch need disable hdr
#ifdef FUNC_EN_HDR

#endif /* FUNC_EN_HDR */
}

void dpss_hdr_disable(void)
{
#ifdef FUNC_EN_HDR
#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
	amvecm_set_muxio_link_for_dpss(0, NULL, VPP_VCBUS);
#endif
#endif /* FUNC_EN_HDR */
}

