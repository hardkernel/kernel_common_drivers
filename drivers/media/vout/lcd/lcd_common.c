// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 *
 * Copyright (C) 2019 Amlogic, Inc. All rights reserved.
 *
 */

#include <linux/init.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_reserved_mem.h>
#include <linux/cma.h>
#include <linux/dma-map-ops.h>
#include <linux/dma-mapping.h>
#include <linux/reset.h>
#include <linux/clk.h>
#include <linux/amlogic/media/vout/lcd/aml_lcd.h>
#include <linux/amlogic/media/vout/lcd/lcd_vout.h>
#include <linux/amlogic/media/vout/lcd/lcd_extern.h>
#include <linux/amlogic/media/vout/lcd/lcd_notify.h>
#include <linux/amlogic/media/vout/lcd/lcd_unifykey.h>
#include <linux/amlogic/media/vout/lcd/aml_bl.h>
#include <linux/amlogic/media/vout/vinfo.h>
#include <linux/amlogic/media/vout/lcd/lcd_resman.h>
#include <linux/page-flags.h>
#include <linux/mm.h>
#include "lcd_common.h"
#include "lcd_reg.h"
#include "lcd_clk/lcd_clk_config.h"

void lcd_delay_us(int us)
{
	if (!us)
		return;
	else if (us < 20)
		udelay(us);
	else if (us < 1000)
		usleep_range(us, us + 2);
	else if (us < 20000)
		usleep_range(us, us + 100);
	else
		msleep(us / 1000);
}

void lcd_delay_ms(int ms)
{
	if (!ms)
		return;
	else if (ms < 20)
		usleep_range(ms * 1000, ms * 1000 + 100);
	else
		msleep(ms);
}

static struct lcd_i2c_match_s lcd_i2c_match_table[] = {
	{LCD_EXT_I2C_BUS_0,   "i2c_0"},
	{LCD_EXT_I2C_BUS_1,   "i2c_1"},
	{LCD_EXT_I2C_BUS_2,   "i2c_2"},
	{LCD_EXT_I2C_BUS_3,   "i2c_3"},
	{LCD_EXT_I2C_BUS_4,   "i2c_4"},
	{LCD_EXT_I2C_BUS_0,   "i2c_a"},
	{LCD_EXT_I2C_BUS_1,   "i2c_b"},
	{LCD_EXT_I2C_BUS_2,   "i2c_c"},
	{LCD_EXT_I2C_BUS_3,   "i2c_d"},
	{LCD_EXT_I2C_BUS_4,   "i2c_ao"},
	{LCD_EXT_I2C_BUS_0,   "i2c_bus_0"},
	{LCD_EXT_I2C_BUS_1,   "i2c_bus_1"},
	{LCD_EXT_I2C_BUS_2,   "i2c_bus_2"},
	{LCD_EXT_I2C_BUS_3,   "i2c_bus_3"},
	{LCD_EXT_I2C_BUS_4,   "i2c_bus_4"},
	{LCD_EXT_I2C_BUS_0,   "i2c_bus_a"},
	{LCD_EXT_I2C_BUS_1,   "i2c_bus_b"},
	{LCD_EXT_I2C_BUS_2,   "i2c_bus_c"},
	{LCD_EXT_I2C_BUS_3,   "i2c_bus_d"},
	{LCD_EXT_I2C_BUS_4,   "i2c_bus_ao"},
	{LCD_EXT_I2C_BUS_MAX, "invalid"}
};

unsigned char aml_lcd_i2c_bus_get_str(const char *str)
{
	unsigned char i2c_bus = LCD_EXT_I2C_BUS_MAX;
	int i;

	for (i = 0; i < ARRAY_SIZE(lcd_i2c_match_table); i++) {
		if (strcmp(lcd_i2c_match_table[i].bus_str, str) == 0) {
			i2c_bus = lcd_i2c_match_table[i].bus_id;
			break;
		}
	}

	if (i2c_bus == LCD_EXT_I2C_BUS_MAX)
		LCDERR("%s: invalid i2c_bus: %s\n", __func__, str);

	return i2c_bus;
}

/* **********************************
 * lcd type
 * **********************************
 */
struct lcd_type_match_s {
	char *name;
	enum lcd_type_e type;
};

static struct lcd_type_match_s lcd_type_match_table[] = {
	{"rgb",      LCD_RGB},
	{"lvds",     LCD_LVDS},
	{"vbyone",   LCD_VBYONE},
	{"mipi",     LCD_MIPI},
	{"minilvds", LCD_MLVDS},
	{"p2p",      LCD_P2P},
	{"edp",      LCD_EDP},
	{"bt656",    LCD_BT656},
	{"bt1120",   LCD_BT1120},
	{"invalid",  LCD_TYPE_MAX},
};

int lcd_type_str_to_type(const char *str)
{
	int type = LCD_TYPE_MAX;
	int i;

	for (i = 0; i < ARRAY_SIZE(lcd_type_match_table); i++) {
		if (!strcmp(str, lcd_type_match_table[i].name)) {
			type = lcd_type_match_table[i].type;
			break;
		}
	}
	return type;
}

char *lcd_type_type_to_str(int type)
{
	char *name = lcd_type_match_table[LCD_TYPE_MAX].name;
	int i;

	for (i = 0; i < ARRAY_SIZE(lcd_type_match_table); i++) {
		if (type == lcd_type_match_table[i].type) {
			name = lcd_type_match_table[i].name;
			break;
		}
	}
	return name;
}

static char *lcd_mode_table[] = {
	"tv",
	"tablet",
	"invalid",
};

unsigned char lcd_mode_str_to_mode(const char *str)
{
	unsigned char mode;

	for (mode = 0; mode < ARRAY_SIZE(lcd_mode_table); mode++) {
		if (!strcmp(str, lcd_mode_table[mode]))
			break;
	}
	return mode;
}

char *lcd_mode_mode_to_str(int mode)
{
	return lcd_mode_table[mode];
}

void lcd_dbg_mem_dump(void *addr, size_t size)
{
	int i = 0, j = 0, k = 0, len = 0;
	unsigned char buf[128], *p = (unsigned char *)addr;

	if (!addr)
		return;
	LCDPR("memory dump addr:%px, size:0x%x\n", addr, (unsigned int)size);
	for (j = 0; j < size / 16; j++) {
		len = 0;
		for (i = 0; i < 16; i++, k++)
			len += sprintf(buf, "%02x ", p[k]);
		LCDPR("0x%04x:%s\n", j * 0x10, buf);
	}
	for (i = 0, len = 0; i < size % 16; i++, k++)
		len += sprintf(buf, "%02x ", p[k]);
	LCDPR("0x%04x:%s\n", j * 0x10, buf);
}

int __of_reserved_mem_device_init_by_name(struct device *dev,
					struct device_node *np,
					const char *name)
{
	int idx = of_property_match_string(np, "memory-region-names", name);

	return of_reserved_mem_device_init_by_idx(dev, np, idx);
}

void lcd_cma_pool_init(struct aml_lcd_drv_s *pdrv, struct platform_device *pdev)
{
	struct device *dev;
	int ret;
	char rsv_mem_name[64];

	if (!pdev || !pdrv)
		return;

	if (pdrv->index)
		sprintf(rsv_mem_name, "lcd%d_cma_reserved", pdrv->index);
	else
		sprintf(rsv_mem_name, "lcd_cma_reserved");
	dev = &pdrv->pdev->dev;
	ret = __of_reserved_mem_device_init_by_name(dev, dev->of_node, rsv_mem_name);
	if (ret == 0) {
		pdrv->lcd_cma_ready = 1;
		//dma_set_coherent_mask(dev, 0xffffffff);//4G
	} else {
		pdrv->lcd_cma_ready = -1;
		LCDPR("warning:lcd cma init failed\n");
	}
}

void *lcd_alloc_dma_buffer(struct aml_lcd_drv_s *pdrv, unsigned int size, dma_addr_t *paddr)
{
	if (!pdrv || !pdrv->pdev)
		return NULL;

	if (pdrv->lcd_cma_ready == 0)
		lcd_cma_pool_init(pdrv, pdrv->pdev);

	return dma_alloc_coherent(&pdrv->pdev->dev, size, paddr, GFP_KERNEL);
}

u8 *lcd_vmap(ulong addr, u32 size)
{
	u8 *vaddr = NULL;
	struct page **pages = NULL;
	u32 i, npages, offset = 0;
	ulong phys, page_start;
	/*pgprot_t pgprot = pgprot_noncached(PAGE_KERNEL);*/
	pgprot_t pgprot = PAGE_KERNEL;

	if (!PageHighMem(phys_to_page(addr)))
		return phys_to_virt(addr);

	offset = offset_in_page(addr);
	page_start = addr - offset;
	npages = DIV_ROUND_UP(size + offset, PAGE_SIZE);

	pages = kmalloc_array(npages, sizeof(struct page *), GFP_KERNEL);
	if (!pages)
		return NULL;
	for (i = 0; i < npages; i++) {
		phys = page_start + i * PAGE_SIZE;
		pages[i] = pfn_to_page(phys >> PAGE_SHIFT);
	}

	vaddr = vmap(pages, npages, VM_MAP, pgprot);
	if (!vaddr) {
		LCDERR("the phy(%lx) vmaped fail, size: %d\n",
		       page_start, npages << PAGE_SHIFT);
		kfree(pages);
		return NULL;
	}
	kfree(pages);

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("[lcd HIGH-MEM-MAP] %s, pa(%lx) to va(%p), size: %d\n",
		      __func__, page_start, vaddr, npages << PAGE_SHIFT);
	}

	return vaddr + offset;
}

void lcd_unmap_phyaddr(u8 *vaddr)
{
	void *addr = (void *)(PAGE_MASK & (ulong)vaddr);

	if (is_vmalloc_or_module_addr(vaddr)) {
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
			LCDPR("lcd unmap v: %p\n", addr);
		vunmap(addr);
	}
}

/* **********************************
 * lcd gpio
 * **********************************
 */

void lcd_cpu_gpio_probe(struct aml_lcd_drv_s *pdrv, unsigned int index)
{
	struct lcd_cpu_gpio_s *cpu_gpio;
	const char *str;
	int ret;

	if (!pdrv->dev->of_node) {
		LCDERR("[%d]: %s: dev of_node is null\n", pdrv->index, __func__);
		return;
	}

	if (index >= LCD_CPU_GPIO_NUM_MAX) {
		LCDERR("[%d]: gpio index %d, exit\n", pdrv->index, index);
		return;
	}
	cpu_gpio = &pdrv->config.power.cpu_gpio[index];
	if (cpu_gpio->probe_flag) {
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			LCDPR("[%d]: gpio %s[%d] is already registered\n",
			      pdrv->index, cpu_gpio->name, index);
		}
		return;
	}

	/* get gpio name */
	ret = of_property_read_string_index(pdrv->dev->of_node, "lcd_cpu_gpio_names", index, &str);
	if (ret) {
		LCDERR("[%d]: failed to get lcd_cpu_gpio_names: %d\n",
		       pdrv->index, index);
		str = "unknown";
	}
	strcpy(cpu_gpio->name, str);

	/* init gpio flag */
	cpu_gpio->probe_flag = 1;
	cpu_gpio->register_flag = 0;
}

static int lcd_cpu_gpio_register(struct aml_lcd_drv_s *pdrv, unsigned int index, int init_value)
{
	struct lcd_cpu_gpio_s *cpu_gpio;
	int value;

	if (index >= LCD_CPU_GPIO_NUM_MAX) {
		LCDERR("[%d]: %s: gpio index %d, exit\n",
		       pdrv->index, __func__, index);
		return -1;
	}
	cpu_gpio = &pdrv->config.power.cpu_gpio[index];
	if (cpu_gpio->probe_flag == 0) {
		LCDERR("[%d]: %s: gpio [%d] is not probed, exit\n",
		       pdrv->index, __func__, index);
		return -1;
	}
	if (cpu_gpio->register_flag) {
		LCDPR("[%d]: %s: gpio %s[%d] is already registered\n",
		      pdrv->index, __func__, cpu_gpio->name, index);
		return 0;
	}

	switch (init_value) {
	case LCD_GPIO_OUTPUT_LOW:
		value = GPIOD_OUT_LOW;
		break;
	case LCD_GPIO_OUTPUT_HIGH:
		value = GPIOD_OUT_HIGH;
		break;
	case LCD_GPIO_INPUT:
	default:
		value = GPIOD_IN;
		break;
	}

	/* request gpio */
	cpu_gpio->gpio = devm_gpiod_get_index(pdrv->dev, "lcd_cpu", index, value);
	if (IS_ERR_OR_NULL(cpu_gpio->gpio)) {
		LCDERR("[%d]: register gpio %s[%d]: %p, err: %d\n",
		       pdrv->index, cpu_gpio->name, index, cpu_gpio->gpio,
		       IS_ERR(cpu_gpio->gpio));
		return -1;
	}
	cpu_gpio->register_flag = 1;
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("[%d]: register gpio %s[%d]: %p, init value: %d\n",
		      pdrv->index, cpu_gpio->name, index,
		      cpu_gpio->gpio, init_value);
	}

	return 0;
}

void lcd_cpu_gpio_set(struct aml_lcd_drv_s *pdrv, unsigned int index, int value)
{
	struct lcd_cpu_gpio_s *cpu_gpio;

	if (index >= LCD_CPU_GPIO_NUM_MAX) {
		LCDERR("[%d]: gpio index %d, exit\n", pdrv->index, index);
		return;
	}
	cpu_gpio = &pdrv->config.power.cpu_gpio[index];
	if (cpu_gpio->probe_flag == 0) {
		LCDERR("[%d]: %s: gpio [%d] is not probed, exit\n",
		       pdrv->index, __func__, index);
		return;
	}
	if (cpu_gpio->register_flag == 0) {
		lcd_cpu_gpio_register(pdrv, index, value);
		return;
	}

	if (IS_ERR_OR_NULL(cpu_gpio->gpio)) {
		LCDERR("[%d]: gpio %s[%d]: %p, err: %ld\n",
		       pdrv->index, cpu_gpio->name, index, cpu_gpio->gpio,
		       PTR_ERR(cpu_gpio->gpio));
		return;
	}

	switch (value) {
	case LCD_GPIO_OUTPUT_LOW:
	case LCD_GPIO_OUTPUT_HIGH:
		gpiod_direction_output(cpu_gpio->gpio, value);
		break;
	case LCD_GPIO_INPUT:
	default:
		gpiod_direction_input(cpu_gpio->gpio);
		break;
	}
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("[%d]: set gpio %s[%d] value: %d\n",
		      pdrv->index, cpu_gpio->name, index, value);
	}
}

unsigned int lcd_cpu_gpio_get(struct aml_lcd_drv_s *pdrv, unsigned int index)
{
	struct lcd_cpu_gpio_s *cpu_gpio;

	cpu_gpio = &pdrv->config.power.cpu_gpio[index];
	if (cpu_gpio->probe_flag == 0) {
		LCDERR("[%d]: %s: gpio [%d] is not probed\n",
		       pdrv->index, __func__, index);
		return -1;
	}
	if (cpu_gpio->register_flag == 0) {
		LCDERR("[%d]: %s: gpio %s[%d] is not registered\n",
		       pdrv->index, __func__, cpu_gpio->name, index);
		return -1;
	}
	if (IS_ERR_OR_NULL(cpu_gpio->gpio)) {
		LCDERR("[%d]: gpio[%d]: %p, err: %ld\n",
		       pdrv->index, index, cpu_gpio->gpio,
		       PTR_ERR(cpu_gpio->gpio));
		return -1;
	}

	return gpiod_get_value(cpu_gpio->gpio);
}

static void lcd_custom_pinmux_set(struct aml_lcd_drv_s *pdrv, int status)
{
	struct lcd_config_s *pconf;
	unsigned int index;
	char pinmux_str[35];

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s: %d\n", pdrv->index, __func__, status);

	pconf = &pdrv->config;

	memset(pinmux_str, 0, sizeof(pinmux_str));
	if (status) {
		index = 0;
		sprintf(pinmux_str, "%s", pconf->basic.model_name);
	} else {
		index = 1;
		sprintf(pinmux_str, "%s_off", pconf->basic.model_name);
	}

	if (pconf->pinmux_flag == index) {
		LCDPR("pinmux %s is already selected\n", pinmux_str);
		return;
	}

	pconf->pin = devm_pinctrl_get_select(pdrv->dev, pinmux_str);
	if (IS_ERR_OR_NULL(pconf->pin)) {
		LCDERR("set custom_pinmux %s error\n", pinmux_str);
	} else {
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			LCDPR("set custom_pinmux %s: 0x%px\n",
			      pinmux_str, pconf->pin);
		}
	}
	pconf->pinmux_flag = index;
}

static char *lcd_rgb_pinmux_str[] = {
	"rgb_sync_on",      /* 0 */
	"rgb_de_on",        /* 1 */
	"rgb_sync_de_on",   /* 2 */
	"rgb_off"          /* 3 */
};

void lcd_rgb_pinmux_set(struct aml_lcd_drv_s *pdrv, int status)
{
	struct lcd_config_s *pconf;
	unsigned int index;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s: %d\n", pdrv->index, __func__, status);

	pconf = &pdrv->config;
	if (status) {
		if (pconf->control.rgb_cfg.sync_valid && pconf->control.rgb_cfg.de_valid) {
			index = 2;
		} else if (pconf->control.rgb_cfg.de_valid) {
			index = 1;
		} else if (pconf->control.rgb_cfg.sync_valid) {
			index = 0;
		} else {
			LCDERR("[%d]: rgb pinmux error\n", pdrv->index);
			return;
		}
	} else {
		index = 3;
	}

	if (pconf->pinmux_flag == index) {
		LCDPR("[%d]: pinmux %s is already selected\n",
				pdrv->index, lcd_rgb_pinmux_str[index]);
		return;
	}

	/* request pinmux */
	pconf->pin = devm_pinctrl_get_select(pdrv->dev, lcd_rgb_pinmux_str[index]);
	if (IS_ERR_OR_NULL(pconf->pin)) {
		LCDERR("[%d]: set rgb pinmux %s error\n", pdrv->index, lcd_rgb_pinmux_str[index]);
		return;
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("[%d]: set rgb pinmux %s: 0x%px\n",
			pdrv->index, lcd_rgb_pinmux_str[index], pconf->pin);
	}
	pconf->pinmux_flag = index;
}

static char *lcd_bt_pinmux_str[] = {
	"bt656_on",   /* 0 */
	"bt656_off",  /* 1 */
	"bt1120_on",   /* 2 */
	"bt1120_off",  /* 3 */
};

void lcd_bt_pinmux_set(struct aml_lcd_drv_s *pdrv, int status)
{
	struct lcd_config_s *pconf;
	unsigned int index;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s: %d\n", pdrv->index, __func__, status);

	pconf = &pdrv->config;

	if (pdrv->config.basic.lcd_type == LCD_BT656) {
		index = 0;
	} else if (pdrv->config.basic.lcd_type == LCD_BT1120) {
		index = 2;
	} else {
		LCDERR("[%d]: bt pinmux invalid\n", pdrv->index);
		return;
	}

	if (status == 0)
		index++;

	if (pconf->pinmux_flag == index) {
		LCDPR("[%d]: pinmux %s is already selected\n",
		      pdrv->index, lcd_bt_pinmux_str[index]);
		return;
	}

	/* request pinmux */
	pconf->pin = devm_pinctrl_get_select(pdrv->dev, lcd_bt_pinmux_str[index]);
	if (IS_ERR_OR_NULL(pconf->pin)) {
		LCDERR("[%d]: set bt pinmux %s error\n", pdrv->index, lcd_bt_pinmux_str[index]);
		return;
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("[%d]: set bt pinmux %s: 0x%px\n",
				pdrv->index, lcd_bt_pinmux_str[index], pconf->pin);
	}
	pconf->pinmux_flag = index;
}

static char *lcd_vbyone_pinmux_str[] = {
	"vbyone",
	"vbyone_off",
	"none",
};

/* set VX1_LOCKN && VX1_HTPDN */
void lcd_vbyone_pinmux_set(struct aml_lcd_drv_s *pdrv, int status)
{
	struct lcd_config_s *pconf;
	unsigned int index;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s: %d\n", pdrv->index, __func__, status);

	pconf = &pdrv->config;
	index = (status) ? 0 : 1;

	if (pconf->pinmux_flag == index) {
		LCDPR("[%d]: pinmux %s is already selected\n",
		      pdrv->index, lcd_vbyone_pinmux_str[index]);
		return;
	}

	pconf->pin = devm_pinctrl_get_select(pdrv->dev, lcd_vbyone_pinmux_str[index]);
	if (IS_ERR_OR_NULL(pconf->pin)) {
		LCDERR("[%d]: set vbyone pinmux %s error\n",
		       pdrv->index, lcd_vbyone_pinmux_str[index]);
	} else {
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			LCDPR("[%d]: set vbyone pinmux %s: 0x%px\n",
			      pdrv->index, lcd_vbyone_pinmux_str[index],
			      pconf->pin);
		}
	}
	pconf->pinmux_flag = index;
}

static char *lcd_tcon_pinmux_str[] = {
	"tcon_p2p",       /* 0 */
	"tcon_p2p_usit",  /* 1 */
	"tcon_p2p_off",   /* 2 */
	"tcon_mlvds",     /* 3 */
	"tcon_mlvds_off", /* 4 */
	"none"		      /* 5 */
};

void lcd_mlvds_pinmux_set(struct aml_lcd_drv_s *pdrv, int status)
{
	struct lcd_config_s *pconf;
	unsigned int index;

	if (!pdrv)
		return;

	if (pdrv->config.custom_pinmux) {
		lcd_custom_pinmux_set(pdrv, status);
		return;
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s: %d\n", pdrv->index, __func__, status);

	pconf = &pdrv->config;
	index = (status) ? 3 : 4;

	if (pconf->pinmux_flag == index) {
		LCDPR("[%d]: pinmux %s is already selected\n",
		      pdrv->index, lcd_tcon_pinmux_str[index]);
		return;
	}

	pconf->pin = devm_pinctrl_get_select(pdrv->dev, lcd_tcon_pinmux_str[index]);
	if (IS_ERR_OR_NULL(pconf->pin)) {
		LCDERR("[%d]: set mlvds pinmux %s error\n",
		       pdrv->index, lcd_tcon_pinmux_str[index]);
	} else {
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			LCDPR("[%d]: set mlvds pinmux %s: 0x%px\n",
			      pdrv->index, lcd_tcon_pinmux_str[index],
			      pconf->pin);
		}
	}
	pconf->pinmux_flag = index;
}

void lcd_p2p_pinmux_set(struct aml_lcd_drv_s *pdrv, int status)
{
	struct lcd_config_s *pconf;
	unsigned int index, p2p_type;

	if (!pdrv)
		return;

	if (pdrv->config.custom_pinmux) {
		lcd_custom_pinmux_set(pdrv, status);
		return;
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s: %d\n", pdrv->index, __func__, status);

	pconf = &pdrv->config;
	p2p_type = pconf->control.p2p_cfg.p2p_type & 0x1f;
	if (p2p_type == P2P_USIT)
		index = (status) ? 1 : 2;
	else
		index = (status) ? 0 : 2;

	if (pconf->pinmux_flag == index) {
		LCDPR("[%d]: pinmux %s is already selected\n",
		      pdrv->index, lcd_tcon_pinmux_str[index]);
		return;
	}

	pconf->pin = devm_pinctrl_get_select(pdrv->dev, lcd_tcon_pinmux_str[index]);
	if (IS_ERR_OR_NULL(pconf->pin)) {
		LCDERR("[%d]: set p2p pinmux %s error\n",
		       pdrv->index, lcd_tcon_pinmux_str[index]);
	} else {
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			LCDPR("[%d]: set p2p pinmux %s: 0x%px\n",
			      pdrv->index, lcd_tcon_pinmux_str[index],
			      pconf->pin);
		}
	}
	pconf->pinmux_flag = index;
}

static char *lcd_edp_pinmux_str[] = {
	"edp",
	"edp_off",
	"none",
};

void lcd_edp_pinmux_set(struct aml_lcd_drv_s *pdrv, int status)
{
	struct lcd_config_s *pconf;
	unsigned int index;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s: %d\n", pdrv->index, __func__, status);

	pconf = &pdrv->config;
	index = (status) ? 0 : 1;

	if (pconf->pinmux_flag == index) {
		LCDPR("[%d]: pinmux %s is already selected\n",
		      pdrv->index, lcd_edp_pinmux_str[index]);
		return;
	}

	pconf->pin = devm_pinctrl_get_select(pdrv->dev, lcd_edp_pinmux_str[index]);
	if (IS_ERR_OR_NULL(pconf->pin)) {
		LCDERR("[%d]: set edp pinmux %s error\n",
		       pdrv->index, lcd_edp_pinmux_str[index]);
	} else {
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			LCDPR("[%d]: set edp pinmux %s: 0x%px\n",
			      pdrv->index, lcd_edp_pinmux_str[index],
			      pconf->pin);
		}
	}
	pconf->pinmux_flag = index;
}

static char *lcd_mipi_pinmux_str[] = {
	"dsi_on",
	"dsi_off",
	"none",
};

void lcd_mipi_pinmux_set(struct aml_lcd_drv_s *pdrv, int status)
{
	struct lcd_config_s *pconf;
	unsigned int index;

	if (pdrv->data->chip_type != LCD_CHIP_C3)
		return;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s: %d\n", pdrv->index, __func__, status);

	pconf = &pdrv->config;
	index = (status) ? 0 : 1;

	if (pconf->pinmux_flag == index) {
		LCDPR("[%d]: pinmux %s is already selected\n",
		      pdrv->index, lcd_mipi_pinmux_str[index]);
		return;
	}

	pconf->pin = devm_pinctrl_get_select(pdrv->dev, lcd_mipi_pinmux_str[index]);
	if (IS_ERR_OR_NULL(pconf->pin)) {
		LCDERR("[%d]: set mipi pinmux %s error\n",
		       pdrv->index, lcd_mipi_pinmux_str[index]);
	} else {
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			LCDPR("[%d]: set mipi pinmux %s: 0x%px\n",
			      pdrv->index, lcd_mipi_pinmux_str[index],
			      pconf->pin);
		}
	}
	pconf->pinmux_flag = index;
}

static void lcd_ss_config_fix(struct aml_lcd_drv_s *pdrv)
{
	int i = 0;

	//fix ss in detail timing and phy_attr if not config
	for (i = 0; i < pdrv->config.phy_cfg.group_num; i++) {
		if (pdrv->config.phy_cfg.phys[i]->ss.freq == 255)
			pdrv->config.phy_cfg.phys[i]->ss.freq = pdrv->config.timing.ss_freq;
		if (pdrv->config.phy_cfg.phys[i]->ss.level == 255)
			pdrv->config.phy_cfg.phys[i]->ss.level = pdrv->config.timing.ss_level;
		if (pdrv->config.phy_cfg.phys[i]->ss.mode == 255)
			pdrv->config.phy_cfg.phys[i]->ss.mode = pdrv->config.timing.ss_mode;
	}

	for (i = 0; i < pdrv->config.timing.num_timings; i++) {
		if (pdrv->config.timing.timings[i]->ss_level == 255)
			pdrv->config.timing.timings[i]->ss_level = pdrv->config.timing.ss_level;
		if (pdrv->config.timing.timings[i]->ss_freq == 255)
			pdrv->config.timing.timings[i]->ss_freq = pdrv->config.timing.ss_freq;
		if (pdrv->config.timing.timings[i]->ss_mode == 255)
			pdrv->config.timing.timings[i]->ss_mode = pdrv->config.timing.ss_mode;
	}
}

/* ************************************************** *
 * lcd config
 * **************************************************
 */

int lcd_detail_timing_print(struct lcd_detail_timing_s *dt, char *buf, int offset, int max_len)
{
	s32 herr, verr, len;
	char *ck[3] = {"(x)", "(!)", ""};
	char *ck_hbp, *ck_hfp, *ck_vbp, *ck_vfp;

	if (!buf || offset >= max_len - 1)
		return 0;

	herr = dt->check_status & 0xf;
	verr = (dt->check_status >> 4) & 0xf;
	ck_hbp = (herr & 0x4) ? ck[0] : (herr & 0x8) ? ck[1] : ck[2];
	ck_hfp = (herr & 0x1) ? ck[0] : (herr & 0x2) ? ck[1] : ck[2];
	ck_vbp = (verr & 0x4) ? ck[0] : (verr & 0x8) ? ck[1] : ck[2];
	ck_vfp = (verr & 0x1) ? ck[0] : (verr & 0x2) ? ck[1] : ck[2];

	len = offset;
	len += snprintf(buf + len, max_len - len - 1,
			"ht:%4d(%4d ~ %4d), hact:%4d, hbp:%d%s, hsw:%2d, hfp:%3d%s, hpol:%d\n"
			"vt:%4d(%4d ~ %4d), vact:%4d, vbp:%d%s, vsw:%2d, vfp:%3d%s, vpol:%d\n"
			"lcd_bits:%d, cfmt:%d, fr_adj_type:%d, switch_type:0x%x\n"
			"ss_level:%d, ss_mode:%d, ss_freq:%d, ss_force:%d\n"
			"pclk:%d(%d ~ %d)\n"
			"frame_rate:%d (%d ~ %d)\n"
			"vrr_range:[%d ~ %d]\n",
			dt->h_period, dt->h_period_min, dt->h_period_max, dt->h_active,
			dt->hsync_bp, ck_hbp, dt->hsync_width, dt->hsync_fp, ck_hfp, dt->hsync_pol,
			dt->v_period, dt->v_period_min, dt->v_period_max, dt->v_active,
			dt->vsync_bp, ck_vbp, dt->vsync_width, dt->vsync_fp, ck_vfp, dt->vsync_pol,
			dt->lcd_bits, dt->cfmt, dt->fr_adjust_type, dt->switch_type,
			dt->ss_level, dt->ss_mode, dt->ss_freq, dt->ss_force,
			dt->pixel_clk, dt->pclk_min, dt->pclk_max,
			dt->frame_rate, dt->frame_rate_min, dt->frame_rate_max,
			dt->vfreq_vrr_min, dt->vfreq_vrr_max);

	return len - offset;
}

int lcd_phy_cfg_print(struct phy_config_s *cfg, char *buf, int offset, int max_len)
{
	int i, len = 0;
	struct phy_ch_ctrl_s *ch_ctrl;

	if (!buf || offset >= max_len - 1)
		return 0;

	len = offset;
	len += snprintf(buf + len, max_len - len - 1,
			"state: %d, flag: 0x%x, lane_num: %d, nphys: %d\n"
			"swap0: 0x%08x, swap1: 0x%08x, lane ofst: 0x%x, mask: 0x%x, valid: 0x%x\n"
			"ckdi: 0x%x, weakly_pd: 0x%x, low_com: 0x%x\n",
			cfg->state, cfg->flag, cfg->lane_num, cfg->group_num,
			cfg->ch_swap0, cfg->ch_swap1,
			cfg->lane_offset, cfg->lane_mask, cfg->lane_valid,
			cfg->ckdi, cfg->weakly_pull_down, cfg->low_common_mode);

	if (len >= max_len - 1)
		return len - offset;
	len += snprintf(buf + len, max_len - len - 1, "lane  en    sel   phase_sel\n");

	ch_ctrl = cfg->ch_ctrl;
	for (i = 0; i < cfg->lane_num; i++) {
		if (len >= max_len - 1)
			return len - offset;
		len += snprintf(buf + len, max_len - len - 1, "[%2d]  %d     %02d    0x%02x\n",
				i, ch_ctrl[i].en, ch_ctrl[i].sel, ch_ctrl[i].phase_sel);
	}
	if (len >= max_len - 1)
		return len - offset;

	len += snprintf(buf + len, max_len - len - 1, "\n");

	return len - offset;
}

int lcd_phy_attr_print(struct phy_attr_s *phy, u32 lane_num, char *buf, int offset, int max_len)
{
	int m, n, i, len;
	struct phy_lane_s *lane;

	if (!buf || offset >= max_len - 1)
		return 0;

	len = offset;
	len += snprintf(buf + len, max_len - len - 1,
			"cv_mode:%d, vswing:0x%x, vcm:0x%x, odt:0x%x, ref_bias:0x%x\n"
			"phy_clk:%d, clk_phase:%d, ss_level:%d, ss_freq:%d, ss_mode:%d\n",
			phy->cv_mode, phy->vswing, phy->vcm, phy->odt, phy->ref_bias,
			phy->phy_clk, phy->clk_phase, phy->ss.level, phy->ss.freq, phy->ss.mode);

	m = (lane_num + 1) / 2;
	n = m;
	lane = phy->lane;
	len += snprintf(buf + len, max_len - len - 1, "lane  amp   preem     lane  amp   preem\n");
	for (i = 0; i < m; i++, n++) {
		if (len >= max_len - 1)
			return len - offset;
		len += snprintf(buf + len, max_len - len - 1,
				"[%2d]  0x%02x  0x%02x      [%2d]  0x%02x  0x%02x\n",
				i, lane[i].amp, lane[i].preem, n, lane[n].amp, lane[n].preem);
	}
	if (len >= max_len - 1)
		return len - offset;
	len += snprintf(buf + len, max_len - len - 1, "\n");

	return len - offset;
}

static void lcd_config_load_print(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_config_s *pconf = &pdrv->config;
	struct lcd_detail_timing_s *ptiming = pdrv->config.timing.dft_timing;
	struct phy_attr_s *phy;
	struct phy_config_s *phy_cfg = &pdrv->config.phy_cfg;
	union lcd_ctrl_config_u *pctrl;
	int i = 0, pr_len = 4 * 1024;
	char *pr_buf;

	if (!ptiming)
		return;

	if ((lcd_debug_print_flag & LCD_DBG_PR_NORMAL) == 0)
		return;

	LCDPR("[%d]: %s, %s\n",
	      pdrv->index, pconf->basic.model_name,
	      lcd_type_type_to_str(pconf->basic.lcd_type));
	LCDPR("pll_flag = %d\n", pconf->timing.pll_flag);
	LCDPR("clk_mode = %d\n", pconf->timing.clk_mode);
	LCDPR("custom_pinmux = %d\n", pconf->custom_pinmux);
	LCDPR("fr_auto_cus = 0x%x\n", pconf->fr_auto_cus);

	pr_buf = kzalloc(pr_len, GFP_KERNEL);
	if (!pr_buf)
		return;

	LCDPR("\nconfig timings:\n");
	for (i = 0; i < pdrv->config.timing.num_timings; i++) {
		ptiming = pdrv->config.timing.timings[i];
		if (!ptiming)
			continue;
		LCDPR("timing[%d]:\n", i);
		lcd_detail_timing_print(ptiming, pr_buf, 0, pr_len);
		lcd_debug_info_print(pr_buf);
	}

	LCDPR("phy config:\n");
	lcd_phy_cfg_print(phy_cfg, pr_buf, 0, pr_len);
	lcd_debug_info_print(pr_buf);

	for (i = 0; i < phy_cfg->group_num; i++) {
		phy = pdrv->config.phy_cfg.phys[i];
		if (!phy)
			continue;
		LCDPR("phy group[%d]:\n", i);
		lcd_phy_attr_print(phy, phy_cfg->lane_num, pr_buf, 0, pr_len);
		lcd_debug_info_print(pr_buf);
	}

	pctrl = &pconf->control;
	if (pconf->basic.lcd_type == LCD_RGB) {
		LCDPR("type = %d\n", pctrl->rgb_cfg.type);
		LCDPR("clk_pol = %d\n", pctrl->rgb_cfg.clk_pol);
		LCDPR("de_valid = %d\n", pctrl->rgb_cfg.de_valid);
		LCDPR("sync_valid = %d\n", pctrl->rgb_cfg.sync_valid);
		LCDPR("rb_swap = %d\n", pctrl->rgb_cfg.rb_swap);
		LCDPR("bit_swap = %d\n", pctrl->rgb_cfg.bit_swap);
	} else if ((pconf->basic.lcd_type == LCD_BT656) ||
		   (pconf->basic.lcd_type == LCD_BT1120)) {
		LCDPR("clk_phase = 0x%x\n", pctrl->bt_cfg.clk_phase);
		LCDPR("field_type = %d\n", pctrl->bt_cfg.field_type);
		LCDPR("mode_422 = %d\n", pctrl->bt_cfg.mode_422);
		LCDPR("yc_swap = %d\n", pctrl->bt_cfg.yc_swap);
		LCDPR("cbcr_swap = %d\n", pctrl->bt_cfg.cbcr_swap);
	} else if (pconf->basic.lcd_type == LCD_LVDS) {
		LCDPR("lvds_repack = %d\n", pctrl->lvds_cfg.lvds_repack);
		LCDPR("pn_swap = %d\n", pctrl->lvds_cfg.pn_swap);
		LCDPR("dual_port = %d\n", pctrl->lvds_cfg.dual_port);
		LCDPR("port_swap = %d\n", pctrl->lvds_cfg.port_swap);
		LCDPR("lane_reverse = %d\n", pctrl->lvds_cfg.lane_reverse);
		LCDPR("phy_vswing = 0x%x\n", pctrl->lvds_cfg.phy_vswing);
		LCDPR("phy_preem = 0x%x\n", pctrl->lvds_cfg.phy_preem);
	} else if (pconf->basic.lcd_type == LCD_VBYONE) {
		LCDPR("lane_count = %d\n", pctrl->vbyone_cfg.lane_count);
		LCDPR("byte_mode = %d\n", pctrl->vbyone_cfg.byte_mode);
		LCDPR("region_num = %d\n", pctrl->vbyone_cfg.region_num);
		LCDPR("color_fmt = %d\n", pctrl->vbyone_cfg.color_fmt);
		LCDPR("phy_vswing = 0x%x\n", pctrl->vbyone_cfg.phy_vswing);
		LCDPR("phy_preem = 0x%x\n", pctrl->vbyone_cfg.phy_preem);
	} else if (pconf->basic.lcd_type == LCD_MLVDS) {
		LCDPR("channel_num = %d\n", pctrl->mlvds_cfg.channel_num);
		LCDPR("channel_sel0 = 0x%x\n", pctrl->mlvds_cfg.channel_sel0);
		LCDPR("channel_sel1 = 0x%x\n", pctrl->mlvds_cfg.channel_sel1);
		LCDPR("clk_phase = 0x%x\n", pctrl->mlvds_cfg.clk_phase);
		LCDPR("phy_vswing = 0x%x\n", pctrl->mlvds_cfg.phy_vswing);
		LCDPR("phy_preem = 0x%x\n", pctrl->mlvds_cfg.phy_preem);
	} else if (pconf->basic.lcd_type == LCD_P2P) {
		LCDPR("p2p_type = 0x%x\n", pctrl->p2p_cfg.p2p_type);
		LCDPR("lane_num = %d\n", pctrl->p2p_cfg.lane_num);
		LCDPR("channel_sel0 = 0x%x\n", pctrl->p2p_cfg.channel_sel0);
		LCDPR("channel_sel1 = 0x%x\n", pctrl->p2p_cfg.channel_sel1);
		LCDPR("phy_vswing = 0x%x\n", pctrl->p2p_cfg.phy_vswing);
		LCDPR("phy_preem = 0x%x\n", pctrl->p2p_cfg.phy_preem);
	} else if (pconf->basic.lcd_type == LCD_MIPI) {
		if (pctrl->mipi_cfg.check_en) {
			LCDPR("check_reg = 0x%02x\n", pctrl->mipi_cfg.check_reg);
			LCDPR("check_cnt = %d\n", pctrl->mipi_cfg.check_cnt);
		}
		LCDPR("lane_num = %d\n", pctrl->mipi_cfg.lane_num);
		LCDPR("bit_rate_max = %d\n", pctrl->mipi_cfg.bit_rate_max);
		LCDPR("operation_mode_init = %d\n", pctrl->mipi_cfg.operation_mode_init);
		LCDPR("operation_mode_disp = %d\n", pctrl->mipi_cfg.operation_mode_display);
		LCDPR("video_mode_type = %d\n", pctrl->mipi_cfg.video_mode_type);
		LCDPR("clk_always_hs = %d\n", pctrl->mipi_cfg.clk_always_hs);
		LCDPR("extern_init = %d\n", pctrl->mipi_cfg.extern_init);
	} else if (pconf->basic.lcd_type == LCD_EDP) {
		LCDPR("max_lane_count = %d\n", pctrl->edp_cfg.max_lane_count);
		LCDPR("max_link_rate  = %d\n", pctrl->edp_cfg.max_link_rate);
		LCDPR("training_mode  = %d\n", pctrl->edp_cfg.training_mode);
		LCDPR("edid_en        = %d\n", pctrl->edp_cfg.edid_en);
		LCDPR("sync_clk_mode  = %d\n", pctrl->edp_cfg.sync_clk_mode);
		LCDPR("lane_count     = %d\n", pctrl->edp_cfg.lane_count);
		LCDPR("link_rate      = %d\n", pctrl->edp_cfg.link_rate);
		LCDPR("phy_vswing = 0x%x\n", pctrl->edp_cfg.phy_vswing_preset);
		LCDPR("phy_preem  = 0x%x\n", pctrl->edp_cfg.phy_preem_preset);
	}
}

void lcd_act_timing_dbg_print(struct aml_lcd_drv_s *pdrv)
{
	if ((lcd_debug_print_flag & LCD_DBG_PR_NORMAL) == 0)
		return;

	LCDPR("[%d]: act_timing: %dx%dp%dhz\n",
		pdrv->index, pdrv->config.timing.act_timing.h_active,
		pdrv->config.timing.act_timing.v_active,
		pdrv->config.timing.act_timing.frame_rate);
}

//ret: bit[0]:hfp: fatal error, block driver
//     bit[1]:hfp: warning, only print warning message
//     bit[2]:hswbp: fatal error, block driver
//     bit[3]:hswbp: warning, only print warning message
//     bit[4]:vfp: fatal error, block driver
//     bit[5]:vfp: warning, only print warning message
//     bit[6]:vswbp: fatal error, block driver
//     bit[7]:vswbp: warning, only print warning message
unsigned int lcd_config_timing_check(struct aml_lcd_drv_s *pdrv,
				     struct lcd_detail_timing_s *ptiming)
{
	short hpw = ptiming->hsync_width;
	short hbp = ptiming->hsync_bp;
	short hfp = ptiming->hsync_fp;
	short vpw = ptiming->vsync_width;
	short vbp = ptiming->vsync_bp;
	short vfp = ptiming->vsync_fp;
	short hfp_min, vfp_min, vfp_cmpr_tail = 0, temp;
	char *ferr_str = NULL, *warn_str = NULL;
	int ferr_len = 0, warn_len = 0, ferr_left, warn_left;
	int ret = 0;

	ferr_str = kzalloc(PR_BUF_MAX, GFP_KERNEL);
	if (!ferr_str) {
		LCDERR("config_check fail for NOMEM\n");
		return 0;
	}
	warn_str = kzalloc(PR_BUF_MAX, GFP_KERNEL);
	if (!warn_str) {
		LCDERR("config_check fail for NOMEM\n");
		kfree(ferr_str);
		return 0;
	}

	if (pdrv->config.basic.lcd_type == LCD_MLVDS ||
	    pdrv->config.basic.lcd_type == LCD_P2P) {
		vfp_cmpr_tail = 3;
	}

	if (hfp <= 0) {
		ferr_left = lcd_debug_info_len(ferr_len);
		ferr_len += snprintf(ferr_str + ferr_len, ferr_left,
			"  hfp: %d, for panel, req: >0!!!\n", hfp);
		ret |= (1 << 0);
	}
	if (ptiming->h_period_min) {
		hfp_min = ptiming->h_period_min - ptiming->h_active - hpw - hbp;
		if (hfp_min <= 0) {
			warn_left = lcd_debug_info_len(warn_len);
			warn_len += snprintf(warn_str + warn_len, warn_left,
				"  hfp with h_period_min: %d, for panel, req: >0!!!\n", hfp_min);
			ret |= (1 << 1);
		}
	}

	if (vfp <= vfp_cmpr_tail) {
		ferr_left = lcd_debug_info_len(ferr_len);
		ferr_len += snprintf(ferr_str + ferr_len, ferr_left,
			"  vfp: %d, for panel, req: >%d!!!\n", vfp, vfp_cmpr_tail);
		ret |= (1 << 4);
	}
	if (ptiming->v_period_min) {
		vfp_min = ptiming->v_period_min - ptiming->v_active - vpw - vbp;
		if (vfp_min <= vfp_cmpr_tail) {
			ferr_left = lcd_debug_info_len(ferr_len);
			ferr_len += snprintf(ferr_str + ferr_len, ferr_left,
				"  vfp with v_period_min: %d, for panel, req: >%d!!!\n",
				vfp_min, vfp_cmpr_tail);
			ret |= (1 << 4);
		}
	}

	//display timing check
	//hswbp
	if (pdrv->disp_req.hswbp_vid == 0)
		goto lcd_config_timing_check_vid_hfp;
	temp = hpw + hbp;
	if (temp < pdrv->disp_req.hswbp_vid) {
		if (pdrv->disp_req.alert_level == 1) {
			warn_left = lcd_debug_info_len(warn_len);
			warn_len += snprintf(warn_str + warn_len, warn_left,
				"  hpw + hbp: %d, for display path, req: >=%d!\n",
				temp, pdrv->disp_req.hswbp_vid);
			ret |= (1 << 3);
		} else if (pdrv->disp_req.alert_level == 2) {
			ferr_left = lcd_debug_info_len(ferr_len);
			ferr_len += snprintf(ferr_str + ferr_len, ferr_left,
				"  hpw + hbp: %d, for display path, req: >=%d!!!\n",
				temp, pdrv->disp_req.hswbp_vid);
			ret |= (1 << 2);
		}
	}

lcd_config_timing_check_vid_hfp:
	//hfp
	if (pdrv->disp_req.hfp_vid == 0)
		goto lcd_config_timing_check_vid_vswbp;
	if (hfp < pdrv->disp_req.hfp_vid) {
		if (pdrv->disp_req.alert_level == 1) {
			warn_left = lcd_debug_info_len(warn_len);
			warn_len += snprintf(warn_str + warn_len, warn_left,
				"  hfp: %d, for display path, req: >=%d!\n",
				hfp, pdrv->disp_req.hfp_vid);
			ret |= (1 << 5);
		} else if (pdrv->disp_req.alert_level == 2) {
			ferr_left = lcd_debug_info_len(ferr_len);
			ferr_len += snprintf(ferr_str + ferr_len, ferr_left,
				"  hfp: %d, for display path, req: >=%d!!!\n",
				hfp, pdrv->disp_req.hfp_vid);
			ret |= (1 << 4);
		}
	}

lcd_config_timing_check_vid_vswbp:
	//vswbp
	if (pdrv->disp_req.vswbp_vid == 0)
		goto lcd_config_timing_check_vid_vfp;
	temp = vpw + vbp;
	if (temp < pdrv->disp_req.vswbp_vid) {
		if (pdrv->disp_req.alert_level == 1) {
			warn_left = lcd_debug_info_len(warn_len);
			warn_len += snprintf(warn_str + warn_len, warn_left,
				"  vpw + vbp: %d, for display path, req: >=%d!\n",
				temp, pdrv->disp_req.vswbp_vid);
			ret |= (1 << 7);
		} else if (pdrv->disp_req.alert_level == 2) {
			ferr_left = lcd_debug_info_len(ferr_len);
			ferr_len += snprintf(ferr_str + ferr_len, ferr_left,
				"  vpw + vbp: %d, for display path, req: >=%d!!!\n",
				temp, pdrv->disp_req.vswbp_vid);
			ret |= (1 << 6);
		}
	}

lcd_config_timing_check_vid_vfp:
	//vfp
	if (pdrv->disp_req.vfp_vid == 0)
		goto lcd_config_timing_check_end;
	temp = pdrv->disp_req.vfp_vid + vfp_cmpr_tail;
	if (vfp < temp) {
		if (pdrv->disp_req.alert_level == 1) {
			warn_left = lcd_debug_info_len(warn_len);
			warn_len += snprintf(warn_str + warn_len, warn_left,
				"  vfp: %d, for display path, req: >=%d!\n",
				vfp, temp);
			ret |= (1 << 5);
		} else if (pdrv->disp_req.alert_level == 2) {
			ferr_left = lcd_debug_info_len(ferr_len);
			ferr_len += snprintf(ferr_str + ferr_len, ferr_left,
				"  vfp: %d, for display path, req: >=%d!!!\n",
				vfp, temp);
			ret |= (1 << 4);
		}
	}

lcd_config_timing_check_end:
	if (ret) {
		pr_err("**************** lcd config timing check ****************\n");
		if (ret & 0x55) {
			pr_err("lcd: FATAL ERROR:\n"
				"%s\n", ferr_str);
		}
		if (ret & 0xaa) {
			pr_err("lcd: WARNING:\n"
				"%s\n", warn_str);
		}
		pr_err("************** lcd config timing check end ****************\n");
	}
	kfree(ferr_str);
	kfree(warn_str);

	ptiming->check_status = ret;

	return ret;
}

int lcd_base_config_load_from_dts(struct aml_lcd_drv_s *pdrv)
{
	const struct device_node *np;
	const char *mode_str, *str;
	unsigned int val, para[8];
	char str_info[128];
	int str_info_len = 0, ret = 0;

	if (!pdrv->dev->of_node) {
		LCDERR("dev of_node is null\n");
		pdrv->mode = LCD_MODE_MAX;
		return -1;
	}
	np = pdrv->dev->of_node;

	/* lcd driver assign */
	switch (pdrv->debug_ctrl->debug_lcd_mode) {
	case 1:
		LCDPR("[%d]: debug_lcd_mode: 1,tv mode\n", pdrv->index);
		pdrv->mode = LCD_MODE_TV;
		break;
	case 2:
		LCDPR("[%d]: debug_lcd_mode: 2,tablet mode\n", pdrv->index);
		pdrv->mode = LCD_MODE_TABLET;
		break;
	default:
		ret = of_property_read_string(np, "mode", &mode_str);
		if (ret) {
			LCDERR("[%d]: failed to get mode\n", pdrv->index);
			return -1;
		}
		pdrv->mode = lcd_mode_str_to_mode(mode_str);
		break;
	}

	ret = of_property_read_u32(np, "pxp", &val);
	if (ret == 0)
		pdrv->lcd_pxp = (unsigned char)val;

	ret = of_property_read_u32(np, "fr_auto_policy", &val);
	if (ret == 0)
		pdrv->fr_auto_policy = (unsigned char)val;

	ret = of_property_read_u32(np, "vout_regist_on", &val);
	if (ret) {
		pdrv->vout_regist_on_ctrl = 0;
	} else {
		pdrv->vout_regist_on_ctrl = (unsigned char)val;
		LCDPR("set lcd drv regist:%s%s%s\n",
			pdrv->vout_regist_on_ctrl & 0x1 ? " vout"  : "",
			pdrv->vout_regist_on_ctrl & 0x2 ? " vout2" : "",
			pdrv->vout_regist_on_ctrl & 0x4 ? " vout3" : "");
	}

	switch (pdrv->debug_ctrl->debug_para_source) {
	case 1:
		LCDPR("[%d]: debug_para_source: 1,dts\n", pdrv->index);
		pdrv->key_valid = 0;
		break;
	case 2:
		LCDPR("[%d]: debug_para_source: 2,unifykey\n", pdrv->index);
		pdrv->key_valid = 1;
		break;
	default:
		ret = of_property_read_u32(np, "key_valid", &val);
		if (ret) {
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
				LCDPR("failed to get key_valid\n");
			pdrv->key_valid = 0;
		} else {
			pdrv->key_valid = (unsigned char)val;
		}
		break;
	}

	ret = of_property_read_u32(np, "clk_path", &val);
	if (ret) {
		pdrv->clk_path = 0;
	} else {
		pdrv->clk_path = (unsigned char)val;
		LCDPR("[%d]: detect clk_path: %d\n",
		      pdrv->index, pdrv->clk_path);
	}

	ret = of_property_read_u32(np, "auto_test", &val);
	if (ret) {
		pdrv->auto_test = 0;
	} else {
		pdrv->auto_test = (unsigned char)val;
		LCDPR("[%d]: detect auto_test: %d\n",
		      pdrv->index, pdrv->auto_test);
	}

	ret = of_property_read_u32(np, "resume_type", &val);
	if (ret)
		pdrv->resume_type = 1; /* default workqueue */
	else
		pdrv->resume_type = (unsigned char)val;

	ret = of_property_read_u32(np, "config_check_glb", &val);
	if (ret)
		pdrv->config_check_glb = 0;
	else
		pdrv->config_check_glb = (unsigned char)val;

	str_info_len += sprintf(str_info + str_info_len, "fr_auto: %d, ",
			pdrv->fr_auto_policy);
	str_info_len += sprintf(str_info + str_info_len, "resume_type: 0x%x, ",
			pdrv->resume_type);
	sprintf(str_info + str_info_len, "cfg_chk_glb: %d, ", pdrv->config_check_glb);
	LCDPR("[%d]: drv_ver: %s(%d-%s), lcd_mode: %s, key_valid: %d, %s\n",
	      pdrv->index, LCD_DRV_VERSION, pdrv->data->chip_type, pdrv->data->chip_name,
	      mode_str, pdrv->key_valid, str_info);

	ret = of_property_read_u32_array(np, "display_timing_req_min", &para[0], 5);
	if (ret) {
		pdrv->disp_req.alert_level = 0;
		pdrv->disp_req.hswbp_vid = 0;
		pdrv->disp_req.hfp_vid = 0;
		pdrv->disp_req.vswbp_vid = 0;
		pdrv->disp_req.vfp_vid = 0;
	} else {
		pdrv->disp_req.alert_level = para[0];
		pdrv->disp_req.hswbp_vid = para[1];
		pdrv->disp_req.hfp_vid = para[2];
		pdrv->disp_req.vswbp_vid = para[3];
		pdrv->disp_req.vfp_vid = para[4];
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
			LCDPR("[%d]: find display_timing_req_min: alert_level:%d\n"
				"hswbp:%d, hfp:%d, vswbp:%d, vfp:%d\n",
				pdrv->index, pdrv->disp_req.alert_level,
				pdrv->disp_req.hswbp_vid, pdrv->disp_req.hfp_vid,
				pdrv->disp_req.vswbp_vid, pdrv->disp_req.vfp_vid);
		}
	}

	/* only for test */
	ret = of_property_read_string(np, "lcd_propname_sel", &str);
	if (ret == 0) {
		strcpy(pdrv->config.propname, str);
		LCDPR("[%d]: find lcd_propname_sel: %s\n",
			pdrv->index, pdrv->config.propname);
	}

	ret = of_property_read_u32_array(np, "sw_vlock", &para[0], 7);
	if (ret == 0) {
		pdrv->fr_lock_en = para[0];
		pdrv->fr_lock = kzalloc(sizeof(*pdrv->fr_lock), GFP_KERNEL);
		if (pdrv->fr_lock) {
			pdrv->fr_lock->en = para[0];
			pdrv->fr_lock->mode = para[1];
			pdrv->fr_lock->kp = para[2];
			pdrv->fr_lock->ki = para[3];
			pdrv->fr_lock->kd = para[4];
			pdrv->fr_lock->line_limit = para[5];
			pdrv->fr_lock->freq_limit = para[6];
			LCDPR("fr_lock:%d, mode:%d, kp:%d, ki:%d, kd:%d,\n"
				  "line_limit:%d, freq_limit:%d\n",
				pdrv->fr_lock_en, pdrv->fr_lock->mode, pdrv->fr_lock->kp,
				pdrv->fr_lock->ki, pdrv->fr_lock->kd,
				pdrv->fr_lock->line_limit, pdrv->fr_lock->freq_limit);
		}
	}

	return 0;
}

static int lcd_power_load_from_dts(struct aml_lcd_drv_s *pdrv, struct device_node *child)
{
	struct lcd_power_ctrl_s *power_step = &pdrv->config.power;
	int ret = 0, append_more = 1;
	unsigned int para[5];
	unsigned int val;
	int i, j, temp;
	unsigned int index;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s\n", pdrv->index, __func__);

	if (!child) {
		LCDERR("[%d]: error: failed to get %s\n",
		      pdrv->index, pdrv->config.propname);
		return -1;
	}

	ret = of_property_read_u32_array(child, "power_on_step", &para[0], 4);
	if (ret) {
		LCDPR("[%d]: failed to get power_on_step\n", pdrv->index);
		power_step->power_on_step[0].type = LCD_POWER_TYPE_MAX;
	} else {
		i = 0;
		while (i < LCD_PWR_STEP_MAX) {
			power_step->power_on_step_max = i;
			j = 4 * i;
			ret = of_property_read_u32_index(child, "power_on_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_on_step %d\n",
				      pdrv->index, i);
				power_step->power_on_step[i].type = 0xff;
				break;
			}
			power_step->power_on_step[i].type = (unsigned char)val;
			if (val == 0xff) /* ending */
				break;
			j = 4 * i + 1;
			ret = of_property_read_u32_index(child, "power_on_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_on_step %d\n",
				      pdrv->index, i);
				power_step->power_on_step[i].type = 0xff;
				break;
			}
			power_step->power_on_step[i].index = val;
			j = 4 * i + 2;
			ret = of_property_read_u32_index(child, "power_on_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_on_step %d\n",
				      pdrv->index, i);
				power_step->power_on_step[i].type = 0xff;
				break;
			}
			power_step->power_on_step[i].value = val;
			j = 4 * i + 3;
			ret = of_property_read_u32_index(child, "power_on_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_on_step %d\n",
				      pdrv->index, i);
				power_step->power_on_step[i].type = 0xff;
				break;
			}
			power_step->power_on_step[i].delay = val;

			/* gpio/extern probe */
			index = power_step->power_on_step[i].index;
			switch (power_step->power_on_step[i].type) {
			case LCD_POWER_TYPE_CPU:
			case LCD_POWER_TYPE_WAIT_GPIO:
				if (index < LCD_CPU_GPIO_NUM_MAX)
					lcd_cpu_gpio_probe(pdrv, index);
				break;
#ifdef CONFIG_AMLOGIC_LCD_EXTERN
			case LCD_POWER_TYPE_EXTERN:
				lcd_resource_add(pdrv, LCD_RES_EXTERN, index);
				lcd_extern_dev_index_add(pdrv->index, index);
				break;
#endif
			case LCD_POWER_TYPE_CLK_SS:
				temp = power_step->power_on_step[i].value;
				pdrv->config.timing.ss_freq = temp & 0xf;
				pdrv->config.timing.ss_mode = (temp >> 4) & 0xf;
				if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
					LCDPR("[%d]: clk_ss value=0x%x: ss_freq=%d, ss_mode=%d\n",
					      pdrv->index, temp,
					      pdrv->config.timing.ss_freq,
					      pdrv->config.timing.ss_mode);
				}
				break;
			case LCD_POWER_TYPE_BACKLIGHT:
			case LCD_POWER_TYPE_MUTE:
				append_more = 0;
				break;
			default:
				break;
			}
			i++;
		}
		if (append_more && i + 2 < LCD_PWR_STEP_MAX) {
			power_step->power_on_step[i].type = LCD_POWER_TYPE_BACKLIGHT;
			power_step->power_on_step[i].index = 0;
			power_step->power_on_step[i].value = 1; //bl on
			power_step->power_on_step[i].delay = 0;
			i++;

			power_step->power_on_step[i].type = LCD_POWER_TYPE_MUTE;
			power_step->power_on_step[i].index = 0;
			power_step->power_on_step[i].value = 0;//unmute
			power_step->power_on_step[i].delay = pdrv->unmute_cnt;
			i++;
			power_step->power_on_step[i].type = LCD_POWER_TYPE_MAX;
			power_step->power_on_step_max = i;
		}
	}

	ret = of_property_read_u32_array(child, "power_off_step", &para[0], 4);
	if (ret) {
		LCDPR("[%d]: failed to get power_off_step\n", pdrv->index);
		power_step->power_off_step[0].type = LCD_POWER_TYPE_MAX;
	} else {
		append_more = 1;
		i = 0;
		while (i < LCD_PWR_STEP_MAX) {
			power_step->power_off_step_max = i;

			j = 4 * i;
			ret = of_property_read_u32_index(child, "power_off_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_off_step %d\n", pdrv->index, i);
				power_step->power_off_step[i].type = 0xff;
				break;
			}
			power_step->power_off_step[i].type = (unsigned char)val;
			if (val == 0xff) /* ending */
				break;
			j = 4 * i + 1;
			ret = of_property_read_u32_index(child, "power_off_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_off_step %d\n", pdrv->index, i);
				power_step->power_off_step[i].type = 0xff;
				break;
			}
			power_step->power_off_step[i].index = val;
			j = 4 * i + 2;
			ret = of_property_read_u32_index(child, "power_off_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_off_step %d\n", pdrv->index, i);
				power_step->power_off_step[i].type = 0xff;
				break;
			}
			power_step->power_off_step[i].value = val;
			j = 4 * i + 3;
			ret = of_property_read_u32_index(child, "power_off_step", j, &val);
			if (ret) {
				LCDPR("[%d]: failed to get power_off_step %d\n", pdrv->index, i);
				power_step->power_off_step[i].type = 0xff;
				break;
			}
			power_step->power_off_step[i].delay = val;

			/* gpio/extern probe */
			index = power_step->power_off_step[i].index;
			switch (power_step->power_off_step[i].type) {
			case LCD_POWER_TYPE_CPU:
			case LCD_POWER_TYPE_WAIT_GPIO:
				if (index < LCD_CPU_GPIO_NUM_MAX)
					lcd_cpu_gpio_probe(pdrv, index);
				break;
#ifdef CONFIG_AMLOGIC_LCD_EXTERN
			case LCD_POWER_TYPE_EXTERN:
				lcd_resource_add(pdrv, LCD_RES_EXTERN, index);
				lcd_extern_dev_index_add(pdrv->index, index);
				break;
#endif
			case LCD_POWER_TYPE_BACKLIGHT:
			case LCD_POWER_TYPE_MUTE:
				append_more = 0;
				break;
			default:
				break;
			}
			i++;
		}
		if (append_more && i + 2 < LCD_POWER_TYPE_MAX) {
			for (i = i + 2; i >= 2; i--)
				memcpy(&power_step->power_off_step[i],
				       &power_step->power_off_step[i - 2],
				       sizeof(struct lcd_power_step_s));
			power_step->power_off_step_max += 2;
			power_step->power_off_step[0].type  = LCD_POWER_TYPE_MUTE;
			power_step->power_off_step[0].index = 0;
			power_step->power_off_step[0].value = 1; //mute
			power_step->power_off_step[0].delay = pdrv->mute_cnt;

			power_step->power_off_step[1].type = LCD_POWER_TYPE_BACKLIGHT;
			power_step->power_off_step[1].index = 0;
			power_step->power_off_step[1].value = 0;//bl off
			power_step->power_off_step[1].delay = 0;
		}
	}

	return ret;
}

static int lcd_power_load_from_unifykey(struct aml_lcd_drv_s *pdrv,
					unsigned char *buf, int key_len, int len)
{
	struct lcd_power_ctrl_s *power_step = &pdrv->config.power;
	int i, j, temp;
	unsigned char *p;
	unsigned int index;
	int ret, append_more = 1;

	/* power: (5byte * n) */
	p = buf + len;
	i = 0;
	while (i < LCD_PWR_STEP_MAX) {
		power_step->power_on_step_max = i;
		len += 5;
		ret = lcd_unifykey_len_check(key_len, len);
		if (ret < 0) {
			power_step->power_on_step[i].type = 0xff;
			power_step->power_on_step[i].index = 0;
			power_step->power_on_step[i].value = 0;
			power_step->power_on_step[i].delay = 0;
			LCDERR("[%d]: unifykey power_on length is incorrect\n", pdrv->index);
			return -1;
		}
		power_step->power_on_step[i].type = *(p + LCD_UKEY_PWR_TYPE + 5 * i);
		power_step->power_on_step[i].index = *(p + LCD_UKEY_PWR_INDEX + 5 * i);
		power_step->power_on_step[i].value = *(p + LCD_UKEY_PWR_VAL + 5 * i);
		power_step->power_on_step[i].delay =
			(*(p + LCD_UKEY_PWR_DELAY + 5 * i) |
			((*(p + LCD_UKEY_PWR_DELAY + 5 * i + 1)) << 8));

		/* gpio/extern probe */
		index = power_step->power_on_step[i].index;
		switch (power_step->power_on_step[i].type) {
		case LCD_POWER_TYPE_CPU:
		case LCD_POWER_TYPE_WAIT_GPIO:
			if (index < LCD_CPU_GPIO_NUM_MAX)
				lcd_cpu_gpio_probe(pdrv, index);
			break;
#ifdef CONFIG_AMLOGIC_LCD_EXTERN
		case LCD_POWER_TYPE_EXTERN:
			lcd_resource_add(pdrv, LCD_RES_EXTERN, index);
			lcd_extern_dev_index_add(pdrv->index, index);
			break;
#endif
		case LCD_POWER_TYPE_CLK_SS:
			temp = power_step->power_on_step[i].value;
			pdrv->config.timing.ss_freq = temp & 0xf;
			pdrv->config.timing.ss_mode = (temp >> 4) & 0xf;
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: clk_ss value=0x%x: ss_freq=%d, ss_mode=%d\n",
					      pdrv->index, temp,
					      pdrv->config.timing.ss_freq,
					      pdrv->config.timing.ss_mode);
			}
			break;
		case LCD_POWER_TYPE_BACKLIGHT:
		case LCD_POWER_TYPE_MUTE:
			append_more = 0;
			break;
		default:
			break;
		}
		if (power_step->power_on_step[i].type >= LCD_POWER_TYPE_MAX)
			break;
		i++;
	}
	p += (5 * (i + 1));
	if (append_more && i + 2 < LCD_PWR_STEP_MAX) {
		power_step->power_on_step[i].type = LCD_POWER_TYPE_BACKLIGHT;
		power_step->power_on_step[i].index = 0;
		power_step->power_on_step[i].value = 1; //bl on
		power_step->power_on_step[i].delay = 0;
		i++;
		power_step->power_on_step[i].type = LCD_POWER_TYPE_MUTE;
		power_step->power_on_step[i].index = 0;
		power_step->power_on_step[i].value = 0;//unmute
		power_step->power_on_step[i].delay = pdrv->unmute_cnt;
		i++;
		power_step->power_on_step[i].type = LCD_POWER_TYPE_MAX;
		power_step->power_on_step_max = i;
	}

	append_more = 1;
	j = 0;
	while (j < LCD_PWR_STEP_MAX) {
		power_step->power_off_step_max = j;
		len += 5;
		ret = lcd_unifykey_len_check(key_len, len);
		if (ret < 0) {
			power_step->power_off_step[j].type = 0xff;
			power_step->power_off_step[j].index = 0;
			power_step->power_off_step[j].value = 0;
			power_step->power_off_step[j].delay = 0;
			LCDERR("[%d]: unifykey power_off length is incorrect\n", pdrv->index);
			return -1;
		}
		power_step->power_off_step[j].type = *(p + LCD_UKEY_PWR_TYPE + 5 * j);
		power_step->power_off_step[j].index = *(p + LCD_UKEY_PWR_INDEX + 5 * j);
		power_step->power_off_step[j].value = *(p + LCD_UKEY_PWR_VAL + 5 * j);
		power_step->power_off_step[j].delay =
				(*(p + LCD_UKEY_PWR_DELAY + 5 * j) |
				((*(p + LCD_UKEY_PWR_DELAY + 5 * j + 1)) << 8));

		/* gpio/extern probe */
		index = power_step->power_off_step[j].index;
		switch (power_step->power_off_step[j].type) {
		case LCD_POWER_TYPE_CPU:
		case LCD_POWER_TYPE_WAIT_GPIO:
			if (index < LCD_CPU_GPIO_NUM_MAX)
				lcd_cpu_gpio_probe(pdrv, index);
			break;
#ifdef CONFIG_AMLOGIC_LCD_EXTERN
		case LCD_POWER_TYPE_EXTERN:
			lcd_resource_add(pdrv, LCD_RES_EXTERN, index);
			lcd_extern_dev_index_add(pdrv->index, index);
			break;
#endif
		case LCD_POWER_TYPE_BACKLIGHT:
		case LCD_POWER_TYPE_MUTE:
			append_more = 0;
			break;
		default:
			break;
		}
		if (power_step->power_off_step[j].type >= LCD_POWER_TYPE_MAX)
			break;
		j++;
	}

	if (append_more && j + 2 < LCD_POWER_TYPE_MAX) {
		for (i = j + 2; i >= 2; i--)
			memcpy(&power_step->power_off_step[i],
			       &power_step->power_off_step[i - 2],
			       sizeof(struct lcd_power_step_s));
		power_step->power_off_step_max += 2;
		power_step->power_off_step[0].type  = LCD_POWER_TYPE_MUTE;
		power_step->power_off_step[0].index = 0;
		power_step->power_off_step[0].value = 1; //mute
		power_step->power_off_step[0].delay = pdrv->mute_cnt;

		power_step->power_off_step[1].type = LCD_POWER_TYPE_BACKLIGHT;
		power_step->power_off_step[1].index = 0;
		power_step->power_off_step[1].value = 0;//bl off
		power_step->power_off_step[1].delay = 0;
	}

	return 0;
}

static int lcd_vlock_param_load_from_dts(struct aml_lcd_drv_s *pdrv, struct device_node *child)
{
	unsigned int para[4];
	int ret;

	pdrv->config.vlock_param[0] = LCD_VLOCK_PARAM_BIT_UPDATE;

	ret = of_property_read_u32_array(child, "vlock_attr", &para[0], 4);
	if (ret == 0) {
		LCDPR("[%d]: find vlock_attr\n", pdrv->index);
		pdrv->config.vlock_param[0] |= LCD_VLOCK_PARAM_BIT_VALID;
		pdrv->config.vlock_param[1] = para[0];
		pdrv->config.vlock_param[2] = para[1];
		pdrv->config.vlock_param[3] = para[2];
		pdrv->config.vlock_param[4] = para[3];
	}

	return 0;
}

static int lcd_vlock_param_load_from_unifykey(struct aml_lcd_drv_s *pdrv, unsigned char *buf)
{
	unsigned char *p;

	p = buf;

	pdrv->config.vlock_param[0] = LCD_VLOCK_PARAM_BIT_UPDATE;
	pdrv->config.vlock_param[1] = *(p + LCD_UKEY_VLOCK_VAL_0);
	pdrv->config.vlock_param[2] = *(p + LCD_UKEY_VLOCK_VAL_1);
	pdrv->config.vlock_param[3] = *(p + LCD_UKEY_VLOCK_VAL_2);
	pdrv->config.vlock_param[4] = *(p + LCD_UKEY_VLOCK_VAL_3);
	if (pdrv->config.vlock_param[1] ||
	    pdrv->config.vlock_param[2] ||
	    pdrv->config.vlock_param[3] ||
	    pdrv->config.vlock_param[4]) {
		LCDPR("[%d]: find vlock_attr\n", pdrv->index);
		pdrv->config.vlock_param[0] |= LCD_VLOCK_PARAM_BIT_VALID;
	}

	return 0;
}

static int lcd_optical_load_from_dts(struct aml_lcd_drv_s *pdrv, struct device_node *child)
{
	unsigned int para[13];
	int ret;

	ret = of_property_read_u32_array(child, "optical_attr", &para[0], 13);
	if (ret == 0) {
		LCDPR("[%d]: find optical_attr\n", pdrv->index);
		pdrv->config.optical.hdr_support = para[0];
		pdrv->config.optical.features = para[1];
		pdrv->config.optical.primaries_r_x = para[2];
		pdrv->config.optical.primaries_r_y = para[3];
		pdrv->config.optical.primaries_g_x = para[4];
		pdrv->config.optical.primaries_g_y = para[5];
		pdrv->config.optical.primaries_b_x = para[6];
		pdrv->config.optical.primaries_b_y = para[7];
		pdrv->config.optical.white_point_x = para[8];
		pdrv->config.optical.white_point_y = para[9];
		pdrv->config.optical.luma_max = para[10];
		pdrv->config.optical.luma_min = para[11];
		pdrv->config.optical.luma_avg = para[12];
	}
	ret = of_property_read_u32_array(child, "optical_adv_val", &para[0], 13);
	if (ret == 0) {
		LCDPR("[%d]: find optical_adv_val\n", pdrv->index);
		pdrv->config.optical.ldim_support = para[0];
		pdrv->config.optical.luma_peak = para[4];
	}

	lcd_optical_vinfo_update(pdrv);

	return 0;
}

static int lcd_optical_load_from_unifykey(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_optical_info_s *opt_info = &pdrv->config.optical;
	char key_str[15];
	unsigned char *para, *p;
	int key_len, len;
	int ret;

	memset(key_str, 0, 15);
	if (pdrv->index == 0)
		sprintf(key_str, "lcd_optical");
	else
		sprintf(key_str, "lcd%d_optical", pdrv->index);

	ret = lcd_unifykey_get_size(key_str, &key_len);
	if (ret < 0)
		return -1;
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s: find ukey: %s\n", pdrv->index, __func__, key_str);

	para = kzalloc(key_len, GFP_KERNEL);
	if (!para)
		return -1;

	ret = lcd_unifykey_get(key_str, para, key_len);
	if (ret < 0) {
		kfree(para);
		return -1;
	}

	/* check parameters */
	len = LCD_UKEY_OPTICAL_SIZE;
	ret = lcd_unifykey_len_check(key_len, len);
	if (ret < 0) {
		LCDERR("[%d]: %s: unifykey parameters length is incorrect\n",
		       pdrv->index, key_str);
		kfree(para);
		return -1;
	}

	/* attr (52Byte) */
	p = para;

	opt_info->hdr_support = (*(p + LCD_UKEY_OPT_HDR_SUPPORT) |
		((*(p + LCD_UKEY_OPT_HDR_SUPPORT + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_HDR_SUPPORT + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_HDR_SUPPORT + 3)) << 24));
	opt_info->features = (*(p + LCD_UKEY_OPT_FEATURES) |
		((*(p + LCD_UKEY_OPT_FEATURES + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_FEATURES + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_FEATURES + 3)) << 24));
	opt_info->primaries_r_x = (*(p + LCD_UKEY_OPT_PRI_R_X) |
		((*(p + LCD_UKEY_OPT_PRI_R_X + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_PRI_R_X + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_PRI_R_X + 3)) << 24));
	opt_info->primaries_r_y = (*(p + LCD_UKEY_OPT_PRI_R_Y) |
		((*(p + LCD_UKEY_OPT_PRI_R_Y + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_PRI_R_Y + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_PRI_R_Y + 3)) << 24));
	opt_info->primaries_g_x = (*(p + LCD_UKEY_OPT_PRI_G_X) |
		((*(p + LCD_UKEY_OPT_PRI_G_X + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_PRI_G_X + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_PRI_G_X + 3)) << 24));
	opt_info->primaries_g_y = (*(p + LCD_UKEY_OPT_PRI_G_Y) |
		((*(p + LCD_UKEY_OPT_PRI_G_Y + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_PRI_G_Y + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_PRI_G_Y + 3)) << 24));
	opt_info->primaries_b_x = (*(p + LCD_UKEY_OPT_PRI_B_X) |
		((*(p + LCD_UKEY_OPT_PRI_B_X + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_PRI_B_X + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_PRI_B_X + 3)) << 24));
	opt_info->primaries_b_y = (*(p + LCD_UKEY_OPT_PRI_B_Y) |
		((*(p + LCD_UKEY_OPT_PRI_B_Y + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_PRI_B_Y + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_PRI_B_Y + 3)) << 24));
	opt_info->white_point_x = (*(p + LCD_UKEY_OPT_WHITE_X) |
		((*(p + LCD_UKEY_OPT_WHITE_X + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_WHITE_X + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_WHITE_X + 3)) << 24));
	opt_info->white_point_y = (*(p + LCD_UKEY_OPT_WHITE_Y) |
		((*(p + LCD_UKEY_OPT_WHITE_Y + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_WHITE_Y + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_WHITE_Y + 3)) << 24));
	opt_info->luma_max = (*(p + LCD_UKEY_OPT_LUMA_MAX) |
		((*(p + LCD_UKEY_OPT_LUMA_MAX + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_LUMA_MAX + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_LUMA_MAX + 3)) << 24));
	opt_info->luma_min = (*(p + LCD_UKEY_OPT_LUMA_MIN) |
		((*(p + LCD_UKEY_OPT_LUMA_MIN + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_LUMA_MIN + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_LUMA_MIN + 3)) << 24));
	opt_info->luma_avg = (*(p + LCD_UKEY_OPT_LUMA_AVG) |
		((*(p + LCD_UKEY_OPT_LUMA_AVG + 1)) << 8) |
		((*(p + LCD_UKEY_OPT_LUMA_AVG + 2)) << 16) |
		((*(p + LCD_UKEY_OPT_LUMA_AVG + 3)) << 24));

	opt_info->ldim_support = *(p + LCD_UKEY_OPT_ADV_FLAG0);
	opt_info->luma_peak = *(unsigned int *)(p + LCD_UKEY_OPT_ADV_VAL1);

	kfree(para);

	lcd_optical_vinfo_update(pdrv);

	return 0;
}

static int lcd_config_load_from_dts(struct aml_lcd_drv_s *pdrv)
{
	struct device_node *child;
	struct lcd_config_s *pconf = &pdrv->config;
	struct lcd_detail_timing_s *ptiming;
	struct lcd_timing_s *tims = &pdrv->config.timing;
	union lcd_ctrl_config_u *pctrl = &pdrv->config.control;
	struct phy_config_s *phy_cfg = &pdrv->config.phy_cfg;
	struct phy_attr_s *phy = NULL;
	unsigned int para[10], val;
	const char *str;
	char str_info[128];
	int i, str_info_len = 0, ret = 0;

	if (!pdrv->dev->of_node) {
		LCDERR("[%d]: dev of_node is null\n", pdrv->index);
		return -1;
	}

	child = of_get_child_by_name(pdrv->dev->of_node, pconf->propname);
	if (!child) {
		LCDERR("[%d]: failed to get %s\n", pdrv->index, pconf->propname);
		return -1;
	}

	ret = of_property_read_string(child, "model_name", &str);
	if (ret) {
		LCDERR("[%d]: failed to get model_name\n", pdrv->index);
		strncpy(pconf->basic.model_name, pconf->propname, MOD_LEN_MAX);
	} else {
		strncpy(pconf->basic.model_name, str, MOD_LEN_MAX);
	}
	/* ensure string ending */
	pconf->basic.model_name[MOD_LEN_MAX - 1] = '\0';

	ret = of_property_read_string(child, "interface", &str);
	if (ret) {
		LCDERR("[%d]: failed to get interface\n", pdrv->index);
		str = "invalid";
	}
	pconf->basic.lcd_type = lcd_type_str_to_type(str);

	ret = of_property_read_u32(child, "config_check", &val);
	if (ret == 0)
		pconf->basic.config_check = val ? 0x3 : 0x2;

	ret = of_property_read_u32_array(child, "basic_setting", &para[0], 7);
	if (ret) {
		LCDERR("[%d]: failed to get basic_setting\n", pdrv->index);
		return -1;
	}

	ptiming = lcd_timing_alloc(pdrv);
	if (!ptiming) {
		LCDERR("[%d] dft_timing alloc fail\n", pdrv->index);
		return -1;
	}
	memset(ptiming, 0, sizeof(*ptiming));
	tims->dft_timing = tims->timings[0];

	ptiming->h_active = para[0];
	ptiming->v_active = para[1];
	ptiming->h_period = para[2];
	ptiming->v_period = para[3];
	ptiming->lcd_bits = para[4] * 3;
	pconf->basic.screen_width = para[5];
	pconf->basic.screen_height = para[6];

	ret = of_property_read_u32_array(child, "range_setting", &para[0], 6);
	if (ret) {
		LCDPR("[%d]: no range_setting\n", pdrv->index);
		ptiming->h_period_min = ptiming->h_period;
		ptiming->h_period_max = ptiming->h_period;
		ptiming->v_period_min = ptiming->v_period;
		ptiming->v_period_max = ptiming->v_period;
		ptiming->pclk_min = 0;
		ptiming->pclk_max = 0;
	} else {
		ptiming->h_period_min = para[0];
		ptiming->h_period_max = para[1];
		ptiming->v_period_min = para[2];
		ptiming->v_period_max = para[3];
		ptiming->pclk_min = para[4];
		ptiming->pclk_max = para[5];
	}

	ret = of_property_read_u32_array(child, "range_frame_rate", &para[0], 6);
	if (ret == 0) {
		ptiming->frame_rate_min = para[0];
		ptiming->frame_rate_max = para[1];
	}

	ret = of_property_read_u32_array(child, "lcd_timing", &para[0], 6);
	if (ret) {
		LCDERR("[%d]: failed to get lcd_timing\n", pdrv->index);
		lcd_timing_free_last(pdrv);
		return -1;
	}
	ptiming->hsync_width = (unsigned short)(para[0]);
	ptiming->hsync_bp = (unsigned short)(para[1]);
	ptiming->hsync_fp = ptiming->h_period - ptiming->h_active -
			ptiming->hsync_width - ptiming->hsync_bp;
	ptiming->hsync_pol = (unsigned short)(para[2]);
	ptiming->vsync_width = (unsigned short)(para[3]);
	ptiming->vsync_bp = (unsigned short)(para[4]);
	ptiming->vsync_fp = ptiming->v_period - ptiming->v_active -
			ptiming->vsync_width - ptiming->vsync_bp;
	ptiming->vsync_pol = (unsigned short)(para[5]);
	ret = of_property_read_u32(child, "ppc_mode", &val);
	if (ret)
		pconf->timing.ppc = 1;
	else
		pconf->timing.ppc = val;

	ret = of_property_read_u32(child, "clk_mode", &val);
	if (ret)
		pconf->timing.clk_mode = LCD_CLK_MODE_DEPENDENCE;
	else
		pconf->timing.clk_mode = val;

	ret = of_property_read_u32_array(child, "pre_de", &para[0], 2);
	if (ret) {
		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
			LCDERR("failed to get pre_de\n");
		pconf->timing.pre_de_h = 0;
		pconf->timing.pre_de_v = 0;
	} else {
		pconf->timing.pre_de_h = (unsigned short)(para[0]);
		pconf->timing.pre_de_v = (unsigned short)(para[1]);
	}

	ret = of_property_read_u32_array(child, "clk_attr", &para[0], 4);
	if (ret) {
		LCDERR("[%d]: failed to get clk_attr\n", pdrv->index);
		ptiming->fr_adjust_type = 0;
		pconf->timing.ss_level = 0;
		pconf->timing.ss_freq = 0;
		pconf->timing.ss_mode = 0;
		pconf->timing.pll_flag = 1;
		ptiming->pixel_clk = 60;
	} else {
		ptiming->fr_adjust_type = (unsigned char)(para[0]);
		pconf->timing.ss_level = para[1] & 0xff;
		pconf->timing.ss_freq = (para[1] >> 8) & 0xf;
		pconf->timing.ss_mode = (para[1] >> 12) & 0xf;
		pconf->timing.pll_flag = para[2] & 0xf;
		ptiming->pixel_clk = para[3];
	}
	ptiming->switch_type = LCD_VMODE_SWITCH_NONE;
	ptiming->ss_force = 0;
	ptiming->ss_freq = pconf->timing.ss_freq;
	ptiming->ss_level = pconf->timing.ss_level;
	ptiming->ss_mode = pconf->timing.ss_mode;

	lcd_clk_frame_rate_init(ptiming);
	lcd_default_to_basic_timing_init_config(pdrv);
	lcd_config_timing_check(pdrv, ptiming);

	ret = of_property_read_u32(child, "custom_pinmux", &val);
	if (ret) {
		ret = of_property_read_u32(child, "customer_pinmux", &val);
		if (ret)
			pconf->custom_pinmux = 0;
		else
			pconf->custom_pinmux = val;
	} else {
		pconf->custom_pinmux = val;
	}

	ret = of_property_read_u32(child, "fr_auto_disable", &val);
	if (ret) {
		ret = of_property_read_u32(child, "fr_auto_custom", &val);
		if (ret)
			pconf->fr_auto_cus = 0;
		else
			pconf->fr_auto_cus = val;
	} else {
		if (val)
			pconf->fr_auto_cus = 0xff;
		else
			pconf->fr_auto_cus = 0;
	}

	str_info_len += sprintf(str_info + str_info_len, "ppc:%d, ",
			pconf->timing.ppc);
	str_info_len += sprintf(str_info + str_info_len, "clk_mode:%d, ",
			pconf->timing.clk_mode);
	if (pconf->timing.pre_de_h || pconf->timing.pre_de_v) {
		str_info_len += sprintf(str_info + str_info_len, "pre_de:%d,%d, ",
				pconf->timing.pre_de_h, pconf->timing.pre_de_v);
	}
	str_info_len += sprintf(str_info + str_info_len, "cfg_chk:0x%x, ",
			pconf->basic.config_check);
	str_info_len += sprintf(str_info + str_info_len, "cus_pinmux:%d, ",
			pconf->custom_pinmux);
	sprintf(str_info + str_info_len, "afr_cus:0x%x", pconf->fr_auto_cus);

	LCDPR("[%d]: load dts config: %s, %s, %dbit, %dx%d, %s\n",
	      pdrv->index, pconf->basic.model_name,
	      lcd_type_type_to_str(pconf->basic.lcd_type),
	      ptiming->lcd_bits, ptiming->h_active, ptiming->v_active,
	      str_info);

	switch (pconf->basic.lcd_type) {
	case LCD_RGB:
		ret = of_property_read_u32_array(child, "rgb_attr", &para[0], 6);
		if (ret) {
			LCDERR("[%d]: failed to get rgb_attr\n", pdrv->index);
			return -1;
		}
		pctrl->rgb_cfg.type = para[0];
		pctrl->rgb_cfg.clk_pol = para[1];
		pctrl->rgb_cfg.de_valid = para[2];
		pctrl->rgb_cfg.sync_valid = para[3];
		pctrl->rgb_cfg.rb_swap = para[4];
		pctrl->rgb_cfg.bit_swap = para[5];
		break;
	case LCD_BT656:
	case LCD_BT1120:
		ret = of_property_read_u32_array(child, "bt_attr", &para[0], 8);
		if (ret) {
			LCDERR("[%d]: failed to get bt_attr\n", pdrv->index);
			return -1;
		}
		pctrl->bt_cfg.clk_phase = para[0];
		pctrl->bt_cfg.field_type = para[1];
		pctrl->bt_cfg.mode_422 = para[2];
		pctrl->bt_cfg.yc_swap = para[3];
		pctrl->bt_cfg.cbcr_swap = para[4];
		break;
	case LCD_LVDS:
		ret = of_property_read_u32_array(child, "lvds_attr", &para[0], 5);
		if (ret) {
			LCDPR("[%d]: failed to get lvds_attr\n", pdrv->index);
			return -1;
		}
		pctrl->lvds_cfg.lvds_repack = para[0];
		pctrl->lvds_cfg.dual_port = para[1];
		pctrl->lvds_cfg.pn_swap = para[2];
		pctrl->lvds_cfg.port_swap = para[3];
		pctrl->lvds_cfg.lane_reverse = para[4];

		ret = of_property_read_u32_array(child, "phy_attr", &para[0], 2);
		if (ret) {
			LCDPR("[%d]: failed to get phy_attr\n", pdrv->index);
			pctrl->lvds_cfg.phy_vswing = 0x5;
			pctrl->lvds_cfg.phy_preem  = 0x1;
		} else {
			pctrl->lvds_cfg.phy_vswing = para[0];
			pctrl->lvds_cfg.phy_preem = para[1];
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: phy vswing_level=0x%x, preem_level=0x%x\n",
				      pdrv->index,
				      pctrl->lvds_cfg.phy_vswing,
				      pctrl->lvds_cfg.phy_preem);
			}
		}

		phy_cfg->vswing_level = pctrl->lvds_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->lvds_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->lvds_cfg.phy_preem;
		break;
	case LCD_VBYONE:
		ret = of_property_read_u32_array(child, "vbyone_attr", &para[0], 4);
		if (ret) {
			LCDERR("[%d]: failed to get vbyone_attr\n", pdrv->index);
			return -1;
		}
		pctrl->vbyone_cfg.lane_count = para[0];
		pctrl->vbyone_cfg.region_num = para[1];
		pctrl->vbyone_cfg.byte_mode = para[2];
		pctrl->vbyone_cfg.color_fmt = para[3];

		pctrl->vbyone_cfg.slice = pdrv->config.timing.ppc ? pdrv->config.timing.ppc : 1;

		ret = of_property_read_u32_array(child, "vbyone_intr_enable", &para[0], 2);
		if (ret) {
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
				LCDERR("[%d]: failed to get vbyone_intr_enable\n", pdrv->index);
		} else {
			pctrl->vbyone_cfg.intr_en = para[0];
			pctrl->vbyone_cfg.vsync_intr_en = para[1];
		}
		ret = of_property_read_u32_array(child, "phy_attr", &para[0], 2);
		if (ret) {
			LCDPR("[%d]: failed to get phy_attr\n", pdrv->index);
			pctrl->vbyone_cfg.phy_vswing = 0x5;
			pctrl->vbyone_cfg.phy_preem  = 0x1;
		} else {
			pctrl->vbyone_cfg.phy_vswing = para[0];
			pctrl->vbyone_cfg.phy_preem = para[1];
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: phy vswing_level=0x%x, preem_level=0x%x\n",
				      pdrv->index,
				      pctrl->vbyone_cfg.phy_vswing,
				      pctrl->vbyone_cfg.phy_preem);
			}
		}

		phy_cfg->vswing_level = pctrl->vbyone_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->vbyone_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->vbyone_cfg.phy_preem;

		ret = of_property_read_u32(child, "vbyone_ctrl_flag", &val);
		if (ret) {
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
				LCDPR("[%d]: failed to get vbyone_ctrl_flag\n", pdrv->index);
		} else {
			pctrl->vbyone_cfg.ctrl_flag = val;
			LCDPR("[%d]: vbyone ctrl_flag=0x%x\n",
			      pdrv->index, pctrl->vbyone_cfg.ctrl_flag);
		}
		if (pctrl->vbyone_cfg.ctrl_flag & 0x7) {
			ret = of_property_read_u32_array(child, "vbyone_ctrl_timing", &para[0], 3);
			if (ret) {
				LCDPR("[%d]: failed to get vbyone_ctrl_timing\n", pdrv->index);
			} else {
				pctrl->vbyone_cfg.power_on_reset_delay = para[0];
				pctrl->vbyone_cfg.hpd_data_delay = para[1];
				pctrl->vbyone_cfg.cdr_training_hold = para[2];
			}
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: power_on_reset_delay: %d\n",
				      pdrv->index,
				      pctrl->vbyone_cfg.power_on_reset_delay);
				LCDPR("[%d]: hpd_data_delay: %d\n",
				      pdrv->index,
				      pctrl->vbyone_cfg.hpd_data_delay);
				LCDPR("[%d]: cdr_training_hold: %d\n",
				      pdrv->index,
				      pctrl->vbyone_cfg.cdr_training_hold);
			}
		}
		ret = of_property_read_u32_array(child, "hw_filter", &para[0], 2);
		if (ret) {
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
				LCDPR("[%d]: failed to get hw_filter\n", pdrv->index);
		} else {
			pctrl->vbyone_cfg.hw_filter_time = para[0];
			pctrl->vbyone_cfg.hw_filter_cnt = para[1];
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: vbyone hw_filter=0x%x 0x%x\n",
				      pdrv->index,
				      pctrl->vbyone_cfg.hw_filter_time,
				      pctrl->vbyone_cfg.hw_filter_cnt);
			}
		}
		break;
	case LCD_MLVDS:
		ret = of_property_read_u32_array(child, "minilvds_attr", &para[0], 6);
		if (ret) {
			LCDERR("[%d]: failed to get minilvds_attr\n", pdrv->index);
			return -1;
		}
		pctrl->mlvds_cfg.channel_num = para[0];
		pctrl->mlvds_cfg.channel_sel0 = para[1];
		pctrl->mlvds_cfg.channel_sel1 = para[2];
		pctrl->mlvds_cfg.clk_phase = para[3];
		pctrl->mlvds_cfg.pn_swap = para[4];
		pctrl->mlvds_cfg.bit_swap = para[5];

		ret = of_property_read_u32_array(child, "phy_attr", &para[0], 2);
		if (ret) {
			LCDPR("[%d]: failed to get phy_attr\n", pdrv->index);
			pctrl->mlvds_cfg.phy_vswing = 0x5;
			pctrl->mlvds_cfg.phy_preem  = 0x1;
		} else {
			pctrl->mlvds_cfg.phy_vswing = para[0];
			pctrl->mlvds_cfg.phy_preem = para[1];
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: phy vswing_level=0x%x, preem_level=0x%x\n",
				      pdrv->index,
				      pctrl->mlvds_cfg.phy_vswing,
				      pctrl->mlvds_cfg.phy_preem);
			}
		}

		phy_cfg->vswing_level = pctrl->mlvds_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->mlvds_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->mlvds_cfg.phy_preem;
		break;
	case LCD_P2P:
		ret = of_property_read_u32_array(child, "p2p_attr", &para[0], 6);
		if (ret) {
			LCDERR("[%d]: failed to get p2p_attr\n", pdrv->index);
			return -1;
		}
		pctrl->p2p_cfg.p2p_type = para[0];
		pctrl->p2p_cfg.lane_num = para[1];
		pctrl->p2p_cfg.channel_sel0 = para[2];
		pctrl->p2p_cfg.channel_sel1 = para[3];
		pctrl->p2p_cfg.pn_swap = para[4];
		pctrl->p2p_cfg.bit_swap = para[5];

		ret = of_property_read_u32_array(child, "phy_attr", &para[0], 2);
		if (ret) {
			LCDPR("[%d]: failed to get phy_attr\n", pdrv->index);
			pctrl->p2p_cfg.phy_vswing = 0x5;
			pctrl->p2p_cfg.phy_preem  = 0x1;
		} else {
			pctrl->p2p_cfg.phy_vswing = para[0];
			pctrl->p2p_cfg.phy_preem = para[1];
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: phy vswing_level=0x%x, preem_level=0x%x\n",
				      pdrv->index,
				      pctrl->p2p_cfg.phy_vswing,
				      pctrl->p2p_cfg.phy_preem);
			}
		}

		phy_cfg->vswing_level = pctrl->p2p_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->p2p_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->p2p_cfg.phy_preem;
		break;
	case LCD_MIPI:
		ret = of_property_read_u32_array(child, "mipi_attr", &para[0], 8);
		if (ret) {
			LCDERR("[%d]: failed to get mipi_attr\n", pdrv->index);
			return -1;
		}
		pctrl->mipi_cfg.lane_num = para[0];
		pctrl->mipi_cfg.bit_rate_max = para[1];
		pctrl->mipi_cfg.operation_mode_init = para[3];
		pctrl->mipi_cfg.operation_mode_display = para[4];
		pctrl->mipi_cfg.video_mode_type = para[5];
		pctrl->mipi_cfg.clk_always_hs = para[6];

#ifdef CONFIG_AMLOGIC_LCD_TABLET
		lcd_mipi_dsi_init_table_detect(pdrv, child);
#endif
#ifdef CONFIG_AMLOGIC_LCD_EXTERN
		ret = of_property_read_u32_array(child, "extern_init", &para[0], 1);
		if (ret) {
			LCDPR("[%d]: failed to get extern_init\n", pdrv->index);
		} else {
			pctrl->mipi_cfg.extern_init = para[0];
			lcd_resource_add(pdrv, LCD_RES_EXTERN, para[0]);
			lcd_extern_dev_index_add(pdrv->index, para[0]);
		}
#endif

		phy_cfg->vswing_level = 0;
		phy_cfg->preem_level = 0;
		break;
	case LCD_EDP:
		ret = of_property_read_u32_array(child, "edp_attr", &para[0], 9);
		if (ret) {
			LCDERR("[%d]: failed to get edp_attr\n", pdrv->index);
			return -1;
		}
		pctrl->edp_cfg.max_lane_count = (unsigned char)para[0];
		pctrl->edp_cfg.max_link_rate = (unsigned char)(para[1] < 0x6 ? 0 : para[1]);
		pctrl->edp_cfg.training_mode = (unsigned char)para[2];
		pctrl->edp_cfg.edid_en = (unsigned char)para[3];

		ret = of_property_read_u32_array(child, "phy_attr", &para[0], 2);
		if (ret) {
			LCDPR("[%d]: failed to get phy_attr\n", pdrv->index);
			pctrl->edp_cfg.phy_vswing_preset = 0x5;
			pctrl->edp_cfg.phy_preem_preset  = 0x1;
		} else {
			pctrl->edp_cfg.phy_vswing_preset = para[0];
			pctrl->edp_cfg.phy_preem_preset = para[1];
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("[%d]: phy vswing_level=0x%x, preem_level=0x%x\n",
				      pdrv->index,
				      pctrl->edp_cfg.phy_vswing_preset,
				      pctrl->edp_cfg.phy_preem_preset);
			}
		}

		phy_cfg->vswing_level = pctrl->edp_cfg.phy_vswing_preset & 0xf;
		phy_cfg->preem_level = pctrl->edp_cfg.phy_preem_preset;
		break;
	default:
		LCDERR("[%d]: invalid lcd type\n", pdrv->index);
		break;
	}

	phy = lcd_phy_alloc(pdrv);// first parse, it is only be phy_cfg->phys[0];
	if (!phy) {
		LCDERR("%s phy alloc fail\n", __func__);
		return -1;
	}
	memset(phy, 0, sizeof(*phy));
	phy_cfg->act_phy = phy_cfg->phys[0];
	lcd_phy_param_preset(pdrv);
	lcd_lane_map_preset(pdrv);
	phy->ss.freq = 255;
	phy->ss.level = 255;
	phy->ss.mode = 255;

	ret = of_property_read_u32_array(child, "phy_adv_attr", &para[0], 5);
	if (ret)
		goto config_phy_adv_attr_done;

	phy_cfg->flag = para[0];
	phy->vswing   = para[1];
	phy->vcm      = para[2];
	phy->ref_bias = para[3];
	phy->odt      = para[4];
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("%s: ctrl_flag=0x%x vsw=0x%08x vcm=0x%x, ref_bias=0x%x, odt=0x%x\n",
			__func__, phy_cfg->flag, phy->vswing, phy->vcm,
			phy->ref_bias, phy->odt);
	}
	ret = of_property_read_variable_u32_array(child, "phy_lane_ctrl", &para[0], 1, 32);
	if (ret <= 0 || ((phy_cfg->flag & (0x3 << 12)) == 0))
		goto config_phy_adv_attr_done;

	for (i = 0; i < phy_cfg->lane_num; i++) {
		if (i >= ret)
			break;

		if (phy_cfg->flag & (1 << 12))
			phy->lane[i].preem = para[i] & 0xffff;

		if (phy_cfg->flag & (1 << 13))
			phy->lane[i].amp   = para[i] >> 16;

		if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
			LCDPR("%s: lane[%d]: preem=0x%x amp=0x%x\n",
				__func__, i, phy->lane[i].preem, phy->lane[i].amp);
	}
config_phy_adv_attr_done:

	lcd_vlock_param_load_from_dts(pdrv, child);
	ret = lcd_power_load_from_dts(pdrv, child);

	lcd_optical_load_from_dts(pdrv, child);

	lcd_cus_ctrl_load_from_dts(pdrv, child);

	//fix ss in detail timing and phy_attr if not config
	lcd_ss_config_fix(pdrv);

	ret = of_property_read_u32(child, "backlight_index", &para[0]);
	if (ret) {
		LCDPR("[%d]: failed to get backlight_index\n", pdrv->index);
		pconf->backlight_index = 0xff;
	} else {
		pconf->backlight_index = para[0];
#ifdef CONFIG_AMLOGIC_BACKLIGHT
		lcd_resource_add(pdrv, LCD_RES_BACKLIGHT, pconf->backlight_index);
		aml_bl_index_add(pdrv->index, pconf->backlight_index);
#endif
	}

	return ret;
}

static int lcd_config_load_from_unifykey_v2(struct aml_lcd_drv_s *pdrv,
					    unsigned char *p,
					    unsigned int key_len,
					    unsigned int offset)
{
	struct aml_lcd_unifykey_header_s *lcd_header;
	struct phy_config_s *phy_cfg = &pdrv->config.phy_cfg;
	struct phy_attr_s *phy = NULL;
	unsigned int len, size;
	unsigned char version;
	int i, ret;

	if (!phy_cfg->phys[0])
		return -1;

	lcd_header = (struct aml_lcd_unifykey_header_s *)p;
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		lcd_unifykey_header_print(p);

	/* step 2: check lcd parameters */
	len = offset + lcd_header->block_cur_size;
	ret = lcd_unifykey_len_check(key_len, len);
	if (ret < 0) {
		LCDERR("unifykey parameters length is incorrect\n");
		return -1;
	}

	/*phy 356byte*/
	phy_cfg->flag = (*(p + LCD_UKEY_PHY_ATTR_FLAG) |
		((*(p + LCD_UKEY_PHY_ATTR_FLAG + 1)) << 8) |
		((*(p + LCD_UKEY_PHY_ATTR_FLAG + 2)) << 16) |
		((*(p + LCD_UKEY_PHY_ATTR_FLAG + 3)) << 24));
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("%s: ctrl_flag=0x%x\n", __func__, phy_cfg->flag);

	phy = phy_cfg->phys[0];
	if (phy_cfg->flag & PHY_BIT_VSWING) {
		phy->vswing = (*(p + LCD_UKEY_PHY_ATTR_0) |
				*(p + LCD_UKEY_PHY_ATTR_0 + 1) << 8);
	}
	if (phy_cfg->flag & PHY_BIT_VCM) {
		phy->vcm = (*(p + LCD_UKEY_PHY_ATTR_1) |
				*(p + LCD_UKEY_PHY_ATTR_1 + 1) << 8);
	}
	if (phy_cfg->flag & PHY_BIT_REF_BIAS) {
		phy->ref_bias = (*(p + LCD_UKEY_PHY_ATTR_2) |
				*(p + LCD_UKEY_PHY_ATTR_2 + 1) << 8);
	}
	if (phy_cfg->flag & PHY_BIT_ODT) {
		phy->odt = (*(p + LCD_UKEY_PHY_ATTR_3) |
				*(p + LCD_UKEY_PHY_ATTR_3 + 1) << 8);
	}
	if (phy_cfg->flag & PHY_BIT_CV_MODE) {
		phy->cv_mode = (*(p + LCD_UKEY_PHY_ATTR_4) |
				*(p + LCD_UKEY_PHY_ATTR_4 + 1) << 8);
	}
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("%s: vswing=0x%x, vcm=0x%x, ref_bias=0x%x, odt=0x%x, cv_mode=%d\n",
		      __func__, phy->vswing, phy->vcm, phy->ref_bias,
		      phy->cv_mode, phy->odt);
	}

	if (phy_cfg->flag & PHY_BIT_LANE_PREEM) {
		for (i = 0; i < phy_cfg->lane_num; i++) {
			phy->lane[i].preem =
				*(p + LCD_UKEY_PHY_LANE_CTRL + 4 * i) |
				(*(p + LCD_UKEY_PHY_LANE_CTRL + 4 * i + 1) << 8);
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("%s: lane[%d]: preem=0x%x\n",
				      __func__, i,
				      phy->lane[i].preem);
			}
		}
	}

	if (phy_cfg->flag & PHY_BIT_LANE_AMP) {
		for (i = 0; i < phy_cfg->lane_num; i++) {
			phy->lane[i].amp =
				*(p + LCD_UKEY_PHY_LANE_CTRL + 4 * i + 2) |
				(*(p + LCD_UKEY_PHY_LANE_CTRL + 4 * i + 3) << 8);
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("%s: lane[%d]: amp=0x%x\n",
				      __func__, i,
				      phy->lane[i].amp);
			}
		}
	}

	if (phy_cfg->flag & PHY_BIT_LANE_SEL) {
		for (i = 0; i < phy_cfg->lane_num; i++) {
			phy_cfg->ch_ctrl[i].sel = *(p + LCD_UKEY_PHY_LANE_SEL + i);
			if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
				LCDPR("%s: lane[%d]: sel=0x%x\n",
					__func__, i, phy_cfg->ch_ctrl[i].sel);
			}
		}
	}

	size = LCD_UKEY_CUS_CTRL_ATTR_FLAG;
	version = lcd_header->version;
	lcd_cus_ctrl_load_from_unifykey(pdrv, (p + size), (key_len - size), version);

	return 0;
}

static int lcd_config_load_from_unifykey_v3(struct aml_lcd_drv_s *pdrv,
					    unsigned char *p,
					    unsigned int key_len,
					    unsigned int offset)
{
	struct aml_lcd_unifykey_header_s *lcd_header;
	unsigned int len, size;
	unsigned char version;
	int ret;

	lcd_header = (struct aml_lcd_unifykey_header_s *)p;
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		lcd_unifykey_header_print(p);

	/* step 2: check lcd parameters */
	len = offset + lcd_header->block_cur_size;
	ret = lcd_unifykey_len_check(key_len, len);
	if (ret < 0) {
		LCDERR("unifykey parameters length is incorrect\n");
		return -1;
	}

	size = LCD_UKEY_CUS_CTRL_ATTR_FLAG_V3;
	version = lcd_header->version;
	lcd_cus_ctrl_load_from_unifykey(pdrv, (p + size), (key_len - size), version);

	return 0;
}

static int lcd_config_load_from_unifykey(struct aml_lcd_drv_s *pdrv, char *key_str)
{
	unsigned char *para;
	int key_len, len;
	unsigned char *p, val;
	const char *str;
	struct aml_lcd_unifykey_header_s *lcd_header;
	struct lcd_config_s *pconf = &pdrv->config;
	struct lcd_detail_timing_s *ptiming = NULL;
	struct phy_config_s *phy_cfg = &pdrv->config.phy_cfg;
	struct phy_attr_s *phy = NULL;
	union lcd_ctrl_config_u *pctrl = &pdrv->config.control;
	unsigned int temp, lcd_bits = 0;
	char str_info[128];
	int str_info_len = 0, ret;

	ret = lcd_unifykey_get_size(key_str, &key_len);
	if (ret)
		return -1;

	para = kzalloc(key_len, GFP_KERNEL);
	if (!para)
		return -1;

	ret = lcd_unifykey_get(key_str, para, key_len);
	if (ret < 0) {
		kfree(para);
		return -1;
	}

	/* step 1: check header size */
	lcd_header = (struct aml_lcd_unifykey_header_s *)para;
	len = LCD_UKEY_DATA_LEN_V1; /*10+36+18+31+20*/
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		lcd_unifykey_header_print(para);

	/* step 2: check lcd parameters */
	ret = lcd_unifykey_len_check(key_len, len);
	if (ret < 0) {
		LCDERR("[%d]: unifykey parameters length is incorrect\n", pdrv->index);
		goto load_from_unifykey_exit;
	}

	/* panel_type update */
	sprintf(pconf->propname, "%s", "unifykey");

	/* basic: 36byte */
	p = para;
	str = (const char *)(p + LCD_UKEY_HEAD_SIZE);
	strncpy(pconf->basic.model_name, str, MOD_LEN_MAX);
	/* ensure string ending */
	pconf->basic.model_name[MOD_LEN_MAX - 1] = '\0';
	temp = *(p + LCD_UKEY_INTERFACE);
	pconf->basic.lcd_type = temp & 0x3f;
	pconf->basic.config_check = (temp >> 6) & 0x3;
	temp = *(p + LCD_UKEY_LCD_BITS_CFMT);
	lcd_bits = (temp & 0x3f) * 3;
	pconf->basic.screen_width = (*(p + LCD_UKEY_SCREEN_WIDTH) |
		((*(p + LCD_UKEY_SCREEN_WIDTH + 1)) << 8));
	pconf->basic.screen_height = (*(p + LCD_UKEY_SCREEN_HEIGHT) |
		((*(p + LCD_UKEY_SCREEN_HEIGHT + 1)) << 8));

	/* timing: 18byte */
	ptiming = lcd_timing_alloc(pdrv);
	if (!ptiming) {
		ret = -1;
		LCDERR("%s timing alloc fail\n", __func__);
		goto load_from_unifykey_exit;
	}
	memset(ptiming, 0, sizeof(*ptiming));
	ptiming->h_active = (*(p + LCD_UKEY_H_ACTIVE) |
		((*(p + LCD_UKEY_H_ACTIVE + 1)) << 8));
	ptiming->v_active = (*(p + LCD_UKEY_V_ACTIVE)) |
		((*(p + LCD_UKEY_V_ACTIVE + 1)) << 8);
	ptiming->h_period = (*(p + LCD_UKEY_H_PERIOD)) |
		((*(p + LCD_UKEY_H_PERIOD + 1)) << 8);
	ptiming->v_period = (*(p + LCD_UKEY_V_PERIOD)) |
		((*(p + LCD_UKEY_V_PERIOD + 1)) << 8);
	temp = *(unsigned short *)(p + LCD_UKEY_HS_WIDTH_POL);
	ptiming->hsync_width = temp & 0xfff;
	ptiming->hsync_pol = (temp >> 12) & 0xf;
	ptiming->hsync_bp = (*(p + LCD_UKEY_HS_BP) |
		((*(p + LCD_UKEY_HS_BP + 1)) << 8));
	ptiming->hsync_fp = ptiming->h_period - ptiming->h_active -
			ptiming->hsync_width - ptiming->hsync_bp;
	temp = *(unsigned short *)(p + LCD_UKEY_VS_WIDTH_POL);
	ptiming->vsync_width = temp & 0xfff;
	ptiming->vsync_pol = (temp >> 12) & 0xf;
	ptiming->vsync_bp = (*(p + LCD_UKEY_VS_BP) |
		((*(p + LCD_UKEY_VS_BP + 1)) << 8));
	ptiming->vsync_fp = ptiming->v_period - ptiming->v_active -
			ptiming->vsync_width - ptiming->vsync_bp;
	pconf->timing.pre_de_h = *(p + LCD_UKEY_PRE_DE_H);
	pconf->timing.pre_de_v = *(p + LCD_UKEY_PRE_DE_V);

	/* customer: 31byte */
	ptiming->fr_adjust_type = *(p + LCD_UKEY_FR_ADJ_TYPE);
	pconf->timing.ss_level = *(p + LCD_UKEY_SS_LEVEL);
	val = *(p + LCD_UKEY_CUST_VAL0);
	pconf->timing.clk_mode = (val >> 4) & 0xf;
	pconf->timing.pll_flag = val & 0xf;
	ptiming->pixel_clk = (*(p + LCD_UKEY_PCLK) |
		((*(p + LCD_UKEY_PCLK + 1)) << 8) |
		((*(p + LCD_UKEY_PCLK + 2)) << 16) |
		((*(p + LCD_UKEY_PCLK + 3)) << 24));
	ptiming->h_period_min = (*(p + LCD_UKEY_H_PERIOD_MIN) |
		((*(p + LCD_UKEY_H_PERIOD_MIN + 1)) << 8));
	ptiming->h_period_max = (*(p + LCD_UKEY_H_PERIOD_MAX) |
		((*(p + LCD_UKEY_H_PERIOD_MAX + 1)) << 8));
	ptiming->v_period_min = (*(p + LCD_UKEY_V_PERIOD_MIN) |
		((*(p + LCD_UKEY_V_PERIOD_MIN + 1)) << 8));
	ptiming->v_period_max = (*(p + LCD_UKEY_V_PERIOD_MAX) |
		((*(p + LCD_UKEY_V_PERIOD_MAX + 1)) << 8));
	ptiming->pclk_min = (*(p + LCD_UKEY_PCLK_MIN) |
		((*(p + LCD_UKEY_PCLK_MIN + 1)) << 8) |
		((*(p + LCD_UKEY_PCLK_MIN + 2)) << 16) |
		((*(p + LCD_UKEY_PCLK_MIN + 3)) << 24));
	ptiming->pclk_max = (*(p + LCD_UKEY_PCLK_MAX) |
		((*(p + LCD_UKEY_PCLK_MAX + 1)) << 8) |
		((*(p + LCD_UKEY_PCLK_MAX + 2)) << 16) |
		((*(p + LCD_UKEY_PCLK_MAX + 3)) << 24));
	ptiming->frame_rate_min = *(p + LCD_UKEY_FRAME_RATE_MIN);
	ptiming->frame_rate_max = *(p + LCD_UKEY_FRAME_RATE_MAX);
	ptiming->lcd_bits = lcd_bits;

	val = *(p + LCD_UKEY_CUST_VAL1);
	pconf->timing.ppc = (val >> 4) & 0xf;
	pconf->custom_pinmux = val & 0xf;

	pconf->fr_auto_cus = *(p + LCD_UKEY_FR_AUTO_CUS);
	ptiming->switch_type = LCD_VMODE_SWITCH_NONE;
	ptiming->ss_force = 0;
	ptiming->ss_freq = 255;
	ptiming->ss_level = pconf->timing.ss_level;
	ptiming->ss_mode = 255;

	pconf->timing.dft_timing = pconf->timing.timings[0];
	lcd_clk_frame_rate_init(ptiming);
	lcd_config_timing_check(pdrv, ptiming);
	lcd_default_to_basic_timing_init_config(pdrv);

	str_info_len += sprintf(str_info + str_info_len, "ppc:%d, ",
			pconf->timing.ppc);
	str_info_len += sprintf(str_info + str_info_len, "clk_mode:%d, ",
			pconf->timing.clk_mode);
	if (pconf->timing.pre_de_h || pconf->timing.pre_de_v) {
		str_info_len += sprintf(str_info + str_info_len, "pre_de:%d,%d, ",
				pconf->timing.pre_de_h, pconf->timing.pre_de_v);
	}
	str_info_len += sprintf(str_info + str_info_len, "cfg_chk:0x%x, ",
			pconf->basic.config_check);
	str_info_len += sprintf(str_info + str_info_len, "cus_pinmux:%d, ",
			pconf->custom_pinmux);
	sprintf(str_info + str_info_len, "afr_cus:0x%x", pconf->fr_auto_cus);

	LCDPR("[%d]: load ukey config: %s, %s, %dbit, %dx%d, %s\n",
	      pdrv->index, pconf->basic.model_name,
	      lcd_type_type_to_str(pconf->basic.lcd_type),
	      ptiming->lcd_bits, ptiming->h_active, ptiming->v_active,
	      str_info);

	/* interface: 20byte */
	switch (pconf->basic.lcd_type) {
	case LCD_LVDS:
		pctrl->lvds_cfg.lvds_repack = *(p + LCD_UKEY_IF_ATTR_0) |
			((*(p + LCD_UKEY_IF_ATTR_0 + 1)) << 8);
		pctrl->lvds_cfg.dual_port = *(p + LCD_UKEY_IF_ATTR_1) |
			((*(p + LCD_UKEY_IF_ATTR_1 + 1)) << 8);
		pctrl->lvds_cfg.pn_swap = *(p + LCD_UKEY_IF_ATTR_2) |
			((*(p + LCD_UKEY_IF_ATTR_2 + 1)) << 8);
		pctrl->lvds_cfg.port_swap = *(p + LCD_UKEY_IF_ATTR_3) |
			((*(p + LCD_UKEY_IF_ATTR_3 + 1)) << 8);
		pctrl->lvds_cfg.phy_vswing = *(p + LCD_UKEY_IF_ATTR_4) |
			((*(p + LCD_UKEY_IF_ATTR_4 + 1)) << 8);
		pctrl->lvds_cfg.phy_preem = *(p + LCD_UKEY_IF_ATTR_5) |
			((*(p + LCD_UKEY_IF_ATTR_5 + 1)) << 8);
		pctrl->lvds_cfg.lane_reverse = *(p + LCD_UKEY_IF_ATTR_8) |
			((*(p + LCD_UKEY_IF_ATTR_8 + 1)) << 8);

		phy_cfg->vswing_level = pctrl->lvds_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->lvds_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->lvds_cfg.phy_preem;
		break;
	case LCD_VBYONE:
		pctrl->vbyone_cfg.lane_count = *(p + LCD_UKEY_IF_ATTR_0) |
			((*(p + LCD_UKEY_IF_ATTR_0 + 1)) << 8);
		pctrl->vbyone_cfg.region_num = *(p + LCD_UKEY_IF_ATTR_1) |
			((*(p + LCD_UKEY_IF_ATTR_1 + 1)) << 8);
		pctrl->vbyone_cfg.byte_mode = *(p + LCD_UKEY_IF_ATTR_2) |
			((*(p + LCD_UKEY_IF_ATTR_2 + 1)) << 8);
		pctrl->vbyone_cfg.color_fmt = *(p + LCD_UKEY_IF_ATTR_3) |
			((*(p + LCD_UKEY_IF_ATTR_3 + 1)) << 8);
		pctrl->vbyone_cfg.phy_vswing = *(p + LCD_UKEY_IF_ATTR_4) |
			((*(p + LCD_UKEY_IF_ATTR_4 + 1)) << 8);
		pctrl->vbyone_cfg.phy_preem = *(p + LCD_UKEY_IF_ATTR_5) |
			((*(p + LCD_UKEY_IF_ATTR_5 + 1)) << 8);
		pctrl->vbyone_cfg.hw_filter_time =
			*(p + LCD_UKEY_IF_ATTR_8) |
			((*(p + LCD_UKEY_IF_ATTR_8 + 1)) << 8);
		pctrl->vbyone_cfg.hw_filter_cnt =
			*(p + LCD_UKEY_IF_ATTR_9) |
			((*(p + LCD_UKEY_IF_ATTR_9 + 1)) << 8);
		pctrl->vbyone_cfg.ctrl_flag = 0;
		pctrl->vbyone_cfg.power_on_reset_delay = VX1_PWR_ON_RESET_DLY_DFT;
		pctrl->vbyone_cfg.hpd_data_delay = VX1_HPD_DATA_DELAY_DFT;
		pctrl->vbyone_cfg.cdr_training_hold = VX1_CDR_TRAINING_HOLD_DFT;
		pctrl->vbyone_cfg.slice = pdrv->config.timing.ppc ? pdrv->config.timing.ppc : 1;

		phy_cfg->vswing_level = pctrl->vbyone_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->vbyone_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->vbyone_cfg.phy_preem;
		break;
	case LCD_MLVDS:
		pctrl->mlvds_cfg.channel_num = *(p + LCD_UKEY_IF_ATTR_0) |
			((*(p + LCD_UKEY_IF_ATTR_0 + 1)) << 8);
		pctrl->mlvds_cfg.channel_sel0 = *(p + LCD_UKEY_IF_ATTR_1) |
			((*(p + LCD_UKEY_IF_ATTR_1 + 1)) << 8) |
			(*(p + LCD_UKEY_IF_ATTR_2) << 16) |
			((*(p + LCD_UKEY_IF_ATTR_2 + 1)) << 24);
		pctrl->mlvds_cfg.channel_sel1 = *(p + LCD_UKEY_IF_ATTR_3) |
			((*(p + LCD_UKEY_IF_ATTR_3 + 1)) << 8) |
			(*(p + LCD_UKEY_IF_ATTR_4) << 16) |
			((*(p + LCD_UKEY_IF_ATTR_4 + 1)) << 24);
		pctrl->mlvds_cfg.clk_phase = *(p + LCD_UKEY_IF_ATTR_5) |
			((*(p + LCD_UKEY_IF_ATTR_5 + 1)) << 8);
		pctrl->mlvds_cfg.pn_swap = *(p + LCD_UKEY_IF_ATTR_6) |
			((*(p + LCD_UKEY_IF_ATTR_6 + 1)) << 8);
		pctrl->mlvds_cfg.bit_swap = *(p + LCD_UKEY_IF_ATTR_7) |
			((*(p + LCD_UKEY_IF_ATTR_7 + 1)) << 8);
		pctrl->mlvds_cfg.phy_vswing = *(p + LCD_UKEY_IF_ATTR_8) |
			((*(p + LCD_UKEY_IF_ATTR_8 + 1)) << 8);
		pctrl->mlvds_cfg.phy_preem = *(p + LCD_UKEY_IF_ATTR_9) |
			((*(p + LCD_UKEY_IF_ATTR_9 + 1)) << 8);

		phy_cfg->vswing_level = pctrl->mlvds_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->mlvds_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->mlvds_cfg.phy_preem;
		break;
	case LCD_P2P:
		pctrl->p2p_cfg.p2p_type = *(p + LCD_UKEY_IF_ATTR_0) |
			((*(p + LCD_UKEY_IF_ATTR_0 + 1)) << 8);
		pctrl->p2p_cfg.lane_num = *(p + LCD_UKEY_IF_ATTR_1) |
			((*(p + LCD_UKEY_IF_ATTR_1 + 1)) << 8);
		pctrl->p2p_cfg.channel_sel0 = *(p + LCD_UKEY_IF_ATTR_2) |
			((*(p + LCD_UKEY_IF_ATTR_2 + 1)) << 8) |
			(*(p + LCD_UKEY_IF_ATTR_3) << 16) |
			((*(p + LCD_UKEY_IF_ATTR_3 + 1)) << 24);
		pctrl->p2p_cfg.channel_sel1 = *(p + LCD_UKEY_IF_ATTR_4) |
			((*(p + LCD_UKEY_IF_ATTR_4 + 1)) << 8) |
			(*(p + LCD_UKEY_IF_ATTR_5) << 16) |
			((*(p + LCD_UKEY_IF_ATTR_5 + 1)) << 24);
		pctrl->p2p_cfg.pn_swap = *(p + LCD_UKEY_IF_ATTR_6) |
			((*(p + LCD_UKEY_IF_ATTR_6 + 1)) << 8);
		pctrl->p2p_cfg.bit_swap = *(p + LCD_UKEY_IF_ATTR_7) |
			((*(p + LCD_UKEY_IF_ATTR_7 + 1)) << 8);
		pctrl->p2p_cfg.phy_vswing = *(p + LCD_UKEY_IF_ATTR_8) |
			((*(p + LCD_UKEY_IF_ATTR_8 + 1)) << 8);
		pctrl->p2p_cfg.phy_preem = *(p + LCD_UKEY_IF_ATTR_9) |
			((*(p + LCD_UKEY_IF_ATTR_9 + 1)) << 8);

		phy_cfg->vswing_level = pctrl->p2p_cfg.phy_vswing & 0xf;
		phy_cfg->ext_pullup = (pctrl->p2p_cfg.phy_vswing >> 4) & 0x3;
		phy_cfg->preem_level = pctrl->p2p_cfg.phy_preem;
		break;
	default:
		LCDERR("[%d]: unsupport lcd_type: %d\n",
		       pdrv->index, pconf->basic.lcd_type);
		break;
	}
	phy = lcd_phy_alloc(pdrv);// first parse, it is only be phy_cfg->phys[0];
	if (!phy) {
		ret = -1;
		LCDERR("%s phy alloc fail\n", __func__);
		goto load_from_unifykey_exit;
	}
	memset(phy, 0, sizeof(*phy));
	phy_cfg->act_phy = phy_cfg->phys[0];
	lcd_phy_param_preset(pdrv);
	lcd_lane_map_preset(pdrv);
	phy->ss.freq = 255;
	phy->ss.level = 255;
	phy->ss.mode = 255;

	lcd_vlock_param_load_from_unifykey(pdrv, para);

	/* step 3: check power sequence */
	ret = lcd_power_load_from_unifykey(pdrv, para, key_len, len);
	if (ret < 0) {
		LCDERR("%s power parse fail\n", __func__);
		goto load_from_unifykey_exit;
	}

	p = para + lcd_header->block_cur_size;
	switch (lcd_header->version) {
	case 2:
		lcd_config_load_from_unifykey_v2(pdrv, p, key_len, lcd_header->block_cur_size);
		break;
	case 3:
		lcd_config_load_from_unifykey_v3(pdrv, p, key_len, lcd_header->block_cur_size);
		break;
	default:
		break;
	}

	//fix ss in detail timing and phy_attr if not config
	lcd_ss_config_fix(pdrv);

	lcd_optical_load_from_unifykey(pdrv);

#ifdef CONFIG_AMLOGIC_BACKLIGHT
	lcd_resource_add(pdrv, LCD_RES_BACKLIGHT, 0);
	aml_bl_index_add(pdrv->index, 0);
#endif

load_from_unifykey_exit:
	kfree(para);

	return ret;
}

static int lcd_config_load_init(struct aml_lcd_drv_s *pdrv)
{
	if (pdrv->config.basic.config_check & 0x2)
		pdrv->config_check_en = pdrv->config.basic.config_check & 0x1;
	else
		pdrv->config_check_en = pdrv->config_check_glb;

	switch (pdrv->config.fr_auto_cus) {
	case 0: //follow global fr_auto_policy
		pdrv->config.fr_auto_flag = pdrv->fr_auto_policy;
		break;
	case 0xff: //disable fr_auto
		pdrv->config.fr_auto_flag = 0xff;
		break;
	default: //custom fr_auto
		pdrv->config.fr_auto_flag = pdrv->config.fr_auto_cus;
		break;
	}

	if (pdrv->index)
		pdrv->config.timing.ppc = 1;

	if (pdrv->status & LCD_STATUS_ENCL_ON)
		lcd_clk_gate_switch(pdrv, 1);

	return 0;
}

int lcd_get_config(struct aml_lcd_drv_s *pdrv)
{
	char key_str[10];
	int load_id = 0;
	int ret;

	memset(key_str, 0, 10);
	if (pdrv->index == 0)
		sprintf(key_str, "lcd");
	else
		sprintf(key_str, "lcd%d", pdrv->index);

	if (pdrv->key_valid) {
		ret = lcd_unifykey_check(key_str);
		if (ret < 0) {
			load_id = 0;
			LCDERR("[%d]: %s: can't find key %s\n",
			       pdrv->index, __func__, key_str);
		} else {
			load_id = 1;
		}
	}
	pdrv->config_load = load_id;
	if (load_id)
		ret = lcd_config_load_from_unifykey(pdrv, key_str);
	else
		ret = lcd_config_load_from_dts(pdrv);
	if (ret)
		return -1;

	lcd_lane_map_update(pdrv);

	lcd_config_load_init(pdrv);
	lcd_config_load_print(pdrv);

	lcd_tcon_probe(pdrv);

	return 0;
}

void lcd_optical_vinfo_update(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_config_s *pconf;
	struct master_display_info_s *disp_vinfo;

	pconf = &pdrv->config;
	disp_vinfo = &pdrv->vinfo.master_display_info;
	disp_vinfo->present_flag = pconf->optical.hdr_support;
	disp_vinfo->features = pconf->optical.features;
	disp_vinfo->primaries[0][0] = pconf->optical.primaries_g_x;
	disp_vinfo->primaries[0][1] = pconf->optical.primaries_g_y;
	disp_vinfo->primaries[1][0] = pconf->optical.primaries_b_x;
	disp_vinfo->primaries[1][1] = pconf->optical.primaries_b_y;
	disp_vinfo->primaries[2][0] = pconf->optical.primaries_r_x;
	disp_vinfo->primaries[2][1] = pconf->optical.primaries_r_y;
	disp_vinfo->white_point[0] = pconf->optical.white_point_x;
	disp_vinfo->white_point[1] = pconf->optical.white_point_y;
	disp_vinfo->luminance[0] = pconf->optical.luma_max;
	disp_vinfo->luminance[1] = pconf->optical.luma_min;

	pdrv->vinfo.hdr_info.lumi_max = pconf->optical.luma_max;
	pdrv->vinfo.hdr_info.lumi_min = pconf->optical.luma_min;
	pdrv->vinfo.hdr_info.lumi_avg = pconf->optical.luma_avg;
	pdrv->vinfo.hdr_info.lumi_peak = pconf->optical.luma_peak;
	pdrv->vinfo.hdr_info.ldim_support = pconf->optical.ldim_support;
}

static unsigned int vbyone_lane_num[] = {
	1,
	2,
	4,
	8,
	8,
};

#define VBYONE_BIT_RATE_MAX		4100000000ULL //Hz
#define VBYONE_BIT_RATE_MIN		600000000
void lcd_vbyone_bit_rate_config(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_config_s *pconf = &pdrv->config;
	unsigned int byte_mode, lane_count, minlane, phy_div;
	unsigned long long bit_rate, band_width;
	unsigned int temp, i;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s\n", pdrv->index, __func__);

	//auto calculate bandwidth, clock
	lane_count = pconf->control.vbyone_cfg.lane_count;
	byte_mode = pconf->control.vbyone_cfg.byte_mode;
	/* byte_mode * byte2bit * 8/10_encoding * pclk =
	 *   byte_mode * 8 * 10 / 8 * pclk
	 */
	band_width = pconf->timing.act_timing.pixel_clk; /* Hz */
	band_width = byte_mode * 10 * band_width;

	temp = VBYONE_BIT_RATE_MAX;
	temp = lcd_do_div((band_width + temp - 1), temp);
	for (i = 0; i < 4; i++) {
		if (temp <= vbyone_lane_num[i])
			break;
	}
	minlane = vbyone_lane_num[i];
	if (lane_count < minlane) {
		LCDERR("[%d]: vbyone lane_num(%d) is less than min(%d), change to min lane_num\n",
			pdrv->index, lane_count, minlane);
		lane_count = minlane;
		pconf->control.vbyone_cfg.lane_count = lane_count;
	}

	bit_rate = lcd_do_div(band_width, lane_count);
	phy_div = lane_count / lane_count;
	if (phy_div == 8) {
		phy_div /= 2;
		bit_rate = lcd_do_div(bit_rate, 2);
	}
	if (bit_rate > (VBYONE_BIT_RATE_MAX)) {
		LCDERR("[%d]: vbyone bit rate(%lldHz) is out of max(%lldHz)\n",
			pdrv->index, bit_rate, VBYONE_BIT_RATE_MAX);
	}
	if (bit_rate < (VBYONE_BIT_RATE_MIN)) {
		LCDERR("[%d]: vbyone bit rate(%lldHz) is out of min(%dHz)\n",
			pdrv->index, bit_rate, VBYONE_BIT_RATE_MIN);
	}

	pconf->control.vbyone_cfg.phy_div = phy_div;
	pconf->timing.bit_rate = bit_rate;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("[%d]: lane_count=%u, bit_rate = %lluHz, pclk=%uhz\n",
		      pdrv->index, lane_count, bit_rate, pconf->timing.act_timing.pixel_clk);
	}
}

void lcd_mlvds_bit_rate_config(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_config_s *pconf = &pdrv->config;
	unsigned long long bit_rate, band_width;
	unsigned int lcd_bits, channel_num;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s\n", pdrv->index, __func__);

	lcd_bits = pconf->timing.act_timing.lcd_bits;
	channel_num = pconf->control.mlvds_cfg.channel_num;
	band_width = pconf->timing.act_timing.pixel_clk;
	band_width = lcd_bits * band_width;
	bit_rate = lcd_do_div(band_width, channel_num);
	pconf->timing.bit_rate = bit_rate;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("[%d]: channel_num=%u, bit_rate=%lluHz, pclk=%uhz\n",
		      pdrv->index, channel_num,
		      bit_rate, pconf->timing.act_timing.pixel_clk);
	}
}

void lcd_p2p_bit_rate_config(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_config_s *pconf = &pdrv->config;
	unsigned int p2p_type, lcd_bits, lane_num, clk_mode;
	unsigned long long bit_rate, band_width;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL)
		LCDPR("[%d]: %s\n", pdrv->index, __func__);

	lcd_bits = pconf->timing.act_timing.lcd_bits;
	lane_num = pconf->control.p2p_cfg.lane_num;
	band_width = pconf->timing.act_timing.pixel_clk;
	p2p_type = pconf->control.p2p_cfg.p2p_type & 0x1f;
	clk_mode = pconf->timing.clk_mode;
	switch (p2p_type) {
	case P2P_CEDS:
	case P2P_EPI: /*24to28*/
		if (clk_mode == LCD_CLK_MODE_DEPENDENCE)
			band_width = band_width *  lcd_bits;
		else //independence & dependence_adapt
			band_width = band_width * (lcd_bits + 4);
		break;
	case P2P_CHPI: /* 8/10 coding */
		band_width = lcd_do_div((band_width * lcd_bits * 10), 8);
		break;
	case P2P_CSPI:
	case P2P_ISP:
	case P2P_CMPI: /*24to27*/
		if (clk_mode == LCD_CLK_MODE_DEPENDENCE) {
			band_width = band_width * lcd_bits;
		} else { //independence & dependence_adapt
			/* 8/9 coding */
			band_width = lcd_do_div((band_width * lcd_bits * 9), 8);
		}
		break;
	case P2P_USIT: /*9to10*/
		if (clk_mode == LCD_CLK_MODE_DEPENDENCE)
			band_width = band_width * lcd_bits;
		else //independence & dependence_adapt
			band_width = lcd_do_div((band_width * lcd_bits * 10), 9);
		break;
	default:
		band_width = band_width * lcd_bits;
		break;
	}
	bit_rate = lcd_do_div(band_width, lane_num);
	pconf->timing.bit_rate = bit_rate;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("[%d]: lane_num=%u, bit_rate=%lluHz, pclk=%uhz\n",
		      pdrv->index, lane_num,
		      bit_rate, pconf->timing.act_timing.pixel_clk);
	}
}

void lcd_mipi_dsi_bit_rate_config(struct aml_lcd_drv_s *pdrv)
{
	//todo
}

void lcd_edp_bit_rate_config(struct aml_lcd_drv_s *pdrv)
{
	//todo
}

struct lcd_detail_timing_s *lcd_timing_alloc(struct aml_lcd_drv_s *pdrv)
{
	int n;
	struct lcd_detail_timing_s *dt;

	if (!pdrv || pdrv->config.timing.num_timings >= LCD_MAX_NUM_TIMINGS)
		return NULL;

	n = pdrv->config.timing.num_timings;
	if (n < LCD_MAX_NUM_TIMINGS) {
		dt = kmalloc(sizeof(*dt), GFP_KERNEL);
		if (!dt)
			return NULL;
		pdrv->config.timing.timings[n] = dt;
		pdrv->config.timing.num_timings++;
		return dt;
	}

	return NULL;
}

void lcd_timing_free_last(struct aml_lcd_drv_s *pdrv)
{
	if (!pdrv || pdrv->config.timing.num_timings <= 0)
		return;

	kfree(pdrv->config.timing.timings[pdrv->config.timing.num_timings - 1]);
	pdrv->config.timing.timings[pdrv->config.timing.num_timings - 1] = NULL;
	pdrv->config.timing.num_timings--;
	if (pdrv->config.timing.num_timings == 0) {
		pdrv->config.timing.dft_timing = NULL;
		pdrv->config.timing.base_timing = NULL;
	}
}

struct phy_attr_s *lcd_phy_alloc(struct aml_lcd_drv_s *pdrv)
{
	int n;
	struct phy_attr_s *phy;

	if (!pdrv || pdrv->config.phy_cfg.group_num >= MAX_NUM_PHY_CFGS)
		return NULL;

	n = pdrv->config.phy_cfg.group_num;
	if (n < MAX_NUM_PHY_CFGS) {
		phy = kmalloc(sizeof(*phy), GFP_KERNEL);
		if (!phy)
			return NULL;
		pdrv->config.phy_cfg.phys[n] = phy;
		pdrv->config.phy_cfg.group_num++;
		return phy;
	}

	return NULL;
}

void lcd_phy_free_last(struct aml_lcd_drv_s *pdrv)
{
	if (!pdrv || pdrv->config.phy_cfg.group_num <= 0)
		return;

	kfree(pdrv->config.phy_cfg.phys[pdrv->config.phy_cfg.group_num - 1]);
	pdrv->config.phy_cfg.phys[pdrv->config.phy_cfg.group_num - 1] = NULL;
	pdrv->config.phy_cfg.group_num--;
	if (pdrv->config.phy_cfg.group_num == 0)
		pdrv->config.phy_cfg.act_phy = NULL;
}

static void lcd_fr_range_update(struct lcd_detail_timing_s *ptiming)
{
	unsigned int htotal, vmin, vmax, hfreq;
	unsigned long long temp;
	int i = 1;

	ptiming->h_period_min = ptiming->h_period_min ? ptiming->h_period_min : ptiming->h_period;
	ptiming->h_period_max = ptiming->h_period_max ? ptiming->h_period_max : ptiming->h_period;
	ptiming->v_period_min = ptiming->v_period_min ? ptiming->v_period_min : ptiming->v_period;
	ptiming->v_period_max = ptiming->v_period_max ? ptiming->v_period_max : ptiming->v_period;
	temp = ptiming->pixel_clk;
	temp *= 10;
	htotal = ptiming->h_period;
	vmin = ptiming->v_period_min;
	vmax = ptiming->v_period_max;
	hfreq = lcd_do_div(temp, htotal);
	if (vmin > 0)
		ptiming->vfreq_vrr_max = ((hfreq / vmin) + 5) / 10;
	if (vmax > 0)
		ptiming->vfreq_vrr_min = ((hfreq / vmax) + 5) / 10;
	if (ptiming->frame_rate_max == 0) {
		ptiming->frame_rate_max = ptiming->vfreq_vrr_max;
		if (ptiming->frame_rate_max == 0)
			ptiming->frame_rate_max = ptiming->frame_rate;
	}
	if (ptiming->frame_rate_min == 0) {
		ptiming->frame_rate_min = ptiming->vfreq_vrr_min;
		if (ptiming->frame_rate_min == 0) {
			ptiming->frame_rate_min = ptiming->frame_rate_max / 2 + 10;
		} else {
			while ((ptiming->frame_rate_min * 2 * i) < ptiming->frame_rate_max) {
				ptiming->frame_rate_min = ptiming->frame_rate_min * 2 * i;
				i++;
			}
		}
	}

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("%s: pclk=%u, h_period=%d, range: v_period(%d %d), vrr(%d %d), fr(%d %d)\n",
			__func__, ptiming->pixel_clk, ptiming->h_period,
			ptiming->v_period_min, ptiming->v_period_max,
			ptiming->vfreq_vrr_min, ptiming->vfreq_vrr_max,
			ptiming->frame_rate_min, ptiming->frame_rate_max);
	}
}

void lcd_clk_frame_rate_init(struct lcd_detail_timing_s *ptiming)
{
	unsigned int sync_duration, h_period, v_period;
	unsigned long long temp;

	if (ptiming->pixel_clk == 0) /* default 0 for 60hz */
		ptiming->pixel_clk = 60;

	h_period = ptiming->h_period;
	v_period = ptiming->v_period;
	if (ptiming->pixel_clk < 500) { /* regard as frame_rate */
		sync_duration = ptiming->pixel_clk;
		ptiming->pixel_clk = sync_duration * h_period * v_period;
		ptiming->frame_rate = sync_duration;
		ptiming->sync_duration_num = sync_duration;
		ptiming->sync_duration_den = 1;
		ptiming->frac = 0;
	} else { /* regard as pixel clock */
		temp = ptiming->pixel_clk;
		temp *= 1000;
		sync_duration = lcd_do_div(temp, (v_period * h_period));
		ptiming->frame_rate = sync_duration / 1000;
		ptiming->sync_duration_num = sync_duration;
		ptiming->sync_duration_den = 1000;
		ptiming->frac = 0;
	}

	lcd_fr_range_update(ptiming);
}

void lcd_default_to_basic_timing_init_config(struct aml_lcd_drv_s *pdrv)
{
	pdrv->config.timing.base_timing = pdrv->config.timing.dft_timing;
}

//act_timing as enc_timing
void lcd_enc_timing_init_config(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_config_s *pconf = &pdrv->config;
	struct lcd_detail_timing_s *ptiming;
	unsigned short h_period, v_period, h_active, v_active;
	unsigned short hsync_bp, hsync_width, vsync_bp, vsync_width;
	unsigned short de_hstart, de_vstart;
	unsigned short hs_start, hs_end, vs_start, vs_end;
	unsigned short h_delay;

	if (!pdrv->config.timing.base_timing)
		return;

	switch (pconf->basic.lcd_type) {
	case LCD_RGB:
		h_delay = RGB_DELAY;
		break;
	default:
		h_delay = 0;
		break;
	}

	ptiming = &pdrv->config.timing.act_timing;
	memcpy(ptiming, pdrv->config.timing.base_timing, sizeof(struct lcd_detail_timing_s));
	if (pconf->timing.ppc == 0)
		pconf->timing.ppc = 1;
	pconf->timing.enc_clk = pconf->timing.act_timing.pixel_clk / pconf->timing.ppc;
	if (pdrv->config.timing.ppc > 1) {
		LCDPR("[%d]: %s: ppc=%d, pixel_clk=%d, enc_clk=%d\n",
		      pdrv->index, __func__, pdrv->config.timing.ppc,
		      pconf->timing.act_timing.pixel_clk, pconf->timing.enc_clk);
	}

	h_period = ptiming->h_period;
	v_period = ptiming->v_period;
	h_active = ptiming->h_active;
	v_active = ptiming->v_active;
	hsync_bp = ptiming->hsync_bp;
	hsync_width = ptiming->hsync_width;
	vsync_bp = ptiming->vsync_bp;
	vsync_width = ptiming->vsync_width;

	de_hstart = hsync_bp + hsync_width;
	de_vstart = vsync_bp + vsync_width;

	pconf->timing.hstart = de_hstart - h_delay;
	pconf->timing.vstart = de_vstart;
	pconf->timing.hend = h_active + pconf->timing.hstart - 1;
	pconf->timing.vend = v_active + pconf->timing.vstart - 1;

	pconf->timing.de_hs_addr = de_hstart;
	pconf->timing.de_he_addr = de_hstart + h_active;
	pconf->timing.de_vs_addr = de_vstart;
	pconf->timing.de_ve_addr = de_vstart + v_active - 1;

	hs_start = (de_hstart + h_period - hsync_bp - hsync_width) % h_period;
	hs_end = (de_hstart + h_period - hsync_bp) % h_period;
	pconf->timing.hs_hs_addr = hs_start;
	pconf->timing.hs_he_addr = hs_end;
	pconf->timing.hs_vs_addr = 0;
	pconf->timing.hs_ve_addr = v_period - 1;

	pconf->timing.vs_hs_addr = (hs_start + h_period) % h_period;
	pconf->timing.vs_he_addr = pconf->timing.vs_hs_addr;
	vs_start = (de_vstart + v_period - vsync_bp - vsync_width) % v_period;
	vs_end = (de_vstart + v_period - vsync_bp) % v_period;
	pconf->timing.vs_vs_addr = vs_start;
	pconf->timing.vs_ve_addr = vs_end;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("[%d]: %s: act_timing: %dx%dp%dhz\n", pdrv->index, __func__,
			ptiming->h_active, ptiming->v_active, ptiming->frame_rate);

		LCDPR("[%d]: %s: hs_hs=%d hs_he=%d hs_vs=%d hs_ve=%d\n"
		      "          vs_hs=%d vs_he=%d vs_vs=%d vs_ve=%d\n",
			pdrv->index, __func__,
			pconf->timing.hs_hs_addr, pconf->timing.hs_he_addr,
			pconf->timing.hs_vs_addr, pconf->timing.hs_ve_addr,
			pconf->timing.vs_hs_addr, pconf->timing.vs_he_addr,
			pconf->timing.vs_vs_addr, pconf->timing.vs_ve_addr);
	}
}

//h_timing change for multi ppc if needed
void lcd_enc_h_timing_change(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_config_s *pconf = &pdrv->config;
	struct lcd_detail_timing_s *ptiming;
	unsigned short h_period, h_active, hsync_bp, hsync_width;
	unsigned short de_hstart, hs_start, hs_end, h_delay = 0;

	switch (pconf->basic.lcd_type) {
	case LCD_RGB:
		h_delay = RGB_DELAY;
		break;
	default:
		break;
	}

	ptiming = &pdrv->config.timing.act_timing;

	h_period = ptiming->h_period;
	h_active = ptiming->h_active;
	hsync_bp = ptiming->hsync_bp;
	hsync_width = ptiming->hsync_width;

	de_hstart = hsync_bp + hsync_width;

	pconf->timing.hstart = de_hstart - h_delay;
	pconf->timing.hend = h_active + pconf->timing.hstart - 1;

	pconf->timing.de_hs_addr = de_hstart;
	pconf->timing.de_he_addr = de_hstart + h_active;

	hs_start = (de_hstart + h_period - hsync_bp - hsync_width) % h_period;
	hs_end = (de_hstart + h_period - hsync_bp) % h_period;
	pconf->timing.hs_hs_addr = hs_start;
	pconf->timing.hs_he_addr = hs_end;

	pconf->timing.vs_hs_addr = (hs_start + h_period) % h_period;
	pconf->timing.vs_he_addr = pconf->timing.vs_hs_addr;

	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		LCDPR("[%d]: %s: addr: hs_hs=%d hs_he=%d vs_hs=%d vs_he=%d\n",
		      pdrv->index, __func__,
		      pconf->timing.hs_hs_addr, pconf->timing.hs_he_addr,
		      pconf->timing.vs_hs_addr, pconf->timing.vs_he_addr);
	}
}

int lcd_fr_is_fixed(struct aml_lcd_drv_s *pdrv)
{
	int ret = 0;

	switch (pdrv->config.timing.act_timing.fr_adjust_type) {
	case 5: /* free run mode, vlock enabled nearby fixed frame rate */
	case 0xff: /* fix fr mode, vlock disabled */
		ret = 1;
		break;
	default:
		ret = 0;
		break;
	}

	return ret;
}

int lcd_fr_is_frac(struct aml_lcd_drv_s *pdrv, unsigned int frame_rate)
{
	int ret = 0;

	switch (frame_rate) {
	case 47:
	case 59:
	case 95:
	case 119:
	case 191:
	case 239:
		ret = 1;
		break;
	default:
		ret = 0;
		break;
	}

	return ret;
}

int lcd_vmode_frac_is_support(struct aml_lcd_drv_s *pdrv, unsigned int frame_rate)
{
	int ret = 0;

	switch (frame_rate) {
	case 48:
	case 60:
	case 96:
	case 120:
	case 192:
	case 240:
		ret = 1;
		break;
	default:
		ret = 0;
		break;
	}

	return ret;
}

static int lcd_timing_fr_update(struct aml_lcd_drv_s *pdrv)
{
	struct lcd_config_s *pconf = &pdrv->config;
	 /* use base value to avoid offset */
	unsigned int pclk = pconf->timing.base_timing->pixel_clk;
	unsigned int h_period = pconf->timing.base_timing->h_period;
	unsigned int v_period = pconf->timing.base_timing->v_period;
	/* use act value as condition */
	unsigned char type = pconf->timing.act_timing.fr_adjust_type;
	unsigned int pclk_min = pconf->timing.act_timing.pclk_min;
	unsigned int pclk_max = pconf->timing.act_timing.pclk_max;
	unsigned int duration_num = pconf->timing.act_timing.sync_duration_num;
	unsigned int duration_den = pconf->timing.act_timing.sync_duration_den;
	unsigned long long temp;
	char str[100];
	int len = 0;

	/* clear clk_change flag */
	pdrv->config.timing.clk_change &= ~(LCD_CLK_FRAC_UPDATE | LCD_CLK_PLL_CHANGE);
	switch (type) {
	case 0: /* pixel clk adjust */
		temp = duration_num;
		temp = temp * h_period * v_period;
		pclk = lcd_do_div(temp, duration_den);
		if (pconf->timing.act_timing.pixel_clk != pclk)
			pconf->timing.clk_change |= LCD_CLK_PLL_CHANGE;
		break;
	case 1: /* htotal adjust */
		temp = pclk;
		temp =  temp * duration_den * 100;
		h_period = v_period * duration_num;
		h_period = lcd_do_div(temp, h_period);
		h_period = (h_period + 99) / 100; /* round off */
		if (pconf->timing.act_timing.h_period != h_period) {
			/* check clk frac update */
			temp = duration_num;
			temp = temp * h_period * v_period;
			pclk = lcd_do_div(temp, duration_den);
		}
		if (pconf->timing.act_timing.pixel_clk != pclk)
			pconf->timing.clk_change |= LCD_CLK_FRAC_UPDATE;
		break;
	case 2: /* vtotal adjust */
		temp = pclk;
		temp = temp * duration_den * 100;
		v_period = h_period * duration_num;
		v_period = lcd_do_div(temp, v_period);
		v_period = (v_period + 99) / 100; /* round off */
		if (pconf->timing.act_timing.v_period != v_period) {
			/* check clk frac update */
			temp = duration_num;
			temp = temp * h_period * v_period;
			pclk = lcd_do_div(temp, duration_den);
		}
		if (pconf->timing.act_timing.pixel_clk != pclk)
			pconf->timing.clk_change |= LCD_CLK_FRAC_UPDATE;
		break;
	case 3: /* free adjust, use min/max range to calculate */
		temp = pclk;
		temp = temp * duration_den * 100;
		v_period = h_period * duration_num;
		v_period = lcd_do_div(temp, v_period);
		v_period = (v_period + 99) / 100; /* round off */
		if (v_period > pconf->timing.act_timing.v_period_max) {
			v_period = pconf->timing.act_timing.v_period_max;
			h_period = v_period * duration_num;
			h_period = lcd_do_div(temp, h_period);
			h_period = (h_period + 99) / 100; /* round off */
			if (h_period > pconf->timing.act_timing.h_period_max) {
				h_period = pconf->timing.act_timing.h_period_max;
				temp = duration_num;
				temp = temp * h_period * v_period;
				pclk = lcd_do_div(temp, duration_den);
				if (pclk > pclk_max) {
					// pclk = pclk_max;
					LCDERR("[%d]:  %s: invalid vmode\n",
						pdrv->index, __func__);
					return -1;
				}
				if (pconf->timing.act_timing.pixel_clk != pclk)
					pconf->timing.clk_change |= LCD_CLK_PLL_CHANGE;
			}
		} else if (v_period < pconf->timing.act_timing.v_period_min) {
			v_period = pconf->timing.act_timing.v_period_min;
			h_period = v_period * duration_num;
			h_period = lcd_do_div(temp, h_period);
			h_period = (h_period + 99) / 100; /* round off */
			if (h_period < pconf->timing.act_timing.h_period_min) {
				h_period = pconf->timing.act_timing.h_period_min;
				temp = duration_num;
				temp = temp * h_period * v_period;
				pclk = lcd_do_div(temp, duration_den);
				if (pclk < pclk_min) {
					// pclk = pclk_min;
					LCDERR("[%d]: %s: invalid vmode\n",
						pdrv->index, __func__);
					return -1;
				}
				if (pconf->timing.act_timing.pixel_clk != pclk)
					pconf->timing.clk_change |= LCD_CLK_PLL_CHANGE;
			}
		}
		/* check clk frac update */
		if ((pconf->timing.clk_change & LCD_CLK_PLL_CHANGE) == 0) {
			temp = duration_num;
			temp = temp * h_period * v_period;
			pclk = lcd_do_div(temp, duration_den);
			if (pconf->timing.act_timing.pixel_clk != pclk)
				pconf->timing.clk_change |= LCD_CLK_FRAC_UPDATE;
		}
		break;
	case 4: /* hdmi mode */
		if (((duration_num / duration_den) == 59) ||
		    ((duration_num / duration_den) == 119)) {
			/* pixel clk adjust */
			temp = duration_num;
			temp = temp * h_period * v_period;
			pclk = lcd_do_div(temp, duration_den);
			if (pconf->timing.act_timing.pixel_clk != pclk)
				pconf->timing.clk_change |= LCD_CLK_PLL_CHANGE;
		} else if ((duration_num / duration_den) == 47) {
			/* htotal adjust */
			temp = pclk;
			h_period = v_period * 50;
			h_period = lcd_do_div(temp, h_period);
			if (pconf->timing.act_timing.h_period != h_period) {
				/* check clk adjust */
				temp = duration_num;
				temp = temp * h_period * v_period;
				pclk = lcd_do_div(temp, duration_den);
			}
			if (pconf->timing.act_timing.pixel_clk != pclk)
				pconf->timing.clk_change |= LCD_CLK_PLL_CHANGE;
		} else if ((duration_num / duration_den) == 95) {
			/* htotal adjust */
			temp = pclk;
			h_period = v_period * 100;
			h_period = lcd_do_div(temp, h_period);
			if (pconf->timing.act_timing.h_period != h_period) {
				/* check clk adjust */
				temp = duration_num;
				temp = temp * h_period * v_period;
				pclk = lcd_do_div(temp, duration_den);
			}
			if (pconf->timing.act_timing.pixel_clk != pclk)
				pconf->timing.clk_change |= LCD_CLK_PLL_CHANGE;
		} else {
			/* htotal adjust */
			temp = pclk;
			temp = temp * duration_den * 100;
			h_period = v_period * duration_num;
			h_period = lcd_do_div(temp, h_period);
			h_period = (h_period + 99) / 100; /* round off */
			if (pconf->timing.act_timing.h_period != h_period) {
				/* check clk frac update */
				temp = duration_num;
				temp = temp * h_period * v_period;
				pclk = lcd_do_div(temp, duration_den);
			}
			if (pconf->timing.act_timing.pixel_clk != pclk)
				pconf->timing.clk_change |= LCD_CLK_FRAC_UPDATE;
		}
		break;
	default:
		LCDERR("[%d]: %s: invalid fr_adjust_type: %d\n",
		       pdrv->index, __func__, type);
		return 0;
	}

	memset(str, 0, 100);
	if (pconf->timing.act_timing.v_period != v_period) {
		len += sprintf(str + len, "v_period %u->%u",
			pconf->timing.act_timing.v_period, v_period);
		/* update v_period */
		pconf->timing.act_timing.v_period = v_period;
	}
	if (pconf->timing.act_timing.h_period != h_period) {
		if (len > 0)
			len += sprintf(str + len, ", ");
		len += sprintf(str + len, "h_period %u->%u",
			pconf->timing.act_timing.h_period, h_period);
		/* update h_period */
		pconf->timing.act_timing.h_period = h_period;
	}
	if (pconf->timing.act_timing.pixel_clk != pclk) {
		if (len > 0)
			len += sprintf(str + len, ", ");
		len += sprintf(str + len, "pclk %uHz->%uHz, clk_change:0x%x",
			       pconf->timing.act_timing.pixel_clk, pclk,
			       pconf->timing.clk_change);
		pconf->timing.act_timing.pixel_clk = pclk;
		pconf->timing.enc_clk = pclk / pconf->timing.ppc;
		if (pdrv->config.timing.ppc > 1) {
			len += sprintf(str + len, ", ppc=%d, enc_clk=%d",
				pdrv->config.timing.ppc, pconf->timing.enc_clk);
		}
		if (pdrv->config.timing.clk_change & LCD_CLK_PLL_CHANGE)
			lcd_clk_generate_parameter(pdrv);
		else if (pdrv->config.timing.clk_change & LCD_CLK_FRAC_UPDATE)
			lcd_clk_frac_generate(pdrv);
	}
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		if (len > 0) {
			LCDPR("[%d]: %s: fr_adj_type: %d, sync_duration: %d/%d, %s\n",
				pdrv->index, __func__, type,
				pconf->timing.act_timing.sync_duration_num,
				pconf->timing.act_timing.sync_duration_den,
				str);
		}
	}

	return 0;
}

void lcd_frame_rate_change(struct aml_lcd_drv_s *pdrv)
{
	struct vinfo_s *info;

	/* update vinfo */
	info = &pdrv->vinfo;
	info->sync_duration_num = pdrv->config.timing.act_timing.sync_duration_num;
	info->sync_duration_den = pdrv->config.timing.act_timing.sync_duration_den;
	info->frac = pdrv->config.timing.act_timing.frac;
	info->std_duration = pdrv->config.timing.act_timing.frame_rate;

	/* update clk & timing config */
	lcd_timing_fr_update(pdrv);
	info->video_clk = pdrv->config.timing.enc_clk;
	info->htotal = pdrv->config.timing.act_timing.h_period;
	info->vtotal = pdrv->config.timing.act_timing.v_period;

	lcd_act_timing_dbg_print(pdrv);
}

void lcd_if_enable_retry(struct aml_lcd_drv_s *pdrv)
{
	pdrv->config.retry_enable_cnt = 0;
	while (pdrv->config.retry_enable_flag) {
		if (pdrv->config.retry_enable_cnt++ >= LCD_ENABLE_RETRY_MAX)
			break;
		LCDPR("[%d]: retry enable...%d\n", pdrv->index, pdrv->config.retry_enable_cnt);
		aml_lcd_notifier_call_chain(LCD_EVENT_POWER_OFF, (void *)pdrv);
		lcd_delay_ms(1000);
		aml_lcd_notifier_call_chain(LCD_EVENT_POWER_ON, (void *)pdrv);
	}
	pdrv->config.retry_enable_cnt = 0;
}

void lcd_vout_notify_mode_change_pre(struct aml_lcd_drv_s *pdrv)
{
	if (pdrv->viu_sel == 1) {
#ifdef CONFIG_AMLOGIC_VOUT_SERVE
		vout_notifier_call_chain(VOUT_EVENT_MODE_CHANGE_PRE, &pdrv->vinfo.mode);
#endif
	} else if (pdrv->viu_sel == 2) {
#ifdef CONFIG_AMLOGIC_VOUT2_SERVE
		vout2_notifier_call_chain(VOUT_EVENT_MODE_CHANGE_PRE, &pdrv->vinfo.mode);
#endif
	} else if (pdrv->viu_sel == 3) {
#ifdef CONFIG_AMLOGIC_VOUT3_SERVE
		vout3_notifier_call_chain(VOUT_EVENT_MODE_CHANGE_PRE, &pdrv->vinfo.mode);
#endif
	}
}

void lcd_vout_notify_mode_change(struct aml_lcd_drv_s *pdrv)
{
	if (pdrv->viu_sel == 1) {
#ifdef CONFIG_AMLOGIC_VOUT_SERVE
		vout_notifier_call_chain(VOUT_EVENT_MODE_CHANGE, &pdrv->vinfo.mode);
#endif
	} else if (pdrv->viu_sel == 2) {
#ifdef CONFIG_AMLOGIC_VOUT2_SERVE
		vout2_notifier_call_chain(VOUT_EVENT_MODE_CHANGE, &pdrv->vinfo.mode);
#endif
	} else if (pdrv->viu_sel == 3) {
#ifdef CONFIG_AMLOGIC_VOUT3_SERVE
		vout3_notifier_call_chain(VOUT_EVENT_MODE_CHANGE, &pdrv->vinfo.mode);
#endif
	}
}

void lcd_vinfo_update(struct aml_lcd_drv_s *pdrv)
{
	struct vinfo_s *vinfo;
	struct lcd_config_s *pconf;

	vinfo = &pdrv->vinfo;
	pconf = &pdrv->config;

	vinfo->width = pconf->timing.act_timing.h_active;
	vinfo->height = pconf->timing.act_timing.v_active;
	vinfo->field_height = pconf->timing.act_timing.v_active;
	vinfo->aspect_ratio_num = pconf->basic.screen_width;
	vinfo->aspect_ratio_den = pconf->basic.screen_height;
	vinfo->screen_real_width = pconf->basic.screen_width;
	vinfo->screen_real_height = pconf->basic.screen_height;
	vinfo->sync_duration_num = pconf->timing.act_timing.sync_duration_num;
	vinfo->sync_duration_den = pconf->timing.act_timing.sync_duration_den;
	vinfo->frac = pconf->timing.act_timing.frac;
	vinfo->std_duration = pconf->timing.act_timing.frame_rate;
	vinfo->vfreq_max = pconf->timing.act_timing.frame_rate_max;
	vinfo->vfreq_min = pconf->timing.act_timing.frame_rate_min;
	vinfo->video_clk = pconf->timing.enc_clk;
	vinfo->htotal = pconf->timing.act_timing.h_period;
	vinfo->vtotal = pconf->timing.act_timing.v_period;
	vinfo->hsw = pconf->timing.act_timing.hsync_width;
	vinfo->hbp = pconf->timing.act_timing.hsync_bp;
	vinfo->hfp = pconf->timing.act_timing.hsync_fp;
	vinfo->vsw = pconf->timing.act_timing.vsync_width;
	vinfo->vbp = pconf->timing.act_timing.vsync_bp;
	vinfo->vfp = pconf->timing.act_timing.vsync_fp;
	vinfo->cur_enc_ppc = pconf->timing.ppc;

	lcd_vout_notify_mode_change(pdrv);
}

static unsigned int lcd_vrr_lfc_switch(void *dev_data, int fps)
{
	struct aml_lcd_drv_s *pdrv;
	unsigned long long temp;
	unsigned int h_period, v_period;

	pdrv = (struct aml_lcd_drv_s *)dev_data;
	if (!pdrv) {
		LCDERR("%s: vrr dev_data is null\n", __func__);
		return 0;
	}
	h_period = pdrv->config.timing.act_timing.h_period;
	v_period = pdrv->config.timing.act_timing.v_period;

	temp = pdrv->config.timing.act_timing.pixel_clk;
	temp *= 100;
	h_period = h_period * fps * 2;
	v_period = lcd_do_div(temp, h_period);
	v_period = (v_period + 99) / 100; /* round off */

	return v_period;
}

static int lcd_vrr_disable_cb(void *dev_data)
{
	struct aml_lcd_drv_s *pdrv;

	pdrv = (struct aml_lcd_drv_s *)dev_data;
	if (!pdrv) {
		LCDERR("%s: vrr dev_data is null\n", __func__);
		return -1;
	}
	lcd_venc_vrr_recovery(pdrv);

	return 0;
}

void lcd_vrr_dev_update(struct aml_lcd_drv_s *pdrv)
{
	if (!pdrv->vrr_dev)
		return;

	if (pdrv->config.timing.act_timing.fr_adjust_type == 2) /* vtotal adj */
		pdrv->vrr_dev->enable = 1;
	else
		pdrv->vrr_dev->enable = 0;

	pdrv->vrr_dev->vline = pdrv->config.timing.act_timing.v_period;
	pdrv->vrr_dev->vline_max = pdrv->config.timing.act_timing.v_period_max;
	pdrv->vrr_dev->vline_min = pdrv->config.timing.act_timing.v_period_min;
	pdrv->vrr_dev->vfreq_max = pdrv->config.timing.act_timing.vfreq_vrr_max;
	pdrv->vrr_dev->vfreq_min = pdrv->config.timing.act_timing.vfreq_vrr_min;
}

void lcd_vrr_dev_register(struct aml_lcd_drv_s *pdrv)
{
	if (pdrv->vrr_dev)
		return;

	pdrv->vrr_dev = kzalloc(sizeof(*pdrv->vrr_dev), GFP_KERNEL);
	if (!pdrv->vrr_dev)
		return;

	sprintf(pdrv->vrr_dev->name, "lcd%d_dev", pdrv->index);
	pdrv->vrr_dev->output_src = VRR_OUTPUT_ENCL;
	pdrv->vrr_dev->lfc_switch = lcd_vrr_lfc_switch;
	pdrv->vrr_dev->disable_cb = lcd_vrr_disable_cb;
	pdrv->vrr_dev->v_active = pdrv->config.timing.act_timing.v_active;
	pdrv->vrr_dev->dev_data = (void *)pdrv;
	lcd_vrr_dev_update(pdrv);
	aml_vrr_register_device(pdrv->vrr_dev, pdrv->index);
}

void lcd_vrr_dev_unregister(struct aml_lcd_drv_s *pdrv)
{
	if (!pdrv->vrr_dev)
		return;

	aml_vrr_unregister_device(pdrv->index);
	kfree(pdrv->vrr_dev);
	pdrv->vrr_dev = NULL;
}

unsigned int str_add_vmode(char *buf, unsigned char newline,
		unsigned short width, unsigned short height, unsigned short fr)
{
	unsigned int i;
	unsigned char use_short = 0;
	struct V_name_s { unsigned short h, v; unsigned short fr[5]; } V_name_table[] = {
		{3840, 2160, { 60, 59, 50, 48, 47}},
		{3840, 2160, { 30, 25, 24,  0,  0}},
		{1920, 1080, {120, 60, 59, 50, 48}},
		{1920, 1080, { 47, 30, 25, 24,  0}},
		{1366,  768, { 60, 59, 50, 48, 47}},
		{1024,  600, { 60, 59, 50, 48, 47}},
		{1280,  720, { 60, 50,  0,  0,  0}},
	};

	for (i = 0; i < 5 * ARRAY_SIZE(V_name_table); i++) {
		if (V_name_table[i / 5].h == width && V_name_table[i / 5].v == height) {
			if (V_name_table[i / 5].fr[i % 5] == 0)
				continue;
			if (fr == V_name_table[i / 5].fr[i % 5] || fr == 0) {
				use_short = 1;
				break;
			}
		}
	}

	i = 0;
	i += use_short ? sprintf(buf + i,    "%up",        height) :
			 sprintf(buf + i, "%ux%up", width, height);
	if (fr)
		i += sprintf(buf + i, "%huhz", fr);
	if (newline)
		i += sprintf(buf + i, "\n");

	return i;
}
