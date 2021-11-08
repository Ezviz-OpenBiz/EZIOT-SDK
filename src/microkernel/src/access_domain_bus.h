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
 * mqtt adaptation interface declaration
 * 
 * Change Logs:
 * Date           Author       Notes

 *******************************************************************************/

#ifndef H_ACCESS_DOMAIN_BUS_H_
#define H_ACCESS_DOMAIN_BUS_H_

#define ACCESS_DOMAIN_BUS_INTERFACE	 \
	mkernel_internal_error access_domain_bus_handle(const ezdev_sdk_kernel_submsg* ptr_submsg);

#endif