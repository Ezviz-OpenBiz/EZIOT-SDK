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

#include <stdlib.h>
#include <flashdb.h>
#include <string.h>
#include "ut_config.h"
#include "kv_imp.h"
#include "utest.h"
#include "fdb_cfg.h"

void ut_kvdb_init10M();
void ut_kvdb_default();
void ut_kvdb_string();
void ut_kvdb_blob_int();
void ut_kvdb_blob_raw4k();
void ut_kvdb_blob_raw8K();
void ut_kvdb_blob_raw16k_less();

// UTEST_TC_EXPORT(ut_kvdb_init10M, NULL, NULL, DEFAULT_TIMEOUT_S);
// UTEST_TC_EXPORT(ut_kvdb_default, NULL, NULL, DEFAULT_TIMEOUT_S);
// UTEST_TC_EXPORT(ut_kvdb_string, NULL, NULL, DEFAULT_TIMEOUT_S);
// UTEST_TC_EXPORT(ut_kvdb_blob_int, NULL, NULL, DEFAULT_TIMEOUT_S);
// UTEST_TC_EXPORT(ut_kvdb_blob_raw4k, NULL, NULL, DEFAULT_TIMEOUT_S);
// UTEST_TC_EXPORT(ut_kvdb_blob_raw8K, NULL, NULL, DEFAULT_TIMEOUT_S);
// UTEST_TC_EXPORT(ut_kvdb_blob_raw16k_less, NULL, NULL, DEFAULT_TIMEOUT_S);

#define UT_KVDB_NAME "data"
#define UT_KVDB_PART_NAME "ez_kvdb"

static uint32_t boot_count = 1;
// static char raw_16k[UT_SEC_SIZE] = {0};
// static char raw_16k2[UT_SEC_SIZE] = {0};

static struct fdb_default_kv_node default_kv_table[] = {
    {"boot_count", &boot_count, sizeof(boot_count)}, /* int type KV */
};

// void ut_kvdb_init10M()
// {
//   int32_t max_size = 1024 * 1024 * 10;
//   int32_t sec_size = 1024 * 1024;
//   int boot_count = 0;

//   struct fdb_default_kv default_kv = {.kvs = default_kv_table, .num = sizeof(default_kv_table) / sizeof(struct fdb_default_kv_node)};
//   struct fdb_kvdb kvdb = {0};

//   uassert_int_equal(FDB_NO_ERR, kv_init_adv(&kvdb, UT_KVDB_NAME, UT_KVDB_PART_NAME, &default_kv, max_size, sec_size));
//   uassert_int_equal(FDB_NO_ERR, fdb_kv_set_default(&kvdb));
// }

void ut_kvdb_default()
{
  int32_t max_size = UT_MAX_SIZE;
  int32_t sec_size = UT_SEC_SIZE;
  struct fdb_blob blob;
  struct fdb_default_kv default_kv = {.kvs = default_kv_table, .num = sizeof(default_kv_table) / sizeof(struct fdb_default_kv_node)};
  struct fdb_kvdb kvdb = {0};
  int boot_count = 0;

  uassert_int_equal(FDB_NO_ERR, kv_init_adv(&kvdb, UT_KVDB_NAME, UT_KVDB_PART_NAME, &default_kv, max_size, sec_size));
  uassert_int_equal(4, fdb_kv_get_blob(&kvdb, "boot_count", fdb_blob_make(&blob, &boot_count, sizeof(boot_count))));
  boot_count++;
  uassert_int_equal(FDB_NO_ERR, fdb_kv_set_blob(&kvdb, "boot_count", fdb_blob_make(&blob, &boot_count, sizeof(boot_count))));
}

void ut_kvdb_string()
{
  int32_t max_size = UT_MAX_SIZE;
  int32_t sec_size = UT_SEC_SIZE;
  struct fdb_blob blob;
  struct fdb_default_kv default_kv = {.kvs = default_kv_table, .num = sizeof(default_kv_table) / sizeof(struct fdb_default_kv_node)};
  struct fdb_kvdb kvdb = {0};
  char temp_data[10] = "36C";
  char *temp_data2;

  uassert_int_equal(FDB_NO_ERR, kv_init_adv(&kvdb, UT_KVDB_NAME, UT_KVDB_PART_NAME, &default_kv, max_size, sec_size));
  uassert_int_equal(FDB_NO_ERR, fdb_kv_set(&kvdb, "temp_str", temp_data));
  uassert_buf_equal(temp_data, temp_data2 = fdb_kv_get(&kvdb, "temp_str"), strlen(temp_data));
  uassert_int_equal(FDB_NO_ERR, fdb_kv_del(&kvdb, "temp_str"));
}

void ut_kvdb_blob_int()
{
  int32_t max_size = UT_MAX_SIZE;
  int32_t sec_size = UT_SEC_SIZE;
  struct fdb_blob blob;
  struct fdb_default_kv default_kv = {.kvs = default_kv_table, .num = sizeof(default_kv_table) / sizeof(struct fdb_default_kv_node)};
  struct fdb_kvdb kvdb = {0};
  int temp_data = 36;
  int temp_data2 = 0;

  uassert_int_equal(FDB_NO_ERR, kv_init_adv(&kvdb, UT_KVDB_NAME, UT_KVDB_PART_NAME, &default_kv, max_size, sec_size));
  uassert_int_equal(FDB_NO_ERR, fdb_kv_set_blob(&kvdb, "temp", fdb_blob_make(&blob, &temp_data, sizeof(temp_data))));
  uassert_int_equal(sizeof(temp_data2), fdb_kv_get_blob(&kvdb, "temp", fdb_blob_make(&blob, &temp_data2, sizeof(temp_data2))));
  uassert_int_equal(FDB_NO_ERR, fdb_kv_del(&kvdb, "temp"));
}

// void ut_kvdb_blob_raw4k()
// {
//   int32_t max_size = UT_MAX_SIZE;
//   int32_t sec_size = UT_SEC_SIZE;
//   struct fdb_blob blob;
//   struct fdb_default_kv default_kv = {.kvs = default_kv_table, .num = sizeof(default_kv_table) / sizeof(struct fdb_default_kv_node)};
//   struct fdb_kvdb kvdb = {0};
//   const size_t raw_len = 1024 * 4;

//   for (size_t i = 0; i < raw_len; i++)
//   {
//     raw_16k[i] = rand();
//   }

//   uassert_int_equal(FDB_NO_ERR, kv_init_adv(&kvdb, UT_KVDB_NAME, UT_KVDB_PART_NAME, &default_kv, max_size, sec_size));
//   uassert_int_equal(FDB_NO_ERR, fdb_kv_set_blob(&kvdb, "temp", fdb_blob_make(&blob, raw_16k, raw_len)));
//   uassert_int_equal(raw_len, fdb_kv_get_blob(&kvdb, "temp", fdb_blob_make(&blob, raw_16k2, raw_len)));
//   uassert_buf_equal(raw_16k, raw_16k2, raw_len);
//   uassert_int_equal(FDB_NO_ERR, fdb_kv_del(&kvdb, "temp"));
// }

// void ut_kvdb_blob_raw8K()
// {
//   int32_t max_size = UT_MAX_SIZE;
//   int32_t sec_size = UT_SEC_SIZE;
//   struct fdb_blob blob;
//   struct fdb_default_kv default_kv = {.kvs = default_kv_table, .num = sizeof(default_kv_table) / sizeof(struct fdb_default_kv_node)};
//   struct fdb_kvdb kvdb = {0};
//   const size_t raw_len = 1024 * 8;

//   for (size_t i = 0; i < raw_len; i++)
//   {
//     raw_16k[i] = rand();
//   }

//   uassert_int_equal(FDB_NO_ERR, kv_init_adv(&kvdb, UT_KVDB_NAME, UT_KVDB_PART_NAME, &default_kv, max_size, sec_size));
//   uassert_int_equal(FDB_NO_ERR, fdb_kv_set_blob(&kvdb, "temp", fdb_blob_make(&blob, raw_16k, raw_len)));
//   uassert_int_equal(raw_len, fdb_kv_get_blob(&kvdb, "temp", fdb_blob_make(&blob, raw_16k2, raw_len)));
//   uassert_buf_equal(raw_16k, raw_16k2, raw_len);
//   uassert_int_equal(FDB_NO_ERR, fdb_kv_del(&kvdb, "temp"));
// }

// void ut_kvdb_blob_raw16k_less()
// {
//   int32_t max_size = UT_MAX_SIZE;
//   int32_t sec_size = UT_SEC_SIZE;
//   struct fdb_blob blob;
//   struct fdb_default_kv default_kv = {.kvs = default_kv_table, .num = sizeof(default_kv_table) / sizeof(struct fdb_default_kv_node)};
//   struct fdb_kvdb kvdb = {0};
//   const size_t raw_len = UT_SEC_SIZE - 512;

//   for (size_t i = 0; i < raw_len; i++)
//   {
//     raw_16k[i] = rand();
//   }

//   uassert_int_equal(FDB_NO_ERR, kv_init_adv(&kvdb, UT_KVDB_NAME, UT_KVDB_PART_NAME, &default_kv, max_size, sec_size));
//   uassert_int_equal(FDB_NO_ERR, fdb_kv_set_blob(&kvdb, "temp", fdb_blob_make(&blob, raw_16k, raw_len)));
//   uassert_int_equal(raw_len, fdb_kv_get_blob(&kvdb, "temp", fdb_blob_make(&blob, raw_16k2, raw_len)));
//   uassert_buf_equal(raw_16k, raw_16k2, raw_len);
//   uassert_int_equal(FDB_NO_ERR, fdb_kv_del(&kvdb, "temp"));
// }