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
 *2021-03-11      xurongjun
 *******************************************************************************/

#include "flashdb.h"
#include <stdint.h>

void kv_lock(fdb_db_t db);

void kv_unlock(fdb_db_t db);

int32_t kv_init_adv(fdb_kvdb_t db, const char *name, const char *part_name, struct fdb_default_kv *default_kv, int32_t max_size, int32_t sec_size);

#include <ez_iot.h>

kv_err_e kv_init(ez_iot_kv_t *default_kv);

kv_err_e kv_raw_set(const int8_t *key, int8_t *value, uint32_t length);

kv_err_e kv_raw_get(const int8_t *key, int8_t *value, uint32_t *length);

kv_err_e kv_del(const int8_t *key);

kv_err_e kv_del_by_prefix(const int8_t *key_prefix);

kv_err_e kv_erase(void);

void kv_print(void);

void kv_deinit(void);
