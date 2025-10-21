// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifdef NO_USED
// dpss_top_regs
//     // size/ratio/addr
//     dpss_frc_init_cfg (  );
//     dpss_frc_basic_cfg (  );
//
//     dpss_frc_bededit_cfg();
//     dpss_frc_tbc_cfg (  );
//         dpss_frc_amtbc_cfg (  );
//             if ( frc_me_share_en )
//                 dpss_frc_nrtbc_cfg
//             else
//                 dpss_frc_nrtbc_cfg (  );
//
//         dpss_frc_phstab_tbc_cfg (  );
//             // cfg the phase table and the buf addr.
//             dpss_frc_lut_cfg (  ); //
//     dpss_vdi_tbc_cfg (  );

//void dpss_frc_tbc_cfg{
//	uint32_t  inp_ofrm_idx_sel   ;  // 1bit, inp out frame idx sel film det phase or calc phase
//	uint32_t  frc_fbuf_ctrl_mode ; // 1bit, 0: phase tab ctrl, 1: automatic ctrl
//	uint32_t  inp_obuf_num = 4;
//	uint32_t  inp_ibuf_num = 4;
//	uint32_t  dae_ibuf_num = 4;
//	uint32_t  dae_obuf_num = 4;
//	uint32_t  inp_ofrm_loop_num ; // 8bit
//	uint32_t  inp_ofrm_loop_tab ; // 32b
//	uint32_t  dae_loop_step_num ; // 5b
//	uint32_t  dae_loop_step_tab[8] ; // 8x32b
//	uint32_t  inp_nr_link_mode  ; // 1bit frcnr_me_share_mode
//
//	uint32_t  frc_nr_en ;  // 2bit
//	uint32_t  mc_phsx2_mode_en  ;  // 1bit
//	uint32_t  frc_src_idx       ;  // 4bit
//
//	uint32_t  frc_film_mode_hw ;  // 8bit
//	uint32_t  frc_film_phs_hw  ;  // 8bit
//	uint32_t  frm_hsize_in  ;  // 13bit
//	uint32_t  frm_vsize_in  ;  // 13bit
//	uint32_t  me_dsx_scale  ;  // 2bit
//	uint32_t  me_dsy_scale  ;  // 2bit
//	uint32_t  me_logo_dsx   ;  // 2bit
//	uint32_t  me_logo_dsy   ;  // 2bit
//	uint32_t  dpss_frc_en   ;  // 1bit
//}
//	if ( film_mode==PD32 ) {
//		inp_ofrm_loop_num = 5;
//		inp_ofrm_loop_tab = 32'b01001;
//		dae_loop_step_num = 5;
//		for(i=0;i<dae_loop_step_num;i++) {
//		dae_loop_step_tab[i] = loop_step_tab_film32[i];
//	}
//}
//
//	//dpss_frc_amtbc_cfg(  );
//	if ( frc_me_share_en ) {
//		w_reg_bit(FRC_AUTO_MODE_ENABLE, inp_ofrm_idx_sel, 4, 1);
//		w_reg_bit(FRC_AUTO_MODE_ENABLE, frc_fbuf_ctrl_mode, 0, 1);
//
//		w_reg_bit(FRC_AUTO_MODE_BUFF_NUM, dae_obuf_num, 24, 5);
//		w_reg_bit(FRC_AUTO_MODE_BUFF_NUM, dae_ibuf_num, 16, 5);
//		w_reg_bit(FRC_AUTO_MODE_BUFF_NUM, inp_obuf_num, 8, 5);
//		w_reg_bit(FRC_AUTO_MODE_BUFF_NUM, inp_ibuf_num, 0, 5);
//
//		w_reg_bit(FRC_AUTO_MODE_LOOP_CTRL, inp_ofrm_loop_num, 8, 8);
//		w_reg_bit(FRC_AUTO_MODE_LOOP_CTRL, dae_loop_step_num, 0, 5);
//
//		w_reg_bit(FRC_AUTO_MODE_LOOP_TAB, inp_ofrm_loop_tab, 0, 32);
//
//		w_reg_bit(FRC_AUTO_MODE_INP_NR_LINK, inp_nr_link_mode, 0, 1);
//
//		w_reg_bit(FRC_AUTO_MODE_STEP_TAB0, dae_loop_step_tab[0], 0, 32);
//		w_reg_bit(FRC_AUTO_MODE_STEP_TAB1, dae_loop_step_tab[1], 0, 32);
//		w_reg_bit(FRC_AUTO_MODE_STEP_TAB2, dae_loop_step_tab[2], 0, 32);
//		w_reg_bit(FRC_AUTO_MODE_STEP_TAB3, dae_loop_step_tab[3], 0, 32);
//		w_reg_bit(FRC_AUTO_MODE_STEP_TAB4, dae_loop_step_tab[4], 0, 32);
//		w_reg_bit(FRC_AUTO_MODE_STEP_TAB5, dae_loop_step_tab[5], 0, 32);
//		w_reg_bit(FRC_AUTO_MODE_STEP_TAB6, dae_loop_step_tab[6], 0, 32);
//		w_reg_bit(FRC_AUTO_MODE_STEP_TAB7, dae_loop_step_tab[7], 0, 32);
//
//		w_reg_bit(DPSS_TOP_FUNC_CTRL, frc_src_idx, 16, 4);
//		w_reg_bit(DPSS_TOP_FUNC_CTRL, frc_nr_en, 4, 2);
//		w_reg_bit(DPSS_TOP_FUNC_CTRL, mc_phsx2_mode_en, 0, 1);
//	}
//    //else
//    //    dpss_frc_nrtbc_cfg(  );
////
////        dpss_frc_phstab_tbc_cfg (  );
////            // cfg the phase table and the buf addr.
////            dpss_frc_lut_cfg (  ); //
////    dpss_vdi_tbc_cfg (  );
//}
//
//
//
//void dpss_frc_cfg(DPSS_FRC_t *frc_tbc,uint32_t frm_hsize,uint32_t frm_vsize)
//{
//	frc_tbc->buf_hsize     = frm_hsize;
//	frc_tbc->buf_vsize     = frm_vsize;
//
//	frc_tbc->buf_bits_mode = 0;
//	frc_tbc->buf_fmt_mode  = 1;
//	frc_tbc->buf_separate  = 0;
//	frc_tbc->buf_full_pack = 1;
//
//	w_reg_bit(DPSS_DAE_FRM_RST, 0x1, 0, 1);//pls_dpss_frm_rst
//}
#endif

