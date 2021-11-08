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
 * 2021-01-28     xurongjun
 *******************************************************************************/

#include <stdlib.h>
#include <flashdb.h>
#include <string.h>
#include "ut_config.h"
#include "ez_iot.h"
#include "ez_iot_ota.h"
#include "ez_iot_log.h"
#include "ez_hal/hal_thread.h"
#include "kv_imp.h"
#include "utest.h"

void ut_ota_errcode();
void ut_module_report();
void ut_progress_report();
void ut_ota_download();

UTEST_TC_EXPORT(ut_ota_errcode, NULL, NULL, DEFAULT_TIMEOUT_S);


static int ota_event_notify(ota_res_t* pres, ota_event_e event, void *data, int len);

static int32_t ez_recv_msg_cb(ez_iot_cloud2dev_msg_type_t msg_type, void *data, int len);
static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len);

static int m_event_id = -1;
static ez_iot_srv_info_t m_lbs_addr = {(int8_t *)EZ_IOT_CLOUD_ENTRY_HOST, EZ_IOT_CLOUD_ENTRY_PORT};
static ez_iot_callbacks_t m_cbs = {ez_recv_msg_cb, ez_recv_event_cb};

static ez_iot_dev_info_t m_dev_info = {
    .auth_mode = EZ_IOT_DEV_AUTH_MODE,
    .dev_subserial = EZ_IOT_DEV_UUID,
    .dev_verification_code = EZ_IOT_DEV_LICENSE,
    .dev_firmwareversion = EZ_IOT_DEV_FWVER,
    .dev_type = EZ_IOT_DEV_PRODUCT_KEY,
    .dev_typedisplay = EZ_IOT_DEV_DISPLAY_NAME,
    .dev_id = EZ_IOT_DEV_ID,
};


static ez_iot_kv_callbacks_t m_kv_cbs = {
    .ez_kv_init = kv_init,
    .ez_kv_raw_set = kv_raw_set,
    .ez_kv_raw_get = kv_raw_get,
    .ez_kv_del = kv_del,
    .ez_kv_del_by_prefix = kv_del_by_prefix,
    .ez_kv_print = kv_print,
    .ez_kv_deinit = kv_deinit,
};

void ut_ota_errcode()
{
    // ota_init_t init_info;
    // char pmodule[64]={0};
    // char fw_ver[64]={0};
    // char* errmsg="failed";
    // ota_res_t  res_t;
    // ota_modules_t module_list;
    // ota_module_t list;
    // memset(&module_list, 0, sizeof(ota_modules_t));
    // memset(&res_t, 0, sizeof(ota_res_t));
    // memset(&list, 0, sizeof(ota_module_t));

    // strncpy(pmodule, "CS-C7-3CWFR", sizeof(pmodule) - 1);
    // strncpy(fw_ver, "V5.1.3 build 170712", sizeof(fw_ver)-1);

    // list.fw_ver = (int8_t*)fw_ver;
    // list.mod_name = (int8_t*)pmodule;
    // module_list.num = 1;
    // module_list.plist = &list;

    // memset(&init_info, 0, sizeof(ota_init_t));
    // init_info.cb.ota_recv_msg = ota_event_notify;

    // //sdk core not init
    // uassert_int_equal(ez_errno_ota_not_init, ez_iot_ota_modules_report(&res_t, &module_list, 5000));
    // uassert_int_equal(ez_errno_ota_not_init, ez_iot_ota_status_ready(&res_t, &module_list));
    // uassert_int_equal(ez_errno_ota_not_init, ez_iot_ota_status_succ(&res_t, &module_list));
    // uassert_int_equal(ez_errno_ota_not_init, ez_iot_ota_status_fail(&res_t, &module_list, (int8_t*)errmsg,ota_code_download));
    // uassert_int_equal(ez_errno_ota_not_init, ez_iot_ota_progress_report(&res_t, &module_list, ota_state_downloading,20));
    // uassert_int_equal(ez_errno_ota_not_init, ez_iot_ota_deinit());

    // uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    // uassert_int_equal(ez_errno_succ, ez_iot_ota_init(&init_info));

    // uassert_int_equal(ez_errno_succ, ez_iot_start());

    // uassert_int_equal(ez_errno_ota_param_invalid, ez_iot_ota_modules_report(&res_t, NULL, 5000));
    // uassert_int_equal(ez_errno_succ, ez_iot_ota_deinit());
    // uassert_int_equal(ez_errno_succ, ez_iot_stop());
    // ez_iot_fini();
}

static int32_t ez_recv_msg_cb(ez_iot_cloud2dev_msg_type_t msg_type, void *data, int len)
{
    return 0;
}

static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len)
{
    switch (event_type)
    {
    case ez_iot_event_online:
        m_event_id = ez_iot_event_online;
        break;
    case ez_iot_event_devid_update:
        /* save devid */
        break;

    default:
        break;
    }

    return 0;
}

static int ota_event_notify(ota_res_t* pres, ota_event_e event, void *data, int len)
{
    int ret = 0;
    int i=0;
    printf("--------------------ota_event notify----------------\n");
    printf("--------------------event :%d----------------\n", event);
    switch (event)
    {
    case start_upgrade:
        {
            printf("--------------------upgrade packet info ----------------\n");
            ota_upgrade_info_t* upgrade_info_t = (ota_upgrade_info_t*)data;
            if(NULL == upgrade_info_t)
            {
                ret = -1;
                break;
            }
            printf("file_num:%d \n", upgrade_info_t->file_num);
            printf("retry_max:%d \n", upgrade_info_t->retry_max);
            printf("interval:%d \n", upgrade_info_t->interval);

           /* g_upgrade_info.file_num = upgrade_info_t->file_num;
            g_upgrade_info.retry_max = upgrade_info_t->retry_max;
            g_upgrade_info.interval = upgrade_info_t->interval;
            g_progress_interval = g_upgrade_info.interval;*/
            for( i = 0;i < upgrade_info_t->file_num; i++)
            {
                printf("pota_files[%d]->module:%s \n", i, upgrade_info_t->pota_files[i].mod_name);
                printf("pota_files[%d]->url:%s \n", i, upgrade_info_t->pota_files[i].url);
                printf("pota_files[%d]->fw_ver:%s \n", i, upgrade_info_t->pota_files[i].fw_ver);
                printf("pota_files[%d]->degist:%s \n", i, upgrade_info_t->pota_files[i].degist);
                printf("pota_files[%d]->size:%d \n", i, upgrade_info_t->pota_files[i].size);
               if(upgrade_info_t->pota_files[i].pdiffs)
                {
                    printf("pdiffs.degist: %s \n", upgrade_info_t->pota_files[i].pdiffs->degist);
                    printf("pdiffs.fw_ver_src: %s \n",upgrade_info_t->pota_files[i].pdiffs->fw_ver_dst);
                    printf("pdiffs.url: %s \n",upgrade_info_t->pota_files[i].pdiffs->url);
                    printf("pdiffs.size: %d \n",upgrade_info_t->pota_files[i].pdiffs->size);
                }
            }
        
        }
        break;
    default:
        break;
    }
    return ret;
}