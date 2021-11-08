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
#include "ez_iot_hub.h"
#include "ez_iot_log.h"
#include "ez_hal/hal_thread.h"
#include "kv_imp.h"
#include "utest.h"

void ut_getway_errcode();
void ut_getway_rw();
void ut_getway_update();
void ut_getway_clean();
void ut_getway_enum();
void ut_getway_full();

rt_err_t test_init();

// UTEST_TC_EXPORT(ut_getway_errcode, test_init, NULL, DEFAULT_TIMEOUT_S);
// UTEST_TC_EXPORT(ut_getway_rw, test_init, NULL, DEFAULT_TIMEOUT_S);
// UTEST_TC_EXPORT(ut_getway_update, test_init, NULL, DEFAULT_TIMEOUT_S);
// UTEST_TC_EXPORT(ut_getway_enum, test_init, NULL, DEFAULT_TIMEOUT_S);
// UTEST_TC_EXPORT(ut_getway_clean, test_init, NULL, DEFAULT_TIMEOUT_S);
// UTEST_TC_EXPORT(ut_getway_full, test_init, NULL, DEFAULT_TIMEOUT_S);

static int32_t ez_recv_msg_cb(ez_iot_cloud2dev_msg_type_t msg_type, void *data, int len);
static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len);

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

static int8_t *m_test_subdev_sn1 = (int8_t *)"E11111111";
static int8_t *m_test_subdev_ver1 = (int8_t *)"V1.0.0 build 210118";
static int8_t *m_test_subdev_type1 = (int8_t *)"TYPE_TEST1";
static int8_t *m_test_subdev_sn2 = (int8_t *)"E22222222";
static int8_t *m_test_subdev_ver2 = (int8_t *)"V2.0.0 build 210118";
static int8_t *m_test_subdev_type2 = (int8_t *)"TYPE_TEST2";
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

void ut_getway_errcode()
{
    hub_subdev_info_t subdev_temp = {0};

    //sdk core not init
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_del(m_test_subdev_sn1));
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_ver_update(m_test_subdev_sn1, m_test_subdev_ver2));
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_status_update(m_test_subdev_sn1, false));
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_subdev_query(m_test_subdev_sn1, &subdev_temp));
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_subdev_next(&subdev_temp));
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_clean());
    uassert_int_equal(ez_errno_hub_not_init, ez_iot_hub_deinit());

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init());

    uassert_int_equal(ez_errno_succ, ez_iot_start());

    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_add(NULL));
    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_del(NULL));
    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_ver_update(NULL, m_test_subdev_ver2));
    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_ver_update(m_test_subdev_sn1, NULL));
    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_status_update(NULL, false));
    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_subdev_query(NULL, &subdev_temp));
    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_subdev_query(m_test_subdev_sn1, NULL));
    uassert_int_equal(ez_errno_hub_param_invalid, ez_iot_hub_subdev_next(NULL));

    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_del(m_test_subdev_sn1));
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_ver_update(m_test_subdev_sn1, m_test_subdev_ver2));
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_status_update(m_test_subdev_sn1, false));
    uassert_int_equal(ez_errno_hub_subdev_not_found, ez_iot_hub_subdev_query(m_test_subdev_sn1, &subdev_temp));
    uassert_int_equal(ez_errno_hub_enum_end, ez_iot_hub_subdev_next(&subdev_temp));

    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_getway_rw()
{
    hub_subdev_info_t subdev_temp = {0};
    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init());
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

void ut_getway_update()
{
    hub_subdev_info_t subdev_temp = {0};
    static int8_t *temp_subdev_ver = (int8_t *)"V3.0.0 build 210118";
    static bool temp_subdev_sta = false;

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init());
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
    uassert_false(subdev_temp.online);

    uassert_int_equal(ez_errno_succ, ez_iot_hub_clean());
    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_getway_clean()
{
    hub_subdev_info_t subdev_temp = {0};

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init());
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

void ut_getway_enum()
{
    hub_subdev_info_t subdev_temp = {0};
    const int subdev_max = 64;
    int subdev_count = 0;

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init());
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

void ut_getway_full()
{
    hub_subdev_info_t subdev_temp = {0};
    const int subdev_max = 64;
    int i = 0;

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_init());
    uassert_int_equal(ez_errno_succ, ez_iot_hub_clean());
    uassert_int_equal(ez_errno_succ, ez_iot_start());
    memcpy((void *)&subdev_temp, (void *)&m_subdev1, sizeof(subdev_temp));

    for (i = 0; i < subdev_max; i++)
    {
        snprintf((char *)subdev_temp.subdev_sn, sizeof(subdev_temp.subdev_sn), "E11111111_%d", i);
        uassert_int_equal(ez_errno_succ, ez_iot_hub_add(&subdev_temp));
    }

    uassert_int_equal(ez_errno_hub_out_of_range, ez_iot_hub_add(&m_subdev1));
    uassert_int_equal(ez_errno_succ, ez_iot_hub_clean());
    uassert_int_equal(ez_errno_succ, ez_iot_hub_deinit());
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
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

rt_err_t test_init()
{
    m_subdev1.online = true;
    strncpy((char *)m_subdev1.subdev_sn, (char *)m_test_subdev_sn1, sizeof(m_subdev1.subdev_sn) - 1);
    strncpy((char *)m_subdev1.subdev_ver, (char *)m_test_subdev_ver1, sizeof(m_subdev1.subdev_ver) - 1);
    strncpy((char *)m_subdev1.subdev_type, (char *)m_test_subdev_type1, sizeof(m_subdev1.subdev_type) - 1);

    m_subdev2.online = true;
    strncpy((char *)m_subdev2.subdev_sn, (char *)m_test_subdev_sn2, sizeof(m_subdev2.subdev_sn) - 1);
    strncpy((char *)m_subdev2.subdev_ver, (char *)m_test_subdev_ver2, sizeof(m_subdev2.subdev_ver) - 1);
    strncpy((char *)m_subdev2.subdev_type, (char *)m_test_subdev_type2, sizeof(m_subdev2.subdev_type) - 1);

    return 0;
}