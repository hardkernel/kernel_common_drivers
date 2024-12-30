# SPDX-License-Identifier: (GPL-2.0+ OR MIT)

$(MODULE_NAME)-$(CONFIG_AMLOGIC_RAMDUMP)	+= ramdump/ram_dump.o

BUILD_TIME := $(shell date +%Y.%m.%d-%H.%M.%S)
ccflags-y += "-DBUILD_TIME=\"$(BUILD_TIME)\""