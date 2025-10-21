// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/amlogic/media/di/dpss_interface.h>
#include <linux/amlogic/media/dpss/frc_common_x.h>
#include <linux/kfifo.h>
#include <linux/types.h>

extern void *dpss_get_fw_data(void);
#ifdef NO_USED

static const char frc_alg_ver[] = "frcalg_ver:dpss_frc_alg_20250310_xxxx_t6w";

u32 frc_fw_inp(struct dpss_frc_fw_data_s *fw_data)
{
	frc_dbg(DPSS_FRC_TOP, "%s	ret 0\n", __func__);
	return 0;
}

void frc_fw_dae(struct dpss_frc_fw_data_s *fw_data)
{
	frc_dbg(DPSS_FRC_TOP, "%s	ret 0\n", __func__);
}

void frc_fw_mc(struct dpss_frc_fw_data_s *fw_data)
{
	frc_dbg(DPSS_FRC_TOP, "%s	ret 0\n", __func__);
}

void frc_fw_alg_init(void)
{
	struct dpss_frc_fw_data_s *pfw_data;
	ssize_t len = 0;

	pfw_data = (struct dpss_frc_fw_data_s *)dpss_get_fw_data();
	if (!pfw_data) {
		pr_info("frc_fw: init failed\n");
		return -1;
	}
	if (strlen(frc_alg_ver) > FRC_ALG_VER_SIZE)
		strncpy(&pfw_data->frc_alg_ver[0], &frc_alg_ver[0],
			FRC_ALG_VER_SIZE);
	else
		strncpy(&pfw_data->frc_alg_ver[0], &frc_alg_ver[0],
			strlen(frc_alg_ver));

	pfw_data->inp_irq_handler = frc_fw_inp;
	pfw_data->dae_irq_handler = frc_fw_dae;
	pfw_data->pre_vsync_irq_handler = frc_fw_mc;
	pr_info("frc_fw: init success\n");
}
#endif
