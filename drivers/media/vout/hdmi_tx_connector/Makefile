# SPDX-License-Identifier: (GPL-2.0+ OR MIT)

ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vin/tvin \
				-I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vin/tvin/hdmirx \
				-I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/efuse_unifykey

$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_TX_CONNECTOR_COMMON) += \
							vout/hdmi_tx_connector/hdmitx_boot_parameters.o \
							vout/hdmi_tx_connector/hdmitx_common.o \
							vout/hdmi_tx_connector/hdmitx_sysfs_common.o \
							vout/hdmi_tx_connector/hdmitx_edid_parse.o \
							vout/hdmi_tx_connector/hdmitx_hw_common.o \
							vout/hdmi_tx_connector/hdmitx_hdr.o \
							vout/hdmi_tx_connector/hdmitx_mode.o \
							vout/hdmi_tx_connector/hdmitx_format_para.o \
							vout/hdmi_tx_connector/hdmitx_mode_set.o \
							vout/hdmi_tx_connector/hdmitx_vout.o \
							vout/hdmi_tx_connector/hdmitx_audio.o \
							vout/hdmi_tx_connector/hdmitx_tracer_linux.o \
							vout/hdmi_tx_connector/hdmitx_event_mgr_linux.o \
							vout/hdmi_tx_connector/hdmitx_log.o \
							vout/hdmi_tx_connector/hdmitx_compliance.o \
							vout/hdmi_tx_connector/hdmitx_check_valid.o \
							vout/hdmi_tx_connector/hdmitx_hdcp.o \
							vout/hdmi_tx_connector/hdmitx_infoframe.o \
							vout/hdmi_tx_connector/hdmitx_dump.o \
							vout/hdmi_tx_connector/hdmitx_ext.o \
							vout/hdmi_tx_connector/hdmitx_drv.o \

$(MEDIA_MODULE_NAME)-$(CONFIG_ODROID_CUSTOM_DISPLAY_MODES) += \
							vout/hdmi_tx_connector/hdmitx_odroid_modes.o

include $(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vout/hdmi_tx_connector/hdmitx/Makefile
include $(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vout/hdmi_tx_connector/hdcp/Makefile