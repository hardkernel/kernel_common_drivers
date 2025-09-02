# SPDX-License-Identifier: (GPL-2.0+ OR MIT)

$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DTV_DEMOD) += dtv_demod/demod_func.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DTV_DEMOD) += dtv_demod/i2c_func.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DTV_DEMOD) += dtv_demod/tuner_func.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DTV_DEMOD) += dtv_demod/amlfrontend.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DTV_DEMOD) += dtv_demod/demod_dbg.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DTV_DEMOD) += dtv_demod/aml_demod.o

ccflags-y += -I.
ccflags-y += -I$(srctree)/drivers/media/dvb-core
ccflags-y += -I$(srctree)/drivers/iio/adc
ccflags-y += -I$(srctree)/include/media
ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/dtv_demod/include

$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_ATSC) += dtv_demod/atsc/atsc_func.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_ATSC) += dtv_demod/atsc/atsc_frontend.o
ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/dtv_demod/atsc

$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_DVBC) += dtv_demod/dvbc/dvbc_func.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_DVBC) += dtv_demod/dvb_qam/dvbc_v2.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_DVBC) += dtv_demod/dvb_qam/dvbc_v3.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_DVBC) += dtv_demod/dvbc/dvbc_frontend.o
ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/dtv_demod/dvbc
ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/dtv_demod/dvb_qam

$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_DVBT) += dtv_demod/dvbt/dvbt_v2.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_DVBT) += dtv_demod/dvbt/dvbt_frontend.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_DVBT) += dtv_demod/dvbt/dvbt2_fw.o
ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/dtv_demod/dvbt

$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_DTMB) += dtv_demod/dtmb/dtmb_func.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_DTMB) += dtv_demod/dtmb/dtmb_frontend.o
ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/dtv_demod/dtmb

ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/dtv_demod/dvbs
ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/dtv_demod/dvbs/lnb_controller
ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/dtv_demod/dvbs/lnb_controller/gpio
ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/dtv_demod/dvbs/lnb_controller/wt20_1811
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_DVBS) += dtv_demod/dvbs/dvbs.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_DVBS) += dtv_demod/dvbs/dvbs_frontend.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_DVBS) += dtv_demod/dvbs/dvbs_diseqc.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_DVBS) += dtv_demod/dvbs/dvbs_singlecable.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_DVBS) += dtv_demod/dvbs/lnb_controller/gpio/gpio_lnbc.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_DVBS) += dtv_demod/dvbs/lnb_controller/wt20_1811/wt20_1811.o

$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_ISDBT) += dtv_demod/isdbt/isdbt_func.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_ISDBT) += dtv_demod/isdbt/isdbt_frontend.o
ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/dtv_demod/isdbt

$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_J83B) += dtv_demod/j83b/j83b_frontend.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_J83B) += dtv_demod/dvb_qam/dvbc_v2.o
$(MEDIA_MODULE_NAME)-$(CONFIG_AMLOGIC_DEMOD_SUPPORT_J83B) += dtv_demod/dvb_qam/dvbc_v3.o
ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/dtv_demod/dvb_qam
ccflags-y += -I$(srctree)/$(COMMON_DRIVERS_DIR)/drivers/media/dtv_demod/j83b

