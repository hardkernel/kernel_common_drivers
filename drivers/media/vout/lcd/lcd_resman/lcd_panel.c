// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/init.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/clk.h>
#include <linux/of_fdt.h>
#include <linux/of_device.h>

#include <linux/compat.h>
#include <linux/workqueue.h>
#include <linux/mm.h>
#include <linux/sched/clock.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_address.h>
#endif
#include <linux/amlogic/pm.h>
#include <linux/of_reserved_mem.h>
#include <linux/amlogic/aml_free_reserved.h>

#include <linux/page-flags.h>
#include <linux/mm.h>
#include <linux/amlogic/tee.h>
#include <linux/amlogic/media/vout/lcd/lcd_vout.h>
#include <linux/amlogic/media/vout/lcd/json_parse.h>

#include <linux/amlogic/media/vout/lcd/lcd_resman.h>
#include <linux/amlogic/media/vout/lcd/lcd_math.h>

static struct json_parse_s panel_jsp[3];

static unsigned char glcd_panel_file_type[3] = {0, 0, 0};

#define PANEL_PARAM_KEY_SIZE 64
#define PANEL_PARAM_HEAD_SIZE PANEL_PARAM_KEY_SIZE

struct panel_param_key_s {
	unsigned int size;
	unsigned int mem_pos;
	char name[PANEL_PARAM_KEY_SIZE - 8];
};

struct panel_param_head_s {
	unsigned int _crc32;
	unsigned int size;
	unsigned short key_cnt;
	unsigned short ukey_exist;
	unsigned char rsvd[64 - 12];
};

struct panel_param_mem_s {
	struct panel_param_head_s *head;
	unsigned char *mem;
	unsigned char *key_mem;
	struct panel_param_key_s *keys;
};

static struct panel_param_mem_s panel_param_mem = {NULL, NULL, NULL, NULL};

int is_ukey_in_param_mem(void)
{
	return (panel_param_mem.head && panel_param_mem.head->ukey_exist) ? 1 : 0;
}

unsigned char *panel_param_mem_get(const char *name, u32 *len)
{
	unsigned int i = 0;
	struct panel_param_key_s *key;

	if (!panel_param_mem.mem || !panel_param_mem.head->key_cnt)
		return NULL;

	for (i = 0; i < panel_param_mem.head->key_cnt; i++) {
		key = &panel_param_mem.keys[i];
		if (strncmp(key->name, name, sizeof(key->name)) == 0) {
			*len = key->size;
			return panel_param_mem.key_mem + key->mem_pos;
		}
	}

	return NULL;
}

unsigned char get_lcd_panel_file_type(int index)
{
	return index < 3 ? glcd_panel_file_type[index] : 0;
}

void set_lcd_panel_file_type(int index, unsigned char type)
{
	glcd_panel_file_type[index] = type;
}

struct json_parse_s *get_panel_jsp(int index)
{
	return &panel_jsp[index];
}

static int lcd_panel_json_init_try_mem(struct json_parse_s *jsp, unsigned char *mem)
{
#define JSON_PANEL_HANDLE_HEAD_SIZE (32)
		struct json_panel_handle_head_s {
			unsigned int size;
			unsigned int json_cnt;
			unsigned int js_len;
			unsigned int json_start;
			unsigned int js_start;
			unsigned char rsvd[JSON_PANEL_HANDLE_HEAD_SIZE - 20];
		} *head; //parse memory handled from uboot

	if (!mem)
		return -1;

	head = (struct json_panel_handle_head_s *)mem;

	jsp->json_cnt =  head->json_cnt;
	jsp->js_len   = head->js_len;
	jsp->js       = kzalloc(jsp->js_len, GFP_KERNEL);
	jsp->root     = kcalloc(jsp->json_cnt, sizeof(*jsp->root), GFP_KERNEL);
	jsp->json_max = jsp->json_cnt;
	jsp->js_max = jsp->js_len;
	if (!jsp->js || !jsp->root) {
		json_deinit(jsp);
		return -1;
	}
	memcpy(jsp->root, mem + head->json_start, jsp->json_cnt * sizeof(*jsp->root));
	memcpy(jsp->js, mem + head->js_start, jsp->js_len);

	return 0;
}

static int lcd_panel_json_init_try_file(struct json_parse_s *jsp, unsigned char *panel_file)
{
	if (json_init(jsp, JSON_STR_MAX, JSON_NODE_MAX) < 0)
		return -1;
	if (!json_parse(jsp, (char *)panel_file, 32 * 1024)) {
		json_deinit(jsp);
		return -1;
	}
	return 0;
}

int lcd_panel_file_pre_proc(void)
{
	char name[32];
	unsigned char *mem;
	unsigned int size, _crc32;
	struct json_parse_s *jsp;
	int ret = -1, i = 0;
	struct panel_param_head_s *head;
	struct panel_param_key_s *key;

	mem = lcd_transmit_mem_get("panel_config", &size);
	if (!mem) {
		LRMPR("panel_config get fail\n");
		return -1;
	}

	head = (struct panel_param_head_s *)mem;
	_crc32 = cal_crc32(0, mem + 4, head->size - 4);
	if (_crc32 != head->_crc32) {
		LRMPR("panel_config crc check fail:cal:0x%x-ori:0x%x\n", _crc32, head->_crc32);
		return -1;
	}

	panel_param_mem.mem = mem;
	panel_param_mem.head = (struct panel_param_head_s *)mem;
	panel_param_mem.keys = (struct panel_param_key_s *)(panel_param_mem.mem +
				PANEL_PARAM_HEAD_SIZE);
	head = panel_param_mem.head;
	size = head->key_cnt * PANEL_PARAM_KEY_SIZE + PANEL_PARAM_HEAD_SIZE;
	panel_param_mem.key_mem = panel_param_mem.mem + size;

	LRMPR("%s panel_param_mem: size:0x%x, key_cnt:%d, ukey_exist:%d\n",
		__func__, head->size, head->key_cnt, head->ukey_exist);
	if (lcd_debug_print_flag & LCD_DBG_PR_NORMAL) {
		for (i = 0; i < head->key_cnt; i++) {
			key = &panel_param_mem.keys[i];
			LRMPR("[%d]: size;0x%x, mem_ofst:0x%x, name:%s\n",
				i, key->size, key->mem_pos, key->name);
		}
	}

	for (i = 0; i < 3; i++) {
		jsp = get_panel_jsp(i);
		json_deinit(jsp);
		jsp->status = JSON_STATUS_NO_FILE;

		snprintf(name, 31, "panel%d_jsp", i);
		mem = panel_param_mem_get(name, &size);
		if (mem) {
			ret = lcd_panel_json_init_try_mem(jsp, mem);
			if (ret) {
				jsp->status = JSON_STATUS_ERROR;
			} else {
				jsp->status = JSON_STATUS_OK;
				set_lcd_panel_file_type(i, PANEL_FILE_JSON);
			}
			LRMPR("%s panel%d_jsp init %s\n", __func__, i, ret ? "fail" : "ok");
			continue;
		}

		snprintf(name, 31, "panel%d_json", i);
		mem = panel_param_mem_get(name, &size);
		if (mem) {
			ret = lcd_panel_json_init_try_file(jsp, mem);
			if (ret) {
				jsp->status = JSON_STATUS_ERROR;
			} else {
				jsp->status = JSON_STATUS_OK;
				set_lcd_panel_file_type(i, PANEL_FILE_JSON);
			}
			LRMPR("%s panel%d_json init %s\n", __func__, i, ret ? "fail" : "ok");
			continue;
		}

		//ini
	}

	return 0;
}

