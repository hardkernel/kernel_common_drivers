# SPDX-License-Identifier: GPL-2.0

load("//common_drivers:amlogic_utils.bzl", "define_common_amlogic")
load("//common_drivers:modules.bzl", "define_modules")

_AMLOGIC_MAKE_GOALS = [
    "modules",
    "dtbs",
]

def define_amlogic(
    name = None,
    project_configs = None):

    modules = define_modules(project_configs)
    define_common_amlogic(
        name = name,
        branch = project_configs.BRANCH,
        outs = project_configs.AMLOGIC_DTBS,
        dtbo_srcs = project_configs.DTBO_DEVICETREE or [ "android_overlay_dt.dtbo" ],
        define_abi_targets = True if project_configs.GKI_CONFIG else False,
        kmi_symbol_list = "//common:gki/aarch64/symbols/amlogic" if project_configs.GKI_CONFIG else None,
        additional_kmi_symbol_lists = native.glob(["gki/aarch64/symbols/%s_*" % project_configs.FULL_KERNEL_VERSION]) if project_configs.GKI_CONFIG else None,
        build_config = "//common_drivers:build_config",
        module_outs = modules.AMLOGIC_MODULES,
        ext_modules = modules.AMLOGIC_DDK_MODULES + project_configs.EXT_MODULES_ANDROID,
        module_grouping = False,
        kconfig_ext = "//common_drivers:Kconfig.ext",
        kconfig_ext_srcs = project_configs.KCONFIG_EXT_SRCS,
        make_goals = _AMLOGIC_MAKE_GOALS,
        ddk_module_defconfig_fragments = None,
        ddk_module_headers = ["//common_drivers:soc_headers"],
    )
