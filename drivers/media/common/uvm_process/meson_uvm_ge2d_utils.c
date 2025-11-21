// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/amlogic/media/ge2d/ge2d.h>
#include <linux/amlogic/media/canvas/canvas_mgr.h>
#include <linux/amlogic/meson_uvm_ge2d_utils.h>

static int uvm_ge2d_debug;
module_param_named(uvm_ge2d_debug, uvm_ge2d_debug, int, 0664);

enum videocom_source_type {
	DECODER_8BIT_NORMAL = 0,
	DECODER_8BIT_BOTTOM,
	DECODER_8BIT_TOP,
	DECODER_10BIT_NORMAL,
	DECODER_10BIT_BOTTOM,
	DECODER_10BIT_TOP
};

static void uvm_canvas_cache_free(struct uvm_canvas_cache *cache)
{
	int i = -1;

	for (i = 0; i < ARRAY_SIZE(cache->res); i++) {
		if (cache->res[i].cid > 0) {
			if (uvm_ge2d_debug)
				pr_info("canvas-free, name:%s, canvas id:%d\n",
					cache->res[i].name,
					cache->res[i].cid);

			canvas_pool_map_free_canvas(cache->res[i].cid);

			cache->res[i].cid = 0;
		}
	}
}

static void uvm_canvas_cache_put(struct uvm_ge2d *ge2d)
{
	struct uvm_canvas_cache *cache = &ge2d->cache;

	mutex_lock(&cache->lock);
	if (uvm_ge2d_debug)
		pr_info("canvas-put, ref:%d\n", cache->ref);
	cache->ref--;
	if (cache->ref == 0)
		uvm_canvas_cache_free(cache);
	mutex_unlock(&cache->lock);
}

static int uvm_canvas_cache_get(struct uvm_ge2d *ge2d, char *usr)
{
	struct uvm_canvas_cache *cache = &ge2d->cache;
	int i;

	mutex_lock(&cache->lock);
	cache->ref++;
	for (i = 0; i < ARRAY_SIZE(cache->res); i++) {
		if (cache->res[i].cid <= 0) {
			snprintf(cache->res[i].name, 32, "%s-%d", usr, i);
			cache->res[i].cid =
				canvas_pool_map_alloc_canvas(cache->res[i].name);
		}

		if (uvm_ge2d_debug)
			pr_info("canvas-alloc, name:%s, canvas id:%d\n",
				cache->res[i].name,
				cache->res[i].cid);

		if (cache->res[i].cid <= 0) {
			pr_err("canvas-fail, name:%s, canvas id:%d.\n",
				cache->res[i].name,
				cache->res[i].cid);

			mutex_unlock(&cache->lock);
			goto err;
		}
	}

	if (uvm_ge2d_debug)
		pr_info("canvas-get, ref:%d\n", cache->ref);

	mutex_unlock(&cache->lock);
	return 0;
err:
	uvm_canvas_cache_put(ge2d);
	return -1;
}

static int uvm_canvas_cache_init(struct uvm_ge2d *ge2d)
{
	ge2d->cache.ref = 0;
	mutex_init(&ge2d->cache.lock);
	if (uvm_ge2d_debug)
		pr_info("canvas-init, ref:%d\n", ge2d->cache.ref);

	return 0;
}

static int get_source_type(struct vframe_s *vf)
{
	enum videocom_source_type ret;
	int interlace_mode;

	interlace_mode = vf->type & VIDTYPE_TYPEMASK;

	if ((vf->bitdepth & BITDEPTH_Y10)  &&
		(!(vf->type & VIDTYPE_COMPRESS))) {
		if (interlace_mode == VIDTYPE_INTERLACE_TOP)
			ret = DECODER_10BIT_TOP;
		else if (interlace_mode == VIDTYPE_INTERLACE_BOTTOM)
			ret = DECODER_10BIT_BOTTOM;
		else
			ret = DECODER_10BIT_NORMAL;
	} else {
		if (interlace_mode == VIDTYPE_INTERLACE_TOP)
			ret = DECODER_8BIT_TOP;
		else if (interlace_mode == VIDTYPE_INTERLACE_BOTTOM)
			ret = DECODER_8BIT_BOTTOM;
		else
			ret = DECODER_8BIT_NORMAL;
	}

	return ret;
}

static int get_input_format(struct vframe_s *vf)
{
	int format = GE2D_FORMAT_M24_YUV420;
	enum videocom_source_type soure_type;

	soure_type = get_source_type(vf);

	switch (soure_type) {
	case DECODER_8BIT_NORMAL:
		if (vf->type & VIDTYPE_VIU_422)
			format = GE2D_FORMAT_S16_YUV422;
		else if (vf->type & VIDTYPE_VIU_NV21)
			format = GE2D_FORMAT_M24_NV21;
		else if (vf->type & VIDTYPE_VIU_NV12)
			format = GE2D_FORMAT_M24_NV12;
		else if (vf->type & VIDTYPE_VIU_444)
			format = GE2D_FORMAT_S24_YUV444;
		else
			format = GE2D_FORMAT_M24_YUV420;
		break;
	case DECODER_8BIT_BOTTOM:
		if (vf->type & VIDTYPE_VIU_422)
			format = GE2D_FORMAT_S16_YUV422
				| (GE2D_FORMAT_S16_YUV422B & (3 << 3));
		else if (vf->type & VIDTYPE_VIU_NV21)
			format = GE2D_FORMAT_M24_NV21
				| (GE2D_FORMAT_M24_NV21B & (3 << 3));
		else if (vf->type & VIDTYPE_VIU_NV12)
			format = GE2D_FORMAT_M24_NV12
				| (GE2D_FORMAT_M24_NV12B & (3 << 3));
		else if (vf->type & VIDTYPE_VIU_444)
			format = GE2D_FORMAT_S24_YUV444
				| (GE2D_FORMAT_S24_YUV444B & (3 << 3));
		else
			format = GE2D_FORMAT_M24_YUV420
				| (GE2D_FMT_M24_YUV420B & (3 << 3));
		break;
	case DECODER_8BIT_TOP:
		if (vf->type & VIDTYPE_VIU_422)
			format = GE2D_FORMAT_S16_YUV422
				| (GE2D_FORMAT_S16_YUV422T & (3 << 3));
		else if (vf->type & VIDTYPE_VIU_NV21)
			format = GE2D_FORMAT_M24_NV21
				| (GE2D_FORMAT_M24_NV21T & (3 << 3));
		else if (vf->type & VIDTYPE_VIU_NV12)
			format = GE2D_FORMAT_M24_NV12
				| (GE2D_FORMAT_M24_NV12T & (3 << 3));
		else if (vf->type & VIDTYPE_VIU_444)
			format = GE2D_FORMAT_S24_YUV444
				| (GE2D_FORMAT_S24_YUV444T & (3 << 3));
		else
			format = GE2D_FORMAT_M24_YUV420
				| (GE2D_FMT_M24_YUV420T & (3 << 3));
		break;
	case DECODER_10BIT_NORMAL:
		if (vf->type & VIDTYPE_VIU_422) {
			if (vf->bitdepth & FULL_PACK_422_MODE)
				format = GE2D_FORMAT_S16_10BIT_YUV422;
			else
				format = GE2D_FORMAT_S16_12BIT_YUV422;
		}
		break;
	case DECODER_10BIT_BOTTOM:
		if (vf->type & VIDTYPE_VIU_422) {
			if (vf->bitdepth & FULL_PACK_422_MODE)
				format = GE2D_FORMAT_S16_10BIT_YUV422
					| (GE2D_FORMAT_S16_10BIT_YUV422B
					& (3 << 3));
			else
				format = GE2D_FORMAT_S16_12BIT_YUV422
					| (GE2D_FORMAT_S16_12BIT_YUV422B
					& (3 << 3));
		}
		break;
	case DECODER_10BIT_TOP:
		if (vf->type & VIDTYPE_VIU_422) {
			if (vf->bitdepth & FULL_PACK_422_MODE)
				format = GE2D_FORMAT_S16_10BIT_YUV422
					| (GE2D_FORMAT_S16_10BIT_YUV422T
					& (3 << 3));
			else
				format = GE2D_FORMAT_S16_12BIT_YUV422
					| (GE2D_FORMAT_S16_12BIT_YUV422T
					& (3 << 3));
		}
		break;
	default:
		format = GE2D_FORMAT_M24_YUV420;
	}
	return format;
}

int uvm_ge2d_init(struct uvm_ge2d **ge2d_handle, int mode)
{
	int ret;
	struct uvm_ge2d *ge2d;

	if (!ge2d_handle)
		return -EINVAL;

	ge2d = kzalloc(sizeof(*ge2d), GFP_KERNEL);
	if (!ge2d)
		return -ENOMEM;

	uvm_canvas_cache_init(ge2d);

	ge2d->work_mode = mode;
	if (!ge2d->work_mode)
		ge2d->work_mode = UVM_GE2D_MODE_CONVERT_LE;
	if (uvm_ge2d_debug)
		pr_info("%s: work_mode:%d\n", __func__, ge2d->work_mode);

	ge2d->ge2d_context = create_ge2d_work_queue();
	if (!ge2d->ge2d_context) {
		pr_err("ge2d_create_instance fail\n");
		ret = -EINVAL;
		goto error;
	}

	if (uvm_canvas_cache_get(ge2d, "uvm-ge2d-dec") < 0) {
		pr_err("canvas pool alloc fail. src(%d, %d, %d) dst(%d, %d, %d).\n",
			ge2d->cache.res[0].cid,
			ge2d->cache.res[1].cid,
			ge2d->cache.res[2].cid,
			ge2d->cache.res[3].cid,
			ge2d->cache.res[4].cid,
			ge2d->cache.res[5].cid);
		ret = -ENOMEM;
		goto error1;
	}

	*ge2d_handle = ge2d;

	return 0;
error1:
	destroy_ge2d_work_queue(ge2d->ge2d_context);
error:
	kfree(ge2d);

	return ret;
}

int uvm_ge2d_copy_data(struct uvm_ge2d *ge2d, struct uvm_ge2d_info *ge2d_info)
{
	struct config_para_ex_s ge2d_config;
	u32 src_fmt = 0, dst_fmt = 0;
	struct canvas_s cd;
	struct timeval start, end;
	unsigned long time_use = 0;

	if (!ge2d)
		return -1;

	if (uvm_ge2d_debug)
		pr_info("%s: start %d\n", __func__, __LINE__);

	memset(&ge2d_config, 0, sizeof(ge2d_config));

	src_fmt = get_input_format(ge2d_info->dst_vf);
	if (ge2d_info->src_canvas_config[0].endian == 7)
		src_fmt |= GE2D_BIG_ENDIAN;
	else
		src_fmt |= GE2D_LITTLE_ENDIAN;

	/* negotiate format of destination */
	dst_fmt = get_input_format(ge2d_info->dst_vf);
	if (ge2d->work_mode & UVM_GE2D_MODE_CONVERT_NV12)
		dst_fmt |= GE2D_FORMAT_M24_NV12;
	else if (ge2d->work_mode & UVM_GE2D_MODE_CONVERT_NV21)
		dst_fmt |= GE2D_FORMAT_M24_NV21;

	if (ge2d->work_mode & UVM_GE2D_MODE_CONVERT_LE)
		dst_fmt |= GE2D_LITTLE_ENDIAN;
	else
		dst_fmt |= GE2D_BIG_ENDIAN;
	if (uvm_ge2d_debug)
		pr_info("%s src_fmt(0x%x), dst_fmt(0x%x)\n", __func__, src_fmt, dst_fmt);

	if ((dst_fmt & GE2D_COLOR_MAP_MASK) == GE2D_COLOR_MAP_NV12) {
		ge2d_info->dst_vf->type |= VIDTYPE_VIU_NV12;
		ge2d_info->dst_vf->type &= ~VIDTYPE_VIU_NV21;
	} else if ((dst_fmt & GE2D_COLOR_MAP_MASK) == GE2D_COLOR_MAP_NV21) {
		ge2d_info->dst_vf->type |= VIDTYPE_VIU_NV21;
		ge2d_info->dst_vf->type &= ~VIDTYPE_VIU_NV12;
	}
	if ((dst_fmt & GE2D_ENDIAN_MASK) == GE2D_LITTLE_ENDIAN) {
		ge2d_info->dst_canvas_config[0].endian = 0;
		ge2d_info->dst_canvas_config[1].endian = 0;
		ge2d_info->dst_canvas_config[2].endian = 0;
	} else if ((dst_fmt & GE2D_ENDIAN_MASK) == GE2D_BIG_ENDIAN) {
		ge2d_info->dst_canvas_config[0].endian = 7;
		ge2d_info->dst_canvas_config[1].endian = 7;
		ge2d_info->dst_canvas_config[2].endian = 7;
	}

	ge2d_info->dst_vf->mem_sec = ge2d_info->dst_vf->flag & VFRAME_FLAG_VIDEO_SECURE ? 1 : 0;

	mutex_lock(&ge2d->cache.lock);

	do_gettimeofday(&start);
	/* src canvas configure. */
	if (ge2d_info->dst_vf->canvas0Addr == 0 ||
		(ge2d_info->dst_vf->canvas0Addr == (u32)-1)) {
		canvas_config_config(ge2d->cache.res[0].cid, &ge2d_info->src_canvas_config[0]);
		canvas_config_config(ge2d->cache.res[1].cid, &ge2d_info->src_canvas_config[1]);
		canvas_config_config(ge2d->cache.res[2].cid, &ge2d_info->src_canvas_config[2]);
		ge2d_config.src_para.canvas_index =
			ge2d->cache.res[0].cid |
			ge2d->cache.res[1].cid << 8 |
			ge2d->cache.res[2].cid << 16;

		ge2d_config.src_planes[0].addr =
			ge2d_info->src_canvas_config[0].phy_addr;
		ge2d_config.src_planes[0].w =
			ge2d_info->src_canvas_config[0].width;
		ge2d_config.src_planes[0].h =
			ge2d_info->src_canvas_config[0].height;
		ge2d_config.src_planes[1].addr =
			ge2d_info->src_canvas_config[1].phy_addr;
		ge2d_config.src_planes[1].w =
			ge2d_info->src_canvas_config[1].width;
		ge2d_config.src_planes[1].h =
			ge2d_info->src_canvas_config[1].height;
		ge2d_config.src_planes[2].addr =
			ge2d_info->src_canvas_config[2].phy_addr;
		ge2d_config.src_planes[2].w =
			ge2d_info->src_canvas_config[2].width;
		ge2d_config.src_planes[2].h =
			ge2d_info->src_canvas_config[2].height;
		if (uvm_ge2d_debug)
			pr_info("src_planes[0](0x%lx, %d, %d), src_planes[1](0x%lx, %d, %d)\n",
			ge2d_config.src_planes[0].addr, ge2d_config.src_planes[0].w,
			ge2d_config.src_planes[0].h, ge2d_config.src_planes[1].addr,
			ge2d_config.src_planes[1].w, ge2d_config.src_planes[1].h);
	} else {
		ge2d_config.src_para.canvas_index = ge2d_info->src_canvasAddr;
	}
	ge2d_config.src_para.mem_type	= CANVAS_TYPE_INVALID;
	ge2d_config.src_para.format = src_fmt;
	ge2d_config.src_para.fill_color_en = 0;
	ge2d_config.src_para.fill_mode	= 0;
	ge2d_config.src_para.x_rev	= 0;
	ge2d_config.src_para.y_rev	= 0;
	ge2d_config.src_para.color	= 0xffffffff;
	ge2d_config.src_para.top	= 0;
	ge2d_config.src_para.left	= 0;
	ge2d_config.src_para.width	= ge2d_info->src_yuv_width;
	if (ge2d_info->dst_vf->type & VIDTYPE_INTERLACE)
		ge2d_config.src_para.height = ge2d_info->src_yuv_height >> 1;
	else
		ge2d_config.src_para.height = ge2d_info->src_yuv_height;
	if (uvm_ge2d_debug)
		pr_info("src_para(%d, %d)\n",
			ge2d_config.src_para.width, ge2d_config.src_para.height);

	/* dst canvas configure. */
	canvas_config_config(ge2d->cache.res[3].cid, &ge2d_info->dst_canvas_config[0]);
	if ((ge2d_config.src_para.format & 0xfffff) == GE2D_FORMAT_M24_YUV420)
		ge2d_info->dst_canvas_config[1].width <<= 1;
	canvas_config_config(ge2d->cache.res[4].cid, &ge2d_info->dst_canvas_config[1]);
	canvas_config_config(ge2d->cache.res[5].cid, &ge2d_info->dst_canvas_config[2]);
	ge2d_config.dst_para.canvas_index =
		ge2d->cache.res[3].cid |
		ge2d->cache.res[4].cid << 8;
	canvas_read(ge2d->cache.res[3].cid, &cd);
	ge2d_config.dst_planes[0].addr = cd.addr;
	ge2d_config.dst_planes[0].w	= cd.width;

	/*
	 * Variable cd is initialised in canvas_read.
	 */
	/* coverity[uninit_use] */
	ge2d_config.dst_planes[0].h	= cd.height;
	canvas_read(ge2d->cache.res[4].cid, &cd);
	ge2d_config.dst_planes[1].addr	= cd.addr;
	ge2d_config.dst_planes[1].w	= cd.width;
	ge2d_config.dst_planes[1].h	= cd.height;

	if (uvm_ge2d_debug)
		pr_info("dst_planes[0](0x%lx, %d, %d), dst_planes[1](0x%lx, %d, %d)\n",
			ge2d_config.dst_planes[0].addr, ge2d_config.dst_planes[0].w,
			ge2d_config.dst_planes[0].h, ge2d_config.dst_planes[1].addr,
			ge2d_config.dst_planes[1].w, ge2d_config.dst_planes[1].h);

	ge2d_config.dst_para.format	=  dst_fmt;
	ge2d_config.dst_para.width	= ge2d_info->dst_yuv_width;
	ge2d_config.dst_para.height	= ge2d_info->dst_yuv_height;
	ge2d_config.dst_para.mem_type	= CANVAS_TYPE_INVALID;
	ge2d_config.dst_para.fill_color_en = 0;
	ge2d_config.dst_para.fill_mode	= 0;
	ge2d_config.dst_para.x_rev	= 0;
	ge2d_config.dst_para.y_rev	= 0;
	ge2d_config.dst_para.color	= 0;
	ge2d_config.dst_para.top	= 0;
	ge2d_config.dst_para.left	= 0;

	if (uvm_ge2d_debug)
		pr_info("dst_para(%d, %d)\n",
			ge2d_config.dst_para.width, ge2d_config.dst_para.height);

	/* other ge2d parameters configure. */
	ge2d_config.src_key.key_enable	= 0;
	ge2d_config.src_key.key_mask	= 0;
	ge2d_config.src_key.key_mode	= 0;
	ge2d_config.alu_const_color	= 0;
	ge2d_config.bitmask_en		= 0;
	ge2d_config.src1_gb_alpha	= 0;
	ge2d_config.dst_xy_swap		= 0;
	ge2d_config.src2_para.mem_type	= CANVAS_TYPE_INVALID;
	ge2d_config.mem_sec	= ge2d_info->dst_vf->flag & VFRAME_FLAG_VIDEO_SECURE ? 1 : 0;

	if (ge2d_context_config_ex(ge2d->ge2d_context, &ge2d_config) < 0) {
		pr_err("uvm_ge2d_context_config_ex error.\n");
		mutex_unlock(&ge2d->cache.lock);
		return -1;
	}

	if (!(ge2d_info->dst_vf->type & VIDTYPE_V4L_EOS)) {
		if (ge2d_info->dst_vf->type & VIDTYPE_INTERLACE) {
			stretchblt_noalpha(ge2d->ge2d_context,
				0, 0, ge2d_info->dst_vf->width, ge2d_info->dst_vf->height / 2,
				0, 0, ge2d_info->dst_vf->width, ge2d_info->dst_vf->height);
		} else {
			stretchblt_noalpha(ge2d->ge2d_context,
				0, 0, ge2d_config.src_para.width, ge2d_config.src_para.height,
				0, 0, ge2d_config.dst_para.width, ge2d_config.dst_para.height);
		}
	}
	mutex_unlock(&ge2d->cache.lock);
	do_gettimeofday(&end);
	time_use = (end.tv_sec - start.tv_sec) * 1000 +
				(end.tv_usec - start.tv_usec) / 1000;
	if (uvm_ge2d_debug)
		pr_info("GE2D copy time: %ldms\n", time_use);

	if (uvm_ge2d_debug)
		pr_info("%s: done %d\n", __func__, __LINE__);

	return 0;
}

void uvm_ge2d_destroy(struct uvm_ge2d *ge2d)
{
	if (uvm_ge2d_debug)
		pr_info("%s: start %d\n", __func__, __LINE__);

	destroy_ge2d_work_queue(ge2d->ge2d_context);
	uvm_canvas_cache_put(ge2d);
	kfree(ge2d);

	if (uvm_ge2d_debug)
		pr_info("%s: done %d\n", __func__, __LINE__);
}
