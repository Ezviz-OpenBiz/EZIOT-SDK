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
 *2021-03-11      xurongjun
 *******************************************************************************/

#include <stdlib.h>
#include "flashdb.h"
#include <string.h>
#include "ut_config.h"
#include "ez_iot.h"
#include "ez_iot_hub.h"
#include "ez_iot_log.h"
#include "ez_hal/hal_thread.h"
#include "kv_imp.h"
#include "ez_iot_ap.h"
#include "ez_iot_errno.h"
#include "utest.h"

#include "bscJSON.h"

const char *TAG_UT_AP = "UT_AP";

static void wifi_cb(ez_iot_ap_wifi_info_t *wifi_info)
{
    switch (wifi_info->err_code)
    {
    case ez_errno_ap_app_connected:
        ez_log_w(TAG_UT_AP, "app connected.");
        break;
    case ez_errno_succ:
        ez_log_w(TAG_UT_AP, "wifi config success.");
        ez_log_i(TAG_UT_AP, "ssid: %s", wifi_info->ssid);
        ez_log_i(TAG_UT_AP, "password: %s", wifi_info->password);
        ez_log_i(TAG_UT_AP, "token: %s", wifi_info->token);
        ez_log_i(TAG_UT_AP, "domain: %s", wifi_info->domain);
        break;
    case ez_errno_ap_connecting_route:
        ez_log_w(TAG_UT_AP, "connecting route.");
        break;
    case ez_errno_ap_connect_failed:
        ez_log_w(TAG_UT_AP, "connect failed.");
        break;
    case ez_errno_ap_wifi_config_timeout:
        ez_log_w(TAG_UT_AP, "wifi config timeout.");
        ez_iot_ap_finit();
        break;
    default:
        break;
    }
}

ez_err_e ut_ap_init()
{
    bscJSON *js_root = bscJSON_CreateObject();
    bscJSON_AddStringToObject(js_root, "ssid", "aaaaaaaaaaaa");
    bscJSON_AddStringToObject(js_root, "security_mode", "open");
    bscJSON_AddNumberToObject(js_root, "signal_strength", 38);
    char *js_str = bscJSON_PrintUnformatted(js_root);
    if (NULL != js_str)
    {
        ez_log_w(TAG_UT_AP, "js_str: %s", js_str);
        free(js_str);
    }
    if (NULL != js_root)
    {
        bscJSON_Delete(js_root);
    }
    char num_buffer[32] = {0};
    sprintf((char *)num_buffer, "%1.15f", -25.0);
    ez_log_e(TAG_UT_AP, "num_buffer: %s", num_buffer);

    ez_iot_ap_dev_info_t dev_info = {0};
    strncpy(dev_info.ap_ssid, "EZVIZ_BL", sizeof(dev_info.ap_ssid) - 1);
    strncpy(dev_info.dev_serial, "1118", sizeof(dev_info.dev_serial) - 1);
    strncpy(dev_info.dev_type, "BL602", sizeof(dev_info.dev_type) - 1);
    strncpy(dev_info.dev_version, "V1.0.0 build 210302", sizeof(dev_info.dev_version) - 1);

    uassert_int_equal(ez_errno_succ, ez_iot_ap_init(&dev_info, wifi_cb, 5));
    return 0;
}