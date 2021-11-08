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
#include <float.h>
#include <math.h>
#include <limits.h>
#include "ezconfig.h"
#include "mcuconfig.h"
#include "ez_iot_def.h"

#include "tsl_info_check.h"
#include "ez_iot_tsl_adapter.h"

#ifndef COMPONENT_TSL_PROFILE_STRIP
static int check_int_value(tsl_schema_desc *schema, tsl_value_t *value)
{
    int ret = 0;

    do
    {
        if (schema->type_integer.enum_num != 0)
        {
            bool is_hit = false;
            for (size_t i = 0; i < schema->type_integer.enum_num; i++)
            {
                if (value->value_int == *(schema->type_integer.int_enum + i))
                {
                    ez_log_d(TAG_TSL, "int value match: %d -- %d", value->value_int, *(schema->type_integer.int_enum + i));
                    is_hit = true;
                    ret = 0;
                    break;
                }
            }
            if (!is_hit)
            {
                ez_log_e(TAG_TSL, "enum not match.");
                ret = ez_errno_tsl_value_illegal;
            }
            break;
        }

        if (schema->type_integer.multiple != 0)
        {
            if (0 != value->value_int % schema->type_integer.multiple)
            {
                ez_log_e(TAG_TSL, "multiple not match.");
                ret = ez_errno_tsl_value_illegal;
                break;
            }
        }

        if (0 != schema->type_integer.maximum && 0 != schema->type_integer.minimum && schema->type_integer.maximum >= schema->type_integer.minimum)
        {
            if (schema->type_integer.maximum >= value->value_int && value->value_int >= schema->type_integer.minimum)
            {
                ez_log_d(TAG_TSL, "int value legal.");
                ret = 0;
                break;
            }
            else
            {
                ez_log_e(TAG_TSL, "int value illegal.min: %d, cur: %d, max: %d", schema->type_integer.minimum, value->value_int, schema->type_integer.maximum);
                ret = ez_errno_tsl_value_illegal;
                break;
            }
        }

        if (0 != schema->type_integer.exmaximum && 0 != schema->type_integer.exminimum && schema->type_integer.exmaximum > schema->type_integer.exminimum)
        {
            if (schema->type_integer.exmaximum > value->value_int && value->value_int > schema->type_integer.exminimum)
            {
                ez_log_d(TAG_TSL, "int value legal.");
                ret = 0;
                break;
            }
            else
            {
                ez_log_e(TAG_TSL, "int value illegal.exmin: %d, cur: %d, exmax: %d", schema->type_integer.exminimum, value->value_int, schema->type_integer.exmaximum);
                ret = ez_errno_tsl_value_illegal;
                break;
            }
        }
    } while (false);

    return ret;
}

static int check_double_value(tsl_schema_desc *schema, tsl_value_t *value)
{
    int ret = 0;
    do
    {
        if (schema->type_number.enum_num != 0)
        {
            bool is_hit = false;
            for (size_t i = 0; i < schema->type_number.enum_num; i++)
            {
                if (value->value_double == *(schema->type_number.num_enum + i))
                {
                    ez_log_d(TAG_TSL, "double value match: %f -- %f", value->value_double, *(schema->type_number.num_enum + i));
                    is_hit = true;
                    ret = 0;
                    break;
                }
            }
            if (!is_hit)
            {
                ez_log_e(TAG_TSL, "enum not match.");
                ret = ez_errno_tsl_value_illegal;
            }
            break;
        }

        if (schema->type_number.multiple != 0)
        {
            if (0 != (value->value_double / schema->type_number.multiple - (int)(value->value_double / schema->type_number.multiple)))
            {
                ez_log_e(TAG_TSL, "multiple not match.");
                ret = ez_errno_tsl_value_illegal;
                break;
            }
        }

        if (0 != schema->type_number.maximum && 0 != schema->type_number.minimum && schema->type_number.maximum >= schema->type_number.minimum)
        {
            if (schema->type_number.maximum >= value->value_double && value->value_double >= schema->type_number.minimum)
            {
                ez_log_d(TAG_TSL, "double value legal.");
                ret = 0;
                break;
            }
            else
            {
                ez_log_e(TAG_TSL, "double value illegal.min: %f, cur: %f, max: %f", schema->type_number.minimum, value->value_double, schema->type_number.maximum);
                ret = ez_errno_tsl_value_illegal;
                break;
            }
        }

        if (0 != schema->type_number.exmaximum && 0 != schema->type_number.exminimum && schema->type_number.exmaximum > schema->type_number.exminimum)
        {
            if (schema->type_number.exmaximum > value->value_double && value->value_double > schema->type_number.exminimum)
            {
                ez_log_d(TAG_TSL, "double value legal.");
                ret = 0;
                break;
            }
            else
            {
                ez_log_e(TAG_TSL, "double value illegal.exmin: %f, cur: %f, exmax: %f", schema->type_number.exminimum, value->value_int, schema->type_number.exmaximum);
                ret = ez_errno_tsl_value_illegal;
                break;
            }
        }
    } while (false);

    return ret;
}

static int check_string_value(tsl_schema_desc *schema, tsl_value_t *value)
{
    int ret = 0;
    do
    {
        if (schema->type_string.enum_num != 0)
        {
            bool is_hit = false;
            for (size_t i = 0; i < schema->type_string.enum_num; i++)
            {
                if (0 == strcmp((char *)value->value, schema->type_string.str_enum + MAX_STRING_ENUM_LENGTH * i))
                {
                    ez_log_d(TAG_TSL, "string value match: %s -- %s", (char *)value->value, schema->type_string.str_enum + MAX_STRING_ENUM_LENGTH * i);
                    is_hit = true;
                    ret = 0;
                    break;
                }
            }
            if (!is_hit)
            {
                ez_log_e(TAG_TSL, "enum not match.");
                ret = ez_errno_tsl_value_illegal;
            }
            break;
        }

        int len = strlen((char *)value->value);
        if (0 != schema->type_string.min_len && 0 != schema->type_string.max_len && schema->type_string.min_len <= schema->type_string.max_len)
        {
            if (schema->type_string.min_len <= len && len <= schema->type_string.max_len)
            {
                ez_log_d(TAG_TSL, "string value len legal.");
                ret = 0;
                break;
            }
            else
            {
                ez_log_e(TAG_TSL, "string value len illegal. min_len: %d, cur_len: %d, max_len: %d", schema->type_string.min_len, len, schema->type_string.max_len);
                ret = ez_errno_tsl_value_illegal;
                break;
            }
        }
    } while (false);

    return ret;
}

static int tranform_value_from_json(tsl_value_t *prop_value, bscJSON *js_prop, char **str_msg)
{
    int ret = 0;

    switch (js_prop->type)
    {
    case bscJSON_False:
        prop_value->value_bool = false;
        prop_value->size = sizeof(bool);
        prop_value->type = tsl_data_type_bool;
        break;
    case bscJSON_True:
        prop_value->value_bool = true;
        prop_value->size = sizeof(bool);
        prop_value->type = tsl_data_type_bool;
        break;
    case bscJSON_Number:
    {
        ez_log_d(TAG_TSL, "js prop int: %d, js prop double: %f", js_prop->valueint, js_prop->valuedouble);
        if (DBL_EPSILON >= fabs(js_prop->valuedouble - js_prop->valueint || js_prop->valueint == INT_MAX || js_prop->valueint == INT_MIN))
        {
            prop_value->value_int = js_prop->valueint;
            prop_value->size = sizeof(int);
            prop_value->type = tsl_data_type_int;
        }
        else
        {
            prop_value->value_double = js_prop->valuedouble;
            prop_value->size = sizeof(double);
            prop_value->type = tsl_data_type_double;
        }
        ez_log_d(TAG_TSL, "prop int: %d, prop double: %f", prop_value->value_int, prop_value->value_double);
    }
    break;
    case bscJSON_String:
        prop_value->size = strlen(js_prop->valuestring);
        prop_value->type = tsl_data_type_string;
        *str_msg = (char *)malloc(prop_value->size + 1);
        if (NULL == *str_msg)
        {
            ez_log_e(TAG_TSL, "memory not enough.");
            ret = -1;
            break;
        }
        strcpy(*str_msg, js_prop->valuestring);
        prop_value->value = *str_msg;
        break;
    case bscJSON_Array:
        *str_msg = bscJSON_PrintUnformatted(js_prop);
        if (NULL != *str_msg)
        {
            prop_value->size = strlen(*str_msg);
            prop_value->type = tsl_data_type_array;
            prop_value->value = *str_msg;
        }
        else
        {
            ret = -1;
        }

        break;
    case bscJSON_Object:
        *str_msg = bscJSON_PrintUnformatted(js_prop);
        if (NULL != *str_msg)
        {
            prop_value->size = strlen(*str_msg);
            prop_value->type = tsl_data_type_object;
            prop_value->value = *str_msg;
        }
        else
        {
            ret = -1;
        }
        break;

    default:
        break;
    }
    if (NULL != *str_msg)
    {
        ez_log_d(TAG_TSL, "str_msg: %s", *str_msg);
    }

    return ret;
}

static int check_array_value(tsl_schema_desc *schema, tsl_value_t *value)
{
    int ret = 0;
    bscJSON *js_root = NULL;
    do
    {
        js_root = bscJSON_Parse((char *)value->value);
        if (NULL == js_root || bscJSON_Array != js_root->type)
        {
            ez_log_e(TAG_TSL, "array value parse failed.");
            ret = ez_errno_tsl_value_illegal;
            break;
        }
        int arr_num = bscJSON_GetArraySize(js_root);
        if (0 != schema->type_array.minItem && 0 != schema->type_array.maxItem && schema->type_array.minItem <= schema->type_array.maxItem)
        {
            if (schema->type_array.minItem < arr_num || schema->type_object.max_props < arr_num)
            {
                ez_log_e(TAG_TSL, "array value props not match. min: %d, cur: %d, max: %d", schema->type_array.minItem, arr_num, schema->type_array.maxItem);
                ret = ez_errno_tsl_value_illegal;
                break;
            }
        }

        bscJSON *js_prop = NULL;
        bscJSON_ArrayForEach(js_prop, js_root)
        {
            tsl_value_t prop_value = {0};
            char *str_msg = NULL;
            if (0 != tranform_value_from_json(&prop_value, js_prop, &str_msg))
            {
                if (NULL != str_msg)
                {
                    free(str_msg);
                }
                ret = ez_errno_tsl_value_illegal;
                break;
            }

            for (size_t i = 0; i < schema->type_array.prop_num; i++)
            {
                tsl_schema_desc *tsl_schema = (tsl_schema_desc *)(schema->type_array.item_prop + sizeof(tsl_schema_desc) * i);
                ret = check_schema_value(tsl_schema, &prop_value);
            }

            if (NULL != str_msg)
            {
                free(str_msg);
            }

            if (0 != ret)
            {
                break;
            }
        }

    } while (false);
    if (NULL != js_root)
    {
        bscJSON_Delete(js_root);
    }

    return ret;
}

static int check_object_value(tsl_schema_desc *schema, tsl_value_t *value)
{
    int ret = 0;
    bscJSON *js_root = NULL;
    do
    {
        js_root = bscJSON_Parse((char *)value->value);
        if (NULL == js_root)
        {
            ez_log_e(TAG_TSL, "object value parse failed.");
            ret = ez_errno_tsl_value_illegal;
            break;
        }

        int obj_num = bscJSON_GetArraySize(js_root);
        if (0 != schema->type_object.min_props && 0 != schema->type_object.max_props && schema->type_object.min_props <= schema->type_object.max_props)
        {
            if (schema->type_object.min_props > obj_num || schema->type_object.max_props < obj_num)
            {
                ez_log_e(TAG_TSL, "object value props not match. min: %d, cur: %d, max: %d", schema->type_object.min_props, obj_num, schema->type_object.max_props);
                ret = ez_errno_tsl_value_illegal;
                break;
            }
        }

        if (0 != schema->type_object.req_num)
        {
            for (size_t i = 0; i < schema->type_object.req_num; i++)
            {
                bscJSON *js_require = bscJSON_GetObjectItem(js_root, schema->type_object.required + MAX_ARR_REQUIRE_LENGTH * i);
                if (NULL == js_require)
                {
                    ez_log_e(TAG_TSL, "object require absent: %s", schema->type_object.required + MAX_ARR_REQUIRE_LENGTH * i);
                    ret = ez_errno_tsl_value_illegal;
                    break;
                }
            }
        }
        if (0 != ret)
        {
            break;
        }

        bscJSON *js_prop = NULL;
        bscJSON_ArrayForEach(js_prop, js_root)
        {
            tsl_value_t prop_value = {0};
            char *str_msg = NULL;
            if (0 != tranform_value_from_json(&prop_value, js_prop, &str_msg))
            {
                if (NULL != str_msg)
                {
                    free(str_msg);
                }
                ret = ez_errno_tsl_value_illegal;
                break;
            }
            bool is_hit = false;
            for (size_t i = 0; i < schema->type_object.prop_num; i++)
            {
                tsl_schema_desc *tsl_schema = (tsl_schema_desc *)(schema->type_object.property + sizeof(tsl_schema_desc) * i);
                if (0 == strcmp(tsl_schema->prop_key, js_prop->string))
                {
                    is_hit = true;
                    ret = check_schema_value(tsl_schema, &prop_value);
                    break;
                }
            }
            if (NULL != str_msg)
            {
                free(str_msg);
            }
            if (!is_hit)
            {
                ez_log_e(TAG_TSL, "identifier not match: %s", js_prop->string);
                ret = ez_errno_tsl_value_illegal;
            }
            if (0 != ret)
            {
                break;
            }
        }
        if (0 != ret)
        {
            break;
        }

    } while (false);

    if (NULL != js_root)
    {
        bscJSON_Delete(js_root);
    }

    return ret;
}

int check_schema_value(const void *schema_dsc, const void *tsl_value)
{
    int ret = 0;
    tsl_schema_desc *schema = (tsl_schema_desc *)schema_dsc;
    tsl_value_t *value = (tsl_value_t *)tsl_value;

    if (schema->item_type != value->type)
    {
        if (schema->item_type == tsl_data_type_double && value->type == tsl_data_type_int)
        {
            tsl_value_t tsl_value = {0};
            tsl_value.type = tsl_data_type_double;
            tsl_value.value_double = (double)value->value_int;
            tsl_value.size = sizeof(double);
            return check_double_value(schema, &tsl_value);
        }
        else
        {
            ez_log_e(TAG_TSL, "type not match. schema type: %d, value type: %d", schema->item_type, value->type);
            return ez_errno_tsl_value_type;
        }
    }

    switch (value->type)
    {
    case tsl_data_type_bool:

        break;
    case tsl_data_type_int:
        ret = check_int_value(schema, value);
        break;
    case tsl_data_type_double:
        ret = check_double_value(schema, value);
        break;
    case tsl_data_type_string:
        ret = check_string_value(schema, value);
        break;
    case tsl_data_type_array:
        ret = check_array_value(schema, value);
        break;
    case tsl_data_type_object:
        ret = check_object_value(schema, value);
        break;

    default:
        break;
    }
    return ret;
}

#endif