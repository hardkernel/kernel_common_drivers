// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * PWM controller driver for Amlogic Meson SoCs.
 *
 * This PWM is only a set of Gates, Dividers and Counters:
 * PWM output is achieved by calculating a clock that permits calculating
 * two periods (low and high). The counter then has to be set to switch after
 * N cycles for the first half period.
 * The hardware has no "polarity" setting. This driver reverses the period
 * cycles (the low length is inverted with the high length) for
 * PWM_POLARITY_INVERSED. This means that .get_state cannot read the polarity
 * from the hardware.
 * Setting the duty cycle will disable and re-enable the PWM output.
 * Disabling the PWM stops the output immediately (without waiting for the
 * current period to complete first).
 *
 * The public S912 (GXM) datasheet contains some documentation for this PWM
 * controller starting on page 543:
 * https://dl.khadas.com/Hardware/VIM2/Datasheet/S912_Datasheet_V0.220170314publicversion-Wesion.pdf
 * An updated version of this IP block is found in S922X (G12B) SoCs. The
 * datasheet contains the description for this IP block revision starting at
 * page 1084:
 * https://dn.odroid.com/S922X/ODROID-N2/Datasheet/S922X_Public_Datasheet_V0.2.pdf
 *
 * Copyright (c) 2016 BayLibre, SAS.
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 * Copyright (C) 2014 Amlogic, Inc.
 */
#include <linux/bitfield.h>
#include <linux/bits.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/math64.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/arm-smccc.h>
#include <linux/amlogic/secure_pwm_i2c.h>

enum sec_pwm {
	SECID_PWM_ENABLE = 0,
	SECID_PWM_DISABLE,
	SECID_PWM_CONSTANT_EN,
	SECID_PWM_CONSTANT_DIS,
};

#define REG_PWM			0x0
#define PWM_LOW_MASK		GENMASK(15, 0)
#define PWM_HIGH_MASK		GENMASK(31, 16)
#define PWM_MISC_REG		0x8
#define MISC_EN			BIT(0)
#define MISC_CONSTANT		BIT(28)
#define MESON_NUM_PWMS		1

struct meson_pwm_tee_channel {
	unsigned long rate;
	unsigned int hi;
	unsigned int lo;
	struct clk *clk;
};

struct meson_pwm_tee {
	struct pwm_chip chip;
	struct meson_pwm_tee_channel channels[MESON_NUM_PWMS];
	void __iomem *base;
	u32 tee_id;
	/*
	 * Protects register (write) access to the PWM_MISC_REG register
	 * that is shared between the two PWMs.
	 */
	spinlock_t lock;
};

static inline struct meson_pwm_tee *to_meson_pwm_tee(struct pwm_chip *chip)
{
	return container_of(chip, struct meson_pwm_tee, chip);
}

static int meson_pwm_tee_request(struct pwm_chip *chip, struct pwm_device *pwm)
{
	struct meson_pwm_tee *meson = to_meson_pwm_tee(chip);
	struct meson_pwm_tee_channel *channel = &meson->channels[pwm->hwpwm];
	struct device *dev = chip->dev;
	int err;

	err = clk_prepare_enable(channel->clk);
	if (err < 0) {
		dev_err(dev, "failed to enable clock %s: %d\n",
			__clk_get_name(channel->clk), err);
		return err;
	}

	return 0;
}

static void meson_pwm_tee_free(struct pwm_chip *chip, struct pwm_device *pwm)
{
	struct meson_pwm_tee *meson = to_meson_pwm_tee(chip);
	struct meson_pwm_tee_channel *channel = &meson->channels[pwm->hwpwm];

	clk_disable_unprepare(channel->clk);
}

static int pwm_constant_enable_tee(struct meson_pwm_tee *meson, struct pwm_device *pwm)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SECURE_PWM_I2C, SECID_PWM, meson->tee_id,
				 SECID_PWM_CONSTANT_EN, 0, 0, 0, 0, &res);

	return 0;
}

static int pwm_constant_disable_tee(struct meson_pwm_tee *meson, struct pwm_device *pwm)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SECURE_PWM_I2C, SECID_PWM, meson->tee_id,
				 SECID_PWM_CONSTANT_DIS, 0, 0, 0, 0, &res);

	return 0;
}

static int meson_pwm_tee_calc(struct meson_pwm_tee *meson, struct pwm_device *pwm,
			  const struct pwm_state *state)
{
	struct meson_pwm_tee_channel *channel = &meson->channels[pwm->hwpwm];
	unsigned int cnt, duty_cnt;
	unsigned long fin_freq;
	u64 duty, period, freq;

	duty = state->duty_cycle;
	period = state->period;

	/*
	 * Note this is wrong. The result is an output wave that isn't really
	 * inverted and so is wrongly identified by .get_state as normal.
	 * Fixing this needs some care however as some machines might rely on
	 * this.
	 */
	if (state->polarity == PWM_POLARITY_INVERSED)
		duty = period - duty;

	freq = div64_u64(NSEC_PER_SEC * 0xffffULL, period);
	if (freq > ULONG_MAX)
		freq = ULONG_MAX;

	fin_freq = clk_round_rate(channel->clk, freq);
	if (fin_freq == 0) {
		dev_err(meson->chip.dev, "invalid source clock frequency\n");
		return -EINVAL;
	}

	dev_dbg(meson->chip.dev, "fin_freq: %lu Hz\n", fin_freq);

	cnt = DIV64_U64_ROUND_CLOSEST(fin_freq * period, NSEC_PER_SEC);
	if (cnt > 0xffff) {
		dev_err(meson->chip.dev, "unable to get period cnt\n");
		return -EINVAL;
	}

	dev_dbg(meson->chip.dev, "period=%llu cnt=%u\n", period, cnt);

	if (duty == period) {
		channel->hi = cnt;
		channel->lo = 0;
	} else if (duty == 0) {
		channel->hi = 0;
		channel->lo = cnt;
	} else {
		duty_cnt = DIV64_U64_ROUND_CLOSEST(fin_freq * duty, NSEC_PER_SEC);

		dev_dbg(meson->chip.dev, "duty=%llu duty_cnt=%u\n", duty, duty_cnt);
		if (duty_cnt == 0)
			duty_cnt++;

		channel->hi = duty_cnt - 1;
		channel->lo = cnt - duty_cnt - 1;
	}
	if (duty == period || duty == 0)
		pwm_constant_enable_tee(meson, pwm);
	else
		pwm_constant_disable_tee(meson, pwm);
	channel->rate = fin_freq;

	return 0;
}

static void meson_pwm_tee_enable(struct meson_pwm_tee *meson, struct pwm_device *pwm)
{
	struct meson_pwm_tee_channel *channel = &meson->channels[pwm->hwpwm];
	int err;
	u32 value;
	struct arm_smccc_res res;

	err = clk_set_rate(channel->clk, channel->rate);
	if (err)
		dev_err(meson->chip.dev, "setting clock rate failed\n");
	value = FIELD_PREP(PWM_HIGH_MASK, channel->hi) |
		FIELD_PREP(PWM_LOW_MASK, channel->lo);
	arm_smccc_smc(SECURE_PWM_I2C, SECID_PWM, meson->tee_id,
				 SECID_PWM_ENABLE, value, 0, 0, 0, &res);
}

static void meson_pwm_tee_disable(struct meson_pwm_tee *meson, struct pwm_device *pwm)
{
	struct arm_smccc_res res;

	arm_smccc_smc(SECURE_PWM_I2C, SECID_PWM, meson->tee_id,
				 SECID_PWM_DISABLE, 0, 0, 0, 0, &res);
}

static int meson_pwm_tee_apply(struct pwm_chip *chip, struct pwm_device *pwm,
			   const struct pwm_state *state)
{
	struct meson_pwm_tee *meson = to_meson_pwm_tee(chip);
	struct meson_pwm_tee_channel *channel = &meson->channels[pwm->hwpwm];
	int err = 0;

	if (!state->enabled) {
		if (state->polarity == PWM_POLARITY_INVERSED) {
			/*
			 * This IP block revision doesn't have an "always high"
			 * setting which we can use for "inverted disabled".
			 * Instead we achieve this by setting mux parent with
			 * highest rate and minimum divider value, resulting
			 * in the shortest possible duration for one "count"
			 * and "period == duty_cycle". This results in a signal
			 * which is LOW for one "count", while being HIGH for
			 * the rest of the (so the signal is HIGH for slightly
			 * less than 100% of the period, but this is the best
			 * we can achieve).
			 */
			channel->rate = ULONG_MAX;
			channel->hi = ~0;
			channel->lo = 0;

			meson_pwm_tee_enable(meson, pwm);
		} else {
			meson_pwm_tee_disable(meson, pwm);
		}
	} else {
		err = meson_pwm_tee_calc(meson, pwm, state);
		if (err < 0)
			return err;

		meson_pwm_tee_enable(meson, pwm);
	}

	return 0;
}

static u64 meson_pwm_tee_cnt_to_ns(struct pwm_chip *chip, struct pwm_device *pwm,
			       u32 cnt)
{
	struct meson_pwm_tee *meson = to_meson_pwm_tee(chip);
	struct meson_pwm_tee_channel *channel;
	unsigned long fin_freq;

	/* to_meson_pwm_tee() can only be used after .get_state() is called */
	channel = &meson->channels[pwm->hwpwm];

	fin_freq = clk_get_rate(channel->clk);
	if (fin_freq == 0)
		return 0;

	return DIV_ROUND_CLOSEST_ULL(NSEC_PER_SEC * (u64)cnt, fin_freq);
}

static void meson_pwm_tee_get_state(struct pwm_chip *chip, struct pwm_device *pwm,
			       struct pwm_state *state)
{
	struct meson_pwm_tee *meson = to_meson_pwm_tee(chip);
	struct meson_pwm_tee_channel *channel;
	u32 value;
	bool constant_enabled;

	if (!state)
		return;

	channel = &meson->channels[pwm->hwpwm];

	value = readl(meson->base + PWM_MISC_REG);
	state->enabled = value & MISC_EN;
	constant_enabled = (value & MISC_CONSTANT) != 0;
	value = readl(meson->base + REG_PWM);
	channel->lo = FIELD_GET(PWM_LOW_MASK, value);
	channel->hi = FIELD_GET(PWM_HIGH_MASK, value);
	state->period = meson_pwm_tee_cnt_to_ns(chip, pwm, channel->lo + channel->hi);
	state->duty_cycle = meson_pwm_tee_cnt_to_ns(chip, pwm, channel->hi);
	if (channel->lo == 0) {
		if (constant_enabled) {
			state->period = meson_pwm_tee_cnt_to_ns(chip, pwm, channel->hi);
			state->duty_cycle = state->period;
		} else {
			state->period = meson_pwm_tee_cnt_to_ns(chip, pwm, channel->hi + 2);
			state->duty_cycle = meson_pwm_tee_cnt_to_ns(chip, pwm, channel->hi + 1);
		}
	} else if (channel->hi == 0) {
		if (constant_enabled) {
			state->period = meson_pwm_tee_cnt_to_ns(chip, pwm, channel->lo);
			state->duty_cycle = 0;
		} else {
			state->period = meson_pwm_tee_cnt_to_ns(chip, pwm, channel->lo + 2);
			state->duty_cycle = meson_pwm_tee_cnt_to_ns(chip, pwm, channel->hi + 1);
		}
	} else {
		state->period = meson_pwm_tee_cnt_to_ns(chip, pwm,
							channel->lo + channel->hi + 2);
		state->duty_cycle = meson_pwm_tee_cnt_to_ns(chip, pwm,
							channel->hi + 1);
	}
	state->polarity = PWM_POLARITY_NORMAL;
	dev_dbg(chip->dev, "%s, get pwm state period: %llu ns, duty: %llu ns\n",
					__func__, state->period, state->duty_cycle);
}

static const struct pwm_ops meson_pwm_tee_ops = {
	.request = meson_pwm_tee_request,
	.free = meson_pwm_tee_free,
	.apply = meson_pwm_tee_apply,
	.get_state = meson_pwm_tee_get_state,
	.owner = THIS_MODULE,
};

static const struct of_device_id meson_pwm_tee_matches[] = {
	{
		.compatible = "amlogic,meson-pwm-tee",
	},
	{},
};
MODULE_DEVICE_TABLE(of, meson_pwm_tee_matches);

static int meson_pwm_tee_init_channels(struct meson_pwm_tee *meson)
{
	struct device *dev = meson->chip.dev;
	unsigned int i;
	char name[255];
	struct meson_pwm_tee_channel *channel;

	for (i = 0; i < meson->chip.npwm; i++) {
		channel = &meson->channels[i];
		snprintf(name, sizeof(name), "clkin%u", i);
		channel->clk = devm_clk_get(dev, name);
		if (IS_ERR(channel->clk)) {
			dev_err(meson->chip.dev, "can't get device clock\n");
			return PTR_ERR(channel->clk);
		}
	}

	return 0;
}

static int meson_pwm_tee_probe(struct platform_device *pdev)
{
	struct meson_pwm_tee *meson;
	int err;
	struct resource *regs;

	meson = devm_kzalloc(&pdev->dev, sizeof(*meson), GFP_KERNEL);
	if (!meson)
		return -ENOMEM;

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	meson->base = devm_ioremap_resource(&pdev->dev, regs);
	if (IS_ERR(meson->base))
		return PTR_ERR(meson->base);
	/* get pwm tee_id property */
	err = of_property_read_u32(pdev->dev.of_node, "tee_id",
			   &meson->tee_id);
	if (err) {
		dev_err(&pdev->dev, "not config tee_id\n");
		return err;
	}
	// meson->base = devm_platform_ioremap_resource(pdev, 0);

	spin_lock_init(&meson->lock);
	meson->chip.dev = &pdev->dev;
	meson->chip.ops = &meson_pwm_tee_ops;
	meson->chip.npwm = MESON_NUM_PWMS;

	err = meson_pwm_tee_init_channels(meson);
	if (err < 0)
		return err;

	err = pwmchip_add(&meson->chip);
	if (err < 0) {
		dev_err(&pdev->dev, "failed to register PWM chip: %d\n", err);
		return err;
	}

	return 0;
}

static struct platform_driver meson_pwm_tee_driver = {
	.driver = {
		.name = "meson-pwm-tee",
		.of_match_table = meson_pwm_tee_matches,
	},
	.probe = meson_pwm_tee_probe,
};

int __init pwm_meson_tee_init(void)
{
	return platform_driver_register(&meson_pwm_tee_driver);
}

void __exit pwm_meson_tee_exit(void)
{
	platform_driver_unregister(&meson_pwm_tee_driver);
}

MODULE_DESCRIPTION("Amlogic Meson PWM Generator driver");
MODULE_AUTHOR("Neil Armstrong <narmstrong@baylibre.com>");
MODULE_LICENSE("Dual BSD/GPL");
