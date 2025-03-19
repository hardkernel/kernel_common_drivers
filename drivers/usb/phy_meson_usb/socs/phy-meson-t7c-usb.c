// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "../phy-meson-usb.h"

/* USB 2 */

static void meson_usb2phy_t7c_cali(struct amlogic_usb_v2 *mphy)
{
	meson_usb2phy_legacy_cali(mphy);
	meson_usb2phy_legacy_cali_disc_squelch(mphy);
}

static struct meson_u2phy_priv meson_u2phy_t7c_priv = {
	.cali = meson_usb2phy_t7c_cali,
	.set_pll = meson_usb2phy_legacy_set_pll,
};

static int meson_u2phy_t7c_set_mode(struct phy *phy, enum phy_mode mode, int submode)
{
	int ret = 0;
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	ret = meson_u2phy_set_mode(mphy, mode);

	return ret;
}

static int meson_u2phy_t7c_init(struct phy *phy)
{
	return meson_u2phy_aml_init(gphy_to_amlusbv2phy(phy), &meson_u2phy_t7c_priv);
}

static int meson_u2phy_t7c_exit(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	return meson_u2phy_exit(mphy);
}

static int meson_u2phy_t7c_power_on(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	return meson_u2phy_power_on(mphy);
}

static int meson_u2phy_t7c_power_off(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	return meson_u2phy_power_off(mphy);
}

/* USB 3 */
#define PCIE_PLL_RATE 100000000

static int meson_u3phy_t7c_exit(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);

	if (mphy->portnum <= 0)
		return 0;

	if (mphy->suspend_flag == 0) {
		if (!(IS_ERR(mphy->hcsl_clk)))
			clk_disable_unprepare(mphy->hcsl_clk);

		clk_disable_unprepare(mphy->clk);

		writel(0xf5, mphy->phy31_cfg);
	}

	mphy->suspend_flag = 1;

	return 0;
}

static void crg_bus_addr(struct amlogic_usb_v2 *phy_v3, unsigned int addr)
{
	union phy3_r4 phy_r4 = {.d32 = 0};
	union phy3_r5 phy_r5 = {.d32 = 0};
	unsigned long timeout_jiffies;

	phy_r4.b.phy_cr_data_in = addr;
	writel(phy_r4.d32, phy_v3->phy3_cfg_r4);

	phy_r4.b.phy_cr_cap_addr = 0;
	writel(phy_r4.d32, phy_v3->phy3_cfg_r4);
	phy_r4.b.phy_cr_cap_addr = 1;
	writel(phy_r4.d32, phy_v3->phy3_cfg_r4);
	timeout_jiffies = jiffies +
			msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy_v3->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 0 &&
		time_is_after_jiffies(timeout_jiffies));

	phy_r4.b.phy_cr_cap_addr = 0;
	writel(phy_r4.d32, phy_v3->phy3_cfg_r4);
	timeout_jiffies = jiffies +
			msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy_v3->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 1 &&
		time_is_after_jiffies(timeout_jiffies));
}

static int crg_bus_read(struct amlogic_usb_v2 *phy_v3, unsigned int addr)
{
	int data;
	union phy3_r4 phy_r4 = {.d32 = 0};
	union phy3_r5 phy_r5 = {.d32 = 0};
	unsigned long timeout_jiffies;

	crg_bus_addr(phy_v3, addr);

	phy_r4.b.phy_cr_read = 0;
	writel(phy_r4.d32, phy_v3->phy3_cfg_r4);
	phy_r4.b.phy_cr_read = 1;
	writel(phy_r4.d32, phy_v3->phy3_cfg_r4);

	timeout_jiffies = jiffies +
			msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy_v3->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 0 &&
		time_is_after_jiffies(timeout_jiffies));

	data = phy_r5.b.phy_cr_data_out;

	phy_r4.b.phy_cr_read = 0;
	writel(phy_r4.d32, phy_v3->phy3_cfg_r4);
	timeout_jiffies = jiffies +
			msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy_v3->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 1 &&
		time_is_after_jiffies(timeout_jiffies));

	return data;
}

static void crg_bus_write(struct amlogic_usb_v2 *phy_v3,
			 unsigned int addr, unsigned int data)
{
	union phy3_r4 phy_r4 = {.d32 = 0};
	union phy3_r5 phy_r5 = {.d32 = 0};
	unsigned long timeout_jiffies;

	crg_bus_addr(phy_v3, addr);

	phy_r4.b.phy_cr_data_in = data;
	writel(phy_r4.d32, phy_v3->phy3_cfg_r4);

	phy_r4.b.phy_cr_cap_data = 0;
	writel(phy_r4.d32, phy_v3->phy3_cfg_r4);
	phy_r4.b.phy_cr_cap_data = 1;
	writel(phy_r4.d32, phy_v3->phy3_cfg_r4);
	timeout_jiffies = jiffies +
		msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy_v3->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 0 &&
		time_is_after_jiffies(timeout_jiffies));

	phy_r4.b.phy_cr_cap_data = 0;
	writel(phy_r4.d32, phy_v3->phy3_cfg_r4);
	timeout_jiffies = jiffies +
		msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy_v3->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 1 &&
		time_is_after_jiffies(timeout_jiffies));

	phy_r4.b.phy_cr_write = 0;
	writel(phy_r4.d32, phy_v3->phy3_cfg_r4);
	phy_r4.b.phy_cr_write = 1;
	writel(phy_r4.d32, phy_v3->phy3_cfg_r4);
	timeout_jiffies = jiffies +
		msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy_v3->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 0 &&
		time_is_after_jiffies(timeout_jiffies));

	phy_r4.b.phy_cr_write = 0;
	writel(phy_r4.d32, phy_v3->phy3_cfg_r4);
	timeout_jiffies = jiffies +
		msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy_v3->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 1 &&
		time_is_after_jiffies(timeout_jiffies));
}

static void meson_u3phy_t7c_config(struct amlogic_usb_v2 *phy)
{
	u32 data = 0;

	/*
	 * WORKAROUND: There is SSPHY suspend bug due to
	 * which USB enumerates
	 * in HS mode instead of SS mode. Workaround it by asserting
	 * LANE0.TX_ALT_BLOCK.EN_ALT_BUS to enable TX to use alt bus
	 * mode
	 */
	data = crg_bus_read(phy, 0x102d);
	data |= (1 << 7);
	crg_bus_write(phy, 0x102D, data);

	data = crg_bus_read(phy, 0x1010);
	data &= ~0xff0;
	data |= 0x20;
	crg_bus_write(phy, 0x1010, data);

	/*
	 * Fix RX Equalization setting as follows
	 * LANE0.RX_OVRD_IN_HI. RX_EQ_EN set to 0
	 * LANE0.RX_OVRD_IN_HI.RX_EQ_EN_OVRD set to 1
	 * LANE0.RX_OVRD_IN_HI.RX_EQ set to 3
	 * LANE0.RX_OVRD_IN_HI.RX_EQ_OVRD set to 1
	 */
	data = crg_bus_read(phy, 0x1006);
	data &= ~(1 << 6);
	data |= (1 << 7);
	data &= ~(0x7 << 8);
	data |= (0x3 << 8);
	data |= (0x1 << 11);
	crg_bus_write(phy, 0x1006, data);

	/*
	 * S	et EQ and TX launch amplitudes as follows
	 * LANE0.TX_OVRD_DRV_LO.PREEMPH set to 22
	 * LANE0.TX_OVRD_DRV_LO.AMPLITUDE set to 127
	 * LANE0.TX_OVRD_DRV_LO.EN set to 1.
	 */
	data = crg_bus_read(phy, 0x1002);
	data &= ~0x3f80;
	data |= (0x16 << 7);
	data &= ~0x7f;
	data |= (0x7f | (1 << 14));
	crg_bus_write(phy, 0x1002, data);

	/*
	 * MPLL_LOOP_CTL.PROP_CNTRL
	 */
	data = crg_bus_read(phy, 0x30);
	data &= ~(0xf << 4);
	data |= (0x8 << 4);
	crg_bus_write(phy, 0x30, data);
	udelay(2);
}

static int meson_u3phy_t7c_init(struct phy *phy)
{
	struct amlogic_usb_v2 *mphy = gphy_to_amlusbv2phy(phy);
	union usb_r3_v2 r3 = {.d32 = 0};
	union usb_r7_v2 r7 = {.d32 = 0};
	union phy3_r2 p3_r2 = {.d32 = 0};
	union phy3_r1 p3_r1 = {.d32 = 0};
	struct usb_aml_regs_v2 usb_crg_drd_aml_regs;
	int i = 0, ret;
	u32 val;
	unsigned long rate;

	if (mphy->portnum <= 0)
		return 0;

	ret = clk_prepare_enable(mphy->clk);
	if (ret) {
		dev_err(mphy->dev, "Failed to enable usb3 bus clock\n");
		ret = PTR_ERR(mphy->clk);
		return ret;
	}

	rate = clk_get_rate(mphy->clk);
	if (rate != PCIE_PLL_RATE)
		dev_err(mphy->dev, "pcie_refpll is not 100M, it is %ld\n",
			rate);

	if (!(IS_ERR(mphy->hcsl_clk))) {
		ret = clk_prepare_enable(mphy->hcsl_clk);
		if (ret) {
			dev_err(mphy->dev, "Failed to enable usb3 hcsl clock\n");
			ret = PTR_ERR(mphy->hcsl_clk);
			return ret;
		}
	}

	val = readl(mphy->phy31_cfg);
	val |= (3 << 5);
	val &= (~(1));
	writel(val, mphy->phy31_cfg);

	mphy->suspend_flag = 0;

	for (i = 0; i < 8; i++)
		usb_crg_drd_aml_regs.usb_r_v2[i] = (void __iomem *)
			((unsigned long)mphy->regs + 4 * i);

	r3.d32 = readl(usb_crg_drd_aml_regs.usb_r_v2[3]);
	r3.b.p30_ref_ssp_en = 1;
	writel(r3.d32, usb_crg_drd_aml_regs.usb_r_v2[3]);
	udelay(2);
	r7.d32 = readl(usb_crg_drd_aml_regs.usb_r_v2[7]);
	r7.b.p31_ssc_en = 1;
	r7.b.p31_ssc_range = 2;
	writel(r7.d32, usb_crg_drd_aml_regs.usb_r_v2[7]);
	udelay(2);
	r7.d32 = readl(usb_crg_drd_aml_regs.usb_r_v2[7]);
	r7.b.p31_pcs_tx_deemph_6db = 0x20;
	r7.b.p31_pcs_tx_swing_full = 127;
	writel(r7.d32, usb_crg_drd_aml_regs.usb_r_v2[7]);
	udelay(2);
	r3.d32 = readl(usb_crg_drd_aml_regs.usb_r_v2[3]);
	r3.b.p31_pcs_tx_deemph_3p5db = 0x15;
	writel(r3.d32, usb_crg_drd_aml_regs.usb_r_v2[3]);

	udelay(2);
	p3_r2.d32 = readl(mphy->phy3_cfg_r2);
	p3_r2.b.phy_tx_vboost_lvl = 0x4;
	writel(p3_r2.d32, mphy->phy3_cfg_r2);
	udelay(2);

	meson_u3phy_t7c_config(mphy);

	/*
	 * LOS_BIAS to 0x5
	 * LOS_LEVEL to 0x9
	 */
	p3_r1.d32 = readl(mphy->phy3_cfg_r1);
	p3_r1.b.phy_los_bias = 0x5;
	p3_r1.b.phy_los_level = 0x9;
	writel(p3_r1.d32, mphy->phy3_cfg_r1);

	return 0;
}

static inline bool meson_u3phy_t7c_muxed(void)
{
	return of_device_is_available(of_find_node_by_name(NULL, "pcie")) ||
			of_device_is_available(of_find_node_by_type(NULL, "pci"));
}

int meson_u3phy_t7c_parse(struct device *dev, struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2			*phy;
	void __iomem *phy31_base;
	void __iomem *phy_pcie_base;
	void __iomem	*reset_base = NULL;
	unsigned int phy31_mem;
	unsigned int phy31_mem_size = 0;
	unsigned int phy_pcie_mem;
	unsigned int phy_pcie_mem_size = 0;
	unsigned int reset_mem;
	unsigned int reset_mem_size = 0;
	const void *prop;
	int retval;
	int ret;
	u32 val;
	int reset_level = 0x40;
	u32 usb3_apb_reset_bit = 23;
	u32 usb3_phy_reset_bit = 22;
	u32 usb3_reset_shift = 4;
	int addr_i = 0;
	u64 addr, size;

	phy = kzalloc(sizeof(*phy), GFP_KERNEL);
	if (!phy)
		return -ENOMEM;

	phy->phy_id = instance->index;
	phy->dev = dev;
	mup_dbg(dev, "phy_id %d.\n", phy->phy_id);
	instance->meson_uphy = phy;
	get_device(dev);

	/* add sw disable control. */
	phy->portnum = of_property_read_bool(dev->of_node, "off") ? 0 : 1;

	ret = of_property_read_reg(dev->of_node, addr_i++, &addr, &size);
	if (ret) {
		mup_err(dev, "failed to get address %d(id-%d)\n",
			addr_i, phy->phy_id);
		return ret;
	}
	phy->regs = devm_ioremap(dev, (resource_size_t)addr, (resource_size_t)size);
	if (IS_ERR(phy->regs))
		return PTR_ERR(phy->regs);
	mup_dbg(dev, "phy_mem:0x%llx, iomap phy_base:0x%px\n",
						addr, phy->regs);

	retval = of_property_read_u32(dev->of_node, "phy1-reg", &phy31_mem);
	if (retval < 0)
		return -EINVAL;

	retval = of_property_read_u32
				(dev->of_node, "phy1-reg-size",
					&phy31_mem_size);
	if (retval < 0)
		return -EINVAL;

	phy31_base = ioremap((resource_size_t)phy31_mem,
					(unsigned long)phy31_mem_size);
	if (!phy31_base)
		return -ENOMEM;

	retval = of_property_read_u32(dev->of_node, "reset-reg", &reset_mem);
	if (retval < 0)
		return -EINVAL;

	retval = of_property_read_u32
			(dev->of_node, "reset-reg-size",
				&reset_mem_size);
	if (retval < 0)
		return -EINVAL;

	reset_base = ioremap((resource_size_t)reset_mem,
					(unsigned long)reset_mem_size);
	if (!reset_base)
		return -ENOMEM;

	prop = of_get_property(dev->of_node, "reset-level", NULL);
	if (prop)
		reset_level = of_read_ulong(prop, 1);
	else
		reset_level = 0x40;

	prop = of_get_property(dev->of_node, "usb3-apb-reset-bit", NULL);
	if (prop)
		usb3_apb_reset_bit = of_read_ulong(prop, 1);
	else
		usb3_apb_reset_bit = 23;

	prop = of_get_property(dev->of_node, "usb3-phy-reset-bit", NULL);
	if (prop)
		usb3_phy_reset_bit = of_read_ulong(prop, 1);
	else
		usb3_phy_reset_bit = 22;

	prop = of_get_property(dev->of_node, "usb3-reset-shit", NULL);
	if (prop)
		usb3_reset_shift = of_read_ulong(prop, 1);
	else
		usb3_reset_shift = 4;

	phy->dev		= dev;
	phy->phy31_cfg	= phy31_base;
	phy->phy3_cfg_r1 = (void __iomem *)
			((unsigned long)phy->phy31_cfg + 4 * 1);
	phy->phy3_cfg_r2 = (void __iomem *)
			((unsigned long)phy->phy31_cfg + 4 * 2);
	phy->phy3_cfg_r4 = (void __iomem *)
			((unsigned long)phy->phy31_cfg + 4 * 4);
	phy->phy3_cfg_r5 = (void __iomem *)
			((unsigned long)phy->phy31_cfg + 4 * 5);
	phy->suspend_flag = 0;
	phy->reset_regs = reset_base;
	phy->reset_level = reset_level;
	phy->usb3_apb_reset_bit = usb3_apb_reset_bit;
	phy->usb3_phy_reset_bit = usb3_phy_reset_bit;
	phy->usb3_reset_shift = usb3_reset_shift;

	/* set the phy from pcie to usb3 */
	if (phy->portnum > 0) {
		retval = of_property_read_u32(dev->of_node,
			 "phy-pcie-reg", &phy_pcie_mem);
		if (retval < 0)
			return -EINVAL;

		retval = of_property_read_u32
					(dev->of_node, "phy-pcie-reg-size",
						&phy_pcie_mem_size);
		if (retval < 0)
			return -EINVAL;

		phy_pcie_base = ioremap((resource_size_t)phy_pcie_mem,
				(unsigned long)phy_pcie_mem_size);
		if (!phy_pcie_base)
			return -ENOMEM;

		if (!meson_u3phy_t7c_muxed()) {
			dev_info(dev, "no pci-e driver!,power down pcie phy\n");
			writel(0x1d, phy_pcie_base);
		}

		writel((0x1 << usb3_apb_reset_bit) | (0x1 << usb3_phy_reset_bit),
			(void __iomem *)
			((unsigned long)phy->reset_regs + phy->usb3_reset_shift));

		usleep_range(100, 200);

		val = readl(phy->reset_regs + phy->reset_level + phy->usb3_reset_shift);
		writel(val | (0x1 << usb3_apb_reset_bit) | (0x1 << usb3_phy_reset_bit),
			(void __iomem *)
			((unsigned long)phy->reset_regs + phy->reset_level
			+ phy->usb3_reset_shift));

		usleep_range(100, 200);

		val = readl(phy->phy31_cfg);
		val |= (3 << 5);
		writel(val, phy->phy31_cfg);

		usleep_range(100, 200);

		val = readl(phy->phy31_cfg + 0x18);
		val &= (~(0x3 << 17));
		val |= (0x1 << 18);
		writel(val, phy->phy31_cfg + 0x18);

		usleep_range(100, 200);

		phy->clk = devm_clk_get(dev, "pcie_refpll");
		if (IS_ERR(phy->clk)) {
			dev_err(dev, "Failed to get usb3 bus clock\n");
			ret = PTR_ERR(phy->clk);
			return ret;
		}

		phy->hcsl_clk = devm_clk_get(dev, "pcie_hcsl");
		if (IS_ERR(phy->hcsl_clk)) {
			dev_dbg(dev, "Failed to get usb3 hcsl clock\n");
			ret = PTR_ERR(phy->hcsl_clk);
		}
	}

	return 0;
}

static struct meson_uphy_ops meson_u2phy_t7c_ops = {
	.init = meson_u2phy_t7c_init,
	.exit = meson_u2phy_t7c_exit,
	.power_on = meson_u2phy_t7c_power_on,
	.power_off = meson_u2phy_t7c_power_off,
	.set_mode = meson_u2phy_t7c_set_mode,
};

static struct meson_uphy_ops meson_u3phy_t7c_ops = {
	.init = meson_u3phy_t7c_init,
	.exit = meson_u3phy_t7c_exit,
};

struct meson_uphy_pdata meson_uphy_t7c_pdata = {
	.u2phy_ops = &meson_u2phy_t7c_ops,
	.u3phy_ops = &meson_u3phy_t7c_ops,
	.u2phy_parse = meson_aml_u2phy_parse,
	.u3phy_parse = meson_u3phy_t7c_parse,
	.otg_parse = meson_u2phy_crg_otg_parse,
	.otg_remove = meson_u2phy_crg_otg_remove,
};

