# SPDX-License-Identifier: GPL-2.0-only

MODULE_NAME = aml-zram

$(MODULE_NAME)-y := zcomp.o zram_drv.o

$(MODULE_NAME)-$(CONFIG_AMLOGIC_ZRAM_BACKEND_LZO) += backend_lzorle.o backend_lzo.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_ZRAM_BACKEND_LZ4) += backend_lz4.o
$(MODULE_NAME)-$(CONFIG_AMLOGIC_ZRAM_BACKEND_AMFC) += backend_amfc.o

obj-$(CONFIG_AMLOGIC_ZRAM) += aml-zram.o
