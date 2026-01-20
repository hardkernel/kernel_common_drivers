// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <linux/printk.h>
#include <linux/sysfs.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/in.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/amlogic/aml_phy_debug.h>
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
#define DATA_TYPE_IPV4_ADDR		0x03	/* Set IPv4 address */
#define DATA_TYPE_OFFLOADSTATE		0x04	/* For mDNS offload */
#define DATA_TYPE_RESET_ALL		0x05	/* For mDNS offload */
#define DATA_TYPE_RECORD_KEY		0x06	/* For mDNS offload */
#define DATA_TYPE_ADD_RESPONSE		0x07	/* For mDNS offload */
#define DATA_TYPE_REMOVE_RESPONSE	0x08	/* For mDNS offload */
#define DATA_TYPE_RESET_HITCOUNT	0x09	/* For mDNS offload */
#define DATA_TYPE_RESET_MISSCOUNT	0x0a	/* For mDNS offload */
#define DATA_TYPE_ADD_PASST		0x0b	/* For mDNS offload */
#define DATA_TYPE_REMOVE_PASST		0x0c	/* For mDNS offload */
#define DATA_TYPE_PASST_BEHAVIOR	0x0d	/* For mDNS offload */
#define DATA_TYPE_MDNS_FRAME_SIZE	0x0e	/* For mDNS offload */
#define DATA_TYPE_MDNS_FRAME_INFO	0x0f	/* For mDNS offload */
#define DATA_TYPE_GET_PASST_STATUS	0x10	/* For mDNS offload */
#define DATA_TYPE_IPV4_MASK		0x11	/* Set IPv4 Mask */
#define DATA_TYPE_SET_WAKEUP_PORT_NUM	0x12	/* For wakeup port */
#define DATA_TYPE_SET_WAKEUP_PORT	0x13	/* For wakeup port */

#define WKUP_REASON_UNUSED		0x00
#define WKUP_REASON_MAGIC		0x01	/* Magic packet wakeup */
#define WKUP_REASON_MDNS		0x02	/* mDNS wakeup */
#define WKUP_REASON_PORT		0x03	/* Port wakeup */

#define WKUP_VALID			BIT(0)
#define WKUP_PROTO			BIT(1)
#define WKUP_TYPE			BIT(2)

static const char * const wakeup_names[] = {
	[WKUP_REASON_UNUSED]	= "unused",
	[WKUP_REASON_MAGIC]	= "magic",
	[WKUP_REASON_MDNS]	= "mdns",
	[WKUP_REASON_PORT]	= "port"
};

static struct device *dev;
static struct mbox_chan *mbox;
static struct mutex lock;
static u32 wakeup_src;
static struct sock *g_sk;
static int g_mdnsoffload_result;

#define MDNS_LIST_CRITERIA_MAX          8
#define MDNS_RAW_DATA_LENGTH_MAX        492
#define MDNS_QNAME_LENGTH_MAX           80
#define MDNS_WAKE_PORT_MAX              16
#define NETLINK_TEST                    17
#define TYPE_SET_OFFLOAD_STATE          1
#define TYPE_ENABLE_NETLINK_SERVER      2
#define TYPE_IPV4_PEER_ADDR             3
#define TYPE_ADD_RESPONSE               4
#define TYPE_REMOVE_RESPONSE            5
#define TYPE_SET_PASSTHROUGH_BEHAVIOR   6
#define TYPE_SET_WAKEUP_PORT            7

#pragma pack(push, 1)
struct match_criteria {
	u16 type;
	u16 offset;
};

#pragma pack(pop)

#pragma pack(push, 1)
struct mdns_protocol_data {
	s32 hit_counter;
	u8 valid;
	u8 list_len;
	u16 raw_len;
	u16 web_port;
	u16 srv_port;
	struct match_criteria list_criteria[MDNS_LIST_CRITERIA_MAX];
	u8 raw_offload_packet[MDNS_RAW_DATA_LENGTH_MAX];
};

#pragma pack(pop)

#pragma pack(push, 1)
struct mdns_passthrough {
	u8 valid;
	u8 qname_len;
	u8 qname[MDNS_QNAME_LENGTH_MAX];
};

#pragma pack(pop)

enum passthrough_behavior_enum {
	FORWARD_ALL,
	DROP_ALL,
	PASSTHROUGH_LIST,
};

enum wp_proto {
	PROTO_TCP = 0,
	PROTO_UDP = 1,
};

enum wp_type {
	PORT_LOCAL = 0,
	PORT_REMOTE = 1,
};

struct wake_up_port {
	u8 flags;
	u16 port;
};

static int handle_offload_msg(unsigned char *puc, const char *mode);
static u8 set_mdns_notify_bl30(int flag);

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

static ssize_t mdnsoffload_show(const struct class *class, const struct class_attribute *attr,
				char *buf)
{
	(void)class;
	(void)attr;
	(void)buf;

	return 0;
}

static ssize_t mdnsoffload_store(const struct class *class, const struct class_attribute *attr,
				 const char *buf, size_t count)
{
	g_mdnsoffload_result = handle_offload_msg((unsigned char *)buf, "SYSFS");
	print_hex_dump(KERN_DEBUG, "mdnsoffload: ", DUMP_PREFIX_OFFSET, 16, 1, buf, count, true);
	return count;
}
static CLASS_ATTR_RW(mdnsoffload);

static ssize_t mdnsoffload_result_show(const struct class *class,
				       const struct class_attribute *attr, char *buf)
{
	int ret = __get_wakeup_reason();

	if (ret < 0)
		return ret;

	return sysfs_emit(buf, "%d\n", g_mdnsoffload_result);
}
static CLASS_ATTR_RO(mdnsoffload_result);

static struct attribute *wol_class_attrs[] = {
	&class_attr_wakeup_src.attr,
	&class_attr_wakeup_reason.attr,
	&class_attr_mdnsoffload.attr,
	&class_attr_mdnsoffload_result.attr,
	NULL,
};
ATTRIBUTE_GROUPS(wol_class);

static struct class wol_class = {
	.name =         "aml_wol",
	.class_groups = wol_class_groups,
};

static void __set_ipv4_address(void)
{
	struct net_device *ndev = dev_get_drvdata(dev);
	struct in_device *in_dev;
	struct in_ifaddr *ifa;
	__be32 ip, ip_mask;
	char ip_str[16];
	bool found = false;

	rcu_read_lock();
	in_dev = __in_dev_get_rcu(ndev);
	if (in_dev) {
		/* Find available IPv4 addresses */
		in_dev_for_each_ifa_rcu(ifa, in_dev) {
			ip = ifa->ifa_local;
			ip_mask = ifa->ifa_mask;
			snprintf(ip_str, sizeof(ip_str), "%pI4", &ip);
			found = true;
			break;
		}
	}
	rcu_read_unlock();

	if (found) {
		__mbox_data_write(DATA_TYPE_IPV4_ADDR, &ip, 4);
		__mbox_data_write(DATA_TYPE_IPV4_MASK, &ip_mask, 4);
	}
}

bool amlogic_wol_wakeup_src_not_empty(void)
{
	bool retval = false;
	int index;

	/* Synchronize from BL30 */
	__mbox_data_read(DATA_TYPE_WKUP_SRC, &wakeup_src, 4);

	for (index = 1; index < ARRAY_SIZE(wakeup_names); index++) {
		if (wakeup_src & BIT(index)) {
			retval = true;
			break;
		}
	}

	return retval;
}
EXPORT_SYMBOL_GPL(amlogic_wol_wakeup_src_not_empty);

void amlogic_wol_wakeup_src_clr_all(void)
{
	wakeup_src = 0;

	/* Synchronize to BL30 */
	__mbox_data_write(DATA_TYPE_WKUP_SRC, &wakeup_src, 4);
}
EXPORT_SYMBOL_GPL(amlogic_wol_wakeup_src_clr_all);

void amlogic_wol_wakeup_src_set_all(void)
{
	int index;

	wakeup_src = 0;

	for (index = 1; index < ARRAY_SIZE(wakeup_names); index++)
		wakeup_src |= BIT(index);

	/* Synchronize to BL30 */
	__mbox_data_write(DATA_TYPE_WKUP_SRC, &wakeup_src, 4);
}
EXPORT_SYMBOL_GPL(amlogic_wol_wakeup_src_set_all);

void amlogic_wol_enter(void)
{
	__set_ipv4_address();
	set_mdns_notify_bl30(1);
}
EXPORT_SYMBOL_GPL(amlogic_wol_enter);

bool amlogic_wol_exit(void)
{
	int ret = __get_wakeup_reason();
	bool report_event = false;

	set_mdns_notify_bl30(0);

	if (ret < 0)
		goto out;
	pr_debug("%s wakeup\n", wakeup_names[ret]);

	switch (ret) {
	case WKUP_REASON_MAGIC:
		report_event = true;
		break;
	case WKUP_REASON_MDNS:
		report_event = false;
		break;
	case WKUP_REASON_PORT:
		report_event = false;
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

	/* Bridging up to the old sysfs control node */
	wol_sysfs_hook.not_empty = amlogic_wol_wakeup_src_not_empty;
	wol_sysfs_hook.clr_all = amlogic_wol_wakeup_src_clr_all;
	wol_sysfs_hook.set_all = amlogic_wol_wakeup_src_set_all;
}
EXPORT_SYMBOL_GPL(amlogic_wol_setup);

void amlogic_wol_remove(void)
{
	memset(&wol_sysfs_hook, 0, sizeof(wol_sysfs_hook));
	class_unregister(&wol_class);
	dev = NULL;
	mbox = NULL;
}
EXPORT_SYMBOL_GPL(amlogic_wol_remove);

static u8 mdns_set_offload_state(u8 enable)
{
	u8 ret = 0;

	__mbox_data_write(DATA_TYPE_OFFLOADSTATE, &enable, 1);
	__mbox_data_read(DATA_TYPE_OFFLOADSTATE, &ret, 1);
	return ret;
}

static void mdns_remove_response(int record_key)
{
	__mbox_data_write(DATA_TYPE_RECORD_KEY, &record_key, 4);
	__mbox_notify(DATA_TYPE_REMOVE_RESPONSE);
}

static int __maybe_unused mdns_get_reset_hitcount(int record_key)
{
	int hit_counter = -1;

	__mbox_data_write(DATA_TYPE_RECORD_KEY, &record_key, 4);
	__mbox_data_read(DATA_TYPE_RESET_HITCOUNT, &hit_counter, 4);
	return hit_counter;
}

static int __maybe_unused mdns_get_reset_misscount(void)
{
	int miss_counter = -1;

	__mbox_data_read(DATA_TYPE_RESET_MISSCOUNT, &miss_counter, 4);
	return miss_counter;
}

static void __maybe_unused mdns_set_passthrough_behavior(u8 behavior)
{
	__mbox_data_write(DATA_TYPE_PASST_BEHAVIOR, &behavior, 1);
}

static void __maybe_unused mdns_reset_all(void)
{
	__mbox_notify(DATA_TYPE_RESET_ALL);
}

static int mdns_get_mdns_frame(u8 *buf, u16 buf_len)
{
	u16 frame_size = 0;

	__mbox_data_read(DATA_TYPE_MDNS_FRAME_SIZE, &frame_size, 1);
	__mbox_data_read(DATA_TYPE_MDNS_FRAME_INFO, buf,
			 buf_len > frame_size ? frame_size : buf_len);

	return frame_size;
}

static int mdns_add_response(u8 *offload_data, u16 offload_size,
			      struct match_criteria *criteria_data,
			      u8 criteria_size, u16 web_port, u16 srv_port)
{
	int ret = -1;
	struct mdns_protocol_data protocol_data;

	protocol_data.hit_counter = 0;
	protocol_data.valid = true;
	protocol_data.list_len = criteria_size;
	protocol_data.raw_len = offload_size;
	protocol_data.web_port = web_port;
	protocol_data.srv_port = srv_port;
	memcpy(protocol_data.list_criteria, criteria_data,
		protocol_data.list_len * sizeof(struct match_criteria));
	memcpy(protocol_data.raw_offload_packet, offload_data, protocol_data.raw_len);

	__mbox_data_write(DATA_TYPE_ADD_RESPONSE, &protocol_data, sizeof(protocol_data));
	__mbox_data_read(DATA_TYPE_RECORD_KEY, &ret, 4);
	return ret;
}

static u8 __maybe_unused mdns_add_passthrough(u8 *qname, u8 len)
{
	u8 ret = 0;
	struct mdns_passthrough passthrough_data;

	passthrough_data.valid = true;
	passthrough_data.qname_len = len;
	memcpy(passthrough_data.qname, qname, passthrough_data.qname_len);

	__mbox_data_write(DATA_TYPE_ADD_PASST, &passthrough_data, sizeof(passthrough_data));
	__mbox_data_read(DATA_TYPE_GET_PASST_STATUS, &ret, 1);
	return ret;
}

static void __maybe_unused mdns_remove_passthrough(u8 *qname, u8 len)
{
	struct mdns_passthrough passthrough_data;

	passthrough_data.valid = true;
	passthrough_data.qname_len = len;
	memcpy(passthrough_data.qname, qname, passthrough_data.qname_len);

	__mbox_data_write(DATA_TYPE_REMOVE_PASST, &passthrough_data, sizeof(passthrough_data));
}

static void __maybe_unused mdns_set_wakeup_port(struct wake_up_port *wp, u32 port_num)
{
	__mbox_data_write(DATA_TYPE_SET_WAKEUP_PORT_NUM, &port_num, 1);
	__mbox_data_write(DATA_TYPE_SET_WAKEUP_PORT, wp, sizeof(struct wake_up_port) * port_num);
}

void mdns_netlink_recv_msg(struct sk_buff *skb)
{
	struct sk_buff *skb_out;
	struct nlmsghdr *nlh;
	int msg_size;
	unsigned char *msg;
	unsigned char *puc;
	int len, type;
	char outmsg[32] = "Done!";
	int outmsg_size = strlen(outmsg);
	int pid;
	int res;

	nlh = (struct nlmsghdr *)skb->data;
	pid = nlh->nlmsg_pid; /* pid of sending process */
	msg = (unsigned char *)nlmsg_data(nlh);
	puc = msg;
	type = *((int *)puc);
	len = *((int *)(puc + 4));

	msg_size = strlen(msg);

	// create reply
	skb_out = nlmsg_new(msg_size, 0);
	if (!skb_out) {
		pr_err("%s: Failed to allocate new skb\n", __func__);
		return;
	}

	// put received message into reply
	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
	if (!nlh) {
		pr_err("%s: Failed to put nlmsg\n", __func__);
		kfree_skb(skb_out);
		return;
	}
	NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */

	res = handle_offload_msg(puc, "NETLINK");
	snprintf(outmsg, sizeof(outmsg), (res < 0) ? "Unknown message" : "Done: %d", res);
	outmsg_size = strlen(outmsg);
	strncpy(nlmsg_data(nlh), outmsg, outmsg_size);

	res = nlmsg_unicast(g_sk, skb_out, pid);
	if (res < 0)
		pr_err("%s: Error while sending skb to user\n", __func__);
}

int mdns_netlink_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = mdns_netlink_recv_msg,
	};
	if (g_sk)
		return 0;

	g_sk = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
	if (!g_sk) {
		pr_err("%s: Error creating socket.\n", __func__);
		return -10;
	}

	return 0;
}

int handle_offload_msg(unsigned char *puc, const char *mode)
{
	int len, type;

	type = *((int *)puc);
	len = *((int *)(puc + 4));

	print_hex_dump(KERN_DEBUG, "OFFLOAD-MSG: ", DUMP_PREFIX_OFFSET, 16, 1, puc + 8, len, true);

	if (type == TYPE_SET_OFFLOAD_STATE && len == 1) {
		unsigned char val = puc[8];

		return set_mdns_notify_bl30(val);
	} else if (type == TYPE_ENABLE_NETLINK_SERVER && len == 1) {
		unsigned char val = puc[8];

		if (val == 1)
			mdns_netlink_init();
	} else if (type == TYPE_ADD_RESPONSE) {
		int response_len = *((int *)(puc + 8));
		unsigned char *response;
		unsigned char *q;
		int i;
		int numberOfCriteria;
		struct match_criteria *list;
		unsigned short web_port, srv_port;

		q = puc + 4 + 4 + 4;
		response = q;
		print_hex_dump(KERN_DEBUG, "MDNS response: ",
			DUMP_PREFIX_OFFSET, 16, 1, q, response_len, true);
		q += response_len;
		numberOfCriteria = *((int *)q);
		q += 4;
		list = (struct match_criteria *)q;
		for (i = 0; i < numberOfCriteria; i++)
			q += sizeof(struct match_criteria);

		web_port = *((unsigned short *)q);
		q += 2;
		srv_port = *((unsigned short *)q);

		return mdns_add_response(response, response_len,
			list, numberOfCriteria, web_port, srv_port);
	} else if (type == TYPE_REMOVE_RESPONSE && len == 1) {
		unsigned char val = puc[8];

		mdns_remove_response(val);
	} else if (type == TYPE_SET_PASSTHROUGH_BEHAVIOR && len == 1) {
		unsigned char val = puc[8];

		mdns_set_passthrough_behavior(val);
	} else if (type == TYPE_SET_WAKEUP_PORT) {
		struct wake_up_port wp[MDNS_WAKE_PORT_MAX];
		int i, port_num;
		unsigned char *q;

		port_num = *((u32 *)(puc + 8));
		if (port_num <= 0 || port_num > MDNS_WAKE_PORT_MAX) {
			pr_err("%s: Invalid port number: %d\n", __func__, port_num);
			return -1;
		}
		q = puc + 4 + 4 + 4;
		for (i = 0; i < port_num; i++) {
			unsigned char protocol, matcher;
			unsigned short port;

			protocol = *((unsigned char *)q);
			q += 1;
			matcher = *((unsigned char *)q);
			q += 1;
			port = *((unsigned short *)q);
			q += 2;
			wp[i].flags = WKUP_VALID;
			if (protocol == PROTO_UDP)
				wp[i].flags |= WKUP_PROTO;
			if (matcher == PORT_REMOTE)
				wp[i].flags |= WKUP_TYPE;
			wp[i].port = port;
		}
		mdns_set_wakeup_port(wp, port_num);
	} else {
		pr_err("%s: unsupported type: %d\n", mode, type);
		return -1;
	}

	return 0;
}

static u8 set_mdns_notify_bl30(int flag)
{
	int ret;
	u8 set_ret;
	int miss_counter;
	int hit_counter;
	int record_key = 0;
	u8 behavior = PASSTHROUGH_LIST;
	char buf[1024] = { 0 };

	if (flag) {
		mdns_set_passthrough_behavior(behavior);
		miss_counter = mdns_get_reset_misscount();
		hit_counter = mdns_get_reset_hitcount(record_key);
		set_ret = mdns_set_offload_state(1);
	} else {
		set_ret = mdns_set_offload_state(0);
		ret = mdns_get_mdns_frame(buf, ARRAY_SIZE(buf));
		print_hex_dump(KERN_DEBUG, "MDNS frame: ",
			DUMP_PREFIX_OFFSET, 16, 1, buf, ret, true);
	}

	return set_ret;
}
