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

static int UDC_exist_flag = -1;

static int meson_u2phy_crg_otg_freeze(struct meson_uphy_instance *instance)
{
	return 0;
}

static int meson_u2phy_crg_otg_thaw(struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)instance->meson_uphy;

	if (mphy->phy3_cfg)
		writel(mphy->otg_helper.pm_buf.usb_r1.d32, mphy->phy3_cfg + 4);
	writel(mphy->otg_helper.pm_buf.u2p_r2.d32,
			&mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r2);
	writel(mphy->otg_helper.pm_buf.u2p_r0.d32,
			&mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r0);

	/* Don't resync here because device_block_probing is still held.
	 * if hibernation fails,  PM_POST_HIBERNATION will handle.
	 */
	return 0;
}

static int meson_u2phy_crg_otg_restore(struct meson_uphy_instance *instance)
{
	return 0;
}

static int meson_u2phy_crg_otg_hibernation_prepare(struct amlogic_usb_v2 *mphy)
{
	union u2p_r2_v2 reg2;

	reg2.d32 = readl(&mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r2);
	mphy->otg_helper.pm_buf.u2p_r2.d32 = reg2.d32;
	/* Quiesce otg func: clear&disable hwirq, cancel works. */
	if (mphy->otg_helper.controller_type == USB_OTG) {
		reg2.b.usb_iddig_irq = 0;
		reg2.b.iddig_en0 = 0;
		reg2.b.iddig_en1 = 0;
		reg2.b.iddig_th = 0;
		writel(reg2.d32, &mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r2);
		cancel_delayed_work_sync(&mphy->otg_helper.work);
	}

	mphy->otg_helper.pm_buf.u2p_r0.d32 =
			readl(&mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r0);
	if (mphy->phy3_cfg)
		mphy->otg_helper.pm_buf.usb_r1.d32 = readl(mphy->phy3_cfg + 4);
	return 0;
}

static int meson_u2phy_crg_otg_post_hibernation(struct amlogic_usb_v2 *mphy)
{
	//union u2p_r2_v2 reg2;

	//reg2.d32 = readl((void __iomem *)((unsigned long)phy->usb2_phy_cfg + 8));
	//pr_err("line state: %d recover to %d\n",
	//	reg2.b.iddig_curr, phy->pm_buf.u2p_r2.b.iddig_curr);

	writel(mphy->otg_helper.pm_buf.usb_r1.d32, mphy->phy3_cfg + 4);
	writel(mphy->otg_helper.pm_buf.u2p_r2.d32,
			&mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r2);
	writel(mphy->otg_helper.pm_buf.u2p_r0.d32,
			&mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r0);

	/* During STD the idpin line state could lose sync with controller state even the
	 * register has been restored. This happens when line state changes after std but
	 * before h-prepare which disables the irq, records linesate then cancels the unscheduled
	 * role-switch irq work.
	 * e.g.
	 * If the otg plug is removed before h-prepare, the restored state remains as host
	 * even when the otg plug is still removed.
	 * Try to resync after restore.
	 */
	if (mphy->otg_helper.controller_type == USB_OTG)
		schedule_delayed_work(&mphy->otg_helper.work, msecs_to_jiffies(100));

	return 0;
}

/* crg_gadget_exit&crg_gadget_init should be synchronous to prevent racing with crg_udc_pm_cb.
 * However dpm_prepare defer all probes until dpm_complete, the probe inside is still asynchronous.
 * Avoid this by using pm_cb.
 */
static int meson_u2phy_crg_otg_pm_cb(struct notifier_block *notifier,
			      unsigned long pm_event,
			      void *unused)
{
	struct amlogic_usb_v2 *mphy =
		container_of(notifier, struct amlogic_usb_v2, otg_helper.pm_notifier);

	mu2p_dbg(mphy->dev, "%s called. pm_event:%lu.\n", __func__, pm_event);

	switch (pm_event) {
	case PM_HIBERNATION_PREPARE:
		meson_u2phy_crg_otg_hibernation_prepare(mphy);
		break;
	case PM_POST_HIBERNATION:
		meson_u2phy_crg_otg_post_hibernation(mphy);
		break;
	case PM_SUSPEND_PREPARE:
	case PM_POST_SUSPEND:
	case PM_RESTORE_PREPARE:
	case PM_POST_RESTORE:
	default:
		break;
	}
	return NOTIFY_DONE;
}

static int meson_u2phy_crg_otg_init(struct amlogic_usb_v2 *mphy)
{
	union usb_r1_v2 r1 = {.d32 = 0};
	union u2p_r2_v2 reg2 = {.d32 = 0};
	struct usb_aml_regs_v2 usb_crg_otg_aml_regs;

	if (mphy->phy3_cfg) {
		usb_crg_otg_aml_regs.usb_r_v2[1] = mphy->phy3_cfg + 4;

		r1.d32 = readl(usb_crg_otg_aml_regs.usb_r_v2[1]);
		r1.b.u3h_fladj_30mhz_reg = 0x20;
		writel(r1.d32, usb_crg_otg_aml_regs.usb_r_v2[1]);
	}

	reg2.d32 = readl(&mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r2);
	reg2.b.iddig_en0 = 1;
	reg2.b.iddig_en1 = 1;
	reg2.b.iddig_th = 255;
	writel(reg2.d32, &mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r2);

	return 0;
}

static int  meson_u2phy_crg_otg_mode_show(struct seq_file *s, void *unused)
{
	struct amlogic_usb_v2 *mphy = s->private;
	int cnt;
	char tmp[8] = {'\0'};
	char buf[16];

	if (mphy->otg_helper.mode & AML_USB_OTG_HOST_MODE_MASK)
		cnt = sprintf(tmp, "%s", "host");
	if (mphy->otg_helper.mode & AML_USB_OTG_DEVICE_MODE_MASK)
		cnt = sprintf(tmp, "%s", "device");
	if (mphy->otg_helper.mode & AML_USB_OTG_OTG_MODE_MASK)
		cnt = sprintf(buf, "%s-%s\n", "otg", tmp);
	else
		cnt = sprintf(buf, "%s-%s\n", "notg", tmp);

	seq_printf(s, "%s", buf);

	return 0;
}

static int meson_u2phy_crg_otg_mode_open(struct inode *inode, struct file *file)
{
	return single_open(file, meson_u2phy_crg_otg_mode_show, inode->i_private);
}

static ssize_t  meson_u2phy_crg_otg_mode_write(struct file *file,  const char __user *ubuf,
			       size_t count, loff_t *ppos)
{
	struct seq_file         *s = file->private_data;
	struct amlogic_usb_v2 *mphy = s->private;
	u8 i_mode = (u8)-1;
	int ret = -EINVAL;
	char                    buf[32];

	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return ret;

	buf[count - 1] = '\0';

	if (mphy->otg_helper.mode & AML_USB_OTG_OTG_MODE_MASK)
		return ret;

	mu2p_dbg(mphy->dev, "%s %lu", buf, (unsigned long)count);

	if (!strncmp("host", buf, count))
		i_mode = AML_USB_OTG_HOST_MODE;

	if (!strncmp("device", buf, count))
		i_mode = AML_USB_OTG_DEVICE_MODE;

	if (i_mode == (u8)-1 || mphy->otg_helper.mode & BIT(i_mode))
		return ret;

	if (i_mode == AML_USB_OTG_HOST_MODE) {
		crg_gadget_exit();
		meson_u2phy_set_mode(mphy, mphy->otg_helper.otg_port_index, PHY_MODE_USB_HOST);
		meson_u2phy_set_vbus_power(mphy, 1);
		crg_init();
		ret = count;
	} else if (i_mode == AML_USB_OTG_DEVICE_MODE) {
		crg_exit();
		meson_u2phy_set_mode(mphy, mphy->otg_helper.otg_port_index, PHY_MODE_USB_DEVICE);
		meson_u2phy_set_vbus_power(mphy, 0);
		crg_gadget_init();
		if (UDC_exist_flag != 1) {
			if (crg_udc_get_UDC_name()) {
				pr_err("%s.\n", crg_udc_get_UDC_name());
				ret = crg_otg_write_UDC(crg_udc_get_UDC_name());
				if (ret == 0 || ret == -EBUSY)
					UDC_exist_flag = 1;
			}
		}
		ret = count;
	}

	return ret;
}

static const struct file_operations  meson_u2phy_crg_mode_fops = {
	.open			= meson_u2phy_crg_otg_mode_open,
	.write          = meson_u2phy_crg_otg_mode_write,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};

void  meson_u2phy_crg_otg_debugfs_init(struct amlogic_usb_v2 *mphy)
{
	mphy->otg_helper.debugfs_root =
			debugfs_create_dir("crg-drd-otg", amlogic_usb_debugfs_root);

	debugfs_create_file("mode", 0644, mphy->otg_helper.debugfs_root,
							mphy, &meson_u2phy_crg_mode_fops);
}

//void amlogic_crg_otg_debugfs_exit(struct amlogic_crg_otg *phy)
//{
//	debugfs_remove_recursive(phy->debugfs_root);
//	phy->debugfs_root = NULL;
//}

static void meson_u2phy_crg_otg_work(struct work_struct *work)
{
	struct amlogic_usb_v2 *mphy = container_of(work, struct amlogic_usb_v2,
								otg_helper.set_mode_work.work);
	union u2p_r2_v2 reg2;
	int ret;
	bool curr;

	if (mphy->otg_helper.mode_work_flag == 1) {
		cancel_delayed_work_sync(&mphy->otg_helper.set_mode_work);
		mphy->otg_helper.mode_work_flag = 0;
	}
	mutex_lock(mphy->otg_helper.otg_mutex);
	reg2.d32 = readl(&mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r2);
	curr = reg2.b.iddig_curr;
	if (reg2.b.iddig_curr == 0) {
		crg_gadget_exit();
		meson_u2phy_set_mode(mphy, mphy->otg_helper.otg_port_index, PHY_MODE_USB_HOST);
		meson_u2phy_set_vbus_power(mphy, 1);
		crg_init();
	} else {
		crg_exit();
		meson_u2phy_set_mode(mphy, mphy->otg_helper.otg_port_index, PHY_MODE_USB_DEVICE);
		meson_u2phy_set_vbus_power(mphy, 0);
		crg_gadget_init();
		if (UDC_exist_flag != 1) {
			if (crg_udc_get_UDC_name()) {
				ret = crg_otg_write_UDC(crg_udc_get_UDC_name());
				if (ret == 0 || ret == -EBUSY)
					UDC_exist_flag = 1;
			}
		}
	}

	reg2.d32 = readl(&mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r2);
	reg2.b.usb_iddig_irq = 0;
	/* PHY has reg val reset feature may reset otg related bits
	 * to default. Restore reg bits we concern.
	 */
	reg2.b.iddig_curr = curr;
	writel(reg2.d32, &mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r2);
	meson_u2phy_crg_otg_init(mphy);

	mu2p_dbg(mphy->dev, "otg_work r0, r1, r2: 0x%x 0x%x 0x%x.\n",
			readl(&mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r0),
			readl(&mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r1),
			readl(&mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r2));

	mutex_unlock(mphy->otg_helper.otg_mutex);
}

static void meson_u2phy_crg_otg_set_m_work(struct work_struct *work)
{
	struct amlogic_usb_v2 *mphy = container_of(work, struct amlogic_usb_v2,
								otg_helper.set_mode_work.work);
	union u2p_r2_v2 reg2;

	mutex_lock(mphy->otg_helper.otg_mutex);
	mphy->otg_helper.mode_work_flag = 0;
	reg2.d32 = readl(&mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r2);
	if (reg2.b.iddig_curr == 0) {
		meson_u2phy_set_mode(mphy, mphy->otg_helper.otg_port_index, PHY_MODE_USB_HOST);
		meson_u2phy_set_vbus_power(mphy, 1);
		crg_init();
	} else {
		meson_u2phy_set_mode(mphy, mphy->otg_helper.otg_port_index, PHY_MODE_USB_DEVICE);
		meson_u2phy_set_vbus_power(mphy, 0);
		crg_gadget_init();
	}
	reg2.b.usb_iddig_irq = 0;
	writel(reg2.d32, &mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r2);
	mutex_unlock(mphy->otg_helper.otg_mutex);
}

static irqreturn_t meson_u2phy_crg_otg_detect_irq(int irq, void *dev)
{
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)dev;
	union u2p_r2_v2 reg2;

	reg2.d32 = readl(&mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r2);
	reg2.b.usb_iddig_irq = 0;
	writel(reg2.d32, &mphy->u2p_aml_regs[mphy->otg_helper.otg_port_index]->r2);

	schedule_delayed_work(&mphy->work, msecs_to_jiffies(100));

	return IRQ_HANDLED;
}

int meson_u2phy_crg_otg_parse(struct device *dev, struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2 *mphy = (struct amlogic_usb_v2 *)instance->meson_uphy;
	int irq;
	int retval;
	int otg = 0;

	retval = of_property_read_u32(dev->of_node, "controller-type",
								&mphy->otg_helper.controller_type);
	if (retval < 0)
		mphy->otg_helper.controller_type = USB_NORMAL;

	retval = of_property_read_u32(dev->of_node, "otg-port-index",
								&mphy->otg_helper.otg_port_index);
	if (retval < 0) {
		mu2p_err(dev, "no otg port index, use 0 as default.\n");
		mphy->otg_helper.otg_port_index = 0;
		return retval;
	}

	if (mphy->otg_helper.controller_type == USB_OTG)
		otg = 1;

	mu2p_info(dev, "controller_type is %d\n"
				"crg_force_device_mode is %d\n"
				"otg is %d\n",
				mphy->otg_helper.controller_type,
				get_otg_mode(), otg);

	mphy->otg_helper.mode_work_flag = 0;
	mphy->otg_helper.otg_mutex = kmalloc(sizeof(*mphy->otg_helper.otg_mutex),
				GFP_KERNEL);
	mutex_init(mphy->otg_helper.otg_mutex);

	INIT_DELAYED_WORK(&mphy->otg_helper.work, meson_u2phy_crg_otg_work);

	mphy->otg_helper.pm_notifier.notifier_call = meson_u2phy_crg_otg_pm_cb;
	register_pm_notifier(&mphy->otg_helper.pm_notifier);

	if (otg) {
		/* m31 */
//		val = readl(phy->m31_phy_cfg + 0x8);
//		val |= 1;
//		writel(val, phy->m31_phy_cfg + 0x8);

		irq = of_irq_get(dev->of_node, 0);
		if (irq < 0)
			return -ENODEV;
		retval = request_irq(irq, meson_u2phy_crg_otg_detect_irq,
				 IRQF_SHARED, "amlogic_botg_detect", mphy);

		if (retval) {
			mu2p_err(dev, "request of irq%d failed\n", irq);
			retval = -EBUSY;
			return retval;
		}
	}

	/* The otg driver firstly touches the phy reg. The reset maybe low at boot. */
	meson_u2phy_hold_reset(mphy, mphy->otg_helper.otg_port_index, true);

	if (otg == 0) {
		if (get_otg_mode() || mphy->otg_helper.controller_type == USB_DEVICE_ONLY) {
			meson_u2phy_set_mode(mphy, mphy->otg_helper.otg_port_index,
									PHY_MODE_USB_DEVICE);
			meson_u2phy_set_vbus_power(mphy, 0);
			crg_gadget_init();
		} else if (mphy->otg_helper.controller_type == USB_HOST_ONLY) {
			meson_u2phy_set_mode(mphy, mphy->otg_helper.otg_port_index,
									PHY_MODE_USB_HOST);
			meson_u2phy_set_vbus_power(mphy, 1);
			crg_init();
		}
	} else {
		meson_u2phy_crg_otg_init(mphy);
		meson_u2phy_set_mode(mphy, mphy->otg_helper.otg_port_index, PHY_MODE_USB_OTG);
		/* The usb2_phy_cfg iddig bit is default 0 and may not change to 1 instantly
		 * with otg connecter plugged at boot time. Defer the mode init here to avoid
		 * excess host mode init.
		 */
		mphy->otg_helper.mode_work_flag = 1;
		INIT_DELAYED_WORK(&mphy->otg_helper.set_mode_work, meson_u2phy_crg_otg_set_m_work);
		schedule_delayed_work(&mphy->otg_helper.set_mode_work, msecs_to_jiffies(500));
	}

	meson_u2phy_crg_otg_debugfs_init(mphy);

	instance->pm_ops.freeze = meson_u2phy_crg_otg_freeze;
	instance->pm_ops.thaw = meson_u2phy_crg_otg_thaw;
	instance->pm_ops.restore = meson_u2phy_crg_otg_restore;

	return 0;
}
