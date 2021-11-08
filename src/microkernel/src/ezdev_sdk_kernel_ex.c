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
#include "ezconfig.h"
#include "mcuconfig.h"

#include "ezdev_sdk_kernel_ex.h"
#include "lbs_transport.h"
#include "sdk_kernel_def.h"
#include "utils.h"
#include "dev_protocol_def.h"
#include "bscJSON.h"
#include "das_transport.h"
#include "mkernel_internal_error.h"
#include "ezdev_sdk_kerne_queuel.h"
#include "ase_support.h"

extern ezdev_sdk_kernel g_ezdev_sdk_kernel;
EXTERN_QUEUE_FUN(pubmsg_exchange)
LBS_TRANSPORT_INTERFACE
DAS_TRANSPORT_INTERFACE
ASE_SUPPORT_INTERFACE

EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_get_stun(stun_info* ptr_stun, EZDEV_SDK_BOOL bforce_refresh)
{
	static stun_info stun_info_cache = {0};
	ezdev_sdk_kernel_error rv = ezdev_sdk_kernel_succ;

	do
	{
		if (g_ezdev_sdk_kernel.my_state != sdk_start)
		{
			rv = ezdev_sdk_kernel_invald_call;
			break;
		}

		if(NULL == ptr_stun)
		{
			rv = ezdev_sdk_kernel_params_invalid;
			break;
		}

		if(!bforce_refresh)
		{
			memcpy(ptr_stun, &stun_info_cache, sizeof(stun_info_cache));
			break;
		}

		memset(ptr_stun, 0, sizeof(stun_info));

		if(ezdev_sdk_kernel_succ != (rv = mkiE2ezE(lbs_getstun(&g_ezdev_sdk_kernel, ptr_stun))))
			break;

		memcpy(&stun_info_cache, ptr_stun, sizeof(stun_info_cache));
	}while(0);

	return rv;
}

EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_set_keepalive_interval(EZDEV_SDK_UINT16 internal, EZDEV_SDK_UINT16 timeout_s)
{
	ezdev_sdk_kernel_error rv = 0;
	bscJSON * pJsonRoot = NULL;
	char * json_buf= NULL;
	ezdev_sdk_kernel_pubmsg_exchange *new_pubmsg_exchange = NULL;
	EZDEV_SDK_INT32 input_length_padding = 0;

	if (g_ezdev_sdk_kernel.my_state != sdk_start)
	{
		return ezdev_sdk_kernel_invald_call; 
	}

	do 
	{
		pJsonRoot = bscJSON_CreateObject();
		new_pubmsg_exchange = (ezdev_sdk_kernel_pubmsg_exchange*)malloc(sizeof(ezdev_sdk_kernel_pubmsg_exchange));
		
		if(!pJsonRoot || !new_pubmsg_exchange)
		{
			rv = ezdev_sdk_kernel_memory;
			break;
		}
        memset(new_pubmsg_exchange, 0, sizeof(ezdev_sdk_kernel_pubmsg_exchange));
		//组织报文
        bscJSON_AddNumberToObject(pJsonRoot, "interval", internal);
		if(NULL == (json_buf = bscJSON_PrintUnformatted(pJsonRoot)))
		{
			rv = ezdev_sdk_kernel_memory;
			break;
		}

		memset(new_pubmsg_exchange, 0, sizeof(ezdev_sdk_kernel_pubmsg_exchange));
		new_pubmsg_exchange->msg_conntext.msg_response = 0;
		new_pubmsg_exchange->msg_conntext.msg_seq =	0;
		new_pubmsg_exchange->msg_conntext.msg_domain_id = DAS_CMD_DOMAIN;
		new_pubmsg_exchange->msg_conntext.msg_command_id = DAS_CMD_PU2CENPLTSETKEEPALIVETIMEREQ;

		input_length_padding = strlen(json_buf);
		new_pubmsg_exchange->msg_conntext.msg_body = (unsigned char*)malloc(input_length_padding+1);
		if (NULL == new_pubmsg_exchange->msg_conntext.msg_body)
		{
			rv = ezdev_sdk_kernel_memory;
			break;
		}

		memset(new_pubmsg_exchange->msg_conntext.msg_body, 0, input_length_padding);
		new_pubmsg_exchange->msg_conntext.msg_body_len = input_length_padding;
		memcpy(new_pubmsg_exchange->msg_conntext.msg_body , json_buf,  strlen(json_buf));

		buf_padding(new_pubmsg_exchange->msg_conntext.msg_body, input_length_padding, strlen(json_buf));
		new_pubmsg_exchange->max_send_count = ezdev_sdk_max_publish_count;	//默认最多发送2次

		//设置心跳的信令优先级别较高，优先发送
		rv = push_queue_head_pubmsg_exchange(new_pubmsg_exchange);
    }while(0);

	if(json_buf)
        free(json_buf);

    if(NULL != pJsonRoot)
	{
        bscJSON_Delete(pJsonRoot);
	}

	if (rv != ezdev_sdk_kernel_succ)
	{
		if (new_pubmsg_exchange != NULL)
		{
			if (new_pubmsg_exchange->msg_conntext.msg_body != NULL)
			{
				free(new_pubmsg_exchange->msg_conntext.msg_body);
				new_pubmsg_exchange->msg_conntext.msg_body=NULL;
			}

			free(new_pubmsg_exchange);
			new_pubmsg_exchange = NULL;
		}
	}

	return rv;
}