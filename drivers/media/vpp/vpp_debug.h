/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#ifndef __VPP_DEBUG_H__
#define __VPP_DEBUG_H__

int vpp_debug_init(struct vpp_dev_s *dev);
ssize_t vpp_debug_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_reg_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_reg_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_cm_reg_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_cm_reg_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_brightness_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_brightness_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_brightness_post_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_brightness_post_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_contrast_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_contrast_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_contrast_post_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_contrast_post_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_saturation_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_saturation_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_saturation_post_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_saturation_post_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_hue_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_hue_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_hue_post_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_hue_post_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_sharpness_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_sharpness_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_pre_gamma_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_pre_gamma_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_pre_gamma_pattern_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_pre_gamma_pattern_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_gamma_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_gamma_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_gamma_pattern_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_gamma_pattern_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_white_balance_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_white_balance_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_module_ctrl_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_module_ctrl_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_dnlp_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_dnlp_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_lc_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_lc_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_hdr_type_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_hdr_type_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_pc_mode_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_pc_mode_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_csc_type_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_csc_type_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_color_primary_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_color_primary_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_histogram_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_histogram_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_histogram_ave_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_histogram_ave_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_histogram_hdr_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_histogram_hdr_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_hdr_metadata_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_hdr_metadata_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_matrix_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_matrix_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_eye_protect_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_eye_protect_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_color_curve_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_color_curve_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_overscan_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_overscan_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_blkext_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_blkext_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_ccoring_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_ccoring_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_blue_stretch_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_blue_stretch_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_lut3d_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_lut3d_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);
ssize_t vpp_debug_data_show(const struct class *class,
	const struct class_attribute *attr, char *buf);
ssize_t vpp_debug_data_store(const struct class *class,
	const struct class_attribute *attr, const char *buf, size_t count);

#endif
#endif

