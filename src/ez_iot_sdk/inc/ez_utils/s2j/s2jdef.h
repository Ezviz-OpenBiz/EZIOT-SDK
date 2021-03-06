/*
 * This file is part of the struct2json Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: It is an head file for this library.
 * Created on: 2015-10-14
 */

#ifndef __S2JDEF_H__
#define __S2JDEF_H__

#include "cjson/bscJSON.h"
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        void *(*malloc_fn)(size_t sz);
        void (*free_fn)(void *ptr);
    } S2jHook, *S2jHook_t;

#define S2J_STRUCT_GET_int_ELEMENT(to_struct, from_json, _element) \
    json_temp = bscJSON_GetObjectItem(from_json, #_element);         \
    if (json_temp)                                                 \
        (to_struct)->_element = json_temp->valueint;

#define S2J_STRUCT_GET_int_ELEMENT_EX(to_struct, from_json, _element, _defval) \
    {                                                                          \
        if (from_json)                                                         \
        {                                                                      \
            json_temp = bscJSON_GetObjectItem(from_json, #_element);             \
            if (json_temp)                                                     \
                (to_struct)->_element = json_temp->valueint;                   \
            else                                                               \
                (to_struct)->_element = _defval;                               \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            (to_struct)->_element = _defval;                                   \
        }                                                                      \
    }

#define S2J_STRUCT_GET_string_ELEMENT(to_struct, from_json, _element) \
    json_temp = bscJSON_GetObjectItem(from_json, #_element);            \
    if (json_temp)                                                    \
        strncpy((to_struct)->_element, json_temp->valuestring, sizeof((to_struct)->_element) - 1);

#define S2J_STRUCT_GET_string_ELEMENT_EX(to_struct, from_json, _element, _defval)                          \
    {                                                                                                      \
        if (from_json)                                                                                     \
        {                                                                                                  \
            json_temp = bscJSON_GetObjectItem(from_json, #_element);                                         \
            if (json_temp)                                                                                 \
                strncpy((to_struct)->_element, json_temp->valuestring, sizeof((to_struct)->_element) - 1); \
            else                                                                                           \
                strncpy((to_struct)->_element, _defval, sizeof((to_struct)->_element) - 1);                \
        }                                                                                                  \
        else                                                                                               \
        {                                                                                                  \
            strncpy((to_struct)->_element, _defval, sizeof((to_struct)->_element) - 1);                    \
        }                                                                                                  \
    }

#define S2J_STRUCT_GET_double_ELEMENT(to_struct, from_json, _element) \
    json_temp = bscJSON_GetObjectItem(from_json, #_element);            \
    if (json_temp)                                                    \
        (to_struct)->_element = json_temp->valuedouble;

#define S2J_STRUCT_GET_double_ELEMENT_EX(to_struct, from_json, _element, _defval) \
    {                                                                             \
        if (from_json)                                                            \
        {                                                                         \
            json_temp = bscJSON_GetObjectItem(from_json, #_element);                \
            if (json_temp)                                                        \
                (to_struct)->_element = json_temp->valuedouble;                   \
            else                                                                  \
                (to_struct)->_element = _defval;                                  \
        }                                                                         \
        else                                                                      \
        {                                                                         \
            (to_struct)->_element = _defval;                                      \
        }                                                                         \
    }

#define S2J_STRUCT_ARRAY_GET_int_ELEMENT(to_struct, from_json, _element, index) \
    (to_struct)->_element[index] = from_json->valueint;

#define S2J_STRUCT_ARRAY_GET_string_ELEMENT(to_struct, from_json, _element, index) \
    strncpy((to_struct)->_element[index], from_json->valuestring, sizeof((to_struct)->_element) - 1);

#define S2J_STRUCT_ARRAY_GET_double_ELEMENT(to_struct, from_json, _element, index) \
    (to_struct)->_element[index] = from_json->valuedouble;

#define S2J_STRUCT_ARRAY_GET_ELEMENT(to_struct, from_json, type, _element, index) \
    S2J_STRUCT_ARRAY_GET_##type##_ELEMENT(to_struct, from_json, _element, index)

#define S2J_STRUCT_ARRAY_GET_int_ELEMENT_EX(to_struct, from_json, _element, index, _defval) \
    if (from_json)                                                                          \
        (to_struct)->_element[index] = from_json->valueint;                                 \
    else                                                                                    \
        (to_struct)->_element[index] = _defval;

#define S2J_STRUCT_ARRAY_GET_string_ELEMENT_EX(to_struct, from_json, _element, index, _defval)            \
    if (from_json)                                                                                        \
        strncpy((to_struct)->_element[index], from_json->valuestring, sizeof((to_struct)->_element) - 1); \
    else                                                                                                  \
        strncpy((to_struct)->_element[index], _defval, sizeof((to_struct)->_element) - 1);

#define S2J_STRUCT_ARRAY_GET_double_ELEMENT_EX(to_struct, from_json, _element, index) \
    if (from_json)                                                                    \
        (to_struct)->_element[index] = from_json->valuedouble;                        \
    else                                                                              \
        (to_struct)->_element[index] = _defval;

#define S2J_STRUCT_ARRAY_GET_ELEMENT_EX(to_struct, from_json, type, _element, index, _defval) \
    S2J_STRUCT_ARRAY_GET_##type##_ELEMENT_EX(to_struct, from_json, _element, index, _defval)

#define S2J_JSON_SET_Bool_ELEMENT(to_json, from_struct, _element) \
    bscJSON_AddBoolToObject(to_json, #_element, (from_struct)->_element);
    
#define S2J_JSON_SET_int_ELEMENT(to_json, from_struct, _element) \
    bscJSON_AddNumberToObject(to_json, #_element, (from_struct)->_element);

#define S2J_JSON_SET_double_ELEMENT(to_json, from_struct, _element) \
    bscJSON_AddNumberToObject(to_json, #_element, (from_struct)->_element);

#define S2J_JSON_SET_string_ELEMENT(to_json, from_struct, _element) \
    bscJSON_AddStringToObject(to_json, #_element, (char *)(from_struct)->_element);

#define S2J_JSON_ARRAY_SET_int_ELEMENT(to_json, from_struct, _element, index) \
    bscJSON_AddItemToArray(to_json, bscJSON_CreateNumber((from_struct)->_element[index]));

#define S2J_JSON_ARRAY_SET_double_ELEMENT(to_json, from_struct, _element, index) \
    bscJSON_AddItemToArray(to_json, bscJSON_CreateNumber((from_struct)->_element[index]));

#define S2J_JSON_ARRAY_SET_string_ELEMENT(to_json, from_struct, _element, index) \
    bscJSON_AddItemToArray(to_json, bscJSON_CreateString((from_struct)->_element[index]));

#define S2J_JSON_ARRAY_SET_ELEMENT(to_json, from_struct, type, _element, index) \
    S2J_JSON_ARRAY_SET_##type##_ELEMENT(to_json, from_struct, _element, index)

#define S2J_CREATE_JSON_OBJECT(json_obj) \
    bscJSON *json_obj = bscJSON_CreateObject();

#define S2J_DELETE_JSON_OBJECT(json_obj) \
    bscJSON_Delete(json_obj);

#define S2J_JSON_SET_BASIC_ELEMENT(to_json, from_struct, type, _element) \
    S2J_JSON_SET_##type##_ELEMENT(to_json, from_struct, _element)

#define S2J_JSON_SET_ARRAY_ELEMENT(to_json, from_struct, type, _element, size)           \
    {                                                                                    \
        bscJSON *array;                                                                    \
        size_t index = 0;                                                                \
        array = bscJSON_CreateArray();                                                     \
        if (array)                                                                       \
        {                                                                                \
            while (index < size)                                                         \
            {                                                                            \
                S2J_JSON_ARRAY_SET_ELEMENT(array, from_struct, type, _element, index++); \
            }                                                                            \
            bscJSON_AddItemToObject(to_json, #_element, array);                            \
        }                                                                                \
    }

#define S2J_JSON_SET_STRUCT_ELEMENT(child_json, to_json, child_struct, from_struct, type, _element) \
    type *child_struct = &((from_struct)->_element);                                                \
    bscJSON *child_json = bscJSON_CreateObject();                                                       \
    if (child_json)                                                                                 \
        bscJSON_AddItemToObject(to_json, #_element, child_json);

#define S2J_CREATE_STRUCT_OBJECT(struct_obj, type)      \
    bscJSON *json_temp;                                   \
    type *struct_obj = s2jHook.malloc_fn(sizeof(type)); \
    if (struct_obj)                                     \
        memset(struct_obj, 0, sizeof(type));

#define S2J_DELETE_STRUCT_OBJECT(struct_obj) \
    s2jHook.free_fn(struct_obj);

#define S2J_STRUCT_GET_BASIC_ELEMENT(to_struct, from_json, type, _element) \
    S2J_STRUCT_GET_##type##_ELEMENT(to_struct, from_json, _element)

#define S2J_STRUCT_GET_BASIC_ELEMENT_EX(to_struct, from_json, type, _element, _defval) \
    S2J_STRUCT_GET_##type##_ELEMENT_EX(to_struct, from_json, _element, _defval)

#define S2J_STRUCT_GET_ARRAY_ELEMENT(to_struct, from_json, type, _element)                           \
    {                                                                                                \
        bscJSON *array, *array_element;                                                                \
        size_t index = 0, size = 0;                                                                  \
        array = bscJSON_GetObjectItem(from_json, #_element);                                           \
        if (array)                                                                                   \
        {                                                                                            \
            size = bscJSON_GetArraySize(array);                                                        \
            while (index < size)                                                                     \
            {                                                                                        \
                array_element = bscJSON_GetArrayItem(array, index);                                    \
                if (array_element)                                                                   \
                    S2J_STRUCT_ARRAY_GET_ELEMENT(to_struct, array_element, type, _element, index++); \
            }                                                                                        \
        }                                                                                            \
    }

#define S2J_STRUCT_GET_ARRAY_ELEMENT_EX(to_struct, from_json, type, _element, size, _defval)                     \
    {                                                                                                            \
        size_t index = 0, realsize = 0;                                                                          \
        if (from_json)                                                                                           \
        {                                                                                                        \
            bscJSON *array = NULL, *array_element = NULL;                                                          \
            array = bscJSON_GetObjectItem(from_json, #_element);                                                   \
            if (array)                                                                                           \
            {                                                                                                    \
                realsize = bscJSON_GetArraySize(array);                                                            \
                while (index < realsize)                                                                         \
                {                                                                                                \
                    array_element = bscJSON_GetArrayItem(array, index);                                            \
                    S2J_STRUCT_ARRAY_GET_ELEMENT_EX(to_struct, array_element, type, _element, index++, _defval); \
                }                                                                                                \
            }                                                                                                    \
            else                                                                                                 \
            {                                                                                                    \
                while (index < size)                                                                             \
                {                                                                                                \
                    S2J_STRUCT_ARRAY_GET_ELEMENT_EX(to_struct, array_element, type, _element, index++, _defval); \
                }                                                                                                \
            }                                                                                                    \
        }                                                                                                        \
        else                                                                                                     \
        {                                                                                                        \
            while (index < size)                                                                                 \
            {                                                                                                    \
                S2J_STRUCT_ARRAY_GET_ELEMENT_EX(to_struct, from_json, type, _element, index++, _defval);         \
            }                                                                                                    \
        }                                                                                                        \
    }

#define S2J_STRUCT_GET_STRUCT_ELEMENT(child_struct, to_struct, child_json, from_json, type, _element) \
    type *child_struct = &((to_struct)->_element);                                                    \
    bscJSON *child_json = bscJSON_GetObjectItem(from_json, #_element);

#ifdef __cplusplus
}
#endif

#endif /* __S2JDEF_H__ */
