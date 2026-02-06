// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>
#include <linux/of.h>
#include <linux/amlogic/media/dvb-core/dvbdev.h>

#include "aml_ci.h"

#include "aml_ci_bus.h"

static int aml_ci_debug;

#define pr_dbg(args...)\
	do {\
		if (aml_ci_debug)\
			pr_info("ci:" args);\
	} while (0)

#define pr_error(fmt, args...) pr_err("ci:" fmt, ## args)

/**\brief aml_ci_mem_read:mem read from cam
 * \param en50221: en50221 obj,used this data to get dvb_ci obj
 * \param slot: slot index
 * \param addr: read addr
 * \return
 *   - read value:ok
 *   - -EINVAL : error
 */
static int aml_ci_mem_read(struct dvb_ca_en50221_cimcu *en50221, int slot, int addr)
{
	struct aml_ci *ci = en50221->data;

	if (slot != 0)
		return -EINVAL;

	if (ci->ci_mem_read)
		return ci->ci_mem_read(ci, slot, addr);

	// pr_error("ci_mem_read is null\r\n");
	return -EINVAL;
}

/**\brief aml_ci_mem_write:mem write to cam
 * \param en50221: en50221 obj,used this data to get dvb_ci obj
 * \param slot: slot index
 * \param addr: write addr
 * \param addr: write value
 * \return
 *   - 0:ok
 *   - -EINVAL : error
 */
static int aml_ci_mem_write(struct dvb_ca_en50221_cimcu *en50221,
			int slot, int addr, u8 data)
{
	struct aml_ci *ci = en50221->data;

	if (slot != 0)
		return -EINVAL;

	if (ci->ci_mem_write)
		return ci->ci_mem_write(ci, slot, addr, data);

	// pr_error("ci_mem_write is null\r\n");
	return -EINVAL;
}

/**\brief aml_ci_io_read:io read from cam
 * \param en50221: en50221 obj,used this data to get dvb_ci obj
 * \param slot: slot index
 * \param addr: read addr
 * \return
 *   - read value:ok
 *   - -EINVAL : error
 */
static int aml_ci_io_read(struct dvb_ca_en50221_cimcu *en50221, int slot, u8 addr)
{
	struct aml_ci *ci = en50221->data;

	if (slot != 0)
		return -EINVAL;

	if (ci->ci_io_read)
		return ci->ci_io_read(ci, slot, addr);

	// pr_error("ci_io_read is null\r\n");
	return -EINVAL;
}

/**\brief aml_ci_io_write:io write to cam
 * \param en50221: en50221 obj,used this data to get dvb_ci obj
 * \param slot: slot index
 * \param addr: write addr
 * \param addr: write value
 * \return
 *   - 0:ok
 *   - -EINVAL : error
 */
static int aml_ci_io_write(struct dvb_ca_en50221_cimcu *en50221,
		int slot, u8 addr, u8 data)
{
	struct aml_ci *ci = en50221->data;

	if (slot != 0)
		return -EINVAL;

	if (ci->ci_mem_write)
		return ci->ci_io_write(ci, slot, addr, data);

	// pr_error("ci_io_write is null\r\n");
	return -EINVAL;
}

/**\brief aml_ci_slot_reset:reset slot
 * \param en50221: en50221 obj,used this data to get dvb_ci obj
 * \param slot: slot index
 * \return
 *   - 0:ok
 *   - -EINVAL : error
 */
static int aml_ci_slot_reset(struct dvb_ca_en50221_cimcu *en50221, int slot)
{
	struct aml_ci *ci = en50221->data;

	// pr_dbg("Slot(%d): Slot RESET\n", slot);
	if (ci->ci_slot_reset) {
		ci->ci_slot_reset(ci, slot);
		return 0;
	} else {
		// pr_error("ci_slot_reset is null\r\n");
		return -EINVAL;
	}
}

/**\brief aml_ci_slot_shutdown:show slot
 * \param en50221: en50221 obj,used this data to get dvb_ci obj
 * \param slot: slot index
 * \return
 *   - 0:ok
 *   - -EINVAL : error
 */
static int aml_ci_slot_shutdown(struct dvb_ca_en50221_cimcu *en50221, int slot)
{
	struct aml_ci *ci = en50221->data;

	// pr_dbg("Slot(%d): Slot shutdown\n", slot);
	if (ci->ci_slot_shutdown) {
		ci->ci_slot_shutdown(ci, slot);
		return 0;
	} else {
		// pr_error("ci_slot_shutdown is null\r\n");
		return -EINVAL;
	}
}

/**\brief aml_ci_ts_control:control slot ts
 * \param en50221: en50221 obj,used this data to get dvb_ci obj
 * \param slot: slot index
 * \return
 *   - 0:ok
 *   - -EINVAL : error
 */
static int aml_ci_ts_control(struct dvb_ca_en50221_cimcu *en50221, int slot)
{
	struct aml_ci *ci = en50221->data;

	// pr_dbg("Slot(%d): TS control\n", slot);
	if (ci->ci_slot_ts_enable) {
		ci->ci_slot_ts_enable(ci, slot);
		return 0;
	} else {
		// pr_error("ci_slot_ts_enable is null\r\n");
		return -EINVAL;
	}
}

/**\brief aml_ci_slot_status:get slot status
 * \param en50221: en50221 obj,used this data to get dvb_ci obj
 * \param slot: slot index
 * \param open: no used
 * \return
 *   - cam status
 *   - -EINVAL : error
 */
static int aml_ci_slot_status(struct dvb_ca_en50221_cimcu *en50221,
		int slot, int open)
{
	struct aml_ci *ci = en50221->data;

	//pr_dbg("Slot(%d): Poll Slot status\n", slot);

	if (ci->ci_poll_slot_status)
		return ci->ci_poll_slot_status(ci, slot, open);

	return 0;
}

/**\brief aml_ci_slot_status:get slot status
 * \param en50221: en50221 obj,used this data to get dvb_ci obj
 * \param slot: slot index
 * \param open: no used
 * \return
 *   - cam status
 *   - -EINVAL : error
 */
static int aml_ci_slot_wakeup(struct dvb_ca_en50221_cimcu *en50221,
		int slot)
{
	struct aml_ci *ci = en50221->data;

	if (ci->ci_get_slot_wakeup)
		return ci->ci_get_slot_wakeup(ci, slot);

	return 1;
}

/**\brief get ci config from dts
 * \param np: device node
 * \return
 *   - 0 成功
 *   - 其他值 :
 */
static int aml_ci_get_config_from_dts(struct platform_device *pdev,
		struct aml_ci *ci)
{
	char buf[32];
	int ret = 0;
	int value;

	snprintf(buf, sizeof(buf), "%s", "io_type");
	ret = of_property_read_u32(pdev->dev.of_node, buf, &value);
	if (!ret) {
		pr_dbg("%s: 0x%x\n", buf, value);
		ci->io_type = value;
	}
	snprintf(buf, sizeof(buf), "%s", "raw_mode");
	ret = of_property_read_u32(pdev->dev.of_node, buf, &value);
	if (!ret) {
		pr_dbg("%s: 0x%x\n", buf, value);
		ci->raw_mode = value;
	}
	ci->regulator_vcc5v = NULL;
	if (of_find_property(pdev->dev.of_node, "vcc5v-supply", NULL)) {
		ci->regulator_vcc5v = devm_regulator_get(&pdev->dev, "vcc5v");
		if (IS_ERR_OR_NULL(ci->regulator_vcc5v)) {
			pr_error("failed in regulator vcc5v %ld\n",
				PTR_ERR(ci->regulator_vcc5v));
			ci->regulator_vcc5v = NULL;
		} else {
			pr_dbg("Use regulator_vcc5v\n");
		}
	}
	return 0;
}

/**\brief aml_ci_init:ci dev init
 * \param pdev: platform_device device node,used to get dts info
 * \param dvb: aml_dvb obj,used to get dvb_adapter for en0211 to use
 * \param cip: ci_dev pp
 * \return
 *   - 0 成功
 *   - 其他值 :
 */
int aml_ci_init(struct platform_device *pdev,
	struct dvb_adapter *dvb_adapter, struct aml_ci **cip)
{
	struct aml_ci *ci = NULL;
	int ca_flags, result = 0;
	int err_val = 0;

	ci = kzalloc(sizeof(*ci), GFP_KERNEL);
	if (!ci) {
		err_val = -1;
		result = -ENOMEM;
		goto err;
	}
	ci->id = 0;
	aml_ci_get_config_from_dts(pdev, ci);

	//ci->priv		= dvb;
	/* register CA interface */

	{
		ca_flags		= ~DVB_CA_EN50221_FLAG_IRQ_CAMCHANGE;
		/* register CA interface */
		ci->en50221_cimcu.owner		= THIS_MODULE;
		ci->en50221_cimcu.read_attribute_mem	= aml_ci_mem_read;
		ci->en50221_cimcu.write_attribute_mem	= aml_ci_mem_write;
		ci->en50221_cimcu.read_cam_control	= aml_ci_io_read;
		ci->en50221_cimcu.write_cam_control	= aml_ci_io_write;
		ci->en50221_cimcu.slot_reset		= aml_ci_slot_reset;
		ci->en50221_cimcu.slot_shutdown	= aml_ci_slot_shutdown;
		ci->en50221_cimcu.slot_ts_enable	= aml_ci_ts_control;
		ci->en50221_cimcu.poll_slot_status	= aml_ci_slot_status;
		ci->en50221_cimcu.get_slot_wakeup	= aml_ci_slot_wakeup;

		ci->en50221_cimcu.data		= ci;

		if (ci->raw_mode == 0) {
			pr_dbg("Registering EN50221 device\n");
			result = dvb_ca_en50221_cimcu_init(dvb_adapter,
				&ci->en50221_cimcu, ca_flags, 1);
			if (result != 0) {
				err_val = -2;
				goto err;
			}
		}
	}
	*cip = ci;
	pr_dbg("Registered EN50221 device\n");

	if (ci->io_type == AML_DVB_IO_TYPE_CIBUS) {
		ci->ci_init = aml_ci_bus_init;
		ci->ci_exit = aml_ci_bus_exit;
	} else {
		/* no io dev init,is error */
		pr_dbg("unknown io type\r\n");
	}

	if (ci->ci_init)
		result = ci->ci_init(pdev, ci);

	return result;
err:
	pr_error("init failed: %d\n", err_val);
	kfree(ci);
	return result;
}

void aml_ci_exit(struct aml_ci *ci)
{
	pr_dbg("Unregistering EN50221 device\n");
	if (ci) {
		if (ci->raw_mode == 0)
			dvb_ca_en50221_cimcu_release(&ci->en50221_cimcu);
		if (ci->ci_exit)
			ci->ci_exit(ci);
		kfree(ci);
	}
}

static struct aml_ci *ci_dev;

static ssize_t ts_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf)
{
	int ret;

	ret = sprintf(buf, "ts%d\n", 1);
	return ret;
}

static ssize_t ci_params_show(const struct class *class,
			const struct class_attribute *attr, char *buf)
{
	int ret = 0, total = 0;

	ret = sprintf(buf, "aml_ci_debug:%d\n", aml_ci_debug);
	total += ret;
	ret = sprintf(buf + total, "cammcu_debug:%d\n", cimcu_get_param(0));
	total += ret;
	ret = sprintf(buf + total, "cammcu_usleep:%d\n", cimcu_get_param(1));
	total += ret;
	ret = sprintf(buf + total, "ciplus_enable:%d\n", cimcu_get_param(2));
	total += ret;
	ret = sprintf(buf + total, "ci_profire:%d\n", cimcu_get_param(3));
	total += ret;
	ret = sprintf(buf + total, "ca_slotstate_validate:%d\n", cimcu_get_param(4));
	total += ret;
	ret = sprintf(buf + total, "read_tuple_time:%d\n", cimcu_get_param(5));
	total += ret;
	ret = sprintf(buf + total, "force_wakeup:%d\n", cimcu_get_param(6));
	total += ret;
	ret = sprintf(buf + total, "ci_bus_debug:%d\n", aml_ci_bus_get_param(0));
	total += ret;
	ret = sprintf(buf + total, "ci_bus_set_delay:%d\n", aml_ci_bus_get_param(1));
	total += ret;
	ret = sprintf(buf + total, "ci_bus_time:%d\n", aml_ci_bus_get_param(2));
	total += ret;
	ret = sprintf(buf + total, "pcmcia_debug:%d\n", pcmcia_get_param(0));
	total += ret;
	ret = sprintf(buf + total, "reset_time_h:%d\n", pcmcia_get_param(1));
	total += ret;
	ret = sprintf(buf + total, "reset_time_l:%d\n", pcmcia_get_param(2));
	total += ret;

	return total;
}

static ssize_t ci_params_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf, size_t size)
{
	char param_name[32];
	int param_value = 0, ret = 0;

	ret = sscanf(buf, "%s %d", param_name, &param_value);
	if (ret != 2) {
		pr_err("invalid command, please use ");
		pr_err("param_name param_value\n");
		return  -EINVAL;
	}

	if (!strncmp(param_name, "aml_ci_debug", strlen("aml_ci_debug")))
		aml_ci_debug = param_value;
	else if (!strncmp(param_name, "cammcu_debug", strlen("cammcu_debug")))
		cimcu_set_param(CI_PARAMS_EN50221_DEBUG, param_value);
	else if (!strncmp(param_name, "cammcu_usleep", strlen("cammcu_usleep")))
		cimcu_set_param(CI_PARAMS_EN50221_USLEEP, param_value);
	else if (!strncmp(param_name, "ciplus_enable", strlen("ciplus_enable")))
		cimcu_set_param(CI_PARAMS_CIPLUS_ENABLE, param_value);
	else if (!strncmp(param_name, "ci_profire", strlen("ci_profire")))
		cimcu_set_param(CI_PARAMS_CI_PROFIRE, param_value);
	else if (!strncmp(param_name, "ca_slotstate_validate", strlen("ca_slotstate_validate")))
		cimcu_set_param(CI_PARAMS_SLOT_STATUS_VALIDATE, param_value);
	else if (!strncmp(param_name, "read_tuple_time", strlen("read_tuple_time")))
		cimcu_set_param(CI_PARAMS_READ_TUPLE_TIME, param_value);
	else if (!strncmp(param_name, "force_wakeup", strlen("force_wakeup")))
		cimcu_set_param(CI_PARAMS_FORCE_WAKEUP, param_value);
	else if (!strncmp(param_name, "ci_bus_debug", strlen("ci_bus_debug")))
		aml_ci_bus_set_param(CI_PARAMS_CI_BUS_DEBUG, param_value);
	else if (!strncmp(param_name, "ci_bus_set_delay", strlen("ci_bus_set_delay")))
		aml_ci_bus_set_param(CI_PARAMS_CI_BUS_SET_DELAY, param_value);
	else if (!strncmp(param_name, "ci_bus_time", strlen("ci_bus_time")))
		aml_ci_bus_set_param(CI_PARAMS_CI_BUS_TIME, param_value);
	else if (!strncmp(param_name, "pcmcia_debug", strlen("pcmcia_debug")))
		pcmcia_set_param(CI_PARAMS_PCMCIA_DEBUG, param_value);
	else if (!strncmp(param_name, "reset_time_h", strlen("reset_time_h")))
		pcmcia_set_param(CI_PARAMS_RESET_TIME_H, param_value);
	else if (!strncmp(param_name, "reset_time_l", strlen("reset_time_l")))
		pcmcia_set_param(CI_PARAMS_RESET_TIME_L, param_value);
	else
		pr_err("commoand invalid: %s\n", buf);
	return size;
}

static CLASS_ATTR_RO(ts);
static CLASS_ATTR_RW(ci_params);

static struct attribute *aml_ci_attrs[] = {
	&class_attr_ts.attr,
	&class_attr_ci_params.attr,
	NULL
};

ATTRIBUTE_GROUPS(aml_ci);

static int aml_ci_register_class(struct aml_ci *ci)
{
	#define CLASS_NAME_LEN 48
	int ret;
	struct class *clp;

	clp = &ci->class;

	clp->name = kzalloc(CLASS_NAME_LEN, GFP_KERNEL);
	if (!clp->name)
		return -ENOMEM;

	snprintf((char *)clp->name, CLASS_NAME_LEN, "amlci");
	clp->class_groups = aml_ci_groups;
	ret = class_register(clp);
	if (ret)
		kfree(clp->name);

	return 0;
}

static int aml_ci_unregister_class(struct aml_ci *ci)
{
	class_unregister(&ci->class);
	kfree(ci->class.name);
	return 0;
}

extern struct dvb_adapter *aml_dvb_get_adapter(struct device *dev);

static int aml_ci_probe(struct platform_device *pdev)
{
	struct dvb_adapter *dvb_adapter = NULL;
	int err = 0;

	dvb_adapter = aml_dvb_get_adapter(&pdev->dev);

	pr_dbg("probe [%p]\n", dvb_adapter);

	err = aml_ci_init(pdev, dvb_adapter, &ci_dev);
	if (err < 0)
		return err;

	if (ci_dev->io_type == AML_DVB_IO_TYPE_CIBUS) {
		pr_dbg("mod init\n");
		aml_ci_bus_mod_init();
	}

	platform_set_drvdata(pdev, ci_dev);
	aml_ci_register_class(ci_dev);

	return 0;
}

static void aml_ci_remove(struct platform_device *pdev)
{
	aml_ci_unregister_class(ci_dev);
	platform_set_drvdata(pdev, NULL);

	if (ci_dev->io_type == AML_DVB_IO_TYPE_CIBUS) {
		aml_ci_bus_mod_exit();
		aml_ci_bus_exit(ci_dev);
	} else {
		pr_dbg("remove io_type error\n");
	}

	aml_ci_exit(ci_dev);
}

static int aml_ci_suspend(struct platform_device *pdev, pm_message_t state)
{
	if (ci_dev->io_type == AML_DVB_IO_TYPE_CIBUS)
		aml_ci_bus_exit(ci_dev);
	else
		pr_dbg("suspend io_type error\n");

	return 0;
}

static int aml_ci_resume(struct platform_device *pdev)
{
	int err = 0;

	if (ci_dev->io_type == AML_DVB_IO_TYPE_CIBUS)
		aml_ci_bus_init(pdev, ci_dev);
	else
		pr_dbg("resume io_type error\n");
	return err;
}

static const struct of_device_id dvbci_dev_dt_match[] = {
	{
		.compatible = "amlogic, dvbci",
	},
	{},
};

static struct platform_driver aml_ci_driver = {
	.probe		= aml_ci_probe,
	.remove		= aml_ci_remove,
	.suspend        = aml_ci_suspend,
	.resume         = aml_ci_resume,
	.driver		= {
		.name	= "dvbci",
		.of_match_table = dvbci_dev_dt_match,
		.owner	= THIS_MODULE,
	}
};

static int  aml_ci_mod_init(void)
{
	pr_dbg("mode init\n");
	return platform_driver_register(&aml_ci_driver);
}

static void  aml_ci_mod_exit(void)
{
	//pr_dbg("mode exit\n");
	platform_driver_unregister(&aml_ci_driver);
}

module_init(aml_ci_mod_init);
module_exit(aml_ci_mod_exit);
MODULE_LICENSE("GPL");

