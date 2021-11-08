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
#ifndef _EZVIZ_PROFILE_PARSE_H_
#define _EZVIZ_PROFILE_PARSE_H_

#include "ez_iot_tsl.h"

#define MAX_LOCAL_INDEX_LENGTH 8
#define MAX_STRING_ENUM_LENGTH 32
#define MAX_ARR_REQUIRE_LENGTH 32
#define MAX_EVENT_TYPE_KEY_LENGTH 32

typedef enum
{
    ACCESS_READ = 0x00000001,
    ACCESS_WRITE = 0x00000002,
} tsl_access_type_e;

#ifndef COMPONENT_TSL_PROFILE_STRIP
typedef struct
{
    tsl_data_type_e item_type;
    char prop_key[32];
    union
    {
        struct
        {
            int minItem;
            int maxItem;
            int prop_num;
            void *item_prop; //tsl_schema_desc
        } type_array;

        struct
        {
            int max_props;
            int min_props;
            int req_num;
            char *required;
            int prop_num;
            void *property; //tsl_schema_desc
        } type_object;

        struct
        {
            int min_len;
            int max_len;
            int enum_num;
            char *str_enum;
        } type_string;

        struct
        {
            int res;
        } type_bool;

        struct
        {
            double minimum;
            double exminimum;
            double maximum;
            double exmaximum;
            double multiple;
            int enum_num;
            double *num_enum;
        } type_number;

        struct
        {
            int minimum;
            int exminimum;
            int maximum;
            int exmaximum;
            int multiple;
            int enum_num;
            int *int_enum;
        } type_integer;
    };
} tsl_schema_desc;
#endif

typedef struct
{
    char identifier[32];
    int access;
#ifndef COMPONENT_TSL_PROFILE_STRIP
    char version[8];
    tsl_schema_desc prop_desc;
#endif
} tsl_domain_prop;

typedef struct
{
    char identifier[32];
#ifndef COMPONENT_TSL_PROFILE_STRIP
    char version[8];
    char direction[16];
    tsl_schema_desc input_schema;
    tsl_schema_desc output_schema;
#endif
} tsl_domain_action;

typedef struct
{
    char identifier[32];
#ifndef COMPONENT_TSL_PROFILE_STRIP
    char version[8];
    int enum_num;
    char *event_type;
    tsl_schema_desc input_schema;
    tsl_schema_desc output_schema;
#endif
} tsl_domain_event;

typedef struct
{
    char identifier[32];
    int prop_num;
    tsl_domain_prop *prop;
    int action_num;
    tsl_domain_action *action;
    int event_num;
    tsl_domain_event *event;
} tsl_rsc_domain;

typedef struct
{
    char rsc_category[32];
    int index_num;
    char *index;
    int domain_num;
    tsl_rsc_domain *domain;
} tsl_profile_resource;

typedef struct
{
    short ref;
    char dev_type[24];
    char dev_fw_ver[36];
    int rsc_num;
    tsl_profile_resource *resource;
} ez_iot_tsl_capacity_t;

int profile_parse(char *profile_value, int profile_len, ez_iot_tsl_capacity_t *capacity);

int free_profile_memory(ez_iot_tsl_capacity_t *capacity);

#endif // _EZVIZ_PROFILE_PARSE_H_