/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _OPTEE_LOG_H_
#define _OPTEE_LOG_H_

#define DEF_LOGGER_SHM_SIZE             (256 * 1024)

int optee_log_init(void);

void optee_log_uninit(void);

#endif//_OPTEE_LOG_H_
