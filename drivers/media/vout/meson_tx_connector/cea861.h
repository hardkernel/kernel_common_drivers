/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#ifndef __CEA861_H__
#define __CEA861_H__

#include <linux/types.h>
#include <linux/string.h>

#define INFOFRAME_TYPE_OFFSET 0x80

enum infoframe_type {
	INFOFRAME_TYPE_VENDOR = 0x1 | INFOFRAME_TYPE_OFFSET,
	INFOFRAME_TYPE_AVI = 0x2 | INFOFRAME_TYPE_OFFSET,
	INFOFRAME_TYPE_SPD = 0x3 | INFOFRAME_TYPE_OFFSET,
	INFOFRAME_TYPE_AUDIO = 0x4 | INFOFRAME_TYPE_OFFSET,
	INFOFRAME_TYPE_MPEG_SOURCE = 0x5 | INFOFRAME_TYPE_OFFSET,
	INFOFRAME_TYPE_NTSC_VBI = 0x6 | INFOFRAME_TYPE_OFFSET,
	INFOFRAME_TYPE_DRM = 0x7 | INFOFRAME_TYPE_OFFSET,
};

#define HDMI_IEEE_OUI 0x000c03
#define HDMI_FORUM_IEEE_OUI 0xc45dd8
#define INFOFRAME_HEADER_SIZE  4
#define DP_INFOFRAME_DB_MAX_SZ  32
#define DP_INFOFRAME_TOTAL_SZ (INFOFRAME_HEADER_SIZE + DP_INFOFRAME_DB_MAX_SZ)
#define HDMI_INFOFRAME_DB_MAX_SZ  28
#define HDMI_INFOFRAME_TOTAL_SZ (INFOFRAME_HEADER_SIZE + HDMI_INFOFRAME_DB_MAX_SZ)
#define AVI_INFOFRAME_SIZE     13
#define SPD_INFOFRAME_SIZE     25
#define AUDIO_INFOFRAME_SIZE   10
#define DRM_INFOFRAME_SIZE     26

/*
 * HDMI 1.3a table 5-14 states that the largest InfoFrame_length is 27,
 * not including the packet header or checksum byte. We include the
 * checksum byte in INFOFRAME_HEADER_SIZE, so this should allow
 * INFOFRAME_SIZE(MAX) to be the largest buffer we could ever need
 * for any HDMI infoframe.
 */

#define INFOFRAME_SIZE(type)	\
	(INFOFRAME_HEADER_SIZE + type ## _INFOFRAME_SIZE)

struct any_infoframe {
	enum infoframe_type type;
	unsigned char version;
	unsigned char length;
};

/**
 * enum colorimetry_format - Pixel encoding formats
 *
 * This enum is used to indicate DP VSC SDP Pixel encoding formats or HDMI formats.
 *
 * @COLORIMETRY_FORMAT_RGB: RGB pixel encoding format
 * @COLORIMETRY_FORMAT_YUV444: YCbCr 4:4:4 pixel encoding format
 * @COLORIMETRY_FORMAT_YUV422: YCbCr 4:2:2 pixel encoding format
 * @COLORIMETRY_FORMAT_YUV420: YCbCr 4:2:0 pixel encoding format
 * @COLORIMETRY_FORMAT_Y_ONLY: Y Only pixel encoding format
 * @COLORIMETRY_FORMAT_RAW: RAW pixel encoding format
 */
enum colorimetry_format {
	COLORIMETRY_FORMAT_RGB = 0,
	COLORIMETRY_FORMAT_YUV422 = 1,
	COLORIMETRY_FORMAT_YUV444 = 2,
	COLORIMETRY_FORMAT_YUV420 = 3,
	COLORIMETRY_FORMAT_Y_ONLY = 4,
	COLORIMETRY_FORMAT_RAW = 5,
	COLORIMETRY_FORMAT_MAX,
};

enum scan_mode {
	SCAN_MODE_NONE,
	SCAN_MODE_OVERSCAN,
	SCAN_MODE_UNDERSCAN,
	SCAN_MODE_RESERVED,
};

enum colorimetry {
	COLORIMETRY_NONE,
	COLORIMETRY_ITU_601,
	COLORIMETRY_ITU_709,
	COLORIMETRY_EXTENDED,
};

enum picture_aspect {
	PICTURE_ASPECT_NONE,
	PICTURE_ASPECT_4_3,
	PICTURE_ASPECT_16_9,
	PICTURE_ASPECT_64_27,
	PICTURE_ASPECT_256_135,
	PICTURE_ASPECT_RESERVED,
};

enum active_aspect {
	ACTIVE_ASPECT_16_9_TOP = 2,
	ACTIVE_ASPECT_14_9_TOP = 3,
	ACTIVE_ASPECT_16_9_CENTER = 4,
	ACTIVE_ASPECT_PICTURE = 8,
	ACTIVE_ASPECT_4_3 = 9,
	ACTIVE_ASPECT_16_9 = 10,
	ACTIVE_ASPECT_14_9 = 11,
	ACTIVE_ASPECT_4_3_SP_14_9 = 13,
	ACTIVE_ASPECT_16_9_SP_14_9 = 14,
	ACTIVE_ASPECT_16_9_SP_4_3 = 15,
};

enum extended_colorimetry {
	EXTENDED_COLORIMETRY_XV_YCC_601,
	EXTENDED_COLORIMETRY_XV_YCC_709,
	EXTENDED_COLORIMETRY_S_YCC_601,
	EXTENDED_COLORIMETRY_OPYCC_601,
	EXTENDED_COLORIMETRY_OPRGB,

	/* The following EC values are only defined in CEA-861-F. */
	EXTENDED_COLORIMETRY_BT2020_CONST_LUM,
	EXTENDED_COLORIMETRY_BT2020,
	EXTENDED_COLORIMETRY_RESERVED,
};

enum quantization_range {
	QUANTIZATION_RANGE_DEFAULT,
	QUANTIZATION_RANGE_LIMITED,
	QUANTIZATION_RANGE_FULL,
	QUANTIZATION_RANGE_RESERVED,
};

/* non-uniform picture scaling */
enum nups {
	NUPS_UNKNOWN,
	NUPS_HORIZONTAL,
	NUPS_VERTICAL,
	NUPS_BOTH,
};

enum ycc_quantization_range {
	YCC_QUANTIZATION_RANGE_LIMITED,
	YCC_QUANTIZATION_RANGE_FULL,
};

enum content_type {
	CONTENT_TYPE_GRAPHICS,
	CONTENT_TYPE_PHOTO,
	CONTENT_TYPE_CINEMA,
	CONTENT_TYPE_GAME,
};

enum metadata_type {
	STATIC_METADATA_TYPE1 = 0,
};

enum eotf {
	EOTF_TRADITIONAL_GAMMA_SDR,
	EOTF_TRADITIONAL_GAMMA_HDR,
	EOTF_SMPTE_ST2084,
	EOTF_BT_2100_HLG,
};

struct avi_infoframe {
	enum infoframe_type type;
	unsigned char version;
	unsigned char length;
	bool itc;
	unsigned char pixel_repeat;
	enum colorimetry_format colorspace;
	enum scan_mode scan_mode;
	enum colorimetry colorimetry;
	enum picture_aspect picture_aspect;
	enum active_aspect active_aspect;
	enum extended_colorimetry extended_colorimetry;
	enum quantization_range quantization_range;
	enum nups nups;
	unsigned char video_code;
	enum ycc_quantization_range ycc_quantization_range;
	enum content_type content_type;
	unsigned short top_bar;
	unsigned short bottom_bar;
	unsigned short left_bar;
	unsigned short right_bar;
};

/* DRM Infoframe as per CTA 861.G spec */
struct drm_infoframe {
	enum infoframe_type type;
	unsigned char version;
	unsigned char length;
	enum eotf eotf;
	enum metadata_type metadata_type;
	struct {
		u16 x, y;
	} display_primaries[3];
	struct {
		u16 x, y;
	} white_point;
	u16 max_display_mastering_luminance;
	u16 min_display_mastering_luminance;
	u16 max_cll;
	u16 max_fall;
};

void avi_infoframe_init(struct avi_infoframe *frame);
ssize_t avi_infoframe_pack(struct avi_infoframe *frame, void *buffer, size_t size);
ssize_t avi_infoframe_pack_only(const struct avi_infoframe *frame, void *buffer, size_t size);
int avi_infoframe_check(struct avi_infoframe *frame);
int drm_infoframe_init(struct drm_infoframe *frame);
ssize_t drm_infoframe_pack(struct drm_infoframe *frame, void *buffer, size_t size);
ssize_t drm_infoframe_pack_only(const struct drm_infoframe *frame, void *buffer, size_t size);
int drm_infoframe_check(struct drm_infoframe *frame);
int drm_infoframe_unpack_only(struct drm_infoframe *frame, const void *buffer, size_t size);

enum spd_sdi {
	SPD_SDI_UNKNOWN,
	SPD_SDI_DSTB,
	SPD_SDI_DVDP,
	SPD_SDI_DVHS,
	SPD_SDI_HDDVR,
	SPD_SDI_DVC,
	SPD_SDI_DSC,
	SPD_SDI_VCD,
	SPD_SDI_GAME,
	SPD_SDI_PC,
	SPD_SDI_BD,
	SPD_SDI_SACD,
	SPD_SDI_HDDVD,
	SPD_SDI_PMP,
};

struct spd_infoframe {
	enum infoframe_type type;
	unsigned char version;
	unsigned char length;
	char vendor[8];
	char product[16];
	enum spd_sdi sdi;
};

int spd_infoframe_init(struct spd_infoframe *frame, const char *vendor, const char *product);
ssize_t spd_infoframe_pack(struct spd_infoframe *frame, void *buffer, size_t size);
ssize_t spd_infoframe_pack_only(const struct spd_infoframe *frame, void *buffer, size_t size);
int spd_infoframe_check(struct spd_infoframe *frame);

enum audio_coding_type {
	AUDIO_CODING_TYPE_STREAM,
	AUDIO_CODING_TYPE_PCM,
	AUDIO_CODING_TYPE_AC3,
	AUDIO_CODING_TYPE_MPEG1,
	AUDIO_CODING_TYPE_MP3,
	AUDIO_CODING_TYPE_MPEG2,
	AUDIO_CODING_TYPE_AAC_LC,
	AUDIO_CODING_TYPE_DTS,
	AUDIO_CODING_TYPE_ATRAC,
	AUDIO_CODING_TYPE_DSD,
	AUDIO_CODING_TYPE_EAC3,
	AUDIO_CODING_TYPE_DTS_HD,
	AUDIO_CODING_TYPE_MLP,
	AUDIO_CODING_TYPE_DST,
	AUDIO_CODING_TYPE_WMA_PRO,
	AUDIO_CODING_TYPE_CXT,
};

enum audio_sample_size {
	AUDIO_SAMPLE_SIZE_STREAM,
	AUDIO_SAMPLE_SIZE_16,
	AUDIO_SAMPLE_SIZE_20,
	AUDIO_SAMPLE_SIZE_24,
};

enum audio_sample_frequency {
	AUDIO_SAMPLE_FREQUENCY_STREAM,
	AUDIO_SAMPLE_FREQUENCY_32000,
	AUDIO_SAMPLE_FREQUENCY_44100,
	AUDIO_SAMPLE_FREQUENCY_48000,
	AUDIO_SAMPLE_FREQUENCY_88200,
	AUDIO_SAMPLE_FREQUENCY_96000,
	AUDIO_SAMPLE_FREQUENCY_176400,
	AUDIO_SAMPLE_FREQUENCY_192000,
};

enum audio_coding_type_ext {
	/* Refer to Audio Coding Type (CT) field in Data Byte 1 */
	AUDIO_CODING_TYPE_EXT_CT,

	/*
	 * The next three CXT values are defined in CEA-861-E only.
	 * They do not exist in older versions, and in CEA-861-F they are
	 * defined as 'Not in use'.
	 */
	AUDIO_CODING_TYPE_EXT_HE_AAC,
	AUDIO_CODING_TYPE_EXT_HE_AAC_V2,
	AUDIO_CODING_TYPE_EXT_MPEG_SURROUND,

	/* The following CXT values are only defined in CEA-861-F. */
	AUDIO_CODING_TYPE_EXT_MPEG4_HE_AAC,
	AUDIO_CODING_TYPE_EXT_MPEG4_HE_AAC_V2,
	AUDIO_CODING_TYPE_EXT_MPEG4_AAC_LC,
	AUDIO_CODING_TYPE_EXT_DRA,
	AUDIO_CODING_TYPE_EXT_MPEG4_HE_AAC_SURROUND,
	AUDIO_CODING_TYPE_EXT_MPEG4_AAC_LC_SURROUND = 10,
};

struct audio_infoframe {
	enum infoframe_type type;
	unsigned char version;
	unsigned char length;
	unsigned char channels;
	enum audio_coding_type coding_type;
	enum audio_sample_size sample_size;
	enum audio_sample_frequency sample_frequency;
	enum audio_coding_type_ext coding_type_ext;
	unsigned char channel_allocation;
	unsigned char level_shift_value;
	bool downmix_inhibit;

};

int audio_infoframe_init(struct audio_infoframe *frame);
ssize_t audio_infoframe_pack(struct audio_infoframe *frame, void *buffer, size_t size);
ssize_t audio_infoframe_pack_only(const struct audio_infoframe *frame, void *buffer, size_t size);
int audio_infoframe_check(const struct audio_infoframe *frame);
void audio_infoframe_pack_payload(const struct audio_infoframe *frame, u8 *buffer);

enum video_3d_structure {
	VIDEO_3D_STRUCTURE_INVALID = -1,
	VIDEO_3D_STRUCTURE_FRAME_PACKING = 0,
	VIDEO_3D_STRUCTURE_FIELD_ALTERNATIVE,
	VIDEO_3D_STRUCTURE_LINE_ALTERNATIVE,
	VIDEO_3D_STRUCTURE_SIDE_BY_SIDE_FULL,
	VIDEO_3D_STRUCTURE_L_DEPTH,
	VIDEO_3D_STRUCTURE_L_DEPTH_GFX_GFX_DEPTH,
	VIDEO_3D_STRUCTURE_TOP_AND_BOTTOM,
	VIDEO_3D_STRUCTURE_SIDE_BY_SIDE_HALF = 8,
};

struct vendor_infoframe {
	enum infoframe_type type;
	unsigned char version;
	unsigned char length;
	unsigned int oui;
	u8 vic;
	enum video_3d_structure s3d_struct;
	unsigned int s3d_ext_data;
};

/* HDR Metadata as per 861.G spec */
struct _hdr_static_metadata {
	__u8 eotf;
	__u8 metadata_type;
	__u16 max_cll;
	__u16 max_fall;
	__u16 min_cll;
};

/**
 * struct _hdr_sink_metadata - HDR sink metadata
 *
 * Metadata Information read from Sink's EDID
 */
struct _hdr_sink_metadata {
	/**
	 * @metadata_type: Static_Metadata_Descriptor_ID.
	 */
	__u32 metadata_type;
	/**
	 * @type1: HDR Metadata Infoframe.
	 */
	union {
		struct _hdr_static_metadata type1;
	};
};

int vendor_infoframe_init(struct vendor_infoframe *frame);
ssize_t vendor_infoframe_pack(struct vendor_infoframe *frame, void *buffer, size_t size);
ssize_t vendor_infoframe_pack_only(const struct vendor_infoframe *frame,
				   void *buffer, size_t size);
int vendor_infoframe_check(struct vendor_infoframe *frame);

union vendor_any_infoframe {
	struct {
		enum infoframe_type type;
		unsigned char version;
		unsigned char length;
		unsigned int oui;
	} any;
	struct vendor_infoframe info;
};

/**
 * union infoframe - overall union of all abstract infoframe representations
 * @any: generic infoframe
 * @avi: avi infoframe
 * @spd: spd infoframe
 * @vendor: union of all vendor infoframes
 * @audio: audio infoframe
 * @drm: Dynamic Range and Mastering infoframe
 *
 * This is used by the generic pack function. This works since all infoframes
 * have the same header which also indicates which type of infoframe should be
 * packed.
 */
union infoframe {
	struct any_infoframe any;
	struct avi_infoframe avi;
	struct spd_infoframe spd;
	union vendor_any_infoframe vendor;
	struct audio_infoframe audio;
	struct drm_infoframe drm;
};

ssize_t infoframe_pack(union infoframe *frame, void *buffer, size_t size);
ssize_t infoframe_pack_only(const union infoframe *frame, void *buffer, size_t size);
int infoframe_check(union infoframe *frame);
int infoframe_unpack(union infoframe *frame, const void *buffer, size_t size);

#endif /* __CEA861_H__ */
