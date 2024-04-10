// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/bitfield.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/iio/iio.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>

#define SARADC_REG0				0x00
#define SARADC_REG0_SAMPLING_STOP		BIT(14)
#define SARADC_REG0_ADC_EN			BIT(9)
#define SARADC_REG0_FIFO_CNT_IRQ_MASK		GENMASK(8, 4)
#define SARADC_REG0_FIFO_IRQ_EN			BIT(3)
#define SARADC_REG0_SAMPLE_START		BIT(2)
#define SARADC_REG0_CONTINUOUS_EN		BIT(1)
#define SARADC_REG0_SAMPLING_ENABLE		BIT(0)

#define SARADC_REG1				0x04
#define SARADC_REG1_MAX_INDEX_MASK		GENMASK(26, 24)
#define SARADC_REG1_ENTRY_SHIFT(_chan)		((_chan) * 3)
#define SARADC_REG1_ENTRY_MASK(_chan)		(GENMASK(2, 0) << ((_chan) * 3))

#define SARADC_REG2				0x08
#define SARADC_REG2_AVG_MODE_SHIFT(_chan)	(16 + ((_chan) * 2))
#define SARADC_REG2_AVG_MODE_MASK(_chan)	(GENMASK(17, 16) << ((_chan) * 2))
#define SARADC_REG2_NUM_SAMPLES_SHIFT(_chan)	(0 + ((_chan) * 2))
#define SARADC_REG2_NUM_SAMPLES_MASK(_chan)	(GENMASK(1, 0) << ((_chan) * 2))

#define SARADC_REG3				0x0c
#define SARADC_REG4				0x10
#define SARADC_REG5				0x14
#define SARADC_REG6				0x18
#define SARADC_REG7				0x1c
#define SARADC_REG8				0x20
#define SARADC_REG9				0x24
#define SARADC_REG10				0x28

#define SARADC_REG11				0x2c
#define SARADC_REG11_CHANNEL7_AUX_CTRL_MASK	GENMASK(27, 25)

#define SARADC_REG12				0x30
#define SARADC_REG13				0x34
#define SARADC_REG14				0x38

#define SARADC_STATUS0				0x3c
#define SARADC_STATUS0_BUSY_MASK		GENMASK(14, 12)
#define SARADC_STATUS0_FIFO_COUNT_MASK		GENMASK(7, 3)

#define SARADC_STATUS1				0x40
#define SARADC_STATUS2				0x44
#define SARADC_STATUS3				0x48
#define SARADC_STATUS4				0x4c
#define SARADC_STATUS5				0x50
#define SARADC_STATUS6				0x54
#define SARADC_STATUS7				0x58
#define SARADC_STATUS8				0x5c
#define SARADC_STATUS9				0x60
#define SARADC_STATUS10				0x64
#define SARADC_STATUS11				0x68
#define SARADC_STATUS12				0x6c
#define SARADC_STATUS13				0x70
#define SARADC_STATUS14				0x74
#define SARADC_STATUS15				0x78
#define SARADC_RDY				0x80

#define SARADC_CHANNEL_MAX			8
#define SARADC_MAX_FIFO_SIZE			16

enum amlogic_saradc_avg_mode {
	NO_AVERAGING = 0x0,
	MEAN_AVERAGING = 0x1,
	MEDIAN_AVERAGING = 0x2,
};

enum amlogic_saradc_num_samples {
	ONE_SAMPLE = 0x0,
	TWO_SAMPLES = 0x1,
	FOUR_SAMPLES = 0x2,
	EIGHT_SAMPLES = 0x3,
};

struct amlogic_saradc_priv {
	struct iio_chan_spec iio_channels[SARADC_CHANNEL_MAX];
	struct regmap *regmap;
	struct clk *clk_src;
	struct clk *clk_core;
	struct clk *clk_gate;
	struct clk *clk_mux;
	int info_offset;
	int info_scale;
};

static void amlogic_saradc_set_averaging(struct amlogic_saradc_priv *priv, int address,
					 enum amlogic_saradc_avg_mode mode,
					 enum amlogic_saradc_num_samples samples)
{
	int val;

	val = samples << SARADC_REG2_NUM_SAMPLES_SHIFT(address);
	regmap_update_bits(priv->regmap, SARADC_REG2,
			   SARADC_REG2_NUM_SAMPLES_MASK(address), val);

	val = mode << SARADC_REG2_AVG_MODE_SHIFT(address);
	regmap_update_bits(priv->regmap, SARADC_REG2,
			   SARADC_REG2_AVG_MODE_MASK(address), val);
}

static void amlogic_saradc_enable_channel(struct amlogic_saradc_priv *priv,
					  int address, int index)
{
	u32 regval;

	regval = SARADC_REG1_ENTRY_MASK(index) & (address << (index * 3));
	regmap_update_bits(priv->regmap, SARADC_REG1,
			   SARADC_REG1_ENTRY_MASK(index), regval);
}

static int amlogic_saradc_wait_busy_clear(struct amlogic_saradc_priv *priv)
{
	int regval, timeout = 20000;

	do {
		usleep_range(1, 2);
		regmap_read(priv->regmap, SARADC_STATUS0, &regval);
	} while (FIELD_GET(SARADC_STATUS0_BUSY_MASK, regval) && timeout--);

	if (timeout < 0)
		return -ETIMEDOUT;

	return 0;
}

static void amlogic_saradc_clear_fifo(struct amlogic_saradc_priv *priv)
{
	unsigned int count, tmp;

	for (count = 0; count < SARADC_MAX_FIFO_SIZE; count++) {
		regmap_read(priv->regmap, SARADC_STATUS0, &tmp);
		count = FIELD_GET(SARADC_STATUS0_FIFO_COUNT_MASK, tmp);
		if (!count)
			break;

		regmap_read(priv->regmap, SARADC_STATUS3, &tmp);
	}
}

static int amlogic_saradc_read_fifo(struct amlogic_saradc_priv *priv, int *value)
{
	unsigned int count, tmp;

	regmap_read(priv->regmap, SARADC_STATUS0, &tmp);
	count = FIELD_GET(SARADC_STATUS0_FIFO_COUNT_MASK, tmp);
	if (count != 1)
		return -EINVAL;

	regmap_read(priv->regmap, SARADC_STATUS3, &tmp);

	*value = tmp & 0xffff;

	return 0;
}

static int amlogic_saradc_get_sample(struct iio_dev *indio_dev,
				     const struct iio_chan_spec *chan, int *val)
{
	struct amlogic_saradc_priv *priv = iio_priv(indio_dev);
	int ret;
	int raw;

	mutex_lock(&indio_dev->mlock);

	/* Set channel */
	amlogic_saradc_enable_channel(priv, chan->address, 0);

	amlogic_saradc_clear_fifo(priv);

	/* Start sample */
	regmap_update_bits(priv->regmap, SARADC_REG0,
			   SARADC_REG0_SAMPLE_START,
			   SARADC_REG0_SAMPLE_START);

	ret = amlogic_saradc_wait_busy_clear(priv);
	if (ret)
		goto fail;

	ret = amlogic_saradc_read_fifo(priv, &raw);
	if (ret)
		goto fail;

	*val = (short)raw;

	mutex_unlock(&indio_dev->mlock);

	return IIO_VAL_INT;

fail:
	mutex_unlock(&indio_dev->mlock);
	return ret;
}

static int amlogic_saradc_iio_info_read_raw(struct iio_dev *indio_dev,
					    const struct iio_chan_spec *chan,
					    int *val, int *val2, long mask)
{
	struct amlogic_saradc_priv *priv = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		return amlogic_saradc_get_sample(indio_dev, chan, val);

	case IIO_CHAN_INFO_OFFSET:
		*val = priv->info_offset;
		return IIO_VAL_INT;

	case IIO_CHAN_INFO_SCALE:
		*val = priv->info_scale;
		*val2 = 1000;
		return IIO_VAL_FRACTIONAL;
	}

	return -EINVAL;
}

static int amlogic_saradc_self_calib(struct iio_dev *indio_dev)
{
	struct amlogic_saradc_priv *priv = iio_priv(indio_dev);
	unsigned int regval;
	int raw_gnd;
	int raw_vref;
	int ret;

	/* Set to GND */
	regval = FIELD_PREP(SARADC_REG11_CHANNEL7_AUX_CTRL_MASK, 0);
	regmap_update_bits(priv->regmap, SARADC_REG11,
			   SARADC_REG11_CHANNEL7_AUX_CTRL_MASK, regval);
	usleep_range(10, 20);
	ret = amlogic_saradc_get_sample(indio_dev, &priv->iio_channels[7], &raw_gnd);
	if (ret < 0) {
		dev_err(indio_dev->dev.parent, "self-calibration failed: sample error\n");
		return ret;
	}

	/* Set to VREF */
	regval = FIELD_PREP(SARADC_REG11_CHANNEL7_AUX_CTRL_MASK, 4);
	regmap_update_bits(priv->regmap, SARADC_REG11,
			   SARADC_REG11_CHANNEL7_AUX_CTRL_MASK, regval);
	usleep_range(10, 20);
	ret = amlogic_saradc_get_sample(indio_dev, &priv->iio_channels[7], &raw_vref);
	if (ret < 0) {
		dev_err(indio_dev->dev.parent, "self-calibration failed: sample error\n");
		return ret;
	}

	priv->info_offset = -raw_gnd;
	priv->info_scale = 1800 * 1000 / (raw_vref - raw_gnd);

	if (priv->info_scale <= 0) {
		dev_err(indio_dev->dev.parent, "self-calibration failed: gnd=%d, vref=%d\n",
			raw_gnd, raw_vref);
		return -EINVAL;
	}

	return 0;
}

static int amlogic_saradc_init(struct iio_dev *indio_dev)
{
	struct amlogic_saradc_priv *priv = iio_priv(indio_dev);
	int ret;

	ret = clk_set_parent(priv->clk_mux, priv->clk_src);
	if (ret) {
		dev_err(indio_dev->dev.parent,
			"failed to set mux parent to src\n");
		return ret;
	}

	ret = clk_set_rate(priv->clk_gate, 1200000);
	if (ret) {
		dev_err(indio_dev->dev.parent, "failed to set adc clock rate\n");
		return ret;
	}

	usleep_range(5, 10);

	/* filter Control */
	regmap_write(priv->regmap, SARADC_REG7, 0x00000c21);
	regmap_write(priv->regmap, SARADC_REG8, 0x0280614d);

	/* delay */
	regmap_write(priv->regmap, SARADC_REG3, 0x10a02403);
	regmap_write(priv->regmap, SARADC_REG4, 0x00000080);
	regmap_write(priv->regmap, SARADC_REG5, 0x0010341b);
	regmap_write(priv->regmap, SARADC_REG6, 0x00000031);

	/* set aux and extern vref */
	regmap_write(priv->regmap, SARADC_REG9, 0x0000e4e4);
	regmap_write(priv->regmap, SARADC_REG10, 0x74543414);
	regmap_write(priv->regmap, SARADC_REG11, 0xf4d4b494);

	/* other */
	regmap_write(priv->regmap, SARADC_REG12, 0x00000000);
	regmap_write(priv->regmap, SARADC_REG13, 0x00000000);
	regmap_write(priv->regmap, SARADC_REG14, 0x00000000);

	regmap_write(priv->regmap, SARADC_REG0, 0x00400000);

	amlogic_saradc_set_averaging(priv, 0, MEDIAN_AVERAGING, EIGHT_SAMPLES);

	return 0;
}

static int amlogic_saradc_hw_enable(struct iio_dev *indio_dev)
{
	struct amlogic_saradc_priv *priv = iio_priv(indio_dev);
	int ret;

	mutex_lock(&indio_dev->mlock);

	ret = clk_prepare_enable(priv->clk_core);
	if (ret) {
		dev_err(indio_dev->dev.parent, "failed to enable core clk\n");
		goto err_core;
	}

	ret = clk_prepare_enable(priv->clk_gate);
	if (ret) {
		dev_err(indio_dev->dev.parent, "failed to enable gate clk\n");
		goto err_gate;
	}

	usleep_range(5, 10);

	regmap_update_bits(priv->regmap, SARADC_REG0,
			   SARADC_REG0_ADC_EN,
			   SARADC_REG0_ADC_EN);

	regmap_update_bits(priv->regmap, SARADC_REG0,
			   SARADC_REG0_SAMPLING_ENABLE,
			   SARADC_REG0_SAMPLING_ENABLE);

	regmap_update_bits(priv->regmap, SARADC_REG0,
			   SARADC_REG0_SAMPLING_STOP, 0);

	amlogic_saradc_clear_fifo(priv);

	mutex_unlock(&indio_dev->mlock);

	return 0;

err_gate:
	clk_disable_unprepare(priv->clk_core);
err_core:
	mutex_unlock(&indio_dev->mlock);
	return ret;
}

static int amlogic_saradc_hw_disable(struct iio_dev *indio_dev)
{
	struct amlogic_saradc_priv *priv = iio_priv(indio_dev);

	mutex_lock(&indio_dev->mlock);

	regmap_update_bits(priv->regmap, SARADC_REG0,
			   SARADC_REG0_SAMPLING_STOP,
			   SARADC_REG0_SAMPLING_STOP);
	amlogic_saradc_wait_busy_clear(priv);

	regmap_update_bits(priv->regmap, SARADC_REG0,
			   SARADC_REG0_SAMPLING_ENABLE, 0);

	regmap_update_bits(priv->regmap, SARADC_REG0,
			   SARADC_REG0_ADC_EN, 0);

	clk_disable_unprepare(priv->clk_gate);
	clk_disable_unprepare(priv->clk_core);

	mutex_unlock(&indio_dev->mlock);

	return 0;
}

static const struct iio_info amlogic_saradc_iio_info = {
	.read_raw = amlogic_saradc_iio_info_read_raw,
};

static const struct regmap_config amlogic_saradc_regmap_config = {
	.reg_bits = 32,
	.val_bits = 32,
	.reg_stride = 4,
};

static const struct of_device_id amlogic_saradc_of_match[] = {
	{ .compatible = "amlogic,saradc", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, amlogic_saradc_of_match);

static int amlogic_saradc_probe(struct platform_device *pdev)
{
	struct amlogic_saradc_priv *priv;
	struct iio_dev *indio_dev;
	void __iomem *base;
	int index, ret;

	indio_dev = devm_iio_device_alloc(&pdev->dev, sizeof(*priv));
	if (!indio_dev) {
		dev_err(&pdev->dev, "failed allocating iio device\n");
		return -ENOMEM;
	}
	priv = iio_priv(indio_dev);

	indio_dev->name = dev_name(&pdev->dev);
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->info = &amlogic_saradc_iio_info;

	/* initialize the structure for each channel */
	for (index = 0; index < SARADC_CHANNEL_MAX; index++) {
		priv->iio_channels[index].type = IIO_VOLTAGE;
		priv->iio_channels[index].indexed = 1;
		priv->iio_channels[index].channel = index;
		priv->iio_channels[index].address = index;
		priv->iio_channels[index].info_mask_separate =
					BIT(IIO_CHAN_INFO_RAW);
		priv->iio_channels[index].info_mask_shared_by_type =
					BIT(IIO_CHAN_INFO_SCALE) |
					BIT(IIO_CHAN_INFO_OFFSET);
	}
	indio_dev->channels = priv->iio_channels;
	indio_dev->num_channels = SARADC_CHANNEL_MAX;

	base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(base))
		return PTR_ERR(base);

	priv->regmap = devm_regmap_init_mmio(&pdev->dev, base,
					     &amlogic_saradc_regmap_config);
	if (IS_ERR(priv->regmap))
		return PTR_ERR(priv->regmap);

	priv->clk_src = devm_clk_get(&pdev->dev, "src");
	if (IS_ERR(priv->clk_src)) {
		dev_err(&pdev->dev, "failed to get src clk\n");
		return PTR_ERR(priv->clk_src);
	}

	priv->clk_core = devm_clk_get(&pdev->dev, "core");
	if (IS_ERR(priv->clk_core)) {
		dev_err(&pdev->dev, "failed to get core clk\n");
		return PTR_ERR(priv->clk_core);
	}

	priv->clk_gate = devm_clk_get(&pdev->dev, "gate");
	if (IS_ERR(priv->clk_gate)) {
		if (PTR_ERR(priv->clk_gate) == -ENOENT) {
			priv->clk_gate = NULL;
		} else {
			dev_err(&pdev->dev, "failed to get gate clk\n");
			return PTR_ERR(priv->clk_gate);
		}
	}

	priv->clk_mux = devm_clk_get(&pdev->dev, "mux");
	if (IS_ERR(priv->clk_mux)) {
		if (PTR_ERR(priv->clk_mux) == -ENOENT) {
			priv->clk_mux = NULL;
		} else {
			dev_err(&pdev->dev, "failed to get mux clk\n");
			return PTR_ERR(priv->clk_mux);
		}
	}

	ret = amlogic_saradc_init(indio_dev);
	if (ret)
		goto err;

	ret = amlogic_saradc_hw_enable(indio_dev);
	if (ret)
		goto err;

	platform_set_drvdata(pdev, indio_dev);

	ret = amlogic_saradc_self_calib(indio_dev);
	if (ret)
		goto err_hw;

	ret = iio_device_register(indio_dev);
	if (ret)
		goto err_hw;

	return 0;

err_hw:
	amlogic_saradc_hw_disable(indio_dev);
err:
	return ret;
}

static int amlogic_saradc_remove(struct platform_device *pdev)
{
	struct iio_dev *indio_dev = platform_get_drvdata(pdev);

	iio_device_unregister(indio_dev);

	return amlogic_saradc_hw_disable(indio_dev);
}

static int __maybe_unused amlogic_saradc_suspend(struct device *dev)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);

	return amlogic_saradc_hw_disable(indio_dev);
}

static int __maybe_unused amlogic_saradc_resume(struct device *dev)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);

	return amlogic_saradc_hw_enable(indio_dev);
}

static SIMPLE_DEV_PM_OPS(amlogic_saradc_pm_ops,
			 amlogic_saradc_suspend, amlogic_saradc_resume);

static struct platform_driver amlogic_saradc_driver = {
	.probe		= amlogic_saradc_probe,
	.remove		= amlogic_saradc_remove,
	.driver		= {
		.name	= "amlogic-saradc",
		.of_match_table = amlogic_saradc_of_match,
		.pm = &amlogic_saradc_pm_ops,
	},
};

int __init amlogic_saradc_driver_init(void)
{
	return platform_driver_register(&amlogic_saradc_driver);
}

void __exit amlogic_saradc_driver_exit(void)
{
	platform_driver_unregister(&amlogic_saradc_driver);
}

MODULE_AUTHOR("Huqiang Qin <huqiang.qin@amlogic.com>");
MODULE_DESCRIPTION("Amlogic SAR ADC driver");
MODULE_LICENSE("GPL v2");
