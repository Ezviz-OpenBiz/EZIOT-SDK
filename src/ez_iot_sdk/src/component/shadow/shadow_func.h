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
* 2018-08-09     liuxiangchen
*******************************************************************************/


#ifndef H_SHADOW_FUNC_H_
#define H_SHADOW_FUNC_H_

#include <stdint.h>
#include "bscJSON.h"
#define SHADOW_SYNC_FLAG_USER 0
#define SHADOW_SYNC_FLAG_DEVICE 1

#ifdef __cplusplus
extern "C"
{
#endif

    int ezDevSDK_Shadow_Report_v3(char *devsn, char *res_type, int index, char *domain, char *key, void *json_value, unsigned int ver, unsigned int *seq);

    int Shadow_QuerySyncListToPlt_v3(char *devsn, char *res_type, int index);

    int Shadow_Check_Reply_v3(char *payload);

    int Shadow_Set_Reply_v3(char *devsn, char *res_type, int index, int code, int seq);

#ifdef __cplusplus
}
#endif

#endif
