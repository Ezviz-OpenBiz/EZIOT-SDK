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
 * 2021-07-21     xurongjun
 *******************************************************************************/

#include <stdlib.h>
#include <flashdb.h>
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
void ut_online_bind();
void ut_online_contact_bind();

UTEST_TC_EXPORT(ut_online_bind, NULL, NULL, DEFAULT_TIMEOUT_S);
UTEST_TC_EXPORT(ut_online_contact_bind, NULL, NULL, DEFAULT_TIMEOUT_S);

static int32_t ez_recv_msg_cb(ez_iot_cloud2dev_msg_type_t msg_type, void *data, int len);
static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len);
static int dev_event_waitfor(int event_id, int time_ms);
static int dev_contact_bind_waitfor(int32_t *challenge_code, int time_ms);

static int m_event_id = -1;
static int m_challenge_code = -1;
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

void ut_online_bind()
{
    ez_iot_srv_info_t lbs_addr_null = {NULL, 8666};
    ez_iot_kv_callbacks_t kv_cbs_null = {0};
    int8_t *dev_token = (int8_t *)"1111111";

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_start());
    uassert_int_equal(ez_errno_succ, ez_iot_binding(dev_token));
    uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
    hal_thread_sleep(3000);
    uassert_int_equal(ez_errno_succ, ez_iot_stop());
    ez_iot_fini();
}

void ut_online_contact_bind()
{
    int32_t challenge_code = 0;

    uassert_int_equal(ez_errno_succ, ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs));
    uassert_int_equal(ez_errno_succ, ez_iot_start());
    uassert_int_equal(ez_errno_succ, dev_event_waitfor(ez_iot_event_online, DEFAULT_TIMEOUT_S * 1000));
    uassert_int_equal(ez_errno_succ, dev_contact_bind_waitfor(&challenge_code, DEFAULT_TIMEOUT_S * 1000 * 100));

    hal_thread_sleep(3000);
    uassert_int_equal(ez_errno_succ, ez_iot_contact_binding(challenge_code));

    hal_thread_sleep(3000);
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

        hal_thread_sleep(10);
        index += 10;
    } while (index < time_ms);

    return ret;
}

static int dev_contact_bind_waitfor(int32_t *challenge_code, int time_ms)
{
    int ret = -1;
    int index = 0;
    m_challenge_code = -1;

    do
    {
        if (-1 != m_challenge_code)
        {
            *challenge_code = m_challenge_code;
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
    ez_iot_challenge_t *challenge = NULL;

    switch (msg_type)
    {
    case ez_iot_msg_contact_binding:
        challenge = (ez_iot_challenge_t *)data;
        m_challenge_code = challenge->challenge_code;
        break;

    default:
        break;
    }

    return 0;
}

static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len)
{
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