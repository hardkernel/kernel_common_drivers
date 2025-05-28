// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/* Copyright (c) 2025 Amlogic, Inc. All rights reserved. */

#include <linux/printk.h>
#include <linux/sysfs.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include "amlogic-wol.h"

#undef pr_fmt
#define pr_fmt(fmt) "amlogic-wol: " fmt

#define PAYLOAD_DATA_SIZE		(MBOX_DATA_SIZE - 4)

#pragma pack(push, 1)
struct mbox_payload {
	u8	ctrl;				/* Controlling the transfer buffer */
	u8	type;
	u8	len;				/* Payload data size */
	u8	mode;				/* This value is always 0x80 */
	u8	data[PAYLOAD_DATA_SIZE];
};

#pragma pack(pop)

#define CTRL_FLAG_WRITE			BIT(1)	/* 0: Read  1: Write */
#define CTRL_FLAG_START			BIT(2)
#define CTRL_FLAG_APPEND		BIT(3)
#define CTRL_FLAG_STOP			BIT(4)

#define DATA_TYPE_WKUP_REASON		0x01	/* Get wakeup reason */
#define DATA_TYPE_WKUP_SRC		0x02	/* Controlling the sources that can wake up */

#define WKUP_REASON_UNUSED		0x00
#define WKUP_REASON_MAGIC		0x01	/* Magic packet wakeup */
#define WKUP_REASON_MDNS		0x02	/* mDNS wakeup */

static const char * const wakeup_names[] = {
	[WKUP_REASON_UNUSED]	= "unused",
	[WKUP_REASON_MAGIC]	= "magic",
	[WKUP_REASON_MDNS]	= "mdns"
};

static struct device *dev;
static struct mbox_chan *mbox;
static struct mutex lock;
static u32 wakeup_src;

static inline void __mbox_data_swap(void *wdata, u32 wlen, void *rdata, u32 rlen)
{
	int ret;

	ret = aml_mbox_transfer_data(mbox, MBOX_CMD_SET_ETHERNET_WOL,
				     wdata, wlen, rdata, rlen, MBOX_SYNC);
	if (ret < 0)
		pr_err("mbox transfer failed\n");
}

static void __maybe_unused __mbox_notify(u8 type)
{
	struct mbox_payload payload = {
		.mode	= 0x80,
		.ctrl	= CTRL_FLAG_START | CTRL_FLAG_STOP,
		.type	= type,
		.len	= 0,
	};

	mutex_lock(&lock);
	__mbox_data_swap(&payload, sizeof(payload), NULL, 0);
	mutex_unlock(&lock);
}

static void __maybe_unused __mbox_data_write(u8 type, const void *data, u32 len)
{
	u32 offset = 0;
	u32 left_size;
	struct mbox_payload payload = {
		.mode	= 0x80,
		.ctrl	= CTRL_FLAG_START | CTRL_FLAG_WRITE,
		.type	= type,
	};

	mutex_lock(&lock);
	while (offset < len) {
		left_size = len - offset;
		payload.len = (u8)(left_size > PAYLOAD_DATA_SIZE ?
				   PAYLOAD_DATA_SIZE : left_size);
		memcpy(payload.data, (const u8 *)data + offset, payload.len);
		payload.ctrl |= (offset + payload.len) == len ? CTRL_FLAG_STOP : CTRL_FLAG_APPEND;
		__mbox_data_swap(&payload, sizeof(payload), NULL, 0);
		offset += payload.len;
		payload.ctrl = CTRL_FLAG_WRITE;
	}
	mutex_unlock(&lock);
}

static void __maybe_unused __mbox_data_read(u8 type, void *data, u32 len)
{
	u32 offset = 0;
	u32 left_size;
	struct mbox_payload payload = {
		.mode	= 0x80,
		.ctrl	= CTRL_FLAG_START,
		.type	= type,
	};

	mutex_lock(&lock);
	while (offset < len) {
		left_size = len - offset;
		payload.len = (u8)(left_size > PAYLOAD_DATA_SIZE ?
				   PAYLOAD_DATA_SIZE : left_size);
		payload.ctrl |= (offset + payload.len) == len ? CTRL_FLAG_STOP : CTRL_FLAG_APPEND;
		__mbox_data_swap(&payload, sizeof(payload), &payload, sizeof(payload));
		memcpy((u8 *)data + offset, payload.data, payload.len);
		offset += payload.len;
		payload.ctrl = 0;
	}
	mutex_unlock(&lock);
}

static ssize_t wakeup_src_show(const struct class *class, const struct class_attribute *attr,
			       char *buf)
{
	int len = 0;
	int index;

	/* Synchronize from BL30 */
	__mbox_data_read(DATA_TYPE_WKUP_SRC, &wakeup_src, 4);

	for (index = 1; index < ARRAY_SIZE(wakeup_names); index++) {
		if (!(wakeup_src & BIT(index)))
			continue;
		len += sysfs_emit_at(buf, len, "%s,", wakeup_names[index]);
	}
	if (len > 0 && buf[len - 1] == ',')
		buf[len - 1] = '\n';

	return len;
}

static ssize_t wakeup_src_store(const struct class *class, const struct class_attribute *attr,
				const char *buf, size_t count)
{
	char *nbuf;
	char *p;
	char *name;
	int index;

	nbuf = kzalloc(count + 1, GFP_KERNEL);
	if (!nbuf)
		return -ENOMEM;
	p = nbuf;

	memcpy(nbuf, buf, count);
	strreplace(nbuf, '\n', '\0');

	/* clear all sources */
	wakeup_src = 0;

	while (1) {
		name = strsep(&p, ",");
		if (!name || !strlen(name))
			break;

		for (index = 1; index < ARRAY_SIZE(wakeup_names); index++) {
			if (!strcmp(wakeup_names[index], name))
				break;
		}

		if (index == ARRAY_SIZE(wakeup_names))
			pr_warn("unknown wakeup source: %s\n", name);
		else
			wakeup_src |= BIT(index);
	}

	kfree(nbuf);

	/* Synchronize to BL30 */
	__mbox_data_write(DATA_TYPE_WKUP_SRC, &wakeup_src, 4);

	return count;
}
static CLASS_ATTR_RW(wakeup_src);

static int __get_wakeup_reason(void)
{
	u8 reason = 0;

	__mbox_data_read(DATA_TYPE_WKUP_REASON, &reason, 1);
	if (reason >= ARRAY_SIZE(wakeup_names)) {
		pr_err("unknown wakeup reason: 0x%02x\n", reason);
		return -EINVAL;
	}

	return reason;
}

static ssize_t wakeup_reason_show(const struct class *class, const struct class_attribute *attr,
				  char *buf)
{
	int ret = __get_wakeup_reason();

	if (ret < 0)
		return ret;

	return sysfs_emit(buf, "%s\n", wakeup_names[ret]);
}
static CLASS_ATTR_RO(wakeup_reason);

static struct attribute *wol_class_attrs[] = {
	&class_attr_wakeup_src.attr,
	&class_attr_wakeup_reason.attr,
	NULL,
};
ATTRIBUTE_GROUPS(wol_class);

static struct class wol_class = {
	.name =         "aml_wol",
	.class_groups = wol_class_groups,
};

void amlogic_wol_enter(void)
{
}
EXPORT_SYMBOL_GPL(amlogic_wol_enter);

bool amlogic_wol_exit(void)
{
	int ret = __get_wakeup_reason();
	bool report_event = false;

	if (ret < 0)
		goto out;
	pr_info("%s wakeup\n", wakeup_names[ret]);

	switch (ret) {
	case WKUP_REASON_MAGIC:
		report_event = true;
		break;
	case WKUP_REASON_MDNS:
		report_event = true;
		break;
	}

out:
	return report_event;
}
EXPORT_SYMBOL_GPL(amlogic_wol_exit);

void amlogic_wol_setup(struct device *device, struct mbox_chan *mbox_chan)
{
	dev = device;
	mbox = mbox_chan;
	mutex_init(&lock);
	WARN_ON(class_register(&wol_class) < 0);
}
EXPORT_SYMBOL_GPL(amlogic_wol_setup);

void amlogic_wol_remove(void)
{
	class_unregister(&wol_class);
	dev = NULL;
	mbox = NULL;
}
EXPORT_SYMBOL_GPL(amlogic_wol_remove);
