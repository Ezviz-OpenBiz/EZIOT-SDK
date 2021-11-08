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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ez_iot_ota.h"
#include "ez_iot_log.h"
#include "hal_thread.h"

#ifdef RT_THREAD
#include <rtthread.h>
#include <finsh.h>
#endif

extern int ez_cloud_init();
extern const char *ez_cloud_get_sn();
extern const char *ez_cloud_get_ver();
extern const char *ez_cloud_get_type();
extern int ez_cloud_start();
extern void ez_cloud_deint();

static int ota_event_notify(ota_res_t *pres, ota_event_e event, void *data, int len);

static int download_data_cb(uint32_t total_len, uint32_t offset, void *data, uint32_t len, void *user_data);

static void download_result_cb(ota_cb_result_e result, void *user_data);

static void show_upgrade_info(ota_upgrade_info_t *upgrade_infos);

static char g_devsn[72] = {0};

int ez_ota_init()
{
    ota_init_t init_info = {.cb.ota_recv_msg = ota_event_notify};

    ez_iot_ota_init(&init_info);
}

int ez_ota_start()
{
    ota_res_t ota_res = {0};
    ota_module_t module = {ez_cloud_get_type(), ez_cloud_get_ver()};
    ota_modules_t modules = {1, &module};

    ez_iot_ota_modules_report(&ota_res, &modules, 5000);
    ez_iot_ota_status_ready(&ota_res, NULL);

    return 0;
}

static int ota_event_notify(ota_res_t *pres, ota_event_e event, void *data, int len)
{
    int rv = -1;

    switch (event)
    {
    case start_upgrade:
    {
        ota_upgrade_info_t *upgrade_infos = (ota_upgrade_info_t *)data;
        if (NULL == upgrade_infos || sizeof(ota_upgrade_info_t) != len)
        {
            break;
        }

        /* 异常情况 */
        if (upgrade_infos->file_num <= 0)
        {
            break;
        }

        /* 正在升级中 */
        if (0 != strlen(g_devsn))
        {
            break;
        }

        show_upgrade_info(upgrade_infos);
        strncpy(g_devsn, pres->dev_serial, sizeof(g_devsn) - 1);

        ota_download_info_t download_info = {0};
        snprintf(download_info.url, sizeof(download_info.url) - 1, "http://%s", upgrade_infos->pota_files[0].url);
        strncpy(download_info.degist, upgrade_infos->pota_files[0].degist, sizeof(download_info.degist) - 1);
        download_info.block_size = 1024 * 5;
        download_info.timeout_s = 60 * 5;
        download_info.retry_max = upgrade_infos->retry_max;
        download_info.total_size = upgrade_infos->pota_files[0].size;

        rv = ez_iot_ota_download(&download_info, download_data_cb, download_result_cb, (void *)g_devsn);
        if (0 == rv)
        {
            /* 0. ota begin */
            ota_res_t pres = {0};
            ez_iot_ota_progress_report(&pres, NULL, ota_state_starting, 0);
        }
    }
    break;
    default:
        break;
    }

    return rv;
}

static void show_upgrade_info(ota_upgrade_info_t *upgrade_infos)
{
    ez_log_i(TAG_APP, "file_num:%d", upgrade_infos->file_num);
    ez_log_i(TAG_APP, "retry_max:%d", upgrade_infos->retry_max);
    ez_log_i(TAG_APP, "interval:%d", upgrade_infos->interval);

    for (int i = 0; i < upgrade_infos->file_num; i++)
    {
        ez_log_i(TAG_APP, "pota_files[%d]->module:%s", i, upgrade_infos->pota_files[i].mod_name);
        ez_log_i(TAG_APP, "pota_files[%d]->url:%s", i, upgrade_infos->pota_files[i].url);
        ez_log_i(TAG_APP, "pota_files[%d]->fw_ver:%s", i, upgrade_infos->pota_files[i].fw_ver);
        ez_log_i(TAG_APP, "pota_files[%d]->degist:%s", i, upgrade_infos->pota_files[i].degist);
        ez_log_i(TAG_APP, "pota_files[%d]->size:%d", i, upgrade_infos->pota_files[i].size);
        if (upgrade_infos->pota_files[i].pdiffs)
        {
            ez_log_i(TAG_APP, "pdiffs.degist: %s", upgrade_infos->pota_files[i].pdiffs->degist);
            ez_log_i(TAG_APP, "pdiffs.fw_ver_src: %s", upgrade_infos->pota_files[i].pdiffs->fw_ver_dst);
            ez_log_i(TAG_APP, "pdiffs.url: %s", upgrade_infos->pota_files[i].pdiffs->url);
            ez_log_i(TAG_APP, "pdiffs.size: %d", upgrade_infos->pota_files[i].pdiffs->size);
        }
    }
}

static int download_data_cb(uint32_t total_len, uint32_t offset, void *data, uint32_t len, void *user_data)
{
    ota_res_t pres = {0};
    ez_log_d(TAG_APP, "download_data_cb, total_len:%d, offset:%d, len:%d", total_len, offset, len);

    /* 1. downloading */
    if (total_len > offset + len)
    {
        ez_iot_ota_progress_report(&pres, NULL, ota_state_downloading, ((offset + len) * (100 / 2) / total_len));
        return 0;
    }

    /* 2. integrity check and signature check */
    hal_thread_sleep(5000);

    /* 3. burning */
    ez_iot_ota_progress_report(&pres, NULL, ota_state_burning, 70);
    hal_thread_sleep(5000);

    /* 4. burning completed */
    ez_iot_ota_progress_report(&pres, NULL, ota_state_burning_completed, 80);
    hal_thread_sleep(5000);

    /* 5. reboot */
    ez_iot_ota_progress_report(&pres, NULL, ota_state_rebooting, 90);
    hal_thread_sleep(5000);

    ez_iot_ota_status_succ(&pres, NULL);

    return 0;
}

static void download_result_cb(ota_cb_result_e result, void *user_data)
{
    ota_res_t pres = {0};
    memset(g_devsn, 0, sizeof(g_devsn));

    if (result_failed == result)
    {
        ez_iot_ota_status_fail(&pres, NULL, "", ota_code_download);
    }
    else if (result_suc == result)
    {
    }

    ez_log_w(TAG_APP, "download_result_cb, result:%d", result);
}

int example_ota(int argc, char **argv)
{
    if (0 != ez_cloud_init() ||
        0 != ez_ota_init() ||
        0 != ez_cloud_start() ||
        0 != ez_ota_start())
    {
        ez_log_e(TAG_APP, "example ota init err");
    }

    return 0;
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_ota, run ez-iot-sdk example ota);
#else
// int main(int argc, char **argv)
// {
//     return example_kv(argc, argv);
// }
#endif