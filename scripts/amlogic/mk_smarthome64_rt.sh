#!/bin/bash
# SPDX-License-Identifier: (GPL-2.0+ OR MIT)
#
# Copyright (c) 2019 Amlogic, Inc. All rights reserved.
#

ROOT_DIR=`pwd`

ARCH=arm64
DEFCONFIG=meson64_a64_smarthome_rt_defconfig

KCONFIG_CONFIG=${ROOT_DIR}/common_drivers/arch/arm64/configs/${DEFCONFIG}
export KCONFIG_CONFIG

${ROOT_DIR}/common/scripts/kconfig/merge_config.sh -m -r \
	${ROOT_DIR}/common_drivers/arch/arm64/configs/meson64_a64_smarthome_defconfig \
	${ROOT_DIR}/common_drivers/arch/arm64/configs/amlogic_preempt_rt.fragment

export -n KCONFIG_CONFIG

source ${ROOT_DIR}/common_drivers/scripts/amlogic/mk_smarthome_common.sh $@

rm ${KCONFIG_CONFIG}*
