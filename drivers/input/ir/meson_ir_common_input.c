// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/jiffies.h>
#include <linux/input.h>
#include "meson_ir_main.h"

struct meson_ir_common_input_data {
	unsigned long last_jiffies;

	struct input_dev *common_input;
	spinlock_t input_lock; /*mutex for common input */
};

static struct meson_ir_common_input_data common_input_data;

void meson_ir_common_input_report(unsigned int code, int value)
{
	spin_lock(&common_input_data.input_lock);

	if (jiffies_to_msecs(jiffies - common_input_data.last_jiffies) < 50) {
		spin_unlock(&common_input_data.input_lock);
		return;
	}

	input_report_key(common_input_data.common_input, code, value);
	input_sync(common_input_data.common_input);
	common_input_data.last_jiffies = jiffies;
	spin_unlock(&common_input_data.input_lock);
}

int meson_ir_common_input_init(void)
{
	int i, ret;
	struct input_dev *input_device;

	spin_lock_init(&common_input_data.input_lock);

	input_device = input_allocate_device();
	if (!input_device)
		return -ENOMEM;

	input_device->name = "ir_keypad_common";
	input_device->phys = "keypad/input0";
	input_device->rep[REP_DELAY] = 0xffffffff;  /*close input repeat*/
	input_device->rep[REP_PERIOD] = 0xffffffff; /*close input repeat*/

	input_device->id.bustype = BUS_ISA;
	input_device->id.vendor  = 0x0001;
	input_device->id.product = 0x0001;
	input_device->id.version = 0x0100;

	for (i = KEY_ESC; i < KEY_UWB; i++)
		input_set_capability(input_device, EV_KEY, i);
	for (i = KEY_OK; i < KEY_FN_B; i++)
		input_set_capability(input_device, EV_KEY, i);
	for (i = BTN_MOUSE; i < BTN_SIDE; i++)
		input_set_capability(input_device, EV_KEY, i);
	input_set_capability(input_device, EV_REL, REL_X);
	input_set_capability(input_device, EV_REL, REL_Y);
	input_set_capability(input_device, EV_REL, REL_WHEEL);

	ret = input_register_device(input_device);
	if (ret < 0) {
		input_free_device(input_device);
		return ret;
	}

	common_input_data.common_input = input_device;

	return 0;
}

void meson_ir_common_input_exit(void)
{
	input_unregister_device(common_input_data.common_input);
	input_free_device(common_input_data.common_input);
}

