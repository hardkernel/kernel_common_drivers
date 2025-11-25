// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

//#define DEBUG
#include <linux/module.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>

#define INPUT_FILTER_KEY_MAX	32
#define INPUT_FILTER_MODE_USAGE	"usage:\n0:report all events\n"		\
				"1:block all events\n"			\
				"2:block events which in [filter_key]\n"

#define INPUT_FILTER_KEY_USAGE	"usage:\nnull: clear all keys\n"	\
				"+{key1}:{key2}:...:{key32} : add keys\n"   \
				"-{key1}:{key2}:...:{key32} : remove keys\n"

struct meson_input_filter {
	u8 filter_mode;
	int filter_key[INPUT_FILTER_KEY_MAX];
	struct class filter_class;
};

static ssize_t filter_mode_show(const struct class *cls, const struct class_attribute *attr,
				char *buf)
{
	struct meson_input_filter *input_filter = container_of(cls,
				       struct meson_input_filter, filter_class);

	return sprintf(buf, "mode: %d\n" INPUT_FILTER_MODE_USAGE,
		       input_filter->filter_mode);
}

static ssize_t filter_mode_store(const struct class *cls,
				 const struct class_attribute *attr,
				 const char *buf, size_t count)
{
	struct meson_input_filter *input_filter = container_of(cls,
				       struct meson_input_filter, filter_class);
	int mode, ret;

	ret = kstrtoint(buf, 0, &mode);
	if (ret != 0)
		return -EINVAL;
	if (mode != 0 && mode != 1 && mode != 2)
		return -EINVAL;

	input_filter->filter_mode = mode;

	return count;
}

static ssize_t filter_key_show(const struct class *cls, const struct class_attribute *attr,
			       char *buf)
{
	struct meson_input_filter *input_filter = container_of(cls,
				       struct meson_input_filter, filter_class);
	int i, len = 0;

	for (i = 0; i < INPUT_FILTER_KEY_MAX; i++) {
		if (!input_filter->filter_key[i])
			break;
		len += sprintf(buf + len, "%d: ", input_filter->filter_key[i]);
	}
	len += sprintf(buf + len, "\n");
	len += sprintf(buf + len, INPUT_FILTER_KEY_USAGE);

	return len;
}

static ssize_t filter_key_store(const struct class *cls, const struct class_attribute *attr,
				const char *buf, size_t count)
{
	struct meson_input_filter *input_filter = container_of(cls,
				       struct meson_input_filter, filter_class);
	int keycode, nsize = 0, pos = 0;
	char nbuf[INPUT_FILTER_KEY_MAX * 4];
	char *pbuf = nbuf;
	char *pval;
	int i, op;

	/*count included '\0'*/
	if (count > (INPUT_FILTER_KEY_MAX * 4)) {
		pr_err("write data is too long\n");
		return -EINVAL;
	}

	/*write "null" or "NULL" to clean up all key table*/
	if (memcmp("null", buf, 4) == 0) {
		memset(input_filter->filter_key, 0,
		       sizeof(input_filter->filter_key));
		return count;
	} else if (memcmp("+", buf, 1) == 0) {	/* 1:append 0:delete */
		op = 1;
	} else if (memcmp("-", buf, 1) == 0) {	/* 1:append 0:delete */
		op = 0;
	} else {
		return -EINVAL;
	}

	/*trim all invisible characters include '\0', tab, space etc*/
	while (*buf) {
		if (*buf >= '0')
			nbuf[nsize++] = *buf;
		buf++;
	}
	nbuf[nsize] = '\0';

	while (input_filter->filter_key[pos]) {
		pos++;
		if (pos >= INPUT_FILTER_KEY_MAX)
			break;
	}

	if (op && pos == INPUT_FILTER_KEY_MAX)
		return -ENOSPC;

	do {
		pval = strsep(&pbuf, ":"); /*code*/
		if (!pval)
			continue;

		if (kstrtoint(pval, 0, &keycode) < 0)
			return -EINVAL;
		if (keycode >= KEY_MAX)
			return -EINVAL;
		for (i = 0; i < pos; i++)
			if (keycode == input_filter->filter_key[i])
				break;

		if (op) {
			if (i != pos)
				continue;
			input_filter->filter_key[pos++] = keycode;
			if (pos >= INPUT_FILTER_KEY_MAX)
				return -ENOSPC;
		} else {
			if (i == pos)
				continue;
			for (; i < pos - 1; i++)
				input_filter->filter_key[i] =
						input_filter->filter_key[i + 1];
			input_filter->filter_key[i] = 0;
			pos--;
		}
	} while (pval);

	return count;
}

static CLASS_ATTR_RW(filter_mode);
static CLASS_ATTR_RW(filter_key);

static struct attribute *input_filter_attrs[] = {
	&class_attr_filter_mode.attr,
	&class_attr_filter_key.attr,
	NULL
};
ATTRIBUTE_GROUPS(input_filter);

static bool input_filter_filter(struct input_handle *handle, unsigned int type,
				unsigned int code, int value)
{
	struct meson_input_filter *input_filter = (struct meson_input_filter *)
						  handle->handler->private;
	int i;

	if (type == EV_SYN)
		return false;

	switch (input_filter->filter_mode) {
	case 0:
		return false;
	case 1:
		return true;
	case 2:
		if (type != EV_KEY)
			return false;
		for (i = 0; i < ARRAY_SIZE(input_filter->filter_key); i++) {
			if (code == input_filter->filter_key[i])
				return true;
		}
		return false;
	default:
		break;
	}

	return false;
}

static int input_filter_connect(struct input_handler *handler,
			struct input_dev *dev, const struct input_device_id *id)
{
	struct input_handle *handle;
	int error;

	handle = kzalloc(sizeof(*handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "input_filter";

	error = input_register_handle(handle);
	if (error)
		goto err_free_handle;

	error = input_open_device(handle);
	if (error)
		goto err_unregister_handle;

	return 0;

err_unregister_handle:
	input_unregister_handle(handle);
err_free_handle:
	kfree(handle);
	return error;
}

static void input_filter_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

static const struct input_device_id input_ids[] = {
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT,
		.evbit = { BIT_MASK(EV_KEY) },
	},
	{ },
};

static struct input_handler input_filter_handler = {
	.filter = input_filter_filter,
	.connect = input_filter_connect,
	.disconnect = input_filter_disconnect,
	.name = "input_filter",
	.id_table = input_ids,
};

int __init input_filter_init(void)
{
	int ret;
	struct meson_input_filter *input_filter;

	input_filter = kzalloc(sizeof(*input_filter), GFP_KERNEL);
	if (!input_filter)
		return -ENOMEM;

	input_filter_handler.private = input_filter;
	ret = input_register_handler(&input_filter_handler);
	if (ret)
		return ret;

	input_filter->filter_class.name = input_filter_handler.name;
	input_filter->filter_class.class_groups = input_filter_groups;
	ret = class_register(&input_filter->filter_class);
	if (ret) {
		input_unregister_handler(&input_filter_handler);
		return ret;
	}

	return 0;
}

void __exit input_filter_exit(void)
{
	struct meson_input_filter *input_filter = input_filter_handler.private;

	class_unregister(&input_filter->filter_class);
	input_unregister_handler(&input_filter_handler);
	kfree(input_filter);
}
