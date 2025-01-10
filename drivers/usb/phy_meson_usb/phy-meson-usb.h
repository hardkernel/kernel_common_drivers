/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _PHY_MESON_USB2_H
#define _PHY_MESON_USB2_H

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/amlogic/usb-v2.h>
#include <linux/amlogic/usb-v2-common.h>
#include <linux/phy/phy.h>

extern bool meson_usb2phy_dbg;

#define mup_dbg(dev, fmt, args...)	\
		dev_dbg(dev, fmt, ## args)

#define mup_info(dev, fmt, args...)	\
		dev_info(dev, fmt, ## args)

#define mup_err(dev, fmt, args...)	\
		dev_err(dev, fmt, ## args)

#define MESON_UPHY_XHCI_PORT_A_MEM_SIZE 4UL

enum meson_uphy_port_speed {
	MESON_USB_SPEED_HIGH,
	MESON_USB_SPEED_HIGH_PLUS, /* Meson's private enhanced 960M. */
	MESON_USB_SPEED_SUPER,
};

enum meson_uphy_mode {
	MESON_USB_MODE_HOST,
	MESON_USB_MODE_DEVICE,
	MESON_USB_MODE_OTG,
};

/* meson phy struct to struct phy glue layer. */
struct meson_uphy_instance {
	union {
		u32 port_speed;
		enum meson_uphy_port_speed speed;
	};
	struct phy *phy;
	u32 index;
	struct meson_uphy_pm_ops {
		int (*prepare)(struct meson_uphy_instance *instance);
		void (*complete)(struct meson_uphy_instance *instance);
		int (*suspend)(struct meson_uphy_instance *instance);
		int (*resume)(struct meson_uphy_instance *instance);
		int (*freeze)(struct meson_uphy_instance *instance);
		int (*thaw)(struct meson_uphy_instance *instance);
		int (*poweroff)(struct meson_uphy_instance *instance);
		int (*restore)(struct meson_uphy_instance *instance);
		int (*suspend_late)(struct meson_uphy_instance *instance);
		int (*resume_early)(struct meson_uphy_instance *instance);
		int (*freeze_late)(struct meson_uphy_instance *instance);
		int (*thaw_early)(struct meson_uphy_instance *instance);
		int (*poweroff_late)(struct meson_uphy_instance *instance);
		int (*restore_early)(struct meson_uphy_instance *instance);
		int (*suspend_noirq)(struct meson_uphy_instance *instance);
		int (*resume_noirq)(struct meson_uphy_instance *instance);
		int (*freeze_noirq)(struct meson_uphy_instance *instance);
		int (*thaw_noirq)(struct meson_uphy_instance *instance);
		int (*poweroff_noirq)(struct meson_uphy_instance *instance);
		int (*restore_noirq)(struct meson_uphy_instance *instance);
		int (*runtime_suspend)(struct meson_uphy_instance *instance);
		int (*runtime_resume)(struct meson_uphy_instance *instance);
		int (*runtime_idle)(struct meson_uphy_instance *instance);
	} pm_ops;
	void *meson_uphy;
};

#define to_amlusbv2phy(phy)\
	((struct amlogic_usb_v2 *)((struct meson_uphy_instance *)\
	phy_get_drvdata(phy))->meson_uphy)\

struct meson_uphy_pool {
	struct device *dev;
	const struct meson_uphy_pdata *pdata;
	struct meson_uphy_instance **phys;
	int nphys;
};

struct meson_uphy_ops {
	int	(*init)(struct phy *phy);
	int	(*exit)(struct phy *phy);
	int	(*power_on)(struct phy *phy);
	int	(*power_off)(struct phy *phy);
	int	(*set_mode)(struct phy *phy, enum phy_mode mode, int submode);
	int	(*configure)(struct phy *phy, struct meson_uphy_configure_opts *opts);
	int	(*validate)(struct phy *phy, enum phy_mode mode, int submode,
				struct meson_uphy_configure_opts *opts);
	int	(*calibrate)(struct phy *phy);
	int	(*connect)(struct phy *phy, int port);
	int	(*disconnect)(struct phy *phy, int port);
};

struct meson_uphy_pdata {
	struct meson_uphy_ops *u2phy_ops;
	struct meson_uphy_ops *u3phy_ops;
	int (*u2phy_parse)(struct device *dev, struct meson_uphy_instance *instance);
	int (*u3phy_parse)(struct device *dev, struct meson_uphy_instance *instance);
	int (*otg_parse)(struct device *dev, struct meson_uphy_instance *instance);
};

extern struct meson_uphy_pdata meson_uphy_s7_pdata;
extern struct meson_uphy_pdata meson_uphy_s7d_pdata;
extern struct meson_uphy_pdata meson_uphy_sc2_pdata;

int meson_u2phy_usb_reset(struct amlogic_usb_v2 *phy);
int meson_u2phy_usb_hold_reset(struct amlogic_usb_v2 *phy, bool on);
int meson_u2phy_hold_reset(struct amlogic_usb_v2 *phy, bool on);
int meson_u2phy_reset_phycfg(struct amlogic_usb_v2 *phy);
int meson_u2phy_reg_reset(struct amlogic_usb_v2 *phy);
int meson_u2phy_reg_hold_reset(struct amlogic_usb_v2 *phy, bool on);
void meson_u2phy_set_vbus_power(struct amlogic_usb_v2 *phy, bool is_power_on);
void meson_u2phy_phy_legacy_device_tuning(struct amlogic_usb_v2 *phy, bool tune);
void meson_usb2phy_legacy_set_pll(struct amlogic_usb_v2 *phy);
void meson_usb2phy_legacy_cali(struct amlogic_usb_v2 *phy);
void meson_usb2phy_legacy_cali_n(struct amlogic_usb_v2 *phy);
int meson_u2phy_legacy_set_mode(struct amlogic_usb_v2 *phy, enum phy_mode mode);
void meson_usb2phy_set_calibration_trim(struct amlogic_usb_v2 *phy);
int meson_u2phy_set_mode(struct amlogic_usb_v2 *phy, enum phy_mode mode);
int meson_usb2phy_wait_ready(struct amlogic_usb_v2 *phy, unsigned int timeout);
int meson_u2phy_exit(struct amlogic_usb_v2 *phy);
int meson_u2phy_power_on(struct amlogic_usb_v2 *phy);
int meson_u2phy_power_off(struct amlogic_usb_v2 *phy);
int meson_aml_u2phy_parse(struct device *dev, struct meson_uphy_instance *instance);
int meson_synopsis_u3phy_init(struct amlogic_usb_v2 *phy);
int meson_synopsis_u3phy_exit(struct amlogic_usb_v2 *phy);
int meson_synopsis_u3phy_parse(struct device *dev, struct meson_uphy_instance *instance);
int meson_m31_u2phy_parse(struct device *dev, struct meson_uphy_instance *instance);
int meson_m31_u3phy_parse(struct device *dev, struct meson_uphy_instance *instance);
int meson_u2phy_crg_otg_parse(struct device *dev, struct meson_uphy_instance *instance);
int meson_uphy_rtmux_otg_parse(struct device *dev, struct meson_uphy_instance *instance);
int meson_uphy_rtmux_otg_init(struct amlogic_usb_v2 *phy);
int meson_uphy_rtmux_otg_register_notifier(struct notifier_block *nb);
int meson_uphy_rtmux_otg_unregister_notifier(struct notifier_block *nb);
void crg_exit(void);
int crg_init(void);
void crg_gadget_exit(void);
int crg_gadget_init(void);
const char *crg_udc_get_UDC_name(void);
int crg_otg_write_UDC(const char *udc_name);
#endif/* _PHY_MESON_USB2_H */
