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
#include "ez_iot.h"
#include "ez_iot_tsl.h"
#include "ez_iot_log.h"
#include "hal_thread.h"

#ifdef RT_THREAD
#include <rtthread.h>
#include <finsh.h>
#endif

#define NEWLINE_SIGN "\r\n"
static void *m_thread = NULL;

extern int ez_cloud_init();
extern int ez_cloud_start();
extern void ez_cloud_deint();
extern void ez_cloud_cb_subscribe(ez_iot_callbacks_t *cbs);
extern void ez_cloud_cb_unsubscribe();

static int32_t tsl_things_action2dev(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info,
                                     const tsl_value_t *value_in, tsl_value_t *value_out)
{
    return 0;
}

static int32_t tsl_things_property2cloud(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, tsl_value_t *value_out)
{
    return -1;
}

static int32_t tsl_things_property2dev(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, const tsl_value_t *value)
{
    return 1;
}

static void *dev_event_push(void *user_data)
{
    printf("example event push thread yeild%s", NEWLINE_SIGN);

    example_event_report_null();
    example_event_report_obj();

    /**
     * @brief 结束本次示例，实际开发不需要反复初始化。
     * 
     */
    hal_thread_sleep(3000);
    ez_iot_tsl_deinit();
    ez_cloud_deint();

    printf("example event push thread exit%s", NEWLINE_SIGN);
    return NULL;
}

static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len)
{
    switch (event_type)
    {
    case ez_iot_event_online:
        printf("example event recv online msg!%s", NEWLINE_SIGN);

        /**
         * @brief 启动独立任务模拟事件发送，不阻塞连接云平台任务。
         * 
         */
        m_thread = hal_thread_create((int8_t *)"event_push", dev_event_push, 1024 * 4, 2, NULL);
        hal_thread_detach(m_thread);
        m_thread = NULL;
        ez_cloud_cb_unsubscribe();

        break;

    default:
        break;
    }

    return 0;
}

void example_event_report_null()
{
    ez_err_e rv = ez_errno_succ;
    tsl_devinfo_t tsl_devinfo = {(int8_t *)ez_cloud_get_sn(), (int8_t *)ez_cloud_get_type(), (int8_t *)ez_cloud_get_ver()};

    /**
     * @brief "event_test"、"event_null"、"PetDryerRes"等功能点均为设备在开发控制台定义的功能点，
     * 该模拟设备功能点描述参考device_feautre->4LYV8SK7UKLBOUOVS6HXVX_V1.2.0_build_201201.bin
     */
    tsl_key_info_t tsl_keyinfo = {.domain = (int8_t *)"event_test", .key = (int8_t *)"event_null"};
    tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

    rv = ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, NULL);
    if (ez_errno_tsl_profile_loading == rv)
    {
        hal_thread_sleep(3000);
        rv = ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, NULL);
    }
}

void example_event_report_obj()
{
    ez_err_e rv = ez_errno_succ;

    tsl_devinfo_t tsl_devinfo = {(int8_t *)ez_cloud_get_sn(), (int8_t *)ez_cloud_get_type(), (int8_t *)ez_cloud_get_ver()};

    /**
     * @brief "event_test"、"event_ext"、"PetDryerRes"等功能点均为设备在开发控制台定义的功能点，
     * 该模拟设备功能点描述参考device_feautre->4LYV8SK7UKLBOUOVS6HXVX_V1.2.0_build_201201.bin
     */
    tsl_key_info_t tsl_keyinfo = {.domain = (int8_t *)"event_test", .key = (int8_t *)"event_ext"};
    tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};
    tsl_param_t tsl_value = {.key = (int8_t *)"ext", .value.size = strlen("{\"psd\":\"consequat sit in\"}"), .value.type = tsl_data_type_object, .value.value = (int8_t *)"{\"psd\":\"consequat sit in\"}"};

    rv = ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value);
    if (ez_errno_tsl_profile_loading == rv)
    {
        hal_thread_sleep(3000);
        rv = ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, NULL);
    }
}

static int ez_tsl_init()
{
    tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
    return ez_iot_tsl_init(&tsl_things_cbs);
}

int example_event(int argc, char **argv)
{
    /**
     * @brief 订阅设备上线事件，设备上线后再发送事件
     * 
     */
    ez_iot_callbacks_t cbs = {NULL, ez_recv_event_cb};
    ez_cloud_cb_subscribe(&cbs);

    if (0 != ez_cloud_init() ||
        0 != ez_tsl_init() ||
        0 != ez_cloud_start())
    {
        ez_log_e(TAG_APP, "example event init err");
    }

    return 0;
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_event, run ez-iot-sdk example event);
#else
// int main(int argc, char **argv)
// {
//     return example_kv(argc, argv);
// }
#endif