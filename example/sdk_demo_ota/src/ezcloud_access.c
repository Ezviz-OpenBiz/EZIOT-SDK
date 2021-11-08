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
#include "ezcloud_access.h"
#include "kv_imp.h"

// ez cloud info
// =================================================
#define EZ_IOT_CLOUD_ENTRY_HOST "test15.ys7.com"
#define EZ_IOT_CLOUD_ENTRY_PORT 8666
// =================================================

// ez iot device info
// =================================================
#define EZ_IOT_DEV_AUTH_MODE 1
#define EZ_IOT_DEV_UUID "4LYV8SK7UKLBOUOVS6HXVX:AFDH8DRTL728"
#define EZ_IOT_DEV_LICENSE "3tYndbcr4hNYdkfn27ZKAG"
#define EZ_IOT_DEV_PRODUCT_KEY "4LYV8SK7UKLBOUOVS6HXVX"
#define EZ_IOT_DEV_NAME "AFDH8DRTL728"
#define EZ_IOT_DEV_DISPLAY_NAME "IOT_UTEST_DEV"
#define EZ_IOT_DEV_ID ""
#define EZ_IOT_DEV_FWVER "V1.2.0 build 201212"
// =================================================

static int32_t ez_recv_msg_cb(ez_iot_cloud2dev_msg_type_t msg_type, void *data, int len);
static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len);

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

int ez_cloud_init()
{
    ez_iot_log_init();
    ez_iot_log_start();
    ez_iot_log_filter_lvl(4);

    ez_iot_init(&m_lbs_addr, &m_dev_info, &m_cbs, &m_kv_cbs);

    return 0;
}

const char *ez_cloud_get_sn()
{
    return EZ_IOT_DEV_UUID;
}

const char *ez_cloud_get_ver()
{
    return EZ_IOT_DEV_FWVER;
}

const char *ez_cloud_get_type()
{
    return EZ_IOT_DEV_PRODUCT_KEY;
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

static int32_t ez_recv_msg_cb(ez_iot_cloud2dev_msg_type_t msg_type, void *data, int len)
{
  return 0;
}

static int32_t ez_recv_event_cb(ez_iot_event_t event_type, void *data, int len)
{
  switch (event_type)
  {
  case ez_iot_event_online:
    break;
  case ez_iot_event_devid_update:
    /* save devid */
    break;

  default:
    break;
  }

  return 0;
}