/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _MESON_OSD_GFCD_H_
#define _MESON_OSD_GFCD_H_

/*s6 gfcd div*/
#define VPP_OSD_HDR_DIV_ALPHA                      0x1d7f

/*a9 gfcd div*/
#define VPP_OSD1_HDR_DIV_ALPHA                     0x1a87
#define VPP_OSD2_HDR_DIV_ALPHA                     0x1a8b
#define VPP_OSD3_HDR_DIV_ALPHA                     0x1a8c

#define GFCD_FRM_BGN                               0x5950
#define GFCD_FRM_END                               0x5951
#define GFCD_CLK_CTRL                              0x5952
#define GFCD_SW_RESET                              0x5953
#define GFCD_DBG_CTRL                              0x5954
#define GFCD_TOP_CTRL                              0x5955
#define GFCD_AFRC_CTRL                             0x5956
#define GFCD_AFRC_PARAM                            0x5957
#define GFCD_AFBC_CTRL                             0x5958
#define GFCD_FRM_SIZE                              0x5959
#define GFCD_RO_AM_G0                              0x595a
#define GFCD_RO_AM_G1                              0x595b
#define GFCD_RO_SOLID_G0                           0x595c
#define GFCD_RO_SOLID_G1                           0x595d
#define GFCD_RO_SOLID_G2                           0x595e
#define GFCD_RO_COMB_G0                            0x595f
#define GFCD_RO_CUT_G0                             0x5960
#define GFCD_RO_CUT_G1                             0x5961

#define GFCD_GLB_CLK_CTRL                          0x59f0
#define GFCD_GLB_SW_RST                            0x59f1
#define GFCD_MIF_CLK_CTRL                          0x5900
#define GFCD_MIF_SW_RST                            0x5901
#define GFCD_MIF_MODE_SEL                          0x5902
#define GFCD_MIF_HEAD_CTRL0                        0x5903
#define GFCD_MIF_HEAD_BADDR                        0x5904
#define GFCD_MIF_HEAD_URGENT                       0x5905
#define GFCD_MIF_HEAD_DIMENSION                    0x5906
#define GFCD_MIF_BODY_CTRL                         0x5907
#define GFCD_MIF_BODY_BADDR                        0x5908
#define GFCD_MIF_BODY_URGENT                       0x5909
#define GFCD_MIF_BODY_DIMENSION                    0x590a
#define GFCD_RO_HEAD_G0                            0x590b
#define GFCD_RO_HEAD_G1                            0x590c
#define GFCD_RO_HEAD_G2                            0x590d
#define GFCD_RO_HEAD_G3                            0x590e
#define GFCD_RO_BODY_G0                            0x590f
#define GFCD_RO_BODY_G1                            0x5910
#define GFCD_RO_BODY_G2                            0x5911
#define GFCD_RO_MIF_G0                             0x5912
#define GFCD_RO_MIF_G1                             0x5913

//offset 0x70
//#define GFCD_MIF_MODE_SEL_S1                       0x591e
//#define GFCD_MIF_HEAD_BADDR_S1                     0x5920
//#define GFCD_MIF_BODY_BADDR_S1                     0x5924
//#define GFCD_FRM_BGN_S1                            0x596c
//#define GFCD_FRM_END_S1                            0x596d
//#define GFCD_TOP_CTRL_S1                           0x5971
//#define GFCD_AFRC_CTRL_S1                          0x5973
//#define GFCD_AFBC_CTRL_S1                          0x5974
//#define GFCD_FRM_SIZE_S1                           0x5975
//#define GFCD_MIF_HEAD_CTRL0_S1                     0x591f
//#define GFCD_MIF_BODY_CTRL_S1                      0x5923

#define GFCD_MIF_MODE_SEL_S1                       0x5972
#define GFCD_MIF_HEAD_BADDR_S1                     0x5974
#define GFCD_MIF_BODY_BADDR_S1                     0x5978
#define GFCD_FRM_BGN_S1                            0x59c0
#define GFCD_FRM_END_S1                            0x59c1
#define GFCD_TOP_CTRL_S1                           0x59c5
#define GFCD_AFRC_CTRL_S1                          0x59c6
#define GFCD_AFBC_CTRL_S1                          0x59c8
#define GFCD_FRM_SIZE_S1                           0x59c9
#define GFCD_MIF_HEAD_CTRL0_S1                     0x5973
#define GFCD_MIF_BODY_CTRL_S1                      0x5977
#define GFCD_RO_SOLID_G0_S1                        0x59cc
#define GFCD_RO_SOLID_G1_S1                        0x59cd
#define GFCD_RO_SOLID_G2_S1                        0x59ce

//GFCD S0 For A9
#define GFCD_GLB_CLK_CTRL_A9                          0x45f0
#define GFCD_GLB_SW_RST_A9                            0x45f1

#define GFCD_FRM_BGN_A9                               0x4550
#define GFCD_FRM_END_A9                               0x4551
#define GFCD_CLK_CTRL_A9                              0x4552
#define GFCD_SW_RESET_A9                              0x4553
#define GFCD_DBG_CTRL_A9                              0x4554
#define GFCD_TOP_CTRL_A9                              0x4555
#define GFCD_AFRC_CTRL_A9                             0x4556
#define GFCD_AFRC_PARAM_A9                            0x4557
#define GFCD_AFBC_CTRL_A9                             0x4558
#define GFCD_FRM_SIZE_A9                              0x4559
#define GFCD_RO_SOLID_G0_A9                           0x455c
#define GFCD_RO_SOLID_G1_A9                           0x455d
#define GFCD_RO_SOLID_G2_A9                           0x455e
#define GFCD_GLB_ALPHA_A9                             0X4562

#define GFCD_MIF_CLK_CTRL_A9                          0x4500
#define GFCD_MIF_SW_RST_A9                            0x4501
#define GFCD_MIF_MODE_SEL_A9                          0x4502
#define GFCD_MIF_HEAD_CTRL0_A9                        0x4503
#define GFCD_MIF_HEAD_BADDR_A9                        0x4504
#define GFCD_MIF_HEAD_URGENT_A9                       0x4505
#define GFCD_MIF_HEAD_DIMENSION_A9                    0x4506
#define GFCD_MIF_BODY_CTRL_A9                         0x4507
#define GFCD_MIF_BODY_BADDR_A9                        0x4508
#define GFCD_MIF_BODY_URGENT_A9                       0x4509
#define GFCD_MIF_BODY_DIMENSION_A9                    0x450a

//GFCD S1 For A9
#define GFCD_FRM_BGN_S1_A9                               0x45c0
#define GFCD_FRM_END_S1_A9                               0x45c1
#define GFCD_CLK_CTRL_S1_A9                              0x45c2
#define GFCD_SW_RESET_S1_A9                              0x45c3
#define GFCD_DBG_CTRL_S1_A9                              0x45c4
#define GFCD_TOP_CTRL_S1_A9                              0x45c5
#define GFCD_AFRC_CTRL_S1_A9                             0x45c6
#define GFCD_AFRC_PARAM_S1_A9                            0x45c7
#define GFCD_AFBC_CTRL_S1_A9                             0x45c8
#define GFCD_FRM_SIZE_S1_A9                              0x45c9
#define GFCD_GLB_ALPHA_S1_A9                             0X45d2

#define GFCD_MIF_CLK_CTRL_S1_A9                          0x4570
#define GFCD_MIF_SW_RST_S1_A9                            0x4571
#define GFCD_MIF_MODE_SEL_S1_A9                          0x4572
#define GFCD_MIF_HEAD_CTRL0_S1_A9                        0x4573
#define GFCD_MIF_HEAD_BADDR_S1_A9                        0x4574
#define GFCD_MIF_HEAD_URGENT_S1_A9                       0x4575
#define GFCD_MIF_HEAD_DIMENSION_S1_A9                    0x4576
#define GFCD_MIF_BODY_CTRL_S1_A9                         0x4577
#define GFCD_MIF_BODY_BADDR_S1_A9                        0x4578
#define GFCD_MIF_BODY_URGENT_S1_A9                       0x4579
#define GFCD_MIF_BODY_DIMENSION_S1_A9                    0x457a

//GFCD S2 For A9
#define GFCD_GLB_CLK_CTRL_S2_A9                          0x46f0
#define GFCD_GLB_SW_RST_S2_A9                            0x46f1

#define GFCD_FRM_BGN_S2_A9                               0x4650
#define GFCD_FRM_END_S2_A9                               0x4651
#define GFCD_CLK_CTRL_S2_A9                              0x4652
#define GFCD_SW_RESET_S2_A9                              0x4653
#define GFCD_DBG_CTRL_S2_A9                              0x4654
#define GFCD_TOP_CTRL_S2_A9                              0x4655
#define GFCD_AFRC_CTRL_S2_A9                             0x4656
#define GFCD_AFRC_PARAM_S2_A9                            0x4657
#define GFCD_AFBC_CTRL_S2_A9                             0x4658
#define GFCD_FRM_SIZE_S2_A9                              0x4659
#define GFCD_GLB_ALPHA_S2_A9                             0X4662

#define GFCD_MIF_CLK_CTRL_S2_A9                          0x4600
#define GFCD_MIF_SW_RST_S2_A9                            0x4601
#define GFCD_MIF_MODE_SEL_S2_A9                          0x4602
#define GFCD_MIF_HEAD_CTRL0_S2_A9                        0x4603
#define GFCD_MIF_HEAD_BADDR_S2_A9                        0x4604
#define GFCD_MIF_HEAD_URGENT_S2_A9                       0x4605
#define GFCD_MIF_HEAD_DIMENSION_S2_A9                    0x4606
#define GFCD_MIF_BODY_CTRL_S2_A9                         0x4607
#define GFCD_MIF_BODY_BADDR_S2_A9                        0x4608
#define GFCD_MIF_BODY_URGENT_S2_A9                       0x4609
#define GFCD_MIF_BODY_DIMENSION_S2_A9                    0x460a

#define VIU_OSD1_PATH_CTRL                         0x1a76
#define VIU_OSD2_PATH_CTRL                         0x1a77
#define VIU_OSD3_PATH_CTRL                         0x1a78
#define VIU_OSD4_PATH_CTRL                         0x1a79
#define VIU_OSD5_PATH_CTRL                         0x1a86

struct gfcd_reg_s {
	u32 gfcd_mif_mode_sel;
	u32 gfcd_mif_head_baddr;
	u32 gfcd_mif_body_baddr;
	u32 gfcd_frm_bgn;
	u32 gfcd_frm_end;
	u32 gfcd_top_ctrl;
	u32 gfcd_afrc_ctrl;
	u32 gfcd_afbc_ctrl;
	u32 gfcd_frm_size;
	u32 gfcd_mif_head_ctrl0;
	u32 gfcd_mif_body_ctrl;
	u32 gfcd_ro_solid_g0;
	u32 vpp_osd_hdr_div_alpha;
	u32 gfcd_glb_alpha;
};

enum gfcd_core_e {
	GFCD_CORE1 = 0,
	GFCD_CORE2,
};

extern const struct meson_drm_format_info *
	__meson_drm_gfcd_format_info(u32 format);

#endif
