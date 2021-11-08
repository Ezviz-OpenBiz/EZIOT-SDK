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
#ifndef H_EZDEV_SDK_KERNEL_PLATFORM_H_
#define H_EZDEV_SDK_KERNEL_PLATFORM_H_

#define EZDEV_SDK_KERNEL_PLATFORM_INTERFACE	\
	extern ezdev_sdk_mutex ezdev_sdk_kernel_platform_thread_mutex_create();	\
	extern void  ezdev_sdk_kernel_platform_thread_mutex_destroy(ezdev_sdk_mutex ptr_mutex); \
	extern int ezdev_sdk_kernel_platform_thread_mutex_lock(ezdev_sdk_mutex ptr_mutex); \
	extern int ezdev_sdk_kernel_platform_thread_mutex_unlock(ezdev_sdk_mutex ptr_mutex);
#endif