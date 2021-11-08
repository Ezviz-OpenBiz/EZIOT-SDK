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
 * 2021-01-28     xurongjun
 *******************************************************************************/

#include <stdlib.h>
#include <flashdb.h>
#include <string.h>
#include "ut_config.h"
#include "ez_iot.h"
#include "ez_iot_hub.h"
#include "ez_iot_log.h"
#include "ez_hal/hal_thread.h"
#include "kv_imp.h"
#include "utest.h"

void ut_gateway_errcode();  ///< 测试接口返回错误码
void ut_gateway_authsap();  ///< 测试序列号验证添加
void ut_gateway_authlic();  ///< 测试license设备添加
void ut_gateway_authfail(); ///< 测试认证失败的流程
void ut_gateway_dup();      ///< 测试重复添加
void ut_gateway_update();   ///< 测试子设备信息更新(版本号、在线状态)
void ut_gateway_enum();     ///< 测试子设备遍历
void ut_gateway_clean();    ///< 测试清空功能
void ut_gateway_stress();   ///< 压力测试

static rt_err_t test_init();

UTEST_TC_EXPORT(ut_gateway_errcode, test_init, NULL, DEFAULT_TIMEOUT_S);
UTEST_TC_EXPORT(ut_gateway_authsap, test_init, NULL, DEFAULT_TIMEOUT_S);
UTEST_TC_EXPORT(ut_gateway_authlic, test_init, NULL, DEFAULT_TIMEOUT_S);
UTEST_TC_EXPORT(ut_gateway_authfail, test_init, NULL, DEFAULT_TIMEOUT_S);
UTEST_TC_EXPORT(ut_gateway_dup, test_init, NULL, DEFAULT_TIMEOUT_S);
UTEST_TC_EXPORT(ut_gateway_update, test_init, NULL, DEFAULT_TIMEOUT_S);
UTEST_TC_EXPORT(ut_gateway_enum, test_init, NULL, DEFAULT_TIMEOUT_S);
UTEST_TC_EXPORT(ut_gateway_clean, test_init, NULL, DEFAULT_TIMEOUT_S);
UTEST_TC_EXPORT(ut_gateway_stress, test_init, NULL, DEFAULT_TIMEOUT_S);

static int32_t ez_recv_msg_cb(ez_iot_cloud2dev_msg_type_t msg_type, void *data, int len);
static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len);
static int32_t hub_recv_event_cb(hub_event_t event_type, void *data, int len);

static int m_event_id = -1;
static int m_add_result = -1;
static int8_t m_add_result_sn[32] = {0};

static void *m_event_ctx = NULL;
static int m_last_err = 0;
static ez_iot_srv_info_t m_lbs_addr = {(int8_t *)EZ_IOT_CLOUD_ENTRY_HOST, EZ_IOT_CLOUD_ENTRY_PORT};
static ez_iot_callbacks_t m_cbs = {ez_recv_msg_cb, ez_recv_event_cb};
static hub_callbacks_t m_hub_cbs = {hub_recv_event_cb};
static int wait4add_result(hub_event_t event_type, int8_t *sn, int time_ms);

static ez_iot_dev_info_t m_dev_info = {
    .auth_mode = EZ_IOT_DEV_AUTH_MODE,
    .dev_subserial = EZ_IOT_DEV_UUID,
    .dev_verification_code = EZ_IOT_DEV_LICENSE,
    .dev_firmwareversion = EZ_IOT_DEV_FWVER,
    .dev_type = EZ_IOT_DEV_PRODUCT_KEY,
    .dev_typedisplay = EZ_IOT_DEV_DISPLAY_NAME,
    .dev_id = EZ_IOT_DEV_ID,
};

static int8_t m_test_subdev1_authm = 0;
static int8_t *m_test_subdev1_sn = (int8_t *)"P20150916";
static int8_t *m_test_subdev1_ver = (int8_t *)"V1.6.0 build 210118";
static int8_t *m_test_subdev1_type = (int8_t *)"4LYV8SK7UKLBOUOVS6HXVX";
static int8_t *m_test_subdev1_vcode = (int8_t *)"VS7ij3";
static int8_t *m_test_subdev1_uuid = (int8_t *)"FFEEDDCCBBAA";

static int8_t m_test_subdev2_authm = 1;
static int8_t *m_test_subdev2_sn = (int8_t *)"ARZCWQSRG3CW";
static int8_t *m_test_subdev2_ver = (int8_t *)"V1.6.0 build 210118";
static int8_t *m_test_subdev2_type = (int8_t *)"4LYV8SK7UKLBOUOVS6HXVX";
static int8_t *m_test_subdev2_vcode = (int8_t *)"Jcjkn8xah1qUk5JKJFjKD2";
static int8_t *m_test_subdev2_uuid = (int8_t *)"AABBCCDDEEFF";

static hub_subdev_info_t m_subdev1 = {0};
static hub_subdev_info_t m_subdev2 = {0};

static ez_iot_kv_callbacks_t m_kv_cbs = {
    .ez_kv_init = kv_init,
    .ez_kv_raw_set = kv_raw_set,
    .ez_kv_raw_get = kv_raw_get,
    .ez_kv_del = kv_del,
    .ez_kv_del_by_prefix = kv_del_by_prefix,
    .ez_kv_print = kv_print,
    .ez_kv_deinit = kv_deinit,
};

void ut_gateway_errcode()
{
    hub_subdev_info_t subdev_temp = {0};

    //sdk core not init
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_del(m_test_subdev1_sn));
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_ver_update(m_test_subdev1_sn, m_test_subdev2_ver));
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_status_update(m_test_subdev1_sn, false));
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_subdev_query(m_test_subdev1_sn, &subdev_temp));
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_subdev_next(&subdev_temp));
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_clean());
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_deinit());

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_init(NULL));

    uassert_int_equal(ez_errno_succ, ez_iot_start());

    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_add(NULL));
    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_del(NULL));
    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_ver_update(NULL, m_test_subdev2_ver));
    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_ver_update(m_test_subdev1_sn, NULL));
    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_status_update(NULL, false));
    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_subdev_query(NULL, &subdev_temp));
    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_subdev_query(m_test_subdev1_sn, NULL));
    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_subdev_next(NULL));

    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_del(m_test_subdev1_sn));
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_ver_update(m_test_subdev1_sn, m_test_subdev2_ver));
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_status_update(m_test_subdev1_sn, false));
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_subdev_query(m_test_subdev1_sn, &subdev_temp));
    uassert_int_equal(ez_errno_hub_enum_end, ez_iot_hub_subdev_next(&subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_gateway_authsap()
{
    hub_subdev_info_t subdev_temp = {0};

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_start());
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(ez_errno_succ, wait4add_result(hub_event_add_succ, m_subdev1.subdev_sn, DEFAULT_TIMEOUT_S * 1000));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(0, memcmp((void *)&m_subdev1, (void *)&subdev_temp, sizeof(subdev_temp)));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_del(m_subdev1.subdev_sn));
    memset((void *)&subdev_temp, 0, sizeof(subdev_temp));
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_gateway_authlic()
{
    hub_subdev_info_t subdev_temp = {0};

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_start());
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_subdev_query(m_subdev2.subdev_sn, &subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev2));
    uassert_int_equal(ez_errno_succ, wait4add_result(hub_event_add_succ, m_subdev2.subdev_sn, DEFAULT_TIMEOUT_S * 1000));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_subdev_query(m_subdev2.subdev_sn, &subdev_temp));
    uassert_int_equal(0, memcmp((void *)&m_subdev2, (void *)&subdev_temp, sizeof(subdev_temp)));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_del(m_subdev2.subdev_sn));
    memset((void *)&subdev_temp, 0, sizeof(subdev_temp));
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_subdev_query(m_subdev2.subdev_sn, &subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_gateway_authfail()
{
    hub_subdev_info_t subdev_temp = {0};

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_start());
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_subdev_query(m_subdev2.subdev_sn, &subdev_temp));

    strncpy((char *)m_subdev1.subdev_vcode, "ABCDEF", sizeof(m_subdev1.subdev_vcode) - 1);
    strncpy((char *)m_subdev2.subdev_vcode, "ABCDEF", sizeof(m_subdev2.subdev_vcode) - 1);

    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(ez_errno_succ, wait4add_result(hub_event_add_fail, m_subdev1.subdev_sn, DEFAULT_TIMEOUT_S * 1000));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev2));
    uassert_int_equal(ez_errno_succ, wait4add_result(hub_event_add_fail, m_subdev2.subdev_sn, DEFAULT_TIMEOUT_S * 1000));

    /* 添加失败，自动删除 */
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_subdev_query(m_subdev2.subdev_sn, &subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_gateway_dup()
{
    hub_subdev_info_t subdev_temp = {0};

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_start());
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev1));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(0, memcmp((void *)&m_subdev1, (void *)&subdev_temp, sizeof(subdev_temp)));
    uassert_int_equal(ez_errno_hub_subdev_existed, ez_iot_hub_add(&m_subdev1));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_del(m_subdev1.subdev_sn));
    memset((void *)&subdev_temp, 0, sizeof(subdev_temp));
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_gateway_update()
{
    hub_subdev_info_t subdev_temp = {0};
    static int8_t *temp_subdev_ver = (int8_t *)"V3.0.0 build 210118";
    static bool temp_subdev_sta = false;

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_start());

    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(0, memcmp((void *)&m_subdev1, (void *)&subdev_temp, sizeof(subdev_temp)));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_ver_update(m_subdev1.subdev_sn, temp_subdev_ver));
    memset((void *)&subdev_temp, 0, sizeof(subdev_temp));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(0, strncmp((char *)temp_subdev_ver, (char *)subdev_temp.subdev_ver, sizeof(subdev_temp.subdev_ver) - 1));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_status_update(m_subdev1.subdev_sn, temp_subdev_sta));
    memset((void *)&subdev_temp, 0, sizeof(subdev_temp));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_false(subdev_temp.sta);

    uassert_int_equal(ez_errno_succ, ez_iot_hub_del(m_subdev1.subdev_sn));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_gateway_clean()
{
    hub_subdev_info_t subdev_temp = {0};

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_start());

    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev2));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_clean());

    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_subdev_query(m_subdev1.subdev_sn, &subdev_temp));
    uassert_int_equal(ez_errno_hub_enum_end, ez_iot_hub_subdev_next(&subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_gateway_enum()
{
    hub_subdev_info_t subdev_temp = {0};
    const int subdev_max = 64;
    int subdev_count = 0;

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_start());

    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&m_subdev2));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_subdev_next(&subdev_temp));
    uassert_int_equal(0, memcmp((void *)&m_subdev1, (void *)&subdev_temp, sizeof(subdev_temp)));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_subdev_next(&subdev_temp));
    uassert_int_equal(0, memcmp((void *)&m_subdev2, (void *)&subdev_temp, sizeof(subdev_temp)));
    uassert_int_equal(ez_errno_hub_enum_end, ez_iot_hub_subdev_next(&subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_clean());
    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_gateway_stress()
{
    hub_subdev_info_t subdev_temp = {0};
    const int subdev_max = 16;
    int i = 0;

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init(&m_hub_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_clean());
    uassert_int_equal(ez_errno_succ, ez_iot_start());
    memcpy((void *)&subdev_temp, (void *)&m_subdev1, sizeof(subdev_temp));

    for (i = 0; i < subdev_max; i++)
    {
        snprintf((char *)subdev_temp.subdev_sn, sizeof(subdev_temp.subdev_sn), "E111111%02d", i);
        uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&subdev_temp));
    }

    uassert_int_equal(ez_errno_succ, wait4add_result(hub_event_add_fail, subdev_temp.subdev_sn, DEFAULT_TIMEOUT_S * 1000));

    memset(&subdev_temp, 0, sizeof(subdev_temp));
    uassert_int_equal(ez_errno_hub_enum_end, ez_iot_hub_subdev_next(&subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

static int wait4add_result(hub_event_t event_type, int8_t *sn, int time_ms)
{
    int ret = -1;
    int index = 0;
    m_add_result = -1;
    memset(m_add_result_sn, 0, sizeof(m_add_result_sn));

    do
    {
        if (event_type == m_add_result &&
            0 == strcmp(m_add_result_sn, sn))
        {
            ret = 0;
            break;
        }

        hal_thread_sleep(10);
        index += 10;
    } while (index < time_ms);

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

static int32_t hub_recv_event_cb(hub_event_t event_type, void *data, int len)
{
    hub_subdev_info_t *sub_dev_info = NULL;

    switch (event_type)
    {
    case hub_event_add_succ:
        strncpy(m_add_result_sn, (char *)data, sizeof(m_add_result_sn) - 1);
        ez_log_d("UT_HUB", "add succ, sn:%s", (char *)data);
        break;
    case hub_event_add_fail:
        sub_dev_info = (hub_subdev_info_t *)data;
        strncpy(m_add_result_sn, (char *)sub_dev_info->subdev_sn, sizeof(m_add_result_sn) - 1);
        ez_log_e("UT_HUB", "add fail, sn:%s", (char *)sub_dev_info->subdev_sn);
        break;

    default:
        break;
    }

    m_add_result = event_type;

    return 0;
}

static rt_err_t test_init()
{
    m_subdev1.auth_mode = m_test_subdev1_authm;
    strncpy((char *)m_subdev1.subdev_sn, (char *)m_test_subdev1_sn, sizeof(m_subdev1.subdev_sn) - 1);
    strncpy((char *)m_subdev1.subdev_ver, (char *)m_test_subdev1_ver, sizeof(m_subdev1.subdev_ver) - 1);
    strncpy((char *)m_subdev1.subdev_type, (char *)m_test_subdev1_type, sizeof(m_subdev1.subdev_type) - 1);
    strncpy((char *)m_subdev1.subdev_vcode, (char *)m_test_subdev1_vcode, sizeof(m_subdev1.subdev_vcode) - 1);
    strncpy((char *)m_subdev1.subdev_uuid, (char *)m_test_subdev1_uuid, sizeof(m_subdev1.subdev_uuid) - 1);
    m_subdev1.sta = 1;

    m_subdev2.auth_mode = m_test_subdev2_authm;
    strncpy((char *)m_subdev2.subdev_sn, (char *)m_test_subdev2_sn, sizeof(m_subdev2.subdev_sn) - 1);
    strncpy((char *)m_subdev2.subdev_ver, (char *)m_test_subdev2_ver, sizeof(m_subdev2.subdev_ver) - 1);
    strncpy((char *)m_subdev2.subdev_type, (char *)m_test_subdev2_type, sizeof(m_subdev2.subdev_type) - 1);
    strncpy((char *)m_subdev2.subdev_vcode, (char *)m_test_subdev2_vcode, sizeof(m_subdev2.subdev_vcode) - 1);
    strncpy((char *)m_subdev2.subdev_uuid, (char *)m_test_subdev2_uuid, sizeof(m_subdev2.subdev_uuid) - 1);
    m_subdev2.sta = 0;

    return 0;
}