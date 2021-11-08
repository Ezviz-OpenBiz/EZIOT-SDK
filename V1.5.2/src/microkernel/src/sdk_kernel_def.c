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
#include "sdk_kernel_def.h"
#include <stdarg.h>
#include "ezconfig.h"
#include "mcuconfig.h"

ezdev_sdk_kernel g_ezdev_sdk_kernel;
char g_binding_nic[ezdev_sdk_name_len] = {0}; ///<	设备绑定的本地网卡名称

void ezdev_sdk_kernel_log (sdk_log_level level, int sdk_error, int othercode, \
						   const char *fmt, ...)
{
	va_list ap;
	char logbuf[512];
	memset(logbuf, 0, 512);

	if (g_ezdev_sdk_kernel.platform_handle.sdk_kernel_log == NULL)
	{
		return;
	}

	va_start(ap, fmt);
	vsnprintf(logbuf, 512, fmt, ap);
	va_end(ap);

	g_ezdev_sdk_kernel.platform_handle.sdk_kernel_log(level, sdk_error, othercode, logbuf);
}

ezdev_sdk_kernel *get_ezdev_sdk_kernel()
{
	return &g_ezdev_sdk_kernel;
}