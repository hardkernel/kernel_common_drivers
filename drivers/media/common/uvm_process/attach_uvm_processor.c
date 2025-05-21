// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/meson_uvm_allocator.h>
#include <linux/amlogic/meson_uvm_core.h>
#include <linux/amlogic/meson_uvm_interface.h>
#include "meson_uvm_aicolor_processor.h"
#include "meson_uvm_aiface_processor.h"
#include "meson_uvm_aipq_processor.h"
#include "meson_uvm_buffer_info.h"
#include "meson_uvm_lcevc_processor.h"
#include "meson_uvm_nn_processor.h"

int attach_uvm_info(struct dma_buf *dmabuf, int fd, int type, char *buf)
{
	struct uvm_hook_mod_info info;
	int ret = 0;

	memset(&info, 0, sizeof(struct uvm_hook_mod_info));
	info.type = PROCESS_INVALID;
	info.arg = NULL;
	info.acquire_fence = NULL;

	switch (type) {
	case PROCESS_AICOLOR:
		ret = attach_aicolor_hook_mod_info(fd, buf, &info);
		break;
	case PROCESS_AIFACE:
		ret = attach_aiface_hook_mod_info(fd, buf, &info);
		break;
	case PROCESS_AIPQ:
		ret = attach_aipq_hook_mod_info(fd, buf, &info);
		break;
	case PROCESS_LCEVC:
		ret = attach_lcevc_hook_mod_info(fd, buf, &info);
		break;
	case PROCESS_NN:
		ret = attach_nn_hook_mod_info(fd, buf, &info);
		break;
	default:
		pr_err("mod_type is not valid.\n");
	}
	if (ret) {
		pr_err("attach_hook_mod_info failed.\n");
		return -EINVAL;
	}

	if (IS_ERR_OR_NULL(dmabuf) || !dmabuf_is_uvm(dmabuf)) {
		pr_err("dmabuf is not uvm. %s %d\n", __func__, __LINE__);
		return -EINVAL;
	}

	if (info.type >= VF_SRC_DECODER && info.type < PROCESS_INVALID)
		ret = uvm_attach_hook_mod(dmabuf, &info);

	return ret;
}

int __init attach_uvm_processor_init(void)
{
	register_amlogic_attach_uvm_info_fun(attach_uvm_info);
	register_amlogic_get_uvm_video_info_fun(get_uvm_video_info);
	return 0;
}

void __exit attach_uvm_processor_exit(void)
{
	unregister_amlogic_attach_uvm_info_fun();
	unregister_amlogic_get_uvm_video_info_fun();
}
