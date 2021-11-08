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
#include "ez_iot_tsl_adapter.h"
#include <float.h>
#include <limits.h>
#include <math.h>
#include "mbedtls/md5.h"
#include "base64/base64.h"
#include "profile_parse.h"
#include "ezdev_sdk_kernel.h"
#include "webclient.h"
#include "ez_iot_errno.h"
#include "tsl_info_check.h"

#ifdef HUOMAN_TRANSLATE_COMPAT
extern char *get_lic_productKey();
#define HUOMAN_LIC_PTID "54528FB7E4C44EFCAD9E10"
#define HUOMAN_LIC_PTID_TEST "64528FB7E4C44EFCAD9E10"
#endif

#ifdef EZ_IOT_SDK
#include "ezconfig.h"
#include "mcuconfig.h"
#include "ez_iot_shadow.h"
#else
#include "ezDevSDK_shadow.h"
#include "ezDevSDK_base_function.h"
#include "ezDevSDK_base_function_def.h"
#define ez_iot_shadow_value_t ezDevSDK_Shadow_value_t
#define ez_iot_shadow_module_t ezDevSDK_Shadow_DomainReg_V3_S
#define ez_iot_shadow_business2dev_param_t ezDevSDK_Shadow_ParseSync_Param_S
#define ez_iot_shadow_business2cloud_param_t ezDevSDK_Shadow_GenValue_Param_S
#define ez_iot_shadow_res_t ezDevSDK_Shadow_Res_t
#define ez_iot_shadow_business_t ezDevSDK_Shadow_Service_S_v3
#define ez_iot_shadow_register_v3 ezDevSDK_Shadow_DomainReg_v3
#define ez_iot_is_online ezdev_get_register_status
#endif

#ifdef EZ_IOT_TSL_USE_PRIME
#include "ez_model.h"
#include "ez_model_def.h"
#endif

#ifdef COMPONENT_TSL_CONN_ENABLE
#include "ez_iot_tsl_conn.h"
#endif

#define MAX_RECV_BUFFER 128
#define PARTITION_PROFILE_LABEL "log"
#define TSLMAP_JSON_KEY_SN "sn"
#define TSLMAP_JSON_KEY_HANDLE "handle"

#define FUN_IN() ez_log_w(TAG_TSL, "%s IN", __FUNCTION__);
#define FUN_OUT() ez_log_w(TAG_TSL, "%s OUT", __FUNCTION__);

typedef struct
{
    char dev_sn[48];
    char url[260];
    char md5[33];
    int expire;
} ez_iot_download_info_t;

typedef enum
{
    status_done = 0,        ///< 描述文件完成加载
    status_need_update = 1, ///< 需要更新描述文件
    status_query = 2,       ///< 正在查询升级包信息
    status_loading = 3,     ///< 正在下载
} dev_stauts_e;

typedef struct
{
    char dev_sn[48];
    char dev_type[24];
    char dev_fw_ver[36];
    dev_stauts_e status;
    void *timer;
    void *download_info;
} dev_info_t;

typedef struct
{
    char sn[48];
    char handle[32];
} tslmap_metadata_t;

// *tsl things to dev cbs
tsl_things_callbacks_t g_tsl_things_cbs = {0};

static void *g_profile_download_thread = NULL;
static int g_profile_download_thread_running = 0;

static ezlist_t *g_tsl_dev_info_list = NULL;
static void *g_profile_mutex = NULL;
ezlist_t *g_capacities_list = NULL;
void *g_capacities_mutex = NULL;

/**
 * @brief 增加功能描述文件引用计数
 * 
 * @param dev_info 设备信息
 * @return true 成功
 * @return false 失败，找不到对应功能描述文件
 */
static bool tsl_adapter_ref_add(tsl_devinfo_t *dev_info);

/**
 * @brief 减少功能描述文件引用计数，计数为0则销毁描述文件对象
 * 
 * @param dev_info 设备信息
 * @return true 成功
 * @return false 失败，找不到功能点或者引用计数为0
 */
static bool tsl_adapter_ref_del(tsl_devinfo_t *dev_info);

/**
 * @brief 从本地加载功能描述文件
 * 
 * @param dev_info 设备信息
 * @return true 成功
 * @return false 失败，找不到对应功能描述文件或文件损坏
 */
static bool tsl_adapter_profile_load(tsl_devinfo_t *dev_info);

/**
 * @brief 保存功能描述文件
 * 
 * @param dev_info 设备信息
 * @param profile 功能点文件数据，如果为空表示只直接保存map
 * @param length 功能点文件数据长度
 * @return true 成功
 * @return false 失败，找不到对应功能描述文件或文件损坏
 */
static bool tsl_adapter_profile_save(tsl_devinfo_t *dev_info, const char *profile, int length);

/**
 * @brief 在线下载功能描述文件
 * 
 * @param dev_info 设备信息
 * @return true 成功
 * @return false 失败，资源不足或内部错误
 */
static bool tsl_adapter_profile_download(tsl_devinfo_t *dev_info);

/**
 * @brief 设备能力集列表中新增一项
 * 
 * @param capacity 设备能力集
 * @return int32_t 
 */
static bool tsl_adapter_capacity_add(ez_iot_tsl_capacity_t *capacity);

/**
 * @brief 根据能力集注册shadow
 * 
 * @param dev_info 设备信息
 * @param capacity 设备能力集
 */
static void tsl_adapter_shadow_inst(tsl_devinfo_t *dev_info, ez_iot_tsl_capacity_t *capacity);

/**
 * @brief 根据能力集注册原语组件
 * 
 * @param dev_info 设备信息
 * @param capacity 设备能力集
 */
static void tsl_adapter_prime_inst(tsl_devinfo_t *dev_info, ez_iot_tsl_capacity_t *capacity);

/**
 * @brief 增加一个设备
 * 
 * @param dev_info 设备信息
 * @param need_get 是否需要下载更新
 */
static bool tsl_adapter_dev_add(tsl_devinfo_t *dev_info, bool need_get);

/**
 * @brief 更新设备状态
 * 
 * @param dev_info 设备信息
 * @param status 设备状态
 */
static void tsl_adapter_dev_update(tsl_devinfo_t *dev_info, dev_stauts_e status);

/**
 * @brief 删除一个设备
 * 
 * @param dev_info 设备信息
 */
static void tsl_adapter_dev_del(tsl_devinfo_t *dev_info);

/**
 * @brief 查询设备信息
 * 
 * @param dev_info 设备信息
 * @param status 设备状态
 */
static bool tsl_adapter_dev_find(const int8_t *sn, tsl_devinfo_t *dev_info, dev_stauts_e *status);

/**
 * @brief 启动功能点下载任务
 * 
 * @return true 
 * @return false 
 */
static bool tsl_adapter_dl_task_yeild();

/**
 * @brief 设备信息文件查找索引
 * 
 * @param dev_info 
 * @param index 
 */
void devinfo2index(tsl_devinfo_t *dev_info, char index[32]);

/**
 * @brief 下载
 * 
 * @param dev_info 下载链接
 * @param buf 下载缓存
 * @param length 描述文件大小
 * @return int 
 */
static int profile_downloading(dev_info_t *dev_info, char **buf, int *length);

static int32_t domain_and_hub_register(const char *str_buss, sdk_kernel_extend_id extend_id);

static char *assemble_rsp_code_msg(int code, tsl_value_t *value_out)
{
    bscJSON *js_root = NULL;
    char *rv = NULL;

    do
    {
        js_root = bscJSON_CreateObject();
        if (NULL == js_root)
        {
            ez_log_e(TAG_TSL, "json create object failed.");
            break;
        }

        bscJSON_AddNumberToObject(js_root, "code", code);

        if (NULL != value_out)
        {
            bscJSON_AddRawToObject(js_root, "data", (char *)value_out->value);
        }

        rv = bscJSON_PrintUnformatted(js_root);
        if (NULL == rv)
        {
            ez_log_e(TAG_TSL, "json print error.");
            break;
        }
    } while (false);

    bscJSON_Delete(js_root);

    return rv;
}

static tsl_data_type_e json_type_transform_dev(int type)
{
    tsl_data_type_e ret_type = tsl_data_type_max;
    switch (type)
    {
    case bscJSON_False:
    case bscJSON_True:
        ret_type = tsl_data_type_bool;
        break;
    case bscJSON_NULL:
        break;
    case bscJSON_Number:
        ret_type = tsl_data_type_int;
        break;
    case bscJSON_String:
        ret_type = tsl_data_type_string;
        break;

    case bscJSON_Array:
        ret_type = tsl_data_type_array;
        break;

    case bscJSON_Object:
        ret_type = tsl_data_type_object;
        break;
    default:
        break;
    }

    return ret_type;
}

static void strip_msg_wrap(void *buf, tsl_value_t *tsl_data)
{
    bscJSON *js_msg = NULL;
    bscJSON *js_data = NULL;

    do
    {
        js_msg = bscJSON_Parse((char *)buf);
        if (NULL == js_msg)
        {
            ez_log_e(TAG_TSL, "msg parse: %s", buf);
            break;
        }

        js_data = bscJSON_GetObjectItem(js_msg, "data");
        if (NULL == js_data)
        {
            ez_log_e(TAG_TSL, "msg format error: %s", (char *)buf);
            break;
        }

        tsl_data->type = json_type_transform_dev(js_data->type);

        switch (tsl_data->type)
        {
        case tsl_data_type_bool:
            tsl_data->size = sizeof(bool);
            tsl_data->value_bool = (bool)js_data->valueint;
            break;
        case tsl_data_type_int:
            tsl_data->value_int = js_data->valueint;
            tsl_data->size = sizeof(int);
            break;
        case tsl_data_type_double:
            tsl_data->size = sizeof(double);
            tsl_data->value_double = js_data->valuedouble;
            break;
        case tsl_data_type_string:
            tsl_data->value = (char *)malloc(strlen(js_data->valuestring) + 1);
            if (NULL == tsl_data->value)
            {
                break;
            }

            tsl_data->size = strlen(js_data->valuestring);
            strcpy(tsl_data->value, js_data->valuestring);
            break;
        case tsl_data_type_array:
        case tsl_data_type_object:
            tsl_data->value = bscJSON_PrintUnformatted(js_data); //此接口申请新的内存
            if (NULL == tsl_data->value)
            {
                break;
            }

            tsl_data->size = strlen(tsl_data->value);
            // tsl_data->value = (char *)malloc(tsl_data->size + 1);
            //strcpy(tsl_data->value, js_data->valuestring);
            break;

        default:
            break;
        }
    } while (0);

    bscJSON_Delete(js_msg);
}

static int tsl_action_process(ezdev_sdk_kernel_submsg_v3 *submsg)
{
    int rv = 0;
    char *rsp_buf = NULL;
    tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)submsg->resource_type, .local_index = (int8_t *)submsg->resource_id};
    tsl_key_info_t key_info = {.domain = (int8_t *)submsg->domain_id, .key = (int8_t *)submsg->identifier};
    tsl_value_t value_in = {0};
    value_in.type = tsl_data_type_null;
    tsl_value_t value_out = {0};
    value_out.type = tsl_data_type_null;
    ezdev_sdk_kernel_pubmsg_v3 pubmsg = {0};

    do
    {
        ez_log_w(TAG_TSL, "seq in: %d", submsg->msg_seq);
        ez_log_d(TAG_TSL, "recv msg: %s", (char *)submsg->buf);

        strip_msg_wrap(submsg->buf, &value_in);

        rv = g_tsl_things_cbs.action2dev((const int8_t *)submsg->sub_serial, &rsc_info, &key_info, &value_in, &value_out);
        if (0 != rv)
        {
            ez_log_e(TAG_TSL, "action2dev got err. rv = %d", rv);
        }

        rsp_buf = assemble_rsp_code_msg(rv, &value_out);
        if (NULL == rsp_buf)
        {
            ez_log_e(TAG_TSL, "assemble rsp code failed.");
            break;
        }

        pubmsg.msg_response = 1;
        pubmsg.msg_qos = QOS_T1;
        pubmsg.msg_seq = submsg->msg_seq;
        pubmsg.msg_body_len = strlen(rsp_buf);
        pubmsg.extend_id = sdk_service;
        pubmsg.msg_body = (unsigned char *)rsp_buf;

        strncpy(pubmsg.domain_id, (char *)submsg->domain_id, sizeof(pubmsg.domain_id) - 1);
        strncpy(pubmsg.resource_id, (char *)submsg->resource_id, sizeof(pubmsg.resource_id) - 1);
        strncpy(pubmsg.resource_type, (char *)submsg->resource_type, sizeof(pubmsg.resource_type) - 1);
        strncpy(pubmsg.business_type, "operate_reply", sizeof(pubmsg.business_type) - 1);
        strncpy(pubmsg.identifier, (char *)submsg->identifier, sizeof(pubmsg.identifier) - 1);
        strncpy(pubmsg.sub_serial, (char *)submsg->sub_serial, sizeof(pubmsg.sub_serial) - 1);

        rv = ezdev_sdk_kernel_send_v3(&pubmsg);
        if (ez_errno_succ != rv)
        {
            ez_log_e(TAG_TSL, "sdk kernel send v3 failed.");
        }
    } while (false);

    SAFE_FREE(rsp_buf);

    if (tsl_data_type_string == value_in.type || tsl_data_type_array == value_in.type || tsl_data_type_object == value_in.type)
    {
        SAFE_FREE(value_in.value);
    }

    if (tsl_data_type_string == value_out.type || tsl_data_type_array == value_out.type || tsl_data_type_object == value_out.type)
    {
        SAFE_FREE(value_out.value);
    }

    return rv;
}

static int32_t business2dev_imp(const ez_iot_shadow_value_t *pvalue, ez_iot_shadow_business2dev_param_t *ppram)
{
    int32_t rv = -1;
    ez_iot_shadow_res_t *shadow_res = (ez_iot_shadow_res_t *)ppram->pres;
    tsl_value_t tsl_value = {0};
    char local_index[8] = {0};
    snprintf(local_index, sizeof(local_index), "%d", shadow_res->local_index);

    tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)shadow_res->res_type, .local_index = (int8_t *)local_index};
    tsl_key_info_t key_info = {.domain = (int8_t *)ppram->pdomain, .key = (int8_t *)ppram->pkey};
    tsl_value.type = pvalue->type;
    tsl_value.size = pvalue->length;
    tsl_value.value_double = pvalue->value_double;

#ifdef COMPONENT_TSL_CONN_ENABLE
    if (0 == strcmp((char *)ppram->pkey, "rule_add") || 0 == strcmp((char *)ppram->pkey, "rule_del"))
    {
        ez_log_i(TAG_TSL, "conn rule to dev. key: %s", ppram->pkey);
        rv = ez_iot_tsl_conn_info_process(ppram->pkey, pvalue->value);
        if (0 != rv)
        {
            ez_log_e(TAG_TSL, "conn info process failed.");
        }
        return rv;
    }
#endif
    rv = g_tsl_things_cbs.property2dev(shadow_res->dev_serial, &rsc_info, &key_info, &tsl_value);
    if (0 != rv)
    {
        ez_log_e(TAG_TSL, "dev process failed.");
    }

    return rv;
}

static int32_t business2cloud_imp(ez_iot_shadow_value_t *pvalue, ez_iot_shadow_business2cloud_param_t *ppram)
{
    int32_t rv = -1;
    ez_iot_shadow_res_t *shadow_res = (ez_iot_shadow_res_t *)ppram->pres;
    tsl_value_t *tsl_value = (tsl_value_t *)pvalue;
    char local_index[8] = {0};
    snprintf(local_index, sizeof(local_index), "%d", shadow_res->local_index);

    tsl_rsc_info_t rsc_info = {.res_type = (int8_t *)shadow_res->res_type, .local_index = (int8_t *)local_index};
    tsl_key_info_t key_info = {.domain = (int8_t *)ppram->pdomain, .key = (int8_t *)ppram->pkey};

    ez_log_d(TAG_TSL, "busi to cloud. dev_sn: %s", shadow_res->dev_serial);
    rv = g_tsl_things_cbs.property2cloud((const int8_t *)shadow_res->dev_serial, &rsc_info, &key_info, tsl_value);
    if (0 != rv)
    {
        ez_log_e(TAG_TSL, "property2cloud failed.");
    }

#ifdef COMPONENT_TSL_CONN_ENABLE
    conn_dev_info conn_info = {0};
    switch (tsl_value->type)
    {
    case tsl_data_type_bool:
    case tsl_data_type_int:
    case tsl_data_type_double:
        conn_info.data.data_num = tsl_value->value_bool;
        break;
    case tsl_data_type_string:
    case tsl_data_type_array:
    case tsl_data_type_object:
        strncpy((char *)conn_info.data.data_str, (char *)tsl_value->value, sizeof(conn_info.data.data_str) - 1);
        break;

    default:
        break;
    }

    if (0 == strcmp((char *)shadow_res->dev_serial, "global"))
    {
        strncpy(conn_info.dev_sn, ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial"), sizeof(conn_info.dev_sn) - 1);
    }
    else
    {
        strncpy(conn_info.dev_sn, (char *)shadow_res->dev_serial, sizeof(conn_info.dev_sn) - 1);
    }

    strncpy(conn_info.domain_identifier, (char *)ppram->pdomain, sizeof(conn_info.domain_identifier) - 1);
    strncpy(conn_info.identifier, (char *)ppram->pkey, sizeof(conn_info.identifier) - 1);
    ez_iot_tsl_check_conn_do(&conn_info);
#endif

    return rv;
}

static void tsl_adapter_shadow_inst(tsl_devinfo_t *dev_info, ez_iot_tsl_capacity_t *capacity)
{
    int rsc_num = capacity->rsc_num;
    ez_log_w(TAG_TSL, "resource num: %d", rsc_num);

    for (size_t i = 0; i < rsc_num; i++)
    {
        int index_num = capacity->resource[i].index_num;
        ez_log_w(TAG_TSL, "index num: %d", index_num);

        for (size_t j = 0; j < index_num; j++)
        {
            int domain_num = capacity->resource[i].domain_num;
            ez_log_w(TAG_TSL, "domain num: %d", domain_num);
            for (size_t k = 0; k < domain_num; k++)
            {
                int prop_num = capacity->resource[i].domain[k].prop_num;
                ez_log_w(TAG_TSL, "prop num: %d", prop_num);
                for (size_t l = 0; l < prop_num; l++)
                {
                    tsl_domain_prop *prop = capacity->resource[i].domain[k].prop + l;

                    ez_iot_shadow_business_t shadow_busi = {0};
                    memset(&shadow_busi, 0, sizeof(ez_iot_shadow_business_t));
                    shadow_busi.business2cloud = (prop->access & ACCESS_READ) ? business2cloud_imp : NULL;
                    shadow_busi.business2dev = (prop->access & ACCESS_WRITE) ? business2dev_imp : NULL;
                    strncpy((char *)shadow_busi.key, prop->identifier, sizeof(shadow_busi.key) - 1);

                    ez_iot_shadow_res_t shadow_res = {0};
                    ez_iot_shadow_module_t shadow_module = {0};
                    strncpy((char *)shadow_res.dev_serial, (char *)dev_info->dev_subserial, sizeof(shadow_res.dev_serial) - 1);
                    strncpy((char *)shadow_res.res_type, (char *)capacity->resource[i].rsc_category, sizeof(shadow_res.res_type) - 1);
                    shadow_res.local_index = atoi(capacity->resource[i].index + MAX_LOCAL_INDEX_LENGTH * j);

                    shadow_module.num = 1;
                    shadow_module.reset = 0;
                    shadow_module.business = &shadow_busi;
                    int ret = ez_iot_shadow_register_v3(&shadow_res, (int8_t *)capacity->resource[i].domain[k].identifier, &shadow_module);
                    ez_log_d(TAG_TSL, "dev_sn: %s, rsc_type: %s, domain: %s, identifier: %s", shadow_res.dev_serial, shadow_res.res_type, capacity->resource[i].domain[k].identifier, prop->identifier);
                    if (0 != ret)
                    {
                        ez_log_e(TAG_TSL, "shadow register failed.");
                        hal_thread_sleep(100);
                        continue;
                    }

                    ez_log_w(TAG_TSL, "shadow register succ. dev_sn: %s, identifier: %s", dev_info->dev_subserial, prop->identifier);
                }
            }
        }
    }
}

static void tsl_adapter_prime_inst(tsl_devinfo_t *dev_info, ez_iot_tsl_capacity_t *capacity)
{
}

struct fetch_data
{
    char *buf;
    int len;
    int code, closed;
};

static int profile_downloading(dev_info_t *dev_info, char **buf, int *length)
{
    int rv = -1;

    unsigned char md5_hex_up[16 * 2 + 1] = {0};
    unsigned char md5_hex[16 * 2 + 1] = {0};
    int already_len = 0;
    unsigned char md5_output[16] = {0};
    bscomptls_md5_context md5_ctx = {0};

    struct webclient_session *session = NULL;
    bscomptls_md5_init(&md5_ctx);
    bscomptls_md5_starts(&md5_ctx);

    session = webclient_session_create(512);
    int rsp_status = 0;
    do
    {
        ez_iot_download_info_t *download_info = (ez_iot_download_info_t *)dev_info->download_info;
        ez_log_w(TAG_TSL, "url: %s", download_info->url);
        rsp_status = webclient_get(session, download_info->url);
        if (200 != rsp_status)
        {
            ez_log_e(TAG_TSL, "webclient get request failed. http_code: %d", rsp_status);
            break;
        }

        if (0 >= session->content_length)
        {
            ez_log_e(TAG_TSL, "content length illegal: %d", session->content_length);
            break;
        }

        *buf = (char *)malloc(session->content_length + 1);
        if (NULL == *buf)
        {
            ez_log_e(TAG_TSL, "memory not enough.");
            break;
        }
        memset(*buf, 0, session->content_length + 1);

        int read_len = 0;
        do
        {
            read_len = webclient_read(session, *buf + already_len, session->content_length - already_len);
            if (0 >= read_len)
            {
                break;
            }
            if (already_len == session->content_length)
            {
                break;
            }
            already_len += read_len;

        } while (true);

        bscomptls_md5_update(&md5_ctx, (unsigned char *)(*buf), session->content_length);
        bscomptls_md5_finish(&md5_ctx, md5_output);
        bscomptls_hexdump(md5_output, sizeof(md5_output), 1, md5_hex_up);
        bscomptls_hexdump(md5_output, sizeof(md5_output), 0, md5_hex);

        if (0 != strcmp((char *)md5_hex, download_info->md5) && 0 != strcmp((char *)md5_hex_up, download_info->md5))
        {
            ez_log_e(TAG_TSL, "check_sum mismatch:%s, profile md5:%s", md5_hex, download_info->md5);
            break;
        }
        *length = session->content_length;
        rv = 0;
    } while (0);

    webclient_close(session);

    if (0 != rv)
    {
        free(*buf);
    }

    return rv;
}

/**
 * @brief 查询下载功能
 *
 * @param
 */
static void ez_profile_get_thread(void *user_data)
{
    ez_log_w(TAG_TSL, "profile thread enter.");
    dev_info_t *dev = NULL;
    tsl_devinfo_t tsl_dev = {0};
    g_profile_download_thread_running = 1;
#ifndef COMPONENT_TSL_PROFILE_STRIP
    bool need_schema = true;
#else
    bool need_schema = false;
#endif

    do
    {
        if (!g_profile_download_thread_running)
        {
            break;
        }

        if (!ez_iot_is_online())
        {
            hal_thread_sleep(500);
            continue;
        }

        hal_thread_mutex_lock(g_profile_mutex);
        dev = NULL;
        size_t obj_size = 0;
        for (size_t i = 0; i < ezlist_size(g_tsl_dev_info_list); i++)
        {
            dev = ezlist_getat(g_tsl_dev_info_list, i, NULL, false);
            if (status_done != dev->status)
            {
                dev = ezlist_getat(g_tsl_dev_info_list, i, &obj_size, true);
                break;
            }

            dev = NULL;
        }
        hal_thread_mutex_unlock(g_profile_mutex);

        if (NULL == dev)
        {
            /* 下载完成，任务退出 */
            break;
        }

        tsl_dev.dev_type = (int8_t *)dev->dev_type;
        tsl_dev.dev_firmwareversion = (int8_t *)dev->dev_fw_ver;
        tsl_dev.dev_subserial = (int8_t *)dev->dev_sn;

        if (status_loading == dev->status)
        {
            ez_log_d(TAG_TSL, "profile thread get url");
            /* 已拿到下载链接，开始下载 */
            char *buf = NULL;
            int length = 0;
            if (0 != profile_downloading(dev, &buf, &length))
            {
                free(dev);
                hal_thread_sleep(100);
                continue;
            }

            ez_log_w(TAG_TSL, "%s profile dl succ.", dev->dev_sn);

            if (!tsl_adapter_profile_save(&tsl_dev, buf, length))
            {
                free(buf);
                free(dev);
                hal_thread_sleep(100);
                continue;
            }

            ez_log_w(TAG_TSL, "%s profile save succ.", dev->dev_sn);

            if (!tsl_adapter_profile_load(&tsl_dev))
            {
                free(buf);
                free(dev);
                hal_thread_sleep(100);
                continue;
            }

            tsl_adapter_dev_update(&tsl_dev, status_done);

            ez_log_w(TAG_TSL, "%s profile load succ.", dev->dev_sn);

            free(buf);
            free(dev);
        }
        else if (status_query == dev->status && !hal_time_isexpired(dev->timer))
        {
            /* 正在查询下载链接 */
            ez_log_v(TAG_TSL, "profile thread wait url");
            free(dev);
            hal_thread_sleep(100);
        }
        else
        {
            /* 查询下载链接 */
#ifdef EZ_IOT_SDK
            if (0 == dev2cloud_query_profile_url_req(dev->dev_sn, dev->dev_type, dev->dev_fw_ver, need_schema))
#else
            ezDevSDK_base_function_profile_query_info query_info = {0};
            ezDevSDK_base_function_msg msg = {EZDEVSDK_BASE_FUNC_QUERY_PROFILE, &query_info};
            strncpy(query_info.dev_serial, dev->dev_sn, sizeof(query_info.dev_serial) - 1);
            strncpy(query_info.dev_mode, dev->dev_type, sizeof(query_info.dev_mode) - 1);
            strncpy(query_info.dev_version, dev->dev_fw_ver, sizeof(query_info.dev_version) - 1);
            query_info.need_schema = need_schema;

            ez_log_i(TAG_TSL, "query_info.dev_serial: %s", query_info.dev_serial);
            ez_log_i(TAG_TSL, "query_info.dev_mode: %s", query_info.dev_mode);
            ez_log_i(TAG_TSL, "query_info.dev_version: %s", query_info.dev_version);
            if (0 == ezDevSDK_base_function_send_msg(&msg))
#endif
            {
                ez_log_d(TAG_TSL, "profile thread status_query");
                tsl_adapter_dev_update(&tsl_dev, status_query);
            }
            else
            {
                tsl_adapter_dev_update(&tsl_dev, status_need_update);
            }

            free(dev);
        }
    } while (1);

    g_profile_download_thread_running = 0;
#ifdef _FREE_RTOS_
    hal_thread_destroy(g_profile_download_thread);
    g_profile_download_thread = NULL;
#endif

    return;
}

int32_t ez_iot_tsl_adapter_init(tsl_things_callbacks_t *things_cbs)
{
    ez_log_w(TAG_TSL, "tsl adapter init.");
    ez_err_e rv = ez_errno_succ;

    memcpy(&g_tsl_things_cbs, things_cbs, sizeof(tsl_things_callbacks_t));

#ifdef EZ_IOT_SDK
    rv = domain_and_hub_register(KERNEL_SERVICE_DOMAIN_NAME, sdk_service);
    CHECK_COND_RETURN(rv, rv);

    rv = domain_and_hub_register(KERNEL_EVENT_DOMAIN_NAME, sdk_event);
    CHECK_COND_RETURN(rv, rv);
#else
    (void)tsl_action_process;
    (void)assemble_rsp_code_msg;
#endif

    if (NULL == g_tsl_dev_info_list)
    {
        g_tsl_dev_info_list = ezlist(ezlist_THREADSAFE);
    }

    if (NULL == g_capacities_list)
    {
        g_capacities_list = ezlist(ezlist_THREADSAFE);
    }

    g_profile_mutex = hal_thread_mutex_create();
    if (NULL == g_profile_mutex)
    {
        ez_log_e(TAG_TSL, "profile mutex create failed.");
        return -1;
    }

    g_capacities_mutex = hal_thread_mutex_create();
    if (NULL == g_capacities_mutex)
    {
        ez_log_e(TAG_TSL, "capacities mutex create failed.");
        return -1;
    }

    return 0;
}

int32_t ez_iot_tsl_adapter_add(tsl_devinfo_t *dev_info)
{
    int32_t rv = 0;

    ez_log_d(TAG_TSL, "adapter add");
    CHECK_COND_RETURN(tsl_adapter_dev_find(dev_info->dev_subserial, NULL, NULL), ez_errno_succ);

    do
    {
        /* 内存中有功能描述文件，直接增加引用计数*/
        ez_log_d(TAG_TSL, "try ram");
        if (tsl_adapter_ref_add(dev_info))
        {
            tsl_adapter_dev_add(dev_info, false);
            break;
        }

        /* 从flash中加载 */
        ez_log_d(TAG_TSL, "try flash");
        if (tsl_adapter_profile_load(dev_info))
        {
            tsl_adapter_dev_add(dev_info, false);
            break;
        }

        /* 在线下载 */
        ez_log_d(TAG_TSL, "try download");
        rv = tsl_adapter_profile_download(dev_info);
    } while (0);

    return rv;
}

int32_t ez_iot_tsl_adapter_del(tsl_devinfo_t *dev_info)
{
    ez_log_d(TAG_TSL, "adapter del, %s", dev_info->dev_subserial);
    CHECK_COND_RETURN(!tsl_adapter_dev_find(dev_info->dev_subserial, NULL, NULL), ez_errno_succ);

    tsl_adapter_ref_del(dev_info);
    tsl_adapter_dev_del(dev_info);

    return 0;
}

void ez_iot_tsl_adapter_profile_url(char *dev_sn, char *url, char *md5, int expire)
{
    ez_iot_download_info_t *download_info = NULL;
    int8_t dev_subserial[48] = {0};
    int8_t dev_type[24] = {0};
    int8_t dev_firmwareversion[36] = {0};
    tsl_devinfo_t tsl_dev = {dev_subserial, dev_type, dev_firmwareversion};

    hal_thread_mutex_lock(g_profile_mutex);
    for (size_t i = 0; i < ezlist_size(g_tsl_dev_info_list); i++)
    {
        dev_info_t *dev = ezlist_getat(g_tsl_dev_info_list, i, NULL, false);
        if (0 == strcmp(dev->dev_sn, dev_sn) && status_done != dev->status)
        {
            if (NULL != dev->download_info)
            {
                download_info = dev->download_info;
            }
            else
            {
                download_info = (void *)malloc(sizeof(ez_iot_download_info_t));
                memset(download_info, 0, sizeof(ez_iot_download_info_t));
                dev->download_info = download_info;
            }

            if (NULL == download_info)
            {
                ez_log_e(TAG_TSL, "memory not enough.");
                break;
            }

            memset((void *)download_info, 0, sizeof(ez_iot_download_info_t)),
                snprintf(download_info->url, sizeof(download_info->url) - 1, "http://%s", url);
            download_info->expire = expire;
            strncpy(download_info->md5, md5, sizeof(download_info->md5) - 1);
            strncpy(download_info->dev_sn, dev_sn, sizeof(download_info->dev_sn) - 1);

            strncpy((char *)dev_subserial, (char *)dev->dev_sn, sizeof(dev_subserial) - 1);
            strncpy((char *)dev_type, (char *)dev->dev_type, sizeof(dev_type) - 1);
            strncpy((char *)dev_firmwareversion, (char *)dev->dev_fw_ver, sizeof(dev_firmwareversion) - 1);

            break;
        }
    }

    hal_thread_mutex_unlock(g_profile_mutex);

    tsl_adapter_dev_update(&tsl_dev, status_loading);
}

void ez_iot_tsl_adapter_deinit()
{
    ez_log_w(TAG_TSL, "tsl adapter deinit.");

    hal_thread_mutex_lock(g_profile_mutex);
    size_t size = ezlist_size(g_tsl_dev_info_list);
    for (size_t i = 0; i < size; i++)
    {
        ezlist_removefirst(g_tsl_dev_info_list);
    }
    hal_thread_mutex_unlock(g_profile_mutex);

    g_profile_download_thread_running = 0;
    hal_thread_destroy(g_profile_download_thread);
    g_profile_download_thread = NULL;

    hal_thread_mutex_destroy(g_profile_mutex);
    g_profile_mutex = NULL;

    hal_thread_mutex_destroy(g_capacities_mutex);
    g_capacities_mutex = NULL;
}

int32_t ez_iot_tsl_action_value_legal(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, const tsl_value_t *value)
{
    int32_t rv = ez_errno_succ;

    char dev_sn[48] = {0};
    char dev_type[24] = {0};
    char dev_fw_ver[36] = {0};
    dev_stauts_e dev_status = 0;
    tsl_devinfo_t dev_info = {.dev_subserial = (int8_t *)dev_sn, .dev_type = (int8_t *)dev_type, .dev_firmwareversion = (int8_t *)dev_fw_ver};
    ez_iot_tsl_capacity_t *capacity = NULL;
    tsl_domain_action *action = NULL;
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    CHECK_COND_RETURN(!tsl_adapter_dev_find(sn, &dev_info, &dev_status), ez_errno_tsl_dev_not_found);
    CHECK_COND_RETURN(status_done != dev_status, ez_errno_tsl_profile_loading);

    hal_thread_mutex_lock(g_capacities_mutex);

    /* find capacity */
    size_t num = ezlist_size(g_capacities_list);
    for (i = 0; i < num; i++)
    {
        ez_iot_tsl_capacity_t *capacity_temp = (ez_iot_tsl_capacity_t *)ezlist_getat(g_capacities_list, i, NULL, false);
        CHECK_COND_DONE(!capacity_temp, ez_errno_tsl_profile_loading);

        if (0 != strcmp((const char *)dev_info.dev_type, capacity_temp->dev_type) ||
            0 != strcmp((const char *)dev_info.dev_firmwareversion, capacity_temp->dev_fw_ver))
        {
            continue;
        }

        capacity = capacity_temp;
        break;
    }

    CHECK_COND_DONE(!capacity, ez_errno_tsl_profile_loading);

    /* find  resource*/
    size_t rsc_num = capacity->rsc_num;
    for (i = 0; i < rsc_num; i++)
    {
        if (0 == strcmp((const char *)rsc_info->res_type, capacity->resource[i].rsc_category))
        {
            break;
        }
    }

    CHECK_COND_DONE(i == rsc_num, ez_errno_tsl_rsctype_not_found);

    /* find local index*/
    size_t index_num = capacity->resource[i].index_num;
    for (j = 0; j < index_num; j++)
    {
        if (0 == strcmp(rsc_info->local_index, capacity->resource[i].index))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == index_num, ez_errno_tsl_index_not_found);

    /*find domain*/
    size_t domain_num = capacity->resource[i].domain_num;
    for (j = 0; j < domain_num; j++)
    {
        if (0 == strcmp(capacity->resource[i].domain[j].identifier, (char *)key_info->domain))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == domain_num, ez_errno_tsl_domain_not_found);

    /*find key*/
    size_t action_num = capacity->resource[i].domain[j].action_num;
    for (k = 0; k < action_num; k++)
    {
        tsl_domain_action *action_temp = capacity->resource[i].domain[j].action + k;

        if (0 == strcmp(action->identifier, (char *)key_info->key))
        {
            action = action_temp;
            break;
        }
    }

    CHECK_COND_DONE(k == action_num, ez_errno_tsl_key_not_found);

#ifndef COMPONENT_TSL_PROFILE_STRIP
    if (value)
    {
        rv = check_schema_value(&action->input_schema, value);
    }
#endif

done:
    hal_thread_mutex_unlock(g_capacities_mutex);

    return rv;
}

int32_t ez_iot_tsl_property_value_legal(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, const tsl_value_t *value)
{
    int32_t rv = ez_errno_succ;

    char dev_sn[48] = {0};
    char dev_type[24] = {0};
    char dev_fw_ver[36] = {0};
    dev_stauts_e dev_status = 0;
    tsl_devinfo_t dev_info = {.dev_subserial = (int8_t *)dev_sn, .dev_type = (int8_t *)dev_type, .dev_firmwareversion = (int8_t *)dev_fw_ver};
    ez_iot_tsl_capacity_t *capacity = NULL;
    tsl_domain_prop *prop = NULL;

    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    CHECK_COND_RETURN(!tsl_adapter_dev_find(sn, &dev_info, &dev_status), ez_errno_tsl_dev_not_found);
    CHECK_COND_RETURN(status_done != dev_status, ez_errno_tsl_profile_loading);

    hal_thread_mutex_lock(g_capacities_mutex);

    /* find capacity */
    size_t num = ezlist_size(g_capacities_list);
    for (i = 0; i < num; i++)
    {
        ez_iot_tsl_capacity_t *capacity_temp = (ez_iot_tsl_capacity_t *)ezlist_getat(g_capacities_list, i, NULL, false);
        CHECK_COND_DONE(!capacity_temp, ez_errno_tsl_profile_loading);

        if (0 != strcmp((const char *)dev_info.dev_type, capacity_temp->dev_type) ||
            0 != strcmp((const char *)dev_info.dev_firmwareversion, capacity_temp->dev_fw_ver))
        {
            continue;
        }

        capacity = capacity_temp;
        break;
    }

    CHECK_COND_DONE(!capacity, ez_errno_tsl_profile_loading);

    /* find  resource*/
    size_t rsc_num = capacity->rsc_num;
    for (i = 0; i < rsc_num; i++)
    {
        if (0 == strcmp((const char *)rsc_info->res_type, capacity->resource[i].rsc_category))
        {
            break;
        }
    }

    CHECK_COND_DONE(i == rsc_num, ez_errno_tsl_rsctype_not_found);

    /* find local index*/
    size_t index_num = capacity->resource[i].index_num;
    for (j = 0; j < index_num; j++)
    {
        if (0 == strcmp(rsc_info->local_index, capacity->resource[i].index))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == index_num, ez_errno_tsl_index_not_found);

    /*find domain*/
    size_t domain_num = capacity->resource[i].domain_num;
    for (j = 0; j < domain_num; j++)
    {
        if (0 == strcmp(capacity->resource[i].domain[j].identifier, (char *)key_info->domain))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == domain_num, ez_errno_tsl_domain_not_found);

    /*find key*/
    size_t prop_num = capacity->resource[i].domain[j].prop_num;
    for (k = 0; k < prop_num; k++)
    {
        tsl_domain_prop *prop_temp = capacity->resource[i].domain[j].prop + k;
        if (0 == strcmp(prop_temp->identifier, (char *)key_info->key))
        {
            prop = prop_temp;
            break;
        }
    }

    CHECK_COND_DONE(k == prop_num, ez_errno_tsl_key_not_found);

#ifndef COMPONENT_TSL_PROFILE_STRIP
    if (value)
    {
        rv = check_schema_value(&prop->prop_desc, value);
    }
#endif

done:
    hal_thread_mutex_unlock(g_capacities_mutex);

    return rv;
}

int32_t ez_iot_tsl_event_value_legal(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, const tsl_value_t *value)
{
    int32_t rv = ez_errno_succ;

    char dev_sn[48] = {0};
    char dev_type[24] = {0};
    char dev_fw_ver[36] = {0};
    dev_stauts_e dev_status = 0;
    tsl_devinfo_t dev_info = {.dev_subserial = (int8_t *)dev_sn, .dev_type = (int8_t *)dev_type, .dev_firmwareversion = (int8_t *)dev_fw_ver};
    ez_iot_tsl_capacity_t *capacity = NULL;
    tsl_domain_event *event = NULL;
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    CHECK_COND_RETURN(!tsl_adapter_dev_find(sn, &dev_info, &dev_status), ez_errno_tsl_dev_not_found);
    CHECK_COND_RETURN(status_done != dev_status, ez_errno_tsl_profile_loading);

    hal_thread_mutex_lock(g_capacities_mutex);

    /* find capacity */
    size_t num = ezlist_size(g_capacities_list);
    for (i = 0; i < num; i++)
    {
        ez_iot_tsl_capacity_t *capacity_temp = (ez_iot_tsl_capacity_t *)ezlist_getat(g_capacities_list, i, NULL, false);
        CHECK_COND_DONE(!capacity_temp, ez_errno_tsl_profile_loading);

        if (0 != strcmp((const char *)dev_info.dev_type, capacity_temp->dev_type) ||
            0 != strcmp((const char *)dev_info.dev_firmwareversion, capacity_temp->dev_fw_ver))
        {
            continue;
        }

        capacity = capacity_temp;
        break;
    }

    CHECK_COND_DONE(!capacity, ez_errno_tsl_profile_loading);

    /* find  resource*/
    size_t rsc_num = capacity->rsc_num;

    for (i = 0; i < rsc_num; i++)
    {
        if (0 == strcmp((const char *)rsc_info->res_type, capacity->resource[i].rsc_category))
        {
            break;
        }
    }

    CHECK_COND_DONE(i == rsc_num, ez_errno_tsl_rsctype_not_found);

    /* find local index*/
    size_t index_num = capacity->resource[i].index_num;
    for (j = 0; j < index_num; j++)
    {
        if (0 == strcmp(rsc_info->local_index, capacity->resource[i].index))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == index_num, ez_errno_tsl_index_not_found);

    /*find domain*/
    size_t domain_num = capacity->resource[i].domain_num;
    for (j = 0; j < domain_num; j++)
    {
        if (0 == strcmp(capacity->resource[i].domain[j].identifier, (char *)key_info->domain))
        {
            break;
        }
    }

    CHECK_COND_DONE(j == domain_num, ez_errno_tsl_domain_not_found);

    /*find key*/
    size_t event_num = capacity->resource[i].domain[j].event_num;
    for (k = 0; k < event_num; k++)
    {
        tsl_domain_event *event_temp = capacity->resource[i].domain[j].event + k;

        if (0 == strcmp(event_temp->identifier, (char *)key_info->key))
        {
            event = event_temp;
            break;
        }
    }

    CHECK_COND_DONE(k == event_num, ez_errno_tsl_key_not_found);

#ifndef COMPONENT_TSL_PROFILE_STRIP
    if (value)
    {
#ifdef HUOMAN_TRANSLATE_COMPAT
        if ((0 == strcmp(HUOMAN_LIC_PTID, get_lic_productKey())) || (0 == strcmp(HUOMAN_LIC_PTID_TEST, get_lic_productKey())))
        {
            ez_log_i(TAG_TSL, "huoman event report,no need check schema");
        }
        else
        {

            rv = check_schema_value(&event->input_schema, value);
        }

#else
        rv = check_schema_value(&event->input_schema, value);
#endif
    }
#endif

done:
    hal_thread_mutex_unlock(g_capacities_mutex);

    return rv;
}

int32_t tsl_find_property_rsc_by_keyinfo(const int8_t *sn, const tsl_key_info_t *key_info, int8_t *res_type, int32_t len)
{
    int32_t rv = ez_errno_tsl_rsctype_not_found;

    char dev_sn[48] = {0};
    char dev_type[24] = {0};
    char dev_fw_ver[36] = {0};
    dev_stauts_e dev_status = 0;
    tsl_devinfo_t dev_info = {.dev_subserial = (int8_t *)dev_sn, .dev_type = (int8_t *)dev_type, .dev_firmwareversion = (int8_t *)dev_fw_ver};
    ez_iot_tsl_capacity_t *capacity = NULL;

    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    CHECK_COND_RETURN(!tsl_adapter_dev_find(sn, &dev_info, &dev_status), ez_errno_tsl_dev_not_found);
    CHECK_COND_RETURN(status_done != dev_status, ez_errno_tsl_profile_loading);

    hal_thread_mutex_lock(g_capacities_mutex);

    /* find capacity */
    size_t num = ezlist_size(g_capacities_list);
    for (i = 0; i < num; i++)
    {
        ez_iot_tsl_capacity_t *capacity_temp = (ez_iot_tsl_capacity_t *)ezlist_getat(g_capacities_list, i, NULL, false);
        CHECK_COND_DONE(!capacity_temp, ez_errno_tsl_profile_loading);

        if (0 != strcmp((const char *)dev_info.dev_type, capacity_temp->dev_type) ||
            0 != strcmp((const char *)dev_info.dev_firmwareversion, capacity_temp->dev_fw_ver))
        {
            continue;
        }

        capacity = capacity_temp;
        break;
    }

    for (i = 0; i < capacity->rsc_num; i++)
    {
        /* 查找domain */
        for (j = 0; j < capacity->resource[i].domain_num; j++)
        {
            if (0 != strcmp(capacity->resource[i].domain[j].identifier, (char *)key_info->domain))
            {
                continue;
            }
        }

        /* 查找key */
        for (k = 0; k < capacity->resource[i].domain[j].prop_num; k++)
        {
            tsl_domain_prop *prop_temp = capacity->resource[i].domain[j].prop + k;
            if (0 != strcmp(prop_temp->identifier, (char *)key_info->key))
            {
                continue;
            }
        }

        strncpy(res_type, capacity->resource[i].rsc_category, len - 1);
        rv = 0;
    }

done:
    hal_thread_mutex_unlock(g_capacities_mutex);

    return rv;
}

int32_t tsl_find_event_rsc_by_keyinfo(const int8_t *sn, const tsl_key_info_t *key_info, int8_t *res_type, int32_t len)
{
    int32_t rv = ez_errno_tsl_rsctype_not_found;

    char dev_sn[48] = {0};
    char dev_type[24] = {0};
    char dev_fw_ver[36] = {0};
    dev_stauts_e dev_status = 0;
    tsl_devinfo_t dev_info = {.dev_subserial = (int8_t *)dev_sn, .dev_type = (int8_t *)dev_type, .dev_firmwareversion = (int8_t *)dev_fw_ver};
    ez_iot_tsl_capacity_t *capacity = NULL;

    size_t i = 0;
    size_t j = 0;
    size_t k = 0;

    CHECK_COND_RETURN(!tsl_adapter_dev_find(sn, &dev_info, &dev_status), ez_errno_tsl_dev_not_found);
    CHECK_COND_RETURN(status_done != dev_status, ez_errno_tsl_profile_loading);

    hal_thread_mutex_lock(g_capacities_mutex);

    /* find capacity */
    size_t num = ezlist_size(g_capacities_list);
    for (i = 0; i < num; i++)
    {
        ez_iot_tsl_capacity_t *capacity_temp = (ez_iot_tsl_capacity_t *)ezlist_getat(g_capacities_list, i, NULL, false);
        CHECK_COND_DONE(!capacity_temp, ez_errno_tsl_profile_loading);

        if (0 != strcmp((const char *)dev_info.dev_type, capacity_temp->dev_type) ||
            0 != strcmp((const char *)dev_info.dev_firmwareversion, capacity_temp->dev_fw_ver))
        {
            continue;
        }

        capacity = capacity_temp;
        break;
    }

    for (i = 0; i < capacity->rsc_num; i++)
    {
        /* 查找domain */
        for (j = 0; j < capacity->resource[i].domain_num; j++)
        {
            if (0 != strcmp(capacity->resource[i].domain[j].identifier, (char *)key_info->domain))
            {
                continue;
            }
        }

        /* 查找key */
        for (k = 0; k < capacity->resource[i].domain[j].event_num; k++)
        {
            tsl_domain_event *event = capacity->resource[i].domain[j].event + k;
            if (0 != strcmp(event->identifier, (char *)key_info->key))
            {
                continue;
            }
        }

        strncpy(res_type, capacity->resource[i].rsc_category, len - 1);
        rv = 0;
    }

done:
    hal_thread_mutex_unlock(g_capacities_mutex);

    return rv;
}

static bool tsl_adapter_capacity_add(ez_iot_tsl_capacity_t *capacity)
{
    hal_thread_mutex_lock(g_capacities_mutex);
    ezlist_addlast(g_capacities_list, (void *)capacity, sizeof(ez_iot_tsl_capacity_t));
    hal_thread_mutex_unlock(g_capacities_mutex);

    return true;
}

// bool ez_iot_tsl_checkupdate(const int8_t *dev_type, const int8_t *dev_ver)
// {
//     if (NULL == dev_ver || NULL == dev_type)
//     {
//         return false;
//     }

//     if (NULL != g_capacities_list)
//     {
//         return false;
//     }

//     hal_thread_mutex_lock(g_capacities_mutex);
//     ez_iot_tsl_capacity_t *capacity = NULL;
//     size_t num = ezlist_size(g_capacities_list);
//     size_t size = sizeof(ez_iot_tsl_capacity_t);
//     for (size_t i = 0; i < num; i++)
//     {
//         capacity = (ez_iot_tsl_capacity_t *)ezlist_getat(g_capacities_list, i, &size, false);
//         if (0 == strcmp((char *)dev_type, capacity->dev_type) && 0 == strcmp((char *)dev_ver, capacity->dev_fw_ver))
//         {
//             ez_log_w(TAG_TSL, "profile already exist.");
//             hal_thread_mutex_unlock(g_capacities_mutex);
//             return true;
//         }
//     }
//     hal_thread_mutex_unlock(g_capacities_mutex);

//     return false;
// }

static bool tsl_adapter_ref_add(tsl_devinfo_t *dev_info)
{
    if (NULL == dev_info || NULL != g_capacities_list)
    {
        return false;
    }

    hal_thread_mutex_lock(g_capacities_mutex);

    for (size_t i = 0; i < ezlist_size(g_capacities_list); i++)
    {
        ez_iot_tsl_capacity_t *capacity = (ez_iot_tsl_capacity_t *)ezlist_getat(g_capacities_list, i, NULL, false);
        if (0 == strcmp((char *)dev_info->dev_type, capacity->dev_type) && 0 == strcmp((char *)dev_info->dev_firmwareversion, capacity->dev_fw_ver))
        {
            capacity->ref++;
            tsl_adapter_shadow_inst(dev_info, capacity);
            ez_log_w(TAG_TSL, "profile found, ++ref:%d.", capacity->ref);
            hal_thread_mutex_unlock(g_capacities_mutex);
            return true;
        }
    }

    hal_thread_mutex_unlock(g_capacities_mutex);

    return false;
}

static int find_dev_by_sn(bscJSON *json_obj, char *sn)
{
    int index = -1;

    for (int i = 0; i < bscJSON_GetArraySize(json_obj); i++)
    {
        bscJSON *js_item = bscJSON_GetArrayItem(json_obj, i);
        if (NULL == js_item)
        {
            continue;
        }

        bscJSON *js_sn = bscJSON_GetObjectItem(js_item, TSLMAP_JSON_KEY_SN);
        if (NULL == js_sn)
        {
            continue;
        }

        if (0 == strcmp(js_sn->valuestring, (char *)sn))
        {
            index = i;
            break;
        }
    }

    return index;
}

static int get_tsl_handle_count(bscJSON *json_obj, char *handle)
{
    int count = 0;

    for (int i = 0; i < bscJSON_GetArraySize(json_obj); i++)
    {
        bscJSON *js_item = bscJSON_GetArrayItem(json_obj, i);
        if (NULL == js_item)
        {
            continue;
        }

        bscJSON *js_sn = bscJSON_GetObjectItem(js_item, TSLMAP_JSON_KEY_HANDLE);
        if (NULL == js_sn)
        {
            continue;
        }

        if (0 == strcmp(js_sn->valuestring, (char *)handle))
        {
            count++;
            break;
        }
    }

    return count;
}

static bscJSON *tslmap_metadata_to_json(tslmap_metadata_t *struct_obj)
{
    s2j_create_json_obj(metadata_json);
    CHECK_COND_RETURN(!metadata_json, NULL);

    s2j_json_set_basic_element(metadata_json, struct_obj, string, sn);
    s2j_json_set_basic_element(metadata_json, struct_obj, string, handle);

    return metadata_json;
}

static void json_to_tslmap_metadata(bscJSON *json_obj, tslmap_metadata_t *struct_obj)
{
    bscJSON *json_temp = NULL;

    s2j_struct_get_basic_element_ex(struct_obj, json_obj, string, sn, "");
    s2j_struct_get_basic_element_ex(struct_obj, json_obj, string, handle, "");
}

static bool tsl_adapter_profile_load(tsl_devinfo_t *dev_info)
{
    int32_t length = 0;
    bool rv = false;
    ez_iot_tsl_capacity_t capacity = {0};
    tslmap_metadata_t tsl_metadata = {0};
    uint32_t tslmap_len = 0;
    char *pbuf = NULL;
    bscJSON *js_root = NULL;
    int index = -1;

    CHECK_COND_RETURN(ez_kv_raw_get((const int8_t *)EZ_TSL_KEY_TSLMAP, NULL, &tslmap_len), false);
    CHECK_COND_RETURN(0 == tslmap_len, false);

    pbuf = (char *)malloc(tslmap_len + 1);
    CHECK_COND_RETURN(!pbuf, false);

    memset(pbuf, 0, tslmap_len + 1);
    CHECK_COND_RETURN(ez_kv_raw_get((const int8_t *)EZ_TSL_KEY_TSLMAP, (int8_t *)pbuf, &tslmap_len), false);
    CHECK_COND_DONE(!(js_root = bscJSON_Parse(pbuf)), false);
    SAFE_FREE(pbuf);

    CHECK_COND_DONE(-1 == (index = find_dev_by_sn(js_root, (char *)dev_info->dev_subserial)), false);

    bscJSON *js_item = bscJSON_GetArrayItem(js_root, index);
    json_to_tslmap_metadata(js_item, &tsl_metadata);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)tsl_metadata.handle, NULL, &length), false);

    pbuf = (int8_t *)malloc(length + 1);
    CHECK_COND_DONE(!pbuf, false);

    memset(pbuf, 0, length + 1);
    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)tsl_metadata.handle, pbuf, &length), false);
    ez_log_d(TAG_TSL, "length: %d,read_buf: %s, ", length, pbuf);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)tsl_metadata.handle, pbuf, &length), false);
    CHECK_COND_DONE(0 != profile_parse((char *)pbuf, length, &capacity), false);

    strncpy(capacity.dev_fw_ver, (char *)dev_info->dev_firmwareversion, sizeof(capacity.dev_fw_ver) - 1);
    strncpy(capacity.dev_type, (char *)dev_info->dev_type, sizeof(capacity.dev_type) - 1);

    tsl_adapter_capacity_add(&capacity);

    tsl_adapter_shadow_inst(dev_info, &capacity);
    tsl_adapter_prime_inst(dev_info, &capacity);

#ifdef COMPONENT_TSL_CONN_ENABLE
    extern int32_t report_center_device();
    if (0 == strcmp((char *)dev_info->dev_subserial, ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial")))
    {
        ez_log_d(TAG_TSL, "gateway dev register, report center device.");
        if (0 != report_center_device())
        {
            ez_log_e(TAG_TSL, "center device report failed.");
        }
    }
#endif

    rv = true;
done:
    SAFE_FREE(pbuf);
    bscJSON_Delete(js_root);

    return rv;
}

static bool tsl_adapter_profile_save(tsl_devinfo_t *dev_info, const char *profile, int length)
{
    bool rv = false;
    bscJSON *js_root = NULL;
    bscJSON *js_tslmap_item = NULL;
    char *pbuf = NULL;
    char *pbuf_save = NULL;
    int8_t handle_curr[32] = {0};
    int index = -1;
    tslmap_metadata_t tsl_metadata = {0};
    uint32_t tslmap_len = 0;

    ez_kv_print();
    devinfo2index(dev_info, (char *)handle_curr);
    CHECK_COND_RETURN(ez_kv_raw_get((const int8_t *)EZ_TSL_KEY_TSLMAP, NULL, &tslmap_len), false);

    if (0 == tslmap_len)
    {
        ez_log_w(TAG_TSL, "tslmap null, try clear all!");
        ez_kv_del_by_prefix((const int8_t *)EZ_TSL_KEY_TSL_PREFIX);
        js_root = bscJSON_CreateArray();
        CHECK_COND_RETURN(!js_root, false);
    }
    else
    {
        pbuf = (char *)malloc(tslmap_len + 1);
        CHECK_COND_RETURN(!pbuf, false);
        memset(pbuf, 0, tslmap_len + 1);

        CHECK_COND_RETURN(ez_kv_raw_get((const int8_t *)EZ_TSL_KEY_TSLMAP, (int8_t *)pbuf, &tslmap_len), false);
        CHECK_COND_DONE(!(js_root = bscJSON_Parse(pbuf)), false);
        index = find_dev_by_sn(js_root, (char *)dev_info->dev_subserial);
    }

    do
    {
        if (-1 == index)
        {
            /* 新增设备，tslmap增加元数据 */
            ez_log_d(TAG_TSL, "add handle 2 tslmap:%s", handle_curr);
            strncpy(tsl_metadata.sn, (char *)dev_info->dev_subserial, sizeof(tsl_metadata.sn) - 1);
            strncpy(tsl_metadata.handle, (char *)handle_curr, sizeof(tsl_metadata.handle) - 1);

            CHECK_COND_DONE(!(js_tslmap_item = tslmap_metadata_to_json(&tsl_metadata)), false);
            bscJSON_AddItemToArray(js_root, js_tslmap_item);
            CHECK_COND_DONE(!(pbuf_save = bscJSON_PrintUnformatted(js_root)), ez_errno_hub_memory);
            CHECK_COND_DONE(ez_kv_raw_set((const int8_t *)EZ_TSL_KEY_TSLMAP, (int8_t *)pbuf_save, strlen(pbuf_save)), ez_errno_hub_storage);
            ez_log_d(TAG_TSL, "add handle succ!!");
        }
        else
        {
            bscJSON *js_item = bscJSON_GetArrayItem(js_root, index);
            json_to_tslmap_metadata(js_item, &tsl_metadata);

            if (0 == strcmp(tsl_metadata.handle, handle_curr))
            {
                /* 元数据相同，无需更新tslmap */
                break;
            }

            if (1 == get_tsl_handle_count(js_root, tsl_metadata.handle))
            {
                /* 功能描述文件只有当前设备在使用，设备型号或者版本号已发生变更，tslpf文件删除 */
                ez_log_w(TAG_TSL, "ref = 0, del old pf");
                ez_kv_del((const int8_t *)tsl_metadata.handle);
            }

            ez_log_w(TAG_TSL, "tslmap chg, update");
            bscJSON_ReplaceItemInObject(js_item, TSLMAP_JSON_KEY_HANDLE, bscJSON_CreateString(handle_curr));
            CHECK_COND_DONE(!(pbuf_save = bscJSON_PrintUnformatted(js_root)), ez_errno_hub_memory);
            CHECK_COND_DONE(ez_kv_raw_set((const int8_t *)EZ_TSL_KEY_TSLMAP, (int8_t *)pbuf_save, (uint32_t)strlen(pbuf_save)), ez_errno_hub_storage);
            ez_log_w(TAG_TSL, "update succ!");
        }
    } while (false);

    /* 只有在tslmap中找不到索引句柄情况下才会保存功能描述文件 */
    if (NULL != profile && 1 == get_tsl_handle_count(js_root, handle_curr))
    {
        ez_log_i(TAG_TSL, "save pf:%s", handle_curr);
        CHECK_COND_DONE(ez_kv_no_err != ez_kv_raw_set(handle_curr, (int8_t *)profile, length), false);
        ez_log_i(TAG_TSL, "save pf succ!");
    }

    rv = true;
done:
    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    bscJSON_Delete(js_root);

    return rv;
}

bool ez_iot_tsl_adapter_profile_del(const int8_t *dev_subserial)
{
    bool rv = false;
    bscJSON *js_root = NULL;
    char *pbuf = NULL;
    char *pbuf_save = NULL;
    int index = -1;
    tslmap_metadata_t tsl_metadata = {0};
    uint32_t tslmap_len = 0;

    ez_kv_print();
    CHECK_COND_RETURN(ez_kv_raw_get((const int8_t *)EZ_TSL_KEY_TSLMAP, NULL, &tslmap_len), false);
    CHECK_COND_RETURN(0 == tslmap_len, true);

    pbuf = (char *)malloc(tslmap_len + 1);
    CHECK_COND_RETURN(!pbuf, false);
    memset(pbuf, 0, tslmap_len + 1);

    CHECK_COND_RETURN(ez_kv_raw_get((const int8_t *)EZ_TSL_KEY_TSLMAP, (int8_t *)pbuf, &tslmap_len), false);
    CHECK_COND_DONE(!(js_root = bscJSON_Parse(pbuf)), false);

    CHECK_COND_DONE(-1 == (index = find_dev_by_sn(js_root, (char *)dev_subserial)), false);
    bscJSON *js_item = bscJSON_GetArrayItem(js_root, index);
    json_to_tslmap_metadata(js_item, &tsl_metadata);

    if (1 == get_tsl_handle_count(js_root, tsl_metadata.handle))
    {
        /* 功能描述文件只有当前设备在使用，设备型号或者版本号已发生变更，tslpf文件删除 */
        ez_log_w(TAG_TSL, "ref = 0, del");
        CHECK_COND_DONE(ez_kv_del((const int8_t *)tsl_metadata.handle), false);
        ez_log_w(TAG_TSL, "del pf succ!");
    }

    ez_log_w(TAG_TSL, "tslmap chg, update");
    bscJSON_DeleteItemFromArray(js_root, index);
    CHECK_COND_DONE(!(pbuf_save = bscJSON_PrintUnformatted(js_root)), false);
    CHECK_COND_DONE(ez_kv_raw_set((const int8_t *)EZ_TSL_KEY_TSLMAP, (int8_t *)pbuf_save, (uint32_t)strlen(pbuf_save)), false);
    ez_log_w(TAG_TSL, "update succ!");

done:
    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    bscJSON_Delete(js_root);

    return rv;
}

static bool tsl_adapter_profile_download(tsl_devinfo_t *dev_info)
{
    if (!tsl_adapter_dev_add(dev_info, true))
    {
        return false;
    }

    if (!tsl_adapter_dl_task_yeild())
    {
        return false;
    }

    return true;
}

static bool tsl_adapter_ref_del(tsl_devinfo_t *dev_info)
{
    bool rv = false;

    if (NULL == dev_info || NULL == g_capacities_list)
    {
        return false;
    }

    hal_thread_mutex_lock(g_capacities_mutex);

    for (size_t i = 0; i < ezlist_size(g_capacities_list); i++)
    {
        ez_iot_tsl_capacity_t *capacity = (ez_iot_tsl_capacity_t *)ezlist_getat(g_capacities_list, i, NULL, false);
        if (0 == strcmp((char *)dev_info->dev_type, capacity->dev_type) && 0 == strcmp((char *)dev_info->dev_firmwareversion, capacity->dev_fw_ver))
        {
            ez_log_w(TAG_TSL, "profile found, --ref:%d.", capacity->ref - 1);

            if (--capacity->ref <= 0)
            {
                free_profile_memory(capacity);
                ezlist_removeat(g_capacities_list, i);
                rv = false;
            }
            else
            {
                rv = true;
            }

            break;
        }
    }

    hal_thread_mutex_unlock(g_capacities_mutex);

    return rv;
}

static bool tsl_adapter_dev_add(tsl_devinfo_t *dev_info, bool need_get)
{
    dev_info_t tsl_devinfo = {0};

    hal_thread_mutex_lock(g_profile_mutex);

    strncpy(tsl_devinfo.dev_fw_ver, (char *)dev_info->dev_firmwareversion, sizeof(tsl_devinfo.dev_fw_ver) - 1);
    strncpy(tsl_devinfo.dev_type, (char *)dev_info->dev_type, sizeof(tsl_devinfo.dev_type) - 1);
    strncpy(tsl_devinfo.dev_sn, (char *)dev_info->dev_subserial, sizeof(tsl_devinfo.dev_sn) - 1);
    tsl_devinfo.status = need_get ? status_need_update : status_done;

    ezlist_addlast(g_tsl_dev_info_list, (void *)&tsl_devinfo, sizeof(dev_info_t));

    hal_thread_mutex_unlock(g_profile_mutex);

    tsl_adapter_profile_save(dev_info, NULL, 0);

    return true;
}

static void tsl_adapter_dev_update(tsl_devinfo_t *dev_info, dev_stauts_e status)
{
    hal_thread_mutex_lock(g_profile_mutex);

    dev_info_t *dev = NULL;
    size_t size = ezlist_size(g_tsl_dev_info_list);
    for (size_t i = 0; i < size; i++)
    {
        dev = ezlist_getat(g_tsl_dev_info_list, i, NULL, false);
        if (0 == strcmp(dev->dev_sn, (char *)dev_info->dev_subserial))
        {
            break;
        }

        dev = NULL;
    }

    if (NULL == dev)
    {
        goto done;
    }

    switch (status)
    {
    case status_done: {
        hal_timer_destroy(dev->timer);
        dev->timer = NULL;
        if (dev->download_info)
        {
            free(dev->download_info);
            dev->download_info = NULL;
        }
        dev->status = status;
    }
    break;
    case status_need_update: {
        hal_time_countdown(dev->timer, 30);
    }
    break;
    case status_query: {
        hal_timer_destroy(dev->timer);
        dev->timer = hal_timer_creat();
        hal_time_countdown(dev->timer, 30);
        dev->status = status;
    }
    break;
    case status_loading: {
        dev->status = status;
    }
    break;
    default:
        break;
    }

done:
    hal_thread_mutex_unlock(g_profile_mutex);
}

static void tsl_adapter_dev_del(tsl_devinfo_t *dev_info)
{
    hal_thread_mutex_lock(g_profile_mutex);

    dev_info_t *dev = NULL;
    size_t size = ezlist_size(g_tsl_dev_info_list);
    for (size_t i = 0; i < size; i++)
    {
        dev = ezlist_getat(g_tsl_dev_info_list, i, NULL, false);
        if (0 != strcmp(dev->dev_sn, (char *)dev_info->dev_subserial))
        {
            continue;
        }

        hal_timer_destroy(dev->timer);

        if (dev->download_info)
        {
            free(dev->download_info);
        }

        ezlist_removeat(g_tsl_dev_info_list, i);
        break;
    }

    hal_thread_mutex_unlock(g_profile_mutex);
}

static bool tsl_adapter_dev_find(const int8_t *sn, tsl_devinfo_t *dev_info, dev_stauts_e *status)
{
    dev_info_t *dev = NULL;
    bool rv = false;

    CHECK_COND_RETURN(NULL == sn, false);

    hal_thread_mutex_lock(g_profile_mutex);

    size_t size = ezlist_size(g_tsl_dev_info_list);
    for (size_t i = 0; i < size; i++)
    {
        dev = ezlist_getat(g_tsl_dev_info_list, i, NULL, false);
        if (0 != strcmp(dev->dev_sn, (char *)sn))
        {
            continue;
        }

        if (dev_info)
        {
            strncpy((char *)dev_info->dev_subserial, dev->dev_sn, sizeof(dev->dev_sn) - 1);
            strncpy((char *)dev_info->dev_type, dev->dev_type, sizeof(dev->dev_type) - 1);
            strncpy((char *)dev_info->dev_firmwareversion, dev->dev_fw_ver, sizeof(dev->dev_fw_ver) - 1);
        }

        if (status)
        {
            *status = dev->status;
        }
        rv = true;
    }

    hal_thread_mutex_unlock(g_profile_mutex);

    return rv;
}

static bool tsl_adapter_dl_task_yeild()
{
    bool rv = false;
    hal_thread_mutex_lock(g_profile_mutex);

    do
    {
        if (g_profile_download_thread_running)
        {
            rv = true;
            break;
        }

        if (NULL != g_profile_download_thread)
        {
            hal_thread_destroy(g_profile_download_thread);
        }

        g_profile_download_thread = hal_thread_create((int8_t *)"ez_profile_get", ez_profile_get_thread, 1024 * 6, 5, NULL);
        if (NULL == g_profile_download_thread)
        {
            ez_log_e(TAG_TSL, "create profile get thread failed.");
            break;
        }

        rv = true;
    } while (0);

    hal_thread_mutex_unlock(g_profile_mutex);

    return rv;
}

void devinfo2index(tsl_devinfo_t *dev_info, char index[32])
{
    char md5_output[16] = {0};
    char base64_output[24 + 1] = {0};
    size_t olen = sizeof(base64_output);

    bscomptls_md5_context md5_ctx = {0};

    bscomptls_md5_init(&md5_ctx);
    bscomptls_md5_starts(&md5_ctx);

    bscomptls_md5_update(&md5_ctx, (unsigned char *)dev_info->dev_type, strlen((char *)dev_info->dev_type));
    bscomptls_md5_update(&md5_ctx, (unsigned char *)dev_info->dev_firmwareversion, strlen((char *)dev_info->dev_firmwareversion));
    bscomptls_md5_finish(&md5_ctx, md5_output);

    ez_base64_encode(base64_output, sizeof(base64_output), &olen, md5_output, sizeof(md5_output));
    memset(index, 0, 32);
    snprintf(index, 32, "%s_%s", EZ_TSL_KEY_TSL_PREFIX, base64_output);
}

static void domain_data_route_cb(ezdev_sdk_kernel_submsg_v3 *psub_msg, EZDEV_SDK_PTR puser)
{
    ez_log_v(TAG_TSL, "extend_id:%d, domain_id:%s, behavior:%s", psub_msg->extend_id, psub_msg->domain_id, psub_msg->business_type);

    switch (psub_msg->extend_id)
    {
    case sdk_service: {
        if (0 != strcmp(KERNEL_OPERATE_BUSI_TYPE, psub_msg->business_type))
        {
            ez_log_e(TAG_TSL, "not operate business type.");
            break;
        }

        tsl_action_process(psub_msg);
    }
    break;

    default:
        break;
    }
}

static void hub_domain_data_route_cb(ezdev_sdk_kernel_submsg_v3 *psub_msg)
{
    if (sdk_service == psub_msg->extend_id && 0 == strcmp(KERNEL_OPERATE_BUSI_TYPE, psub_msg->business_type))
    {
        tsl_action_process(psub_msg);
    }
}

static int32_t domain_and_hub_register(const char *str_buss, sdk_kernel_extend_id extend_id)
{
    ez_err_e rv = ez_errno_succ;

    do
    {
        ezdev_sdk_kernel_extend_v3 extend_info = {0};
        extend_info.extend_id = extend_id;
        extend_info.pUser = NULL;
        extend_info.ezdev_sdk_kernel_data_route = domain_data_route_cb;
        strncpy(extend_info.extend_module_name, str_buss, sizeof(extend_info.extend_module_name) - 1);
        rv = ezdev_sdk_kernel_extend_load_v3(&extend_info);
        if (ez_errno_succ != rv)
        {
            ez_log_e(TAG_TSL, "sdk kernel load_v3 error.");
            break;
        }

        ezdev_sdk_kernel_hub_extend hub_extend_info = {0};
        hub_extend_info.extend_id = extend_id;
        hub_extend_info.extend_hub_data_route = hub_domain_data_route_cb;
        rv = ezdev_sdk_kernel_hub_extend_load(&hub_extend_info);
        if (ez_errno_succ != rv)
        {
            ez_log_e(TAG_TSL, "hub extend load error.");
            break;
        }

    } while (false);

    return rv;
}
