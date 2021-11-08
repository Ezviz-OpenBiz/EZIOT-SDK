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
#ifndef H_EZDEV_SDK_KERNEL_INNER_H_
#define H_EZDEV_SDK_KERNEL_INNER_H_

#include "ezdev_sdk_kernel_struct.h"
#include "ezdev_sdk_kernel_error.h"

#ifdef __cplusplus
extern "C"
{
#endif

	/** 
	 *  \brief		����SDK���汾��(ֻ������SDK���ã�������΢�ں˵Ļ�����Ҫ����) �ڵ���ezdev_sdk_kernel_start֮ǰ����
	 *  \method		ezdev_sdk_kernel_set_sdk_main_version
	 *  \param[in] 	char szMainVersion[ezdev_sdk_extend_name_len]
	 *  \return 	EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error
	 */
	EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_set_sdk_main_version(char szMainVersion[version_max_len]);

#ifdef __cplusplus
}
#endif

#endif