# SPDX-License-Identifier: (GPL-2.0+ OR MIT)

ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vout/hdmitx/hw20 \
				-I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vout/hdmitx/hw21 \
				-I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vin/tvin \
				-I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vin/tvin/hdmirx \
				-I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/efuse_unifykey

$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_HDMITX_COMMON) += \
							vout/hdmitx/hdmitx_boot_parameters.o \
							vout/hdmitx/hdmitx_common.o \
							vout/hdmitx/hdmitx_sysfs_common.o \
							vout/hdmitx/hdmitx_edid_parse.o \
							vout/hdmitx/hdmitx_hw_common.o \
							vout/hdmitx/hdmitx_packet.o \
							vout/hdmitx/hdmitx_mode.o \
							vout/hdmitx/hdmitx_format_para.o \
							vout/hdmitx/hdmitx_mode_set.o \
							vout/hdmitx/hdmitx_vout.o \
							vout/hdmitx/hdmitx_audio.o \
							vout/hdmitx/hdmitx_tracer_linux.o \
							vout/hdmitx/hdmitx_event_mgr_linux.o \
							vout/hdmitx/hdmitx_log.o \
							vout/hdmitx/hdmitx_compliance.o \
							vout/hdmitx/hdmitx_check_valid.o \
							vout/hdmitx/hdmitx_hdcp.o \
							vout/hdmitx/hdmitx_infoframe.o \
							vout/hdmitx/hdmitx_dump.o \
							vout/hdmitx/hdmitx_drv.o \

include $(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vout/hdmitx/hw20/Makefile

include $(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vout/hdmitx/hw21/Makefile
