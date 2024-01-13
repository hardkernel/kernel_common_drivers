// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2023 Amlogic, Inc. All rights reserved.
 */

#include "meson_saradc_m8.h"
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>

#define MESON_SAR_ADC_REG11					0x2c
	#define MESON_SAR_ADC_REG11_PD_IRQ_OUT			BIT(26)
	#define MESON_SAR_ADC_REG11_PD_IRQ_CLR			BIT(25)
	#define MESON_SAR_ADC_REG11_PD_IRQ_EN			BIT(24)
	#define MESON_SAR_ADC_REG11_PD_DETECT_TRIM_MASK		GENMASK(14, 9)
	#define MESON_SAR_ADC_REG11_PD_DETECT_EN		BIT(3)
	#define MESON_SAR_ADC_REG11_PD_DETECT_CTRL_MASK		GENMASK(2, 1)

#define MESON_SAR_ADC_CHAN(_chan) {					\
	.type = IIO_VOLTAGE,						\
	.indexed = 1,							\
	.channel = _chan,						\
	.address = _chan,						\
	.scan_index = _chan,						\
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |			\
			      BIT(IIO_CHAN_INFO_AVERAGE_RAW) |		\
			      BIT(IIO_CHAN_INFO_PROCESSED),		\
	.info_mask_shared_by_type = BIT(IIO_CHAN_INFO_SCALE),		\
	.scan_type = {							\
		.sign = 'u',						\
		.storagebits = 16,					\
		.shift = 0,						\
		.endianness = IIO_CPU,					\
	},								\
	.datasheet_name = "SAR_ADC_CH"#_chan,				\
}

static struct iio_chan_spec meson_s7_sar_adc_iio_channels[] = {
	MESON_SAR_ADC_CHAN(0),
	MESON_SAR_ADC_CHAN(1),
	MESON_SAR_ADC_CHAN(2),
	MESON_SAR_ADC_CHAN(3),
	MESON_SAR_ADC_CHAN(4),
	MESON_SAR_ADC_CHAN(5),
	MESON_SAR_ADC_CHAN(6),
	MESON_SAR_ADC_CHAN(7),
	IIO_CHAN_SOFT_TIMESTAMP(8),
};

static irqreturn_t meson_s7_sar_adc_pd_irq(int irq, void *data)
{
	struct iio_dev *indio_dev = data;
	struct meson_sar_adc_priv *priv = iio_priv(indio_dev);
	struct device *pdev = indio_dev->dev.parent;
	u32 regval;

	regmap_read(priv->regmap, MESON_SAR_ADC_REG11, &regval);
	if (regval & MESON_SAR_ADC_REG11_PD_IRQ_OUT) {
		dev_warn(pdev, "DETECTED DOWN\n");
		/* Do something */
	}

	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG11,
			   MESON_SAR_ADC_REG11_PD_IRQ_CLR,
			   MESON_SAR_ADC_REG11_PD_IRQ_CLR);

	return IRQ_HANDLED;
}

static int meson_s7_sar_adc_extra_init(struct iio_dev *indio_dev)
{
	int regval, pd_irq, ret;
	bool pd_state;
	u32 pd_ctrl, pd_trim;
	struct meson_sar_adc_priv *priv = iio_priv(indio_dev);
	struct device_node *pd_node;
	struct device *pdev = indio_dev->dev.parent;

	/* Disable internal ring counter */
	meson_m8_sar_adc_disable_ring(priv);

	/* PD node */
	pd_node = of_find_node_by_name(pdev->of_node, "pd");
	if (!pd_node) {
		dev_info(pdev, "power outage detection config property not found\n");
		goto without_pd;
	}

	/* Ctrl property */
	ret = of_property_read_u32(pd_node, "ctrl", &pd_ctrl);
	if (ret) {
		dev_err(pdev, "failed with error %d, need to check ctrl property\n", ret);
		return -EINVAL;
	}
	if (pd_ctrl & ~0x3) {
		dev_warn(pdev, "ctrl exceeds the maximum value!\n");
		pd_ctrl = 0x3;
	}
	dev_dbg(pdev, "pd_ctrl: %u\n", pd_ctrl);

	/* Trim property */
	ret = of_property_read_u32(pd_node, "trim", &pd_trim);
	if (ret) {
		dev_err(pdev, "failed with error %d, need to check trim property\n", ret);
		return -EINVAL;
	}
	if (pd_trim & ~0x3f) {
		dev_warn(pdev, "trim exceeds the maximum value!\n");
		pd_trim = 0x3f;
	}
	dev_dbg(pdev, "pd_trim: %u\n", pd_trim);

	/* Status property */
	pd_state = of_property_read_bool(pd_node, "enabled");
	dev_dbg(pdev, "pd_state: %s\n", pd_state ? "enabled" : "disabled");

	regval = FIELD_PREP(MESON_SAR_ADC_REG11_PD_DETECT_CTRL_MASK, pd_ctrl);
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG11,
			   MESON_SAR_ADC_REG11_PD_DETECT_CTRL_MASK,
			   regval);

	regval = FIELD_PREP(MESON_SAR_ADC_REG11_PD_DETECT_TRIM_MASK, pd_trim);
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG11,
			   MESON_SAR_ADC_REG11_PD_DETECT_TRIM_MASK,
			   regval);

	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG11,
			   MESON_SAR_ADC_REG11_PD_DETECT_EN,
			   pd_state ? MESON_SAR_ADC_REG11_PD_DETECT_EN : 0);

	/* Clear PD IRQ */
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG11,
			   MESON_SAR_ADC_REG11_PD_IRQ_CLR,
			   MESON_SAR_ADC_REG11_PD_IRQ_CLR);

	pd_irq = irq_of_parse_and_map(pdev->of_node, 1);
	if (!pd_irq)
		return -EINVAL;

	ret = devm_request_irq(pdev, pd_irq, meson_s7_sar_adc_pd_irq,
			       IRQF_SHARED, dev_name(pdev), indio_dev);
	if (ret)
		return ret;

	/* Enalbe PD IRQ */
	regmap_update_bits(priv->regmap, MESON_SAR_ADC_REG11,
			   MESON_SAR_ADC_REG11_PD_IRQ_EN,
			   MESON_SAR_ADC_REG11_PD_IRQ_EN);

	dev_dbg(pdev, "init done, pd_irq: %d\n", pd_irq);

without_pd:
	return 0;
}

static const struct meson_sar_adc_diff_ops meson_s7_diff_ops = {
	.extra_init = meson_s7_sar_adc_extra_init,
	.set_test_input = meson_m8_sar_adc_set_test_input,
	.read_fifo = meson_m8_sar_adc_read_fifo,
	.enable_chnl = meson_m8_sar_adc_enable_chnl,
	.read_chnl = meson_m8_sar_adc_read_chnl,
};

static const struct meson_sar_adc_calib meson_sar_adc_calib_s7 = {
	.test_upper = TEST_MUX_VDD,
	.test_lower = TEST_MUX_VSS,
	.test_channel = 4,
};

const struct meson_sar_adc_param meson_sar_adc_s7_param __initconst = {
	.has_bl30_integration = false,
	.clock_rate = 1200000,
	.regmap_config = &meson_sar_adc_regmap_config_g12a,
	.resolution = 10,
	.disable_ring_counter = 0,
	.dops = &meson_s7_diff_ops,
	.channels = meson_s7_sar_adc_iio_channels,
	.num_channels = ARRAY_SIZE(meson_s7_sar_adc_iio_channels),
	.calib = &meson_sar_adc_calib_s7,
};
