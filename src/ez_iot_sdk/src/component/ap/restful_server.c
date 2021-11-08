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

#include "http_server.h"
#include "prenetwork_entry.h"
#include "hal_wifi_drv.h"
#include "timer/timer.h"

#include "ez_iot_ap.h"
#include "ez_iot_log.h"
#include "hal_thread.h"

static void *g_time_out_timer = NULL;
httpd_handle_t g_server;                 // httpserver定义
wifi_info_cb wifi_cb;                    // wifi配置结果回调函数
ez_iot_ap_dev_info_t g_dev_info = {0};   // 设备信息
ez_iot_ap_wifi_info_t g_wifi_info = {0}; // wifi信息
bool g_exit_flag = true;
bool g_server_busy = false;
bool g_support_apsta = false;

AP_NET_DES *g_ap_net_param = NULL;

bool is_support_apsta()
{
    return g_support_apsta;
}

int reset_wifi_time()
{
    if (NULL != g_time_out_timer)
    {
        ez_timer_reset(g_time_out_timer);
    }
    return 0;
}

static void stop_webserver()
{
    if (NULL != g_server)
    {
        ez_log_d(TAG_AP, "stopping httpserver...");
        httpd_stop(g_server);
        g_server = NULL;
        hal_thread_sleep(1000);
        ez_log_d(TAG_AP, "stop httpserver success.");
    }
    else
    {
        ez_log_d(TAG_AP, "http server is NULL.");
    }

    if (NULL != g_ap_net_param)
    {
        free(g_ap_net_param);
        g_ap_net_param = NULL;
    }
}

static int send_http_resp(httpd_req_t *req, AP_NET_DES *ap_net_param)
{
    switch (ap_net_param->resp.httpcode)
    {
    case HTTP_OK:
        httpd_resp_set_status(req, HTTPD_200);
        httpd_resp_set_type(req, HTTPD_TYPE_JSON);
        break;
    case HTTP_BAD_REQUEST:
        httpd_resp_set_status(req, HTTPD_400);
        httpd_resp_set_type(req, HTTPD_TYPE_JSON);
        break;
    case HTTP_UNAUTHORIZED:
        httpd_resp_set_status(req, "401 Unauthorized");
        httpd_resp_set_type(req, HTTPD_TYPE_JSON);
        break;
    case HTTP_NOT_FOUND:
        httpd_resp_set_status(req, HTTPD_404);
        httpd_resp_set_type(req, HTTPD_TYPE_JSON);
        break;
    case HTTP_NOT_ALLOWED:
        httpd_resp_set_status(req, "405 Method Not Allowed");
        httpd_resp_set_type(req, HTTPD_TYPE_JSON);
        break;
    case HTTP_SERVER_ERROR:
        httpd_resp_set_status(req, HTTPD_500);
        httpd_resp_set_type(req, HTTPD_TYPE_JSON);
        break;
    default:
        // TODO
        break;
    }

    httpd_resp_send(req, ap_net_param->resp.httpbody, strlen(ap_net_param->resp.httpbody));
    ez_log_v(TAG_AP, "rsp http body: %s", ap_net_param->resp.httpbody);
    return AP_OK;
}

ez_err_e http_get_devinfo_handler(httpd_req_t *req)
{
    while (g_server_busy)
    {
        hal_thread_sleep(200);

        ez_log_i(TAG_AP, "http server is busy.");
        if (g_exit_flag)
        {
            ez_log_i(TAG_AP, "get devinfo handler exit.");
            return AP_OK;
        }
    }

    g_server_busy = true;

    ez_iot_ap_wifi_info_t wifi_info = {0};
    wifi_info.err_code = ez_errno_ap_app_connected;
    wifi_cb(&wifi_info);

    ez_log_i(TAG_AP, "app connected success. err_code:0x%x.", wifi_info.err_code);

    memset(g_ap_net_param, 0, sizeof(AP_NET_DES));

    do
    {
        bscJSON *root = bscJSON_CreateObject();
        if (NULL == root)
        {
            ez_log_e(TAG_AP, "json create object error.");
            g_ap_net_param->resp.httpcode = HTTP_SERVER_ERROR;
            break;
        }

        bscJSON_AddStringToObject(root, "ap_version", "1.0");
        bscJSON_AddStringToObject(root, "dev_subserial", g_dev_info.dev_serial);
        bscJSON_AddStringToObject(root, "dev_type", g_dev_info.dev_type);
        bscJSON_AddStringToObject(root, "dev_firmwareversion", g_dev_info.dev_version);

        char *json_str = bscJSON_PrintUnformatted(root);
        if (NULL == json_str)
        {
            ez_log_e(TAG_AP, "json print error.");
            bscJSON_Delete(root);
            g_ap_net_param->resp.httpcode = HTTP_SERVER_ERROR;
            break;
        }

        memcpy(g_ap_net_param->resp.httpbody, json_str, strlen(json_str));
        bscJSON_Delete(root);
        free(json_str);
        g_ap_net_param->resp.httpcode = HTTP_OK;
    } while (false);
    send_http_resp(req, g_ap_net_param);

    ez_log_i(TAG_AP, "get dev_info success.");
    g_server_busy = false;
    return AP_OK;
}

ez_err_e http_get_point_handler(httpd_req_t *req)
{
    while (g_server_busy)
    {
        hal_thread_sleep(200);

        ez_log_i(TAG_AP, "http server is busy.");
        if (g_exit_flag)
        {
            ez_log_i(TAG_AP, "get ap point handler exit.");

            return AP_OK;
        }
    }
    g_server_busy = true;

    memset(g_ap_net_param, 0, sizeof(AP_NET_DES));

    g_ap_net_param->req.method = METHOD_GET;
    strcpy(g_ap_net_param->req.url, "/PreNetwork/SecurityAndAccessPoint");

    prenetwork_entry(g_ap_net_param);
    send_http_resp(req, g_ap_net_param);

    ez_log_i(TAG_AP, "get ap list success.");
    g_server_busy = false;
    return AP_OK;
}

ez_err_e http_put_wifi_handler(httpd_req_t *req)
{
    if (g_exit_flag)
    {
        return AP_OK;
    }
    while (g_server_busy)
    {
        hal_thread_sleep(200);

        ez_log_i(TAG_AP, "http server is busy.");
        if (g_exit_flag)
        {
            ez_log_i(TAG_AP, "put wifi handler exit.");

            return AP_OK;
        }
    }
    g_server_busy = true;

    memset(g_ap_net_param, 0, sizeof(AP_NET_DES));

    g_ap_net_param->req.method = METHOD_PUT;
    strcpy(g_ap_net_param->req.url, "/PreNetwork/WifiConfig");

    int ret = 0;
    int remaining = req->content_len;
    while (remaining > 0)
    {
        if ((ret = httpd_req_recv(req, g_ap_net_param->req.httpbody, MINNUM(remaining, (int)sizeof(g_ap_net_param->req.httpbody)))) <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                continue;
            }
            break;
        }
        remaining -= ret;
    }
    ez_log_d(TAG_AP, "req.httpbody:%s.", g_ap_net_param->req.httpbody);


    if (!g_support_apsta)
    {
        g_ap_net_param->wifi_info.err_code = EZVIZ_WIFI_STATE_CONNECT_SUCCESS;
        encapsulate_httpbody_with_errcode(g_ap_net_param);
        send_http_resp(req, g_ap_net_param);

        ez_log_e(TAG_AP, "wifi config success.");
    }

    prenetwork_entry(g_ap_net_param);

    ez_iot_ap_wifi_info_t wifi_info = {0};
    wifi_info.err_code = ez_errno_ap_connecting_route;
    wifi_cb(&wifi_info);
    ez_log_i(TAG_AP, "connecting route..., err_code:0x%x.", wifi_info.err_code);


    int i = 0;
    int wifi_state;
    for (i = 0; i < 100; ++i) // wifi连接超时时间10s
    {
    	//如果不支持共存模式，连接路由器时间改为30s
	    if (g_support_apsta)
	    {
        	hal_thread_sleep(100);
        }
        else
        {
        	hal_thread_sleep(300);
        }
        
        wifi_state = iot_sta_get_state();
        if (wifi_state == EZVIZ_WIFI_STATE_CONNECT_SUCCESS)
        {
            break;
        }
    }
    g_ap_net_param->wifi_info.err_code = wifi_state;
    if (g_ap_net_param->wifi_info.err_code != EZVIZ_WIFI_STATE_CONNECT_SUCCESS)
    {
        iot_sta_disconnect(); // 若10秒之后还未连接上wifi，则停止当前连接
    }

    if (g_support_apsta)
    {
        encapsulate_httpbody_with_errcode(g_ap_net_param);

        send_http_resp(req, g_ap_net_param);
    }

    // 如果wifi已连接，则需要等app重新连接ap，才可以发送http resp
    if (g_support_apsta)
    {
	    if (g_ap_net_param->wifi_info.err_code == EZVIZ_WIFI_STATE_CONNECT_SUCCESS)
	    {
	        for (i = 0; i < 100; ++i) // 设备连上wifi到手机重新连接上ap，需要一定的时间
	        {
	            wifi_state = iot_sta_get_state();
	            if (wifi_state == EZVIZ_WIFI_STATE_AP_STA_CONNECTED) // 检测到app端重新连接上ap，则跳出循环
	            {               
	                send_http_resp(req, g_ap_net_param);
	                hal_thread_sleep(3000);
	                break;
	            }
	            hal_thread_sleep(100);
	        }
	    }
	    else // 若wifi未连接，则不需要等ap重新连接
	    {
	        hal_thread_sleep(3000);
	    }
    }

    if (g_ap_net_param->wifi_info.err_code == EZVIZ_WIFI_STATE_CONNECT_SUCCESS)
    {
        ez_log_e(TAG_AP, "wifi config success.");
        memcpy(&g_wifi_info, &g_ap_net_param->wifi_info, sizeof(ez_iot_ap_wifi_info_t));
        g_exit_flag = true;
        g_server_busy = true;
        if (NULL != g_time_out_timer)
        {
            ez_timer_change_period(g_time_out_timer, 1000);
        }
    }
    else
    {
        ez_log_e(TAG_AP, "Wifi config error_code:%d", g_ap_net_param->wifi_info.err_code);
        ez_iot_ap_wifi_info_t wifi_info = {0};
        wifi_info.err_code = ez_errno_ap_connect_failed;
        wifi_cb(&wifi_info);
        g_server_busy = false;
        if (!g_support_apsta)
        {
            if (NULL != g_time_out_timer)
            {
                ez_timer_change_period(g_time_out_timer, 1000);
            }
        }
        else
        {
            reset_wifi_time();
        }

        ez_log_i(TAG_AP, "wifi connect failed. err_code:0x%x.", wifi_info.err_code);
    }

    ez_log_i(TAG_AP, "wifi config end.");
    return AP_OK;
}

httpd_uri_t get_dev_info = {
    .uri = "/AccessDevInfo",
    .method = HTTP_GET,
    .handler = http_get_devinfo_handler,
    .user_ctx = NULL};

httpd_uri_t get_access_point = {
    .uri = "/PreNetwork/SecurityAndAccessPoint",
    .method = HTTP_GET,
    .handler = http_get_point_handler,
    .user_ctx = NULL};

httpd_uri_t put_wifi_config = {
    .uri = "/PreNetwork/WifiConfig",
    .method = HTTP_PUT,
    .handler = http_put_wifi_handler,
    .user_ctx = NULL};


httpd_handle_t start_webserver(void)
{
    httpd_handle_t tmp_server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8 * 1024;
    config.max_open_sockets = 2;
    config.backlog_conn = 2;
    config.max_uri_handlers = 3;
    config.lru_purge_enable = true;

    do
    {
        if (AP_OK != httpd_start(&tmp_server, &config))
        {
            ez_log_e(TAG_AP, "httpd_start error");
            tmp_server = NULL;
            free(g_ap_net_param);
            g_ap_net_param = NULL;
            break;
        }

        if (ez_errno_succ != httpd_register_uri_handler(tmp_server, &get_dev_info))
        {
            ez_log_e(TAG_AP, "register GET dev info handler error.");
            tmp_server = NULL;
            stop_webserver();
            break;
        }

        if (ez_errno_succ != httpd_register_uri_handler(tmp_server, &get_access_point))
        {
            ez_log_e(TAG_AP, "register GET wifi point handler error.");
            tmp_server = NULL;
            stop_webserver();
            break;
        }
        if (ez_errno_succ != httpd_register_uri_handler(tmp_server, &put_wifi_config))
        {
            ez_log_e(TAG_AP, "register PUT wifi config handler error.");
            tmp_server = NULL;
            stop_webserver();
            break;
        }
        
    } while (0);

    return tmp_server;
}

static int time_out_cb(void)
{
    if (g_exit_flag)
    {
        ez_log_w(TAG_AP, "wifi config success, I need detach.");

        g_wifi_info.err_code = ez_errno_succ;
        ez_log_i(TAG_AP, "wifi config success. err_code:0x%x.", g_wifi_info.err_code);
        wifi_cb(&g_wifi_info);

        return 0;
    }

    ez_log_w(TAG_AP, "Wifi config time out, stop now.");
    ez_iot_ap_wifi_info_t wifi_info = {0};
    wifi_info.err_code = ez_errno_ap_wifi_config_timeout;
    g_exit_flag = true;
    wifi_cb(&wifi_info);

    ez_log_i(TAG_AP, "wifi config timeout. err_code:0x%x.", wifi_info.err_code);
    return 0;
}

ez_err_e ez_iot_ap_init(ez_iot_ap_dev_info_t *dev_info, wifi_info_cb cb, int timeout, bool support_apsta)
{
    if (!g_exit_flag)
    {
        ez_log_w(TAG_AP, "ap not exit, can not init again.");
        return ERROR;
    }

    if (NULL == dev_info)
    {
        ez_log_e(TAG_AP, "dev_info error.");
        return ERROR;
    }

    if (0 == strlen(dev_info->dev_serial))
    {
        ez_log_e(TAG_AP, "dev_serial error.");
        return ERROR;
    }

    if (timeout < MIN_TIME_OUT || timeout > MAX_TIME_OUT)
    {
        timeout = DEFAULT_TIME_OUT;
    }
    ez_log_w(TAG_AP, "ap time out after %d s.", timeout * 60);

    g_exit_flag = false;
    g_server_busy = false;
    g_support_apsta = support_apsta;
    
    g_time_out_timer = ez_timer_create("time out", (timeout * 60 * 1000), false, time_out_cb);
    if (NULL == g_time_out_timer)
    {
        ez_log_e(TAG_AP, "start time_out timer failed.");
        return ERROR;
    }
    
    ez_log_w(TAG_AP, "dev_serial:%s", dev_info->dev_serial);
    ez_log_w(TAG_AP, "dev_type:%s", dev_info->dev_type);
    ez_log_w(TAG_AP, "dev_version:%s", dev_info->dev_version);

    memcpy(&g_dev_info, dev_info, sizeof(ez_iot_ap_dev_info_t));
    wifi_cb = cb;

    if (NULL == (g_ap_net_param = (AP_NET_DES *)malloc(sizeof(AP_NET_DES))))
    {
        ez_log_e(TAG_AP, "g_ap_net_param malloc failed.");
        g_exit_flag = true;
        return ERROR;
    }

    if (0 == strlen(dev_info->ap_password))
    {
        ez_log_w(TAG_AP, "ap_ssid:%s, ap_password:NULL", dev_info->ap_ssid);
    }
    else
    {
        ez_log_w(TAG_AP, "ap_ssid:%s", dev_info->ap_ssid);
        ez_log_d(TAG_AP, "ap_password:%s", dev_info->ap_password);
    }
    ez_log_w(TAG_AP, "ap auth_mode:%d", dev_info->auth_mode);

    iot_ap_start(dev_info->ap_ssid, dev_info->ap_password, dev_info->auth_mode, support_apsta);

    g_server = start_webserver();

    if (NULL == g_server)
    {
        ez_log_e(TAG_AP, "http server start failed.");
        iot_wifi_finit();
        if (NULL != g_ap_net_param)
        {
            free(g_ap_net_param);
            g_ap_net_param = NULL;
        }
        g_exit_flag = true;
        return ERROR;
    }

    ez_log_w(TAG_AP, "ap start success.");
    return AP_OK;
}

ez_err_e ez_iot_ap_finit()
{
    //if (NULL != g_time_out_timer)
    //{
    //    ez_timer_delete(g_time_out_timer);
    //}
    stop_webserver();
    iot_ap_stop();
    
    g_exit_flag = true;

    return 0;
}

ez_err_e ez_iot_wifi_init()
{
    iot_wifi_init();
    return 0;
}

ez_err_e ez_iot_sta_init(char *ssid, char *password)
{
    iot_sta_connect(ssid, password);
    return 0;
}
