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
#include "ezconfig.h"
#include "mcuconfig.h"
#include "ez_iot_def.h"
#include "hub_func.h"
#include "hub_extern.h"
#include "ez_iot_hub.h"
#include "bscJSON.h"
#include "mbedtls/md5.h"
#include "timer/timer.h"

#ifdef COMPONENT_TSL_ENABLE
#include "ez_iot_tsl.h"
#include "ez_iot_tsl_adapter.h"
#endif

#define SAP_DEV_SN_LEN 9
#define SUBLIST_JSON_KEY_STA "sta"
#define SUBLIST_JSON_KEY_SN "sn"
#define SUBLIST_JSON_KEY_VER "ver"
#define SUBLIST_JSON_KEY_TYPE "type"
#define SUBLIST_JSON_KEY_ACCESS "access"
#define HUB_AUTH_SALT "www.88075998.com"

typedef struct
{
    bool connected;
    char childdevid[64];                ///< 子设备序列号
    char version[64];                   ///< 子设备版本号
    char type[64];                      ///< 子设备型号
    char FirmwareIdentificationCode[1]; ///< 固件识别码
} hub_subdev_info_report_t;

static hub_callbacks_t g_phub_cbs = {0};

static void *g_hlock = NULL;

static void *g_auth_timer = NULL;

static int32_t g_unauth_count = 0;

static bscJSON *subdev_to_json(void *struct_obj);

static void json_to_subdev(bscJSON *json_obj, void *struct_obj);

static void subdev_to_relation_lst(bscJSON *json_relation_lst, void *struct_obj);

static void subdev_to_status_lst(bscJSON *json_status_lst, void *struct_obj);

static int find_dev_by_sn(bscJSON *json_obj, char *sn);

static ez_err_e hub_tsl_reg(const hub_subdev_info_internal_t *subdev_info);

static ez_err_e hub_tsl_unreg(const hub_subdev_info_internal_t *subdev_info);

static void hub_tsl_profile_del(const int8_t *subdev_sn);

static void hub_tsl_clean(void);

static void hub_tsl_checkupdate();

static hub_callbacks_t *hub_callbacks_get(void);

static void hub_subdev_auth_passed(int8_t *subdev_sn);

static void hub_subdev_auth_failure(int8_t *subdev_sn);

/**
 * @brief 子设备认证重试
 * 
 * @return int 
 */
int auth_retry_timer_cb(void);

static hub_callbacks_t *hub_callbacks_get(void)
{
    return &g_phub_cbs;
}

int hub_func_init(const hub_callbacks_t *phub_cbs)
{
    memcpy((void *)&g_phub_cbs, (const void *)phub_cbs, sizeof(g_phub_cbs));
    g_hlock = hal_thread_mutex_create();
    if (NULL == g_hlock)
    {
        return ez_errno_hub_internal;
    }

    if (0 != hub_tsl_reg_all())
    {
        return ez_errno_hub_internal;
    }

    g_auth_timer = ez_timer_create("auth_retry_timer", (2 * 1000 * 60), false, auth_retry_timer_cb);
    if (NULL == g_auth_timer)
    {
        ez_log_e(TAG_AP, "start auth retry timer failed.");
    }

    return 0;
}

void hub_func_deinit()
{
    memset((void *)&g_phub_cbs, 0, sizeof(g_phub_cbs));
    hub_tsl_unreg_all();
    ez_timer_delete(g_auth_timer);

    hal_thread_mutex_destroy(g_hlock);
}

ez_err_e hub_add_do(const hub_subdev_info_internal_t *subdev_info)
{
    ez_err_e rv = ez_errno_succ;
    uint32_t length = 0;
    char *pbuf = NULL;
    char *pbuf_save = NULL;
    bscJSON *js_root = NULL;
    bscJSON *js_subdev = NULL;

    hal_thread_mutex_lock(g_hlock);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, NULL, &length), ez_errno_hub_storage);

    if (0 == length)
    {
        /* 子设备列表为空，创建列表 */
        js_root = bscJSON_CreateArray();
        CHECK_COND_DONE(!js_root, ez_errno_hub_memory);
    }
    else
    {
        /* 防止重复添加 */
        pbuf = (char *)malloc(length + 1);
        CHECK_COND_DONE(!pbuf, ez_errno_hub_memory);
        memset(pbuf, 0, length + 1);

        CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, (int8_t *)pbuf, &length), ez_errno_hub_storage);
        CHECK_COND_DONE(!(js_root = bscJSON_Parse(pbuf)), ez_errno_hub_memory);
        CHECK_COND_DONE(-1 != find_dev_by_sn(js_root, (char *)subdev_info->sn), ez_errno_hub_subdev_existed);
    }

    /* 子设备数量不能超上限 */
    CHECK_COND_DONE(bscJSON_GetArraySize(js_root) >= COMPONENT_HUB_SUBLIST_MAX, ez_errno_hub_out_of_range);

    CHECK_COND_DONE(!(js_subdev = subdev_to_json((void *)subdev_info)), ez_errno_hub_memory);
    bscJSON_AddItemToArray(js_root, js_subdev);
    CHECK_COND_DONE(!(pbuf_save = bscJSON_PrintUnformatted(js_root)), ez_errno_hub_memory);
    CHECK_COND_DONE(ez_kv_raw_set((const int8_t *)EZ_HUB_KEY_HUBLIST, (int8_t *)pbuf_save, strlen(pbuf_save)), ez_errno_hub_storage);

    g_unauth_count++;
    hub_subdev_auth_do((void *)subdev_info);

done:
    hal_thread_mutex_unlock(g_hlock);

    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    bscJSON_Delete(js_root);

    if (ez_errno_succ != rv)
    {
        ez_log_e(TAG_HUB, "rv:%08x", rv);
    }

    return rv;
}

ez_err_e hub_del_do(const int8_t *subdev_sn)
{
    ez_err_e rv = ez_errno_succ;
    uint32_t length = 0;
    char *pbuf = NULL;
    char *pbuf_save = NULL;
    bscJSON *js_root = NULL;
    int index = -1;
    hub_subdev_info_internal_t subdev_info = {0};

    hal_thread_mutex_lock(g_hlock);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, NULL, &length), ez_errno_hub_storage);
    CHECK_COND_DONE(0 == length, ez_errno_hub_subdev_not_found);

    CHECK_COND_DONE(!(pbuf = (char *)malloc(length + 1)), ez_errno_hub_memory);
    memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, (int8_t *)pbuf, &length), ez_errno_hub_storage);
    CHECK_COND_DONE(!(js_root = bscJSON_Parse(pbuf)), ez_errno_hub_memory);
    CHECK_COND_DONE(-1 == (index = find_dev_by_sn(js_root, (char *)subdev_sn)), ez_errno_hub_subdev_not_found);

    bscJSON *js_item = bscJSON_GetArrayItem(js_root, index);
    CHECK_COND_DONE(!js_item, ez_errno_hub_subdev_not_found);
    json_to_subdev(js_item, (void *)&subdev_info);

    bscJSON_DeleteItemFromArray(js_root, index);
    CHECK_COND_DONE(!(pbuf_save = bscJSON_PrintUnformatted(js_root)), ez_errno_hub_memory);
    CHECK_COND_DONE(ez_kv_raw_set((const int8_t *)EZ_HUB_KEY_HUBLIST, (int8_t *)pbuf_save, strlen(pbuf_save)), ez_errno_hub_storage);

    if (0 == subdev_info.access)
    {
        g_unauth_count--;
        goto done;
    }

    hub_tsl_unreg(&subdev_info);
    hub_tsl_profile_del(subdev_info.sn);
done:

    hal_thread_mutex_unlock(g_hlock);
    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    bscJSON_Delete(js_root);

    if (ez_errno_succ == rv)
    {
        hub_subdev_list_report();
    }
    else
    {
        ez_log_e(TAG_HUB, "rv:%08x", rv);
    }

    return rv;
}

ez_err_e hub_ver_update_do(const int8_t *subdev_sn, const int8_t *subdev_ver)
{
    ez_err_e rv = ez_errno_succ;
    uint32_t length = 0;
    char *pbuf = NULL;
    char *pbuf_save = NULL;
    bscJSON *js_root = NULL;
    int index = -1;
    hub_subdev_info_internal_t subdev_info_old = {0};
    hub_subdev_info_internal_t subdev_info_new = {0};

    hal_thread_mutex_lock(g_hlock);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, NULL, &length), ez_errno_hub_storage);
    CHECK_COND_DONE(0 == length, ez_errno_hub_subdev_not_found);

    CHECK_COND_DONE(!(pbuf = (char *)malloc(length + 1)), ez_errno_hub_memory);
    memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, (int8_t *)pbuf, &length), ez_errno_hub_storage);
    CHECK_COND_DONE(!(js_root = bscJSON_Parse(pbuf)), ez_errno_hub_memory);
    CHECK_COND_DONE(-1 == (index = find_dev_by_sn(js_root, (char *)subdev_sn)), ez_errno_hub_subdev_not_found);

    bscJSON *js_item = bscJSON_GetArrayItem(js_root, index);
    CHECK_COND_DONE(!js_item, ez_errno_hub_subdev_not_found);
    json_to_subdev(js_item, (void *)&subdev_info_old);

    bscJSON_ReplaceItemInObject(js_item, SUBLIST_JSON_KEY_VER, bscJSON_CreateString((char *)subdev_ver));
    CHECK_COND_DONE(!(pbuf_save = bscJSON_PrintUnformatted(js_root)), ez_errno_hub_memory);
    CHECK_COND_DONE(ez_kv_raw_set((const int8_t *)EZ_HUB_KEY_HUBLIST, (int8_t *)pbuf_save, strlen(pbuf_save)), ez_errno_hub_storage);
    json_to_subdev(js_item, (void *)&subdev_info_new);

    hub_tsl_unreg(&subdev_info_old);
    hub_tsl_reg(&subdev_info_new);
done:

    hal_thread_mutex_unlock(g_hlock);
    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    bscJSON_Delete(js_root);

    if (ez_errno_succ == rv)
    {
        hub_subdev_list_report();
    }
    else
    {
        ez_log_e(TAG_HUB, "rv:%08x", rv);
    }

    return rv;
}

ez_err_e hub_status_update_do(const int8_t *subdev_sn, bool online)
{
    ez_err_e rv = ez_errno_succ;
    uint32_t length = 0;
    char *pbuf = NULL;
    char *pbuf_save = NULL;
    bscJSON *js_root = NULL;
    int index = -1;

    hal_thread_mutex_lock(g_hlock);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, NULL, &length), ez_errno_hub_storage);
    CHECK_COND_DONE(0 == length, ez_errno_hub_subdev_not_found);

    CHECK_COND_DONE(!(pbuf = (char *)malloc(length + 1)), ez_errno_hub_memory);
    memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, (int8_t *)pbuf, &length), ez_errno_hub_storage);
    CHECK_COND_DONE(!(js_root = bscJSON_Parse(pbuf)), ez_errno_hub_memory);
    CHECK_COND_DONE(-1 == (index = find_dev_by_sn(js_root, (char *)subdev_sn)), ez_errno_hub_subdev_not_found);

    bscJSON *js_item = bscJSON_GetArrayItem(js_root, index);
    CHECK_COND_DONE(!js_item, ez_errno_hub_subdev_not_found);

    bscJSON_ReplaceItemInObject(js_item, SUBLIST_JSON_KEY_STA, bscJSON_CreateBool((bscJSON_bool)online));
    CHECK_COND_DONE(!(pbuf_save = bscJSON_PrintUnformatted(js_root)), ez_errno_hub_memory);
    CHECK_COND_DONE(ez_kv_raw_set((const int8_t *)EZ_HUB_KEY_HUBLIST, (int8_t *)pbuf_save, strlen(pbuf_save)), ez_errno_hub_storage);
done:

    hal_thread_mutex_unlock(g_hlock);
    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    bscJSON_Delete(js_root);

    if (ez_errno_succ == rv)
    {
        hub_subdev_sta_report();
    }
    else
    {
        ez_log_e(TAG_HUB, "rv:%08x", rv);
    }

    return rv;
}

ez_err_e hub_subdev_query(const int8_t *subdev_sn, hub_subdev_info_internal_t *subdev_info)
{
    ez_err_e rv = ez_errno_succ;
    uint32_t length = 0;
    char *pbuf = NULL;
    char *pbuf_save = NULL;
    bscJSON *js_root = NULL;
    int index = -1;

    hal_thread_mutex_lock(g_hlock);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, NULL, &length), ez_errno_hub_storage);
    CHECK_COND_DONE(0 == length, ez_errno_hub_subdev_not_found);
    CHECK_COND_DONE(!(pbuf = (char *)malloc(length + 1)), ez_errno_hub_memory);
    memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, (int8_t *)pbuf, &length), ez_errno_hub_storage);
    CHECK_COND_DONE(!(js_root = bscJSON_Parse(pbuf)), ez_errno_hub_memory);

    index = find_dev_by_sn(js_root, (char *)subdev_sn);
    if (-1 == index)
    {
        rv = ez_errno_hub_subdev_not_found;
        goto done;
    }

    bscJSON *js_item = bscJSON_GetArrayItem(js_root, index);
    CHECK_COND_DONE(!js_item, ez_errno_hub_subdev_not_found);
    json_to_subdev(js_item, (void *)subdev_info);

done:
    hal_thread_mutex_unlock(g_hlock);

    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    bscJSON_Delete(js_root);

    return rv;
}

ez_err_e hub_subdev_next(hub_subdev_info_internal_t *subdev_info)
{
    ez_err_e rv = ez_errno_succ;
    uint32_t length = 0;
    char *pbuf = NULL;
    char *pbuf_save = NULL;
    bscJSON *js_root = NULL;
    int index = -1;

    hal_thread_mutex_lock(g_hlock);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, NULL, &length), ez_errno_hub_storage);
    CHECK_COND_DONE(0 == length, ez_errno_hub_enum_end);

    CHECK_COND_DONE(!(pbuf = (char *)malloc(length + 1)), ez_errno_hub_memory);
    memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, (int8_t *)pbuf, &length), ez_errno_hub_storage);
    CHECK_COND_DONE(!(js_root = bscJSON_Parse(pbuf)), ez_errno_hub_memory);

    if (0 == strlen((char *)subdev_info->sn))
    {
        index = find_dev_by_sn(js_root, NULL);
        if (-1 == index)
        {
            rv = ez_errno_hub_enum_end;
            goto done;
        }
    }
    else
    {
        index = find_dev_by_sn(js_root, (char *)subdev_info->sn);
        if (-1 == index || bscJSON_GetArraySize(js_root) <= index + 1)
        {
            rv = ez_errno_hub_enum_end;
            goto done;
        }

        index++;
    }

    bscJSON *js_item = bscJSON_GetArrayItem(js_root, index);
    CHECK_COND_DONE(!js_item, ez_errno_hub_subdev_not_found);
    json_to_subdev(js_item, (void *)subdev_info);

done:
    hal_thread_mutex_unlock(g_hlock);

    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    bscJSON_Delete(js_root);

    if (ez_errno_succ != rv && ez_errno_hub_enum_end != rv)
    {
        ez_log_e(TAG_HUB, "rv:%08x", rv);
    }

    return rv;
}

ez_err_e hub_clean_do(void)
{
    int32_t rv = ez_errno_succ;

    hub_tsl_unreg_all();
    hub_tsl_clean();

    hal_thread_mutex_lock(g_hlock);
    CHECK_COND_DONE(ez_kv_raw_set((const int8_t *)EZ_HUB_KEY_HUBLIST, (int8_t *)"", 0), ez_errno_hub_storage);
    hal_thread_mutex_unlock(g_hlock);

    hub_subdev_list_report();

done:
    return rv;
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

        bscJSON *js_sn = bscJSON_GetObjectItem(js_item, SUBLIST_JSON_KEY_SN);
        if (NULL == js_sn)
        {
            continue;
        }

        if (NULL == sn || 0 == strcmp(js_sn->valuestring, (char *)sn))
        {
            index = i;
            break;
        }
    }

    return index;
}

int hub_subdev_list_report(void)
{
    int rv = 0;
    uint32_t length = 0;
    char *pbuf = NULL;
    char *pbuf_report = NULL;
    bscJSON *js_root = NULL;
    hub_subdev_info_internal_t subdev_info = {0};
    bscJSON *subdev_lst = bscJSON_CreateArray();

    hal_thread_mutex_lock(g_hlock);

    CHECK_COND_DONE(!subdev_lst, ez_errno_hub_memory);
    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, NULL, &length), ez_errno_hub_storage);

    if (0 == length)
    {
        CHECK_COND_DONE(hub_send_msg_to_platform("{}", strlen("{}"), kPu2CenPltHubReportRelationShipReq, EZDEVSDK_HUB_REQ, 0), ez_errno_hub_internal);
        goto done;
    }

    CHECK_COND_DONE(!(pbuf = (char *)malloc(length + 1)), ez_errno_hub_memory);
    memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, (int8_t *)pbuf, &length), ez_errno_hub_storage);
    CHECK_COND_DONE(!(js_root = bscJSON_Parse(pbuf)), ez_errno_hub_memory);
    ez_log_i(TAG_HUB, "buf:%s", pbuf);
    SAFE_FREE(pbuf);

    for (size_t i = 0; i < bscJSON_GetArraySize(js_root); i++)
    {
        bscJSON *js_item = bscJSON_GetArrayItem(js_root, i);
        memset((void *)&subdev_info, 0, sizeof(subdev_info));
        json_to_subdev(js_item, (void *)&subdev_info);
        if (0 == subdev_info.access)
        {
            continue;
        }

        subdev_to_relation_lst(subdev_lst, (void *)&subdev_info);
    }

    CHECK_COND_DONE(!(pbuf_report = bscJSON_PrintUnformatted(subdev_lst)), ez_errno_hub_memory);
    CHECK_COND_DONE(hub_send_msg_to_platform(pbuf_report, strlen(pbuf_report), kPu2CenPltHubReportRelationShipReq, EZDEVSDK_HUB_REQ, 0), ez_errno_hub_internal);

done:
    hal_thread_mutex_unlock(g_hlock);

    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_report);
    bscJSON_Delete(js_root);
    bscJSON_Delete(subdev_lst);

    if (ez_errno_succ != rv)
    {
        ez_log_e(TAG_HUB, "rv:%08x", rv);
    }

    return rv;
}

int hub_subdev_sta_report(void)
{
    int rv = 0;
    uint32_t length = 0;
    char *pbuf = NULL;
    char *pbuf_report = NULL;
    bscJSON *js_root = NULL;
    hub_subdev_info_internal_t subdev_info = {0};
    bscJSON *subdev_lst = bscJSON_CreateArray();

    hal_thread_mutex_lock(g_hlock);

    CHECK_COND_DONE(!subdev_lst, ez_errno_hub_memory);
    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, NULL, &length), ez_errno_hub_storage);
    CHECK_COND_DONE(0 == length, ez_errno_hub_subdev_not_found);

    CHECK_COND_DONE(!(pbuf = (char *)malloc(length + 1)), ez_errno_hub_memory);
    memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, (int8_t *)pbuf, &length), ez_errno_hub_storage);
    CHECK_COND_DONE(!(js_root = bscJSON_Parse(pbuf)), ez_errno_hub_memory);
    SAFE_FREE(pbuf);

    for (size_t i = 0; i < bscJSON_GetArraySize(js_root); i++)
    {
        bscJSON *js_item = bscJSON_GetArrayItem(js_root, i);
        memset((void *)&subdev_info, 0, sizeof(subdev_info));
        json_to_subdev(js_item, (void *)&subdev_info);
        subdev_to_status_lst(subdev_lst, (void *)&subdev_info);
    }

    CHECK_COND_DONE(!(pbuf_report = bscJSON_PrintUnformatted(subdev_lst)), ez_errno_hub_memory);
    CHECK_COND_DONE(hub_send_msg_to_platform(pbuf_report, strlen(pbuf_report), kPu2CenPltHubReportOnlineStatusReq, EZDEVSDK_HUB_REQ, 0), ez_errno_hub_internal);

done:
    hal_thread_mutex_unlock(g_hlock);

    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_report);
    bscJSON_Delete(js_root);
    bscJSON_Delete(subdev_lst);

    if (ez_errno_succ != rv)
    {
        ez_log_e(TAG_HUB, "rv:%08x", rv);
    }

    return rv;
}

int hub_subdev_auth_do(void *subdev_info)
{
    hub_subdev_info_internal_t *_subdev_info = (hub_subdev_info_internal_t *)subdev_info;
    int rv = -1;
    int8_t sn[64] = {0};
    int8_t MAC[32 + 1] = {0};
    int8_t md5[16] = {0};
    char *pbuf_report = NULL;
    bscomptls_md5_context md5_ctx;

    bscJSON *auth_info = bscJSON_CreateObject();
    CHECK_COND_RETURN(!auth_info, ez_errno_hub_memory);

    if (0 == _subdev_info->authm)
        strncpy(sn, _subdev_info->sn, sizeof(sn) - 1);
    else
        snprintf(sn, sizeof(sn) - 1, "%s:%s", _subdev_info->type, _subdev_info->sn);

    {
        //生成MAC方法：md5(md5(md5(verifycode+childserial) + www.88075998.com))

        //首次MAC
        bscomptls_md5_init(&md5_ctx);
        bscomptls_md5_starts(&md5_ctx);
        bscomptls_md5_update(&md5_ctx, _subdev_info->vcode, strlen(_subdev_info->vcode));
        bscomptls_md5_update(&md5_ctx, sn, strlen(sn));
        bscomptls_md5_finish(&md5_ctx, md5);
        bscomptls_md5_free(&md5_ctx);
        bscomptls_hexdump(md5, 16, 1, MAC);

        //第二次MAC
        bscomptls_md5_init(&md5_ctx);
        bscomptls_md5_starts(&md5_ctx);
        bscomptls_md5_update(&md5_ctx, MAC, strlen(MAC));
        bscomptls_md5_update(&md5_ctx, HUB_AUTH_SALT, strlen(HUB_AUTH_SALT));
        bscomptls_md5_finish(&md5_ctx, md5);
        bscomptls_md5_free(&md5_ctx);
        bscomptls_hexdump(md5, 16, 1, MAC);

        //计算摘要值
        bscomptls_md5(MAC, strlen(MAC), md5);
        bscomptls_hexdump(md5, 16, 1, MAC);
    }

    bscJSON_AddStringToObject(auth_info, "childserial", sn);
    bscJSON_AddStringToObject(auth_info, "childverifycode", MAC);
    bscJSON_AddNumberToObject(auth_info, "devAccessMode", _subdev_info->authm);
    CHECK_COND_DONE(!(pbuf_report = bscJSON_PrintUnformatted(auth_info)), ez_errno_hub_memory);

    rv = hub_send_msg_to_platform(pbuf_report, strlen(pbuf_report), kPu2CenPltHubAuthChildDeviceReq, EZDEVSDK_HUB_REQ, 0);

done:
    SAFE_FREE(pbuf_report);
    bscJSON_Delete(auth_info);

    return rv;
}

void hub_subdev_auth_done(void *buf, int len)
{
    bscJSON *js_root = NULL;
    bscJSON *js_sn;
    bscJSON *js_result;
    int8_t sn[16 + 1];

    if (NULL == buf || 0 == len)
    {
        ez_log_e(TAG_HUB, "PDU invaild!");
        goto done;
    }

    // {"childserial":"4LYV8SK7UKLBOUOVS6HXVX:ARZCWQSRG3CW","result":1}
    if (NULL == (js_root = bscJSON_Parse(buf)))
    {
        ez_log_e(TAG_HUB, "PDU invaild, json format!");
        goto done;
    }

    js_sn = bscJSON_GetObjectItem(js_root, "childserial");
    js_result = bscJSON_GetObjectItem(js_root, "result");

    if (NULL == js_sn || NULL == js_result ||
        bscJSON_String != js_sn->type || bscJSON_Number != js_result->type)
    {
        ez_log_e(TAG_HUB, "PDU invaild, obj!");
        goto done;
    }

    if (SAP_DEV_SN_LEN == strlen(js_sn->valuestring))
    {
        strncpy(sn, js_sn->valuestring, sizeof(sn) - 1);
    }
    else
    {
        /* license类型设备需要截取deviceName */
        sscanf(js_sn->valuestring, "%*[^:]:%s", sn);
    }

    ez_log_i(TAG_HUB, "sn:%s", sn);
    switch (js_result->valueint)
    {
    case 0:
        hub_subdev_auth_passed(sn);
        break;
    case 1:
        hub_subdev_auth_failure(sn);
        break;
    default:
        ez_log_e(TAG_HUB, "Server internal err");
        break;
    }

done:
    CJSON_SAFE_DELETE(js_root);
}

static ez_err_e hub_tsl_reg(const hub_subdev_info_internal_t *subdev_info)
{
    ez_err_e rv = ez_errno_succ;
#ifdef COMPONENT_TSL_ENABLE
    extern ez_err_e ez_iot_tsl_reg(tsl_devinfo_t * pevinfo);
    tsl_devinfo_t dev_info = {.dev_subserial = (int8_t *)subdev_info->sn, .dev_type = (int8_t *)subdev_info->type, .dev_firmwareversion = (int8_t *)subdev_info->ver};
    rv = ez_iot_tsl_reg(&dev_info);
#endif

    return rv;
}

static ez_err_e hub_tsl_unreg(const hub_subdev_info_internal_t *subdev_info)
{
    ez_err_e rv = ez_errno_succ;
#ifdef COMPONENT_TSL_ENABLE
    extern ez_err_e ez_iot_tsl_unreg(tsl_devinfo_t * pevinfo);
    tsl_devinfo_t dev_info = {.dev_subserial = (int8_t *)subdev_info->sn, .dev_type = (int8_t *)subdev_info->type, .dev_firmwareversion = (int8_t *)subdev_info->ver};
    rv = ez_iot_tsl_unreg(&dev_info);
#endif
    return rv;
}

void hub_tsl_profile_del(const int8_t *subdev_sn)
{
#ifdef COMPONENT_TSL_ENABLE
    /* 删除子设备profile */
    extern bool ez_iot_tsl_adapter_profile_del(const int8_t *dev_subserial);
    ez_iot_tsl_adapter_profile_del(subdev_sn);
#endif
}

static void hub_tsl_checkupdate()
{
#ifdef COMPONENT_TSL_ENABLE
    extern void ez_iot_tsl_checkupdate();
    ez_iot_tsl_checkupdate();
#endif
}

ez_err_e hub_tsl_reg_all(void)
{
    ez_err_e rv = ez_errno_succ;
    int32_t unauth_count = 0;
    hub_subdev_info_internal_t subdev_info = {0};

    do
    {
        rv = hub_subdev_next(&subdev_info);
        if (ez_errno_hub_enum_end == rv)
        {
            ez_log_v(TAG_HUB, "tsl reg, hub enum end!");
            rv = ez_errno_succ;
            break;
        }
        else if (ez_errno_succ != rv)
        {
            ez_log_e(TAG_HUB, "err occur in subdev enum, rv = 0x%08x", rv);
            break;
        }

        /* 未认证，不注册物模型 */
        if (0 == subdev_info.access)
        {
            unauth_count++;
            continue;
        }

        rv = hub_tsl_reg(&subdev_info);
        if (ez_errno_succ != rv)
        {
            ez_log_e(TAG_HUB, "tsl reg, err occur, rv = 0x%08x", rv);
            break;
        }
    } while (0 == rv);

    hal_thread_mutex_lock(g_hlock);
    g_unauth_count = unauth_count;
    hal_thread_mutex_unlock(g_hlock);

    if (ez_errno_succ == rv)
    {
        hub_tsl_checkupdate();
    }

    return rv;
}

ez_err_e hub_tsl_unreg_all(void)
{
    ez_err_e rv = ez_errno_succ;
    hub_subdev_info_internal_t subdev_info = {0};

    do
    {
        rv = hub_subdev_next(&subdev_info);
        if (ez_errno_hub_enum_end == rv)
        {
            ez_log_v(TAG_HUB, "tsl unreg, hub enum end!");
            rv = ez_errno_succ;
            break;
        }
        else if (ez_errno_succ != rv)
        {
            ez_log_e(TAG_HUB, "err occur in subdev enum, rv = 0x%08x", rv);
            break;
        }

        /* 未完成认证，未注册物模型 */
        if (0 == subdev_info.access)
        {
            continue;
        }

        hub_tsl_unreg(&subdev_info);
    } while (0 == rv);

    if (ez_errno_succ == rv)
    {
        hub_tsl_checkupdate();
    }

    return rv;
}

static void hub_tsl_clean(void)
{
    ez_err_e rv = ez_errno_succ;
    hub_subdev_info_internal_t subdev_info = {0};

    do
    {
        rv = hub_subdev_next(&subdev_info);
        if (ez_errno_hub_enum_end == rv)
        {
            ez_log_v(TAG_HUB, "tsl unreg, hub enum end!");
            rv = ez_errno_succ;
            break;
        }
        else if (ez_errno_succ != rv)
        {
            ez_log_e(TAG_HUB, "err occur in subdev enum, rv = 0x%08x", rv);
            break;
        }

        hub_tsl_profile_del(subdev_info.sn);
    } while (0 == rv);
}

static bscJSON *subdev_to_json(void *struct_obj)
{
    hub_subdev_info_internal_t *subdev_obj = (hub_subdev_info_internal_t *)struct_obj;
    s2j_create_json_obj(subdev_json);
    CHECK_COND_RETURN(!subdev_obj, NULL);

    s2j_json_set_basic_element(subdev_json, subdev_obj, Bool, sta);
    s2j_json_set_basic_element(subdev_json, subdev_obj, int, authm);
    s2j_json_set_basic_element(subdev_json, subdev_obj, string, type);
    s2j_json_set_basic_element(subdev_json, subdev_obj, string, sn);
    s2j_json_set_basic_element(subdev_json, subdev_obj, string, vcode);
    s2j_json_set_basic_element(subdev_json, subdev_obj, string, ver);
    s2j_json_set_basic_element(subdev_json, subdev_obj, string, uuid);
    s2j_json_set_basic_element(subdev_json, subdev_obj, int, access);

    return subdev_json;
}

static void json_to_subdev(bscJSON *json_obj, void *struct_obj)
{
    hub_subdev_info_internal_t *struct_obj_internal = (hub_subdev_info_internal_t *)struct_obj;
    bscJSON *json_temp = NULL;

    s2j_struct_get_basic_element_ex(struct_obj_internal, json_obj, int, sta, 0);
    s2j_struct_get_basic_element_ex(struct_obj_internal, json_obj, int, authm, 0);
    s2j_struct_get_basic_element_ex(struct_obj_internal, json_obj, string, type, "");
    s2j_struct_get_basic_element_ex(struct_obj_internal, json_obj, string, sn, "");
    s2j_struct_get_basic_element_ex(struct_obj_internal, json_obj, string, vcode, "");
    s2j_struct_get_basic_element_ex(struct_obj_internal, json_obj, string, ver, "");
    s2j_struct_get_basic_element_ex(struct_obj_internal, json_obj, string, uuid, "");
    s2j_struct_get_basic_element_ex(struct_obj_internal, json_obj, int, access, 0);
}

static void subdev_to_relation_lst(bscJSON *json_relation_lst, void *struct_obj)
{
    hub_subdev_info_internal_t *struct_obj_internal = (hub_subdev_info_internal_t *)struct_obj;
    hub_subdev_info_report_t struct_obj_report = {0};

    s2j_create_json_obj(subdev_json);
    struct_obj_report.connected = struct_obj_internal->sta;
    strncpy(struct_obj_report.childdevid, struct_obj_internal->sn, sizeof(struct_obj_report.childdevid) - 1);
    strncpy(struct_obj_report.type, struct_obj_internal->type, sizeof(struct_obj_report.childdevid) - 1);
    strncpy(struct_obj_report.version, struct_obj_internal->ver, sizeof(struct_obj_report.childdevid) - 1);

    s2j_json_set_basic_element(subdev_json, &struct_obj_report, Bool, connected);
    s2j_json_set_basic_element(subdev_json, &struct_obj_report, string, childdevid);
    s2j_json_set_basic_element(subdev_json, &struct_obj_report, string, version);
    s2j_json_set_basic_element(subdev_json, &struct_obj_report, string, type);
    s2j_json_set_basic_element(subdev_json, &struct_obj_report, string, FirmwareIdentificationCode);

    bscJSON_AddItemToArray(json_relation_lst, subdev_json);
}

static void subdev_to_status_lst(bscJSON *json_status_lst, void *struct_obj)
{
    hub_subdev_info_internal_t *struct_obj_internal = (hub_subdev_info_internal_t *)struct_obj;
    hub_subdev_info_report_t struct_obj_report = {0};

    s2j_create_json_obj(subdev_json);
    struct_obj_report.connected = struct_obj_internal->sta;
    strncpy(struct_obj_report.childdevid, struct_obj_internal->sn, sizeof(struct_obj_report.childdevid) - 1);
    strncpy(struct_obj_report.type, struct_obj_internal->type, sizeof(struct_obj_report.childdevid) - 1);
    strncpy(struct_obj_report.version, struct_obj_internal->ver, sizeof(struct_obj_report.childdevid) - 1);

    s2j_json_set_basic_element(subdev_json, &struct_obj_report, Bool, connected);
    s2j_json_set_basic_element(subdev_json, &struct_obj_report, string, childdevid);

    bscJSON_AddItemToArray(json_status_lst, subdev_json);
}

static void hub_subdev_auth_passed(int8_t *subdev_sn)
{
    ez_log_w(TAG_HUB, "auth_passed");

    ez_err_e rv = ez_errno_succ;
    uint32_t length = 0;
    char *pbuf = NULL;
    char *pbuf_save = NULL;
    bscJSON *js_root = NULL;
    int index = -1;
    hub_subdev_info_internal_t subdev_info = {0};

    hal_thread_mutex_lock(g_hlock);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, NULL, &length), ez_errno_hub_storage);
    CHECK_COND_DONE(0 == length, ez_errno_hub_subdev_not_found);

    CHECK_COND_DONE(!(pbuf = (char *)malloc(length + 1)), ez_errno_hub_memory);
    memset(pbuf, 0, length + 1);

    CHECK_COND_DONE(ez_kv_raw_get((const int8_t *)EZ_HUB_KEY_HUBLIST, (int8_t *)pbuf, &length), ez_errno_hub_storage);

    CHECK_COND_DONE(!(js_root = bscJSON_Parse(pbuf)), ez_errno_hub_memory);
    CHECK_COND_DONE(-1 == (index = find_dev_by_sn(js_root, (char *)subdev_sn)), ez_errno_hub_subdev_not_found);

    bscJSON *js_item = bscJSON_GetArrayItem(js_root, index);
    CHECK_COND_DONE(!js_item, ez_errno_hub_subdev_not_found);
    json_to_subdev(js_item, (void *)&subdev_info);

    bscJSON_ReplaceItemInObject(js_item, SUBLIST_JSON_KEY_ACCESS, bscJSON_CreateNumber(1));
    CHECK_COND_DONE(!(pbuf_save = bscJSON_PrintUnformatted(js_root)), ez_errno_hub_memory);
    CHECK_COND_DONE(ez_kv_raw_set((const int8_t *)EZ_HUB_KEY_HUBLIST, (int8_t *)pbuf_save, strlen(pbuf_save)), ez_errno_hub_storage);

    hub_callbacks_get()->recv_event(hub_event_add_succ, (void *)subdev_info.sn, strlen(subdev_info.sn));
    hub_tsl_reg(&subdev_info);

done:
    hal_thread_mutex_unlock(g_hlock);

    SAFE_FREE(pbuf);
    SAFE_FREE(pbuf_save);
    bscJSON_Delete(js_root);

    if (ez_errno_succ == rv)
    {
        hub_subdev_list_report();
    }
    else
    {
        ez_log_e(TAG_HUB, "rv:%08x", rv);
    }
}

static void hub_subdev_auth_failure(int8_t *subdev_sn)
{
    ez_log_w(TAG_HUB, "auth_failure");
    hub_subdev_info_internal_t subdev_info = {0};

    hub_subdev_query(subdev_sn, &subdev_info);
    hub_callbacks_get()->recv_event(hub_event_add_fail, (void *)&subdev_info, sizeof(hub_subdev_info_t));
    hub_del_do(subdev_sn);
}

int auth_retry_timer_cb(void)
{
    ez_log_d(TAG_HUB, "auth retry cb in");
    int rv = 0;
    hub_subdev_info_internal_t subdev_info = {0};

    CHECK_COND_DONE(!(0 < g_unauth_count), 0);
    ez_log_i(TAG_HUB, "count:%d", g_unauth_count);
    CHECK_COND_DONE(!ez_iot_is_online(), 0);

    do
    {
        rv = hub_subdev_next(&subdev_info);
        if (ez_errno_hub_enum_end == rv)
        {
            ez_log_v(TAG_HUB, "auth retry, hub enum end!");
            rv = ez_errno_succ;
            break;
        }
        else if (ez_errno_succ != rv)
        {
            ez_log_e(TAG_HUB, "err occur in subdev enum, rv = 0x%08x", rv);
            break;
        }

        if (1 == subdev_info.access)
        {
            continue;
        }

        if (hub_subdev_auth_do((void *)&subdev_info))
        {
            ez_log_e(TAG_HUB, "auth do, err occur");
        }
    } while (0 == rv);

done:
    ez_log_d(TAG_HUB, "auth retry cb out");
    return rv;
}