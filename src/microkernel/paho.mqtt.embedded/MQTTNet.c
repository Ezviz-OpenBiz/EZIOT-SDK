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
#include "MQTTNet.h"
#include "sdk_kernel_def.h"
#include "ezconfig.h"
#include "mcuconfig.h"


extern ezdev_sdk_kernel g_ezdev_sdk_kernel;
extern char g_binding_nic[ezdev_sdk_name_len];

static mkernel_internal_error g_mqttlaster = mkernel_internal_succ;

int MQTTNet_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	int real_write_buf_size = 0;
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	g_mqttlaster = mkernel_internal_succ;
	sdk_error = g_ezdev_sdk_kernel.platform_handle.net_work_write(n->my_socket, buffer, len, timeout_ms, &real_write_buf_size);
	g_mqttlaster = sdk_error;
	if (sdk_error == mkernel_internal_succ)
	{
		ezdev_sdk_kernel_log_trace(0, 0, "mqtt w succ, want:%d, send len:%d", len, real_write_buf_size);
		return real_write_buf_size;
	}
	else
	{
		ezdev_sdk_kernel_log_error(sdk_error, sdk_error, "mqtt w error:%d", sdk_error);
		return 0;
	}
}

int MQTTNet_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	g_mqttlaster = mkernel_internal_succ;
	sdk_error = g_ezdev_sdk_kernel.platform_handle.net_work_read(n->my_socket, buffer, len, timeout_ms);
	g_mqttlaster = sdk_error;
	if (sdk_error == mkernel_internal_succ)
	{
		return len;
	}
	else
	{
		return 0;
	}
}

mkernel_internal_error MQTTNetGetLastError()
{
	return g_mqttlaster;
}

void MQTTNetSetLastError(mkernel_internal_error code)
{
	g_mqttlaster = code;
}

void MQTTNetInit(Network* network)
{
	network->my_socket = NULL;
	network->mqttread = MQTTNet_read;
	network->mqttwrite = MQTTNet_write;
}

mkernel_internal_error MQTTNetConnect(Network* network, char* ip, int port )
{
	mkernel_internal_error error_code = mkernel_internal_succ;
	EZDEV_SDK_INT8 szRealIp[ezdev_sdk_ip_max_len] = {0};

	do 
	{
		network->my_socket = g_ezdev_sdk_kernel.platform_handle.net_work_create(g_binding_nic);
		if (network->my_socket == NULL)
		{
			error_code = mkernel_internal_create_sock_error;
			break;
		}

		error_code = g_ezdev_sdk_kernel.platform_handle.net_work_connect(network->my_socket, ip, port, 5*1000, (char*)szRealIp);
		if (mkernel_internal_succ != error_code && mkernel_internal_net_gethostbyname_error != error_code)
		{
			error_code = mkernel_internal_net_connect_error;
			break;
		}

	} while (0);

	if (error_code == mkernel_internal_succ)
	{
		return error_code;
	}
	
	if (network->my_socket != NULL)
	{
		g_ezdev_sdk_kernel.platform_handle.net_work_disconnect(network->my_socket);
		g_ezdev_sdk_kernel.platform_handle.net_work_destroy(network->my_socket);
		network->my_socket = NULL;
	}

	return error_code;
}

void MQTTNetDisconnect( Network* network)
{
	if (network->my_socket != NULL)
	{
		g_ezdev_sdk_kernel.platform_handle.net_work_disconnect(network->my_socket);
	}
}

void MQTTNetFini( Network* network)
{
	if (network->my_socket == NULL)
	{
		return;
	}
	g_ezdev_sdk_kernel.platform_handle.net_work_destroy(network->my_socket);
	network->my_socket = NULL;
}

void TimerInit( Timer* assign_timer )
{
	assign_timer->end_time = g_ezdev_sdk_kernel.platform_handle.time_creator();
}

char TimerIsExpiredByDiff(Timer* assign_timer, unsigned int time_ms)
{
	if (assign_timer->end_time == NULL)
	{
		return 0;
	}
	return g_ezdev_sdk_kernel.platform_handle.time_isexpired_bydiff(assign_timer->end_time, time_ms);
}

char TimerIsExpired( Timer* assign_timer )
{
	if (assign_timer->end_time == NULL)
	{
		return 0;
	}
	return g_ezdev_sdk_kernel.platform_handle.time_isexpired(assign_timer->end_time);
}

void TimerCountdownMS( Timer* assign_timer, unsigned int time_count )
{
	if (assign_timer->end_time == NULL)
	{
		return;
	}
	g_ezdev_sdk_kernel.platform_handle.time_countdownms(assign_timer->end_time, time_count);
}

void TimerCountdown( Timer* assign_timer, unsigned int time_count )
{
	if (assign_timer->end_time == NULL)
	{
		return;
	}
	g_ezdev_sdk_kernel.platform_handle.time_countdown(assign_timer->end_time, time_count);
}

int TimerLeftMS( Timer* assign_timer )
{
	if (assign_timer->end_time == NULL)
	{
		return 0;
	}
	return g_ezdev_sdk_kernel.platform_handle.time_leftms(assign_timer->end_time);
}

void TimerFini( Timer* assign_timer )
{	
	if (assign_timer->end_time == NULL)
	{
		return;
	}
	g_ezdev_sdk_kernel.platform_handle.time_destroy(assign_timer->end_time);
	assign_timer->end_time = NULL;
}