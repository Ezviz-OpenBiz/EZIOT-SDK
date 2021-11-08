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
 * 2020-11-06     xurongjun
 *******************************************************************************/

#include <stdlib.h>
#include "flashdb.h"
#include <string.h>
#include "ut_config.h"
#include "ez_iot.h"
#include "ez_iot_log.h"
#include "ez_iot_tsl.h"
#include "ez_hal/hal_thread.h"
#include "kv_imp.h"
#include "utest.h"

/* 测试接口错误码 */
void ut_tsl_errcode();
UTEST_TC_EXPORT(ut_tsl_errcode, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 设备首次注册 */
// void ut_tsl_first_reg();
// UTEST_TC_EXPORT(ut_tsl_first_reg, NULL, NULL, DEFAULT_TIMEOUT_S);

// /* 同一个设备多次重复注册 */
// void ut_tsl_repeat_reg();
// UTEST_TC_EXPORT(ut_tsl_repeat_reg, NULL, NULL, DEFAULT_TIMEOUT_S);

// /* 多个设备注册 */
// void ut_tsl_multi_reg();
// UTEST_TC_EXPORT(ut_tsl_multi_reg, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报属性-布尔 */
void ut_tsl_prop_report_bool();
UTEST_TC_EXPORT(ut_tsl_prop_report_bool, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报属性-整形 */
void ut_tsl_prop_report_int();
UTEST_TC_EXPORT(ut_tsl_prop_report_int, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报属性-浮点 */
void ut_tsl_prop_report_float();
UTEST_TC_EXPORT(ut_tsl_prop_report_float, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报属性-字符串 */
void ut_tsl_prop_report_str();
UTEST_TC_EXPORT(ut_tsl_prop_report_str, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报属性-json对象 */
void ut_tsl_prop_report_jobj();
UTEST_TC_EXPORT(ut_tsl_prop_report_jobj, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报属性-json数组 */
void ut_tsl_prop_report_jarr();
UTEST_TC_EXPORT(ut_tsl_prop_report_jarr, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报事件-无数据 */
void ut_tsl_event_report_null();
UTEST_TC_EXPORT(ut_tsl_event_report_null, NULL, NULL, DEFAULT_TIMEOUT_S);

/* 上报事件-json对象 */
void ut_tsl_event_report_obj();
UTEST_TC_EXPORT(ut_tsl_event_report_obj, NULL, NULL, DEFAULT_TIMEOUT_S);

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

static int32_t tsl_things_action2dev(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info,
                                     const tsl_param_t *value_in, tsl_param_t *value_out)
{
  return 0;
}

static int32_t tsl_things_property2cloud(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, tsl_value_t *value_out)
{
  return 0;
}

int32_t tsl_things_property2dev(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, const tsl_value_t *value)
{
  return 1;
}

void ut_tsl_errcode()
{
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_things_callbacks_t tsl_things_cbs_null = {0};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  tsl_key_info_t tsl_keyinfo = {.domain = (int8_t *)"attr_test", .key = (int8_t *)"attr_r_bool"};
  tsl_key_info_t tsl_keyinfo_event = {.domain = (int8_t *)"event_test", .key = (int8_t *)"event_null"};
  tsl_value_t tsl_value = {.size = 1, .type = tsl_data_type_bool, .value_bool = false};
  tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  //sdk core not init
  uassert_int_equal(ez_errno_tsl_not_init, ez_iot_tsl_init(NULL));
  uassert_int_equal(ez_errno_tsl_not_init, ez_iot_tsl_property_report(NULL, NULL, NULL, NULL));
  uassert_int_equal(ez_errno_tsl_not_init, ez_iot_tsl_event_report(NULL, NULL, NULL, NULL));
  uassert_int_equal(ez_errno_tsl_not_init, ez_iot_tsl_deinit());

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));

  //tsl not init
  uassert_int_equal(ez_errno_tsl_not_init, ez_iot_tsl_property_report(NULL, NULL, NULL, NULL));
  uassert_int_equal(ez_errno_tsl_not_init, ez_iot_tsl_event_report(NULL, NULL, NULL, NULL));
  uassert_int_equal(ez_errno_tsl_not_init, ez_iot_tsl_deinit());

  //tsl init with invalid params
  uassert_int_equal(ez_errno_tsl_param_invalid, ez_iot_tsl_init(NULL));
  uassert_int_equal(ez_errno_tsl_param_invalid, ez_iot_tsl_init(&tsl_things_cbs_null));

  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));

  //sdk core module is not started
  uassert_int_equal(ez_errno_tsl_not_ready, ez_iot_tsl_property_report(NULL, NULL, NULL, NULL));
  uassert_int_equal(ez_errno_tsl_not_ready, ez_iot_tsl_event_report(NULL, NULL, NULL, NULL));

  uassert_int_equal(ez_errno_succ, ez_iot_start());

  //interface called with invaild params
  uassert_int_equal(ez_errno_tsl_param_invalid, ez_iot_tsl_property_report(NULL, &rsc_info, &tsl_keyinfo, &tsl_value));
  uassert_int_equal(ez_errno_tsl_param_invalid, ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, NULL, &tsl_keyinfo, &tsl_value));
  uassert_int_equal(ez_errno_tsl_param_invalid, ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, NULL, &tsl_value));
  uassert_int_equal(ez_errno_tsl_param_invalid, ez_iot_tsl_event_report(NULL, &rsc_info, &tsl_keyinfo_event, NULL));
  uassert_int_equal(ez_errno_tsl_param_invalid, ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, NULL, &tsl_keyinfo_event, NULL));
  uassert_int_equal(ez_errno_tsl_param_invalid, ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, &rsc_info, NULL, NULL));

  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_prop_report_bool()
{
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  tsl_key_info_t tsl_keyinfo = {.domain = (int8_t *)"attr_test", .key = (int8_t *)"attr_r_bool"};
  tsl_value_t tsl_value = {.size = 1, .type = tsl_data_type_bool, .value_bool = false};
  tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value));

  hal_thread_sleep(5000);
  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_prop_report_int()
{
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  tsl_key_info_t tsl_keyinfo = {.domain = (int8_t *)"attr_test", .key = (int8_t *)"attr_r_int"};
  tsl_value_t tsl_value = {.size = 4, .type = tsl_data_type_int, .value_int = 2};
  tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value));

  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_prop_report_float()
{
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  tsl_key_info_t tsl_keyinfo = {.domain = (int8_t *)"attr_test", .key = (int8_t *)"attr_r_num"};
  tsl_value_t tsl_value = {.size = sizeof(double), .type = tsl_data_type_double, .value_double = 10086.11112222};
  tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value));

  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_prop_report_str()
{
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  tsl_key_info_t tsl_keyinfo = {.domain = (int8_t *)"attr_test", .key = (int8_t *)"attr_r_str"};
  tsl_value_t tsl_value = {.size = strlen("test_str_111222333"), .type = tsl_data_type_string, .value = "test_str_111222333"};
  tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value));

  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_prop_report_jobj()
{
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  tsl_key_info_t tsl_keyinfo = {.domain = (int8_t *)"attr_test", .key = (int8_t *)"attr_r_obj"};
  tsl_value_t tsl_value = {.size = strlen("{\"test_int\":89622462}"), .type = tsl_data_type_object, .value = "{\"test_int\":89622462}"};
  tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value));

  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_prop_report_jarr()
{
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  tsl_key_info_t tsl_keyinfo = {.domain = (int8_t *)"attr_test", .key = (int8_t *)"attr_r_array"};
  tsl_value_t tsl_value = {.size = strlen("[-42570399,56004448]"), .type = tsl_data_type_array, .value = "[-42570399,56004448]"};
  tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_property_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value));

  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_event_report_null()
{
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  tsl_key_info_t tsl_keyinfo = {.domain = (int8_t *)"event_test", .key = (int8_t *)"event_null"};
  tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, NULL));

  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
  ez_iot_fini();
}

void ut_tsl_event_report_obj()
{
  tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
  tsl_devinfo_t tsl_devinfo = {(int8_t *)EZ_IOT_DEV_UUID, (int8_t *)EZ_IOT_DEV_PRODUCT_KEY, (int8_t *)EZ_IOT_DEV_FWVER};
  tsl_key_info_t tsl_keyinfo = {.domain = (int8_t *)"event_test", .key = (int8_t *)"event_ext"};
  tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)"PetDryerRes", .local_index = (int8_t *)"0"};
  tsl_param_t tsl_value = {.key = (int8_t *)"ext", .value.size = strlen("{\"psd\":\"consequat sit in\"}"), .value.type = tsl_data_type_object, .value.value = (int8_t *)"{\"psd\":\"consequat sit in\"}"};

  uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_init(&tsl_things_cbs));
  uassert_int_equal(ez_errno_succ, ez_iot_start());

  uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_event_report(tsl_devinfo.dev_subserial, &rsc_info, &tsl_keyinfo, &tsl_value));

  uassert_int_equal(ez_errno_succ, ez_iot_stop());
  uassert_int_equal(ez_errno_succ, ez_iot_tsl_deinit());
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
//  * @brief 物模型初始化
//  *
//  */
// TEST(sdk_tsl, init)
// {
//   char devinfo_path[128] = {0};
//   char masterkey_path[128] = {0};
//   char devid_path[128] = {0};
//   size_t buf_len = 0;

//   ez_iot_srv_info_t lbs_addr = {0};
//   ez_iot_dev_info_t dev_info = {0};
//   ez_iot_callbacks_t cbs = {ez_recv_msg_cb, ez_recv_event_cb};
//   tsl_data_callbacks_t tsl_data_cbs = {NULL, NULL, NULL};
//   tsl_things_callbacks_t tsl_things_cbs = {tsl_things_report2dev, tsl_things_reply2dev, tsl_things_properity2cloud};

//   /* 本地文件路径 */
//   sprintf(devinfo_path, "./%s/%s", dir_cache_name, file_devinfo_name);
//   sprintf(masterkey_path, "./%s/%s", dir_cache_name, file_masterkey_name);
//   sprintf(devid_path, "./%s/%s", dir_cache_name, file_devid_name);

//   /* 清空缓存文件 */
//   EXPECT_TRUE(clearCache());
//   EXPECT_TRUE(makeCache());

//   /* 读取服务器信息*/
//   lbs_addr.svr_name = (int8_t *)getCfgSingleton().getItemString(CCfgLocal::lbs_domain);
//   lbs_addr.svr_port = (int16_t)getCfgSingleton().getItemInt(CCfgLocal::lbs_port);

//   /* 读取dev info文件*/
//   CDevInfo m_dev_info(devinfo_path);
//   EXPECT_TRUE(m_dev_info.load());
//   EXPECT_TRUE(m_dev_info.bFormatValid());
//   dev_info.auth_mode = m_dev_info.getItemInt(CDevInfo::dev_auth_mode);
//   strncpy((char *)dev_info.dev_subserial, m_dev_info.getItemString(CDevInfo::dev_subserial), sizeof(dev_info.dev_subserial) - 1);
//   strncpy((char *)dev_info.dev_verification_code, m_dev_info.getItemString(CDevInfo::dev_verification_code), sizeof(dev_info.dev_verification_code) - 1);
//   strncpy((char *)dev_info.dev_type, m_dev_info.getItemString(CDevInfo::dev_type), sizeof(dev_info.dev_type) - 1);
//   strncpy((char *)dev_info.dev_typedisplay, m_dev_info.getItemString(CDevInfo::dev_typedisplay), sizeof(dev_info.dev_typedisplay) - 1);
//   strncpy((char *)dev_info.dev_firmwareversion, m_dev_info.getItemString(CDevInfo::dev_firmwareversion), sizeof(dev_info.dev_firmwareversion) - 1);

//   /* 读取masterkey和devid */
//   buf_len = sizeof(dev_info.dev_id);
//   getFileValue(devid_path, (char *)dev_info.dev_id, &buf_len);
//   buf_len = sizeof(dev_info.dev_masterkey);
//   getFileValue(masterkey_path, (char *)dev_info.dev_masterkey, &buf_len);

//   /* 开始测试 */
//   EXPECT_EQ(ez_errno_succ, ez_iot_init(&lbs_addr, &dev_info, &cbs));
//   EXPECT_EQ(ez_errno_succ, ez_iot_tsl_init(&tsl_data_cbs, &tsl_things_cbs));
//   EXPECT_EQ(ez_errno_succ, ez_iot_start());
//   EXPECT_EQ(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, 10 * 1000));
//   EXPECT_TRUE(isFileExist(masterkey_path));
//   EXPECT_TRUE(isFileExist(devid_path));
//   EXPECT_EQ(ez_errno_succ, ez_iot_stop());
//   EXPECT_EQ(ez_errno_succ, ez_iot_tsl_deinit());
//   ez_iot_fini();
// }

// /**
//  * @brief 物模型注册，正常逻辑
//  *
//  */
// TEST(sdk_tsl, reg)
// {
//   char devinfo_path[128] = {0};
//   char masterkey_path[128] = {0};
//   char devid_path[128] = {0};
//   size_t buf_len = 0;
//   char profile_fn[128] = {0};

//   ez_iot_srv_info_t lbs_addr = {0};
//   ez_iot_dev_info_t dev_info = {0};
//   ez_iot_callbacks_t cbs = {ez_recv_msg_cb, ez_recv_event_cb};
//   tsl_data_callbacks_t tsl_data_cbs = {tsl_things_data_load, tsl_things_data_save, tsl_things_data_delete};
//   tsl_things_callbacks_t tsl_things_cbs = {tsl_things_report2dev, tsl_things_reply2dev, tsl_things_properity2cloud};
//   tsl_devinfo_t tsl_devinfo = {dev_info.dev_subserial, dev_info.dev_type, dev_info.dev_firmwareversion};

//   /* 本地文件路径 */
//   sprintf(devinfo_path, "./%s/%s", dir_cache_name, file_devinfo_name);
//   sprintf(masterkey_path, "./%s/%s", dir_cache_name, file_masterkey_name);
//   sprintf(devid_path, "./%s/%s", dir_cache_name, file_devid_name);

//   /* 清空缓存文件 */
//   EXPECT_TRUE(clearCache());
//   EXPECT_TRUE(makeCache());

//   /* 读取服务器信息*/
//   lbs_addr.svr_name = (int8_t *)getCfgSingleton().getItemString(CCfgLocal::lbs_domain);
//   lbs_addr.svr_port = (int16_t)getCfgSingleton().getItemInt(CCfgLocal::lbs_port);

//   /* 读取dev info文件*/
//   CDevInfo m_dev_info(devinfo_path);
//   EXPECT_TRUE(m_dev_info.load());
//   EXPECT_TRUE(m_dev_info.bFormatValid());
//   dev_info.auth_mode = m_dev_info.getItemInt(CDevInfo::dev_auth_mode);
//   strncpy((char *)dev_info.dev_subserial, m_dev_info.getItemString(CDevInfo::dev_subserial), sizeof(dev_info.dev_subserial) - 1);
//   strncpy((char *)dev_info.dev_verification_code, m_dev_info.getItemString(CDevInfo::dev_verification_code), sizeof(dev_info.dev_verification_code) - 1);
//   strncpy((char *)dev_info.dev_type, m_dev_info.getItemString(CDevInfo::dev_type), sizeof(dev_info.dev_type) - 1);
//   strncpy((char *)dev_info.dev_typedisplay, m_dev_info.getItemString(CDevInfo::dev_typedisplay), sizeof(dev_info.dev_typedisplay) - 1);
//   strncpy((char *)dev_info.dev_firmwareversion, m_dev_info.getItemString(CDevInfo::dev_firmwareversion), sizeof(dev_info.dev_firmwareversion) - 1);
//   snprintf(profile_fn, sizeof(profile_fn), "./%s/%s_%s", dir_cache_name, (char *)dev_info.dev_type, (char *)dev_info.dev_firmwareversion);

//   /* 读取masterkey和devid */
//   buf_len = sizeof(dev_info.dev_id);
//   getFileValue(devid_path, (char *)dev_info.dev_id, &buf_len);
//   buf_len = sizeof(dev_info.dev_masterkey);
//   getFileValue(masterkey_path, (char *)dev_info.dev_masterkey, &buf_len);

//   /* 开始测试 */
//   EXPECT_EQ(ez_errno_succ, ez_iot_init(&lbs_addr, &dev_info, &cbs));
//   EXPECT_EQ(ez_errno_succ, ez_iot_tsl_init(&tsl_data_cbs, &tsl_things_cbs));
//   EXPECT_EQ(ez_errno_succ, ez_iot_start());
//   EXPECT_EQ(ez_errno_succ, ez_iot_tsl_register(&tsl_devinfo));

//   EXPECT_EQ(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, 10 * 1000));

//   for (size_t i = 0; i < 5000; i++)
//   {
//     if (isFileExist(profile_fn))
//     {
//       break;
//     }

//     usleep(1000);
//   }

//   EXPECT_TRUE(isFileExist(masterkey_path));
//   EXPECT_TRUE(isFileExist(devid_path));
//   EXPECT_TRUE(isFileExist(profile_fn));

//   EXPECT_EQ(ez_errno_succ, ez_iot_stop());
//   EXPECT_EQ(ez_errno_succ, ez_iot_tsl_unregister(&tsl_devinfo));
//   EXPECT_FALSE(isFileExist(profile_fn));
//   EXPECT_EQ(ez_errno_succ, ez_iot_tsl_deinit());
//   ez_iot_fini();
// }

// /**
//  * @brief 物模型注册，异常逻辑-未反注册不清空描述文件
//  *
//  */
// TEST(sdk_tsl, reg_exp1)
// {
//   char devinfo_path[128] = {0};
//   char masterkey_path[128] = {0};
//   char devid_path[128] = {0};
//   size_t buf_len = 0;
//   char profile_fn[128] = {0};

//   ez_iot_srv_info_t lbs_addr = {0};
//   ez_iot_dev_info_t dev_info = {0};
//   ez_iot_callbacks_t cbs = {ez_recv_msg_cb, ez_recv_event_cb};
//   tsl_data_callbacks_t tsl_data_cbs = {tsl_things_data_load, tsl_things_data_save, tsl_things_data_delete};
//   tsl_things_callbacks_t tsl_things_cbs = {tsl_things_report2dev, tsl_things_reply2dev, tsl_things_properity2cloud};
//   tsl_devinfo_t tsl_devinfo = {dev_info.dev_subserial, dev_info.dev_type, dev_info.dev_firmwareversion};

//   /* 本地文件路径 */
//   sprintf(devinfo_path, "./%s/%s", dir_cache_name, file_devinfo_name);
//   sprintf(masterkey_path, "./%s/%s", dir_cache_name, file_masterkey_name);
//   sprintf(devid_path, "./%s/%s", dir_cache_name, file_devid_name);

//   /* 清空缓存文件 */
//   EXPECT_TRUE(clearCache());
//   EXPECT_TRUE(makeCache());

//   /* 读取服务器信息*/
//   lbs_addr.svr_name = (int8_t *)getCfgSingleton().getItemString(CCfgLocal::lbs_domain);
//   lbs_addr.svr_port = (int16_t)getCfgSingleton().getItemInt(CCfgLocal::lbs_port);

//   /* 读取dev info文件*/
//   CDevInfo m_dev_info(devinfo_path);
//   EXPECT_TRUE(m_dev_info.load());
//   EXPECT_TRUE(m_dev_info.bFormatValid());
//   dev_info.auth_mode = m_dev_info.getItemInt(CDevInfo::dev_auth_mode);
//   strncpy((char *)dev_info.dev_subserial, m_dev_info.getItemString(CDevInfo::dev_subserial), sizeof(dev_info.dev_subserial) - 1);
//   strncpy((char *)dev_info.dev_verification_code, m_dev_info.getItemString(CDevInfo::dev_verification_code), sizeof(dev_info.dev_verification_code) - 1);
//   strncpy((char *)dev_info.dev_type, m_dev_info.getItemString(CDevInfo::dev_type), sizeof(dev_info.dev_type) - 1);
//   strncpy((char *)dev_info.dev_typedisplay, m_dev_info.getItemString(CDevInfo::dev_typedisplay), sizeof(dev_info.dev_typedisplay) - 1);
//   strncpy((char *)dev_info.dev_firmwareversion, m_dev_info.getItemString(CDevInfo::dev_firmwareversion), sizeof(dev_info.dev_firmwareversion) - 1);
//   snprintf(profile_fn, sizeof(profile_fn), "./%s/%s_%s", dir_cache_name, (char *)dev_info.dev_type, (char *)dev_info.dev_firmwareversion);

//   /* 读取masterkey和devid */
//   buf_len = sizeof(dev_info.dev_id);
//   getFileValue(devid_path, (char *)dev_info.dev_id, &buf_len);
//   buf_len = sizeof(dev_info.dev_masterkey);
//   getFileValue(masterkey_path, (char *)dev_info.dev_masterkey, &buf_len);

//   /* 开始测试 */
//   EXPECT_EQ(ez_errno_succ, ez_iot_init(&lbs_addr, &dev_info, &cbs));
//   EXPECT_EQ(ez_errno_succ, ez_iot_tsl_init(&tsl_data_cbs, &tsl_things_cbs));
//   EXPECT_EQ(ez_errno_succ, ez_iot_start());
//   EXPECT_EQ(ez_errno_succ, ez_iot_tsl_register(&tsl_devinfo));

//   EXPECT_EQ(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, 10 * 1000));

//   for (size_t i = 0; i < 5000; i++)
//   {
//     if (isFileExist(profile_fn))
//     {
//       break;
//     }

//     usleep(1000);
//   }

//   EXPECT_TRUE(isFileExist(masterkey_path));
//   EXPECT_TRUE(isFileExist(devid_path));
//   EXPECT_TRUE(isFileExist(profile_fn));

//   EXPECT_EQ(ez_errno_succ, ez_iot_stop());
//   // EXPECT_EQ(ez_errno_succ, ez_iot_tsl_unregister(&tsl_devinfo));
//   EXPECT_EQ(ez_errno_succ, ez_iot_tsl_deinit());
//   EXPECT_TRUE(isFileExist(profile_fn));

//   ez_iot_fini();
// }

// /**
//  * @brief 物模型上报属性
//  *
//  */
// TEST(sdk_tsl, report_prop)
// {
//   char devinfo_path[128] = {0};
//   char masterkey_path[128] = {0};
//   char devid_path[128] = {0};
//   size_t buf_len = 0;
//   char profile_fn[128] = {0};

//   ez_iot_srv_info_t lbs_addr = {0};
//   ez_iot_dev_info_t dev_info = {0};
//   ez_iot_callbacks_t cbs = {ez_recv_msg_cb, ez_recv_event_cb};
//   tsl_data_callbacks_t tsl_data_cbs = {tsl_things_data_load, tsl_things_data_save, tsl_things_data_delete};
//   tsl_things_callbacks_t tsl_things_cbs = {tsl_things_report2dev, tsl_things_reply2dev, tsl_things_properity2cloud};
//   tsl_devinfo_t tsl_devinfo = {dev_info.dev_subserial, dev_info.dev_type, dev_info.dev_firmwareversion};

//   /* 本地文件路径 */
//   sprintf(devinfo_path, "./%s/%s", dir_cache_name, file_devinfo_name);
//   sprintf(masterkey_path, "./%s/%s", dir_cache_name, file_masterkey_name);
//   sprintf(devid_path, "./%s/%s", dir_cache_name, file_devid_name);

//   /* 清空缓存文件 */
//   EXPECT_TRUE(clearCache());
//   EXPECT_TRUE(makeCache());

//   /* 读取服务器信息*/
//   lbs_addr.svr_name = (int8_t *)getCfgSingleton().getItemString(CCfgLocal::lbs_domain);
//   lbs_addr.svr_port = (int16_t)getCfgSingleton().getItemInt(CCfgLocal::lbs_port);

//   /* 读取dev info文件*/
//   CDevInfo m_dev_info(devinfo_path);
//   EXPECT_TRUE(m_dev_info.load());
//   EXPECT_TRUE(m_dev_info.bFormatValid());
//   dev_info.auth_mode = m_dev_info.getItemInt(CDevInfo::dev_auth_mode);
//   strncpy((char *)dev_info.dev_subserial, m_dev_info.getItemString(CDevInfo::dev_subserial), sizeof(dev_info.dev_subserial) - 1);
//   strncpy((char *)dev_info.dev_verification_code, m_dev_info.getItemString(CDevInfo::dev_verification_code), sizeof(dev_info.dev_verification_code) - 1);
//   strncpy((char *)dev_info.dev_type, m_dev_info.getItemString(CDevInfo::dev_type), sizeof(dev_info.dev_type) - 1);
//   strncpy((char *)dev_info.dev_typedisplay, m_dev_info.getItemString(CDevInfo::dev_typedisplay), sizeof(dev_info.dev_typedisplay) - 1);
//   strncpy((char *)dev_info.dev_firmwareversion, m_dev_info.getItemString(CDevInfo::dev_firmwareversion), sizeof(dev_info.dev_firmwareversion) - 1);
//   snprintf(profile_fn, sizeof(profile_fn), "./%s/%s_%s", dir_cache_name, (char *)dev_info.dev_type, (char *)dev_info.dev_firmwareversion);

//   /* 读取masterkey和devid */
//   buf_len = sizeof(dev_info.dev_id);
//   getFileValue(devid_path, (char *)dev_info.dev_id, &buf_len);
//   buf_len = sizeof(dev_info.dev_masterkey);
//   getFileValue(masterkey_path, (char *)dev_info.dev_masterkey, &buf_len);

//   /* 开始测试 */
//   EXPECT_EQ(ez_errno_succ, ez_iot_init(&lbs_addr, &dev_info, &cbs));
//   EXPECT_EQ(ez_errno_succ, ez_iot_tsl_init(&tsl_data_cbs, &tsl_things_cbs));
//   EXPECT_EQ(ez_errno_succ, ez_iot_start());
//   EXPECT_EQ(ez_errno_succ, ez_iot_tsl_register(&tsl_devinfo));

//   EXPECT_EQ(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, 10 * 1000));
//   for (size_t i = 0; i < 5000; i++)
//   {
//     if (isFileExist(profile_fn))
//     {
//       break;
//     }

//     usleep(1000);
//   }

//   EXPECT_TRUE(isFileExist(masterkey_path));
//   EXPECT_TRUE(isFileExist(devid_path));
//   EXPECT_TRUE(isFileExist(profile_fn));

//   /* 上报属性 */
//   tsl_key_info_t tsl_keyinfo;
//   tsl_value_t tsl_value;

//   tsl_devinfo.dev_firmwareversion = dev_info.dev_firmwareversion;
//   tsl_devinfo.dev_subserial = dev_info.dev_subserial;
//   tsl_devinfo.dev_type = dev_info.dev_type;

//   tsl_value.size = 1;
//   tsl_value.type = tsl_data_type_bool;
//   tsl_value.value_bool = false;

//   tsl_keyinfo.domain = (int8_t *)"attr_test";
//   tsl_keyinfo.key = (int8_t *)"attr_r_bool";
//   EXPECT_EQ(ez_errno_succ, ez_iot_tsl_property_report(&tsl_keyinfo, &tsl_devinfo, &tsl_value));

//   EXPECT_EQ(ez_errno_succ, ez_iot_stop());
//   EXPECT_EQ(ez_errno_succ, ez_iot_tsl_unregister(&tsl_devinfo));
//   EXPECT_EQ(ez_errno_succ, ez_iot_tsl_deinit());
//   ez_iot_fini();
// }
