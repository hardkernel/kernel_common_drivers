/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2023 Amlogic, Inc. All rights reserved.
 */

#include "meson_saradc.h"

void meson_m8_sar_adc_set_ch7_mux(struct iio_dev *indio_dev,
				  enum meson_sar_adc_chan7_mux_sel sel);
int meson_m8_sar_adc_read_fifo(struct iio_dev *indio_dev,
			       const struct iio_chan_spec *chan,
			       bool chk_channel);
void meson_m8_sar_adc_enable_chnl(struct iio_dev *indio_dev, bool en);
int meson_m8_sar_adc_read_chnl(struct iio_dev *indio_dev,
			       const struct iio_chan_spec *chan);
void meson_m8_sar_adc_disable_ring(struct meson_sar_adc_priv *priv);

extern const struct regmap_config meson_sar_adc_regmap_config_g12a;
