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
#ifndef H_DAS_TRANSPORT_H_
#define H_DAS_TRANSPORT_H_

#define DAS_TRANSPORT_INTERFACE	 \
	extern void das_object_init(ezdev_sdk_kernel* sdk_kernel);\
	extern void das_object_fini(ezdev_sdk_kernel* sdk_kernel);\
	extern mkernel_internal_error das_reg(ezdev_sdk_kernel* sdk_kernel); \
	extern mkernel_internal_error das_unreg(ezdev_sdk_kernel* sdk_kernel); \
	extern mkernel_internal_error das_light_reg(ezdev_sdk_kernel* sdk_kernel); \
	extern mkernel_internal_error das_light_reg_v2(ezdev_sdk_kernel * sdk_kernel);\
    extern mkernel_internal_error das_light_reg_v3(ezdev_sdk_kernel* sdk_kernel);\
	extern mkernel_internal_error das_yield(ezdev_sdk_kernel* sdk_kernel); \
	extern mkernel_internal_error das_send_pubmsg_async(ezdev_sdk_kernel* sdk_kernel, ezdev_sdk_kernel_pubmsg_exchange* msg_exchange); \
	extern mkernel_internal_error das_send_pubmsg_async_v3(ezdev_sdk_kernel* sdk_kernel, ezdev_sdk_kernel_pubmsg_exchange_v3* msg_exchange); \
	extern mkernel_internal_error das_change_keep_alive_interval(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT16 interval); \
	int ezdev_sdk_kernel_get_das_socket(ezdev_sdk_kernel* sdk_kernel);
#endif