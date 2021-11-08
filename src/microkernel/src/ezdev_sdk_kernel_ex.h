/*******************************************************************************
 * Copyright 漏 2017-2021 Ezviz Inc.
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
 * 2017/3/4       panlong
 *******************************************************************************/

#ifndef H_EZDEV_SDK_KERNEL_EX_H_
#define H_EZDEV_SDK_KERNEL_EX_H_

#include "ezdev_sdk_kernel_struct.h"
#include "ezdev_sdk_kernel_error.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 *  \brief		获取STUN服务器信息接口
 *  \method		ezdev_sdk_kernel_get_stun
 *  \param[in] 	stun_info * ptr_stun
 *  \return 	EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_get_stun(stun_info* ptr_stun, EZDEV_SDK_BOOL bforce_refresh);

/** 
 *  \brief		设置心跳时间
 *  \method		ezdev_sdk_kernel_set_keepalive_interval
 *  \param[in] 	EZDEV_SDK_UINT16 internal   设置心跳间隔时间
 *  \param[in]  EZDEV_SDK_UINT16 timeout_s  等待服务器响应的超时时间，0表示不等待
 *  \return 	如果不等待响应，信令送进队列返回成功，反之等待平台响应错误码，如果超时，返回等待信令超时。
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_set_keepalive_interval(EZDEV_SDK_UINT16 internal, EZDEV_SDK_UINT16 timeout_s);

#ifdef __cplusplus
}
#endif

#endif