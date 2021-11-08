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

#include "prenetwork_root.h"
#include "dna_wlan.h"
#include "ez_iot_log.h"

static const char security_mode[][32] = {
    "open",
    "WEP",
    "WPA-personal",
    "WPA2-personal",
    "WPA-WPA2-personal",
    "WPA2-enterprise"
    };

dna_wlan_scan_result_t g_ap_list[MAX_AP_LIST_NUM] = {0};
int g_ap_list_num = 0;

static int scan_result_cb(unsigned int count)
{
    if (count > MAX_AP_LIST_NUM)
    {
        count = MAX_AP_LIST_NUM;
    }
    g_ap_list_num = count;
    int i = 0;
    for (i = 0; i < count; i++)
    {
        dna_wlan_get_scan_result(i, &g_ap_list[i]);
    }
    return 0;
}

INT32 prenetwork_get_wireless_server_cfg(AP_NET_DES *ap_net, CGI_PAGE *page)
{
    if(NULL == ap_net || NULL == page)
    {
        ez_log_d(TAG_AP, "wlan scan param error.");
        return COMBSTA_DEV_ERR;
    }

    if (0 != dna_wlan_scan(scan_result_cb))
    {
        ez_log_d(TAG_AP, "wlan scan start failed.");
        return COMBSTA_DEV_ERR;
    }
    
    bscJSON *AccessPointList = NULL;

    page->json_root = bscJSON_CreateObject();
    if(NULL == page->json_root)
    {
        ez_log_d(TAG_AP, "json create object error.");
        return COMBSTA_DEV_ERR;
    }

    AccessPointList = bscJSON_CreateArray();
    if(NULL == AccessPointList)
    {
        ez_log_d(TAG_AP, "no AccessPointList param.");
        bscJSON_Delete(page->json_root);
        return COMBSTA_DEV_ERR;
    }

    int i = 0;
    for (i = 0; i < g_ap_list_num; ++i)
    {
        bscJSON *AccessPoint = bscJSON_CreateObject();
        if(NULL == AccessPoint)
        {
            continue;
        }

        bscJSON_AddStringToObject(AccessPoint, "ssid", (char*)g_ap_list[i].ssid);
        bscJSON_AddNumberToObject(AccessPoint, "signal_strength", g_ap_list[i].rssi);

        int authmode = 0;
        if (g_ap_list[i].wpa2 == 1)
        {
            authmode = 3;
        }
        else if (g_ap_list[i].wpa == 1)
        {
            authmode = 2;
        }
        else if (g_ap_list[i].wep == 1)
        {
            authmode = 1;
        }
        else
        {
            authmode = 0;
        }
        bscJSON_AddStringToObject(AccessPoint, "security_mode", security_mode[authmode]);
        bscJSON_AddItemToArray(AccessPointList, AccessPoint);
    }
    bscJSON_AddItemToObject(page->json_root,"access_point_list",AccessPointList);

    char *json_str = bscJSON_PrintUnformatted(page->json_root);
    if (NULL == json_str)
    {
        ez_log_d(TAG_AP, "Json print error.");
        bscJSON_Delete(page->json_root);
        return COMBSTA_DEV_ERR;
    }
    memcpy(ap_net->resp.httpbody, json_str, strlen(json_str));
    bscJSON_Delete(page->json_root);
    free(json_str);

    ez_log_d(TAG_AP, "list of scanned wifi:%s", ap_net->resp.httpbody);
    return COMBSTA_OK;
}

int pre_SecurityAndAccessPoint_basic(AP_NET_DES *ap_net, CGI_PAGE *page)
{
    INT32 ret = COMBSTA_OK;
        
    if(NULL == ap_net || NULL == page)
    {
        ez_log_d(TAG_AP, "wlan scan basic param NULL.");
        return COMBSTA_DEV_ERR;
    }

    if( METHOD_GET == ap_net->req.method)
    {

        ret = prenetwork_get_wireless_server_cfg(ap_net, page);
        if(COMBSTA_OK != ret)
        {
            ez_log_d(TAG_AP, "Get wlan list failed.");
            return ret; 
        }

        return COMBSTA_OK;
    }
    else
    {
        return COMBSTA_METHOD_NOT_ALLOWED;
    }
}

INT32 pre_put_WifiConfig_json(AP_NET_DES *ap_net, bscJSON *jsonDoc)
{
    if(NULL == ap_net || NULL == jsonDoc)
    {
        ez_log_d(TAG_AP, "put wifi config param NULL.");
        
        return COMBSTA_DEV_ERR;
    }
        
    bscJSON *js_token= NULL;
    bscJSON *js_domain = NULL;
    bscJSON *js_devid = NULL;
    bscJSON *js_wifiinfo = NULL;
    bscJSON *js_ssid = NULL;
    bscJSON *js_passwd = NULL;
    js_token = bscJSON_GetObjectItem(jsonDoc, "token");
    if(NULL != js_token)
    {
        if(NULL != js_token->valuestring)
        {
            memcpy(ap_net->wifi_info.token, js_token->valuestring, MINNUM(sizeof(ap_net->wifi_info.token), strlen(js_token->valuestring)));
        }
    }
    else
    {
        ez_log_d(TAG_AP, "token is absent.");

        bscJSON_Delete(jsonDoc);
        return COMBSTA_INVAL_PARAM;
    }

    js_domain = bscJSON_GetObjectItem(jsonDoc, "lbs_domain");
    if(NULL != js_domain)
    {
        if(NULL != js_domain->valuestring)
        {
            memcpy(ap_net->wifi_info.domain, js_domain->valuestring, MINNUM(sizeof(ap_net->wifi_info.domain), strlen(js_domain->valuestring)));
        }
    }
    else
    {
        ez_log_d(TAG_AP, "lbs_domain absent.");
        bscJSON_Delete(jsonDoc);
        return COMBSTA_INVAL_PARAM;
    }

    js_devid = bscJSON_GetObjectItem(jsonDoc, "device_id");
    if(NULL != js_devid)
    {
        if(NULL != js_devid->valuestring)
        {
            memcpy(ap_net->wifi_info.device_id, js_devid->valuestring, MINNUM(sizeof(ap_net->wifi_info.device_id), strlen(js_devid->valuestring)));
        }
    }

    js_wifiinfo = bscJSON_GetObjectItem(jsonDoc, "wifi_info");
    if(NULL == js_wifiinfo)
    {
        ez_log_d(TAG_AP, "wifi_info absent.");
        bscJSON_Delete(jsonDoc);
        return COMBSTA_INVAL_PARAM;
    }

    js_ssid = bscJSON_GetObjectItem(js_wifiinfo, "ssid");
    if(NULL != js_ssid)
    {
        if(NULL != js_ssid->valuestring)
        {
            (void)memcpy(ap_net->wifi_info.ssid, js_ssid->valuestring, MINNUM(sizeof(ap_net->wifi_info.ssid), strlen(js_ssid->valuestring)));
        }
    }
    else
    {
        ez_log_d(TAG_AP, "ssid absent.");
        bscJSON_Delete(jsonDoc);
        return COMBSTA_INVAL_PARAM;
    }

    js_passwd = bscJSON_GetObjectItem(js_wifiinfo, "password");
    if(NULL != js_passwd)
    {
        if(NULL != js_passwd->valuestring)
        {
            memcpy(ap_net->wifi_info.password, js_passwd->valuestring, MINNUM(sizeof(ap_net->wifi_info.password), strlen(js_passwd->valuestring)));
        }
    }
    else
    {
        ez_log_d(TAG_AP, "password absent.");
        bscJSON_Delete(jsonDoc);
        return COMBSTA_INVAL_PARAM;
    }
    return COMBSTA_OK;
}

int pre_WifiConfig_basic(AP_NET_DES *ap_net, CGI_PAGE *page)
{
    AP_NET_REQ_DES *req = NULL;
    
    if(NULL == ap_net || NULL == page)
    {
        ez_log_d(TAG_AP, "put wifi config basic param NULL.");
        return COMBSTA_DEV_ERR;
    }

    req = &(ap_net->req);
    if( METHOD_PUT == req->method )
    {

        if(COMBSTA_OK != pre_put_WifiConfig_json(ap_net, req->jsonDoc))
        {
            ez_log_d(TAG_AP, "put wifi config error.");
            return COMBSTA_DEV_ERR;
        }

        return COMBSTA_OK;
    }
    else
    {
        return COMBSTA_METHOD_NOT_ALLOWED;
    }
}
