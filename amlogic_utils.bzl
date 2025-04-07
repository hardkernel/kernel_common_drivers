"""Functions that are useful in the common kernel package (usually `//common`)."""

load("@bazel_skylib//lib:dicts.bzl", "dicts")
load("@bazel_skylib//lib:selects.bzl", "selects")
load("@bazel_skylib//rules:common_settings.bzl", "bool_flag", "string_flag")
load("@bazel_skylib//rules:write_file.bzl", "write_file")
load("@rules_pkg//pkg:install.bzl", "pkg_install")
load("@rules_pkg//pkg:mappings.bzl", "pkg_files", "strip_prefix")
load(
    "//build/kernel/kleaf:kernel.bzl",
    "dtbo",
    "initramfs",
    "kernel_abi",
    "kernel_abi_dist",
    "kernel_build",
    "kernel_build_config",
    "kernel_compile_commands",
    "kernel_dtstree",
    "kernel_filegroup",
    "kernel_images",
    "kernel_kythe",
    "kernel_modules_install",
    "kernel_unstripped_modules_archive",
    "merged_kernel_uapi_headers",
)
load("//build/bazel_common_rules/dist:dist.bzl", "copy_to_dist_dir")
load("//build/kernel/kleaf:print_debug.bzl", "print_debug")

def define_common_amlogic(
        name,
        branch,
        outs,
        dtbo_srcs,
        build_config = None,
        module_outs = None,
        make_goals = None,
        define_abi_targets = None,
        kmi_symbol_list = None,
        additional_kmi_symbol_lists = None,
        module_grouping = None,
        unstripped_modules_archive = None,
        dist_dir = None,
        ext_modules = None,
        kconfig_ext = None,
        kconfig_ext_srcs = None,
        ddk_module_defconfig_fragments = None):

    if build_config == None:
        build_config = ":build.config.amlogic.bazel"

    kmi_symbol_list_add_only = True if define_abi_targets else None

    if dist_dir == None:
        dist_dir = "out/{}/dist".format(branch)

    kernel_dtstree(
        name = name + "_dtstree",
        srcs = native.glob([
            "arch/arm64/boot/dts/**",
            "include/dt-bindings/**",
        ]),
        makefile = "arch/arm64/boot/dts/Makefile",
    )

    kernel_build(
        name = name,
        outs = outs,
        srcs = [":amlogic_sources"] + kconfig_ext_srcs,
        # List of in-tree kernel modules.
        module_outs = module_outs,
        build_config = build_config,
        # Enable mixed build.
        base_kernel = "//common:kernel_aarch64",
        # defconfig = "//common:arch/arm64/configs/gki_defconfig",
        # pre_defconfig_fragments = ["arch/arm64/configs/amlogic_gki.fragment"],
        # check_defconfig = "disabled",
        ddk_module_defconfig_fragments = ddk_module_defconfig_fragments,
        kmi_symbol_list = kmi_symbol_list,
        additional_kmi_symbol_lists = additional_kmi_symbol_lists,
        collect_unstripped_modules = True,
        strip_modules = True,
        make_goals = make_goals,
        makefile = "//common:Makefile",
        kconfig_ext = kconfig_ext,
        dtstree = ":" + name + "_dtstree",
        visibility = ["//visibility:public"],
    )

    kernel_abi(
        name = name + "_abi",
        kernel_build = name,
        define_abi_targets = define_abi_targets,
        kernel_modules = ext_modules,
        kmi_symbol_list_add_only = True,		#kmi_symbol_list_add_only,
        module_grouping = module_grouping,
    )

    kernel_modules_install(
        name = name + "_modules_install",
        kernel_build = name,
        # List of external modules.
        kernel_modules = ext_modules,
    )

    merged_kernel_uapi_headers(
        name = name + "_merged_kernel_uapi_headers",
        kernel_build = name,
        kernel_modules = ext_modules,
    )

    initramfs(
        name = name + "_initramfs",
        kernel_modules_install = name + "_modules_install",
    )

    dtbo(
        name = name + "_dtbo",
        srcs = [":" + name + "/" + e for e in dtbo_srcs],
        kernel_build = name,
    )

    pkg_files(
        name = name + "_dist_files",
        srcs = [
            name,
            name + "_dtbo",
            name + "_initramfs",
            name + "_modules_install",
            name + "_merged_kernel_uapi_headers",
            # Mixed build: Additional GKI artifacts.
            "//common:kernel_aarch64",
            "//common:kernel_aarch64_additional_artifacts",
        ],
        strip_prefix = strip_prefix.files_only(),
        visibility = ["//visibility:private"],
    )

    pkg_install(
        name = name + "_dist",
        srcs = [":" + name + "_dist_files"],
        destdir = dist_dir,
    )

def m_k_files():
    native.filegroup(
        name = "m_k_files",
        srcs = native.glob(
            [
               "Kconfig",
               "**/Kconfig",
               "Makefile",
               "**/Makefile",
            ],
        ),
        visibility = ["//visibility:public"],
    )
