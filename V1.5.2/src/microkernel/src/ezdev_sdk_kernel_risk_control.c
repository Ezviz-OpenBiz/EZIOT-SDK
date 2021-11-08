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
#include "ezdev_sdk_kernel_risk_control.h"
#include "base_typedef.h"
#include "sdk_kernel_def.h"
#include "ezdev_sdk_kernel_extend.h"
#include "bscJSON.h"
#include "dev_protocol_def.h"
#include "mkernel_internal_error.h"
#include "ezconfig.h"
#include "mcuconfig.h"

EZDEV_SDK_KERNEL_EXTEND_INTERFACE
EZDEV_SDK_KERNEL_RISK_CONTROL_INTERFACE

void add_access_risk_control(ezdev_sdk_kernel* sdk_kernel)
{
	sdk_kernel->access_risk = sdk_risk_control;
}

void add_domain_risk_control(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT32 domain_id)
{
	ezdev_sdk_kernel_domain_info* domain_info = extend_get(domain_id);
	if (domain_info == NULL)
	{
		return;
	}
	domain_info->domain_risk = sdk_risk_control;
}

void add_cmd_risk_control(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT32 domain_id, EZDEV_SDK_UINT32 cmd_id)
{
	int index = 0;
	ezdev_sdk_kernel_domain_info* domain_info = extend_get(domain_id);
	if (domain_info == NULL)
	{
		return;
	}

	for (index = 0; index < ezdev_sdk_risk_control_cmd_max; index++)
	{
		if (0 == domain_info->cmd_risk_array[index])
		{
			domain_info->cmd_risk_array[index] = cmd_id;
			break;
		}
	}
}

char check_access_risk_control(ezdev_sdk_kernel* sdk_kernel)
{
	if (sdk_kernel->access_risk == sdk_no_risk_control)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

char check_cmd_risk_control(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT32 domain_id, EZDEV_SDK_UINT32 cmd_id)
{
	int index = 0;
	ezdev_sdk_kernel_domain_info*  domain_info = NULL;
	//特殊处理公共领域不做风控
	if (DAS_CMD_COMMON_FUN == domain_id|| DAS_CMD_DOMAIN == domain_id || ezdev_sdk_offline_cmd_id == cmd_id)
	{
		return 0;
	}
	
	domain_info = extend_get(domain_id);
	if (domain_info == NULL)
	{
		return 1;
	}

	if (domain_info->domain_risk == sdk_risk_control)
	{
		return 2;
	}

	for (index = 0; index < ezdev_sdk_risk_control_cmd_max; index++)
	{
		if (0 == domain_info->cmd_risk_array[index])
		{
			return 0;
			break;
		}
		if (cmd_id == domain_info->cmd_risk_array[index])
		{
			return 3;
			break;
		}
	}
	return 0;
}