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

#define KV_RSCC "PetDryerRes"
#define KV_LOCALINDEX "0"
#define KV_DOMAIN "attr_test"

#define KV_NAME1 "attr_r_bool"
#define KV_NAME2 "attr_rw_bool"
#define KV_NAME3 "attr_rw_int"
#define KV_NAME4 "attr_r_int"
#define KV_NAME5 "attr_rw_str"
#define KV_NAME6 "attr_r_str"
#define KV_NAME7 "attr_rw_num"
#define KV_NAME8 "attr_r_num"
#define KV_NAME9 "attr_rw_obj"
#define KV_NAME10 "attr_r_obj"
#define KV_NAME11 "attr_r_array"
#define KV_NAME12 "attr_rw_array"
#define KV_NAME13 "attr_rw_int_multi"
#define KV_NAME14 "attr_rw_int_enum"
#define KV_NAME15 "attr_rw_str_enum"

static void *m_thread = NULL;

extern int ez_cloud_init();
extern int ez_cloud_start();
extern void ez_cloud_deint();
extern void ez_cloud_cb_subscribe(ez_iot_callbacks_t *cbs);
extern void ez_cloud_cb_unsubscribe();

extern kv_err_e example_kv_init(ez_iot_kv_t *default_kv);
extern kv_err_e example_kv_raw_set(const int8_t *key, int8_t *value, uint32_t length);
extern kv_err_e example_kv_raw_get(const int8_t *key, int8_t *value, uint32_t *length);
extern kv_err_e example_kv_del(const int8_t *key);
extern kv_err_e example_kv_del_by_prefix(const int8_t *key_prefix);
extern void example_kv_print(void);
extern void example_kv_deinit(void);

typedef struct
{
    ez_iot_kv_node_t kv;
    tsl_data_type_e type;
} kv_type_t;

static const int bvalue = 0;
static const int ivalue = 99;
static const double dvalue = 9.99;
static const int evalue1 = 2;
static const int evalue2 = 30;

static const int bvalue_chg1 = 1;
static const int ivalue_chg1 = 88;
static const double dvalue_chg1 = 8.88;
static const int evalue1_chg1 = 2;
static const int evalue2_chg1 = 20;

static const int bvalue_chg2 = 0;
static const int ivalue_chg2 = 77;
static const double dvalue_chg2 = 7.77;
static const int evalue1_chg2 = 1;
static const int evalue2_chg2 = 10;

/**
 * @brief 默认数据
 * 
 */
static kv_type_t default_kv_set[] = {
    {{KV_NAME1, &bvalue, sizeof(bvalue)}, tsl_data_type_bool},
    {{KV_NAME2, &bvalue, sizeof(bvalue)}, tsl_data_type_bool},
    {{KV_NAME3, &ivalue, sizeof(ivalue)}, tsl_data_type_int},
    {{KV_NAME4, &ivalue, sizeof(ivalue)}, tsl_data_type_int},
    {{KV_NAME5, "99", 2}, tsl_data_type_string},
    {{KV_NAME6, "99", 2}, tsl_data_type_string},
    {{KV_NAME7, &dvalue, sizeof(dvalue)}, tsl_data_type_double},
    {{KV_NAME8, &dvalue, sizeof(dvalue)}, tsl_data_type_double},
    {{KV_NAME9, "{\"value\":\"99\"}", 14}, tsl_data_type_object},
    {{KV_NAME10, "{\"value\":\"99\"}", 14}, tsl_data_type_object},
    {{KV_NAME11, "[{\"p_int\":99,\"p_enum\":30},{\"p_int\":88,\"p_enum\":30}]", 51}, tsl_data_type_array},
    {{KV_NAME12, "[{\"p_int\":99,\"p_enum\":30},{\"p_int\":88,\"p_enum\":30}]", 51}, tsl_data_type_array},
    {{KV_NAME13, &evalue2, sizeof(evalue2)}, tsl_data_type_int},
    {{KV_NAME14, &evalue1, sizeof(evalue1)}, tsl_data_type_int},
    {{KV_NAME15, "enum3", 5}, tsl_data_type_string}};

/**
 * @brief 第一次变更的数据
 * 
 */
static kv_type_t default_kv_set_1[] = {
    {{KV_NAME1, &bvalue_chg1, sizeof(bvalue_chg1)}, tsl_data_type_bool},
    {{KV_NAME2, &bvalue_chg1, sizeof(bvalue_chg1)}, tsl_data_type_bool},
    {{KV_NAME3, &ivalue_chg1, sizeof(ivalue_chg1)}, tsl_data_type_int},
    {{KV_NAME4, &ivalue_chg1, sizeof(ivalue_chg1)}, tsl_data_type_int},
    {{KV_NAME5, "88", 2}, tsl_data_type_string},
    {{KV_NAME6, "88", 2}, tsl_data_type_string},
    {{KV_NAME7, &dvalue_chg1, sizeof(dvalue_chg1)}, tsl_data_type_double},
    {{KV_NAME8, &dvalue_chg1, sizeof(dvalue_chg1)}, tsl_data_type_double},
    {{KV_NAME9, "{\"value\":\"88\"}", 14}, tsl_data_type_object},
    {{KV_NAME10, "{\"value\":\"88\"}", 14}, tsl_data_type_object},
    {{KV_NAME11, "[{\"p_int\":88,\"p_enum\":20},{\"p_int\":88,\"p_enum\":20}]", 51}, tsl_data_type_array},
    {{KV_NAME12, "[{\"p_int\":88,\"p_enum\":20},{\"p_int\":88,\"p_enum\":20}]", 51}, tsl_data_type_array},
    {{KV_NAME13, &evalue2_chg1, sizeof(evalue2_chg1)}, tsl_data_type_int},
    {{KV_NAME14, &evalue1_chg1, sizeof(evalue1_chg1)}, tsl_data_type_int},
    {{KV_NAME15, "enum2", 5}, tsl_data_type_string}};

/**
 * @brief 第二次变更的数据
 * 
 */
static kv_type_t default_kv_set_2[] = {
    {{KV_NAME1, &bvalue_chg2, sizeof(bvalue_chg2)}, tsl_data_type_bool},
    {{KV_NAME2, &bvalue_chg2, sizeof(bvalue_chg2)}, tsl_data_type_bool},
    {{KV_NAME3, &ivalue_chg2, sizeof(ivalue_chg2)}, tsl_data_type_int},
    {{KV_NAME4, &ivalue_chg2, sizeof(ivalue_chg2)}, tsl_data_type_int},
    {{KV_NAME5, "77", 2}, tsl_data_type_string},
    {{KV_NAME6, "77", 2}, tsl_data_type_string},
    {{KV_NAME7, &dvalue_chg2, sizeof(dvalue_chg2)}, tsl_data_type_double},
    {{KV_NAME8, &dvalue_chg2, sizeof(dvalue_chg2)}, tsl_data_type_double},
    {{KV_NAME9, "{\"value\":\"77\"}", 14}, tsl_data_type_object},
    {{KV_NAME10, "{\"value\":\"77\"}", 14}, tsl_data_type_object},
    {{KV_NAME11, "[{\"p_int\":77,\"p_enum\":20},{\"p_int\":77,\"p_enum\":20}]", 51}, tsl_data_type_array},
    {{KV_NAME12, "[{\"p_int\":77,\"p_enum\":20},{\"p_int\":77,\"p_enum\":20}]", 51}, tsl_data_type_array},
    {{KV_NAME13, &evalue2_chg2, sizeof(evalue2_chg2)}, tsl_data_type_int},
    {{KV_NAME14, &evalue1_chg2, sizeof(evalue1_chg2)}, tsl_data_type_int},
    {{KV_NAME15, "enum1", 5}, tsl_data_type_string}};

static int find_index_by_key(const char *key)
{
    int index = -1;

    for (size_t i = 0; i < sizeof(default_kv_set) / sizeof(kv_type_t); i++)
    {
        if (0 == strcmp(default_kv_set[i].kv.key, key))
        {
            index = i;
            break;
        }
    }

    return index;
}

static int32_t tsl_things_action2dev(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info,
                                     const tsl_value_t *value_in, tsl_value_t *value_out)
{
    return 0;
}

/**
 * @brief 设备属性上报
 * 
 */
static int32_t tsl_things_property2cloud(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, tsl_value_t *value_out)
{
    printf("tsl_things_property2cloud%s", NEWLINE_SIGN);

    int32_t rv;
    void *p = NULL;

    printf("device uuid:%s%s", sn, NEWLINE_SIGN);                      // 非网关类的单品设备，序列号就是设备本身
    printf("resourceCategory:%s%s", rsc_info->res_type, NEWLINE_SIGN); // 资源类型, 参考device_feautre->4LYV8SK7UKLBOUOVS6HXVX_V1.2.0_build_201201.bin
    printf("localIndex:%s%s", rsc_info->local_index, NEWLINE_SIGN);    // 通道号, 参考device_feautre->4LYV8SK7UKLBOUOVS6HXVX_V1.2.0_build_201201.bin
    printf("domain:%s%s", key_info->domain, NEWLINE_SIGN);             // 领域, 参考device_feautre->4LYV8SK7UKLBOUOVS6HXVX_V1.2.0_build_201201.bin
    printf("key:%s%s", key_info->key, NEWLINE_SIGN);                   // 键值, 参考device_feautre->4LYV8SK7UKLBOUOVS6HXVX_V1.2.0_build_201201.bin

    int index = find_index_by_key(key_info->key);
    if (-1 == index)
    {
        printf("error occour, key not found!!%s", NEWLINE_SIGN);
        return -1;
    }

    value_out->type = default_kv_set[index].type;

    if (tsl_data_type_bool == value_out->type ||
        tsl_data_type_int == value_out->type)
    {
        p = &value_out->value_int;
    }
    else if (tsl_data_type_double == value_out->type)
    {
        p = &value_out->value_double;
    }
    else
    {
        p = value_out->value;
    }

    rv = example_kv_raw_get(key_info->key, p, &value_out->size);
    if (ez_kv_name_err == rv)
    {
        memcpy(p, default_kv_set[index].kv.value, default_kv_set[index].kv.value_len);
        value_out->size = default_kv_set[index].kv.value_len;
        rv = 0;
    }

    return rv;
}

/**
 * @brief 云端下发属性
 * 
 */
static int32_t tsl_things_property2dev(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, const tsl_value_t *value)
{
    printf("tsl_things_property2dev%s", NEWLINE_SIGN);

    int32_t rv;
    void *p;
    uint32_t len;

    printf("device uuid:%s%s", sn, NEWLINE_SIGN);                      // 非网关类的单品设备，序列号就是设备本身
    printf("resourceCategory:%s%s", rsc_info->res_type, NEWLINE_SIGN); // 资源类型, 参考device_feautre->4LYV8SK7UKLBOUOVS6HXVX_V1.2.0_build_201201.bin
    printf("localIndex:%s%s", rsc_info->local_index, NEWLINE_SIGN);    // 通道号, 参考device_feautre->4LYV8SK7UKLBOUOVS6HXVX_V1.2.0_build_201201.bin
    printf("domain:%s%s", key_info->domain, NEWLINE_SIGN);             // 领域, 参考device_feautre->4LYV8SK7UKLBOUOVS6HXVX_V1.2.0_build_201201.bin
    printf("key:%s%s", key_info->key, NEWLINE_SIGN);                   // 键值, 参考device_feautre->4LYV8SK7UKLBOUOVS6HXVX_V1.2.0_build_201201.bin

    int index = find_index_by_key(key_info->key);
    if (-1 == index)
    {
        printf("error occour, key not found!!%s", NEWLINE_SIGN);
        return -1;
    }

    if (tsl_data_type_bool == value->type ||
        tsl_data_type_int == value->type)
    {
        p = &value->value_int;
        len = sizeof(value->value_int);
    }
    else if (tsl_data_type_double == value->type)
    {
        p = &value->value_double;
        len = sizeof(value->value_double);
    }
    else
    {
        p = value->value;
        len = value->size;
    }

    rv = example_kv_raw_set(key_info->key, p, len);
    if (0 != rv)
    {
        printf("error occour, save kv!!%s", NEWLINE_SIGN);
    }
    else
    {
        /**
         * @brief 收到云端下发属性，应该执行主动上报。
         * 
         */
        tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)KV_RSCC, .local_index = (int8_t *)KV_LOCALINDEX};
        tsl_key_info_t tsl_keyinfo = {.domain = (int8_t *)KV_DOMAIN, .key = key_info->key};
        ez_iot_tsl_property_report(ez_cloud_get_sn(), &rsc_info, &tsl_keyinfo, NULL);
    }

    return rv;
}

static void *dev_prop_push(void *user_data)
{
    printf("example prop push thread yeild%s", NEWLINE_SIGN);

    hal_thread_sleep(1000 * 10);
    example_prop_report_by_direct();

    hal_thread_sleep(1000 * 10);
    example_event_report_by_callback();

    printf("example prop push thread exit%s", NEWLINE_SIGN);
    return NULL;
}

static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len)
{
    switch (event_type)
    {
    case ez_iot_event_online:
        printf("example prop recv online msg!%s", NEWLINE_SIGN);

        /**
         * @brief 启动独立任务模拟属性上报，不阻塞连接云平台任务。
         * 
         */
        m_thread = hal_thread_create((int8_t *)"prop_push", dev_prop_push, 1024 * 4, 2, NULL);
        hal_thread_detach(m_thread);
        m_thread = NULL;
        ez_cloud_cb_unsubscribe();

        break;

    default:
        break;
    }

    return 0;
}

/**
 * @brief 属性上报方式1：直接发送
 * 
 */
void example_prop_report_by_direct()
{
    printf("property changed, report by direct%s", NEWLINE_SIGN);

    tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)KV_RSCC, .local_index = (int8_t *)KV_LOCALINDEX};
    tsl_key_info_t tsl_keyinfo = {.domain = (int8_t *)KV_DOMAIN, .key = NULL};
    tsl_value_t tsl_value = {0};

    for (size_t i = 0; i < sizeof(default_kv_set_1) / sizeof(kv_type_t); i++)
    {
        tsl_keyinfo.key = default_kv_set_1[i].kv.key;
        tsl_value.type = default_kv_set_1[i].type;

        if (tsl_data_type_bool == default_kv_set_1[i].type ||
            tsl_data_type_int == default_kv_set_1[i].type)
        {
            tsl_value.size = sizeof(tsl_value.value_int);
            memcpy(&tsl_value.value_int, default_kv_set_1[i].kv.value, tsl_value.size);
        }
        else if (tsl_data_type_double == default_kv_set_1[i].type)
        {
            tsl_value.size = sizeof(tsl_value.value_double);
            memcpy(&tsl_value.value_double, default_kv_set_1[i].kv.value, tsl_value.size);
        }
        else
        {
            tsl_value.value = default_kv_set_1[i].kv.value;
            tsl_value.size = default_kv_set_1[i].kv.value_len;
        }

        ez_iot_tsl_property_report(ez_cloud_get_sn(), &rsc_info, &tsl_keyinfo, &tsl_value);
    }
}

/**
 * @brief 属性上报方式2：通过回调函数获取
 * 
 */
void example_event_report_by_callback()
{
    printf("property changed, report by callback%s", NEWLINE_SIGN);

    /**
     * @brief 模拟数据变更
     * 
     */
    for (size_t i = 0; i < sizeof(default_kv_set) / sizeof(kv_type_t); i++)
    {
        example_kv_raw_set(default_kv_set_2[i].kv.key, default_kv_set_2[i].kv.value, default_kv_set_2[i].kv.value_len);
    }

    tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)KV_RSCC, .local_index = (int8_t *)KV_LOCALINDEX};
    tsl_key_info_t tsl_keyinfo = {.domain = (int8_t *)KV_DOMAIN, .key = NULL};

    for (size_t i = 0; i < sizeof(default_kv_set_2) / sizeof(kv_type_t); i++)
    {
        tsl_keyinfo.key = default_kv_set_2[i].kv.key;
        ez_iot_tsl_property_report(ez_cloud_get_sn(), &rsc_info, &tsl_keyinfo, NULL);
    }
}

static int ez_tsl_init()
{
    tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};

    return ez_iot_tsl_init(&tsl_things_cbs);
}

int example_prop(int argc, char **argv)
{
    /**
     * @brief 订阅设备上线事件
     * 
     */
    ez_iot_callbacks_t cbs = {NULL, ez_recv_event_cb};
    ez_cloud_cb_subscribe(&cbs);

    if (0 != ez_cloud_init() ||
        0 != ez_tsl_init() ||
        0 != ez_cloud_start())
    {
        ez_log_e(TAG_APP, "example prop init err");
    }

    return 0;
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_prop, run ez - iot - sdk example property);
#else
// int main(int argc, char **argv)
// {
//     return example_kv(argc, argv);
// }
#endif