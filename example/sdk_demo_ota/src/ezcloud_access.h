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
  2021-03-10     xurongjun
 *******************************************************************************/
#ifndef _EZCLOUD_ACCESS_H_
#define _EZCLOUD_ACCESS_H_

#ifdef __cplusplus
extern "C"
{
#endif

    int ez_cloud_init();

    int ez_cloud_start();

    const char *ez_cloud_get_sn();

    const char *ez_cloud_get_ver();

    const char *ez_cloud_get_type();

    void ez_cloud_deint();

#ifdef __cplusplus
}
#endif

#endif