// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/bug.h>
#include "cea861.h"

static u8 infoframe_checksum(const u8 *ptr, size_t size)
{
	u8 csum = 0;
	size_t i;

	/* compute checksum */
	for (i = 0; i < size; i++)
		csum += ptr[i];

	return 256 - csum;
}

static void infoframe_set_checksum(void *buffer, size_t size)
{
	u8 *ptr = buffer;

	ptr[3] = infoframe_checksum(buffer, size);
}

/**
 * avi_infoframe_init() - initialize an AVI infoframe
 * @frame: AVI infoframe
 */
void avi_infoframe_init(struct avi_infoframe *frame)
{
	memset(frame, 0, sizeof(*frame));

	frame->type = INFOFRAME_TYPE_AVI;
	frame->version = 2;
	frame->length = AVI_INFOFRAME_SIZE;
}

static int avi_infoframe_check_only(const struct avi_infoframe *frame)
{
	if (frame->type != INFOFRAME_TYPE_AVI ||
	    frame->version < 2 ||
	    frame->version > 4 ||
	    frame->length != AVI_INFOFRAME_SIZE)
		return -EINVAL;

	if (frame->picture_aspect > PICTURE_ASPECT_16_9)
		return -EINVAL;

	return 0;
}

/**
 * avi_infoframe_check() - check an AVI infoframe
 * @frame: AVI infoframe
 *
 * Validates that the infoframe is consistent and updates derived fields
 * (eg. length) based on other fields.
 *
 * Returns 0 on success or a negative error code on failure.
 */
int avi_infoframe_check(struct avi_infoframe *frame)
{
	return avi_infoframe_check_only(frame);
}

/**
 * avi_infoframe_pack_only() - write AVI infoframe to binary buffer
 * @frame: AVI infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 *
 * Packs the information contained in the @frame structure into a binary
 * representation that can be written into the corresponding controller
 * registers. Also computes the checksum as required by section 5.3.5 of
 * the HDMI 1.4 specification.
 *
 * Returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
ssize_t avi_infoframe_pack_only(const struct avi_infoframe *frame, void *buffer, size_t size)
{
	u8 *ptr = buffer;
	size_t length;
	int ret;

	ret = avi_infoframe_check_only(frame);
	if (ret)
		return ret;

	length = INFOFRAME_HEADER_SIZE + frame->length;

	if (size < length)
		return -ENOSPC;

	memset(buffer, 0, size);

	ptr[0] = frame->type;
	ptr[1] = frame->version;
	ptr[2] = frame->length;
	ptr[3] = 0; /* checksum */

	/* start infoframe payload */
	ptr += INFOFRAME_HEADER_SIZE;

	ptr[0] = ((frame->colorspace & 0x3) << 5) | (frame->scan_mode & 0x3);

	/*
	 * Data byte 1, bit 4 has to be set if we provide the active format
	 * aspect ratio
	 */
	if (frame->active_aspect & 0xf)
		ptr[0] |= BIT(4);

	/* Bit 3 and 2 indicate if we transmit horizontal/vertical bar data */
	if (frame->top_bar || frame->bottom_bar)
		ptr[0] |= BIT(3);

	if (frame->left_bar || frame->right_bar)
		ptr[0] |= BIT(2);

	ptr[1] = ((frame->colorimetry & 0x3) << 6) |
		 ((frame->picture_aspect & 0x3) << 4) |
		 (frame->active_aspect & 0xf);

	ptr[2] = ((frame->extended_colorimetry & 0x7) << 4) |
		 ((frame->quantization_range & 0x3) << 2) |
		 (frame->nups & 0x3);

	if (frame->itc)
		ptr[2] |= BIT(7);

	ptr[3] = frame->video_code;

	ptr[4] = ((frame->ycc_quantization_range & 0x3) << 6) |
		 ((frame->content_type & 0x3) << 4) |
		 (frame->pixel_repeat & 0xf);

	ptr[5] = frame->top_bar & 0xff;
	ptr[6] = (frame->top_bar >> 8) & 0xff;
	ptr[7] = frame->bottom_bar & 0xff;
	ptr[8] = (frame->bottom_bar >> 8) & 0xff;
	ptr[9] = frame->left_bar & 0xff;
	ptr[10] = (frame->left_bar >> 8) & 0xff;
	ptr[11] = frame->right_bar & 0xff;
	ptr[12] = (frame->right_bar >> 8) & 0xff;

	infoframe_set_checksum(buffer, length);

	return length;
}

/**
 * avi_infoframe_pack() - check a AVI infoframe,
 *                             and write it to binary buffer
 * @frame: AVI infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 *
 * Validates that the infoframe is consistent and updates derived fields
 * (eg. length) based on other fields, after which it packs the information
 * contained in the @frame structure into a binary representation that
 * can be written into the corresponding controller registers. This function
 * also computes the checksum as required by section 5.3.5 of the HDMI 1.4
 * specification.
 *
 * Returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
ssize_t avi_infoframe_pack(struct avi_infoframe *frame, void *buffer, size_t size)
{
	int ret;

	ret = avi_infoframe_check(frame);
	if (ret)
		return ret;

	return avi_infoframe_pack_only(frame, buffer, size);
}

/**
 * spd_infoframe_init() - initialize an SPD infoframe
 * @frame: SPD infoframe
 * @vendor: vendor string
 * @product: product string
 *
 * Returns 0 on success or a negative error code on failure.
 */
int spd_infoframe_init(struct spd_infoframe *frame, const char *vendor, const char *product)
{
	size_t len;

	memset(frame, 0, sizeof(*frame));

	frame->type = INFOFRAME_TYPE_SPD;
	frame->version = 1;
	frame->length = SPD_INFOFRAME_SIZE;

	len = strlen(vendor);
	memcpy(frame->vendor, vendor, min(len, sizeof(frame->vendor)));
	len = strlen(product);
	memcpy(frame->product, product, min(len, sizeof(frame->product)));
	frame->sdi = SPD_SDI_DSTB; /* fixed value for ott/stb */

	return 0;
}

static int spd_infoframe_check_only(const struct spd_infoframe *frame)
{
	if (frame->type != INFOFRAME_TYPE_SPD ||
	    frame->version != 1 ||
	    frame->length != SPD_INFOFRAME_SIZE)
		return -EINVAL;

	return 0;
}

/**
 * spd_infoframe_check() - check a SPD infoframe
 * @frame: SPD infoframe
 *
 * Validates that the infoframe is consistent and updates derived fields
 * (eg. length) based on other fields.
 *
 * Returns 0 on success or a negative error code on failure.
 */
int spd_infoframe_check(struct spd_infoframe *frame)
{
	return spd_infoframe_check_only(frame);
}

/**
 * spd_infoframe_pack_only() - write SPD infoframe to binary buffer
 * @frame: SPD infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 *
 * Packs the information contained in the @frame structure into a binary
 * representation that can be written into the corresponding controller
 * registers. Also computes the checksum as required by section 5.3.5 of
 * the HDMI 1.4 specification.
 *
 * Returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
ssize_t spd_infoframe_pack_only(const struct spd_infoframe *frame, void *buffer, size_t size)
{
	u8 *ptr = buffer;
	size_t length;
	int ret;

	ret = spd_infoframe_check_only(frame);
	if (ret)
		return ret;

	length = INFOFRAME_HEADER_SIZE + frame->length;

	if (size < length)
		return -ENOSPC;

	memset(buffer, 0, size);

	ptr[0] = frame->type;
	ptr[1] = frame->version;
	ptr[2] = frame->length;
	ptr[3] = 0; /* checksum */

	/* start infoframe payload */
	ptr += INFOFRAME_HEADER_SIZE;

	memcpy(ptr, frame->vendor, sizeof(frame->vendor));
	memcpy(ptr + 8, frame->product, sizeof(frame->product));

	ptr[24] = frame->sdi;

	infoframe_set_checksum(buffer, length);

	return length;
}

/**
 * spd_infoframe_pack() - check a SPD infoframe,
 *                             and write it to binary buffer
 * @frame: SPD infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 *
 * Validates that the infoframe is consistent and updates derived fields
 * (eg. length) based on other fields, after which it packs the information
 * contained in the @frame structure into a binary representation that
 * can be written into the corresponding controller registers. This function
 * also computes the checksum as required by section 5.3.5 of the HDMI 1.4
 * specification.
 *
 * Returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
ssize_t spd_infoframe_pack(struct spd_infoframe *frame, void *buffer, size_t size)
{
	int ret;

	ret = spd_infoframe_check(frame);
	if (ret)
		return ret;

	return spd_infoframe_pack_only(frame, buffer, size);
}

/**
 * audio_infoframe_init() - initialize an audio infoframe
 * @frame: audio infoframe
 *
 * Returns 0 on success or a negative error code on failure.
 */
int audio_infoframe_init(struct audio_infoframe *frame)
{
	memset(frame, 0, sizeof(*frame));

	frame->type = INFOFRAME_TYPE_AUDIO;
	frame->version = 1;
	frame->length = AUDIO_INFOFRAME_SIZE;

	return 0;
}

static int audio_infoframe_check_only(const struct audio_infoframe *frame)
{
	if (frame->type != INFOFRAME_TYPE_AUDIO ||
	    frame->version != 1 ||
	    frame->length != AUDIO_INFOFRAME_SIZE)
		return -EINVAL;

	return 0;
}

/**
 * audio_infoframe_check() - check an audio infoframe
 * @frame: audio infoframe
 *
 * Validates that the infoframe is consistent and updates derived fields
 * (eg. length) based on other fields.
 *
 * Returns 0 on success or a negative error code on failure.
 */
int audio_infoframe_check(const struct audio_infoframe *frame)
{
	return audio_infoframe_check_only(frame);
}

void audio_infoframe_pack_payload(const struct audio_infoframe *frame, u8 *buffer)
{
	u8 channels;

	if (frame->channels >= 2)
		channels = frame->channels - 1;
	else
		channels = 0;

	buffer[0] = ((frame->coding_type & 0xf) << 4) | (channels & 0x7);
	buffer[1] = ((frame->sample_frequency & 0x7) << 2) |
		 (frame->sample_size & 0x3);
	buffer[2] = frame->coding_type_ext & 0x1f;
	buffer[3] = frame->channel_allocation;
	buffer[4] = (frame->level_shift_value & 0xf) << 3;

	if (frame->downmix_inhibit)
		buffer[4] |= BIT(7);
}

/**
 * audio_infoframe_pack_only() - write audio infoframe to binary buffer
 * @frame: audio infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 *
 * Packs the information contained in the @frame structure into a binary
 * representation that can be written into the corresponding controller
 * registers. Also computes the checksum as required by section 5.3.5 of
 * the HDMI 1.4 specification.
 *
 * Returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
ssize_t audio_infoframe_pack_only(const struct audio_infoframe *frame, void *buffer, size_t size)
{
	unsigned char channels;
	u8 *ptr = buffer;
	size_t length;
	int ret;

	ret = audio_infoframe_check_only(frame);
	if (ret)
		return ret;

	length = INFOFRAME_HEADER_SIZE + frame->length;

	if (size < length)
		return -ENOSPC;

	memset(buffer, 0, size);

	if (frame->channels >= 2)
		channels = frame->channels - 1;
	else
		channels = 0;

	ptr[0] = frame->type;
	ptr[1] = frame->version;
	ptr[2] = frame->length;
	ptr[3] = 0; /* checksum */

	audio_infoframe_pack_payload(frame, ptr + INFOFRAME_HEADER_SIZE);

	infoframe_set_checksum(buffer, length);

	return length;
}

/**
 * audio_infoframe_pack() - check an Audio infoframe,
 *                               and write it to binary buffer
 * @frame: Audio infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 *
 * Validates that the infoframe is consistent and updates derived fields
 * (eg. length) based on other fields, after which it packs the information
 * contained in the @frame structure into a binary representation that
 * can be written into the corresponding controller registers. This function
 * also computes the checksum as required by section 5.3.5 of the HDMI 1.4
 * specification.
 *
 * Returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
ssize_t audio_infoframe_pack(struct audio_infoframe *frame, void *buffer, size_t size)
{
	int ret;

	ret = audio_infoframe_check(frame);
	if (ret)
		return ret;

	return audio_infoframe_pack_only(frame, buffer, size);
}

/**
 * vendor_infoframe_init() - initialize a vendor infoframe
 * @frame: vendor infoframe
 *
 * Returns 0 on success or a negative error code on failure.
 */
int vendor_infoframe_init(struct vendor_infoframe *frame)
{
	memset(frame, 0, sizeof(*frame));

	frame->type = INFOFRAME_TYPE_VENDOR;
	frame->version = 1;

	frame->oui = HDMI_IEEE_OUI;

	/*
	 * 0 is a valid value for s3d_struct, so we use a special "not set"
	 * value
	 */
	frame->s3d_struct = VIDEO_3D_STRUCTURE_INVALID;
	frame->length = INFOFRAME_HEADER_SIZE;

	return 0;
}

static int vendor_infoframe_length(const struct vendor_infoframe *frame)
{
	/* for side by side (half) we also need to provide 3D_Ext_Data */
	if (frame->s3d_struct >= VIDEO_3D_STRUCTURE_SIDE_BY_SIDE_HALF)
		return 6;
	else if (frame->vic != 0 || frame->s3d_struct != VIDEO_3D_STRUCTURE_INVALID)
		return 5;
	else
		return 4;
}

static int vendor_infoframe_check_only(const struct vendor_infoframe *frame)
{
	if (frame->type != INFOFRAME_TYPE_VENDOR ||
	    frame->version != 1 ||
	    frame->oui != HDMI_IEEE_OUI)
		return -EINVAL;

	/* only one of those can be supplied */
	if (frame->vic != 0 && frame->s3d_struct != VIDEO_3D_STRUCTURE_INVALID)
		return -EINVAL;

	if (frame->length != vendor_infoframe_length(frame))
		return -EINVAL;

	return 0;
}

/**
 * vendor_infoframe_check() - check a vendor infoframe
 * @frame: infoframe
 *
 * Validates that the infoframe is consistent and updates derived fields
 * (eg. length) based on other fields.
 *
 * Returns 0 on success or a negative error code on failure.
 */
int vendor_infoframe_check(struct vendor_infoframe *frame)
{
	frame->length = vendor_infoframe_length(frame);

	return vendor_infoframe_check_only(frame);
}

/**
 * vendor_infoframe_pack_only() - write a vendor infoframe to binary buffer
 * @frame: infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 *
 * Packs the information contained in the @frame structure into a binary
 * representation that can be written into the corresponding controller
 * registers. Also computes the checksum as required by section 5.3.5 of
 * the HDMI 1.4 specification.
 *
 * Returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
ssize_t vendor_infoframe_pack_only(const struct vendor_infoframe *frame, void *buffer, size_t size)
{
	u8 *ptr = buffer;
	size_t length;
	int ret;

	ret = vendor_infoframe_check_only(frame);
	if (ret)
		return ret;

	length = INFOFRAME_HEADER_SIZE + frame->length;

	if (size < length)
		return -ENOSPC;

	memset(buffer, 0, size);

	ptr[0] = frame->type;
	ptr[1] = frame->version;
	ptr[2] = frame->length;
	ptr[3] = 0; /* checksum */

	/* HDMI OUI */
	ptr[4] = 0x03;
	ptr[5] = 0x0c;
	ptr[6] = 0x00;

	if (frame->s3d_struct != VIDEO_3D_STRUCTURE_INVALID) {
		ptr[7] = 0x2 << 5;	/* video format */
		ptr[8] = (frame->s3d_struct & 0xf) << 4;
		if (frame->s3d_struct >= VIDEO_3D_STRUCTURE_SIDE_BY_SIDE_HALF)
			ptr[9] = (frame->s3d_ext_data & 0xf) << 4;
	} else if (frame->vic) {
		ptr[7] = 0x1 << 5;	/* video format */
		ptr[8] = frame->vic;
	} else {
		ptr[7] = 0x0 << 5;	/* video format */
	}

	infoframe_set_checksum(buffer, length);

	return length;
}

/**
 * vendor_infoframe_pack() - check a Vendor infoframe,
 *                                and write it to binary buffer
 * @frame: Vendor infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 *
 * Validates that the infoframe is consistent and updates derived fields
 * (eg. length) based on other fields, after which it packs the information
 * contained in the @frame structure into a binary representation that
 * can be written into the corresponding controller registers. This function
 * also computes the checksum as required by section 5.3.5 of the HDMI 1.4
 * specification.
 *
 * Returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
ssize_t vendor_infoframe_pack(struct vendor_infoframe *frame, void *buffer, size_t size)
{
	int ret;

	ret = vendor_infoframe_check(frame);
	if (ret)
		return ret;

	return vendor_infoframe_pack_only(frame, buffer, size);
}

static int vendor_any_infoframe_check_only(const union vendor_any_infoframe *frame)
{
	if (frame->any.type != INFOFRAME_TYPE_VENDOR ||
	    frame->any.version != 1)
		return -EINVAL;

	return 0;
}

/**
 * drm_infoframe_init() - initialize a dynamic Range and
 * mastering infoframe
 * @frame: DRM infoframe
 *
 * Returns 0 on success or a negative error code on failure.
 */
int drm_infoframe_init(struct drm_infoframe *frame)
{
	memset(frame, 0, sizeof(*frame));

	frame->type = INFOFRAME_TYPE_DRM;
	frame->version = 1;
	frame->length = DRM_INFOFRAME_SIZE;

	return 0;
}

static int drm_infoframe_check_only(const struct drm_infoframe *frame)
{
	if (frame->type != INFOFRAME_TYPE_DRM ||
	    frame->version != 1)
		return -EINVAL;

	if (frame->length != DRM_INFOFRAME_SIZE)
		return -EINVAL;

	return 0;
}

/**
 * drm_infoframe_check() - check a DRM infoframe
 * @frame: DRM infoframe
 *
 * Validates that the infoframe is consistent.
 * Returns 0 on success or a negative error code on failure.
 */
int drm_infoframe_check(struct drm_infoframe *frame)
{
	return drm_infoframe_check_only(frame);
}

/**
 * drm_infoframe_pack_only() - write DRM infoframe to binary buffer
 * @frame: DRM infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 *
 * Packs the information contained in the @frame structure into a binary
 * representation that can be written into the corresponding controller
 * registers. Also computes the checksum as required by section 5.3.5 of
 * the HDMI 1.4 specification.
 *
 * Returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
ssize_t drm_infoframe_pack_only(const struct drm_infoframe *frame, void *buffer, size_t size)
{
	u8 *ptr = buffer;
	size_t length;
	int i;

	length = INFOFRAME_HEADER_SIZE + frame->length;

	if (size < length)
		return -ENOSPC;

	memset(buffer, 0, size);

	ptr[0] = frame->type;
	ptr[1] = frame->version;
	ptr[2] = frame->length;
	ptr[3] = 0; /* checksum */

	/* start infoframe payload */
	ptr += INFOFRAME_HEADER_SIZE;

	*ptr++ = frame->eotf;
	*ptr++ = frame->metadata_type;

	for (i = 0; i < 3; i++) {
		*ptr++ = frame->display_primaries[i].x;
		*ptr++ = frame->display_primaries[i].x >> 8;
		*ptr++ = frame->display_primaries[i].y;
		*ptr++ = frame->display_primaries[i].y >> 8;
	}

	*ptr++ = frame->white_point.x;
	*ptr++ = frame->white_point.x >> 8;

	*ptr++ = frame->white_point.y;
	*ptr++ = frame->white_point.y >> 8;

	*ptr++ = frame->max_display_mastering_luminance;
	*ptr++ = frame->max_display_mastering_luminance >> 8;

	*ptr++ = frame->min_display_mastering_luminance;
	*ptr++ = frame->min_display_mastering_luminance >> 8;

	*ptr++ = frame->max_cll;
	*ptr++ = frame->max_cll >> 8;

	*ptr++ = frame->max_fall;
	*ptr++ = frame->max_fall >> 8;

	infoframe_set_checksum(buffer, length);

	return length;
}

/**
 * drm_infoframe_pack() - check a DRM infoframe,
 *                             and write it to binary buffer
 * @frame: DRM infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 *
 * Validates that the infoframe is consistent and updates derived fields
 * (eg. length) based on other fields, after which it packs the information
 * contained in the @frame structure into a binary representation that
 * can be written into the corresponding controller registers. This function
 * also computes the checksum as required by section 5.3.5 of the HDMI 1.4
 * specification.
 *
 * Returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
ssize_t drm_infoframe_pack(struct drm_infoframe *frame, void *buffer, size_t size)
{
	int ret;

	ret = drm_infoframe_check(frame);
	if (ret)
		return ret;

	return drm_infoframe_pack_only(frame, buffer, size);
}

/*
 * vendor_any_infoframe_check() - check a vendor infoframe
 */
static int vendor_any_infoframe_check(union vendor_any_infoframe *frame)
{
	int ret;

	ret = vendor_any_infoframe_check_only(frame);
	if (ret)
		return ret;

	/* we only know about HDMI vendor infoframes */
	if (frame->any.oui != HDMI_IEEE_OUI)
		return -EINVAL;

	return vendor_infoframe_check(&frame->info);
}

/*
 * vendor_any_infoframe_pack_only() - write a vendor infoframe to binary buffer
 */
static ssize_t vendor_any_infoframe_pack_only(const union vendor_any_infoframe *frame,
					      void *buffer, size_t size)
{
	int ret;

	ret = vendor_any_infoframe_check_only(frame);
	if (ret)
		return ret;

	/* we only know about HDMI vendor infoframes */
	if (frame->any.oui != HDMI_IEEE_OUI)
		return -EINVAL;

	return vendor_infoframe_pack_only(&frame->info, buffer, size);
}

/*
 * vendor_any_infoframe_pack() - check a vendor infoframe,
 *                                    and write it to binary buffer
 */
static ssize_t vendor_any_infoframe_pack(union vendor_any_infoframe *frame,
					 void *buffer, size_t size)
{
	int ret;

	ret = vendor_any_infoframe_check(frame);
	if (ret)
		return ret;

	return vendor_any_infoframe_pack_only(frame, buffer, size);
}

/**
 * infoframe_check() - check an infoframe
 * @frame: infoframe
 *
 * Validates that the infoframe is consistent and updates derived fields
 * (eg. length) based on other fields.
 *
 * Returns 0 on success or a negative error code on failure.
 */
int infoframe_check(union infoframe *frame)
{
	switch (frame->any.type) {
	case INFOFRAME_TYPE_AVI:
		return avi_infoframe_check(&frame->avi);
	case INFOFRAME_TYPE_SPD:
		return spd_infoframe_check(&frame->spd);
	case INFOFRAME_TYPE_AUDIO:
		return audio_infoframe_check(&frame->audio);
	case INFOFRAME_TYPE_VENDOR:
		return vendor_any_infoframe_check(&frame->vendor);
	default:
		WARN(1, "Bad infoframe type %d\n", frame->any.type);
		return -EINVAL;
	}
}

/**
 * infoframe_pack_only() - write an infoframe to binary buffer
 * @frame: infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 *
 * Packs the information contained in the @frame structure into a binary
 * representation that can be written into the corresponding controller
 * registers. Also computes the checksum as required by section 5.3.5 of
 * the HDMI 1.4 specification.
 *
 * Returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
ssize_t infoframe_pack_only(const union infoframe *frame, void *buffer, size_t size)
{
	ssize_t length;

	switch (frame->any.type) {
	case INFOFRAME_TYPE_AVI:
		length = avi_infoframe_pack_only(&frame->avi,
						      buffer, size);
		break;
	case INFOFRAME_TYPE_DRM:
		length = drm_infoframe_pack_only(&frame->drm,
						      buffer, size);
		break;
	case INFOFRAME_TYPE_SPD:
		length = spd_infoframe_pack_only(&frame->spd,
						      buffer, size);
		break;
	case INFOFRAME_TYPE_AUDIO:
		length = audio_infoframe_pack_only(&frame->audio,
							buffer, size);
		break;
	case INFOFRAME_TYPE_VENDOR:
		length = vendor_any_infoframe_pack_only(&frame->vendor,
							     buffer, size);
		break;
	default:
		WARN(1, "Bad infoframe type %d\n", frame->any.type);
		length = -EINVAL;
	}

	return length;
}

/**
 * infoframe_pack() - check an infoframe,
 *                         and write it to binary buffer
 * @frame: infoframe
 * @buffer: destination buffer
 * @size: size of buffer
 *
 * Validates that the infoframe is consistent and updates derived fields
 * (eg. length) based on other fields, after which it packs the information
 * contained in the @frame structure into a binary representation that
 * can be written into the corresponding controller registers. This function
 * also computes the checksum as required by section 5.3.5 of the HDMI 1.4
 * specification.
 *
 * Returns the number of bytes packed into the binary buffer or a negative
 * error code on failure.
 */
ssize_t infoframe_pack(union infoframe *frame, void *buffer, size_t size)
{
	ssize_t length;

	switch (frame->any.type) {
	case INFOFRAME_TYPE_AVI:
		length = avi_infoframe_pack(&frame->avi, buffer, size);
		break;
	case INFOFRAME_TYPE_DRM:
		length = drm_infoframe_pack(&frame->drm, buffer, size);
		break;
	case INFOFRAME_TYPE_SPD:
		length = spd_infoframe_pack(&frame->spd, buffer, size);
		break;
	case INFOFRAME_TYPE_AUDIO:
		length = audio_infoframe_pack(&frame->audio, buffer, size);
		break;
	case INFOFRAME_TYPE_VENDOR:
		length = vendor_any_infoframe_pack(&frame->vendor,
							buffer, size);
		break;
	default:
		WARN(1, "Bad infoframe type %d\n", frame->any.type);
		length = -EINVAL;
	}

	return length;
}

/**
 * avi_infoframe_unpack() - unpack binary buffer to an AVI infoframe
 * @frame: AVI infoframe
 * @buffer: source buffer
 * @size: size of buffer
 *
 * Unpacks the information contained in binary @buffer into a structured
 * @frame of the Auxiliary Video (AVI) information frame.
 * Also verifies the checksum as required by section 5.3.5 of the HDMI 1.4
 * specification.
 *
 * Returns 0 on success or a negative error code on failure.
 */
static int avi_infoframe_unpack(struct avi_infoframe *frame, const void *buffer, size_t size)
{
	const u8 *ptr = buffer;

	if (size < INFOFRAME_SIZE(AVI))
		return -EINVAL;

	if (ptr[0] != INFOFRAME_TYPE_AVI ||
	    ptr[1] < 2 ||
	    ptr[1] > 4 ||
	    ptr[2] != AVI_INFOFRAME_SIZE)
		return -EINVAL;

	if (infoframe_checksum(buffer, INFOFRAME_SIZE(AVI)) != 0)
		return -EINVAL;

	avi_infoframe_init(frame);

	ptr += INFOFRAME_HEADER_SIZE;

	frame->colorspace = (ptr[0] >> 5) & 0x3;
	if (ptr[0] & 0x10)
		frame->active_aspect = ptr[1] & 0xf;
	if (ptr[0] & 0x8) {
		frame->top_bar = (ptr[6] << 8) | ptr[5];
		frame->bottom_bar = (ptr[8] << 8) | ptr[7];
	}
	if (ptr[0] & 0x4) {
		frame->left_bar = (ptr[10] << 8) | ptr[9];
		frame->right_bar = (ptr[12] << 8) | ptr[11];
	}
	frame->scan_mode = ptr[0] & 0x3;

	frame->colorimetry = (ptr[1] >> 6) & 0x3;
	frame->picture_aspect = (ptr[1] >> 4) & 0x3;
	frame->active_aspect = ptr[1] & 0xf;

	frame->itc = ptr[2] & 0x80 ? true : false;
	frame->extended_colorimetry = (ptr[2] >> 4) & 0x7;
	frame->quantization_range = (ptr[2] >> 2) & 0x3;
	frame->nups = ptr[2] & 0x3;

	frame->video_code = ptr[3];
	frame->ycc_quantization_range = (ptr[4] >> 6) & 0x3;
	frame->content_type = (ptr[4] >> 4) & 0x3;

	frame->pixel_repeat = ptr[4] & 0xf;

	return 0;
}

/**
 * spd_infoframe_unpack() - unpack binary buffer to a SPD infoframe
 * @frame: SPD infoframe
 * @buffer: source buffer
 * @size: size of buffer
 *
 * Unpacks the information contained in binary @buffer into a structured
 * @frame of the Source Product Description (SPD) information frame.
 * Also verifies the checksum as required by section 5.3.5 of the HDMI 1.4
 * specification.
 *
 * Returns 0 on success or a negative error code on failure.
 */
static int spd_infoframe_unpack(struct spd_infoframe *frame, const void *buffer, size_t size)
{
	const u8 *ptr = buffer;
	int ret;

	if (size < INFOFRAME_SIZE(SPD))
		return -EINVAL;

	if (ptr[0] != INFOFRAME_TYPE_SPD ||
	    ptr[1] != 1 ||
	    ptr[2] != SPD_INFOFRAME_SIZE) {
		return -EINVAL;
	}

	if (infoframe_checksum(buffer, INFOFRAME_SIZE(SPD)) != 0)
		return -EINVAL;

	ptr += INFOFRAME_HEADER_SIZE;

	ret = spd_infoframe_init(frame, ptr, ptr + 8);
	if (ret)
		return ret;

	frame->sdi = ptr[24];

	return 0;
}

/**
 * audio_infoframe_unpack() - unpack binary buffer to an AUDIO infoframe
 * @frame: Audio infoframe
 * @buffer: source buffer
 * @size: size of buffer
 *
 * Unpacks the information contained in binary @buffer into a structured
 * @frame of the Audio information frame.
 * Also verifies the checksum as required by section 5.3.5 of the HDMI 1.4
 * specification.
 *
 * Returns 0 on success or a negative error code on failure.
 */
static int audio_infoframe_unpack(struct audio_infoframe *frame, const void *buffer, size_t size)
{
	const u8 *ptr = buffer;
	int ret;

	if (size < INFOFRAME_SIZE(AUDIO))
		return -EINVAL;

	if (ptr[0] != INFOFRAME_TYPE_AUDIO ||
	    ptr[1] != 1 ||
	    ptr[2] != AUDIO_INFOFRAME_SIZE) {
		return -EINVAL;
	}

	if (infoframe_checksum(buffer, INFOFRAME_SIZE(AUDIO)) != 0)
		return -EINVAL;

	ret = audio_infoframe_init(frame);
	if (ret)
		return ret;

	ptr += INFOFRAME_HEADER_SIZE;

	frame->channels = ptr[0] & 0x7;
	frame->coding_type = (ptr[0] >> 4) & 0xf;
	frame->sample_size = ptr[1] & 0x3;
	frame->sample_frequency = (ptr[1] >> 2) & 0x7;
	frame->coding_type_ext = ptr[2] & 0x1f;
	frame->channel_allocation = ptr[3];
	frame->level_shift_value = (ptr[4] >> 3) & 0xf;
	frame->downmix_inhibit = ptr[4] & 0x80 ? true : false;

	return 0;
}

/**
 * vendor_any_infoframe_unpack() - unpack binary buffer to a vendor infoframe
 * @frame: Vendor infoframe
 * @buffer: source buffer
 * @size: size of buffer
 *
 * Unpacks the information contained in binary @buffer into a structured
 * @frame of the Vendor information frame.
 * Also verifies the checksum as required by section 5.3.5 of the HDMI 1.4
 * specification.
 *
 * Returns 0 on success or a negative error code on failure.
 */
static int vendor_any_infoframe_unpack(union vendor_any_infoframe *frame, const void *buffer,
					size_t size)
{
	const u8 *ptr = buffer;
	size_t length;
	int ret;
	u8 video_format;
	struct vendor_infoframe *hvf = &frame->info;

	if (size < INFOFRAME_HEADER_SIZE)
		return -EINVAL;

	if (ptr[0] != INFOFRAME_TYPE_VENDOR ||
	    ptr[1] != 1 ||
	    (ptr[2] != 4 && ptr[2] != 5 && ptr[2] != 6))
		return -EINVAL;

	length = ptr[2];

	if (size < INFOFRAME_HEADER_SIZE + length)
		return -EINVAL;

	if (infoframe_checksum(buffer, INFOFRAME_HEADER_SIZE + length) != 0)
		return -EINVAL;

	ptr += INFOFRAME_HEADER_SIZE;

	/* HDMI OUI */
	if (ptr[0] != 0x03 || ptr[1] != 0x0c || ptr[2] != 0x00)
		return -EINVAL;

	video_format = ptr[3] >> 5;

	if (video_format > 0x2)
		return -EINVAL;

	ret = vendor_infoframe_init(hvf);
	if (ret)
		return ret;

	hvf->length = length;

	if (video_format == 0x2) {
		if (length != 5 && length != 6)
			return -EINVAL;
		hvf->s3d_struct = ptr[4] >> 4;
		if (hvf->s3d_struct >= VIDEO_3D_STRUCTURE_SIDE_BY_SIDE_HALF) {
			if (length != 6)
				return -EINVAL;
			hvf->s3d_ext_data = ptr[5] >> 4;
		}
	} else if (video_format == 0x1) {
		if (length != 5)
			return -EINVAL;
		hvf->vic = ptr[4];
	} else {
		if (length != 4)
			return -EINVAL;
	}

	return 0;
}

/**
 * drm_infoframe_unpack_only() - unpack binary buffer of CTA-861-G DRM
 *                                    infoframe DataBytes to a DRM infoframe
 * @frame: DRM infoframe
 * @buffer: source buffer
 * @size: size of buffer
 *
 * Unpacks CTA-861-G DRM infoframe DataBytes contained in the binary @buffer
 * into a structured @frame of the Dynamic Range and Mastering (DRM)
 * infoframe.
 *
 * Returns 0 on success or a negative error code on failure.
 */
int drm_infoframe_unpack_only(struct drm_infoframe *frame, const void *buffer, size_t size)
{
	const u8 *ptr = buffer;
	const u8 *temp;
	u8 x_lsb, x_msb;
	u8 y_lsb, y_msb;
	int ret;
	int i;

	if (size < DRM_INFOFRAME_SIZE)
		return -EINVAL;

	ret = drm_infoframe_init(frame);
	if (ret)
		return ret;

	frame->eotf = ptr[0] & 0x7;
	frame->metadata_type = ptr[1] & 0x7;

	temp = ptr + 2;
	for (i = 0; i < 3; i++) {
		x_lsb = *temp++;
		x_msb = *temp++;
		frame->display_primaries[i].x = (x_msb << 8) | x_lsb;
		y_lsb = *temp++;
		y_msb = *temp++;
		frame->display_primaries[i].y = (y_msb << 8) | y_lsb;
	}

	frame->white_point.x = (ptr[15] << 8) | ptr[14];
	frame->white_point.y = (ptr[17] << 8) | ptr[16];

	frame->max_display_mastering_luminance = (ptr[19] << 8) | ptr[18];
	frame->min_display_mastering_luminance = (ptr[21] << 8) | ptr[20];
	frame->max_cll = (ptr[23] << 8) | ptr[22];
	frame->max_fall = (ptr[25] << 8) | ptr[24];

	return 0;
}

/**
 * drm_infoframe_unpack() - unpack binary buffer to a DRM infoframe
 * @frame: DRM infoframe
 * @buffer: source buffer
 * @size: size of buffer
 *
 * Unpacks the CTA-861-G DRM infoframe contained in the binary @buffer into
 * a structured @frame of the Dynamic Range and Mastering (DRM)
 * infoframe. It also verifies the checksum as required by section 5.3.5 of
 * the HDMI 1.4 specification.
 *
 * Returns 0 on success or a negative error code on failure.
 */
static int drm_infoframe_unpack(struct drm_infoframe *frame, const void *buffer, size_t size)
{
	const u8 *ptr = buffer;
	int ret;

	if (size < INFOFRAME_SIZE(DRM))
		return -EINVAL;

	if (ptr[0] != INFOFRAME_TYPE_DRM ||
	    ptr[1] != 1 ||
	    ptr[2] != DRM_INFOFRAME_SIZE)
		return -EINVAL;

	if (infoframe_checksum(buffer, INFOFRAME_SIZE(DRM)) != 0)
		return -EINVAL;

	ret = drm_infoframe_unpack_only(frame, ptr + INFOFRAME_HEADER_SIZE,
					     size - INFOFRAME_HEADER_SIZE);
	return ret;
}

/**
 * infoframe_unpack() - unpack binary buffer to an infoframe
 * @frame: infoframe
 * @buffer: source buffer
 * @size: size of buffer
 *
 * Unpacks the information contained in binary buffer @buffer into a structured
 * @frame of an infoframe.
 * Also verifies the checksum as required by section 5.3.5 of the HDMI 1.4
 * specification.
 *
 * Returns 0 on success or a negative error code on failure.
 */
int infoframe_unpack(union infoframe *frame, const void *buffer, size_t size)
{
	int ret;
	const u8 *ptr = buffer;

	if (size < INFOFRAME_HEADER_SIZE)
		return -EINVAL;

	switch (ptr[0]) {
	case INFOFRAME_TYPE_AVI:
		ret = avi_infoframe_unpack(&frame->avi, buffer, size);
		break;
	case INFOFRAME_TYPE_DRM:
		ret = drm_infoframe_unpack(&frame->drm, buffer, size);
		break;
	case INFOFRAME_TYPE_SPD:
		ret = spd_infoframe_unpack(&frame->spd, buffer, size);
		break;
	case INFOFRAME_TYPE_AUDIO:
		ret = audio_infoframe_unpack(&frame->audio, buffer, size);
		break;
	case INFOFRAME_TYPE_VENDOR:
		ret = vendor_any_infoframe_unpack(&frame->vendor, buffer, size);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}
