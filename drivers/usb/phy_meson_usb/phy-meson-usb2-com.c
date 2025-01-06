// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "phy-meson-usb.h"

bool meson_usb2phy_dbg;
module_param(meson_usb2phy_dbg, bool, 0644);

/* Reset usb controller. */
int meson_u2phy_usb_reset(struct amlogic_usb_v2 *phy)
{
	static int	init_count;

	mu2p_dbg(phy->dev, "%s initial: 0x%x.\n", __func__,
				readl(phy->reset_regs + phy->reset_level));
	if (!init_count) {
		init_count++;
		if (phy->usb_reset_bit == 2)
			writel((readl(phy->reset_regs) |
				(0x1 << phy->usb_reset_bit)), phy->reset_regs);
		else
			writel((0x1 << phy->usb_reset_bit), phy->reset_regs);
	}
	mu2p_dbg(phy->dev, "%s after: 0x%x.\n", __func__,
				readl(phy->reset_regs + phy->reset_level));
	return 0;
}

int meson_u2phy_usb_hold_reset(struct amlogic_usb_v2 *phy, bool on)
{
	u32 val = 0;
	size_t mask = 0;
	u32 tmp = 0;

	mu2p_dbg(phy->dev, "%s initial: 0x%x.\n", __func__,
			readl(phy->reset_regs + phy->reset_level));

	mask = (size_t)phy->reset_regs & 0xf;

	if (phy->usb_reset_bit != -1U)
		tmp |= BIT(phy->usb_reset_bit);
	if (phy->usb_comb_reset_bit != -1U)
		tmp |= BIT(phy->usb_comb_reset_bit);

	val = readl((void __iomem *)
		((unsigned long)phy->reset_regs + (phy->reset_level - mask)));

	if (on) {
		writel(val | tmp, (void __iomem	*)
			((unsigned long)phy->reset_regs + (phy->reset_level - mask)));
	} else {
		writel(val & ~tmp, (void __iomem *)
			((unsigned long)phy->reset_regs + (phy->reset_level - mask)));
	}

	mu2p_dbg(phy->dev, "%s after: 0x%x.\n", __func__,
			readl(phy->reset_regs + phy->reset_level));

	return 0;
}

int meson_u2phy_hold_reset(struct amlogic_usb_v2 *phy, int port, bool on)
{
	u32 val = 0, temp = 0;
	size_t mask = 0;

	if (port >= phy->portnum)
		mu2p_err(phy->dev, "%s port index %d overflow.\n", __func__, port);

	mu2p_dbg(phy->dev, "%s initial: 0x%x.\n", __func__,
				readl(phy->reset_regs + phy->reset_level));
	mask = (size_t)phy->reset_regs & 0xf;

	temp = temp | (1 << phy->phy_reset_level_bit[port]);

	val = readl((void __iomem		*)
		((unsigned long)phy->reset_regs + (phy->reset_level - mask)));

	if (on) {
		writel(val | temp, (void __iomem	*)
			((unsigned long)phy->reset_regs + (phy->reset_level - mask)));
	} else {
		writel(val & ~temp, (void __iomem	*)
			((unsigned long)phy->reset_regs + (phy->reset_level - mask)));
	}
	mu2p_dbg(phy->dev, "%s after: 0x%x.\n", __func__,
				readl(phy->reset_regs + phy->reset_level));

	return 0;
}

/* Reset phy hw state machine. */
int meson_u2phy_reset_phycfg(struct amlogic_usb_v2 *phy, int port)
{
	meson_u2phy_hold_reset(phy, port, false);
	usleep_range(50, 100);
	meson_u2phy_hold_reset(phy, port, true);

	return 0;
}

int meson_u2phy_reg_hold_reset(struct amlogic_usb_v2 *phy, int port, bool on)
{
	u32 val = 0, temp = 0;
	size_t mask = 0;

	if (port >= phy->portnum)
		mu2p_err(phy->dev, "%s port index %d overflow.\n", __func__, port);

	mu2p_dbg(phy->dev, "%s initial: 0x%x.\n", __func__,
				readl(phy->reset_regs + phy->reset_level));

	mask = (size_t)phy->reset_regs & 0xf;

	if (phy->phy_reg_reset_level_bit[port] != -1U)
		temp = temp | (1 << phy->phy_reg_reset_level_bit[port]);

	val = readl((void __iomem		*)
		((unsigned long)phy->reset_regs + (phy->reset_level - mask)));

	if (on) {
		writel(val | temp, (void __iomem	*)
			((unsigned long)phy->reset_regs + (phy->reset_level - mask)));
	} else {
		writel(val & ~temp, (void __iomem	*)
			((unsigned long)phy->reset_regs + (phy->reset_level - mask)));
	}
	mu2p_dbg(phy->dev, "%s after: 0x%x.\n", __func__,
				readl(phy->reset_regs + phy->reset_level));

	return 0;
}

/* Reset phy reg values. */
int meson_u2phy_reg_reset(struct amlogic_usb_v2 *phy, int port)
{
	int ret = 0;

	ret = meson_u2phy_reg_hold_reset(phy, port, false);
	if (ret)
		goto done;
	ret = meson_u2phy_reg_hold_reset(phy, port, true);
	if (ret)
		goto err;
done:
	return ret;
err:
	return ret;
}

void meson_u2phy_set_vbus_power(struct amlogic_usb_v2 *phy, bool is_power_on)
{
	mu2p_dbg(phy->dev, "%s %d pin:%d.\n", __func__, is_power_on, phy->vbus_power_pin);

	if (phy->vbus_power_pin == -1)
		return;

	if (is_power_on)
		gpiod_direction_output(phy->usb_gpio_desc, 1);
	else
		gpiod_direction_output(phy->usb_gpio_desc, 0);
}

void meson_usb2phy_set_calibration_trim(struct amlogic_usb_v2 *mphy, int port)
{
	u32 value = 0;
	u32 cali, i;
	u8 cali_en;

	if (!mphy->usb_phy_trim_reg) {
		mu2p_err(mphy->dev, "No usb-phy-trim-reg\n");
		return;
	}

	cali = readl(mphy->usb_phy_trim_reg);
	cali_en = (cali >> 12) & 0x1;
	cali = cali >> 8;
	if (cali_en) {
		cali = cali & 0xf;
		/* s7 modify. */
		cali = cali + 2;
		if (cali > 12)
			cali = 12;
	} else {
		cali = mphy->pll_setting[4];
	}
	value = readl(mphy->phy_cfg[port] + 0x10);
	value &= (~0xfff);
	for (i = 0; i < cali; i++)
		value |= (1 << i);

	writel(value, mphy->phy_cfg[port] + 0x10);

	mu2p_dbg(mphy->dev, "phy trim value= 0x%08x\n", value);
}

int meson_u2phy_set_mode(struct amlogic_usb_v2 *phy, int port,
			enum phy_mode mode)
{
	union u2p_r0_v2 reg0;
	void __iomem *cfg = phy->phy_cfg[port];

	switch (mode) {
	case PHY_MODE_USB_HOST:
		reg0.d32 = readl(&phy->u2p_aml_regs[port]->r0);
		//if (phy->suspend_flag == 0) {
			reg0.b.host_device = 1;
			reg0.b.POR = 0;
			reg0.b.IDPULLUP0 = 1;
			reg0.b.DRVVBUS0 = 1;
		//}
		writel(reg0.d32, &phy->u2p_aml_regs[port]->r0);
		phy->otg_helper.mode &= ~(AML_USB_OTG_HOST_MODE_MASK |
								AML_USB_OTG_DEVICE_MODE_MASK);
		phy->otg_helper.mode |= AML_USB_OTG_HOST_MODE_MASK;
		phy->last_mode = PHY_MODE_USB_HOST;
		break;
	case PHY_MODE_USB_DEVICE:
		reg0.d32 = readl(&phy->u2p_aml_regs[port]->r0);
		//if (phy->suspend_flag == 0) {
			reg0.b.host_device = 0;
			reg0.b.POR = 0;
			reg0.b.IDPULLUP0 = 1;
			reg0.b.DRVVBUS0 = 1;
		//}
		writel(reg0.d32, &phy->u2p_aml_regs[port]->r0);
		phy->otg_helper.mode &= ~(AML_USB_OTG_HOST_MODE_MASK |
								AML_USB_OTG_DEVICE_MODE_MASK);
		phy->otg_helper.mode |= AML_USB_OTG_DEVICE_MODE_MASK;
		phy->last_mode = PHY_MODE_USB_DEVICE;
		break;
	case PHY_MODE_USB_OTG:
		/* ID DETECT: usb2_otg_aca_en set to 0 */
		writel(readl(cfg + 0x54) & (~(1 << 2)), (cfg + 0x54));
		/* usb2_otg_iddet_en set to 1 by otg driver. */
		phy->otg_helper.mode |= AML_USB_OTG_OTG_MODE_MASK;
		break;
	default:
		break;
	}
	return 0;
}

int meson_usb2phy_wait_ready(struct amlogic_usb_v2 *phy,
		int port, unsigned int timeout)
{
	union u2p_r1_v2 reg1;
	unsigned int cnt = 0;
	int ret = 0;

	reg1.d32 = readl(&phy->u2p_aml_regs[port]->r1);

	while (reg1.b.phy_rdy != 1) {
		reg1.d32 = readl(&phy->u2p_aml_regs[port]->r1);
		/*we wait phy ready max 1ms, common is 100us*/
		if (cnt > timeout) {
			ret = -ETIMEDOUT;
			break;
		}
		cnt++;
		udelay(5);
	}

	return ret;
}

int meson_u2phy_exit(struct amlogic_usb_v2 *phy, int port)
{
	int ret = 0;

	ret = meson_u2phy_hold_reset(phy, port, false);

	if (phy->suspend_flag == 0)
		//if (phy->phy.flags == AML_USB2_PHY_ENABLE)
		clk_bulk_disable_unprepare(phy->clk_num, phy->clks);

	phy->suspend_flag = 1;

	return ret;
}

int meson_u2phy_power_on(struct amlogic_usb_v2 *phy)
{
	meson_u2phy_set_vbus_power(phy, 1);
	return 0;
}

int meson_u2phy_power_off(struct amlogic_usb_v2 *phy)
{
	meson_u2phy_set_vbus_power(phy, 0);
	return 0;
}

/* Of node parse common codes. */
int meson_aml_u2phy_parse(struct device *dev, struct meson_uphy_instance *instance)
{
	struct amlogic_usb_v2 *aml_u2phy;
	const char *gpio_name = NULL;
	int i, cnt, ret = 0;
	u64 addr, size;

	aml_u2phy = devm_kzalloc(dev, sizeof(*aml_u2phy), GFP_KERNEL);
	if (!aml_u2phy)
		return -ENOMEM;

	aml_u2phy->phy_id = instance->index;
	aml_u2phy->dev = dev;
	mu2p_dbg(dev, "phy_id %d.\n", aml_u2phy->phy_id);
	instance->meson_uphy = aml_u2phy;
	get_device(dev);

	gpio_name = of_get_property(dev->of_node, "gpio-vbus-power", NULL);
	if (gpio_name) {
		aml_u2phy->vbus_power_pin = 1;
		aml_u2phy->usb_gpio_desc = devm_gpiod_get_index(dev,
					 NULL, 0, GPIOD_OUT_LOW);
		if (IS_ERR(aml_u2phy->usb_gpio_desc))
			return -ENODEV;
	}

	/* Assume that all phy node represent _only_ one hw port.
	 * FIXME: What if the phy controller cfg regs and phy config regs are
	 * not one-to-one maps?
	 */
	aml_u2phy->portnum = 1;

	ret = of_property_read_reg(dev->of_node, 0, &addr, &size);
	if (ret) {
		mu2p_err(dev, "failed to get address 0(id-%d)\n",
			aml_u2phy->phy_id);
		return ret;
	}

	aml_u2phy->regs = devm_ioremap(dev, (resource_size_t)addr, (resource_size_t)size);
	if (IS_ERR(aml_u2phy->regs))
		return PTR_ERR(aml_u2phy->regs);

	mu2p_dbg(dev, "phy_mem:0x%llx, iomap phy_base:0x%px\n",
						addr, aml_u2phy->regs);

	ret = of_property_read_reg(dev->of_node, 1, &addr, &size);
	if (ret) {
		mu2p_err(dev, "failed to get address 1(id-%d)\n",
			aml_u2phy->phy_id);
		return ret;
	}

	aml_u2phy->reset_regs = devm_ioremap(dev, (resource_size_t)addr, (resource_size_t)size);
	if (IS_ERR(aml_u2phy->reset_regs))
		return PTR_ERR(aml_u2phy->reset_regs);

	mu2p_dbg(dev, "reset_mem:0x%llx, iomap reset_base:0x%px\n",
						addr, aml_u2phy->reset_regs);

	ret = of_property_read_reg(dev->of_node, 2, &addr, &size);
	if (ret) {
		mu2p_err(dev, "failed to get address 2(id-%d)\n",
			aml_u2phy->phy_id);
		return ret;
	}

	aml_u2phy->usb_phy_trim_reg = devm_ioremap(dev, (resource_size_t)addr,
									(resource_size_t)size);
	if (IS_ERR(aml_u2phy->usb_phy_trim_reg))
		return PTR_ERR(aml_u2phy->usb_phy_trim_reg);

	mu2p_dbg(dev, "usb_phy_trim_reg:0x%llx, iomap usb_phy_trim_reg:0x%px\n",
						addr, aml_u2phy->usb_phy_trim_reg);

	for (i = 0; i < aml_u2phy->portnum; i++) {
		ret = of_property_read_reg(dev->of_node, i + 3, &addr, &size);
		if (ret) {
			mu2p_err(dev, "failed to get address resource %d (id-%d)\n",
				i + 3, aml_u2phy->phy_id);
			return ret;
		}

		aml_u2phy->phy_cfg[i] = devm_ioremap(dev, (resource_size_t)addr,
									(resource_size_t)size);
		if (IS_ERR(aml_u2phy->phy_cfg[i]))
			return PTR_ERR(aml_u2phy->phy_cfg[i]);

		mu2p_dbg(dev, "phy_cfg%d_mem:0x%llx, iomap phy_cfg%d_base:0x%px\n",
							i, addr, i,
							aml_u2phy->phy_cfg[i]);

		aml_u2phy->u2p_aml_regs[i] = aml_u2phy->regs + 0x20 * i;
	}

	ret = of_property_read_u32(dev->of_node, "reset-level", &aml_u2phy->reset_level);
	if (ret < 0)
		aml_u2phy->reset_level = 0x40;

	for (i = 0; i < aml_u2phy->portnum; i++)
		aml_u2phy->phy_reset_level_bit[i] = -1U;

	ret = of_property_read_u32_array(dev->of_node, "phy-reset-level-bits",
								aml_u2phy->phy_reset_level_bit,
								aml_u2phy->portnum);
	if (ret == -EOVERFLOW) {
		mu2p_err(dev, "phy-reset-level-bits should contains %d values\n",
								aml_u2phy->portnum);
	} else if (ret < 0) {
		mu2p_err(dev, "no phy-reset-level-bits? exit.\n");
		return -EINVAL;
	}

	for (i = 0; i < aml_u2phy->portnum; i++)
		mu2p_dbg(dev, "phy%d_reset_level bits: %d\n", i, aml_u2phy->phy_reset_level_bit[i]);

	ret = of_property_read_u32(dev->of_node, "usb-reset-bit", &aml_u2phy->usb_reset_bit);
	if (ret < 0)
		aml_u2phy->usb_reset_bit = -1U;

	mu2p_dbg(dev, "usb reset bit: %d\n", aml_u2phy->usb_reset_bit);

	ret = of_property_read_u32(dev->of_node, "usb-comb-reset-bit",
								&aml_u2phy->usb_comb_reset_bit);
	if (ret < 0)
		aml_u2phy->usb_comb_reset_bit = -1U;

	mu2p_dbg(dev, "usb comb reset bit: %d\n", aml_u2phy->usb_comb_reset_bit);

	cnt = of_property_count_u32_elems(dev->of_node, "pll-settings");
	if (cnt < 0) {
		mu2p_err(dev, "no pll-settings? exit.");
		return -EINVAL;
	}

	ret = of_property_read_u32_array(dev->of_node, "pll-settings",
									aml_u2phy->pll_setting,
									cnt);
	if (ret == -EOVERFLOW) {
		mu2p_err(dev, "pll-settings should contains %d values\n", cnt);
	} else if (ret < 0) {
		mu2p_err(dev, "no pll-settings? exit.\n");
		return -EINVAL;
	}

	for (i = 0; i < 8; i++)
		mu2p_dbg(dev, "pll-settings: %d\n", aml_u2phy->pll_setting[i]);

	ret = of_property_read_s32(dev->of_node, "portspeed", &aml_u2phy->portspeed);
	if (ret < 0)
		aml_u2phy->portspeed = MESON_USB_SPEED_HIGH;

	mu2p_dbg(dev, "portspeed: %d\n", aml_u2phy->portspeed);

	cnt = of_property_count_strings(dev->of_node, "clock-names");
	if (cnt < 0) {
		mu2p_err(dev, "no clks? exit.");
		return -EINVAL;
	}

	mu2p_dbg(dev, "clk num: %d\n", cnt);

	for (i = 0; i < cnt; i++) {
		ret = of_property_read_string_index(dev->of_node, "clock-names",
									i, &aml_u2phy->clks[i].id);
		if (ret < 0) {
			mu2p_err(dev, "read clk-names idx:%d err", i);
			return -EINVAL;
		}
	}

	ret = devm_clk_bulk_get(dev, aml_u2phy->clk_num, aml_u2phy->clks);
	if (ret) {
		mu2p_dbg(dev, "Failed to get usb2 phy bus clocks\n");
		return ret;
	}

	for (i = 0; i < AML_USB_PHY_MAX_CLK_NUMBER; i++)
		mu2p_dbg(dev, "%s %px.\n", aml_u2phy->clks[i].id, (void *)aml_u2phy->clks[i].clk);

	return ret;
}

int meson_aml_u3phy_parse(struct device *dev, struct meson_uphy_instance *instance)
{
	return 0;
}

int meson_m31_u2phy_parse(struct device *dev, struct meson_uphy_instance *instance)
{
	return 0;
}

int meson_m31_u3phy_parse(struct device *dev, struct meson_uphy_instance *instance)
{
	return 0;
}
