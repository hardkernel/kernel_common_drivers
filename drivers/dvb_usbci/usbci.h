/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */
#ifndef _AMLOGIC_USBCAM_H
#define _AMLOGIC_USBCAM_H

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/usb.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include <linux/mm.h>
#include <linux/moduleparam.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/gfp.h>
#include <linux/compat.h>
#include <asm/current.h>
#include <linux/poll.h>
#include <linux/fs.h>

#define USBCAM_MODULE_DRIVER_VERSION	0x02020000	//Version: 2.2.0.0
#define AML_USBCAM_MAX_NAME_LEN			30

#define DEVICE_CLASS					0xEF
#define DEVICE_SUBCLASS					0x02
#define DEVICE_PROTOCOL					0x01

#define MATCHED_INTF_CLASS				0xFF
#define MATCHED_INTF_SUBCLASS			0x11

#define IAD_FUNCTION_CLASS				0xFF
#define IAD_FUNCTION_SUBCLASS			0x11
#define IAD_FUNCTION_PROTOCOL			0x01

#define COMMAND_INTF_CLASS				0xFF
#define COMMAND_INTF_SUBCLASS			0x11
#define COMMAND_INTF_PROTOCOL			0x01

#define MEDIA_INTF_CLASS				0xFF
#define MEDIA_INTF_SUBCLASS				0x11
#define MEDIA_INTF_PROTOCOL				0x02

#define CI20_IAD_FUNCTION_CLASS			0xEF
#define CI20_IAD_FUNCTION_SUBCLASS		0x07
#define CI20_IAD_FUNCTION_PROTOCOL		0x01

#define CI20_MATCHED_INTF_CLASS			0xEF
#define CI20_MATCHED_INTF_SUBCLASS		0x07

#define CI20_COMMAND_INTF_CLASS			0xEF
#define CI20_COMMAND_INTF_SUBCLASS		0x07
#define CI20_COMMAND_INTF_PROTOCOL		0x01

#define CI20_MEDIA_INTF_CLASS			0xEF
#define CI20_MEDIA_INTF_SUBCLASS		0x07
#define CI20_MEDIA_INTF_PROTOCOL		0x02

#define USBCAM_COMMAND_MINOR_BASE		192
#define USBCAM_MEDIA_MINOR_BASE			208

#define USBCAM_MEDIA_BUFFER_SIZE		0x40000
#define USBCAM_CMD_BUFFER_SIZE			0x4000
#define TRUE							1
#define FALSE							0

#define USB_DEVICE_INFO_AND_INTF_INFO(dcl, dsc, dpr, icl, isc) \
				.match_flags = USB_DEVICE_ID_MATCH_DEV_INFO \
						| USB_DEVICE_ID_MATCH_INT_CLASS \
						| USB_DEVICE_ID_MATCH_INT_SUBCLASS, \
				.bDeviceClass = (dcl), \
				.bDeviceSubClass = (dsc), \
				.bDeviceProtocol = (dpr), \
				.bInterfaceClass = (icl), \
				.bInterfaceSubClass = (isc)

static struct usb_device_id aml_usbcam_table[] = {
	{USB_DEVICE_INFO_AND_INTF_INFO(DEVICE_CLASS, DEVICE_SUBCLASS, DEVICE_PROTOCOL,
						MATCHED_INTF_CLASS, MATCHED_INTF_SUBCLASS)},
	{USB_DEVICE_INFO_AND_INTF_INFO(DEVICE_CLASS, DEVICE_SUBCLASS, DEVICE_PROTOCOL,
					CI20_MATCHED_INTF_CLASS, CI20_MATCHED_INTF_SUBCLASS)},
	{}
};

MODULE_DEVICE_TABLE(usb, aml_usbcam_table);

static struct usb_driver aml_usbcam_driver;

enum aml_usbcam_device_type {
	DEVICE_COMMAND = 0,
	DEVICE_MEDIA = 1
};

enum aml_write_status {
	WRITE_STATUS_READY =			0,
	WRITE_STATUS_WAIT_TS_DONE =		1,
	WRITE_STATUS_EMPTY =			2,
	WRITE_STATUS_WAIT_EMPTY_DONE =	3,
	WRITE_STATUS_ERROR =			4
};

enum aml_read_status {
	READ_STATUS_READY =				0,
	READ_STATUS_WAIT_SUBMIT =		1,
	READ_STATUS_EMPTY =				2,
	READ_STATUS_ERROR =				3
};

enum aml_usbcam_device_status {
	DEVICE_STATUS_MATCH =			0,
	DEVICE_STATUS_RUNNING =			1,
	DEVICE_STATUS_DISCONNECT =		2
};

enum aml_usbcam_device_state {
	DEVICE_CONNECT =				0,
	DEVICE_DISCONNECT =				1
};

enum aml_write_open {
	WRITE_OPEN =					0,
	WRITE_CLOSE =					1
};

enum aml_read_open {
	READ_OPEN =						0,
	READ_CLOSE =					1
};

struct aml_usbcam_info {
	unsigned short vendor_id;	//vendor ID
	unsigned short product_id;	//product id
	u32 ci_compatibility;	//compatibility
	unsigned char is_ci20_detected;
	unsigned char arreserve[31];
};

struct aml_usbcam_module_capabilities {
	char ci_manufacturer_name[AML_USBCAM_MAX_NAME_LEN];
	char ci_product_name[AML_USBCAM_MAX_NAME_LEN];
	bool ci_plus_supported;
	bool op_profile_supported;
};

struct aml_usbcam {
	struct usb_device *usbdev;		//usb device descriptor
	struct usb_interface *intf;		//interface descriptor
	struct usb_endpoint_descriptor *in, *out;	//input/output endpoint descriptor

	unsigned char *read_buf;		//read buffer(media 0x40000,cmd 0x4000)
	unsigned char *write_buf;		//write buffer
	dma_addr_t read_dma;			//read buffer DMA address
	dma_addr_t write_dma;			//write buffer DMA address

	struct task_struct *thread;		//kernel thread
	struct device *dev;			//device pointer
	struct cdev cdev;			//character device structure

	size_t buf_size;			//read/write buffer size
	size_t write_size;			//write length
	size_t read_len;			//read length
	size_t write_len;			//write length
	size_t in_transfor;			//read pointer

	struct mutex read_mux;			//read mutex
	struct mutex write_mux;			//write mutex
	struct mutex open_mux;			//open mutex
	spinlock_t read_lock;			//read spin lock
	spinlock_t write_lock;			//write spin lock
	wait_queue_head_t read_wq;		//read wait queue
	wait_queue_head_t write_wq;		//write wait queue

	struct kref ref_count;			//inserted or removed count
	unsigned int open_ref;			//open reference count
	unsigned int read_open_ref;		//read open ref
	unsigned int write_open_ref;		//write open ref
	unsigned char intf_reg;			//interface registration flag

	int read_err;				//read error identifier
	int write_err;				//write error identifier
	int dev_idx;				//device index

	enum aml_usbcam_device_type device_type;	//device type(cmd or media)
	enum aml_usbcam_device_status device_status;		//device status
	enum aml_read_status read_status;		//read buffer status
	enum aml_write_status write_status;		//write status
	enum aml_read_open read_open_status;		//read open status
	enum aml_write_open write_open_status;		//write open status
	enum aml_usbcam_device_state device_state;	//device connection state

	struct urb *read_urb;			//read urb
	struct urb *write_urb;			//write urb

	struct aml_usbcam_info usbcam_module_info;		//usbcam interface message
	struct aml_usbcam_module_capabilities usbcam_module_capabilities;//usbcam capabilities
};

#define AML_USBCAM_IOC_MAGIC					'c'
#define AML_USBCAM_IOC_GET_DRIVER_VERSION		_IOR(AML_USBCAM_IOC_MAGIC, 0, uint32_t)
#define AML_USBCAM_IOC_GET_INFO					_IOR(AML_USBCAM_IOC_MAGIC, 1, \
	struct aml_usbcam_info)
#define AML_USBCAM_IOC_RESET					_IO(AML_USBCAM_IOC_MAGIC, 2)
#define AML_USBCAM_IOC_CANCEL_TRANSFER			_IO(AML_USBCAM_IOC_MAGIC, 3)
#define AML_USBCAM_IOC_MODULE_CAPABILITIES		_IOR(AML_USBCAM_IOC_MAGIC, 4, \
	struct aml_usbcam_module_capabilities)
#define AML_USBCAM_IOC_GET_MODULE_STATE			_IOR(AML_USBCAM_IOC_MAGIC, 5, uint32_t)
#define AML_USBCAM_IOC_SET_MODULE_STATE			_IOR(AML_USBCAM_IOC_MAGIC, 6, uint32_t)
#define AML_USBCAM_IOC_CHECK_DEVICE_PRESENCE	_IO(AML_USBCAM_IOC_MAGIC, 7)

#endif
