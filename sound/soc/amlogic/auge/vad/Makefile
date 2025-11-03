# SPDX-License-Identifier: (GPL-2.0+ OR MIT)

KBUILD_CFLAGS += -Wlarger-than=32768
# KBUILD_CFLAGS += -Wstack-usage=1856

MODULE_NAME = amlogic-snd-vad

PR_FMT_VAD = $(subst amlogic-,,$(MODULE_NAME))
PR_FMT_DEFINE_VAD="-Dpr_fmt(fmt)= \"[$(PR_FMT_VAD)]: \" fmt"
ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/sound/soc/amlogic/auge/

obj-$(CONFIG_AMLOGIC_SND_SOC) += $(MODULE_NAME).o
$(MODULE_NAME)-y += vad_open.o
CFLAGS_vad_open.o += $(PR_FMT_DEFINE_VAD)
