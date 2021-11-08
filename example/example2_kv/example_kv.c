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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ez_iot.h"
#include "ez_iot_log.h"

#ifdef RT_THREAD
#include <rtthread.h>
#include <finsh.h>
#endif

#define KV_NODE_MAX 20
#define KV_BLOB_NAME "kv_blob_test"
#define KV_NAME_INT "kv_test_int"
#define KV_NAME_STR "kv_test_string"
#define KV_NAME_RAW "kv_test_raw"

static ez_iot_kv_t g_kvdb = {0};
static uint8_t g_inited = 0;

static int find_data_by_key(const char *key);
static int find_freespace(void);

static ez_iot_kv_node_t default_kv_set[] = {
    {"iap_need_copy_app", "0", 1},
    {"iap_need_crc32_check", "0", 1},
    {"iap_copy_app_size", "0", 1},
    {"stop_in_bootloader", "0", 1},
};

kv_err_e example_kv_init(ez_iot_kv_t *default_kv)
{
    if (1 == g_inited)
        return ez_kv_no_err;

    if (!default_kv || !default_kv->kvs || default_kv->num > KV_NODE_MAX)
        return ez_kv_init_failed;

    /***********************************************************************************/
    // TODO(需要移植) 初始化设备数据库
    g_kvdb.kvs = (ez_iot_kv_node_t *)malloc(sizeof(ez_iot_kv_node_t) * KV_NODE_MAX);
    memset(g_kvdb.kvs, 0, sizeof(ez_iot_kv_node_t) * KV_NODE_MAX);
    /***********************************************************************************/

    /***********************************************************************************/
    // TODO(需要移植) 如果数据库中缺少默认数据，需写入
    for (size_t i = 0; i < default_kv->num; i++)
    {
        int index = find_data_by_key(default_kv->kvs[i].key);
        if (-1 != index)
            continue;

        int key_len = strlen(default_kv->kvs[i].key) + 1;
        int value_len = default_kv->kvs[i].value_len;
        g_kvdb.kvs[g_kvdb.num].key = (char *)malloc(key_len);
        g_kvdb.kvs[g_kvdb.num].value = (void *)malloc(value_len);

        strncpy(g_kvdb.kvs[g_kvdb.num].key, default_kv->kvs[i].key, key_len);
        strncpy(g_kvdb.kvs[g_kvdb.num].value, default_kv->kvs[i].value, value_len);
        g_kvdb.kvs[g_kvdb.num].value_len = value_len;
        g_kvdb.num++;
    }
    /***********************************************************************************/

    g_inited = 1;

    return ez_kv_no_err;
}

kv_err_e example_kv_raw_set(const int8_t *key, int8_t *value, uint32_t length)
{
    if (0 == g_inited)
        return ez_kv_not_init_err;

    /***********************************************************************************/
    // TODO(需要移植) 在数据库中查找数据
    int index = find_data_by_key(key);
    /***********************************************************************************/

    /***********************************************************************************/
    // TODO(需要移植) 找不到，需创建
    if (-1 == index)
    {
        index = find_freespace();
        if (-1 == index)
        {
            return ez_kv_saved_full_err;
        }

        g_kvdb.kvs[index].key = (char *)malloc(strlen(key) + 1);
        g_kvdb.kvs[index].value = (void *)malloc(length);
        g_kvdb.num++;
    }
    /***********************************************************************************/

    /***********************************************************************************/
    // TODO(需要移植) 更新数据，写入出错需return ez_kv_write_err
    if (length > g_kvdb.kvs[index].value_len)
    {
        free(g_kvdb.kvs[index].value);
        g_kvdb.kvs[index].value = (void *)malloc(length);
    }

    strncpy(g_kvdb.kvs[index].key, key, strlen(key) + 1);
    memcpy(g_kvdb.kvs[index].value, value, length);
    g_kvdb.kvs[index].value_len = length;
    /***********************************************************************************/

    return ez_kv_no_err;
}

kv_err_e example_kv_raw_get(const int8_t *key, int8_t *value, uint32_t *length)
{
    if (0 == g_inited)
        return ez_kv_not_init_err;

    /***********************************************************************************/
    // TODO(需要移植) 在数据库中查找数据
    int index = find_data_by_key(key);
    if (-1 == index)
        return ez_kv_name_err;
    /***********************************************************************************/

    /***********************************************************************************/
    // TODO(需要移植) 更新数据，读取出错需return ez_kv_read_err
    if (NULL == value)
    {
        *length = g_kvdb.kvs[index].value_len;
        return ez_kv_no_err;
    }

    if (*length < g_kvdb.kvs[index].value_len)
    {
        return ez_kv_read_err;
    }

    memcpy(value, g_kvdb.kvs[index].value, g_kvdb.kvs[index].value_len);
    *length = g_kvdb.kvs[index].value_len;
    /***********************************************************************************/

    return ez_kv_no_err;
}

kv_err_e example_kv_del(const int8_t *key)
{
    if (0 == g_inited)
        return ez_kv_not_init_err;

    /***********************************************************************************/
    // TODO(需要移植) 在数据库中查找数据
    int index = find_data_by_key(key);
    if (-1 == index)
        return ez_kv_name_err;
    /***********************************************************************************/

    /***********************************************************************************/
    // TODO(需要移植) 删除数据对象
    free(g_kvdb.kvs[index].key);
    g_kvdb.kvs[index].key = NULL;
    free(g_kvdb.kvs[index].value);
    g_kvdb.kvs[index].value = NULL;
    g_kvdb.kvs[index].value_len = 0;

    g_kvdb.num--;
    /***********************************************************************************/

    return ez_kv_no_err;
}

kv_err_e example_kv_del_by_prefix(const int8_t *key_prefix)
{
    if (0 == g_inited)
        return ez_kv_not_init_err;

    //预留接口，暂无需实现

    return ez_kv_no_err;
}

void example_kv_print(void)
{
    if (0 == g_inited)
        return;

    //预留接口，暂无需实现
}

void example_kv_deinit(void)
{
    if (0 == g_inited)
        return;

    /***********************************************************************************/
    // TODO(需要移植) 反初始化数据库对象
    for (int i = 0; i < g_kvdb.num; i++)
    {
        free(g_kvdb.kvs[i].key);
        free(g_kvdb.kvs[i].value);
    }

    free(g_kvdb.kvs);
    g_inited = 0;
    /***********************************************************************************/
}

static int find_data_by_key(const char *key)
{
    int index = -1;

    for (int i = 0; i < g_kvdb.num; i++)
    {
        if (0 == strcmp(key, g_kvdb.kvs[i].key))
        {
            index = i;
            break;
        }
    }

    return index;
}

static int find_freespace(void)
{
    int index = -1;

    for (int i = 0; i < KV_NODE_MAX; i++)
    {
        if (!g_kvdb.kvs[i].key)
        {
            index = i;
            break;
        }
    }

    return index;
}

int example_kv(int argc, char **argv)
{
    int rv = 0;

    ez_iot_kv_t default_kv;
    default_kv.kvs = default_kv_set;
    default_kv.num = sizeof(default_kv_set) / sizeof(ez_iot_kv_node_t);

    example_kv_init(&default_kv);

    /**
     * @brief Test int
     */
    {
        int isrc = 4;
        int idst = 0;
        int length = sizeof(idst);
        example_kv_raw_set(KV_NAME_INT, &isrc, sizeof(isrc));
        example_kv_raw_get(KV_NAME_INT, &idst, &length);
        if (isrc != idst)
        {
            rv = -1;
            ez_log_e(TAG_APP, "kv int test err");
        }
    }

    /**
     * @brief Test string
     */
    {
        char *ssrc = "ezapp, easy your life!";
        char sdst[24] = {0};
        int length = strlen(ssrc);
        example_kv_raw_set(KV_NAME_STR, ssrc, strlen(ssrc));
        example_kv_raw_get(KV_NAME_STR, sdst, &length);
        if (0 != strcmp(ssrc, sdst))
        {
            rv = -1;
            ez_log_e(TAG_APP, "kv string test err");
        }
    }

    /**
     * @brief Test Raw
     */
    {
        char rsrc[24] = {0x00, 0x11, 0x22, 0x33, 0x00, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0xff, 0xee};
        char rdst[24] = {0};
        int length = sizeof(rsrc);
        example_kv_raw_set(KV_NAME_STR, rsrc, length);
        example_kv_raw_get(KV_NAME_STR, rdst, &length);
        if (0 != memcmp(rsrc, rdst, sizeof(rsrc)))
        {
            rv = -1;
            ez_log_e(TAG_APP, "kv raw test err");
        }
    }

    example_kv_deinit();

    if (0 == rv)
    {
        ez_log_d(TAG_APP, "example kv test succ");
    }

    return 0;
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_kv, run ez - iot - sdk example kv mock);
#else
// int main(int argc, char **argv)
// {
//     return example_kv(argc, argv);
// }
#endif