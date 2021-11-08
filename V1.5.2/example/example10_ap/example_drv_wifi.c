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
  
 *******************************************************************************/

/******************************* ezviz_wifi_bsp_api*******************************/
#include "ezconfig.h"
#include "mcuconfig.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "hal_wifi_drv.h"

#include "esp_timer.h"
#include "math.h"
#include "ez_iot_log.h"
#include "esp_err.h"
#include "hal_thread.h"

static const char *TAG_WIFI = "T_WIFI";
static const char *TAG_EVENT = "[WIFI EVENT]";

//static int8_t g_wifi_country_code = 0;   //0->CN 1->EU 2->US 3->JP 4->BR 6->OTHER
#define COUNTRY_CODE_LEN 3
#define ARRAY_SIZE_N(a) (sizeof(a) / sizeof((a)[0]))

/*8*/
static char us_country_code_table[][COUNTRY_CODE_LEN] = {
    "US", "CA", "MX", "PE", "TW", "SR", "VI"};

/*29*/
static char br_country_code_table[][COUNTRY_CODE_LEN] = {
    "BR", "CO", "BB", "CR", "EC", "UY", "BM", "DM", "DO", "GY", "HT", "TT",
    "NI", "HN", "PA", "VE", "PY", "BS", "FM", "GD", "GT", "JM", "PW", "SV",
    "VU", "CL", "AR", "IN", "PK"};

/*30*/
static char eu_country_code_table[][COUNTRY_CODE_LEN] = {
    "AT", "BE", "BG", "CY", "CZ", "DE", "DK", "EE", "ES", "FI", "FR", "GB",
    "GR", "HR", "HU", "IE", "IT", "LU", "MT", "NL", "PL", "PT", "RO", "SE",
    "SI", "SK", "TR", "LV", "LT", "EU"};

/*1*/
static char jp_country_code_table[][COUNTRY_CODE_LEN] = {
    "JP"};

/*46*/
static char other_country_code_table[][COUNTRY_CODE_LEN] = {
    "CN", "AF", "BF", "BH", "BI", "BN", "BO", "BT", "BW", "CD", "CF", "CG",
    "CI", "CK", "CU", "DJ", "DZ", "ER", "ET", "GH", "GM", "GN", "GQ", "GW",
    "KM", "KP", "LR", "MG", "MH", "MM", "MU", "NE", "NG", "NR", "NU", "PS",
    "RW", "SB", "SD", "SL", "SO", "ST", "SY", "TO", "TP", "TV"};

typedef enum
{
    eCN = 0,
    eEU,
    eUS,
    eJP,
    eBR,
} LIGHT_STATE_E;

int Ezviz_Wifi_Config_Country_CodeMap(char *country_code);

int woal_is_us_country(char *country_code)
{
    unsigned char i = 0;

    for (i = 0; i < ARRAY_SIZE_N(us_country_code_table); i++)
    {
        if (!memcmp(country_code, us_country_code_table[i],
                    COUNTRY_CODE_LEN - 1))
        {
            ez_log_i(TAG_WIFI, "found region code=%s in US table.\n",
                     us_country_code_table[i]);

            return 1;
        }
    }
    return 0;
}

int woal_is_br_country(char *country_code)
{
    unsigned char i = 0;

    for (i = 0; i < ARRAY_SIZE_N(br_country_code_table); i++)
    {
        if (!memcmp(country_code, br_country_code_table[i], COUNTRY_CODE_LEN - 1))
        {
            ez_log_i(TAG_WIFI, "found region code=%s in BR table.\n", br_country_code_table[i]);
            return 1;
        }
    }
    return 0;
}

int woal_is_eu_country(char *country_code)
{
    unsigned char i = 0;

    for (i = 0; i < ARRAY_SIZE_N(eu_country_code_table); i++)
    {
        if (!memcmp(country_code, eu_country_code_table[i], COUNTRY_CODE_LEN - 1))
        {
            ez_log_i(TAG_WIFI, "found region code=%s in EU table\n", eu_country_code_table[i]);
            return 1;
        }
    }
    return 0;
}

int woal_is_jp_country(char *country_code)
{
    unsigned char i = 0;

    for (i = 0; i < ARRAY_SIZE_N(jp_country_code_table); i++)
    {
        if (!memcmp(country_code, jp_country_code_table[i], COUNTRY_CODE_LEN - 1))
        {
            printf("found region code=%s in JP table.\n", jp_country_code_table[i]);
            return 1;
        }
    }
    return 0;
}

int woal_is_other_country(char *country_code)
{
    unsigned char i = 0;

    for (i = 0; i < ARRAY_SIZE_N(other_country_code_table); i++)
    {
        if (!memcmp(country_code, other_country_code_table[i], COUNTRY_CODE_LEN - 1))
        {
            ez_log_i(TAG_WIFI, "found region code=%s in other table.\n", other_country_code_table[i]);
            return 1;
        }
    }
    return 0;
}

int Ezviz_Wifi_Config_Country_CodeMap(char *country_code)
{
    int8_t wifi_country_code = eCN;

    if (country_code == NULL)
    {
        printf("%s country_code is NULL !!! \n", __FUNCTION__);
        return -1;
    }

    if (woal_is_eu_country(country_code))
    {
        wifi_country_code = eEU;
    }
    else if (woal_is_us_country(country_code))
    {
        wifi_country_code = eUS;
    }
    else if (woal_is_jp_country(country_code))
    {
        wifi_country_code = eJP;
    }
    else if (woal_is_br_country(country_code))
    {
        wifi_country_code = eBR;
    }
    else
    {
        wifi_country_code = eCN;
    }
    return wifi_country_code;
    ez_log_i(TAG_WIFI, "config country code(%d)success.\n", wifi_country_code);
}

void iot_set_country_code(char *country_code)
{
    wifi_country_t country = {"US", 1, 13, 20, WIFI_COUNTRY_POLICY_MANUAL};
    int iWifiCountryCodeMap = 0;
    iWifiCountryCodeMap = Ezviz_Wifi_Config_Country_CodeMap(country_code);
    switch (iWifiCountryCodeMap)
    {
    case eCN:
        country.nchan = 13;
        country.max_tx_power = 20;
        break;
    case eEU:
        country.nchan = 13;
        country.max_tx_power = 17;
        break;
    case eUS:
        country.nchan = 11;
        country.max_tx_power = 20;
        break;
    case eJP:
        country.nchan = 14;
        country.max_tx_power = 20;
        break;
    case eBR:
        country.nchan = 13;
        country.max_tx_power = 20;
        break;
    default:
        break;
    }

    ez_log_i(TAG_WIFI, "set countrycode(%s) end,wifi actual start %d and end %d !", country_code, country.schan, country.nchan);
    //¡ä?o¡¥¨ºy?¨¢o???country.max_tx_power2?¨ºy
    esp_wifi_set_country(&country);
    // esp_wifi_set_max_tx_power(country.max_tx_power);
}

/*************************************************************************************************
*                                       WIFI CONFIG                                              *                                                         
*************************************************************************************************/

#define MAX_STA_CONN 4
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t g_wifi_event_group = NULL;
static iot_wifi_state_t g_wifi_connect_state = EZVIZ_WIFI_STATE_NOT_CONNECT;
static int g_wifi_disconnect_flag = 0;
static int8_t g_wifi_start_flag = 0;
ip_info_cb g_ip_cb = NULL;
set_sta_hostname_cb g_hostname_cb = NULL;
sta_update_cb g_sta_cb = NULL;
ssid_update_cb g_ssid_cb = NULL;
const int g_WIFI_SCAN_DONE_BIT = BIT0;
bool g_if_wifi_got_ip = false;

static void Ezviz_Wifi_Disconn_Reason(uint8_t reason)
{
    switch (reason)
    {
#ifdef _FREE_RTOS_
    case WIFI_REASON_BASIC_RATE_NOT_SUPPORT:
        ez_log_i(TAG_EVENT, "WIFI_REASON_BASIC_RATE_NOT_SUPPORT(%d)", reason);
        g_wifi_connect_state = EZVIZ_WIFI_STATE_BASIC_RATE_NOT_SUPPORT;
        esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
        break;
#endif
#ifdef _FREE_RTOS_
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
    case WIFI_REASON_HANDSHAKE_TIMEOUT:
#endif
    case WIFI_REASON_AUTH_FAIL:
    case WIFI_REASON_AUTH_EXPIRE:
        ez_log_i(TAG_EVENT, "WIFI_REASON_PWD_ERROR(%d)", reason);
        g_wifi_connect_state = EZVIZ_WIFI_STATE_PASSWORD_ERROR;
        break;
    case WIFI_REASON_NO_AP_FOUND:
        ez_log_i(TAG_EVENT, "WIFI_REASON_NO_AP_FOUND(%d)", reason);
        g_wifi_connect_state = EZVIZ_WIFI_STATE_NO_AP_FOUND;
        break;
    case WIFI_REASON_ASSOC_FAIL:
        ez_log_i(TAG_EVENT, "WIFI_REASON_ASSOC_FAIL(%d)", reason);
        g_wifi_connect_state = EZVIZ_WIFI_STATE_ASSOC_FAIL;
        break;

    default:
        ez_log_e(TAG_EVENT, "unknow reason(%d)", reason);

        break;
    }
    return;
}

static esp_err_t Ezviz_Wifi_Event_Handler(void *ctx, system_event_t *event)
{
    system_event_info_t *info = &event->event_info;
    wifi_country_t config_country;

    ez_log_d(TAG_WIFI, "enter!!event->event_id=%d!!!", event->event_id);

    switch (event->event_id)
    {
    case SYSTEM_EVENT_AP_START:
        ez_log_i(TAG_EVENT, "AP_START SUCCESS!!!!!");
        break;
    case SYSTEM_EVENT_AP_STOP:
        ez_log_i(TAG_EVENT, "AP_STOP SUCCESS!!!!!");
        break;
    case SYSTEM_EVENT_SCAN_DONE:
        ez_log_i(TAG_EVENT, "SYSTEM_EVENT_SCAN_DONE!!!!!");
        xEventGroupSetBits(g_wifi_event_group, g_WIFI_SCAN_DONE_BIT);
        break;
    case SYSTEM_EVENT_STA_START:
        ez_log_i(TAG_EVENT, "SYSTEM_EVENT_STA_START!!!!!");
        if (NULL != g_hostname_cb)
        {
            g_hostname_cb();
        }
        break;
    case SYSTEM_EVENT_STA_STOP:
        ez_log_i(TAG_EVENT, "SYSTEM_EVENT_STA_STOP!!!!!");
        g_wifi_connect_state = EZVIZ_WIFI_STATE_NOT_CONNECT;
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        ez_log_i(TAG_EVENT, "SYSTEM_EVENT_STA_CONNECTED!!!!!");
        if (NULL != g_sta_cb)
        {
            g_sta_cb(0x02, 0);
        }
        esp_wifi_get_country(&config_country);
        ez_log_w(TAG_WIFI, "Country info:CC:%c%c,SC:%d,NC:%d\n", config_country.cc[0], config_country.cc[1], config_country.schan, config_country.nchan);
        g_wifi_connect_state = EZVIZ_WIFI_STATE_CONNECT_SUCCESS;
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ez_log_i(TAG_EVENT, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        if (NULL != g_ip_cb)
        {
            g_ip_cb(&event->event_info.got_ip.ip_info);
        }
        g_if_wifi_got_ip = true;
        break;
    case SYSTEM_EVENT_AP_STACONNECTED: /**< a station connected to ESP8266 soft-AP */
        ez_log_w(TAG_EVENT, "AP_STA_CONNECTED");
        ez_log_i(TAG_EVENT, "station:" MACSTR " join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED: /**< ESP8266 soft-AP assign an IP to a connected station */
        ez_log_w(TAG_EVENT, "AP_STA_IPASSIGNED");
        g_wifi_connect_state = EZVIZ_WIFI_STATE_AP_STA_CONNECTED;
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED: /**< a station disconnected from ESP8266 soft-AP */
        ez_log_w(TAG_EVENT, "AP_STA_DISCONNECTED");
        ez_log_i(TAG_EVENT, "station:" MACSTR "leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ez_log_w(TAG_WIFI, "SYSTEM_EVENT_STA_DISCONNECTED");
        if (NULL != g_sta_cb)
        {
            g_sta_cb(0x02, 1);
        }
        Ezviz_Wifi_Disconn_Reason(info->disconnected.reason);

        if (0 == g_wifi_disconnect_flag)
        {
            esp_wifi_connect();
        }
        break;

    default:
        ez_log_e(TAG_EVENT, "unknow event id(%d)", event->event_id);
        break;
    }
    return ESP_OK;
}

bool iot_sta_if_got_ip()
{
    return g_if_wifi_got_ip;
}

void Ezviz_Wifi_Event_Loop_Init(void)
{
	ez_log_d(TAG_WIFI, "%s enter!!!!!", __FUNCTION__);
	ESP_ERROR_CHECK(esp_event_loop_init(Ezviz_Wifi_Event_Handler, NULL));
	return ;
}

void iot_wifi_init(void)
{
    ez_log_d(TAG_WIFI, "%s enter!!!!!", __FUNCTION__);
    g_wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();
    Ezviz_Wifi_Event_Loop_Init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

void iot_wifi_finit(void)
{
    wifi_mode_t wifi_mode = WIFI_MODE_NULL;
    esp_err_t ret = ESP_OK;

    ez_log_d(TAG_WIFI, "%s enter!!", __FUNCTION__);

    g_wifi_disconnect_flag = 1;

    ret = esp_wifi_get_mode(&wifi_mode);
    if (ret != ESP_OK)
    {
        ez_log_e(TAG_WIFI, "%s Wifi Stop fail!!", __FUNCTION__);
        return;
    }
    switch (wifi_mode)
    {
    case WIFI_MODE_STA:
        esp_wifi_deauth_sta(0);
        ez_log_i(TAG_WIFI, "%s stop sta mode!!", __FUNCTION__);
        break;
    case WIFI_MODE_AP:
        esp_wifi_disconnect();
        ez_log_i(TAG_WIFI, "%s stop ap mode!!", __FUNCTION__);
        break;
    case WIFI_MODE_APSTA:
        esp_wifi_disconnect();
        esp_wifi_deauth_sta(0);
        ez_log_i(TAG_WIFI, "%s stop ap and sta!!", __FUNCTION__);
        break;
    default:
        ez_log_i(TAG_WIFI, "%s unknow wifi mode!!", __FUNCTION__);
        break;
    }

    ESP_ERROR_CHECK(esp_wifi_stop());

 	ESP_ERROR_CHECK(esp_wifi_deinit());

    if (g_wifi_event_group) {
        vEventGroupDelete(g_wifi_event_group);
        g_wifi_event_group = NULL;
    }
    return;
}

bool g_wifi_scan_start = false;
uint16_t iot_sta_get_scan_list(uint16_t max_ap_num, wifi_info_list_t *aplist)
{
    if (g_wifi_scan_start)
    {
        ez_log_w(TAG_WIFI, "wifi scan already started.");
        return 0;
    }
    g_wifi_scan_start = true;
    static wifi_scan_config_t scanConf =
        {
            .ssid = NULL,
            .bssid = NULL,
            .channel = 0,
            .show_hidden = true,
        };

    ez_log_d(TAG_WIFI, "%s enter!!", __FUNCTION__);

    if (max_ap_num == 0 || aplist == NULL)
    {
        ez_log_e(TAG_WIFI, "!%s parameter erroe!", __FUNCTION__);
        g_wifi_scan_start = false;
        return 0;
    }

    if (g_wifi_start_flag != 1)
    {
        ESP_ERROR_CHECK(esp_wifi_start());
        //Ezviz_Wifi_set_country_code();
    }

    ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, true));
    xEventGroupWaitBits(g_wifi_event_group, g_WIFI_SCAN_DONE_BIT, 0, 1, portMAX_DELAY);
    ez_log_i(TAG_WIFI, "WIFI scan done");
    xEventGroupClearBits(g_wifi_event_group, g_WIFI_SCAN_DONE_BIT);

    wifi_ap_record_t tmp_list[20] = {0};
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&max_ap_num, tmp_list));
    for (int i = 0; i < max_ap_num; i++)
    {
        aplist[i].authmode = tmp_list[i].authmode;
        memcpy(aplist[i].bssid, tmp_list[i].bssid, sizeof(aplist[i].bssid));
        aplist[i].channel = tmp_list[i].primary;
        aplist[i].rssi = tmp_list[i].rssi;
        strncpy(aplist[i].ssid, tmp_list[i].ssid, sizeof(aplist[i].ssid) - 1);
    }

    // for(int i = 0; i < max_ap_num;i++)
    // {
    //     ez_log_v(TAG_WIFI, "NO.%2d ssid:%32s bssid:"MACSTR" channel:%2d rssi:%2d authmode:%2d ", i + 1, aplist[i].ssid, MAC2STR(aplist[i].bssid), aplist[i].primary, aplist[i].rssi, aplist[i].authmode);
    // }

    g_wifi_scan_start = false;
    return max_ap_num;
}

int iot_sta_connect(char *ssid, char *pwd )
{
    wifi_config_t wifi_config;
    esp_err_t ret = ESP_OK;

    ez_log_d(TAG_WIFI, "%s enter!!!!!", __FUNCTION__);

    g_wifi_connect_state = EZVIZ_WIFI_STATE_NOT_CONNECT; //init connect state
    g_wifi_disconnect_flag = 0;

    memset(&wifi_config, 0, sizeof(wifi_config));

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
    else
    {
        memcpy(wifi_config.sta.ssid, ssid, 32);
    }

    if (NULL != g_ssid_cb)
    {
        g_ssid_cb((char *)wifi_config.sta.ssid);
    }

    if (NULL == pwd)
    {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
        ez_log_i(TAG_WIFI, "connect to ap SSID:%.32s password: NULL", wifi_config.sta.ssid);
    }
    else
    {
        if (strlen((char *)(pwd)) > 64)
        {
            ez_log_e(TAG_WIFI, "%s wifi config error, pwd is too long!!!!", __FUNCTION__);
            return -1;
        }
        //wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK;
        memcpy(wifi_config.ap.password, pwd, 64);
        ez_log_i(TAG_WIFI, "connect to ap SSID:%.32s password:**", wifi_config.sta.ssid);
    }

    wifi_config.sta.listen_interval = 10;
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

    g_wifi_start_flag = 1;

    ret = esp_wifi_connect();
    if (ESP_OK != ret)
    {
        ez_log_i(TAG_WIFI, "wifi connect error. ret = %d", ret);
        g_wifi_connect_state = EZVIZ_WIFI_STATE_PARAMETER_ERROR;
    }
    return 0;
}

void iot_sta_disconnect(void)
{
    ez_log_d(TAG_WIFI, "%s enter!!!!!", __FUNCTION__);

    if (1 == g_wifi_disconnect_flag)
    {
        return;
    }

    g_wifi_disconnect_flag = 1;
    g_wifi_connect_state = EZVIZ_WIFI_STATE_NOT_CONNECT; //init connect state

    esp_wifi_disconnect();
}

iot_wifi_state_t iot_sta_get_state(void)
{
    ez_log_d(TAG_WIFI, "g_wifi_connect_state= %d!!", g_wifi_connect_state);
    return g_wifi_connect_state;
}

int iot_sta_start(void)
{
    int ret = 0;

    ez_log_d(TAG_WIFI, "%s enter!!!!!", __FUNCTION__);
    ret = esp_wifi_set_mode(WIFI_MODE_STA);

    if (ret != ESP_OK)
    {
        ez_log_e(TAG_WIFI, "wifi config sta mode error!!!!");
        return -1;
    }

    return 0;
}

void iot_sta_stop()
{
    iot_wifi_finit();
}

void iot_ap_stop()
{
    iot_wifi_finit();
}

int iot_ap_start(char *ssid, char *pwd, int authmode, bool ap_sta)
{
    wifi_config_t wifi_config;
    int ssid_len = 0, pwd_len = 0;

    ez_log_d(TAG_WIFI, "%s enter!!!!!", __FUNCTION__);

    memset(&wifi_config, 0, sizeof(wifi_config));

    if ((ssid == NULL) || (strlen((char *)(ssid)) == 0))
    {
        ez_log_e(TAG_WIFI, "%s wifi config  error, please check ssid!!!", __FUNCTION__);
        return -1;
    }

    if (NULL == pwd || (0 == strlen(pwd) && 0 != authmode))
    {
        ez_log_e(TAG_WIFI, "%s wifi config error, please set open mode or set pwd!!!", __FUNCTION__);
        return -1;
    }

    ssid_len = strlen((char *)(ssid));

    if (ssid_len > 32)
    {
        ez_log_e(TAG_WIFI, "%s wifi config error, ssid is too long!!!!", __FUNCTION__);
        return -1;
    }

    if (0 != authmode)
    {
        pwd_len = strlen(pwd);
        if (pwd_len > 64)
        {
            ez_log_e(TAG_WIFI, "%s wifi config error, pwd is too long!!!!", __FUNCTION__);
            return -1;
        }

        memcpy(wifi_config.ap.password, pwd, 64);
    }

    memcpy(wifi_config.ap.ssid, ssid, 32);
    wifi_config.ap.ssid_len = ssid_len;
    wifi_config.ap.max_connection = MAX_STA_CONN;
    wifi_config.ap.authmode = authmode;
    strncpy(wifi_config.ap.password, pwd, sizeof(wifi_config.ap.password));
    //wifi_config.ap.channel = 1;
    if (ap_sta)
    {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    }
    else
    {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));

    ez_log_i(TAG_WIFI, "wifi_init_softap finished.SSID:%s password:%s", wifi_config.sta.ssid, wifi_config.sta.password);

    ESP_ERROR_CHECK(esp_wifi_start());

    g_wifi_start_flag = 1;

    return 0;
}

int iot_get_ap_rssi(int8_t *rssi)
{
    int ret = -1;

    if (EZVIZ_WIFI_STATE_CONNECT_SUCCESS == iot_sta_get_state())
    {
#ifdef _FREE_RTOS_
        int8_t rssi_temp = 0;
        rssi_temp = esp_wifi_get_ap_rssi();
        *rssi = rssi_temp;
#elif _FREE_RTOS_32_
        wifi_ap_record_t wifi_param = {0};
        esp_wifi_sta_get_ap_info(&wifi_param);
        *rssi = wifi_param.rssi;
#endif

        ez_log_i(TAG_WIFI, "Get Rssi success! rssi = %d", *rssi);

        ret = 0;
    }
    else
    {
        ez_log_i(TAG_WIFI, "Wifi Disconnected, Rssi can't get.");
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
