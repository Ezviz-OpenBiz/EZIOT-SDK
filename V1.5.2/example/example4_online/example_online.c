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
#include "ez_iot.h"
#include "ez_iot_log.h"

#ifdef RT_THREAD
#include <rtthread.h>
#include <finsh.h>
#endif

static int32_t ez_recv_msg_cb(ez_iot_cloud2dev_msg_type_t msg_type, void *data, int len);
static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len);

static ez_iot_srv_info_t m_lbs_addr = {(int8_t *)EZ_IOT_TEST_CLOUD_HOST, EZ_IOT_TEST_CLOUD_PORT};
static ez_iot_callbacks_t m_cbs = {ez_recv_msg_cb, ez_recv_event_cb};
static ez_iot_callbacks_t m_cbs_subscribe = {0};

extern kv_err_e example_kv_init(ez_iot_kv_t *default_kv);
extern kv_err_e example_kv_raw_set(const int8_t *key, int8_t *value, uint32_t length);
extern kv_err_e example_kv_raw_get(const int8_t *key, int8_t *value, uint32_t *length);
extern kv_err_e example_kv_del(const int8_t *key);
extern kv_err_e example_kv_del_by_prefix(const int8_t *key_prefix);
extern void example_kv_print(void);
extern void example_kv_deinit(void);

static ez_iot_kv_callbacks_t m_kv_cbs = {
    .ez_kv_init = example_kv_init,
    .ez_kv_raw_set = example_kv_raw_set,
    .ez_kv_raw_get = example_kv_raw_get,
    .ez_kv_del = example_kv_del,
    .ez_kv_del_by_prefix = example_kv_del_by_prefix,
    .ez_kv_print = example_kv_print,
    .ez_kv_deinit = example_kv_deinit,
};

int ez_cloud_init()
{
    int8_t uuid[64] = {0};
    snprintf(uuid, sizeof(uuid), "%s:%s", EZ_IOT_TEST_DEV_PRODUCT_KEY, EZ_IOT_TEST_DEV_NAME);

    ez_iot_dev_info_t m_dev_info = {
        .auth_mode = 1,
        .dev_verification_code = EZ_IOT_TEST_DEV_LICENSE,
        .dev_firmwareversion = EZ_IOT_TEST_DEV_FWVER,
        .dev_type = EZ_IOT_TEST_DEV_PRODUCT_KEY,
        .dev_typedisplay = EZ_IOT_TEST_DEV_DISPLAY_NAME,
        .dev_id = {0},
    };

    strncpy(m_dev_info.dev_subserial, uuid, sizeof(uuid));
    ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs);

    return 0;
}

const char *ez_cloud_get_sn()
{
    static int8_t uuid[64] = {0};
    snprintf(uuid, sizeof(uuid), "%s:%s", EZ_IOT_TEST_DEV_PRODUCT_KEY, EZ_IOT_TEST_DEV_NAME);

    return uuid;
}

const char *ez_cloud_get_ver()
{
    return EZ_IOT_TEST_DEV_FWVER;
}

const char *ez_cloud_get_type()
{
    return EZ_IOT_TEST_DEV_PRODUCT_KEY;
}

int ez_cloud_start()
{
    return ez_iot_start();
}

void ez_cloud_deint()
{
    ez_iot_stop();

    ez_iot_fini();
}

void ez_cloud_cb_subscribe(ez_iot_callbacks_t* cbs)
{
    m_cbs_subscribe.recv_event = cbs->recv_event;
    m_cbs_subscribe.recv_msg = cbs->recv_msg;
}

void ez_cloud_cb_unsubscribe()
{
    memset(&m_cbs_subscribe, 0, sizeof(m_cbs_subscribe));
}

static int32_t ez_recv_msg_cb(ez_iot_cloud2dev_msg_type_t msg_type, void *data, int len)
{
    if (m_cbs_subscribe.recv_msg)
    {
        m_cbs_subscribe.recv_msg(msg_type, data, len);
    }

    return 0;
}

static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len)
{
    switch (event_type)
    {
    case ez_iot_event_online:
        break;
    case ez_iot_event_offline:
        break;
    case ez_iot_event_devid_update:
        //TODO 需要永久固化devid，维修和恢复出厂设置都不能擦除。
        break;

    default:
        break;
    }

    if (m_cbs_subscribe.recv_event)
    {
        m_cbs_subscribe.recv_event(event_type, data, len);
    }

    return 0;
}

int example_online(int argc, char **argv)
{
    if (0 != ez_cloud_init() || 0 != ez_cloud_start())
    {
        ez_log_e(TAG_APP, "example online init err");
    }

    return 0;
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_online, run ez - iot - sdk example online);
#else
// int main(int argc, char **argv)
// {
//     return example_kv(argc, argv);
// }
#endif