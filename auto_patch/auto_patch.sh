#!/bin/bash
# SPDX-License-Identifier: (GPL-2.0+ OR MIT)
#
# Copyright (c) 2019 Amlogic, Inc. All rights reserved.
#

function get_paths_and_enter_common_path()
{
	ENTER_PATH=$(pwd)
	AUTO_PATCH_PATH=$(dirname $(realpath -s "$0"))
	common_driver_path=$(dirname ${AUTO_PATCH_PATH})
	temp_path=$(dirname ${common_driver_path})
	if [[ -f ${temp_path}/init/main.c ]]; then
		common_kernel_path=${temp_path}
		COMMON_PATH=$(dirname ${temp_path})
	elif [[ -f ${temp_path}/common/init/main.c ]]; then
		common_kernel_path=${temp_path}/common
		COMMON_PATH=${temp_path}
	elif [[ -f ${KERNEL_DIR}/init/main.c ]]; then
		common_kernel_path=$(realpath -s ${KERNEL_DIR})
		COMMON_PATH=$(realpath -s ${common_kernel_path}/..)
	else
		echo "The directory of kernel does not exist";
		exit
	fi
	COMMON_DRIVERS_DIR=$(realpath --relative-to=${common_kernel_path} ${common_driver_path})
	KERNEL_DIR=${common_kernel_path##*/}
	cd ${COMMON_PATH}

	if [[ ${DEBUG} == 1 ]]; then
		echo ENTER_PATH=${ENTER_PATH}
		echo COMMON_PATH=${COMMON_PATH}
		echo KERNEL_DIR=${KERNEL_DIR}
		echo COMMON_DRIVERS_DIR=${COMMON_DRIVERS_DIR}
		echo AUTO_PATCH_PATH=${AUTO_PATCH_PATH}
	fi
}

function get_full_kernel_version()
{
	if [[ $# == 1 && -d ${AUTO_PATCH_PATH}/$1 ]]; then
		FULL_KERNEL_VERSION=$1
	elif [[ -f ${KERNEL_DIR}/build.config.constants || -f ${KERNEL_DIR}/build.config.common ]]; then
		version_message=$(grep -rn BRANCH= ${KERNEL_DIR}/build.config.constants)
		if [[ -z ${version_message} ]]; then
			version_message=$(grep -rn BRANCH= ${KERNEL_DIR}/build.config.common)
		fi
		if [[ -z ${version_message} ]]; then
			cd ${ENTER_PATH}
			echo "Can't find BRANCH in build.config.constants and build.config.common"
			exit
		fi
		version_message="common${version_message##*android}"
		FULL_KERNEL_VERSION=${version_message}
	else
		version=`grep "^VERSION = " common/Makefile | cut -d ' ' -f 3`
		patchlevel=`grep "^PATCHLEVEL = " common/Makefile | cut -d ' ' -f 3`
		matching_paths=$(find ${AUTO_PATCH_PATH} -type d -name "common*-${version}\.${patchlevel}")
		path_array=($(echo "${matching_paths}"))
		path_count=${#path_array[@]}
		if [[ $path_count == 1 ]]; then
			FULL_KERNEL_VERSION=${matching_paths##*/}
		else
			cd ${ENTER_PATH}
			echo "Can't find FULL_KERNEL_VERSION"
			exit
		fi
	fi
	if [[ ${DEBUG} == 1 ]]; then
		echo FULL_KERNEL_VERSION=${FULL_KERNEL_VERSION}
	fi
	PATCHES_PATH=${AUTO_PATCH_PATH}/${FULL_KERNEL_VERSION}
	if [[ ! -d ${PATCHES_PATH} ]]; then
		cd ${ENTER_PATH}
		echo "None patch to am, ${PATCHES_PATH}/${FULL_KERNEL_VERSION}/patches does not exist!!!"
		exit
	fi
	if [[ ${DEBUG} == 1 ]]; then
		echo PATCHES_PATH=${PATCHES_PATH}
	fi
}

function am_patch()
{
	local patch=$1
	local dir=$2
	local compare_id=$3
	local change_id=`grep 'Change-Id' $patch | head -n1 | awk '{print $2}'`

	if [ -d "${dir}" ]; then
		cd ${dir}
		if [[ ! "${compare_id[@]}" =~ "${change_id}" ]]; then
			if [[ ${DEBUG} == 1 ]]; then
				echo patch ${dir}/${patch}
			fi
			git am -q ${patch} 1>/dev/null 2>&1;
			if [ $? != 0 ]; then
				git am --abort
				cd ${ENTER_PATH}
				if [[ ${DEBUG} != 1 ]]; then
					echo
				fi
				echo "Patch Error : Failed to patch [$patch], Need check it. exit!!!"
				exit -1
			fi
			if [[ ${DEBUG} != 1 ]]; then
				echo -n .
			fi
		fi
		cd ${COMMON_PATH}
	fi
}

function auto_patch()
{
	local patch_dir=$1

	for file in `find ${patch_dir} -name "*.patch" | sort`; do
		local file_name=${file%.*};           #echo file_name $file_name
		local resFile=`basename $file_name`;  #echo resFile $resFile
		local dir_name1=${resFile//#/\/};     #echo dir_name $dir_name
		local dir_name=${dir_name1%/*};       #echo dir_name $dir_name
		local dir=${COMMON_PATH}/$dir_name;   #echo $dir

		[[ ! -d ${dir}/.git ]] && continue
		local compare_change_id=$(cd ${dir} && git log -n 50 | grep "Change-Id:" | awk '{print $2}')
		am_patch ${file} ${dir} "${compare_change_id[@]}"
	done
}
function auto_release_patch()
{
	local kernel_type=
	if (cd "${KERNEL_DIR}" && git log -n 200 | grep -q "release: kernel_aarch64 release patch flag"); then
		kernel_type=kernel_aarch64
	fi

	if (cd "${KERNEL_DIR}" && git log -n 200 | grep -q "release: kernel_aarch64_tv release patch flag"); then
		kernel_type=kernel_aarch64_tv
	fi

	if [[ -z ${kernel_type} ]]; then
		(cd "${KERNEL_DIR}" &&  git commit --allow-empty -m "release: ${KERNEL_IMAGE_TYPE} release patch flag")
	fi

	if [[ -n ${kernel_type} && ${kernel_type} != ${KERNEL_IMAGE_TYPE} ]]; then
		echo "Kernel Type '${kernel_type}' change to '${KERNEL_IMAGE_TYPE}'"
		echo -e "\e[31mWarning: save your modify in the '${KERNEL_DIR}' directory before execute the cmd\e[0m"
		echo -e "\e[33mExecute the following cmd and rebuild\e[0m"
		echo -e "\e[33m	cd ${KERNEL_DIR}\e[0m"
		echo -e "\e[33m	git reset --hard $(awk '/AML_PATCH_VERSION/ { match($0, /"([^"]+)"/, arr); print arr[1] }' common_drivers/include/linux/upstream_version.h)\e[0m"
		exit 1
	fi

	local release_change_id=$(cd ${KERNEL_DIR} && git log -n 200 | grep "Change-Id:" | awk '{print $2}')
	if [ -d ${PATCHES_PATH}/release/${KERNEL_IMAGE_TYPE} ]; then
		for patch in `find ${PATCHES_PATH}/release/${KERNEL_IMAGE_TYPE} -name "*.patch" | sort`; do
			am_patch ${patch} ${KERNEL_DIR} "${release_change_id[@]}"
		done
	fi
}

function traverse_patch_dir()
{
	# git am common and common_driver patches
	echo "Auto Patch Start"
	{
		local common_change_id=$(cd ${KERNEL_DIR} && git log -n 200 | grep "Change-Id:" | awk '{print $2}')
		auto_release_patch
		if [[ "${PATCH_PARM}" != "non_common" ]]; then
			for file in `ls ${PATCHES_PATH}/common`; do
				if [ -d ${PATCHES_PATH}/common/${file} ]; then
					for patch in `find ${PATCHES_PATH}/common/${file} -name "*.patch" | sort`; do
						am_patch ${patch} ${KERNEL_DIR} "${common_change_id[@]}"
					done
				fi
			done
		fi
	} &

	{
		if [[ -d ${PATCHES_PATH}/common_drivers ]]; then
			local common_drivers_change_id=$(cd ${KERNEL_DIR}/${COMMON_DRIVERS_DIR} && git log -n 100 | grep "Change-Id:" | awk '{print $2}')
			for patch in `find ${PATCHES_PATH}/common_drivers -name "*.patch" | sort`; do
				am_patch ${patch} ${KERNEL_DIR}/${COMMON_DRIVERS_DIR} "${common_drivers_change_id[@]}"
			done
		fi
	} &

	[[ -d ${PATCHES_PATH}/tools ]] && auto_patch ${PATCHES_PATH}/tools

	wait
	if [[ ${DEBUG} != 1 ]]; then
		echo
	fi
	echo "Patch Finish: ${COMMON_PATH}"
}

function handle_lunch_patch()
{
	echo "Auto Patch lunch's patch Start"

	auto_release_patch
	auto_patch ${PATCHES_PATH}/tools

	echo
	echo "Patch Finish: ${COMMON_PATH}"
}

function main()
{
	get_paths_and_enter_common_path
	get_full_kernel_version $@
	if [[ "${PATCH_PARM}" == "lunch" ]]; then
		handle_lunch_patch
	else
		traverse_patch_dir
	fi
	cd ${ENTER_PATH}
}

main $@
