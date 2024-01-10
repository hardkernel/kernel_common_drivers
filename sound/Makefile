# SPDX-License-Identifier: (GPL-2.0+ OR MIT)

subdir-ccflags-y += "-D__module_param(name, type, perm)="
subdir-ccflags-y += "-D__module_param_named(name, value, type, perm)="
subdir-ccflags-y += "-D__module_param_array(name, type, nump, perm)="
subdir-ccflags-y += "-D__module_param_cb(name, ops, arg, perm)="
subdir-ccflags-y += "-D__module_param_array_named(name, array, type, nump, perm)="

obj-$(CONFIG_SND)	+= soc/

KBUILD_CFLAGS_MODULE += $(GKI_EXT_KERNEL_MODULE_PREDEFINE)
all:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules $(GKI_EXT_KERNEL_MODULE_CONFIG)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean
