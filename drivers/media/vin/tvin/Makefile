# SPDX-License-Identifier: (GPL-2.0+ OR MIT)
#
# Makefile for Tvin.
#

$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_MEDIA_TVIN) += vin/tvin/tvin_format_table.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_MEDIA_TVIN) += vin/tvin/tvin_frontend.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_MEDIA_TVIN) += vin/tvin/tvin_global.o

include $(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vin/tvin/vdin/Makefile
include $(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vin/tvin/tvafe/Makefile
include $(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vin/tvin/viu/Makefile
include $(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vin/tvin/bt656/Makefile
include $(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vin/tvin/csi/Makefile
include $(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vin/tvin/hdmirx/Makefile
include $(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/vin/tvin/dsc_dec/Makefile
