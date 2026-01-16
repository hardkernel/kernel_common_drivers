// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef CONFIG_AMLOGIC_ZAPPER_CUT

#include <linux/module.h>

#include "vpp_common.h"
#include "vpp_data.h"
#include "vpp_vf_proc.h"
#include "vpp_modules_inc.h"

#define SIGNAL_FORMAT(type) ((type >> 26) & 0x7)
#define SIGNAL_RANGE(type) ((type >> 25) & 0x1)
#define SIGNAL_COLOR_PRIMARIES(type) ((type >> 16) & 0xff)
#define SIGNAL_TRANSFER_CHARACTERISTIC(type) ((type >> 8) & 0xff)

struct _overscan_data_s {
	unsigned int load_flag;
	unsigned int afd_enable;
	unsigned int screen_mode;
	enum vpp_overscan_input_e source;
	enum vpp_overscan_timing_e timing;
	unsigned int hs;
	unsigned int he;
	unsigned int vs;
	unsigned int ve;
};

static unsigned int cur_signal_type[EN_VD_PATH_MAX] = {
	0xffffffff,
	0xffffffff,
	0xffffffff
};

static struct vpp_hdr_metadata_s hdr_metadata;
static struct vframe_master_display_colour_s cur_master_display_color[EN_VD_PATH_MAX];

static bool pc_mode_change;
static bool hist_sel;
static bool video_status[EN_VD_PATH_MAX] = {0};
static bool video_layer_wait_on[EN_VD_PATH_MAX] = {0};
static int pc_mode_status;
static int fix_range_mode;
static int skip_hdr_mode;
static enum vpp_pw_state_e pre_pw_state;
static enum vpp_hdr_type_e cur_hdr_type;
static enum vpp_color_primary_e cur_color_primary;

static int overscan_status;
static int overscan_reset;
static int overscan_update_flag;
static int overscan_atv_source;
static struct _overscan_data_s overscan_data[EN_TIMING_MAX];

/*Internal functions*/
static struct vinfo_s *_get_vinfo(enum vpp_vf_top_e vpp_top)
{
	struct vinfo_s *vinfo = NULL;

	if (vpp_top == EN_VF_TOP1)
		vinfo = get_current_vinfo2();
#ifdef CONFIG_AMLOGIC_VOUT3_SERVE
	else if (vpp_top == EN_VF_TOP2)
		vinfo = get_current_vinfo3();
#endif
	else
		vinfo = get_current_vinfo();

	return vinfo;
}

static int _get_vinfo_lcd_support(void)
{
	struct vinfo_s *vinfo = get_current_vinfo();

	if (vinfo->mode == VMODE_LCD ||
		vinfo->mode == VMODE_DUMMY_ENCP)
		return 1;
	else
		return 0;
}

static int _is_vinfo_available(const struct vinfo_s *vinfo)
{
	int ret;

	if (!vinfo || !vinfo->name)
		ret = 0;
	else
		ret = strcmp(vinfo->name, "invalid") &&
			strcmp(vinfo->name, "null") &&
			strcmp(vinfo->name, "576cvbs") &&
			strcmp(vinfo->name, "470cvbs");

	return ret;
}

static bool _is_video_layer_on(enum vpp_vd_path_e vd_path)
{
	bool video_on = false;

	/*
	 *if (vd_path == EN_VD1_PATH)
	 *	video_on = get_video_enabled();
	 *else if (vd_path == EN_VD2_PATH)
	 *	video_on = get_videopip_enabled();
	 *else if (vd_path == EN_VD3_PATH)
	 *	video_on = get_videopip2_enabled();
	 *else
	 *	video_on = false;

	 *if (video_on)
	 *	video_layer_wait_on[vd_path] = false;
	 */

	return video_on || video_layer_wait_on[vd_path];
}

static int _is_video_turn_on(bool *vd_on, enum vpp_vd_path_e vd_path)
{
	int ret = 0;
	bool tmp = _is_video_layer_on(vd_path);

	if (!vd_on)
		return ret;

	if (vd_on[vd_path] != tmp) {
		vd_on[vd_path] = tmp;
		ret = vd_on[vd_path] ? 1 : -1;
	}

	return ret;
}

static unsigned int _update_signal_type(struct vframe_s *vf)
{
	unsigned int signal_type = 0;
	unsigned int present_flag = 1;                  /*video available*/
	unsigned int video_format = 5;                  /*unspecified*/
	unsigned int range = 0;                         /*limited*/
	unsigned int color_description_present_flag = 1;/*color available*/
	unsigned int color_primaries = 1;               /*bt709*/
	unsigned int transfer_characteristic = 1;       /*bt709*/
	unsigned int matrix_coefficient = 1;            /*bt709*/
	unsigned int src_height = 0;

	if (!vf)
		return 0;

	if (vf->signal_type & (1 << 29)) {
		signal_type = vf->signal_type;
	} else {
		if (vf->source_type == VFRAME_SOURCE_TYPE_TUNER ||
			vf->source_type == VFRAME_SOURCE_TYPE_CVBS ||
			vf->source_type == VFRAME_SOURCE_TYPE_COMP ||
			vf->source_type == VFRAME_SOURCE_TYPE_HDMI) {
			if (fix_range_mode == 0)
				range = vf->signal_type & (1 << 25);
			else
				range = 1;/*full*/
		} else {/*for multimedia*/
			if (vf->type & VIDTYPE_COMPRESS)
				src_height = vf->compHeight;
			else
				src_height = vf->height;

			if (src_height < 720) {
				/*SD default 601 limited*/
				color_primaries = 3;
				transfer_characteristic = 3;
				matrix_coefficient = 3;
			}
		}

		signal_type = (present_flag << 29) |
			(video_format << 26) |
			(range << 25) |
			(color_description_present_flag << 24) |
			(color_primaries << 16) |
			(transfer_characteristic << 8) |
			matrix_coefficient;
	}

	if (skip_hdr_mode > 0) {
		if ((signal_type & 0xff) != 1 &&
			(signal_type & 0xff) != 3) {
			signal_type &= 0xffffff00;
			signal_type |= 0x00000001;
		}

		if ((signal_type & 0xff00) >> 8 != 1 &&
			(signal_type & 0xff00) >> 8 != 3) {
			signal_type &= 0xffff00ff;
			signal_type |= 0x00000100;
		}

		if ((signal_type & 0xff0000) >> 16 != 1 &&
			(signal_type & 0xff0000) >> 16 != 3) {
			signal_type &= 0xff00ffff;
			signal_type |= 0x00010000;
		}
	}

	return signal_type;
}

static void _update_hdr_type(unsigned int signal_type)
{
	unsigned int cp = SIGNAL_COLOR_PRIMARIES(signal_type);
	unsigned int tc = SIGNAL_TRANSFER_CHARACTERISTIC(signal_type);

	/*hdr type*/
	if ((tc == 14 || tc == 18) && cp == 9)
		cur_hdr_type = EN_TYPE_HLG;
	else if (tc == 0x30 && cp == 9)
		cur_hdr_type = EN_TYPE_HDR10PLUS;
	else if (tc == 16)
		cur_hdr_type = EN_TYPE_HDR10;
	else
		cur_hdr_type = EN_TYPE_SDR;

	/*color primary*/
	if (cp == 1)
		cur_color_primary = EN_COLOR_PRI_BT709;
	else if (cp == 3)
		cur_color_primary = EN_COLOR_PRI_BT601;
	else if (cp == 9)
		cur_color_primary = EN_COLOR_PRI_BT2020;
	else
		cur_color_primary = EN_COLOR_PRI_NULL;
}

static void _update_hdr_metadata(struct vframe_s *vf)
{
	unsigned int cp = SIGNAL_COLOR_PRIMARIES(vf->signal_type);
	struct vframe_master_display_colour_s *tmp;

	if (cp == 9) {
		tmp = &vf->prop.master_display_colour;
		if (tmp->present_flag) {
			memcpy(hdr_metadata.primaries,
				tmp->primaries, sizeof(u32) * 6);
			memcpy(hdr_metadata.white_point,
				tmp->white_point, sizeof(u32) * 2);
			hdr_metadata.luminance[0] = tmp->luminance[0];
			hdr_metadata.luminance[1] = tmp->luminance[1];
		} else {
			memset(hdr_metadata.primaries, 0, sizeof(hdr_metadata.primaries));
		}
	} else {
		memset(hdr_metadata.primaries, 0, sizeof(hdr_metadata.primaries));
	}
}

int _get_primaries_type(struct vframe_master_display_colour_s *mdc)
{
	if (!mdc->present_flag)
		return 0;

	if (mdc->primaries[0][1] > mdc->primaries[1][1] &&
		mdc->primaries[0][1] > mdc->primaries[2][1] &&
		mdc->primaries[2][0] > mdc->primaries[0][0] &&
		mdc->primaries[2][0] > mdc->primaries[1][0]) {
		return 2; /*reasonable g,b,r*/
	} else if (mdc->primaries[0][0] > mdc->primaries[1][0] &&
		mdc->primaries[0][0] > mdc->primaries[2][0] &&
		mdc->primaries[1][1] > mdc->primaries[0][1] &&
		mdc->primaries[1][1] > mdc->primaries[2][1]) {
		return 1; /*reasonable r,g,b*/
	}

	return 0; /*source not usable, use standard bt2020*/
}

enum vpp_csc_type_e _get_csc_type(enum vpp_vd_path_e vd_path)
{
	unsigned int type = 0;
	unsigned int range = 0;
	unsigned int color_primaries = 0;
	unsigned int transfer_characteristic = 0;
	enum vpp_csc_type_e csc_type = EN_CSC_MATRIX_NULL;

	if (vd_path == EN_VD_PATH_MAX)
		return csc_type;

	type = cur_signal_type[vd_path];
	range = SIGNAL_RANGE(type);
	color_primaries = SIGNAL_COLOR_PRIMARIES(type);
	transfer_characteristic = SIGNAL_TRANSFER_CHARACTERISTIC(type);

	if (color_primaries == 1 && transfer_characteristic < 14) {
		if (range == 0)
			csc_type = EN_CSC_MATRIX_YUV709_RGB;
		else
			csc_type = EN_CSC_MATRIX_YUV709F_RGB;
	} else if ((color_primaries == 3) && (transfer_characteristic < 14)) {
		if (range == 0)
			csc_type = EN_CSC_MATRIX_YUV601_RGB;
		else
			csc_type = EN_CSC_MATRIX_YUV601F_RGB;
	} else if ((color_primaries == 9) || (transfer_characteristic >= 14)) {
		if (transfer_characteristic == 16) { /*smpte st-2084*/
			csc_type = EN_CSC_MATRIX_BT2020YUV_BT2020RGB;
		} else if (transfer_characteristic == 14 ||
			transfer_characteristic == 18) { /*14: bt2020-10, 18: bt2020-12*/
			csc_type = EN_CSC_MATRIX_BT2020YUV_BT2020RGB;
		} else if (transfer_characteristic == 15) { /*bt2020-12*/
			if (range == 0)
				csc_type = EN_CSC_MATRIX_YUV709_RGB;
			else
				csc_type = EN_CSC_MATRIX_YUV709F_RGB;
		} else if (transfer_characteristic == 48) {
			if (color_primaries == 9)
				csc_type = EN_CSC_MATRIX_BT2020YUV_BT2020RGB_DYNAMIC;
			else
				csc_type = EN_CSC_MATRIX_BT2020YUV_BT2020RGB;
		} else { /*unknown transfer characteristic*/
			if (range == 0)
				csc_type = EN_CSC_MATRIX_YUV709_RGB;
			else
				csc_type = EN_CSC_MATRIX_YUV709F_RGB;
		}
	} else {
		if (range == 0)
			csc_type = EN_CSC_MATRIX_YUV601_RGB;
		else
			csc_type = EN_CSC_MATRIX_YUV601F_RGB;
	}

	pr_vpp(PR_DEBUG,
		"csc_type/cp/tc/range/type = 0x%x/%x/%x/%x/%x\n",
			csc_type, color_primaries,
			transfer_characteristic, range, type);

	return csc_type;
}

enum vpp_csc_type_e _get_csc_type_by_timing(struct vframe_s *vf,
	struct vinfo_s *vinfo)
{
	enum vpp_csc_type_e csc_type = EN_CSC_MATRIX_NULL;
	unsigned int width, height;

	if (!_get_vinfo_lcd_support()) {
		width = (vf->type & VIDTYPE_COMPRESS) ?
			vf->compWidth : vf->width;
		height = (vf->type & VIDTYPE_COMPRESS) ?
			vf->compHeight : vf->height;

		if (height < 720 && vinfo->height >= 720)
			csc_type = EN_CSC_MATRIX_YUV601L_YUV709L;
		else if (height >= 720 && vinfo->height < 720)
			csc_type = EN_CSC_MATRIX_YUV709L_YUV601L;
		else
			csc_type = EN_CSC_MATRIX_NULL;
	}

	return csc_type;
}

/*t3 for power consumption, disable clock
 *modules only before post blend can be disable
 *because after postblend it used for color temperature
 *or color correction
 */
static void _enhance_clock_ctrl(int val)
{
	vpp_module_ve_set_clock_ctrl(EN_CLK_BLUE_STRETCH,
		val, EN_MODE_RDMA);
	vpp_module_ve_set_clock_ctrl(EN_CLK_CM,
		(val | (val << 2)), EN_MODE_RDMA);
	vpp_module_ve_set_clock_ctrl(EN_CLK_CCORING,
		val, EN_MODE_RDMA);
	vpp_module_ve_set_clock_ctrl(EN_CLK_BLKEXT,
		val, EN_MODE_RDMA);
}

static void _set_enhance_clock(struct vframe_s *vf,
	struct vframe_s *rpt_vf)
{
	/*cm/blue stretch/black extension/chroma coring*/
	/*other modules(sr0/sr1/dnlp/lc disable in amvideo)*/
	/*en = 1: disable clock, 0: enable clock*/
	if (vf || rpt_vf) {
		if (pre_pw_state != EN_PW_ON) {
			_enhance_clock_ctrl(EN_PW_ON);
			pre_pw_state = EN_PW_ON;
		}
	} else {
		if (pre_pw_state != EN_PW_OFF) {
			_enhance_clock_ctrl(EN_PW_ON);
			pre_pw_state = EN_PW_OFF;
		}
	}
}

static void _pc_mode_proc(int mode)
{
}

static void _overscan_fresh(struct vframe_s *vf)
{
	unsigned int height = 0;
	unsigned int cur_overscan_timing = 0;
#ifdef CONFIG_AMLOGIC_MEDIA_TVIN
	unsigned int cur_fmt;
	unsigned int offset = EN_TIMING_UHD + 1;/*av&atv*/
#endif

	if (!overscan_status || !vf)
		return;

	if (overscan_data[0].load_flag) {
		height = (vf->type & VIDTYPE_COMPRESS) ?
			vf->compHeight : vf->height;

		if (height <= 480)
			cur_overscan_timing = EN_TIMING_SD_480;
		else if (height <= 576)
			cur_overscan_timing = EN_TIMING_SD_576;
		else if (height <= 720)
			cur_overscan_timing = EN_TIMING_HD;
		else if (height <= 1088)
			cur_overscan_timing = EN_TIMING_FHD;
		else
			cur_overscan_timing = EN_TIMING_UHD;

		vf->pic_mode.AFD_enable =
			overscan_data[cur_overscan_timing].afd_enable;

		/*local play screen mode set by decoder*/
		if (!(vf->pic_mode.screen_mode == VIDEO_WIDEOPTION_CUSTOM &&
			vf->pic_mode.AFD_enable)) {
			if (overscan_data[0].source == EN_INPUT_MPEG)
				vf->pic_mode.screen_mode = 0xff;
			else
				vf->pic_mode.screen_mode =
					overscan_data[cur_overscan_timing].screen_mode;
		}

		if (vf->pic_mode.provider == PIC_MODE_PROVIDER_WSS &&
			vf->pic_mode.AFD_enable) {
			vf->pic_mode.hs += overscan_data[cur_overscan_timing].hs;
			vf->pic_mode.he += overscan_data[cur_overscan_timing].he;
			vf->pic_mode.vs += overscan_data[cur_overscan_timing].vs;
			vf->pic_mode.ve += overscan_data[cur_overscan_timing].ve;
		} else {
			vf->pic_mode.hs = overscan_data[cur_overscan_timing].hs;
			vf->pic_mode.he = overscan_data[cur_overscan_timing].he;
			vf->pic_mode.vs = overscan_data[cur_overscan_timing].vs;
			vf->pic_mode.ve = overscan_data[cur_overscan_timing].ve;
		}

		vf->ratio_control |= DISP_RATIO_ADAPTED_PICMODE;
	}

#ifdef CONFIG_AMLOGIC_MEDIA_TVIN
	if (overscan_data[offset].load_flag) {
		cur_fmt = vf->sig_fmt;
		if (cur_fmt == TVIN_SIG_FMT_CVBS_NTSC_M)
			cur_overscan_timing = EN_TIMING_NTST_M;
		else if (cur_fmt == TVIN_SIG_FMT_CVBS_NTSC_443)
			cur_overscan_timing = EN_TIMING_NTST_443;
		else if (cur_fmt == TVIN_SIG_FMT_CVBS_PAL_I)
			cur_overscan_timing = EN_TIMING_PAL_I;
		else if (cur_fmt == TVIN_SIG_FMT_CVBS_PAL_M)
			cur_overscan_timing = EN_TIMING_PAL_M;
		else if (cur_fmt == TVIN_SIG_FMT_CVBS_PAL_60)
			cur_overscan_timing = EN_TIMING_PAL_60;
		else if (cur_fmt == TVIN_SIG_FMT_CVBS_PAL_CN)
			cur_overscan_timing = EN_TIMING_PAL_CN;
		else if (cur_fmt == TVIN_SIG_FMT_CVBS_SECAM)
			cur_overscan_timing = EN_TIMING_SECAM;
		else if (cur_fmt == TVIN_SIG_FMT_CVBS_NTSC_50)
			cur_overscan_timing = EN_TIMING_NTSC_50;
		else
			return;

		vf->pic_mode.AFD_enable =
			overscan_data[cur_overscan_timing].afd_enable;

		if (!(vf->pic_mode.screen_mode == VIDEO_WIDEOPTION_CUSTOM &&
			vf->pic_mode.AFD_enable))
			vf->pic_mode.screen_mode =
				overscan_data[cur_overscan_timing].screen_mode;

		if (vf->pic_mode.provider == PIC_MODE_PROVIDER_WSS &&
			vf->pic_mode.AFD_enable) {
			vf->pic_mode.hs += overscan_data[cur_overscan_timing].hs;
			vf->pic_mode.he += overscan_data[cur_overscan_timing].he;
			vf->pic_mode.vs += overscan_data[cur_overscan_timing].vs;
			vf->pic_mode.ve += overscan_data[cur_overscan_timing].ve;
		} else {
			vf->pic_mode.hs = overscan_data[cur_overscan_timing].hs;
			vf->pic_mode.he = overscan_data[cur_overscan_timing].he;
			vf->pic_mode.vs = overscan_data[cur_overscan_timing].vs;
			vf->pic_mode.ve = overscan_data[cur_overscan_timing].ve;
		}

		vf->ratio_control |= DISP_RATIO_ADAPTED_PICMODE;
	}
#endif

	if (overscan_reset) {
		vf->pic_mode.AFD_enable = 0;
		vf->pic_mode.screen_mode = 0;
		vf->pic_mode.hs = 0;
		vf->pic_mode.he = 0;
		vf->pic_mode.vs = 0;
		vf->pic_mode.ve = 0;
		vf->ratio_control &= ~DISP_RATIO_ADAPTED_PICMODE;
		overscan_reset = 0;
	}
}

static void _overscan_reset(void)
{
	unsigned int offset = EN_TIMING_UHD + 1; /*av&atv*/
	enum vpp_overscan_input_e src;

	src = overscan_data[0].source;

	if (!overscan_status)
		return;

	if (src != EN_INPUT_DTV && src != EN_INPUT_MPEG) {
		overscan_data[0].load_flag = 0;
	} else {
		if (!overscan_atv_source)
			overscan_data[offset].load_flag = 0;
	}
}

static void _overscan_proc(struct vframe_s *vf,
	struct vframe_s *toggle_vf, int flags,
	enum vpp_vd_path_e vd_path)
{
	if (vd_path != EN_VD1_PATH)
		return;

	if (flags & 0x2) {
		if (toggle_vf)
			_overscan_fresh(toggle_vf);
		else if (vf)
			_overscan_fresh(vf);
	} else {
		if (!toggle_vf && !vf) {
			_overscan_reset();
		} else {
			if (vf)
				_overscan_fresh(vf);
			else
				_overscan_reset();
		}
	}
}

static void _overscan_dump_data(void)
{
	int i = 0;

	pr_info("\n*****dump overscan_data after parsing*****\n");
	pr_info("i, hs, he, vs, ve, screen_mode, afd, flag\n");

	for (i = 0; i < EN_TIMING_MAX; i++)
		pr_info("%d, %d, %d, %d, %d, %d, %d, %d\n",
			i, overscan_data[i].hs, overscan_data[i].he,
			overscan_data[i].vs, overscan_data[i].ve,
			overscan_data[i].screen_mode,
			overscan_data[i].afd_enable,
			overscan_data[i].load_flag);
}

/*External functions*/
int vpp_vf_init(struct vpp_dev_s *dev)
{
	enum vpp_chip_type_e chip_id;
	int i = 0;

	chip_id = dev->m_data->chip_id;

	cur_hdr_type = EN_TYPE_SDR;
	fix_range_mode = 0;
	hist_sel = 1; /*1: vpp, 0: vdin*/
	pc_mode_change = false;
	pc_mode_status = 0xff;
	pre_pw_state = EN_PW_MAX;

	for (i = 0; i < EN_VD_PATH_MAX; i++)
		memset(&cur_master_display_color[i], 0,
			sizeof(struct vframe_master_display_colour_s));

	return 0;
}

void vpp_vf_set_pc_mode(int val)
{
	if (val != pc_mode_status) {
		pc_mode_status = val;
		pc_mode_change = true;
	}
}

void vpp_vf_set_overscan_mode(int val)
{
	/*0: disable, 1: enable*/
	overscan_status = val;
}

void vpp_vf_set_overscan_reset(int val)
{
	overscan_reset = val;
}

void vpp_vf_set_overscan_table(unsigned int length,
	struct vpp_overscan_table_s *load_table)
{
	unsigned int i;
	unsigned int offset = EN_TIMING_UHD + 1;

	if (!load_table)
		return;

	memset(overscan_data, 0, sizeof(overscan_data));

	for (i = 0; i < length; i++) {
		overscan_data[i].load_flag =
			(load_table[i].src_timing >> 31) & 0x1;
		overscan_data[i].afd_enable =
			(load_table[i].src_timing >> 30) & 0x1;
		overscan_data[i].screen_mode =
			(load_table[i].src_timing >> 24) & 0x3f;
		overscan_data[i].source =
			(load_table[i].src_timing >> 16) & 0xff;
		overscan_data[i].timing =
			load_table[i].src_timing & 0xffff;
		overscan_data[i].hs =
			load_table[i].value1 & 0xffff;
		overscan_data[i].he =
			(load_table[i].value1 >> 16) & 0xffff;
		overscan_data[i].vs =
			load_table[i].value2 & 0xffff;
		overscan_data[i].ve =
			(load_table[i].value2 >> 16) & 0xffff;
	}

	/*overscan reset for dtv auto afd set.*/
	/*if auto set load_flag = 0 by user, overscan set by dtv afd*/
	if (!overscan_data[0].load_flag &&
		!overscan_data[offset].load_flag)
		overscan_update_flag = 1;

	/*because EN_INPUT_TV is 0, need to add flag to check ATV*/
	if (overscan_data[offset].load_flag == 1 &&
		overscan_data[offset].source == EN_INPUT_TV)
		overscan_atv_source = 1;
	else
		overscan_atv_source = 0;
}

void vpp_vf_get_signal_info(enum vpp_vd_path_e vd_path,
	struct vpp_vf_signal_info_s *info)
{
	unsigned int type = 0;

	if (!info || vd_path == EN_VD_PATH_MAX)
		return;

	type = cur_signal_type[vd_path];
	info->format = SIGNAL_FORMAT(type);
	info->range = SIGNAL_RANGE(type);
	info->color_primaries = SIGNAL_COLOR_PRIMARIES(type);
	info->transfer_characteristic = SIGNAL_TRANSFER_CHARACTERISTIC(type);
}

unsigned int vpp_vf_get_signal_type(enum vpp_vd_path_e vd_path)
{
	if (vd_path == EN_VD_PATH_MAX)
		return 0;

	return cur_signal_type[vd_path];
}

enum vpp_hdr_type_e vpp_vf_get_hdr_type(void)
{
	return cur_hdr_type;
}

enum vpp_color_primary_e vpp_vf_get_color_primary(void)
{
	return cur_color_primary;
}

struct vpp_hdr_metadata_s *vpp_vf_get_hdr_metadata(void)
{
	return &hdr_metadata;
}

enum vpp_csc_type_e vpp_vf_get_csc_type(enum vpp_vd_path_e vd_path)
{
	return _get_csc_type(vd_path);
}

int vpp_vf_get_vinfo_lcd_support(void)
{
	return _get_vinfo_lcd_support();
}

void vpp_vf_dump_data(enum vpp_dump_data_type_e type)
{
	switch (type) {
	case EN_DUMP_DATA_OVERSCAN:
		_overscan_dump_data();
		break;
	default:
		break;
	}
}

void vpp_vf_refresh(struct vframe_s *vf, struct vframe_s *rpt_vf)
{
	int i = 0;
	struct vframe_s *tmp;
	struct vpp_hist_report_s *hist_report;
	struct sr_fmeter_report_s *fm_report;
	int hist_luma_sum = 0;
	unsigned short *hist_data;
	bool do_sat_comp = false;
	int sat_comp_val = 0;

	if (!vf && !rpt_vf)
		return;

	tmp = vf ? vf : rpt_vf;

	/*histogram*/
	vpp_module_meter_hist_on_vs();
	hist_report = vpp_module_meter_get_hist_report();

	/*fmeter*/
	if (vpp_module_sr_get_fmeter_support()) {
		fm_report = vpp_module_sr_get_fmeter_report();

		for (i = 0; i < FMETER_HCNT_MAX; i++) {
			tmp->fmeter0_hcnt[i] = fm_report->hcnt[0][i];
			tmp->fmeter1_hcnt[i] = fm_report->hcnt[1][i];
		}
	}

	/*dnlp*/
	if (hist_sel) {
		hist_luma_sum = hist_report->luma_sum;
		hist_data = &hist_report->gamma[0];
	} else {
		hist_luma_sum = tmp->prop.hist.luma_sum;
		hist_data = &tmp->prop.hist.gamma[0];
	}

	vpp_module_dnlp_on_vs(hist_luma_sum, hist_data);
	vpp_module_dnlp_get_sat_compensation(&do_sat_comp, &sat_comp_val);

	/*if (do_sat_comp) !!!maybe conflict with pq table tuning params*/
		/*vpp_module_cm_set_tuning_param(EN_PARAM_GLB_SAT, &sat_comp_val);*/
}
EXPORT_SYMBOL(vpp_vf_refresh);

void vpp_vf_proc(struct vframe_s *vf,
	struct vframe_s *toggle_vf,
	struct vpp_vf_param_s *vf_param,
	int flags,
	enum vpp_vd_path_e vd_path,
	enum vpp_vf_top_e vpp_top)
{
	int tmp = 0;
	unsigned int signal_type = 0;
	struct vpp_hist_report_s *hist_report = NULL;
	struct lc_vs_param_s lc_vs_param;
	struct sr_vs_param_s sr_vs_param = {0};
	struct data_vs_param_s data_vs_param;
	struct vinfo_s *vinfo = NULL;
	struct vframe_s *vf_using = NULL;
	struct vd_proc_amvecm_info_t *vd_info = NULL;

	vpp_module_go_on_vs();
	vpp_module_lcd_gamma_on_vs();
	vpp_module_pre_gamma_on_vs();
	vpp_module_vadj_on_vs();
	vpp_module_matrix_on_vs();
	vpp_module_lut3d_on_vs();

	if (!vf && !toggle_vf)
		return;

	vf_using = toggle_vf ? toggle_vf : vf;

	signal_type = _update_signal_type(vf_using);
	_update_hdr_type(signal_type);
	_update_hdr_metadata(vf_using);

	vd_info = get_vd_proc_amvecm_info();

	data_vs_param.src_type = 0;
	data_vs_param.vf_signal_change = signal_type;
	data_vs_param.vf_width = vf_using->width;
	data_vs_param.vf_height = vf_using->height;

	vpp_data_on_vs(&data_vs_param);

	if (pc_mode_change)
		_pc_mode_proc(pc_mode_status);

	if (vd_path == EN_VD1_PATH)
		_set_enhance_clock(toggle_vf, vf);

	vpp_module_ve_on_vs();
	vpp_module_cm_on_vs();

	vpp_module_sr_on_vs(&sr_vs_param);

	if (vf_param) {
		lc_vs_param.vf_type = vf_using->type;
		lc_vs_param.vf_signal_type = signal_type;
		lc_vs_param.vf_height = vf_using->height;
		lc_vs_param.vf_width = vf_using->width;
		lc_vs_param.sps_h_en = vf_param->sps_h_en;
		lc_vs_param.sps_v_en = vf_param->sps_v_en;
		lc_vs_param.sps_w_in = vf_param->sps_w_in;
		lc_vs_param.sps_h_in = vf_param->sps_h_in;

		if (vd_info) {
			lc_vs_param.vd_info.vd1_dout_hsize =
				vd_info->vd1_dout_hsize;
			lc_vs_param.vd_info.vd1_dout_vsize =
				vd_info->vd1_dout_vsize;
			lc_vs_param.vd_info.vd1_in_hsize =
				vd_info->vd1_in_hsize;
			lc_vs_param.vd_info.vd1_in_vsize =
				vd_info->vd1_in_vsize;
		}

		hist_report = vpp_module_meter_get_hist_report();
		vpp_module_lc_on_vs(&hist_report->gamma[0], &lc_vs_param);
	}

	/*On-going*/
	vinfo = _get_vinfo(EN_VF_TOP0);
	tmp = _is_vinfo_available(vinfo);
	tmp = _is_video_turn_on(&video_status[0], EN_VD1_PATH);
	_overscan_proc(vf, toggle_vf, flags, EN_VD2_PATH);
}
EXPORT_SYMBOL(vpp_vf_proc);

#endif

