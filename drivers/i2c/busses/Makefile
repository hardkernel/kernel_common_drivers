# SPDX-License-Identifier: (GPL-2.0+ OR MIT)
MODULE_NAME = amlogic-i2c

PR_FMT = $(subst amlogic-,,$(MODULE_NAME))
PR_FMT_DEFINE="-Dpr_fmt(fmt)= \"[$(PR_FMT)]: \" fmt"
ccflags-y += $(PR_FMT_DEFINE)

obj-$(CONFIG_AMLOGIC_I2C) = $(MODULE_NAME).o
$(MODULE_NAME)-y += main.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_I2C_MESON) += i2c-meson.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_I2C_MESON_T6W) += i2c-meson-t6w.o