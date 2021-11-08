/*******************************************************************************
 * Copyright 漏 2017-2021 Ezviz Inc.
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
  2021-01-28     chentengfei
 *******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "ut_config.h"
#include "ez_iot.h"
#include "ez_iot_log.h"
#include "ez_iot_ap.h"
#include "ez_iot_errno.h"
#include "hal_thread.h"

const char *TAG_UT_AP = "UT_AP";

static void wifi_cb(ez_iot_ap_wifi_info_t *wifi_info)
{
/** 由上层处理，根据错误码做上册业务处理 */
/** 
    example code:

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
*/
}

/**
 * 启动ap配网
 * ap热点无密码
 * 支持ap_sta共存
 */
void example_ap_init_open_apsta()
{
    ez_iot_ap_dev_info_t dev_info = {0};
    strncpy(dev_info.ap_ssid, "EZVIZ_AP_11112", sizeof(dev_info.ap_ssid) - 1);
    dev_info.auth_mode = 0;
    strncpy(dev_info.dev_serial, "88888888", sizeof(dev_info.dev_serial) - 1);
    strncpy(dev_info.dev_type, "EZ_001", sizeof(dev_info.dev_type) - 1);
    strncpy(dev_info.dev_version, "V1.0.0 build 210302", sizeof(dev_info.dev_version) - 1);
    ez_iot_wifi_init();
    ez_iot_ap_init(&dev_info, wifi_cb, 5, true);
    return;
}

/**
 * 启动ap配网
 * ap热点有密码
 * 支持ap_sta共存
 */
void example_ap_init_auth_apsta()
{
    ez_iot_ap_dev_info_t dev_info = {0};
    strncpy(dev_info.ap_ssid, "EZVIZ_AP_11112", sizeof(dev_info.ap_ssid) - 1);
    strncpy(dev_info.ap_password, "12345678", sizeof(dev_info.ap_password) - 1);
    dev_info.auth_mode = 4;
    strncpy(dev_info.dev_serial, "88888888", sizeof(dev_info.dev_serial) - 1);
    strncpy(dev_info.dev_type, "EZ_001", sizeof(dev_info.dev_type) - 1);
    strncpy(dev_info.dev_version, "V1.0.0 build 210302", sizeof(dev_info.dev_version) - 1);
    ez_iot_wifi_init();
    ez_iot_ap_init(&dev_info, wifi_cb, 5, true);
    return;
}

void example_ap_init()
{
    example_ap_init_open_apsta();

    hal_thread_sleep(10 * 60 * 1000);

    example_ap_init_auth_apsta();
}
