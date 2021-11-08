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
#include <stdio.h>
#include "ezconfig.h"
#include "mcuconfig.h"
#include "bscJSON.h"
#include "shadow.h"
#include "shadow_func.h"
#include "shadow_extern.h"
#include "ezdev_sdk_kernel.h"
#include "ez_iot_log.h"
#include "shadow_def.h"

typedef int (*shadow_handle_platform_req_and_rsp)(const char *req, char *rsp, int rsp_len);

#define EZDEVSDK_SHADOW_RSP 1
#define EZDEVSDK_SHADOW_REQ 0

int Shadow_QuerySyncListToPlt_v3(char *devsn, char *res_type, int index)
{
    char *protocal_query = "{\"method\":\"query\",\"windowsize\":%d}";
    char buf[128] = {0};

    snprintf(buf, sizeof(buf), protocal_query, PUSH_BUF_MAX_LEN);

    ezdev_sdk_kernel_pubmsg_v3 pubmsg = {0};
    pubmsg.msg_qos = QOS_T0;
    pubmsg.msg_seq = 0;
    pubmsg.msg_body_len = strlen(buf);
    pubmsg.extend_id = sdk_shadow;
    pubmsg.msg_body = (unsigned char *)buf;

    strncpy(pubmsg.sub_serial, devsn, sizeof(pubmsg.sub_serial) - 1);
    sprintf(pubmsg.resource_id, "%d", index);
    strncpy(pubmsg.resource_type, res_type, sizeof(pubmsg.resource_type) - 1);
    strncpy(pubmsg.business_type, "query", sizeof(pubmsg.business_type) - 1);

    int ret = ezdev_sdk_kernel_send_v3(&pubmsg);
    if (ez_errno_succ != ret)
    {
        ez_log_e(TAG_SHADOW, "sdk kernel send v3 failed.%d", ret);
    }

    return ret;
}

int ezDevSDK_Shadow_Report_v3(char *devsn, char *res_type, int index, char *domain, char *key, void *json_value, unsigned int ver, unsigned int *seq)
{
    int rv = -1;
    bscJSON *pstJsRoot = bscJSON_CreateObject();
    bscJSON *pstJsState = bscJSON_CreateObject();
    bscJSON *pstReported = bscJSON_CreateObject();
    ezdev_sdk_kernel_pubmsg_v3 pubmsg = {0};

    do
    {
        if (!pstJsRoot || !pstJsState || !pstReported)
        {
            bscJSON_Delete((bscJSON *)json_value);
            break;
        }
        bscJSON_AddStringToObject(pstReported, "domain", domain);
        bscJSON_AddStringToObject(pstReported, "identifier", key);
        bscJSON_AddItemToObject(pstReported, "value", (bscJSON *)json_value);
        bscJSON_AddItemToObject(pstJsState, "reported", pstReported);
        bscJSON_AddNumberToObject(pstJsState, "version", ver);
        bscJSON_AddStringToObject(pstJsRoot, "method", "report");
        bscJSON_AddNumberToObject(pstJsRoot, "spv", 3);
        bscJSON_AddItemToObject(pstJsRoot, "state", pstJsState);
        pubmsg.msg_body = (unsigned char *)bscJSON_PrintUnformatted(pstJsRoot);
        if (NULL == pubmsg.msg_body)
        {
            break;
        }

        pubmsg.msg_qos = QOS_T1;
        pubmsg.msg_seq = 0;
        pubmsg.msg_body_len = strlen((char *)pubmsg.msg_body);
        pubmsg.extend_id = sdk_shadow;

        strncpy(pubmsg.sub_serial, devsn, sizeof(pubmsg.sub_serial) - 1);
        sprintf(pubmsg.resource_id, "%d", index);
        strncpy(pubmsg.resource_type, res_type, sizeof(pubmsg.resource_type) - 1);
        strncpy(pubmsg.business_type, "report", sizeof(pubmsg.business_type) - 1);
        
        if (ez_errno_succ != (rv = ezdev_sdk_kernel_send_v3(&pubmsg)))
        {
            ez_log_e(TAG_SHADOW, "kernel send v3%d", rv);
            break;
        }

        *seq = pubmsg.msg_seq;
    } while (0);

    if (pstJsRoot)
    {
        bscJSON_Delete(pstJsRoot);
    }

    if (pubmsg.msg_body)
    {
        free(pubmsg.msg_body);
    }

    return rv;
}

int Shadow_Check_Reply_v3(char *payload)
{
    bscJSON *json_root = NULL;
    bscJSON *json_payload = NULL;
    bscJSON *json_code = NULL;
    int rv = -1;

    do
    {
        if (NULL == (json_root = bscJSON_Parse(payload)))
        {
            break;
        }

        if (NULL == (json_payload = bscJSON_GetObjectItem(json_root, "payload")))
        {
            break;
        }

        if (NULL == (json_code = bscJSON_GetObjectItem(json_payload, "code")))
        {
            break;
        }

        if (0 != json_code->valueint)
        {
            break;
        }

        rv = 0;
    } while (0);

    if (0 != rv)
    {
        ez_log_e(TAG_SHADOW, "reply invalid!");
        ez_log_d(TAG_SHADOW, "reply:%s", payload);
    }

    return rv;
}

int Shadow_Set_Reply_v3(char *devsn, char *res_type, int index, int code, int seq)
{
    char protocal_reply[256] = {0};
    sprintf(protocal_reply, "{\"method\":\"set_reply\",\"payload\":{\"code\":%d},\"timestamp\":1469564576}", code);

    ezdev_sdk_kernel_pubmsg_v3 pubmsg = {0};
    pubmsg.msg_response = 1;
    pubmsg.msg_qos = QOS_T0;
    pubmsg.msg_seq = seq;
    pubmsg.msg_body_len = strlen(protocal_reply);
    pubmsg.extend_id = sdk_shadow;
    pubmsg.msg_body = (unsigned char *)protocal_reply;

    strncpy(pubmsg.sub_serial, devsn, sizeof(pubmsg.sub_serial) - 1);
    sprintf(pubmsg.resource_id, "%d", index);
    strncpy(pubmsg.resource_type, res_type, sizeof(pubmsg.resource_type) - 1);
    strncpy(pubmsg.business_type, "set_reply", sizeof(pubmsg.business_type) - 1);

    int ret = ezdev_sdk_kernel_send_v3(&pubmsg);
    if (ez_errno_succ != ret)
    {
        ez_log_e(TAG_SHADOW, "sdk kernel send v3 failed.%d", ret);
    }

    return ret;
}