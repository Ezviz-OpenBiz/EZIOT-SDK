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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ezconfig.h"
#include "mcuconfig.h"

#include "ez_iot_def.h"
#include "connectivity_parse.h"
#include "ez_iot_tsl_conn.h"
#include "bscJSON.h"
#include "ez_iot_log.h"
#include "ez_iot_tsl.h"
#include "ezdev_sdk_kernel.h"
#include "ez_iot_tsl_adapter.h"

const char *conn_nvs_namespace = "conn_ns";
const char *conn_nvs_key = "conn_rule";

const char *conn_key_rule_base_info = "ruleBaseInfoForDeviceTo";
const char *conn_key_rule_id = "ruleId";
const char *conn_key_enable_switch = "enableSwitch";
const char *conn_key_start_time = "startTime";
const char *conn_key_end_time = "endTime";
const char *conn_key_weekdays = "weekdays";

const char *conn_key_on_info = "onInfoForDeviceTo";
const char *conn_key_event_source = "eventSource";
const char *conn_key_local_index = "localIndex";
const char *conn_key_judge_method = "judgeMethod";
const char *conn_key_event_data = "eventData";
const char *conn_key_event_sub_type = "eventSubType";
const char *conn_key_domain_identifier = "domainIdentifier";
const char *conn_key_identifier = "identifier";

// const char *conn_key_if_info = "ifInfoQos";

const char *conn_key_then_info = "thenInfoQos";
const char *conn_key_id = "id";
// const char *conn_key_local_index = "localIndex";
// const char *conn_key_domain_identifier = "domaiIdentifier";
// const char *conn_key_identifier = "identifier";
const char *conn_key_action_target = "actionTarget";
const char *conn_key_action_type = "actionType";
const char *conn_key_data_type = "dataType";
const char *conn_key_action_params = "actionParams";
const char *conn_key_delay = "delay";
const char *conn_key_resource_category = "resourceCategory";
// const char *conn_key_group_id = "groupId";

static struct tm MakeString2Time(const char *ts)
{
    time_t time_now = time(NULL);
    struct tm ptm_time_now = {0};
    localtime_r(&time_now, &ptm_time_now);
    char sep;
    int hour = 0, min = 0;

    if (NULL == strstr(ts, "n"))
    {
        sscanf(ts, "%d%c%d", &hour, &sep, &min);
    }
    else
    {
        sscanf(ts, "n%d%c%d", &hour, &sep, &min);
    }

    ptm_time_now.tm_hour = hour;
    ptm_time_now.tm_min = min;
    ptm_time_now.tm_sec = 0;

    ez_log_d(TAG_CONN, "tm_hour: %d, tm_min: %d", ptm_time_now.tm_hour, ptm_time_now.tm_min);
    return ptm_time_now;
}

static size_t MakeString2WeekDays(int8_t *pday_arr, int8_t count, const char *pdays)
{
    char sep;
    int32_t tmp_days[7];
    memset((void *)tmp_days, -1, sizeof(tmp_days));

    sscanf(pdays, "%c%c%d%c%c%c%d%c%c%c%d%c%c%c%d%c%c%c%d%c%c%c%d%c%c%c%d%c%c", &sep, &sep, &tmp_days[0], &sep, &sep, &sep, &tmp_days[1], &sep, &sep, &sep, &tmp_days[2], &sep, &sep, &sep, &tmp_days[3], &sep, &sep, &sep, &tmp_days[4], &sep, &sep, &sep, &tmp_days[5], &sep, &sep, &sep, &tmp_days[6], &sep, &sep);

    for (size_t i = 0; i < count; i++)
    {
        if (sizeof(tmp_days) / sizeof(tmp_days[i]) <= i)
        {
            break;
        }

        if (tmp_days[i] >= count)
        {
            continue;
        }

        if (-1 == (int8_t)tmp_days[i] || (int8_t)tmp_days[i] > 6)
        {
            continue;
        }

        if (7 == (int8_t)tmp_days[i])
        {
            pday_arr[0] = 1;
        }
        else
        {
            pday_arr[(int8_t)tmp_days[i]] = 1;
        }
    }

    return 0;
}

static int32_t parse_base_info(conn_base_info_t *base_info, bscJSON *js_base_info)
{
    if (NULL == base_info || NULL == js_base_info)
    {
        ez_log_e(TAG_CONN, "parse base info param NULL.");
        return -1;
    }

    int32_t ret = -1;
    do
    {
        bscJSON *js_rule_id = bscJSON_GetObjectItem(js_base_info, conn_key_rule_id);
        if (NULL == js_rule_id || bscJSON_Number != js_rule_id->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_rule_id);
            break;
        }
        base_info->rule_id = js_rule_id->valueint;
        ez_log_d(TAG_CONN, "%s: %d", conn_key_rule_id, base_info->rule_id);

        bscJSON *js_start_time = bscJSON_GetObjectItem(js_base_info, conn_key_start_time);
        if (NULL == js_start_time || bscJSON_String != js_start_time->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_start_time);
            break;
        }
        base_info->plan.begin = MakeString2Time(js_start_time->valuestring);
        ez_log_d(TAG_CONN, "%s: %s", conn_key_start_time, js_start_time->valuestring);

        bscJSON *js_end_time = bscJSON_GetObjectItem(js_base_info, conn_key_end_time);
        if (NULL == js_end_time || bscJSON_String != js_end_time->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_end_time);
            break;
        }
        base_info->plan.end = MakeString2Time(js_end_time->valuestring);
        ez_log_d(TAG_CONN, "%s: %s", conn_key_end_time, js_end_time->valuestring);

        bscJSON *js_weekdays = bscJSON_GetObjectItem(js_base_info, conn_key_weekdays);
        if (NULL == js_weekdays || bscJSON_String != js_weekdays->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_weekdays);
            break;
        }
        MakeString2WeekDays(base_info->plan.week_days, sizeof(base_info->plan.week_days) / sizeof(int8_t), js_weekdays->valuestring);
        ez_log_d(TAG_CONN, "%s: %s", conn_key_start_time, js_weekdays->valuestring);

        bscJSON *js_enable_switch = bscJSON_GetObjectItem(js_base_info, conn_key_enable_switch);
        if (NULL == js_enable_switch || bscJSON_Number != js_enable_switch->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_enable_switch);
            break;
        }
        base_info->enable_switch = js_enable_switch->valueint;
        ez_log_d(TAG_CONN, "%s: %d", conn_key_enable_switch, base_info->enable_switch);

        ret = 0;
    } while (false);

    return ret;
}

static conn_type_e trans_type_from_string(int type)
{
    conn_type_e conn_type = TYPE_MAX;
    switch (type)
    {
    case bscJSON_False:
    case bscJSON_True:
        conn_type = TYPE_BOOL;
        break;
    case bscJSON_NULL:
        break;
    case bscJSON_Number:
        conn_type = TYPE_DOUBLE;
        break;
    case bscJSON_String:
        conn_type = TYPE_STRING;
        break;
    case bscJSON_Array:
        conn_type = TYPE_ARRAY;
        break;
    case bscJSON_Object:
        conn_type = TYPE_OBJECT;
        break;

    default:
        break;
    }
    return conn_type;
}

static int32_t process_event_data(const char *value, conn_value_t *data)
{
    int ret = -1;
    bscJSON *js_root = NULL;
    char *str_obj = NULL;
    do
    {
        js_root = bscJSON_Parse(value);
        if (NULL == js_root)
        {
            ez_log_e(TAG_CONN, "json parse failed.");
            break;
        }
        bscJSON *js_action_param = bscJSON_GetObjectItem(js_root, "value");

        data->type = trans_type_from_string(js_action_param->type);
        if (TYPE_MAX == data->type)
        {
            ez_log_e(TAG_CONN, "%s illegal.", conn_key_data_type);
            break;
        }
        ez_log_d(TAG_CONN, "value type: %d", data->type);

        int len = 0;
        switch (data->type)
        {
        case TYPE_BOOL:
        case TYPE_INT:
        case TYPE_DOUBLE:
            len = sizeof(double);
            data->value = malloc(len);
            break;
        case TYPE_STRING:
            len = strlen(js_action_param->valuestring);
            data->value = malloc(len + 1);
            len = len + 1;
            break;
        case TYPE_ARRAY:
        case TYPE_OBJECT:
            str_obj = bscJSON_PrintUnformatted(js_action_param);
            len = strlen(str_obj);
            ez_log_d(TAG_CONN, "str_obj: %s, len: %d", str_obj, len);
            data->value = malloc(len + 1);
            len = len + 1;
            break;
        default:
            break;
        }
        ez_log_d(TAG_CONN, "value len: %d", len);

        if (NULL == data->value)
        {
            ez_log_e(TAG_CONN, "memory not enough.");
            break;
        }
        memset(data->value, 0, len);
        switch (data->type)
        {
        case TYPE_BOOL:
            *(double *)data->value = (double)js_action_param->valueint;
            ez_log_i(TAG_CONN, "%f: %f", *(double *)data->value, (double)js_action_param->valueint);
            break;
        case TYPE_DOUBLE:
            *(double *)data->value = js_action_param->valuedouble;
            ez_log_i(TAG_CONN, "%f: %f", *(double *)data->value, js_action_param->valuedouble);
            break;
        case TYPE_STRING:
            strncpy((char *)data->value, js_action_param->valuestring, len);
            break;
        case TYPE_ARRAY:
        case TYPE_OBJECT:
            strncpy((char *)data->value, str_obj, len);
            break;
        default:
            break;
        }
        data->len = len;
        ret = 0;
    } while (false);

    if (NULL != str_obj)
    {
        free(str_obj);
    }
    if (NULL != js_root)
    {
        bscJSON_Delete(js_root);
    }

    return ret;
}

static int32_t parse_on_info(conn_on_info_t *on_info, bscJSON *js_on_info)
{
    if (NULL == on_info || NULL == js_on_info)
    {
        ez_log_e(TAG_CONN, "parse on info param NULL.");
        return -1;
    }

    int32_t ret = -1;
    do
    {
        bscJSON *js_event_source = bscJSON_GetObjectItem(js_on_info, conn_key_event_source);
        if (NULL == js_event_source || bscJSON_String != js_event_source->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_event_source);
            break;
        }

        char *pos = strstr(js_event_source->valuestring, "-");
        if (NULL == pos)
        {
            if (0 != strcmp(js_event_source->valuestring, ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial")))
            {
                ez_log_w(TAG_CONN, "local serial not match event source. %s---%s", js_event_source->valuestring, ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial"));
                break;
            }

            strncpy((char *)on_info->event_source, js_event_source->valuestring, sizeof(on_info->event_source) - 1);
            ez_log_d(TAG_CONN, "%s: %s", conn_key_event_source, on_info->event_source);
        }
        else
        {
            strncpy((char *)on_info->event_source, pos + 1, sizeof(on_info->event_source) - 1);
            ez_log_d(TAG_CONN, "%s: %s", conn_key_event_source, on_info->event_source);
        }

        bscJSON *js_local_index = bscJSON_GetObjectItem(js_on_info, conn_key_local_index);
        if (NULL == js_local_index || bscJSON_String != js_local_index->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_local_index);
            break;
        }
        strncpy((char *)on_info->local_index, js_local_index->valuestring, sizeof(on_info->local_index) - 1);
        ez_log_d(TAG_CONN, "%s: %s", conn_key_local_index, on_info->local_index);

        bscJSON *js_judge_method = bscJSON_GetObjectItem(js_on_info, conn_key_judge_method);
        if (NULL == js_judge_method || bscJSON_String != js_judge_method->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_judge_method);
            break;
        }
        strncpy((char *)on_info->judge_method, js_judge_method->valuestring, sizeof(on_info->judge_method) - 1);
        ez_log_d(TAG_CONN, "%s: %s", conn_key_judge_method, on_info->judge_method);

        bscJSON *js_event_data = bscJSON_GetObjectItem(js_on_info, conn_key_event_data);
        if (NULL == js_event_data || bscJSON_String != js_event_data->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_event_data);
            break;
        }
        ret = process_event_data(js_event_data->valuestring, &on_info->event_data);
        if (0 != ret)
        {
            ez_log_e(TAG_CONN, "then info data process failed.");
            break;
        }

        bscJSON *js_event_sub_type = bscJSON_GetObjectItem(js_on_info, conn_key_event_sub_type);
        if (NULL == js_event_sub_type || bscJSON_String != js_event_sub_type->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_event_sub_type);
            break;
        }
        strncpy((char *)on_info->event_sub_type, js_event_sub_type->valuestring, sizeof(on_info->event_sub_type) - 1);
        ez_log_d(TAG_CONN, "%s: %s", conn_key_event_sub_type, on_info->event_sub_type);

        bscJSON *js_domain_identifier = bscJSON_GetObjectItem(js_on_info, conn_key_domain_identifier);
        if (NULL == js_domain_identifier || bscJSON_String != js_domain_identifier->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_domain_identifier);
            break;
        }
        strncpy((char *)on_info->domain_identifier, js_domain_identifier->valuestring, sizeof(on_info->domain_identifier) - 1);
        ez_log_d(TAG_CONN, "%s: %s", conn_key_domain_identifier, on_info->domain_identifier);

        bscJSON *js_identifier = bscJSON_GetObjectItem(js_on_info, conn_key_identifier);
        if (NULL == js_identifier || bscJSON_String != js_identifier->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_identifier);
            break;
        }
        strncpy((char *)on_info->identifier, js_identifier->valuestring, sizeof(on_info->identifier) - 1);
        ez_log_d(TAG_CONN, "%s: %s", conn_key_identifier, on_info->identifier);

        ret = 0;
    } while (false);

    return ret;
}

static int32_t parse_then_info(conn_then_info_t *then_info, bscJSON *js_then_info)
{
    if (NULL == then_info || NULL == js_then_info)
    {
        ez_log_e(TAG_CONN, "parse_then_info param error.");
        return -1;
    }
    int ret = -1;
    do
    {
        bscJSON *js_id = bscJSON_GetObjectItem(js_then_info, conn_key_id);
        if (NULL == js_id || bscJSON_Number != js_id->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_id);
            break;
        }
        then_info->id = js_id->valueint;
        ez_log_d(TAG_CONN, "%s: %d", conn_key_id, then_info->id);

        bscJSON *js_action_target = bscJSON_GetObjectItem(js_then_info, conn_key_action_target);
        if (NULL == js_action_target || bscJSON_String != js_action_target->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_action_target);
            break;
        }
        char *pos = strstr(js_action_target->valuestring, "-");
        if (NULL == pos)
        {
            if (0 != strcmp(js_action_target->valuestring, ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial")))
            {
                ez_log_w(TAG_CONN, "local serial not match action target. %s---%s", js_action_target->valuestring, ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial"));
                break;
            }
            strncpy((char *)then_info->action_target, js_action_target->valuestring, sizeof(then_info->action_target) - 1);
            ez_log_d(TAG_CONN, "%s: %s", conn_key_action_target, then_info->action_target);
        }
        else
        {
            strncpy((char *)then_info->action_target, pos + 1, sizeof(then_info->action_target) - 1);
            ez_log_d(TAG_CONN, "%s: %s", conn_key_action_target, then_info->action_target);
        }

        bscJSON *js_action_type = bscJSON_GetObjectItem(js_then_info, conn_key_action_type);
        if (NULL == js_action_type || bscJSON_String != js_action_type->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_action_type);
            break;
        }
        strncpy((char *)then_info->action_type, js_action_type->valuestring, sizeof(then_info->action_type) - 1);
        ez_log_d(TAG_CONN, "%s: %s", conn_key_action_type, then_info->action_type);

        bscJSON *js_local_index = bscJSON_GetObjectItem(js_then_info, conn_key_local_index);
        if (NULL == js_local_index || bscJSON_String != js_local_index->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_local_index);
            break;
        }
        strncpy((char *)then_info->local_index, js_local_index->valuestring, sizeof(then_info->local_index) - 1);
        ez_log_d(TAG_CONN, "%s: %s", conn_key_local_index, then_info->local_index);

        bscJSON *js_domain_identifier = bscJSON_GetObjectItem(js_then_info, conn_key_domain_identifier);
        if (NULL == js_domain_identifier || bscJSON_String != js_domain_identifier->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_domain_identifier);
            break;
        }
        strncpy((char *)then_info->domain_identifier, js_domain_identifier->valuestring, sizeof(then_info->domain_identifier) - 1);
        ez_log_d(TAG_CONN, "%s: %s", conn_key_domain_identifier, then_info->domain_identifier);

        bscJSON *js_identifier = bscJSON_GetObjectItem(js_then_info, conn_key_identifier);
        if (NULL == js_identifier || bscJSON_String != js_identifier->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_identifier);
            break;
        }
        strncpy((char *)then_info->identifier, js_identifier->valuestring, sizeof(then_info->identifier) - 1);
        ez_log_d(TAG_CONN, "%s: %s", conn_key_identifier, then_info->identifier);

        bscJSON *js_action_param = bscJSON_GetObjectItem(js_then_info, conn_key_action_params);
        if (NULL == js_action_param)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_action_params);
            break;
        }
        ret = process_event_data(js_action_param->valuestring, &then_info->conn_value);
        if (0 != ret)
        {
            ez_log_e(TAG_CONN, "then info data process failed.");
            break;
        }
    } while (false);

    return ret;
}

void free_conn_info(conn_info_t *conn_info)
{
    if (NULL == conn_info)
    {
        ez_log_e(TAG_CONN, "conn_info NULL.");
        return;
    }

    if (NULL != conn_info->on_info.event_data.value)
    {
        free(conn_info->on_info.event_data.value);
    }

    int size = conn_info->then_info_num;
    for (size_t i = 0; i < size; i++)
    {
        if (NULL == conn_info->then_info)
        {
            break;
        }

        if (NULL != (conn_info->then_info + i)->conn_value.value)
        {
            free((conn_info->then_info + i)->conn_value.value);
        }
    }

    if (NULL != conn_info->then_info)
    {
        free(conn_info->then_info);
        conn_info->then_info = NULL;
    }

    return;
}

int32_t connectivity_parse(conn_info_t *conn_info, bscJSON *js_rule_info)
{
    int32_t ret = -1;

    do
    {
        bscJSON *js_base_info = bscJSON_GetObjectItem(js_rule_info, conn_key_rule_base_info);
        if (NULL == js_base_info || bscJSON_Object != js_base_info->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_rule_base_info);
            break;
        }
        ret = parse_base_info(&conn_info->base_info, js_base_info);
        if (0 != ret)
        {
            ez_log_e(TAG_CONN, "parse base info failed.");
            break;
        }
        ez_log_d(TAG_CONN, "base_info parse succ.");

        bscJSON *js_on_info = bscJSON_GetObjectItem(js_rule_info, conn_key_on_info);
        if (NULL == js_on_info || bscJSON_Object != js_on_info->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_on_info);
            break;
        }
        ret = parse_on_info(&conn_info->on_info, js_on_info);
        if (0 != ret)
        {
            ez_log_e(TAG_CONN, "parse on info failed.");
            break;
        }
        ez_log_d(TAG_CONN, "on_info parse succ.");

        bscJSON *js_then_info = bscJSON_GetObjectItem(js_rule_info, conn_key_then_info);
        if (NULL == js_then_info || bscJSON_Array != js_then_info->type)
        {
            ez_log_e(TAG_CONN, "%s absent.", conn_key_then_info);
            break;
        }
        int num = bscJSON_GetArraySize(js_then_info);

        conn_info->then_info = (conn_then_info_t *)malloc(sizeof(conn_then_info_t) * num);
        if (NULL == conn_info->then_info)
        {
            ez_log_e(TAG_CONN, "memory not enough.");
            break;
        }
        memset(conn_info->then_info, 0, sizeof(conn_then_info_t) * num);

        for (size_t i = 0; i < num; i++)
        {
            bscJSON *js_then = bscJSON_GetArrayItem(js_then_info, i);
            if (NULL == js_then || bscJSON_Object != js_then->type)
            {
                ez_log_e(TAG_CONN, "then info absent.");
                break;
            }
            ret = parse_then_info(conn_info->then_info + i, js_then);
            if (0 != ret)
            {
                ez_log_e(TAG_CONN, "parse then info failed.");
                break;
            }
        }

        conn_info->then_info_num = num;
        ez_log_d(TAG_CONN, "then_info_num: %d", conn_info->then_info_num);
        if (0 != ret)
        {
            break;
        }
        ret = 0;
        ez_log_d(TAG_CONN, "then_info parse succ.");
    } while (false);

    if (0 != ret)
    {
        free_conn_info(conn_info);
    }

    return ret;
}

static int32_t check_on_info_valid(conn_on_info_t *on_info)
{
    int ret = 0;
    tsl_rsc_info_t rsc_info = {.res_type = on_info->resource_type, .local_index = on_info->local_index};
    tsl_key_info_t key_info = {.domain = on_info->domain_identifier, .key = on_info->identifier};

    if (0 == strcmp("DEVICE_EVENT_V3_EVENT", (char *)on_info->event_sub_type))
    {
        ret = ez_iot_tsl_event_value_legal(on_info->event_source, &rsc_info, &key_info, NULL);
        if (0 != ret)
        {
            ez_log_e(TAG_CONN, "event key not found. key: %s, code:%08x", on_info->identifier, ret);
        }
    }
    else if (0 == strcmp("DEVICE_EVENT_V3_STATUS", (char *)on_info->event_sub_type))
    {
        ret = ez_iot_tsl_property_value_legal(on_info->event_source, &rsc_info, &key_info, NULL);
        if (0 != ret)
        {
            ez_log_e(TAG_CONN, "prop key not found. key: %s, code:%08x", on_info->identifier, ret);
        }
    }
    else
    {
        ret = -1;
    }

    return ret;
}

static int32_t check_then_info_valid(conn_then_info_t *then_info, int32_t then_info_num)
{
    int ret = -1;
    for (size_t i = 0; i < then_info_num; i++)
    {
        tsl_rsc_info_t rsc_info = {.res_type = (then_info + i)->resource_type, .local_index = (then_info + i)->local_index};
        tsl_key_info_t key_info = {.domain = (then_info + i)->domain_identifier, .key = (then_info + i)->identifier};

        if (0 == strcmp("V3_STATUS", (char *)(then_info + i)->action_type))
        {
            ret = ez_iot_tsl_property_value_legal((uint8_t *)(then_info + i)->action_target, &rsc_info, &key_info, NULL);
            if (0 != ret)
            {
                ez_log_e(TAG_CONN, "event key not found. key: %s", (then_info + i)->identifier);
                break;
            }
        }
        else if (0 == strcmp("V3_OPERATE", (char *)(then_info + i)->action_type))
        {
            ret = ez_iot_tsl_event_value_legal((uint8_t *)(then_info + i)->action_target, &rsc_info, &key_info, NULL);
            if (0 != ret)
            {
                ez_log_e(TAG_CONN, "operate key not found. key: %s", (then_info + i)->identifier);
                break;
            }
        }
        else
        {
            ez_log_w(TAG_CONN, "then_info.action_type: %s.", (then_info + i)->action_type);
        }

        ret = 0;
    }

    return ret;
}

extern int32_t report_to_platform(const char *key, const char *value, int prop_or_event);

int32_t parse_conn_info_rule_add(const int8_t *value, bool need_report)
{
    int ret = -1;
    int len = strlen((char *)value);
    if (len >= 4 * 1024 || len <= 0)
    {
        ez_log_e(TAG_CONN, "rule length illegal. len: %d", len);
        return ret;
    }
    ez_log_i(TAG_CONN, "rule_add msg: %s.", value);
    bscJSON *js_root = NULL;
    bscJSON *js_match_result = NULL;
    char *js_str = NULL;
    do
    {
        js_root = bscJSON_Parse((char *)value);
        if (NULL == js_root || bscJSON_Array != js_root->type)
        {
            ez_log_e(TAG_CONN, "json parse error.");
            break;
        }

        js_match_result = bscJSON_CreateObject();
        if (NULL == js_match_result)
        {
            ez_log_e(TAG_CONN, "json create object failed.");
            break;
        }
        bscJSON_AddStringToObject(js_match_result, "deviceSerial", ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial"));
        bscJSON *js_rule_ids = bscJSON_CreateArray();
        bscJSON_AddItemToObject(js_match_result, "ruleIds", js_rule_ids);

        size_t size = bscJSON_GetArraySize(js_root);
        for (size_t i = 0; i < size; i++)
        {
            bscJSON *js_rule_info = bscJSON_GetArrayItem(js_root, i);
            if (NULL == js_rule_info || bscJSON_Object != js_rule_info->type)
            {
                ez_log_e(TAG_CONN, "rule info absent.");
                continue;
            }

            conn_info_t conn_info = {0};
            ret = connectivity_parse(&conn_info, js_rule_info);
            if (0 != ret)
            {
                ez_log_e(TAG_CONN, "conn info rule_add parse failed.");
                continue;
            }
            ez_log_d(TAG_CONN, "conn info parse succ.");

            if (need_report)
            {
                ret = check_on_info_valid(&conn_info.on_info);
                if (0 != ret)
                {
                    ez_log_e(TAG_CONN, "on info invalid.");
                    continue;
                }

                ret = check_then_info_valid(conn_info.then_info, conn_info.then_info_num);
                if (0 != ret)
                {
                    ez_log_e(TAG_CONN, "then info invalid.");
                    continue;
                }
            }

            ret = ez_iot_tsl_add_conn(&conn_info);
            if (0 != ret)
            {
                ez_log_e(TAG_CONN, "add conn failed.");
                continue;
            }

            bscJSON_AddItemToArray(js_rule_ids, bscJSON_CreateNumber(conn_info.base_info.rule_id));
        }

        if (need_report)
        {
            js_str = bscJSON_PrintUnformatted(js_match_result);
            if (NULL == js_str)
            {
                ez_log_e(TAG_CONN, "json print failed.");
                break;
            }

            ret = report_to_platform("rule_match", js_str, 1);
            if (0 != ret)
            {
                ez_log_e(TAG_CONN, "report rule_match failed.");
                break;
            }

            ret = ez_kv_raw_set(conn_nvs_key, value, len);
            if (0 != ret)
            {
                ez_log_e(TAG_CONN, "write conn info failed.");
                break;
            }
            ez_log_d(TAG_CONN, "conn info save succ.");
        }

    } while (false);

    if (NULL != js_root)
    {
        bscJSON_Delete(js_root);
    }

    if (NULL != js_match_result)
    {
        bscJSON_Delete(js_match_result);
    }

    if (NULL != js_str)
    {
        free(js_str);
    }

    return ret;
}

int32_t parse_conn_info_rule_del()
{
    int ret = -1;
    do
    {
        ret = hal_storage_kv_del(conn_nvs_namespace, conn_nvs_key);
        if (0 != ret)
        {
            ez_log_e(TAG_CONN, "del conn info failed.");
            break;
        }

        ret = ez_iot_tsl_del_all_conn();
    } while (false);

    return ret;
}

int32_t conn_info_init()
{
    ez_log_w(TAG_CONN, "conn info init.");

    int ret = -1;
    char *value = NULL;
    do
    {

        int len = 0;
        ret = ez_kv_raw_get(conn_nvs_key, NULL, &len);
        if (0 != ret)
        {
            ez_log_e(TAG_CONN, "storage read failed. ret: %x", ret);
            break;
        }
        value = (char *)malloc(len + 1);
        if (NULL == value)
        {
            ez_log_e(TAG_CONN, "memory not enough.");
            break;
        }
        memset(value, 0, len + 1);
        ez_kv_raw_get(conn_nvs_key, value, &len);
        ret = parse_conn_info_rule_add((int8_t *)value, false);
        if (0 != ret)
        {
            ez_log_e(TAG_CONN, "conn info rule_add failed.");
            break;
        }
    } while (false);

    if (NULL != value)
    {
        free(value);
    }

    return ret;
}
