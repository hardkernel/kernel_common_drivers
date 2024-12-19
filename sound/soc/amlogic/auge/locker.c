// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2021 Amlogic, Inc. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/amlogic/iomap.h>
#include <linux/time.h>
#include <linux/clk.h>

#include "locker.h"
#include "regs.h"
#include "iomap.h"
#include "locker_hw.h"
#include "../common/iomapres.h"

#define DRV_NAME "audiolocker"

enum locker_src {
	LOCKER_INVAL,
	LOCKER_TDM_A,
	LOCKER_TDM_B,
	LOCKER_TDM_C,
	LOCKER_TDM_D,
	LOCKER_SPDIF,
	LOCKER_SPDIF_B,
	LOCKER_EARCRX,
	LOCKER_ARCRX,
	LOCKER_SPDIFIN,
};

struct audio_locker_clk_cnt {
	unsigned int input_cnt;
	unsigned int output_cnt;
};

struct audiolocker {
	struct device *dev;
	struct clk *lock_out;
	struct clk *lock_in;
	struct clk *earcrx_clk;
	struct clk *arcrx_find_y;
	struct clk *speaker_clk;
	struct regmap *regmap;
	struct audio_locker_clk_cnt clk_cnt;
	enum locker_src in_src;
	enum locker_src out_src;
	int duration;
	bool enable;
	bool prepared;
};

struct audiolocker *s_locker;

static struct audiolocker *get_locker(void)
{
	struct audiolocker *p_locker;

	p_locker = s_locker;

	if (!p_locker) {
		pr_debug("Not init vad\n");
		return NULL;
	}

	return p_locker;
}

static int locker_set_duration(struct regmap *regmap, int duration)
{
	int ref_latch_time_s = 500000000 / 3 * duration;

	audiolocker_refclk_latch(regmap, ref_latch_time_s);
	return 0;
}

static int locker_init(struct regmap *regmap)
{
	audiolocker_refclk_downsample_step(regmap, 0);
	audiolocker_imclk_downsample_step(regmap, 0);
	audiolocker_omclk_downsample_step(regmap, 0);

	/* TODO: chip info set pclk source, sc2 set 1 for sysclk */
	audiolocker_set_refclk_src(regmap, 1);

	audiolocker_latch(regmap);
	audiolocker_interrupt_mask(regmap, 0xe);
	return 0;
}

static int locker_in_src_set(struct audiolocker *locker, enum locker_src in_src)
{
	int ret = 0;

	switch (in_src) {
	case LOCKER_TDM_A:
	case LOCKER_TDM_B:
	case LOCKER_TDM_C:
	case LOCKER_TDM_D:
		pr_info("%s:hdmirx in\n", __func__);
		break;
	case LOCKER_SPDIF:
	case LOCKER_SPDIF_B:
		break;
	case LOCKER_ARCRX:
		if (!IS_ERR(locker->arcrx_find_y)) {
			ret = clk_set_parent(locker->lock_in, locker->arcrx_find_y);
			if (ret < 0)
				dev_err(locker->dev, "%s clk err!\n", __func__);
			pr_info("%s:arc in\n", __func__);
		}
		audiolocker_imclk_downsample_step(locker->regmap, 0);
		break;
	case LOCKER_EARCRX:
		if (!IS_ERR(locker->earcrx_clk)) {
			ret = clk_set_parent(locker->lock_in, locker->earcrx_clk);
			if (ret < 0)
				dev_err(locker->dev, "%s clk err!\n", __func__);
			pr_info("%s:earcrx in\n", __func__);
		}
		audiolocker_imclk_downsample_step(locker->regmap, 128 - 1);
		break;
	case LOCKER_INVAL:
	default:
		pr_info("%s: locker not support src: %d\n", __func__, in_src);
		break;
	}

	locker->in_src = in_src;
	return 0;
}

static int locker_in_src_get(struct audiolocker *locker)
{
	if (!locker)
		return 0;

	return locker->in_src;
}

static int locker_out_src_set(struct audiolocker *locker, enum locker_src out_src)
{
	int ret = 0;

	if (!locker)
		return 0;

	switch (out_src) {
	case LOCKER_TDM_A:
	case LOCKER_TDM_B:
	case LOCKER_TDM_C:
	case LOCKER_TDM_D:
		if (!IS_ERR(locker->speaker_clk)) {
			ret = clk_set_parent(locker->lock_out, locker->speaker_clk);
			if (ret < 0)
				dev_err(locker->dev, "%s clk err!\n", __func__);
		}
		audiolocker_omclk_downsample_step(locker->regmap, 256 - 1);

		break;
	case LOCKER_INVAL:
	default:
		pr_info("%s: locker not support src: %d\n", __func__, out_src);
		break;
	}

	locker->out_src = out_src;
	return 0;
}

static int locker_out_src_get(struct audiolocker *locker)
{
	if (!locker)
		return 0;
	return locker->out_src;
}

static void audio_locker_enable(struct audiolocker *locker, int enable)
{
	if (enable) {
		clk_prepare_enable(locker->lock_in);
		clk_prepare_enable(locker->lock_out);

		locker_set_duration(locker->regmap, locker->duration);
		audiolocker_enable(locker->regmap, 1);
		audiolocker_sw_reset(locker->regmap);
	} else {
		audiolocker_enable(locker->regmap, 0);
		clk_disable_unprepare(locker->lock_in);
		clk_disable_unprepare(locker->lock_out);
	}

	locker->enable = enable;
}

static int audio_locker_is_enable(struct audiolocker *locker)
{
	return locker->enable;
}

static int audio_locker_duration_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct audiolocker *locker = snd_kcontrol_chip(kcontrol);

	ucontrol->value.integer.value[0] = locker->duration;
	return 0;
}

static int audio_locker_duration_set(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct audiolocker *locker = snd_kcontrol_chip(kcontrol);
	int duration = ucontrol->value.integer.value[0];

	if (duration < 0 || duration > 1000) {
		pr_err("%s, invalid duration: %d [Range:0~1000]", __func__, duration);
		return 0;
	}

	locker_set_duration(locker->regmap, duration);
	locker->duration = duration;
	return 0;
}

static const char *const audio_locker_texts[] = {
	"Disable",
	"Enable",
};

static const struct soc_enum audio_locker_enum =
	SOC_ENUM_SINGLE(SND_SOC_NOPM, 0, ARRAY_SIZE(audio_locker_texts),
			audio_locker_texts);

static const char *const audio_locker_src_texts[] = {
	"INVAL",
	"TDM-A",
	"TDM-B",
	"TDM-C",
	"TDM-D",
	"SPDIF",
	"SPDIF-B",
	"EARCRX",
	"ARCRX",
	"SPDIFIN"
};

static const struct soc_enum audio_locker_src_enum =
	SOC_ENUM_SINGLE(SND_SOC_NOPM, 0, ARRAY_SIZE(audio_locker_src_texts),
			audio_locker_src_texts);

static int audio_locker_get_enable(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct audiolocker *locker = snd_kcontrol_chip(kcontrol);

	ucontrol->value.enumerated.item[0] = audio_locker_is_enable(locker);
	return 0;
}

static int audio_locker_set_enable(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct audiolocker *locker = snd_kcontrol_chip(kcontrol);
	int enable = ucontrol->value.enumerated.item[0];

	audio_locker_enable(locker, enable);
	return 0;
}

static int audio_locker_get_reset(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.enumerated.item[0] = 0;
	return 0;
}

static int audio_locker_reset(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct audiolocker *locker = snd_kcontrol_chip(kcontrol);
	int enable = ucontrol->value.enumerated.item[0];

	if (enable)
		audiolocker_sw_reset(locker->regmap);

	return 0;
}

static int audio_locker_get_in_src(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct audiolocker *locker = snd_kcontrol_chip(kcontrol);

	ucontrol->value.enumerated.item[0] = locker_in_src_get(locker);
	return 0;
}

static int audio_locker_set_in_src(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct audiolocker *locker = snd_kcontrol_chip(kcontrol);
	int in_src = ucontrol->value.enumerated.item[0];

	locker_in_src_set(locker, in_src);
	return 0;
}

static int audio_locker_get_out_src(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct audiolocker *locker = snd_kcontrol_chip(kcontrol);

	ucontrol->value.enumerated.item[0] = locker_out_src_get(locker);
	return 0;
}

static int audio_locker_set_out_src(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct audiolocker *locker = snd_kcontrol_chip(kcontrol);
	int out_src = ucontrol->value.enumerated.item[0];

	locker_out_src_set(locker, out_src);
	return 0;
}

static int audio_locker_clk_cnt_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct audiolocker *locker = snd_kcontrol_chip(kcontrol);
	char *val = (char *)ucontrol->value.bytes.data;
	const void *p = &locker->clk_cnt;

	memcpy(val, p, sizeof(struct audio_locker_clk_cnt));
	return 0;
}

static const struct snd_kcontrol_new locker_controls[] = {
	SOC_SINGLE_EXT("audio locker duration",
		     SND_SOC_NOPM, 0,
		     1000, 0,
		     audio_locker_duration_get,
		     audio_locker_duration_set),

	/* audio locker */
	SOC_ENUM_EXT("audio locker enable",
		     audio_locker_enum,
		     audio_locker_get_enable,
		     audio_locker_set_enable),

	SOC_ENUM_EXT("audio locker sw reset",
		     audio_locker_enum,
		     audio_locker_get_reset,
		     audio_locker_reset),

	/* audio locker in src */
	SOC_ENUM_EXT("audio locker in src",
		     audio_locker_src_enum,
		     audio_locker_get_in_src,
		     audio_locker_set_in_src),

	/* audio locker out src */
	SOC_ENUM_EXT("audio locker out src",
		     audio_locker_src_enum,
		     audio_locker_get_out_src,
		     audio_locker_set_out_src),

	SND_SOC_BYTES_EXT("locker clock count",
			sizeof(struct audio_locker_clk_cnt),
			audio_locker_clk_cnt_get, NULL)
};

static int locker_update_count(struct audiolocker *locker)
{
	struct audio_locker_clk_cnt *clk_cnt = &locker->clk_cnt;

	if (mmio_read(locker->regmap, RO_AUD_LOCK_INT_STATUS) == 3) {
		clk_cnt->input_cnt = mmio_read(locker->regmap, RO_REF2IMCLK_CNT_L);
		clk_cnt->output_cnt = mmio_read(locker->regmap, RO_REF2OMCLK_CNT_L);
		pr_debug("in_count:%d, out_count:%d\n",
			clk_cnt->input_cnt, clk_cnt->output_cnt);
	}

	/* audiolocker_sw_reset(locker->regmap); */
	mmio_write(locker->regmap, AUD_LOCK_INT_CLR, 0x3);
	return 0;
}

static irqreturn_t locker_count_isr_handler(int irq, void *data)
{
	struct audiolocker *locker = (struct audiolocker *)data;

	locker_update_count(locker);
	return IRQ_HANDLED;
}

static const struct of_device_id audiolocker_device_id[] = {
	{ .compatible = "amlogic, audiolocker" },
	{}
};

MODULE_DEVICE_TABLE(of, audiolocker_device_id);

static int audio_locker_of_get_clks(struct audiolocker *locker)
{
	struct device *dev = locker->dev;
	int ret = 0;

	locker->lock_in = devm_clk_get(dev, "lock_in");
	if (IS_ERR(locker->lock_in)) {
		dev_err(dev, "Can't retrieve lock_in clock\n");
		ret = PTR_ERR(locker->lock_in);
	}

	locker->lock_out = devm_clk_get(dev, "lock_out");
	if (IS_ERR(locker->lock_out)) {
		dev_err(dev, "Can't retrieve lock_out clock\n");
		ret = PTR_ERR(locker->lock_out);
	}

	locker->earcrx_clk = devm_clk_get(dev, "earcrx_clk");
	if (IS_ERR(locker->earcrx_clk)) {
		dev_err(dev, "Can't retrieve earcrx clock\n");
		ret = PTR_ERR(locker->earcrx_clk);
	}

	locker->speaker_clk = devm_clk_get(dev, "speaker_clk");
	if (IS_ERR(locker->speaker_clk)) {
		dev_err(dev, "Can't retrieve speaker_clk\n");
		ret = PTR_ERR(locker->speaker_clk);
	}

	locker->arcrx_find_y = devm_clk_get(dev, "arcrx_find_y");
	if (IS_ERR(locker->arcrx_find_y)) {
		dev_err(dev, "Can't retrieve in_src clock\n");
		ret = PTR_ERR(locker->arcrx_find_y);
	}

	return ret;
}

static int audiolocker_platform_probe(struct platform_device *pdev)
{
	struct audiolocker *locker = NULL;
	int ret = 0;
	int irq;

	locker = devm_kzalloc(&pdev->dev,
				     sizeof(struct audiolocker),
				     GFP_KERNEL);
	s_locker = locker;
	if (!locker)
		return -ENOMEM;

	locker->regmap = regmap_resource(&pdev->dev, "locker_reg");
	if (IS_ERR(locker->regmap)) {
		dev_err(&pdev->dev, "Can't get locker regmap!!\n");
		return PTR_ERR(locker->regmap);
	}

	locker->dev = &pdev->dev;
	audio_locker_of_get_clks(locker);

	/* irq */
	irq = platform_get_irq_byname(pdev, "irq");
	if (irq < 0) {
		dev_err(&pdev->dev,	"Can't get irq number\n");
		return -EINVAL;
	}

	ret = devm_request_irq(&pdev->dev, irq,
			  locker_count_isr_handler,
			  IRQF_SHARED,
			  "audiolocker",
			  locker);
	if (ret < 0) {
		dev_err(&pdev->dev,
			"audio audiolocker irq register fail, ret: %d\n", ret);
		return -EINVAL;
	}

	locker->prepared = true;
	dev_set_drvdata(&pdev->dev, locker);

	return 0;
}

static struct platform_driver audiolocker_platform_driver = {
	.driver = {
		.name  = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(audiolocker_device_id),
	},
	.probe  = audiolocker_platform_probe,
};

int card_add_locker_kcontrols(struct snd_soc_card *card)
{
	struct audiolocker *locker = get_locker();
	unsigned int idx;
	int err;

	if (!locker)
		return -ENODEV;
	if (!locker->prepared)
		return -ENODEV;

	locker_init(locker->regmap);

	for (idx = 0; idx < ARRAY_SIZE(locker_controls); idx++) {
		err = snd_ctl_add(card->snd_card,
				snd_ctl_new1(&locker_controls[idx],
				locker));
		if (err < 0)
			return err;
	}

	return 0;
}

int __init audio_locker_init(void)
{
	return platform_driver_register(&(audiolocker_platform_driver));
}

void __exit audio_locker_exit(void)
{
	platform_driver_unregister(&audiolocker_platform_driver);
}

#ifndef MODULE
module_init(audio_locker_init);
module_exit(audio_locker_exit);
#endif
