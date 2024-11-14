// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/amlogic/aml_gpio_consumer.h>
#include <linux/amlogic/media/vout/lcd/aml_ldim.h>
#include <linux/amlogic/media/vout/lcd/aml_bl.h>
#include <linux/amlogic/media/vout/lcd/lcd_unifykey.h>
#include <linux/amlogic/aml_spi.h>
#include "ldim_drv.h"
#include "ldim_dev_drv.h"
#include "../../lcd_reg.h"
#include "../../lcd_common.h"
#include "../lcd_bl.h"
#include "ldim_reg.h"

static struct spicc_controller_data ldim_spi_controller_data = {
	.use_ctrl_cs = 0,
	.use_dirspi = 1,
	.ccxfer_en = 0,
	.timing_en = 1,
	.tx_tuning = 0,
	.rx_tuning = 7,
	.dummy_ctl = 0,
	.ss_leading_gap = 2,
	.ss_trailing_gap = 2,
};

static struct spi_board_info ldim_spi_info = {
	.modalias = "ldim_dev",
	.mode = SPI_MODE_0,
	.max_speed_hz = 1000000, /* 1MHz */
	.bus_num = 0, /* SPI bus No. */
	.chip_select = 0, /* the cs pin index on the spi bus */
	.controller_data = &ldim_spi_controller_data,
};

static int ldim_dev_init_table_save(struct ldim_dev_driver_s *dev_drv, int flag,
				    unsigned char *table)
{
	if (!dev_drv || !table) {
		LDIMERR("%s: resource error\n", __func__);
		return -1;
	}

	if (flag) {
		kfree(dev_drv->init_on);
		dev_drv->init_on = kzalloc(dev_drv->init_on_cnt, GFP_KERNEL);
		if (!dev_drv->init_on) {
			LDIMERR("%s: Not enough memory\n", __func__);
			return -1;
		}
		memcpy(dev_drv->init_on, table, dev_drv->init_on_cnt);
	} else {
		kfree(dev_drv->init_off);
		dev_drv->init_off = kzalloc(dev_drv->init_off_cnt, GFP_KERNEL);
		if (!dev_drv->init_off) {
			LDIMERR("%s: Not enough memory\n", __func__);
			return -1;
		}
		memcpy(dev_drv->init_off, table, dev_drv->init_off_cnt);
	}

	return 0;
}

static int ldim_dev_init_dynamic_load_array(struct ldim_dev_driver_s *dev_drv,
					    unsigned int *buf, int max_len, int flag)
{
	unsigned char type, size = 0;
	int i = 0, j, tbl_max, step = 0, ret = 0;
	unsigned char *table;
	char propname[20];

	if (flag) {
		dev_drv->init_on_cnt = 0;
		tbl_max = LDIM_INIT_ON_MAX;
		sprintf(propname, "init_on");
	} else {
		dev_drv->init_off_cnt = 0;
		tbl_max = LDIM_INIT_OFF_MAX;
		sprintf(propname, "init_off");
	}
	if (max_len == 0)
		return 0;

	table = kzalloc(tbl_max, GFP_KERNEL);
	if (!table) {
		LDIMERR("%s: Not enough memory\n", __func__);
		return -1;
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL) {
		LDIMPR("%s: %d: max_len=%d, tbl_max=%d\n",
		       __func__, flag, max_len, tbl_max);
	}

	while (1) {
		if ((i + 2) > max_len) {
			LDIMERR("%s: %s: %s: no ending error\n",
				__func__, dev_drv->name, propname);
			goto init_table_dynamic_array_err;
		}
		if ((i + 2) > tbl_max) {
			LDIMERR("%s: %s: %s: size out of support (max %d)\n",
				__func__, dev_drv->name, propname, tbl_max);
			goto init_table_dynamic_array_err;
		}
		/* type */
		table[i] = buf[i];
		/* size */
		table[i + 1] = buf[i + 1];
		type = table[i];
		size = table[i + 1];
		if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL) {
			LDIMPR("%s: %s: %s, step[%d]: type=0x%x, size=%d, i=%d\n",
				__func__, dev_drv->name, propname, step, type, size, i);
		}
		i += 2;

		if (type == LCD_EXT_CMD_TYPE_END)
			break;
		if (size == 0)
			goto init_table_dynamic_array_next;
		if ((i + size) > max_len) {
			LDIMERR("%s: %s: %s: size out of data buffer (max %d)\n",
				__func__, dev_drv->name, propname, max_len);
			goto init_table_dynamic_array_err;
		}
		if ((i + size) > tbl_max) {
			LDIMERR("%s: %s: %s: size out of support (max %d)\n",
				__func__, dev_drv->name, propname, tbl_max);
			goto init_table_dynamic_array_err;
		}

		/* step3: data */
		for (j = 0; j < size; j++)
			table[i + j] = buf[i + j];
		i += size;

init_table_dynamic_array_next:
		step++;
	}
	if (flag)
		dev_drv->init_on_cnt = i;
	else
		dev_drv->init_off_cnt = i;

	ret = ldim_dev_init_table_save(dev_drv, flag, table);
	if (ret)
		goto init_table_dynamic_array_err;
	kfree(table);
	return 0;

init_table_dynamic_array_err:
	kfree(table);
	return -1;
}

static int ldim_dev_init_table_handle_dts(struct ldim_dev_driver_s *dev_drv,
					  struct device_node *child)
{
	int len_on, len_off, init_max;
	unsigned int *init_buf;
	int ret;

	len_on = of_property_count_u32_elems(child, "init_on");
	if (len_on < 0)
		len_on = 0;
	len_off = of_property_count_u32_elems(child, "init_off");
	if (len_off < 0)
		len_off = 0;

	init_max = len_on >= len_off ? len_on : len_off;
	if (init_max == 0)
		return 0;

	init_buf = kcalloc(init_max, sizeof(unsigned int), GFP_KERNEL);
	if (!init_buf)
		return -1;

	//init_on
	ret = of_property_read_u32_array(child, "init_on", init_buf, len_on);
	if (ret) {
		LDIMERR("%s: get init_on failed\n", dev_drv->name);
		goto ldim_dev_init_table_handle_dts_err;
	}
	ret = ldim_dev_init_dynamic_load_array(dev_drv, init_buf, len_on, 1);
	if (ret)
		goto ldim_dev_init_table_handle_dts_err;

	//init_off
	ret = of_property_read_u32_array(child, "init_off", init_buf, len_off);
	if (ret) {
		LDIMERR("%s: get init_off failed\n", dev_drv->name);
		goto ldim_dev_init_table_handle_dts_err;
	}
	ret = ldim_dev_init_dynamic_load_array(dev_drv, init_buf, len_off, 0);
	if (ret)
		goto ldim_dev_init_table_handle_dts_err;

	dev_drv->init_loaded = 1;

	kfree(init_buf);
	return 0;

ldim_dev_init_table_handle_dts_err:
	kfree(init_buf);
	return -1;
}

static int ldim_dev_get_config_from_dts(struct ldim_dev_driver_s *dev_drv,
					struct device_node *np, int index, phandle pwm_phandle)
{
	char propname[20];
	struct device_node *child;
	const char *str;
	unsigned int *temp, val, size;
	struct bl_pwm_config_s *bl_pwm;
	struct ldim_profile_s *profile;
	struct ldim_fw_s *fw = aml_ldim_get_fw();
	struct ldim_fw_custom_s *fw_cus = aml_ldim_get_fw_cus();
	char dbg_str[256];
	int i, dbg_str_len = 0, ret = 0;

	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
		LDIMPR("load ldim_dev config from dts\n");

	size = (dev_drv->zone_num >= 10) ? dev_drv->zone_num : 10;
	temp = kcalloc(size, sizeof(unsigned int), GFP_KERNEL);
	if (!temp)
		return -1;

	/* get device config */
	sprintf(propname, "ldim_dev_%d", index);
	LDIMPR("load: %s\n", propname);
	child = of_get_child_by_name(np, propname);
	if (!child) {
		LDIMERR("failed to get %s\n", propname);
		goto ldim_get_config_err;
	}

	ret = of_property_read_string(child, "ldim_dev_name", &str);
	if (ret) {
		LDIMERR("failed to get ldim_dev_name\n");
		str = "ldim_dev";
	}
	strncpy(dev_drv->name, str, (LDIM_DEV_NAME_MAX - 1));

	ret = of_property_read_u32(child, "type", &val);
	if (ret) {
		LDIMERR("failed to get type\n");
		dev_drv->type = LDIM_DEV_TYPE_NORMAL;
	} else {
		dev_drv->type = val;
	}
	if (dev_drv->type >= LDIM_DEV_TYPE_MAX) {
		LDIMERR("type num is out of support\n");
		goto ldim_get_config_err;
	}

	switch (dev_drv->type) {
	case LDIM_DEV_TYPE_SPI:
		/* get spi config */
		dev_drv->spi_info = &ldim_spi_info;
		ret = of_property_read_u32(child, "spi_bus_num", &val);
		if (ret) {
			LDIMERR("failed to get spi_bus_num\n");
		} else {
			ldim_spi_info.bus_num = val;
			if (ldim_debug_print) {
				LDIMPR("spi bus_num: %d\n",
				       ldim_spi_info.bus_num);
			}
		}

		ret = of_property_read_u32(child, "spi_chip_select", &val);
		if (ret) {
			LDIMERR("failed to get spi_chip_select\n");
		} else {
			ldim_spi_info.chip_select = val;
			if (ldim_debug_print) {
				LDIMPR("spi chip_select: %d\n",
				       ldim_spi_info.chip_select);
			}
		}

		ret = of_property_read_u32(child, "spi_max_frequency", &val);
		if (ret) {
			LDIMERR("failed to get spi_chip_select\n");
		} else {
			ldim_spi_info.max_speed_hz = val;
			if (ldim_debug_print) {
				LDIMPR("spi max_speed_hz: %d\n",
				       ldim_spi_info.max_speed_hz);
			}
		}

		ret = of_property_read_u32(child, "spi_mode", &val);
		if (ret) {
			LDIMERR("failed to get spi_mode\n");
		} else {
			ldim_spi_info.mode = val;
			if (ldim_debug_print)
				LDIMPR("spi mode: %d\n", ldim_spi_info.mode);
		}

		ret = of_property_read_u32(child, "spi_dma_support", &val);
		if (ret) {
			LDIMERR("failed to get spi_dma_support\n");
		} else {
			dev_drv->dma_support = (unsigned char)val;
			if (ldim_debug_print) {
				LDIMPR("spi_dma_support: %d\n",
				       dev_drv->dma_support);
			}
		}

		ret = of_property_read_u32_array(child, "spi_cs_delay",
						 &temp[0], 2);
		if (ret) {
			dev_drv->cs_hold_delay = 0;
			dev_drv->cs_clk_delay = 0;
			ldim_spi_controller_data.ss_leading_gap = 0;
			ldim_spi_controller_data.ss_trailing_gap = 0;
		} else {
			dev_drv->cs_hold_delay = temp[0];
			dev_drv->cs_clk_delay = temp[1];
			ldim_spi_controller_data.ss_leading_gap = temp[0];
			ldim_spi_controller_data.ss_trailing_gap = temp[1];
		}
		break;
	default:
		break;
	}

	/* ldim pwm config */
	bl_pwm = &dev_drv->ldim_pwm_config;
	bl_pwm->drv_index = 0; /* only venc0 support ldim */
	ret = of_property_read_string(child, "ldim_pwm_port", &str);
	if (ret) {
		LDIMERR("failed to get ldim_pwm_port\n");
	} else {
		bl_pwm->pwm_port = bl_pwm_str_to_num(str);
		LDIMPR("ldim_pwm_port: %s(0x%x)\n", str, bl_pwm->pwm_port);
	}
	if (bl_pwm->pwm_port < BL_PWM_MAX) {
		ret = of_property_read_u32_array(child, "ldim_pwm_attr",
						 temp, 3);
		if (ret) {
			LDIMERR("failed to get ldim_pwm_attr\n");
			bl_pwm->pwm_method = BL_PWM_POSITIVE;
			if (bl_pwm->pwm_port == BL_PWM_VS)
				bl_pwm->pwm_freq = 1;
			else
				bl_pwm->pwm_freq = 60;
			bl_pwm->pwm_duty = 50;
			bl_pwm->pwm_phase = 0;
		} else {
			bl_pwm->pwm_method = temp[0];
			if (bl_pwm->pwm_port == BL_PWM_VS) {
				bl_pwm->pwm_freq = temp[1] & 0xff;
				bl_pwm->pwm_phase = (temp[1] >> 8) & 0xffffff;
			} else {
				bl_pwm->pwm_freq = temp[1];
				bl_pwm->pwm_phase = 0;
			}
			bl_pwm->pwm_duty = temp[2];
		}
		LDIMPR("get ldim_pwm pol = %d, freq = %d, phase = %d, dft duty = %d%%\n",
		       bl_pwm->pwm_method, bl_pwm->pwm_freq, bl_pwm->pwm_phase, bl_pwm->pwm_duty);

		bl_pwm_config_init(bl_pwm);

		if (bl_pwm->pwm_port < BL_PWM_VS) {
			bl_pwm_channel_register(dev_drv->dev,
						pwm_phandle, bl_pwm);
		}
	}

	/* analog pwm config */
	bl_pwm = &dev_drv->analog_pwm_config;
	bl_pwm->drv_index = 0; /* only venc0 support ldim */
	ret = of_property_read_string(child, "analog_pwm_port", &str);
	if (ret)
		bl_pwm->pwm_port = BL_PWM_MAX;
	else
		bl_pwm->pwm_port = bl_pwm_str_to_num(str);
	if (bl_pwm->pwm_port < BL_PWM_VS) {
		LDIMPR("find analog_pwm_port: %s(%u)\n", str, bl_pwm->pwm_port);
		ret = of_property_read_u32_array(child, "analog_pwm_attr",
						 temp, 5);
		if (ret) {
			LDIMERR("failed to get analog_pwm_attr\n");
		} else {
			bl_pwm->pwm_method = temp[0];
			if (bl_pwm->pwm_port == BL_PWM_VS) {
				bl_pwm->pwm_freq = temp[1] & 0xff;
				bl_pwm->pwm_phase = (temp[1] >> 8) & 0xffffff;
			} else {
				bl_pwm->pwm_freq = temp[1];
				bl_pwm->pwm_phase = 0;
			}
			bl_pwm->pwm_duty_max = temp[2];
			bl_pwm->pwm_duty_min = temp[3];
			bl_pwm->pwm_duty = temp[4];
		}
		LDIMPR("get analog_pwm pol = %d, freq = %d, phase = %d\n",
			bl_pwm->pwm_method, bl_pwm->pwm_freq, bl_pwm->pwm_phase);
		LDIMPR("duty max = %d, min = %d, default = %d\n",
			bl_pwm->pwm_duty_max,
			bl_pwm->pwm_duty_min, bl_pwm->pwm_duty);

		bl_pwm_config_init(bl_pwm);
		bl_pwm_channel_register(dev_drv->dev, pwm_phandle, bl_pwm);
	}

	ret = of_property_read_string(child, "ldim_pwm_pinmux_sel", &str);
	if (ret)
		strcpy(dev_drv->pinmux_name, "invalid");
	else
		strcpy(dev_drv->pinmux_name, str);

	ret = of_property_read_u32_array(child, "en_gpio_on_off", temp, 3);
	if (ret) {
		LDIMERR("failed to get en_gpio_on_off\n");
		dev_drv->en_gpio = BL_GPIO_MAX;
		dev_drv->en_gpio_on = BL_GPIO_OUTPUT_HIGH;
		dev_drv->en_gpio_off = BL_GPIO_OUTPUT_LOW;
	} else {
		if (temp[0] >= BL_GPIO_NUM_MAX) {
			dev_drv->en_gpio = BL_GPIO_MAX;
		} else {
			dev_drv->en_gpio = temp[0];
			ldim_gpio_probe(dev_drv, dev_drv->en_gpio);
		}
		dev_drv->en_gpio_on = temp[1];
		dev_drv->en_gpio_off = temp[2];
	}

	ret = of_property_read_u32(child, "lamp_err_gpio", &val);
	if (ret) {
		dev_drv->lamp_err_gpio = BL_GPIO_MAX;
		dev_drv->fault_check = 0;
	} else {
		if (val >= BL_GPIO_NUM_MAX) {
			dev_drv->lamp_err_gpio = BL_GPIO_MAX;
			dev_drv->fault_check = 0;
		} else {
			dev_drv->lamp_err_gpio = val;
			dev_drv->fault_check = 1;
			ldim_gpio_probe(dev_drv, dev_drv->lamp_err_gpio);
			ldim_gpio_set(dev_drv, dev_drv->lamp_err_gpio,
				      BL_GPIO_INPUT);
		}
	}

	ret = of_property_read_u32(child, "spi_write_check", &val);
	if (ret)
		dev_drv->write_check = 0;
	else
		dev_drv->write_check = (unsigned char)val;

	ret = of_property_read_u32_array(child, "dim_max_min", &temp[0], 2);
	if (ret) {
		LDIMERR("failed to get dim_max_min\n");
		dev_drv->dim_max = 0xfff;
		dev_drv->dim_min = 0x7f;
	} else {
		dev_drv->dim_max = temp[0];
		dev_drv->dim_min = temp[1];
	}

	ret = of_property_read_u32(child, "chip_count", &val);
	if (ret)
		dev_drv->chip_cnt = 1;
	else
		dev_drv->chip_cnt = val;

	ret = of_property_read_u32(child, "mcu_header", &val);
	if (ret)
		dev_drv->mcu_header = 0;
	else
		dev_drv->mcu_header = (unsigned int)val;

	ret = of_property_read_u32(child, "mcu_dim", &val);
	if (ret)
		dev_drv->mcu_dim = 0;
	else
		dev_drv->mcu_dim = (unsigned int)val;

	ret = of_property_read_u32(child, "spi_line_n", &val);
	if (ret) {
		dev_drv->spi_sync = SPI_ASYNC;
		dev_drv->spi_line_n = 0;
		ldim_spi_controller_data.use_ctrl_cs = 0;
	} else {
		dev_drv->spi_sync = SPI_DMA_TRIG;
		dev_drv->spi_line_n = (unsigned int)val;
		ldim_spi_controller_data.use_ctrl_cs = 1;
	}
	dev_drv->use_ctrl_cs = ldim_spi_controller_data.use_ctrl_cs;

	dbg_str_len = sprintf(dbg_str, "mcu_header=0x%08x, mcu_dim=0x%08x,",
		dev_drv->mcu_header, dev_drv->mcu_dim);
	dbg_str_len += sprintf(dbg_str + dbg_str_len, "spi_sync:%d, spi_line_n: %d,",
		dev_drv->spi_sync, dev_drv->spi_line_n);
	sprintf(dbg_str + dbg_str_len, "chip_cnt:%d, cus pwm_pinmux_sel:%s",
		dev_drv->chip_cnt, dev_drv->pinmux_name);
	LDIMPR("load dts config: %s: type:%d, %s\n",
		dev_drv->name, dev_drv->type, dbg_str);

	/*boost_conf*/
	ret = of_property_read_u32_array(child, "boost_conf", temp, 7);
	if (ret) {
		LDIMERR("failed to get boost_conf\n");
		dev_drv->boost_conf.en = 0;
		dev_drv->boost_conf.mode = 0;
		dev_drv->boost_conf.i_l100 = 0;
		dev_drv->boost_conf.i_l32 = 0;
		dev_drv->boost_conf.i_l100_val = 0;
		dev_drv->boost_conf.i_l32_val = 0;
		dev_drv->boost_conf.kp_l100 = 0;
		dev_drv->boost_conf.kp_l32 = 0;
	} else {
		dev_drv->boost_conf.en = (unsigned char)temp[0];
		dev_drv->boost_conf.mode = (unsigned char)temp[1];
		dev_drv->boost_conf.i_l100 = (unsigned short)temp[3];
		dev_drv->boost_conf.i_l32 = (unsigned short)temp[4];
		dev_drv->boost_conf.i_l100_val = (unsigned short)temp[5];
		dev_drv->boost_conf.i_l32_val = (unsigned short)temp[6];
		dev_drv->boost_conf.kp_l100 = (unsigned char)temp[7];
		dev_drv->boost_conf.kp_l32 = (unsigned char)temp[8];
	}

	dbg_str_len = sprintf(dbg_str, "i_l100:%d, i_l32:%d, i_l100_val:%d, i_l32_val:%d, ",
		dev_drv->boost_conf.i_l100, dev_drv->boost_conf.i_l32,
		dev_drv->boost_conf.i_l100_val, dev_drv->boost_conf.i_l32_val);
	sprintf(dbg_str + dbg_str_len, "kp_l100:%d, kp_l32:%d",
		dev_drv->boost_conf.kp_l100, dev_drv->boost_conf.kp_l32);
	LDIMPR("boost_conf: en:%d, mode:%d, %s\n",
		dev_drv->boost_conf.en, dev_drv->boost_conf.mode, dbg_str);

	ret = of_property_read_string(child, "ldim_zone_mapping_path", &str);
	if (ret == 0) {
		LDIMPR("find custom ldim_zone_mapping\n");
		strncpy(dev_drv->bl_mapping_path, str, 255);
		goto ldim_dev_get_config_from_dts_profile;
	}
	ret = of_property_read_u32_array(child, "ldim_zone_mapping",
					 &temp[0], dev_drv->zone_num);
	if (ret) {
		ret = of_property_read_u32_array(child, "ldim_region_mapping",
						 &temp[0], dev_drv->zone_num);
		if (ret) {
			for (i = 0; i < dev_drv->zone_num; i++)
				dev_drv->bl_mapping[i] = (unsigned short)i;
			goto ldim_dev_get_config_from_dts_profile;
		}
	}
	LDIMPR("find custom ldim_zone_mapping\n");
	for (i = 0; i < dev_drv->zone_num; i++)
		dev_drv->bl_mapping[i] = (unsigned short)temp[i];

ldim_dev_get_config_from_dts_profile:
	ret = of_property_read_u32(child, "ldim_bl_profile_mode", &val);
	if (ret)
		goto ldim_dev_get_config_from_dts_next;

	LDIMPR("find ldim_bl_profile_mode=%d\n", val);
	profile = kzalloc(sizeof(*profile), GFP_KERNEL);
	if (!profile) {
		LDIMERR("ld_profile malloc failed\n");
		goto ldim_dev_get_config_from_dts_next;
	}
	fw->profile = profile;
	profile->mode = val;

	if (profile->mode == 2) {
		LDIMPR("load bl_profile\n");
		ret = of_property_read_u32_array(child,
			"ldim_bl_profile_k_bits", &temp[0], 2);
		if (ret) {
			LDIMERR("failed to get ldim_bl_profile_k_bits\n");
			profile->profile_k = 24;
			profile->profile_bits = 640;
		} else {
			profile->profile_k = temp[0];
			profile->profile_bits = temp[1];
		}

		ret = of_property_read_string(child, "ldim_bl_profile_path", &str);
		if (ret) {
			LDIMERR("failed to get ldim_bl_profile_path\n");
			strcpy(profile->file_path, "null");
		} else {
			strncpy(profile->file_path, str, 255);
		}
	}

ldim_dev_get_config_from_dts_next:
	/* get cus_param , maximum = 32 x 4 bytes */
	for (i = 0; i < 32; i++) {
		ret = of_property_read_u32_index(child,
			"ldim_cus_param", i, &val);
		if (ret) {
			LDIMPR("get ldim_cus_param[%d] failed, size =%d\n",
				i, i);
			i = 32;
		} else {
			if (fw_cus) {
				fw_cus->param[i] = val;
				LDIMPR("param[%d] = %d\n",
					i, fw_cus->param[i]);
			}
		}
	}

	/* get init_cmd */
	ret = of_property_read_u32(child, "cmd_size", &val);
	if (ret) {
		LDIMPR("no cmd_size\n");
		dev_drv->cmd_size = 0;
	} else {
		dev_drv->cmd_size = (unsigned char)val;
	}
	if (ldim_debug_print)
		LDIMPR("%s: cmd_size = %d\n", dev_drv->name, dev_drv->cmd_size);
	if (dev_drv->cmd_size != LCD_EXT_CMD_SIZE_DYNAMIC)
		goto ldim_dev_get_config_from_dts_end;

	ret = ldim_dev_init_table_handle_dts(dev_drv, child);
	if (ret)
		goto ldim_get_config_err;

ldim_dev_get_config_from_dts_end:
	kfree(temp);
	return 0;

ldim_get_config_err:
	kfree(temp);
	return -1;
}

static int ldim_dev_init_table_handle_ukey(struct ldim_dev_driver_s *dev_drv,
					   unsigned char *p, int key_len)
{
	unsigned int *init_buf;
	int init_offset, init_max;
	int i, ret;

	init_offset = LCD_UKEY_LDIM_DEV_INIT;
	init_max = key_len - LCD_UKEY_LDIM_DEV_INIT;
	if (init_max <= 0)
		return 0;

	init_buf = kcalloc(init_max, sizeof(unsigned int), GFP_KERNEL);
	if (!init_buf)
		return -1;
	for (i = 0; i < init_max; i++)
		init_buf[i] = *(p + init_offset + i);
	ret = ldim_dev_init_dynamic_load_array(dev_drv, init_buf, init_max, 1);
	if (ret)
		goto ldim_dev_init_table_handle_ukey_err;

	init_offset += dev_drv->init_on_cnt;
	init_max -= dev_drv->init_on_cnt;
	if (init_max > 0) {
		for (i = 0; i < init_max; i++)
			init_buf[i] = *(p + init_offset + i);
		ret = ldim_dev_init_dynamic_load_array(dev_drv, init_buf, init_max, 0);
		if (ret)
			goto ldim_dev_init_table_handle_ukey_err;
	} else {
		dev_drv->init_off_cnt = 0;
	}

	dev_drv->init_loaded = 1;

	kfree(init_buf);
	return 0;

ldim_dev_init_table_handle_ukey_err:
	kfree(init_buf);
	return -1;
}

static int ldim_dev_get_config_from_ukey(struct ldim_dev_driver_s *dev_drv, phandle pwm_phandle)
{
	unsigned char *para, *p;
	int key_len, len;
	const char *str;
	unsigned int temp, temp_val;
	struct bl_pwm_config_s *bl_pwm;
	struct ldim_profile_s *profile;
	char dbg_str[256];
	int i, dbg_str_len = 0, ret = 0;
	struct ldim_fw_s *fw = aml_ldim_get_fw();
	struct ldim_fw_custom_s *fw_cus = aml_ldim_get_fw_cus();

	ret = lcd_unifykey_get_size("ldim_dev", &key_len);
	if (ret)
		return -1;
	para = kcalloc(key_len, (sizeof(unsigned char)), GFP_KERNEL);
	if (!para) {
		LDIMERR("para kcalloc failed!!\n");
		return -1;
	}

	ret = lcd_unifykey_get("ldim_dev", para, key_len);
	if (ret < 0)
		goto ldim_dev_get_config_from_ukey_err;

	/* step 1: check header */
	if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL)
		lcd_unifykey_header_print(para);

	/* step 2: check parameters */
	len = 65; //10+30+25
	ret = lcd_unifykey_len_check(key_len, len);
	if (ret < 0) {
		LDIMERR("unifykey length is incorrect\n");
		goto ldim_dev_get_config_from_ukey_err;
	}

	/* basic: 30byte */
	p = para;
	str = (const char *)(p + LCD_UKEY_HEAD_SIZE);
	strncpy(dev_drv->name, str, (LDIM_DEV_NAME_MAX - 1));

	/* interface (25Byte) */
	dev_drv->type = *(p + LCD_UKEY_LDIM_DEV_IF_TYPE);
	if (dev_drv->type >= LDIM_DEV_TYPE_MAX) {
		LDIMERR("invalid type %d\n", dev_drv->type);
		goto ldim_dev_get_config_from_ukey_err;
	}

	switch (dev_drv->type) {
	case LDIM_DEV_TYPE_SPI:
		/* get spi config */
		dev_drv->spi_info = &ldim_spi_info;

		ldim_spi_info.bus_num = *(p + LCD_UKEY_LDIM_DEV_IF_ATTR_0);
		ldim_spi_info.chip_select = *(p + LCD_UKEY_LDIM_DEV_IF_ATTR_1);
		ldim_spi_info.max_speed_hz =
			(*(p + LCD_UKEY_LDIM_DEV_IF_FREQ) |
			((*(p + LCD_UKEY_LDIM_DEV_IF_FREQ + 1)) << 8) |
			((*(p + LCD_UKEY_LDIM_DEV_IF_FREQ + 2)) << 16) |
			((*(p + LCD_UKEY_LDIM_DEV_IF_FREQ + 3)) << 24));
		ldim_spi_info.mode = *(p + LCD_UKEY_LDIM_DEV_IF_ATTR_2);
		dev_drv->dma_support = *(p + LCD_UKEY_LDIM_DEV_IF_ATTR_3);
		dev_drv->cs_hold_delay =
			(*(p + LCD_UKEY_LDIM_DEV_IF_ATTR_4) |
			((*(p + LCD_UKEY_LDIM_DEV_IF_ATTR_4 + 1)) << 8));
		dev_drv->cs_clk_delay =
			(*(p + LCD_UKEY_LDIM_DEV_IF_ATTR_5) |
			((*(p + LCD_UKEY_LDIM_DEV_IF_ATTR_5 + 1)) << 8));
		ldim_spi_controller_data.ss_leading_gap = dev_drv->cs_hold_delay;
		ldim_spi_controller_data.ss_trailing_gap = dev_drv->cs_clk_delay;
		if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL) {
			LDIMPR("spi bus_num: %d\n", ldim_spi_info.bus_num);
			LDIMPR("spi chip_select: %d\n",
			       ldim_spi_info.chip_select);
			LDIMPR("spi max_speed_hz: %d\n",
			       ldim_spi_info.max_speed_hz);
			LDIMPR("spi mode: %d\n", ldim_spi_info.mode);
			LDIMPR("spi dma_support: %d\n", dev_drv->dma_support);
		}
		break;
	default:
		break;
	}

	/* pwm (48Byte) */
	bl_pwm = &dev_drv->ldim_pwm_config;
	bl_pwm->drv_index = 0; /* only venc0 support ldim */
	bl_pwm->pwm_port = *(p + LCD_UKEY_LDIM_DEV_PWM_VS_PORT);
	if (bl_pwm->pwm_port < BL_PWM_MAX) {
		bl_pwm->pwm_method = *(p + LCD_UKEY_LDIM_DEV_PWM_VS_POL);
		if (bl_pwm->pwm_port == BL_PWM_VS) {
			temp = (*(p + LCD_UKEY_LDIM_DEV_PWM_VS_FREQ) |
				((*(p + LCD_UKEY_LDIM_DEV_PWM_VS_FREQ + 1)) << 8) |
				((*(p + LCD_UKEY_LDIM_DEV_PWM_VS_FREQ + 2)) << 16) |
				((*(p + LCD_UKEY_LDIM_DEV_PWM_VS_FREQ + 3)) << 24));

			bl_pwm->pwm_freq = (temp & 0xff);
			bl_pwm->pwm_phase = (temp >> 8) & 0xffffff;

		} else {
			bl_pwm->pwm_freq =
				(*(p + LCD_UKEY_LDIM_DEV_PWM_VS_FREQ) |
				((*(p + LCD_UKEY_LDIM_DEV_PWM_VS_FREQ + 1)) << 8) |
				((*(p + LCD_UKEY_LDIM_DEV_PWM_VS_FREQ + 2)) << 16) |
				((*(p + LCD_UKEY_LDIM_DEV_PWM_VS_FREQ + 3)) << 24));
			bl_pwm->pwm_phase = 0;
		}
		bl_pwm->pwm_duty =
			(*(p + LCD_UKEY_LDIM_DEV_PWM_VS_DUTY) |
			((*(p + LCD_UKEY_LDIM_DEV_PWM_VS_DUTY + 1)) << 8));
		LDIMPR("get ldim_pwm pol = %d, freq = %d, phase = %d, dft duty = %d%%\n",
		       bl_pwm->pwm_method, bl_pwm->pwm_freq, bl_pwm->pwm_phase, bl_pwm->pwm_duty);
		bl_pwm_config_init(bl_pwm);
		if (bl_pwm->pwm_port < BL_PWM_VS)
			bl_pwm_channel_register(dev_drv->dev, pwm_phandle, bl_pwm);
	}

	bl_pwm = &dev_drv->analog_pwm_config;
	bl_pwm->drv_index = 0; /* only venc0 support ldim */
	bl_pwm->pwm_port = *(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_PORT);
	if (bl_pwm->pwm_port < BL_PWM_VS) {
		bl_pwm->pwm_method = *(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_POL);
		if (bl_pwm->pwm_port == BL_PWM_VS) {
			temp = (*(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_FREQ) |
				((*(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_FREQ + 1)) << 8) |
				((*(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_FREQ + 2)) << 16) |
				((*(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_FREQ + 3)) << 24));
			bl_pwm->pwm_freq = (temp & 0xff);
			bl_pwm->pwm_phase = (temp >> 8) & 0xffffff;
		} else {
			bl_pwm->pwm_freq =
				(*(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_FREQ) |
				((*(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_FREQ + 1)) << 8) |
				((*(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_FREQ + 2)) << 16) |
				((*(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_FREQ + 3)) << 24));
			bl_pwm->pwm_phase = 0;
		}

		bl_pwm->pwm_duty =
			(*(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_DUTY) |
			((*(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_DUTY + 1)) << 8));
		bl_pwm->pwm_duty_max =
			(*(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_ATTR_0) |
			((*(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_ATTR_0 + 1)) << 8));
		bl_pwm->pwm_duty_min =
			(*(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_ATTR_1) |
			((*(p + LCD_UKEY_LDIM_DEV_PWM_ADJ_ATTR_1 + 1)) << 8));
		LDIMPR("get analog_pwm pol = %d, freq = %d, phase = %d\n",
			bl_pwm->pwm_method, bl_pwm->pwm_freq, bl_pwm->pwm_phase);
		LDIMPR("duty max = %d, min = %d, default = %d\n",
			bl_pwm->pwm_duty_max,
			bl_pwm->pwm_duty_min, bl_pwm->pwm_duty);
		bl_pwm_config_init(bl_pwm);
		bl_pwm_channel_register(dev_drv->dev, pwm_phandle, bl_pwm);
	}

	str = (const char *)(p + LCD_UKEY_LDIM_DEV_PINMUX_SEL);
	if (strlen(str) == 0)
		strcpy(dev_drv->pinmux_name, "invalid");
	else
		strncpy(dev_drv->pinmux_name, str, (LDIM_DEV_NAME_MAX - 1));

	/* ctrl (271Byte) */
	temp = *(p + LCD_UKEY_LDIM_DEV_EN_GPIO);
	if (temp >= BL_GPIO_NUM_MAX) {
		dev_drv->en_gpio = BL_GPIO_MAX;
	} else {
		dev_drv->en_gpio = temp;
		ldim_gpio_probe(dev_drv, dev_drv->en_gpio);
	}
	dev_drv->en_gpio_on = *(p + LCD_UKEY_LDIM_DEV_EN_GPIO_ON);
	dev_drv->en_gpio_off = *(p + LCD_UKEY_LDIM_DEV_EN_GPIO_OFF);

	temp = *(p + LCD_UKEY_LDIM_DEV_ERR_GPIO);
	if (temp >= BL_GPIO_NUM_MAX) {
		dev_drv->lamp_err_gpio = BL_GPIO_MAX;
		dev_drv->fault_check = 0;
	} else {
		dev_drv->lamp_err_gpio = temp;
		dev_drv->fault_check = 1;
		ldim_gpio_probe(dev_drv, dev_drv->lamp_err_gpio);
		ldim_gpio_set(dev_drv, dev_drv->lamp_err_gpio, BL_GPIO_INPUT);
	}

	temp = (*(p + LCD_UKEY_LDIM_DEV_ON_DELAY) |
			((*(p + LCD_UKEY_LDIM_DEV_ON_DELAY + 1)) << 8));
	dev_drv->hw_on_delay = temp;

	temp = (*(p + LCD_UKEY_LDIM_DEV_OFF_DELAY) |
			((*(p + LCD_UKEY_LDIM_DEV_OFF_DELAY + 1)) << 8));
	dev_drv->hw_off_delay = temp;

	dev_drv->write_check = *(p + LCD_UKEY_LDIM_DEV_WRITE_CHECK);

	dev_drv->dim_max =
		(*(p + LCD_UKEY_LDIM_DEV_DIM_MAX) |
		((*(p + LCD_UKEY_LDIM_DEV_DIM_MAX + 1)) << 8));
	dev_drv->dim_min =
		(*(p + LCD_UKEY_LDIM_DEV_DIM_MIN) |
		((*(p + LCD_UKEY_LDIM_DEV_DIM_MIN + 1)) << 8));

	dev_drv->chip_cnt =
		(*(p + LCD_UKEY_LDIM_DEV_CHIP_COUNT) |
		((*(p + LCD_UKEY_LDIM_DEV_CHIP_COUNT + 1)) << 8));

	dev_drv->mcu_header =
		(*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_0) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_0 + 1)) << 8) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_0 + 2)) << 16) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_0 + 3)) << 24));
	dev_drv->mcu_dim =
		(*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_1) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_1 + 1)) << 8) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_1 + 2)) << 16) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_1 + 3)) << 24));

	temp =	(*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_2) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_2 + 1)) << 8) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_2 + 2)) << 16) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_2 + 3)) << 24));
	if (temp == 0) {
		dev_drv->spi_sync = SPI_ASYNC;
		dev_drv->spi_line_n = 0;
		ldim_spi_controller_data.use_ctrl_cs = 0;
	} else {
		dev_drv->spi_sync = SPI_DMA_TRIG;
		dev_drv->spi_line_n = temp;
		ldim_spi_controller_data.use_ctrl_cs = 1;
	}
	dev_drv->use_ctrl_cs = ldim_spi_controller_data.use_ctrl_cs;

	dbg_str_len = sprintf(dbg_str, "mcu_header=0x%08x, mcu_dim=0x%08x, ",
		dev_drv->mcu_header, dev_drv->mcu_dim);
	dbg_str_len += sprintf(dbg_str + dbg_str_len, "spi_sync:%d, spi_line_n: %d, ",
		dev_drv->spi_sync, dev_drv->spi_line_n);
	sprintf(dbg_str + dbg_str_len,
		"chip_cnt:%d, hw_on_delay: %dms, hw_off_delay: %dms, cus pwm_pinmux_sel:%s",
		dev_drv->chip_cnt, dev_drv->hw_on_delay, dev_drv->hw_off_delay,
		dev_drv->pinmux_name);
	LDIMPR("load ukey config: %s: type:%d, %s\n",
		dev_drv->name, dev_drv->type, dbg_str);

	/* boost_conf (8Byte) */
	temp =	(*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_3) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_3 + 1)) << 8) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_3 + 2)) << 16) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_3 + 3)) << 24));
	dev_drv->boost_conf.en = (temp >> 31) & 0x1;
	dev_drv->boost_conf.mode = (temp >> 24) & 0x7f;
	dev_drv->boost_conf.kp_l32 = (temp >> 8) & 0xff;
	dev_drv->boost_conf.kp_l100 = temp & 0xff;
	temp =	(*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_4) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_4 + 1)) << 8) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_4 + 2)) << 16) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_4 + 3)) << 24));
	dev_drv->boost_conf.i_l100 = temp & 0xffff;
	dev_drv->boost_conf.i_l32 = (temp >> 16) & 0xffff;
	temp =	(*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_5) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_5 + 1)) << 8) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_5 + 2)) << 16) |
		((*(p + LCD_UKEY_LDIM_DEV_CUST_ATTR_5 + 3)) << 24));
	dev_drv->boost_conf.i_l100_val = temp & 0xffff;
	dev_drv->boost_conf.i_l32_val = (temp >> 16) & 0xffff;

	dbg_str_len = sprintf(dbg_str, "i_l100:%d, i_l32:%d, i_l100_val:%d, i_l32_val:%d, ",
		dev_drv->boost_conf.i_l100, dev_drv->boost_conf.i_l32,
		dev_drv->boost_conf.i_l100_val, dev_drv->boost_conf.i_l32_val);
	sprintf(dbg_str + dbg_str_len, "kp_l100:%d, kp_l32:%d",
		dev_drv->boost_conf.kp_l100, dev_drv->boost_conf.kp_l32);
	LDIMPR("boost_conf: en:%d, mode:%d, %s\n",
		dev_drv->boost_conf.en, dev_drv->boost_conf.mode, dbg_str);

	str = (const char *)(p + LCD_UKEY_LDIM_DEV_ZONE_MAP_PATH);
	if (strlen(str) == 0) {
		for (i = 0; i < dev_drv->zone_num; i++)
			dev_drv->bl_mapping[i] = (unsigned short)i;
	} else {
		LDIMPR("find custom zone_mapping\n");
		strncpy(dev_drv->bl_mapping_path, str, 255);
	}

	/* profile (273Byte) */
	temp = *(p + LCD_UKEY_LDIM_DEV_PROFILE_MODE);
	if (temp == 0)
		goto ldim_dev_get_config_from_ukey_next;
	LDIMPR("find ldim_bl_profile_mode=%d\n", temp);
	profile = kzalloc(sizeof(*profile), GFP_KERNEL);
	if (!profile)
		goto ldim_dev_get_config_from_ukey_next;
	fw->profile = profile;
	profile->mode = temp;

	if (profile->mode == 1) {
		LDIMPR("load bl_profile\n");
		str = (const char *)(p + LCD_UKEY_LDIM_DEV_PROFILE_PATH);
		if (strlen(str) == 0) {
			LDIMERR("failed to get ldim_bl_profile_path\n");
			strcpy(profile->file_path, "null");
		} else {
			strncpy(profile->file_path, str, 255);
		}
	} else if (profile->mode == 2) {
		LDIMPR("load bl_profile\n");
		str = (const char *)(p + LCD_UKEY_LDIM_DEV_PROFILE_PATH);
		if (strlen(str) == 0) {
			LDIMERR("failed to get ldim_bl_profile_path\n");
			strcpy(profile->file_path, "null");
		} else {
			strncpy(profile->file_path, str, 255);
		}

		profile->profile_k =
			(*(p + LCD_UKEY_LDIM_DEV_PROFILE_ATTR_0) |
			((*(p + LCD_UKEY_LDIM_DEV_PROFILE_ATTR_0 + 1)) << 8));
		profile->profile_bits =
			(*(p + LCD_UKEY_LDIM_DEV_PROFILE_ATTR_1) |
			((*(p + LCD_UKEY_LDIM_DEV_PROFILE_ATTR_1 + 1)) << 8));
	}

ldim_dev_get_config_from_ukey_next:
	/* param_data : maximum = 32 x 4 Bytes */
	temp = *(p + LCD_UKEY_LDIM_DEV_CUST_PARAM_SIZE);
	LDIMPR("custom param size = %d\n",  temp);
	for (i = 0; i < temp; i++) {
		temp_val =
			(*(p + LCD_UKEY_LDIM_DEV_CUST_PARAM
			+ 4 * i) |
			((*(p + LCD_UKEY_LDIM_DEV_CUST_PARAM
			+ 4 * i + 1)) << 8) |
			((*(p + LCD_UKEY_LDIM_DEV_CUST_PARAM
			+ 4 * i + 2)) << 16) |
			((*(p + LCD_UKEY_LDIM_DEV_CUST_PARAM
			+ 4 * i + 3)) << 24));
		if (fw_cus) {
			fw_cus->param[i] = temp_val;
			LDIMPR("fw_cus->param[%d] = %d =  0x%x\n",
			i, fw_cus->param[i],
			fw_cus->param[i]);
		}
	}

	dev_drv->cmd_size = *(p + LCD_UKEY_LDIM_DEV_CMD_SIZE);
	if (ldim_debug_print)
		LDIMPR("%s: cmd_size = %d\n", dev_drv->name, dev_drv->cmd_size);
	if (dev_drv->cmd_size != LCD_EXT_CMD_SIZE_DYNAMIC)
		goto ldim_dev_get_config_from_ukey_end;

	ret = ldim_dev_init_table_handle_ukey(dev_drv, p, key_len);
	if (ret)
		goto ldim_dev_get_config_from_ukey_err;

ldim_dev_get_config_from_ukey_end:
	kfree(para);
	return 0;

ldim_dev_get_config_from_ukey_err:
	kfree(para);
	return -1;
}

/* json ==========================================================================================*/
struct num_str_s ldim_dev_type[] = {
	{LDIM_DEV_TYPE_NORMAL, "NORMAL"},
	{LDIM_DEV_TYPE_SPI, "SPI"},
	{LDIM_DEV_TYPE_I2C, "I2C"},
	{LDIM_DEV_TYPE_MAX, "MAX"}
};

static int ldim_gpio_name_to_index(struct ldim_dev_driver_s *drv, const char *name)
{
	int i = 0;

	if (!drv || !name)
		return BL_GPIO_MAX;

	for (i = 0; i < BL_GPIO_NUM_MAX; i++)
		if (!strcmp(ldim_gpio[i].name, name))
			return i;
	return BL_GPIO_MAX;
}

static int ldim_dev_get_config_from_json(struct ldim_dev_driver_s *dev_drv, phandle pwm_phandle)
{
	struct json_parse_s *jsp = get_panel_jsp(0);
	struct json_s *parent, *child, *child2, *child3;
	int cnt = 0, i = 0, nums_size;
	const char *str = NULL;
	struct spi_board_info *spi_info;
	struct bl_pwm_config_s *bl_pwm, *pwms[3];
	struct ldim_profile_s *profile;
	struct ldim_fw_s *fw = aml_ldim_get_fw();
	struct ldim_fw_custom_s *fw_cus = aml_ldim_get_fw_cus();
	unsigned int *nums = NULL;

	if (jsp->status != JSON_STATUS_OK) {
		LDIMERR("panel 0 json not ready\n");
		return -1;
	}

	parent = json_path_to_node(jsp, jsp->root, "backlight/ldim_dev");
	if (!parent) {
		LDIMERR("failed find /backlight/ldim_dev\n");
		return -1;
	}

//basic_info
	child = json_get_object_child(jsp, parent, "basic_info");
	if (!child) {
		LDIMERR("fail to get basic_info\n");
		return -1;
	}

	str = json_get_obj_str(jsp, child, "name", NULL);
	strncpy(dev_drv->name, str, str ? LDIM_DEV_NAME_MAX - 1 : 0);
	dev_drv->index    = 0;
	dev_drv->chip_cnt = json_get_obj_u32(jsp, child, "chip_count", 1);
	dev_drv->dim_min  = json_get_obj_u32(jsp, child, "dim_min", 0);
	dev_drv->dim_max  = json_get_obj_u32(jsp, child, "dim_max", 4095);

//interface
	child = json_get_object_child(jsp, parent, "interface");
	if (!child) {
		LDIMERR("fail to get interface\n");
		return -1;
	}

	str = json_get_obj_str(jsp, child, "type", NULL);
	dev_drv->type = strnum_get_num(str, ldim_dev_type,
				       ARRAY_SIZE(ldim_dev_type), LDIM_DEV_TYPE_MAX);
	if (dev_drv->type == LDIM_DEV_TYPE_MAX) {
		LDIMERR("invalid type:%d\n", dev_drv->type);
		return -1;
	}

	switch (dev_drv->type) {
	case LDIM_DEV_TYPE_SPI:
		dev_drv->spi_info = &ldim_spi_info;
		spi_info = dev_drv->spi_info;
		spi_info->bus_num = json_get_obj_u32(jsp, child, "bus_number", 2);
		spi_info->chip_select = json_get_obj_u32(jsp, child, "chip_select", 0);
		spi_info->max_speed_hz = json_get_obj_u32(jsp, child, "max_frequency_hz", 3000000);
		spi_info->mode = json_get_obj_u32(jsp, child, "spi_mode", 0);
		dev_drv->cs_hold_delay = json_get_obj_u32(jsp, child, "cs_hold_delay_ms", 0);
		dev_drv->cs_clk_delay = json_get_obj_u32(jsp, child, "cs_clk_delay_ms", 0);
		dev_drv->spi_line_n = json_get_obj_u32(jsp, child, "line_n", 0);
		dev_drv->spi_sync = json_get_obj_u32(jsp, child, "spi_sync", 0);
		dev_drv->dma_support = json_get_obj_u32(jsp, child, "dma_support", 0);

		ldim_spi_controller_data.ss_leading_gap = dev_drv->cs_hold_delay;
		ldim_spi_controller_data.ss_trailing_gap = dev_drv->cs_clk_delay;
		if (dev_drv->spi_line_n == 0) {
			dev_drv->spi_sync = SPI_ASYNC;
			ldim_spi_controller_data.use_ctrl_cs = 0;
		} else {
			dev_drv->spi_sync = SPI_DMA_TRIG;
			ldim_spi_controller_data.use_ctrl_cs = 1;
		}

		if (lcd_debug_print_flag & LCD_DBG_PR_BL_NORMAL) {
			LDIMPR("spi bus: %d, mode:%d, cs:%d %dhz, dma:%d, cs_hold:%d, cs_clk:%d\n",
			       ldim_spi_info.bus_num,
			       ldim_spi_info.mode,
			       ldim_spi_info.chip_select,
			       ldim_spi_info.max_speed_hz,
			       dev_drv->dma_support,
			       dev_drv->cs_hold_delay,
			       dev_drv->cs_clk_delay);
		}
		break;
	default:
		break;
	}

//pwms
	child = json_get_object_child(jsp, parent, "pwms");
	if (child) {
		cnt = json_get_array_size(jsp, child);
		cnt = lcd_s32_constraint(cnt, 0, 2);
		pwms[0] = &dev_drv->ldim_pwm_config;
		pwms[1] = &dev_drv->analog_pwm_config;
		for (i = 0; i < cnt; i++) {
			child2 = json_get_array_child(jsp, child, i);
			if (!child2) {
				BLPR("fail find pwm[%d]\n", i);
				break;
			}

			bl_pwm = pwms[i];
			bl_pwm->drv_index = 0;
			str = json_get_obj_str(jsp, child2, "port", NULL);
			bl_pwm->pwm_port = bl_pwm_str_to_num(str ? str : "Invalid");
			if (bl_pwm->pwm_port >= BL_PWM_MAX ||
			    (i == 1 && bl_pwm->pwm_port >= BL_PWM_VS))
				continue;

			bl_pwm->pwm_method = json_get_obj_u32(jsp, child2, "polarity", 1);
			bl_pwm->pwm_phase  = json_get_obj_u32(jsp, child2, "phase", 0);
			bl_pwm->pwm_freq   = json_get_obj_u32(jsp, child2, "freq", 300);
			if (bl_pwm->pwm_freq > XTAL_HALF_FREQ_HZ)
				bl_pwm->pwm_freq = XTAL_HALF_FREQ_HZ;

			child3 = json_get_object_child(jsp, child2, "duty_range");
			if (child3) {
				bl_pwm->pwm_duty_min = json_get_arr_u32(jsp, child3, 0, 0);
				bl_pwm->pwm_duty_max = json_get_arr_u32(jsp, child3, 1, 4095);
			}
			bl_pwm->pwm_duty = json_get_obj_u32(jsp, child2, "duty",
							    bl_pwm->pwm_duty_min);

			bl_pwm_config_init(bl_pwm);
			if (bl_pwm->pwm_port < BL_PWM_VS)
				bl_pwm_channel_register(dev_drv->dev, pwm_phandle, bl_pwm);
			LDIMPR("get pwm[%d] pol = %d, freq = %d, phase = %d, duty:%d(%d ~ %d)\n",
				i, bl_pwm->pwm_method, bl_pwm->pwm_freq, bl_pwm->pwm_phase,
				bl_pwm->pwm_duty, bl_pwm->pwm_duty_min, bl_pwm->pwm_duty_max);
		}
	}

//ctrl
	child = json_get_object_child(jsp, parent, "ctrl");
	if (!child) {
		str = json_get_obj_str(jsp, child, "pinmux_name", NULL);
		strncpy(dev_drv->pinmux_name, str ? str : "invalid", (LDIM_DEV_NAME_MAX - 1));

		str = json_get_obj_str(jsp, child, "err_gpio", NULL);
		dev_drv->lamp_err_gpio = ldim_gpio_name_to_index(dev_drv, str);
		str = json_get_obj_str(jsp, child, "en_gpio", NULL);
		dev_drv->en_gpio = ldim_gpio_name_to_index(dev_drv, str);
		dev_drv->en_gpio_on = json_get_obj_u32(jsp, child, "en_gpio_on", 1);
		dev_drv->en_gpio_off = json_get_obj_u32(jsp, child, "en_gpio_off", 0);

		if (dev_drv->en_gpio < BL_GPIO_NUM_MAX)
			ldim_gpio_probe(dev_drv, dev_drv->en_gpio);
		if (dev_drv->lamp_err_gpio < BL_GPIO_NUM_MAX) {
			dev_drv->fault_check = 1;
			ldim_gpio_probe(dev_drv, dev_drv->lamp_err_gpio);
			ldim_gpio_set(dev_drv, dev_drv->lamp_err_gpio, BL_GPIO_INPUT);
		}

		dev_drv->hw_on_delay = json_get_obj_u32(jsp, child, "hw_on_delay_ms", 0);
		dev_drv->hw_off_delay = json_get_obj_u32(jsp, child, "hw_off_delay_ms", 0);
		dev_drv->write_check = json_get_obj_u32(jsp, child, "write_check", 0);
	}

//packet_info
	child = json_get_object_child(jsp, parent, "packet_info");
	if (child) {
		dev_drv->mcu_header = json_get_obj_u32(jsp, child, "header", 0x0);
		dev_drv->mcu_dim = json_get_obj_u32(jsp, child, "mcu_dim", 0x0);
	}

//boost
	child = json_get_object_child(jsp, parent, "boost");
	if (child) {
		dev_drv->boost_conf.en = json_get_obj_u32(jsp, child, "en", 0);
		dev_drv->boost_conf.mode = json_get_obj_u32(jsp, child, "mode", 0);
		dev_drv->boost_conf.kp_l32 = json_get_obj_u32(jsp, child, "kp_l32", 0);
		dev_drv->boost_conf.kp_l100 = json_get_obj_u32(jsp, child, "kp_l100", 0x0);
		dev_drv->boost_conf.i_l32 = json_get_obj_u32(jsp, child, "i_l32", 0x0);
		dev_drv->boost_conf.i_l100 = json_get_obj_u32(jsp, child, "i_l100", 0x0);
		dev_drv->boost_conf.i_l32_val = json_get_obj_u32(jsp, child, "i_l32_val", 0x0);
		dev_drv->boost_conf.i_l100_val = json_get_obj_u32(jsp, child, "i_l100_val", 0x0);
	}

//profile & zone map
	for (i = 0; i < dev_drv->zone_num; i++)
		dev_drv->bl_mapping[i] = (unsigned short)i;

	str = json_get_obj_str(jsp, parent, "profile_path", NULL);
	if (str) {
		profile = kzalloc(sizeof(*profile), GFP_KERNEL);
		strncpy(profile->file_path, str, 255);
		fw->profile = profile;
	}

//custom_params
	child = json_get_object_child(jsp, parent, "custom_params");
	if (child && fw_cus && fw_cus->param) {
		cnt = json_get_array_size(jsp, child);
		cnt = lcd_s32_constraint(cnt, 0, 32);
		for (i = 0; i < cnt; i++)
			fw_cus->param[i] = json_get_arr_u32(jsp, child, i, 0);
	}

//commands
	child = json_get_object_child(jsp, parent, "commands");
	if (child) {
		dev_drv->cmd_size = LCD_EXT_CMD_SIZE_DYNAMIC;

		str = json_get_obj_str(jsp, child, "init_on", NULL);
		nums_size = (strlen(str)) * sizeof(unsigned int);
		nums = kzalloc(nums_size, GFP_KERNEL);
		if (!nums) {
			LDIMPR("ldim find init_on: no memory to save nums\n");
			goto parse_ldim_init_off;
		}

		memset(nums, 0, nums_size);
		cnt = string_to_numbers(str, nums);
		ldim_dev_init_dynamic_load_array(dev_drv, nums, cnt, 1);

parse_ldim_init_off:
		kfree(nums);
		str = json_get_obj_str(jsp, child, "init_off", NULL);
		nums_size = (strlen(str)) * sizeof(unsigned int);
		nums = kzalloc(nums_size, GFP_KERNEL);
		if (!nums) {
			LDIMPR("ldim find init_on: no memory to save nums\n");
			goto ldim_dev_get_config_from_json_end;
		}

		memset(nums, 0, nums_size);
		cnt = string_to_numbers(str, nums);
		ldim_dev_init_dynamic_load_array(dev_drv, nums, cnt, 0);

		dev_drv->init_loaded = 1;
	}

ldim_dev_get_config_from_json_end:
	kfree(nums);

	return 0;
}

static int ldim_check_config_load(struct ldim_dev_driver_s *drv)
{
	drv->config_load = lcd_panel_config_load_detect(0, drv->key_valid);
	if (drv->config_load == LCD_CONFIG_NONE) {
		LDIMERR("config_load_check error: config_load:%d, key_valid:%d",
			drv->config_load, drv->key_valid);
		return -1;
	}

	return 0;
}

int ldim_dev_get_config(struct ldim_dev_driver_s *dev_drv, struct device_node *np, int index)
{
	unsigned int val;
	phandle pwm_phandle;
	int ret = 0, cnt = 0, i;
	const char *ldim_gpio_str[BL_GPIO_NUM_MAX];
	unsigned char file_type = PANEL_FILE_INVILD;

	ret = of_property_read_u32(np, "key_valid", &val);
	if (ret) {
		LDIMERR("failed to get key_valid\n");
		val = 0;
	}
	dev_drv->key_valid = val;

	ret = of_property_read_u32(np, "ldim_pwm_config", &pwm_phandle);
	if (ret) {
		LDIMERR("failed to get ldim_pwm_config node\n");
		return -1;
	}

	cnt = of_property_read_string_array(dev_drv->dev->of_node, "ldim_dev_gpio_names",
					    ldim_gpio_str, BL_GPIO_NUM_MAX);
	LDIMPR("ldim_dev_gpio_names cnt: %d\n", cnt);
	for (i = 0; i < cnt; i++)
		strncpy(ldim_gpio[i].name, ldim_gpio_str[i], LCD_CPU_GPIO_NAME_MAX);

	if (ldim_check_config_load(dev_drv))
		return -1;

	switch (dev_drv->config_load) {
	case LCD_CONFIG_UKEY:
		ret = ldim_dev_get_config_from_ukey(dev_drv, pwm_phandle);
		break;
	case LCD_CONFIG_DTS:
		ret = ldim_dev_get_config_from_dts(dev_drv, np, index, pwm_phandle);
		break;
	case LCD_CONFIG_FILE:
		file_type = get_lcd_panel_file_type(0);
		if (file_type == PANEL_FILE_JSON)
			ret = ldim_dev_get_config_from_json(dev_drv, pwm_phandle);
		else if (file_type == PANEL_FILE_INI)
			ret = -1; //todo
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}
