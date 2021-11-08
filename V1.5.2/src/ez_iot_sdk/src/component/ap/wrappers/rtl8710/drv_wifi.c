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

/******************************* ezviz_wifi_bsp_api*******************************/
#include <string.h>
#include "hal_wifi_drv.h"

#include "ez_iot_log.h"
#include "ez_iot_errno.h"
#include "hal_thread.h"

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

static const char *TAG_WIFI = "T_WIFI";
static const char *TAG_EVENT = "[WIFI EVENT]";

void iot_set_country_code(char *country_code)
{
}

/*************************************************************************************************
*                                       WIFI CONFIG                                                                                             
*************************************************************************************************/

#define MAX_STA_CONN 4
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* FreeRTOS event group to signal when we are connected*/
static iot_wifi_state_t g_wifi_connect_state = EZVIZ_WIFI_STATE_NOT_CONNECT;
static int g_wifi_disconnect_flag = 0;
static int8_t g_wifi_start_flag = 0;
ip_info_cb g_ip_cb = NULL;
set_sta_hostname_cb g_hostname_cb = NULL;
sta_update_cb g_sta_cb = NULL;
ssid_update_cb g_ssid_cb = NULL;
bool g_if_wifi_got_ip = false;

bool iot_sta_if_got_ip()
{
    return g_if_wifi_got_ip;
}

void iot_wifi_init(void)
{
    ez_log_d(TAG_WIFI, "%s enter!!!!!", __FUNCTION__);
}

void iot_ap_stop()
{
}

void iot_sta_stop()
{
}

void iot_wifi_finit(void)
{
    ez_log_d(TAG_WIFI, "%s enter!!", __FUNCTION__);

    g_wifi_disconnect_flag = 1;

    return;
}

bool g_wifi_scan_start = false;

uint16_t iot_sta_get_scan_list(uint16_t max_ap_num, wifi_info_list_t *ap_list)
{
    if (g_wifi_scan_start)
    {
        ez_log_w(TAG_WIFI, "wifi scan already started.");
        return 0;
    }
    g_wifi_scan_start = true;

    ez_log_d(TAG_WIFI, "%s enter!!", __FUNCTION__);

    if (max_ap_num == 0 || ap_list == NULL)
    {
        ez_log_e(TAG_WIFI, "!%s parameter error!", __FUNCTION__);
        g_wifi_scan_start = false;
        return 0;
    }

    uint32_t got_ap_num = 0;

    g_wifi_scan_start = false;
    return 0;
}

int iot_sta_connect(char *ssid, char *pwd)
{
    ez_log_d(TAG_WIFI, "%s enter!", __FUNCTION__);

    g_wifi_connect_state = EZVIZ_WIFI_STATE_NOT_CONNECT; //init connect state
    g_wifi_disconnect_flag = 0;

    if ((ssid == NULL) || (strlen((char *)(ssid)) == 0))
    {
        ez_log_e(TAG_WIFI, "%s wifi config error, please check ssid!!!", __FUNCTION__);
        return -1;
    }

    if (strlen((char *)(ssid)) > 32)
    {
        ez_log_e(TAG_WIFI, "%s wifi config error, ssid is too long!!!!", __FUNCTION__);
        return -1;
    }

    if (NULL != g_ssid_cb)
    {
        g_ssid_cb((char *)ssid);
    }

    if (NULL == pwd)
    {
        ez_log_i(TAG_WIFI, "connect to ap ssid:%.32s password: NULL", ssid);
    }
    else
    {
        if (strlen((char *)(pwd)) > 64)
        {
            ez_log_e(TAG_WIFI, "%s wifi config error, pwd is too long!!!!", __FUNCTION__);
            return -1;
        }
        ez_log_i(TAG_WIFI, "connect to ap ssid:%.32s password:%s", ssid, pwd);
    }

    g_wifi_start_flag = 1;
}

void iot_sta_disconnect(void)
{
    ez_log_d(TAG_WIFI, "%s enter!", __FUNCTION__);

    if (1 == g_wifi_disconnect_flag)
    {
        return;
    }
    g_wifi_disconnect_flag = 1;
    g_wifi_connect_state = EZVIZ_WIFI_STATE_NOT_CONNECT; //init connect state

    wifi_mgmr_sta_disconnect();
}

iot_wifi_state_t iot_sta_get_state(void)
{
    int wifi_state = 0;
    {
        g_wifi_connect_state = EZVIZ_WIFI_STATE_CONNECT_SUCCESS;
        g_wifi_connect_state = EZVIZ_WIFI_STATE_NOT_CONNECT;
        g_wifi_connect_state = EZVIZ_WIFI_STATE_AP_STA_CONNECTED;
        g_wifi_connect_state = EZVIZ_WIFI_STATE_PASSWORD_ERROR;
        g_wifi_connect_state = EZVIZ_WIFI_STATE_NO_AP_FOUND;
    }
    ez_log_d(TAG_WIFI, "g_wifi_connect_state= %d!!", g_wifi_connect_state);
    return g_wifi_connect_state;
}

int iot_sta_start(void)
{
    int ret = 0;
    ez_log_d(TAG_WIFI, "%s enter!", __FUNCTION__);

    return 0;
}

int iot_ap_start(char *ssid, char *pwd, int authmode, bool ap_sta)
{
    ez_log_d(TAG_WIFI, "%s enter!!!!!", __FUNCTION__);

    if ((ssid == NULL) || (strlen((char *)(ssid)) == 0))
    {
        ez_log_e(TAG_WIFI, "%s wifi config  error, please check ssid!", __FUNCTION__);
        return -1;
    }

    if (NULL == pwd && 0 != authmode)
    {
        ez_log_e(TAG_WIFI, "%s wifi config error, please set open mode or set pwd!", __FUNCTION__);
        return -1;
    }

    int ssid_len = strlen((char *)(ssid));

    if (ssid_len > 32)
    {
        ez_log_e(TAG_WIFI, "%s wifi config error, ssid is too long!", __FUNCTION__);
        return -1;
    }

    if (0 != authmode)
    {
        int pwd_len = strlen((char *)(pwd));
        if (pwd_len > 64)
        {
            ez_log_e(TAG_WIFI, "%s wifi config error, pwd is too long!", __FUNCTION__);
            return -1;
        }
    }
    ez_log_i(TAG_WIFI, "ap init finished. ssid:%s password: NULL", ssid);
    g_wifi_start_flag = 1;
    return 0;
}

int iot_get_ap_rssi(int8_t *rssi)
{
    int8_t ret = -1;

    if (EZVIZ_WIFI_STATE_CONNECT_SUCCESS == ez_iot_sta_get_state())
    {
        ez_log_i(TAG_WIFI, "get rssi success! rssi = %d", *rssi);
        ret = 0;
    }
    else
    {
        ez_log_i(TAG_WIFI, "wifi disconnected, rssi can't get.");
    }

    return ret;
}

void iot_wifi_cb_register(ip_info_cb cb)
{
    g_ip_cb = cb;
}
void iot_hostname_cb_register(set_sta_hostname_cb cb)
{
    g_hostname_cb = cb;
}

void iot_netmgr_cb_register(sta_update_cb sta_cb, ssid_update_cb ssid_cb)
{
    g_sta_cb = sta_cb;
    g_ssid_cb = ssid_cb;
}
