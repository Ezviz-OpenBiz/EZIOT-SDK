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
#include "ezconfig.h"
#include "mcuconfig.h"

#include <string.h>
#include <stdlib.h>
#include "bscJSON.h"

#include "ble_msg_parse.h"
#include "ez_iot_log.h"
#include "ez_iot_ap.h"
#include "ez_iot_errno.h"
#include "drv_ble.h"
#include "hal_wifi_drv.h"
#include "hal_thread.h"
extern void set_product_country_code(char *CountryCode);

typedef enum
{
    WIFI_NOT_CONNECT = 0,
    WIFI_LIST_SCAN,
    WIFI_CONNECT_FAILED,
    WIFI_CONNECT_SUCCESS,
} wifi_status;

static wifi_info_cb g_wifi_cb;
static ez_iot_ap_wifi_info_t g_wifi_cb_info = {0};
static int g_wifi_connect_status = WIFI_NOT_CONNECT;

static const char *str_ble_net_func = "ble_net";
static const char *str_ble_access_dev_info = "access_dev_info";
static const char *str_ble_get_wifi_list = "get_wifi_list";
static const char *str_ble_set_wifi_config = "set_wifi_config";
static const char *security_mode[] = {
    "open",
    "WEP",
    "WPA-personal",
    "WPA2-personal",
    "WPA-WPA2-personal",
    "WPA2-enterprise"};

int set_wifi_info_cb(wifi_info_cb cb)
{
    g_wifi_cb = cb;
    return 0;
}

int get_wifi_config_status()
{
    return g_wifi_connect_status;
}

ez_iot_ap_wifi_info_t *get_wifi_config()
{
    return &g_wifi_cb_info;
}

static int ble_get_dev_info(uint8_t *rsp)
{
    int ret = 0;
    ez_iot_ap_dev_info_t *dev_info = get_ble_dev_info();
    bscJSON *js_root = NULL;
    bscJSON *js_value = NULL;
    do
    {
        js_root = bscJSON_CreateObject();
        if (NULL == js_root)
        {
            ez_log_e(TAG_BLE, "json create object error.");
            ret = -1;
            break;
        }

        bscJSON_AddStringToObject(js_root, "key", "dev_info");

        js_value = bscJSON_CreateObject();
        if (NULL == js_value)
        {
            ez_log_e(TAG_BLE, "js_value error.");
            ret = -1;
            break;
        }

        bscJSON_AddStringToObject(js_value, "ap_version", "1.0");
        bscJSON_AddStringToObject(js_value, "dev_subserial", dev_info->dev_serial);
        bscJSON_AddStringToObject(js_value, "dev_type", dev_info->dev_type);
        bscJSON_AddStringToObject(js_value, "dev_firmwareversion", dev_info->dev_version);

        bscJSON_AddItemToObject(js_root, "value", js_value);

        char *json_str = bscJSON_PrintUnformatted(js_root);
        if (NULL == json_str)
        {
            ez_log_e(TAG_BLE, "json print error.");
            ret = -1;
            break;
        }

        strcpy((char *)rsp, json_str);
        ez_log_d(TAG_BLE, "dev_info: %s", rsp);
        free(json_str);
    } while (false);

    if (NULL != js_root)
    {
        bscJSON_Delete(js_root);
    }
    return ret;
}

static int ble_get_wifi_list(uint8_t *rsp)
{
    int ret = 0;

    wifi_info_list_t ap_list[20] = {0};
    uint8_t wifi_list_num = 0;
    int ble_list_legal_num= 0;
    wifi_list_num = iot_sta_get_scan_list(BLE_WIFI_LIST_NUM, ap_list);
    if (wifi_list_num == 0)
    {
        ez_log_e(TAG_BLE, "get wifi list failed.");
        return -1;
    }

    bscJSON *js_root = NULL;
    bscJSON *js_wifi_list = NULL;

    do
    {
        js_root = bscJSON_CreateObject();
        if (NULL == js_root)
        {
            ez_log_e(TAG_BLE, "json create object error.");
            ret = -1;
            break;
        }

        bscJSON_AddStringToObject(js_root, "key", "wifi_list");

        bscJSON *js_value = bscJSON_CreateObject();
        if (NULL == js_value)
        {
            ez_log_e(TAG_BLE, "json create object error.");
            ret = -1;
            break;
        }
        bscJSON_AddItemToObject(js_root, "value", js_value);

        js_wifi_list = bscJSON_CreateArray();
        if (NULL == js_wifi_list)
        {
            ez_log_e(TAG_BLE, "wifi list error.");
            ret = -1;
            break;
        }
        bscJSON_AddItemToObject(js_value, "access_point_list", js_wifi_list);

        int i = 0;
        for (i = 0; i < wifi_list_num; ++i)
        {
            bscJSON *js_wifi = bscJSON_CreateObject();
            if (NULL == js_wifi)
            {
                continue;
            }
            if(0 != strlen((char*)ap_list[i].ssid))
            {
            	bscJSON_AddStringToObject(js_wifi, "ssid", (char *)ap_list[i].ssid);
            	bscJSON_AddNumberToObject(js_wifi, "signal_strength", ap_list[i].rssi);
            	bscJSON_AddStringToObject(js_wifi, "security_mode", security_mode[ap_list[i].authmode]);
            	bscJSON_AddItemToArray(js_wifi_list, js_wifi);
                ble_list_legal_num++;
            }
            else
            {
                ez_log_e(TAG_AP, "accesspointlist ssid is empty.");
            }
            if(20 == ble_list_legal_num)
            {
                printf("\n LW_PRINT DEBUG in line (%d) and function (%s)):the num is%d \n ",__LINE__, __func__,ble_list_legal_num);
                break;
            }
        }

        char *json_str = bscJSON_PrintUnformatted(js_root);
        if (NULL == json_str)
        {
            ez_log_e(TAG_BLE, "json print error.");
            ret = -1;
            break;
        }

        strcpy((char *)rsp, json_str);
        ez_log_d(TAG_BLE, "wifi list:%s.", rsp);
        free(json_str);

    } while (false);

    if (NULL != js_root)
    {
        bscJSON_Delete(js_root);
    }

    return ret;
}

static char *get_err_string(int err_code)
{
    switch (err_code)
    {
    case EZVIZ_WIFI_STATE_NOT_CONNECT:
        return "wifi not connect.";
    case EZVIZ_WIFI_STATE_CONNECT_SUCCESS:
        return "wifi connect success.";
    case EZVIZ_WIFI_STATE_PASSWORD_ERROR:
        return "wifi password error.";
    case EZVIZ_WIFI_STATE_NO_AP_FOUND:
        return "wifi no ap found.";
    case EZVIZ_WIFI_STATE_UNKNOW:
    default:
        return "wifi unknown error.";
    }
}

static int encapsulate_rsp_with_status(uint8_t *rsp, int wifi_status)
{
    bscJSON *json_root = bscJSON_CreateObject();
    if (NULL == json_root)
    {
        ez_log_d(TAG_BLE, "json create object error.");
        return -1;
    }

    bscJSON_AddStringToObject(json_root, "key", "conn_stat");

    bscJSON *json_value = bscJSON_CreateObject();
    bscJSON_AddNumberToObject(json_value, "status_code", wifi_status);
    bscJSON_AddStringToObject(json_value, "status_string", get_err_string(wifi_status));

    bscJSON_AddItemToObject(json_root, "value", json_value);

    char *json_str = bscJSON_PrintUnformatted(json_root);
    if (NULL == json_str)
    {
        ez_log_d(TAG_BLE, "json print error.");
        bscJSON_Delete(json_root);
        return -1;
    }

    memcpy(rsp, json_str, strlen(json_str));
    bscJSON_Delete(json_root);
    free(json_str);

    ez_log_d(TAG_BLE, "wifi config rsp: %s", rsp);

    return 0;
}

static int ble_process_wifi_config_rsp(uint8_t *rsp)
{
    int ret = 0;
    int wifi_status;
    for (int i = 0; i < 10; i++)
    {
        hal_thread_sleep(1000);
        wifi_status = iot_sta_get_state();
        if (wifi_status == EZVIZ_WIFI_STATE_CONNECT_SUCCESS)
        {
            break;
        }
    }

    if (EZVIZ_WIFI_STATE_CONNECT_SUCCESS != wifi_status)
    {
        iot_sta_disconnect();

        g_wifi_connect_status = WIFI_CONNECT_FAILED;
        g_wifi_cb_info.err_code = ez_errno_ap_connect_failed;
        g_wifi_cb(&g_wifi_cb_info);

        ez_log_i(TAG_BLE, "wifi connect failed, wifi time reset. err_code:0x%x.", g_wifi_cb_info.err_code);
    }
    else
    {
        g_wifi_connect_status = WIFI_CONNECT_SUCCESS;
    }

    ret = encapsulate_rsp_with_status(rsp, wifi_status);
    if (0 != ret)
    {
        ez_log_e(TAG_BLE, "encapsulate rsp error.");
        return ret;
    }

    return ret;
}
static int ble_set_wifi_config(bscJSON *js_value)
{
    int ret = 0;

    do
    {
        bscJSON *js_token = bscJSON_GetObjectItem(js_value, "token");
        if (NULL == js_token || bscJSON_String != js_token->type)
        {
            ez_log_e(TAG_BLE, "token absent.");
            ret = -1;
            break;
        }
        strcpy(g_wifi_cb_info.token, js_token->valuestring);

		bscJSON *js_cc = NULL;
      
	    js_cc = bscJSON_GetObjectItem(js_value, "areaCode");
	    if(NULL != js_cc)
	    {
	        if(NULL != js_cc->valuestring)
	        {
	            memcpy(g_wifi_cb_info.cc, js_cc->valuestring, sizeof(g_wifi_cb_info.cc));
	            if (g_wifi_cb_info.cc[3] != '\0')
	            {
	                g_wifi_cb_info.cc[3] = '\0';
	            }
	        }
	    }

        bscJSON *js_domain = bscJSON_GetObjectItem(js_value, "lbs_domain");
        if (NULL == js_domain || bscJSON_String != js_domain->type)
        {
            ez_log_e(TAG_BLE, "lbs_domain absent.");
            ret = -1;
            break;
        }
        strcpy(g_wifi_cb_info.domain, js_domain->valuestring);

        bscJSON *js_devid = bscJSON_GetObjectItem(js_value, "device_id");
        if (NULL != js_devid)
        {
            strcpy(g_wifi_cb_info.device_id, js_devid->valuestring);
        }

        bscJSON *js_wifi_info = bscJSON_GetObjectItem(js_value, "wifi_info");
        if (NULL == js_wifi_info || bscJSON_Object != js_wifi_info->type)
        {
            ez_log_e(TAG_BLE, "wifi_info absent.");
            ret = -1;
            break;
        }

        bscJSON *js_ssid = bscJSON_GetObjectItem(js_wifi_info, "ssid");
        if (NULL == js_ssid || bscJSON_String != js_ssid->type)
        {
            ez_log_e(TAG_BLE, "ssid absent.");
            ret = -1;
            break;
        }
        strcpy(g_wifi_cb_info.ssid, js_ssid->valuestring);

        bscJSON *js_passwd = bscJSON_GetObjectItem(js_wifi_info, "password");
        if (NULL == js_passwd || bscJSON_String != js_passwd->type)
        {
            ez_log_e(TAG_BLE, "password absent.");
            ret = -1;
            break;
        }
        strcpy(g_wifi_cb_info.password, js_passwd->valuestring);

		set_product_country_code(g_wifi_cb_info.cc);       //set国家码

        iot_sta_connect(g_wifi_cb_info.ssid, g_wifi_cb_info.password);

    } while (false);

    return ret;
}

static int ble_net_func_dispatch(bscJSON *js_desc, uint8_t *rsp)
{
    int ret = 0;
    do
    {
        bscJSON *js_key = NULL;
        js_key = bscJSON_GetObjectItem(js_desc, "key");
        if (NULL == js_key)
        {
            ez_log_e(TAG_BLE, "key absent.");
            ret = -1;
            break;
        }
        if (bscJSON_String != js_key->type)
        {
            ez_log_e(TAG_BLE, "key type error.");
            ret = -1;
            break;
        }

        if (0 == strcmp(js_key->valuestring, str_ble_access_dev_info))
        {
            g_wifi_connect_status = WIFI_NOT_CONNECT;

            g_wifi_cb_info.err_code = ez_errno_ap_app_connected;
            g_wifi_cb(&g_wifi_cb_info);
            ez_log_i(TAG_BLE, "app connect success. err_code:0x%x.", g_wifi_cb_info.err_code);

            ret = ble_get_dev_info(rsp);
            if (0 != ret)
            {
                ez_log_e(TAG_BLE, "get dev_info error.");
                ret = -1;
                break;
            }
        }
        else if (0 == strcmp(js_key->valuestring, str_ble_get_wifi_list))
        {
            g_wifi_connect_status = WIFI_LIST_SCAN;

            ret = ble_get_wifi_list(rsp);
            if (0 != ret)
            {
                ez_log_e(TAG_BLE, "get wifi_list error.");
                ret = -1;
                break;
            }
        }
        else if (0 == strcmp(js_key->valuestring, str_ble_set_wifi_config))
        {
            bscJSON *js_value = bscJSON_GetObjectItem(js_desc, "value");
            if (NULL == js_value)
            {
                ez_log_e(TAG_BLE, "value absent.");
                ret = -1;
                break;
            }
            if (bscJSON_Object != js_value->type)
            {
                ez_log_e(TAG_BLE, "value type absent.");
                ret = -1;
                break;
            }

            g_wifi_cb_info.err_code = ez_errno_ap_connecting_route;
            g_wifi_cb(&g_wifi_cb_info);
            ez_log_i(TAG_BLE, "connecting route..., err_code:0x%x.", g_wifi_cb_info.err_code);

            ret = ble_set_wifi_config(js_value);
            if (0 != ret)
            {
                ez_log_e(TAG_BLE, "set wifi config error.");
                ret = -1;
                break;
            }

            ret = ble_process_wifi_config_rsp(rsp);
        }
    } while (false);

    return ret;
}

int process_ble_request(const uint8_t *req, uint32_t req_len, uint8_t *rsp)
{
    //ez_log_i(TAG_BLE, "ble request: %s", req);
    int ret = 0;
    bscJSON *js_root = NULL;
    do
    {
        js_root = bscJSON_Parse((char *)req);
        if (NULL == js_root)
        {
            ez_log_e(TAG_BLE, "json parse error.");
            ret = -1;
            break;
        }

        bscJSON *js_func = NULL;
        bscJSON *js_desc = NULL;

        js_func = bscJSON_GetObjectItem(js_root, "func");
        if (NULL == js_func)
        {
            ez_log_e(TAG_BLE, "func absent.");
            ret = -1;
            break;
        }
        if (bscJSON_String != js_func->type)
        {
            ez_log_e(TAG_BLE, "func type error.");
            ret = -1;
            break;
        }

        js_desc = bscJSON_GetObjectItem(js_root, "desc");
        if (NULL == js_desc)
        {
            ez_log_e(TAG_BLE, "desc absent.");
            ret = -1;
            break;
        }

        if (0 == strcmp(js_func->valuestring, str_ble_net_func))
        {
            ret = ble_net_func_dispatch(js_desc, rsp);
            if (0 != ret)
            {
                ez_log_e(TAG_BLE, "ble_net func process error.");
                ret = -1;
                break;
            }
        }

    } while (false);

    if (NULL != js_root)
    {
        bscJSON_Delete(js_root);
    }

    return ret;
}