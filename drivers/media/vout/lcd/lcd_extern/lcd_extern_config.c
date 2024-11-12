// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/i2c.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/of.h>
#include <linux/amlogic/media/vout/lcd/lcd_extern.h>
#include <linux/amlogic/media/vout/lcd/lcd_unifykey.h>
#include <linux/amlogic/media/vout/lcd/lcd_notify.h>
#include "lcd_extern.h"

static int lcd_extern_init_table_save(struct lcd_extern_config_s *extconf, int flag,
				      unsigned char *table)
{
	if (!extconf || !table) {
		EXTERR("%s: resource error\n", __func__);
		return -1;
	}
	if (flag) {
		kfree(extconf->table_init_on);
		extconf->table_init_on = kzalloc(extconf->table_init_on_cnt, GFP_KERNEL);
		if (!extconf->table_init_on)
			return -1;
		memcpy(extconf->table_init_on, table, extconf->table_init_on_cnt);
	} else {
		kfree(extconf->table_init_off);
		extconf->table_init_off = kzalloc(extconf->table_init_off_cnt, GFP_KERNEL);
		if (!extconf->table_init_off)
			return -1;
		memcpy(extconf->table_init_off, table, extconf->table_init_off_cnt);
	}

	return 0;
}

static int lcd_extern_init_dynamic_load_array(struct lcd_extern_driver_s *edrv,
					      struct lcd_extern_dev_s *edev,
					      unsigned int *buf, int max_len, int flag)
{
	struct lcd_extern_config_s *extconf = &edev->config;
	int i = 0, j, tbl_max, step = 0, ret = 0;
	unsigned char *table, type, size;
	char propname[20];

	if (flag) {
		extconf->table_init_on_cnt = 0;
		tbl_max = LCD_EXTERN_INIT_ON_MAX;
		sprintf(propname, "init_on");
	} else {
		extconf->table_init_off_cnt = 0;
		tbl_max = LCD_EXTERN_INIT_OFF_MAX;
		sprintf(propname, "init_off");
	}
	if (max_len == 0)
		return 0;

	table = kzalloc(tbl_max, GFP_KERNEL);
	if (!table)
		return -1;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		EXTPR("%s: %d: max_len=%d, tbl_max=%d\n",
		      __func__, flag, max_len, tbl_max);
	}

	switch (extconf->type) {
	case LCD_EXTERN_I2C:
	case LCD_EXTERN_SPI:
	case LCD_EXTERN_SIMPLE:
		while (1) {
			if ((i + 2) > max_len) {
				EXTERR("%s: %s: %s: no ending error\n",
				       __func__, extconf->name, propname);
				goto lcd_extern_init_dynamic_load_array_err;
			}
			if ((i + 2) > tbl_max) {
				EXTERR("%s: %s: %s: size out of support (max %d)\n",
					__func__, extconf->name, propname, tbl_max);
				goto lcd_extern_init_dynamic_load_array_err;
			}
			table[i] = buf[i];
			table[i + 1] = buf[i + 1];
			type = table[i];
			size = table[i + 1];
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				EXTPR("[%d]: %s: dev[%d]: %s: step[%d]: type=0x%x, size=%d, i=%d\n",
					edrv->index, __func__, edev->dev_index, propname,
					step, type, size, i);
			}
			i += 2;

			if (type == LCD_EXT_CMD_TYPE_END)
				break;
			if (size == 0)
				goto init_dynamic_i2c_spi_array_next;
			if ((i + size) > max_len) {
				EXTERR("%s: %s size out of support (max_len %d)\n",
				       extconf->name, propname, max_len);
				goto lcd_extern_init_dynamic_load_array_err;
			}
			if ((i + size) > tbl_max) {
				EXTERR("%s: %s: %s: size out of support (max %d)\n",
					__func__, extconf->name, propname, tbl_max);
				goto lcd_extern_init_dynamic_load_array_err;
			}

			/* data */
			for (j = 0; j < size; j++)
				table[i + j] = buf[i + j];
			i += size;

init_dynamic_i2c_spi_array_next:
			step++;
		}
		if (flag)
			extconf->table_init_on_cnt = i;
		else
			extconf->table_init_off_cnt = i;
		break;
	case LCD_EXTERN_MIPI:
		while (1) {
			if ((i + 2) > max_len) {
				EXTERR("%s: get %s array: no ending error\n",
				       extconf->name, propname);
				goto lcd_extern_init_dynamic_load_array_err;
			}
			if ((i + 2) > tbl_max) {
				EXTERR("%s: %s: %s: size out of support (max %d)\n",
					__func__, extconf->name, propname, tbl_max);
				goto lcd_extern_init_dynamic_load_array_err;
			}
			table[i] = buf[i];
			table[i + 1] = buf[i + 1];
			type = table[i];
			size = table[i + 1];
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				EXTPR("[%d]: %s(%d): dev[%d] step[%d]: type=0x%x, size=%d, i=%d\n",
					edrv->index, __func__, flag, edev->dev_index,
					step, type, size, i);
			}
			i += 2;

			if (type == LCD_EXT_CMD_TYPE_END) {
				if (size == 0xff || size == 0)
					break;
				size = 0;
			}
			if (size == 0)
				goto init_dynamic_mipi_array_next;
			if ((i + size) > max_len) {
				EXTERR("%s: %s size out of support (max_len %d)\n",
				       extconf->name, propname, max_len);
				goto lcd_extern_init_dynamic_load_array_err;
			}
			if ((i + size) > tbl_max) {
				EXTERR("%s: %s: %s: size out of support (max %d)\n",
					__func__, extconf->name, propname, tbl_max);
				goto lcd_extern_init_dynamic_load_array_err;
			}

			for (j = 0; j < size; j++)
				table[i + j] = buf[i + j];
			i += size;

init_dynamic_mipi_array_next:
			step++;
		}
		if (flag)
			extconf->table_init_on_cnt = i;
		else
			extconf->table_init_off_cnt = i;
		break;
	default:
		if (flag)
			extconf->table_init_on_cnt = 0;
		else
			extconf->table_init_off_cnt = 0;
		goto lcd_extern_init_dynamic_load_array_err;
	}

	ret = lcd_extern_init_table_save(extconf, flag, table);
	if (ret)
		goto lcd_extern_init_dynamic_load_array_err;
	kfree(table);
	return 0;

lcd_extern_init_dynamic_load_array_err:
	kfree(table);
	return -1;
}

static int lcd_extern_init_fixed_load_array(struct lcd_extern_driver_s *edrv,
					    struct lcd_extern_dev_s *edev,
					    unsigned int *buf, int max_len, int flag)
{
	struct lcd_extern_config_s *extconf = &edev->config;
	unsigned char cmd_size;
	int i = 0, j, tbl_max, ret = 0;
	unsigned char *table;
	char propname[20];

	cmd_size = extconf->cmd_size;
	if (flag) {
		extconf->table_init_on_cnt = 0;
		tbl_max = LCD_EXTERN_INIT_ON_MAX;
		sprintf(propname, "init_on");
	} else {
		extconf->table_init_off_cnt = 0;
		tbl_max = LCD_EXTERN_INIT_OFF_MAX;
		sprintf(propname, "init_off");
	}
	if (max_len == 0)
		return 0;

	table = kzalloc(tbl_max, GFP_KERNEL);
	if (!table)
		return -1;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		EXTPR("%s: %d: max_len=%d, tbl_max=%d\n",
		      __func__, flag, max_len, tbl_max);
	}

	while (1) {
		if ((i + cmd_size) > max_len) {
			EXTERR("%s: %s: no ending error\n", extconf->name, propname);
			goto lcd_extern_init_fixed_load_array_err;
		}
		if ((i + cmd_size) > tbl_max) {
			EXTERR("%s: %s: size out of support (max %d)\n",
				extconf->name, propname, tbl_max);
			goto lcd_extern_init_fixed_load_array_err;
		}

		for (j = 0; j < cmd_size; j++)
			table[i + j] = buf[i + j];
		i += cmd_size;

		if (table[i] == LCD_EXT_CMD_TYPE_END)
			break;
	}

	if (flag)
		extconf->table_init_on_cnt = i;
	else
		extconf->table_init_off_cnt = i;
	ret = lcd_extern_init_table_save(extconf, flag, table);
	if (ret)
		goto lcd_extern_init_fixed_load_array_err;
	kfree(table);
	return 0;

lcd_extern_init_fixed_load_array_err:
	kfree(table);
	return -1;
}

#ifdef CONFIG_OF
struct device_node *aml_lcd_extern_get_dts_child(struct lcd_extern_driver_s *edrv, int index)
{
	char propname[15];
	struct device_node *child;

	sprintf(propname, "extern_%d", index);
	child = of_get_child_by_name(edrv->sub_dev->of_node, propname);
	return child;
}

static int lcd_extern_init_table_handle_dts(struct lcd_extern_driver_s *edrv,
					    struct lcd_extern_dev_s *edev,
					    struct device_node *np)
{
	struct lcd_extern_config_s *extconf = &edev->config;
	int len_on, len_off, init_max;
	unsigned int *init_buf;
	int ret;

	len_on = of_property_count_u32_elems(np, "init_on");
	if (len_on < 0)
		len_on = 0;
	len_off = of_property_count_u32_elems(np, "init_off");
	if (len_off < 0)
		len_off = 0;
	init_max = len_on >= len_off ? len_on : len_off;
	if (init_max <= 0)
		return 0;

	init_buf = kcalloc(init_max, sizeof(unsigned int), GFP_KERNEL);
	if (!init_buf)
		return -1;

	//init_on
	ret = of_property_read_u32_array(np, "init_on", init_buf, len_on);
	if (ret) {
		EXTERR("%s: get init_on failed\n", extconf->name);
		goto lcd_extern_init_table_handle_dts_err;
	}
	if (extconf->cmd_size == LCD_EXT_CMD_SIZE_DYNAMIC)
		ret = lcd_extern_init_dynamic_load_array(edrv, edev, init_buf, len_on, 1);
	else
		ret = lcd_extern_init_fixed_load_array(edrv, edev, init_buf, len_on, 1);
	if (ret)
		goto lcd_extern_init_table_handle_dts_err;

	//init_off
	ret = of_property_read_u32_array(np, "init_off", init_buf, len_off);
	if (ret) {
		EXTERR("%s: get init_off failed\n", extconf->name);
		goto lcd_extern_init_table_handle_dts_err;
	}
	if (extconf->cmd_size == LCD_EXT_CMD_SIZE_DYNAMIC)
		ret = lcd_extern_init_dynamic_load_array(edrv, edev, init_buf, len_off, 0);
	else
		ret = lcd_extern_init_fixed_load_array(edrv, edev, init_buf, len_off, 0);
	if (ret)
		goto lcd_extern_init_table_handle_dts_err;

	extconf->table_init_loaded = 1;

	kfree(init_buf);
	return 0;

lcd_extern_init_table_handle_dts_err:
	kfree(init_buf);
	return -1;
}

static int lcd_extern_get_config_dts(struct device_node *np,
				     struct lcd_extern_driver_s *edrv,
				     struct lcd_extern_dev_s *edev)
{
	struct lcd_extern_config_s *econf;
	char snode[15];
	struct device_node *child;
	const char *str;
	unsigned int val;
	int ret = 0;

	sprintf(snode, "extern_%d", edev->dev_index);
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		EXTPR("[%d]: %s: %s\n", edrv->index, __func__, snode);

	child = of_get_child_by_name(np, snode);
	if (!child) {
		EXTERR("[%d]: failed to get %s\n", edrv->index, snode);
		return -1;
	}

	econf = &edev->config;

	ret = of_property_read_u32(child, "index", &val);
	if (ret) {
		EXTERR("get index failed, exit\n");
		return -1;
	}
	econf->index = (unsigned char)val;

	ret = of_property_read_string(child, "status", &str);
	if (ret) {
		EXTERR("[%d]: get index %d status failed\n", edrv->index, econf->index);
		return -1;
	}
	if (strncmp(str, "okay", 2) == 0)
		econf->status = 1;

	ret = of_property_read_string(child, "extern_name", &str);
	if (ret) {
		EXTERR("[%d]: get extern_name failed\n", edrv->index);
		strncpy(econf->name, "none", (LCD_EXTERN_NAME_LEN_MAX - 1));
	} else {
		strncpy(econf->name, str, (LCD_EXTERN_NAME_LEN_MAX - 1));
	}

	ret = of_property_read_u32(child, "type", &econf->type);
	if (ret) {
		econf->type = LCD_EXTERN_MAX;
		EXTERR("[%d]: %s: get type failed, exit\n", edrv->index, econf->name);
		return -1;
	}

	if (econf->status == 0 || econf->index == LCD_EXTERN_INDEX_INVALID) {
		EXTERR("[%d]: %s[%d] status %d is disabled, or index %d is invalid\n",
		       edrv->index, econf->name, edev->dev_index, econf->status, econf->index);
		return -1;
	}
	EXTPR("[%d]: config from dts: %s[%d], index: %d, type: %d\n",
		edrv->index, econf->name, edev->dev_index, econf->index, econf->type);

	switch (econf->type) {
	case LCD_EXTERN_I2C:
		ret = of_property_read_u32(child, "i2c_address", &val);
		if (ret) {
			EXTERR("[%d]: %s: get i2c_address failed, exit\n",
			       edrv->index, econf->name);
			econf->i2c_addr = LCD_EXT_I2C_ADDR_INVALID;
			return -1;
		}
		econf->i2c_addr = (unsigned char)val;
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			EXTPR("[%d]: %s: i2c_address = 0x%02x\n",
			      edrv->index, econf->name, econf->i2c_addr);
		}
		ret = of_property_read_u32(child, "i2c_address2", &val);
		if (ret) {
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				EXTPR("[%d]: %s: no i2c_address2 exist\n",
				      edrv->index, econf->name);
			}
			econf->i2c_addr2 = LCD_EXT_I2C_ADDR_INVALID;
		} else {
			econf->i2c_addr2 = (unsigned char)val;
		}
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			EXTPR("[%d]: %s: i2c_address2 = 0x%02x\n",
			      edrv->index, econf->name, econf->i2c_addr2);
		}
		ret = of_property_read_u32(child, "i2c_address3", &val);
		if (ret) {
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				EXTPR("[%d]: %s: no i2c_address3 exist\n",
				      edrv->index, econf->name);
			}
			econf->i2c_addr3 = LCD_EXT_I2C_ADDR_INVALID;
		} else {
			econf->i2c_addr3 = (unsigned char)val;
		}
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			EXTPR("[%d]: %s: i2c_address3 = 0x%02x\n",
			      edrv->index, econf->name, econf->i2c_addr3);
		}
		ret = of_property_read_u32(child, "i2c_address4", &val);
		if (ret) {
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				EXTPR("[%d]: %s: no i2c_address4 exist\n",
				      edrv->index, econf->name);
			}
			econf->i2c_addr4 = LCD_EXT_I2C_ADDR_INVALID;
		} else {
			econf->i2c_addr4 = (unsigned char)val;
		}
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			EXTPR("[%d]: %s: i2c_address4 = 0x%02x\n",
			      edrv->index, econf->name, econf->i2c_addr4);
		}

		ret = of_property_read_u32(child, "cmd_size", &val);
		if (ret) {
			EXTPR("[%d]: %s: no cmd_size\n", edrv->index, econf->name);
			econf->cmd_size = 0;
		} else {
			econf->cmd_size = (unsigned char)val;
		}
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			EXTPR("[%d]: %s: cmd_size = %d\n",
			      edrv->index, econf->name, econf->cmd_size);
		}
		if (econf->cmd_size == 0)
			break;
		ret = lcd_extern_init_table_handle_dts(edrv, edev, child);
		break;
	case LCD_EXTERN_SPI:
		ret = of_property_read_u32(child, "gpio_spi_cs", &val);
		if (ret) {
			EXTERR("[%d]: %s: get gpio_spi_cs failed, exit\n",
			       edrv->index, econf->name);
			econf->spi_gpio_cs = LCD_EXT_GPIO_INVALID;
			return -1;
		}
		econf->spi_gpio_cs = val;
		lcd_extern_gpio_probe(edrv, val);
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			EXTPR("[%d]: %s: spi_gpio_cs: %d\n",
			      edrv->index, econf->name, econf->spi_gpio_cs);
		}
		ret = of_property_read_u32(child, "gpio_spi_clk", &val);
		if (ret) {
			EXTERR("[%d]: %s: get gpio_spi_clk failed, exit\n",
			       edrv->index, econf->name);
			econf->spi_gpio_clk = LCD_EXT_GPIO_INVALID;
			return -1;
		}
		econf->spi_gpio_clk = val;
		lcd_extern_gpio_probe(edrv, val);
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			EXTPR("[%d]: %s: spi_gpio_clk: %d\n",
			      edrv->index, econf->name, econf->spi_gpio_clk);
		}
		ret = of_property_read_u32(child, "gpio_spi_data", &val);
		if (ret) {
			EXTERR("[%d]: %s: get gpio_spi_data failed, exit\n",
			       edrv->index, econf->name);
			econf->spi_gpio_data = LCD_EXT_GPIO_INVALID;
			return -1;
		}
		econf->spi_gpio_data = val;
		lcd_extern_gpio_probe(edrv, val);
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			EXTPR("[%d]: %s: spi_gpio_data: %d\n",
			      edrv->index, econf->name, econf->spi_gpio_data);
		}
		ret = of_property_read_u32(child, "spi_clk_freq", &val);
		if (ret) {
			EXTERR("[%d]: %s: get spi_clk_freq failed, default to %dKHz\n",
			       edrv->index, econf->name, LCD_EXT_SPI_CLK_FREQ_DFT);
			econf->spi_clk_freq = LCD_EXT_SPI_CLK_FREQ_DFT;
		} else {
			econf->spi_clk_freq = val;
		}
		ret = of_property_read_u32(child, "spi_clk_pol", &val);
		if (ret) {
			EXTERR("[%d]: %s: get spi_clk_pol failed, default to 1\n",
			       edrv->index, econf->name);
			econf->spi_clk_pol = 1;
		} else {
			econf->spi_clk_pol = (unsigned char)val;
		}
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			EXTPR("[%d]: %s: spi_clk_freq: %dKHz, spi_clk_pol: %d\n",
			      edrv->index, econf->name, econf->spi_clk_freq,
			      econf->spi_clk_pol);
		}
		ret = of_property_read_u32(child, "cmd_size", &val);
		if (ret) {
			EXTPR("[%d]: %s: no cmd_size\n", edrv->index, econf->name);
			econf->cmd_size = 0;
		} else {
			econf->cmd_size = (unsigned char)val;
		}
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			EXTPR("[%d]: %s: cmd_size: %d\n",
			      edrv->index, econf->name, econf->cmd_size);
		}
		if (econf->cmd_size == 0)
			break;
		ret = lcd_extern_init_table_handle_dts(edrv, edev, child);
		break;
	case LCD_EXTERN_MIPI:
		ret = of_property_read_u32(child, "cmd_size", &val);
		if (ret) {
			EXTPR("[%d]: %s: no cmd_size\n", edrv->index, econf->name);
			econf->cmd_size = 0;
		} else {
			econf->cmd_size = (unsigned char)val;
		}
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			EXTPR("[%d]: %s: cmd_size = %d\n",
			      edrv->index, econf->name, econf->cmd_size);
		}
		if (econf->cmd_size != LCD_EXT_CMD_SIZE_DYNAMIC)
			break;
		ret = lcd_extern_init_table_handle_dts(edrv, edev, child);
		break;
	case LCD_EXTERN_SIMPLE:
		ret = of_property_read_u32(child, "cmd_size", &val);
		if (ret) {
			EXTPR("[%d]: %s: no cmd_size\n", edrv->index, econf->name);
			econf->cmd_size = 0;
		} else {
			econf->cmd_size = (unsigned char)val;
		}
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			EXTPR("[%d]: %s: cmd_size: %d\n",
			      edrv->index, econf->name, econf->cmd_size);
		}
		if (econf->cmd_size == 0)
			break;
		ret = lcd_extern_init_table_handle_dts(edrv, edev, child);
		break;
	default:
		break;
	}

	return ret;
}
#endif

static int lcd_extern_init_table_handle_ukey(struct lcd_extern_driver_s *edrv,
					     struct lcd_extern_dev_s *edev,
					     unsigned char *p, int key_len)
{
	unsigned int *init_buf;
	int init_offset, init_max;
	int i, ret;

	init_offset = LCD_UKEY_EXT_INIT;
	init_max = key_len - LCD_UKEY_EXT_INIT;
	if (init_max <= 0)
		return 0;

	init_buf = kcalloc(init_max, sizeof(unsigned int), GFP_KERNEL);
	if (!init_buf)
		return -1;
	for (i = 0; i < init_max; i++)
		init_buf[i] = *(p + init_offset + i);
	if (edev->config.cmd_size == LCD_EXT_CMD_SIZE_DYNAMIC)
		ret = lcd_extern_init_dynamic_load_array(edrv, edev, init_buf, init_max, 1);
	else
		ret = lcd_extern_init_fixed_load_array(edrv, edev, init_buf, init_max, 1);
	if (ret)
		goto lcd_extern_init_table_handle_ukey_err;

	init_offset += edev->config.table_init_on_cnt;
	init_max -= edev->config.table_init_on_cnt;
	if (init_max > 0) {
		for (i = 0; i < init_max; i++)
			init_buf[i] = *(p + init_offset + i);
		if (edev->config.cmd_size == LCD_EXT_CMD_SIZE_DYNAMIC)
			ret = lcd_extern_init_dynamic_load_array(edrv, edev, init_buf, init_max, 0);
		else
			ret = lcd_extern_init_fixed_load_array(edrv, edev, init_buf, init_max, 0);
		if (ret)
			goto lcd_extern_init_table_handle_ukey_err;
	} else {
		edev->config.table_init_off_cnt = 0;
	}

	edev->config.table_init_loaded = 1;

	kfree(init_buf);
	return 0;

lcd_extern_init_table_handle_ukey_err:
	kfree(init_buf);
	return -1;
}

static int lcd_extern_get_config_ukey(struct lcd_extern_driver_s *edrv,
				      struct lcd_extern_dev_s *edev)
{
	struct lcd_extern_config_s *econf;
	unsigned char *para, *p;
	int key_len, len;
	const char *str;
	int ret = 0;

	if (!edev)
		return -1;

	econf = &edev->config;
	ret = lcd_unifykey_get_size(edrv->ukey_name, &key_len);
	if (ret)
		return -1;

	para = kcalloc(key_len, sizeof(unsigned char), GFP_KERNEL);
	if (!para)
		return -1;

	ret = lcd_unifykey_get(edrv->ukey_name, para, key_len);
	if (ret) {
		kfree(para);
		return -1;
	}

	/* check lcd_extern unifykey length */
	len = 10 + 33 + 10;
	ret = lcd_unifykey_len_check(key_len, len);
	if (ret) {
		EXTERR("[%d]: ukey %s length is incorrect\n", edrv->index, edrv->ukey_name);
		kfree(para);
		return -1;
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		lcd_unifykey_header_print(para);

	/* basic: 33byte */
	p = para;
	str = (const char *)(p + LCD_UKEY_HEAD_SIZE);
	strncpy(econf->name, str, (LCD_EXTERN_NAME_LEN_MAX - 1));
	econf->index = *(p + LCD_UKEY_EXT_INDEX);
	econf->type = *(p + LCD_UKEY_EXT_TYPE);
	econf->status = *(p + LCD_UKEY_EXT_STATUS);
	if (econf->status == 0 || econf->index == LCD_EXTERN_INDEX_INVALID) {
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			EXTERR("[%d]: %s[%d] status %d is disabled, or index %d is invalid\n",
				edrv->index, econf->name, edev->dev_index,
				econf->status, econf->index);
		}
		kfree(para);
		return 1;
	}

	EXTPR("[%d]: config from ukey: %s[%d], index: %d, type: %d\n",
		edrv->index, econf->name, edev->dev_index, econf->index, econf->type);

	/* type: 10byte */
	switch (econf->type) {
	case LCD_EXTERN_I2C:
		if (*(p + LCD_UKEY_EXT_TYPE_VAL_0))
			econf->i2c_addr = *(p + LCD_UKEY_EXT_TYPE_VAL_0);
		else
			econf->i2c_addr = LCD_EXT_I2C_ADDR_INVALID;
		if (*(p + LCD_UKEY_EXT_TYPE_VAL_1))
			econf->i2c_addr2 = *(p + LCD_UKEY_EXT_TYPE_VAL_1);
		else
			econf->i2c_addr2 = LCD_EXT_I2C_ADDR_INVALID;
		if (*(p + LCD_UKEY_EXT_TYPE_VAL_4))
			econf->i2c_addr3 = *(p + LCD_UKEY_EXT_TYPE_VAL_4);
		else
			econf->i2c_addr3 = LCD_EXT_I2C_ADDR_INVALID;
		if (*(p + LCD_UKEY_EXT_TYPE_VAL_5))
			econf->i2c_addr4 = *(p + LCD_UKEY_EXT_TYPE_VAL_5);
		else
			econf->i2c_addr4 = LCD_EXT_I2C_ADDR_INVALID;

		econf->cmd_size = *(p + LCD_UKEY_EXT_TYPE_VAL_3);

		/* init */
		if (econf->cmd_size == 0)
			break;
		ret = lcd_extern_init_table_handle_ukey(edrv, edev, p, key_len);
		break;
	case LCD_EXTERN_SPI:
		econf->spi_gpio_cs = *(p + LCD_UKEY_EXT_TYPE_VAL_0);
		lcd_extern_gpio_probe(edrv, econf->spi_gpio_cs);
		econf->spi_gpio_clk = *(p + LCD_UKEY_EXT_TYPE_VAL_1);
		lcd_extern_gpio_probe(edrv, econf->spi_gpio_clk);
		econf->spi_gpio_data = *(p + LCD_UKEY_EXT_TYPE_VAL_2);
		lcd_extern_gpio_probe(edrv, econf->spi_gpio_data);
		econf->spi_clk_freq = (*(p + LCD_UKEY_EXT_TYPE_VAL_3) |
			((*(p + LCD_UKEY_EXT_TYPE_VAL_4)) << 8));
		econf->spi_clk_pol = *(p + LCD_UKEY_EXT_TYPE_VAL_5);
		econf->cmd_size = *(p + LCD_UKEY_EXT_TYPE_VAL_6);

		/* init */
		if (econf->cmd_size == 0)
			break;
		ret = lcd_extern_init_table_handle_ukey(edrv, edev, p, key_len);
		break;
	case LCD_EXTERN_MIPI:
		econf->cmd_size = *(p + LCD_UKEY_EXT_TYPE_VAL_9);
		if (econf->cmd_size != LCD_EXT_CMD_SIZE_DYNAMIC)
			break;
		ret = lcd_extern_init_table_handle_ukey(edrv, edev, p, key_len);
		break;
	case LCD_EXTERN_SIMPLE:
		econf->cmd_size = *(p + LCD_UKEY_EXT_TYPE_VAL_9);

		/* init */
		if (econf->cmd_size == 0)
			break;
		ret = lcd_extern_init_table_handle_ukey(edrv, edev, p, key_len);
		break;
	default:
		goto lcd_ext_get_config_ukey_end;
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		EXTPR("[%d]: %s: cmd_size = %d\n", edrv->index, econf->name, econf->cmd_size);

lcd_ext_get_config_ukey_end:
	kfree(para);
	return ret;
}

static int lcd_extern_dev_probe_ukey(struct lcd_extern_driver_s *edrv)
{
	int dev_index, ret;

	dev_index = 0;
	edrv->dev[edrv->dev_cnt] = lcd_extern_dev_malloc(dev_index);
	ret = lcd_extern_get_config_ukey(edrv, edrv->dev[edrv->dev_cnt]);
	if (ret < 0)
		goto lcd_ext_dev_probe_ukey_err;
	if (ret > 0) {
		lcd_extern_dev_free(edrv->dev[edrv->dev_cnt]);
		edrv->dev[edrv->dev_cnt] = NULL;
		return -1;
	}

	lcd_extern_config_update(edrv, edrv->dev[edrv->dev_cnt]);
	ret = lcd_extern_add_dev(edrv, edrv->dev[edrv->dev_cnt]);
	if (ret)
		goto lcd_ext_dev_probe_ukey_err;

	edrv->dev_cnt++;
	lcd_resource_ready(edrv->index, LCD_RES_EXTERN, dev_index);

	return 0;

lcd_ext_dev_probe_ukey_err:
	EXTPR("[%d]: %s: failed\n", edrv->index, __func__);
	lcd_extern_dev_free(edrv->dev[edrv->dev_cnt]);
	edrv->dev[edrv->dev_cnt] = NULL;
	return -1;
}

static void lcd_extern_dev_probe_work(struct work_struct *p_work)
{
	struct delayed_work *d_work;
	struct lcd_extern_driver_s *edrv;
	bool is_init;
	int ret;

	d_work = container_of(p_work, struct delayed_work, work);
	edrv = container_of(d_work, struct lcd_extern_driver_s, dev_probe_dly_work);

	is_init = lcd_unifykey_init_get();
	if (!is_init) {
		if (edrv->retry_cnt++ < LCD_UNIFYKEY_WAIT_TIMEOUT) {
			lcd_queue_delayed_work(&edrv->dev_probe_dly_work,
				LCD_UNIFYKEY_RETRY_INTERVAL);
			return;
		}
		EXTERR("[%d]: %s: key_init_flag=%d, timeout\n",
			edrv->index, __func__, is_init);
		return;
	}

	ret = lcd_unifykey_check(edrv->ukey_name);
	if (ret)
		return;

	lcd_extern_dev_probe_ukey(edrv);
}

int lcd_extern_config_load(struct lcd_extern_driver_s *edrv, int *ext_index_lut, int dev_cnt)
{
	struct device_node *np;
	unsigned int para[5];
	const char *str;
	int dev_index, i;
	int ret;

	if (!edrv->pdev->dev.of_node) {
		EXTERR("[%d]: %s: dev of_node is null\n", edrv->index, __func__);
		return -1;
	}
	np = edrv->pdev->dev.of_node;

	ret = of_property_read_string(np, "i2c_bus", &str);
	if (ret)
		edrv->i2c_bus = LCD_EXT_I2C_BUS_MAX;
	else
		edrv->i2c_bus = aml_lcd_i2c_bus_get_str(str);

	ret = of_property_read_string(np, "pinctrl-names", &str);
	if (ret)
		edrv->pinmux_valid = 0;
	else
		edrv->pinmux_valid = 1;
	edrv->pinmux_flag = 0xff;

	ret = of_property_read_u32(np, "key_valid", &para[0]);
	if (ret) {
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
			EXTPR("failed to get key_valid\n");
		edrv->key_valid = 0;
	} else {
		edrv->key_valid = (unsigned char)para[0];
	}

	if (edrv->index == 0)
		sprintf(edrv->ukey_name, "lcd_extern");
	else
		sprintf(edrv->ukey_name, "lcd%d_extern", edrv->index);

	if (edrv->key_valid) {
		edrv->config_load = 1;
		INIT_DELAYED_WORK(&edrv->dev_probe_dly_work, lcd_extern_dev_probe_work);
		if (lcd_unifykey_init_get())
			lcd_extern_dev_probe_ukey(edrv);
		else
			lcd_queue_delayed_work(&edrv->dev_probe_dly_work, 0);
	} else {
		edrv->config_load = 0;
		for (i = 0; i < dev_cnt; i++) {
			dev_index = ext_index_lut[i];
			if (dev_index == LCD_EXTERN_INDEX_INVALID) {
				EXTPR("[%d]: %s: invalid dev_index\n", edrv->index, __func__);
				continue;
			}
			edrv->dev[edrv->dev_cnt] = lcd_extern_dev_malloc(dev_index);
			if (!edrv->dev[edrv->dev_cnt])
				continue;

			ret = lcd_extern_get_config_dts(np, edrv, edrv->dev[edrv->dev_cnt]);
			if (ret) {
				lcd_extern_dev_free(edrv->dev[edrv->dev_cnt]);
				edrv->dev[edrv->dev_cnt] = NULL;
				continue;
			}

			lcd_extern_config_update(edrv, edrv->dev[edrv->dev_cnt]);
			ret = lcd_extern_add_dev(edrv, edrv->dev[edrv->dev_cnt]);
			if (ret) {
				lcd_extern_dev_free(edrv->dev[edrv->dev_cnt]);
				edrv->dev[edrv->dev_cnt] = NULL;
				return -1;
			}
			edrv->dev_cnt++;
			lcd_resource_ready(edrv->index, LCD_RES_EXTERN, dev_index);
		}
	}

	return 0;
}
