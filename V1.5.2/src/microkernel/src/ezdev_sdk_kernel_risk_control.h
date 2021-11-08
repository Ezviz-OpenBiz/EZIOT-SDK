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
#ifndef H_EZDEV_SDK_KERNEL_RISK_CONTROL_H_
#define H_EZDEV_SDK_KERNEL_RISK_CONTROL_H_

#define EZDEV_SDK_KERNEL_RISK_CONTROL_INTERFACE		\
	extern void add_access_risk_control(ezdev_sdk_kernel* sdk_kernel);\
	extern void add_domain_risk_control(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT32 domain_id);\
	extern void add_cmd_risk_control(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT32 domain_id, EZDEV_SDK_UINT32 cmd_id);\
	extern char check_access_risk_control(ezdev_sdk_kernel* sdk_kernel);\
	extern char check_cmd_risk_control(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT32 domain_id, EZDEV_SDK_UINT32 cmd_id);

#endif