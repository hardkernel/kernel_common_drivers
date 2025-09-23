// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>

#include <linux/amlogic/meson_uvm_interface.h>

static AMLOGIC_V4LVIDEO_data_copy_fun_t g_v4lvideo_fun;
static AMLOGIC_ATTACH_uvm_info_fun_t g_attach_fun;
static AMLOGIC_GET_UVM_video_info_fun_t g_video_fun;

void AMLOGIC_V4LVIDEO_data_copy(struct v4l_data_t *v4l_data,
			       struct dma_buf *dmabuf, u32 align)
{
	if (g_v4lvideo_fun) {
		g_v4lvideo_fun(v4l_data, dmabuf, align);
		return;
	}
	pr_err("no %s ERRR!!\n", __func__);
}
EXPORT_SYMBOL(AMLOGIC_V4LVIDEO_data_copy);

void register_amlogic_v4lvideo_data_copy_fun(AMLOGIC_V4LVIDEO_data_copy_fun_t fn)
{
	if (g_v4lvideo_fun)
		pr_err("error!!,AMLOGIC_V4LVIDEO_data_copy have register\n");

	g_v4lvideo_fun = fn;
}
EXPORT_SYMBOL(register_amlogic_v4lvideo_data_copy_fun);

void unregister_amlogic_v4lvideo_data_copy_fun(void)
{
	g_v4lvideo_fun = NULL;
}
EXPORT_SYMBOL(unregister_amlogic_v4lvideo_data_copy_fun);

int AMLOGIC_ATTACH_uvm_info(struct dma_buf *dmabuf, int fd, int type, char *buf)
{
	if (g_attach_fun)
		return g_attach_fun(dmabuf, fd, type, buf);
	pr_err("no %s ERRR!!\n", __func__);
	return -1;
}
EXPORT_SYMBOL(AMLOGIC_ATTACH_uvm_info);

int register_amlogic_attach_uvm_info_fun(AMLOGIC_ATTACH_uvm_info_fun_t fn)
{
	if (g_attach_fun) {
		pr_err("error!!,AMLOGIC_ATTACH_uvm_info have register\n");
		return -1;
	}

	g_attach_fun = fn;
	return 0;
}
EXPORT_SYMBOL(register_amlogic_attach_uvm_info_fun);

int unregister_amlogic_attach_uvm_info_fun(void)
{
	g_attach_fun = NULL;

	return 0;
}
EXPORT_SYMBOL(unregister_amlogic_attach_uvm_info_fun);

int AMLOGIC_GET_UVM_video_info(const int fd, struct uvm_fd_info *info)
{
	if (g_video_fun)
		return g_video_fun(fd, info);
	pr_err("no %s ERRR!!\n", __func__);
	return -1;
}

int register_amlogic_get_uvm_video_info_fun(AMLOGIC_GET_UVM_video_info_fun_t fn)
{
	if (g_video_fun) {
		pr_err("error!!,AMLOGIC_GET_UVM_video_info have register\n");
		return -1;
	}

	g_video_fun = fn;
	return 0;
}
EXPORT_SYMBOL(register_amlogic_get_uvm_video_info_fun);

int unregister_amlogic_get_uvm_video_info_fun(void)
{
	g_video_fun = NULL;

	return 0;
}
EXPORT_SYMBOL(unregister_amlogic_get_uvm_video_info_fun);
