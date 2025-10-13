/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _I2C_MAIN_H__
#define _I2C_MAIN_H__

#if IS_ENABLED(CONFIG_AMLOGIC_I2C_MESON)
int i2c_meson_init(void);
void i2c_meson_exit(void);
#else
static inline int i2c_meson_init(void)
{
	return 0;
}

static inline void i2c_meson_exit(void)
{
}
#endif

#if IS_ENABLED(CONFIG_AMLOGIC_I2C_MESON_T6W)
int i2c_meson_t6w_init(void);
void i2c_meson_t6w_exit(void);
#else
static inline int i2c_meson_t6w_init(void)
{
	return 0;
}

static inline void i2c_meson_t6w_exit(void)
{
}
#endif

#if IS_ENABLED(CONFIG_AMLOGIC_I2C_MESON_V2)
int i2c_meson_v2_init(void);
void i2c_meson_v2_exit(void);
#else
static inline int i2c_meson_v2_init(void)
{
	return 0;
}

static inline void i2c_meson_v2_exit(void)
{
}
#endif

#endif /* _I2C_MAIN_H__ */
