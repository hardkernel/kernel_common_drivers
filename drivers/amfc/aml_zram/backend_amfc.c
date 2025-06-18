// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/crypto.h>
#include <linux/amlogic/amfc.h>

#include "backend_amfc.h"

struct amfc_ctx {
	struct crypto_comp *tfm;
};

static void amfc_release_params(struct zcomp_params *params)
{
}

static int amfc_setup_params(struct zcomp_params *params)
{
	return 0;
}

static void amfc_destroy(struct zcomp_ctx *ctx)
{
	struct amfc_ctx *zctx = ctx->context;

	if (!zctx)
		return;

	crypto_free_comp(zctx->tfm);
}

static int amfc_create(struct zcomp_params *params, struct zcomp_ctx *ctx)
{
	struct amfc_ctx *zctx;

	zctx = kzalloc(sizeof(*zctx), GFP_KERNEL);
	if (!zctx)
		return -ENOMEM;

	zctx->tfm = crypto_alloc_comp("amfc", 0, 0);
	if (IS_ERR_OR_NULL(zctx->tfm)) {
		pr_err("creat amfc crypto failed\n");
		return -ENODEV;
	}
	ctx->context = zctx;

	return 0;
}

static int amfc_zram_compress(struct zcomp_params *params, struct zcomp_ctx *ctx,
			      struct zcomp_req *req)
{
	struct amfc_ctx *zctx = ctx->context;
	int ret;
	unsigned int len = req->dst_len;

	ret = crypto_comp_compress(zctx->tfm, req->src, req->src_len,
				 req->dst, &len);
	if (ret < 0)
		return -EINVAL;

	req->dst_len = len;
	return 0;
}

static int amfc_zram_decompress(struct zcomp_params *params, struct zcomp_ctx *ctx,
				struct zcomp_req *req)
{
	struct amfc_ctx *zctx = ctx->context;
	int ret;
	unsigned int len = req->dst_len;

	ret = crypto_comp_decompress(zctx->tfm, req->src, req->src_len,
				     req->dst, &len);
	if (ret < 0)
		return -EINVAL;
	return 0;
}

const struct zcomp_ops backend_amfc = {
	.compress	= amfc_zram_compress,
	.decompress	= amfc_zram_decompress,
	.create_ctx	= amfc_create,
	.destroy_ctx	= amfc_destroy,
	.setup_params	= amfc_setup_params,
	.release_params	= amfc_release_params,
	.name		= "amfc",
};
