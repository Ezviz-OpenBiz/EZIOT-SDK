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
#include "ez_iot_def.h"
#include <time.h>
#include <sys/time.h>
#include "uuid/uuid.h"
#include "ez_iot_tsl.h"
#include "ez_iot_tsl_adapter.h"
#include "ezdev_sdk_kernel.h"
#include "tsl_info_check.h"

#ifdef COMPONENT_TSL_CONN_ENABLE
#include "ez_iot_tsl_conn.h"
#endif

#ifdef EZ_IOT_SDK
#include "ezconfig.h"
#include "mcuconfig.h"
#include "ez_iot_shadow.h"
#else
#include "ezDevSDK_shadow.h"
#include "ezDevSDK_base_function.h"
#include "ezDevSDK_base_function_def.h"
#include "ez_model.h"
#include "ez_model_def.h"
#define ez_iot_shadow_value_t ezDevSDK_Shadow_value_t
#define ez_iot_shadow_module_t ezDevSDK_Shadow_DomainReg_V3_S
#define ez_iot_shadow_business2dev_param_t ezDevSDK_Shadow_ParseSync_Param_S
#define ez_iot_shadow_business2cloud_param_t ezDevSDK_Shadow_GenValue_Param_S
#define ez_iot_shadow_res_t ezDevSDK_Shadow_Res_t
#define ez_iot_shadow_business_t ezDevSDK_Shadow_Service_S_v3
#define ez_iot_shadow_register_v3 ezDevSDK_Shadow_DomainDereg_v3
#define ez_iot_shadow_push_v3 ezDevSDK_Shadow_PushStatus_v3
#endif

static int8_t g_tsl_is_inited = 0;

static ez_err_e ez_iot_tsl_reg_self(void);

static bool make_event_value(const tsl_value_t *value, tsl_value_t *tsl_value);

ez_err_e ez_iot_tsl_init(tsl_things_callbacks_t *ptsl_cbs)
{
    ez_log_w(TAG_TSL, "init");

    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_tsl_not_init);
    CHECK_COND_RETURN(0 != g_tsl_is_inited, ez_errno_succ);

    CHECK_COND_RETURN(NULL == ptsl_cbs, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(NULL == ptsl_cbs->action2dev, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(NULL == ptsl_cbs->property2cloud, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(NULL == ptsl_cbs->property2dev, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(0 != ez_iot_tsl_adapter_init(ptsl_cbs), ez_errno_tsl_internal);
    CHECK_COND_RETURN(0 != ez_iot_shadow_init(), ez_errno_tsl_internal);

    g_tsl_is_inited = 1;

    if (0 != ez_iot_tsl_reg_self())
    {
        ez_log_e(TAG_TSL, "tsl reg self");

        ez_iot_shadow_fini();
        ez_iot_tsl_adapter_deinit();
        g_tsl_is_inited = 0;
        return ez_errno_tsl_memory;
    }

#ifdef COMPONENT_TSL_CONN_ENABLE
    ez_iot_tsl_conn_init();
#endif

    return ez_errno_succ;
}

ez_err_e ez_iot_tsl_property_report(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, const tsl_value_t *value)
{
    ez_log_d(TAG_TSL, "property report");

    ez_iot_shadow_res_t shadow_res = {0};
    ez_iot_shadow_value_t shadow_value = {0};
    ez_iot_shadow_value_t *pvalue = NULL;
    int8_t res_type[32] = {0};
    tsl_rsc_info_t _rsc_info;

    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_tsl_not_init);
    CHECK_COND_RETURN(0 == g_tsl_is_inited, ez_errno_tsl_not_init);
    CHECK_COND_RETURN(!ez_iot_is_ready(), ez_errno_tsl_not_ready);
    CHECK_COND_RETURN(NULL == sn, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(NULL == rsc_info, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(NULL == key_info, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(NULL == key_info->domain, ez_errno_tsl_param_invalid);

    _rsc_info.local_index = (rsc_info->local_index) ? rsc_info->local_index : (int8_t *)"0";
    _rsc_info.res_type = (rsc_info->res_type) ? rsc_info->res_type : res_type;
    tsl_find_property_rsc_by_keyinfo(sn, key_info, res_type, sizeof(res_type));

    /* Legality check */
    ez_err_e rv = ez_iot_tsl_property_value_legal(sn, &_rsc_info, key_info, value);
    ez_log_w(TAG_TSL, "rv:%08x", rv);
    CHECK_COND_RETURN(rv, rv);

    if (NULL != value)
    {
        shadow_value.length = value->size;
        shadow_value.type = value->type;
        switch (value->type)
        {
        case tsl_data_type_bool:
        case tsl_data_type_int:
            shadow_value.value_int = value->value_int;
            ez_log_d(TAG_TSL, "tsl value:%d", value->value_int);
            break;
        case tsl_data_type_double:
            shadow_value.value_double = value->value_double;
            ez_log_d(TAG_TSL, "tsl value:%lf", value->value_double);
            break;
        case tsl_data_type_string:
        case tsl_data_type_array:
        case tsl_data_type_object:
            shadow_value.value = value->value;
            ez_log_d(TAG_TSL, "tsl value:%s", (char *)value->value);
            break;
        default:
            CHECK_COND_RETURN(1, ez_errno_tsl_param_invalid);
            break;
        }

        pvalue = &shadow_value;
    }

    strncpy((char *)shadow_res.dev_serial, (char *)sn, sizeof(shadow_res.dev_serial) - 1);
    strncpy((char *)shadow_res.res_type, (char *)res_type, sizeof(shadow_res.res_type) - 1);
    shadow_res.local_index = atoi((char *)_rsc_info.local_index);

    CHECK_COND_RETURN(ez_iot_shadow_push_v3(&shadow_res, key_info->domain, key_info->key, pvalue), ez_errno_tsl_internal);

#ifdef COMPONENT_TSL_CONN_ENABLE
    conn_dev_info conn_info = {0};
    if (NULL != value)
    {
        switch (value->type)
        {
        case tsl_data_type_bool:
            conn_info.data.data_num = value->value_bool;
            break;
        case tsl_data_type_int:
            conn_info.data.data_num = value->value_int;
            break;
        case tsl_data_type_double:
            conn_info.data.data_num = value->value_double;
            break;
        case tsl_data_type_string:
            strncpy((char *)conn_info.data.data_str, (char *)value->value, sizeof(conn_info.data.data_str) - 1);
            break;
        case tsl_data_type_array:
            strncpy((char *)conn_info.data.data_str, (char *)value->value, sizeof(conn_info.data.data_str) - 1);
            break;
        case tsl_data_type_object:
            strncpy((char *)conn_info.data.data_str, (char *)value->value, sizeof(conn_info.data.data_str) - 1);
            break;

        default:
            break;
        }
    }

    strncpy(conn_info.dev_sn, (char *)sn, sizeof(conn_info.dev_sn) - 1);
    strncpy(conn_info.domain_identifier, (char *)key_info->domain, sizeof(conn_info.domain_identifier) - 1);
    strncpy(conn_info.identifier, (char *)key_info->key, sizeof(conn_info.identifier) - 1);
    ez_iot_tsl_check_conn_do(&conn_info);
#endif

    return ez_errno_succ;
}

ez_err_e ez_iot_tsl_event_report(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, const tsl_value_t *value)
{
    ez_log_d(TAG_TSL, "event report");
    tsl_value_t tsl_value = {0};
    int8_t res_type[32] = {0};
    tsl_rsc_info_t _rsc_info;

    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_tsl_not_init);
    CHECK_COND_RETURN(0 == g_tsl_is_inited, ez_errno_tsl_not_init);
    CHECK_COND_RETURN(!ez_iot_is_ready(), ez_errno_tsl_not_ready);
    CHECK_COND_RETURN(NULL == sn, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(NULL == rsc_info, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(NULL == key_info, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(NULL == key_info->domain, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(!make_event_value(value, &tsl_value), ez_errno_tsl_memory);

    _rsc_info.local_index = (rsc_info->local_index) ? rsc_info->local_index : (int8_t *)"0";
    _rsc_info.res_type = (rsc_info->res_type) ? rsc_info->res_type : res_type;
    tsl_find_event_rsc_by_keyinfo(sn, key_info, res_type, sizeof(res_type));

    /* Legality check */
    ez_err_e rv = ez_iot_tsl_event_value_legal(sn, &_rsc_info, key_info, &tsl_value);
    CHECK_COND_RETURN(rv, rv);

#ifdef EZ_IOT_SDK
    ezdev_sdk_kernel_pubmsg_v3 pubmsg = {0};

    pubmsg.msg_response = 0;
    pubmsg.msg_qos = QOS_T1;
    pubmsg.msg_seq = 0;
    pubmsg.msg_body_len = tsl_value.size;
    pubmsg.extend_id = sdk_event;
    pubmsg.msg_body = tsl_value.value;
    strncpy(pubmsg.command_ver, "3.0", sizeof(pubmsg.command_ver) - 1);
    strncpy(pubmsg.domain_id, (char *)key_info->domain, sizeof(pubmsg.domain_id) - 1);
    strncpy(pubmsg.resource_id, (char *)_rsc_info.local_index, sizeof(pubmsg.resource_id) - 1);
    strncpy(pubmsg.resource_type, (char *)_rsc_info.res_type, sizeof(pubmsg.resource_type) - 1);
    strncpy(pubmsg.business_type, "report", sizeof(pubmsg.business_type) - 1);
    strncpy(pubmsg.identifier, (char *)key_info->key, sizeof(pubmsg.identifier) - 1);

    if (0 == strcmp((char *)sn, (char *)ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial")))
    {
        strncpy(pubmsg.sub_serial, "global", sizeof(pubmsg.sub_serial) - 1);
    }
    else
    {
        strncpy(pubmsg.sub_serial, (char *)sn, sizeof(pubmsg.sub_serial) - 1);
    }

    CHECK_COND_DONE(ezdev_sdk_kernel_send_v3(&pubmsg), ez_errno_tsl_internal);
#else
    ez_basic_info msg_param = {0};
    ez_model_msg msg = {0};
    ez_msg_attr msg_attr = {0};

    msg_param.type = ez_event;

    strncpy(msg_param.identifier, (char *)key_info->key, EZ_COMMON_LEN - 1);
    strncpy(msg_param.domain, (char *)key_info->domain, EZ_COMMON_LEN - 1);
    strncpy(msg_param.resource_id, "0", EZ_RES_ID_LEN - 1);
    strncpy(msg_param.resource_type, "global", EZ_RES_TYPE_LEN - 1);

    msg.value = value->value;
    msg.type = value->type;

    msg_attr.msg_qos = 0;
    msg_attr.msg_seq = 0;
    strncpy(msg_attr.msg_type, "report", EZ_MSG_TYPE_LEN - 1);

    CHECK_COND_RETURN(ez_model_send_msg(&msg_param, &msg, &msg_attr), ez_errno_tsl_internal);
#endif

#ifdef COMPONENT_TSL_CONN_ENABLE
    conn_dev_info conn_info = {0};
    strncpy(conn_info.dev_sn, (char *)sn, sizeof(conn_info.dev_sn) - 1);
    strncpy(conn_info.domain_identifier, (char *)key_info->domain, sizeof(conn_info.domain_identifier) - 1);
    strncpy(conn_info.identifier, (char *)key_info->key, sizeof(conn_info.identifier) - 1);
    ez_iot_tsl_check_conn_do(&conn_info);
#endif

done:
    SAFE_FREE(tsl_value.value);

    return ez_errno_succ;
}

void ez_iot_tsl_set_profile_url(char *dev_sn, char *url, char *md5, int expire)
{
    ez_iot_tsl_adapter_profile_url(dev_sn, url, md5, expire);
}

ez_err_e ez_iot_tsl_action_report(const tsl_key_info_t *key_info, const tsl_devinfo_t *dev_info, tsl_value_t *data)
{
    return ez_errno_succ;
}

ez_err_e ez_iot_tsl_check_value_legal(const char *key, int type, const tsl_devinfo_t *dev_info, tsl_value_t *value)
{
    if (!g_tsl_is_inited)
    {
        ez_log_w(TAG_TSL, "tsl not inited.");
        return -1;
    }
#ifdef EZ_IOT_TSL_NEED_SCHEMA
    return check_value_legal(key, type, dev_info, value);
#endif
    return 0;
}
ez_err_e ez_iot_tsl_deinit(void)
{
    CHECK_COND_RETURN(0 == g_tsl_is_inited, ez_errno_tsl_not_init);
    ez_iot_shadow_fini();
    ez_iot_tsl_adapter_deinit();

#ifdef COMPONENT_TSL_CONN_ENABLE
    ez_iot_tsl_conn_finit();
#endif

    g_tsl_is_inited = 0;

    return ez_errno_succ;
}

ez_err_e ez_iot_tsl_reg(tsl_devinfo_t *pevinfo)
{
    ez_log_w(TAG_TSL, "reg");

    CHECK_COND_RETURN(0 == g_tsl_is_inited, ez_errno_tsl_not_init);
    CHECK_COND_RETURN(NULL == pevinfo, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(NULL == pevinfo->dev_subserial, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(NULL == pevinfo->dev_type, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(NULL == pevinfo->dev_firmwareversion, ez_errno_tsl_param_invalid);

    ez_iot_tsl_adapter_add(pevinfo);

    return ez_errno_succ;
}

ez_err_e ez_iot_tsl_unreg(tsl_devinfo_t *pevinfo)
{
    ez_log_w(TAG_TSL, "unreg");

    CHECK_COND_RETURN(0 == g_tsl_is_inited, ez_errno_tsl_not_init);

    CHECK_COND_RETURN(NULL == pevinfo, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(NULL == pevinfo->dev_subserial, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(NULL == pevinfo->dev_type, ez_errno_tsl_param_invalid);
    CHECK_COND_RETURN(NULL == pevinfo->dev_firmwareversion, ez_errno_tsl_param_invalid);

    ez_iot_tsl_adapter_del(pevinfo);

    return ez_errno_succ;
}

void ez_iot_tsl_checkupdate()
{
    // 强制更新profile文件，清空无效文件
}

static ez_err_e ez_iot_tsl_reg_self(void)
{
    tsl_devinfo_t pevinfo = {0};

    pevinfo.dev_subserial = (int8_t *)ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial");
    pevinfo.dev_type = (int8_t *)ezdev_sdk_kernel_getdevinfo_bykey("dev_type");
    pevinfo.dev_firmwareversion = (int8_t *)ezdev_sdk_kernel_getdevinfo_bykey("dev_firmwareversion");

    return ez_iot_tsl_reg(&pevinfo);
}

char g_time_zone[8] = {0};

static int get_format_time(char *format_time)
{
    time_t time_now = time(NULL);
    struct tm ptm_time_now = {0};

    if (NULL == localtime_r(&time_now, &ptm_time_now))
    {
        return 0;
    }

    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    int time_ms = tv.tv_usec / 1000;

    if (strlen(g_time_zone) == 0)
    {
        sprintf(format_time, "%04d-%02d-%02dT%02d:%02d:%02d.%03d+00:00", ptm_time_now.tm_year + 1900, ptm_time_now.tm_mon + 1, ptm_time_now.tm_mday,
                ptm_time_now.tm_hour, ptm_time_now.tm_min, ptm_time_now.tm_sec, time_ms);
    }
    else
    {
        sprintf(format_time, "%04d-%02d-%02dT%02d:%02d:%02d.%03d%s", ptm_time_now.tm_year + 1900, ptm_time_now.tm_mon + 1, ptm_time_now.tm_mday,
                ptm_time_now.tm_hour, ptm_time_now.tm_min, ptm_time_now.tm_sec, time_ms, g_time_zone);
    }

    return 0;
}

static bool make_event_value(const tsl_value_t *value, tsl_value_t *tsl_value)
{
    bool rv = false;

    bscJSON *js_root = NULL;
    do
    {
        bool has_basic = false;

        if (NULL == value || NULL == value->value || 0 == strlen((char *)value->value))
        {
            has_basic = false;
            js_root = bscJSON_CreateObject();
            if (NULL == js_root)
            {
                ez_log_e(TAG_TSL, "json create failed.");
                break;
            }
        }
        else
        {
            ez_log_d(TAG_TSL, "value: %s", (char *)value->value);
            js_root = bscJSON_Parse((char *)value->value);
            if (NULL == js_root)
            {
                ez_log_e(TAG_TSL, "event value illegal");
                break;
            }

            bscJSON *js_obj = NULL;
            bscJSON_ArrayForEach(js_obj, js_root)
            {
                if (0 == strcmp(js_obj->string, "basic"))
                {
                    has_basic = true;
                    break;
                }
            }
        }

        if (!has_basic)
        {
            bscJSON *js_basic = NULL;
            char format_time[32] = {0};

            ieee_uuid_t uuuuid = {0};
            char uuid[64 + 1] = {0};

            js_basic = bscJSON_CreateObject();
            if (NULL == js_basic)
            {
                ez_log_e(TAG_TSL, "json create failed.");
                break;
            }
            get_format_time(format_time);
            bscJSON_AddStringToObject(js_basic, "dateTime", format_time);

            CreateUUID(&uuuuid);
            CreateStringUUID(uuuuid, uuid);
            bscJSON_AddStringToObject(js_basic, "UUID", uuid);
            bscJSON_AddItemToObject(js_root, "basic", js_basic);
        }

        tsl_value->value = bscJSON_PrintUnformatted(js_root);
        if (NULL == tsl_value->value)
        {
            ez_log_e(TAG_TSL, "json print failed.");
            break;
        }
        tsl_value->size = strlen((char *)tsl_value->value);
        tsl_value->type = tsl_data_type_object;

        rv = true;
    } while (false);

    CJSON_SAFE_DELETE(js_root);
    return rv;
}