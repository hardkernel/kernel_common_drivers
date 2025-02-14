# SPDX-License-Identifier: (GPL-2.0+ OR MIT)

ccflags-y += -Idrivers/staging/android/
ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/common/ion_dev/
ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/frc/

$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_VIDEO_DISPLAY) += video_processor/videodisplay/videodisplay_util.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_VIDEO_DISPLAY) += video_processor/videodisplay/videodisplay_drv.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_VIDEO_DISPLAY) += video_processor/videodisplay/videodisplay_interface.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_VIDEO_DISPLAY) += video_processor/videodisplay/videodisplay_process.o
