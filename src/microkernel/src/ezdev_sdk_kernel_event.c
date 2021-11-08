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

#include "ezdev_sdk_kernel_event.h"
#include "ezdev_sdk_kerne_queuel.h"
#include "mkernel_internal_error.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ezconfig.h"
#include "mcuconfig.h"

EXTERN_QUEUE_FUN(inner_cb_notic)
EXTERN_QUEUE_BASE_FUN
/** 
 *  \brief		添加一条消息至本地消息队列
 *  \method		push_event_to_queue
 *  \param[in] 	prt_inner_cb_notic	消息对象
 *  \return 	成功返0 失败返回对应错误码
 */
static mkernel_internal_error push_event_to_queue(ezdev_sdk_kernel_inner_cb_notic *prt_inner_cb_notic)
{
	mkernel_internal_error kernel_error = mkernel_internal_succ;
	int count = 0;
	if (NULL == prt_inner_cb_notic)
	{
		return mkernel_internal_input_param_invalid;
	}

	do
	{
		kernel_error = push_queue_inner_cb_notic(prt_inner_cb_notic);
		if (mkernel_internal_queue_full == kernel_error)
		{
			ezdev_sdk_kernel_log_error(kernel_error, 0, "queue_full, try again.");
			get_ezdev_sdk_kernel()->platform_handle.time_sleep(10);

			if(count++ > 50)
				break;
			else
				continue;
		}

		break;
	} while (1);

	if (kernel_error != mkernel_internal_succ)
	{
		ezdev_sdk_kernel_log_error(kernel_error, 0, "push queue err, cb_type:%d, event_type:%d", prt_inner_cb_notic->cb_type, prt_inner_cb_notic->cb_event.event_type);
	}

	return kernel_error;
}

/** 
 *  \brief		广播一条微内核启动消息
 *	\note		接收对象 已向微内核注册的所有领域
 *  \method		broadcast_user_start
 *  \return 	成功返0 失败返回对应错误码
 */
mkernel_internal_error broadcast_user_start()
{
	ezdev_sdk_kernel_inner_cb_notic *prt_inner_cb_notic = NULL;
	prt_inner_cb_notic = (ezdev_sdk_kernel_inner_cb_notic *)malloc(sizeof(ezdev_sdk_kernel_inner_cb_notic));
	if (NULL == prt_inner_cb_notic)
	{
		return mkernel_internal_malloc_error;
	}
	memset(prt_inner_cb_notic, 0, sizeof(ezdev_sdk_kernel_inner_cb_notic));
	prt_inner_cb_notic->cb_type = extend_cb_start;
	prt_inner_cb_notic->cb_event.event_type = 0;
	prt_inner_cb_notic->cb_event.event_context = NULL;

	return push_event_to_queue(prt_inner_cb_notic);
}

/** 
 *  \brief		广播一条微内核停止消息
 *	\note		接收对象 已向微内核注册的所有领域
 *  \method		broadcast_user_stop
 *  \return 	成功返0 失败返回对应错误码
 */
mkernel_internal_error broadcast_user_stop()
{
	ezdev_sdk_kernel_inner_cb_notic *prt_inner_cb_notic = NULL;
	prt_inner_cb_notic = (ezdev_sdk_kernel_inner_cb_notic *)malloc(sizeof(ezdev_sdk_kernel_inner_cb_notic));
	if (NULL == prt_inner_cb_notic)
	{
		return mkernel_internal_malloc_error;
	}
	memset(prt_inner_cb_notic, 0, sizeof(ezdev_sdk_kernel_inner_cb_notic));
	prt_inner_cb_notic->cb_type = extend_cb_stop;
	prt_inner_cb_notic->cb_event.event_type = 0;
	prt_inner_cb_notic->cb_event.event_context = NULL;

	return push_event_to_queue(prt_inner_cb_notic);
}

mkernel_internal_error broadcast_user_event(sdk_kernel_event_type event_type, void *ctx, EZDEV_SDK_UINT32 ctx_size)
{
	mkernel_internal_error rv = mkernel_internal_succ;
	void *context = NULL;
	ezdev_sdk_kernel_inner_cb_notic *prt_inner_cb_notic = NULL;

	do
	{
		prt_inner_cb_notic = (ezdev_sdk_kernel_inner_cb_notic *)malloc(sizeof(ezdev_sdk_kernel_inner_cb_notic));
		if (NULL == prt_inner_cb_notic)
		{
			rv = mkernel_internal_malloc_error;
			break;
		}

		if (NULL != ctx)
		{
			if (NULL == (context = (void *)malloc(ctx_size)))
			{
				free(prt_inner_cb_notic);
				rv = mkernel_internal_malloc_error;
				break;
			}

			memcpy(context, ctx, ctx_size);
		}

		memset(prt_inner_cb_notic, 0, sizeof(ezdev_sdk_kernel_inner_cb_notic));
		prt_inner_cb_notic->cb_type = extend_cb_event;
		prt_inner_cb_notic->cb_event.event_type = event_type;
		prt_inner_cb_notic->cb_event.event_context = context;
		if (mkernel_internal_succ != (rv = push_event_to_queue(prt_inner_cb_notic)))
		{
			free(prt_inner_cb_notic);

			if (NULL != context)
				free(context);
		}
	} while (0);

	return rv;
}

mkernel_internal_error broadcast_runtime_err(err_tag_e err_tag, ezdev_sdk_kernel_error err_code, void *err_ctx, EZDEV_SDK_UINT32 ctx_size)
{
	mkernel_internal_error rv = mkernel_internal_succ;
	sdk_runtime_err_context rt_err_ctx = {err_tag, err_code, NULL};

	if (NULL != err_ctx)
	{
		if (NULL == (rt_err_ctx.err_ctx = (void *)malloc(ctx_size)))
		{
			return mkernel_internal_malloc_error;
		}

		memcpy(rt_err_ctx.err_ctx, err_ctx, ctx_size);
	}

	if (mkernel_internal_succ != (rv = broadcast_user_event(sdk_kernel_event_runtime_err, &rt_err_ctx, sizeof(rt_err_ctx))))
	{
		if (NULL != rt_err_ctx.err_ctx)
			free(rt_err_ctx.err_ctx);
	}

	return rv;
}

mkernel_internal_error broadcast_user_event_reconnect_success()
{
	ezdev_sdk_kernel_inner_cb_notic *prt_inner_cb_notic = NULL;
	prt_inner_cb_notic = (ezdev_sdk_kernel_inner_cb_notic *)malloc(sizeof(ezdev_sdk_kernel_inner_cb_notic));
	if (NULL == prt_inner_cb_notic)
	{
		return mkernel_internal_malloc_error;
	}
	memset(prt_inner_cb_notic, 0, sizeof(ezdev_sdk_kernel_inner_cb_notic));
	prt_inner_cb_notic->cb_type = extend_cb_event;
	prt_inner_cb_notic->cb_event.event_type = sdk_kernel_event_reconnect_success;
	prt_inner_cb_notic->cb_event.event_context = NULL;

	return push_event_to_queue(prt_inner_cb_notic);
}

mkernel_internal_error das_keepalive_interval_changed_event_cb(int interval)
{
	mkernel_internal_error kernel_error = ezdev_sdk_kernel_succ;
	ezdev_sdk_kernel_inner_cb_notic *prt_inner_cb_notic = NULL;
	int *context = NULL;
	prt_inner_cb_notic = (ezdev_sdk_kernel_inner_cb_notic *)malloc(sizeof(ezdev_sdk_kernel_inner_cb_notic));
	if (NULL == prt_inner_cb_notic)
	{
		return mkernel_internal_malloc_error;
	}

	if (NULL == (context = (int *)malloc(sizeof(int))))
	{
		free(prt_inner_cb_notic);
		return mkernel_internal_malloc_error;
	}

	memset(prt_inner_cb_notic, 0, sizeof(ezdev_sdk_kernel_inner_cb_notic));
	*context = interval;
	prt_inner_cb_notic->cb_type = extend_cb_event;
	prt_inner_cb_notic->cb_event.event_type = sdk_kernel_event_heartbeat_interval_changed;
	prt_inner_cb_notic->cb_event.event_context = (void *)context;

	kernel_error = push_event_to_queue(prt_inner_cb_notic);
	ezdev_sdk_kernel_log_info(kernel_error, kernel_error, "heart interval chg");

	return kernel_error;
}
