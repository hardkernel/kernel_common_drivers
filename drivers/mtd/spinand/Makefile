# SPDX-License-Identifier: GPL-2.0

MODULE_NAME = amlogic-spinand
obj-$(CONFIG_AMLOGIC_MTD_SPI_NAND) = $(MODULE_NAME).o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_MTD_SPI_NAND) += core.o gigadevice.o macronix.o micron.o paragon.o toshiba.o winbond.o dosilicon.o meson_spinand.o crypto.o

PR_FMT = $(subst amlogic-,,$(MODULE_NAME))
PR_FMT_DEFINE="-Dpr_fmt(fmt)= \"[$(PR_FMT)]: \" fmt"
ccflags-y += $(PR_FMT_DEFINE)
