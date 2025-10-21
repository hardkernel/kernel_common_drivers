// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include "sys_def.h"		//ary add for sim
#include <linux/amlogic/media/di/dpss_interface.h>
#include <linux/kfifo.h>
#include <linux/types.h>

#include "./hw/dpss.h"
#include "dpss_frc_dbg.h"
#include "dpss_frc_memc_dbg.h"
#include "dpss_s.h"
#include "dpss_sys.h"
#include <linux/amlogic/media/dpss/frc_common_x.h>

ssize_t dpss_frc_bbd_ctrl_param_show(const struct class *class,
			const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_BBD_FINAL_LINE, buf);
	return len;
}

ssize_t dpss_frc_bbd_ctrl_param_store(const struct class *class,
				      const struct class_attribute *attr,
				      const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();

	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
		pfw_data->frc_alg_dbg_stor(pfw_data,
			MEMC_DBG_BBD_FINAL_LINE, buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_vp_ctrl_param_show(const struct class *class,
				    const struct class_attribute *attr, char *buf)
{
	ssize_t len = 0;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_VP_CTRL, buf);
	return len;
}

ssize_t dpss_frc_vp_ctrl_param_store(const struct class *class,
				     const struct class_attribute *attr,
				     const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_VP_CTRL, buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_logo_ctrl_param_show(const struct class *class,
				      const struct class_attribute *attr, char *buf)
{
	ssize_t len = 0;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_LOGO_CTRL, buf);
	return len;
}

ssize_t dpss_frc_logo_ctrl_param_store(const struct class *class,
				       const struct class_attribute *attr,
				       const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();

	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_LOGO_CTRL, buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_iplogo_ctrl_param_show(const struct class *class,
					const struct class_attribute *attr, char *buf)
{
	ssize_t len = 0;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_IPLOGO_CTRL, buf);
	return len;
}

ssize_t dpss_frc_iplogo_ctrl_param_store(const struct class *class,
					 const struct class_attribute *attr,
					 const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_IPLOGO_CTRL, buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_melogo_ctrl_param_show(const struct class *class,
					const struct class_attribute *attr, char *buf)
{
	ssize_t len = 0;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();

	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_MELOGO_CTRL, buf);
	return len;
}

ssize_t dpss_frc_melogo_ctrl_param_store(const struct class *class,
					const struct class_attribute *attr,
					const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();

	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_MELOGO_CTRL,
				buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_scene_chg_detect_param_show(const struct class *class,
		const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_SCENE_CHG_DETECT, buf);
	return len;
}

ssize_t dpss_frc_scene_chg_detect_param_store(const struct class *class,
		const struct class_attribute *attr, const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();

	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_SCENE_CHG_DETECT,
				buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_fb_ctrl_param_show(const struct class *class,
				    const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len = pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_FB_CTRL, buf);
	return len;
}

ssize_t dpss_frc_fb_ctrl_param_store(const struct class *class,
				     const struct class_attribute *attr,
				     const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_FB_CTRL, buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_me_ctrl_param_show(const struct class *class,
				    const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_ME_CTRL, buf);
	return len;
}

ssize_t dpss_frc_me_ctrl_param_store(const struct class *class,
				     const struct class_attribute *attr,
				     const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_ME_CTRL, buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_search_rang_param_show(const struct class *class,
					const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();

	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_SEARCH_RANG, buf);
	return len;
}

ssize_t dpss_frc_search_rang_param_store(const struct class *class,
					 const struct class_attribute *attr,
					 const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();

	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_SEARCH_RANG,
				buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_mc_ctrl_param_show(const struct class *class,
				    const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_PIXEL_LPF, buf);
	return len;
}

ssize_t dpss_frc_mc_ctrl_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_PIXEL_LPF, buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_me_rule_param_show(const struct class *class,
		const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_ME_RULE, buf);
	return len;
}

ssize_t dpss_frc_me_rule_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();

	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_ME_RULE, buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_film_ctrl_param_show(const struct class *class,
				      const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();

	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_FILM_CTRL, buf);
	return len;
}

ssize_t dpss_frc_film_ctrl_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_FILM_CTRL,
				buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_glb_ctrl_param_show(const struct class *class,
			const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_GLB_CTRL, buf);
	return len;
}

ssize_t dpss_frc_glb_ctrl_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
		pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_GLB_CTRL,
			buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_bad_edit_ctrl_param_show(const struct class *class,
					  const struct class_attribute *attr,
					  char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_BAD_EDIT_CTRL, buf);
	return len;
}

ssize_t dpss_frc_bad_edit_ctrl_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_BAD_EDIT_CTRL,
				buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_region_fb_ctrl_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_REGION_FB_CTRL, buf);
	return len;
}

ssize_t dpss_frc_region_fb_ctrl_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_REGION_FB_CTRL,
				buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_me_patch_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_ME_PATCH, buf);
	return len;
}

ssize_t dpss_frc_me_patch_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_ME_PATCH,
				buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_vp_rule_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_VP_RULE, buf);
	return len;
}

ssize_t dpss_frc_vp_rule_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_VP_RULE,
				buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_vp_patch_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_VP_PATCH, buf);
	return len;
}

ssize_t dpss_frc_vp_patch_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_VP_PATCH,
				buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_logo_rule_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_LOGO_RULE, buf);
	return len;
}

ssize_t dpss_frc_logo_rule_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_LOGO_RULE,
				buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_logo_patch_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_LOGO_PATCH, buf);
	return len;
}

ssize_t dpss_frc_logo_patch_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_LOGO_PATCH,
				buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_bbd_rule_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_BBD_RULE, buf);
	return len;
}

ssize_t dpss_frc_bbd_rule_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_BBD_RULE,
				buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_mc_rule_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_MC_RULE, buf);
	return len;
}

ssize_t dpss_frc_mc_rule_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_MC_RULE,
				buf_orig, count);
	kfree(buf_orig);
	return count;
}

ssize_t dpss_frc_film_out_ctrl_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (pfw_data && pfw_data->frc_alg_dbg_show)
		len =
			pfw_data->frc_alg_dbg_show(pfw_data, MEMC_DBG_FILM_OUT_CTRL, buf);
	return len;
}

ssize_t dpss_frc_film_out_ctrl_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count)
{
	char *buf_orig;
	struct dpss_frc_fw_data_s *pfw_data;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	buf_orig = kstrdup(buf, GFP_KERNEL);
	if (pfw_data && pfw_data->frc_alg_dbg_stor)
		count =
			pfw_data->frc_alg_dbg_stor(pfw_data, MEMC_DBG_FILM_OUT_CTRL,
				buf_orig, count);
	kfree(buf_orig);
	return count;
}
