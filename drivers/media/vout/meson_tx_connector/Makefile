# SPDX-License-Identifier: (GPL-2.0+ OR MIT)

ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vin/tvin \
				-I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vin/tvin/hdmirx \
				-I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/efuse_unifykey

$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_TX_CONNECTOR_COMMON) += \
							vout/meson_tx_connector/hdmitx_boot_parameters.o \
							vout/meson_tx_connector/hdmitx_common.o \
							vout/meson_tx_connector/hdmitx_sysfs_common.o \
							vout/meson_tx_connector/hdmitx_edid_parse.o \
							vout/meson_tx_connector/hdmitx_hw_common.o \
							vout/meson_tx_connector/hdmitx_hdr.o \
							vout/meson_tx_connector/hdmitx_mode.o \
							vout/meson_tx_connector/hdmitx_format_para.o \
							vout/meson_tx_connector/hdmitx_mode_set.o \
							vout/meson_tx_connector/hdmitx_vout.o \
							vout/meson_tx_connector/hdmitx_audio.o \
							vout/meson_tx_connector/hdmitx_tracer_linux.o \
							vout/meson_tx_connector/hdmitx_event_mgr_linux.o \
							vout/meson_tx_connector/hdmitx_log.o \
							vout/meson_tx_connector/hdmitx_compliance.o \
							vout/meson_tx_connector/hdmitx_check_valid.o \
							vout/meson_tx_connector/hdmitx_hdcp.o \
							vout/meson_tx_connector/hdmitx_infoframe.o \
							vout/meson_tx_connector/hdmitx_dump.o \
							vout/meson_tx_connector/hdmitx_ext.o \
							vout/meson_tx_connector/hdmitx_drv.o \

include $(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vout/meson_tx_connector/hdmitx/Makefile
include $(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vout/meson_tx_connector/hdcp/Makefile