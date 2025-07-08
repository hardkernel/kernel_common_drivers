# SPDX-License-Identifier: GPL-2.0

load("//common_drivers:amlogic_utils.bzl", "define_common_amlogic")
load("//common_drivers:modules.bzl", "define_modules")

_AMLOGIC_MAKE_GOALS = [
    "modules",
    "dtbs",
]

AMLOGIC_KERNEL_CONFIGS = {
    "kernel_aarch64_tv": {
        "base_kernel": "//common:kernel_aarch64_tv",
        "pre_defconfig_fragments": ["//common:arch/arm64/configs/tv_gki.fragment"],
        "page_size": None,
        "base_pkg_files": [
              # Mixed build: Additional GKI artifacts.
              "//common:kernel_aarch64_tv",
              "//common:kernel_aarch64_tv_additional_artifacts",
        ],
    },
    "kernel_aarch64": {
        "base_kernel": "//common:kernel_aarch64",
        "pre_defconfig_fragments": None,
        "page_size": None,
        "base_pkg_files": [
              # Mixed build: Additional GKI artifacts.
              "//common:kernel_aarch64",
              "//common:kernel_aarch64_additional_artifacts",
        ],
    },
    "kernel_aarch64_16k": {
        "base_kernel": "//common:kernel_aarch64_16k",
        "pre_defconfig_fragments": None,
        "page_size": "16k",
        "base_pkg_files": [
              # Mixed build: Additional GKI artifacts.
              "//common:kernel_aarch64_16k",
              "//common:kernel_aarch64_16k_additional_artifacts",
        ],
    },
}

def get_kernel_config(kernel_image_type):
    if kernel_image_type not in AMLOGIC_KERNEL_CONFIGS:
        print("Warning: Invalid KERNEL_IMAGE_TYPE, using default 'kernel_aarch64_tv'")
    return AMLOGIC_KERNEL_CONFIGS.get(kernel_image_type, AMLOGIC_KERNEL_CONFIGS["kernel_aarch64_tv"])

def get_base_kernel(kernel_image_type):
    return get_kernel_config(kernel_image_type)["base_kernel"]

def get_pre_defconfig_fragments(kernel_image_type):
    return get_kernel_config(kernel_image_type)["pre_defconfig_fragments"]

def get_page_size(kernel_image_type):
    return get_kernel_config(kernel_image_type)["page_size"]

def get_base_pkg_files(kernel_image_type):
    return get_kernel_config(kernel_image_type)["base_pkg_files"]

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
        base_kernel = get_base_kernel(project_configs.KERNEL_IMAGE_TYPE),
        pre_defconfig_fragments = get_pre_defconfig_fragments(project_configs.KERNEL_IMAGE_TYPE),
        page_size = get_page_size(project_configs.KERNEL_IMAGE_TYPE),
        base_pkg_files = get_base_pkg_files(project_configs.KERNEL_IMAGE_TYPE),
        module_outs = modules.AMLOGIC_MODULES,
        ext_modules = modules.AMLOGIC_DDK_MODULES + project_configs.EXT_MODULES_ANDROID,
        module_grouping = False,
        kconfig_ext = "//common_drivers:Kconfig.ext",
        kconfig_ext_srcs = project_configs.KCONFIG_EXT_SRCS,
        make_goals = _AMLOGIC_MAKE_GOALS,
        ddk_module_defconfig_fragments = None,
        ddk_module_headers = ["//common_drivers:soc_headers"],
    )
