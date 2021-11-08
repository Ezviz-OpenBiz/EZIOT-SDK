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

extern int ez_cloud_init();
extern int ez_cloud_start();
extern void ez_cloud_deint();

/**
 * @brief 处理云端下发的操作命令
 * 
 */
static int32_t tsl_things_action2dev(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info,
                                     const tsl_value_t *value_in, tsl_value_t *value_out)
{
    int32_t rv = -1;
    printf("recv action%s", NEWLINE_SIGN);

    printf("device uuid:%s%s", sn, NEWLINE_SIGN);                      // 非网关类的单品设备，序列号就是设备本身
    printf("resourceCategory:%s%s", rsc_info->res_type, NEWLINE_SIGN); // 资源类型, 参考device_feautre->4LYV8SK7UKLBOUOVS6HXVX_V1.2.0_build_201201.bin
    printf("localIndex:%s%s", rsc_info->local_index, NEWLINE_SIGN);    // 通道号, 参考device_feautre->4LYV8SK7UKLBOUOVS6HXVX_V1.2.0_build_201201.bin

    if (0 == strcmp("action_test", key_info->domain))
    {
        if (0 == strcmp("action_null", key_info->key))
        {
            //TODO 设备执行操作
            printf("action_null input:null%s", NEWLINE_SIGN);
            rv = 0;
        }
        else if (0 == strcmp("action_int", key_info->key))
        {
            //TODO 设备执行操作
            printf("action_int input:%d%s", value_in->value_int, NEWLINE_SIGN);
            rv = 0;
        }
        else if (0 == strcmp("action_str", key_info->key))
        {
            //TODO 设备执行操作
            printf("action_str input:%s%s", (char *)value_in->value, NEWLINE_SIGN);
            rv = 0;
        }
        else if (0 == strcmp("action_num", key_info->key))
        {
            //TODO 设备执行操作
            if (tsl_data_type_int == value_in->type)
            {
                printf("action_num input:%d%s", value_in->value_int, NEWLINE_SIGN);
            }
            else
            {
                printf("action_num input:%ld%s", value_in->value_double, NEWLINE_SIGN);
            }

            rv = 0;
        }
        else if (0 == strcmp("action_bool", key_info->key))
        {
            //TODO 设备执行操作
            printf("action_bool input:%d%s", value_in->value_int, NEWLINE_SIGN);
            rv = 0;
        }
        else if (0 == strcmp("action_obj", key_info->key))
        {
            //TODO 设备执行操作
            printf("action_obj input:%s%s", (char *)value_in->value, NEWLINE_SIGN);
            rv = 0;
        }
        else if (0 == strcmp("action_array", key_info->key))
        {
            //TODO 设备执行操作
            printf("action_array input:%s%s", (char *)value_in->value, NEWLINE_SIGN);
            rv = 0;
        }
    }

    return rv;
}

static int32_t tsl_things_property2cloud(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, tsl_value_t *value_out)
{
    return -1;
}

static int32_t tsl_things_property2dev(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, const tsl_value_t *value)
{
    return 1;
}

static int ez_tsl_init()
{
    tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
    return ez_iot_tsl_init(&tsl_things_cbs);
}

int example_action(int argc, char **argv)
{
    /**
     * @brief 订阅设备上线事件，设备上线后再发送事件
     * 
     */
    if (0 != ez_cloud_init() ||
        0 != ez_tsl_init() ||
        0 != ez_cloud_start())
    {
        ez_log_e(TAG_APP, "example event init err");
    }

    return 0;
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_action, run ez - iot - sdk example action);
#else
// int main(int argc, char **argv)
// {
//     return example_kv(argc, argv);
// }
#endif