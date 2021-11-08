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
#include "kv_imp.h"
#include "utest.h"
#include "FreeRTOS.h"
#include "semphr.h"

static struct fdb_kvdb ez_kvdb;
static SemaphoreHandle_t s_lock = NULL;
#define EZ_KVDB_NAME "data"
#define EZ_KVDB_PART_NAME "ez_kvdb"
#define EZ_KVDB_MAX_SIZE 1024 * 16
#define EZ_KVDB_SEC_SIZE 1024 * 8

void kv_lock(fdb_db_t db)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
}

void kv_unlock(fdb_db_t db)
{
    xSemaphoreGive(s_lock);
}

int32_t kv_init_adv(fdb_kvdb_t db, const char *name, const char *part_name, struct fdb_default_kv *default_kv, int32_t max_size, int32_t sec_size)
{
    if (db->parent.init_ok)
    {
        return 0;
    }

    if (s_lock == NULL) {
        s_lock = xSemaphoreCreateCounting(1, 1);
        assert(s_lock != NULL);
    }

    fdb_kvdb_control(db, FDB_KVDB_CTRL_SET_LOCK, (void *)kv_lock);
    fdb_kvdb_control(db, FDB_KVDB_CTRL_SET_UNLOCK, (void *)kv_unlock);
    fdb_kvdb_control(db, FDB_KVDB_CTRL_SET_MAX_SIZE, (void *)&max_size);
    fdb_kvdb_control(db, FDB_KVDB_CTRL_SET_SEC_SIZE, (void *)&sec_size);

    return fdb_kvdb_init(db, name, part_name, default_kv, NULL);
}

kv_err_e kv_init(ez_iot_kv_t *default_kv)
{
    /**
     * @brief For single product devices, 16k is recommended, and for gateway devices,
     *          16*n is recommended, where n is the number of sub-devices.
     */
    int32_t max_size = EZ_KVDB_MAX_SIZE;
    int32_t sec_size = EZ_KVDB_SEC_SIZE;

    if (ez_kvdb.parent.init_ok)
    {
        return ez_kv_no_err;
    }

    if (s_lock == NULL) {
        s_lock = xSemaphoreCreateCounting(1, 1);
        assert(s_lock != NULL);
    }

    fdb_kvdb_control(&ez_kvdb, FDB_KVDB_CTRL_SET_LOCK, (void *)kv_lock);
    fdb_kvdb_control(&ez_kvdb, FDB_KVDB_CTRL_SET_UNLOCK, (void *)kv_unlock);
    fdb_kvdb_control(&ez_kvdb, FDB_KVDB_CTRL_SET_MAX_SIZE, (void *)&max_size);
    fdb_kvdb_control(&ez_kvdb, FDB_KVDB_CTRL_SET_SEC_SIZE, (void *)&sec_size);

    fdb_kvdb_init(&ez_kvdb, EZ_KVDB_NAME, EZ_KVDB_PART_NAME, (struct fdb_default_kv *)default_kv, NULL);

    return ez_kv_no_err;
}

kv_err_e kv_raw_set(const int8_t *key, int8_t *value, uint32_t length)
{
    struct fdb_blob blob;

    return fdb_kv_set_blob(&ez_kvdb, (const char *)key, fdb_blob_make(&blob, value, length));
}

kv_err_e kv_raw_get(const int8_t *key, int8_t *value, uint32_t *length)
{
    struct fdb_blob blob;

    size_t read_len = fdb_kv_get_blob(&ez_kvdb, (const char *)key, fdb_blob_make(&blob, value, *length));
    if (NULL == value)
    {
        *length = read_len;
        return ez_kv_no_err;
    }

    *length = read_len;
    return ez_kv_no_err;
}

kv_err_e kv_del(const int8_t *key)
{
    return fdb_kv_del(&ez_kvdb, (const char *)key);
}

kv_err_e kv_del_by_prefix(const int8_t *key_prefix)
{
    //TODO
    return ez_kv_no_err;
}

void kv_print(void)
{
    fdb_kv_print(&ez_kvdb);
}

void kv_deinit(void)
{
    //do nothing
}