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
#ifdef _FREE_RTOS_32_
#include "ezconfig.h"
#include "mcuconfig.h"
#include <string.h>
#include "ez_iot_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"

#include "esp_blufi_api.h"
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

#include "drv_ble.h"
#include "ble_msg_parse.h"
#include "restful_server.h"

#include "stack/bt_types.h"
#include "stack/btm_ble_api.h"
#include "btc/btc_task.h"
#include "blufi_int.h"

extern tBLUFI_ENV blufi_env;

static SemaphoreHandle_t g_ble_event_sem = NULL;
static ez_iot_ap_dev_info_t g_dev_info = {0};
static char g_ble_ssid[32] = {0};
static wifi_info_cb g_wifi_cb;
static TimerHandle_t g_time_out_timer = NULL;

typedef struct
{
    esp_blufi_cb_event_t event;
    esp_blufi_cb_param_t param;
} ble_event_t;

static const char *TAG_EVENT = "[BLE EVENT]";

static void ble_event_callbacks(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param);

static uint8_t example_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xFF,
    0xFF,
    0x00,
    0x00,
};

//static uint8_t test_manufacturer[TEST_MANUFACTURER_DATA_LEN] =  {0x12, 0x23, 0x45, 0x56};
static esp_ble_adv_data_t g_adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 16,
    .p_service_uuid = example_service_uuid128,
    .flag = 0x6,
};

static esp_blufi_callbacks_t g_blufi_cbs = {
    .event_cb = ble_event_callbacks,
};
static esp_ble_adv_params_t g_adv_params = {
    .adv_int_min = 0x100,
    .adv_int_max = 0x100,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static uint8_t g_server_if = 0;
static uint16_t g_conn_id = 1;
static bool g_ble_is_connected = false;
static uint8_t g_rsp_message[2048] = {0};
static uint8_t g_req_message[512] = {0};
static ble_event_t g_ble_event = {0};

static void ble_event_callbacks(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param)
{
    ez_log_d(TAG_BLE, "ble event: %d", event);
    if (event == ESP_BLUFI_EVENT_RECV_CUSTOM_DATA)
    {
        // ez_log_i(TAG_BLE, "custom data: %s", param->custom_data.data);
        memset(g_req_message, 0, sizeof(g_req_message));
        memcpy(g_req_message, param->custom_data.data, param->custom_data.data_len);
    }
    memset(&g_ble_event, 0, sizeof(ble_event_t));
    g_ble_event.event = event;
    if (NULL != param)
    {
        g_ble_event.param = *param;
    }
    else
    {
        ez_log_i(TAG_BLE, "blufi cb param NULL.");
    }

    // if (event == ESP_BLUFI_EVENT_RECV_CUSTOM_DATA)
    // {
    //     ez_log_i(TAG_BLE, "custom data: %s", g_ble_event.param.custom_data.data);
    //     ez_log_i(TAG_EVENT, "Recv Custom Data %p", g_ble_event.param.custom_data.data);
    //     ez_log_hexdump(TAG_BLE, 16, g_ble_event.param.custom_data.data, g_ble_event.param.custom_data.data_len);
    // }

    if (NULL != g_ble_event_sem)
    {
        xSemaphoreGive(g_ble_event_sem);
    }
}

static void ble_event_task()
{
    for (;;)
    {
        ez_log_d(TAG_BLE, "ble event: %d", g_ble_event.event);
        if (xSemaphoreTake(g_ble_event_sem, portMAX_DELAY))
        {
            switch (g_ble_event.event)
            {
            case ESP_BLUFI_EVENT_INIT_FINISH:
                ez_log_i(TAG_EVENT, "BLUFI init finish");

                g_adv_data.p_manufacturer_data = esp_bt_dev_get_address();
                g_adv_data.manufacturer_len = 6;
                ez_log_d(TAG_SDK, "manufacturer data: %02x:%02x:%02x:%02x:%02x:%02x, len: %d",
                         g_adv_data.p_manufacturer_data[0], g_adv_data.p_manufacturer_data[1], g_adv_data.p_manufacturer_data[2],
                         g_adv_data.p_manufacturer_data[3], g_adv_data.p_manufacturer_data[4], g_adv_data.p_manufacturer_data[5], g_adv_data.manufacturer_len);
                esp_ble_gap_set_device_name(g_ble_ssid);
                esp_ble_gap_config_adv_data(&g_adv_data);
                break;
            case ESP_BLUFI_EVENT_DEINIT_FINISH:
                ez_log_i(TAG_EVENT, "BLUFI deinit finish");
                break;
            case ESP_BLUFI_EVENT_BLE_CONNECT:
                ez_log_i(TAG_EVENT, "BLUFI ble connect\n");
                g_ble_is_connected = true;
                g_server_if = g_ble_event.param.connect.server_if;
                g_conn_id = g_ble_event.param.connect.conn_id;
                ez_log_d(TAG_BLE, "server_if: %d, conn_id: %d", g_server_if, g_conn_id);
                esp_ble_gap_stop_advertising();

                break;
            case ESP_BLUFI_EVENT_BLE_DISCONNECT:
                ez_log_i(TAG_EVENT, "BLUFI ble disconnect\n");
                blufi_env.send_seq = 0;
                g_ble_is_connected = false;
                esp_ble_gap_start_advertising(&g_adv_params);
                break;

            case ESP_BLUFI_EVENT_REPORT_ERROR:
                ez_log_e(TAG_EVENT, "BLUFI report error, error code %d\n", g_ble_event.param.report_error.state);
                esp_blufi_send_error_info(g_ble_event.param.report_error.state);
                break;
            case ESP_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE:
                ez_log_i(TAG_EVENT, "blufi close a gatt connection");
                esp_blufi_close(g_server_if, g_conn_id);
                break;

            case ESP_BLUFI_EVENT_RECV_CUSTOM_DATA:
            {
                memset(g_rsp_message, 0, sizeof(g_rsp_message));
                if (0 != process_ble_request(g_req_message, g_ble_event.param.custom_data.data_len, g_rsp_message))
                {
                    break;
                }

                if (3 == get_wifi_config_status())
                {
                    if (NULL != g_time_out_timer)
                    {
                        xTimerChangePeriod(g_time_out_timer, 1, 0);
                    }
                }
                else if (2 == get_wifi_config_status())
                {
                    if (NULL != g_time_out_timer)
                    {
                        xTimerReset(g_time_out_timer, 0);
                    }
                    reset_wifi_time();
                }
                int ret = esp_blufi_send_custom_data(g_rsp_message, strlen((char *)g_rsp_message));
                ez_log_w(TAG_BLE, "send custom ret: %d, %s", ret, esp_err_to_name(ret));
            }
            break;
            default:
                break;
            }
        }
    }
}

static void example_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising(&g_adv_params);
        break;
    default:
        break;
    }
}

static void time_out_cb(TimerHandle_t pxTimer)
{
    ez_iot_ap_wifi_info_t *wifi_config = get_wifi_config();
    if (3 == get_wifi_config_status())
    {
        ez_log_w(TAG_BLE, "ble wifi config success.");

        wifi_config->err_code = ez_errno_succ;
        ez_log_w(TAG_BLE, "wifi config success. err_code: 0x%x.", wifi_config->err_code);
        g_wifi_cb(wifi_config);
    }
    else
    {
        ez_log_w(TAG_BLE, "ble time out.");
        wifi_config->err_code = ez_errno_ap_wifi_config_timeout;

        g_wifi_cb(wifi_config);

        ez_log_w(TAG_BLE, "wifi config timeout. err_code:0x%x.", wifi_config->err_code);
    }
}

int ez_iot_ble_init(ez_iot_ap_dev_info_t *dev_info, wifi_info_cb cb, int timeout)
{
    int ret = 0;

    if (NULL == dev_info)
    {
        ez_log_e(TAG_BLE, "dev_info error.");
        return -1;
    }

    if (0 == strlen(dev_info->dev_serial))
    {
        ez_log_e(TAG_BLE, "dev_serial error.");
        return -1;
    }

    if (timeout < MIN_TIME_OUT || timeout > MAX_TIME_OUT)
    {
        timeout = DEFAULT_TIME_OUT;
    }
    ez_log_w(TAG_BLE, "ble time out after %d s.", timeout * 60);

    strcpy(g_ble_ssid, dev_info->ap_ssid);
    ez_log_w(TAG_BLE, "ble_ssid:%s", dev_info->ap_ssid);
    ez_log_w(TAG_BLE, "dev_serial:%s", dev_info->dev_serial);
    ez_log_w(TAG_BLE, "dev_type:%s", dev_info->dev_type);
    ez_log_w(TAG_BLE, "dev_version:%s", dev_info->dev_version);

    memcpy(&g_dev_info, dev_info, sizeof(ez_iot_ap_dev_info_t));
    g_wifi_cb = cb;
    set_wifi_info_cb(g_wifi_cb);

    g_ble_event_sem = xSemaphoreCreateBinary();
    if (g_ble_event_sem == NULL)
    {
        return -1;
    }

    //< 创建任务处理事件触发回调
    xTaskCreate(ble_event_task, "ble_event_task", 1024 * 6, NULL, 5, NULL);

    g_time_out_timer = xTimerCreate("time out timer", (timeout * 60 * 1000) / portTICK_PERIOD_MS, pdFALSE, 0, time_out_cb);
    if (NULL != g_time_out_timer)
    {
        if (pdPASS != xTimerStart(g_time_out_timer, 0))
        {
            ez_log_e(TAG_BLE, "start time_out timer failed.");
            g_time_out_timer = NULL;
        }
    }

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (0 != ret)
    {
        ez_log_e(TAG_BLE, "initialize bt controller failed: %s", esp_err_to_name(ret));
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (0 != ret)
    {
        ez_log_e(TAG_BLE, "enable bt controller failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_init();
    if (0 != ret)
    {
        ez_log_e(TAG_BLE, "init bluedroid failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bluedroid_enable();
    if (0 != ret)
    {
        ez_log_e(TAG_BLE, "init bluedroid failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ez_log_i(TAG_BLE, "BD ADDR: " ESP_BD_ADDR_STR "", ESP_BD_ADDR_HEX(esp_bt_dev_get_address()));

    ez_log_i(TAG_BLE, "BLUFI VERSION %04x", esp_blufi_get_version());

    ret = esp_ble_gap_register_callback(example_gap_event_handler);
    if (0 != ret)
    {
        ez_log_e(TAG_BLE, "gap register failed, error code = %x", ret);
        return ret;
    }

    ret = esp_blufi_register_callbacks(&g_blufi_cbs);
    if (0 != ret)
    {
        ez_log_e(TAG_BLE, "blufi register failed, error code = %x", ret);
        return ret;
    }

    esp_blufi_profile_init();

    ez_log_w(TAG_BLE, "ble start success.");

    return ret;
}

ez_iot_ap_dev_info_t *get_ble_dev_info()
{
    return &g_dev_info;
}

int ez_iot_ble_finit()
{
    if (NULL != g_time_out_timer)
    {
        xTimerDelete(g_time_out_timer, 0);
    }

    if (NULL != g_ble_event_sem)
    {
        vSemaphoreDelete(g_ble_event_sem);
    }

    esp_blufi_profile_deinit();
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
    esp_bt_mem_release(ESP_BT_MODE_BTDM);
    ez_log_w(TAG_BLE, "ez_iot_ble_finit.");

    return 0;
}

#endif /* _FREE_RTOS_32_ */