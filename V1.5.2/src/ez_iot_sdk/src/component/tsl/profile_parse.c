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

#include <stdio.h>
#include "ez_iot_def.h"
#include "ezconfig.h"
#include "mcuconfig.h"
#include "ez_iot_tsl_adapter.h"
#include "profile_parse.h"
#include "ez_iot_errno.h"

const char *tsl_key_version = "version";
const char *tsl_key_resources = "resources";
const char *tsl_key_identifier = "identifier";
const char *tsl_key_rsc_category = "resourceCategory";
const char *tsl_key_local_index = "localIndex";
const char *tsl_key_domains = "domains";
const char *tsl_key_props = "props";
const char *tsl_key_actions = "actions";
const char *tsl_key_events = "events";
const char *tsl_key_access = "access";
#ifndef COMPONENT_TSL_PROFILE_STRIP
const char *tsl_key_schema = "schema";
const char *tsl_key_direction = "direction";
const char *tsl_key_input = "input";
const char *tsl_key_output = "output";
const char *tsl_key_eventtype = "eventType";
const char *tsl_key_type = "type";
const char *tsl_key_data_string = "str";
const char *tsl_key_data_boolean = "bool";
const char *tsl_key_data_integer = "int";
const char *tsl_key_data_number = "num";
const char *tsl_key_data_array = "arr";
const char *tsl_key_data_object = "obj";
const char *tsl_key_data_key = "key";
const char *tsl_key_enum = "enum";
const char *tsl_key_minlength = "mnl";
const char *tsl_key_maxlength = "mxl";
const char *tsl_key_minimum = "mn";
const char *tsl_key_exclusive_minimum = "emn";
const char *tsl_key_maximum = "mx";
const char *tsl_key_exclusive_maximum = "emx";
const char *tsl_key_multiple = "multi";
const char *tsl_key_items = "itm";
const char *tsl_key_min_items = "mnitm";
const char *tsl_key_max_items = "mxitm";
const char *tsl_key_unique_items = "unq";
const char *tsl_key_required = "req";
const char *tsl_key_max_properties = "mnp";
const char *tsl_key_min_properties = "mxp";
const char *tsl_key_dependencies = "dep";
const char *tsl_key_schema_properties = "prop";

static int free_schema_memory(tsl_schema_desc *tsl_schema)
{
    switch (tsl_schema->item_type)
    {
    case tsl_data_type_bool:
        break;
    case tsl_data_type_double:
    {
        if (NULL != tsl_schema->type_number.num_enum)
        {
            free(tsl_schema->type_number.num_enum);
            tsl_schema->type_number.num_enum = NULL;
        }
    }
    break;
    case tsl_data_type_int:
    {
        if (NULL != tsl_schema->type_integer.int_enum)
        {
            free(tsl_schema->type_integer.int_enum);
            tsl_schema->type_integer.int_enum = NULL;
        }
    }
    break;
    case tsl_data_type_string:
    {
        if (NULL != tsl_schema->type_string.str_enum)
        {
            free(tsl_schema->type_string.str_enum);
            tsl_schema->type_string.str_enum = NULL;
        }
    }
    break;
    case tsl_data_type_array:
    {
        if (NULL != tsl_schema->type_array.item_prop)
        {
            int index = tsl_schema->type_array.prop_num;
            for (int i = 0; i < index; i++)
            {
                free_schema_memory(tsl_schema->type_array.item_prop + i);
            }
            free(tsl_schema->type_array.item_prop);
            tsl_schema->type_array.item_prop = NULL;
        }
    }
    break;
    case tsl_data_type_object:
    {
        if (NULL != tsl_schema->type_object.required)
        {
            free(tsl_schema->type_object.required);
            tsl_schema->type_object.required = NULL;
        }
        if (NULL != tsl_schema->type_object.property)
        {
            int index = tsl_schema->type_object.prop_num;
            for (int i = 0; i < index; i++)
            {
                free_schema_memory(tsl_schema->type_object.property + i);
            }
            free(tsl_schema->type_object.property);
            tsl_schema->type_object.property = NULL;
        }
    }
    break;

    default:
        break;
    }
    return 0;
}

static int free_props_memory(tsl_domain_prop *tsl_props, int prop_num)
{
    int index = 0;
    for (index = 0; index < prop_num; index++)
    {
        tsl_domain_prop *prop = tsl_props + index;
        free_schema_memory(&prop->prop_desc);
    }
    return 0;
}

static int free_actions_memory(tsl_domain_action *tsl_actions, int action_num)
{
    int index = 0;
    for (index = 0; index < action_num; index++)
    {
        tsl_domain_action *action = tsl_actions + index;
        free_schema_memory(&action->input_schema);
        free_schema_memory(&action->output_schema);
    }
    return 0;
}

static int free_events_memory(tsl_domain_event *tsl_events, int event_num)
{
    int index = 0;
    if (NULL != tsl_events->event_type)
    {
        free(tsl_events->event_type);
        tsl_events->event_type = NULL;
    }

    for (index = 0; index < event_num; index++)
    {
        tsl_domain_event *event = tsl_events + index;
        free_schema_memory(&event->input_schema);
        free_schema_memory(&event->output_schema);
    }
    return 0;
}
#endif

static int free_domain_memory(tsl_rsc_domain *tsl_domain, int domain_num)
{
    int index = 0;
    for (index = 0; index < domain_num; index++)
    {
        tsl_rsc_domain *domain = tsl_domain + index;
        if (NULL != domain->prop)
        {
#ifndef COMPONENT_TSL_PROFILE_STRIP
            free_props_memory(domain->prop, domain->prop_num);
#endif
            free(domain->prop);
            domain->prop = NULL;
        }

        if (NULL != domain->action)
        {
#ifndef COMPONENT_TSL_PROFILE_STRIP
            free_actions_memory(domain->action, domain->action_num);
#endif
            free(domain->action);
            domain->action = NULL;
        }

        if (NULL != domain->event)
        {
#ifndef COMPONENT_TSL_PROFILE_STRIP
            free_events_memory(domain->event, domain->event_num);
#endif
            free(domain->event);
            domain->event = NULL;
        }
    }
    return 0;
}

int free_profile_memory(ez_iot_tsl_capacity_t *capacity)
{
    int rsc_num = capacity->rsc_num;
    for (int i = 0; i < rsc_num; i++)
    {
        tsl_profile_resource *tsl_rsc = capacity->resource + i;
        if (NULL != tsl_rsc)
        {
            if (NULL != tsl_rsc->index)
            {
                free(tsl_rsc->index);
                tsl_rsc->index = NULL;
            }

            if (NULL != tsl_rsc->domain)
            {
                free_domain_memory(tsl_rsc->domain, tsl_rsc->domain_num);
                free(tsl_rsc->domain);
                tsl_rsc->domain = NULL;
            }
        }
    }

    if (NULL != capacity->resource)
    {
        free(capacity->resource);
        capacity->resource = NULL;
    }
    return 0;
}

#ifndef COMPONENT_TSL_PROFILE_STRIP
static int parse_schema(tsl_schema_desc *schema, bscJSON *js_schema)
{
    int rv = ez_errno_succ;
    if (NULL == schema || NULL == js_schema)
    {
        ez_log_e(TAG_TSL, "parse schema param error.");
        return ez_errno_tsl_param_invalid;
    }

    ez_log_v(TAG_TSL, "&&&&&&&&&&&&&&&& schema &&&&&&&&&&");
    do
    {
        bscJSON *js_type = bscJSON_GetObjectItem(js_schema, tsl_key_type);
        if (NULL == js_type || bscJSON_String != js_type->type)
        {
            ez_log_e(TAG_TSL, "schema type absent.");
            break;
        }
        if (0 == strcmp(js_type->valuestring, tsl_key_data_string))
        {
            schema->item_type = tsl_data_type_string;
        }
        else if (0 == strcmp(js_type->valuestring, tsl_key_data_boolean))
        {
            schema->item_type = tsl_data_type_bool;
        }
        else if (0 == strcmp(js_type->valuestring, tsl_key_data_integer))
        {
            schema->item_type = tsl_data_type_int;
        }
        else if (0 == strcmp(js_type->valuestring, tsl_key_data_number))
        {
            schema->item_type = tsl_data_type_double;
        }
        else if (0 == strcmp(js_type->valuestring, tsl_key_data_object))
        {
            schema->item_type = tsl_data_type_object;
        }
        else if (0 == strcmp(js_type->valuestring, tsl_key_data_array))
        {
            schema->item_type = tsl_data_type_array;
        }
        else
        {
            schema->item_type = tsl_data_type_max;
        }

        bscJSON *js_key = bscJSON_GetObjectItem(js_schema, tsl_key_data_key);
        if (NULL != js_key && bscJSON_String == js_key->type)
        {
            strncpy(schema->prop_key, js_key->valuestring, sizeof(schema->prop_key) - 1);
            ez_log_v(TAG_TSL, "schema prop_key: %s", schema->prop_key);
        }

        switch (schema->item_type)
        {
        case tsl_data_type_bool:
            break;

        case tsl_data_type_int:
        {
            bscJSON *js_min = bscJSON_GetObjectItem(js_schema, tsl_key_minimum);
            if (NULL != js_min && bscJSON_Number == js_min->type)
            {
                schema->type_integer.minimum = js_min->valueint;
                ez_log_v(TAG_TSL, "integer minimum: %d", schema->type_integer.minimum);
            }
            else
            {
                js_min = bscJSON_GetObjectItem(js_schema, tsl_key_exclusive_minimum);
                if (NULL != js_min && bscJSON_Number == js_min->type)
                {
                    schema->type_integer.exminimum = js_min->valueint;
                    ez_log_v(TAG_TSL, "integer ex minimum: %d", schema->type_integer.exminimum);
                }
            }

            bscJSON *js_max = bscJSON_GetObjectItem(js_schema, tsl_key_maximum);
            if (NULL != js_max && bscJSON_Number == js_max->type)
            {
                schema->type_integer.maximum = js_max->valueint;
                ez_log_v(TAG_TSL, "integer maximum: %d", schema->type_integer.maximum);
            }
            else
            {
                js_max = bscJSON_GetObjectItem(js_schema, tsl_key_exclusive_maximum);
                if (NULL != js_max && bscJSON_Number == js_max->type)
                {
                    schema->type_integer.exmaximum = js_max->valueint;
                    ez_log_v(TAG_TSL, "integer ex maximum: %d", schema->type_integer.exmaximum);
                }
            }

            bscJSON *js_multi = bscJSON_GetObjectItem(js_schema, tsl_key_multiple);
            if (NULL != js_multi && bscJSON_Number == js_multi->type)
            {
                schema->type_integer.multiple = js_multi->valuedouble;
                ez_log_v(TAG_TSL, "integer multi: %d", schema->type_integer.multiple);
            }

            bscJSON *js_enum = bscJSON_GetObjectItem(js_schema, tsl_key_enum);
            if (NULL != js_enum && bscJSON_Array == js_enum->type)
            {
                int array_size = bscJSON_GetArraySize(js_enum);
                if (0 == array_size)
                {
                    ez_log_e(TAG_TSL, "integer enum size 0.");
                    break;
                }

                schema->type_integer.int_enum = (int *)malloc(sizeof(int) * array_size);
                if (NULL == schema->type_integer.int_enum)
                {
                    ez_log_e(TAG_TSL, "memory not enough.");
                    break;
                }
                memset(schema->type_integer.int_enum, 0, sizeof(int) * array_size);

                schema->type_integer.enum_num = array_size;
                ez_log_v(TAG_TSL, "integer enum size: %d", schema->type_integer.enum_num);

                for (int i = 0; i < array_size; i++)
                {
                    bscJSON *js_int = bscJSON_GetArrayItem(js_enum, i);
                    if (NULL != js_int && bscJSON_Number == js_int->type)
                    {
                        *(schema->type_integer.int_enum + i) = js_int->valueint;
                        ez_log_v(TAG_TSL, "integer enum %d: %d", i, *(schema->type_integer.int_enum + i));
                    }
                }
            }
        }
        break;

        case tsl_data_type_double:
        {
            bscJSON *js_min = bscJSON_GetObjectItem(js_schema, tsl_key_minimum);
            if (NULL != js_min && bscJSON_Number == js_min->type)
            {
                schema->type_number.minimum = js_min->valuedouble;
                ez_log_v(TAG_TSL, "number minimum: %f", schema->type_number.minimum);
            }
            else
            {
                js_min = bscJSON_GetObjectItem(js_schema, tsl_key_exclusive_minimum);
                if (NULL != js_min && bscJSON_Number == js_min->type)
                {
                    schema->type_number.exminimum = js_min->valuedouble;
                    ez_log_v(TAG_TSL, "number ex minimum: %f", schema->type_number.exminimum);
                }
            }

            bscJSON *js_max = bscJSON_GetObjectItem(js_schema, tsl_key_maximum);
            if (NULL != js_max && bscJSON_Number == js_max->type)
            {
                schema->type_number.maximum = js_max->valuedouble;
                ez_log_v(TAG_TSL, "number maximum: %f", schema->type_number.maximum);
            }
            else
            {
                js_max = bscJSON_GetObjectItem(js_schema, tsl_key_exclusive_maximum);
                if (NULL != js_max && bscJSON_Number == js_max->type)
                {
                    schema->type_number.exmaximum = js_max->valuedouble;
                    ez_log_v(TAG_TSL, "number ex maximum: %f", schema->type_number.exmaximum);
                }
            }

            bscJSON *js_multi = bscJSON_GetObjectItem(js_schema, tsl_key_multiple);
            if (NULL != js_multi && bscJSON_Number == js_multi->type)
            {
                schema->type_number.multiple = js_multi->valuedouble;
                ez_log_v(TAG_TSL, "number multi: %f", schema->type_number.multiple);
            }

            bscJSON *js_enum = bscJSON_GetObjectItem(js_schema, tsl_key_enum);
            if (NULL != js_enum && bscJSON_Array == js_enum->type)
            {
                int array_size = bscJSON_GetArraySize(js_enum);
                if (0 == array_size)
                {
                    ez_log_e(TAG_TSL, "number enum size 0");
                    break;
                }

                schema->type_number.num_enum = (double *)malloc(sizeof(double) * array_size);
                if (NULL == schema->type_number.num_enum)
                {
                    ez_log_e(TAG_TSL, "memory not enough.");
                    break;
                }
                memset(schema->type_number.num_enum, 0, sizeof(double) * array_size);

                schema->type_number.enum_num = array_size;
                ez_log_v(TAG_TSL, "number enum size: %d", schema->type_number.enum_num);

                for (int i = 0; i < array_size; i++)
                {
                    bscJSON *js_num = bscJSON_GetArrayItem(js_enum, i);
                    if (NULL != js_num && bscJSON_Number == js_num->type)
                    {
                        *(schema->type_number.num_enum + i) = js_num->valuedouble;
                        ez_log_v(TAG_TSL, "number enum %d: %f", i, *(schema->type_number.num_enum + i));
                    }
                }
            }
        }
        break;

        case tsl_data_type_string:
        {
            bscJSON *js_maxlen = bscJSON_GetObjectItem(js_schema, tsl_key_maxlength);
            if (NULL != js_maxlen && bscJSON_Number == js_maxlen->type)
            {
                schema->type_string.max_len = js_maxlen->valueint;
                ez_log_v(TAG_TSL, "string max len: %d", schema->type_string.max_len);
            }
            bscJSON *js_minlen = bscJSON_GetObjectItem(js_schema, tsl_key_minlength);
            if (NULL != js_minlen && bscJSON_Number == js_minlen->type)
            {
                schema->type_string.min_len = js_minlen->valueint;
                ez_log_v(TAG_TSL, "string min len: %d", schema->type_string.min_len);
            }

            bscJSON *js_enum = bscJSON_GetObjectItem(js_schema, tsl_key_enum);
            if (NULL != js_enum && bscJSON_Array == js_enum->type)
            {
                int array_size = bscJSON_GetArraySize(js_enum);
                if (0 == array_size)
                {
                    ez_log_e(TAG_TSL, "string enum size 0.");
                    break;
                }

                schema->type_string.str_enum = (char *)malloc(MAX_STRING_ENUM_LENGTH * array_size);
                if (NULL == schema->type_string.str_enum)
                {
                    ez_log_e(TAG_TSL, "memory not enough.");
                    break;
                }
                memset(schema->type_string.str_enum, 0, MAX_STRING_ENUM_LENGTH * array_size);

                schema->type_string.enum_num = array_size;
                ez_log_v(TAG_TSL, "string enum size: %d", schema->type_string.enum_num);

                for (int i = 0; i < array_size; i++)
                {
                    bscJSON *js_str = bscJSON_GetArrayItem(js_enum, i);
                    if (NULL != js_str && bscJSON_String == js_str->type)
                    {
                        strncpy(schema->type_string.str_enum + (MAX_STRING_ENUM_LENGTH * i), js_str->valuestring, MAX_STRING_ENUM_LENGTH - 1);
                        ez_log_v(TAG_TSL, "string enum %d: %s", i, schema->type_string.str_enum + (MAX_STRING_ENUM_LENGTH * i));
                    }
                }
            }
        }
        break;

        case tsl_data_type_array:
        {
            bscJSON *js_minitems = bscJSON_GetObjectItem(js_schema, tsl_key_min_items);
            if (NULL != js_minitems && bscJSON_Number == js_minitems->type)
            {
                schema->type_array.minItem = js_minitems->valueint;
                ez_log_v(TAG_TSL, "array min items: %d", schema->type_array.minItem);
            }

            bscJSON *js_maxitems = bscJSON_GetObjectItem(js_schema, tsl_key_max_items);
            if (NULL != js_maxitems && bscJSON_Number == js_maxitems->type)
            {
                schema->type_array.maxItem = js_maxitems->valueint;
                ez_log_v(TAG_TSL, "array max items: %d", schema->type_array.maxItem);
            }

            bscJSON *js_items = bscJSON_GetObjectItem(js_schema, tsl_key_items);
            if (NULL == js_items || bscJSON_Object != js_items->type)
            {
                ez_log_e(TAG_TSL, "js_items absent.");
                rv = ez_errno_tsl_param_invalid;
                break;
            }

            schema->type_array.item_prop = (tsl_schema_desc *)malloc(sizeof(tsl_schema_desc));
            if (NULL == schema->type_array.item_prop)
            {
                ez_log_e(TAG_TSL, "memory not enough.");
                rv = ez_errno_tsl_memory;
                break;
            }
            memset(schema->type_array.item_prop, 0, sizeof(tsl_schema_desc));
            schema->type_array.prop_num = 1;
            ez_log_v(TAG_TSL, "array prop size: %d", schema->type_array.prop_num);

            parse_schema(schema->type_array.item_prop, js_items);
        }
        break;

        case tsl_data_type_object:
        {
            bscJSON *js_max_props = bscJSON_GetObjectItem(js_schema, tsl_key_max_properties);
            if (NULL != js_max_props && bscJSON_Number == js_max_props->type)
            {
                schema->type_object.max_props = js_max_props->valueint;
                ez_log_v(TAG_TSL, "obj max prop num: %d", schema->type_object.max_props);
            }

            bscJSON *js_min_props = bscJSON_GetObjectItem(js_schema, tsl_key_min_properties);
            if (NULL != js_min_props && bscJSON_Number == js_min_props->type)
            {
                schema->type_object.min_props = js_min_props->valueint;
                ez_log_v(TAG_TSL, "obj min prop num: %d", schema->type_object.min_props);
            }

            bscJSON *js_required = bscJSON_GetObjectItem(js_schema, tsl_key_required);
            if (NULL != js_required && bscJSON_Array == js_required->type)
            {
                int arr_size = bscJSON_GetArraySize(js_required);
                if (0 != arr_size)
                {
                    schema->type_object.required = (char *)malloc(MAX_ARR_REQUIRE_LENGTH * arr_size);
                    if (NULL == schema->type_object.required)
                    {
                        ez_log_e(TAG_TSL, "memory not enough.");
                        rv = ez_errno_tsl_memory;
                        break;
                    }
                    memset(schema->type_object.required, 0, MAX_ARR_REQUIRE_LENGTH * arr_size);

                    schema->type_object.req_num = arr_size;
                    ez_log_v(TAG_TSL, "schema obj required size: %d.", schema->type_object.req_num);

                    for (int i = 0; i < arr_size; i++)
                    {
                        char *enum_str = schema->type_object.required + MAX_ARR_REQUIRE_LENGTH * i;
                        bscJSON *js_str = bscJSON_GetArrayItem(js_required, i);
                        if (NULL != js_str && bscJSON_String == js_str->type)
                        {
                            strncpy(enum_str, js_str->valuestring, MAX_ARR_REQUIRE_LENGTH - 1);
                            ez_log_v(TAG_TSL, "schema required index: %d, %s", i, enum_str);
                        }
                    }
                }
                else
                {
                    ez_log_w(TAG_TSL, "object required size 0.");
                }
            }

            bscJSON *js_obj_prop = bscJSON_GetObjectItem(js_schema, tsl_key_schema_properties);
            if (NULL != js_obj_prop && bscJSON_Array == js_obj_prop->type)
            {
                int arr_size = bscJSON_GetArraySize(js_obj_prop);
                if (0 != arr_size)
                {
                    schema->type_object.property = (tsl_schema_desc *)malloc(sizeof(tsl_schema_desc) * arr_size);
                    if (NULL == schema->type_object.property)
                    {
                        ez_log_e(TAG_TSL, "memory not enough.");
                        rv = ez_errno_tsl_memory;
                        break;
                    }
                    memset(schema->type_object.property, 0, sizeof(tsl_schema_desc) * arr_size);

                    schema->type_object.prop_num = arr_size;
                    ez_log_v(TAG_TSL, "schema prop size: %d", schema->type_object.prop_num);

                    for (int i = 0; i < arr_size; i++)
                    {
                        tsl_schema_desc *sub_schema = schema->type_object.property + sizeof(tsl_schema_desc) * i;
                        bscJSON *js_prop = bscJSON_GetArrayItem(js_obj_prop, i);
                        if (NULL != js_prop && bscJSON_Object == js_prop->type)
                        {
                            parse_schema(sub_schema, js_prop);
                        }
                    }
                }
                else
                {
                    ez_log_w(TAG_TSL, "obj prop size 0.");
                }
            }
        }
        break;

        default:
            break;
        }

    } while (false);

    return rv;
}
#endif

static int parse_domain_props(tsl_domain_prop *p_props, int props_num, bscJSON *js_props)
{
    if (NULL == js_props || NULL == p_props || 0 >= props_num)
    {
        ez_log_e(TAG_TSL, "parse domain props param error.");
        return ez_errno_tsl_param_invalid;
    }

    ez_log_v(TAG_TSL, "@@@@@@@@@@@@ props @@@@@@@@@@");
    int rv = ez_errno_succ;
    for (int i = 0; i < props_num; i++)
    {
        tsl_domain_prop *prop = p_props + i;
        bscJSON *js_obj = bscJSON_GetArrayItem(js_props, i);
        if (NULL == js_obj || bscJSON_Object != js_obj->type)
        {
            ez_log_e(TAG_TSL, "prop object absent.");
            rv = ez_errno_tsl_param_invalid;
            break;
        }
        ez_log_v(TAG_TSL, "prop index: %d", i);

        bscJSON *js_identify = bscJSON_GetObjectItem(js_obj, tsl_key_identifier);
        if (NULL == js_identify || bscJSON_String != js_identify->type)
        {
            ez_log_e(TAG_TSL, "prop identify absent");
            rv = ez_errno_tsl_param_invalid;
            break;
        }
        strncpy(prop->identifier, js_identify->valuestring, sizeof(prop->identifier) - 1);
        ez_log_v(TAG_TSL, "prop identifier: %s", prop->identifier);

        bscJSON *js_access = bscJSON_GetObjectItem(js_obj, tsl_key_access);
        if (NULL == js_access || bscJSON_String != js_access->type)
        {
            ez_log_e(TAG_TSL, "prop schema absent.");
            rv = ez_errno_tsl_param_invalid;
            break;
        }
        if (NULL != strstr(js_access->valuestring, "r"))
        {
            prop->access = ACCESS_READ;
        }
        if (NULL != strstr(js_access->valuestring, "w"))
        {
            prop->access |= ACCESS_WRITE;
        }
        ez_log_v(TAG_TSL, "prop access: %d", prop->access);

#ifndef COMPONENT_TSL_PROFILE_STRIP
        bscJSON *js_ver = bscJSON_GetObjectItem(js_obj, tsl_key_version);
        if (NULL != js_ver && bscJSON_String == js_ver->type)
        {
            strncpy(prop->version, js_ver->valuestring, sizeof(prop->version) - 1);
            ez_log_v(TAG_TSL, "prop version: %s", prop->version);
        }

        bscJSON *js_schema = bscJSON_GetObjectItem(js_obj, tsl_key_schema);
        if (NULL == js_schema || bscJSON_Object != js_schema->type)
        {
            ez_log_e(TAG_TSL, "prop schema absent.");
            rv = ez_errno_tsl_param_invalid;
            break;
        }

        rv = parse_schema(&prop->prop_desc, js_schema);
        if (0 != rv)
        {
            ez_log_e(TAG_TSL, "parse schema error.");
            break;
        }
#endif
    }

    return rv;
}

static int parse_domain_actions(tsl_domain_action *p_actions, int actions_num, bscJSON *js_actions)
{
    if (NULL == js_actions || NULL == p_actions || 0 >= actions_num)
    {
        ez_log_e(TAG_TSL, "parse domain actions param error.");
        return ez_errno_tsl_param_invalid;
    }

    ez_log_v(TAG_TSL, "@@@@@@@@@@@@ actions @@@@@@@@@@");
    int rv = ez_errno_succ;
    for (int i = 0; i < actions_num; i++)
    {
        tsl_domain_action *action = p_actions + i;

        bscJSON *js_obj = bscJSON_GetArrayItem(js_actions, i);
        if (NULL == js_obj || bscJSON_Object != js_obj->type)
        {
            rv = ez_errno_tsl_internal;
            ez_log_e(TAG_TSL, "action absent.");
            break;
        }

        bscJSON *js_identify = bscJSON_GetObjectItem(js_obj, tsl_key_identifier);
        if (NULL == js_identify || bscJSON_String != js_identify->type)
        {
            rv = ez_errno_tsl_param_invalid;
            ez_log_e(TAG_TSL, "identify absent.");
            break;
        }
        strncpy(action->identifier, js_identify->valuestring, sizeof(action->identifier) - 1);
        ez_log_v(TAG_TSL, "action identifier: %s", action->identifier);

#ifndef COMPONENT_TSL_PROFILE_STRIP
        bscJSON *js_ver = bscJSON_GetObjectItem(js_obj, tsl_key_version);
        if (NULL != js_ver && bscJSON_String == js_ver->type)
        {
            strncpy(action->version, js_ver->valuestring, sizeof(action->version) - 1);
            ez_log_v(TAG_TSL, "action version: %s", action->version);
        }

        bscJSON *js_direction = bscJSON_GetObjectItem(js_obj, tsl_key_direction);
        if (NULL == js_direction || bscJSON_String != js_direction->type)
        {
            rv = ez_errno_tsl_param_invalid;
            ez_log_e(TAG_TSL, "direction absent.");
            break;
        }
        strncpy(action->direction, js_direction->valuestring, sizeof(action->direction) - 1);
        ez_log_v(TAG_TSL, "action direction: %s", action->direction);

        bscJSON *js_input = bscJSON_GetObjectItem(js_obj, tsl_key_input);
        if (NULL != js_input && bscJSON_Object == js_input->type)
        {
            bscJSON *js_schema = bscJSON_GetObjectItem(js_input, tsl_key_schema);
            if (NULL == js_schema || bscJSON_Object != js_schema->type)
            {
                rv = ez_errno_tsl_param_invalid;
                ez_log_e(TAG_TSL, "action input schema absent.");
                break;
            }

            rv = parse_schema(&action->input_schema, js_schema);
            if (rv != 0)
            {
                ez_log_e(TAG_TSL, "actiom input absent.");
                break;
            }
        }

        bscJSON *js_output = bscJSON_GetObjectItem(js_obj, tsl_key_output);
        if (NULL != js_output && bscJSON_Object == js_output->type)
        {
            bscJSON *js_schema = bscJSON_GetObjectItem(js_output, tsl_key_schema);
            if (NULL == js_schema || bscJSON_Object != js_schema->type)
            {
                rv = ez_errno_tsl_param_invalid;
                ez_log_e(TAG_TSL, "action output schema absent.");
                break;
            }
            rv = parse_schema(&action->output_schema, js_schema);
            if (rv != 0)
            {
                ez_log_e(TAG_TSL, "action output absent.");
                break;
            }
        }
#endif
    }

    return rv;
}

static int parse_domain_events(tsl_domain_event *p_events, int events_num, bscJSON *js_events)
{
    if (NULL == js_events || NULL == p_events || 0 >= events_num)
    {
        ez_log_e(TAG_TSL, "parse domain events param error.");
        return ez_errno_tsl_param_invalid;
    }
    int rv = ez_errno_succ;

    int arr_size = bscJSON_GetArraySize(js_events);
    for (size_t i = 0; i < arr_size; i++)
    {
        tsl_domain_event *event = p_events + i;
        bscJSON *js_obj = bscJSON_GetArrayItem(js_events, i);
        if (NULL == js_obj || bscJSON_Object != js_obj->type)
        {
            rv = ez_errno_tsl_internal;
            ez_log_e(TAG_TSL, "event absent.");
            break;
        }

        bscJSON *js_identify = bscJSON_GetObjectItem(js_obj, tsl_key_identifier);
        if (NULL == js_identify || bscJSON_String != js_identify->type)
        {
            rv = ez_errno_tsl_param_invalid;
            ez_log_e(TAG_TSL, "identify absent");
            break;
        }
        strncpy(event->identifier, js_identify->valuestring, sizeof(event->identifier) - 1);
        ez_log_v(TAG_TSL, "event identifier: %s", event->identifier);

#ifndef COMPONENT_TSL_PROFILE_STRIP
        bscJSON *js_ver = bscJSON_GetObjectItem(js_obj, tsl_key_version);
        if (NULL != js_ver && bscJSON_String == js_ver->type)
        {
            strncpy(event->version, js_ver->valuestring, sizeof(event->version) - 1);
            ez_log_v(TAG_TSL, "event version: %s", event->version);
        }

        bscJSON *js_event_type = bscJSON_GetObjectItem(js_obj, tsl_key_eventtype);
        if (NULL == js_event_type || bscJSON_Array != js_event_type->type)
        {
            rv = ez_errno_tsl_param_invalid;
            ez_log_e(TAG_TSL, "eventType absent.");
            break;
        }

        int arr_size = bscJSON_GetArraySize(js_event_type);
        if (0 != arr_size)
        {
            event->event_type = (char *)malloc(MAX_EVENT_TYPE_KEY_LENGTH * arr_size);
            if (NULL == event->event_type)
            {
                ez_log_e(TAG_TSL, "memory not enough.");
                rv = ez_errno_tsl_memory;
                break;
            }
            memset(event->event_type, 0, MAX_EVENT_TYPE_KEY_LENGTH * arr_size);

            event->enum_num = arr_size;

            for (int j = 0; j < arr_size; j++)
            {
                char *type = event->event_type + MAX_EVENT_TYPE_KEY_LENGTH * j;
                bscJSON *js_type = bscJSON_GetArrayItem(js_event_type, i);
                if (NULL != js_type && bscJSON_String == js_type->type)
                {
                    strncpy(type, js_type->valuestring, MAX_EVENT_TYPE_KEY_LENGTH - 1);
                }
            }
        }
        else
        {
            ez_log_w(TAG_TSL, "event type arr size 0.");
        }

        bscJSON *js_input = bscJSON_GetObjectItem(js_obj, tsl_key_input);
        if (NULL != js_input && bscJSON_Object == js_input->type)
        {
            bscJSON *js_schema = bscJSON_GetObjectItem(js_input, tsl_key_schema);
            if (NULL == js_schema || bscJSON_Object != js_schema->type)
            {
                rv = ez_errno_tsl_internal;
                ez_log_e(TAG_TSL, "event input schema absent.");
                break;
            }

            rv = parse_schema(&event->input_schema, js_schema);
            if (rv != 0)
            {
                ez_log_e(TAG_TSL, "event schema parse error.");
                break;
            }
        }

        bscJSON *js_output = bscJSON_GetObjectItem(js_obj, tsl_key_output);
        if (NULL != js_output && bscJSON_Object == js_output->type)
        {
            bscJSON *js_schema = bscJSON_GetObjectItem(js_output, tsl_key_schema);
            if (NULL == js_schema || bscJSON_Object != js_schema->type)
            {
                rv = ez_errno_tsl_internal;
                ez_log_e(TAG_TSL, "event output schema absent.");
                break;
            }

            rv = parse_schema(&event->output_schema, js_schema);
            if (rv != 0)
            {
                ez_log_e(TAG_TSL, "event schema parse error.");
                break;
            }
        }
#endif
    }

    return rv;
}

int parse_domain(tsl_rsc_domain *pdomain, int domain_num, bscJSON *js_domains)
{
    if (NULL == pdomain || 0 >= domain_num)
    {
        ez_log_e(TAG_TSL, "parse_domain param error.");
        return ez_errno_tsl_param_invalid;
    }

    int rv = ez_errno_succ;

    ez_log_v(TAG_TSL, "--------------------------- domain ------------------------");
    for (int j = 0; j < domain_num; j++)
    {
        bscJSON *js_domain = bscJSON_GetArrayItem(js_domains, j);
        if (NULL == js_domain || bscJSON_Object != js_domain->type)
        {
            ez_log_e(TAG_TSL, "domain absent.");
            break;
        }
        ez_log_v(TAG_TSL, "################ domain index: %d ###############", j);

        tsl_rsc_domain *domain = pdomain + j;
        bscJSON *js_iden = bscJSON_GetObjectItem(js_domain, tsl_key_identifier);
        if (NULL == js_iden || bscJSON_String != js_iden->type)
        {
            ez_log_e(TAG_TSL, "domain iden absent.");
            rv = ez_errno_tsl_param_invalid;
            break;
        }
        strncpy(domain->identifier, js_iden->valuestring, sizeof(domain->identifier) - 1);
        ez_log_v(TAG_TSL, "domain identifier: %s", domain->identifier);

        bscJSON *js_props = bscJSON_GetObjectItem(js_domain, tsl_key_props);
        if (NULL == js_props || bscJSON_Array != js_props->type)
        {
            ez_log_w(TAG_TSL, "domain prop absent.");
            // rv = ez_errno_tsl_param_invalid;
            goto actions;
        }

        int props_num = bscJSON_GetArraySize(js_props);
        if (0 == props_num)
        {
            ez_log_w(TAG_TSL, "props array size 0.");
            goto actions;
        }

        domain->prop = (tsl_domain_prop *)malloc(sizeof(tsl_domain_prop) * props_num);
        if (NULL == domain->prop)
        {
            ez_log_e(TAG_TSL, "memory not enough.");
            rv = ez_errno_tsl_memory;
            break;
        }
        memset(domain->prop, 0, sizeof(tsl_domain_prop) * props_num);

        domain->prop_num = props_num;
        ez_log_v(TAG_TSL, "domain props num: %d", domain->prop_num);

        rv = parse_domain_props(domain->prop, domain->prop_num, js_props);
        if (0 != rv)
        {
            ez_log_e(TAG_TSL, "domain props parse failed.");
            break;
        }

    actions:
    {
        bscJSON *js_actions = bscJSON_GetObjectItem(js_domain, tsl_key_actions);
        if (NULL == js_actions || bscJSON_Array != js_actions->type)
        {
            ez_log_w(TAG_TSL, "domain action absent.");
            // rv = ez_errno_tsl_param_invalid;
            goto events;
        }
        int actions_num = bscJSON_GetArraySize(js_actions);
        if (0 == actions_num)
        {
            ez_log_w(TAG_TSL, "action array size 0.");
            goto events;
        }

        domain->action = (tsl_domain_action *)malloc(sizeof(tsl_domain_action) * actions_num);
        if (NULL == domain->action)
        {
            ez_log_e(TAG_TSL, "memory not enough.");
            rv = ez_errno_tsl_memory;
            break;
        }
        memset(domain->action, 0, sizeof(tsl_domain_action) * actions_num);

        domain->action_num = actions_num;
        ez_log_v(TAG_TSL, "domain actions num: %d", domain->action_num);

        rv = parse_domain_actions(domain->action, domain->action_num, js_actions);
        if (0 != rv)
        {
            ez_log_e(TAG_TSL, "domain actions parse failed.");
            break;
        }
    }

    events:
    {
        bscJSON *js_events = bscJSON_GetObjectItem(js_domain, tsl_key_events);
        if (NULL == js_events || bscJSON_Array != js_events->type)
        {
            ez_log_w(TAG_TSL, "domain events absent.");
            // rv = ez_errno_tsl_param_invalid;
            continue;
        }

        int events_num = bscJSON_GetArraySize(js_events);
        if (0 == events_num)
        {
            ez_log_w(TAG_TSL, "events array size 0.");
            continue;
        }

        domain->event = (tsl_domain_event *)malloc(sizeof(tsl_domain_event) * events_num);
        if (NULL == domain->event)
        {
            ez_log_e(TAG_TSL, "memory not enough.");
            rv = ez_errno_tsl_memory;
            break;
        }
        memset(domain->event, 0, sizeof(tsl_domain_event) * events_num);

        domain->event_num = events_num;
        ez_log_v(TAG_TSL, "domain events num: %d", domain->event_num);

        rv = parse_domain_events(domain->event, domain->event_num, js_events);
        if (0 != rv)
        {
            ez_log_e(TAG_TSL, "domain events parse failed.");
            break;
        }
    }
    }

    return rv;
}

int profile_parse(char *profile_value, int profile_len, ez_iot_tsl_capacity_t *capacity)
{
    int rv = ez_errno_succ;

    if (NULL == capacity || NULL == profile_value || 0 >= profile_len)
    {
        ez_log_e(TAG_TSL, "profile param error.");
        return ez_errno_tsl_param_invalid;
    }

    if (ez_iot_log_filter_lvl_get() > EZ_ELOG_LVL_DEBUG)
    {
        ez_log_hexdump(TAG_TSL, 16, (uint8_t *)profile_value, profile_len);
    }

    bscJSON *js_root = NULL;
    do
    {
        js_root = bscJSON_Parse(profile_value);
        if (NULL == js_root)
        {
            ez_log_e(TAG_TSL, "profile parse failed.");
            rv = ez_errno_tsl_memory;
            break;
        }

        bscJSON *js_ver = bscJSON_GetObjectItem(js_root, tsl_key_version);
        if (NULL == js_ver || bscJSON_String != js_ver->type)
        {
            ez_log_e(TAG_TSL, "version absent.");
            rv = ez_errno_tsl_param_invalid;
            break;
        }

        bscJSON *js_src = bscJSON_GetObjectItem(js_root, tsl_key_resources);
        if (NULL == js_src || bscJSON_Array != js_src->type)
        {
            ez_log_e(TAG_TSL, "resources absent.");
            rv = ez_errno_tsl_param_invalid;
            break;
        }

        int array_size = bscJSON_GetArraySize(js_src);
        if (0 == array_size)
        {
            ez_log_e(TAG_TSL, "array size 0.");
            rv = ez_errno_tsl_internal;
            break;
        }

        capacity->resource = (tsl_profile_resource *)malloc(sizeof(tsl_profile_resource) * array_size);
        if (NULL == capacity->resource)
        {
            ez_log_e(TAG_TSL, "memory not enough.");
            rv = ez_errno_tsl_memory;
            break;
        }
        memset(capacity->resource, 0, sizeof(tsl_profile_resource) * array_size);

        capacity->ref = 1;
        capacity->rsc_num = array_size;
        ez_log_v(TAG_TSL, "resources num: %d", capacity->rsc_num);

        for (int i = 0; i < array_size; i++)
        {
            tsl_profile_resource *p_src = capacity->resource + i;
            bscJSON *js_obj = bscJSON_GetArrayItem(js_src, i);
            if (NULL == js_obj)
            {
                ez_log_e(TAG_TSL, "array item %d error.", i);
                rv = ez_errno_tsl_internal;
                break;
            }

            bscJSON *js_category = bscJSON_GetObjectItem(js_obj, tsl_key_rsc_category);
            if (NULL == js_category || bscJSON_String != js_category->type)
            {
                ez_log_e(TAG_TSL, "category absent.");
                rv = ez_errno_tsl_param_invalid;
                break;
            }
            strncpy(p_src->rsc_category, js_category->valuestring, sizeof(p_src->rsc_category) - 1);
            ez_log_v(TAG_TSL, "source category: %s", p_src->rsc_category);

            bscJSON *js_index = bscJSON_GetObjectItem(js_obj, tsl_key_local_index);
            if (NULL == js_index || bscJSON_Array != js_index->type)
            {
                ez_log_e(TAG_TSL, "local_index absent.");
                rv = ez_errno_tsl_param_invalid;
                break;
            }

            int index_size = bscJSON_GetArraySize(js_index);
            if (0 == index_size)
            {
                ez_log_e(TAG_TSL, "local_index absent.");
                rv = ez_errno_tsl_internal;
                break;
            }

            p_src->index = (char *)malloc(MAX_LOCAL_INDEX_LENGTH * index_size);
            if (NULL == p_src->index)
            {
                ez_log_e(TAG_TSL, "memory not enough");
                rv = ez_errno_tsl_memory;
                break;
            }
            memset(p_src->index, 0, MAX_LOCAL_INDEX_LENGTH * index_size);

            p_src->index_num = index_size;
            ez_log_v(TAG_TSL, "resource local index num: %d", p_src->index_num);

            for (int j = 0; j < index_size; j++)
            {
                bscJSON *js_idx = bscJSON_GetArrayItem(js_index, j);
                if (NULL == js_idx || bscJSON_String != js_idx->type)
                {
                    ez_log_e(TAG_TSL, "local index absent.");
                    break;
                }
                strncpy(p_src->index + i, js_idx->valuestring, MAX_LOCAL_INDEX_LENGTH - 1);
                ez_log_v(TAG_TSL, "resource local index: %d, %s", j, p_src->index + j * MAX_LOCAL_INDEX_LENGTH);
            }

            bscJSON *js_domain = bscJSON_GetObjectItem(js_obj, tsl_key_domains);
            if (NULL == js_domain || bscJSON_Array != js_domain->type)
            {
                ez_log_e(TAG_TSL, "domain absent.");
                rv = ez_errno_tsl_param_invalid;
                break;
            }
            int domain_size = bscJSON_GetArraySize(js_domain);
            if (0 == domain_size)
            {
                ez_log_e(TAG_TSL, "domain array size 0.");
                rv = ez_errno_tsl_internal;
                break;
            }

            p_src->domain = (tsl_rsc_domain *)malloc(sizeof(tsl_rsc_domain) * domain_size);
            if (NULL == p_src->domain)
            {
                ez_log_e(TAG_TSL, "memory not enough.");
                rv = ez_errno_tsl_memory;
                break;
            }
            memset(p_src->domain, 0, sizeof(tsl_rsc_domain) * domain_size);

            p_src->domain_num = domain_size;
            ez_log_v(TAG_TSL, "resource domain num: %d", p_src->domain_num);

            rv = parse_domain(p_src->domain, p_src->domain_num, js_domain);
            if (0 != rv)
            {
                ez_log_e(TAG_TSL, "parse domain failed.");
                break;
            }
        }
    } while (false);

    if (0 != rv)
    {
        free_profile_memory(capacity);
    }

    if (NULL != js_root)
    {
        bscJSON_Delete(js_root);
    }

    return rv;
}
