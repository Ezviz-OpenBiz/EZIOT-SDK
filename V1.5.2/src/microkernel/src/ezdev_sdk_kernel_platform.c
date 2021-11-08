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
#include "ezdev_sdk_kernel_platform.h"
#include "sdk_kernel_def.h"
#include "ezconfig.h"
#include "mcuconfig.h"

extern ezdev_sdk_kernel g_ezdev_sdk_kernel;

ezdev_sdk_mutex ezdev_sdk_kernel_platform_thread_mutex_create()
{
	return g_ezdev_sdk_kernel.platform_handle.thread_mutex_create();
}

void  ezdev_sdk_kernel_platform_thread_mutex_destroy(ezdev_sdk_mutex ptr_mutex)
{
	g_ezdev_sdk_kernel.platform_handle.thread_mutex_destroy(ptr_mutex);
}

int ezdev_sdk_kernel_platform_thread_mutex_lock(ezdev_sdk_mutex ptr_mutex)
{
	return g_ezdev_sdk_kernel.platform_handle.thread_mutex_lock(ptr_mutex);
}

int ezdev_sdk_kernel_platform_thread_mutex_unlock(ezdev_sdk_mutex ptr_mutex)
{
	return g_ezdev_sdk_kernel.platform_handle.thread_mutex_unlock(ptr_mutex);
}