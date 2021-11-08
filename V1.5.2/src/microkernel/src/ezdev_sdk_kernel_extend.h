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
#ifndef H_EZDEV_SDK_KERNEL_EXTEND_H_
#define H_EZDEV_SDK_KERNEL_EXTEND_H_


#define EZDEV_SDK_KERNEL_EXTEND_INTERFACE	\
	extern void extend_init(sdk_kernel_event_notice kernel_event_notice_cb); \
	extern void extend_fini(void); \
	extern mkernel_internal_error extend_load(const ezdev_sdk_kernel_extend* external_extend); \
	extern mkernel_internal_error extend_load_v3(const ezdev_sdk_kernel_extend_v3* external_extend_v3); \
	extern mkernel_internal_error extend_yield(ezdev_sdk_kernel* sdk_kernel);\
	extern ezdev_sdk_kernel_domain_info* extend_get(EZDEV_SDK_UINT32 domain_id);  \
	extern ezdev_sdk_kernel_domain_info_v3* extend_get_by_extend_id(EZDEV_SDK_UINT16 extend_id); \
	extern mkernel_internal_error sdk_hub_extend_load(const ezdev_sdk_kernel_hub_extend * extend_info);\
	extern ezdev_sdk_kernel_hub_extend* sdk_hub_extend_get(EZDEV_SDK_UINT16 extend_id);  \
	extern mkernel_internal_error extend_serialize_sdk_version(bscJSON * pJsonRoot);  \
    extern mkernel_internal_error clear_queue_pubmsg_exchange(); \
	extern mkernel_internal_error clear_queue_submsg(); \
	extern mkernel_internal_error clear_queue_pubmsg_exchange_v3(); \
	extern mkernel_internal_error clear_queue_submsg_v3(); \

	
#endif //H_EZDEV_SDK_KERNEL_EXTEND_H_