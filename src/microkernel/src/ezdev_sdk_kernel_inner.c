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
#include "ezdev_sdk_kernel_inner.h"
#include "sdk_kernel_def.h"
#include "ezconfig.h"
#include "mcuconfig.h"

extern ezdev_sdk_kernel g_ezdev_sdk_kernel;

EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_set_sdk_main_version( char szMainVersion[version_max_len] )
{
	strncpy(g_ezdev_sdk_kernel.szMainVersion, szMainVersion, version_max_len - 1);
	return ezdev_sdk_kernel_succ;
}