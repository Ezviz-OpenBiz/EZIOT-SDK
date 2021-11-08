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
 * 2017/3/4       panlong
 *******************************************************************************/
#ifndef H_EZDEV_SDK_KERNEL_ACCESS_H_
#define H_EZDEV_SDK_KERNEL_ACCESS_H_

#define EZDEV_SDK_KERNEL_ACCESS_INTERFACE  \
	extern mkernel_internal_error stop_yield(ezdev_sdk_kernel* sdk_kernel); \
	extern mkernel_internal_error access_server_yield(ezdev_sdk_kernel* sdk_kernel);\
    extern mkernel_internal_error stop_das_logout(ezdev_sdk_kernel* sdk_kernel); \
    extern mkernel_internal_error stop_recieve_send_msg(ezdev_sdk_kernel* sdk_kernel); \
    extern mkernel_internal_error send_offline_msg_to_platform(EZDEV_SDK_UINT32 seq);

#endif //H_EZDEV_SDK_KERNEL_ACCESS_H_