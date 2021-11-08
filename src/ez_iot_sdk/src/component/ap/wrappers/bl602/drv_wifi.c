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
#include "FreeRTOS.h"
#include "task.h"
#include "hal_wifi_drv.h"

#include "timers.h"
#include "ez_iot_log.h"
#include "ez_iot_errno.h"
#include "hal_thread.h"

#include "yloop.h"
#include "wifi_mgmr_ext.h"

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

static wifi_interface_t g_sta_interface = {0};
static wifi_interface_t g_ap_interface = {0};

static const char *TAG_WIFI = "T_WIFI";
static const char *TAG_EVENT = "[WIFI EVENT]";

void iot_set_country_code(char *country_code)
{
    wifi_mgmr_set_country_code((char *)country_code);

    ez_log_i(TAG_WIFI, "set countrycode(%s) end", country_code);
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

static wifi_conf_t conf =
    {
        .country_code = "CN",
        .channel_nums = 5,
};

static void event_cb_wifi_event(input_event_t *event, void *private_data)
{
    static char *ssid;
    static char *password;

    switch (event->code)
    {
    case CODE_WIFI_ON_INIT_DONE:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] INIT DONE %lld", aos_now_ms());
        wifi_mgmr_start_background(&conf);
    }
    break;
    case CODE_WIFI_ON_MGMR_DONE:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] MGMR DONE %lld, now %lu ms", aos_now_ms(), bl_timer_now_us() / 1000);
    }
    break;
    case CODE_WIFI_ON_MGMR_DENOISE:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] Microwave Denoise is ON %lld", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_SCAN_DONE:
    {
        ez_log_i(TAG_EVENT, "SYSTEM_EVENT_SCAN_DONE!!!!!");
        // wifi_mgmr_cli_scanlist();
    }
    break;
    case CODE_WIFI_ON_SCAN_DONE_ONJOIN:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] SCAN On Join %lld", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_DISCONNECT:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] disconnect %lld, Reason: %s",
                 aos_now_ms(),
                 wifi_mgmr_status_code_str(event->value));
        g_wifi_connect_state = EZVIZ_WIFI_STATE_NOT_CONNECT;
    }
    break;
    case CODE_WIFI_ON_CONNECTING:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] Connecting %lld", aos_now_ms());
    }
    break;
    case CODE_WIFI_CMD_RECONNECT:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] Reconnect %lld", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_CONNECTED:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] connected %lld", aos_now_ms());
        if (NULL != g_sta_cb)
        {
            g_sta_cb(0x02, 0);
        }
        g_wifi_connect_state = EZVIZ_WIFI_STATE_CONNECT_SUCCESS;
    }
    break;
    case CODE_WIFI_ON_PRE_GOT_IP:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] connected %lld", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_GOT_IP:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] GOT IP %lld", aos_now_ms());
        ez_log_i(TAG_WIFI, "[SYS] Memory left is %d Bytes", xPortGetFreeHeapSize());
        if (NULL != g_ip_cb)
        {
            g_if_wifi_got_ip = true;
        }
    }
    break;
    case CODE_WIFI_ON_EMERGENCY_MAC:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] EMERGENCY MAC %lld", aos_now_ms());
        hal_reboot(); //one way of handling emergency is reboot. Maybe we should also consider solutions
    }
    break;
    case CODE_WIFI_ON_PROV_SSID:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] [PROV] [SSID] %lld: %s",
                 aos_now_ms(),
                 event->value ? (const char *)event->value : "UNKNOWN");
        if (ssid)
        {
            vPortFree(ssid);
            ssid = NULL;
        }
        ssid = (char *)event->value;
    }
    break;
    case CODE_WIFI_ON_PROV_BSSID:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] [PROV] [BSSID] %lld: %s",
                 aos_now_ms(),
                 event->value ? (const char *)event->value : "UNKNOWN");
        if (event->value)
        {
            vPortFree((void *)event->value);
        }
    }
    break;
    case CODE_WIFI_ON_PROV_PASSWD:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] [PROV] [PASSWD] %lld: %s", aos_now_ms(),
                 event->value ? (const char *)event->value : "UNKNOWN");
        if (password)
        {
            vPortFree(password);
            password = NULL;
        }
        password = (char *)event->value;
    }
    break;
    case CODE_WIFI_ON_PROV_CONNECT:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] [PROV] [CONNECT] %lld", aos_now_ms());
        ez_log_i(TAG_WIFI, "connecting to %s:%s...", ssid, password);
        wifi_mgmr_sta_connect(&g_sta_interface, ssid, password, NULL, NULL, 0, 0);
    }
    break;
    case CODE_WIFI_ON_PROV_DISCONNECT:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] [PROV] [DISCONNECT] %lld", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_AP_STA_ADD:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] [AP] [ADD] %lld, sta idx is %lu", aos_now_ms(), (uint32_t)event->value);
        g_wifi_connect_state = EZVIZ_WIFI_STATE_AP_STA_CONNECTED;
    }
    break;
    case CODE_WIFI_ON_AP_STA_DEL:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] [AP] [DEL] %lld, sta idx is %lu", aos_now_ms(), (uint32_t)event->value);
    }
    break;
    default:
    {
        ez_log_i(TAG_WIFI, "[APP] [EVT] Unknown code %u, %lld", event->code, aos_now_ms());
        /*nothing*/
    }
    }
}

bool iot_sta_if_got_ip()
{
    return g_if_wifi_got_ip;
}

static void cmd_stack_wifi(char *buf, int len, int argc, char **argv)
{
    /*wifi fw stack and thread stuff*/
    static uint8_t stack_wifi_init = 0;

    if (1 == stack_wifi_init)
    {
        ez_log_w(TAG_WIFI, "Wi-Fi stack started already!!!");
        return;
    }
    stack_wifi_init = 1;

    ez_log_d(TAG_WIFI, "start Wi-Fi fw @%lu ms", bl_timer_now_us() / 1000);
    hal_wifi_start_firmware_task();
    /*Trigger to start Wi-Fi*/
    ez_log_d(TAG_WIFI, "start Wi-Fi fw is done @%lu ms", bl_timer_now_us() / 1000);
    aos_post_event(EV_WIFI, CODE_WIFI_ON_INIT_DONE, 0);
}

void iot_wifi_init(void)
{
    ez_log_d(TAG_WIFI, "%s enter!!!!!", __FUNCTION__);
    aos_register_event_filter(EV_WIFI, event_cb_wifi_event, NULL);
    cmd_stack_wifi(NULL, 0, 0, NULL);
}

void iot_ap_stop()
{
    wifi_mgmr_ap_stop(&g_ap_interface);
}

void iot_sta_stop()
{
    wifi_mgmr_sta_disable(&g_sta_interface);
}

void iot_wifi_finit(void)
{
    ez_log_d(TAG_WIFI, "%s enter!!", __FUNCTION__);

    g_wifi_disconnect_flag = 1;

    wifi_mgmr_sta_disable(&g_sta_interface);
    wifi_mgmr_ap_stop(&g_ap_interface);

    return;
}

bool g_wifi_scan_start = false;

static int ap_sort_with_rssi(wifi_mgmr_ap_item_t *ap_array, uint32_t ap_num)
{
    wifi_mgmr_ap_item_t temp_ap = {0};
    for (size_t i = 0; i < ap_num - 1; i++)
    {
        for (size_t j = i + 1; j < ap_num; j++)
        {
            if ((ap_array + i)->rssi < (ap_array + j)->rssi)
            {
                memcpy(&temp_ap, ap_array + i, sizeof(wifi_mgmr_ap_item_t));
                memcpy(ap_array + i, ap_array + j, sizeof(wifi_mgmr_ap_item_t));
                memcpy(ap_array + j, &temp_ap, sizeof(wifi_mgmr_ap_item_t));
            }
        }
    }
    return 0;
}

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
    wifi_mgmr_ap_item_t *ap_array = NULL;
    wifi_mgmr_all_ap_scan(&ap_array, &got_ap_num);

    if (NULL == ap_array)
    {
        ez_log_w(TAG_WIFI, "ap_array NULL.");
        return 0;
    }
    ap_sort_with_rssi(ap_array, got_ap_num);

    int ret_ap_num = MIN(max_ap_num, got_ap_num);
    for (size_t i = 0; i < ret_ap_num; i++)
    {
        wifi_info_list_t *ap_info = ap_list + i;
        wifi_mgmr_ap_item_t *ap_scan_info = ap_array + i;
        ap_info->authmode = ap_scan_info->auth;
        ap_info->rssi = ap_scan_info->rssi;
        ap_info->channel = ap_scan_info->channel;
        memcpy(ap_info->bssid, ap_scan_info->bssid, sizeof(ap_info->bssid));
        strncpy(ap_info->ssid, ap_scan_info->ssid, sizeof(ap_info->ssid) - 1);
    }
    free(ap_array);

    g_wifi_scan_start = false;
    return ret_ap_num;
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

    g_sta_interface = wifi_mgmr_sta_enable();
    int ret = wifi_mgmr_sta_connect(&g_sta_interface, ssid, pwd, NULL, NULL, 0, 0);

    if (0 != ret)
    {
        g_wifi_connect_state = EZVIZ_WIFI_STATE_PARAMETER_ERROR;
    }

    return ret;
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
    wifi_mgmr_state_get(&wifi_state);
    switch (wifi_state)
    {
    case WIFI_STATE_CONNECTED_IP_GOT:
        g_wifi_connect_state = EZVIZ_WIFI_STATE_CONNECT_SUCCESS;
        break;
    case WIFI_STATE_DISCONNECT:
        g_wifi_connect_state = EZVIZ_WIFI_STATE_NOT_CONNECT;
        break;
    case WIFI_STATE_WITH_AP_CONNECTED_IP_GOT:
        g_wifi_connect_state = EZVIZ_WIFI_STATE_AP_STA_CONNECTED;
        break;
    case WIFI_STATE_PSK_ERROR:
        g_wifi_connect_state = EZVIZ_WIFI_STATE_PASSWORD_ERROR;
        break;
    case WIFI_STATE_NO_AP_FOUND:
        g_wifi_connect_state = EZVIZ_WIFI_STATE_NO_AP_FOUND;
        break;
    default:
        break;
    }
    ez_log_d(TAG_WIFI, "g_wifi_connect_state= %d!!", g_wifi_connect_state);
    return g_wifi_connect_state;
}

int iot_sta_start(void)
{
    int ret = 0;
    ez_log_d(TAG_WIFI, "%s enter!", __FUNCTION__);
    g_sta_interface = wifi_mgmr_sta_enable();

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

    g_ap_interface = wifi_mgmr_ap_enable();

    wifi_mgmr_ap_start(g_ap_interface, ssid, 0, pwd, 1);

    if (ap_sta)
    {
        g_sta_interface = wifi_mgmr_sta_enable();
    }

    ez_log_i(TAG_WIFI, "ap init finished. ssid:%s password: %s", ssid, pwd);
    g_wifi_start_flag = 1;
    return 0;
}

int iot_get_ap_rssi(int8_t *rssi)
{
    int8_t ret = -1;

    if (EZVIZ_WIFI_STATE_CONNECT_SUCCESS == iot_sta_get_state())
    {
        wifi_mgmr_rssi_get(rssi);
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
