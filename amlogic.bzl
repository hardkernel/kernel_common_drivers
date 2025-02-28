# SPDX-License-Identifier: GPL-2.0

load("//common_drivers:amlogic_utils.bzl", "define_common_amlogic")
load("//common_drivers:modules.bzl", "AMLOGIC_MODULES", "AMLOGIC_DDK_MODULES")
load("//common_drivers:project/project.bzl", "EXT_MODULES_ANDROID", "GKI_CONFIG", "KCONFIG_EXT_SRCS", "DTBO_DEVICETREE", "FULL_KERNEL_VERSION")
load("//common_drivers:project/dtb.bzl", "AMLOGIC_DTBS")

_AMLOGIC_DTBOS = DTBO_DEVICETREE or [ "android_overlay_dt.dtbo" ]

_AMLOGIC_OUTS = [
] + AMLOGIC_DTBS

_AMLOGIC_MODULES = [
] + AMLOGIC_MODULES

_EXT_MODULES = [
] + EXT_MODULES_ANDROID

_AMLOGIC_MAKE_GOALS = [
    "modules",
    "dtbs",
]

def define_amlogic():
    define_common_amlogic(
        name = "amlogic",
        outs = _AMLOGIC_OUTS,
        dtbo_srcs = _AMLOGIC_DTBOS,
        define_abi_targets = False,
        kmi_symbol_list = None,
        #additional_kmi_symbol_lists = native.glob(["common_drivers/android/%s_abi_gki_aarch64_amlogic*" % FULL_KERNEL_VERSION]) if GKI_CONFIG else None,
        kmi_symbol_list_add_only = False,
        build_config = ":build.config.amlogic.bazel",
        module_outs = _AMLOGIC_MODULES,
        ext_modules = AMLOGIC_DDK_MODULES + _EXT_MODULES,
        module_grouping = False,
        kconfig_ext = ":Kconfig.ext",
        kconfig_ext_srcs = KCONFIG_EXT_SRCS,
        make_goals = _AMLOGIC_MAKE_GOALS,
    )
