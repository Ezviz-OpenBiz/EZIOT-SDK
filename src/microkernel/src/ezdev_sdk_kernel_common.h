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
#ifndef H_EZDEV_SDK_KERNEL_COMMON_H_
#define H_EZDEV_SDK_KERNEL_COMMON_H_


#define  EZDEV_SDK_KERNEL_COMMON_INTERFACE	\
	extern void common_module_init(void); \
	extern void common_module_fini(void); \
	extern mkernel_internal_error common_module_load(const ezdev_sdk_kernel_common_module* common_module); \
	extern EZDEV_SDK_INT8 common_module_bus_handle(ezdev_sdk_kernel_submsg* ptr_submsg);

#endif