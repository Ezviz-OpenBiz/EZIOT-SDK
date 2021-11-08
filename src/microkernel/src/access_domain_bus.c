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

 *******************************************************************************/

#include "ezconfig.h"
#include "mcuconfig.h"

#include "access_domain_bus.h"
#include "dev_protocol_def.h"
#include "mkernel_internal_error.h"
#include "sdk_kernel_def.h"
#include "bscJSON.h"
#include "ezdev_sdk_kernel_risk_control.h"
#include "ezdev_sdk_kernel_xml_parser.h"
#include "ezdev_sdk_kernel_event.h"
#include "das_transport.h"
#include "utils.h"

EZDEV_SDK_KERNEL_RISK_CONTROL_INTERFACE
EZDEV_SDK_KERNEL_XML_PARSER_INTERFACE
EZDEV_SDK_KERNEL_EVENT_INTERFACE
DAS_TRANSPORT_INTERFACE

extern ezdev_sdk_kernel g_ezdev_sdk_kernel;

static mkernel_internal_error bus_handle_domainconfig_keepalive(ezdev_sdk_kernel *sdk_kernel, bscJSON *json_item)
{
	bscJSON *json_interval = NULL;
	if (NULL == json_item)
	{
		return mkernel_internal_json_parse_error;
	}

	json_interval = bscJSON_GetObjectItem(json_item, "Interval");
	if (NULL == json_interval)
	{
		return mkernel_internal_get_error_json;
	}

	if (json_interval->type != bscJSON_Number)
	{
		return mkernel_internal_get_error_json;
	}

	/**
	* \brief   更新心跳时间
	*/
	sdk_kernel->das_keepalive_interval = json_interval->valueint;
	/**
	* \brief   心跳时间改变事件回调
	*/
	das_keepalive_interval_changed_event_cb(sdk_kernel->das_keepalive_interval);

	return mkernel_internal_succ;
}

static mkernel_internal_error bus_handle_domainconfig_ntp(ezdev_sdk_kernel *sdk_kernel, bscJSON *json_item)
{
	// 	sdk_kernel_event_ntp ntp_event;
	// 	bscJSON *json_Addr = NULL;
	// 	bscJSON *json_Port = NULL;
	// 	bscJSON *json_Interval = NULL;
	// 	bscJSON *json_Timezone = NULL;
	// 	if (NULL == json_item)
	// 	{
	// 		return mkernel_internal_json_parse_error;
	// 	}
	//
	// 	json_Addr = bscJSON_GetObjectItem(json_item, "Addr");
	// 	json_Port = bscJSON_GetObjectItem(json_item, "Port");
	// 	json_Interval = bscJSON_GetObjectItem(json_item, "Interval");
	// 	json_Timezone = bscJSON_GetObjectItem(json_item, "Timezone");
	// 	if (NULL == json_Addr || NULL == json_Port || NULL == json_Interval || NULL == json_Timezone)
	// 	{
	// 		return mkernel_internal_get_error_json;
	// 	}
	//
	// 	if (json_Port->type != bscJSON_Number || json_Interval->type != bscJSON_Number ||
	// 		json_Addr->type != bscJSON_String || json_Addr->valuestring == NULL ||
	// 		json_Timezone->type != bscJSON_String || json_Timezone->valuestring == NULL)
	// 	{
	// 		return mkernel_internal_get_error_json;
	// 	}
	//
	// 	memset(&ntp_event, 0, sizeof(sdk_kernel_event_ntp));
	//
	// 	ntp_event.ntp_interval = json_Interval->valueint;
	// 	ntp_event.ntp_port = json_Interval->valueint;
	// 	strncpy(ntp_event.ntp_addr, json_Addr->valuestring, ezdev_sdk_ip_max_len - 1);
	// 	strncpy(ntp_event.ntp_timezone, json_Timezone->valuestring, ezdev_sdk_timezone_max_len -1);
	//
	/**
	* \brief   将NTP信息回调出去
	*/
	//sdk_kernel->kernel_event_notice_cb(sdk_kernel_event_ntp_info, (void*)(&ntp_event));

	return mkernel_internal_succ;
}

static mkernel_internal_error bus_handle_domainconfig(ezdev_sdk_kernel *sdk_kernel, const ezdev_sdk_kernel_submsg *ptr_submsg)
{
	bscJSON *json_keepalive = NULL;
	bscJSON *json_ntp = NULL;
	bscJSON *json_root = NULL;
	if (NULL == ptr_submsg->buf)
	{
		return mkernel_internal_rev_invalid_packet;
	}
	json_root = bscJSON_Parse((const char *)ptr_submsg->buf);
	if (json_root == NULL)
	{
		return mkernel_internal_json_parse_error;
	}

	json_keepalive = bscJSON_GetObjectItem(json_root, "KeepAlive");
	json_ntp = bscJSON_GetObjectItem(json_root, "NTP");
	//	bscJSON *json_httpdns = bscJSON_GetObjectItem(json_root, "httpDNS");
	if (json_keepalive != NULL)
	{
		bus_handle_domainconfig_keepalive(sdk_kernel, json_keepalive);
	}
	if (json_ntp != NULL)
	{
		bus_handle_domainconfig_ntp(sdk_kernel, json_keepalive);
	}
	if (json_ntp != NULL)
	{
	}
	bscJSON_Delete(json_root);
	return mkernel_internal_succ;
}

static mkernel_internal_error bus_handle_set_keeplive_time_rsp(ezdev_sdk_kernel *sdk_kernel, const ezdev_sdk_kernel_submsg *ptr_submsg)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	bscJSON *json_root = NULL;
	bscJSON *json_result = NULL;
	bscJSON *json_interval = NULL;

	do
	{
		if (NULL == ptr_submsg->buf)
		{
			sdk_error = mkernel_internal_rev_invalid_packet;
			break;
		}

		json_root = bscJSON_Parse((const char *)ptr_submsg->buf);
		if (NULL == json_root)
		{
			sdk_error = mkernel_internal_rev_invalid_packet;
			break;
		}

		json_result = bscJSON_GetObjectItem(json_root, "retcode");
		json_interval = bscJSON_GetObjectItem(json_root, "interval");
		if (NULL == json_result || NULL == json_interval || bscJSON_Number != json_result->type || bscJSON_Number != json_interval->type)
		{
			sdk_error = mkernel_internal_get_error_json;
			break;
		}

		if (0 == json_result->valueint)
		{
			sdk_error = das_change_keep_alive_interval(sdk_kernel, json_interval->valueint);
			das_keepalive_interval_changed_event_cb(json_interval->valueint);
		}

	} while (0);

	if (NULL != json_root)
		bscJSON_Delete(json_root);

	return sdk_error;
}

static mkernel_internal_error bus_handle_dev_redirect(ezdev_sdk_kernel *sdk_kernel, const ezdev_sdk_kernel_submsg *ptr_submsg)
{
	/**
	* \brief   重定向 entr_state 标记由于平台原因导致切换的 cnt_state 标记需要重新做重定向
	*/
	char domain_name[ezdev_sdk_name_len] = {0};
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	do
	{
		sdk_error = cenplt2pusetlbsdomainnamebydasreq_xml_parser(ptr_submsg->buf, ptr_submsg->buf_len, domain_name);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		if (strcmp(sdk_kernel->server_info.server_name, domain_name) == 0)
		{
			ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "no need redirect cur:%s, want:%s", sdk_kernel->server_info.server_name, domain_name);
			break;
		}

		ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "need redirect cur:%s, want:%s", sdk_kernel->server_info.server_name, domain_name);

		strncpy(sdk_kernel->server_info.server_name, domain_name, ezdev_sdk_name_len);
		memset(sdk_kernel->server_info.server_ip, 0, ezdev_sdk_name_len);
		memset(sdk_kernel->master_key, 0, ezdev_sdk_masterkey_len);

		sdk_kernel->entr_state = sdk_entrance_switchover;
		sdk_kernel->cnt_state = sdk_cnt_unredirect;

	} while (0);

	return sdk_error;
}

static mkernel_internal_error bus_handle_dev_risk(ezdev_sdk_kernel *sdk_kernel, const ezdev_sdk_kernel_submsg *ptr_submsg)
{
	int indexa = 0;
	int indexb = 0;
	int domain_size = 0;
	int domain_cmd_size = 0;
	EZDEV_SDK_UINT32 domain_id = 0;
	bscJSON *json_risk_array = NULL;
	bscJSON *json_domain = NULL;
	bscJSON *json_domain_cmd = NULL;
	bscJSON *json_root = NULL;

	if (NULL == ptr_submsg->buf)
	{
		return mkernel_internal_rev_invalid_packet;
	}

	json_root = bscJSON_Parse((const char *)ptr_submsg->buf);
	if (json_root == NULL)
	{
		return mkernel_internal_json_parse_error;
	}

	json_risk_array = bscJSON_GetObjectItem(json_root, "risk");
	domain_size = bscJSON_GetArraySize(json_risk_array);
	for (indexa = 0; indexa < domain_size; indexa++)
	{
		json_domain = bscJSON_GetArrayItem(json_risk_array, indexa);
		if (NULL == json_domain)
			continue;

		if (NULL != json_domain->string)
		{
			domain_id = atoi(json_domain->string);
		}

		domain_cmd_size = bscJSON_GetArraySize(json_domain);
		if (domain_cmd_size == 0)
		{
			add_domain_risk_control(sdk_kernel, domain_id);
			continue;
		}
		for (indexb = 0; indexb < domain_size; indexb++)
		{
			json_domain_cmd = bscJSON_GetArrayItem(json_domain, indexb);
			if (NULL == json_domain_cmd)
				continue;

			add_cmd_risk_control(sdk_kernel, domain_id, json_domain_cmd->valueint);
		}
	}

	bscJSON_Delete(json_root);
	return mkernel_internal_succ;
}

static mkernel_internal_error bus_handle_force_offline(ezdev_sdk_kernel *sdk_kernel)
{
	ezdev_sdk_kernel_log_warn(mkernel_internal_force_offline, 0, "force offline");
	broadcast_runtime_err(TAG_ACCESS, mkiE2ezE(mkernel_internal_force_offline), NULL, 0);
	add_access_risk_control(sdk_kernel);
	return mkernel_internal_succ;
}

mkernel_internal_error access_domain_bus_handle(const ezdev_sdk_kernel_submsg *ptr_submsg)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	switch (ptr_submsg->msg_command_id)
	{
	case DAS_CMD_CENPLT2PUDOMAINCONFIG:
		sdk_error = bus_handle_domainconfig(&g_ezdev_sdk_kernel, ptr_submsg);
		break;
	case DAS_CMD_CENPLT2PURISKCONFIG:
		sdk_error = bus_handle_dev_risk(&g_ezdev_sdk_kernel, ptr_submsg);
		break;
	case DAS_CMD_CENPLT2PUOFFLINE:
		sdk_error = bus_handle_force_offline(&g_ezdev_sdk_kernel);
		break;
	case DAS_CMD_PU2CENPLTSETKEEPALIVETIMERSP:
		sdk_error = bus_handle_set_keeplive_time_rsp(&g_ezdev_sdk_kernel, ptr_submsg);
	case DAS_CMD_CENPLT2PUSETKEEPALIVETIMEREQ:
		break;
	case DAS_CMD_CENPLT2PUSETLBSDOMAINNAMEBYDASREQ:
	case DAS_CMD_CENPLT2PUSETLBSDOMAINNAMEREQ:
		sdk_error = bus_handle_dev_redirect(&g_ezdev_sdk_kernel, ptr_submsg);
		break;
	default:
		break;
	}
	return sdk_error;
}