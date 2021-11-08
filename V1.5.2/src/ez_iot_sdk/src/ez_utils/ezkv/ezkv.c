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
#include "ezkv.h"

#define KV_BUF_SIZE_DEFAULT 1024 * 8
#define CHECK_NULL_RETURN(p, rv) \
    if (NULL == p)               \
    {                            \
        return (rv);             \
    }

static ez_iot_kv_callbacks_t *m_kv_cb = NULL;

static ez_iot_kv_t m_default_kv;
static ez_iot_kv_node_t m_default_kv_table[] = {
    {EZ_BASIC_KEY_DEVID, "", 0},     /* basic kv */
    {EZ_BASIC_KEY_MASTERKEY, "", 0}, /* basic kv */
    {EZ_BASIC_KEY_USERID, "", 0},    /* basic kv */
    {EZ_TSL_KEY_TSLMAP, "", 0},      /* tslmap profile map */
    {EZ_HUB_KEY_HUBLIST, "", 0},     /* hub module sublist */
};

bool ez_kv_callback_set(ez_iot_kv_callbacks_t *kv_cb)
{
    CHECK_NULL_RETURN(kv_cb, false);
    CHECK_NULL_RETURN(kv_cb->ez_kv_init, false);
    CHECK_NULL_RETURN(kv_cb->ez_kv_raw_set, false);
    CHECK_NULL_RETURN(kv_cb->ez_kv_raw_get, false);
    CHECK_NULL_RETURN(kv_cb->ez_kv_del, false);
    CHECK_NULL_RETURN(kv_cb->ez_kv_del_by_prefix, false);
    CHECK_NULL_RETURN(kv_cb->ez_kv_print, false);
    CHECK_NULL_RETURN(kv_cb->ez_kv_deinit, false);

    m_kv_cb = kv_cb;

    return true;
}

kv_err_e ez_kv_init()
{
    CHECK_NULL_RETURN(m_kv_cb, ez_kv_init_failed);
    CHECK_NULL_RETURN(m_kv_cb->ez_kv_init, ez_kv_init_failed);
    m_default_kv.kvs = m_default_kv_table;
    m_default_kv.num = sizeof(m_default_kv_table) / sizeof(ez_iot_kv_node_t);

    return m_kv_cb->ez_kv_init(&m_default_kv);
}

kv_err_e ez_kv_raw_set(const int8_t *key, int8_t *value, uint32_t length)
{
    CHECK_NULL_RETURN(m_kv_cb, ez_kv_not_init_err);
    CHECK_NULL_RETURN(m_kv_cb->ez_kv_raw_set, ez_kv_not_init_err);

    return m_kv_cb->ez_kv_raw_set(key, value, length);
}

kv_err_e ez_kv_raw_get(const int8_t *key, int8_t *value, uint32_t *length)
{
    CHECK_NULL_RETURN(m_kv_cb, ez_kv_not_init_err);
    CHECK_NULL_RETURN(m_kv_cb->ez_kv_raw_get, ez_kv_not_init_err);
    CHECK_NULL_RETURN(length, ez_kv_read_err);

    if (NULL == value && 0 == *length)
    {
        *length = KV_BUF_SIZE_DEFAULT;
    }

    return m_kv_cb->ez_kv_raw_get(key, value, length);
}

kv_err_e ez_kv_del(const int8_t *key)
{
    CHECK_NULL_RETURN(m_kv_cb, ez_kv_not_init_err);
    CHECK_NULL_RETURN(m_kv_cb->ez_kv_del, ez_kv_not_init_err);

    return m_kv_cb->ez_kv_del(key);
}

kv_err_e ez_kv_del_by_prefix(const int8_t *key_prefix)
{
    CHECK_NULL_RETURN(m_kv_cb, ez_kv_not_init_err);
    CHECK_NULL_RETURN(m_kv_cb->ez_kv_del_by_prefix, ez_kv_not_init_err);

    return m_kv_cb->ez_kv_del_by_prefix(key_prefix);
}

void ez_kv_print(void)
{
    if (!m_kv_cb || !m_kv_cb->ez_kv_print)
    {
        return;
    }

    m_kv_cb->ez_kv_print();
}

void ez_kv_deinit(void)
{
    if (!m_kv_cb || !m_kv_cb->ez_kv_print)
    {
        return;
    }

    m_kv_cb->ez_kv_deinit();
}