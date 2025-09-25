// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <dt-bindings/gpio/meson-t6x-gpio.h>
#include "pinctrl-meson.h"
#include "pinctrl-meson-axg-pmx.h"

static const struct pinctrl_pin_desc meson_t6x_analog_pins[] = {
	MESON_PIN(CDAC_IOUT),
	MESON_PIN(CVBS0),
};

static struct meson_pmx_group meson_t6x_analog_groups[] __initdata = {
	GPIO_GROUP(CDAC_IOUT),
	GPIO_GROUP(CVBS0),
};

static const char * const gpio_analog_groups[] = {
	"CDAC_IOUT", "CVBS0",
};

static struct meson_pmx_func meson_t6x_analog_functions[] __initdata = {
	FUNCTION(gpio_analog),
};

static struct meson_bank meson_t6x_analog_banks[] = {
	BANK("ANALOG", CDAC_IOUT, CVBS0, 152, 153, 0, 30, 0, 28, 1, 0, 0, 26, 0, 0),
};

static struct meson_pmx_bank meson_t6x_analog_pmx_banks[] = {
	BANK_PMX("ANALOG", CDAC_IOUT, CVBS0, 0x000, 24),
};

static struct meson_axg_pmx_data meson_t6x_analog_pmx_banks_data = {
	.pmx_banks	= meson_t6x_analog_pmx_banks,
	.num_pmx_banks	= ARRAY_SIZE(meson_t6x_analog_pmx_banks),
};

static struct meson_pinctrl_data meson_t6x_analog_pinctrl_data __refdata = {
	.name		= "analog-banks",
	.pins		= meson_t6x_analog_pins,
	.groups		= meson_t6x_analog_groups,
	.funcs		= meson_t6x_analog_functions,
	.banks		= meson_t6x_analog_banks,
	.num_pins	= ARRAY_SIZE(meson_t6x_analog_pins),
	.num_groups	= ARRAY_SIZE(meson_t6x_analog_groups),
	.num_funcs	= ARRAY_SIZE(meson_t6x_analog_functions),
	.num_banks	= ARRAY_SIZE(meson_t6x_analog_banks),
	.pmx_ops	= &meson_axg_pmx_ops,
	.pmx_data	= &meson_t6x_analog_pmx_banks_data,
	.parse_dt	= meson_g12a_aobus_parse_dt_extra,
};

static const struct pinctrl_pin_desc meson_t6x_periphs_pins[] = {
	/* GPIOW */
	MESON_PIN(GPIOW_0),
	MESON_PIN(GPIOW_1),
	MESON_PIN(GPIOW_2),
	MESON_PIN(GPIOW_3),
	MESON_PIN(GPIOW_4),
	MESON_PIN(GPIOW_5),
	MESON_PIN(GPIOW_6),
	MESON_PIN(GPIOW_7),
	MESON_PIN(GPIOW_8),
	MESON_PIN(GPIOW_9),
	MESON_PIN(GPIOW_10),
	MESON_PIN(GPIOW_11),
	MESON_PIN(GPIOW_12),
	MESON_PIN(GPIOW_13),
	MESON_PIN(GPIOW_14),
	MESON_PIN(GPIOW_15),
	MESON_PIN(GPIOW_16),
	/* GPIOD(GPIOAO) */
	MESON_PIN(GPIOD_0),
	MESON_PIN(GPIOD_1),
	MESON_PIN(GPIOD_2),
	MESON_PIN(GPIOD_3),
	MESON_PIN(GPIOD_4),
	MESON_PIN(GPIOD_5),
	MESON_PIN(GPIOD_6),
	MESON_PIN(GPIOD_7),
	MESON_PIN(GPIOD_8),
	MESON_PIN(GPIOD_9),
	MESON_PIN(GPIOD_10),
	MESON_PIN(GPIOD_11),
	MESON_PIN(GPIOD_12),
	MESON_PIN(GPIOD_13),
	MESON_PIN(GPIOD_14),
	/* GPIOE */
	MESON_PIN(GPIOE_0),
	MESON_PIN(GPIOE_1),
	/* GPIOB */
	MESON_PIN(GPIOB_0),
	MESON_PIN(GPIOB_1),
	MESON_PIN(GPIOB_2),
	MESON_PIN(GPIOB_3),
	MESON_PIN(GPIOB_4),
	MESON_PIN(GPIOB_5),
	MESON_PIN(GPIOB_6),
	MESON_PIN(GPIOB_7),
	MESON_PIN(GPIOB_8),
	MESON_PIN(GPIOB_9),
	MESON_PIN(GPIOB_10),
	MESON_PIN(GPIOB_11),
	MESON_PIN(GPIOB_12),
	MESON_PIN(GPIOB_13),
	/* GPIOC */
	MESON_PIN(GPIOC_0),
	MESON_PIN(GPIOC_1),
	MESON_PIN(GPIOC_2),
	MESON_PIN(GPIOC_3),
	MESON_PIN(GPIOC_4),
	MESON_PIN(GPIOC_5),
	MESON_PIN(GPIOC_6),
	MESON_PIN(GPIOC_7),
	MESON_PIN(GPIOC_8),
	MESON_PIN(GPIOC_9),
	MESON_PIN(GPIOC_10),
	/* GPIOZ */
	MESON_PIN(GPIOZ_0),
	MESON_PIN(GPIOZ_1),
	MESON_PIN(GPIOZ_2),
	MESON_PIN(GPIOZ_3),
	MESON_PIN(GPIOZ_4),
	MESON_PIN(GPIOZ_5),
	MESON_PIN(GPIOZ_6),
	MESON_PIN(GPIOZ_7),
	MESON_PIN(GPIOZ_8),
	MESON_PIN(GPIOZ_9),
	MESON_PIN(GPIOZ_10),
	MESON_PIN(GPIOZ_11),
	MESON_PIN(GPIOZ_12),
	MESON_PIN(GPIOZ_13),
	MESON_PIN(GPIOZ_14),
	MESON_PIN(GPIOZ_15),
	MESON_PIN(GPIOZ_16),
	MESON_PIN(GPIOZ_17),
	MESON_PIN(GPIOZ_18),
	MESON_PIN(GPIOZ_19),
	/* GPIOH */
	MESON_PIN(GPIOH_0),
	MESON_PIN(GPIOH_1),
	MESON_PIN(GPIOH_2),
	MESON_PIN(GPIOH_3),
	MESON_PIN(GPIOH_4),
	MESON_PIN(GPIOH_5),
	MESON_PIN(GPIOH_6),
	MESON_PIN(GPIOH_7),
	MESON_PIN(GPIOH_8),
	MESON_PIN(GPIOH_9),
	MESON_PIN(GPIOH_10),
	MESON_PIN(GPIOH_11),
	MESON_PIN(GPIOH_12),
	MESON_PIN(GPIOH_13),
	MESON_PIN(GPIOH_14),
	MESON_PIN(GPIOH_15),
	MESON_PIN(GPIOH_16),
	MESON_PIN(GPIOH_17),
	MESON_PIN(GPIOH_18),
	MESON_PIN(GPIOH_19),
	MESON_PIN(GPIOH_20),
	MESON_PIN(GPIOH_21),
	MESON_PIN(GPIOH_22),
	MESON_PIN(GPIOH_23),
	MESON_PIN(GPIOH_24),
	MESON_PIN(GPIOH_25),
	MESON_PIN(GPIOH_26),
	MESON_PIN(GPIOH_27),
	MESON_PIN(GPIOH_28),
	MESON_PIN(GPIOH_29),
	/* GPIOP */
	MESON_PIN(GPIOP_0),
	MESON_PIN(GPIOP_1),
	MESON_PIN(GPIOP_2),
	MESON_PIN(GPIOP_3),
	MESON_PIN(GPIOP_4),
	MESON_PIN(GPIOP_5),
	MESON_PIN(GPIOP_6),
	MESON_PIN(GPIOP_7),
	MESON_PIN(GPIOP_8),
	MESON_PIN(GPIOP_9),
	MESON_PIN(GPIOP_10),
	MESON_PIN(GPIOP_11),
	/* GPIOM */
	MESON_PIN(GPIOM_0),
	MESON_PIN(GPIOM_1),
	MESON_PIN(GPIOM_2),
	MESON_PIN(GPIOM_3),
	MESON_PIN(GPIOM_4),
	MESON_PIN(GPIOM_5),
	MESON_PIN(GPIOM_6),
	MESON_PIN(GPIOM_7),
	MESON_PIN(GPIOM_8),
	MESON_PIN(GPIOM_9),
	MESON_PIN(GPIOM_10),
	MESON_PIN(GPIOM_11),
	MESON_PIN(GPIOM_12),
	MESON_PIN(GPIOM_13),
	MESON_PIN(GPIOM_14),
	MESON_PIN(GPIOM_15),
	MESON_PIN(GPIOM_16),
	MESON_PIN(GPIOM_17),
	MESON_PIN(GPIOM_18),
	MESON_PIN(GPIOM_19),
	MESON_PIN(GPIOM_20),
	MESON_PIN(GPIOM_21),
	MESON_PIN(GPIOM_22),
	MESON_PIN(GPIOM_23),
	MESON_PIN(GPIOM_24),
	MESON_PIN(GPIOM_25),
	MESON_PIN(GPIOM_26),
	MESON_PIN(GPIOM_27),
	MESON_PIN(GPIOM_28),
	MESON_PIN(GPIOM_29),
	/* GPIO_TEST_N */
	MESON_PIN(GPIO_TEST_N)
};

/* Bank[W]:Function[1] */
static const unsigned int hdmirx_hpd_a_pins[]			= { GPIOW_0 };
static const unsigned int hdmirx_5vdet_a_pins[]			= { GPIOW_1 };
static const unsigned int hdmirx_sda_a_pins[]			= { GPIOW_2 };
static const unsigned int hdmirx_scl_a_pins[]			= { GPIOW_3 };
static const unsigned int hdmirx_hpd_b_pins[]			= { GPIOW_4 };
static const unsigned int hdmirx_5vdet_b_pins[]			= { GPIOW_5 };
static const unsigned int hdmirx_sda_b_pins[]			= { GPIOW_6 };
static const unsigned int hdmirx_scl_b_pins[]			= { GPIOW_7 };
static const unsigned int hdmirx_hpd_c_pins[]			= { GPIOW_8 };
static const unsigned int hdmirx_5vdet_c_pins[]			= { GPIOW_9 };
static const unsigned int hdmirx_sda_c_pins[]			= { GPIOW_10 };
static const unsigned int hdmirx_scl_c_pins[]			= { GPIOW_11 };
static const unsigned int hdmirx_hpd_d_pins[]			= { GPIOW_12 };
static const unsigned int hdmirx_5vdet_d_pins[]			= { GPIOW_13 };
static const unsigned int hdmirx_sda_d_pins[]			= { GPIOW_14 };
static const unsigned int hdmirx_scl_d_pins[]			= { GPIOW_15 };
static const unsigned int cec_pins[]				= { GPIOW_16 };

/* Bank[W]:Function[2] */
static const unsigned int uart_b_tx_pins[]			= { GPIOW_2 };
static const unsigned int uart_b_rx_pins[]			= { GPIOW_3 };
static const unsigned int uart_b_tx_w_pins[]			= { GPIOW_6 };
static const unsigned int uart_b_rx_w_pins[]			= { GPIOW_7 };
static const unsigned int uart_b_tx_w10_pins[]			= { GPIOW_10 };
static const unsigned int uart_b_rx_w11_pins[]			= { GPIOW_11 };
static const unsigned int uart_b_tx_w14_pins[]			= { GPIOW_14 };
static const unsigned int uart_b_rx_w15_pins[]			= { GPIOW_15 };

/* Bank[W]:Function[3] */
static const unsigned int jtag_a_clk_pins[]			= { GPIOW_2 };
static const unsigned int jtag_a_tms_pins[]			= { GPIOW_3 };
static const unsigned int tst_clk_out22_pins[]			= { GPIOW_4 };
static const unsigned int jtag_a_tdi_pins[]			= { GPIOW_6 };
static const unsigned int jtag_a_tdo_pins[]			= { GPIOW_7 };
static const unsigned int jtag_a_clk_w_pins[]			= { GPIOW_10 };
static const unsigned int jtag_a_tms_w_pins[]			= { GPIOW_11 };
static const unsigned int jtag_a_tdi_w_pins[]			= { GPIOW_14 };
static const unsigned int jtag_a_tdo_w_pins[]			= { GPIOW_15 };

/* Bank[W]:Function[4] */
static const unsigned int gpiow3_loop_pins[]			= { GPIOW_2 };
static const unsigned int gpiow2_loop_pins[]			= { GPIOW_3 };
static const unsigned int jtag_a_clk_w6_pins[]			= { GPIOW_6 };
static const unsigned int jtag_a_tms_w7_pins[]			= { GPIOW_7 };
static const unsigned int jtag_a_tdi_w10_pins[]			= { GPIOW_10 };
static const unsigned int jtag_a_tdo_w11_pins[]			= { GPIOW_11 };

/* Bank[W]:Function[5] */
static const unsigned int eth_mdio_pins[]			= { GPIOW_6 };
static const unsigned int eth_mdc_pins[]			= { GPIOW_7 };
static const unsigned int eth_mdio_w_pins[]			= { GPIOW_14 };
static const unsigned int eth_mdc_w_pins[]			= { GPIOW_15 };

/* Bank[W]:Function[7] */
static const unsigned int tst_clk_out22_w_pins[]		= { GPIOW_7 };
static const unsigned int tst_clk_out22_w16_pins[]		= { GPIOW_16 };

/* Bank[D]:Function[1] */
static const unsigned int uart_b_tx_d_pins[]			= { GPIOD_0 };
static const unsigned int uart_b_rx_d_pins[]			= { GPIOD_1 };
static const unsigned int i2c_b_scl_pins[]			= { GPIOD_2 };
static const unsigned int i2c_b_sda_pins[]			= { GPIOD_3 };
static const unsigned int clk_32k_in_pins[]			= { GPIOD_4 };
static const unsigned int ir_in_a_pins[]			= { GPIOD_5 };
static const unsigned int jtag_a_clk_d_pins[]			= { GPIOD_6 };
static const unsigned int jtag_a_tms_d_pins[]			= { GPIOD_7 };
static const unsigned int jtag_a_tdi_d_pins[]			= { GPIOD_8 };
static const unsigned int jtag_a_tdo_d_pins[]			= { GPIOD_9 };
static const unsigned int clk12_24m_pins[]			= { GPIOD_10 };
static const unsigned int pwm_a_pins[]				= { GPIOD_11 };
static const unsigned int pwm_f_pins[]				= { GPIOD_12 };
static const unsigned int i2c_b_sda_d_pins[]			= { GPIOD_13 };
static const unsigned int i2c_b_scl_d_pins[]			= { GPIOD_14 };

/* Bank[D]:Function[2] */
static const unsigned int ir_out_pins[]				= { GPIOD_0 };
static const unsigned int atv_if_agc_pins[]			= { GPIOD_4 };
static const unsigned int pwm_c_pins[]				= { GPIOD_5 };
static const unsigned int spdif_out_pins[]			= { GPIOD_6 };
static const unsigned int pwm_c_d_pins[]			= { GPIOD_7 };
static const unsigned int spdif_out_d_pins[]			= { GPIOD_8 };
static const unsigned int pwm_d_pins[]				= { GPIOD_9 };
static const unsigned int pwm_e_pins[]				= { GPIOD_10 };
static const unsigned int pwm_c_d11_pins[]			= { GPIOD_11 };
static const unsigned int uart_d_tx_pins[]			= { GPIOD_13 };
static const unsigned int uart_d_rx_pins[]			= { GPIOD_14 };

/* Bank[D]:Function[3] */
static const unsigned int cicam_waitn_pins[]			= { GPIOD_0 };
static const unsigned int demod_uart_tx_pins[]			= { GPIOD_2 };
static const unsigned int demod_uart_rx_pins[]			= { GPIOD_3 };
static const unsigned int dtv_if_agc_pins[]			= { GPIOD_4 };
static const unsigned int clk12_24m_d_pins[]			= { GPIOD_5 };
static const unsigned int pwm_d_hiz_pins[]			= { GPIOD_6 };
static const unsigned int pwm_c_hiz_pins[]			= { GPIOD_7 };
static const unsigned int ir_out_d_pins[]			= { GPIOD_9 };
static const unsigned int gen_clk_out_pins[]			= { GPIOD_10 };
static const unsigned int pwm_i_pins[]				= { GPIOD_11 };
static const unsigned int pwm_j_pins[]				= { GPIOD_12 };
static const unsigned int ir_in_b_pins[]			= { GPIOD_14 };

/* Bank[D]:Function[4] */
static const unsigned int gen_clk_out_d_pins[]			= { GPIOD_5 };
static const unsigned int uart_c_tx_pins[]			= { GPIOD_6 };
static const unsigned int uart_c_rx_pins[]			= { GPIOD_7 };
static const unsigned int tdm_d2_pins[]				= { GPIOD_8 };
static const unsigned int tdm_d3_pins[]				= { GPIOD_9 };

/* Bank[D]:Function[5] */
static const unsigned int ir_in_b_d_pins[]			= { GPIOD_6 };
static const unsigned int dcon_led_pins[]			= { GPIOD_7 };
static const unsigned int dcon_led_d_pins[]			= { GPIOD_8 };
static const unsigned int dcon_led_d9_pins[]			= { GPIOD_9 };

/* Bank[D]:Function[6] */
static const unsigned int pwm_h_pins[]				= { GPIOD_6 };
static const unsigned int mclk_1_pins[]				= { GPIOD_7 };
static const unsigned int pwm_vs_pins[]				= { GPIOD_9 };

/* Bank[D]:Function[7] */
static const unsigned int i2c_c_scl_pins[]			= { GPIOD_6 };
static const unsigned int i2c_c_sda_pins[]			= { GPIOD_7 };
static const unsigned int i2c_d_scl_pins[]			= { GPIOD_8 };
static const unsigned int i2c_d_sda_pins[]			= { GPIOD_9 };

/* Bank[E]:Function[1] */
static const unsigned int pwm_a_e_pins[]			= { GPIOE_0 };
static const unsigned int pwm_b_pins[]				= { GPIOE_1 };

/* Bank[E]:Function[2] */
static const unsigned int i2c_c_scl_e_pins[]			= { GPIOE_0 };
static const unsigned int i2c_c_sda_e_pins[]			= { GPIOE_1 };

/* Bank[E]:Function[3] */
static const unsigned int tst_clk_out4_pins[]			= { GPIOE_0 };

/* Bank[B]:Function[1] */
static const unsigned int emmc_d0_pins[]			= { GPIOB_0 };
static const unsigned int emmc_d1_pins[]			= { GPIOB_1 };
static const unsigned int emmc_d2_pins[]			= { GPIOB_2 };
static const unsigned int emmc_d3_pins[]			= { GPIOB_3 };
static const unsigned int emmc_d4_pins[]			= { GPIOB_4 };
static const unsigned int emmc_d5_pins[]			= { GPIOB_5 };
static const unsigned int emmc_d6_pins[]			= { GPIOB_6 };
static const unsigned int emmc_d7_pins[]			= { GPIOB_7 };
static const unsigned int emmc_clk_pins[]			= { GPIOB_8 };
static const unsigned int emmc_cmd_pins[]			= { GPIOB_10 };
static const unsigned int emmc_ds_pins[]			= { GPIOB_11 };
static const unsigned int gpiob11_loop_pins[]			= { GPIOB_12 };
static const unsigned int gpiob12_loop_pins[]			= { GPIOB_13 };

/* Bank[B]:Function[2] */
static const unsigned int nand_wen_clk_pins[]			= { GPIOB_8 };
static const unsigned int nand_ale_pins[]			= { GPIOB_9 };
static const unsigned int nand_ren_wr_pins[]			= { GPIOB_10 };
static const unsigned int nand_cle_pins[]			= { GPIOB_11 };
static const unsigned int nand_ce0_pins[]			= { GPIOB_12 };
static const unsigned int gpioh21_loop_pins[]			= { GPIOB_13 };

/* Bank[B]:Function[3] */
static const unsigned int spinf_mo_d0_pins[]			= { GPIOB_0 };
static const unsigned int spinf_mi_d1_pins[]			= { GPIOB_1 };
static const unsigned int spinf_wp_d2_pins[]			= { GPIOB_2 };
static const unsigned int spinf_hold_d3_pins[]			= { GPIOB_3 };
static const unsigned int spinf_d4_pins[]			= { GPIOB_4 };
static const unsigned int spinf_d5_pins[]			= { GPIOB_5 };
static const unsigned int spinf_d6_pins[]			= { GPIOB_6 };
static const unsigned int spinf_d7_pins[]			= { GPIOB_7 };
static const unsigned int spinf_clk_pins[]			= { GPIOB_10 };
static const unsigned int spinf_cs0_pins[]			= { GPIOB_12 };
static const unsigned int spinf_cs1_pins[]			= { GPIOB_13 };

/* Bank[B]:Function[4] */
static const unsigned int ir_out_b_pins[]			= { GPIOB_12 };

/* Bank[B]:Function[5] */
static const unsigned int gpioh21_loop_b_pins[]			= { GPIOB_12 };

/* Bank[B]:Function[7] */
static const unsigned int gen_clk_out_b_pins[]			= { GPIOB_6 };

/* Bank[C]:Function[1] */
static const unsigned int uart_a_tx_pins[]			= { GPIOC_6 };
static const unsigned int uart_a_rx_pins[]			= { GPIOC_7 };
static const unsigned int uart_a_cts_pins[]			= { GPIOC_8 };
static const unsigned int uart_a_rts_pins[]			= { GPIOC_9 };
static const unsigned int pwm_f_c_pins[]			= { GPIOC_10 };

/* Bank[C]:Function[2] */
static const unsigned int spi_miso_c_pins[]			= { GPIOC_0 };
static const unsigned int spi_mosi_c_pins[]			= { GPIOC_1 };
static const unsigned int spi_clk_c_pins[]			= { GPIOC_2 };
static const unsigned int spi_ss0_c_pins[]			= { GPIOC_3 };
static const unsigned int spi_ss1_c_pins[]			= { GPIOC_4 };
static const unsigned int spi_ss2_c_pins[]			= { GPIOC_5 };
static const unsigned int mclk_1_c_pins[]			= { GPIOC_8 };
static const unsigned int mclk_2_pins[]				= { GPIOC_9 };
static const unsigned int pwm_j_c_pins[]			= { GPIOC_10 };

/* Bank[C]:Function[3] */
static const unsigned int cicam_a2_pins[]			= { GPIOC_0 };
static const unsigned int cicam_a3_pins[]			= { GPIOC_1 };
static const unsigned int cicam_a4_pins[]			= { GPIOC_2 };
static const unsigned int cicam_a5_pins[]			= { GPIOC_3 };
static const unsigned int cicam_a6_pins[]			= { GPIOC_4 };
static const unsigned int cicam_a7_pins[]			= { GPIOC_5 };
static const unsigned int cicam_a8_pins[]			= { GPIOC_6 };
static const unsigned int cicam_a9_pins[]			= { GPIOC_7 };
static const unsigned int cicam_a10_pins[]			= { GPIOC_8 };
static const unsigned int cicam_a11_pins[]			= { GPIOC_9 };
static const unsigned int cicam_a12_pins[]			= { GPIOC_10 };

/* Bank[C]:Function[4] */
static const unsigned int tsin_c_clk_pins[]			= { GPIOC_0 };
static const unsigned int tsin_c_sop_pins[]			= { GPIOC_1 };
static const unsigned int tsin_c_valid_pins[]			= { GPIOC_2 };
static const unsigned int tsin_c_d0_pins[]			= { GPIOC_3 };
static const unsigned int spi_dq2_c_pins[]			= { GPIOC_4 };
static const unsigned int spi_dq3_c_pins[]			= { GPIOC_5 };
static const unsigned int demod_uart_tx_c_pins[]		= { GPIOC_6 };
static const unsigned int demod_uart_rx_c_pins[]		= { GPIOC_7 };
static const unsigned int pwm_vs_c_pins[]			= { GPIOC_10 };

/* Bank[C]:Function[5] */
static const unsigned int tdm_sclk2_pins[]			= { GPIOC_2 };
static const unsigned int tdm_fs2_pins[]			= { GPIOC_3 };
static const unsigned int tdm_d6_pins[]				= { GPIOC_4 };
static const unsigned int tdm_d7_pins[]				= { GPIOC_5 };
static const unsigned int tdm_fs1_pins[]			= { GPIOC_6 };
static const unsigned int tdm_sclk1_pins[]			= { GPIOC_7 };
static const unsigned int tdm_d4_pins[]				= { GPIOC_8 };
static const unsigned int tdm_d5_pins[]				= { GPIOC_9 };
static const unsigned int spdif_out_c_pins[]			= { GPIOC_10 };

/* Bank[C]:Function[6] */
static const unsigned int spi_miso_d_pins[]			= { GPIOC_0 };
static const unsigned int spi_mosi_d_pins[]			= { GPIOC_1 };
static const unsigned int spi_clk_d_pins[]			= { GPIOC_2 };
static const unsigned int spi_ss0_d_pins[]			= { GPIOC_3 };
static const unsigned int spi_mosi_e_pins[]			= { GPIOC_4 };
static const unsigned int spi_clk_e_pins[]			= { GPIOC_5 };
static const unsigned int spi_ss0_e_pins[]			= { GPIOC_6 };
static const unsigned int spi_mosi_f_pins[]			= { GPIOC_7 };
static const unsigned int spi_clk_f_pins[]			= { GPIOC_8 };
static const unsigned int spi_ss0_f_pins[]			= { GPIOC_9 };
static const unsigned int spi_miso_f_pins[]			= { GPIOC_10 };

/* Bank[C]:Function[7] */
static const unsigned int gen_clk_out_c_pins[]			= { GPIOC_2 };
static const unsigned int dcon_led_c_pins[]			= { GPIOC_10 };

/* Bank[Z]:Function[1] */
static const unsigned int tdm_fs2_z_pins[]			= { GPIOZ_0 };
static const unsigned int tdm_sclk2_z_pins[]			= { GPIOZ_1 };
static const unsigned int tdm_d4_z_pins[]			= { GPIOZ_2 };
static const unsigned int tdm_d5_z_pins[]			= { GPIOZ_3 };
static const unsigned int tdm_d6_z_pins[]			= { GPIOZ_4 };
static const unsigned int tdm_d7_z_pins[]			= { GPIOZ_5 };
static const unsigned int mclk_2_z_pins[]			= { GPIOZ_6 };
static const unsigned int tsout_clk_pins[]			= { GPIOZ_7 };
static const unsigned int tsout_sop_pins[]			= { GPIOZ_8 };
static const unsigned int tsout_valid_pins[]			= { GPIOZ_9 };
static const unsigned int tsout_d0_pins[]			= { GPIOZ_10 };
static const unsigned int tsout_d1_pins[]			= { GPIOZ_11 };
static const unsigned int tsout_d2_pins[]			= { GPIOZ_12 };
static const unsigned int tsout_d3_pins[]			= { GPIOZ_13 };
static const unsigned int tsout_d4_pins[]			= { GPIOZ_14 };
static const unsigned int tsout_d5_pins[]			= { GPIOZ_15 };
static const unsigned int tsout_d6_pins[]			= { GPIOZ_16 };
static const unsigned int tsout_d7_pins[]			= { GPIOZ_17 };
static const unsigned int dcon_led_z_pins[]			= { GPIOZ_18 };
static const unsigned int spdif_out_z_pins[]			= { GPIOZ_19 };

/* Bank[Z]:Function[2] */
static const unsigned int tsin_a_clk_pins[]			= { GPIOZ_0 };
static const unsigned int tsin_a_sop_pins[]			= { GPIOZ_1 };
static const unsigned int tsin_a_valid_pins[]			= { GPIOZ_2 };
static const unsigned int tsin_a_d0_pins[]			= { GPIOZ_3 };
static const unsigned int dtv_rf_agc_pins[]			= { GPIOZ_6 };
static const unsigned int cicam_a2_z_pins[]			= { GPIOZ_8 };
static const unsigned int cicam_a3_z_pins[]			= { GPIOZ_9 };
static const unsigned int cicam_a4_z_pins[]			= { GPIOZ_10 };
static const unsigned int cicam_a5_z_pins[]			= { GPIOZ_11 };
static const unsigned int cicam_a6_z_pins[]			= { GPIOZ_12 };
static const unsigned int cicam_a7_z_pins[]			= { GPIOZ_13 };
static const unsigned int cicam_a8_z_pins[]			= { GPIOZ_14 };
static const unsigned int cicam_a9_z_pins[]			= { GPIOZ_15 };
static const unsigned int cicam_a10_z_pins[]			= { GPIOZ_16 };
static const unsigned int cicam_a11_z_pins[]			= { GPIOZ_17 };
static const unsigned int dtv_rf_agc_z_pins[]			= { GPIOZ_18 };
static const unsigned int tst_clk_out4_z_pins[]			= { GPIOZ_19 };

/* Bank[Z]:Function[3] */
static const unsigned int iso7816_clk_pins[]			= { GPIOZ_0 };
static const unsigned int iso7816_data_pins[]			= { GPIOZ_1 };
static const unsigned int cicam_a13_pins[]			= { GPIOZ_2 };
static const unsigned int cicam_a14_pins[]			= { GPIOZ_3 };
static const unsigned int i2c_a_scl_pins[]			= { GPIOZ_4 };
static const unsigned int i2c_a_sda_pins[]			= { GPIOZ_5 };
static const unsigned int atv_if_agc_z_pins[]			= { GPIOZ_6 };
static const unsigned int tsin_c_clk_z_pins[]			= { GPIOZ_7 };
static const unsigned int tsin_c_sop_z_pins[]			= { GPIOZ_8 };
static const unsigned int tsin_c_valid_z_pins[]			= { GPIOZ_9 };
static const unsigned int tsin_c_d0_z_pins[]			= { GPIOZ_10 };
static const unsigned int tsin_c_d1_pins[]			= { GPIOZ_11 };
static const unsigned int tsin_c_d2_pins[]			= { GPIOZ_12 };
static const unsigned int tsin_c_d3_pins[]			= { GPIOZ_13 };
static const unsigned int tsin_c_d4_pins[]			= { GPIOZ_14 };
static const unsigned int tsin_c_d5_pins[]			= { GPIOZ_15 };
static const unsigned int tsin_c_d6_pins[]			= { GPIOZ_16 };
static const unsigned int tsin_c_d7_pins[]			= { GPIOZ_17 };
static const unsigned int atv_if_agc_z18_pins[]			= { GPIOZ_18 };

/* Bank[Z]:Function[4] */
static const unsigned int spi_miso_b_pins[]			= { GPIOZ_0 };
static const unsigned int spi_mosi_b_pins[]			= { GPIOZ_1 };
static const unsigned int spi_clk_b_pins[]			= { GPIOZ_2 };
static const unsigned int spi_ss0_b_pins[]			= { GPIOZ_3 };
static const unsigned int spi_dq2_b_pins[]			= { GPIOZ_4 };
static const unsigned int spi_dq3_b_pins[]			= { GPIOZ_5 };
static const unsigned int dtv_if_agc_z_pins[]			= { GPIOZ_6 };
static const unsigned int dtv_if_agc_z18_pins[]			= { GPIOZ_18 };
static const unsigned int pwm_g_pins[]				= { GPIOZ_19 };

/* Bank[Z]:Function[5] */
static const unsigned int uart_a_tx_z_pins[]			= { GPIOZ_0 };
static const unsigned int uart_a_rx_z_pins[]			= { GPIOZ_1 };
static const unsigned int uart_a_cts_z_pins[]			= { GPIOZ_2 };
static const unsigned int uart_a_rts_z_pins[]			= { GPIOZ_3 };
static const unsigned int pwm_b_z_pins[]			= { GPIOZ_4 };
static const unsigned int pwm_d_z_pins[]			= { GPIOZ_5 };
static const unsigned int pwm_e_z_pins[]			= { GPIOZ_6 };
static const unsigned int uart_d_tx_z_pins[]			= { GPIOZ_8 };
static const unsigned int uart_d_rx_z_pins[]			= { GPIOZ_9 };
static const unsigned int pwm_g_z_pins[]			= { GPIOZ_12 };
static const unsigned int pwm_h_z_pins[]			= { GPIOZ_13 };
static const unsigned int pwm_i_z_pins[]			= { GPIOZ_14 };
static const unsigned int pwm_j_z_pins[]			= { GPIOZ_15 };
static const unsigned int ir_in_b_z_pins[]			= { GPIOZ_18 };

/* Bank[Z]:Function[6] */
static const unsigned int cicam_a12_z_pins[]			= { GPIOZ_0 };
static const unsigned int pdm_dclk_pins[]			= { GPIOZ_2 };
static const unsigned int pdm_din0_pins[]			= { GPIOZ_3 };
static const unsigned int pdm_din1_pins[]			= { GPIOZ_6 };
static const unsigned int tdm_d1_pins[]				= { GPIOZ_10 };
static const unsigned int tdm_d0_pins[]				= { GPIOZ_11 };
static const unsigned int tdm_sclk1_z_pins[]			= { GPIOZ_16 };
static const unsigned int tdm_fs1_z_pins[]			= { GPIOZ_17 };
static const unsigned int diseqc_out_pins[]			= { GPIOZ_19 };

/* Bank[Z]:Function[7] */
static const unsigned int diseqc_out_z_pins[]			= { GPIOZ_0 };
static const unsigned int s2_demod_gpio0_pins[]			= { GPIOZ_1 };
static const unsigned int spdif_out_z2_pins[]			= { GPIOZ_2 };
static const unsigned int eth_mdio_z_pins[]			= { GPIOZ_4 };
static const unsigned int eth_mdc_z_pins[]			= { GPIOZ_5 };
static const unsigned int eth_link_led_pins[]			= { GPIOZ_6 };
static const unsigned int eth_rgmii_rx_clk_pins[]		= { GPIOZ_7 };
static const unsigned int eth_rx_dv_pins[]			= { GPIOZ_8 };
static const unsigned int eth_rxd0_pins[]			= { GPIOZ_9 };
static const unsigned int eth_rxd1_pins[]			= { GPIOZ_10 };
static const unsigned int eth_rxd2_rgmii_pins[]			= { GPIOZ_11 };
static const unsigned int eth_rxd3_rgmii_pins[]			= { GPIOZ_12 };
static const unsigned int eth_rgmii_tx_clk_pins[]		= { GPIOZ_13 };
static const unsigned int eth_txen_pins[]			= { GPIOZ_14 };
static const unsigned int eth_txd0_pins[]			= { GPIOZ_15 };
static const unsigned int eth_txd1_pins[]			= { GPIOZ_16 };
static const unsigned int eth_txd2_rgmii_pins[]			= { GPIOZ_17 };
static const unsigned int eth_txd3_rgmii_pins[]			= { GPIOZ_18 };
static const unsigned int eth_act_led_pins[]			= { GPIOZ_19 };

/* Bank[H]:Function[1] */
static const unsigned int tcon_0_pins[]				= { GPIOH_0 };
static const unsigned int tcon_1_pins[]				= { GPIOH_1 };
static const unsigned int tcon_2_pins[]				= { GPIOH_2 };
static const unsigned int tcon_3_pins[]				= { GPIOH_3 };
static const unsigned int tcon_4_pins[]				= { GPIOH_4 };
static const unsigned int tcon_5_pins[]				= { GPIOH_5 };
static const unsigned int tcon_6_pins[]				= { GPIOH_6 };
static const unsigned int tcon_7_pins[]				= { GPIOH_7 };
static const unsigned int tcon_8_pins[]				= { GPIOH_8 };
static const unsigned int tcon_9_pins[]				= { GPIOH_9 };
static const unsigned int tcon_10_pins[]			= { GPIOH_10 };
static const unsigned int tcon_11_pins[]			= { GPIOH_11 };
static const unsigned int tcon_12_pins[]			= { GPIOH_12 };
static const unsigned int tcon_13_pins[]			= { GPIOH_13 };
static const unsigned int tcon_14_pins[]			= { GPIOH_14 };
static const unsigned int tcon_15_pins[]			= { GPIOH_15 };
static const unsigned int hsync_pins[]				= { GPIOH_18 };
static const unsigned int vsync_pins[]				= { GPIOH_19 };
static const unsigned int i2c_tcon_scl_pins[]			= { GPIOH_20 };
static const unsigned int i2c_tcon_sda_pins[]			= { GPIOH_21 };
static const unsigned int i2c_c_scl_h_pins[]			= { GPIOH_22 };
static const unsigned int i2c_c_sda_h_pins[]			= { GPIOH_23 };
static const unsigned int i2c_d_scl_h_pins[]			= { GPIOH_24 };
static const unsigned int i2c_d_sda_h_pins[]			= { GPIOH_25 };
static const unsigned int pwm_g_h_pins[]			= { GPIOH_26 };
static const unsigned int pwm_e_h_pins[]			= { GPIOH_27 };
static const unsigned int i2c_tcon_scl_h_pins[]			= { GPIOH_28 };
static const unsigned int i2c_tcon_sda_h_pins[]			= { GPIOH_29 };

/* Bank[H]:Function[2] */
static const unsigned int tcon_lock_pins[]			= { GPIOH_0 };
static const unsigned int tcon_lock_h_pins[]			= { GPIOH_1 };
static const unsigned int i2c_tcon_scl_h2_pins[]		= { GPIOH_2 };
static const unsigned int i2c_tcon_sda_h3_pins[]		= { GPIOH_3 };
static const unsigned int sync_3d_out_pins[]			= { GPIOH_5 };
static const unsigned int tdm_d3_h_pins[]			= { GPIOH_6 };
static const unsigned int spi_ss1_a_pins[]			= { GPIOH_7 };
static const unsigned int spi_ss0_a_pins[]			= { GPIOH_8 };
static const unsigned int spi_miso_a_pins[]			= { GPIOH_9 };
static const unsigned int spi_mosi_a_pins[]			= { GPIOH_10 };
static const unsigned int spi_clk_a_pins[]			= { GPIOH_11 };
static const unsigned int spi_ss2_a_pins[]			= { GPIOH_12 };
static const unsigned int tdm_d3_h13_pins[]			= { GPIOH_13 };
static const unsigned int mclk_1_h_pins[]			= { GPIOH_14 };
static const unsigned int tdm_fs1_h_pins[]			= { GPIOH_15 };
static const unsigned int tdm_sclk1_h_pins[]			= { GPIOH_16 };
static const unsigned int tdm_d0_h_pins[]			= { GPIOH_17 };
static const unsigned int tdm_d1_h_pins[]			= { GPIOH_18 };
static const unsigned int tdm_d2_h_pins[]			= { GPIOH_19 };
static const unsigned int tdm_d3_h20_pins[]			= { GPIOH_20 };
static const unsigned int tdm_d4_h_pins[]			= { GPIOH_21 };
static const unsigned int tdm_d5_h_pins[]			= { GPIOH_22 };
static const unsigned int tdm_d6_h_pins[]			= { GPIOH_23 };
static const unsigned int spi_ss0_c_h_pins[]			= { GPIOH_26 };
static const unsigned int spi_miso_c_h_pins[]			= { GPIOH_27 };
static const unsigned int spi_mosi_c_h_pins[]			= { GPIOH_28 };
static const unsigned int spi_clk_c_h_pins[]			= { GPIOH_29 };

/* Bank[H]:Function[3] */
static const unsigned int tcon_sfc_pins[]			= { GPIOH_0 };
static const unsigned int tcon_sfc_h_pins[]			= { GPIOH_1 };
static const unsigned int uart_c_tx_h_pins[]			= { GPIOH_2 };
static const unsigned int uart_c_rx_h_pins[]			= { GPIOH_3 };
static const unsigned int uart_c_cts_pins[]			= { GPIOH_4 };
static const unsigned int uart_c_rts_pins[]			= { GPIOH_5 };
static const unsigned int i2c_tcon_scl_h10_pins[]		= { GPIOH_10 };
static const unsigned int i2c_tcon_sda_h11_pins[]		= { GPIOH_11 };
static const unsigned int pwm_vs_h_pins[]			= { GPIOH_12 };
static const unsigned int pwm_vs_h13_pins[]			= { GPIOH_13 };
static const unsigned int pdm_din1_h_pins[]			= { GPIOH_14 };
static const unsigned int pdm_din0_h_pins[]			= { GPIOH_18 };
static const unsigned int pdm_dclk_h_pins[]			= { GPIOH_19 };
static const unsigned int gpiob13_loop_pins[]			= { GPIOH_21 };
static const unsigned int pdm_dclk_h22_pins[]			= { GPIOH_22 };
static const unsigned int pdm_din1_h23_pins[]			= { GPIOH_23 };
static const unsigned int i2c_b_scl_h_pins[]			= { GPIOH_24 };
static const unsigned int i2c_b_sda_h_pins[]			= { GPIOH_25 };
static const unsigned int pdm_din0_h26_pins[]			= { GPIOH_26 };
static const unsigned int pdm_din1_h27_pins[]			= { GPIOH_27 };
static const unsigned int pdm_din0_h28_pins[]			= { GPIOH_28 };
static const unsigned int pdm_dclk_h29_pins[]			= { GPIOH_29 };

/* Bank[H]:Function[4] */
static const unsigned int vx1_a_lockn_pins[]			= { GPIOH_0 };
static const unsigned int vx1_a_htpdn_pins[]			= { GPIOH_1 };
static const unsigned int vx1_b_lockn_pins[]			= { GPIOH_2 };
static const unsigned int vx1_b_htpdn_pins[]			= { GPIOH_3 };
static const unsigned int pwm_d_h_pins[]			= { GPIOH_5 };
static const unsigned int pwm_d_h12_pins[]			= { GPIOH_12 };
static const unsigned int pwm_e_h13_pins[]			= { GPIOH_13 };
static const unsigned int tdm_d3_h14_pins[]			= { GPIOH_14 };
static const unsigned int eth_act_led_h_pins[]			= { GPIOH_18 };
static const unsigned int eth_link_led_h_pins[]			= { GPIOH_19 };
static const unsigned int i2c_d_scl_h20_pins[]			= { GPIOH_20 };
static const unsigned int i2c_d_sda_h21_pins[]			= { GPIOH_21 };
static const unsigned int uart_c_cts_h_pins[]			= { GPIOH_22 };
static const unsigned int uart_c_rts_h_pins[]			= { GPIOH_23 };
static const unsigned int uart_c_tx_h24_pins[]			= { GPIOH_24 };
static const unsigned int uart_c_rx_h25_pins[]			= { GPIOH_25 };
static const unsigned int uart_c_cts_h26_pins[]			= { GPIOH_26 };
static const unsigned int uart_c_rts_h27_pins[]			= { GPIOH_27 };
static const unsigned int uart_c_tx_h28_pins[]			= { GPIOH_28 };
static const unsigned int uart_c_rx_h29_pins[]			= { GPIOH_29 };

/* Bank[H]:Function[5] */
static const unsigned int tdm_pins[]				= { GPIOH_5 };
static const unsigned int clk12_24m_h_pins[]			= { GPIOH_13 };
static const unsigned int spi_clk_a_h_pins[]			= { GPIOH_14 };
static const unsigned int tdm_fs1_h18_pins[]			= { GPIOH_18 };
static const unsigned int spi_ss1_a_h_pins[]			= { GPIOH_19 };
static const unsigned int demod_uart_tx_h_pins[]		= { GPIOH_20 };
static const unsigned int demod_uart_rx_h_pins[]		= { GPIOH_21 };
static const unsigned int spi_ss0_b_h_pins[]			= { GPIOH_22 };
static const unsigned int spi_miso_b_h_pins[]			= { GPIOH_23 };
static const unsigned int spi_mosi_b_h_pins[]			= { GPIOH_24 };
static const unsigned int spi_clk_b_h_pins[]			= { GPIOH_25 };
static const unsigned int tdm_d2_h26_pins[]			= { GPIOH_26 };
static const unsigned int tdm_d3_h27_pins[]			= { GPIOH_27 };
static const unsigned int tdm_fs2_h_pins[]			= { GPIOH_28 };
static const unsigned int tdm_sclk2_h_pins[]			= { GPIOH_29 };

/* Bank[H]:Function[6] */
static const unsigned int gen_clk_out_h_pins[]			= { GPIOH_13 };
static const unsigned int s2_demod_gpio0_h_pins[]		= { GPIOH_14 };
static const unsigned int s2_demod_gpio1_pins[]			= { GPIOH_15 };
static const unsigned int s2_demod_gpio2_pins[]			= { GPIOH_16 };
static const unsigned int s2_demod_gpio3_pins[]			= { GPIOH_17 };
static const unsigned int s2_demod_gpio4_pins[]			= { GPIOH_18 };
static const unsigned int s2_demod_gpio5_pins[]			= { GPIOH_19 };
static const unsigned int s2_demod_gpio6_pins[]			= { GPIOH_20 };
static const unsigned int s2_demod_gpio7_pins[]			= { GPIOH_21 };
static const unsigned int tdm_sclk2_h22_pins[]			= { GPIOH_22 };
static const unsigned int tdm_fs2_h23_pins[]			= { GPIOH_23 };
static const unsigned int spi_ss1_a_h26_pins[]			= { GPIOH_26 };
static const unsigned int dcon_led_h_pins[]			= { GPIOH_27 };
static const unsigned int eth_mdio_h_pins[]			= { GPIOH_28 };
static const unsigned int eth_mdc_h_pins[]			= { GPIOH_29 };

/* Bank[H]:Function[7] */
static const unsigned int tst_clk_out22_h_pins[]		= { GPIOH_0 };
static const unsigned int debug_bus_in0_pins[]			= { GPIOH_1 };
static const unsigned int debug_bus_in1_pins[]			= { GPIOH_2 };
static const unsigned int debug_bus_in2_pins[]			= { GPIOH_3 };
static const unsigned int debug_bus_in3_pins[]			= { GPIOH_4 };
static const unsigned int debug_bus_in4_pins[]			= { GPIOH_5 };
static const unsigned int debug_bus_in5_pins[]			= { GPIOH_6 };
static const unsigned int debug_bus_in6_pins[]			= { GPIOH_7 };
static const unsigned int debug_bus_in7_pins[]			= { GPIOH_8 };
static const unsigned int debug_bus_in8_pins[]			= { GPIOH_9 };
static const unsigned int debug_bus_in9_pins[]			= { GPIOH_10 };
static const unsigned int debug_bus_in10_pins[]			= { GPIOH_11 };
static const unsigned int debug_bus_in11_pins[]			= { GPIOH_12 };
static const unsigned int bcon_8_pins[]				= { GPIOH_13 };
static const unsigned int debug_bus_in12_pins[]			= { GPIOH_14 };
static const unsigned int debug_bus_in13_pins[]			= { GPIOH_15 };
static const unsigned int debug_bus_in14_pins[]			= { GPIOH_16 };
static const unsigned int debug_bus_in15_pins[]			= { GPIOH_17 };
static const unsigned int debug_bus_in16_pins[]			= { GPIOH_18 };
static const unsigned int debug_bus_in17_pins[]			= { GPIOH_19 };
static const unsigned int debug_bus_in18_pins[]			= { GPIOH_20 };
static const unsigned int debug_bus_in19_pins[]			= { GPIOH_21 };
static const unsigned int bcon_0_pins[]				= { GPIOH_22 };
static const unsigned int bcon_1_pins[]				= { GPIOH_23 };
static const unsigned int bcon_2_pins[]				= { GPIOH_24 };
static const unsigned int bcon_3_pins[]				= { GPIOH_25 };
static const unsigned int bcon_4_pins[]				= { GPIOH_26 };
static const unsigned int bcon_5_pins[]				= { GPIOH_27 };
static const unsigned int bcon_6_pins[]				= { GPIOH_28 };
static const unsigned int bcon_7_pins[]				= { GPIOH_29 };

/* Bank[P]:Function[1] */
static const unsigned int spi_miso_c_p_pins[]			= { GPIOP_0 };
static const unsigned int spi_mosi_c_p_pins[]			= { GPIOP_1 };
static const unsigned int spi_clk_c_p_pins[]			= { GPIOP_2 };
static const unsigned int spi_ss0_c_p_pins[]			= { GPIOP_3 };
static const unsigned int i2c_d_scl_p_pins[]			= { GPIOP_4 };
static const unsigned int i2c_d_sda_p_pins[]			= { GPIOP_5 };
static const unsigned int mclk_2_p_pins[]			= { GPIOP_6 };
static const unsigned int tdm_sclk2_p_pins[]			= { GPIOP_7 };
static const unsigned int tdm_fs2_p_pins[]			= { GPIOP_8 };
static const unsigned int tdm_d2_p_pins[]			= { GPIOP_9 };
static const unsigned int dcon_led_p_pins[]			= { GPIOP_10 };
static const unsigned int spdif_out_p_pins[]			= { GPIOP_11 };

/* Bank[P]:Function[2] */
static const unsigned int uart_c_tx_p_pins[]			= { GPIOP_0 };
static const unsigned int uart_c_rx_p_pins[]			= { GPIOP_1 };
static const unsigned int uart_c_cts_p_pins[]			= { GPIOP_2 };
static const unsigned int uart_c_rts_p_pins[]			= { GPIOP_3 };
static const unsigned int pwm_i_p_pins[]			= { GPIOP_4 };
static const unsigned int pwm_j_p_pins[]			= { GPIOP_5 };
static const unsigned int tdm_d4_p_pins[]			= { GPIOP_6 };
static const unsigned int ir_in_b_p_pins[]			= { GPIOP_7 };
static const unsigned int ir_out_p_pins[]			= { GPIOP_8 };
static const unsigned int tdm_d3_p_pins[]			= { GPIOP_9 };
static const unsigned int ir_in_b_p10_pins[]			= { GPIOP_10 };
static const unsigned int ir_out_p11_pins[]			= { GPIOP_11 };

/* Bank[P]:Function[3] */
static const unsigned int spdif_out_p0_pins[]			= { GPIOP_0 };
static const unsigned int pwm_g_p_pins[]			= { GPIOP_1 };
static const unsigned int pwm_h_p_pins[]			= { GPIOP_2 };
static const unsigned int spi_dq2_c_p_pins[]			= { GPIOP_4 };
static const unsigned int spi_dq3_c_p_pins[]			= { GPIOP_5 };
static const unsigned int pwm_e_p_pins[]			= { GPIOP_6 };
static const unsigned int pwm_g_p7_pins[]			= { GPIOP_7 };
static const unsigned int pwm_h_p8_pins[]			= { GPIOP_8 };
static const unsigned int dcon_led_p9_pins[]			= { GPIOP_9 };
static const unsigned int gen_clk_out_p_pins[]			= { GPIOP_10 };

/* Bank[P]:Function[4] */
static const unsigned int bcon_8_p_pins[]			= { GPIOP_0 };
static const unsigned int bcon_9_pins[]				= { GPIOP_1 };
static const unsigned int bcon_10_pins[]			= { GPIOP_2 };
static const unsigned int bcon_11_pins[]			= { GPIOP_3 };
static const unsigned int bcon_12_pins[]			= { GPIOP_4 };
static const unsigned int bcon_13_pins[]			= { GPIOP_5 };
static const unsigned int bcon_14_pins[]			= { GPIOP_6 };
static const unsigned int bcon_15_pins[]			= { GPIOP_7 };
static const unsigned int bcon_16_pins[]			= { GPIOP_8 };
static const unsigned int bcon_17_pins[]			= { GPIOP_9 };

/* Bank[P]:Function[5] */
static const unsigned int spi_miso_d_p_pins[]			= { GPIOP_0 };
static const unsigned int spi_mosi_d_p_pins[]			= { GPIOP_1 };
static const unsigned int spi_clk_d_p_pins[]			= { GPIOP_2 };
static const unsigned int spi_ss0_d_p_pins[]			= { GPIOP_3 };
static const unsigned int spi_mosi_e_p_pins[]			= { GPIOP_4 };
static const unsigned int spi_clk_e_p_pins[]			= { GPIOP_5 };
static const unsigned int spi_ss0_e_p_pins[]			= { GPIOP_6 };
static const unsigned int spi_mosi_f_p_pins[]			= { GPIOP_7 };
static const unsigned int spi_clk_f_p_pins[]			= { GPIOP_8 };
static const unsigned int spi_ss0_f_p_pins[]			= { GPIOP_9 };
static const unsigned int spi_miso_e_pins[]			= { GPIOP_10 };
static const unsigned int spi_miso_f_p_pins[]			= { GPIOP_11 };

/* Bank[P]:Function[6] */
static const unsigned int tsin_c_clk_p_pins[]			= { GPIOP_0 };
static const unsigned int tsin_c_sop_p_pins[]			= { GPIOP_1 };
static const unsigned int tsin_c_valid_p_pins[]			= { GPIOP_2 };
static const unsigned int tsin_c_d0_p_pins[]			= { GPIOP_3 };
static const unsigned int tsin_c_d1_p_pins[]			= { GPIOP_4 };
static const unsigned int tsin_c_d2_p_pins[]			= { GPIOP_5 };
static const unsigned int tsin_c_d3_p_pins[]			= { GPIOP_6 };
static const unsigned int tsin_c_d4_p_pins[]			= { GPIOP_7 };
static const unsigned int tsin_c_d5_p_pins[]			= { GPIOP_8 };
static const unsigned int tsin_c_d6_p_pins[]			= { GPIOP_9 };
static const unsigned int tsin_c_d7_p_pins[]			= { GPIOP_10 };

/* Bank[P]:Function[7] */
static const unsigned int eth_rgmii_rx_clk_p_pins[]		= { GPIOP_0 };
static const unsigned int eth_rx_dv_p_pins[]			= { GPIOP_1 };
static const unsigned int eth_rxd0_p_pins[]			= { GPIOP_2 };
static const unsigned int eth_rxd1_p_pins[]			= { GPIOP_3 };
static const unsigned int eth_rxd2_rgmii_p_pins[]		= { GPIOP_4 };
static const unsigned int eth_rxd3_rgmii_p_pins[]		= { GPIOP_5 };
static const unsigned int eth_rgmii_tx_clk_p_pins[]		= { GPIOP_6 };
static const unsigned int eth_txen_p_pins[]			= { GPIOP_7 };
static const unsigned int eth_txd0_p_pins[]			= { GPIOP_8 };
static const unsigned int eth_txd1_p_pins[]			= { GPIOP_9 };
static const unsigned int eth_txd2_rgmii_p_pins[]		= { GPIOP_10 };
static const unsigned int eth_txd3_rgmii_p_pins[]		= { GPIOP_11 };

/* Bank[M]:Function[1] */
static const unsigned int tsin_b_clk_pins[]			= { GPIOM_0 };
static const unsigned int tsin_b_sop_pins[]			= { GPIOM_1 };
static const unsigned int tsin_b_valid_pins[]			= { GPIOM_2 };
static const unsigned int tsin_b_d0_pins[]			= { GPIOM_3 };
static const unsigned int tsin_b_d1_pins[]			= { GPIOM_4 };
static const unsigned int tsin_b_d2_pins[]			= { GPIOM_5 };
static const unsigned int tsin_b_d3_pins[]			= { GPIOM_6 };
static const unsigned int tsin_b_d4_pins[]			= { GPIOM_7 };
static const unsigned int tsin_b_d5_pins[]			= { GPIOM_8 };
static const unsigned int tsin_b_d6_pins[]			= { GPIOM_9 };
static const unsigned int tsin_b_d7_pins[]			= { GPIOM_10 };
static const unsigned int cicam_data0_pins[]			= { GPIOM_11 };
static const unsigned int cicam_data1_pins[]			= { GPIOM_12 };
static const unsigned int cicam_data2_pins[]			= { GPIOM_13 };
static const unsigned int cicam_data3_pins[]			= { GPIOM_14 };
static const unsigned int cicam_data4_pins[]			= { GPIOM_15 };
static const unsigned int cicam_data5_pins[]			= { GPIOM_16 };
static const unsigned int cicam_data6_pins[]			= { GPIOM_17 };
static const unsigned int cicam_data7_pins[]			= { GPIOM_18 };
static const unsigned int cicam_a0_pins[]			= { GPIOM_19 };
static const unsigned int cicam_a1_pins[]			= { GPIOM_20 };
static const unsigned int cicam_cen_pins[]			= { GPIOM_21 };
static const unsigned int cicam_oen_pins[]			= { GPIOM_22 };
static const unsigned int cicam_wen_pins[]			= { GPIOM_23 };
static const unsigned int cicam_iordn_pins[]			= { GPIOM_24 };
static const unsigned int cicam_iowrn_pins[]			= { GPIOM_25 };
static const unsigned int cicam_reset_pins[]			= { GPIOM_26 };
static const unsigned int cicam_waitn_m_pins[]			= { GPIOM_29 };

/* Bank[M]:Function[2] */
static const unsigned int spi_miso_c_m_pins[]			= { GPIOM_0 };
static const unsigned int spi_mosi_c_m_pins[]			= { GPIOM_1 };
static const unsigned int spi_clk_c_m_pins[]			= { GPIOM_2 };
static const unsigned int spi_ss0_c_m_pins[]			= { GPIOM_3 };
static const unsigned int spi_miso_d_m_pins[]			= { GPIOM_4 };
static const unsigned int spi_mosi_d_m_pins[]			= { GPIOM_5 };
static const unsigned int spi_clk_d_m_pins[]			= { GPIOM_6 };
static const unsigned int spi_ss0_d_m_pins[]			= { GPIOM_7 };
static const unsigned int pdm_din0_m_pins[]			= { GPIOM_10 };
static const unsigned int pdm_dclk_m_pins[]			= { GPIOM_11 };
static const unsigned int mclk_2_m_pins[]			= { GPIOM_12 };
static const unsigned int tdm_fs2_m_pins[]			= { GPIOM_13 };
static const unsigned int tdm_sclk2_m_pins[]			= { GPIOM_14 };
static const unsigned int tdm_d4_m_pins[]			= { GPIOM_15 };
static const unsigned int tdm_d5_m_pins[]			= { GPIOM_16 };
static const unsigned int tdm_d6_m_pins[]			= { GPIOM_17 };
static const unsigned int tdm_d7_m_pins[]			= { GPIOM_18 };
static const unsigned int iso7816_clk_m_pins[]			= { GPIOM_19 };
static const unsigned int iso7816_data_m_pins[]			= { GPIOM_20 };
static const unsigned int mclk_1_m_pins[]			= { GPIOM_21 };
static const unsigned int tdm_fs1_m_pins[]			= { GPIOM_22 };
static const unsigned int tdm_sclk1_m_pins[]			= { GPIOM_23 };
static const unsigned int tdm_d0_m_pins[]			= { GPIOM_24 };
static const unsigned int tdm_d1_m_pins[]			= { GPIOM_25 };
static const unsigned int tdm_d2_m_pins[]			= { GPIOM_26 };
static const unsigned int tdm_d3_m_pins[]			= { GPIOM_29 };

/* Bank[M]:Function[3] */
static const unsigned int uart_a_tx_m_pins[]			= { GPIOM_0 };
static const unsigned int uart_a_rx_m_pins[]			= { GPIOM_1 };
static const unsigned int uart_a_cts_m_pins[]			= { GPIOM_2 };
static const unsigned int uart_a_rts_m_pins[]			= { GPIOM_3 };
static const unsigned int pwm_f_m_pins[]			= { GPIOM_10 };
static const unsigned int pwm_i_m_pins[]			= { GPIOM_11 };
static const unsigned int pwm_j_m_pins[]			= { GPIOM_12 };
static const unsigned int i2c_d_scl_m_pins[]			= { GPIOM_17 };
static const unsigned int i2c_d_sda_m_pins[]			= { GPIOM_18 };
static const unsigned int spi_miso_a_m_pins[]			= { GPIOM_19 };
static const unsigned int spi_mosi_a_m_pins[]			= { GPIOM_20 };
static const unsigned int spi_clk_a_m_pins[]			= { GPIOM_21 };
static const unsigned int spi_ss0_a_m_pins[]			= { GPIOM_22 };
static const unsigned int spi_ss1_a_m_pins[]			= { GPIOM_23 };
static const unsigned int spi_ss2_a_m_pins[]			= { GPIOM_24 };
static const unsigned int i2c_d_scl_m25_pins[]			= { GPIOM_25 };
static const unsigned int i2c_d_sda_m26_pins[]			= { GPIOM_26 };
static const unsigned int i2c_c_scl_m_pins[]			= { GPIOM_27 };
static const unsigned int i2c_c_sda_m_pins[]			= { GPIOM_28 };
static const unsigned int spdif_out_m_pins[]			= { GPIOM_29 };

/* Bank[M]:Function[4] */
static const unsigned int pwm_d_m_pins[]			= { GPIOM_1 };
static const unsigned int uart_d_tx_m_pins[]			= { GPIOM_5 };
static const unsigned int uart_d_rx_m_pins[]			= { GPIOM_6 };
static const unsigned int pwm_g_m_pins[]			= { GPIOM_8 };
static const unsigned int pwm_h_m_pins[]			= { GPIOM_9 };
static const unsigned int uart_d_tx_m17_pins[]			= { GPIOM_17 };
static const unsigned int uart_d_rx_m18_pins[]			= { GPIOM_18 };
static const unsigned int uart_c_tx_m_pins[]			= { GPIOM_19 };
static const unsigned int uart_c_rx_m_pins[]			= { GPIOM_20 };
static const unsigned int uart_c_cts_m_pins[]			= { GPIOM_21 };
static const unsigned int uart_c_rts_m_pins[]			= { GPIOM_22 };
static const unsigned int pwm_d_m23_pins[]			= { GPIOM_23 };
static const unsigned int pwm_e_m_pins[]			= { GPIOM_24 };
static const unsigned int pwm_f_m26_pins[]			= { GPIOM_26 };
static const unsigned int gen_clk_out_m_pins[]			= { GPIOM_29 };

/* Bank[M]:Function[5] */
static const unsigned int tsin_a_clk_m_pins[]			= { GPIOM_19 };
static const unsigned int tsin_a_sop_m_pins[]			= { GPIOM_20 };
static const unsigned int tsin_a_valid_m_pins[]			= { GPIOM_21 };
static const unsigned int tsin_a_d0_m_pins[]			= { GPIOM_22 };
static const unsigned int demod_uart_tx_m_pins[]		= { GPIOM_23 };
static const unsigned int demod_uart_rx_m_pins[]		= { GPIOM_24 };
static const unsigned int demod_uart_tx_m25_pins[]		= { GPIOM_25 };
static const unsigned int demod_uart_rx_m26_pins[]		= { GPIOM_26 };

/* Bank[M]:Function[6] */
static const unsigned int tst_clk_out0_pins[]			= { GPIOM_0 };
static const unsigned int tst_clk_out1_pins[]			= { GPIOM_1 };
static const unsigned int tst_clk_out2_pins[]			= { GPIOM_2 };
static const unsigned int tst_clk_out3_pins[]			= { GPIOM_3 };
static const unsigned int tst_clk_out4_m_pins[]			= { GPIOM_4 };
static const unsigned int tst_clk_out5_pins[]			= { GPIOM_5 };
static const unsigned int tst_clk_out6_pins[]			= { GPIOM_6 };
static const unsigned int tst_clk_out7_pins[]			= { GPIOM_7 };
static const unsigned int tst_clk_out8_pins[]			= { GPIOM_8 };
static const unsigned int tst_clk_out9_pins[]			= { GPIOM_9 };
static const unsigned int tst_clk_out10_pins[]			= { GPIOM_10 };
static const unsigned int tst_clk_out11_pins[]			= { GPIOM_11 };
static const unsigned int tst_clk_out12_pins[]			= { GPIOM_12 };
static const unsigned int tst_clk_out13_pins[]			= { GPIOM_13 };
static const unsigned int tst_clk_out14_pins[]			= { GPIOM_14 };
static const unsigned int tst_clk_out15_pins[]			= { GPIOM_15 };
static const unsigned int tst_clk_out16_pins[]			= { GPIOM_16 };
static const unsigned int tst_clk_out17_pins[]			= { GPIOM_17 };
static const unsigned int tst_clk_out18_pins[]			= { GPIOM_18 };
static const unsigned int tst_clk_out19_pins[]			= { GPIOM_19 };
static const unsigned int tst_clk_out20_pins[]			= { GPIOM_20 };
static const unsigned int tst_clk_out21_pins[]			= { GPIOM_21 };
static const unsigned int tst_clk_out22_m_pins[]		= { GPIOM_22 };
static const unsigned int tst_clk_out23_pins[]			= { GPIOM_23 };
static const unsigned int tst_clk_out24_pins[]			= { GPIOM_24 };
static const unsigned int tst_clk_out25_pins[]			= { GPIOM_25 };
static const unsigned int ir_in_b_m_pins[]			= { GPIOM_29 };

/* Bank[M]:Function[7] */
static const unsigned int debug_bus_out0_pins[]			= { GPIOM_0 };
static const unsigned int debug_bus_out1_pins[]			= { GPIOM_1 };
static const unsigned int debug_bus_out2_pins[]			= { GPIOM_2 };
static const unsigned int debug_bus_out3_pins[]			= { GPIOM_3 };
static const unsigned int debug_bus_out4_pins[]			= { GPIOM_4 };
static const unsigned int debug_bus_out5_pins[]			= { GPIOM_5 };
static const unsigned int debug_bus_out6_pins[]			= { GPIOM_6 };
static const unsigned int debug_bus_out7_pins[]			= { GPIOM_7 };
static const unsigned int debug_bus_out8_pins[]			= { GPIOM_8 };
static const unsigned int debug_bus_out9_pins[]			= { GPIOM_9 };
static const unsigned int debug_bus_out10_pins[]		= { GPIOM_10 };
static const unsigned int debug_bus_out11_pins[]		= { GPIOM_11 };
static const unsigned int debug_bus_out12_pins[]		= { GPIOM_12 };
static const unsigned int debug_bus_out13_pins[]		= { GPIOM_13 };
static const unsigned int debug_bus_out14_pins[]		= { GPIOM_14 };
static const unsigned int debug_bus_out15_pins[]		= { GPIOM_15 };
static const unsigned int debug_bus_out16_pins[]		= { GPIOM_16 };
static const unsigned int debug_bus_out17_pins[]		= { GPIOM_17 };
static const unsigned int debug_bus_out18_pins[]		= { GPIOM_18 };
static const unsigned int debug_bus_out19_pins[]		= { GPIOM_19 };
static const unsigned int tst_clk_out22_m28_pins[]		= { GPIOM_28 };

static struct meson_pmx_group meson_t6x_periphs_groups[] __initdata = {
	/* GPIOW */
	GPIO_GROUP(GPIOW_0),
	GPIO_GROUP(GPIOW_1),
	GPIO_GROUP(GPIOW_2),
	GPIO_GROUP(GPIOW_3),
	GPIO_GROUP(GPIOW_4),
	GPIO_GROUP(GPIOW_5),
	GPIO_GROUP(GPIOW_6),
	GPIO_GROUP(GPIOW_7),
	GPIO_GROUP(GPIOW_8),
	GPIO_GROUP(GPIOW_9),
	GPIO_GROUP(GPIOW_10),
	GPIO_GROUP(GPIOW_11),
	GPIO_GROUP(GPIOW_12),
	GPIO_GROUP(GPIOW_13),
	GPIO_GROUP(GPIOW_14),
	GPIO_GROUP(GPIOW_15),
	GPIO_GROUP(GPIOW_16),
	/* GPIOD(GPIOAO) */
	GPIO_GROUP(GPIOD_0),
	GPIO_GROUP(GPIOD_1),
	GPIO_GROUP(GPIOD_2),
	GPIO_GROUP(GPIOD_3),
	GPIO_GROUP(GPIOD_4),
	GPIO_GROUP(GPIOD_5),
	GPIO_GROUP(GPIOD_6),
	GPIO_GROUP(GPIOD_7),
	GPIO_GROUP(GPIOD_8),
	GPIO_GROUP(GPIOD_9),
	GPIO_GROUP(GPIOD_10),
	GPIO_GROUP(GPIOD_11),
	GPIO_GROUP(GPIOD_12),
	GPIO_GROUP(GPIOD_13),
	GPIO_GROUP(GPIOD_14),
	/* GPIOE */
	GPIO_GROUP(GPIOE_0),
	GPIO_GROUP(GPIOE_1),
	/* GPIOB */
	GPIO_GROUP(GPIOB_0),
	GPIO_GROUP(GPIOB_1),
	GPIO_GROUP(GPIOB_2),
	GPIO_GROUP(GPIOB_3),
	GPIO_GROUP(GPIOB_4),
	GPIO_GROUP(GPIOB_5),
	GPIO_GROUP(GPIOB_6),
	GPIO_GROUP(GPIOB_7),
	GPIO_GROUP(GPIOB_8),
	GPIO_GROUP(GPIOB_9),
	GPIO_GROUP(GPIOB_10),
	GPIO_GROUP(GPIOB_11),
	GPIO_GROUP(GPIOB_12),
	GPIO_GROUP(GPIOB_13),
	/* GPIOC */
	GPIO_GROUP(GPIOC_0),
	GPIO_GROUP(GPIOC_1),
	GPIO_GROUP(GPIOC_2),
	GPIO_GROUP(GPIOC_3),
	GPIO_GROUP(GPIOC_4),
	GPIO_GROUP(GPIOC_5),
	GPIO_GROUP(GPIOC_6),
	GPIO_GROUP(GPIOC_7),
	GPIO_GROUP(GPIOC_8),
	GPIO_GROUP(GPIOC_9),
	GPIO_GROUP(GPIOC_10),
	/* GPIOZ */
	GPIO_GROUP(GPIOZ_0),
	GPIO_GROUP(GPIOZ_1),
	GPIO_GROUP(GPIOZ_2),
	GPIO_GROUP(GPIOZ_3),
	GPIO_GROUP(GPIOZ_4),
	GPIO_GROUP(GPIOZ_5),
	GPIO_GROUP(GPIOZ_6),
	GPIO_GROUP(GPIOZ_7),
	GPIO_GROUP(GPIOZ_8),
	GPIO_GROUP(GPIOZ_9),
	GPIO_GROUP(GPIOZ_10),
	GPIO_GROUP(GPIOZ_11),
	GPIO_GROUP(GPIOZ_12),
	GPIO_GROUP(GPIOZ_13),
	GPIO_GROUP(GPIOZ_14),
	GPIO_GROUP(GPIOZ_15),
	GPIO_GROUP(GPIOZ_16),
	GPIO_GROUP(GPIOZ_17),
	GPIO_GROUP(GPIOZ_18),
	GPIO_GROUP(GPIOZ_19),
	/* GPIOH */
	GPIO_GROUP(GPIOH_0),
	GPIO_GROUP(GPIOH_1),
	GPIO_GROUP(GPIOH_2),
	GPIO_GROUP(GPIOH_3),
	GPIO_GROUP(GPIOH_4),
	GPIO_GROUP(GPIOH_5),
	GPIO_GROUP(GPIOH_6),
	GPIO_GROUP(GPIOH_7),
	GPIO_GROUP(GPIOH_8),
	GPIO_GROUP(GPIOH_9),
	GPIO_GROUP(GPIOH_10),
	GPIO_GROUP(GPIOH_11),
	GPIO_GROUP(GPIOH_12),
	GPIO_GROUP(GPIOH_13),
	GPIO_GROUP(GPIOH_14),
	GPIO_GROUP(GPIOH_15),
	GPIO_GROUP(GPIOH_16),
	GPIO_GROUP(GPIOH_17),
	GPIO_GROUP(GPIOH_18),
	GPIO_GROUP(GPIOH_19),
	GPIO_GROUP(GPIOH_20),
	GPIO_GROUP(GPIOH_21),
	GPIO_GROUP(GPIOH_22),
	GPIO_GROUP(GPIOH_23),
	GPIO_GROUP(GPIOH_24),
	GPIO_GROUP(GPIOH_25),
	GPIO_GROUP(GPIOH_26),
	GPIO_GROUP(GPIOH_27),
	GPIO_GROUP(GPIOH_28),
	GPIO_GROUP(GPIOH_29),
	/* GPIOP */
	GPIO_GROUP(GPIOP_0),
	GPIO_GROUP(GPIOP_1),
	GPIO_GROUP(GPIOP_2),
	GPIO_GROUP(GPIOP_3),
	GPIO_GROUP(GPIOP_4),
	GPIO_GROUP(GPIOP_5),
	GPIO_GROUP(GPIOP_6),
	GPIO_GROUP(GPIOP_7),
	GPIO_GROUP(GPIOP_8),
	GPIO_GROUP(GPIOP_9),
	GPIO_GROUP(GPIOP_10),
	GPIO_GROUP(GPIOP_11),
	/* GPIOM */
	GPIO_GROUP(GPIOM_0),
	GPIO_GROUP(GPIOM_1),
	GPIO_GROUP(GPIOM_2),
	GPIO_GROUP(GPIOM_3),
	GPIO_GROUP(GPIOM_4),
	GPIO_GROUP(GPIOM_5),
	GPIO_GROUP(GPIOM_6),
	GPIO_GROUP(GPIOM_7),
	GPIO_GROUP(GPIOM_8),
	GPIO_GROUP(GPIOM_9),
	GPIO_GROUP(GPIOM_10),
	GPIO_GROUP(GPIOM_11),
	GPIO_GROUP(GPIOM_12),
	GPIO_GROUP(GPIOM_13),
	GPIO_GROUP(GPIOM_14),
	GPIO_GROUP(GPIOM_15),
	GPIO_GROUP(GPIOM_16),
	GPIO_GROUP(GPIOM_17),
	GPIO_GROUP(GPIOM_18),
	GPIO_GROUP(GPIOM_19),
	GPIO_GROUP(GPIOM_20),
	GPIO_GROUP(GPIOM_21),
	GPIO_GROUP(GPIOM_22),
	GPIO_GROUP(GPIOM_23),
	GPIO_GROUP(GPIOM_24),
	GPIO_GROUP(GPIOM_25),
	GPIO_GROUP(GPIOM_26),
	GPIO_GROUP(GPIOM_27),
	GPIO_GROUP(GPIOM_28),
	GPIO_GROUP(GPIOM_29),
	/* GPIO_TEST_N */
	GPIO_GROUP(GPIO_TEST_N),
	/* Bank[W]:Function[1] */
	GROUP(hdmirx_hpd_a,			1),
	GROUP(hdmirx_5vdet_a,			1),
	GROUP(hdmirx_sda_a,			1),
	GROUP(hdmirx_scl_a,			1),
	GROUP(hdmirx_hpd_b,			1),
	GROUP(hdmirx_5vdet_b,			1),
	GROUP(hdmirx_sda_b,			1),
	GROUP(hdmirx_scl_b,			1),
	GROUP(hdmirx_hpd_c,			1),
	GROUP(hdmirx_5vdet_c,			1),
	GROUP(hdmirx_sda_c,			1),
	GROUP(hdmirx_scl_c,			1),
	GROUP(hdmirx_hpd_d,			1),
	GROUP(hdmirx_5vdet_d,			1),
	GROUP(hdmirx_sda_d,			1),
	GROUP(hdmirx_scl_d,			1),
	GROUP(cec,				1),
	/* Bank[W]:Function[2] */
	GROUP(uart_b_tx,			2),
	GROUP(uart_b_rx,			2),
	GROUP(uart_b_tx_w,			2),
	GROUP(uart_b_rx_w,			2),
	GROUP(uart_b_tx_w10,			2),
	GROUP(uart_b_rx_w11,			2),
	GROUP(uart_b_tx_w14,			2),
	GROUP(uart_b_rx_w15,			2),
	/* Bank[W]:Function[3] */
	GROUP(jtag_a_clk,			3),
	GROUP(jtag_a_tms,			3),
	GROUP(tst_clk_out22,			3),
	GROUP(jtag_a_tdi,			3),
	GROUP(jtag_a_tdo,			3),
	GROUP(jtag_a_clk_w,			3),
	GROUP(jtag_a_tms_w,			3),
	GROUP(jtag_a_tdi_w,			3),
	GROUP(jtag_a_tdo_w,			3),
	/* Bank[W]:Function[4] */
	GROUP(gpiow3_loop,			4),
	GROUP(gpiow2_loop,			4),
	GROUP(jtag_a_clk_w6,			4),
	GROUP(jtag_a_tms_w7,			4),
	GROUP(jtag_a_tdi_w10,			4),
	GROUP(jtag_a_tdo_w11,			4),
	/* Bank[W]:Function[5] */
	GROUP(eth_mdio,				5),
	GROUP(eth_mdc,				5),
	GROUP(eth_mdio_w,			5),
	GROUP(eth_mdc_w,			5),
	/* Bank[W]:Function[7] */
	GROUP(tst_clk_out22_w,			7),
	GROUP(tst_clk_out22_w16,		7),
	/* Bank[D]:Function[1] */
	GROUP(uart_b_tx_d,			1),
	GROUP(uart_b_rx_d,			1),
	GROUP(i2c_b_scl,			1),
	GROUP(i2c_b_sda,			1),
	GROUP(clk_32k_in,			1),
	GROUP(ir_in_a,				1),
	GROUP(jtag_a_clk_d,			1),
	GROUP(jtag_a_tms_d,			1),
	GROUP(jtag_a_tdi_d,			1),
	GROUP(jtag_a_tdo_d,			1),
	GROUP(clk12_24m,			1),
	GROUP(pwm_a,				1),
	GROUP(pwm_f,				1),
	GROUP(i2c_b_sda_d,			1),
	GROUP(i2c_b_scl_d,			1),
	/* Bank[D]:Function[2] */
	GROUP(ir_out,				2),
	GROUP(atv_if_agc,			2),
	GROUP(pwm_c,				2),
	GROUP(spdif_out,			2),
	GROUP(pwm_c_d,				2),
	GROUP(spdif_out_d,			2),
	GROUP(pwm_d,				2),
	GROUP(pwm_e,				2),
	GROUP(pwm_c_d11,			2),
	GROUP(uart_d_tx,			2),
	GROUP(uart_d_rx,			2),
	/* Bank[D]:Function[3] */
	GROUP(cicam_waitn,			3),
	GROUP(demod_uart_tx,			3),
	GROUP(demod_uart_rx,			3),
	GROUP(dtv_if_agc,			3),
	GROUP(clk12_24m_d,			3),
	GROUP(pwm_d_hiz,			3),
	GROUP(pwm_c_hiz,			3),
	GROUP(ir_out_d,				3),
	GROUP(gen_clk_out,			3),
	GROUP(pwm_i,				3),
	GROUP(pwm_j,				3),
	GROUP(ir_in_b,				3),
	/* Bank[D]:Function[4] */
	GROUP(gen_clk_out_d,			4),
	GROUP(uart_c_tx,			4),
	GROUP(uart_c_rx,			4),
	GROUP(tdm_d2,				4),
	GROUP(tdm_d3,				4),
	/* Bank[D]:Function[5] */
	GROUP(ir_in_b_d,			5),
	GROUP(dcon_led,				5),
	GROUP(dcon_led_d,			5),
	GROUP(dcon_led_d9,			5),
	/* Bank[D]:Function[6] */
	GROUP(pwm_h,				6),
	GROUP(mclk_1,				6),
	GROUP(pwm_vs,				6),
	/* Bank[D]:Function[7] */
	GROUP(i2c_c_scl,			7),
	GROUP(i2c_c_sda,			7),
	GROUP(i2c_d_scl,			7),
	GROUP(i2c_d_sda,			7),
	/* Bank[E]:Function[1] */
	GROUP(pwm_a_e,				1),
	GROUP(pwm_b,				1),
	/* Bank[E]:Function[2] */
	GROUP(i2c_c_scl_e,			2),
	GROUP(i2c_c_sda_e,			2),
	/* Bank[E]:Function[3] */
	GROUP(tst_clk_out4,			3),
	/* Bank[B]:Function[1] */
	GROUP(emmc_d0,				1),
	GROUP(emmc_d1,				1),
	GROUP(emmc_d2,				1),
	GROUP(emmc_d3,				1),
	GROUP(emmc_d4,				1),
	GROUP(emmc_d5,				1),
	GROUP(emmc_d6,				1),
	GROUP(emmc_d7,				1),
	GROUP(emmc_clk,				1),
	GROUP(emmc_cmd,				1),
	GROUP(emmc_ds,				1),
	GROUP(gpiob11_loop,			1),
	GROUP(gpiob12_loop,			1),
	/* Bank[B]:Function[2] */
	GROUP(nand_wen_clk,			2),
	GROUP(nand_ale,				2),
	GROUP(nand_ren_wr,			2),
	GROUP(nand_cle,				2),
	GROUP(nand_ce0,				2),
	GROUP(gpioh21_loop,			2),
	/* Bank[B]:Function[3] */
	GROUP(spinf_mo_d0,			3),
	GROUP(spinf_mi_d1,			3),
	GROUP(spinf_wp_d2,			3),
	GROUP(spinf_hold_d3,			3),
	GROUP(spinf_d4,				3),
	GROUP(spinf_d5,				3),
	GROUP(spinf_d6,				3),
	GROUP(spinf_d7,				3),
	GROUP(spinf_clk,			3),
	GROUP(spinf_cs0,			3),
	GROUP(spinf_cs1,			3),
	/* Bank[B]:Function[4] */
	GROUP(ir_out_b,				4),
	/* Bank[B]:Function[5] */
	GROUP(gpioh21_loop_b,			5),
	/* Bank[B]:Function[7] */
	GROUP(gen_clk_out_b,			7),
	/* Bank[C]:Function[1] */
	GROUP(uart_a_tx,			1),
	GROUP(uart_a_rx,			1),
	GROUP(uart_a_cts,			1),
	GROUP(uart_a_rts,			1),
	GROUP(pwm_f_c,				1),
	/* Bank[C]:Function[2] */
	GROUP(spi_miso_c,			2),
	GROUP(spi_mosi_c,			2),
	GROUP(spi_clk_c,			2),
	GROUP(spi_ss0_c,			2),
	GROUP(spi_ss1_c,			2),
	GROUP(spi_ss2_c,			2),
	GROUP(mclk_1_c,				2),
	GROUP(mclk_2,				2),
	GROUP(pwm_j_c,				2),
	/* Bank[C]:Function[3] */
	GROUP(cicam_a2,				3),
	GROUP(cicam_a3,				3),
	GROUP(cicam_a4,				3),
	GROUP(cicam_a5,				3),
	GROUP(cicam_a6,				3),
	GROUP(cicam_a7,				3),
	GROUP(cicam_a8,				3),
	GROUP(cicam_a9,				3),
	GROUP(cicam_a10,			3),
	GROUP(cicam_a11,			3),
	GROUP(cicam_a12,			3),
	/* Bank[C]:Function[4] */
	GROUP(tsin_c_clk,			4),
	GROUP(tsin_c_sop,			4),
	GROUP(tsin_c_valid,			4),
	GROUP(tsin_c_d0,			4),
	GROUP(spi_dq2_c,			4),
	GROUP(spi_dq3_c,			4),
	GROUP(demod_uart_tx_c,			4),
	GROUP(demod_uart_rx_c,			4),
	GROUP(pwm_vs_c,				4),
	/* Bank[C]:Function[5] */
	GROUP(tdm_sclk2,			5),
	GROUP(tdm_fs2,				5),
	GROUP(tdm_d6,				5),
	GROUP(tdm_d7,				5),
	GROUP(tdm_fs1,				5),
	GROUP(tdm_sclk1,			5),
	GROUP(tdm_d4,				5),
	GROUP(tdm_d5,				5),
	GROUP(spdif_out_c,			5),
	/* Bank[C]:Function[6] */
	GROUP(spi_miso_d,			6),
	GROUP(spi_mosi_d,			6),
	GROUP(spi_clk_d,			6),
	GROUP(spi_ss0_d,			6),
	GROUP(spi_mosi_e,			6),
	GROUP(spi_clk_e,			6),
	GROUP(spi_ss0_e,			6),
	GROUP(spi_mosi_f,			6),
	GROUP(spi_clk_f,			6),
	GROUP(spi_ss0_f,			6),
	GROUP(spi_miso_f,			6),
	/* Bank[C]:Function[7] */
	GROUP(gen_clk_out_c,			7),
	GROUP(dcon_led_c,			7),
	/* Bank[Z]:Function[1] */
	GROUP(tdm_fs2_z,			1),
	GROUP(tdm_sclk2_z,			1),
	GROUP(tdm_d4_z,				1),
	GROUP(tdm_d5_z,				1),
	GROUP(tdm_d6_z,				1),
	GROUP(tdm_d7_z,				1),
	GROUP(mclk_2_z,				1),
	GROUP(tsout_clk,			1),
	GROUP(tsout_sop,			1),
	GROUP(tsout_valid,			1),
	GROUP(tsout_d0,				1),
	GROUP(tsout_d1,				1),
	GROUP(tsout_d2,				1),
	GROUP(tsout_d3,				1),
	GROUP(tsout_d4,				1),
	GROUP(tsout_d5,				1),
	GROUP(tsout_d6,				1),
	GROUP(tsout_d7,				1),
	GROUP(dcon_led_z,			1),
	GROUP(spdif_out_z,			1),
	/* Bank[Z]:Function[2] */
	GROUP(tsin_a_clk,			2),
	GROUP(tsin_a_sop,			2),
	GROUP(tsin_a_valid,			2),
	GROUP(tsin_a_d0,			2),
	GROUP(dtv_rf_agc,			2),
	GROUP(cicam_a2_z,			2),
	GROUP(cicam_a3_z,			2),
	GROUP(cicam_a4_z,			2),
	GROUP(cicam_a5_z,			2),
	GROUP(cicam_a6_z,			2),
	GROUP(cicam_a7_z,			2),
	GROUP(cicam_a8_z,			2),
	GROUP(cicam_a9_z,			2),
	GROUP(cicam_a10_z,			2),
	GROUP(cicam_a11_z,			2),
	GROUP(dtv_rf_agc_z,			2),
	GROUP(tst_clk_out4_z,			2),
	/* Bank[Z]:Function[3] */
	GROUP(iso7816_clk,			3),
	GROUP(iso7816_data,			3),
	GROUP(cicam_a13,			3),
	GROUP(cicam_a14,			3),
	GROUP(i2c_a_scl,			3),
	GROUP(i2c_a_sda,			3),
	GROUP(atv_if_agc_z,			3),
	GROUP(tsin_c_clk_z,			3),
	GROUP(tsin_c_sop_z,			3),
	GROUP(tsin_c_valid_z,			3),
	GROUP(tsin_c_d0_z,			3),
	GROUP(tsin_c_d1,			3),
	GROUP(tsin_c_d2,			3),
	GROUP(tsin_c_d3,			3),
	GROUP(tsin_c_d4,			3),
	GROUP(tsin_c_d5,			3),
	GROUP(tsin_c_d6,			3),
	GROUP(tsin_c_d7,			3),
	GROUP(atv_if_agc_z18,			3),
	/* Bank[Z]:Function[4] */
	GROUP(spi_miso_b,			4),
	GROUP(spi_mosi_b,			4),
	GROUP(spi_clk_b,			4),
	GROUP(spi_ss0_b,			4),
	GROUP(spi_dq2_b,			4),
	GROUP(spi_dq3_b,			4),
	GROUP(dtv_if_agc_z,			4),
	GROUP(dtv_if_agc_z18,			4),
	GROUP(pwm_g,				4),
	/* Bank[Z]:Function[5] */
	GROUP(uart_a_tx_z,			5),
	GROUP(uart_a_rx_z,			5),
	GROUP(uart_a_cts_z,			5),
	GROUP(uart_a_rts_z,			5),
	GROUP(pwm_b_z,				5),
	GROUP(pwm_d_z,				5),
	GROUP(pwm_e_z,				5),
	GROUP(uart_d_tx_z,			5),
	GROUP(uart_d_rx_z,			5),
	GROUP(pwm_g_z,				5),
	GROUP(pwm_h_z,				5),
	GROUP(pwm_i_z,				5),
	GROUP(pwm_j_z,				5),
	GROUP(ir_in_b_z,			5),
	/* Bank[Z]:Function[6] */
	GROUP(cicam_a12_z,			6),
	GROUP(pdm_dclk,				6),
	GROUP(pdm_din0,				6),
	GROUP(pdm_din1,				6),
	GROUP(tdm_d1,				6),
	GROUP(tdm_d0,				6),
	GROUP(tdm_sclk1_z,			6),
	GROUP(tdm_fs1_z,			6),
	GROUP(diseqc_out,			6),
	/* Bank[Z]:Function[7] */
	GROUP(diseqc_out_z,			7),
	GROUP(s2_demod_gpio0,			7),
	GROUP(spdif_out_z2,			7),
	GROUP(eth_mdio_z,			7),
	GROUP(eth_mdc_z,			7),
	GROUP(eth_link_led,			7),
	GROUP(eth_rgmii_rx_clk,			7),
	GROUP(eth_rx_dv,			7),
	GROUP(eth_rxd0,				7),
	GROUP(eth_rxd1,				7),
	GROUP(eth_rxd2_rgmii,			7),
	GROUP(eth_rxd3_rgmii,			7),
	GROUP(eth_rgmii_tx_clk,			7),
	GROUP(eth_txen,				7),
	GROUP(eth_txd0,				7),
	GROUP(eth_txd1,				7),
	GROUP(eth_txd2_rgmii,			7),
	GROUP(eth_txd3_rgmii,			7),
	GROUP(eth_act_led,			7),
	/* Bank[H]:Function[1] */
	GROUP(tcon_0,				1),
	GROUP(tcon_1,				1),
	GROUP(tcon_2,				1),
	GROUP(tcon_3,				1),
	GROUP(tcon_4,				1),
	GROUP(tcon_5,				1),
	GROUP(tcon_6,				1),
	GROUP(tcon_7,				1),
	GROUP(tcon_8,				1),
	GROUP(tcon_9,				1),
	GROUP(tcon_10,				1),
	GROUP(tcon_11,				1),
	GROUP(tcon_12,				1),
	GROUP(tcon_13,				1),
	GROUP(tcon_14,				1),
	GROUP(tcon_15,				1),
	GROUP(hsync,				1),
	GROUP(vsync,				1),
	GROUP(i2c_tcon_scl,			1),
	GROUP(i2c_tcon_sda,			1),
	GROUP(i2c_c_scl_h,			1),
	GROUP(i2c_c_sda_h,			1),
	GROUP(i2c_d_scl_h,			1),
	GROUP(i2c_d_sda_h,			1),
	GROUP(pwm_g_h,				1),
	GROUP(pwm_e_h,				1),
	GROUP(i2c_tcon_scl_h,			1),
	GROUP(i2c_tcon_sda_h,			1),
	/* Bank[H]:Function[2] */
	GROUP(tcon_lock,			2),
	GROUP(tcon_lock_h,			2),
	GROUP(i2c_tcon_scl_h2,			2),
	GROUP(i2c_tcon_sda_h3,			2),
	GROUP(sync_3d_out,			2),
	GROUP(tdm_d3_h,				2),
	GROUP(spi_ss1_a,			2),
	GROUP(spi_ss0_a,			2),
	GROUP(spi_miso_a,			2),
	GROUP(spi_mosi_a,			2),
	GROUP(spi_clk_a,			2),
	GROUP(spi_ss2_a,			2),
	GROUP(tdm_d3_h13,			2),
	GROUP(mclk_1_h,				2),
	GROUP(tdm_fs1_h,			2),
	GROUP(tdm_sclk1_h,			2),
	GROUP(tdm_d0_h,				2),
	GROUP(tdm_d1_h,				2),
	GROUP(tdm_d2_h,				2),
	GROUP(tdm_d3_h20,			2),
	GROUP(tdm_d4_h,				2),
	GROUP(tdm_d5_h,				2),
	GROUP(tdm_d6_h,				2),
	GROUP(spi_ss0_c_h,			2),
	GROUP(spi_miso_c_h,			2),
	GROUP(spi_mosi_c_h,			2),
	GROUP(spi_clk_c_h,			2),
	/* Bank[H]:Function[3] */
	GROUP(tcon_sfc,				3),
	GROUP(tcon_sfc_h,			3),
	GROUP(uart_c_tx_h,			3),
	GROUP(uart_c_rx_h,			3),
	GROUP(uart_c_cts,			3),
	GROUP(uart_c_rts,			3),
	GROUP(i2c_tcon_scl_h10,			3),
	GROUP(i2c_tcon_sda_h11,			3),
	GROUP(pwm_vs_h,				3),
	GROUP(pwm_vs_h13,			3),
	GROUP(pdm_din1_h,			3),
	GROUP(pdm_din0_h,			3),
	GROUP(pdm_dclk_h,			3),
	GROUP(gpiob13_loop,			3),
	GROUP(pdm_dclk_h22,			3),
	GROUP(pdm_din1_h23,			3),
	GROUP(i2c_b_scl_h,			3),
	GROUP(i2c_b_sda_h,			3),
	GROUP(pdm_din0_h26,			3),
	GROUP(pdm_din1_h27,			3),
	GROUP(pdm_din0_h28,			3),
	GROUP(pdm_dclk_h29,			3),
	/* Bank[H]:Function[4] */
	GROUP(vx1_a_lockn,			4),
	GROUP(vx1_a_htpdn,			4),
	GROUP(vx1_b_lockn,			4),
	GROUP(vx1_b_htpdn,			4),
	GROUP(pwm_d_h,				4),
	GROUP(pwm_d_h12,			4),
	GROUP(pwm_e_h13,			4),
	GROUP(tdm_d3_h14,			4),
	GROUP(eth_act_led_h,			4),
	GROUP(eth_link_led_h,			4),
	GROUP(i2c_d_scl_h20,			4),
	GROUP(i2c_d_sda_h21,			4),
	GROUP(uart_c_cts_h,			4),
	GROUP(uart_c_rts_h,			4),
	GROUP(uart_c_tx_h24,			4),
	GROUP(uart_c_rx_h25,			4),
	GROUP(uart_c_cts_h26,			4),
	GROUP(uart_c_rts_h27,			4),
	GROUP(uart_c_tx_h28,			4),
	GROUP(uart_c_rx_h29,			4),
	/* Bank[H]:Function[5] */
	GROUP(tdm,				5),
	GROUP(clk12_24m_h,			5),
	GROUP(spi_clk_a_h,			5),
	GROUP(tdm_fs1_h18,			5),
	GROUP(spi_ss1_a_h,			5),
	GROUP(demod_uart_tx_h,			5),
	GROUP(demod_uart_rx_h,			5),
	GROUP(spi_ss0_b_h,			5),
	GROUP(spi_miso_b_h,			5),
	GROUP(spi_mosi_b_h,			5),
	GROUP(spi_clk_b_h,			5),
	GROUP(tdm_d2_h26,			5),
	GROUP(tdm_d3_h27,			5),
	GROUP(tdm_fs2_h,			5),
	GROUP(tdm_sclk2_h,			5),
	/* Bank[H]:Function[6] */
	GROUP(gen_clk_out_h,			6),
	GROUP(s2_demod_gpio0_h,			6),
	GROUP(s2_demod_gpio1,			6),
	GROUP(s2_demod_gpio2,			6),
	GROUP(s2_demod_gpio3,			6),
	GROUP(s2_demod_gpio4,			6),
	GROUP(s2_demod_gpio5,			6),
	GROUP(s2_demod_gpio6,			6),
	GROUP(s2_demod_gpio7,			6),
	GROUP(tdm_sclk2_h22,			6),
	GROUP(tdm_fs2_h23,			6),
	GROUP(spi_ss1_a_h26,			6),
	GROUP(dcon_led_h,			6),
	GROUP(eth_mdio_h,			6),
	GROUP(eth_mdc_h,			6),
	/* Bank[H]:Function[7] */
	GROUP(tst_clk_out22_h,			7),
	GROUP(debug_bus_in0,			7),
	GROUP(debug_bus_in1,			7),
	GROUP(debug_bus_in2,			7),
	GROUP(debug_bus_in3,			7),
	GROUP(debug_bus_in4,			7),
	GROUP(debug_bus_in5,			7),
	GROUP(debug_bus_in6,			7),
	GROUP(debug_bus_in7,			7),
	GROUP(debug_bus_in8,			7),
	GROUP(debug_bus_in9,			7),
	GROUP(debug_bus_in10,			7),
	GROUP(debug_bus_in11,			7),
	GROUP(bcon_8,				7),
	GROUP(debug_bus_in12,			7),
	GROUP(debug_bus_in13,			7),
	GROUP(debug_bus_in14,			7),
	GROUP(debug_bus_in15,			7),
	GROUP(debug_bus_in16,			7),
	GROUP(debug_bus_in17,			7),
	GROUP(debug_bus_in18,			7),
	GROUP(debug_bus_in19,			7),
	GROUP(bcon_0,				7),
	GROUP(bcon_1,				7),
	GROUP(bcon_2,				7),
	GROUP(bcon_3,				7),
	GROUP(bcon_4,				7),
	GROUP(bcon_5,				7),
	GROUP(bcon_6,				7),
	GROUP(bcon_7,				7),
	/* Bank[P]:Function[1] */
	GROUP(spi_miso_c_p,			1),
	GROUP(spi_mosi_c_p,			1),
	GROUP(spi_clk_c_p,			1),
	GROUP(spi_ss0_c_p,			1),
	GROUP(i2c_d_scl_p,			1),
	GROUP(i2c_d_sda_p,			1),
	GROUP(mclk_2_p,				1),
	GROUP(tdm_sclk2_p,			1),
	GROUP(tdm_fs2_p,			1),
	GROUP(tdm_d2_p,				1),
	GROUP(dcon_led_p,			1),
	GROUP(spdif_out_p,			1),
	/* Bank[P]:Function[2] */
	GROUP(uart_c_tx_p,			2),
	GROUP(uart_c_rx_p,			2),
	GROUP(uart_c_cts_p,			2),
	GROUP(uart_c_rts_p,			2),
	GROUP(pwm_i_p,				2),
	GROUP(pwm_j_p,				2),
	GROUP(tdm_d4_p,				2),
	GROUP(ir_in_b_p,			2),
	GROUP(ir_out_p,				2),
	GROUP(tdm_d3_p,				2),
	GROUP(ir_in_b_p10,			2),
	GROUP(ir_out_p11,			2),
	/* Bank[P]:Function[3] */
	GROUP(spdif_out_p0,			3),
	GROUP(pwm_g_p,				3),
	GROUP(pwm_h_p,				3),
	GROUP(spi_dq2_c_p,			3),
	GROUP(spi_dq3_c_p,			3),
	GROUP(pwm_e_p,				3),
	GROUP(pwm_g_p7,				3),
	GROUP(pwm_h_p8,				3),
	GROUP(dcon_led_p9,			3),
	GROUP(gen_clk_out_p,			3),
	/* Bank[P]:Function[4] */
	GROUP(bcon_8_p,				4),
	GROUP(bcon_9,				4),
	GROUP(bcon_10,				4),
	GROUP(bcon_11,				4),
	GROUP(bcon_12,				4),
	GROUP(bcon_13,				4),
	GROUP(bcon_14,				4),
	GROUP(bcon_15,				4),
	GROUP(bcon_16,				4),
	GROUP(bcon_17,				4),
	/* Bank[P]:Function[5] */
	GROUP(spi_miso_d_p,			5),
	GROUP(spi_mosi_d_p,			5),
	GROUP(spi_clk_d_p,			5),
	GROUP(spi_ss0_d_p,			5),
	GROUP(spi_mosi_e_p,			5),
	GROUP(spi_clk_e_p,			5),
	GROUP(spi_ss0_e_p,			5),
	GROUP(spi_mosi_f_p,			5),
	GROUP(spi_clk_f_p,			5),
	GROUP(spi_ss0_f_p,			5),
	GROUP(spi_miso_e,			5),
	GROUP(spi_miso_f_p,			5),
	/* Bank[P]:Function[6] */
	GROUP(tsin_c_clk_p,			6),
	GROUP(tsin_c_sop_p,			6),
	GROUP(tsin_c_valid_p,			6),
	GROUP(tsin_c_d0_p,			6),
	GROUP(tsin_c_d1_p,			6),
	GROUP(tsin_c_d2_p,			6),
	GROUP(tsin_c_d3_p,			6),
	GROUP(tsin_c_d4_p,			6),
	GROUP(tsin_c_d5_p,			6),
	GROUP(tsin_c_d6_p,			6),
	GROUP(tsin_c_d7_p,			6),
	/* Bank[P]:Function[7] */
	GROUP(eth_rgmii_rx_clk_p,		7),
	GROUP(eth_rx_dv_p,			7),
	GROUP(eth_rxd0_p,			7),
	GROUP(eth_rxd1_p,			7),
	GROUP(eth_rxd2_rgmii_p,			7),
	GROUP(eth_rxd3_rgmii_p,			7),
	GROUP(eth_rgmii_tx_clk_p,		7),
	GROUP(eth_txen_p,			7),
	GROUP(eth_txd0_p,			7),
	GROUP(eth_txd1_p,			7),
	GROUP(eth_txd2_rgmii_p,			7),
	GROUP(eth_txd3_rgmii_p,			7),
	/* Bank[M]:Function[1] */
	GROUP(tsin_b_clk,			1),
	GROUP(tsin_b_sop,			1),
	GROUP(tsin_b_valid,			1),
	GROUP(tsin_b_d0,			1),
	GROUP(tsin_b_d1,			1),
	GROUP(tsin_b_d2,			1),
	GROUP(tsin_b_d3,			1),
	GROUP(tsin_b_d4,			1),
	GROUP(tsin_b_d5,			1),
	GROUP(tsin_b_d6,			1),
	GROUP(tsin_b_d7,			1),
	GROUP(cicam_data0,			1),
	GROUP(cicam_data1,			1),
	GROUP(cicam_data2,			1),
	GROUP(cicam_data3,			1),
	GROUP(cicam_data4,			1),
	GROUP(cicam_data5,			1),
	GROUP(cicam_data6,			1),
	GROUP(cicam_data7,			1),
	GROUP(cicam_a0,				1),
	GROUP(cicam_a1,				1),
	GROUP(cicam_cen,			1),
	GROUP(cicam_oen,			1),
	GROUP(cicam_wen,			1),
	GROUP(cicam_iordn,			1),
	GROUP(cicam_iowrn,			1),
	GROUP(cicam_reset,			1),
	GROUP(cicam_waitn_m,			1),
	/* Bank[M]:Function[2] */
	GROUP(spi_miso_c_m,			2),
	GROUP(spi_mosi_c_m,			2),
	GROUP(spi_clk_c_m,			2),
	GROUP(spi_ss0_c_m,			2),
	GROUP(spi_miso_d_m,			2),
	GROUP(spi_mosi_d_m,			2),
	GROUP(spi_clk_d_m,			2),
	GROUP(spi_ss0_d_m,			2),
	GROUP(pdm_din0_m,			2),
	GROUP(pdm_dclk_m,			2),
	GROUP(mclk_2_m,				2),
	GROUP(tdm_fs2_m,			2),
	GROUP(tdm_sclk2_m,			2),
	GROUP(tdm_d4_m,				2),
	GROUP(tdm_d5_m,				2),
	GROUP(tdm_d6_m,				2),
	GROUP(tdm_d7_m,				2),
	GROUP(iso7816_clk_m,			2),
	GROUP(iso7816_data_m,			2),
	GROUP(mclk_1_m,				2),
	GROUP(tdm_fs1_m,			2),
	GROUP(tdm_sclk1_m,			2),
	GROUP(tdm_d0_m,				2),
	GROUP(tdm_d1_m,				2),
	GROUP(tdm_d2_m,				2),
	GROUP(tdm_d3_m,				2),
	/* Bank[M]:Function[3] */
	GROUP(uart_a_tx_m,			3),
	GROUP(uart_a_rx_m,			3),
	GROUP(uart_a_cts_m,			3),
	GROUP(uart_a_rts_m,			3),
	GROUP(pwm_f_m,				3),
	GROUP(pwm_i_m,				3),
	GROUP(pwm_j_m,				3),
	GROUP(i2c_d_scl_m,			3),
	GROUP(i2c_d_sda_m,			3),
	GROUP(spi_miso_a_m,			3),
	GROUP(spi_mosi_a_m,			3),
	GROUP(spi_clk_a_m,			3),
	GROUP(spi_ss0_a_m,			3),
	GROUP(spi_ss1_a_m,			3),
	GROUP(spi_ss2_a_m,			3),
	GROUP(i2c_d_scl_m25,			3),
	GROUP(i2c_d_sda_m26,			3),
	GROUP(i2c_c_scl_m,			3),
	GROUP(i2c_c_sda_m,			3),
	GROUP(spdif_out_m,			3),
	/* Bank[M]:Function[4] */
	GROUP(pwm_d_m,				4),
	GROUP(uart_d_tx_m,			4),
	GROUP(uart_d_rx_m,			4),
	GROUP(pwm_g_m,				4),
	GROUP(pwm_h_m,				4),
	GROUP(uart_d_tx_m17,			4),
	GROUP(uart_d_rx_m18,			4),
	GROUP(uart_c_tx_m,			4),
	GROUP(uart_c_rx_m,			4),
	GROUP(uart_c_cts_m,			4),
	GROUP(uart_c_rts_m,			4),
	GROUP(pwm_d_m23,			4),
	GROUP(pwm_e_m,				4),
	GROUP(pwm_f_m26,			4),
	GROUP(gen_clk_out_m,			4),
	/* Bank[M]:Function[5] */
	GROUP(tsin_a_clk_m,			5),
	GROUP(tsin_a_sop_m,			5),
	GROUP(tsin_a_valid_m,			5),
	GROUP(tsin_a_d0_m,			5),
	GROUP(demod_uart_tx_m,			5),
	GROUP(demod_uart_rx_m,			5),
	GROUP(demod_uart_tx_m25,		5),
	GROUP(demod_uart_rx_m26,		5),
	/* Bank[M]:Function[6] */
	GROUP(tst_clk_out0,			6),
	GROUP(tst_clk_out1,			6),
	GROUP(tst_clk_out2,			6),
	GROUP(tst_clk_out3,			6),
	GROUP(tst_clk_out4_m,			6),
	GROUP(tst_clk_out5,			6),
	GROUP(tst_clk_out6,			6),
	GROUP(tst_clk_out7,			6),
	GROUP(tst_clk_out8,			6),
	GROUP(tst_clk_out9,			6),
	GROUP(tst_clk_out10,			6),
	GROUP(tst_clk_out11,			6),
	GROUP(tst_clk_out12,			6),
	GROUP(tst_clk_out13,			6),
	GROUP(tst_clk_out14,			6),
	GROUP(tst_clk_out15,			6),
	GROUP(tst_clk_out16,			6),
	GROUP(tst_clk_out17,			6),
	GROUP(tst_clk_out18,			6),
	GROUP(tst_clk_out19,			6),
	GROUP(tst_clk_out20,			6),
	GROUP(tst_clk_out21,			6),
	GROUP(tst_clk_out22_m,			6),
	GROUP(tst_clk_out23,			6),
	GROUP(tst_clk_out24,			6),
	GROUP(tst_clk_out25,			6),
	GROUP(ir_in_b_m,			6),
	/* Bank[M]:Function[7] */
	GROUP(debug_bus_out0,			7),
	GROUP(debug_bus_out1,			7),
	GROUP(debug_bus_out2,			7),
	GROUP(debug_bus_out3,			7),
	GROUP(debug_bus_out4,			7),
	GROUP(debug_bus_out5,			7),
	GROUP(debug_bus_out6,			7),
	GROUP(debug_bus_out7,			7),
	GROUP(debug_bus_out8,			7),
	GROUP(debug_bus_out9,			7),
	GROUP(debug_bus_out10,			7),
	GROUP(debug_bus_out11,			7),
	GROUP(debug_bus_out12,			7),
	GROUP(debug_bus_out13,			7),
	GROUP(debug_bus_out14,			7),
	GROUP(debug_bus_out15,			7),
	GROUP(debug_bus_out16,			7),
	GROUP(debug_bus_out17,			7),
	GROUP(debug_bus_out18,			7),
	GROUP(debug_bus_out19,			7),
	GROUP(tst_clk_out22_m28,		7)
};

static const char * const gpio_periphs_groups[] = {
	"GPIOB_0",  "GPIOB_1",  "GPIOB_2",  "GPIOB_3",  "GPIOB_4",
	"GPIOB_5",  "GPIOB_6",  "GPIOB_7",  "GPIOB_8",  "GPIOB_9",
	"GPIOC_0",  "GPIOC_1",  "GPIOC_2",  "GPIOC_3",  "GPIOC_4",
	"GPIOC_5",  "GPIOC_6",  "GPIOC_7",  "GPIOC_8",  "GPIOC_9",
	"GPIOD_0",  "GPIOD_1",  "GPIOD_2",  "GPIOD_3",  "GPIOD_4",
	"GPIOD_5",  "GPIOD_6",  "GPIOD_7",  "GPIOD_8",  "GPIOD_9",
	"GPIOE_0",  "GPIOE_1",  "GPIOH_0",  "GPIOH_1",  "GPIOH_2",
	"GPIOH_3",  "GPIOH_4",  "GPIOH_5",  "GPIOH_6",  "GPIOH_7",
	"GPIOH_8",  "GPIOH_9",  "GPIOM_0",  "GPIOM_1",  "GPIOM_2",
	"GPIOM_3",  "GPIOM_4",  "GPIOM_5",  "GPIOM_6",  "GPIOM_7",
	"GPIOM_8",  "GPIOM_9",  "GPIOP_0",  "GPIOP_1",  "GPIOP_2",
	"GPIOP_3",  "GPIOP_4",  "GPIOP_5",  "GPIOP_6",  "GPIOP_7",
	"GPIOP_8",  "GPIOP_9",  "GPIOW_0",  "GPIOW_1",  "GPIOW_2",
	"GPIOW_3",  "GPIOW_4",  "GPIOW_5",  "GPIOW_6",  "GPIOW_7",
	"GPIOW_8",  "GPIOW_9",  "GPIOZ_0",  "GPIOZ_1",  "GPIOZ_2",
	"GPIOZ_3",  "GPIOZ_4",  "GPIOZ_5",  "GPIOZ_6",  "GPIOZ_7",
	"GPIOZ_8",  "GPIOZ_9",  "GPIOB_10", "GPIOB_11", "GPIOB_12",
	"GPIOB_13", "GPIOC_10", "GPIOD_10", "GPIOD_11", "GPIOD_12",
	"GPIOD_13", "GPIOD_14", "GPIOH_10", "GPIOH_11", "GPIOH_12",
	"GPIOH_13", "GPIOH_14", "GPIOH_15", "GPIOH_16", "GPIOH_17",
	"GPIOH_18", "GPIOH_19", "GPIOH_20", "GPIOH_21", "GPIOH_22",
	"GPIOH_23", "GPIOH_24", "GPIOH_25", "GPIOH_26", "GPIOH_27",
	"GPIOH_28", "GPIOH_29", "GPIOM_10", "GPIOM_11", "GPIOM_12",
	"GPIOM_13", "GPIOM_14", "GPIOM_15", "GPIOM_16", "GPIOM_17",
	"GPIOM_18", "GPIOM_19", "GPIOM_20", "GPIOM_21", "GPIOM_22",
	"GPIOM_23", "GPIOM_24", "GPIOM_25", "GPIOM_26", "GPIOM_27",
	"GPIOM_28", "GPIOM_29", "GPIOP_10", "GPIOP_11", "GPIOW_10",
	"GPIOW_11", "GPIOW_12", "GPIOW_13", "GPIOW_14", "GPIOW_15",
	"GPIOW_16", "GPIOZ_10", "GPIOZ_11", "GPIOZ_12", "GPIOZ_13",
	"GPIOZ_14", "GPIOZ_15", "GPIOZ_16", "GPIOZ_17", "GPIOZ_18",
	"GPIOZ_19", "GPIO_TEST_N"
};

static const char * const atv_groups[] = {
	"atv_if_agc", "atv_if_agc_z", "atv_if_agc_z18"
};

static const char * const bcon_groups[] = {
	"bcon_0",  "bcon_1",  "bcon_2",  "bcon_3",   "bcon_4",
	"bcon_5",  "bcon_6",  "bcon_7",  "bcon_8",   "bcon_9",
	"bcon_10", "bcon_11", "bcon_12", "bcon_13",  "bcon_14",
	"bcon_15", "bcon_16", "bcon_17", "bcon_8_p"
};

static const char * const cec_groups[] = {
	"cec"
};

static const char * const cicam_groups[] = {
	"cicam_a0",    "cicam_a1",      "cicam_a2",    "cicam_a3",
	"cicam_a4",    "cicam_a5",      "cicam_a6",    "cicam_a7",
	"cicam_a8",    "cicam_a9",      "cicam_a10",   "cicam_a11",
	"cicam_a12",   "cicam_a13",     "cicam_a14",   "cicam_cen",
	"cicam_oen",   "cicam_wen",     "cicam_a2_z",  "cicam_a3_z",
	"cicam_a4_z",  "cicam_a5_z",    "cicam_a6_z",  "cicam_a7_z",
	"cicam_a8_z",  "cicam_a9_z",    "cicam_a10_z", "cicam_a11_z",
	"cicam_a12_z", "cicam_data0",   "cicam_data1", "cicam_data2",
	"cicam_data3", "cicam_data4",   "cicam_data5", "cicam_data6",
	"cicam_data7", "cicam_iordn",   "cicam_iowrn", "cicam_reset",
	"cicam_waitn", "cicam_waitn_m"
};

static const char * const clk_groups[] = {
	"clk12_24m", "clk_32k_in", "clk12_24m_d", "clk12_24m_h"
};

static const char * const dcon_groups[] = {
	"dcon_led",   "dcon_led_c", "dcon_led_d",  "dcon_led_h",
	"dcon_led_p", "dcon_led_z", "dcon_led_d9", "dcon_led_p9"
};

static const char * const debug_groups[] = {
	"debug_bus_in0",   "debug_bus_in1",   "debug_bus_in2",
	"debug_bus_in3",   "debug_bus_in4",   "debug_bus_in5",
	"debug_bus_in6",   "debug_bus_in7",   "debug_bus_in8",
	"debug_bus_in9",   "debug_bus_in10",  "debug_bus_in11",
	"debug_bus_in12",  "debug_bus_in13",  "debug_bus_in14",
	"debug_bus_in15",  "debug_bus_in16",  "debug_bus_in17",
	"debug_bus_in18",  "debug_bus_in19",  "debug_bus_out0",
	"debug_bus_out1",  "debug_bus_out2",  "debug_bus_out3",
	"debug_bus_out4",  "debug_bus_out5",  "debug_bus_out6",
	"debug_bus_out7",  "debug_bus_out8",  "debug_bus_out9",
	"debug_bus_out10", "debug_bus_out11", "debug_bus_out12",
	"debug_bus_out13", "debug_bus_out14", "debug_bus_out15",
	"debug_bus_out16", "debug_bus_out17", "debug_bus_out18",
	"debug_bus_out19"
};

static const char * const demod_groups[] = {
	"demod_uart_rx",     "demod_uart_tx",   "demod_uart_rx_c",
	"demod_uart_rx_h",   "demod_uart_rx_m", "demod_uart_tx_c",
	"demod_uart_tx_h",   "demod_uart_tx_m", "demod_uart_rx_m26",
	"demod_uart_tx_m25"
};

static const char * const diseqc_groups[] = {
	"diseqc_out", "diseqc_out_z"
};

static const char * const dtv_groups[] = {
	"dtv_if_agc",     "dtv_rf_agc", "dtv_if_agc_z", "dtv_rf_agc_z",
	"dtv_if_agc_z18"
};

static const char * const emmc_groups[] = {
	"emmc_d0",  "emmc_d1", "emmc_d2", "emmc_d3", "emmc_d4",
	"emmc_d5",  "emmc_d6", "emmc_d7", "emmc_ds", "emmc_clk",
	"emmc_cmd"
};

static const char * const eth_groups[] = {
	"eth_mdc",          "eth_mdio",           "eth_rxd0",
	"eth_rxd1",         "eth_txd0",           "eth_txd1",
	"eth_txen",         "eth_mdc_h",          "eth_mdc_w",
	"eth_mdc_z",        "eth_rx_dv",          "eth_mdio_h",
	"eth_mdio_w",       "eth_mdio_z",         "eth_rxd0_p",
	"eth_rxd1_p",       "eth_txd0_p",         "eth_txd1_p",
	"eth_txen_p",       "eth_act_led",        "eth_rx_dv_p",
	"eth_link_led",     "eth_act_led_h",      "eth_link_led_h",
	"eth_rxd2_rgmii",   "eth_rxd3_rgmii",     "eth_txd2_rgmii",
	"eth_txd3_rgmii",   "eth_rgmii_rx_clk",   "eth_rgmii_tx_clk",
	"eth_rxd2_rgmii_p", "eth_rxd3_rgmii_p",   "eth_txd2_rgmii_p",
	"eth_txd3_rgmii_p", "eth_rgmii_rx_clk_p", "eth_rgmii_tx_clk_p"
};

static const char * const gen_clk_groups[] = {
	"gen_clk_out",   "gen_clk_out_b", "gen_clk_out_c", "gen_clk_out_d",
	"gen_clk_out_h", "gen_clk_out_m", "gen_clk_out_p"
};

static const char * const loop_groups[] = {
	"gpiow2_loop",  "gpiow3_loop",  "gpiob11_loop",   "gpiob12_loop",
	"gpiob13_loop", "gpioh21_loop", "gpioh21_loop_b"
};

static const char * const hdmirx_groups[] = {
	"hdmirx_hpd_a",   "hdmirx_hpd_b",   "hdmirx_hpd_c",   "hdmirx_hpd_d",
	"hdmirx_scl_a",   "hdmirx_scl_b",   "hdmirx_scl_c",   "hdmirx_scl_d",
	"hdmirx_sda_a",   "hdmirx_sda_b",   "hdmirx_sda_c",   "hdmirx_sda_d",
	"hdmirx_5vdet_a", "hdmirx_5vdet_b", "hdmirx_5vdet_c", "hdmirx_5vdet_d"
};

static const char * const sync_3d_groups[] = {
	"sync_3d_out"
};

static const char * const sync_groups[] = {
	"hsync", "vsync"
};

static const char * const i2c_a_groups[] = {
	"i2c_a_scl", "i2c_a_sda"
};

static const char * const i2c_b_groups[] = {
	"i2c_b_scl",   "i2c_b_sda",   "i2c_b_scl_d", "i2c_b_scl_h",
	"i2c_b_sda_d", "i2c_b_sda_h"
};

static const char * const i2c_c_groups[] = {
	"i2c_c_scl",   "i2c_c_sda",   "i2c_c_scl_e", "i2c_c_scl_h",
	"i2c_c_scl_m", "i2c_c_sda_e", "i2c_c_sda_h", "i2c_c_sda_m"
};

static const char * const i2c_d_groups[] = {
	"i2c_d_scl",     "i2c_d_sda",     "i2c_d_scl_h",   "i2c_d_scl_m",
	"i2c_d_scl_p",   "i2c_d_sda_h",   "i2c_d_sda_m",   "i2c_d_sda_p",
	"i2c_d_scl_h20", "i2c_d_scl_m25", "i2c_d_sda_h21", "i2c_d_sda_m26"
};

static const char * const i2c_tcon_groups[] = {
	"i2c_tcon_scl",     "i2c_tcon_sda",     "i2c_tcon_scl_h",
	"i2c_tcon_sda_h",   "i2c_tcon_scl_h2",  "i2c_tcon_sda_h3",
	"i2c_tcon_scl_h10", "i2c_tcon_sda_h11"
};

static const char * const ir_in_groups[] = {
	"ir_in_a",   "ir_in_b",   "ir_in_b_d",   "ir_in_b_m",
	"ir_in_b_p", "ir_in_b_z", "ir_in_b_p10"
};

static const char * const ir_out_groups[] = {
	"ir_out",     "ir_out_b", "ir_out_d", "ir_out_p",
	"ir_out_p11"
};

static const char * const iso7816_groups[] = {
	"iso7816_clk", "iso7816_data", "iso7816_clk_m", "iso7816_data_m"
};

static const char * const jtag_a_groups[] = {
	"jtag_a_clk",    "jtag_a_tdi",    "jtag_a_tdo",     "jtag_a_tms",
	"jtag_a_clk_d",  "jtag_a_clk_w",  "jtag_a_tdi_d",   "jtag_a_tdi_w",
	"jtag_a_tdo_d",  "jtag_a_tdo_w",  "jtag_a_tms_d",   "jtag_a_tms_w",
	"jtag_a_clk_w6", "jtag_a_tms_w7", "jtag_a_tdi_w10", "jtag_a_tdo_w11"
};

static const char * const mclk_groups[] = {
	"mclk_1",   "mclk_2",   "mclk_1_c", "mclk_1_h", "mclk_1_m",
	"mclk_2_m", "mclk_2_p", "mclk_2_z"
};

static const char * const nand_groups[] = {
	"nand_ale",     "nand_ce0", "nand_cle", "nand_ren_wr",
	"nand_wen_clk"
};

static const char * const pdm_groups[] = {
	"pdm_dclk",     "pdm_din0",     "pdm_din1",     "pdm_dclk_h",
	"pdm_dclk_m",   "pdm_din0_h",   "pdm_din0_m",   "pdm_din1_h",
	"pdm_dclk_h22", "pdm_dclk_h29", "pdm_din0_h26", "pdm_din0_h28",
	"pdm_din1_h23", "pdm_din1_h27"
};

static const char * const pwm_a_groups[] = {
	"pwm_a", "pwm_a_e"
};

static const char * const pwm_b_groups[] = {
	"pwm_b", "pwm_b_z"
};

static const char * const pwm_c_groups[] = {
	"pwm_c", "pwm_c_d", "pwm_c_d11", "pwm_c_hiz"
};

static const char * const pwm_d_groups[] = {
	"pwm_d",     "pwm_d_h",   "pwm_d_m", "pwm_d_z", "pwm_d_h12",
	"pwm_d_hiz", "pwm_d_m23"
};

static const char * const pwm_e_groups[] = {
	"pwm_e",     "pwm_e_h", "pwm_e_m", "pwm_e_p", "pwm_e_z",
	"pwm_e_h13"
};

static const char * const pwm_f_groups[] = {
	"pwm_f", "pwm_f_c", "pwm_f_m", "pwm_f_m26"
};

static const char * const pwm_g_groups[] = {
	"pwm_g",    "pwm_g_h", "pwm_g_m", "pwm_g_p", "pwm_g_z",
	"pwm_g_p7"
};

static const char * const pwm_h_groups[] = {
	"pwm_h", "pwm_h_m", "pwm_h_p", "pwm_h_z", "pwm_h_p8"
};

static const char * const pwm_i_groups[] = {
	"pwm_i", "pwm_i_m", "pwm_i_p", "pwm_i_z"
};

static const char * const pwm_j_groups[] = {
	"pwm_j", "pwm_j_c", "pwm_j_m", "pwm_j_p", "pwm_j_z"
};

static const char * const pwm_vs_groups[] = {
	"pwm_vs", "pwm_vs_c", "pwm_vs_h", "pwm_vs_h13"
};

static const char * const s2_demod_groups[] = {
	"s2_demod_gpio0", "s2_demod_gpio1", "s2_demod_gpio2",
	"s2_demod_gpio3", "s2_demod_gpio4", "s2_demod_gpio5",
	"s2_demod_gpio6", "s2_demod_gpio7", "s2_demod_gpio0_h"
};

static const char * const spdif_groups[] = {
	"spdif_out",   "spdif_out_c", "spdif_out_d",  "spdif_out_m",
	"spdif_out_p", "spdif_out_z", "spdif_out_p0", "spdif_out_z2"
};

static const char * const spi_groups[] = {
	"spi_clk_a",    "spi_clk_b",     "spi_clk_c",    "spi_clk_d",
	"spi_clk_e",    "spi_clk_f",     "spi_dq2_b",    "spi_dq2_c",
	"spi_dq3_b",    "spi_dq3_c",     "spi_ss0_a",    "spi_ss0_b",
	"spi_ss0_c",    "spi_ss0_d",     "spi_ss0_e",    "spi_ss0_f",
	"spi_ss1_a",    "spi_ss1_c",     "spi_ss2_a",    "spi_ss2_c",
	"spi_miso_a",   "spi_miso_b",    "spi_miso_c",   "spi_miso_d",
	"spi_miso_e",   "spi_miso_f",    "spi_mosi_a",   "spi_mosi_b",
	"spi_mosi_c",   "spi_mosi_d",    "spi_mosi_e",   "spi_mosi_f",
	"spi_clk_a_h",  "spi_clk_a_m",   "spi_clk_b_h",  "spi_clk_c_h",
	"spi_clk_c_m",  "spi_clk_c_p",   "spi_clk_d_m",  "spi_clk_d_p",
	"spi_clk_e_p",  "spi_clk_f_p",   "spi_dq2_c_p",  "spi_dq3_c_p",
	"spi_ss0_a_m",  "spi_ss0_b_h",   "spi_ss0_c_h",  "spi_ss0_c_m",
	"spi_ss0_c_p",  "spi_ss0_d_m",   "spi_ss0_d_p",  "spi_ss0_e_p",
	"spi_ss0_f_p",  "spi_ss1_a_h",   "spi_ss1_a_m",  "spi_ss2_a_m",
	"spi_miso_a_m", "spi_miso_b_h",  "spi_miso_c_h", "spi_miso_c_m",
	"spi_miso_c_p", "spi_miso_d_m",  "spi_miso_d_p", "spi_miso_f_p",
	"spi_mosi_a_m", "spi_mosi_b_h",  "spi_mosi_c_h", "spi_mosi_c_m",
	"spi_mosi_c_p", "spi_mosi_d_m",  "spi_mosi_d_p", "spi_mosi_e_p",
	"spi_mosi_f_p", "spi_ss1_a_h26"
};

static const char * const spinf_groups[] = {
	"spinf_d4",    "spinf_d5",    "spinf_d6",      "spinf_d7",
	"spinf_clk",   "spinf_cs0",   "spinf_cs1",     "spinf_mi_d1",
	"spinf_mo_d0", "spinf_wp_d2", "spinf_hold_d3"
};

static const char * const tcon_groups[] = {
	"tcon_0",   "tcon_1",    "tcon_2",     "tcon_3",
	"tcon_4",   "tcon_5",    "tcon_6",     "tcon_7",
	"tcon_8",   "tcon_9",    "tcon_10",    "tcon_11",
	"tcon_12",  "tcon_13",   "tcon_14",    "tcon_15",
	"tcon_sfc", "tcon_lock", "tcon_sfc_h", "tcon_lock_h"
};

static const char * const tdm_groups[] = {
	"tdm",           "tdm_d0",      "tdm_d1",      "tdm_d2",
	"tdm_d3",        "tdm_d4",      "tdm_d5",      "tdm_d6",
	"tdm_d7",        "tdm_fs1",     "tdm_fs2",     "tdm_d0_h",
	"tdm_d0_m",      "tdm_d1_h",    "tdm_d1_m",    "tdm_d2_h",
	"tdm_d2_m",      "tdm_d2_p",    "tdm_d3_h",    "tdm_d3_m",
	"tdm_d3_p",      "tdm_d4_h",    "tdm_d4_m",    "tdm_d4_p",
	"tdm_d4_z",      "tdm_d5_h",    "tdm_d5_m",    "tdm_d5_z",
	"tdm_d6_h",      "tdm_d6_m",    "tdm_d6_z",    "tdm_d7_m",
	"tdm_d7_z",      "tdm_fs1_h",   "tdm_fs1_m",   "tdm_fs1_z",
	"tdm_fs2_h",     "tdm_fs2_m",   "tdm_fs2_p",   "tdm_fs2_z",
	"tdm_sclk1",     "tdm_sclk2",   "tdm_d2_h26",  "tdm_d3_h13",
	"tdm_d3_h14",    "tdm_d3_h20",  "tdm_d3_h27",  "tdm_fs1_h18",
	"tdm_fs2_h23",   "tdm_sclk1_h", "tdm_sclk1_m", "tdm_sclk1_z",
	"tdm_sclk2_h",   "tdm_sclk2_m", "tdm_sclk2_p", "tdm_sclk2_z",
	"tdm_sclk2_h22"
};

static const char * const tsin_a_groups[] = {
	"tsin_a_d0",    "tsin_a_clk",   "tsin_a_sop",   "tsin_a_d0_m",
	"tsin_a_clk_m", "tsin_a_sop_m", "tsin_a_valid", "tsin_a_valid_m"
};

static const char * const tsin_b_groups[] = {
	"tsin_b_d0",  "tsin_b_d1",  "tsin_b_d2",    "tsin_b_d3",
	"tsin_b_d4",  "tsin_b_d5",  "tsin_b_d6",    "tsin_b_d7",
	"tsin_b_clk", "tsin_b_sop", "tsin_b_valid"
};

static const char * const tsin_c_groups[] = {
	"tsin_c_d0",      "tsin_c_d1",      "tsin_c_d2",    "tsin_c_d3",
	"tsin_c_d4",      "tsin_c_d5",      "tsin_c_d6",    "tsin_c_d7",
	"tsin_c_clk",     "tsin_c_sop",     "tsin_c_d0_p",  "tsin_c_d0_z",
	"tsin_c_d1_p",    "tsin_c_d2_p",    "tsin_c_d3_p",  "tsin_c_d4_p",
	"tsin_c_d5_p",    "tsin_c_d6_p",    "tsin_c_d7_p",  "tsin_c_clk_p",
	"tsin_c_clk_z",   "tsin_c_sop_p",   "tsin_c_sop_z", "tsin_c_valid",
	"tsin_c_valid_p", "tsin_c_valid_z"
};

static const char * const tsout_groups[] = {
	"tsout_d0",  "tsout_d1",  "tsout_d2",    "tsout_d3",
	"tsout_d4",  "tsout_d5",  "tsout_d6",    "tsout_d7",
	"tsout_clk", "tsout_sop", "tsout_valid"
};

static const char * const tst_clk_groups[] = {
	"tst_clk_out0",    "tst_clk_out1",      "tst_clk_out2",
	"tst_clk_out3",    "tst_clk_out4",      "tst_clk_out5",
	"tst_clk_out6",    "tst_clk_out7",      "tst_clk_out8",
	"tst_clk_out9",    "tst_clk_out10",     "tst_clk_out11",
	"tst_clk_out12",   "tst_clk_out13",     "tst_clk_out14",
	"tst_clk_out15",   "tst_clk_out16",     "tst_clk_out17",
	"tst_clk_out18",   "tst_clk_out19",     "tst_clk_out20",
	"tst_clk_out21",   "tst_clk_out22",     "tst_clk_out23",
	"tst_clk_out24",   "tst_clk_out25",     "tst_clk_out4_m",
	"tst_clk_out4_z",  "tst_clk_out22_h",   "tst_clk_out22_m",
	"tst_clk_out22_w", "tst_clk_out22_m28", "tst_clk_out22_w16"
};

static const char * const uart_a_groups[] = {
	"uart_a_rx",    "uart_a_tx",    "uart_a_cts",   "uart_a_rts",
	"uart_a_rx_m",  "uart_a_rx_z",  "uart_a_tx_m",  "uart_a_tx_z",
	"uart_a_cts_m", "uart_a_cts_z", "uart_a_rts_m", "uart_a_rts_z"
};

static const char * const uart_b_groups[] = {
	"uart_b_rx",     "uart_b_tx",     "uart_b_rx_d",   "uart_b_rx_w",
	"uart_b_tx_d",   "uart_b_tx_w",   "uart_b_rx_w11", "uart_b_rx_w15",
	"uart_b_tx_w10", "uart_b_tx_w14"
};

static const char * const uart_c_groups[] = {
	"uart_c_rx",      "uart_c_tx",      "uart_c_cts",    "uart_c_rts",
	"uart_c_rx_h",    "uart_c_rx_m",    "uart_c_rx_p",   "uart_c_tx_h",
	"uart_c_tx_m",    "uart_c_tx_p",    "uart_c_cts_h",  "uart_c_cts_m",
	"uart_c_cts_p",   "uart_c_rts_h",   "uart_c_rts_m",  "uart_c_rts_p",
	"uart_c_rx_h25",  "uart_c_rx_h29",  "uart_c_tx_h24", "uart_c_tx_h28",
	"uart_c_cts_h26", "uart_c_rts_h27"
};

static const char * const uart_d_groups[] = {
	"uart_d_rx",   "uart_d_tx",   "uart_d_rx_m",   "uart_d_rx_z",
	"uart_d_tx_m", "uart_d_tx_z", "uart_d_rx_m18", "uart_d_tx_m17"
};

static const char * const vx1_a_groups[] = {
	"vx1_a_htpdn", "vx1_a_lockn"
};

static const char * const vx1_b_groups[] = {
	"vx1_b_htpdn", "vx1_b_lockn"
};

static struct meson_pmx_func meson_t6x_periphs_functions[] __initdata = {
	FUNCTION(gpio_periphs),
	FUNCTION(atv),
	FUNCTION(bcon),
	FUNCTION(cec),
	FUNCTION(cicam),
	FUNCTION(clk),
	FUNCTION(dcon),
	FUNCTION(debug),
	FUNCTION(demod),
	FUNCTION(diseqc),
	FUNCTION(dtv),
	FUNCTION(emmc),
	FUNCTION(eth),
	FUNCTION(gen_clk),
	FUNCTION(loop),
	FUNCTION(hdmirx),
	FUNCTION(sync_3d),
	FUNCTION(sync),
	FUNCTION(i2c_a),
	FUNCTION(i2c_b),
	FUNCTION(i2c_c),
	FUNCTION(i2c_d),
	FUNCTION(i2c_tcon),
	FUNCTION(ir_in),
	FUNCTION(ir_out),
	FUNCTION(iso7816),
	FUNCTION(jtag_a),
	FUNCTION(mclk),
	FUNCTION(nand),
	FUNCTION(pdm),
	FUNCTION(pwm_a),
	FUNCTION(pwm_b),
	FUNCTION(pwm_c),
	FUNCTION(pwm_d),
	FUNCTION(pwm_e),
	FUNCTION(pwm_f),
	FUNCTION(pwm_g),
	FUNCTION(pwm_h),
	FUNCTION(pwm_i),
	FUNCTION(pwm_j),
	FUNCTION(pwm_vs),
	FUNCTION(s2_demod),
	FUNCTION(spdif),
	FUNCTION(spi),
	FUNCTION(spinf),
	FUNCTION(tcon),
	FUNCTION(tdm),
	FUNCTION(tsin_a),
	FUNCTION(tsin_b),
	FUNCTION(tsin_c),
	FUNCTION(tsout),
	FUNCTION(tst_clk),
	FUNCTION(uart_a),
	FUNCTION(uart_b),
	FUNCTION(uart_c),
	FUNCTION(uart_d),
	FUNCTION(vx1_a),
	FUNCTION(vx1_b)
};

static struct meson_bank meson_t6x_periphs_banks[] = {
	/*    name   first   last   irq   pullen   pull   dir   out   in   ds */
	BANK_DS("W",    GPIOW_0,  GPIOW_16,  25,  41,
		0x063,  0, 0x064,  0, 0x062,  0, 0x061,  0, 0x060,  0, 0x067,  0),
	BANK_DS("D",    GPIOD_0,  GPIOD_14,  42,  56,
		0x003,  0, 0x004,  0, 0x002,  0, 0x001,  0, 0x000,  0, 0x007,  0),
	BANK_DS("E",    GPIOE_0,   GPIOE_1,  57,  58,
		0x00b,  0, 0x00c,  0, 0x00a,  0, 0x009,  0, 0x008,  0, 0x00f,  0),
	BANK_DS("B",    GPIOB_0,  GPIOB_13,   0,  13,
		0x02b,  0, 0x02c,  0, 0x02a,  0, 0x029,  0, 0x028,  0, 0x02f,  0),
	BANK_DS("C",    GPIOC_0,  GPIOC_10,  14,  24,
		0x033,  0, 0x034,  0, 0x032,  0, 0x031,  0, 0x030,  0, 0x037,  0),
	BANK_DS("Z",    GPIOZ_0,  GPIOZ_15,  59,  74,
		0x013,  0, 0x014,  0, 0x012,  0, 0x011,  0, 0x010,  0, 0x017,  0),
	BANK_DS("Z1",   GPIOZ_16, GPIOZ_19,  75,  78,
		0x013, 16, 0x014, 16, 0x012, 16, 0x011, 16, 0x010, 16, 0x0c0,  0),
	BANK_DS("H",    GPIOH_0,  GPIOH_15, 121, 136,
		0x01b,  0, 0x01c,  0, 0x01a,  0, 0x019,  0, 0x018,  0, 0x01f,  0),
	BANK_DS("H1",  GPIOH_16,  GPIOH_29, 137, 150,
		0x01b, 16, 0x01c, 16, 0x01a, 16, 0x019, 16, 0x018, 16, 0x0c1,  0),
	BANK_DS("P",    GPIOP_0,  GPIOP_11, 109, 120,
		0x03b,  0, 0x03c,  0, 0x03a,  0, 0x039,  0, 0x038,  0, 0x03f,  0),
	BANK_DS("M",    GPIOM_0,  GPIOM_15,  79,  94,
		0x073,  0, 0x074,  0, 0x072,  0, 0x071,  0, 0x070,  0, 0x077,  0),
	BANK_DS("M1",  GPIOM_16,  GPIOM_29,  95, 108,
		0x073, 16, 0x074, 16, 0x072, 16, 0x071, 16, 0x070, 16, 0x0c2,  0),
	BANK_DS("TEST_N", GPIO_TEST_N, GPIO_TEST_N, 151, 151,
		0x083,  0, 0x084,  0, 0x082,  0, 0x081,  0, 0x080,  0, 0x087,  0)
};

static struct meson_pmx_bank meson_t6x_periphs_pmx_banks[] = {
	/*       name      first        last         reg   offset */
	BANK_PMX("W",      GPIOW_0,    GPIOW_16,     0x012,  0),
	BANK_PMX("D",      GPIOD_0,    GPIOD_14,     0x00b,  0),
	BANK_PMX("E",      GPIOE_0,     GPIOE_1,     0x00d,  0),
	BANK_PMX("B",      GPIOB_0,    GPIOB_13,     0x000,  0),
	BANK_PMX("C",      GPIOC_0,    GPIOC_10,     0x002,  0),
	BANK_PMX("Z",      GPIOZ_0,    GPIOZ_19,     0x004,  0),
	BANK_PMX("H",      GPIOH_0,    GPIOH_29,     0x007,  0),
	BANK_PMX("P",      GPIOP_0,    GPIOP_11,     0x014, 12),
	BANK_PMX("M",      GPIOM_0,    GPIOM_29,     0x00e,  0),
	BANK_PMX("TEST_N", GPIO_TEST_N, GPIO_TEST_N, 0x00c, 28)
};

static struct meson_axg_pmx_data meson_t6x_periphs_pmx_banks_data = {
	.pmx_banks	= meson_t6x_periphs_pmx_banks,
	.num_pmx_banks	= ARRAY_SIZE(meson_t6x_periphs_pmx_banks),
};

static struct meson_pinctrl_data meson_t6x_periphs_pinctrl_data __refdata = {
	.name		= "periphs-banks",
	.pins		= meson_t6x_periphs_pins,
	.groups		= meson_t6x_periphs_groups,
	.funcs		= meson_t6x_periphs_functions,
	.banks		= meson_t6x_periphs_banks,
	.num_pins	= ARRAY_SIZE(meson_t6x_periphs_pins),
	.num_groups	= ARRAY_SIZE(meson_t6x_periphs_groups),
	.num_funcs	= ARRAY_SIZE(meson_t6x_periphs_functions),
	.num_banks	= ARRAY_SIZE(meson_t6x_periphs_banks),
	.pmx_ops	= &meson_axg_pmx_ops,
	.pmx_data	= &meson_t6x_periphs_pmx_banks_data,
	.parse_dt	= &meson_a1_parse_dt_extra,
};

static const struct of_device_id meson_t6x_pinctrl_dt_match[] = {
	{
		.compatible = "amlogic,meson-t6x-periphs-pinctrl",
		.data = &meson_t6x_periphs_pinctrl_data,
	},
	{
		.compatible = "amlogic,meson-t6x-analog-pinctrl",
		.data = &meson_t6x_analog_pinctrl_data,
	},
	{ },
};

static struct platform_driver meson_t6x_pinctrl_driver = {
	.probe  = meson_pinctrl_probe,
	.driver = {
		.name = "meson-t6x-pinctrl",
	},
};

#ifndef MODULE
static int __init t6x_pmx_init(void)
{
	meson_t6x_pinctrl_driver.driver.of_match_table =
			meson_t6x_pinctrl_dt_match;
	return platform_driver_register(&meson_t6x_pinctrl_driver);
}
arch_initcall(t6x_pmx_init);
#else
int __init meson_t6x_pinctrl_init(void)
{
	meson_t6x_pinctrl_driver.driver.of_match_table =
			meson_t6x_pinctrl_dt_match;
	return platform_driver_register(&meson_t6x_pinctrl_driver);
}
module_init(meson_t6x_pinctrl_init);
#endif

MODULE_LICENSE("GPL v2");
