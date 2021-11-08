/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 * 
 * Brief:
 * 
 * 
 * Change Logs:
 * Date           Author       Notes
 * 
 *******************************************************************************/
#include "ezdev_sdk_kernel_common.h"
#include "ezdev_sdk_kernel_struct.h"
#include "sdk_kernel_def.h"
#include "dev_protocol_def.h"
#include "mkernel_internal_error.h"
#include "ezconfig.h"
#include "mcuconfig.h"

static ezdev_sdk_kernel_common_module	g_common_module = {0};	///<	通用模块

void common_module_init()
{
	memset(&g_common_module, 0, sizeof(ezdev_sdk_kernel_common_module));
}

void common_module_fini()
{
	memset(&g_common_module, 0, sizeof(ezdev_sdk_kernel_common_module));
}

mkernel_internal_error common_module_load(const ezdev_sdk_kernel_common_module* common_module)
{
	memcpy(&g_common_module, common_module, sizeof(ezdev_sdk_kernel_common_module));

	return mkernel_internal_succ;
}


EZDEV_SDK_INT8 common_module_bus_handle(ezdev_sdk_kernel_submsg* ptr_submsg)
{
	EZDEV_SDK_INT8 go_on = 1;

	EZDEV_SDK_INT32 cmd_id = ptr_submsg->msg_command_id;

	if (cmd_id == DAS_CMD_CENPLT2PUSETSWITCHENABLEREQ || \
		cmd_id == DAS_CMD_CENPLT2PUQUERYSTATUSREQ || \
		cmd_id == DAS_CMD_CENPLT2PUSETDEVPLANREQ || \
		cmd_id == DAS_CMD_COMM_DOMAIN_PU2CENPLTUPDATESTATUS || \
		cmd_id == DAS_CMD_CENPLT2PUSETCANARYTESTSTATUSREQ || \
		cmd_id == DAS_CMD_PU2CENPLTUPGRADERSP || \
		cmd_id == DEV_COMMON_CENPLT2PUSETKEYVALUEREQ)
	{
		if (g_common_module.ezdev_sdk_kernel_common_module_data_handle != NULL)
		{
			go_on = g_common_module.ezdev_sdk_kernel_common_module_data_handle(ptr_submsg, g_common_module.pUser);
		}
		else
		{
			go_on = 1;
		}
	}
	else
	{
		go_on = 1;
	}
	return go_on;
}