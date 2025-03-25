// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "phy-meson-usb.h"

int meson_m31_u3phy_exit(struct amlogic_usb_m31 *phy)
{
	u32 val, temp, shift = 0;
	size_t mask = 0;

	if (phy->portnum > 0) {
		mask = (size_t)phy->reset_regs & 0xf;
		shift = (phy->m31phy_reset_level_bit / 32) * 4;
		temp = 1 << (phy->m31phy_reset_level_bit % 32);
		val = readl((void __iomem		*)
			((unsigned long)phy->reset_regs +
			(phy->reset_level - mask) + shift));
		writel((val & (~temp)), (void __iomem	*)
			((unsigned long)phy->reset_regs +
			(phy->reset_level - mask) + shift));
	}

	phy->suspend_flag = 1;

	return 0;
}

static int meson_m31_u3phy_host_init(struct amlogic_usb_m31 *phy)
{
	u32 val, temp, shift = 0;
	size_t mask = 0;
	union phy_m31_r0 r0;

	if (!phy->portnum)
		return 0;

	if (!phy->suspend_flag) {
		dev_err(phy->dev, "%s excessive init\n", __func__);
		return 0;
	}

	if (phy->m31_utmi_reset_level_bit == -1U ||
			phy->m31ctl_reset_level_bit == -1U ||
			phy->u3_combx0_reset_bit == -1U ||
			phy->m31phy_reset_level_bit == -1U) {
		dev_err(phy->dev, "%s err no m31_utmi_reset_level_bit or\n"
			"u3_combx0_reset_bit in drd phy port or\n"
			"m31phy_reset_level_bit, exit.\n", __func__);
		return -EINVAL;
	}

	writel(readl(phy->reset_regs + phy->reset_level + (phy->u3_combx0_reset_bit / 32) * 4)
		| (1 << phy->u3_combx0_reset_bit % 32),
		phy->reset_regs + phy->reset_level + (phy->u3_combx0_reset_bit / 32) * 4);

	writel(readl(phy->reset_regs + phy->reset_level + (phy->m31ctl_reset_level_bit / 32) * 4) |
			(1 << phy->m31ctl_reset_level_bit % 32),
		phy->reset_regs + phy->reset_level + (phy->m31ctl_reset_level_bit / 32) * 4);

	writel(readl(phy->reset_regs + phy->reset_level + (phy->m31_utmi_reset_level_bit / 32) * 4)
		| (1 << phy->m31_utmi_reset_level_bit % 32),
		phy->reset_regs + phy->reset_level + (phy->m31_utmi_reset_level_bit / 32) * 4);

	writel(readl(phy->reset_regs + phy->reset_level + (phy->m31phy_reset_level_bit / 32) * 4) |
			(1 << phy->m31phy_reset_level_bit % 32),
		phy->reset_regs + phy->reset_level + (phy->m31phy_reset_level_bit / 32) * 4);

	usleep_range(12, 100);

	mask = (size_t)phy->reset_regs & 0xf;

	temp = 1 << (phy->m31phy_reset_level_bit % 32);
	shift = (phy->m31phy_reset_level_bit / 32) * 4;
	val = readl((void __iomem		*)
		((unsigned long)phy->reset_regs +
		(phy->reset_level - mask) + shift));
	writel((val & (~temp)), (void __iomem	*)
		((unsigned long)phy->reset_regs +
		(phy->reset_level - mask) + shift));
	udelay(9);
	writel((val | (temp)), (void __iomem	*)
		((unsigned long)phy->reset_regs +
		(phy->reset_level - mask) + shift));

	temp = 1 << (phy->u3_combx0_reset_bit % 32);
	shift = (phy->u3_combx0_reset_bit / 32) * 4;
	val = readl((void __iomem		*)
		((unsigned long)phy->reset_regs +
		(phy->reset_level - mask) + shift));
	writel((val & (~temp)), (void __iomem	*)
		((unsigned long)phy->reset_regs +
		(phy->reset_level - mask) + shift));
	usleep_range(100, 200);
	writel((val | (temp)), (void __iomem	*)
		((unsigned long)phy->reset_regs +
		(phy->reset_level - mask) + shift));
	usleep_range(100, 200);

	temp = 1 << (phy->m31_utmi_reset_level_bit % 32);
	shift = (phy->m31_utmi_reset_level_bit / 32) * 4;
	val = readl((void __iomem		*)
		((unsigned long)phy->reset_regs +
		(phy->reset_level - mask) + shift));
	writel((val & (~temp)), (void __iomem	*)
		((unsigned long)phy->reset_regs +
		(phy->reset_level - mask) + shift));
	usleep_range(100, 200);
	writel((val | (temp)), (void __iomem	*)
		((unsigned long)phy->reset_regs +
		(phy->reset_level - mask) + shift));
	usleep_range(100, 200);

	temp = 1 << (phy->m31ctl_reset_level_bit % 32);
	shift = (phy->m31ctl_reset_level_bit / 32) * 4;
	val = readl((void __iomem		*)
		((unsigned long)phy->reset_regs +
		(phy->reset_level - mask) + shift));
	writel((val & (~temp)), (void __iomem	*)
		((unsigned long)phy->reset_regs +
		(phy->reset_level - mask) + shift));
	udelay(9);
	writel((val | (temp)), (void __iomem	*)
		((unsigned long)phy->reset_regs +
		(phy->reset_level - mask) + shift));

	writel(0x1, phy->phy3_cfg + 0x8);
	usleep_range(100, 200);

	writel(0, phy->phy3_cfg + 0xc);
	usleep_range(100, 200);

	if (phy->uncomposite) {
		val = readl(phy->phy3_cfg + 0x82c);
		val |= (1 << 5);
		writel(val, phy->phy3_cfg + 0x82c);
		usleep_range(100, 200);

		val = readl(phy->phy3_cfg + 0x848);
		val |= (3 << 0);
		writel(val, phy->phy3_cfg + 0x848);
		usleep_range(100, 200);
	}

	r0.d32 = readl(phy->phy3_cfg);
	r0.b.PHY_SEL = 0;
	r0.b.U3_HOST_PHY = 1;
	r0.b.PCIE_CLKSEL = 0;
	r0.b.U3_SSRX_SEL = 1;
	r0.b.U3_SSTX_SEL = 1;
	r0.b.REFPAD_EXT_100M_EN = 0;
	r0.b.TX_ENABLE_N = 1;
	r0.b.TX_SE0 = 0;
	r0.b.FSLSSERIALMODE = 0;

	writel(r0.d32, phy->phy3_cfg);
	usleep_range(100, 200);

	if (phy->version == 1) {
		val = readl(phy->phy3_cfg + 0x81c);
		val = (val & ~(0x7 << 13)) | (5 << 13);
		writel(val, phy->phy3_cfg + 0x81c);
		usleep_range(100, 200);
	}

	dev_dbg(phy->dev, "%s finished\n", __func__);

	if (phy->suspend_flag)
		phy->suspend_flag = 0;

	return 0;
}

static void meson_m31_u3phy_device_init_v0(struct amlogic_usb_m31 *phy)
{
#define M31_SETTING 0x1E30CEB9
	writel(1, phy->phy3_cfg + 0x8);
	udelay(9);

	writel(0, phy->phy3_cfg + 0xc);
	udelay(9);

	writel(0x3, phy->phy3_cfg + 0x848);
	udelay(9);

	/* to do */
	writel(M31_SETTING, phy->phy3_cfg);
	udelay(9);

	dev_dbg(phy->dev, "%s finished\n", __func__);
}

static void meson_m31_u3phy_device_init_v1(struct amlogic_usb_m31 *phy)
{
	u32 val;

	if (!phy->pipe_clk_reg || !phy->pipe_clk_gate_reg) {
		dev_err(phy->dev, "no pipe clk reg, udc may not be functional.\n");
		return;
	}

	if (phy->m31_settings == -1U) {
		dev_err(phy->dev, "no M31 settings, udc may not be functional.\n");
		return;
	}

	if (phy->u3_combx0_reset_bit == -1U) {
		dev_err(phy->dev, "no u3_combx0_reset_bit(u3drd0 general reset), exit.\n");
		return;
	}

	if (phy->m31_utmi_reset_level_bit == -1U) {
		dev_err(phy->dev, "no m31_utmi_reset_level_bit, exit.\n");
		return;
	}

	/* u3drd/phy reset bit is checked in the phy probe. */

	/*step 1: power on domain, if default is not on*/
	/*default is power on*/

	/*step 2: usb bus clock*/
	/*sys_clk  gate*/

	/*step 3: power on*/
	writel(readl(phy->reset_regs + phy->reset_level + (phy->m31ctl_reset_level_bit / 32) * 4) |
			(1 << phy->m31ctl_reset_level_bit % 32),
		phy->reset_regs + phy->reset_level + (phy->m31ctl_reset_level_bit / 32) * 4);

	writel(readl(phy->reset_regs + phy->reset_level + (phy->m31_utmi_reset_level_bit / 32) * 4)
		| (1 << phy->m31_utmi_reset_level_bit % 32),
		phy->reset_regs + phy->reset_level + (phy->m31_utmi_reset_level_bit / 32) * 4);

	writel(readl(phy->reset_regs + phy->reset_level + (phy->m31phy_reset_level_bit / 32) * 4) |
			(1 << phy->m31phy_reset_level_bit % 32),
		phy->reset_regs + phy->reset_level + (phy->m31phy_reset_level_bit / 32) * 4);

	usleep_range(12, 100);

	/*step 4: usb controller reset*/
	/*bit21: u3drd0 general reset*/
	writel((1 << phy->u3_combx0_reset_bit % 32),
			phy->reset_regs + (phy->u3_combx0_reset_bit / 32) * 4);
	usleep_range(12, 100);

	/*bit2: usb3drd0 utmi reset   bit6: usb3drd0 reset   bit10: usb30 phy reset*/
	writel((1 << phy->m31ctl_reset_level_bit % 32),
			phy->reset_regs + (phy->m31ctl_reset_level_bit / 32) * 4);

	writel((1 << phy->m31_utmi_reset_level_bit % 32),
			phy->reset_regs + (phy->m31_utmi_reset_level_bit / 32) * 4);

	writel((1 << phy->m31phy_reset_level_bit % 32),
			phy->reset_regs + (phy->m31phy_reset_level_bit / 32) * 4);

	usleep_range(12, 100);

	/* m31 phy setting*/
	writel((u32)phy->m31_settings, phy->phy3_cfg);
	usleep_range(12, 100);

	/*setp 6: bypass usb3*/
	val = readl(phy->phy3_cfg + 0x8);
	val |= (0x1830 << 6);
	writel(val, phy->phy3_cfg + 0x8);
	usleep_range(12, 100);

	val = readl(phy->phy3_cfg + 0xc);
	val |= (0xe0 << 3);
	writel(val, phy->phy3_cfg + 0xc);
	usleep_range(12, 100);

	/*step 7: pipe clk setting*/
	writel(0x103, phy->pipe_clk_reg);
	usleep_range(12, 100);

	val = readl(phy->pipe_clk_gate_reg);
	val |= (1 << 13);
	writel(val, phy->pipe_clk_gate_reg);
	usleep_range(12, 100);

	/*step 8: reset u3drdx0 & M31phy*/
	writel((1 << phy->m31ctl_reset_level_bit % 32),
			phy->reset_regs + (phy->m31ctl_reset_level_bit / 32) * 4);
	writel((1 << phy->m31phy_reset_level_bit % 32),
		phy->reset_regs + (phy->m31phy_reset_level_bit / 32) * 4);
	usleep_range(12, 100);

	dev_dbg(phy->dev, "%s finished\n", __func__);
}

static int meson_m31_u3phy_device_init(struct amlogic_usb_m31 *phy)
{
	if (!phy)
		return -ENODEV;

	if (!phy->suspend_flag) {
		dev_err(phy->dev, "%s excessive init\n", __func__);
		return 0;
	}

	switch (phy->version) {
	case 0:
		meson_m31_u3phy_device_init_v0(phy);
		break;
	case 1:
		meson_m31_u3phy_device_init_v1(phy);
		break;
	default:
		dev_err(phy->dev, "no matched version. Check dts node?\n");
		break;
	}

	if (phy->suspend_flag)
		phy->suspend_flag = 0;

	return 0;
}

int meson_m31_u3phy_set_mode(struct amlogic_usb_m31 *phy, enum meson_uphy_mode mode)
{
	int ret = 0;

	dev_dbg(phy->dev, "%s: set mode %d.\n", __func__, mode);

	switch (mode) {
	case MESON_USB_MODE_HOST:
		meson_m31_u3phy_host_init(phy);
		break;
	case MESON_USB_MODE_DEVICE:
		meson_m31_u3phy_device_init(phy);
		break;
	default:
		dev_dbg(phy->dev, "ERROR %s unknown mode %d.\n", __func__, mode);
		ret = -EINVAL;
		break;
	}
	return ret;
}

/* The mode set and init are hard to decouple. Do all these stuffs in set_mode. */
int meson_m31_u3phy_init(struct amlogic_usb_m31 *phy)
{
	return 0;
}

static inline bool meson_m31_u3phy_muxed(void)
{
	return of_device_is_available(of_find_node_by_name(NULL, "pcie")) ||
			of_device_is_available(of_find_node_by_type(NULL, "pci"));
}

int meson_m31_u3phy_parse(struct device *dev, struct meson_uphy_instance *instance)
{
	struct amlogic_usb_m31			*phy;
	const void *prop;
	int addr_i = 0, ret = 0;
	u64 addr, size;
	u32 phy3_mem;
	u32 phy3_mem_size = 0;
	u32 pipe_clk_reg_mem;
	u32 pipe_clk_reg_mem_size = 0;
	u32 pipe_clk_gate_reg_mem;
	u32 pipe_clk_gate_reg_mem_size = 0;
	union phy_m31_r0 r0;
	void __iomem *pipe_clk_reg;
	void __iomem *pipe_clk_gate_reg;
	u32 reset_level = 0x84;
	u32 m31phy_reset_level_bit = -1U;
	u32 m31ctl_reset_level_bit = -1U;
	u32 m31_utmi_reset_level_bit = -1U;
	u32 u3_combx0_reset_bit = -1U;
	u32 m31_settings = -1U;
	int m31_utmi_reset_level_flag = 0;
	int u3_combx0_reset_flag = 0;
	/* Only aml p1 soc use version 0 m31 phy. */
	u32 version = 0;

	phy = kzalloc(sizeof(*phy), GFP_KERNEL);
	if (!phy)
		return -ENOMEM;

	phy->phy_id = instance->index;
	phy->dev = dev;
	dev_dbg(dev, "phy_id %d.\n", phy->phy_id);
	instance->meson_uphy = phy;
	get_device(dev);

	phy->portnum = 1;

	ret = of_property_read_u32(dev->of_node, "uncomposite", &phy->uncomposite);
	if (ret < 0)
		phy->uncomposite = 0;

	ret = of_property_read_u32(dev->of_node, "phy-reg", &phy3_mem);
	if (ret < 0)
		return -EINVAL;

	ret = of_property_read_u32
				(dev->of_node, "phy-reg-size", &phy3_mem_size);
	if (ret < 0)
		return -EINVAL;

	dev_dbg(phy->dev, "phy-reg:0x%x, phy-reg-size 0x%x.\n", phy3_mem, phy3_mem_size);

	phy->phy3_cfg = devm_ioremap
				(dev, (resource_size_t)phy3_mem,
				(resource_size_t)phy3_mem_size);

	dev_dbg(phy->dev, "phy3_cfg 0x%px.\n", phy->phy3_cfg);
	if (!phy->phy3_cfg)
		return -ENOMEM;

	if (phy->uncomposite == 0 && meson_m31_u3phy_muxed()) {
		dev_info(dev,
			"port muxed to pci-e, stop probing USB3 phy port%d.\n", phy->phy_id);
		phy->portnum = 0;
		return 0;
	}

	ret = of_property_read_u32(dev->of_node, "pipe-clk-reg", &pipe_clk_reg_mem);
	if (ret >= 0) {
		ret = of_property_read_u32
				(dev->of_node, "pipe-clk-reg-size", &pipe_clk_reg_mem_size);
		if (ret < 0) {
			pipe_clk_reg = NULL;
		} else {
			dev_dbg(phy->dev, "pipe-clk-reg:0x%x, pipe-clk-reg-size 0x%x.\n",
						pipe_clk_reg_mem, pipe_clk_reg_mem_size);
			pipe_clk_reg = devm_ioremap(dev,
				(resource_size_t)pipe_clk_reg_mem,
				(resource_size_t)pipe_clk_reg_mem_size);
			if (!pipe_clk_reg)
				return -ENOMEM;
		}
	} else {
		pipe_clk_reg = NULL;
	}

	ret = of_property_read_u32(dev->of_node, "pipe-clk-gate-reg", &pipe_clk_gate_reg_mem);
	if (ret >= 0) {
		ret = of_property_read_u32
				(dev->of_node, "pipe-clk-reg-size", &pipe_clk_gate_reg_mem_size);
		if (ret < 0) {
			pipe_clk_gate_reg = NULL;
		} else {
			dev_dbg(phy->dev, "pipe-clk-gate-reg:0x%x, pipe-clk-gate-reg-size 0x%x.\n",
						pipe_clk_gate_reg_mem, pipe_clk_gate_reg_mem_size);
			pipe_clk_gate_reg = devm_ioremap(dev,
				(resource_size_t)pipe_clk_gate_reg_mem,
				(resource_size_t)pipe_clk_gate_reg_mem_size);
			if (!pipe_clk_gate_reg)
				return -ENOMEM;
		}
	} else {
		pipe_clk_gate_reg = NULL;
	}

	ret = of_property_read_reg(dev->of_node, addr_i++, &addr, &size);
	if (ret) {
		dev_err(dev, "failed to get address %d (id-%d)\n", addr_i,
			phy->phy_id);
		return ret;
	}

	dev_dbg(phy->dev, "reset-regs:0x%llx, reset-regs-size 0x%llx.\n",
				addr, size);

	phy->reset_regs = devm_ioremap(dev, (resource_size_t)addr, (resource_size_t)size);
	if (!phy->reset_regs)
		return PTR_ERR(phy->reset_regs);

	prop = of_get_property(dev->of_node, "reset-level", NULL);
	if (prop)
		reset_level = of_read_ulong(prop, 1);
	else
		reset_level = 0x84;
	prop = of_get_property(dev->of_node, "m31phy-reset-level-bit", NULL);
	if (prop)
		m31phy_reset_level_bit = of_read_ulong(prop, 1);

	prop = of_get_property(dev->of_node, "m31ctl-reset-level-bit", NULL);
	if (prop)
		m31ctl_reset_level_bit = of_read_ulong(prop, 1);

	prop = of_get_property(dev->of_node, "m31-utmi-reset-level-bit", NULL);
	if (prop) {
		m31_utmi_reset_level_bit = of_read_ulong(prop, 1);
		m31_utmi_reset_level_flag = 1;
	}

	prop = of_get_property(dev->of_node, "u3-combx0-reset-bit", NULL);
	if (prop) {
		u3_combx0_reset_bit = of_read_ulong(prop, 1);
		u3_combx0_reset_flag = 1;
	}

	prop = of_get_property(dev->of_node, "m31-settings", NULL);
	if (prop)
		m31_settings = of_read_ulong(prop, 1);

	prop = of_get_property(dev->of_node, "version", NULL);
	if (prop)
		version = of_read_ulong(prop, 1);
	else
		version = 0;

	phy->phy_off = of_property_read_bool(dev->of_node, "off");

	phy->pipe_clk_reg = pipe_clk_reg;
	phy->pipe_clk_gate_reg = pipe_clk_gate_reg;
	phy->reset_level = reset_level;
	phy->m31phy_reset_level_bit = m31phy_reset_level_bit;
	phy->m31ctl_reset_level_bit = m31ctl_reset_level_bit;
	phy->m31_utmi_reset_level_bit = m31_utmi_reset_level_bit;
	phy->u3_combx0_reset_bit = u3_combx0_reset_bit;
	phy->version = version;
	phy->m31_settings = m31_settings;

	if (phy->m31ctl_reset_level_bit == -1U || phy->m31phy_reset_level_bit == -1U) {
		dev_err(phy->dev, "no basic u3drd/phy reset bits, exit.\n");
		return -EINVAL;
	}

	if (phy->phy_off && phy->uncomposite) {
		r0.d32 = readl(phy->phy3_cfg);
		r0.b.TX_ENABLE_N = 1;
		r0.b.PHY_SEL = 1;
		r0.b.FSLSSERIALMODE = 0;
		r0.b.PHY_SSCG_ON = 0;
		writel(r0.d32, phy->phy3_cfg);
		usleep_range(90, 100);
	}
	/* phy->phy_off && !phy->uncomposite, do nothing. */

	meson_m31_u3phy_exit(phy);

	return 0;
}

int meson_u2phy_m31_set_test_mode(struct amlogic_usb_v2 *phy, u32 test_mode)
{
	if (test_mode == MESON_USB_DEVICE_TEST_MODE_COMPL) {
		writel(0x3, phy->phy3_cfg + 0x848);
		udelay(9);
	}

	if (test_mode == MESON_USB_DEVICE_TEST_JK_COMPL) {
		writel(0x0, phy->phy3_cfg + 0x848);
		udelay(9);
	}

	return 0;
}

int meson_m31_u2phy_parse(struct device *dev, struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2 *phy;
	int addr_i = 0, ret = 0;
	u64 addr, size;

	phy = kzalloc(sizeof(*phy), GFP_KERNEL);
	if (!phy)
		return -ENOMEM;

	phy->phy_id = instance->index;
	phy->dev = dev;
	dev_dbg(dev, "phy_id %d.\n", phy->phy_id);
	instance->meson_uphy = phy;
	get_device(dev);

	phy->portspeed = instance->port_speed;

	dev_dbg(dev, "portspeed: %d\n", phy->portspeed);

	ret = of_property_read_reg(dev->of_node, addr_i++, &addr, &size);
	if (ret) {
		dev_err(dev, "failed to get address %d (id-%d)\n", addr_i,
			phy->phy_id);
		return ret;
	}

	phy->regs = devm_ioremap(dev, (resource_size_t)addr, (resource_size_t)size);
	if (IS_ERR(phy->regs))
		return PTR_ERR(phy->regs);

	dev_dbg(dev, "phy_mem:0x%llx, iomap phy_base:0x%px\n",
						addr, phy->regs);

	phy->u2p_aml_regs[0] = phy->regs;

	ret = of_property_read_reg(dev->of_node, addr_i++, &addr, &size);
	if (ret) {
		dev_err(dev, "failed to get address %d (id-%d)\n", addr_i,
			phy->phy_id);
		return ret;
	}

	phy->phy3_cfg = devm_ioremap(dev, (resource_size_t)addr, (resource_size_t)size);
	if (IS_ERR(phy->phy3_cfg))
		return PTR_ERR(phy->phy3_cfg);

	return ret;
}

