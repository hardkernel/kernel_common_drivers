// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include "vpq_debug.h"
#include "vpq_printk.h"
#include "vpq_common.h"
#include "vpq_table_logic.h"
#include "modules/vpq_module_dump.h"

unsigned int log_lev;

#define STATE_AS_STRING(state) (((state) == 1) ? "enable" : "disable")

#if ENABLE_USAGE_STR
static const char *vpq_debug_usage_str = {
	"Description:\n"
	" vpq module support below debug cmd:\n"
	" 1. get log level\n"
	"    --> cat /sys/class/vpq/log_lev\n"
	" 2. set log level\n"
	"    --> echo log_lev xx > /sys/class/vpq/log_lev\n"
	"\n"
	" 3. get chip id\n"
	"    --> cat /sys/class/vpq/chip_infor\n"
	"\n"
	" 4. get reg table version\n"
	"    --> cat /sys/class/vpq/reg_table_ver\n"
	"\n"
	" 5. get pq modules config\n"
	"    --> cat /sys/class/vpq/module_cfg\n"
	"\n"
	" 6. get aml_vpp pq modules status\n"
	"    --> cat /sys/class/vpq/module_status\n"
	" 7. set aml_vpp pq modules status\n"
	"    --> echo xx yy > /sys/class/vpq/module_status\n"
	"\n"
	" 8. get source signal infor\n"
	"    --> cat /sys/class/vpq/src_infor\n"
	"\n"
	" 9. get other infor\n"
	"    --> cat /sys/class/vpq/other_infor\n"
	"\n"
	" 10. get his information\n"
	"    --> cat /sys/class/vpq/his_avg\n"
	"\n"
	" 11. set pq setting value\n"
	"    --> echo xx yy > /sys/class/vpq/pq_setting\n"
	"\n"
	" 12. dump register table\n"
	"    --> echo xx > /sys/class/vpq/dump_table\n"
	"\n"
};

static const char *vpq_log_lev_usage_str = {
	"Usage:\n"
	"echo log_lev value > /sys/class/vpq/log_lev\n"
	"--> value: 1(ioctl); 2(table); 3(module); 4(vfm); 5(proc);\n"
	"\n"
};

static const char *vpq_module_status_usage_str = {
	"Usage:\n"
	"echo pq_module value1 value2 > /sys/class/vpq/module_status\n"
	"--> value1: vadj1; vadj2; pregamma; gamma; wb; dnlp; ccoring;\n"
	"            sr0; sr1; lc; cm; ble; bls; lut3d; dejaggy_sr0;\n"
	"            dejaggy_sr1; dering_sr0; dering_sr1; all;\n"
	"--> value2: 0~1\n"
	"\n"
};

static const char *vpq_pq_setting_usage_str = {
	"Usage:\n"
	"echo bri value > /sys/class/vpq/pq_setting\n"
	"--> value: -512 ~ +512\n"
	"\n"
	"echo con value > /sys/class/vpq/pq_setting\n"
	"--> value: -1023 ~ +1023\n"
	"\n"
	"echo sat value > /sys/class/vpq/pq_setting\n"
	"--> value: -127 ~ +127\n"
	"\n"
	"echo hue value > /sys/class/vpq/pq_setting\n"
	"--> value: -127 ~ +127\n"
	"\n"
	"echo sharp value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 255\n"
	"\n"
	"echo bri_post value > /sys/class/vpq/pq_setting\n"
	"--> value: -512 ~ +512\n"
	"\n"
	"echo cont_post value > /sys/class/vpq/pq_setting\n"
	"--> value: -1023 ~ +1023\n"
	"\n"
	"echo sat_post value > /sys/class/vpq/pq_setting\n"
	"--> value: -127 ~ +127\n"
	"\n"
	"echo hue_post value > /sys/class/vpq/pq_setting\n"
	"--> value: -127 ~ +127\n"
	"\n"
	"echo dnlp value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 3\n"
	"\n"
	"echo lc value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 3\n"
	"\n"
	"echo blkext value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 3\n"
	"\n"
	"echo rgb_ogo value1...value9 > /sys/class/vpq/pq_setting\n"
	"--> value1 ~ value3: -1024~1023 (r_pre_offset; g_pre_offset; b_pre_offset)\n"
	"--> value4 ~ value6: 0~2047 (r_gain; g_gain; b_gain;)\n"
	"--> value7 ~ value9: -1024~1023 (r_post_offset; g_post_offset; b_post_offset)\n"
	"\n"
	"echo cms value1...value5 > /sys/class/vpq/pq_setting\n"
	"--> value1: 0~1 (color_type: 9_color; 14_color)\n"
	"--> value2: 0~8 (color_9: read; green; blue; cyan; magenta; yellow; skin; yellow_green; blue_green)\n"
	"--> value3: 0~13 (color_14: blue_purple; purple; purple_red; red; fleshtone_cheeks; fleshtone_hair_cheeks;\n"
	"                  fleshtone_yellow; yellow; yellow_green; green; green_cyan; cyan; cyan_blue; blue)\n"
	"--> value4: 0~2 (cms_type: sat; hue; luma)\n"
	"--> value5: -128~127\n"
	"\n"
	"echo blue_stretch value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 3\n"
	"\n"
	"echo aipq value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 3\n"
	"\n"
	"echo aisr value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 3\n"
	"\n"
	"echo csc_type value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 3\n"
	"\n"
	"echo chroma_coring value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 3\n"
	"\n"
	"echo amdv_pic_mode_id value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 255\n"
	"\n"
	"echo amdv_dark_detail value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 255\n"
	"\n"
	"echo amdv_light_sensor value1 value2 value3 > /sys/class/vpq/pq_setting\n"
	"--> value1: 0 ~ 1 (flag)\n"
	"--> value2: 0 ~ 255 (t_frontLux)\n"
	"--> value3: 0 ~ 255 (t_rearLum)\n"
	"\n"
	"echo amdv_precision_detail value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 255\n"
	"\n"
	"echo memc_en value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 1\n"
	"\n"
	"echo memc_deblur value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 255\n"
	"\n"
	"echo memc_dejudder value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 255\n"
	"\n"
	"echo memc_demo value > /sys/class/vpq/pq_setting\n"
	"--> value: 0 ~ 255\n"
	"\n"
};

static const char *vpq_dump_table_usage_str = {
	"Usage:\n"
	"echo dump_data value > /sys/class/vpq/dump_table\n"
	"--> value: 0(dnlp); 1(hdrtmo); 2(aipq); 3(lc); 4(chroma cor); 5(black ext)\n"
	"           6(blue stretch);\n"
	"           20(nr); 21(deblock)); 22(demosquito); 23(smoothplus); 24(di);\n"
	"\n"
};
#endif

static void parse_param(char *pbuf, char **param)
{
	char *ps, *token;
	unsigned int n = 0;
	char delim1[3] = " ";
	char delim2[2] = "\n";

	ps = pbuf;
	strcat(delim1, delim2);
	while (1) {
		token = strsep(&ps, delim1);
		if (!token)
			break;
		if (*token == '\0')
			continue;
		param[n++] = token;
	}
}

static int atoi(const char *str)
{
	int n = 0;

	while (*str != '\0') {
		n = n * 10 + (*str - '0');
		str++;
	}

	return n;
}

ssize_t vpq_debug_cmd_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf)
{
#if ENABLE_USAGE_STR
	return sprintf(buf, "%s\n", vpq_debug_usage_str);
#else
	return 0;
#endif
}

ssize_t vpq_debug_cmd_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count)
{
	return count;
}

ssize_t vpq_log_lev_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf)
{
	pr_inf(lev_dbg, "show log_lev: %d\n", log_lev);
	pr_inf(lev_dbg, "\n");

#if ENABLE_USAGE_STR
	return sprintf(buf, "%s\n", vpq_log_lev_usage_str);
#else
	return 0;
#endif
}

ssize_t vpq_log_lev_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count)
{
	char *buf_org;
	char *param[8];

	buf_org = kstrdup(buf, GFP_KERNEL);
	if (!buf_org)
		return -ENOMEM;

	parse_param(buf_org, param);

	if (!strcmp(param[0], "log_lev")) {
		if (kstrtouint(param[1], 10, &log_lev) < 0)
			goto free_buf;

		pr_inf(lev_dbg, "log_lev %d\n", log_lev);
	} else {
		pr_inf(lev_dbg, "cmd error\n");
		goto free_buf;
	}

free_buf:
	kfree(buf_org);

	return count;
}

#if OPEN_PRINTK
static char *chip_type_tostring(enum vpq_chip_class_e chip_type)
{
	if (chip_type == CHIP_CLASS_NULL)
		return "NULL";
	else if (chip_type == VPQ_CHIP_TV)
		return "TV";
	else if (chip_type == VPQ_CHIP_STB)
		return "STB";
	else if (chip_type == VPQ_CHIP_SMT)
		return "SMT";
	else if (chip_type == VPQ_CHIP_AD)
		return "AD";
	else
		return "NULL";
}

static char *chip_id_tostring(enum vpq_chip_e chip_id)
{
	if (chip_id == CHIP_ID_NULL)
		return "NULL";
	else if (chip_id == VPQ_CHIP_T5W)
		return "T5W";
	else if (chip_id == VPQ_CHIP_T3)
		return "T3";
	else if (chip_id == VPQ_CHIP_T6W)
		return "T6W";
	else if (chip_id == VPQ_CHIP_T6X)
		return "T6X";
	else
		return "NULL";
}
#endif

ssize_t vpq_chip_infor_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf)
{
	__maybe_unused enum vpq_chip_class_e chip_type = vpq_get_chip_type();
	__maybe_unused enum vpq_chip_e chip_id = vpq_get_chip_id();

	pr_inf(lev_dbg, "current chip information:\n");
	pr_inf(lev_dbg, "  type:%s\n", chip_type_tostring(chip_type));
	pr_inf(lev_dbg, "  id:%s\n", chip_id_tostring(chip_id));
	pr_inf(lev_dbg, "--------------------------------\n");

	return 0;
}

ssize_t vpq_chip_infor_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count)
{
	return count;
}

ssize_t vpq_reg_table_ver_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf)
{
	pr_inf(lev_dbg, "current pq table ver information\n");
	pr_inf(lev_dbg, "  project_version :%s\n", pq_table_ver.project_version);
	pr_inf(lev_dbg, "  chip_version    :%s\n", pq_table_ver.chip_version);
	pr_inf(lev_dbg, "  table_version   :%s\n", pq_table_ver.table_version);
	pr_inf(lev_dbg, "  oem_model       :%s\n", pq_table_ver.oem_model);
	pr_inf(lev_dbg, "  panel_index     :%s\n", pq_table_ver.panel_index);
	pr_inf(lev_dbg, "--------------------------------\n");

	return 0;
}

ssize_t vpq_reg_table_ver_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count)
{
	return count;
}

ssize_t vpq_pq_setting_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf)
{
#if ENABLE_USAGE_STR
	return sprintf(buf, "%s\n", vpq_pq_setting_usage_str);
#else
	return 0;
#endif
}

ssize_t vpq_pq_setting_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count)
{
	char *buf_org;
	char *param[20];
	int value = 0;

	buf_org = kstrdup(buf, GFP_KERNEL);
	if (!buf_org)
		return -ENOMEM;

	parse_param(buf_org, param);

	if (!strcmp(param[0], "bri")) {
		if (kstrtoint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_brightness(value);
	} else if (!strcmp(param[0], "con")) {
		if (kstrtoint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_contrast(value);
	} else if (!strcmp(param[0], "sat")) {
		if (kstrtoint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_saturation(value);
	} else if (!strcmp(param[0], "hue")) {
		if (kstrtoint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_hue(value);
	} else if (!strcmp(param[0], "sharp")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_sharpness(value);
	} else if (!strcmp(param[0], "bri_post")) {
		if (kstrtoint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_brightness_post(value);
	} else if (!strcmp(param[0], "con_post")) {
		if (kstrtoint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_contrast_post(value);
	} else if (!strcmp(param[0], "sat_post")) {
		if (kstrtoint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_saturation_post(value);
	} else if (!strcmp(param[0], "hue_post")) {
		if (kstrtoint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_hue_post(value);
	} else if (!strcmp(param[0], "rgb_ogo")) {
		struct vpq_rgb_ogo_s rgb_ogo = {0};

		if (kstrtoint(param[1], 10, &rgb_ogo.r_pre_offset) < 0)
			goto free_buf;
		if (kstrtoint(param[2], 10, &rgb_ogo.g_pre_offset) < 0)
			goto free_buf;
		if (kstrtoint(param[3], 10, &rgb_ogo.b_pre_offset) < 0)
			goto free_buf;
		if (kstrtouint(param[4], 10, &rgb_ogo.r_gain) < 0)
			goto free_buf;
		if (kstrtouint(param[5], 10, &rgb_ogo.g_gain) < 0)
			goto free_buf;
		if (kstrtouint(param[6], 10, &rgb_ogo.b_gain) < 0)
			goto free_buf;
		if (kstrtoint(param[7], 10, &rgb_ogo.r_post_offset) < 0)
			goto free_buf;
		if (kstrtoint(param[8], 10, &rgb_ogo.g_post_offset) < 0)
			goto free_buf;
		if (kstrtoint(param[9], 10, &rgb_ogo.b_post_offset) < 0)
			goto free_buf;

		vpq_set_rgb_ogo(&rgb_ogo);
	} else if (!strcmp(param[0], "dnlp")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_dnlp_mode(value);
	} else if (!strcmp(param[0], "lc")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_lc_mode(value);
	} else if (!strcmp(param[0], "cms")) {
		struct vpq_cms_s cms_param = {0};

		if (kstrtouint(param[1], 10, &cms_param.color_type) < 0)
			goto free_buf;
		if (kstrtouint(param[2], 10, &cms_param.color_9) < 0)
			goto free_buf;
		if (kstrtouint(param[3], 10, &cms_param.color_14) < 0)
			goto free_buf;
		if (kstrtouint(param[4], 10, &cms_param.cms_type) < 0)
			goto free_buf;
		if (kstrtoint(param[5], 10, &cms_param.value) < 0)
			goto free_buf;

		vpq_set_color_customize(&cms_param);
	} else if (!strcmp(param[0], "blkext")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_black_stretch(value);
	} else if (!strcmp(param[0], "blue_stretch")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_blue_stretch(value);
	} else if (!strcmp(param[0], "aipq")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_aipq_mode(value);
	} else if (!strcmp(param[0], "aisr")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_aisr_mode(value);
	} else if (!strcmp(param[0], "csc_type")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_csc_type(value);
	} else if (!strcmp(param[0], "chroma_coring")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_chroma_coring(value);
	} else if (!strcmp(param[0], "amdv_pic_mode_id")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_amdv_pic_mode_id(value);
	} else if (!strcmp(param[0], "amdv_dark_detail")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_amdv_dark_detail(value);
	} else if (!strcmp(param[0], "amdv_light_sensor")) {
		struct vpq_light_sensor_s light_sensor = {0};

		if (kstrtouint(param[1], 10, &light_sensor.flag) < 0)
			goto free_buf;
		if (kstrtouint(param[2], 10, &light_sensor.t_frontLux) < 0)
			goto free_buf;
		if (kstrtouint(param[3], 10, &light_sensor.t_rearLum) < 0)
			goto free_buf;

		vpq_set_amdv_light_sensor(&light_sensor);
	} else if (!strcmp(param[0], "amdv_precision_detail")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_amdv_precision_detail(value);
	}  else if (!strcmp(param[0], "memc_en")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_memc_on_off(value);
	}  else if (!strcmp(param[0], "memc_deblur")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_memc_deblur_level(value);
	}  else if (!strcmp(param[0], "memc_dejudder")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_memc_dejudder_level(value);
	}  else if (!strcmp(param[0], "memc_demo")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		vpq_set_memc_demo_mode(value);
	} else {
		pr_inf(lev_dbg, "cmd error\n");
		goto free_buf;
	}

free_buf:
	kfree(buf_org);

	return count;
}

ssize_t vpq_pq_module_cfg_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf)
{
	pr_inf(lev_dbg, "show pq modules cfg:\n");
	pr_inf(lev_dbg, " pq_en           :%s\n", STATE_AS_STRING(pq_module_cfg.pq_en));
	pr_inf(lev_dbg, " vadj1_en        :%s\n", STATE_AS_STRING(pq_module_cfg.vadj1_en));
	pr_inf(lev_dbg, " vd1_ctrst_en    :%s\n", STATE_AS_STRING(pq_module_cfg.vd1_ctrst_en));
	pr_inf(lev_dbg, " vadj2_en        :%s\n", STATE_AS_STRING(pq_module_cfg.vadj2_en));
	pr_inf(lev_dbg, " post_ctrst_en   :%s\n", STATE_AS_STRING(pq_module_cfg.post_ctrst_en));
	pr_inf(lev_dbg, " pregamma_en     :%s\n", STATE_AS_STRING(pq_module_cfg.pregamma_en));
	pr_inf(lev_dbg, " gamma_en        :%s\n", STATE_AS_STRING(pq_module_cfg.gamma_en));
	pr_inf(lev_dbg, " wb_en           :%s\n", STATE_AS_STRING(pq_module_cfg.wb_en));
	pr_inf(lev_dbg, " dnlp_en         :%s\n", STATE_AS_STRING(pq_module_cfg.dnlp_en));
	pr_inf(lev_dbg, " lc_en           :%s\n", STATE_AS_STRING(pq_module_cfg.lc_en));
	pr_inf(lev_dbg, " black_ext_en    :%s\n", STATE_AS_STRING(pq_module_cfg.black_ext_en));
	pr_inf(lev_dbg, " blue_stretch_en :%s\n", STATE_AS_STRING(pq_module_cfg.blue_stretch_en));
	pr_inf(lev_dbg, " chroma_cor_en   :%s\n", STATE_AS_STRING(pq_module_cfg.chroma_cor_en));
	pr_inf(lev_dbg, " sharpness0_en   :%s\n", STATE_AS_STRING(pq_module_cfg.sharpness0_en));
	pr_inf(lev_dbg, " sharpness1_en   :%s\n", STATE_AS_STRING(pq_module_cfg.sharpness1_en));
	pr_inf(lev_dbg, " cm_en           :%s\n", STATE_AS_STRING(pq_module_cfg.cm_en));
	pr_inf(lev_dbg, " lut3d_en        :%s\n", STATE_AS_STRING(pq_module_cfg.lut3d_en));
	pr_inf(lev_dbg, " dejaggy_sr0_en  :%s\n", STATE_AS_STRING(pq_module_cfg.dejaggy_sr0_en));
	pr_inf(lev_dbg, " dejaggy_sr1_en  :%s\n", STATE_AS_STRING(pq_module_cfg.dejaggy_sr1_en));
	pr_inf(lev_dbg, " dering_sr0_en   :%s\n", STATE_AS_STRING(pq_module_cfg.dering_sr0_en));
	pr_inf(lev_dbg, " dering_sr1_en   :%s\n", STATE_AS_STRING(pq_module_cfg.dering_sr1_en));
	pr_inf(lev_dbg, " di_en           :%s\n", STATE_AS_STRING(pq_module_cfg.di_en));
	pr_inf(lev_dbg, " mcdi_en         :%s\n", STATE_AS_STRING(pq_module_cfg.mcdi_en));
	pr_inf(lev_dbg, " deblock_en      :%s\n", STATE_AS_STRING(pq_module_cfg.deblock_en));
	pr_inf(lev_dbg, " demosquito_en   :%s\n", STATE_AS_STRING(pq_module_cfg.demosquito_en));
	pr_inf(lev_dbg, " smoothplus_en   :%s\n", STATE_AS_STRING(pq_module_cfg.smoothplus_en));
	pr_inf(lev_dbg, " nr_en           :%s\n", STATE_AS_STRING(pq_module_cfg.nr_en));
	pr_inf(lev_dbg, " hdrtmo_en       :%s\n", STATE_AS_STRING(pq_module_cfg.hdrtmo_en));
	pr_inf(lev_dbg, " ai_en           :%s\n", STATE_AS_STRING(pq_module_cfg.ai_en));
	pr_inf(lev_dbg, " aisr_en         :%s\n", STATE_AS_STRING(pq_module_cfg.aisr_en));

	pr_inf(lev_dbg, "--------------------------------\n");

	return 0;
}

ssize_t vpq_pq_module_cfg_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count)
{
	return count;
}

ssize_t vpq_pq_module_status_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf)
{
	struct vpq_pq_state_s pq_state = {0};

	vpq_get_pq_module_status(&pq_state);

	pr_inf(lev_dbg, "show vpp pq module status:\n");
	pr_inf(lev_dbg, " pq           :%s\n", STATE_AS_STRING(pq_state.pq_en));
	pr_inf(lev_dbg, " vadj1        :%s\n", STATE_AS_STRING(pq_state.pq_cfg.vadj1_en));
	pr_inf(lev_dbg, " vd1_ctrst    :%s\n", STATE_AS_STRING(pq_state.pq_cfg.vd1_ctrst_en));
	pr_inf(lev_dbg, " vadj2        :%s\n", STATE_AS_STRING(pq_state.pq_cfg.vadj2_en));
	pr_inf(lev_dbg, " post_ctrst   :%s\n", STATE_AS_STRING(pq_state.pq_cfg.post_ctrst_en));
	pr_inf(lev_dbg, " pregamma     :%s\n", STATE_AS_STRING(pq_state.pq_cfg.pregamma_en));
	pr_inf(lev_dbg, " gamma        :%s\n", STATE_AS_STRING(pq_state.pq_cfg.gamma_en));
	pr_inf(lev_dbg, " wb           :%s\n", STATE_AS_STRING(pq_state.pq_cfg.wb_en));
	pr_inf(lev_dbg, " dnlp         :%s\n", STATE_AS_STRING(pq_state.pq_cfg.dnlp_en));
	pr_inf(lev_dbg, " lc           :%s\n", STATE_AS_STRING(pq_state.pq_cfg.lc_en));
	pr_inf(lev_dbg, " black_ext    :%s\n", STATE_AS_STRING(pq_state.pq_cfg.black_ext_en));
	pr_inf(lev_dbg, " blue_stretch :%s\n", STATE_AS_STRING(pq_state.pq_cfg.blue_stretch_en));
	pr_inf(lev_dbg, " chroma_cor   :%s\n", STATE_AS_STRING(pq_state.pq_cfg.chroma_cor_en));
	pr_inf(lev_dbg, " sharpness0   :%s\n", STATE_AS_STRING(pq_state.pq_cfg.sharpness0_en));
	pr_inf(lev_dbg, " sharpness1   :%s\n", STATE_AS_STRING(pq_state.pq_cfg.sharpness1_en));
	pr_inf(lev_dbg, " cm           :%s\n", STATE_AS_STRING(pq_state.pq_cfg.cm_en));
	pr_inf(lev_dbg, " lut3d        :%s\n", STATE_AS_STRING(pq_state.pq_cfg.lut3d_en));
	pr_inf(lev_dbg, " dejaggy_sr0  :%s\n", STATE_AS_STRING(pq_state.pq_cfg.dejaggy_sr0_en));
	pr_inf(lev_dbg, " dejaggy_sr1  :%s\n", STATE_AS_STRING(pq_state.pq_cfg.dejaggy_sr0_en));
	pr_inf(lev_dbg, " dering_sr0   :%s\n", STATE_AS_STRING(pq_state.pq_cfg.dering_sr0_en));
	pr_inf(lev_dbg, " dering_sr1   :%s\n", STATE_AS_STRING(pq_state.pq_cfg.dering_sr0_en));

	pr_inf(lev_dbg, "--------------------------------\n");
	pr_inf(lev_dbg, "\n");

#if ENABLE_USAGE_STR
	return sprintf(buf, "%s\n", vpq_module_status_usage_str);
#else
	return 0;
#endif
}

ssize_t vpq_pq_module_status_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count)
{
	char *buf_org;
	char *param[8];
	int value = 0;

	buf_org = kstrdup(buf, GFP_KERNEL);
	if (!buf_org)
		return -ENOMEM;

	parse_param(buf_org, param);

	if (!strcmp(param[0], "pq_module")) {
		enum vpq_module_e module = VPQ_MODULE_VADJ1;

		if (!strcmp(param[1], "vadj1"))
			module = VPQ_MODULE_VADJ1;
		else if (!strcmp(param[1], "vadj2"))
			module = VPQ_MODULE_VADJ2;
		else if (!strcmp(param[1], "pregamma"))
			module = VPQ_MODULE_PREGAMMA;
		else if (!strcmp(param[1], "gamma"))
			module = VPQ_MODULE_GAMMA;
		else if (!strcmp(param[1], "wb"))
			module = VPQ_MODULE_WB;
		else if (!strcmp(param[1], "dnlp"))
			module = VPQ_MODULE_DNLP;
		else if (!strcmp(param[1], "ccoring"))
			module = VPQ_MODULE_CCORING;
		else if (!strcmp(param[1], "sr0"))
			module = VPQ_MODULE_SR0;
		else if (!strcmp(param[1], "sr1"))
			module = VPQ_MODULE_SR1;
		else if (!strcmp(param[1], "lc"))
			module = VPQ_MODULE_LC;
		else if (!strcmp(param[1], "cm"))
			module = VPQ_MODULE_CM;
		else if (!strcmp(param[1], "ble"))
			module = VPQ_MODULE_BLE;
		else if (!strcmp(param[1], "bls"))
			module = VPQ_MODULE_BLS;
		else if (!strcmp(param[1], "lut3d"))
			module = VPQ_MODULE_LUT3D;
		else if (!strcmp(param[1], "dejaggy_sr0"))
			module = VPQ_MODULE_DEJAGGY_SR0;
		else if (!strcmp(param[1], "dejaggy_sr1"))
			module = VPQ_MODULE_DEJAGGY_SR1;
		else if (!strcmp(param[1], "dering_sr0"))
			module = VPQ_MODULE_DERING_SR0;
		else if (!strcmp(param[1], "dering_sr1"))
			module = VPQ_MODULE_DERING_SR1;
		else if (!strcmp(param[1], "all"))
			module = VPQ_MODULE_ALL;
		else
			module = VPQ_MODULE_ALL;

		if (kstrtouint(param[2], 10, &value) < 0)
			goto free_buf;

		pr_inf(lev_dbg, "pq_module_status %d %d\n", module, value);
		vpq_set_pq_module_status(module, value);
	}

free_buf:
	kfree(buf_org);

	return count;
}

#if OPEN_PRINTK
static char *src_type_tostring(enum vpq_source_type_e src_type)
{
	if (src_type == VPQ_SRC_TYPE_MPEG)
		return "MPEG";
	else if (src_type == VPQ_SRC_TYPE_DTV)
		return "DTV";
	else if (src_type == VPQ_SRC_TYPE_CVBS)
		return "AV";
	else if (src_type == VPQ_SRC_TYPE_HDMI)
		return "HDMI";
	else
		return "NULL";
}

static char *port_type_tostring(enum vpq_hdmi_port_e hdmi_port)
{
	if (hdmi_port == VPQ_HDMI_PORT_0)
		return "HDMI1";
	else if (hdmi_port == VPQ_HDMI_PORT_1)
		return "HDMI2";
	else if (hdmi_port == VPQ_HDMI_PORT_2)
		return "HDMI3";
	else if (hdmi_port == VPQ_HDMI_PORT_3)
		return "HDMI4";
	else
		return "NULL";
}

static char *hdr_type_tostring(enum vpq_hdr_type_e hdr_type)
{
	if (hdr_type == VPQ_HDR_TYPE_HDR10)
		return "HDR10";
	else if (hdr_type == VPQ_HDR_TYPE_HDR10PLUS)
		return "HDR10+";
	else if (hdr_type == VPQ_HDR_TYPE_AMDV)
		return "DV";
	else if (hdr_type == VPQ_HDR_TYPE_PRIMESL)
		return "PRIMESL";
	else if (hdr_type == VPQ_HDR_TYPE_HLG)
		return "HLG";
	else if (hdr_type == VPQ_HDR_TYPE_SDR)
		return "SDR";
	else if (hdr_type == VPQ_HDR_TYPE_MVC)
		return "MVC";
	else
		return "NONE";
}
#endif

ssize_t vpq_src_infor_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf)
{
	struct vpq_signal_info_s sig_info = {0};

	vpq_get_signal_info(&sig_info);

	pr_inf(lev_dbg, "current signal information:\n");
	pr_inf(lev_dbg, " source              :%s\n", src_type_tostring(sig_info.src_type));
	pr_inf(lev_dbg, " hdmi port           :%s\n", port_type_tostring(sig_info.hdmi_port));
	pr_inf(lev_dbg, " signal fmt(vdin)    :0x%x\n", sig_info.sig_fmt);
	pr_inf(lev_dbg, " trans fmt(vdin)     :%d\n", sig_info.trans_fmt);
	pr_inf(lev_dbg, " height              :%d\n", sig_info.height);
	pr_inf(lev_dbg, " width               :%d\n", sig_info.width);
	pr_inf(lev_dbg, " fps                 :%dhz\n", sig_info.fps);
	pr_inf(lev_dbg, " hdr type            :%s\n", hdr_type_tostring(sig_info.hdr_type));
	pr_inf(lev_dbg, " dv                  :%d\n", sig_info.is_amdv);
	pr_inf(lev_dbg, " game                :%d\n", sig_info.is_game);
	pr_inf(lev_dbg, " pc                  :%d\n", sig_info.is_pc);
	pr_inf(lev_dbg, " scan_mode           :%s\n",
				(sig_info.scan_mode == VPQ_SCAN_MODE_PROGRESSIVE) ? "P" :
				((sig_info.scan_mode == VPQ_SCAN_MODE_INTERLACED) ? "I" : "NULL"));
	pr_inf(lev_dbg, "--------------------------------\n");

	return 0;
}

ssize_t vpq_src_infor_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count)
{
	return count;
}

char *csctype[29][2] = {
	//csc type                      value
	{"NULL",                        "0"},
	{"RGB_YUV601",                  "1"},
	{"RGB_YUV601F",                 "2"},
	{"RGB_YUV709",                  "3"},
	{"RGB_YUV709F",                 "4"},
	{"YUV601_RGB",                  "16"},
	{"YUV601_YUV601F",              "17"},
	{"YUV601_YUV709",               "18"},
	{"YUV601_YUV709F",              "19"},
	{"YUV601F_RGB",                 "20"},
	{"YUV601F_YUV601",              "21"},
	{"YUV601F_YUV709",              "22"},
	{"YUV601F_YUV709F",             "23"},
	{"YUV709_RGB",                  "32"},
	{"YUV709_YUV601",               "33"},
	{"YUV709_YUV601F",              "34"},
	{"YUV709_YUV709F",              "35"},
	{"YUV709F_RGB",                 "36"},
	{"YUV709F_YUV601",              "37"},
	{"YUV709F_YUV709",              "38"},
	{"YUV601L_YUV709L",             "39"},
	{"YUV709L_YUV601L",             "40"},
	{"YUV709F_YUV601F",             "41"},
	{"BT2020YUV_BT2020RGB",         "64"},
	{"BT2020RGB_709RGB",            "65"},
	{"BT2020RGB_CUSRGB",            "66"},
	{"BT2020YUV_BT2020RGB_DYNAMIC", "80"},
	{"BT2020YUV_BT2020RGB_CUVA",    "81"},
	{"\0"},
};

ssize_t vpq_other_infor_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf)
{
	unsigned char i = 0;

	for (i = 0; i < 29; i++) {
		if (strcmp(csctype[i][0], "\0") == 0) {
			pr_inf(lev_dbg, "show csc_type: NULL\n");
			break;
		}

		if (vpq_get_csc_type() == atoi(csctype[i][1])) {
			pr_inf(lev_dbg, "current csc_type: %s\n", csctype[i][0]);
			break;
		}
	}

	return 0;
}

ssize_t vpq_other_infor_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count)
{
	return count;
}

ssize_t vpq_hist_avg_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf)
{
	struct vpq_hist_ave_s hist_avg = {0};

	vpq_get_hist_avg(&hist_avg);
	pr_inf(lev_dbg, "current hist avg:\n");
	pr_inf(lev_dbg, " sum    :%d\n", hist_avg.sum);
	pr_inf(lev_dbg, " height :%d\n", hist_avg.height);
	pr_inf(lev_dbg, " width  :%d\n", hist_avg.width);
	pr_inf(lev_dbg, " ave    :%d\n", hist_avg.ave);
	pr_inf(lev_dbg, "--------------------------------\n");

	return 0;
}

ssize_t vpq_hist_avg_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count)
{
	return count;
}

ssize_t vpq_dump_show(const struct class *class,
			const struct class_attribute *attr,
			char *buf)
{
#if ENABLE_USAGE_STR
	return sprintf(buf, "%s\n", vpq_dump_table_usage_str);
#else
	return 0;
#endif
}

ssize_t vpq_dump_store(const struct class *class,
			const struct class_attribute *attr,
			const char *buf,
			size_t count)
{
	char *buf_org;
	char *param[8];
	int value = 0;

	buf_org = kstrdup(buf, GFP_KERNEL);
	if (!buf_org)
		return -ENOMEM;

	parse_param(buf_org, param);

	if (!strcmp(param[0], "dump_data")) {
		if (kstrtouint(param[1], 10, &value) < 0)
			goto free_buf;

		pr_inf(lev_dbg, "dump_module %d\n", value);
		vpq_module_dump_pq_table(value);
	} else {
		pr_inf(lev_dbg, "cmd error\n");
		goto free_buf;
	}

free_buf:
	kfree(buf_org);

	return count;
}
