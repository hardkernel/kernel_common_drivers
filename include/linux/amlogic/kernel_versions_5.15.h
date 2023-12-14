/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __KERNEL_VERSIONS_5_15_H
#define __KERNEL_VERSIONS_5_15_H

#include <drm/drm_gem_cma_helper.h>
#include <drm/drm_fb_cma_helper.h>
#include <drm/drm_hdcp.h>
#include <linux/genhd.h>
#include <drm/drm_hashtab.h>
#include <drm/drm_legacy.h>
#include <../../../../drivers/thermal/thermal_core.h>

void __module_init_hook(struct module *m);
#define module_init_hook(initfn)      \
	int __init init_module(void) \
	{       \
		__module_init_hook(THIS_MODULE); \
		return initfn();     \
	}	\
	__CFI_ADDRESSABLE(init_module, __initdata);

static inline void *kv_pde_data(const struct inode *inode)
{
	return PDE_DATA(inode);
}

static inline struct class *kv_class_create(struct module *owner, const char *name)
{
	return class_create(owner, name);
}

static inline int kv_cpufreq_unregister_driver(struct cpufreq_driver *driver)
{
	return cpufreq_unregister_driver(driver);
}

#define KV_CLASS_CONST
#define KV_CLASS_ATTR_CONST
#define KV_CLASS_DEV_CONST
#define KV_CLASS_DEF_OWNER	.owner = THIS_MODULE,
#define KV_BUS_UEVENT_DEV_CONST
#define KV_I2C_PROBE_ID , const struct i2c_device_id *id
#define KV_I2C_REMOVE_TYPE int
#define KV_I2C_REMOVE_RET(ret) return ret
#define I2C_PROBE_DEF_ID(client, device_id)
#define KV_I2C_PROBE_DEF_ID(client, device_id)
#define KV_SPI_REMOVE_TYPE int
#define KV_SPI_REMOVE_RET(ret) return ret

static inline void kv_set_class_owner(struct class *class)
{
	class->owner = THIS_MODULE;
}

static inline u32 kv_aml_random_u32(void)
{
	return prandom_u32();
}

static inline void kv_get_random_bytes(void *buf, size_t len)
{
	prandom_bytes(buf, len);
}

static inline void kv_fb_memcpy_fromfb(void *to, const volatile void __iomem *from, size_t n)
{
	fb_memcpy_fromfb(to, from, n);
}

static inline void kv_fb_memcpy_tofb(volatile void __iomem *to, const void *from, size_t n)
{
	fb_memcpy_tofb(to, from, n);
}

#define kv_register_shrinker(shrinker, ...) \
	register_shrinker(shrinker);

#define KV_OPP_RATE(opp, n) opp->rate

#define kv_drm_vmap_map dma_buf_map
#define kv_map_set_vaddr(map, vaddr) dma_buf_map_set_vaddr(map, vaddr)
#define kv_map_clear(map) dma_buf_map_clear(map)

static inline void kv_vm_flags_clear(struct vm_area_struct *vma,
				     vm_flags_t flags)
{
	vma->vm_flags &= ~flags;
}

static inline void kv_vm_flags_set(struct vm_area_struct *vma,
				   vm_flags_t flags)
{
	vma->vm_flags |= flags;
}

#define kv_vm_operations_ops drm_gem_cma_vm_ops

static inline void kv_set_drm_allow_fb_modifiers(struct drm_device *drm, bool allow)
{
	drm->mode_config.allow_fb_modifiers = allow;
}

static inline int kv_drm_writeback_connector_init(struct drm_device *dev,
				    struct drm_writeback_connector *wb_connector,
				    const struct drm_connector_funcs *con_funcs,
				    const struct drm_encoder_helper_funcs *enc_helper_funcs,
				    const u32 *formats, int n_formats,
				    u32 possible_crtcs)
{
	return drm_writeback_connector_init(dev, wb_connector, con_funcs,
					    enc_helper_funcs, formats,
					    n_formats);
}

enum dma_resv_usage {
	DMA_RESV_USAGE_KERNEL,
	DMA_RESV_USAGE_WRITE,
	DMA_RESV_USAGE_READ,
	DMA_RESV_USAGE_BOOKKEEP
};

static inline void kv_dma_resv_add_fence(struct dma_resv *obj, struct dma_fence *fence,
					 enum dma_resv_usage usage)
{
	dma_resv_add_excl_fence(obj, fence);
}

static inline void kv_led_trigger_blink_oneshot(struct led_trigger *trig,
						unsigned long *delay_on,
						unsigned long *delay_off,
						int invert)
{
	led_trigger_blink_oneshot(trig, delay_on, delay_off, invert);
}

#define abs_diff(a, b) ({                       \
	typeof(a) __a = (a);                    \
	typeof(b) __b = (b);                    \
	(void)(&__a == &__b);                   \
	__a > __b ? (__a - __b) : (__b - __a);  \
})

static inline int kv_mmc_sw_reset(struct mmc_card *card)
{
	return mmc_sw_reset(card->host);
}

static inline int kv_power_supply_get_battery_info(struct power_supply *psy,
						struct power_supply_battery_info **info_out)
{
	struct power_supply_battery_info *info;
	int err;

	info = devm_kzalloc(&psy->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	err = power_supply_get_battery_info(psy, info);
	if (err < 0 )
		devm_kfree(&psy->dev, info);

	*info_out = info;
	return err;
}

static inline void kv_power_supply_put_battery_info(struct power_supply *psy,
						    struct power_supply_battery_info *info)
{
	power_supply_put_battery_info(psy, info);
	devm_kfree(&psy->dev, info);
}

#define KV_PWM_GET_STATE_TYPE void
#define KV_PWM_GET_STATE_RET(ret) return

static inline int kv_thermal_zone_get_trip(struct thermal_zone_device *tz, int trip_id,
					   struct thermal_trip *trip)
{
	const struct thermal_trip *trips;

	trips = of_thermal_get_trip_points(tz);
	if (!trips)
		return -EINVAL;
	*trip = trips[trip_id];

	return 0;
}

static inline int kv___thermal_zone_get_trip(struct thermal_zone_device *tz, int trip_id,
					     struct thermal_trip *trip)
{
	const struct thermal_trip *trips;

	trips = of_thermal_get_trip_points(tz);
	if (!trips)
		return -EINVAL;
	*trip = trips[trip_id];

	return 0;
}

#define kv_thermal_get_num_trips(tz) of_thermal_get_ntrips(tz)

typedef struct thermal_zone_of_device_ops kv_thermal_zone_device_ops;
typedef void kv_get_temp_p_or_tz;
static inline void *kv_get_temp_meson_data(kv_get_temp_p_or_tz *p)
{
	return p;
}

static inline struct thermal_zone_device *kv_devm_thermal_zone_register(
	struct device *dev, int sensor_id,
	void *data, const kv_thermal_zone_device_ops *ops)
{
	return devm_thermal_zone_of_sensor_register(dev, sensor_id, data, ops);
}

static inline void kv_devm_thermal_zone_unregister(struct device *dev,
						   struct thermal_zone_device *tz)
{
	devm_thermal_zone_of_sensor_unregister(dev, tz);
}

typedef int kv_uart_putchar_char_type;
#define KV_UART_SET_TERMIOS_OLD_CONST
#define KV_UART_XMIT_SIZE SERIAL_XMIT_SIZE

static inline int kv_usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	return usb_gadget_probe_driver(driver);
}

static inline u16 kv_usb_maxpacket(struct usb_device *udev, int pipe, int is_out)
{
	return usb_maxpacket(udev, pipe, is_out);
}

static inline int kv_snd_soc_of_get_dai_name(struct device_node *of_node,
					     const char **dai_name, int index)
{
	return snd_soc_of_get_dai_name(of_node, dai_name);
}

#define KV_SND_DAI_OPS_DEF_PROBE(func)
#define KV_SND_DAI_DRIVER_DEF_PROBE(func) .probe = func,

#define KV_SND_SOC_DRIVER_DEF_NON_LEGACY_NAMEING(i) .non_legacy_dai_naming = i,

#define KV_SCHED_SWITCH_HOOK_PREV_STAT

static inline int kv_snd_soc_card_jack_new(struct snd_soc_card *card, const char *id, int type,
					   struct snd_soc_jack *jack,
					   struct snd_soc_jack_pin *pins, unsigned int num_pins)
{
	return snd_soc_card_jack_new(card, id, type, jack, pins, num_pins);
}

#endif //__KERNEL_VERSIONS_5_15_H
