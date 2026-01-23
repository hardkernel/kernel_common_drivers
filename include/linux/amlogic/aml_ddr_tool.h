/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __AML_DDR_TOOL_H__
#define __AML_DDR_TOOL_H__

/* ddr_tool: dev_access */
struct dmc_dev_access_data {
	unsigned long addr;
	unsigned long size;
};

struct dmc_side_band {
	unsigned char dmc;
	unsigned char bus;
	unsigned char rw;
	unsigned char block_num;
	unsigned char block_bus[32];
};

#if IS_ENABLED(CONFIG_AMLOGIC_DMC_DEV_ACCESS)
int register_dmc_dev_access_notifier(char *dev_name, struct notifier_block *nb);
int unregister_dmc_dev_access_notifier(char *dev_name, struct notifier_block *nb);
#else
static inline int register_dmc_dev_access_notifier(char *dev_name, struct notifier_block *nb)
{
	return 0;
}

static inline int unregister_dmc_dev_access_notifier(char *dev_name, struct notifier_block *nb)
{
	return 0;
}
#endif

#if IS_ENABLED(CONFIG_AMLOGIC_DDR_BANDWIDTH)
void aml_get_all_channel_grant(u64 *channel_grant);
#else
static inline void aml_get_all_channel_grant(u64 *channel_grant)
{
}
#endif

/* ddr_tool: outstanding */
#if IS_ENABLED(CONFIG_AMLOGIC_DDR_BANDWIDTH)

int get_dmc_priority(unsigned int bus_id, char rw);
int set_dmc_priority(unsigned int bus_id, unsigned int priority, char rw);
int set_dev_priority(unsigned int port_id, unsigned int priority, char rw);
int get_dev_priority(unsigned int port_id, char rw);
int set_vpu_super_priority(unsigned char axi, char rw, unsigned char priority);
int set_vpu_top_priority(unsigned char axi, char rw, unsigned char top_i, unsigned char priority);
int set_vpu_sub_dev_priority(unsigned char axi, char rw, unsigned int id, unsigned char priority);
int set_demod_priority(unsigned char priority);
int set_demux_priority(unsigned char priority, char rw);
int set_audio_x_select_priority(char x, unsigned char ugt);
int set_audio_priority(unsigned char idx, unsigned char priority, char rw);
int show_audio_priority_setting(void);
int set_bcon_hw_urgent(unsigned char low, unsigned char high);
int set_bcon_sw_urgent(unsigned char ugt);
int show_bcon_priority_setting(void);
int set_hevc_level_id(char rw, unsigned char level, unsigned char i, unsigned char id);
int set_hcodec_level_id(char rw, unsigned char level, unsigned char i, unsigned char id);
int set_hevc_low_priority(char rw, unsigned char priority);
int set_hcodec_low_priority(char rw, unsigned char priority);
int show_hevc_priority_setting(void);
int show_hcodec_priority_setting(void);
int set_device_accessible(char *dev, unsigned char access);

int get_bus_num(void);
int get_bus_ots_value(int bus);
int set_bus_ots_by_value(int bus, int value);
int set_bus_ots_by_level(int bus, unsigned int level);
int get_ots_level(void);
int set_all_ots_by_level(unsigned int level);

int enable_side_band(struct dmc_side_band *sb);
int disable_side_band(unsigned char dmc, unsigned char bus);
int get_vpu_bus_num(void);
int get_side_band(struct dmc_side_band *sb, unsigned char num);
#else
static inline int get_bus_num(void)
{
	return -1;
}

static inline int get_bus_ots_value(int bus)
{
	return -1;
}

static inline int set_bus_ots_by_value(int bus, int value)
{
	return -1;
}

static inline int set_bus_ots_by_level(int bus, unsigned int level)
{
	return -1;
}

static inline int get_ots_level(void)
{
	return -1;
}

static inline int set_all_ots_by_level(unsigned int level)
{
	return -1;
}

static inline int enable_side_band(struct dmc_side_band *sb)
{
	return -1;
}

static inline int disable_side_band(unsigned char dmc, unsigned char bus)
{
	return -1;
}

static inline int get_vpu_bus_num(void)
{
	return -1;
}

static inline int get_side_band(struct dmc_side_band *sb, unsigned char num)
{
	return -1;
}
#endif

#endif
