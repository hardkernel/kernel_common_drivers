/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __DPSS_FRC_MEMC_DBG_H__
#define __DPSS_FRC_MEMC_DBG_H__

ssize_t dpss_frc_bbd_ctrl_param_show(const struct class *class,
				     const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_bbd_ctrl_param_store(const struct class *class,
				      const struct class_attribute *attr,
				      const char *buf, size_t count);
ssize_t dpss_frc_vp_ctrl_param_show(const struct class *class,
				    const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_vp_ctrl_param_store(const struct class *class,
				     const struct class_attribute *attr,
				     const char *buf, size_t count);
ssize_t dpss_frc_logo_ctrl_param_show(const struct class *class,
				      const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_logo_ctrl_param_store(const struct class *class,
				       const struct class_attribute *attr,
				       const char *buf, size_t count);
ssize_t dpss_frc_iplogo_ctrl_param_show(const struct class *class,
					const struct class_attribute *attr,
					char *buf);
ssize_t dpss_frc_iplogo_ctrl_param_store(const struct class *class,
					 const struct class_attribute *attr,
					 const char *buf, size_t count);

ssize_t dpss_frc_melogo_ctrl_param_show(const struct class *class,
					const struct class_attribute *attr,
					char *buf);
ssize_t dpss_frc_melogo_ctrl_param_store(const struct class *class,
					 const struct class_attribute *attr,
					 const char *buf, size_t count);

ssize_t dpss_frc_scene_chg_detect_param_show(const struct class *class,
					     const struct class_attribute *attr,
					     char *buf);
ssize_t dpss_frc_scene_chg_detect_param_store(const struct class *class,
					      const struct class_attribute *attr,
					      const char *buf, size_t count);

ssize_t dpss_frc_fb_ctrl_param_show(const struct class *class,
				    const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_fb_ctrl_param_store(const struct class *class,
				     const struct class_attribute *attr,
				     const char *buf, size_t count);

ssize_t dpss_frc_me_ctrl_param_show(const struct class *class,
				    const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_me_ctrl_param_store(const struct class *class,
				     const struct class_attribute *attr,
				     const char *buf, size_t count);

ssize_t dpss_frc_search_rang_param_show(const struct class *class,
					const struct class_attribute *attr,
					char *buf);
ssize_t dpss_frc_search_rang_param_store(const struct class *class,
					 const struct class_attribute *attr,
					 const char *buf, size_t count);

ssize_t dpss_frc_mc_ctrl_param_show(const struct class *class,
				    const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_mc_ctrl_param_store(const struct class *class,
				     const struct class_attribute *attr,
				     const char *buf, size_t count);

ssize_t dpss_frc_me_rule_param_show(const struct class *class,
				    const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_me_rule_param_store(const struct class *class,
				     const struct class_attribute *attr,
				     const char *buf, size_t count);

ssize_t dpss_frc_film_ctrl_param_show(const struct class *class,
				      const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_film_ctrl_param_store(const struct class *class,
				       const struct class_attribute *attr,
				       const char *buf, size_t count);

ssize_t dpss_frc_glb_ctrl_param_show(const struct class *class,
				     const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_glb_ctrl_param_store(const struct class *class,
				      const struct class_attribute *attr,
				      const char *buf, size_t count);

ssize_t dpss_frc_bad_edit_ctrl_param_show(const struct class *class,
					  const struct class_attribute *attr,
					  char *buf);
ssize_t dpss_frc_bad_edit_ctrl_param_store(const struct class *class,
					   const struct class_attribute *attr,
					   const char *buf, size_t count);

ssize_t dpss_frc_region_fb_ctrl_param_show(const struct class *class,
					   const struct class_attribute *attr,
					   char *buf);
ssize_t dpss_frc_region_fb_ctrl_param_store(const struct class *class,
					    const struct class_attribute *attr,
					    const char *buf, size_t count);

ssize_t dpss_frc_me_patch_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_me_patch_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t dpss_frc_vp_rule_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_vp_rule_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t dpss_frc_vp_patch_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_vp_patch_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t dpss_frc_logo_rule_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_logo_rule_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t dpss_frc_logo_patch_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_logo_patch_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t dpss_frc_bbd_rule_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_bbd_rule_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t dpss_frc_mc_rule_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_mc_rule_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t dpss_frc_film_out_ctrl_param_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t dpss_frc_film_out_ctrl_param_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);

#endif
