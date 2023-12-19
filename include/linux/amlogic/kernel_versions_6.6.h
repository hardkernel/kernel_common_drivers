/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __KERNEL_VERSIONS_6_6_H
#define __KERNEL_VERSIONS_6_6_H
/*drm_gem_cma_helper.h change to drm_gem_dma_helper.h*/
#include <drm/drm_gem_dma_helper.h>
/*drm_fb_cma_helper.h change to drm_fb_dma_helper.h*/
#include <drm/drm_fb_dma_helper.h>
#include <linux/iosys-map.h>
#include <drm/display/drm_hdcp_helper.h>
#include "../../../../kernel/module/internal.h"

/*__CFI_ADDRESSABLE change to ___ADDRESSABLE*/
void __module_init_hook(struct module *m);
#define module_init_hook(initfn)      \
	int __init init_module(void) \
	{       \
		__module_init_hook(THIS_MODULE); \
		return initfn();     \
	}	\
	___ADDRESSABLE(init_module, __initdata);

/*PDE_DATA change to pde_data*/
static inline void *kv_pde_data(const struct inode *inode)
{
	return pde_data(inode);
}

/*class_create(owner, name) delete the member owner*/
static inline struct class *kv_class_create(struct module *owner, const char *name)
{
	return class_create(name);
}

/*this function add the new return value in 6.6*/
static inline int kv_cpufreq_unregister_driver(struct cpufreq_driver *driver)
{
	cpufreq_unregister_driver(driver);
	return 0;
}

#define KV_CLASS_CONST const      //kernel 6.6 change the type of parameter to const
#define KV_CLASS_ATTR_CONST const //kernel 6.6 change the type of parameter to const
#define KV_CLASS_DEV_CONST const  //kernel 6.6 change the type of parameter to const
#define KV_CLASS_DEF_OWNER        //kernel 6.6 delete the .owner member in some struct
#define KV_BUS_UEVENT_DEV_CONST const //kernel 6.6 change the type of parameter to const
#define KV_I2C_PROBE_ID           //kernel 6.6 delete the parameter i2c_device_id
#define KV_I2C_REMOVE_TYPE void   //kernel 6.6 change the type of parameter to void
#define KV_I2C_REMOVE_RET(ret)    //kernel 6.6 change the return value
#define KV_I2C_PROBE_DEF_ID(client, device_id) \
	const struct i2c_device_id *id = i2c_match_id(device_id, client)
#define KV_SPI_REMOVE_TYPE void   //kernel 6.6 change the type of parameter to void
#define KV_SPI_REMOVE_RET(ret) return

/*kernel 6.6 remove the class->owner = THIS_MODULE*/
static inline void kv_set_class_owner(struct class *class)
{
}

/*kernel 6.6 change prandom_u32 to get_random_u32*/
static inline u32 kv_aml_random_u32(void)
{
	return get_random_u32();
}

/*kernel 6.6 change prandom_bytes to get_random_bytes*/
static inline void kv_get_random_bytes(void *buf, size_t len)
{
	get_random_bytes(buf, len);
}

/*kernel 6.6 change fb_memcpy_fromfb to fb_memcpy_fromio*/
static inline void kv_fb_memcpy_fromfb(void *to, const volatile void __iomem *from, size_t n)
{
	fb_memcpy_fromio(to, from, n);
}

/*kernel 6.6 change fb_memcpy_tofb to fb_memcpy_toio*/
static inline void kv_fb_memcpy_tofb(volatile void __iomem *to, const void *from, size_t n)
{
	fb_memcpy_toio(to, from, n);
}

/*kernel6.6 add new member ##__VA_ARGS__*/
#define kv_register_shrinker(shrinker, ...) \
	register_shrinker(shrinker, ##__VA_ARGS__);

#define KV_OPP_RATE(opp, n) opp->rates[n]

#define kv_drm_vmap_map iosys_map //kernel 6.6 change dma_buf_map to iosys_map
#define kv_map_set_vaddr(map, vaddr) iosys_map_set_vaddr(map, vaddr)
#define kv_map_clear(map) iosys_map_clear(map)

/*kernel6.6 use vm_flags_clear to clear bit flag*/
static inline void kv_vm_flags_clear(struct vm_area_struct *vma,
				     vm_flags_t flags)
{
	vm_flags_clear(vma, flags);
}

/*kernel6.6 use vm_flags_set to set bit flag*/
static inline void kv_vm_flags_set(struct vm_area_struct *vma,
				   vm_flags_t flags)
{
	vm_flags_set(vma, flags);
}

#define kv_vm_operations_ops drm_gem_dma_vm_ops

/*kernel6.6 use fb_modifiers_not_supported to Invert this value*/
static inline void kv_set_drm_allow_fb_modifiers(struct drm_device *drm, bool allow)
{
	drm->mode_config.fb_modifiers_not_supported = !allow;
}

/*kernel6.6 add the member possible_crtcs*/
static inline int kv_drm_writeback_connector_init(struct drm_device *dev,
				    struct drm_writeback_connector *wb_connector,
				    const struct drm_connector_funcs *con_funcs,
				    const struct drm_encoder_helper_funcs *enc_helper_funcs,
				    const u32 *formats, int n_formats,
				    u32 possible_crtcs)
{
	return drm_writeback_connector_init(dev, wb_connector, con_funcs,
					    enc_helper_funcs, formats,
					    n_formats, possible_crtcs);
}

/*kernel6.6 change dma_resv_add_excl_fence to dma_resv_add_fence*/
static inline void kv_dma_resv_add_fence(struct dma_resv *obj, struct dma_fence *fence,
					 enum dma_resv_usage usage)
{
	dma_resv_add_fence(obj, fence, usage);
}

/*kernel6.6 change to use pointer in delay_on and delay_off*/
static inline void kv_led_trigger_blink_oneshot(struct led_trigger *trig,
						unsigned long *delay_on,
						unsigned long *delay_off,
						int invert)
{
	led_trigger_blink_oneshot(trig, *delay_on, *delay_off, invert);
}

/*kernel6.6 don't use the member card->host*/
static inline int kv_mmc_sw_reset(struct mmc_card *card)
{
	return mmc_sw_reset(card);
}

static inline int kv_power_supply_get_battery_info(struct power_supply *psy,
						struct power_supply_battery_info **info_out)
{
	return power_supply_get_battery_info(psy, info_out);
}

static inline void kv_power_supply_put_battery_info(struct power_supply *psy,
						    struct power_supply_battery_info *info)
{
	power_supply_put_battery_info(psy, info);
}

/*kernel6.6 change the type void to int*/
#define KV_PWM_GET_STATE_TYPE int
/*kernel6.6 add the return value*/
#define KV_PWM_GET_STATE_RET(ret) return ret

static inline int kv_thermal_zone_get_trip(struct thermal_zone_device *tz, int trip_id,
					   struct thermal_trip *trip)
{
	return thermal_zone_get_trip(tz, trip_id, trip);
}

static inline int kv___thermal_zone_get_trip(struct thermal_zone_device *tz, int trip_id,
					     struct thermal_trip *trip)
{
	return __thermal_zone_get_trip(tz, trip_id, trip);
}

#define kv_thermal_get_num_trips(tz) thermal_zone_get_num_trips(tz)

typedef struct thermal_zone_device_ops kv_thermal_zone_device_ops;
/*kernel6.6 change the type void to thermal_zone_device*/
typedef struct thermal_zone_device kv_get_temp_p_or_tz;

/*kernel6.6 change the return value*/
static inline void *kv_get_temp_meson_data(kv_get_temp_p_or_tz *tz)
{
	return tz->devdata;
}

/*kernel6.6 change the return function*/
static inline struct thermal_zone_device *kv_devm_thermal_zone_register(
	struct device *dev, int sensor_id,
	void *data, const kv_thermal_zone_device_ops *ops)
{
	return devm_thermal_of_zone_register(dev, sensor_id, data, ops);
}

static inline void kv_devm_thermal_zone_unregister(struct device *dev,
						   struct thermal_zone_device *tz)
{
	devm_thermal_of_zone_unregister(dev, tz);
}

/*kernel6.6 change the type int to unsigned char*/
typedef unsigned char kv_uart_putchar_char_type;
/*kernel6.6 change the type to const*/
#define KV_UART_SET_TERMIOS_OLD_CONST const
/*kernel6.6 change the SERIAL_XMIT_SIZE to UART_XMIT_SIZE*/
#define KV_UART_XMIT_SIZE UART_XMIT_SIZE

/*kernel6.6 change the return function*/
static inline int kv_usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	return usb_gadget_register_driver(driver);
}

/*kernel6.6 usb_maxpacket change parameter usb_maxpacket(udev, pipe, is_out)*/
static inline u16 kv_usb_maxpacket(struct usb_device *udev, int pipe, int is_out)
{
	return usb_maxpacket(udev, pipe);
}

/*kernel6.6 add parameter index*/
static inline int kv_snd_soc_of_get_dai_name(struct device_node *of_node,
					     const char **dai_name, int index)
{
	return snd_soc_of_get_dai_name(of_node, dai_name, index);
}

/*kernel6.6 add new member .probe = func*/
#define KV_SND_DAI_OPS_DEF_PROBE(func) .probe = func,
#define KV_SND_DAI_DRIVER_DEF_PROBE(func)

/*kernel6.6 remove member .non_legacy_dai_naming = i*/
#define KV_SND_SOC_DRIVER_DEF_NON_LEGACY_NAMEING(i)
/*kernel6.6 add parameter unsigned int prev_stat*/
#define KV_SCHED_SWITCH_HOOK_PREV_STAT , unsigned int prev_stat

/*return function change to snd_soc_card_jack_new_pins*/
static inline int kv_snd_soc_card_jack_new(struct snd_soc_card *card, const char *id, int type,
					   struct snd_soc_jack *jack,
					   struct snd_soc_jack_pin *pins, unsigned int num_pins)
{
	return snd_soc_card_jack_new_pins(card, id, type, jack, pins, num_pins);
}

#endif //__KERNEL_VERSIONS_6_6_H

