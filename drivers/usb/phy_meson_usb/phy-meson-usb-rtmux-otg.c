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
#include "phy-meson-usb.h"
#include "../usb_main.h"

BLOCKING_NOTIFIER_HEAD(meson_uphy_rtmux_otg_notifier_list);

int meson_uphy_rtmux_otg_register_notifier(struct notifier_block *nb)
{
	int ret;

	ret = blocking_notifier_chain_register
			(&meson_uphy_rtmux_otg_notifier_list, nb);

	return ret;
}
EXPORT_SYMBOL(meson_uphy_rtmux_otg_register_notifier);

int meson_uphy_rtmux_otg_unregister_notifier(struct notifier_block *nb)
{
	int ret;

	ret = blocking_notifier_chain_unregister
			(&meson_uphy_rtmux_otg_notifier_list, nb);

	return ret;
}
EXPORT_SYMBOL(meson_uphy_rtmux_otg_unregister_notifier);

static void meson_uphy_rtmux_otg_notifier_call(unsigned long is_device_on)
{
	blocking_notifier_call_chain
			(&meson_uphy_rtmux_otg_notifier_list, is_device_on, NULL);
}

static int meson_uphy_rtmux_otg_set_mode(struct amlogic_usb_v2 *phy, enum meson_uphy_mode mode,
											bool locked)
{
	int ret = 0;
	struct meson_uphy_instance *inst = dev_get_drvdata(phy->dev);
	struct phy *gphy = inst->phy;
	const struct meson_uphy_pdata *pdata = ((struct meson_uphy_pool *)
				dev_get_drvdata(gphy->dev.parent))->pdata;

	switch (mode) {
	case MESON_USB_MODE_HOST:
		meson_u2phy_set_vbus_power(phy, 1);
		meson_uphy_rtmux_otg_notifier_call(0);
		if (locked)
			pdata->u2phy_ops->set_mode(gphy, PHY_MODE_USB_HOST, 0);
		else
			phy_set_mode(gphy, PHY_MODE_USB_HOST);
		break;
	case MESON_USB_MODE_DEVICE:
		if (locked)
			pdata->u2phy_ops->set_mode(gphy, PHY_MODE_USB_DEVICE, 0);
		else
			phy_set_mode(gphy, PHY_MODE_USB_DEVICE);
		meson_uphy_rtmux_otg_notifier_call(1);
		meson_u2phy_set_vbus_power(phy, 0);
		break;
	case MESON_USB_MODE_OTG:
		if (locked)
			pdata->u2phy_ops->set_mode(gphy, PHY_MODE_USB_OTG, 0);
		else
			phy_set_mode(gphy, PHY_MODE_USB_OTG);
		break;
	default:
		mup_err(phy->dev, "%s: unknown phy mode.\n", __func__);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static void meson_uphy_rtmux_otg_work(struct work_struct *work)
{
	struct amlogic_usb_v2 *phy =
		container_of(work, struct amlogic_usb_v2, otg_helper.work.work);
	union usb_r5_v2 r5 = {.d32 = 0};
	union u2p_r2_v2 reg2;

	if (phy->otg_helper.iddig_type == 1) {
		r5.d32 = readl(&phy->usb_aml_regs->r5);
		if (r5.b.iddig_curr == 0)
			meson_uphy_rtmux_otg_set_mode(phy, MESON_USB_MODE_HOST, false);
		else
			meson_uphy_rtmux_otg_set_mode(phy, MESON_USB_MODE_DEVICE, false);

		r5.b.usb_iddig_irq = 0;
		writel(r5.d32, &phy->usb_aml_regs->r5);
	} else if (phy->otg_helper.iddig_type == 0) {
		reg2.d32 = readl(&phy->u2p_aml_regs[0]->r2);
		if (reg2.b.iddig_curr == 0)
			meson_uphy_rtmux_otg_set_mode(phy, MESON_USB_MODE_HOST, false);
		else
			meson_uphy_rtmux_otg_set_mode(phy, MESON_USB_MODE_DEVICE, false);

		reg2.b.usb_iddig_irq = 0;
		writel(reg2.d32, &phy->u2p_aml_regs[0]->r2);
	}
}

static irqreturn_t  meson_uphy_rtmux_otg_detect_irq(int irq, void *data)
{
	struct amlogic_usb_v2 *phy = (struct amlogic_usb_v2 *)data;
	union usb_r5_v2 r5 = {.d32 = 0};
	union u2p_r2_v2 reg2;

	if (phy->otg_helper.iddig_type == 1) {
		if (!phy->usb_aml_regs) {
			mup_err(phy->dev, "No u3 usb reg set. Double check the dts node?\n");
			return IRQ_HANDLED;
		}
		r5.d32 = readl(&phy->usb_aml_regs->r5);
			r5.b.usb_iddig_irq = 0;
		writel(r5.d32, &phy->usb_aml_regs->r5);
	} else if (phy->otg_helper.iddig_type == 0) {
		if (!phy->u2p_aml_regs[0]) {
			mup_err(phy->dev, "No u2 usb reg set. Double check the dts node?\n");
			return IRQ_HANDLED;
		}
		reg2.d32 = readl(&phy->u2p_aml_regs[0]->r2);
		reg2.b.usb_iddig_irq = 0;
		writel(reg2.d32, &phy->u2p_aml_regs[0]->r2);
	}

	schedule_delayed_work(&phy->otg_helper.work, msecs_to_jiffies(10));

	return IRQ_HANDLED;
}

static void meson_uphy_rtmux_id_gpio_work(struct work_struct *work)
{
	struct amlogic_usb_v2 *phy =
		container_of(work, struct amlogic_usb_v2, otg_helper.id_gpio_work.work);
	int id_gpio_status;

	id_gpio_status = gpiod_get_value(phy->otg_helper.idgpiodesc);

	if (id_gpio_status == 1)
		meson_uphy_rtmux_otg_set_mode(phy, MESON_USB_MODE_HOST, false);
	else
		meson_uphy_rtmux_otg_set_mode(phy, MESON_USB_MODE_DEVICE, false);
}

static struct device *meson_uphydev_to_sysdev(struct amlogic_usb_v2 *phy)
{
	struct device *sysdev;

	for (sysdev = phy->dev; sysdev; sysdev = sysdev->parent) {
		if (is_of_node(sysdev->fwnode))
			return sysdev;
	}

	return sysdev;
}

static irqreturn_t meson_uphy_rtmux_id_gpio_detect_irq(int irq, void *dev)
{
	struct amlogic_usb_v2 *phy = (struct amlogic_usb_v2 *)dev;

	schedule_delayed_work(&phy->otg_helper.id_gpio_work, msecs_to_jiffies(10));

	return IRQ_HANDLED;
}

static int meson_uphy_rtmux_id_pin_config(struct amlogic_usb_v2 *phy)
{
	int ret;
	struct gpio_desc *desc;
	struct device *sysdev = meson_uphydev_to_sysdev(phy);
	int id_irqnr;

	if (!sysdev)
		return -ENODEV;

	desc = gpiod_get_index(sysdev, NULL, 1, GPIOD_IN);
	if (IS_ERR(desc)) {
		mup_err(phy->dev, "fail to get id-gpioirq\n");
		return -ENODEV;
	}

	phy->otg_helper.idgpiodesc = desc;

	id_irqnr = gpiod_to_irq(desc);
	phy->otg_helper.irqline = id_irqnr;

	ret = request_irq(id_irqnr,
			       meson_uphy_rtmux_id_gpio_detect_irq,
			       ID_GPIO_IRQ_FLAGS,
			       "meson-usb-gpio-otg", phy);

	if (ret) {
		mup_err(phy->dev, "failed to request ret=%d, id_irqnr=%d\n",
		       ret, id_irqnr);
		return -ENODEV;
	}

	mup_dbg(phy->dev, "<%s> ok\n", __func__);

	return 0;
}

static void meson_uphy_rtmux_id_pin_deconfig(struct amlogic_usb_v2 *phy)
{
	free_irq(phy->otg_helper.irqline, phy);

	cancel_delayed_work_sync(&phy->otg_helper.id_gpio_work);

	if (phy->otg_helper.idgpiodesc)
		gpiod_put(phy->otg_helper.idgpiodesc);

	if (phy->otg_helper.irqline)
		gpiod_put(phy->otg_helper.idgpiodesc);

	mup_dbg(phy->dev, "<%s> ok\n", __func__);
}

void meson_uphy_rtmux_otg_complete(struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2 *phy = (struct amlogic_usb_v2 *)instance->meson_uphy;

	if (phy->current_mode == PHY_MODE_USB_HOST)
		resume_xhci_port_a();
	else if (phy->current_mode == PHY_MODE_USB_DEVICE)
		force_disable_xhci_port_a();
}

int meson_uphy_rtmux_otg_suspend(struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2 *phy = (struct amlogic_usb_v2 *)instance->meson_uphy;

	/* Let phy mode settle. */
	if (phy->otg_helper.hwotg && phy->otg_helper.hwotg_type == MESON_USB_OTG_T_PHY)
		flush_delayed_work(&phy->otg_helper.work);

	if (phy->otg_helper.hwotg && phy->otg_helper.hwotg_type == MESON_USB_OTG_T_GPIO)
		flush_delayed_work(&phy->otg_helper.id_gpio_work);

	if (phy->current_mode == PHY_MODE_USB_HOST)
		meson_u2phy_set_vbus_power(phy, 0);

	return 0;
}

int meson_uphy_rtmux_otg_resume(struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2 *phy = (struct amlogic_usb_v2 *)instance->meson_uphy;

	if (phy->current_mode == PHY_MODE_USB_HOST)
		meson_u2phy_set_vbus_power(phy, 1);

	return 0;
}

/* Phase 1. */
int meson_uphy_rtmux_otg_parse(struct device *dev, struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2 *phy = (struct amlogic_usb_v2 *)instance->meson_uphy;
	unsigned int xhci_port_a_mem;
	int ret;

	phy->otg_helper.otg_port = true;

	ret = of_property_read_u32(dev->of_node, "controller-type",
								&phy->otg_helper.controller_type);
	if (ret < 0)
		phy->otg_helper.controller_type = USB_NORMAL;

	if (phy->otg_helper.controller_type == USB_OTG)
		phy->otg_helper.hwotg = true;

	ret = of_property_read_u32
				(dev->of_node, "xhci-port-a-reg", &xhci_port_a_mem);
	if (ret < 0) {
		xhci_port_a_mem = 0;
		phy->xhci_port_a_addr = NULL;
	} else {
		phy->xhci_port_a_addr = ioremap((resource_size_t)xhci_port_a_mem,
					MESON_UPHY_XHCI_PORT_A_MEM_SIZE);
		if (!phy->xhci_port_a_addr)
			return -ENOMEM;
	}

	if (phy->otg_helper.hwotg) {
		ret = of_property_read_u32(dev->of_node, "otg-type", &phy->otg_helper.hwotg_type);
		if (ret < 0) {
			mup_err(dev, " Please set the otg type in the dts node.\n");
			return ret;
		}

		ret = of_property_read_u32(dev->of_node, "iddig-type", &phy->otg_helper.iddig_type);
		if (ret < 0) {
			mup_err(dev, " Please set the otg type in the dts node.\n");
			return ret;
		}

		if (phy->otg_helper.iddig_type == 1 && !phy->usb_aml_regs) {
			mup_err(dev, " Please set the usb_aml_regs in the dts node.\n");
			return -EINVAL;
		}
	}

	mup_dbg(dev, "controller_type %d\n"
			"force_device_mode %d\n"
			"otg %d type %d\n"
			"xhci_port_a addr 0x%x mem %px",
			phy->otg_helper.controller_type,
			get_otg_mode(), phy->otg_helper.hwotg, phy->otg_helper.hwotg_type,
			xhci_port_a_mem, phy->xhci_port_a_addr);

	instance->pm_ops.complete = meson_uphy_rtmux_otg_complete;
	instance->pm_ops.suspend = meson_uphy_rtmux_otg_suspend;
	instance->pm_ops.resume = meson_uphy_rtmux_otg_resume;
//	instance->pm_ops.thaw = meson_u2phy_crg_otg_thaw;
//	instance->pm_ops.restore = meson_u2phy_crg_otg_restore;
	return 0;
}

/* Phase 2. */
int meson_uphy_rtmux_otg_init(struct amlogic_usb_v2 *phy)
{
	struct device *dev = phy->dev;
	int irq;
	int ret = 0;

	if (!phy->otg_helper.otg_port) {
		mup_err(dev, "%s Not OTG port. Exit.\n", __func__);
		return -ENODEV;
	}

	/* The otg driver firstly touches the phy reg. The reset maybe low at boot. */
	//meson_u2phy_hold_reset(phy, true);
	if (phy->otg_helper.hwotg) {
		if (phy->otg_helper.hwotg_type == MESON_USB_OTG_T_GPIO) {
			ret = meson_uphy_rtmux_id_pin_config(phy);
			if (ret) {
				mup_err(dev, "request of gpio-irq failed\n");
				return ret;
			}
			meson_uphy_rtmux_otg_set_mode(phy, MESON_USB_MODE_OTG, true);
			INIT_DELAYED_WORK(&phy->otg_helper.id_gpio_work,
							meson_uphy_rtmux_id_gpio_work);
			if (gpiod_get_value(phy->otg_helper.idgpiodesc) == 1)
				meson_uphy_rtmux_otg_set_mode(phy, MESON_USB_MODE_DEVICE, true);
			else
				meson_uphy_rtmux_otg_set_mode(phy, MESON_USB_MODE_HOST, true);
		} else if (phy->otg_helper.hwotg_type == MESON_USB_OTG_T_PHY) {
			union usb_r5_v2 r5 = {.d32 = 0};
			union u2p_r2_v2 reg2;

			irq = of_irq_get(dev->of_node, 0);
			if (irq < 0) {
				mup_err(dev, " Cannot retrieve the irq prop.\n");
				return -ENODEV;
			}
			phy->otg_helper.irqline = irq;
			ret = request_irq(irq, meson_uphy_rtmux_otg_detect_irq,
					     IRQF_SHARED | IRQ_LEVEL,
					     "meson-usb-id-otg", phy);

			if (ret) {
				mup_err(dev, "request of irq%d failed\n",
					irq);
				ret = -EBUSY;
				return ret;
			}
			meson_uphy_rtmux_otg_set_mode(phy, MESON_USB_MODE_OTG, true);
			INIT_DELAYED_WORK(&phy->otg_helper.work, meson_uphy_rtmux_otg_work);
			if (phy->otg_helper.iddig_type == 1) {
				r5.d32 = readl(&phy->usb_aml_regs->r5);
				if (r5.b.iddig_curr == 0)
					meson_uphy_rtmux_otg_set_mode(phy,
						MESON_USB_MODE_HOST, true);
				else
					meson_uphy_rtmux_otg_set_mode(phy,
						MESON_USB_MODE_DEVICE, true);
			} else if (phy->otg_helper.iddig_type == 0) {
				reg2.d32 = readl(&phy->u2p_aml_regs[0]->r2);
				if (reg2.b.iddig_curr == 0)
					meson_uphy_rtmux_otg_set_mode(phy,
						MESON_USB_MODE_HOST, true);
				else
					meson_uphy_rtmux_otg_set_mode(phy,
						MESON_USB_MODE_DEVICE, true);
			}
		}  else {
			mup_err(dev, "otg type %d not supported\n", phy->otg_helper.hwotg_type);
		}
	} else {
		if (get_otg_mode() || phy->otg_helper.controller_type == USB_DEVICE_ONLY)
			meson_uphy_rtmux_otg_set_mode(phy, MESON_USB_MODE_DEVICE, true);
		else if (phy->otg_helper.controller_type == USB_HOST_ONLY)
			meson_uphy_rtmux_otg_set_mode(phy, MESON_USB_MODE_HOST, true);
	}

	return ret;
}

void meson_uphy_rtmux_otg_remove(struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2 *phy = (struct amlogic_usb_v2 *)instance->meson_uphy;

	dev_dbg(phy->dev, "%s phy ist %d.\n", __func__, instance->index);

	if (!phy) {
		pr_err("%s non meson_uphy!", __func__);
		return;
	}

	if (!phy->otg_helper.otg_port) {
		dev_dbg(phy->dev, "%s not meson_uphy, skip.\n", __func__);
		return;
	}

	dev_dbg(phy->dev, "%s phy dev ptr: %px.\n", __func__, phy->dev);

	if (phy->xhci_port_a_addr)
		iounmap(phy->xhci_port_a_addr);

	if (phy->otg_helper.hwotg) {
		if (phy->otg_helper.hwotg_type == MESON_USB_OTG_T_GPIO) {
			meson_uphy_rtmux_id_pin_deconfig(phy);
		} else if (phy->otg_helper.hwotg_type == MESON_USB_OTG_T_PHY) {
			free_irq(phy->otg_helper.irqline, phy);
			cancel_delayed_work_sync
				(&phy->otg_helper.work);
		}
	}

	/* Add this once the role switch is added. */
//#if IS_ENABLED(CONFIG_USB_ROLE_SWITCH)
//		if (phy->role_sw)
//			usb_role_switch_unregister(phy->role_sw);
//#endif
	meson_u2phy_set_vbus_power(phy, 0);
}

