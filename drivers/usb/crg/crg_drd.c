// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/acpi.h>
#include <linux/pinctrl/consumer.h>
#include <linux/reset.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/of.h>
#include <linux/usb/otg.h>
#include <linux/amlogic/usbtype.h>
#include <linux/clk.h>
#include <linux/phy/phy.h>
#include <linux/amlogic/aml_gpio_consumer.h>
#include <linux/android_kabi.h>
#include <linux/amlogic/usb-v2.h>

#include <linux/kthread.h>
#include <linux/amlogic/cpu_version.h>

#include "../xhci_amlogic/xhci-meson.h"
#include "../xhci_amlogic/xhci-plat-meson.h"

static struct aml_xhci_plat_priv crg_xhci_plat_priv = {
	.quirks = XHCI_NO_64BIT_SUPPORT | XHCI_RESET_ON_RESUME | XHCI_SKIP_PHY_INIT,
};

#define CRG_DEFAULT_AUTOSUSPEND_DELAY	5000 /* ms */
#define CRG_XHCI_RESOURCES_NUM	2
#define CRG_GCTL_PRTCAP_HOST	1
#define CRG_GCTL_PRTCAP_DEVICE	2
#define CRG_GCTL_PRTCAP_OTG	3
#define CRG_XHCI_REGS_END		0x7fff
#define	CRG_U3DC_CFG0_MAXSPEED_SS		(0x4L << 0)

#define CRGDRD_USB2_MAX_PORTS	4
#define CRGDRD_USB3_MAX_PORTS	4

struct crg_drd {
	struct device		*dev;

	struct platform_device	*xhci;
	struct resource		xhci_resources[CRG_XHCI_RESOURCES_NUM];

	void __iomem		*regs;
	size_t			regs_size;
	enum usb_dr_mode	dr_mode;
	u32			maximum_speed;
	void			*mem;
	struct usb_phy		*usb2_phy;
	struct usb_phy		*usb3_phy;
	struct phy		*usb2_generic_phy[CRGDRD_USB2_MAX_PORTS];
	struct phy		*usb3_generic_phy[CRGDRD_USB3_MAX_PORTS];
	int num_usb2_ports;
	int num_usb3_ports;
	struct reset_control *reset;

	unsigned		super_speed_support:1;
	bool		usb_phy_init;
	bool		usb_suspend;
	struct clk		*general_clk;
};

/* -------------------------------------------------------------------------- */
static void crg_set_mode(struct crg_drd *crg, u32 mode)
{
	u64 tmp;

	if (mode == USB_DR_MODE_HOST) {
		/* set controller host role*/
		tmp = readl(crg->regs + 0x20FC) & ~0x1;
		writel(tmp, crg->regs + 0x20FC);
	}
}

static int crg_reset_line_assert(struct crg_drd *crg, bool on)
{
	int ret;

	if (on) {
		ret = reset_control_assert(crg->reset);
		if (ret)
			dev_err(crg->dev, "%s %d ret %d.\n", __func__, on, ret);
	} else {
		ret = reset_control_deassert(crg->reset);
		if (ret)
			dev_err(crg->dev, "%s %d ret %d.\n", __func__, on, ret);
	}

	return ret;
}

static int crg_core_get_reset(struct crg_drd *crg)
{
	crg->reset = of_reset_control_get_exclusive(crg->dev->of_node, "udrd");
	if (IS_ERR(crg->reset)) {
		dev_dbg(crg->dev, "failed to get reset: %ld\n", PTR_ERR(crg->reset));
		crg->reset = NULL;
	}

	return 0;
}

static int crg_core_put_reset(struct crg_drd *crg)
{
	if (crg->reset)
		reset_control_put(crg->reset);

	return 0;
}

static int crg_core_power(struct crg_drd *crg, bool on)
{
	dev_dbg(crg->dev, "%s %d.\n", __func__, on);
	return crg_reset_line_assert(crg, !on);
}

static int crg_get_num_ports(struct crg_drd *crg)
{
	void __iomem *base;
	u8 major_revision;
	u32 offset;
	u32 val;

	/*
	 * Remap xHCI address space to access XHCI ext cap regs since it is
	 * needed to get information on number of ports present.
	 */
	base = crg->regs;

	if (!base)
		return -ENOMEM;

	offset = 0;
	do {
		offset = xhci_find_next_ext_cap(base, offset,
						XHCI_EXT_CAPS_PROTOCOL);
		if (!offset)
			break;

		val = readl(base + offset);
		major_revision = XHCI_EXT_PORT_MAJOR(val);

		val = readl(base + offset + 0x08);
		if (major_revision == 0x03) {
			crg->num_usb3_ports += XHCI_EXT_PORT_COUNT(val);
		} else if (major_revision <= 0x02) {
			crg->num_usb2_ports += XHCI_EXT_PORT_COUNT(val);
		} else {
			dev_warn(crg->dev, "unrecognized port major revision %d\n",
				 major_revision);
		}
	} while (1);

	dev_dbg(crg->dev, "hs-ports: %u ss-ports: %u\n",
		crg->num_usb2_ports, crg->num_usb3_ports);

	if (crg->num_usb2_ports > CRGDRD_USB2_MAX_PORTS ||
	    crg->num_usb3_ports > CRGDRD_USB3_MAX_PORTS) {
		dev_warn(crg->dev, "num_usb2_ports %d num_usb3_ports %d too large\n",
			 crg->num_usb2_ports, crg->num_usb3_ports);
		return -EINVAL;
	}

	return 0;
}

static int crg_core_get_phy(struct crg_drd *crg)
{
	struct device *dev = crg->dev;
	int i, ret;
	char phy_name[9];

	crg->usb2_phy = devm_usb_get_phy_by_phandle(dev, "usb-phy", 0);
	if (IS_ERR(crg->usb2_phy))
		crg->usb2_phy = NULL;

	crg->usb3_phy = devm_usb_get_phy_by_phandle(dev, "usb-phy", 1);
	if (IS_ERR(crg->usb3_phy))
		crg->usb3_phy = NULL;
	else if (crg->usb3_phy->flags == AML_USB3_PHY_ENABLE)
		crg->super_speed_support = true;

	if (crg->usb2_phy)
		return 0;

	for (i = 0; i < crg->num_usb2_ports; i++) {
		if (crg->num_usb2_ports == 1)
			snprintf(phy_name, sizeof(phy_name), "usb2-phy");
		else
			snprintf(phy_name, sizeof(phy_name),  "usb2-%u", i);

		crg->usb2_generic_phy[i] = phy_get(dev, phy_name);
		if (IS_ERR(crg->usb2_generic_phy[i])) {
			ret = PTR_ERR(crg->usb2_generic_phy[i]);
			if (ret == -ENODEV) {
				crg->usb2_generic_phy[i] = NULL;
				dev_err(dev, "no u2phy%d.\n", i);
			} else {
				return dev_err_probe(dev, ret, "failed to lookup phy %s\n",
							phy_name);
			}
		}
	}

	for (i = 0; i < crg->num_usb3_ports; i++) {
		if (crg->num_usb3_ports == 1)
			snprintf(phy_name, sizeof(phy_name), "usb3-phy");
		else
			snprintf(phy_name, sizeof(phy_name), "usb3-%u", i);

		crg->usb3_generic_phy[i] = phy_get(dev, phy_name);
		if (IS_ERR(crg->usb3_generic_phy[i])) {
			ret = PTR_ERR(crg->usb3_generic_phy[i]);
			if (ret == -ENODEV) {
				crg->usb3_generic_phy[i] = NULL;
				dev_err(dev, "no u3phy%d.\n", i);
			} else {
				return dev_err_probe(dev, ret, "failed to lookup phy %s\n",
							phy_name);
			}
		}
		if (crg->usb3_generic_phy[i])
			crg->super_speed_support = true;
	}

	return 0;
}

static int crg_phy_init(struct crg_drd *crg)
{
	int ret;
	int i;
	int j;

	usb_phy_init(crg->usb2_phy);
	usb_phy_init(crg->usb3_phy);

	for (i = 0; i < crg->num_usb2_ports; i++) {
		ret = phy_init(crg->usb2_generic_phy[i]);
		if (ret < 0)
			goto err_exit_usb2_phy;
	}

	for (j = 0; j < crg->num_usb3_ports; j++) {
		ret = phy_init(crg->usb3_generic_phy[j]);
		if (ret < 0)
			goto err_exit_usb3_phy;
	}

	return 0;

err_exit_usb3_phy:
	while (--j >= 0)
		phy_exit(crg->usb3_generic_phy[j]);

err_exit_usb2_phy:
	while (--i >= 0)
		phy_exit(crg->usb2_generic_phy[i]);

	usb_phy_shutdown(crg->usb3_phy);
	usb_phy_shutdown(crg->usb2_phy);

	return ret;
}

static void crg_phy_exit(struct crg_drd *crg)
{
	int i;

	for (i = 0; i < crg->num_usb3_ports; i++)
		phy_exit(crg->usb3_generic_phy[i]);

	for (i = 0; i < crg->num_usb2_ports; i++)
		phy_exit(crg->usb2_generic_phy[i]);

	usb_phy_shutdown(crg->usb3_phy);
	usb_phy_shutdown(crg->usb2_phy);
}

static int crg_phy_power_on(struct crg_drd *crg)
{
	int ret;
	int i;
	int j;

	usb_phy_set_suspend(crg->usb2_phy, 0);
	usb_phy_set_suspend(crg->usb3_phy, 0);

	for (i = 0; i < crg->num_usb2_ports; i++) {
		ret = phy_power_on(crg->usb2_generic_phy[i]);
		if (ret < 0)
			goto err_power_off_usb2_phy;
	}

	for (j = 0; j < crg->num_usb3_ports; j++) {
		ret = phy_power_on(crg->usb3_generic_phy[j]);
		if (ret < 0)
			goto err_power_off_usb3_phy;
	}

	return 0;

err_power_off_usb3_phy:
	while (--j >= 0)
		phy_power_off(crg->usb3_generic_phy[j]);

err_power_off_usb2_phy:
	while (--i >= 0)
		phy_power_off(crg->usb2_generic_phy[i]);

	usb_phy_set_suspend(crg->usb3_phy, 1);
	usb_phy_set_suspend(crg->usb2_phy, 1);

	return ret;
}

static void crg_phy_power_off(struct crg_drd *crg)
{
	int i;

	for (i = 0; i < crg->num_usb3_ports; i++)
		phy_power_off(crg->usb3_generic_phy[i]);

	for (i = 0; i < crg->num_usb2_ports; i++)
		phy_power_off(crg->usb2_generic_phy[i]);

	usb_phy_set_suspend(crg->usb3_phy, 1);
	usb_phy_set_suspend(crg->usb2_phy, 1);
}

static void crg_core_exit(struct crg_drd	*crg)
{
	int	i;

	crg_phy_power_off(crg);
	crg_phy_exit(crg);

	for (i = 0; i < crg->num_usb2_ports; i++) {
		if (crg->usb2_generic_phy[i])
			phy_put(crg->dev, crg->usb2_generic_phy[i]);
	}
	for (i = 0; i < crg->num_usb3_ports; i++) {
		if (crg->usb3_generic_phy[i])
			phy_put(crg->dev, crg->usb3_generic_phy[i]);
	}

	crg_core_power(crg, false);
	crg_core_put_reset(crg);
}

static int crg_core_init(struct crg_drd *crg)
{
	int	i;
	int ret;

	ret = crg_core_get_reset(crg);
	if (ret)
		return ret;

	crg_core_power(crg, true);

	ret = crg_get_num_ports(crg);
	if (ret < 0) {
		crg_core_power(crg, false);
		dev_err(crg->dev, "cannot get port num.\n");
		return ret;
	}

	ret = crg_core_get_phy(crg);
	if (ret) {
		crg_core_power(crg, false);
		dev_err(crg->dev, "cannot get phy.\n");
		return ret;
	}

	ret = crg_phy_init(crg);
	if (ret)
		return ret;

	ret = crg_phy_power_on(crg);
	if (ret) {
		crg_phy_exit(crg);
		return ret;
	}

	switch (crg->dr_mode) {
	case USB_DR_MODE_PERIPHERAL:
		crg_set_mode(crg, CRG_GCTL_PRTCAP_DEVICE);
		break;
	case USB_DR_MODE_HOST:
		for (i = 0; i < crg->num_usb2_ports; i++)
			phy_set_mode(crg->usb2_generic_phy[i], PHY_MODE_USB_HOST);
		for (i = 0; i < crg->num_usb3_ports; i++)
			phy_set_mode(crg->usb3_generic_phy[i], PHY_MODE_USB_HOST);
		crg_set_mode(crg, CRG_GCTL_PRTCAP_HOST);
		break;
	case USB_DR_MODE_OTG:
		crg_set_mode(crg, CRG_GCTL_PRTCAP_OTG);
		break;
	default:
		dev_warn(crg->dev, "Unsupported mode %d\n", crg->dr_mode);
		break;
	}

	return 0;
}

static int crg_core_resume(struct crg_drd *crg)
{
	int			ret;

	crg_core_power(crg, true);

	ret = crg_phy_init(crg);
	if (ret)
		return ret;

	ret = crg_phy_power_on(crg);
	if (ret) {
		crg_phy_exit(crg);
		return ret;
	}

	switch (crg->dr_mode) {
	case USB_DR_MODE_PERIPHERAL:
		crg_set_mode(crg, CRG_GCTL_PRTCAP_DEVICE);
		break;
	case USB_DR_MODE_HOST:
		crg_set_mode(crg, CRG_GCTL_PRTCAP_HOST);
		break;
	case USB_DR_MODE_OTG:
		crg_set_mode(crg, CRG_GCTL_PRTCAP_OTG);
		break;
	default:
		dev_warn(crg->dev, "Unsupported mode %d\n", crg->dr_mode);
		break;
	}

	return 0;
}

static void crg_host_exit(struct crg_drd *crg)
{
	/* amlogic_crg_otg_work may call crg_exit (which calls crg_remove)
	 * after crg_shutdown. It is err-prone to dereference crg->xhci member and
	 * unregister crg->xhci. Set crg->xhci to NULL to avoid this.
	 * Since the xhci plat dev has been removed by crg_shutdown, this function
	 * should have been called. It is safe to return here.
	 */
	if (!crg->xhci) {
		dev_warn(crg->dev,
			"%s: crg->xhci already removed, exit...\n",
			__func__);
		return;
	}

	/*crg_reset_thread_stop(crg->xhci);*/
	platform_device_unregister(crg->xhci);
	crg->xhci = NULL;
}

static void crg_host_fill_xhci_irq_res(struct crg_drd *crg,
					int irq, char *name)
{
	struct platform_device *pdev = to_platform_device(crg->dev);
	struct device_node *np = dev_of_node(&pdev->dev);

	crg->xhci_resources[1].start = irq;
	crg->xhci_resources[1].end = irq;
	crg->xhci_resources[1].flags = IORESOURCE_IRQ | irq_get_trigger_type(irq);
	if (!name && np)
		crg->xhci_resources[1].name = of_node_full_name(pdev->dev.of_node);
	else
		crg->xhci_resources[1].name = name;
}

static int crg_host_get_irq(struct crg_drd *crg)
{
	struct platform_device	*crg_pdev = to_platform_device(crg->dev);
	int irq;

	irq = platform_get_irq_byname_optional(crg_pdev, "host");
	if (irq > 0) {
		crg_host_fill_xhci_irq_res(crg, irq, "host");
		goto out;
	}

	if (irq == -EPROBE_DEFER)
		goto out;

	irq = platform_get_irq_byname_optional(crg_pdev, "crg_usb3");
	if (irq > 0) {
		crg_host_fill_xhci_irq_res(crg, irq, "crg_usb3");
		goto out;
	}

	if (irq == -EPROBE_DEFER)
		goto out;

	irq = platform_get_irq(crg_pdev, 0);
	if (irq > 0) {
		crg_host_fill_xhci_irq_res(crg, irq, NULL);
		goto out;
	}

	if (!irq)
		irq = -EINVAL;

out:
	return irq;
}

static struct property_entry	props[64];
static int crg_host_init(struct crg_drd *crg)
{
	//struct property_entry	props[4];
	struct platform_device	*xhci;
	int			ret, irq;
	//struct resource		*res;
	//struct platform_device	*crg_pdev = to_platform_device(crg->dev);
	int			prop_idx = 0;
	int cpu_type;

	irq = crg_host_get_irq(crg);
	if (irq < 0)
		return irq;

	xhci = platform_device_alloc("xhci-hcd-meson", PLATFORM_DEVID_AUTO);
	if (!xhci) {
		dev_err(crg->dev, "couldn't allocate xHCI device\n");
		return -ENOMEM;
	}

	dma_set_coherent_mask(&xhci->dev, crg->dev->coherent_dma_mask);

	xhci->dev.parent	= crg->dev;
	xhci->dev.dma_mask	= crg->dev->dma_mask;
	xhci->dev.dma_parms	= crg->dev->dma_parms;

	crg->xhci = xhci;

	ret = platform_device_add_resources(xhci, crg->xhci_resources,
						CRG_XHCI_RESOURCES_NUM);
	if (ret) {
		dev_err(crg->dev, "couldn't add resources to xHCI device\n");
		goto err1;
	}

	memset(props, 0, sizeof(struct property_entry) * ARRAY_SIZE(props));

	if (crg->super_speed_support)
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("super_speed_support");

	props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host");
	props[prop_idx++] = PROPERTY_ENTRY_BOOL("usb2-lpm-disable");

	cpu_type = get_cpu_type();
	switch (cpu_type) {
	case MESON_CPU_MAJOR_ID_T5:
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-003");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-008");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-011");
		break;
	case MESON_CPU_MAJOR_ID_T5D:
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-003");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-008");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-011");
		break;
	case MESON_CPU_MAJOR_ID_T7:
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-007");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-010");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-014");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-008");
		break;
	case MESON_CPU_MAJOR_ID_S4:
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-008");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-009");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-011");
		break;
	case MESON_CPU_MAJOR_ID_T3:
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-007");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-010");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-014");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-008");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-011");
		break;
	case MESON_CPU_MAJOR_ID_S4D:
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-008");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-009");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-011");
		break;
	case MESON_CPU_MAJOR_ID_T5W:
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-007");
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-011");
		break;
	case MESON_CPU_MAJOR_ID_S6:
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-008");
		break;
	default:
		break;
	}

	props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-plug-died");
	props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-eproto");
	props[prop_idx++] = PROPERTY_ENTRY_BOOL("xhci-crg-host-016");

	/* Some quirky devices take >2s to turn on Rterm and begin polling.
	 * this leads to wait_for_connected timeout when resuming.
	 */
	if (crg->super_speed_support)
		props[prop_idx++] = PROPERTY_ENTRY_BOOL("resume_stuck_warm_reset");

	if (prop_idx) {
		ret = device_create_managed_software_node(&xhci->dev, props, NULL);
		if (ret) {
			dev_err(crg->dev, "failed to add properties to xHCI\n");
			goto err1;
		}
	}

	ret = platform_device_add_data(xhci, &crg_xhci_plat_priv,
								sizeof(crg_xhci_plat_priv));
	if (ret)
		goto err1;

	ret = platform_device_add(xhci);
	if (ret) {
		dev_err(crg->dev, "failed to register xHCI device\n");
		goto err1;
	}

	return 0;

err1:
	platform_device_put(xhci);
	return ret;
}

static int crg_core_init_mode(struct crg_drd *crg)
{
	struct device *dev = crg->dev;
	int ret;

	switch (crg->dr_mode) {
	case USB_DR_MODE_PERIPHERAL:
	case USB_DR_MODE_OTG:
		break;
	case USB_DR_MODE_HOST:
		ret = crg_host_init(crg);
		if (ret) {
			if (ret != -EPROBE_DEFER)
				dev_err(dev, "failed to initialize host\n");
			return ret;
		}
		break;
	default:
		dev_err(dev, "Unsupported mode of operation %d\n", crg->dr_mode);
		return -EINVAL;
	}

	return 0;
}

#define CRG_ALIGN_MASK		(16 - 1)

static int crg_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource	*res;
	struct crg_drd *crg;
	int	ret;
	void *mem;
	u32 tmp;
	const void *prop;
	unsigned int wr_outstanding_tune = 0;
	unsigned int rd_outstanding_tune = 0;
	unsigned int innakrty = 0;

	mem = devm_kzalloc(dev, sizeof(*crg) + CRG_ALIGN_MASK, GFP_KERNEL);
	if (!mem)
		return -ENOMEM;

	crg = PTR_ALIGN(mem, CRG_ALIGN_MASK + 1);
	crg->mem = mem;
	crg->dev = dev;
	crg->dr_mode = USB_DR_MODE_HOST;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "missing memory resource\n");
		return -ENODEV;
	}

	crg->xhci_resources[0].start = res->start;
	crg->xhci_resources[0].end = crg->xhci_resources[0].start +
					CRG_XHCI_REGS_END;
	crg->xhci_resources[0].flags = res->flags;
	crg->xhci_resources[0].name = res->name;
	crg->regs = ioremap(res->start, resource_size(res));
	if (!crg->regs) {
		dev_err(dev, "ioremap failed\n");
		return -ENODEV;
	}
	crg->general_clk = devm_clk_get(dev, "crg_general");
	if (IS_ERR(crg->general_clk)) {
		ret = PTR_ERR(crg->general_clk);
		goto err0;
	} else {
		clk_prepare_enable(crg->general_clk);
	}
	platform_set_drvdata(pdev, crg);

	if (!dev->dma_mask) {
		dev->dma_mask = dev->parent->dma_mask;
		dev->dma_parms = dev->parent->dma_parms;
		dma_set_coherent_mask(dev, dev->parent->coherent_dma_mask);
	}

	pm_runtime_set_active(dev);
	pm_runtime_use_autosuspend(dev);
	pm_runtime_set_autosuspend_delay(dev, CRG_DEFAULT_AUTOSUSPEND_DELAY);
	pm_runtime_enable(dev);
	ret = pm_runtime_get_sync(dev);
	if (ret < 0)
		goto err1;

	pm_runtime_forbid(dev);

	ret = crg_core_init(crg);
	if (ret) {
		dev_err(dev, "failed to initialize core\n");
		goto err1;
	}

	if (crg->super_speed_support)
		crg->maximum_speed = USB_SPEED_SUPER;
	else
		crg->maximum_speed = USB_SPEED_HIGH;

	/* Check the maximum_speed parameter */
	switch (crg->maximum_speed) {
	case USB_SPEED_LOW:
	case USB_SPEED_FULL:
	case USB_SPEED_HIGH:
		break;
	case USB_SPEED_SUPER:
		tmp = CRG_U3DC_CFG0_MAXSPEED_SS;
		writel(tmp, crg->regs + 0x2410);
		break;
	case USB_SPEED_SUPER_PLUS:
		break;
	default:
		dev_err(dev, "invalid maximum_speed parameter %d\n",
			crg->maximum_speed);
		break;
	}

	prop = of_get_property(pdev->dev.of_node, "dma-64bit-support", NULL);
	if (prop)
		if (of_read_ulong(prop, 1))
			crg_xhci_plat_priv.quirks &= (~XHCI_NO_64BIT_SUPPORT);

	ret = crg_core_init_mode(crg);
	if (ret)
		goto err1;

	prop = of_get_property(pdev->dev.of_node, "wr-outstanding-tune", NULL);
	if (prop)
		wr_outstanding_tune = of_read_ulong(prop, 1);
	prop = of_get_property(pdev->dev.of_node, "rd-outstanding-tune", NULL);
	if (prop)
		rd_outstanding_tune = of_read_ulong(prop, 1);

	if (wr_outstanding_tune) {
		wr_outstanding_tune = readl((void __iomem *)((unsigned long)crg->regs + 0x210c));
		wr_outstanding_tune &= (~0x7f000);
		writel(wr_outstanding_tune, (void __iomem *)((unsigned long)crg->regs + 0x210c));
	}

	if (rd_outstanding_tune) {
		tmp = readl((void __iomem *)((unsigned long)crg->regs + 0x210c));
		tmp &= ~(u32)GENMASK(25, 19);
		tmp |= rd_outstanding_tune << 19 & (u32)GENMASK(25, 19);
		writel(tmp, (void __iomem *)((unsigned long)crg->regs + 0x210c));
	}

	prop = of_get_property(dev->of_node, "in-nak-rty", NULL);
	if (prop) {
		innakrty = of_read_ulong(prop, 1);
		innakrty = (readl((void __iomem *)((unsigned long)crg->regs + 0x2110)) &
					~(0xff << 13)) | (innakrty << 13);
		writel(innakrty, (void __iomem *)((unsigned long)crg->regs + 0x2110));
	}

	pm_runtime_put(dev);

	return 0;

err1:
	pm_runtime_disable(dev);
err0:
	iounmap(crg->regs);
	return ret;
}

static void crg_shutdown(struct platform_device *pdev)
{
	struct crg_drd     *crg = platform_get_drvdata(pdev);

	pm_runtime_get_sync(&pdev->dev);

	crg_host_exit(crg);
	crg_core_exit(crg);
	if (crg->regs)
		iounmap(crg->regs);

	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_allow(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	clk_disable_unprepare(crg->general_clk);
}

static void crg_remove(struct platform_device *pdev)
{
	struct crg_drd	   *crg = platform_get_drvdata(pdev);

	pm_runtime_get_sync(&pdev->dev);

	crg_host_exit(crg);
	crg_core_exit(crg);
	if (crg->regs)
		iounmap(crg->regs);

	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_allow(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	clk_disable_unprepare(crg->general_clk);

	return;
}

#ifdef CONFIG_PM
static int crg_suspend_common(struct crg_drd *crg)
{
	switch (crg->dr_mode) {
	case USB_DR_MODE_PERIPHERAL:
	case USB_DR_MODE_OTG:
		break;
	case USB_DR_MODE_HOST:
	default:
		/* do nothing */
		break;
	}

	crg_phy_power_off(crg);
	crg_phy_exit(crg);
	crg_core_power(crg, false);

	return 0;
}

static int crg_resume_common(struct crg_drd *crg)
{
	int		ret;

	ret = crg_core_resume(crg);
	if (ret)
		return ret;

	crg_set_mode(crg, CRG_GCTL_PRTCAP_HOST);

	return 0;
}

static int crg_runtime_checks(struct crg_drd *crg)
{
	switch (crg->dr_mode) {
	case USB_DR_MODE_PERIPHERAL:
	case USB_DR_MODE_OTG:
	case USB_DR_MODE_HOST:
	default:
		/* do nothing */
		break;
	}

	return 0;
}

static int crg_runtime_suspend(struct device *dev)
{
	struct crg_drd     *crg = dev_get_drvdata(dev);
	int		ret;

	if (crg_runtime_checks(crg))
		return -EBUSY;

	ret = crg_suspend_common(crg);
	if (ret)
		return ret;

	device_init_wakeup(dev, true);

	return 0;
}

static int crg_runtime_resume(struct device *dev)
{
	struct crg_drd     *crg = dev_get_drvdata(dev);
	int		ret;

	device_init_wakeup(dev, false);

	ret = crg_resume_common(crg);
	if (ret)
		return ret;

	switch (crg->dr_mode) {
	case USB_DR_MODE_PERIPHERAL:
	case USB_DR_MODE_OTG:
	case USB_DR_MODE_HOST:
	default:
		/* do nothing */
		break;
	}

	pm_runtime_mark_last_busy(dev);
	pm_runtime_put(dev);

	return 0;
}

static int crg_runtime_idle(struct device *dev)
{
	struct crg_drd     *crg = dev_get_drvdata(dev);

	switch (crg->dr_mode) {
	case USB_DR_MODE_PERIPHERAL:
	case USB_DR_MODE_OTG:
		if (crg_runtime_checks(crg))
			return -EBUSY;
		break;
	case USB_DR_MODE_HOST:
	default:
		/* do nothing */
		break;
	}

	pm_runtime_mark_last_busy(dev);
	pm_runtime_autosuspend(dev);

	return 0;
}
#endif /* CONFIG_PM */

#ifdef CONFIG_PM_SLEEP
static int crg_suspend(struct device *dev)
{
	struct crg_drd	*crg = dev_get_drvdata(dev);
	int		ret;

	crg->usb_suspend = 1;
	ret = crg_suspend_common(crg);
	if (ret)
		return ret;

	clk_disable_unprepare(crg->general_clk);

	pinctrl_pm_select_sleep_state(dev);

	return 0;
}

static int crg_resume(struct device *dev)
{
	struct crg_drd	*crg = dev_get_drvdata(dev);
	int		ret;

	crg->usb_suspend = 0;

	pinctrl_pm_select_default_state(dev);

	clk_prepare_enable(crg->general_clk);

	ret = crg_resume_common(crg);
	if (ret)
		return ret;

	pm_runtime_disable(dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);

	return 0;
}
#endif /* CONFIG_PM_SLEEP */

static const struct dev_pm_ops crg_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(crg_suspend, crg_resume)
	SET_RUNTIME_PM_OPS(crg_runtime_suspend, crg_runtime_resume,
			crg_runtime_idle)
};

#ifdef CONFIG_OF
static const struct of_device_id of_crg_drd_match[] = {
	{
		.compatible = "amlogic, crg-drd"
	},
	{ },
};
MODULE_DEVICE_TABLE(of, of_crg_drd_match);
#endif

static struct platform_driver crg_driver = {
	.probe		= crg_probe,
	.remove		= crg_remove,
	.shutdown	= crg_shutdown,
	.driver		= {
		.name	= "crg_drd_otg",
		.of_match_table	= of_match_ptr(of_crg_drd_match),
		.pm	= &crg_dev_pm_ops,
	},
};

#ifdef CONFIG_OF
static const struct of_device_id of_crg_host_match[] = {
	{
		.compatible = "amlogic, crg-host-drd"
	},
	{ },
};
MODULE_DEVICE_TABLE(of, of_crg_host_match);
#endif

static struct platform_driver crg_host_driver = {
	.probe		= crg_probe,
	.remove		= crg_remove,
	.shutdown	= crg_shutdown,
	.driver		= {
		.name	= "crg_drd",
		.of_match_table	= of_match_ptr(of_crg_host_match),
		.pm	= &crg_dev_pm_ops,
	},
};

static int crg_driver_state;
void crg_exit(void)
{
	pr_info("crg exit\n");
	if (crg_driver_state != 1)
		return;
	crg_driver_state = 0;
	platform_driver_unregister(&crg_driver);
}

int crg_init(void)
{
	int ret;

	pr_info("crg init\n");
	if (crg_driver_state != 0) {
		ret = -EBUSY;
		goto exit;
	}

	ret = platform_driver_register(&crg_driver);
	if (ret) {
		pr_info("crg_driver register error %d, exit\n", ret);
		goto exit;
	}
	crg_driver_state = 1;
exit:
	return ret;
}

int __init amlogic_crg_host_driver_init(void)
{
	return platform_driver_register(&crg_host_driver);
}

void __exit amlogic_crg_host_driver_exit(void)
{
	platform_driver_unregister(&crg_host_driver);
}

#if 0
late_initcall(amlogic_crg_init);
MODULE_ALIAS("platform:crg");
MODULE_AUTHOR("yue wang <yue.wang@amlogic.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("corigine USB3 DRD Controller Driver");
#endif
