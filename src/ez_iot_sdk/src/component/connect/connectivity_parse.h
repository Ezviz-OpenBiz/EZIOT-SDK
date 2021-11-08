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
#ifndef _EZ_IOT_CONNECTIVITY_PARSE_H_
#define _EZ_IOT_CONNECTIVITY_PARSE_H_

#include <time.h>
#include <stdint.h>
#define TAG_CONN "T_CONN"

typedef enum
{
    TYPE_BOOL,
    TYPE_INT,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_OBJECT,

    TYPE_MAX,
} conn_type_e;

typedef struct
{
    conn_type_e type;
    int32_t len;
    void *value;
} conn_value_t;

typedef struct
{
    struct tm begin; ///< 计划开始时间
    struct tm end;   ///< 计划结束时间
    int8_t week_days[7];
} conn_base_plan_metadata_t;

typedef struct
{
    int8_t enable_switch;
    int32_t rule_id;
    conn_base_plan_metadata_t plan;
} conn_base_info_t;

typedef struct
{
    int8_t event_source[64];
    int8_t resource_type[32];
    int8_t local_index[16];
    int8_t judge_method[16];
    int8_t event_sub_type[32];
    int8_t domain_identifier[32];
    int8_t identifier[32];
    conn_value_t event_data;
    int8_t enable;

} conn_on_info_t;

typedef struct
{
    int8_t id;
    int32_t delay;
    int8_t action_type[16];
    int8_t action_target[48];
    int8_t resource_type[32];
    int8_t local_index[8];
    int8_t domain_identifier[32];
    int8_t identifier[32];
    int8_t data_type[16];
    conn_value_t conn_value;
} conn_then_info_t;

typedef struct
{
    conn_base_info_t base_info;
    conn_on_info_t on_info;
    int8_t then_info_num;
    conn_then_info_t *then_info;
} conn_info_t;

/**
 * @brief 
 * 
 * @param conn_info 
 */
void free_conn_info(conn_info_t *conn_info);

int32_t parse_conn_info_rule_add(const int8_t *value, bool need_report);

int32_t parse_conn_info_rule_del();

int32_t conn_info_init();
#endif //_EZ_IOT_CONNECTIVITY_PARSE_H_