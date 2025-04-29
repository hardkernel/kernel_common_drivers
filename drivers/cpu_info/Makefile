# SPDX-License-Identifier: (GPL-2.0+ OR MIT)

MODULE_NAME = amlogic-cpuinfo
obj-$(CONFIG_AMLOGIC_CPU_INFO) = $(MODULE_NAME).o
$(MODULE_NAME)-y += cpu_info.o

PR_FMT = $(subst amlogic-,,$(MODULE_NAME))
PR_FMT_DEFINE="-Dpr_fmt(fmt)= \"[$(PR_FMT)]: \" fmt"
ccflags-y += $(PR_FMT_DEFINE)
BUILD_TIME := $(shell date +%Y.%m.%d-%H.%M.%S)
ccflags-y += "-DBUILD_TIME=\"$(BUILD_TIME)\""

COMMON_DRIVERS_RELEASE=$(subst -g,, $(shell $(srctree)/scripts/setlocalversion $(srctree)/$(COMMON_DRIVERS_DIR)))
COMMON_DRIVERS_RELEASE_DEFINE="-DCOMMON_DRIVERS_RELEASE=\"$(COMMON_DRIVERS_RELEASE)\""
ccflags-y += $(COMMON_DRIVERS_RELEASE_DEFINE)

BUILD_GKI_VERSION := $(shell echo $$GKI_CONFIG)
BUILD_GKI_VERSION_DEFINE="-DGKI_CONFIG=\"$(BUILD_GKI_VERSION)\""
ccflags-y += $(BUILD_GKI_VERSION_DEFINE)
