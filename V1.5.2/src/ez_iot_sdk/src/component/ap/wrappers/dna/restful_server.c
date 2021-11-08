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
#include <stdbool.h>

#include "dna_wlan.h"
#include "dna_os.h"
#include "dna_sockets.h"
#include "dna_httpd.h"

#include "ez_iot_ap.h"
#include "ez_iot_log.h"

#include "prenetwork_entry.h"

static const char *http_resp[] = {
    "HTTP/1.1 200 OK\r\n\r\n\0",
    "HTTP/1.1 400 Bad Request\r\n\r\n\0",
    "HTTP/1.1 401 Unauthorized\r\n\r\n\0",
    "HTTP/1.1 404 Not Found\r\n\r\n\0",
    "HTTP/1.1 405 Method Not Allowed\r\n\r\n\0",
    "HTTP/1.1 500 Internal Server Error\r\n\r\n\0"};

wifi_info_cb wifi_cb;                  // wifi配置结果回调函数
ez_iot_ap_dev_info_t g_dev_info = {0}; // 设备信息
int g_time_out_count = 0;
bool g_exit_flag = false;
dna_task_handle_t g_time_out_task;
AP_NET_DES g_ap_net_param;
ez_iot_ap_wifi_info_t g_wifi_info = {0};

static int send_http_resp(int sd, AP_NET_DES *ap_net_param)
{
    switch (ap_net_param->resp.httpcode)
    {
    case HTTP_OK:
        dna_httpd_send(sd, (unsigned char *)http_resp[0], strlen(http_resp[0]));
        break;
    case HTTP_BAD_REQUEST:
        dna_httpd_send(sd, (unsigned char *)http_resp[1], strlen(http_resp[1]));
        break;
    case HTTP_UNAUTHORIZED:
        dna_httpd_send(sd, (unsigned char *)http_resp[2], strlen(http_resp[2]));
        break;
    case HTTP_NOT_FOUND:
        dna_httpd_send(sd, (unsigned char *)http_resp[3], strlen(http_resp[3]));
        break;
    case HTTP_NOT_ALLOWED:
        dna_httpd_send(sd, (unsigned char *)http_resp[4], strlen(http_resp[4]));
        break;
    case HTTP_SERVER_ERROR:
    default:
        dna_httpd_send(sd, (unsigned char *)http_resp[5], strlen(http_resp[5]));
        break;
    }
    dna_httpd_send(sd, (unsigned char *)ap_net_param->resp.httpbody, strlen(ap_net_param->resp.httpbody));
    return AP_OK;
}

int http_get_devinfo(int sd, AP_NET_DES *ap_net_param)
{
    do
    {
        bscJSON *root = bscJSON_CreateObject();
        if (NULL == root)
        {
            ez_log_d(TAG_AP, "Json create object error.");
            ap_net_param->resp.httpcode = HTTP_SERVER_ERROR;
            break;
        }

        bscJSON_AddStringToObject(root, "ap_version", "1.0");
        bscJSON_AddStringToObject(root, "dev_subserial", g_dev_info.dev_serial);
        bscJSON_AddStringToObject(root, "dev_type", g_dev_info.dev_type);
        bscJSON_AddStringToObject(root, "dev_firmwareversion", g_dev_info.dev_version);

        char *json_str = bscJSON_PrintUnformatted(root);
        if (NULL == json_str)
        {
            ez_log_d(TAG_AP, "Json print error.");
            bscJSON_Delete(root);
            ap_net_param->resp.httpcode = HTTP_SERVER_ERROR;
            break;
        }

        memcpy(ap_net_param->resp.httpbody, json_str, strlen(json_str));
        bscJSON_Delete(root);
        free(json_str);
        ap_net_param->resp.httpcode = HTTP_OK;
    } while (false);
    send_http_resp(sd, ap_net_param);
    return AP_OK;
}

int http_get_point(int sd, AP_NET_DES *ap_net_param)
{
    prenetwork_entry(ap_net_param);
    send_http_resp(sd, ap_net_param);
    return AP_OK;
}

int http_put_wifi(int sd, AP_NET_DES *ap_net_param)
{
    prenetwork_entry(ap_net_param);
    if (ap_net_param->resp.httpcode != HTTP_OK)
    {
        return -1;
    }
    memcpy(&g_wifi_info, &ap_net_param->wifi_info, sizeof(ez_iot_ap_wifi_info_t));
    g_exit_flag = true;

    return AP_OK;
}

int dna_http_handler(int sd, int method, const char *hostname, const char *path, const char *body, int len)
{
    if (NULL == hostname || NULL == path)
    {
        ez_log_d(TAG_AP, "http param error.");
        return -1;
    }
    memset(&g_ap_net_param, 0, sizeof(AP_NET_DES));

    if (DNA_HTTPD_METHOD_GET == method && 0 == strcmp(path, "/AccessDevInfo"))
    {
        g_ap_net_param.req.method = METHOD_GET;
        memcpy(g_ap_net_param.req.url, path, strlen(path));
        memcpy(g_ap_net_param.req.httpbody, body, len);
        http_get_devinfo(sd, &g_ap_net_param);
    }
    else if (DNA_HTTPD_METHOD_GET == method && 0 == strcmp(path, "/PreNetwork/SecurityAndAccessPoint"))
    {
        g_ap_net_param.req.method = METHOD_GET;
        memcpy(g_ap_net_param.req.url, path, strlen(path));
        memcpy(g_ap_net_param.req.httpbody, body, len);
        http_get_point(sd, &g_ap_net_param);
    }
    else if (DNA_HTTPD_METHOD_PUT == method && 0 == strcmp(path, "/PreNetwork/WifiConfig"))
    {
        dna_httpd_send(sd, (unsigned char *)http_resp[0], strlen(http_resp[0]));
        g_ap_net_param.req.method = METHOD_PUT;
        memcpy(g_ap_net_param.req.url, path, strlen(path));
        memcpy(g_ap_net_param.req.httpbody, body, len);
        http_put_wifi(sd, &g_ap_net_param);
    }
    else
    {
        dna_httpd_send(sd, (unsigned char *)http_resp[5], strlen(http_resp[5]));
    }

    return 0;
}

int start_webserver(void)
{
    if (0 != dna_httpd_init(80))
    {
        ez_log_e(TAG_AP, "http server init failed.");
        return -1;
    }
    if (0 != dna_httpd_event_register(dna_http_handler))
    {
        ez_log_e(TAG_AP, "http event regist failed.");
        return -1;
    }
    return 0;
}

void time_out_check(void *arg)
{
    int i = 0;
    int j = 0;

    for (i = 0; i < g_time_out_count; ++i)
    {
        for (j = 0; j < 60; ++j)
        {
            if (g_exit_flag)
            {
                ez_log_w(TAG_AP, "wifi config success, I need detach.");

                ez_log_w(TAG_AP, "wifi_ssid:%s", g_wifi_info.ssid);
                ez_log_w(TAG_AP, "wifi_passwd:%s", g_wifi_info.password);
                ez_log_w(TAG_AP, "lbs_domain:%s", g_wifi_info.domain);
                ez_iot_ap_finit();
                wifi_cb(&g_wifi_info);

                dna_task_delete(NULL);
                return;
            }

            dna_task_sleep(1000);
        }
    }
    ez_log_w(TAG_AP, "wifi config %d min time out, stop now.\n", g_time_out_count);
    ez_iot_ap_wifi_info_t wifi_info = {0};
    wifi_info.err_code = ez_errno_ap_wifi_config_timeout;
    ez_iot_ap_finit();
    wifi_cb(&wifi_info);
    dna_task_delete(NULL);
}

ez_err_e ez_iot_ap_init(ez_iot_ap_dev_info_t *dev_info, wifi_info_cb cb, int timeout)
{
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
    g_time_out_count = timeout;
    g_exit_flag = false;

    ez_log_d(TAG_AP, "ap time out after %dmin.", g_time_out_count);

    int task_stack_size = 4 * 1024;
    g_time_out_task = dna_task_create("ap_time_out_task", task_stack_size, 1, time_out_check, NULL);

    if (NULL == g_time_out_task)
    {
        ez_log_e(TAG_AP, "create time out thread failed.\n");
        return ERROR;
    }

    if (dev_info->dev_serial[71] != '\0')
    {
        dev_info->dev_serial[71] = '\0';
    }

    char ap_ssid[64] = {0};
    bool has_colon = false;
    int i = 0;
    for (i = 0; i < strlen(dev_info->dev_serial); i++)
    {
        if (dev_info->dev_serial[i] == ':')
        {
            has_colon = true;
            break;
        }
    }

    if (has_colon)
    {
        sprintf(ap_ssid, "%s_%s", dev_info->ap_prefix, &dev_info->dev_serial[i + 1]);
    }
    else
    {
        sprintf(ap_ssid, "%s_%s", dev_info->ap_prefix, dev_info->dev_serial);
    }

    if (ap_ssid[31] != '\0')
    {
        ap_ssid[31] = '\0';
    }
    ez_log_w(TAG_AP, "ap_ssid:%s, passwd:NULL", ap_ssid);
    ez_log_w(TAG_AP, "dev_serial:%s", dev_info->dev_serial);
    ez_log_w(TAG_AP, "dev_type:%s", dev_info->dev_type);
    ez_log_w(TAG_AP, "dev_version:%s", dev_info->dev_version);

    memcpy(&g_dev_info, dev_info, sizeof(ez_iot_ap_dev_info_t));
    wifi_cb = cb;

    dna_ipconfig_t ip_config = {0};
    ip_config.ip = dna_inet_addr("192.168.4.1");
    ip_config.gw = dna_inet_addr("192.168.4.1");
    ip_config.netmask = dna_inet_addr("255.255.255.0");

    dna_wlan_ap_start(ap_ssid, NULL, &ip_config, 1);
    bool b_ap_start = false;
    for (size_t i = 0; i < 10; i++)
    {
        if (1 == dna_wlan_ap_status())
        {
            b_ap_start = true;
            break;
        }
        dna_task_sleep(1000);
    }
    if (!b_ap_start)
    {
        ez_log_e(TAG_AP, "ap start failed.");
        return ERROR;
    }

    ez_log_d(TAG_AP, "ap start success.");
    if (0 != start_webserver())
    {
        ez_log_e(TAG_AP, "http server start failed.");
        dna_wlan_ap_stop();
        return ERROR;
    }

    return AP_OK;
}

ez_err_e ez_iot_ap_finit()
{
    dna_httpd_deinit();
    dna_wlan_ap_stop();
    ez_log_w(TAG_AP, "ap stop success.");
    return 0;
}