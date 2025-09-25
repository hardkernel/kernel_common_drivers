# SPDX-License-Identifier: GPL-2.0

MODULE_NAME = amlogic-spinor
obj-$(CONFIG_AMLOGIC_MTD_SPI_NOR) = $(MODULE_NAME).o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_MTD_SPI_NOR) += core.o sfdp.o swp.o otp.o sysfs.o atmel.o eon.o esmt.o everspin.o gigadevice.o intel.o issi.o macronix.o micron-st.o spansion.o sst.o winbond.o xmc.o
$(MODULE_NAME)-$(CONFIG_DEBUG_FS) += debugfs.o

PR_FMT = $(subst amlogic-,,$(MODULE_NAME))
PR_FMT_DEFINE="-Dpr_fmt(fmt)= \"[$(PR_FMT)]: \" fmt"
ccflags-y += $(PR_FMT_DEFINE)
