/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPTX_HDCP_HW_H__
#define __DPTX_HDCP_HW_H__

#include <linux/types.h>
#include <linux/kernel.h>
#include "dptx_hw.h"

#define DPTX_HDCP2_RXFIFO_LEN 0x460 // TODO
#define DPTX_HDCP2_RXFIFO_DAT 0x464 // TODO
#define MAX_FIFO_SIZE 600

// ------------------------------------------------------------
//   HDCP2 CORE
// ------------------------------------------------------------
#define DP_CP2TX_CTRL_0_IVCTX           0x1800
#define DP_CP2TX_CTRL_1_IVCTX           0x1801
#define DP_CP2TX_CTRL_2_IVCTX           0x1802
#define DP_CP2TX_INTR0_IVCTX            0x1803
#define DP_CP2TX_INTR1_IVCTX            0x1804
#define DP_CP2TX_INTR2_IVCTX            0x1805
#define DP_CP2TX_INTR3_IVCTX            0x1806
#define DP_CP2TX_INTR0_MASK_IVCTX       0x1807
#define DP_CP2TX_INTR1_MASK_IVCTX       0x1808
#define DP_CP2TX_INTR2_MASK_IVCTX       0x1809
#define DP_CP2TX_INTR3_MASK_IVCTX       0x180a
#define DP_CP2TX_INTRSTATUS_IVCTX       0x180b
#define DP_CP2TX_AUTH_STAT_IVCTX        0x180c
#define DP_CP2TX_STATE_IVCTX            0x180d
#define DP_CP2TX_GEN_STATUS_IVCTX       0x180e
#define DP_CP2TX_ROSC_BIST_IVCTX        0x180f
#define DP_CP2TX_TP0_IVCTX              0x1810
#define DP_CP2TX_TP1_IVCTX              0x1811
#define DP_CP2TX_TP2_IVCTX              0x1812
#define DP_CP2TX_TP3_IVCTX              0x1813
#define DP_CP2TX_TP4_IVCTX              0x1814
#define DP_CP2TX_TP5_IVCTX              0x1815
#define DP_CP2TX_TP6_IVCTX              0x1816
#define DP_CP2TX_TP7_IVCTX              0x1817
#define DP_CP2TX_TP8_IVCTX              0x1818
#define DP_CP2TX_TP9_IVCTX              0x1819
#define DP_CP2TX_TP10_IVCTX             0x181a
#define DP_CP2TX_TP11_IVCTX             0x181b
#define DP_CP2TX_TP12_IVCTX             0x181c
#define DP_CP2TX_TP13_IVCTX             0x181d
#define DP_CP2TX_TP14_IVCTX             0x181e
#define DP_CP2TX_TP15_IVCTX             0x181f
#define DP_CP2TX_GP_IN0_IVCTX           0x1820
#define DP_CP2TX_GP_IN1_IVCTX           0x1821
#define DP_CP2TX_GP_IN2_IVCTX           0x1822
#define DP_CP2TX_GP_IN3_IVCTX           0x1823
#define DP_CP2TX_GP_IN4_IVCTX           0x1824
#define DP_CP2TX_GP_IN5_IVCTX           0x1825
#define DP_CP2TX_GPCTL_IVCTX            0x1826
#define DP_CP2TX_GP_OUT0_IVCTX          0x1827
#define DP_CP2TX_GP_OUT1_IVCTX          0x1828
#define DP_CP2TX_GP_OUT2_IVCTX          0x1829
#define DP_CP2TX_GP_OUT3_IVCTX          0x182a
#define DP_CP2TX_GP_OUT4_IVCTX          0x182b
#define DP_CP2TX_GP_OUT5_IVCTX          0x182c
#define DP_CP2TX_RX_ID_CORE_0_IVCTX     0x182d
#define DP_CP2TX_RX_ID_CORE_1_IVCTX     0x182e
#define DP_CP2TX_RX_ID_CORE_2_IVCTX     0x182f
#define DP_CP2TX_RX_ID_CORE_3_IVCTX     0x1830
#define DP_CP2TX_RX_ID_CORE_4_IVCTX     0x1831
#define DP_CP2TX_RPT_DETAIL_IVCTX       0x1832
#define DP_CP2TX_RPT_SMNG_K_IVCTX       0x1833
#define DP_CP2TX_RPT_DEPTH_IVCTX        0x1834
#define DP_CP2TX_RPT_DEVCNT_IVCTX       0x1835
#define DP_CP2TX_SEQ_NUM_V_0_IVCTX      0x1836
#define DP_CP2TX_SEQ_NUM_V_1_IVCTX      0x1837
#define DP_CP2TX_SEQ_NUM_V_2_IVCTX      0x1838
#define DP_CP2TX_SEQ_NUM_M_0_IVCTX      0x1839
#define DP_CP2TX_SEQ_NUM_M_1_IVCTX      0x183a
#define DP_CP2TX_SEQ_NUM_M_2_IVCTX      0x183b
#define DP_CP2TX_IPT_CTR_7to0_IVCTX     0x183c
#define DP_CP2TX_IPT_CTR_15to8_IVCTX    0x183d
#define DP_CP2TX_AESCTL_IVCTX           0x183e
#define DP_CP2TX_TX_CTRL_0_IVCTX        0x1870
#define DP_CP2TX_TX_STATUS_IVCTX        0x1871
#define DP_CP2TX_TX_RPT_SMNG_IN_IVCTX   0x1872
#define DP_CP2TX_RPT_RCVID_OUT_IVCTX    0x1873
#define DP_CP2TX_CIPHER_CTL2_IVCTX      0x1874
#define DP_CP2TX_GP0_IVCTX              0x1875
#define DP_CP2TX_GP1_IVCTX              0x1876

//==================== HDCP2XWRAPPER_REG===============
#define             DP_HDCP2X_DEBUG_CTRL0_IVCTX    0x18a0
#define             DP_HDCP2X_DEBUG_CTRL1_IVCTX    0x18a1
#define             DP_HDCP2X_DEBUG_CTRL2_IVCTX    0x18a2
#define             DP_HDCP2X_DEBUG_CTRL3_IVCTX    0x18a3
#define             DP_HDCP2X_DEBUG_CTRL4_IVCTX    0x18a4
#define             DP_HDCP2X_DEBUG_STAT0_IVCTX    0x18a5
#define             DP_HDCP2X_DEBUG_STAT1_IVCTX    0x18a6
#define             DP_HDCP2X_DEBUG_STAT2_IVCTX    0x18a7
#define             DP_HDCP2X_DEBUG_STAT3_IVCTX    0x18a8
#define             DP_HDCP2X_DEBUG_STAT4_IVCTX    0x18a9
#define             DP_HDCP2X_DEBUG_STAT5_IVCTX    0x18aa
#define             DP_HDCP2X_DEBUG_STAT6_IVCTX    0x18ab
#define             DP_HDCP2X_DEBUG_STAT7_IVCTX    0x18ac
#define             DP_HDCP2X_DEBUG_STAT8_IVCTX    0x18ad
#define             DP_HDCP2X_DEBUG_STAT9_IVCTX    0x18ae
#define            DP_HDCP2X_DEBUG_STAT10_IVCTX    0x18af
#define            DP_HDCP2X_DEBUG_STAT11_IVCTX    0x18b0
#define            DP_HDCP2X_DEBUG_STAT12_IVCTX    0x18b1
#define            DP_HDCP2X_DEBUG_STAT13_IVCTX    0x18b2
#define            DP_HDCP2X_DEBUG_STAT14_IVCTX    0x18b3
#define            DP_HDCP2X_DEBUG_STAT15_IVCTX    0x18b4
#define                 DP_HDCP2X_TX_SRST_IVCTX    0x18b5
#define                 DP_HDCP2X_POLL_CS_IVCTX    0x18b6
#define      DP_HDCP2X_CUPD_START_ADDR_LO_IVCTX    0x18b7
#define      DP_HDCP2X_CUPD_START_ADDR_HI_IVCTX    0x18b8
#define DP_HDCP2X_CUPD_SIGN_START_ADDR_LO_IVCTX    0x18b9
#define DP_HDCP2X_CUPD_SIGN_START_ADDR_HI_IVCTX    0x18ba
#define   DP_HDCP2X_PRAM_SIGN_END_ADDR_LO_IVCTX    0x18bb
#define   DP_HDCP2X_CUPD_SIGN_END_ADDR_HI_IVCTX    0x18bc
#define                   DP_HDCP2X_CTL_0_IVCTX    0x18bd
#define                   DP_HDCP2X_CTL_1_IVCTX    0x18be
#define                   DP_HDCP2X_CTL_2_IVCTX    0x18bf
#define            DP_HDCP2X_CUPD_SIZE_LO_IVCTX    0x18c0
#define            DP_HDCP2X_CUPD_SIZE_HI_IVCTX    0x18c1
#define                 DP_HDCP2X_GEN_STA_IVCTX    0x18c2
#define               DP_HDCP2X_POLL_VAL0_IVCTX    0x18c3
#define               DP_HDCP2X_POLL_VAL1_IVCTX    0x18c4
#define                DP_HDCP2X_DDCM_STS_IVCTX    0x18c5
#define               DP_HDCP2X_ROSC_BIST_IVCTX    0x18c6
#define                DP_HDCP2_PRAM_DATA_IVCTX    0x18cf
#define            DP_HDCP2X_DEBUG_STAT16_IVCTX    0x18d0

enum hdcp_msgid {
	NULL_MSG = 1,
	AKE_INIT = 2,
	AKE_SEND_CERT = 3,
	AKE_NO_STORED_EKM = 4,
	AKE_STORED_EKM = 5,
	AKE_SEND_RRX = 6,
	AKE_SEND_H_PRIME = 7,
	AKE_SEND_PAIRING_INFO = 8,
	LC_INIT = 9,
	LC_SEND_L_PRIME = 10,
	SKE_SEND_EKS = 11,
	REP_AUTH_SEND_RXID_LIST = 12,
	REP_AUTH_SEND_ACK = 15,
	REP_AUTH_STREAM_MANAGE = 16,
	REP_AUTH_STREAM_READY = 17,
	REP_AUTH_WRITE_TYPE = 18,
	UNKNOWN_MSG_ID,
};

void dptx_hdcp22_proc(struct dptx_hw_common *tx_comm);
void dptx_hdcp2_sndfifo_intr_handler(struct dptx_hw_common *tx_comm);

#endif
