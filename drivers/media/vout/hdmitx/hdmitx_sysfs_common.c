// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>

#include <linux/amlogic/media/vout/vinfo.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_audio.h>

#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
#include <linux/amlogic/media/amvecm/amvecm.h>
#endif

#include "hdmitx_sysfs_common.h"
#include "hdmitx_log.h"
#include "hdmitx_compliance.h"
#include "hdmitx_check_valid.h"
#include "hdmitx_module.h"

/*!!Only one instance supported.*/
static struct hdmitx_common *global_tx_common;
static struct hdmitx_hw_common *global_tx_hw;

const char *hdmitx_mode_get_timing_name(enum hdmi_vic vic);

/************************common sysfs*************************/
static ssize_t hdmi_efuse_state_show(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "FEAT_DISABLE_HDMI_60HZ = %d\n\r",
		global_tx_common->efuse_dis_hdmi_4k60);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "FEAT_DISABLE_OUTPUT_4K = %d\n\r",
		global_tx_common->efuse_dis_output_4k);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "FEAT_DISABLE_HDCP_TX_22 = %d\n\r",
		global_tx_common->efuse_dis_hdcp_tx22);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "FEAT_DISABLE_HDMI_TX_3D = %d\n\r",
		global_tx_common->efuse_dis_hdmi_tx3d);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "FEAT_DISABLE_HDMI = %d\n\r",
		global_tx_common->efuse_dis_hdcp_tx14);
	return pos;
}

static DEVICE_ATTR_RO(hdmi_efuse_state);

static ssize_t attr_show(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	int pos = 0;
	char fmt_attr[16];

	hdmitx_get_attr(global_tx_common, fmt_attr);
	pos = snprintf(buf, PAGE_SIZE, "%s\n\r", fmt_attr);

	return pos;
}

static DEVICE_ATTR_RO(attr);

/* for pxp test */
static ssize_t test_attr_show(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	int pos = 0;
	char fmt_attr[16] = {0};

	memcpy(fmt_attr, global_tx_common->tst_fmt_attr, sizeof(fmt_attr));
	pos = snprintf(buf, PAGE_SIZE, "%s\n\r", fmt_attr);

	return pos;
}

static ssize_t test_attr_store(struct device *dev,
		   struct device_attribute *attr,
		   const char *buf, size_t count)
{
	strncpy(global_tx_common->tst_fmt_attr, buf, sizeof(global_tx_common->tst_fmt_attr));
	global_tx_common->tst_fmt_attr[15] = '\0';

	return count;
}
static DEVICE_ATTR_RW(test_attr);

static ssize_t _hpd_state_show(struct device *dev,
			      struct device_attribute *attr, char *buf, int size)
{
	int pos = 0;

	pos += snprintf(buf + pos, size, "%d",
		global_tx_common->hpd_state);
	return pos;
}

static ssize_t hpd_state_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	return _hpd_state_show(dev, attr, buf, PAGE_SIZE);
}

static DEVICE_ATTR_RO(hpd_state);

/* rawedid attr */
static ssize_t _rawedid_show(struct device *dev,
			    struct device_attribute *attr, char *buf, int size)
{
	int pos = 0;
	int i;
	int num;
	int block_num = 0;

	block_num = hdmitx_edid_valid_block_num(global_tx_common->EDID_buf);
	if (block_num <= 8)
		num = block_num * 128;
	else
		num = 8 * 128;

	for (i = 0; i < num; i++)
		pos += snprintf(buf + pos, size - pos, "%02x",
			global_tx_common->EDID_buf[i]);

	pos += snprintf(buf + pos, size - pos, "\n");
	return pos;
}

static ssize_t rawedid_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	return _rawedid_show(dev, attr, buf, PAGE_SIZE);
}

static DEVICE_ATTR_RO(rawedid);

/*
 * edid_parsing attr
 * If RX edid data are all correct, HEAD(00ffffffffffff00), checksum,
 * version, etc), then return "ok". Otherwise, "ng"
 * Actually, in some old televisions, EDID is stored in EEPROM.
 * some bits in EEPROM may reverse with time.
 * But it does not affect  edid_parsing.
 * Therefore, we consider the RX edid data are all correct, return "OK"
 */
static ssize_t _edid_parsing_show(struct device *dev,
				 struct device_attribute *attr, char *buf, int size)
{
	int pos = 0;

	if (hdmitx_edid_check_data_valid(global_tx_common->rxcap.edid_check,
		global_tx_common->EDID_buf))
		pos += snprintf(buf + pos, size - pos, "ok\n");
	else
		pos += snprintf(buf + pos, size - pos, "ng\n");

	return pos;
}

static ssize_t edid_parsing_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	return _edid_parsing_show(dev, attr, buf, PAGE_SIZE);
}

static DEVICE_ATTR_RO(edid_parsing);

static ssize_t edid_show(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	return hdmitx_edid_print_sink_cap(&global_tx_common->rxcap, buf, PAGE_SIZE);
}

static int load_edid_string_data(char *string)
{
	size_t str_len;
	int i;
	bool valid_len;
	unsigned char *buf = NULL;
	bool ret;
	size_t edid_len;
	unsigned char tmp[3];

	if (!string)
		return 0;

	str_len = strlen(string);
	valid_len = 0;
	for (i = 1; i <= EDID_MAX_BLOCK; i++) {
		if (str_len == (256 * i + 1)) {
			valid_len = 1;
			break;
		}
	}
	if (valid_len == 0)
		return 0;

	edid_len = (str_len - 1) / 2;
	buf = kmalloc(edid_len, GFP_KERNEL);
	if (!buf)
		return 0;
	memset(buf, 0, edid_len);
	/* convert the edid string to hex data */
	for (i = 0; i < edid_len; i++) {
		tmp[0] = string[i * 2];
		tmp[1] = string[i * 2 + 1];
		tmp[2] = '\0';
		ret = kstrtou8(tmp, 16, &buf[i]);
		if (ret)
			HDMITX_INFO("%s[%d] covert error %c%c ret = %d\n", __func__, __LINE__,
				string[i * 2], string[i * 2 + 1], ret);
	}

	hdmitx_edid_buffer_clear(global_tx_common->EDID_buf, sizeof(global_tx_common->EDID_buf));
	memcpy(global_tx_common->EDID_buf, buf, edid_len);

	kfree(buf);
	HDMITX_INFO("%s: %zu bytes loaded from edid string\n", __func__, str_len);
	return 1;
}

int hdmitx_load_edid_file(u32 type, char *path)
{
	if (type == 1)
		return load_edid_string_data(path);
	return 0;
}

int hdmitx_save_edid_file(unsigned char *rawedid, char *path)
{
#ifdef CONFIG_AMLOGIC_ENABLE_MEDIA_FILE
	struct file *filp = NULL;
	loff_t pos = 0;
	char line[128] = {0};
	u32 i = 0, j = 0, k = 0, size = 0, block_cnt = 0;
	u32 index = 0, tmp = 0;

	filp = filp_open(path, O_RDWR | O_CREAT, 0666);
	if (IS_ERR(filp)) {
		HDMITX_ERROR("[%s] failed to open/create file: |%s|\n",
			__func__, path);
		goto PROCESS_END;
	}

	block_cnt = rawedid[0x7e] + 1;
	if (rawedid[0x7e] && rawedid[128 + 4] == EXTENSION_EEODB_EXT_TAG &&
		rawedid[128 + 5] == EXTENSION_EEODB_EXT_CODE)
		block_cnt = rawedid[128 + 6] + 1;

	/* dump as txt file*/
	for (i = 0; i < block_cnt; i++) {
		for (j = 0; j < 8; j++) {
			for (k = 0; k < 16; k++) {
				index = i * 128 + j * 16 + k;
				tmp = rawedid[index];
				snprintf((char *)&line[k * 6], 7,
					 "0x%02x, ",
					 tmp);
			}
			line[16 * 6 - 1] = '\n';
			line[16 * 6] = 0x0;
			pos = (i * 8 + j) * 16 * 6;
		}
	}

	HDMITX_INFO("[%s] write %d bytes to file %s\n", __func__, size, path);

	vfs_fsync(filp, 0);
	filp_close(filp, NULL);

PROCESS_END:
#else
	HDMITX_ERROR("Not support write file.\n");
#endif
	return 0;
}

static ssize_t edid_store(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf, size_t count)
{
	u32 argn = 0;
	char *p = NULL, *para = NULL, *temp_p = NULL, *argv[8] = {NULL};
	u32 path_length = 0;
	int ret = 0;

	p = kstrdup(buf, GFP_KERNEL);
	temp_p = p;
	if (!p)
		return count;

	do {
		para = strsep(&p, " ");
		if (para) {
			argv[argn] = para;
			argn++;
			if (argn > 7)
				break;
		}
	} while (para);

	if (!strncmp(argv[0], "save", strlen("save"))) {
		u32 type = 0;

		if (argn != 3) {
			HDMITX_INFO("[%s] cmd format: save bin/txt edid_file_path\n",
				__func__);
			goto PROCESS_END;
		}
		if (!strncmp(argv[1], "bin", strlen("bin")))
			type = 1;
		else if (!strncmp(argv[1], "txt", strlen("txt")))
			type = 2;

		if (type == 1 || type == 2) {
			/* clean '\n' from file path*/
			path_length = strlen(argv[2]);
			if (argv[2][path_length - 1] == '\n')
				argv[2][path_length - 1] = 0x0;

			hdmitx_save_edid_file(global_tx_common->EDID_buf, argv[2]);
		}
	} else if (!strncmp(argv[0], "load", strlen("load"))) {
		if (argn != 2) {
			HDMITX_INFO("[%s] cmd format: load edid_file_path\n",
				__func__);
			goto PROCESS_END;
		}

		/* get the EDID from current RX device */
		if (strncmp(argv[1], "0000000000000000", 16) == 0) {
			HDMITX_INFO("%s[%d] get current RX edid\n", __func__, __LINE__);
			global_tx_common->forced_edid = 0;
			hdmitx_common_get_edid(global_tx_common);
			goto PROCESS_END;
		}

		/* clean '\n' from file path*/
		path_length = strlen(argv[1]);
		ret = hdmitx_load_edid_file(1, argv[1]); /* edid data as string for debug */
		if (ret == 1) {
			global_tx_common->forced_edid = 1;
			hdmitx_edid_rxcap_clear(&global_tx_common->rxcap);
			hdmitx_edid_parse(&global_tx_common->rxcap, global_tx_common->EDID_buf);
			hdmitx_cec_phy_addr_parse(&global_tx_common->rxcap,
					global_tx_common->EDID_buf);
			hdmitx_audio_parse(&global_tx_common->rxcap, global_tx_common->EDID_buf);
			hdmitx_common_edid_tracer_post_proc(global_tx_common,
					&global_tx_common->rxcap);
			hdmitx_edid_print(global_tx_common->EDID_buf);
			HDMITX_INFO("%s[%d] using the fixed edid\n", __func__, __LINE__);
		}
	}

PROCESS_END:
	kfree(temp_p);
	return count;
}
static DEVICE_ATTR_RW(edid);

static ssize_t _hdr_cap_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf,
			     const struct hdr_info *hdr, int size)
{
	int pos = 0;
	unsigned int i, j;
	int hdr10plugsupported = 0;
	const struct cuva_info *cuva = &hdr->cuva_info;
	const struct hdr10_plus_info *hdr10p = &hdr->hdr10plus_info;
	const struct sbtm_info *sbtm = &hdr->sbtm_info;

#ifdef CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_VECM
	/* HDR10plus is only supported by OTT when is_hdr10plus_enable is true */
	if (hdr10p->ieeeoui == HDR10_PLUS_IEEE_OUI &&
		hdr10p->application_version != 0xFF &&
		is_hdr10plus_enable())
#else
	if (hdr10p->ieeeoui == HDR10_PLUS_IEEE_OUI &&
			hdr10p->application_version != 0xFF)
#endif
		hdr10plugsupported = 1;
	pos += snprintf(buf + pos, size - pos, "HDR10Plus Supported: %d\n",
		hdr10plugsupported);
	pos += snprintf(buf + pos, size - pos, "HDR Static Metadata:\n");
	pos += snprintf(buf + pos, size - pos, "    Supported EOTF:\n");
	pos += snprintf(buf + pos, size - pos, "        Traditional SDR: %d\n",
		!!(hdr->hdr_support & 0x1));
	pos += snprintf(buf + pos, size - pos, "        Traditional HDR: %d\n",
		!!(hdr->hdr_support & 0x2));
	pos += snprintf(buf + pos, size - pos, "        SMPTE ST 2084: %d\n",
		!!(hdr->hdr_support & 0x4));
	pos += snprintf(buf + pos, size - pos, "        Hybrid Log-Gamma: %d\n",
		!!(hdr->hdr_support & 0x8));
	pos += snprintf(buf + pos, size - pos, "    Supported SMD type1: %d\n",
		hdr->static_metadata_type1);
	pos += snprintf(buf + pos, size - pos, "    Luminance Data\n");
	pos += snprintf(buf + pos, size - pos, "        Max: %d\n",
		hdr->lumi_max);
	pos += snprintf(buf + pos, size - pos, "        Avg: %d\n",
		hdr->lumi_avg);
	pos += snprintf(buf + pos, size - pos, "        Min: %d\n\n",
		hdr->lumi_min);
	pos += snprintf(buf + pos, size - pos, "HDR Dynamic Metadata:");

	for (i = 0; i < 4; i++) {
		if (hdr->dynamic_info[i].type == 0)
			continue;
		pos += snprintf(buf + pos, size - pos,
			"\n    metadata_version: %x\n",
			hdr->dynamic_info[i].type);
		pos += snprintf(buf + pos, size - pos,
			"        support_flags: %x\n",
			hdr->dynamic_info[i].support_flags);
		pos += snprintf(buf + pos, size - pos,
			"        optional_fields:");
		for (j = 0; j <
			(hdr->dynamic_info[i].of_len - 3); j++)
			pos += snprintf(buf + pos, size - pos, " %x",
				hdr->dynamic_info[i].optional_fields[j]);
	}

	pos += snprintf(buf + pos, size - pos, "\n\ncolorimetry_data: %x\n",
		hdr->colorimetry_support);
	if (cuva->ieeeoui == CUVA_IEEEOUI) {
		pos += snprintf(buf + pos, size - pos, "CUVA supported: 1\n");
		pos += snprintf(buf + pos, size - pos,
			"  system_start_code: %u\n", cuva->system_start_code);
		pos += snprintf(buf + pos, size - pos,
			"  version_code: %u\n", cuva->version_code);
		pos += snprintf(buf + pos, size - pos,
			"  display_maximum_luminance: %u\n",
			cuva->display_max_lum);
		pos += snprintf(buf + pos, size - pos,
			"  display_minimum_luminance: %u\n",
			cuva->display_min_lum);
		pos += snprintf(buf + pos, size - pos,
			"  monitor_mode_support: %u\n", cuva->monitor_mode_sup);
		pos += snprintf(buf + pos, size - pos,
			"  rx_mode_support: %u\n", cuva->rx_mode_sup);
		for (i = 0; i < (cuva->length + 1); i++)
			pos += snprintf(buf + pos, size - pos, "%02x",
				cuva->rawdata[i]);
		pos += snprintf(buf + pos, size - pos, "\n");
	}
	/* sbtm capability show */
	if (sbtm->sbtm_support) {
		pos += snprintf(buf + pos, size - pos, "SBTM supported: 1\n");
		if (sbtm->max_sbtm_ver)
			pos += snprintf(buf + pos, size - pos, "  Max_SBTM_Ver: 0x%x\n",
				sbtm->max_sbtm_ver);
		if (sbtm->grdm_support)
			pos += snprintf(buf + pos, size - pos, "  grdm_support: 0x%x\n",
				sbtm->grdm_support);
		if (sbtm->drdm_ind)
			pos += snprintf(buf + pos, size - pos, "  drdm_ind: 0x%x\n",
				sbtm->drdm_ind);
		if (sbtm->hgig_cat_drdm_sel)
			pos += snprintf(buf + pos, size - pos, "  hgig_cat_drdm_sel: 0x%x\n",
				sbtm->hgig_cat_drdm_sel);
		if (sbtm->use_hgig_drdm)
			pos += snprintf(buf + pos, size - pos, "  use_hgig_drdm: 0x%x\n",
				sbtm->use_hgig_drdm);
		if (sbtm->maxrgb)
			pos += snprintf(buf + pos, size - pos, "  maxrgb: 0x%x\n",
				sbtm->maxrgb);
		if (sbtm->gamut)
			pos += snprintf(buf + pos, size - pos, "  gamut: 0x%x\n",
				sbtm->gamut);
		if (sbtm->red_x)
			pos += snprintf(buf + pos, size - pos, "  red_x: 0x%x\n",
				sbtm->red_x);
		if (sbtm->red_y)
			pos += snprintf(buf + pos, size - pos, "  red_y: 0x%x\n",
				sbtm->red_y);
		if (sbtm->green_x)
			pos += snprintf(buf + pos, size - pos, "  green_x: 0x%x\n",
				sbtm->green_x);
		if (sbtm->green_y)
			pos += snprintf(buf + pos, size - pos, "  green_y: 0x%x\n",
				sbtm->green_y);
		if (sbtm->blue_x)
			pos += snprintf(buf + pos, size - pos, "  blue_x: 0x%x\n",
				sbtm->blue_x);
		if (sbtm->blue_y)
			pos += snprintf(buf + pos, size - pos, "  blue_y: 0x%x\n",
				sbtm->blue_y);
		if (sbtm->white_x)
			pos += snprintf(buf + pos, size - pos, "  white_x: 0x%x\n",
				sbtm->white_x);
		if (sbtm->white_y)
			pos += snprintf(buf + pos, size - pos, "  white_y: 0x%x\n",
				sbtm->white_y);
		if (sbtm->min_bright_10)
			pos += snprintf(buf + pos, size - pos, "  min_bright_10: 0x%x\n",
				sbtm->min_bright_10);
		if (sbtm->peak_bright_100)
			pos += snprintf(buf + pos, size - pos, "  peak_bright_100: 0x%x\n",
				sbtm->peak_bright_100);
		if (sbtm->p0_exp)
			pos += snprintf(buf + pos, size - pos, "  p0_exp: 0x%x\n",
				sbtm->p0_exp);
		if (sbtm->p0_mant)
			pos += snprintf(buf + pos, size - pos, "  p0_mant: 0x%x\n",
				sbtm->p0_mant);
		if (sbtm->peak_bright_p0)
			pos += snprintf(buf + pos, size - pos, "  peak_bright_p0: 0x%x\n",
				sbtm->peak_bright_p0);
		if (sbtm->p1_exp)
			pos += snprintf(buf + pos, size - pos, "  p1_exp: 0x%x\n",
				sbtm->p1_exp);
		if (sbtm->p1_mant)
			pos += snprintf(buf + pos, size - pos, "  p1_mant: 0x%x\n",
				sbtm->p1_mant);
		if (sbtm->peak_bright_p1)
			pos += snprintf(buf + pos, size - pos, "  peak_bright_p1: 0x%x\n",
				sbtm->peak_bright_p1);
		if (sbtm->p2_exp)
			pos += snprintf(buf + pos, size - pos, "  p2_exp: 0x%x\n",
				sbtm->p2_exp);
		if (sbtm->p2_mant)
			pos += snprintf(buf + pos, size - pos, "  p2_mant: 0x%x\n",
				sbtm->p2_mant);
		if (sbtm->peak_bright_p2)
			pos += snprintf(buf + pos, size - pos, "  peak_bright_p2: 0x%x\n",
				sbtm->peak_bright_p2);
		if (sbtm->p3_exp)
			pos += snprintf(buf + pos, size - pos, "  p3_exp: 0x%x\n",
				sbtm->p3_exp);
		if (sbtm->p3_mant)
			pos += snprintf(buf + pos, size - pos, "  p3_mant: 0x%x\n",
				sbtm->p3_mant);
		if (sbtm->peak_bright_p3)
			pos += snprintf(buf + pos, size - pos, "  peak_bright_p3: 0x%x\n",
				sbtm->peak_bright_p3);
	}
	return pos;
}

static ssize_t hdr_cap_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	const struct hdr_info *info = &global_tx_common->rxcap.hdr_info;

	return _hdr_cap_show(dev, attr, buf, info, PAGE_SIZE);
}

static DEVICE_ATTR_RO(hdr_cap);

static ssize_t hdr_cap2_show(struct device *dev,
			    struct device_attribute *attr,
			    char *buf)
{
	const struct hdr_info *info2 = &global_tx_common->rxcap.hdr_info2;

	return _hdr_cap_show(dev, attr, buf, info2, PAGE_SIZE);
}

static DEVICE_ATTR_RO(hdr_cap2);

static ssize_t _show_dv_cap(struct device *dev,
			    struct device_attribute *attr,
			    char *buf,
			    const struct dv_info *dv, int size)
{
	int pos = 0;
	int i;

	if (dv->ieeeoui != DV_IEEE_OUI || dv->block_flag != CORRECT) {
		pos += snprintf(buf + pos, size - pos,
			"The Rx don't support DolbyVision\n");
		return pos;
	}
	pos += snprintf(buf + pos, size - pos,
		"DolbyVision RX support list:\n");

	if (dv->ver == 0) {
		pos += snprintf(buf + pos, size - pos,
			"VSVDB Version: V%d\n", dv->ver);
		pos += snprintf(buf + pos, size - pos,
			"2160p%shz: 1\n",
			dv->sup_2160p60hz ? "60" : "30");
		pos += snprintf(buf + pos, size - pos,
			"Support mode:\n");
		pos += snprintf(buf + pos, size - pos,
			"  DV_RGB_444_8BIT\n");
		if (dv->sup_yuv422_12bit)
			pos += snprintf(buf + pos, size - pos,
				"  DV_YCbCr_422_12BIT\n");
	}
	if (dv->ver == 1) {
		pos += snprintf(buf + pos, size - pos,
			"VSVDB Version: V%d(%d-byte)\n",
			dv->ver, dv->length + 1);
		if (dv->length == 0xB) {
			pos += snprintf(buf + pos, size - pos,
				"2160p%shz: 1\n",
				dv->sup_2160p60hz ? "60" : "30");
		pos += snprintf(buf + pos, size - pos,
			"Support mode:\n");
		pos += snprintf(buf + pos, size - pos,
			"  DV_RGB_444_8BIT\n");
		if (dv->sup_yuv422_12bit)
			pos += snprintf(buf + pos, size - pos,
			"  DV_YCbCr_422_12BIT\n");
		if (dv->low_latency == 0x01)
			pos += snprintf(buf + pos, size - pos,
				"  LL_YCbCr_422_12BIT\n");
		}

		if (dv->length == 0xE) {
			pos += snprintf(buf + pos, size - pos,
				"2160p%shz: 1\n",
				dv->sup_2160p60hz ? "60" : "30");
			pos += snprintf(buf + pos, size - pos,
				"Support mode:\n");
			pos += snprintf(buf + pos, size - pos,
				"  DV_RGB_444_8BIT\n");
			if (dv->sup_yuv422_12bit)
				pos += snprintf(buf + pos, size - pos,
				"  DV_YCbCr_422_12BIT\n");
		}
	}
	if (dv->ver == 2) {
		pos += snprintf(buf + pos, size - pos,
			"VSVDB Version: V%d\n", dv->ver);
		pos += snprintf(buf + pos, size - pos,
			"2160p%shz: 1\n",
			dv->sup_2160p60hz ? "60" : "30");
		pos += snprintf(buf + pos, size - pos,
			"Parity: %d\n", dv->parity);
		pos += snprintf(buf + pos, size - pos,
			"Support mode:\n");
		if (dv->Interface != 0x00 && dv->Interface != 0x01) {
			pos += snprintf(buf + pos, size - pos,
				"  DV_RGB_444_8BIT\n");
			if (dv->sup_yuv422_12bit)
				pos += snprintf(buf + pos, size - pos,
					"  DV_YCbCr_422_12BIT\n");
		}
		pos += snprintf(buf + pos, size - pos,
			"  LL_YCbCr_422_12BIT\n");
		if (dv->Interface == 0x01 || dv->Interface == 0x03) {
			if (dv->sup_10b_12b_444 == 0x1) {
				pos += snprintf(buf + pos, size - pos,
					"  LL_RGB_444_10BIT\n");
			}
			if (dv->sup_10b_12b_444 == 0x2) {
				pos += snprintf(buf + pos, size - pos,
					"  LL_RGB_444_12BIT\n");
			}
		}
	}
	pos += snprintf(buf + pos, size - pos,
		"IEEEOUI: 0x%06x\n", dv->ieeeoui);
	pos += snprintf(buf + pos, size - pos,
		"EMP: %d\n", dv->dv_emp_cap);
	pos += snprintf(buf + pos, size - pos, "VSVDB: ");
	for (i = 0; i < (dv->length + 1); i++)
		pos += snprintf(buf + pos, size - pos, "%02x",
		dv->rawdata[i]);
	pos += snprintf(buf + pos, size - pos, "\n");
	return pos;
}

static ssize_t dv_cap_show(struct device *dev,
			   struct device_attribute *attr,
			   char *buf)
{
	const struct dv_info *dv = &global_tx_common->rxcap.dv_info;

	return _show_dv_cap(dev, attr, buf, dv, PAGE_SIZE);
}

static DEVICE_ATTR_RO(dv_cap);

static ssize_t dv_cap2_show(struct device *dev,
			    struct device_attribute *attr,
			    char *buf)
{
	const struct dv_info *dv2 = &global_tx_common->rxcap.dv_info2;

	return _show_dv_cap(dev, attr, buf, dv2, PAGE_SIZE);
}

static DEVICE_ATTR_RO(dv_cap2);

static ssize_t frac_rate_policy_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf,
				size_t count)
{
	int val = 0;

	if (isdigit(buf[0])) {
		val = buf[0] - '0';
		HDMITX_DEBUG("set frac_rate_policy as %d\n", val);
		if (val == 0 || val == 1)
			global_tx_common->frac_rate_policy = val;
		else
			HDMITX_INFO("only accept as 0 or 1\n");
	}

	return count;
}

static ssize_t frac_rate_policy_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\n",
		global_tx_common->frac_rate_policy);

	return pos;
}

static DEVICE_ATTR_RW(frac_rate_policy);

/*
 *  1: enable hdmitx phy
 *  0: disable hdmitx phy
 */
static ssize_t phy_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	global_tx_hw->tmds_phy_op = TMDS_PHY_NONE;
	unsigned int mute_us;
	int cnt = 0;
	/*
	 * special WHALEY WTV55K1J TV, need to wait for > 3 frames
	 * after phy disable and before set new mode
	 */
	int delay_frame = 5;

	HDMITX_INFO("%s %s\n", __func__, buf);
	mute_us = hdmitx_get_frame_duration();
	if (strncmp(buf, "0", 1) == 0) {
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
		/* for s5 frl mode */
		hdmitx_disable_frl_work(global_tx_common);
#endif
		global_tx_hw->tmds_phy_op = TMDS_PHY_DISABLE;
		/*
		 * It is necessary to finish disable phy during the vsync interrupt
		 * before performing other actions. If the vsync interrupt does not come,
		 * there is a 3-frame timeout mechanism.
		 */
		while (global_tx_hw->tmds_phy_op) {
			usleep_range(mute_us, mute_us + 10);
			cnt++;
			if (cnt > 3) {
				HDMITX_INFO("not have vsync intr, manually turn off phy\n");
				hdmitx_hw_cntl_misc(global_tx_hw,
					MISC_TMDS_PHY_OP, TMDS_PHY_DISABLE);
				global_tx_hw->tmds_phy_op = TMDS_PHY_NONE;
				break;
			}
		}
		if (hdmitx_find_vendor_phy_delay(global_tx_common->EDID_buf)) {
			usleep_range(delay_frame * mute_us, delay_frame * mute_us + 10);
			HDMITX_DEBUG("delay %d frame after phy disable\n", delay_frame);
		}
	} else if (strncmp(buf, "1", 1) == 0) {
		global_tx_hw->tmds_phy_op = TMDS_PHY_ENABLE;
		hdmitx_hw_cntl_misc(global_tx_hw, MISC_TMDS_PHY_OP, TMDS_PHY_ENABLE);
	} else {
		HDMITX_INFO("set phy wrong: %s\n", buf);
	}
	return count;
}

static ssize_t phy_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int pos = 0;
	int state = 0;

	state = hdmitx_hw_get_state(global_tx_hw, STAT_TX_PHY, 0);
	pos += snprintf(buf + pos, PAGE_SIZE, "%d\n", state);

	return pos;
}

static DEVICE_ATTR_RW(phy);

static ssize_t _contenttype_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int pos = 0;
	static char * const ct_names[] = {
		"off",
		"game",
		"graphics",
		"photo",
		"cinema",
	};

	if (global_tx_common->ct_mode < ARRAY_SIZE(ct_names))
		pos += snprintf(buf + pos, PAGE_SIZE, "%s\n\r",
					ct_names[global_tx_common->ct_mode]);

	return pos;
}

static inline int com_str(const char *buf, const char *str)
{
	return strncmp(buf, str, strlen(str)) == 0;
}

static ssize_t _contenttype_mode_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	u32 ct_mode = SET_CT_OFF;

	HDMITX_INFO("store contenttype_mode as %s\n", buf);

	if (global_tx_common->allm_mode == 1) {
		global_tx_common->allm_mode = 0;
		hdmitx_common_setup_vsif_packet(global_tx_common, VT_ALLM, 0, NULL);
	}
	/* recover hdmi1.4 vsif */
	if (hdmitx_edid_get_hdmi14_4k_vic(global_tx_common->fmt_para.vic) &&
		!hdmitx_dv_en(global_tx_common->tx_hw) &&
		!hdmitx_hdr10p_en(global_tx_common->tx_hw))
		hdmitx_common_setup_vsif_packet(global_tx_common, VT_HDMI14_4K, 1, NULL);

	if (com_str(buf, "0") || com_str(buf, "off")) {
		ct_mode = SET_CT_OFF;
		global_tx_common->it_content = 0;
	} else if (com_str(buf, "1") || com_str(buf, "game")) {
		ct_mode = SET_CT_GAME;
		global_tx_common->it_content = 1;
	} else if (com_str(buf, "2") || com_str(buf, "graphics")) {
		ct_mode = SET_CT_GRAPHICS;
		global_tx_common->it_content = 1;
	} else if (com_str(buf, "3") || com_str(buf, "photo")) {
		ct_mode = SET_CT_PHOTO;
		global_tx_common->it_content = 1;
	} else if (com_str(buf, "4") || com_str(buf, "cinema")) {
		ct_mode = SET_CT_CINEMA;
		global_tx_common->it_content = 1;
	}
	hdmitx_hw_cntl_config(global_tx_hw, CONF_CT_MODE,
		global_tx_common->it_content << 4 | ct_mode);
	global_tx_common->ct_mode = ct_mode;

	return count;
}

static ssize_t _allm_mode_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\n\r",
		global_tx_common->allm_mode);

	return pos;
}

static ssize_t _llm_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int pos = 0;
	struct rx_cap *prxcap = &global_tx_common->rxcap;

	if (prxcap->allm)
		return _allm_mode_show(dev, attr, buf);
	if (prxcap->cnc3)
		return _contenttype_mode_show(dev, attr, buf);
	return pos;
}

static ssize_t _allm_mode_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf,
			       size_t count)
{
	int mode = 0;

	HDMITX_INFO("store allm_mode as %s\n", buf);

	if (com_str(buf, "0"))
		mode = 0;
	else if (com_str(buf, "1"))
		mode = 1;
	else if (com_str(buf, "-1"))
		mode = -1;

	hdmitx_common_set_allm_mode(global_tx_common, mode);

	return count;
}

static ssize_t _llm_mode_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct rx_cap *prxcap = &global_tx_common->rxcap;

	if (prxcap->allm)
		return _allm_mode_store(dev, attr, buf, count);
	if (prxcap->cnc3)
		return _contenttype_mode_store(dev, attr, buf, count);
	return count;
}

static ssize_t contenttype_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return _llm_mode_show(dev, attr, buf);
}

static ssize_t contenttype_mode_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	return _llm_mode_store(dev, attr, buf, count);
}

static DEVICE_ATTR_RW(contenttype_mode);

static ssize_t allm_mode_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	return _llm_mode_show(dev, attr, buf);
}

static ssize_t allm_mode_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf,
			       size_t count)
{
	return _llm_mode_store(dev, attr, buf, count);
}

static DEVICE_ATTR_RW(allm_mode);

/*
 * sync with hdmitx_common_get_vic_list()
 * step1, only select VIC which is supported in EDID
 * step2, check if VIC is supported by SOC hdmitx
 * step3, build format with basic mode/attr and check
 * if it's supported by EDID/hdmitx_cap
 */
static ssize_t _disp_cap_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf, int size)
{
	struct rx_cap *prxcap = &global_tx_common->rxcap;
	const struct hdmi_timing *timing = NULL;
	enum hdmi_vic vic;
	const char *mode_name;
	int i, pos = 0;
	int vic_len = prxcap->VIC_count + VESA_MAX_TIMING;
	int *edid_vics = vmalloc(vic_len * sizeof(int));
	enum hdmi_vic prefer_vic = HDMI_0_UNKNOWN;

	mutex_lock(&global_tx_common->valid_mutex);
	memset(edid_vics, 0, vic_len * sizeof(int));

	/* step1: only select VIC which is supported in EDID */
	/* copy edid vic list */
	if (prxcap->VIC_count > 0)
		memcpy(edid_vics, prxcap->VIC, sizeof(int) * prxcap->VIC_count);
	for (i = 0; i < VESA_MAX_TIMING && prxcap->vesa_timing[i]; i++)
		edid_vics[prxcap->VIC_count + i] = prxcap->vesa_timing[i];

	for (i = 0; i < vic_len; i++) {
		vic = edid_vics[i];
		if (vic == HDMI_0_UNKNOWN)
			continue;

		prefer_vic = hdmitx_get_prefer_vic(global_tx_common, vic);
		/* if mode_best_vic is support by RX, try 16x9 first */
		if (prefer_vic != vic) {
			HDMITX_DEBUG("%s: check prefer vic:%d exist, ignore [%d].\n",
					__func__, prefer_vic, vic);
			continue;
		}

		timing = hdmitx_mode_vic_to_hdmi_timing(vic);
		if (!timing) {
			/* HDMITX_ERROR("%s: unsupport vic [%d]\n", __func__, vic); */
			continue;
		}

		/* step2, check if VIC is supported by SOC hdmitx */
		if (hdmitx_common_validate_vic(global_tx_common, vic) != 0) {
			/* HDMITX_ERROR("%s: vic[%d] over range.\n", __func__, vic); */
			continue;
		}

		/*
		 * step3, build format with basic mode/attr and check
		 * if it's supported by EDID/hdmitx_cap
		 */
		if (hdmitx_common_check_valid_para_of_vic(global_tx_common, vic) != 0) {
			/* HDMITX_ERROR("%s: vic[%d] check fmt attr failed.\n", __func__, vic); */
			continue;
		}

		mode_name = timing->sname ? timing->sname : timing->name;

		pos += snprintf(buf + pos, size - pos, "%s", mode_name);
		if (vic == prxcap->native_vic)
			pos += snprintf(buf + pos, size - pos, "*\n");
		else
			pos += snprintf(buf + pos, size - pos, "\n");
	}

	vfree(edid_vics);
	mutex_unlock(&global_tx_common->valid_mutex);
	return pos;
}

static ssize_t disp_cap_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	return _disp_cap_show(dev, attr, buf, PAGE_SIZE);
}

static DEVICE_ATTR_RO(disp_cap);

/* cea_cap, a clone of disp_cap */
static ssize_t cea_cap_show(struct device *dev,
			    struct device_attribute *attr,
			    char *buf)
{
	return disp_cap_show(dev, attr, buf);
}

static DEVICE_ATTR_RO(cea_cap);

static ssize_t vesa_cap_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	int i;
	enum hdmi_vic *vesa_t = &global_tx_common->rxcap.vesa_timing[0];
	int pos = 0;

	for (i = 0; vesa_t[i] && i < VESA_MAX_TIMING; i++) {
		const struct hdmi_timing *timing = hdmitx_mode_vic_to_hdmi_timing(vesa_t[i]);

		if (timing && timing->vic >= HDMITX_VESA_OFFSET)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "%s\n",
					timing->name);
	}
	return pos;
}

static DEVICE_ATTR_RO(vesa_cap);

static ssize_t _dc_cap_show(struct device *dev,
			   struct device_attribute *attr, char *buf, int size)
{
	int pos = 0;
	struct rx_cap *prxcap = &global_tx_common->rxcap;
	const struct dv_info *dv =  &prxcap->dv_info;
	const struct dv_info *dv2 = &prxcap->dv_info2;
	int i;

	/* DVI case, only rgb,8bit */
	if (prxcap->ieeeoui != HDMI_IEEE_OUI) {
		pos += snprintf(buf + pos, size - pos, "rgb,8bit\n");
		return pos;
	}

	if (prxcap->dc_36bit_420)
		pos += snprintf(buf + pos, size - pos, "420,12bit\n");
	if (prxcap->dc_30bit_420)
		pos += snprintf(buf + pos, size - pos, "420,10bit\n");

	for (i = 0; i < Y420_VIC_MAX_NUM; i++) {
		if (prxcap->y420_vic[i]) {
			pos += snprintf(buf + pos, size - pos,
				"420,8bit\n");
			break;
		}
	}

	if (prxcap->native_Mode & (1 << 5)) {
		if (prxcap->dc_y444) {
			if (prxcap->dc_36bit || dv->sup_10b_12b_444 == 0x2 ||
			    dv2->sup_10b_12b_444 == 0x2)
				pos += snprintf(buf + pos, size - pos, "444,12bit\n");
			if (prxcap->dc_30bit || dv->sup_10b_12b_444 == 0x1 ||
			    dv2->sup_10b_12b_444 == 0x1) {
				pos += snprintf(buf + pos, size - pos, "444,10bit\n");
			}
		}
		pos += snprintf(buf + pos, size - pos, "444,8bit\n");
	}
	/* y422, not check dc */
	if (prxcap->native_Mode & (1 << 4))
		pos += snprintf(buf + pos, size - pos, "422,12bit\n");

	if (prxcap->dc_36bit || dv->sup_10b_12b_444 == 0x2 ||
	    dv2->sup_10b_12b_444 == 0x2)
		pos += snprintf(buf + pos, size - pos, "rgb,12bit\n");
	if (prxcap->dc_30bit || dv->sup_10b_12b_444 == 0x1 ||
	    dv2->sup_10b_12b_444 == 0x1)
		pos += snprintf(buf + pos, size - pos, "rgb,10bit\n");
	pos += snprintf(buf + pos, size - pos, "rgb,8bit\n");
	return pos;
}

static ssize_t dc_cap_show(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	return _dc_cap_show(dev, attr, buf, PAGE_SIZE);
}

static DEVICE_ATTR_RO(dc_cap);

static ssize_t aud_cap_show(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct rx_cap *prxcap = &global_tx_common->rxcap;

	return _show_aud_cap(prxcap, buf, PAGE_SIZE);
}

static DEVICE_ATTR_RO(aud_cap);

static ssize_t preferred_mode_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	int pos = 0;
	struct rx_cap *prxcap = &global_tx_common->rxcap;
	const char *modename =
		hdmitx_mode_get_timing_name(prxcap->preferred_mode);

	pos += snprintf(buf + pos, PAGE_SIZE, "%s\n", modename);

	return pos;
}

static DEVICE_ATTR_RO(preferred_mode);

static bool pre_process_str(const char *name, char *mode, char *attr)
{
	int i;
	const char *color_format[4] = {"444", "422", "420", "rgb"};
	char *search_pos = 0;

	if (!mode || !attr)
		return false;

	for (i = 0 ; i < 4 ; i++) {
		search_pos = strstr(name, color_format[i]);
		if (search_pos)
			break;
	}
	/* no cs parsed, return error */
	if (!search_pos)
		return false;

	/* search remaining color_formats, if have more than one cs string, return error */
	i++;
	for (; i < 4 ; i++) {
		if (strstr(search_pos, color_format[i]))
			return false;
	}

	/*copy mode name;*/
	memcpy(mode, name, search_pos - name);
	/*copy attr str;*/
	memcpy(attr, search_pos, strlen(search_pos));

	//HDMITX_INFO("%s parse (%s,%s) from (%s)\n", __func__, mode, attr, name);

	return true;
}

/* validation step:
 * step1, check if mode related VIC is supported in EDID
 * step2, check if VIC is supported by SOC hdmitx
 * step3, build format with mode/attr and check if it's
 * supported by EDID/hdmitx_cap
 */
static ssize_t valid_mode_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int ret;
	bool valid_mode = false;
	char cvalid_mode[32];
	char modename[32], attrstr[32];
	struct hdmi_format_para tst_para;
	enum hdmi_vic vic = HDMI_0_UNKNOWN;

	mutex_lock(&global_tx_common->valid_mutex);
	memset(modename, 0, sizeof(modename));
	memset(attrstr, 0, sizeof(attrstr));
	memset(cvalid_mode, 0, sizeof(cvalid_mode));

	strncpy(cvalid_mode, buf, sizeof(cvalid_mode));
	cvalid_mode[31] = '\0';
	if (cvalid_mode[0])
		valid_mode = pre_process_str(cvalid_mode, modename, attrstr);

	if (valid_mode) {
		vic = hdmitx_common_parse_vic_in_edid(global_tx_common, modename);
		if (vic == HDMI_0_UNKNOWN) {
			HDMITX_DEBUG("parse vic fail or vic not in EDID %s\n", modename);
			valid_mode = false;
		} else {
			ret = hdmitx_common_validate_vic(global_tx_common, vic);
			if (ret != 0) {
				HDMITX_DEBUG("vic %d not supported by hdmitx,ret: %d\n", vic, ret);
				valid_mode = false;
			}
		}
	}

	if (valid_mode) {
		hdmitx_parse_color_attr(attrstr, &tst_para.cs, &tst_para.cd, &tst_para.cr);
		HDMITX_DEBUG("parse cs %d cd %d\n", tst_para.cs, tst_para.cd);
		ret = hdmitx_common_build_format_para(global_tx_common,
			&tst_para, vic, global_tx_common->frac_rate_policy,
			tst_para.cs, tst_para.cd, tst_para.cr);
		if (ret != 0) {
			HDMITX_DEBUG("build format para failed %d\n", ret);
			hdmitx_format_para_reset(&tst_para);
			valid_mode = false;
		}
	}

	if (valid_mode) {
		ret = hdmitx_common_validate_format_para(global_tx_common, &tst_para);
		if (ret != 0) {
			HDMITX_DEBUG("validate format para failed %d\n", ret);
			valid_mode = false;
		}
	}

	if (valid_mode) {
		ret = count;
	} else {
		HDMITX_DEBUG("invalid_mode input:%s\n", cvalid_mode);
		ret = -1;
	}

	mutex_unlock(&global_tx_common->valid_mutex);
	return ret;
}

static DEVICE_ATTR_WO(valid_mode);

static ssize_t hdmitx_cur_status_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	int pos = 0;

	pos = hdmitx_tracer_read_event(global_tx_common->tx_tracer,
		buf, PAGE_SIZE);
	return pos;
}

static DEVICE_ATTR_RO(hdmitx_cur_status);

static ssize_t debug_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	if (strncmp(buf, "hw ", 3) == 0) {
		global_tx_hw->debugfunc(global_tx_hw, buf + 3);
	} else if (strncmp(buf, "sw ", 3) == 0) {
		if (global_tx_hw->chip_data->chip_type < MESON_CPU_ID_T7)
			hdmitx20_sw_debugfunc(global_tx_common, buf + 3);
		else
			hdmitx21_sw_debugfunc(global_tx_common, buf + 3);
		hdmitx_common_sw_debugfunc(global_tx_common, buf + 3);
	/* Compatible with the original commonly used debug cmd */
	} else if ((strncmp(buf, "bist", 4) == 0) ||
		(strncmp(buf, "hpd_lock", 8) == 0) ||
		(strncmp(buf, "set_div40", 9) == 0)) {
		global_tx_hw->debugfunc(global_tx_hw, buf);
	}
	return count;
}

static DEVICE_ATTR_WO(debug);

/* Indicate whether a rptx under repeater */
static ssize_t hdmi_repeater_tx_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\n",
		!!global_tx_common->tx_hw->hdcp_repeater_en);

	return pos;
}

static DEVICE_ATTR_RO(hdmi_repeater_tx);

static ssize_t hdmi_used_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d",
		global_tx_common->already_used);
	return pos;
}

static DEVICE_ATTR_RO(hdmi_used);

static ssize_t fake_plug_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d", global_tx_common->hpd_state);
}

static ssize_t fake_plug_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	HDMITX_INFO("fake plug %s\n", buf);

	if (strncmp(buf, "1", 1) == 0)
		global_tx_common->hpd_state = 1;

	if (strncmp(buf, "0", 1) == 0)
		global_tx_common->hpd_state = 0;

	hdmitx_common_notify_hpd_status(global_tx_common, false);
	/* notify to drm hdmi */
	hdmitx_fire_drm_hpd_cb_unlocked(global_tx_common);

	return count;
}

static DEVICE_ATTR_RW(fake_plug);

static ssize_t _llm_cap_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	int pos = 0;
	struct rx_cap *prxcap = &global_tx_common->rxcap;

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "%d\n\r", prxcap->cnc3 || prxcap->allm);
	return pos;
}

static ssize_t contenttype_cap_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	return _llm_cap_show(dev, attr, buf);
}

static DEVICE_ATTR_RO(contenttype_cap);

static ssize_t allm_cap_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	return _llm_cap_show(dev, attr, buf);
}

static DEVICE_ATTR_RO(allm_cap);

/*
 * sink_type attr
 * sink, or repeater
 */
static ssize_t sink_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int pos = 0;

	if (!global_tx_common->hpd_state) {
		pos += snprintf(buf + pos, PAGE_SIZE, "none\n");
		return pos;
	}

	if (global_tx_common->rxcap.vsdb_phy_addr.b)
		pos += snprintf(buf + pos, PAGE_SIZE, "repeater\n");
	else
		pos += snprintf(buf + pos, PAGE_SIZE, "sink\n");

	return pos;
}

static DEVICE_ATTR_RO(sink_type);

static ssize_t hdmirx_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int pos = 0;
	const struct dv_info *dv = &global_tx_common->rxcap.dv_info;
	const struct hdr_info *info = &global_tx_common->rxcap.hdr_info;
	struct rx_cap *prxcap = &global_tx_common->rxcap;

	pos += snprintf(buf + pos, PAGE_SIZE - pos,
		"************hdmirx_info************\n");

	pos += snprintf(buf + pos, PAGE_SIZE - pos,
			"******hpd_edid_parsing******\n");
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "hpd:");
	pos += _hpd_state_show(dev, attr, buf + pos, PAGE_SIZE - pos);

	pos += snprintf(buf + pos, PAGE_SIZE - pos,
		"\n******rawedid******\n");
	pos += _rawedid_show(dev, attr, buf + pos, PAGE_SIZE - pos);

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "\nedid_parsing:");
	pos += _edid_parsing_show(dev, attr, buf + pos, PAGE_SIZE - pos);

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "\n******edid******\n");
	pos += hdmitx_edid_print_sink_cap(&global_tx_common->rxcap,
		buf + pos, PAGE_SIZE - pos);

	pos += snprintf(buf + pos, PAGE_SIZE - pos,
		"\n******dc_cap******\n");
	pos += _dc_cap_show(dev, attr, buf + pos, PAGE_SIZE - pos);

	pos += snprintf(buf + pos, PAGE_SIZE - pos,
		"\n******disp_cap******\n");
	pos += _disp_cap_show(dev, attr, buf + pos, PAGE_SIZE - pos);

	pos += snprintf(buf + pos, PAGE_SIZE - pos,
		"\n******dv_cap******\n");
	pos += _show_dv_cap(dev, attr, buf + pos, dv, PAGE_SIZE - pos);

	pos += snprintf(buf + pos, PAGE_SIZE - pos,
		"\n******hdr_cap******\n");
	pos +=  _hdr_cap_show(dev, attr, buf + pos, info, PAGE_SIZE - pos);

	pos += snprintf(buf + pos, PAGE_SIZE - pos,
		"\n******aud_cap******\n");
	pos += _show_aud_cap(prxcap, buf + pos, PAGE_SIZE - pos);

	return pos > PAGE_SIZE ? PAGE_SIZE : pos;
}

static DEVICE_ATTR_RO(hdmirx_info);

static ssize_t support_3d_show(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\n",
			global_tx_common->rxcap.threeD_present);
	return pos;
}

static DEVICE_ATTR_RO(support_3d);

static ssize_t avmute_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int ret = 0;
	int pos = 0;

	ret = hdmitx_hw_cntl_misc(global_tx_hw, MISC_READ_AVMUTE_OP, 0);
	pos += snprintf(buf + pos, PAGE_SIZE, "%d", ret);

	return pos;
}

/*
 *  1: set avmute
 * -1: clear avmute
 *  0: off avmute
 */
static ssize_t avmute_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int cmd = OFF_AVMUTE;
	static int mask0;
	static int mask1;

	HDMITX_INFO("%s %s\n", __func__, buf);
	if (strncmp(buf, "-1", 2) == 0) {
		cmd = CLR_AVMUTE;
		mask0 = -1;
	} else if (strncmp(buf, "0", 1) == 0) {
		cmd = OFF_AVMUTE;
		mask0 = 0;
	} else if (strncmp(buf, "1", 1) == 0) {
		cmd = SET_AVMUTE;
		mask0 = 1;
	}
	if (strncmp(buf, "r-1", 3) == 0) {
		cmd = CLR_AVMUTE;
		mask1 = -1;
	} else if (strncmp(buf, "r0", 2) == 0) {
		cmd = OFF_AVMUTE;
		mask1 = 0;
	} else if (strncmp(buf, "r1", 2) == 0) {
		cmd = SET_AVMUTE;
		mask1 = 1;
	}
	if (mask0 == 1 || mask1 == 1)
		cmd = SET_AVMUTE;
	else if ((mask0 == -1) && (mask1 == -1))
		cmd = CLR_AVMUTE;

	hdmitx_common_avmute_locked(global_tx_common, cmd, AVMUTE_PATH_1);

	return count;
}

static DEVICE_ATTR_RW(avmute);

static ssize_t config_csc_en_show(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\n\r",
		global_tx_common->config_csc_en);

	return pos;
}

static ssize_t config_csc_en_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf,
					size_t count)
{
	int csc_en = 0;

	HDMITX_INFO("store config_csc_en as %s\n", buf);

	if (com_str(buf, "0"))
		csc_en = 0;
	else if (com_str(buf, "1"))
		csc_en = 1;

	hdmitx_hw_cntl_config(global_tx_hw, CONFIG_CSC_EN, csc_en);

	return count;
}

static DEVICE_ATTR_RW(config_csc_en);

static ssize_t soundbar_en_show(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\n\r",
		global_tx_common->event_mgr->soundbar_en_flag);

	return pos;
}

/* For controlling the soundbar switch on Android.
 * 1: turn on
 * 0: turn off
 */
static ssize_t soundbar_en_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf,
					size_t count)
{
	int soundbar_en = 0;

	mutex_lock(&global_tx_common->hdmimode_mutex);
	if (com_str(buf, "0")) {
		soundbar_en = 0;
		if (global_tx_common->hpd_state)
			hdmitx_event_mgr_send_uevent(global_tx_common->event_mgr,
					HDMITX_SOUNDBAR_EVENT, 1, 1);
	} else if (com_str(buf, "1")) {
		soundbar_en = 1;
		if (global_tx_common->hpd_state)
			hdmitx_event_mgr_send_uevent(global_tx_common->event_mgr,
					HDMITX_SOUNDBAR_EVENT, 0, 1);
	}
	global_tx_common->event_mgr->soundbar_en_flag = soundbar_en;
	mutex_unlock(&global_tx_common->hdmimode_mutex);
	return count;
}

static DEVICE_ATTR_RW(soundbar_en);

/* disp_mode attr */
static ssize_t disp_mode_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	int pos = 0;
	int i = 0;
	struct hdmi_format_para *para = &global_tx_common->fmt_para;
	struct hdmi_timing *timing = &para->timing;
	struct vinfo_s *vinfo = &global_tx_common->hdmitx_vinfo;

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "cd/cs/cr: %d/%d/%d\n", para->cd,
		para->cs, para->cr);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "scramble/tmds_clk_div40: %d/%d\n",
		para->scrambler_en, para->tmds_clk_div40);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "tmds_clk: %d\n", para->tmds_clk);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "vic: %d\n", timing->vic);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "name: %s\n", timing->name);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "enc_idx: %d\n", global_tx_common->enc_idx);
	if (timing->sname)
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "sname: %s\n",
			timing->sname);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "pi_mode: %c\n",
		timing->pi_mode ? 'P' : 'I');
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "h/v_freq: %d/%d\n",
		timing->h_freq, timing->v_freq);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "pixel_freq: %d\n",
		timing->pixel_freq);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "h_total: %d\n", timing->h_total);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "h_blank: %d\n", timing->h_blank);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "h_front: %d\n", timing->h_front);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "h_sync: %d\n", timing->h_sync);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "h_back: %d\n", timing->h_back);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "h_active: %d\n", timing->h_active);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "v_total: %d\n", timing->v_total);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "v_blank: %d\n", timing->v_blank);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "v_front: %d\n", timing->v_front);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "v_sync: %d\n", timing->v_sync);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "v_back: %d\n", timing->v_back);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "v_active: %d\n", timing->v_active);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "v_sync_ln: %d\n", timing->v_sync_ln);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "h/v_pol: %d/%d\n",
		timing->h_pol, timing->v_pol);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "h/v_pict: %d/%d\n",
		timing->h_pict, timing->v_pict);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "h/v_pixel: %d/%d\n",
		timing->h_pixel, timing->v_pixel);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "name: %s\n", vinfo->name);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "mode: %d\n", vinfo->mode);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "ext_name: %s\n", vinfo->ext_name);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "frac: %d\n", vinfo->frac);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "width/height: %d/%d\n",
		vinfo->width, vinfo->height);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "field_height: %d\n", vinfo->field_height);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "aspect_ratio_num/den: %d/%d\n",
		vinfo->aspect_ratio_num, vinfo->aspect_ratio_den);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "screen_real_width/height: %d/%d\n",
		vinfo->screen_real_width, vinfo->screen_real_height);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "sync_duration_num/den: %d/%d\n",
		vinfo->sync_duration_num, vinfo->sync_duration_den);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "brr_duration: %d\n", vinfo->brr_duration);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "video_clk: %d\n", vinfo->video_clk);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "h/vtotal: %d/%d\n",
		vinfo->htotal, vinfo->vtotal);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "hdmichecksum:\n");
	for (i = 0; i < sizeof(vinfo->hdmichecksum); i++)
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "%02x", vinfo->hdmichecksum[i]);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "\n");
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "info_3d: %d\n", vinfo->info_3d);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "fr_adj_type: %d\n", vinfo->fr_adj_type);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "viu_color_fmt: %d\n", vinfo->viu_color_fmt);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "viu_mux: %d\n", vinfo->viu_mux);
	/* master_display_info / hdr_info / rx_latency */

	return pos;
}

static ssize_t disp_mode_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	int ret = 0;

	ret = set_disp_mode(global_tx_common, buf);
	if (ret < 0)
		HDMITX_ERROR("%s: set mode failed\n", __func__);
	return count;
}

static DEVICE_ATTR_RW(disp_mode);

static ssize_t hdmi_hdr_status_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	int pos = 0;
	enum hdmi_tf_type type = HDMI_NONE;

	type = hdmitx_hw_get_state(global_tx_hw, STAT_TX_HDR10P, 0);
	if (type) {
		if (type == HDMI_HDR10P_DV_VSIF) {
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "HDR10Plus-VSIF");
			return pos;
		}
	}
	type = hdmitx_hw_get_state(global_tx_hw, STAT_TX_DV, 0);
	if (type) {
		if (type == HDMI_DV_VSIF_STD) {
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "DolbyVision-Std");
			return pos;
		} else if (type == HDMI_DV_VSIF_LL) {
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "DolbyVision-Lowlatency");
			return pos;
		}
	}
	type = hdmitx_hw_get_state(global_tx_hw, STAT_TX_HDR, 0);
	if (type) {
		if (type == HDMI_HDR_SMPTE_2084) {
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "HDR10-GAMMA_ST2084");
			return pos;
		} else if (type == HDMI_HDR_HLG) {
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "HDR10-GAMMA_HLG");
			return pos;
		} else if (type == HDMI_HDR_HDR) {
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "HDR10-others");
			return pos;
		}
	}
	/* default is SDR */
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "SDR");

	return pos;
}

static DEVICE_ATTR_RO(hdmi_hdr_status);

static ssize_t sspll_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf,
			   size_t count)
{
	int val = 0;

	if (isdigit(buf[0])) {
		val = buf[0] - '0';
		HDMITX_INFO("set sspll : %d\n", val);
		if (val == 0 || val == 1)
			global_tx_common->sspll = val;
		else
			HDMITX_INFO("sspll only accept as 0 or 1\n");
	}

	return count;
}

static ssize_t sspll_show(struct device *dev,
			  struct device_attribute *attr,
			  char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\n",
		global_tx_common->sspll);

	return pos;
}

static DEVICE_ATTR_RW(sspll);

static ssize_t rxsense_policy_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	int val = 0;

	if (isdigit(buf[0])) {
		val = buf[0] - '0';
		HDMITX_INFO("set rxsense_policy as %d\n", val);
		if (val == 0 || val == 1)
			global_tx_common->rxsense_policy = val;
		else
			HDMITX_INFO("only accept as 0 or 1\n");
	}
	if (global_tx_common->rxsense_policy)
		queue_delayed_work(global_tx_common->rxsense_wq,
				   &global_tx_common->work_rxsense, 0);
	else
		cancel_delayed_work(&global_tx_common->work_rxsense);

	return count;
}

static ssize_t rxsense_policy_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\n",
		global_tx_common->rxsense_policy);

	return pos;
}

static DEVICE_ATTR_RW(rxsense_policy);

static ssize_t rxsense_state_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	int pos = 0;
	int sense;

	sense = hdmitx_hw_cntl_misc(global_tx_hw, MISC_TMDS_RXSENSE, 0);

	pos += snprintf(buf + pos, PAGE_SIZE, "%d", sense);
	return pos;
}

static DEVICE_ATTR_RO(rxsense_state);

/*
 * cedst_policy: 0, no CED feature
 *	       1, auto mode, depends on RX scdc_present
 *	       2, forced CED feature
 */
static ssize_t cedst_policy_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf,
				  size_t count)
{
	int val = 0;

	if (isdigit(buf[0])) {
		val = buf[0] - '0';
		HDMITX_INFO("set cedst_policy as %d\n", val);
		if (val == 0 || val == 1 || val == 2) {
			global_tx_common->cedst_policy = val;
			/* Auto mode, depends on Rx */
			if (val == 1) {
				/* check RX scdc_present */
				if (global_tx_common->rxcap.scdc_present)
					global_tx_common->cedst_en = 1;
				else
					global_tx_common->cedst_en = 0;
			} else if (val == 2) {
				global_tx_common->cedst_en = 1;
			}
		} else {
			HDMITX_INFO("only accept as 0, 1(auto), or 2(force)\n");
		}
	}
	if (global_tx_common->cedst_en)
		queue_delayed_work(global_tx_common->cedst_wq, &global_tx_common->work_cedst, 0);
	else
		cancel_delayed_work(&global_tx_common->work_cedst);

	return count;
}

static ssize_t cedst_policy_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\n",
		global_tx_common->cedst_policy);

	return pos;
}

static DEVICE_ATTR_RW(cedst_policy);

static ssize_t ready_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\r\n",
		global_tx_common->ready);
	return pos;
}

static DEVICE_ATTR_RO(ready);

static ssize_t hdr_priority_mode_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\r\n",
		global_tx_common->hdr_priority);

	return pos;
}

/*
 * hide or enable HDR capabilities.
 * 0 : No HDR capabilities are hidden
 * 1 : DV Capabilities are hidden
 * 2 : All HDR capabilities are hidden
 */
static ssize_t hdr_priority_mode_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	unsigned int val = 0;
	struct vinfo_s *info = NULL;

	HDMITX_INFO("%s[%d] buf:%s hdr_priority:0x%x\n", __func__, __LINE__, buf,
		global_tx_common->hdr_priority);
	if ((strncmp("0", buf, 1) == 0) || (strncmp("1", buf, 1) == 0) ||
	    (strncmp("2", buf, 1) == 0)) {
		val = buf[0] - '0';
	}

	if (val == global_tx_common->hdr_priority)
		return count;
	info = hdmitx_get_current_vinfo(NULL);
	if (!info)
		return count;
	mutex_lock(&global_tx_common->hdmimode_mutex);
	global_tx_common->hdr_priority = val;
	mutex_unlock(&global_tx_common->hdmimode_mutex);
	return count;
}

static DEVICE_ATTR_RW(hdr_priority_mode);

static ssize_t hdr_mute_frame_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\r\n", global_tx_common->hdr_mute_frame);
	return pos;
}

static ssize_t hdr_mute_frame_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long mute_frame = 0;

	HDMITX_INFO("set hdr_mute_frame: %s\n", buf);
	if (kstrtoul(buf, 10, &mute_frame) == 0)
		global_tx_common->hdr_mute_frame = mute_frame;
	return count;
}

static DEVICE_ATTR_RW(hdr_mute_frame);

static ssize_t clkmsr_show(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	return global_tx_hw->get_clk(buf, PAGE_SIZE);
}

static DEVICE_ATTR_RO(clkmsr);

static ssize_t lipsync_cap_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	int pos = 0;
	struct rx_cap *prxcap = &global_tx_common->rxcap;

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "Lipsync(in ms)\n");
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "%d, %d\n",
				prxcap->vLatency, prxcap->aLatency);
	return pos;
}

static DEVICE_ATTR_RO(lipsync_cap);

static ssize_t vid_mute_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\n",
		atomic_read(&global_tx_common->kref_video_mute));
	return pos;
}

static ssize_t vid_mute_store(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t count)
{
	atomic_t kref_video_mute = global_tx_common->kref_video_mute;

	HDMITX_INFO("store vid_mute %s\n", buf);

	if (buf[0] == '1') {
		atomic_inc(&kref_video_mute);
		if (atomic_read(&kref_video_mute) == 1)
			global_tx_common->vid_mute_op = VIDEO_MUTE;
	}
	if (buf[0] == '0') {
		if (!(atomic_sub_and_test(0, &kref_video_mute))) {
			atomic_dec(&kref_video_mute);
			if (atomic_sub_and_test(0, &kref_video_mute))
				global_tx_common->vid_mute_op = VIDEO_UNMUTE;
		}
	}

	global_tx_common->kref_video_mute = kref_video_mute;

	return count;
}

static DEVICE_ATTR_RW(vid_mute);

/* is hdcp cts test equipment */
static ssize_t is_hdcp_cts_te_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\n",
		hdmitx_edid_only_support_sd(&global_tx_common->rxcap));

	return pos;
}

static DEVICE_ATTR_RO(is_hdcp_cts_te);

ssize_t hdcp_lstore_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	int pos = 0;
	int lstore = global_tx_hw->lstore;

	/* if current TX is RP-TX, then return lstore as 00 */
	/* hdcp_lstore is used under only TX */
	if (global_tx_hw->hdcp_repeater_en == 1) {
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "00\n");
		return pos;
	}

	if (lstore < 0x10) {
		lstore = 0;
		if (hdmitx_hw_cntl_ddc(global_tx_hw, DDC_HDCP_14_LSTORE, 0))
			lstore |= BIT(0);
		else
			hdmitx_current_status(HDMITX_HDCP_AUTH_NO_14_KEYS_ERROR);
		if (hdmitx_hw_cntl_ddc(global_tx_hw,
			DDC_HDCP_22_LSTORE, 0))
			lstore |= BIT(1);
		else
			hdmitx_current_status(HDMITX_HDCP_AUTH_NO_22_KEYS_ERROR);
	}
	if ((lstore & 0x3) == 0x3) {
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "14+22\n");
	} else {
		if (lstore & 0x1)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "14\n");
		if (lstore & 0x2)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "22\n");
		if ((lstore & 0xf) == 0)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "00\n");
	}
	return pos;
}

static ssize_t hdcp_lstore_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	HDMITX_INFO("hdcp: set lstore as %s\n", buf);
	if (strncmp(buf, "-1", 2) == 0)
		global_tx_hw->lstore = 0x0;
	if (strncmp(buf, "0", 1) == 0)
		global_tx_hw->lstore = 0x10;
	if (strncmp(buf, "11", 2) == 0)
		global_tx_hw->lstore = 0x11;
	if (strncmp(buf, "12", 2) == 0)
		global_tx_hw->lstore = 0x12;
	if (strncmp(buf, "13", 2) == 0)
		global_tx_hw->lstore = 0x13;

	return count;
}

static DEVICE_ATTR_RW(hdcp_lstore);

ssize_t hdcp_mode_show(struct device *dev,
			      struct device_attribute *attr,
			      char *buf)
{
	int pos = 0;
	unsigned int hdcp_ret = 0;

	switch (global_tx_common->hdcp_mode) {
	case 1:
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "14");
		break;
	case 2:
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "22");
		break;
	default:
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "off");
		break;
	}
	if (global_tx_common->hdcp_ctl_lvl > 0 &&
	    global_tx_common->hdcp_mode > 0) {
		hdcp_ret = hdmitx_hw_cntl_ddc(global_tx_hw,
						      DDC_HDCP_GET_AUTH, 0);
		if (hdcp_ret == 1)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, ": succeed\n");
		else
			pos += snprintf(buf + pos, PAGE_SIZE - pos, ": fail\n");
	}

	return pos;
}

static ssize_t hdcp_mode_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	hdmitx_set_hdcp_mode(global_tx_common, buf);
	return count;
}

static DEVICE_ATTR_RW(hdcp_mode);

ssize_t hdcp_ver_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	return hdmitx_get_hdcp_ver(global_tx_common, buf, PAGE_SIZE);
}

static DEVICE_ATTR_RO(hdcp_ver);

static ssize_t hdmitx_pkt_dump_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	return global_tx_hw->pkt_dump(buf, PAGE_SIZE);
}

static DEVICE_ATTR_RO(hdmitx_pkt_dump);

static ssize_t hdmitx_basic_config_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int pos = 0;
	u8 *conf;
	u8 *tmp;
	u32 colormetry;
	u32 hcnt, vcnt;
	char bksv_buf[5];
	int cs, cd, i;
	enum hdmi_vic vic;
	enum hdmi_hdr_transfer hdr_transfer_feature;
	enum hdmi_hdr_color hdr_color_feature;
	struct dv_vsif_para *data;
	enum hdmi_tf_type type = HDMI_NONE;

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "************\n");
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "hdmi_config_info\n");
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "************\n");

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "display_mode in:%s\n",
		get_vout_mode_internal());

	vic = hdmitx_hw_get_state(global_tx_hw, STAT_VIDEO_VIC, 0);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "display_mode out:%s\n",
		hdmitx_mode_get_timing_name(vic));

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "attr in:%s\n\r",
		global_tx_common->tst_fmt_attr);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "attr out:");
	cs = hdmitx_hw_get_state(global_tx_hw, STAT_VIDEO_CS, 0);
	switch (cs & 0x3) {
	case 0:
		conf = "RGB";
		break;
	case 1:
		conf = "422";
		break;
	case 2:
		conf = "444";
		break;
	case 3:
		conf = "420";
		break;
	}
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "%s,", conf);

	cd = hdmitx_hw_get_state(global_tx_hw, STAT_VIDEO_CD, 0);
	switch (cd) {
	case 4:
		conf = "8bit";
		break;
	case 5:
		conf = "10bit";
		break;
	case 6:
		conf = "12bit";
		break;
	case 7:
		conf = "16bit";
		break;
	default:
		conf = "reserved";
	}
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "%s\n", conf);

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "hdr_status:");
	type = hdmitx_hw_get_state(global_tx_hw, STAT_TX_HDR10P, 0);
	if (type) {
		if (type == HDMI_HDR10P_DV_VSIF)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "HDR10Plus-VSIF");
	}
	type = hdmitx_hw_get_state(global_tx_hw, STAT_TX_DV, 0);
	if (type) {
		if (type == HDMI_DV_VSIF_STD)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "DolbyVision-Std");
		else if (type == HDMI_DV_VSIF_LL)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "DolbyVision-Lowlatency");
	}
	type = hdmitx_hw_get_state(global_tx_hw, STAT_TX_HDR, 0);
	if (type) {
		if (type == HDMI_HDR_SMPTE_2084)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "HDR10-GAMMA_ST2084");
		else if (type == HDMI_HDR_HLG)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "HDR10-GAMMA_HLG");
		else if (type == HDMI_HDR_HDR)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "HDR10-others");
		else if (type == HDMI_HDR_SDR)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "SDR");
	}
	if (type == HDMI_NONE)
		/* default is SDR */
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "SDR");
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "\n");

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "******config******\n");
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "cur_VIC: %d\n",
		global_tx_common->fmt_para.vic);
	pos += hdmitx_format_para_print(&global_tx_common->fmt_para, buf + pos);

	if (global_tx_common->cur_audio_param.aud_output_en)
		conf = "on";
	else
		conf = "off";
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "audio config: %s\n", conf);

	pos += hdmitx_audio_para_print(&global_tx_common->cur_audio_param, buf + pos);

	if (global_tx_common->flag_3dfp)
		conf = "FramePacking";
	else if (global_tx_common->flag_3dss)
		conf = "SidebySide";
	else if (global_tx_common->flag_3dtb)
		conf = "TopButtom";
	else
		conf = "off";
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "3D config: %s\n", conf);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "\n");

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "******hdcp******\n");
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "hdcp mode:");
	pos += hdcp_mode_show(dev, attr, buf + pos);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "hdcp_lstore:");
	pos += hdcp_lstore_show(dev, attr, buf + pos);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "hdcp_ver:");
	pos += hdcp_ver_show(dev, attr, buf + pos);
	hdmitx_hw_cntl_ddc(global_tx_hw, DDC_HDCP_GET_BKSV,
			(unsigned long)bksv_buf);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "HDCP14 BKSV: ");
	for (i = 0; i < 5; i++) {
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "%02x",
			bksv_buf[i]);
	}

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "hdcp_ctl_lvl:%d\n",
		global_tx_common->hdcp_ctl_lvl);

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "******scdc******\n");
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "div40:%d\n",
		global_tx_common->pre_tmds_clk_div40);

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "******hdmi_pll******\n");
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "sspll:%d\n", global_tx_common->sspll);

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "******dv_vsif_info******\n");
	data = &global_tx_common->vsif_debug_info.data;
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "type: %u, tunnel: %u, sigsdr: %u\n",
		global_tx_common->vsif_debug_info.type,
		global_tx_common->vsif_debug_info.tunnel_mode,
		global_tx_common->vsif_debug_info.signal_sdr);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "dv_vsif_para:\n");
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "ver: %u len: %u\n",
		data->ver, data->length);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "ll: %u dvsig: %u\n",
		data->vers.ver2.low_latency,
		data->vers.ver2.dobly_vision_signal);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "bcMD: %u axMD: %u\n",
		data->vers.ver2.backlt_ctrl_MD_present,
		data->vers.ver2.auxiliary_MD_present);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "PQhi: %u PQlow: %u\n",
		data->vers.ver2.eff_tmax_PQ_hi,
		data->vers.ver2.eff_tmax_PQ_low);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "axrm: %u, axrv: %u, ",
		data->vers.ver2.auxiliary_runmode,
		data->vers.ver2.auxiliary_runversion);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "axdbg: %u\n",
		data->vers.ver2.auxiliary_debug0);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "\n");

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "***drm_config_data***\n");
	hdr_transfer_feature = (global_tx_common->drm_config_data.features >> 8) & 0xff;
	hdr_color_feature = (global_tx_common->drm_config_data.features >> 16) & 0xff;
	colormetry = (global_tx_common->drm_config_data.features >> 30) & 0x1;
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "tf=%u, cf=%u, colormetry=%u\n",
		hdr_transfer_feature, hdr_color_feature,
		colormetry);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "primaries:\n");
	for (vcnt = 0; vcnt < 3; vcnt++) {
		for (hcnt = 0; hcnt < 2; hcnt++)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "%u, ",
			global_tx_common->drm_config_data.primaries[vcnt][hcnt]);
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "\n");
	}
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "white_point: ");
	for (hcnt = 0; hcnt < 2; hcnt++)
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "%u, ",
		global_tx_common->drm_config_data.white_point[hcnt]);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "\n");
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "luminance: ");
	for (hcnt = 0; hcnt < 2; hcnt++)
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "%u, ",
		global_tx_common->drm_config_data.luminance[hcnt]);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "\n");
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "max_content: %u, ",
		global_tx_common->drm_config_data.max_content);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "max_frame_average: %u\n",
		global_tx_common->drm_config_data.max_frame_average);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "\n");

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "***hdr10p_config_data***\n");
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "appver: %u, tlum: %u, avgrgb: %u\n",
		global_tx_common->hdr10p_config_data.application_version,
		global_tx_common->hdr10p_config_data.targeted_max_lum,
		global_tx_common->hdr10p_config_data.average_maxrgb);
	tmp = global_tx_common->hdr10p_config_data.distribution_values;
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "distribution_values:\n");
	for (vcnt = 0; vcnt < 3; vcnt++) {
		for (hcnt = 0; hcnt < 3; hcnt++)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "%u, ", tmp[vcnt * 3 + hcnt]);
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "\n");
	}
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "nbca: %u, knpx: %u, knpy: %u\n",
		global_tx_common->hdr10p_config_data.num_bezier_curve_anchors,
		global_tx_common->hdr10p_config_data.knee_point_x,
		global_tx_common->hdr10p_config_data.knee_point_y);
	tmp = global_tx_common->hdr10p_config_data.bezier_curve_anchors;
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "bezier_curve_anchors:\n");
	for (vcnt = 0; vcnt < 3; vcnt++) {
		for (hcnt = 0; hcnt < 3; hcnt++)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "%u, ", tmp[vcnt * 3 + hcnt]);
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "\n");
	}
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "gof: %u, ndf: %u\n",
		global_tx_common->hdr10p_config_data.graphics_overlay_flag,
		global_tx_common->hdr10p_config_data.no_delay_flag);
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "\n");

	pos += snprintf(buf + pos, PAGE_SIZE - pos, "***hdmiaud_config_data***\n");
	pos += snprintf(buf + pos, PAGE_SIZE - pos,
		"type: %u, chnum: %u, samrate: %u, samsize: %u\n",
		global_tx_common->hdmiaud_config_data.type,
		global_tx_common->hdmiaud_config_data.chs,
		global_tx_common->hdmiaud_config_data.rate,
		global_tx_common->hdmiaud_config_data.size);

	return pos;
}

static DEVICE_ATTR_RO(hdmitx_basic_config);

static ssize_t config_store(struct device *dev,
			    struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct master_display_info_s data = {0};
	struct hdr10plus_para hdr_data = {0x1, 0x2, 0x3};
	struct cuva_hdr_vs_emds_para cuva_data = {0x1, 0x2, 0x3};
	struct dv_vsif_para vsif_para = {0};
	/* unsigned int mute_us = 0; */
	/* unsigned int mute_frames = 0; */

	HDMITX_INFO("config: %s\n", buf);

	if (strncmp(buf, "info", 4) == 0) {
		HDMITX_INFO("%x %x %x %x %x %x\n",
			hdmitx_hw_get_hdr_st(global_tx_hw),
			hdmitx_hw_get_dv_st(global_tx_hw),
			hdmitx_hw_get_hdr10p_st(global_tx_hw),
			hdmitx_hdr_en(global_tx_hw),
			hdmitx_dv_en(global_tx_hw),
			hdmitx_hdr10p_en(global_tx_hw)
			);
	} else if (strncmp(buf, "sdr_hdr_dov", 11) == 0) {
		/*
		 * firstly stay at SDR state, then send hdr->dv packet to
		 * emulate SDR->HDR->DV switch, DRM-TX-47
		 */
		/* step1: SDR-->HDR */
		data.features = 0x00091000;
		hdmitx_set_drm_pkt(&data);
		/* mute_us = mute_frames * hdmitx_get_frame_duration(); */
		/* usleep_range(mute_us, mute_us + 10); */
		/* step2: HDR->DV_LL */
		vsif_para.ver = 0x1;
		vsif_para.length = 0x1b;
		vsif_para.ver2_l11_flag = 0;
		vsif_para.vers.ver2.low_latency = 1;
		vsif_para.vers.ver2.dobly_vision_signal = 1;
		hdmitx_set_vsif_pkt(4, 0, &vsif_para, false);
	} else if (strncmp(buf, "sdr", 3) == 0) {
		data.features = 0x00010100;
		hdmitx_set_drm_pkt(&data);
	} else if (strncmp(buf, "hdr", 3) == 0) {
		data.features = 0x00091000;
		hdmitx_set_drm_pkt(&data);
	} else if (strncmp(buf, "sbtm", 4) == 0) {
		struct vtem_sbtm_st sbtm = {
			.sbtm_ver = 0x2,
			.sbtm_mode = 0x3,
			.sbtm_type = 0x1,
			.grdm_min = 0x1,
			.grdm_lum = 2,
			/* MD2/3 */
			.frmpblimitint = 0xdcba,
		};
		hdmitx_set_sbtm_pkt(&sbtm);
	} else if (strncmp(buf, "hlg", 3) == 0) {
		data.features = 0x00091200;
		hdmitx_set_drm_pkt(&data);
	} else if (strncmp(buf, "vsif", 4) == 0) {
		if (buf[4] == '1' && buf[5] == '1') {
			/* DV STD */
			vsif_para.ver = 0x1;
			vsif_para.length = 0x1b;
			vsif_para.ver2_l11_flag = 0;
			vsif_para.vers.ver2.low_latency = 0;
			vsif_para.vers.ver2.dobly_vision_signal = 1;
			hdmitx_set_vsif_pkt(1, 1, &vsif_para, false);
		} else if (buf[4] == '1' && buf[5] == '0') {
			/* DV STD packet, but dolby_vision_signal bit cleared */
			vsif_para.ver = 0x1;
			vsif_para.length = 0x1b;
			vsif_para.ver2_l11_flag = 0;
			vsif_para.vers.ver2.low_latency = 0;
			vsif_para.vers.ver2.dobly_vision_signal = 0;
			hdmitx_set_vsif_pkt(1, 1, &vsif_para, false);
		} else if (buf[4] == '4' && buf[5] == '1') {
			/* DV LL */
			vsif_para.ver = 0x1;
			vsif_para.length = 0x1b;
			vsif_para.ver2_l11_flag = 0;
			vsif_para.vers.ver2.low_latency = 1;
			vsif_para.vers.ver2.dobly_vision_signal = 1;
			hdmitx_set_vsif_pkt(4, 0, &vsif_para, false);
		}  else if (buf[4] == '4' && buf[5] == '0') {
			/* DV LL packet, but dolby_vision_signal bit cleared */
			vsif_para.ver = 0x1;
			vsif_para.length = 0x1b;
			vsif_para.ver2_l11_flag = 0;
			vsif_para.vers.ver2.low_latency = 1;
			vsif_para.vers.ver2.dobly_vision_signal = 0;
			hdmitx_set_vsif_pkt(4, 0, &vsif_para, false);
		} else if (buf[4] == '0') {
			/* exit DV to SDR */
			hdmitx_set_vsif_pkt(0, 0, NULL, true);
		}
	} else if (strncmp(buf, "hdr10+", 6) == 0) {
		hdmitx_set_hdr10plus_pkt(1, &hdr_data);
	} else if (strncmp(buf, "cuva", 4) == 0) {
		hdmitx_set_cuva_hdr_vs_emds(&cuva_data);
	}
	return count;
}

static DEVICE_ATTR_WO(config);

#ifdef CONFIG_AMLOGIC_HDMITX21
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT
static ssize_t vrr_cap_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	if (global_tx_common->hdmi_init == HDMITX21)
		return _vrr_cap_show(dev, attr, buf);
	return 0;
}

static DEVICE_ATTR_RO(vrr_cap);
#endif

/*
 * for decoder/hwc or sysctl to control the low latency mode,
 * as they don't care if sink support ALLM OR HDMI1.X game mode
 * so need hdmitx driver to device to send ALLM OR HDMI1.X game
 * mode according to capability of EDID
 */
static ssize_t ll_mode_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	int pos = 0;

	if (global_tx_common->rxcap.allm) {
		if (global_tx_common->allm_mode == 1)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "HDMI2.1_ALLM_ENABLED\n\r");
		else
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "HDMI2.1_ALLM_DISABLED\n\r");
	}
	if (global_tx_common->rxcap.cnc3) {
		if (global_tx_common->ct_mode == 1)
			pos += snprintf(buf + pos, PAGE_SIZE - pos,
			"HDMI1.x_GAME_MODE_ENABLED\n\r");
		else
			pos += snprintf(buf + pos, PAGE_SIZE - pos,
			"HDMI1.x_GAME_MODE_DISABLED\n\r");
	}

	if (!global_tx_common->rxcap.allm && !global_tx_common->rxcap.cnc3)
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "HDMI_LATENCY_MODE_UNKNOWN\n\r");
	return pos;
}

/*
 * 1.echo 1 to enable ALLM OR HDMI1.X game mode
 * if sink support ALLM, then output ALLM mode;
 * else if support HDMI1.X game mode, then output
 * HDMI1.X game mode; else, do nothing
 * 2.echo 0 to disable ALLM and HDMI1.X game mode
 */
static ssize_t ll_mode_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf,
			       size_t count)
{
	global_tx_common->ll_enabled_in_auto_mode = com_str(buf, "1");
	struct hdmitx_hw_common *tx_hw = global_tx_common->tx_hw;

	HDMITX_INFO("store ll_enabled_in_auto_mode: %d, ll_user_set_mode:%d\n",
		global_tx_common->ll_enabled_in_auto_mode, global_tx_common->ll_user_set_mode);

	if (global_tx_common->ll_user_set_mode == HDMI_LL_MODE_AUTO) {
		HDMITX_INFO("store ll_mode as %s, calling hdmi_tx_enable_ll_mode()\n", buf);
		hdmitx_hw_cntl_config(tx_hw, CONFIG_ALLM,
				global_tx_common->ll_enabled_in_auto_mode);
	} else {
		HDMITX_INFO("ll mode is forced on/off: %d\n", global_tx_common->ll_user_set_mode);
	}

	return count;
}

static DEVICE_ATTR_RW(ll_mode);

/* for user to force enable/disable low-latency modes */
static ssize_t ll_user_mode_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	int pos = 0;

	switch (global_tx_common->ll_user_set_mode) {
	case HDMI_LL_MODE_ENABLE:
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "HDMI_LL_MODE_ENABLE\n\r");
		break;
	case HDMI_LL_MODE_DISABLE:
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "HDMI_LL_MODE_DISABLE\n\r");
		break;
	case HDMI_LL_MODE_AUTO:
	default:
		pos += snprintf(buf + pos, PAGE_SIZE - pos, "HDMI_LL_MODE_AUTO\n\r");
		break;
	}
	return pos;
}

/*
 * 1.echo enable to enable ALLM OR HDMI1.X game mode
 * if sink support ALLM, then output ALLM mode;
 * else if support HDMI1.X game mode, then output
 * HDMI1.X game mode; else, do nothing
 * 2.echo disable to disable ALLM and HDMI1.X game mode
 * 3.echo auto to enable/disable low-latency mode per
 * content type
 */
static ssize_t ll_user_mode_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf,
			       size_t count)
{
	struct hdmitx_hw_common *tx_hw = global_tx_common->tx_hw;

	HDMITX_INFO("store ll_user_set_mode as %s\n", buf);
	if (com_str(buf, "enable")) {
		global_tx_common->ll_user_set_mode = HDMI_LL_MODE_ENABLE;
		hdmitx_hw_cntl_config(tx_hw, CONFIG_ALLM, ENABLE_ALLM);
	} else if (com_str(buf, "disable")) {
		global_tx_common->ll_user_set_mode = HDMI_LL_MODE_DISABLE;
		hdmitx_hw_cntl_config(tx_hw, CONFIG_ALLM, DISABLE_ALLM);
	} else {
		global_tx_common->ll_user_set_mode = HDMI_LL_MODE_AUTO;
		hdmitx_hw_cntl_config(tx_hw, CONFIG_ALLM,
				global_tx_common->ll_enabled_in_auto_mode);
	}
	return count;
}

static DEVICE_ATTR_RW(ll_user_mode);

static ssize_t aon_output_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\r\n",
		global_tx_common->aon_output);
	return pos;
}

static ssize_t aon_output_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	if (strncmp(buf, "0", 1) == 0)
		global_tx_common->aon_output = 0;
	if (strncmp(buf, "1", 1) == 0)
		global_tx_common->aon_output = 1;
	return count;
}

static DEVICE_ATTR_RW(aon_output);

static ssize_t def_stream_type_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\n",
		global_tx_common->def_stream_type);

	return pos;
}

static ssize_t def_stream_type_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf,
				      size_t count)
{
	u8 val = 0;

	if (isdigit(buf[0])) {
		val = buf[0] - '0';
		HDMITX_INFO("set def_stream_type as %d\n", val);
		if (val == 0 || val == 1)
			global_tx_common->def_stream_type = val;
		else
			HDMITX_INFO("only accept as 0 or 1\n");
	}

	return count;
}

static DEVICE_ATTR_RW(def_stream_type);

static ssize_t is_passthrough_switch_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\n", global_tx_common->is_passthrough_switch);

	return pos;
}

static ssize_t is_passthrough_switch_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf,
				      size_t count)
{
	u8 val = 0;

	if (isdigit(buf[0])) {
		val = buf[0] - '0';
		HDMITX_INFO("set is_passthrough_switch as %d\n", val);
		if (val == 0 || val == 1)
			global_tx_common->is_passthrough_switch = val;
		else
			HDMITX_INFO("only accept as 0 or 1\n");
	}

	return count;
}

static DEVICE_ATTR_RW(is_passthrough_switch);

/*
 * hdcp fail event method 1: add hdcp fail uevent filter
 * below need_filter_hdcp_off and filter_hdcp_off_period
 * sysfs node are for this filter
 */
static ssize_t need_filter_hdcp_off_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\r\n",
		global_tx_common->need_filter_hdcp_off);

	return pos;
}

/*
 * if need to filter hdcp fail uevent, systemcontrol
 * write 1 to this node. for example:
 * when start player game movie, it switch from DV STD to DV LL,
 * before switch mode, write 1 to this node. it means that it
 * won't send hdcp fail uevent if hdcp fail but retry auth
 * pass during filter_hdcp_off_period seconds.
 * note: need_filter_hdcp_off is self cleared after filter
 * period expired
 */
static ssize_t need_filter_hdcp_off_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	unsigned int val = 0;

	if ((strncmp("0", buf, 1) == 0) || (strncmp("1", buf, 1) == 0))
		val = buf[0] - '0';

	global_tx_common->need_filter_hdcp_off = val;
	HDMITX_INFO("set need_filter_hdcp_off: %d\n", val);

	return count;
}

static DEVICE_ATTR_RW(need_filter_hdcp_off);

/*
 * if hdcp fail but retry auth pass during this period(unit: second),
 * then won't sent hdcp fail uevent. the default is 6 second
 */
static ssize_t filter_hdcp_off_period_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\r\n",
		global_tx_common->filter_hdcp_off_period);

	return pos;
}

/* example: write 5 to filter 5 second */
static ssize_t filter_hdcp_off_period_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	unsigned long filter_second = 0;

	HDMITX_INFO("set hdcp fail filter_second: %s\n", buf);
	if (kstrtoul(buf, 10, &filter_second) == 0)
		global_tx_common->filter_hdcp_off_period = filter_second;

	return count;
}

static DEVICE_ATTR_RW(filter_hdcp_off_period);

/* hdcp fail event method 2: don't stop-restart hdcp auth */
/*
 * when start play game movie, it will switch from
 * DV STD to DV LL, hdcp will stop and restart after
 * mode setting done.
 * now provide an option: when DV STD <-> DV LL
 * switch caused by game movie, systemcontrol write 1
 * to not_restart_hdcp node before do mode switch, it
 * means that this colorspace(mode) switch will only
 * switch mode, but won't stop->restart hdcp action to
 * prevent hdcp fail uevent sent to app.
 * note: 1.not sure if it will always work on different TV.
 * 2.after mode switch done, not_restart_hdcp will be self cleared
 */
static ssize_t not_restart_hdcp_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\r\n",
		global_tx_common->not_restart_hdcp);

	return pos;
}

static ssize_t not_restart_hdcp_store(struct device *dev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	unsigned int val = 0;

	if ((strncmp("0", buf, 1) == 0) || (strncmp("1", buf, 1) == 0))
		val = buf[0] - '0';

	global_tx_common->not_restart_hdcp = val;
	HDMITX_INFO("set not_restart_hdcp: %d\n", val);

	return count;
}

static DEVICE_ATTR_RW(not_restart_hdcp);
#endif

#ifdef CONFIG_AMLOGIC_HDMITX
static ssize_t hdcp_clkdis_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	if (global_tx_common->hdmi_init == HDMITX20) {
		hdmitx_hw_cntl_misc(global_tx_hw, MISC_HDCP_CLKDIS,
			buf[0] == '1' ? 1 : 0);
	}
	return count;
}

static DEVICE_ATTR_WO(hdcp_clkdis);

static ssize_t hdcp_pwr_store(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t count)
{
	if (global_tx_common->hdmi_init == HDMITX20) {
		if (buf[0] == '1') {
			global_tx_common->hdcp_tst_sig = 1;
			HDMITX_DEBUG(SYS "set hdcp_pwr 1\n");
		}
	}
	return count;
}

static ssize_t hdcp_pwr_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	int pos = 0;

	if (global_tx_common->hdmi_init == HDMITX20) {
		if (global_tx_common->hdcp_tst_sig == 1) {
			pos += snprintf(buf + pos, PAGE_SIZE, "%d\n",
				global_tx_common->hdcp_tst_sig);
			global_tx_common->hdcp_tst_sig = 0;
			HDMITX_DEBUG(SYS "restore hdcp_pwr 0\n");
		}
	}

	return pos;
}

static DEVICE_ATTR_RW(hdcp_pwr);

static ssize_t hdcp_type_policy_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int val = 0;

	if (global_tx_common->hdmi_init == HDMITX20) {
		if (strncmp(buf, "0", 1) == 0)
			val = 0;
		if (strncmp(buf, "1", 1) == 0)
			val = 1;
		if (strncmp(buf, "2", 1) == 0)
			val = 2;
		if (strncmp(buf, "-1", 2) == 0)
			val = -1;
		HDMITX_INFO(SYS "set hdcp_type_policy as %d\n", val);
		global_tx_common->hdcp_type_policy = val;
	}
	return count;
}

static ssize_t hdcp_type_policy_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int pos = 0;

	if (global_tx_common->hdmi_init == HDMITX20) {
		pos += snprintf(buf + pos, PAGE_SIZE, "%d\n",
			global_tx_common->hdcp_type_policy);
	}

	return pos;
}

static DEVICE_ATTR_RW(hdcp_type_policy);

/*
 * hdcp_topo_info attr
 * For hdcp 22, hdcp_tx22 will write to hdcp_topo_info_store
 * For hdcp 14, directly get from HW
 */

static ssize_t hdcp_topo_info_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	int pos = 0;
	struct hdcprp_topo *topoinfo = NULL;

	if (global_tx_common->hdmi_init == HDMITX20) {
		topoinfo = global_tx_common->topo_info;
		if (!global_tx_common->hdcp_mode) {
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "hdcp mode: 0\n");
			return pos;
		}
		if (!topoinfo)
			return pos;

		if (global_tx_common->hdcp_mode == 1) {
			memset(topoinfo, 0, sizeof(struct hdcprp_topo));
			hdmitx_hw_cntl_ddc(global_tx_hw, DDC_HDCP14_GET_TOPO_INFO,
				(unsigned long)&topoinfo->topo.topo14);
		}

		pos += snprintf(buf + pos, PAGE_SIZE - pos, "hdcp mode: %s\n",
			global_tx_common->hdcp_mode == 1 ? "14" : "22");
		if (global_tx_common->hdcp_mode == 2) {
			topoinfo->hdcp_ver = HDCPVER_22;
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "max_devs_exceeded: %d\n",
				topoinfo->topo.topo22.max_devs_exceeded);
			pos += snprintf(buf + pos, PAGE_SIZE - pos,
				"max_cascade_exceeded: %d\n",
				topoinfo->topo.topo22.max_cascade_exceeded);
			pos += snprintf(buf + pos, PAGE_SIZE - pos,
					"v2_0_repeater_down: %d\n",
				topoinfo->topo.topo22.v2_0_repeater_down);
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "v1_X_device_down: %d\n",
				topoinfo->topo.topo22.v1_X_device_down);
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "device_count: %d\n",
				topoinfo->topo.topo22.device_count);
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "depth: %d\n",
				topoinfo->topo.topo22.depth);
			return pos;
		}
		if (global_tx_common->hdcp_mode == 1) {
			topoinfo->hdcp_ver = HDCPVER_14;
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "max_devs_exceeded: %d\n",
				topoinfo->topo.topo14.max_devs_exceeded);
			pos += snprintf(buf + pos, PAGE_SIZE - pos,
				"max_cascade_exceeded: %d\n",
				topoinfo->topo.topo14.max_cascade_exceeded);
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "device_count: %d\n",
				topoinfo->topo.topo14.device_count);
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "depth: %d\n",
				topoinfo->topo.topo14.depth);
			return pos;
		}
	}

	return pos;
}

static ssize_t hdcp_topo_info_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct hdcprp_topo *topoinfo = NULL;
	int cnt;

	if (global_tx_common->hdmi_init == HDMITX20) {
		topoinfo = global_tx_common->topo_info;
		if (!topoinfo)
			return count;

		if (global_tx_common->hdcp_mode == 2) {
			memset(topoinfo, 0, sizeof(struct hdcprp_topo));
			cnt = sscanf(buf, "%x %x %x %x %x %x",
					(int *)&topoinfo->topo.topo22.max_devs_exceeded,
					(int *)&topoinfo->topo.topo22.max_cascade_exceeded,
					(int *)&topoinfo->topo.topo22.v2_0_repeater_down,
					(int *)&topoinfo->topo.topo22.v1_X_device_down,
					(int *)&topoinfo->topo.topo22.device_count,
					(int *)&topoinfo->topo.topo22.depth);
			if (cnt < 0)
				return count;
		}
	}

	return count;
}

static DEVICE_ATTR_RW(hdcp_topo_info);

static ssize_t hdcp22_type_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	int pos = 0;

	if (global_tx_common->hdmi_init == HDMITX20)
		pos += snprintf(buf + pos, PAGE_SIZE, "%d\n", global_tx_common->hdcp22_type);
	return pos;
}

static ssize_t hdcp22_type_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	int type = 0;

	if (global_tx_common->hdmi_init == HDMITX20) {
		if (buf[0] == '1')
			type = 1;
		else
			type = 0;
		global_tx_common->hdcp22_type = type;

		HDMITX_INFO("set hdcp22 content type %d\n", type);
		hdmitx_hw_cntl_ddc(global_tx_hw, DDC_HDCP_SET_TOPO_INFO, type);
	}

	return count;
}

static DEVICE_ATTR_RW(hdcp22_type);

static ssize_t hdcp22_base_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	int pos = 0;

	if (global_tx_common->hdmi_init == HDMITX20)
		pos += snprintf(buf + pos, PAGE_SIZE, "0x%x\n", get_hdcp22_base());
	return pos;
}

static DEVICE_ATTR_RO(hdcp22_base);

static ssize_t hdcp22_stopauth_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int pos = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\r\n",
		is_hdcp22_stop_state());
	return pos;
}

static DEVICE_ATTR_RO(hdcp22_stopauth);

/* the workaround is enabled only for special TV
 * it can also be forcely enabled for debug on other TV by
 * echo poll_en1 > /sys/class/amhdmitx/amhdmitx0/debug
 */
static ssize_t reset_tv_hdcp_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	u8 data;

	if (!global_tx_common->en_poll_rx_status) {
		if (!hdmitx_find_vendor_hdcp22_non_std(global_tx_common->EDID_buf))
			return count;
	}
	if (strncmp(buf, "1", 1) == 0) {
		if (global_tx_common->poll_rx_status_mtd == 0) {
			HDMITX_INFO("force poll rx_status to reset TV hdcp\n");
			hdmitx_reset_tv_hdcp();
		} else if (global_tx_common->poll_rx_status_mtd == 1) {
			HDMITX_INFO("force read 1byte hdcp msg\n");
			ddc_read_1byte(HDCP_SLAVE, HDCP2_RD_MSG, &data);
		}
	}
	return count;
}

static DEVICE_ATTR_WO(reset_tv_hdcp);

/*
 * hdcp_repeater attr
 * For hdcp 22, hdcp_tx22 will write to hdcp_repeater_store
 * For hdcp 14, directly get bcaps bit
 */
static ssize_t hdcp_repeater_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	int pos = 0;

	if (global_tx_common->hdcp_mode == 1)
		global_tx_common->hdcp_bcaps_repeater = hdmitx_hw_cntl_ddc(global_tx_hw,
			DDC_HDCP14_GET_BCAPS_RP, 0);

	pos += snprintf(buf + pos, PAGE_SIZE, "%d\n",
			global_tx_common->hdcp_bcaps_repeater);

	return pos;
}

static ssize_t hdcp_repeater_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	if (global_tx_common->hdcp_mode == 2)
		global_tx_common->hdcp_bcaps_repeater = (buf[0] == '1');

	return count;
}

static DEVICE_ATTR_RW(hdcp_repeater);

static ssize_t hdcp_ctrl_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	if (hdmitx_hw_cntl_ddc(global_tx_hw, DDC_HDCP_14_LSTORE, 0) == 0)
		return count;

	/* for repeater */
	if (global_tx_hw->hdcp_repeater_en) {
		HDMITX_DEBUG_HDCP("hdmitx20: %s\n", buf);
		if (strncmp(buf, "rstop", 5) == 0) {
			if (strncmp(buf + 5, "14", 2) == 0)
				hdmitx_hw_cntl_ddc(global_tx_hw,
					DDC_HDCP_OP, HDCP14_OFF);
			if (strncmp(buf + 5, "22", 2) == 0)
				hdmitx_hw_cntl_ddc(global_tx_hw,
					DDC_HDCP_OP, HDCP22_OFF);
			global_tx_common->hdcp_mode = 0;
			hdmitx_hdcp_do_work(global_tx_common);
		}
		return count;
	}
	/* for non repeater */
	if (strncmp(buf, "stop", 4) == 0) {
		HDMITX_DEBUG_HDCP("hdmitx20: %s\n", buf);
		if (strncmp(buf + 4, "14", 2) == 0)
			hdmitx_hw_cntl_ddc(global_tx_hw,
				DDC_HDCP_OP, HDCP14_OFF);
		if (strncmp(buf + 4, "22", 2) == 0)
			hdmitx_hw_cntl_ddc(global_tx_hw,
				DDC_HDCP_OP, HDCP22_OFF);
		global_tx_common->hdcp_mode = 0;
		hdmitx_hdcp_do_work(global_tx_common);
	}

	return count;
}

static ssize_t hdcp_ctrl_show(struct device *dev,
			      struct device_attribute *attr,
			      char *buf)
{
	return 0;
}

static DEVICE_ATTR_RW(hdcp_ctrl);

static ssize_t hdcp22_top_reset_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	mutex_lock(&global_tx_common->hdmimode_mutex);
	/* should not reset hdcp2.2 after hdcp2.2 auth start */
	if (global_tx_common->ready) {
		mutex_unlock(&global_tx_common->hdmimode_mutex);
		return count;
	}
	HDMITX_INFO("reset hdcp2.2 module after exit hdcp2.2 auth\n");
	hdmitx_hw_cntl_misc(global_tx_hw, MISC_HDCP_CLKDIS, 1);
	hdmitx_hw_cntl_ddc(global_tx_hw, DDC_RESET_HDCP, 0);
	mutex_unlock(&global_tx_common->hdmimode_mutex);
	return count;
}

static DEVICE_ATTR_WO(hdcp22_top_reset);

/*For hdcp daemon, dont del.*/
static ssize_t hdmitx_drm_flag_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int pos = 0;
	int flag = 0;

	/* notify hdcp_tx22: use flow of drm */
	if (global_tx_common->hdcp_ctl_lvl > 0)
		flag = 1;
	else
		flag = 0;

	pos += snprintf(buf + pos, PAGE_SIZE, "%d", flag);
	return pos;
}

static DEVICE_ATTR_RO(hdmitx_drm_flag);

static ssize_t dump_debug_reg_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return global_tx_hw->dump_debug_reg(global_tx_hw, buf, PAGE_SIZE);
}

static DEVICE_ATTR_RO(dump_debug_reg);
#endif
/*********************************************************/
int hdmitx_sysfs_common_create(struct device *dev,
		struct hdmitx_common *tx_comm,
		struct hdmitx_hw_common *tx_hw)
{
	int ret = 0;

	global_tx_common = tx_comm;
	global_tx_hw = tx_hw;

	ret = device_create_file(dev, &dev_attr_hdmi_efuse_state);
	ret = device_create_file(dev, &dev_attr_attr);
	ret = device_create_file(dev, &dev_attr_test_attr);
	ret = device_create_file(dev, &dev_attr_hpd_state);
	ret = device_create_file(dev, &dev_attr_frac_rate_policy);

	ret = device_create_file(dev, &dev_attr_rawedid);
	ret = device_create_file(dev, &dev_attr_edid_parsing);
	ret = device_create_file(dev, &dev_attr_edid);
	ret = device_create_file(dev, &dev_attr_disp_cap);
	ret = device_create_file(dev, &dev_attr_preferred_mode);
	ret = device_create_file(dev, &dev_attr_cea_cap);
	ret = device_create_file(dev, &dev_attr_vesa_cap);
	ret = device_create_file(dev, &dev_attr_dc_cap);
	ret = device_create_file(dev, &dev_attr_aud_cap);
	ret = device_create_file(dev, &dev_attr_valid_mode);

	ret = device_create_file(dev, &dev_attr_support_3d);
	ret = device_create_file(dev, &dev_attr_allm_cap);
	ret = device_create_file(dev, &dev_attr_contenttype_cap);
	ret = device_create_file(dev, &dev_attr_hdr_cap);
	ret = device_create_file(dev, &dev_attr_hdr_cap2);
	ret = device_create_file(dev, &dev_attr_dv_cap);
	ret = device_create_file(dev, &dev_attr_dv_cap2);
	ret = device_create_file(dev, &dev_attr_sink_type);
	ret = device_create_file(dev, &dev_attr_hdmirx_info);

	ret = device_create_file(dev, &dev_attr_phy);
	ret = device_create_file(dev, &dev_attr_avmute);

	ret = device_create_file(dev, &dev_attr_contenttype_mode);
	ret = device_create_file(dev, &dev_attr_allm_mode);

	ret = device_create_file(dev, &dev_attr_hdmitx_cur_status);
	ret = device_create_file(dev, &dev_attr_hdmi_repeater_tx);
	ret = device_create_file(dev, &dev_attr_hdmi_used);

	ret = device_create_file(dev, &dev_attr_debug);
	ret = device_create_file(dev, &dev_attr_fake_plug);
	ret = device_create_file(dev, &dev_attr_config_csc_en);
	ret = device_create_file(dev, &dev_attr_soundbar_en);
	ret = device_create_file(dev, &dev_attr_disp_mode);
	ret = device_create_file(dev, &dev_attr_hdmi_hdr_status);
	ret = device_create_file(dev, &dev_attr_sspll);
	ret = device_create_file(dev, &dev_attr_rxsense_policy);
	ret = device_create_file(dev, &dev_attr_rxsense_state);
	ret = device_create_file(dev, &dev_attr_cedst_policy);
	ret = device_create_file(dev, &dev_attr_ready);
	ret = device_create_file(dev, &dev_attr_hdr_priority_mode);
	ret = device_create_file(dev, &dev_attr_hdr_mute_frame);
	ret = device_create_file(dev, &dev_attr_clkmsr);
	ret = device_create_file(dev, &dev_attr_lipsync_cap);
	ret = device_create_file(dev, &dev_attr_vid_mute);
	ret = device_create_file(dev, &dev_attr_is_hdcp_cts_te);
	ret = device_create_file(dev, &dev_attr_hdcp_lstore);
	ret = device_create_file(dev, &dev_attr_hdcp_mode);
	ret = device_create_file(dev, &dev_attr_hdcp_ver);
	ret = device_create_file(dev, &dev_attr_hdmitx_pkt_dump);
	ret = device_create_file(dev, &dev_attr_hdmitx_basic_config);
	ret = device_create_file(dev, &dev_attr_config);
#ifdef CONFIG_AMLOGIC_HDMITX21
	if (global_tx_common->hdmi_init == HDMITX21) {
		ret = device_create_file(dev, &dev_attr_vrr_cap);
		ret = device_create_file(dev, &dev_attr_ll_mode);
		ret = device_create_file(dev, &dev_attr_ll_user_mode);
		ret = device_create_file(dev, &dev_attr_aon_output);
		ret = device_create_file(dev, &dev_attr_def_stream_type);
		ret = device_create_file(dev, &dev_attr_is_passthrough_switch);
		ret = device_create_file(dev, &dev_attr_need_filter_hdcp_off);
		ret = device_create_file(dev, &dev_attr_filter_hdcp_off_period);
		ret = device_create_file(dev, &dev_attr_not_restart_hdcp);
	}
#endif

#ifdef CONFIG_AMLOGIC_HDMITX
	if (global_tx_common->hdmi_init == HDMITX20) {
		ret = device_create_file(dev, &dev_attr_hdcp_clkdis);
		ret = device_create_file(dev, &dev_attr_hdcp_pwr);
		ret = device_create_file(dev, &dev_attr_hdcp_type_policy);
		ret = device_create_file(dev, &dev_attr_hdcp_topo_info);
		ret = device_create_file(dev, &dev_attr_hdcp22_type);
		ret = device_create_file(dev, &dev_attr_hdcp22_base);
		ret = device_create_file(dev, &dev_attr_hdcp22_stopauth);
		ret = device_create_file(dev, &dev_attr_reset_tv_hdcp);
		ret = device_create_file(dev, &dev_attr_hdcp_repeater);
		ret = device_create_file(dev, &dev_attr_hdcp_ctrl);
		ret = device_create_file(dev, &dev_attr_hdcp22_top_reset);
		ret = device_create_file(dev, &dev_attr_hdmitx_drm_flag);
		ret = device_create_file(dev, &dev_attr_dump_debug_reg);
	}
#endif
	return ret;
}

int hdmitx_sysfs_common_destroy(struct device *dev)
{
	device_remove_file(dev, &dev_attr_hdmi_efuse_state);
	device_remove_file(dev, &dev_attr_attr);
	device_remove_file(dev, &dev_attr_test_attr);
	device_remove_file(dev, &dev_attr_hpd_state);
	device_remove_file(dev, &dev_attr_frac_rate_policy);

	device_remove_file(dev, &dev_attr_rawedid);
	device_remove_file(dev, &dev_attr_edid_parsing);
	device_remove_file(dev, &dev_attr_edid);
	device_remove_file(dev, &dev_attr_disp_cap);
	device_remove_file(dev, &dev_attr_preferred_mode);
	device_remove_file(dev, &dev_attr_cea_cap);
	device_remove_file(dev, &dev_attr_vesa_cap);
	device_remove_file(dev, &dev_attr_dc_cap);
	device_remove_file(dev, &dev_attr_aud_cap);
	device_remove_file(dev, &dev_attr_valid_mode);

	device_remove_file(dev, &dev_attr_support_3d);
	device_remove_file(dev, &dev_attr_allm_cap);
	device_remove_file(dev, &dev_attr_contenttype_cap);
	device_remove_file(dev, &dev_attr_hdr_cap);
	device_remove_file(dev, &dev_attr_hdr_cap2);
	device_remove_file(dev, &dev_attr_dv_cap);
	device_remove_file(dev, &dev_attr_dv_cap2);
	device_remove_file(dev, &dev_attr_sink_type);
	device_remove_file(dev, &dev_attr_hdmirx_info);

	device_remove_file(dev, &dev_attr_phy);
	device_remove_file(dev, &dev_attr_avmute);

	device_remove_file(dev, &dev_attr_contenttype_mode);
	device_remove_file(dev, &dev_attr_allm_mode);

	device_remove_file(dev, &dev_attr_hdmitx_cur_status);
	device_remove_file(dev, &dev_attr_hdmi_repeater_tx);
	device_remove_file(dev, &dev_attr_hdmi_used);

	device_remove_file(dev, &dev_attr_debug);
	device_remove_file(dev, &dev_attr_fake_plug);
	device_remove_file(dev, &dev_attr_config_csc_en);
	device_remove_file(dev, &dev_attr_soundbar_en);
	device_remove_file(dev, &dev_attr_disp_mode);
	device_remove_file(dev, &dev_attr_hdmi_hdr_status);
	device_remove_file(dev, &dev_attr_sspll);
	device_remove_file(dev, &dev_attr_rxsense_policy);
	device_remove_file(dev, &dev_attr_rxsense_state);
	device_remove_file(dev, &dev_attr_cedst_policy);
	device_remove_file(dev, &dev_attr_ready);
	device_remove_file(dev, &dev_attr_hdr_priority_mode);
	device_remove_file(dev, &dev_attr_hdr_mute_frame);
	device_remove_file(dev, &dev_attr_clkmsr);
	device_remove_file(dev, &dev_attr_lipsync_cap);
	device_remove_file(dev, &dev_attr_vid_mute);
	device_remove_file(dev, &dev_attr_is_hdcp_cts_te);
	device_remove_file(dev, &dev_attr_hdcp_lstore);
	device_remove_file(dev, &dev_attr_hdcp_mode);
	device_remove_file(dev, &dev_attr_hdcp_ver);
	device_remove_file(dev, &dev_attr_hdmitx_pkt_dump);
	device_remove_file(dev, &dev_attr_hdmitx_basic_config);
	device_remove_file(dev, &dev_attr_config);
#ifdef CONFIG_AMLOGIC_HDMITX21
	if (global_tx_common->hdmi_init == HDMITX21) {
		device_remove_file(dev, &dev_attr_vrr_cap);
		device_remove_file(dev, &dev_attr_ll_mode);
		device_remove_file(dev, &dev_attr_ll_user_mode);
		device_remove_file(dev, &dev_attr_aon_output);
		device_remove_file(dev, &dev_attr_def_stream_type);
		device_remove_file(dev, &dev_attr_is_passthrough_switch);
		device_remove_file(dev, &dev_attr_need_filter_hdcp_off);
		device_remove_file(dev, &dev_attr_filter_hdcp_off_period);
		device_remove_file(dev, &dev_attr_not_restart_hdcp);
	}
#endif

#ifdef CONFIG_AMLOGIC_HDMITX
	if (global_tx_common->hdmi_init == HDMITX20) {
		device_remove_file(dev, &dev_attr_hdcp_clkdis);
		device_remove_file(dev, &dev_attr_hdcp_pwr);
		device_remove_file(dev, &dev_attr_hdcp_type_policy);
		device_remove_file(dev, &dev_attr_hdcp_topo_info);
		device_remove_file(dev, &dev_attr_hdcp22_type);
		device_remove_file(dev, &dev_attr_hdcp22_base);
		device_remove_file(dev, &dev_attr_hdcp22_stopauth);
		device_remove_file(dev, &dev_attr_reset_tv_hdcp);
		device_remove_file(dev, &dev_attr_hdcp_repeater);
		device_remove_file(dev, &dev_attr_hdcp_ctrl);
		device_remove_file(dev, &dev_attr_hdcp22_top_reset);
		device_remove_file(dev, &dev_attr_hdmitx_drm_flag);
		device_remove_file(dev, &dev_attr_dump_debug_reg);
	}
#endif
	global_tx_common = 0;
	global_tx_hw = 0;

	return 0;
}

