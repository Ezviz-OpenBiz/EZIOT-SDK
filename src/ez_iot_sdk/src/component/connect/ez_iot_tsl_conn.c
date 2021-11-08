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
#include <string.h>
#include <stdbool.h>

#include "ezconfig.h"
#include "mcuconfig.h"

#include "ez_iot_def.h"
#include "ez_iot_tsl_conn.h"
#include "ez_iot_tsl.h"
#include "ezdev_sdk_kernel.h"
#include "connectivity_parse.h"

const char *conn_key_domain = "lan_link";
const char *conn_key_center_device = "center_device_status";
const char *conn_key_center_dev_serial = "centerDeviceSerial";
const char *conn_key_enable = "enable";
const char *conn_key_dev_ability = "devAbility";

static ezlist_t *g_conn_list = NULL;
static void *g_conn_list_mutex = NULL;
static bool g_conn_is_inited = false;

extern tsl_things_callbacks_t g_tsl_things_cbs;

int32_t report_to_platform(const char *key, const char *value, int prop_or_event)
{
    int ret = -1;

    tsl_devinfo_t dev_info = {0};
    dev_info.dev_subserial = (uint8_t *)ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial");
    dev_info.dev_firmwareversion = (uint8_t *)ezdev_sdk_kernel_getdevinfo_bykey("dev_firmwareversion");
    dev_info.dev_type = (uint8_t *)ezdev_sdk_kernel_getdevinfo_bykey("dev_type");

    tsl_value_t tsl_value = {0};

    tsl_key_info_t key_info = {0};
    key_info.domain = (uint8_t *)conn_key_domain;
    key_info.key = (uint8_t *)key;

    tsl_value.size = strlen(value);
    tsl_value.type = tsl_data_type_object;
    tsl_value.value = (void *)value;
    ez_log_d(TAG_CONN, "report to platform msg: %s", value);
    if (0 == prop_or_event)
    {
        ret = ez_iot_tsl_property_report(&key_info, &dev_info, &tsl_value);
    }
    else
    {
        ret = ez_iot_tsl_event_report(&key_info, &dev_info, &tsl_value);
    }

    return ret;
}

int32_t report_center_device()
{
    int ret = -1;

    bscJSON *js_root = NULL;
    char *js_str = NULL;
    do
    {
        js_root = bscJSON_CreateObject();
        if (NULL == js_root)
        {
            ez_log_e(TAG_CONN, "create object failed.");
            break;
        }
        bscJSON_AddStringToObject(js_root, conn_key_center_dev_serial, (char *)ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial"));
        bscJSON_AddNumberToObject(js_root, conn_key_enable, 1);
        bscJSON_AddNumberToObject(js_root, conn_key_dev_ability, 4096);

        js_str = bscJSON_PrintUnformatted(js_root);
        if (NULL == js_str)
        {
            ez_log_e(TAG_CONN, "json print failed.");
            break;
        }

        ret = report_to_platform(conn_key_center_device, js_str, 0);
    } while (false);

    if (NULL != js_root)
    {
        bscJSON_Delete(js_root);
    }

    if (NULL != js_str)
    {
        free(js_str);
    }

    return ret;
}

void ez_iot_tsl_conn_init()
{
    ez_log_w(TAG_CONN, "tsl conn init.");
    if (g_conn_is_inited)
    {
        ez_log_e(TAG_CONN, "tsl conn is already inited.");
        return;
    }

    if (NULL == g_conn_list)
    {
        g_conn_list = ezlist(ezlist_THREADSAFE);
    }

    if (NULL == g_conn_list_mutex)
    {
        g_conn_list_mutex = hal_thread_mutex_create();
        if (NULL == g_conn_list_mutex)
        {
            ez_log_e(TAG_CONN, "conn list mutex create failed.");
            return;
        }
    }

    g_conn_is_inited = true;

    conn_info_init();

    return;
}

void ez_iot_tsl_conn_finit()
{
    if (!g_conn_is_inited)
    {
        ez_log_w(TAG_CONN, "tsl conn is not inited.");
        return;
    }

    parse_conn_info_rule_del();

    return;
}

int32_t ez_iot_tsl_add_conn(conn_info_t *conn_info)
{
    if (!g_conn_is_inited)
    {
        ez_log_w(TAG_CONN, "tsl conn is not inited.");
        return -1;
    }

    if (NULL == conn_info)
    {
        ez_log_e(TAG_CONN, "conn_info NULL.");
        return -1;
    }

    conn_info_t *conn = NULL;
    size_t info_size = sizeof(conn_info_t);

    hal_thread_mutex_lock(g_conn_list_mutex);

    size_t size = ezlist_size(g_conn_list);
    for (size_t i = 0; i < size; i++)
    {
        conn = ezlist_getat(g_conn_list, i, &info_size, false);
        if (conn_info->base_info.rule_id == conn->base_info.rule_id)
        {
            ez_log_w(TAG_CONN, "rule id already exist. rule_id: %d", conn_info->base_info.rule_id);
            hal_thread_mutex_unlock(g_conn_list_mutex);
            return 0;
        }
    }

    ezlist_addlast(g_conn_list, (void *)conn_info, sizeof(conn_info_t));
    hal_thread_mutex_unlock(g_conn_list_mutex);

    return 0;
}

int32_t ez_iot_tsl_del_all_conn()
{
    if (!g_conn_is_inited)
    {
        ez_log_w(TAG_CONN, "tsl conn is not inited.");
        return -1;
    }

    conn_info_t *conn_info = NULL;
    size_t info_size = sizeof(conn_info_t);

    hal_thread_mutex_lock(g_conn_list_mutex);

    size_t size = ezlist_size(g_conn_list);
    for (size_t i = 0; i < size; i++)
    {
        conn_info = ezlist_getfirst(g_conn_list, &info_size, false);
        free_conn_info(conn_info);
        ezlist_removefirst(g_conn_list);
    }

    hal_thread_mutex_unlock(g_conn_list_mutex);
    return 0;
}

int32_t ez_iot_tsl_conn_info_process(const int8_t *key, const int8_t *value)
{
    int ret = 0;
    do
    {
        if (0 == strcmp((char *)key, "rule_add"))
        {
            ez_log_i(TAG_CONN, "conn info. key: rule_add.");
            parse_conn_info_rule_del();
            ret = parse_conn_info_rule_add(value, true);
        }
        else if (0 == strcmp((char *)key, "rule_del"))
        {
            ez_log_i(TAG_CONN, "conn info. key: rule_del.");
            ret = parse_conn_info_rule_del();
        }
        if (0 == ret)
        {
            report_to_platform((char *)key, (char *)value, 0);
        }

    } while (false);

    return ret;
}

static int get_format_time(char *format_time)
{
    time_t time_now = time(NULL);
    struct tm ptm_time_now = {0};

    if (NULL == localtime_r(&time_now, &ptm_time_now))
    {
        return 0;
    }
    sprintf(format_time, "%04d-%02d-%02dT%02d:%02d:%02d", ptm_time_now.tm_year + 1900, ptm_time_now.tm_mon + 1, ptm_time_now.tm_mday,
            ptm_time_now.tm_hour, ptm_time_now.tm_min, ptm_time_now.tm_sec);

    return 0;
}

static int32_t conn_do(conn_then_info_t *then_info, int then_info_num, int rule_id)
{
    int ret = -1;
    bscJSON *js_succ_root = NULL;
    bscJSON *js_fail_root = NULL;
    char *js_str = NULL;
    do
    {
        int succ_count = 0;
        int fail_count = 0;
        // success
        js_succ_root = bscJSON_CreateObject();
        if (NULL == js_succ_root)
        {
            ez_log_e(TAG_CONN, "create object failed.");
            break;
        }
        bscJSON_AddNumberToObject(js_succ_root, "ruleId", rule_id);
        bscJSON_AddStringToObject(js_succ_root, "deviceSerial", ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial"));
        ieee_uuid_t uuuuid = {0};
        CreateUUID(&uuuuid);
        char uuid[64 + 1] = {0};
        CreateStringUUID(uuuuid, uuid);
        bscJSON_AddStringToObject(js_succ_root, "groupId", uuid);
        char format_time[32] = {0};
        get_format_time(format_time);
        bscJSON_AddStringToObject(js_succ_root, "execTime", format_time);
        bscJSON *js_extends = bscJSON_CreateArray();
        bscJSON *js_extend = bscJSON_CreateObject();
        bscJSON_AddItemToObject(js_succ_root, "thenResultInfoTo", js_extends);

        // fail
        js_fail_root = bscJSON_CreateObject();
        if (NULL == js_fail_root)
        {
            ez_log_e(TAG_CONN, "create object failed.");
            break;
        }
        bscJSON_AddNumberToObject(js_fail_root, "ruleId", rule_id);
        bscJSON *js_then_info = bscJSON_CreateArray();
        bscJSON_AddItemToObject(js_fail_root, "thenIds", js_then_info);

        ez_log_i(TAG_CONN, "then_info_num: %d", then_info_num);
        for (size_t i = 0; i < then_info_num; i++)
        {
            tsl_report_type_e type = ez_iot_tsl_property;
            if (0 == strcmp("V3_STATUS", (char *)(then_info + i)->action_type))
            {
                type = ez_iot_tsl_property;
            }
            else if (0 == strcmp("V3_OPERATE", (char *)(then_info + i)->action_type))
            {
                type = ez_iot_tsl_action;
            }

            tsl_data_t tsl_data = {0};
            tsl_data.domain = (uint8_t *)(then_info + i)->domain_identifier;
            tsl_data.rsc_type = (uint8_t *)"global";
            tsl_data.key = (uint8_t *)(then_info + i)->identifier;
            tsl_data.local_index = (uint8_t *)(then_info + i)->local_index;

            tsl_data.value.type = (then_info + i)->conn_value.type;
            tsl_data.value.size = (then_info + i)->conn_value.len;

            tsl_data.value.value = (then_info + i)->conn_value.value;
            tsl_data.seq = 0;

            int rsp_num = 0;
            tsl_param_t rsp_param = {0};
            tsl_param_t *p_rsp_param = &rsp_param;
            ez_log_d(TAG_TSL, "then info to subdev. dev_sn: %s, key: %s", (then_info + i)->action_target, (then_info + i)->identifier);
            ret = g_tsl_things_cbs.report2dev((uint8_t *)(then_info + i)->action_target, type, &tsl_data, 0, NULL, &rsp_num, &p_rsp_param);
            if (0 != ret)
            {
                ez_log_e(TAG_TSL, "dev process failed.");
            }

            for (size_t j = 0; j < rsp_num; j++)
            {
                if (NULL != rsp_param.value.value)
                {
                    free(rsp_param.value.value);
                }
            }

            if (0 == ret)
            {
                succ_count++;
                bscJSON_AddStringToObject(js_extend, "deviceSerial", (char *)(then_info + i)->action_target);
                get_format_time(format_time);
                bscJSON_AddStringToObject(js_extend, "execTime", format_time);
                bscJSON_AddItemToArray(js_extends, js_extend);
            }
            else
            {
                fail_count++;
                bscJSON_AddItemToArray(js_then_info, bscJSON_CreateNumber((then_info + i)->id));
            }
        }

        if (0 == succ_count)
        {
            ez_log_w(TAG_CONN, "conn succ count 0.");
            if (NULL != js_succ_root)
            {
                bscJSON_Delete(js_succ_root);
                js_succ_root = NULL;
            }
        }
        else
        {
            js_str = bscJSON_PrintUnformatted(js_succ_root);
            if (NULL != js_str)
            {
                report_to_platform("exec_logs", js_str, 1);
                free(js_str);
                js_str = NULL;
            }
        }

        if (0 == fail_count)
        {
            ez_log_w(TAG_CONN, "conn fail count 0.");
            if (NULL != js_fail_root)
            {
                bscJSON_Delete(js_fail_root);
                js_fail_root = NULL;
            }
        }
        else
        {
            js_str = bscJSON_PrintUnformatted(js_fail_root);
            if (NULL != js_str)
            {
                report_to_platform("exec_fails", js_str, 1);
                free(js_str);
                js_str = NULL;
            }
        }

    } while (false);
    if (NULL != js_succ_root)
    {
        bscJSON_Delete(js_succ_root);
    }
    if (NULL != js_fail_root)
    {
        bscJSON_Delete(js_fail_root);
    }
    if (NULL != js_str)
    {
        free(js_str);
    }
    return ret;
}

static int32_t check_event_data(conn_dev_info *dev_info, conn_on_info_t *on_info)
{
    if (0 == strcmp("DEVICE_EVENT_V3_EVENT", (char *)on_info->event_sub_type))
    {
        ez_log_d(TAG_CONN, "event. no need to check.");
        return 0;
    }

    if (0 == strlen((char *)on_info->judge_method))
    {
        ez_log_d(TAG_CONN, "judge null. no need to check.");
        return 0;
    }

    if (0 == strcmp((char *)on_info->judge_method, "string_equals"))
    {
        ez_log_d(TAG_CONN, "dev_info.data: %s, on_info.data: %s", dev_info->data.data_str, on_info->event_data.value);
        return strcmp((char *)dev_info->data.data_str, (char *)on_info->event_data.value);
    }

    ez_log_d(TAG_CONN, "dev_info.data: %f, on_info.data: %f", dev_info->data.data_num, *(double *)on_info->event_data.value);
    ez_log_d(TAG_CONN, "dev_info.data: %d, on_info.data: %d", (int)(dev_info->data.data_num * 100), (int)(*(double *)(on_info->event_data.value) * 100));

    if (0 == strcmp((char *)on_info->judge_method, "number_equals"))
    {
        return !((int)(dev_info->data.data_num * 100) == (int)(*(double *)(on_info->event_data.value) * 100));
    }
    else if (0 == strcmp((char *)on_info->judge_method, "gt"))
    {
        return !((int)(dev_info->data.data_num * 100) > (int)(*(double *)(on_info->event_data.value) * 100));
    }
    else if (0 == strcmp((char *)on_info->judge_method, "gte"))
    {
        return !((int)(dev_info->data.data_num * 100) >= (int)(*(double *)(on_info->event_data.value) * 100));
    }
    else if (0 == strcmp((char *)on_info->judge_method, "lt"))
    {
        return !((int)(dev_info->data.data_num * 100) < (int)(*(double *)(on_info->event_data.value) * 100));
    }
    else if (0 == strcmp((char *)on_info->judge_method, "lte"))
    {
        return !((int)(dev_info->data.data_num * 100) <= (int)(*(double *)(on_info->event_data.value) * 100));
    }
    return -1;
}

static int32_t check_event_time(conn_base_info_t *base_info)
{
    time_t time_now = time(NULL);
    struct tm *ptm_time_now = localtime(&time_now);
    int8_t week = ptm_time_now->tm_wday;

    if (0 == base_info->plan.week_days[week])
    {
        return -1;
    }
    int time_begin = base_info->plan.begin.tm_hour * 3600 + base_info->plan.begin.tm_min * 60 + base_info->plan.begin.tm_sec;
    int time_end = base_info->plan.end.tm_hour * 3600 + base_info->plan.end.tm_min * 60 + base_info->plan.end.tm_sec;
    time_now = (time_now + 3600 * 8) % (3600 * 24);

    ez_log_d(TAG_CONN, "time_begin: %d, time_now: %ld, time_end: %d", time_begin, time_now, time_end);

    if ((time_begin == 0 && time_end == 0) || (time_begin <= time_now && time_now <= time_end))
    {
        ez_log_i(TAG_CONN, "on info time match.");
        return 0;
    }
    return -1;
}

int32_t ez_iot_tsl_check_conn_do(conn_dev_info *dev_info)
{
    int ret = 0;
    // hal_thread_mutex_lock(g_conn_list_mutex);

    conn_info_t *conn_info = NULL;
    size_t conn_info_size = sizeof(conn_info_t);
    size_t size = ezlist_size(g_conn_list);
    ez_log_i(TAG_CONN, "check conn do. size: %d", size);
    for (size_t i = 0; i < size; i++)
    {
        conn_info = ezlist_getat(g_conn_list, i, &conn_info_size, false);
        ez_log_d(TAG_CONN, "%s---%s", conn_info->on_info.event_source, dev_info->dev_sn);
        ez_log_d(TAG_CONN, "%s---%s", dev_info->domain_identifier, conn_info->on_info.domain_identifier);
        ez_log_d(TAG_CONN, "%s---%s", dev_info->identifier, conn_info->on_info.identifier);
        if (0 == strcmp((char *)conn_info->on_info.event_source, dev_info->dev_sn) &&
            0 == strcmp(dev_info->domain_identifier, (char *)conn_info->on_info.domain_identifier) &&
            0 == strcmp(dev_info->identifier, (char *)conn_info->on_info.identifier))
        {
            ez_log_i(TAG_CONN, "on info check success.");
            if (0 == conn_info->base_info.enable_switch)
            {
                ez_log_i(TAG_CONN, "enable switch false.");
                continue;
            }

            ret = check_event_data(dev_info, &conn_info->on_info);
            if (0 != ret)
            {
                ez_log_w(TAG_CONN, "check event data failed. ret: %d", ret);
                continue;
            }

            ret = check_event_time(&conn_info->base_info);
            if (0 != ret)
            {
                ez_log_w(TAG_CONN, "check event time failed.");
                continue;
            }

            conn_do(conn_info->then_info, conn_info->then_info_num, conn_info->base_info.rule_id);
        }
    }

    // hal_thread_mutex_unlock(g_conn_list_mutex);
    return 0;
}