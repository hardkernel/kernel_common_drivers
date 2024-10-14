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
#include <linux/amlogic/media/vout/lcd/lcd_resman.h>
#include <linux/amlogic/media/vout/lcd/lcd_math.h>

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

int lcd_panel_file_pre_proc(void)
{
	unsigned char *mem;
	unsigned int i = 0, size, _crc32;
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

	return 0;
}

