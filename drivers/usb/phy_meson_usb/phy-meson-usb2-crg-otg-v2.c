// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/irqreturn.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/amlogic/usb-v2.h>
#include <linux/amlogic/aml_gpio_consumer.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/suspend.h>
#include <linux/amlogic/usbtype.h>
#include <linux/usb/role.h>

#include "phy-meson-usb.h"
#include "../usb_main.h"

static int UDC_v2_exist_flag = -1;

static int meson_u2phy_crg_otg_v2_init(struct amlogic_usb_v2 *phy)
{
	union usb_r1_v2 r1 = {.d32 = 0};
	union usb_r5_v2 r5 = {.d32 = 0};

	if (WARN_ON(!phy->usb_aml_regs))
		return -ENOMEM;

	r1.d32 = readl(&phy->usb_aml_regs->r1);
	r1.b.u3h_fladj_30mhz_reg = 0x20;
	writel(r1.d32, &phy->usb_aml_regs->r1);

	r5.d32 = readl(&phy->usb_aml_regs->r5);
	r5.b.iddig_en0 = 1;
	r5.b.iddig_en1 = 1;
	r5.b.iddig_th = 255;
	writel(r5.d32, &phy->usb_aml_regs->r5);

	return 0;
}

#if IS_ENABLED(CONFIG_USB_ROLE_SWITCH)
static int meson_u2phy_crg_otg_v2_role_switch_set(struct usb_role_switch *sw,
				    enum usb_role role)
{
	struct amlogic_usb_v2	*mphy = usb_role_switch_get_drvdata(sw);
	u32 mode;
	int ret;

	dev_dbg(mphy->dev, " MODE=%u, %s\n", role, __func__);

	switch (role) {
	case USB_ROLE_HOST:
		mode = USB_HOST_ONLY;
		break;
	case USB_ROLE_DEVICE:
		mode = USB_DEVICE_ONLY;
		break;
	default:
		if (mphy->role_switch_default_mode == USB_HOST_ONLY)
			mode = USB_HOST_ONLY;
		else
			mode = USB_DEVICE_ONLY;
		break;
	}

	if (mode == USB_HOST_ONLY) {
		crg_gadget_exit();
		crg_init();
	} else {
		crg_exit();
		crg_gadget_init();
		if (UDC_v2_exist_flag != 1) {
			if (crg_udc_get_UDC_name()) {
				ret = crg_otg_write_UDC(crg_udc_get_UDC_name());
				if (ret == 0 || ret == -EBUSY)
					UDC_v2_exist_flag = 1;
			}
		}
	}

	return 0;
}

static enum usb_role meson_u2phy_crg_otg_v2_role_switch_get(struct usb_role_switch *sw)
{
	struct amlogic_usb_v2	*mphy = usb_role_switch_get_drvdata(sw);
	enum usb_role role;

	switch (mphy->current_mode) {
	case MESON_USB_MODE_HOST:
		role = USB_ROLE_HOST;
		break;
	case MESON_USB_MODE_DEVICE:
		role = USB_ROLE_DEVICE;
		break;
	//case PHY_MODE_USB_OTG:
		//role = USB_OTG;
		//break;
	default:
		if (mphy->role_switch_default_mode == USB_HOST_ONLY)
			role = USB_ROLE_HOST;
		else
			role = USB_ROLE_DEVICE;
		break;
	}
	return role;
}

static int meson_u2phy_crg_otg_v2_setup_role_switch(struct amlogic_usb_v2 *mphy)
{
	struct usb_role_switch_desc meson_role_switch = {NULL};
	int ret = 0;

	meson_role_switch.fwnode = dev_fwnode(mphy->dev->parent);
	meson_role_switch.allow_userspace_control = true;
	meson_role_switch.set = meson_u2phy_crg_otg_v2_role_switch_set;
	meson_role_switch.get = meson_u2phy_crg_otg_v2_role_switch_get;
	meson_role_switch.name = "drd";
	meson_role_switch.driver_data = mphy;
	mphy->role_sw = usb_role_switch_register(mphy->dev->parent, &meson_role_switch);
	if (IS_ERR(mphy->role_sw)) {
		ret = PTR_ERR(mphy->role_sw);
		mphy->role_sw = NULL;
	}

	return ret;
}
#endif

static int meson_u2phy_crg_otg_v2_set_mode(struct amlogic_usb_v2 *mphy, enum meson_uphy_mode mode)
{
	int ret = 0;
	struct meson_uphy_instance *inst = dev_get_drvdata(mphy->dev);
	const struct meson_uphy_pdata *pdata = inst->domain_pool->pdata;

	switch (mode) {
	case MESON_USB_MODE_HOST:
#if (IS_ENABLED(CONFIG_USB_ROLE_SWITCH))
		ret = usb_role_switch_set_role(mphy->role_sw, USB_ROLE_HOST);
#else
		crg_gadget_exit();
		crg_init();
#endif
		break;
	case MESON_USB_MODE_DEVICE:
#if (IS_ENABLED(CONFIG_USB_ROLE_SWITCH))
		ret = usb_role_switch_set_role(mphy->role_sw, USB_ROLE_DEVICE);
#else
		crg_exit();
		crg_gadget_init();
		if (UDC_v2_exist_flag != 1) {
			if (crg_udc_get_UDC_name()) {
				ret = crg_otg_write_UDC(crg_udc_get_UDC_name());
				if (ret == 0 || ret == -EBUSY)
					UDC_v2_exist_flag = 1;
			}
		}
#endif
		break;
	case MESON_USB_MODE_OTG:
		meson_u2phy_crg_otg_v2_init(mphy);
		ret = pdata->u2phy_ops->set_mode(mphy, MESON_USB_MODE_OTG);
		break;
	default:
		dev_err(mphy->dev, "%s: unknown phy mode.\n", __func__);
		ret = -EINVAL;
		break;
	}

	if (ret)
		dev_err(mphy->dev, "failed to set %s %d ret %d\n",
			IS_ENABLED(CONFIG_USB_ROLE_SWITCH) ? "role" : "mode",
			mode, ret);

	return ret;
}

static void meson_u2phy_crg_otg_v2_work(struct work_struct *work)
{
	struct amlogic_usb_v2 *mphy = container_of(work, struct amlogic_usb_v2,
								otg_helper.work.work);
	union usb_r5_v2 reg5;

	if (mphy->otg_helper.mode_work_flag == 1) {
		cancel_delayed_work_sync(&mphy->otg_helper.set_mode_work);
		mphy->otg_helper.mode_work_flag = 0;
	}
	mutex_lock(mphy->otg_helper.otg_mutex);
	reg5.d32 = readl(&mphy->usb_aml_regs->r5);
	if (reg5.b.iddig_curr == 0)
		meson_u2phy_crg_otg_v2_set_mode(mphy, MESON_USB_MODE_HOST);
	else
		meson_u2phy_crg_otg_v2_set_mode(mphy, MESON_USB_MODE_DEVICE);

	reg5.b.usb_iddig_irq = 0;
	writel(reg5.d32, &mphy->usb_aml_regs->r5);

	dev_dbg(mphy->dev, "otg_work reg5: 0x%x.\n",
			readl(&mphy->usb_aml_regs->r5));

	mutex_unlock(mphy->otg_helper.otg_mutex);
}

static void meson_u2phy_crg_otg_v2_set_m_work(struct work_struct *work)
{
	struct amlogic_usb_v2 *mphy = container_of(work, struct amlogic_usb_v2,
								otg_helper.set_mode_work.work);
	union usb_r5_v2 reg5;

	mutex_lock(mphy->otg_helper.otg_mutex);
	mphy->otg_helper.mode_work_flag = 0;
	reg5.d32 = readl(&mphy->usb_aml_regs->r5);

	if (reg5.b.iddig_curr == 0)
		meson_u2phy_crg_otg_v2_set_mode(mphy, MESON_USB_MODE_HOST);
	else
		meson_u2phy_crg_otg_v2_set_mode(mphy, MESON_USB_MODE_DEVICE);

	reg5.b.usb_iddig_irq = 0;
	writel(reg5.d32, &mphy->usb_aml_regs->r5);
	mutex_unlock(mphy->otg_helper.otg_mutex);
}

static irqreturn_t meson_u2phy_crg_otg_v2_detect_irq(int irq, void *dev)
{
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)dev;
	union usb_r5_v2 reg5;

	reg5.d32 = readl(&mphy->usb_aml_regs->r5);
	reg5.b.usb_iddig_irq = 0;
	writel(reg5.d32, &mphy->usb_aml_regs->r5);

	schedule_delayed_work(&mphy->otg_helper.work, msecs_to_jiffies(10));

	return IRQ_HANDLED;
}

static int meson_u2phy_crg_otg_v2_regulator(struct device *dev, struct amlogic_usb_v2 *phy)
{
	int retval = 0;

	if (of_property_read_bool(dev->of_node, "regulator")) {
		phy->otg_helper.usb_regulator_ao1v8 = devm_regulator_get(dev, "usb1v8");
		retval = PTR_ERR_OR_ZERO(phy->otg_helper.usb_regulator_ao1v8);
		if (retval) {
			if (retval == -EPROBE_DEFER) {
				dev_err(dev, "regulator usb1v8 not ready, retry\n");
				return retval;
			}
			dev_err(dev, "failed in regulator usb1v8 %ld\n",
				PTR_ERR(phy->otg_helper.usb_regulator_ao1v8));
			phy->otg_helper.usb_regulator_ao1v8 = NULL;
		} else {
			retval = regulator_enable(phy->otg_helper.usb_regulator_ao1v8);
			if (retval) {
				dev_err(dev,
					"regulator usb1v8 enable failed:%d\n", retval);
				phy->otg_helper.usb_regulator_ao1v8 = NULL;
			}
		}

		phy->otg_helper.usb_regulator_ao3v3 = devm_regulator_get(dev, "usb3v3");
		retval = PTR_ERR_OR_ZERO(phy->otg_helper.usb_regulator_ao3v3);
		if (retval) {
			if (retval == -EPROBE_DEFER) {
				dev_err(dev, "regulator usb3v3 not ready, retry\n");
				return retval;
			}
			dev_err(dev, "failed in regulator usb3v3 %ld\n",
				PTR_ERR(phy->otg_helper.usb_regulator_ao3v3));
			phy->otg_helper.usb_regulator_ao3v3 = NULL;
		} else {
			retval = regulator_enable(phy->otg_helper.usb_regulator_ao3v3);
			if (retval) {
				dev_err(dev,
					"regulator usb3v3 enable failed:   %d\n", retval);
				phy->otg_helper.usb_regulator_ao3v3 = NULL;
			}
		}

		phy->otg_helper.usb_regulator_vcc5v = devm_regulator_get(dev, "usb5v");
		retval = PTR_ERR_OR_ZERO(phy->otg_helper.usb_regulator_vcc5v);
		if (retval) {
			if (retval == -EPROBE_DEFER) {
				dev_err(dev, "regulator usb5v not ready, retry\n");
				return retval;
			}
			dev_err(dev, "failed in regulator usb5v %ld\n",
				PTR_ERR(phy->otg_helper.usb_regulator_vcc5v));
			phy->otg_helper.usb_regulator_vcc5v = NULL;
		} else {
			retval = regulator_enable(phy->otg_helper.usb_regulator_vcc5v);
			if (retval) {
				dev_err(dev,
					"regulator usb5v enable failed:   %d\n", retval);
				phy->otg_helper.usb_regulator_vcc5v = NULL;
			}
		}
		dev_dbg(phy->dev, "usb1v8 %d usb3v3 %d usb5v %d",
			regulator_is_enabled(phy->otg_helper.usb_regulator_ao1v8),
			regulator_is_enabled(phy->otg_helper.usb_regulator_ao3v3),
			regulator_is_enabled(phy->otg_helper.usb_regulator_vcc5v));
	} else {
		phy->otg_helper.usb_regulator_ao1v8 = NULL;
		phy->otg_helper.usb_regulator_ao3v3 = NULL;
		phy->otg_helper.usb_regulator_vcc5v = NULL;
	}

	return retval;
}

static int meson_u2phy_crg_otg_v2_suspend(struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)instance->meson_uphy;

	if (mphy->otg_helper.usb_regulator_vcc5v)
		regulator_disable(mphy->otg_helper.usb_regulator_vcc5v);
	if (mphy->otg_helper.usb_regulator_ao3v3)
		regulator_disable(mphy->otg_helper.usb_regulator_ao3v3);
	if (mphy->otg_helper.usb_regulator_ao1v8)
		regulator_disable(mphy->otg_helper.usb_regulator_ao1v8);

	return 0;
}

static int meson_u2phy_crg_otg_v2_resume(struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)instance->meson_uphy;
	int ret = 0;

	if (mphy->otg_helper.usb_regulator_vcc5v)
		ret = regulator_enable(mphy->otg_helper.usb_regulator_vcc5v);
	if (mphy->otg_helper.usb_regulator_ao3v3)
		ret = regulator_enable(mphy->otg_helper.usb_regulator_ao3v3);
	if (mphy->otg_helper.usb_regulator_ao1v8)
		ret = regulator_enable(mphy->otg_helper.usb_regulator_ao1v8);

	return ret;
}

int meson_u2phy_crg_otg_v2_parse(struct device *dev, struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)instance->meson_uphy;
	int irq;
	int retval;

	mphy->otg_helper.otg_port = true;

	retval = of_property_read_u32(dev->of_node, "controller-type",
								&mphy->otg_helper.controller_type);
	if (retval < 0)
		mphy->otg_helper.controller_type = USB_NORMAL;

	retval = 0;

	if (mphy->otg_helper.controller_type == USB_OTG)
		mphy->otg_helper.hwotg = true;

	dev_dbg(dev, "controller_type is %d\n"
				"crg_force_device_mode is %d\n"
				"otg is %d\n",
				mphy->otg_helper.controller_type,
				get_otg_mode(), mphy->otg_helper.hwotg);

	mphy->otg_helper.mode_work_flag = 0;
	mphy->otg_helper.otg_mutex = kzalloc(sizeof(*mphy->otg_helper.otg_mutex),
				GFP_KERNEL);
	if (!mphy->otg_helper.otg_mutex)
		return -ENOMEM;

	mutex_init(mphy->otg_helper.otg_mutex);

	INIT_DELAYED_WORK(&mphy->otg_helper.work, meson_u2phy_crg_otg_v2_work);

	retval = meson_u2phy_crg_otg_v2_regulator(dev, mphy);
	if (retval)
		return retval;

	if (mphy->otg_helper.hwotg) {
		irq = of_irq_get(dev->of_node, 0);
		if (irq < 0) {
			retval = -ENODEV;
			goto err_irq;
		}
		mphy->otg_helper.irqline = irq;

		retval = request_irq(irq, meson_u2phy_crg_otg_v2_detect_irq,
				 IRQF_SHARED, "amlogic_botg_detect", mphy);
		if (retval) {
			dev_err(dev, "request of irq%d failed\n", irq);
			retval = -EBUSY;
			goto err_irq;
		}
	}

#if IS_ENABLED(CONFIG_USB_ROLE_SWITCH)
	if (get_otg_mode())
		mphy->role_switch_default_mode = USB_DEVICE_ONLY;
	else
		mphy->role_switch_default_mode = USB_HOST_ONLY;
	if (ROLE_SWITCH_FLAG) {
		retval = meson_u2phy_crg_otg_v2_setup_role_switch(mphy);
		if (retval) {
			dev_err(dev, "setup role switch failed\n");
			return retval;
		}
	}
#endif

	if (mphy->otg_helper.hwotg == 0) {
		if (get_otg_mode() || mphy->otg_helper.controller_type == USB_DEVICE_ONLY) {
#if IS_ENABLED(CONFIG_USB_ROLE_SWITCH)
			mphy->role_switch_default_mode = USB_DEVICE_ONLY;
#endif
			meson_u2phy_crg_otg_v2_set_mode(mphy, MESON_USB_MODE_DEVICE);
		} else if (mphy->otg_helper.controller_type == USB_HOST_ONLY) {
#if IS_ENABLED(CONFIG_USB_ROLE_SWITCH)
			mphy->role_switch_default_mode = USB_HOST_ONLY;
#endif
			meson_u2phy_crg_otg_v2_set_mode(mphy, MESON_USB_MODE_HOST);
		}
	} else {
		/* The hw otg configuration is slightly different. The otg helper must firstly
		 * configure the phy reg to enable otg irq but the otg config in the reg maybe
		 * reset during phy init called by controller driver afterwards so init the phy
		 * here then do the otg stuffs.
		 */
		retval = instance->phy->ops->init(instance->phy);
		if (retval) {
			dev_err(dev, "init phy failed at %s.\n", __func__);
			return retval;
		}

		meson_u2phy_crg_otg_v2_set_mode(mphy, MESON_USB_MODE_OTG);

		/* The usb2_phy_cfg iddig bit is default 0 and may not change to 1 instantly
		 * with otg connecter plugged at boot time. Defer the mode init here to avoid
		 * excess host mode init.
		 */
		mphy->otg_helper.mode_work_flag = 1;
		INIT_DELAYED_WORK(&mphy->otg_helper.set_mode_work,
					meson_u2phy_crg_otg_v2_set_m_work);
		schedule_delayed_work(&mphy->otg_helper.set_mode_work, msecs_to_jiffies(1000));

		dev_dbg(mphy->dev, "otg r5 0x%x.\n", readl(&mphy->usb_aml_regs->r5));
	}

	instance->pm_ops.suspend = meson_u2phy_crg_otg_v2_suspend;
	instance->pm_ops.resume = meson_u2phy_crg_otg_v2_resume;
	/* todo: PHY has comb reset feature may reset otg related bits
	 * to default at suspend. Restore reg bits we concern at resume.
	 */

	return retval;

err_irq:
	if (mphy->otg_helper.usb_regulator_vcc5v)
		regulator_disable(mphy->otg_helper.usb_regulator_vcc5v);
	if (mphy->otg_helper.usb_regulator_ao3v3)
		regulator_disable(mphy->otg_helper.usb_regulator_ao3v3);
	if (mphy->otg_helper.usb_regulator_ao1v8)
		regulator_disable(mphy->otg_helper.usb_regulator_ao1v8);

	return retval;
}

void meson_u2phy_crg_otg_v2_remove(struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)instance->meson_uphy;

	dev_dbg(mphy->dev, "%s entered.\n", __func__);

	if (!mphy) {
		pr_err("%s non meson_uphy!", __func__);
		return;
	}

	if (!mphy->otg_helper.otg_port) {
		dev_dbg(mphy->dev, "%s not meson_uphy, skip.\n", __func__);
		return;
	}

	dev_dbg(mphy->dev, "%s phy dev ptr: %px.\n", __func__, mphy->dev);

	if (mphy->otg_helper.irqline)
		free_irq(mphy->otg_helper.irqline, mphy);

#if IS_ENABLED(CONFIG_USB_ROLE_SWITCH)
	if (mphy->role_sw)
		usb_role_switch_unregister(mphy->role_sw);
#endif

	if (mphy->otg_helper.set_mode_work.work.func)
		cancel_delayed_work_sync(&mphy->otg_helper.set_mode_work);
	if (mphy->otg_helper.work.work.func)
		cancel_delayed_work_sync(&mphy->otg_helper.work);

	kfree(mphy->otg_helper.otg_mutex);

	if (mphy->current_mode == MESON_USB_MODE_HOST)
		crg_exit();

	if (mphy->current_mode == MESON_USB_MODE_DEVICE)
		crg_gadget_exit();

	if (mphy->otg_helper.usb_regulator_vcc5v &&
			regulator_is_enabled(mphy->otg_helper.usb_regulator_vcc5v))
		regulator_disable(mphy->otg_helper.usb_regulator_vcc5v);
	if (mphy->otg_helper.usb_regulator_ao3v3 &&
			regulator_is_enabled(mphy->otg_helper.usb_regulator_ao3v3))
		regulator_disable(mphy->otg_helper.usb_regulator_ao3v3);
	if (mphy->otg_helper.usb_regulator_ao1v8 &&
			regulator_is_enabled(mphy->otg_helper.usb_regulator_ao1v8))
		regulator_disable(mphy->otg_helper.usb_regulator_ao1v8);
}

