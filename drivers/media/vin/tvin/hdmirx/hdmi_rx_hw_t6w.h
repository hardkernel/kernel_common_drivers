/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _HDMI_RX_T6W_H
#define _HDMI_RX_T6W_H

/* T6W PHY register */
#define T6W_HDMIRX20PLL_CTRL0			(0x000 << 2)
#define T6W_HDMIRX20PLL_CTRL1			(0x001 << 2)
#define T6W_HDMIRX20PHY_DCHA_AFE		(0x002 << 2)
	#define T6W_LEQ_HYPER_GAIN_CH0		_BIT(3)
	#define T6W_LEQ_HYPER_GAIN_CH1		_BIT(7)
	#define T6W_LEQ_HYPER_GAIN_CH2		_BIT(11)
	#define T6W_BUF_BST					MSK(4, 24)
	#define T6W_LEQ_BUF_GAIN            MSK(3, 16)
	#define T6W_LEQ_POLE                MSK(3, 12)
	#define T6W_VGA_GAIN				MSK(12, 0)
#define T6W_HDMIRX20PHY_DCHA_DFE		(0x003 << 2)
#define T6W_PI_BAND_SEL	MSK(2, 26)
#define T6W_PI_DIV_SEL	MSK(2, 24)
#define T6W_TMDS_DIV_SEL	MSK(3, 20)
	#define T6W_SLICER_OFSTCAL_START	_BIT(13)
	#define T6W_DCHA_DFE				MSK(5, 12)
	#define T6W_PI_CFG					MSK(8, 20)
	#define T6W_DFE_OFST_CAL_START		_BIT(9)
#define T6W_HDMIRX20PHY_DCHD_CDR		(0x004 << 2)
#define T6W_EHM_DBG_SEL			_BIT(31)
#define T6W_DFE_OFST_DBG_SEL		MSK(3, 28)
	#define T6W_OFSET_CAL_START		_BIT(27)
	#define T6W_CDR_LKDET_EN		_BIT(14)
	#define T6W_CDR_RSTB			_BIT(13)
	#define T6W_CDR_EN              _BIT(12)
	#define T6W_CDR_FR_EN				_BIT(6)
	#define T6W_MUX_CDR_DBG_SEL		_BIT(19)
	#define T6W_CDR_PI_DIV			MSK(2, 10)
	#define T6W_CDR_OS_RATE			MSK(2, 8)
#define T6W_CDR_PH_DIV	MSK(3, 0)
	#define T6W_DFE_OFST_DBG_SEL		MSK(3, 28)
	#define T6W_ERROR_CNT			0X0
	#define T6W_SCAN_STATE			0X1
	#define T6W_POSITIVE_EYE_HEIGHT		0x2
	#define T6W_NEGATIVE_EYE_HEIGHT		0x3
	#define T6W_LEFT_EYE_WIDTH		0x4
	#define T6W_RIGHT_EYE_WIDTH		0x5
#define T6W_HDMIRX20PHY_DCHD_EQ			(0x005 << 2)
	#define T6W_BW_MAX_EN			_BIT(31)
	#define T6W_BYP_TAP0_EN			_BIT(30)
	#define T6W_BYP_TAP_EN			_BIT(19)
	#define T6W_DFE_HOLD_EN			_BIT(18)
	#define T6W_DFE_RSTB			_BIT(17)
	#define T6W_DFE_EN			_BIT(16)
	#define T6W_EHM_SW_SCAN_EN		_BIT(15)
	#define T6W_EHM_HW_SCAN_EN		_BIT(14)
	#define T6W_EQ_RSTB			_BIT(13)
	#define T6W_EQ_EN			_BIT(12)
	#define T6W_EN_BYP_EQ			_BIT(5)
	#define T6W_BYP_EQ			MSK(5, 0)
	#define T6W_BYP_EN			_BIT(5)
	#define T6W_BYP_EQ_VAL			MSK(4, 0)
	#define T6W_EQ_MODE			MSK(2, 8)
	#define T6W_EQ_ERR_MODE			MSK(2, 10)
	#define T6W_STATUS_MUX_SEL		MSK(2, 22)
#define T6W_HDMIRX20PHY_DCHA_MISC1		(0x006 << 2)
	#define T6W_SQ_RSTN			_BIT(26)
	#define T6W_VCO_TMDS_EN			_BIT(20)
	#define T6W_RTERM_CNTL			MSK(4, 12)
	#define T6W_DCH_RSTN			MSK(3, 4)
	#define RTERM_FLAG_EFUSE_T6W	_BIT(2)
#define T6W_HDMIRX20PHY_DCHA_MISC2		(0x007 << 2)
	#define RTERM_EN_SEL	MSK(4, 28)
	#define T6W_TMDS_VALID_SEL		_BIT(10)
	#define T6W_PLL_CLK_SEL			_BIT(9)
#define T6W_CABLE_CLK_SEL			_BIT(5)
#define T6W_HDMIRX20PHY_DCHD_STAT       (0x009 << 2)
#define T6W_HDMIRX_EARCTX_CNTL0         (0x040 << 2)
#define T6W_HDMIRX_EARCTX_CNTL1         (0x041 << 2)
#define T6W_HDMIRX_ARC_CNTL             (0x042 << 2)
#define T6W_HDMIRX_PHY_PROD_TEST0       (0x080 << 2)
#define T6W_HDMIRX_PHY_PROD_TEST1       (0x081 << 2)

#define T6W_RG_RX20PLL_0		0x000
#define T6W_RG_RX20PLL_1		0x004

/* i2c monitor reg */
#define T6W_I2C_MONITOR_BASE		0x2c000

/* audio pll reg */
#define ANACTRL_AUD_PLL_CNTL_T6W	(0x0080  << 2)
#define ANACTRL_AUD_PLL_CNTL2_T6W	(0x0081  << 2)
#define ANACTRL_AUD_PLL_CNTL3_T6W	(0x0082  << 2)
#define ANACTRL_AUD_PLL_STS_T6W	(0x0083  << 2)
#define ANACTRL_AUD_PLL4X_SCAN_CNTL_T6W	(0x0084  << 2)
#define ANACTRL_VDAC_CTRL0_T6W	(0x01b0  << 2)
#define ANACTRL_VDAC_CTRL1_T6W	(0x01b1  << 2)

extern int tapx_value;
extern int agc_enable;
extern u32 afe_value;
extern u32 dfe_value;
extern u32 cdr_value;
extern u32 eq_value;
extern u32 misc2_value;
extern u32 misc1_value;
/*--------------------------function declare------------------*/
/* T6W */
void aml_phy_init_t6w(void);
void aml_eq_eye_monitor_t6w(void);
void dump_reg_phy_t6w(void);
void dump_aml_phy_sts_t6w(void);
void aml_phy_short_bist_t6w(void);
bool aml_get_tmds_valid_t6w(void);
void aml_phy_power_off_t6w(void);
void aml_phy_switch_port_t6w(void);
void aml_phy_iq_skew_monitor_t6w(void);
void dump_vsi_reg_t6w(u8 port);
unsigned int rx_sec_hdcp_cfg_t6w(void);
void rx_set_aud_output_t6w(u32 param);
void rx_sw_reset_t6w(int level);
void hdcp_init_t6w(void);
void aml_phy_get_trim_val_t6w(void);
void comb_val_t6w(void (*p)(char *, unsigned int, int),
	     char *type, unsigned int val_0, unsigned int val_1,
		 unsigned int val_2, int len);
void get_flag_val_t6w(char *temp, unsigned int val, int len);
void get_val_t6w(char *temp, unsigned int val, int len);
void get_eq_val_t6w(void);
void bubble_sort(u32 *sort_array);
void quick_sort2_t6w(int arr[], int l, int r);
void clk_init_cor_t6w(void);
void rx_dig_clk_en_t6w(bool en);
void dump_aml_phy_sts_t6w(void);
void aml_phy_offset_cal_t6w(void);
void rx_aud_pll_ctl_t6w(bool en, u8 port);
void aml_phy_exbist_t6w(u8 port, u8 ch);
void rx_i2c_mux_cfg_t6w(u8 port);
bool rx_is_power_off_t6w(void);
void rx_dump_pll_param_t6w(void);
/*function declare end*/

#endif /*_HDMI_RX_T6W_H*/
