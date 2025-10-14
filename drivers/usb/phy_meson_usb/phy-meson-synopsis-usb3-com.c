// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "phy-meson-usb.h"
/* Meson synopsis usb3 phy. */

static void meson_synopsis_u3phy_addr(struct amlogic_usb_v2 *phy, unsigned int addr)
{
	union phy3_r4 phy_r4 = {.d32 = 0};
	union phy3_r5 phy_r5 = {.d32 = 0};
	unsigned long timeout_jiffies;

	phy_r4.b.phy_cr_data_in = addr;
	writel(phy_r4.d32, phy->phy3_cfg_r4);

	phy_r4.b.phy_cr_cap_addr = 0;
	writel(phy_r4.d32, phy->phy3_cfg_r4);
	phy_r4.b.phy_cr_cap_addr = 1;
	writel(phy_r4.d32, phy->phy3_cfg_r4);
	timeout_jiffies = jiffies +
			msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 0 &&
		time_is_after_jiffies(timeout_jiffies));

	phy_r4.b.phy_cr_cap_addr = 0;
	writel(phy_r4.d32, phy->phy3_cfg_r4);
	timeout_jiffies = jiffies +
			msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 1 &&
		time_is_after_jiffies(timeout_jiffies));
}

static int meson_synopsis_u3phy_read(struct amlogic_usb_v2 *phy, unsigned int addr)
{
	int data;
	union phy3_r4 phy_r4 = {.d32 = 0};
	union phy3_r5 phy_r5 = {.d32 = 0};
	unsigned long timeout_jiffies;

	meson_synopsis_u3phy_addr(phy, addr);

	phy_r4.b.phy_cr_read = 0;
	writel(phy_r4.d32, phy->phy3_cfg_r4);
	phy_r4.b.phy_cr_read = 1;
	writel(phy_r4.d32, phy->phy3_cfg_r4);

	timeout_jiffies = jiffies +
			msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 0 &&
		time_is_after_jiffies(timeout_jiffies));

	data = phy_r5.b.phy_cr_data_out;

	phy_r4.b.phy_cr_read = 0;
	writel(phy_r4.d32, phy->phy3_cfg_r4);
	timeout_jiffies = jiffies +
			msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 1 &&
		time_is_after_jiffies(timeout_jiffies));

	return data;
}

static void meson_synopsis_u3phy_write(struct amlogic_usb_v2 *phy,
		unsigned int addr, unsigned int data)
{
	union phy3_r4 phy_r4 = {.d32 = 0};
	union phy3_r5 phy_r5 = {.d32 = 0};
	unsigned long timeout_jiffies;

	meson_synopsis_u3phy_addr(phy, addr);

	phy_r4.b.phy_cr_data_in = data;
	writel(phy_r4.d32, phy->phy3_cfg_r4);

	phy_r4.b.phy_cr_cap_data = 0;
	writel(phy_r4.d32, phy->phy3_cfg_r4);
	phy_r4.b.phy_cr_cap_data = 1;
	writel(phy_r4.d32, phy->phy3_cfg_r4);
	timeout_jiffies = jiffies +
		msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 0 &&
		time_is_after_jiffies(timeout_jiffies));

	phy_r4.b.phy_cr_cap_data = 0;
	writel(phy_r4.d32, phy->phy3_cfg_r4);
	timeout_jiffies = jiffies +
		msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 1 &&
		time_is_after_jiffies(timeout_jiffies));

	phy_r4.b.phy_cr_write = 0;
	writel(phy_r4.d32, phy->phy3_cfg_r4);
	phy_r4.b.phy_cr_write = 1;
	writel(phy_r4.d32, phy->phy3_cfg_r4);
	timeout_jiffies = jiffies +
		msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 0 &&
		time_is_after_jiffies(timeout_jiffies));

	phy_r4.b.phy_cr_write = 0;
	writel(phy_r4.d32, phy->phy3_cfg_r4);
	timeout_jiffies = jiffies +
		msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(phy->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 1 &&
		time_is_after_jiffies(timeout_jiffies));
}

int meson_synopsis_u3phy_init(struct amlogic_usb_v2 *phy)
{
	struct usb_aml_regs_v2 usb_new_aml_regs_v2;
	union usb_r1_v2 r1 = {.d32 = 0};
	union usb_r2_v2 r2 = {.d32 = 0};
	union usb_r3_v2 r3 = {.d32 = 0};
	union phy3_r2 p3_r2 = {.d32 = 0};
	union phy3_r1 p3_r1 = {.d32 = 0};
	int ret = 0, i = 0;
	u32 data = 0;

	dev_dbg(phy->dev, "meson_synopsis_u3phy init start.\n");

	if (!phy->suspend_flag) {
		dev_err(phy->dev, "%s excessive init\n", __func__);
		return -EBUSY;
	}

	ret = clk_bulk_prepare_enable(phy->clk_num, phy->clks);

	if (ret) {
		dev_err(phy->dev, "Failed to enable usb phy bus clock at %d\n",
							__LINE__);
		return ret;
	}
	/* Power on. */
	writel(0x7c, phy->phy3_cfg);

	/* set the phy from pcie to usb3 */
	writel((readl(phy->phy3_cfg) | (3 << 5)), phy->phy3_cfg);
	usleep_range(100, 150);

	for (i = 0; i < 6; i++) {
		usb_new_aml_regs_v2.usb_r_v2[i] = (void __iomem *)
			((unsigned long)phy->regs + 4 * i);
	}

	r1.d32 = readl(usb_new_aml_regs_v2.usb_r_v2[1]);
	r1.b.u3h_fladj_30mhz_reg = 0x20;
	writel(r1.d32, usb_new_aml_regs_v2.usb_r_v2[1]);

	/* config usb3 phy */
	r3.d32 = readl(usb_new_aml_regs_v2.usb_r_v2[3]);
	r3.b.p30_ssc_en = 1;
	r3.b.p30_ssc_range = 2;
	r3.b.p30_ref_ssp_en = 1;
	writel(r3.d32, usb_new_aml_regs_v2.usb_r_v2[3]);
	udelay(2);
	r2.d32 = readl(usb_new_aml_regs_v2.usb_r_v2[2]);
	r2.b.p30_pcs_tx_deemph_3p5db = 0x15;
	r2.b.p30_pcs_tx_deemph_6db = 0x20;
	writel(r2.d32, usb_new_aml_regs_v2.usb_r_v2[2]);
	udelay(2);
	r1.d32 = readl(usb_new_aml_regs_v2.usb_r_v2[1]);
	r1.b.u3h_host_port_power_control_present = 1;
	r1.b.u3h_fladj_30mhz_reg = 0x20;
	r1.b.p30_pcs_tx_swing_full = 127;
	writel(r1.d32, usb_new_aml_regs_v2.usb_r_v2[1]);
	udelay(2);
	p3_r2.d32 = readl(phy->phy3_cfg_r2);
	p3_r2.b.phy_tx_vboost_lvl = 0x4;
	writel(p3_r2.d32, phy->phy3_cfg_r2);
	udelay(2);

	/*
	 * WORKAROUND: There is SSPHY suspend bug due to
	 * which USB enumerates
	 * in HS mode instead of SS mode. Workaround it by asserting
	 * LANE0.TX_ALT_BLOCK.EN_ALT_BUS to enable TX to use alt bus
	 * mode
	 */
	data = meson_synopsis_u3phy_read(phy, 0x102d);
	data |= (1 << 7);
	meson_synopsis_u3phy_write(phy, 0x102D, data);

	data = meson_synopsis_u3phy_read(phy, 0x1010);
	data &= ~0xff0;
	data |= 0x20;
	meson_synopsis_u3phy_write(phy, 0x1010, data);

	/*
	 * Fix RX Equalization setting as follows
	 * LANE0.RX_OVRD_IN_HI. RX_EQ_EN set to 0
	 * LANE0.RX_OVRD_IN_HI.RX_EQ_EN_OVRD set to 1
	 * LANE0.RX_OVRD_IN_HI.RX_EQ set to 3
	 * LANE0.RX_OVRD_IN_HI.RX_EQ_OVRD set to 1
	 */
	data = meson_synopsis_u3phy_read(phy, 0x1006);
	data &= ~(1 << 6);
	data |= (1 << 7);
	data &= ~(0x7 << 8);
	data |= (0x3 << 8);
	data |= (0x1 << 11);
	meson_synopsis_u3phy_write(phy, 0x1006, data);

	/*
	 * Set EQ and TX launch amplitudes as follows
	 * LANE0.TX_OVRD_DRV_LO.PREEMPH set to 22
	 * LANE0.TX_OVRD_DRV_LO.AMPLITUDE set to 127
	 * LANE0.TX_OVRD_DRV_LO.EN set to 1.
	 */
	data = meson_synopsis_u3phy_read(phy, 0x1002);
	data &= ~0x3f80;
	data |= (0x16 << 7);
	data &= ~0x7f;
	data |= (0x7f | (1 << 14));
	meson_synopsis_u3phy_write(phy, 0x1002, data);

	/*
	 * MPLL_LOOP_CTL.PROP_CNTRL
	 */
	data = meson_synopsis_u3phy_read(phy, 0x30);
	data &= ~(0xf << 4);
	data |= (0x8 << 4);
	meson_synopsis_u3phy_write(phy, 0x30, data);
	udelay(2);

	/*
	 * LOS_BIAS	to 0x5
	 * LOS_LEVEL to 0x9
	 */
	p3_r1.d32 = readl(phy->phy3_cfg_r1);
	p3_r1.b.phy_los_bias = 0x4;
	p3_r1.b.phy_los_level = 0x9;
	writel(p3_r1.d32, phy->phy3_cfg_r1);

	phy->suspend_flag = 0;

	dev_dbg(phy->dev, "meson_synopsis_u3phy init end.\n");
	return 0;
}

int meson_synopsis_u3phy_exit(struct amlogic_usb_v2 *phy)
{
	if (phy->suspend_flag) {
		dev_err(phy->dev, "%s excessive exit\n", __func__);
		return -EBUSY;
	}

	clk_bulk_disable_unprepare(phy->clk_num, phy->clks);

	/* Power off. */
	writel(0x1d, phy->phy3_cfg);

	phy->suspend_flag = 1;

	return 0;
}

int meson_synopsis_u3phy_set_mode(struct amlogic_usb_v2 *phy, enum meson_uphy_mode mode)
{
	/* Support controller super_speed_support config. */
	if (!phy->portnum)
		return -EINVAL;

	return 0;
}

static bool meson_synopsis_u3phy_muxed(void)
{
	return meson_uphy_of_device_pci_available();
}

int meson_synopsis_u3phy_parse(struct device *dev, struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2 *aml_u3phy;
	int i, cnt, addr_i = 0, ret = 0;
	u64 addr, size;

	aml_u3phy = kzalloc(sizeof(*aml_u3phy), GFP_KERNEL);
	if (!aml_u3phy)
		return -ENOMEM;

	aml_u3phy->phy_id = instance->index;
	aml_u3phy->dev = dev;
	dev_dbg(dev, "phy_id %d.\n", aml_u3phy->phy_id);
	instance->meson_uphy = aml_u3phy;
	get_device(dev);

	aml_u3phy->portnum = 1;

	if (meson_synopsis_u3phy_muxed()) {
		dev_info(dev,
			"port muxed to pci-e, stop probing USB3 phy port%d.\n", aml_u3phy->phy_id);
		/* TODO: track this path to phy destroy. */
		return -ENODEV;
	}

	ret = of_property_read_u32(dev->of_node, "version", &aml_u3phy->version);
	if (ret)
		aml_u3phy->version = 0;

	if (!aml_u3phy->version)
		dev_info(dev, "Normal phy.\n");

	ret = of_property_read_reg(dev->of_node, addr_i++, &addr, &size);
	if (ret) {
		dev_err(dev, "failed to get address %d(id-%d)\n",
			addr_i, aml_u3phy->phy_id);
		return ret;
	}
	aml_u3phy->regs = devm_ioremap(dev, (resource_size_t)addr, (resource_size_t)size);
	if (IS_ERR(aml_u3phy->regs))
		return PTR_ERR(aml_u3phy->regs);
	aml_u3phy->usb_aml_regs = aml_u3phy->regs;
	dev_dbg(dev, "phy_mem:0x%llx, iomap phy_base:0x%px\n",
						addr, aml_u3phy->regs);

	ret = of_property_read_reg(dev->of_node, addr_i++, &addr, &size);
	if (ret) {
		dev_err(dev, "failed to get address %d(id-%d)\n",
			addr_i, aml_u3phy->phy_id);
		return ret;
	}
	aml_u3phy->phy3_cfg = devm_ioremap(dev, (resource_size_t)addr, (resource_size_t)size);
	if (IS_ERR(aml_u3phy->phy3_cfg))
		return PTR_ERR(aml_u3phy->phy3_cfg);
	dev_dbg(dev, "phy_cfg_mem:0x%llx, iomap phy_cfg_base:0x%px\n",
						addr, aml_u3phy->phy3_cfg);
	aml_u3phy->phy3_cfg_r1 = (void __iomem *)
			((unsigned long)aml_u3phy->phy3_cfg + 4 * 1);
	aml_u3phy->phy3_cfg_r2 = (void __iomem *)
			((unsigned long)aml_u3phy->phy3_cfg + 4 * 2);
	aml_u3phy->phy3_cfg_r4 = (void __iomem *)
			((unsigned long)aml_u3phy->phy3_cfg + 4 * 4);
	aml_u3phy->phy3_cfg_r5 = (void __iomem *)
			((unsigned long)aml_u3phy->phy3_cfg + 4 * 5);

	cnt = of_property_count_strings(dev->of_node, "clock-names");
	if (cnt < 0) {
		dev_err(dev, "no clks? exit.");
		return -EINVAL;
	} else if (cnt > AML_USB_PHY_MAX_CLK_NUMBER) {
		dev_err(dev, "too many clks. exit.");
		return -EOVERFLOW;
	}
	dev_dbg(dev, "clk num: %d\n", cnt);
	for (i = 0; i < cnt; i++) {
		ret = of_property_read_string_index(dev->of_node, "clock-names",
									i, &aml_u3phy->clks[i].id);
		if (ret < 0) {
			dev_err(dev, "read clk-names idx:%d err", i);
			return -EINVAL;
		}
	}

	aml_u3phy->clk_num = cnt;

	ret = devm_clk_bulk_get(dev, aml_u3phy->clk_num, aml_u3phy->clks);
	if (ret) {
		dev_dbg(dev, "Failed to get usb phy bus clocks\n");
		return ret;
	}

	for (i = 0; i < aml_u3phy->clk_num; i++)
		dev_dbg(dev, "%s %px.\n", aml_u3phy->clks[i].id, (void *)aml_u3phy->clks[i].clk);

	/* Default OFF. */
	writel(0x1d, aml_u3phy->phy3_cfg);
	aml_u3phy->suspend_flag = 1;

	return ret;
}

