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

#include <stdlib.h>
#include "flashdb.h"
#include <string.h>
#include "ut_config.h"
#include "ez_iot.h"
#include "ez_iot_log.h"
#include "ez_hal/hal_thread.h"
#include "kv_imp.h"
#include "utest.h"

/**
 * @brief Test interface compatibility and error codes
 * 
 */
void ut_online_errcode();
void ut_online_access();
void ut_online_restart();

UTEST_TC_EXPORT(ut_online_errcode, NULL, NULL, DEFAULT_TIMEOUT_S);
UTEST_TC_EXPORT(ut_online_access, NULL, NULL, DEFAULT_TIMEOUT_S);
UTEST_TC_EXPORT(ut_online_restart, NULL, NULL, DEFAULT_TIMEOUT_S);

static int32_t ez_recv_msg_cb(ez_iot_cloud2dev_msg_type_t msg_type, void *data, int len);
static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len);
static int dev_event_waitfor(int event_id, int time_ms);

static int m_event_id = -1;
static void *m_event_ctx = NULL;
static int m_last_err = 0;
static ez_iot_srv_info_t m_lbs_addr = {(int8_t *)EZ_IOT_CLOUD_ENTRY_HOST, EZ_IOT_CLOUD_ENTRY_PORT};
static ez_iot_callbacks_t m_cbs = {ez_recv_msg_cb, ez_recv_event_cb};
static ez_iot_dev_info_t m_dev_info = {
    .auth_mode = EZ_IOT_DEV_AUTH_MODE,
    .dev_subserial = EZ_IOT_DEV_UUID,
    .dev_verification_code = EZ_IOT_DEV_LICENSE,
    .dev_firmwareversion = EZ_IOT_DEV_FWVER,
    .dev_type = EZ_IOT_DEV_PRODUCT_KEY,
    .dev_typedisplay = EZ_IOT_DEV_DISPLAY_NAME,
    .dev_id = EZ_IOT_DEV_ID,
};

static ez_iot_kv_callbacks_t m_kv_cbs = {
    .ez_kv_init = kv_init,
    .ez_kv_raw_set = kv_raw_set,
    .ez_kv_raw_get = kv_raw_get,
    .ez_kv_del = kv_del,
    .ez_kv_del_by_prefix = kv_del_by_prefix,
    .ez_kv_print = kv_print,
    .ez_kv_deinit = kv_deinit,
};

void ut_online_errcode()
{
  ez_iot_srv_info_t lbs_addr_null = {NULL, 8666};
  ez_iot_kv_callbacks_t kv_cbs_null = {0};

  int8_t *dev_token = (int8_t *)"1111111";

  ez_iot_dev_info_t dev_info_null = {0};

  uassert_int_equal(ez_errno_not_init, ez_iot_start());
  uassert_int_equal(ez_errno_not_init, ez_iot_stop());
  uassert_int_equal(ez_errno_not_init, ez_iot_binding(dev_token));
  uassert_int_equal(ez_errno_not_init, ez_iot_set_keepalive(100));

  uassert_int_equal(ez_errno_param_invalid, ez_iot_init(NULL, NULL, NULL, NULL));
  uassert_int_equal(ez_errno_param_invalid, ez_iot_init(&m_lbs_addr, NULL, NULL, NULL));
  uassert_int_equal(ez_errno_param_invalid, ez_iot_init(&m_lbs_addr, &dev_info_null, NULL, NULL));
  uassert_int_equal(ez_errno_param_invalid, ez_iot_init(&m_lbs_addr, &dev_info_null, &m_cbs, NULL));
  uassert_int_equal(ez_errno_param_invalid, ez_iot_init(&m_lbs_addr, &dev_info_null, &m_cbs, NULL));
  uassert_int_equal(ez_errno_param_invalid, ez_iot_init(&m_lbs_addr, &dev_info_null, &m_cbs, &kv_cbs_null));
  uassert_int_equal(ez_errno_param_invalid, ez_iot_init(&lbs_addr_null, &dev_info_null, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &dev_info_null, &m_cbs, &m_kv_cbs));

  uassert_int_equal(ez_errno_not_ready, ez_iot_stop());
  printf("%d %s\r\n", __LINE__, __func__);

  uassert_int_equal(ez_errno_not_ready, ez_iot_binding(dev_token));
  printf("%d %s\r\n", __LINE__, __func__);

  uassert_int_equal(ez_errno_not_ready, ez_iot_set_keepalive(100));

  printf("%d %s\r\n", __LINE__, __func__);
  uassert_int_equal(ez_errno_succ, ez_iot_start());
  // hal_thread_sleep(30000);
  printf("%d %s\r\n", __LINE__, __func__);

  uassert_int_equal(ez_errno_succ, ez_iot_binding(dev_token));
  printf("%d %s\r\n", __LINE__, __func__);

  uassert_int_equal(ez_errno_succ, ez_iot_set_keepalive(100));
  printf("%d %s\r\n", __LINE__, __func__);

  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  printf("%d %s\r\n", __LINE__, __func__);

  // uassert_int_equal(ez_errno_not_ready, ez_iot_binding(dev_token));
  // printf("%d %s\r\n", __LINE__, __func__);

  // uassert_int_equal(ez_errno_not_ready, ez_iot_set_keepalive(100));
  // printf("%d %s\r\n", __LINE__, __func__);

  // ez_iot_fini();
  // printf("%d %s\r\n", __LINE__, __func__);

  // //re test
  // uassert_int_equal(ez_errno_not_init, ez_iot_start());
  // printf("%d %s\r\n", __LINE__, __func__);

  // uassert_int_equal(ez_errno_not_init, ez_iot_stop());
  // uassert_int_equal(ez_errno_not_init, ez_iot_binding(dev_token));
  // uassert_int_equal(ez_errno_not_init, ez_iot_set_keepalive(100));

  // uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));

  // uassert_int_equal(ez_errno_not_ready, ez_iot_stop());
  // uassert_int_equal(ez_errno_not_ready, ez_iot_binding(dev_token));
  // uassert_int_equal(ez_errno_not_ready, ez_iot_set_keepalive(100));

  // uassert_int_equal(ez_errno_succ, ez_iot_start());
  // uassert_int_equal(ez_errno_succ, ez_iot_binding(dev_token));
  // uassert_int_equal(ez_errno_succ, ez_iot_set_keepalive(100));
  // uassert_int_equal(ez_errno_succ, ez_iot_stop());
  ez_iot_fini();
}

void ut_online_access()
{
  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());
  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  ez_iot_fini();
}

void ut_online_restart()
{
  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());
  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  uassert_int_equal(ez_errno_succ, ez_iot_stop());

  uassert_int_equal(ez_errno_succ, ez_iot_start());
  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  uassert_int_equal(ez_errno_succ, ez_iot_stop());

  uassert_int_equal(ez_errno_succ, ez_iot_start());
  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  uassert_int_equal(ez_errno_succ, ez_iot_stop());

  ez_iot_fini();
}

static int dev_event_waitfor(int event_id, int time_ms)
{
  int ret = -1;
  int index = 0;
  m_event_id = -1;
  m_last_err = 0;

  do
  {
    if (event_id == m_event_id)
    {
      m_last_err = 0;
      ret = 0;
      break;
    }

    hal_thread_sleep(1);
  } while (++index < time_ms);

  return ret;
}

static int32_t ez_recv_msg_cb(ez_iot_cloud2dev_msg_type_t msg_type, void *data, int len)
{
  return 0;
}

static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len)
{
  char file_path[128] = {0};

  switch (event_type)
  {
  case ez_iot_event_online:
    m_event_id = ez_iot_event_online;
    break;
  case ez_iot_event_devid_update:
    /* save devid */
    break;

  default:
    break;
  }

  return 0;
}

// /**
//  * @brief 缓存的devid、masterkey是否有效
//  *
//  */
// TEST(mk_online, cache_valid)
// {
//   ezDevSDK_all_config allconfig = {0};
//   showkey_info showkey_info[2] = {0};
//   int count = 1;

//   sprintf(allconfig.config.devinfo_path, "./%s/%s", dir_cache_name, file_devinfo_name);
//   sprintf(allconfig.config.dev_id, "./%s/%s", dir_cache_name, file_devid_name);
//   sprintf(allconfig.config.dev_masterkey, "./%s/%s", dir_cache_name, file_masterkey_name);
//   allconfig.config.keyValueLoadFun = key_value_load_cb;
//   allconfig.config.keyValueSaveFun = key_value_save_cb;
//   allconfig.config.curingDataLoadFun = curing_data_load_cb;
//   allconfig.config.curingDataSaveFun = curing_data_save_cb;
//   allconfig.notice.event_notice = event_notice_cb;
//   allconfig.notice.log_notice = log_notice_cb;

//   EXPECT_TRUE(clearCache());
//   EXPECT_TRUE(makeCache());
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Init(getCfgSingleton().getItemString(CCfgLocal::lbs_domain), getCfgSingleton().getItemInt(CCfgLocal::lbs_port), &allconfig, 1));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Start());
//   EXPECT_EQ(0, dev_event_waitfor(ezDevSDK_App_Event_Online, 10 * 1000, NULL));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_id));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_masterkey));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezdev_sdk_kernel_show_key_info(&showkey_info[0]));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Stop());
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Fini());

//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Init(getCfgSingleton().getItemString(CCfgLocal::lbs_domain), getCfgSingleton().getItemInt(CCfgLocal::lbs_port), &allconfig, 1));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Start());
//   EXPECT_EQ(0, dev_event_waitfor(ezDevSDK_App_Event_Online, 10 * 1000, NULL));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_id));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_masterkey));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezdev_sdk_kernel_show_key_info(&showkey_info[1]));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Stop());
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Fini());
//   EXPECT_EQ(0, strcmp((const char *)showkey_info[0].dev_id, (const char *)showkey_info[1].dev_id));
//   EXPECT_EQ(0, strcmp((const char *)showkey_info[0].master_key, (const char *)showkey_info[1].master_key));
// }

// /**
//  * @brief 本地无devid
//  *
//  */
// TEST(mk_online, nodevid)
// {
//   ezDevSDK_all_config allconfig = {0};

//   sprintf(allconfig.config.devinfo_path, "./%s/%s", dir_cache_name, file_devinfo_name);
//   sprintf(allconfig.config.dev_id, "./%s/%s", dir_cache_name, file_devid_name);
//   sprintf(allconfig.config.dev_masterkey, "./%s/%s", dir_cache_name, file_masterkey_name);
//   allconfig.config.keyValueLoadFun = key_value_load_cb;
//   allconfig.config.keyValueSaveFun = key_value_save_cb;
//   allconfig.config.curingDataLoadFun = curing_data_load_cb;
//   allconfig.config.curingDataSaveFun = curing_data_save_cb;
//   allconfig.notice.event_notice = event_notice_cb;
//   allconfig.notice.log_notice = log_notice_cb;

//   EXPECT_TRUE(clearCache());
//   EXPECT_TRUE(makeCache());
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Init(getCfgSingleton().getItemString(CCfgLocal::lbs_domain), getCfgSingleton().getItemInt(CCfgLocal::lbs_port), &allconfig, 1));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Start());
//   EXPECT_EQ(0, dev_event_waitfor(ezDevSDK_App_Event_Online, 10 * 1000, NULL));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_id));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_masterkey));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Stop());
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Fini());

//   EXPECT_TRUE(deleteFile(allconfig.config.dev_id));
//   EXPECT_FALSE(isFileExist(allconfig.config.dev_id));

//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Init(getCfgSingleton().getItemString(CCfgLocal::lbs_domain), getCfgSingleton().getItemInt(CCfgLocal::lbs_port), &allconfig, 1));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Start());
//   EXPECT_EQ(0, dev_event_waitfor(ezDevSDK_App_Event_Online, 10 * 1000, NULL));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_id));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_masterkey));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Stop());
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Fini());
// }

// /**
//  * @brief 本地无masterkey
//  *
//  */
// TEST(mk_online, nomasterkey)
// {
//   ezDevSDK_all_config allconfig = {0};

//   sprintf(allconfig.config.devinfo_path, "./%s/%s", dir_cache_name, file_devinfo_name);
//   sprintf(allconfig.config.dev_id, "./%s/%s", dir_cache_name, file_devid_name);
//   sprintf(allconfig.config.dev_masterkey, "./%s/%s", dir_cache_name, file_masterkey_name);
//   allconfig.config.keyValueLoadFun = key_value_load_cb;
//   allconfig.config.keyValueSaveFun = key_value_save_cb;
//   allconfig.config.curingDataLoadFun = curing_data_load_cb;
//   allconfig.config.curingDataSaveFun = curing_data_save_cb;
//   allconfig.notice.event_notice = event_notice_cb;
//   allconfig.notice.log_notice = log_notice_cb;

//   EXPECT_TRUE(clearCache());
//   EXPECT_TRUE(makeCache());
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Init(getCfgSingleton().getItemString(CCfgLocal::lbs_domain), getCfgSingleton().getItemInt(CCfgLocal::lbs_port), &allconfig, 1));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Start());
//   EXPECT_EQ(0, dev_event_waitfor(ezDevSDK_App_Event_Online, 10 * 1000, NULL));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_id));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_masterkey));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Stop());
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Fini());

//   EXPECT_TRUE(deleteFile(allconfig.config.dev_masterkey));
//   EXPECT_FALSE(isFileExist(allconfig.config.dev_masterkey));

//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Init(getCfgSingleton().getItemString(CCfgLocal::lbs_domain), getCfgSingleton().getItemInt(CCfgLocal::lbs_port), &allconfig, 1));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Start());
//   EXPECT_EQ(0, dev_event_waitfor(ezDevSDK_App_Event_Online, 10 * 1000, NULL));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_id));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_masterkey));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Stop());
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Fini());
// }

// /**
//  * @brief masterkey更新
//  *
//  */
// TEST(mk_online, masterkey_update)
// {
//   ezDevSDK_all_config allconfig = {0};

//   sprintf(allconfig.config.devinfo_path, "./%s/%s", dir_cache_name, file_devinfo_name);
//   sprintf(allconfig.config.dev_id, "./%s/%s", dir_cache_name, file_devid_name);
//   sprintf(allconfig.config.dev_masterkey, "./%s/%s", dir_cache_name, file_masterkey_name);
//   allconfig.config.keyValueLoadFun = key_value_load_cb;
//   allconfig.config.keyValueSaveFun = key_value_save_cb;
//   allconfig.config.curingDataLoadFun = curing_data_load_cb;
//   allconfig.config.curingDataSaveFun = curing_data_save_cb;
//   allconfig.notice.event_notice = event_notice_cb;
//   allconfig.notice.log_notice = log_notice_cb;

//   EXPECT_TRUE(clearCache());
//   EXPECT_TRUE(makeCache());
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Init(getCfgSingleton().getItemString(CCfgLocal::lbs_domain), getCfgSingleton().getItemInt(CCfgLocal::lbs_port), &allconfig, 1));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Start());
//   EXPECT_EQ(0, dev_event_waitfor(ezDevSDK_App_Event_Online, 10 * 1000, NULL));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_id));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_masterkey));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Stop());
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Fini());

//   EXPECT_TRUE(setFileValue(allconfig.config.dev_masterkey, invalid_masterkey, strlen(invalid_masterkey)));

//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Init(getCfgSingleton().getItemString(CCfgLocal::lbs_domain), getCfgSingleton().getItemInt(CCfgLocal::lbs_port), &allconfig, 1));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Start());
//   EXPECT_EQ(0, dev_event_waitfor(ezDevSDK_App_Event_Online, 10 * 1000, NULL));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_id));
//   EXPECT_TRUE(isFileExist(allconfig.config.dev_masterkey));
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Stop());
//   EXPECT_EQ(ezdev_sdk_kernel_succ, ezDevSDK_Fini());
//   EXPECT_NE(0, strcmp((const char *)invalid_masterkey, (const char *)allconfig.config.dev_masterkey));
// }