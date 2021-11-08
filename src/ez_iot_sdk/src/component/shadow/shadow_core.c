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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include "ez_iot_def.h"
#include "shadow_core.h"
#include "shadow_func.h"
#include "ezdev_sdk_kernel.h"
#include "shadow_def.h"

#ifdef EZ_IOT_SDK
#include "ezconfig.h"
#include "mcuconfig.h"
#include "hal_thread.h"
#include "hal_semaphore.h"
#include "ez_utils/lstLib.h"
#include "ez_iot_shadow.h"
#include "stddef.h"
#include "ez_iot_log.h"
#include "ez_iot_ctx.h"
#include "hal_time.h"

#endif

typedef enum
{
    shadow_core_status_report = 0, ///< 上报所有状态
    shadow_core_status_sync,       ///< 期望状态同步
    shadow_core_status_chg,        ///< 状态变更上报
    shadow_core_status_max,        ///< 状态值上线
} shadow_core_status_e;

typedef enum
{
    shadow_prot_ver_min = 0,
    shadow_prot_ver_1,
    shadow_prot_ver_2,
    shadow_prot_ver_3,
    shadow_prot_ver_max,
} shadow_prot_ver_e;

typedef int (*shadow_cloud2dev_cb)(const ez_iot_shadow_value_t *pValue, ez_iot_shadow_business2dev_param_t *pstParam);

typedef int (*shadow_devcloud_cb)(ez_iot_shadow_value_t *pValue, ez_iot_shadow_business2cloud_param_t *pstParam);

typedef struct
{
    NODE node;    ///< 双向链表的节点
    char key[32]; /// Shadow服务的Key

    shadow_cloud2dev_cb cloud2dev;
    shadow_devcloud_cb dev2cloud;

    uint32_t stat_ver;   ///< 状态版本号
    uint8_t need_report; ///< 是否需要上报标志
    void *timer;         ///< 定时器
    uint32_t msg_seq;    ///< 上报时序
    void *json_value;    ///< 缓存的value
} node_key_t;

typedef struct
{
    NODE node;                  ///< 双向链表的节点
    shadow_prot_ver_e prot_ver; ///< 协议版本号
    int domain_int;             ///< 领域id(int)
    char domain_string[32];     ///< 领域id(string)
    LIST key_lst;               ///< 信令路由表
} node_domain_t;

typedef struct
{
    NODE node;           ///< 双向链表的节点
    int index;           ///< 资源id
    uint8_t need_sync;   ///< 是否需要同步
    void *timer;         ///< 定时器
    uint8_t retry_count; ///< 同步请求重试计数
    LIST domain_lst;     ///< 领域列表
} node_index_t;

typedef struct
{
    NODE node;      ///< 双向链表的节点
    char type[32];  ///< 资源类型
    LIST index_lst; ///< 资源列表
} node_index_set_t;

typedef struct
{
    NODE node;       ///< 双向链表的节点
    char dev_sn[48]; ///< 设备序列号
    LIST type_lst;   ///< 资源类型列表
} node_dev_t;

/* shadow线程句柄 */
static void *g_shadow_thread = NULL;

/* 线程运行标志位 */
static bool g_running = false;

/* 信号量句柄 */
static void *g_sem = NULL;

static char *g_p2c_buf = NULL;

/* shadow 状态*/
static shadow_core_status_e g_shadow_status = shadow_core_status_report;

static bool g_need_reset = false;
static bool g_need_report = false;
static bool g_need_sync = false;

/* shadow 线程函数 */
static void shadow_core_yeild(void *pparam);

/* 主流程函数 */
static int shadow_proc_do();

/* 强制上报所有状态 */
static int shadow_proc_report();

/* 发起期望状态同步 */
static int shadow_proc_sync();

/* 状态变更上报 */
static int shadow_proc_chg();

/* 检测模块是否被重置 */
static void check_status_update();

/* 重置所有状态 */
static void shadow_status_reset();

static void shadow_status2sync();

static void shadow_reply2report(uint32_t seq, int32_t code);

static void shadow_reply2query(char *devsn, char *res_type, int index, char *payload);

static int shadow_set2dev(char *devsn, char *res_type, int index, char *payload);

static void shadow_status_sync_disable(char *devsn, char *res_type, int index);

/**
 * @brief 执行上报
 * 
 * @param devsn 
 * @param res_type 
 * @param index 
 * @param domain 
 * @param key 
 * @return int32_t -1上报失败，0不需要上报，1表示正在上报中
 */
static int32_t do_report(char *devsn, char *res_type, int index, char *domain, node_key_t *node_key);

static int32_t do_set(char *devsn, char *res_type, int index, char *domain, char *key, bscJSON *value);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//< shadow module manager class
static LIST g_shaodw_modules;
static void *g_hlock = NULL;
static bool shadow_module_init();
static void shadow_module_lock();
static void shadow_module_unlock();
static void shadow_module_deinit();

static node_dev_t *shadow_module_find_dev(LIST *lst, int8_t *sn);
static node_index_set_t *shadow_module_find_indexs(LIST *lst, int8_t *res_type);
static node_index_t *shadow_module_find_index(LIST *lst, int16_t index);
static node_domain_t *shadow_module_find_domain(LIST *lst, int8_t *domain_id);
static node_key_t *shadow_module_find_key(LIST *lst, int8_t *key);

static node_dev_t *shadow_module_add_dev(LIST *lst, int8_t *sn);
static node_index_set_t *shadow_module_add_index_set(LIST *lst, int8_t *res_type);
static node_index_t *shadow_module_add_index(LIST *lst, int16_t index);
static node_domain_t *shadow_module_add_domain(LIST *lst, int8_t *domain_id, uint16_t props_num, void *props, shadow_prot_ver_e ver);

static void *shadow_module_get_next(LIST *lst, NODE *node);

static void shadow_module_delete_domain(int8_t *sn, int8_t *res_type, int16_t index, int8_t *domain_id);
////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bscJSON *tlv2json(ez_iot_shadow_value_t *ptlv);

static int32_t json2tlv(bscJSON *pvalue, ez_iot_shadow_value_t *ptlv);

/**
 * @brief 判断json number是int或double
 * 
 * @param a pvalue->valueint
 * @param b pvalue->valuedouble
 * @return true double
 * @return false int
 */
static bool is_double(int a, double b);

/**
 * @brief destroy the objs obtained from json2tlv
 * 
 * @param ptlv 
 */
static void tlv_destroy(ez_iot_shadow_value_t *ptlv);

bool shadow_core_start()
{
    void *phandle = NULL;

    if (NULL != g_shadow_thread)
    {
        return true;
    }

    if (NULL == g_sem && NULL == (g_sem = hal_semaphore_create()))
    {
        return false;
    }

    if (!shadow_module_init())
    {
        return false;
    }

    g_running = true;
    ezdev_lstInit(&g_shaodw_modules);
#ifdef EZ_IOT_SDK
    phandle = hal_thread_create((int8_t *)"ez_shadow_core", shadow_core_yeild, 1024 * 6, 2, g_sem);
    if (NULL == phandle)
#else
    if (0 != ezdev_pthreadCreate(phandle, NULL, shadow_core_yeild, g_sem))
#endif
    {
        ez_log_e(TAG_SHADOW, "shadow thread create error");
        hal_semaphore_destroy(g_sem);
        g_running = false;
        return false;
    }
    g_p2c_buf = (char *)malloc(PUSH_BUF_MAX_LEN);
    if (NULL == g_p2c_buf)
    {
        g_running = false;
        return false;
    }
    memset(g_p2c_buf, 0, PUSH_BUF_MAX_LEN);

    g_shadow_thread = phandle;
    return true;
}

void shadow_core_stop()
{
    do
    {
        if (false == g_running)
            break;

        g_running = false;
        hal_semaphore_post(g_sem);
        hal_thread_destroy(g_shadow_thread);
        hal_semaphore_destroy(g_sem);
        shadow_module_deinit();

        if (NULL != g_p2c_buf)
        {
            free(g_p2c_buf);
        }

        g_sem = NULL;
        g_shadow_thread = NULL;
    } while (0);
}

static void shadow_core_yeild(void *pparam)
{
    void *hsem = pparam;
    int proc_result = -1;

#ifdef _linux_
    ezdev_setPthreadName("ez_shadow_core");
#endif

    while (g_running)
    {
        // if (!ez_iot_is_online() || !ez_iot_is_binding())
        if (!ez_iot_is_online())
        {
            /* 设备不在线或者未绑定，则不进行上报 */
            ez_log_v(TAG_SHADOW, "!online");
            hal_thread_sleep(1000);
            continue;
        }

        /* 如果上次处理完成才会等待信号量 */
        if (0 == proc_result)
        {
            if (-1 == hal_semaphore_wait_ms(hsem, FORCE_SYNC_TIME))
            {
                /* 长时间状态没变化，强制同步 */
                shadow_status_reset();
            }

            ez_log_d(TAG_SHADOW, "get semaphore");
        }

        proc_result = shadow_proc_do();
        if (0 != proc_result)
        {
            ez_log_v(TAG_SHADOW, "proc_result:%d", proc_result);
            hal_thread_sleep(1000);
        }
    };
}

static int shadow_proc_do()
{
    int ret = -1;
    check_status_update();

    ez_log_v(TAG_SHADOW, "shadow_proc_do, status:%d", g_shadow_status);

    switch (g_shadow_status)
    {
    case shadow_core_status_report:
        ret = shadow_proc_report();
        break;
    case shadow_core_status_sync:
        ret = shadow_proc_sync();
        break;
    case shadow_core_status_chg:
        ret = shadow_proc_chg();
        break;
    default:
        break;
    }

    if (0 == ret)
    {
        shadow_module_lock();
        if (g_shadow_status < shadow_core_status_max - 1)
        {
            ret = 2;
            ez_log_w(TAG_SHADOW, "status chg++, %d", g_shadow_status++);
        }
        shadow_module_unlock();
    }

    return ret;
}

void shadow_core_event_occured(shadow_event_type_e event_type)
{
    ez_log_d(TAG_SHADOW, "set semaphore, t:%d", event_type);

    switch (event_type)
    {
    case shadow_event_type_reset:
        shadow_module_lock();
        g_need_reset = true;
        shadow_module_unlock();
        break;
    case shadow_event_type_online:
    case shadow_event_type_add:
        shadow_module_lock();
        g_need_report = true;
        shadow_module_unlock();
        break;
    case shadow_event_type_offline:
        shadow_module_lock();
        g_need_sync = true;
        shadow_module_unlock();
        break;
    case shadow_event_type_report:
    {
        //< 防止同步期间服务没响应，设备发生状态变更不能上报的问题
        if (shadow_core_status_sync == g_shadow_status)
        {
            shadow_module_lock();
            g_need_report = true;
            shadow_module_unlock();
        }
    }
    case shadow_event_type_recv:
        /* do nothing */
        break;
    default:
        break;
    }

    if (!g_running || NULL == g_sem)
        return;

    hal_semaphore_post(g_sem);
}

static void check_status_update()
{
    int status_chg = -1;

    if (g_need_reset)
    {
        shadow_status_reset();
        status_chg = shadow_core_status_report;
    }
    else if (g_need_sync)
    {
        shadow_status2sync();
        status_chg = shadow_core_status_report;
    }
    else if (g_need_report)
    {
        status_chg = shadow_core_status_report;
    }

    if (-1 == status_chg)
    {
        return;
    }

    shadow_module_lock();
    g_need_reset = false;
    g_need_sync = false;
    g_need_report = false;
    ez_log_w(TAG_SHADOW, "status chg, %d to %d", g_shadow_status, status_chg);
    g_shadow_status = (shadow_core_status_e)status_chg;
    shadow_module_unlock();
}

static int shadow_proc_report()
{
    int rv = 0;
    int total_remaining = 0;

    NODE *node_dev = NULL;
    NODE *node_index_set = NULL;
    NODE *node_index = NULL;
    NODE *node_domain = NULL;
    NODE *node_key = NULL;

    node_dev_t *pnode_dev = NULL;
    node_index_set_t *pnode_index_set = NULL;
    node_index_t *pnode_index = NULL;
    node_domain_t *pnode_domain = NULL;
    node_key_t *pnode_key = NULL;

    ez_log_w(TAG_SHADOW, "func report");

    shadow_module_lock();

    while (NULL != (node_dev = shadow_module_get_next(&g_shaodw_modules, node_dev)))
    {
        pnode_dev = (node_dev_t *)node_dev;
        node_index_set = NULL;

        ez_log_v(TAG_SHADOW, "found dev:%s", pnode_dev->dev_sn);
        while (NULL != (node_index_set = shadow_module_get_next(&pnode_dev->type_lst, node_index_set)))
        {
            pnode_index_set = (node_index_set_t *)node_index_set;
            node_index = NULL;

            ez_log_v(TAG_SHADOW, "found index set:%s", pnode_index_set->type);
            while (NULL != (node_index = shadow_module_get_next(&pnode_index_set->index_lst, node_index)))
            {
                pnode_index = (node_index_t *)node_index;
                node_domain = NULL;

                ez_log_v(TAG_SHADOW, "found index:%d", pnode_index->index);
                while (NULL != (node_domain = shadow_module_get_next(&pnode_index->domain_lst, node_domain)))
                {
                    pnode_domain = (node_domain_t *)node_domain;
                    node_key = NULL;

                    ez_log_v(TAG_SHADOW, "found domain:%s", pnode_domain->domain_string);
                    while (NULL != (node_key = shadow_module_get_next(&pnode_domain->key_lst, node_key)))
                    {
                        pnode_key = (node_key_t *)node_key;
                        ez_log_v(TAG_SHADOW, "found key:%s", pnode_key->key);

                        int result = do_report(pnode_dev->dev_sn, pnode_index_set->type, pnode_index->index, pnode_domain->domain_string, pnode_key);
                        if (-1 == result)
                        {
                            rv = -1;
                            goto done;
                        }
                        else if (1 == result)
                        {
                            total_remaining++;
                        }
                    }
                }
            }
        }
    }

    if (total_remaining)
    {
        ez_log_v(TAG_SHADOW, "total_remaining:%d", total_remaining);
        rv = 1;
    }

done:
    shadow_module_unlock();

    if (0 != rv)
    {
        hal_thread_sleep(1000);
    }

    return rv;
}

static int shadow_proc_sync()
{
    int rv = 0;
    int total_remaining = 0;

    NODE *node_dev = NULL;
    NODE *node_index_set = NULL;
    NODE *node_index = NULL;

    node_dev_t *pnode_dev = NULL;
    node_index_set_t *pnode_index_set = NULL;
    node_index_t *pnode_index = NULL;

    ez_log_w(TAG_SHADOW, "func sync");

    shadow_module_lock();

    while (NULL != (node_dev = shadow_module_get_next(&g_shaodw_modules, node_dev)))
    {
        pnode_dev = (node_dev_t *)node_dev;
        node_index_set = NULL;

        ez_log_d(TAG_SHADOW, "found dev:%s", pnode_dev->dev_sn);
        while (NULL != (node_index_set = shadow_module_get_next(&pnode_dev->type_lst, node_index_set)))
        {
            pnode_index_set = (node_index_set_t *)node_index_set;
            node_index = NULL;

            ez_log_d(TAG_SHADOW, "found index set:%s", pnode_index_set->type);
            while (NULL != (node_index = shadow_module_get_next(&pnode_index_set->index_lst, node_index)))
            {
                pnode_index = (node_index_t *)node_index;

                if (0 == pnode_index->need_sync)
                {
                    continue;
                }

                if (NULL != pnode_index->timer)
                {
                    if (!hal_time_isexpired(pnode_index->timer))
                    {
                        ez_log_d(TAG_SHADOW, "wait4sync rsp");
                        total_remaining++;
                        continue;
                    }
                    else
                    {
                        ez_log_e(TAG_SHADOW, "wait4sync rsp timeout");
                        hal_timer_destroy(pnode_index->timer);
                        pnode_index->timer = NULL;
                    }
                }

                ///< 如果同步超过最大次数，则不再进行
                if (++pnode_index->retry_count > MAXIMUM_RETRY_TIMES)
                {
                    pnode_index->retry_count = 0;
                    pnode_index->need_sync = 0;
                    continue;
                }

                if (Shadow_QuerySyncListToPlt_v3(pnode_dev->dev_sn, pnode_index_set->type, pnode_index->index))
                {
                    ez_log_e(TAG_SHADOW, "query req");
                    rv = -1;
                    break;
                }

                total_remaining++;
                pnode_index->timer = hal_timer_creat();
                hal_time_countdown_ms(pnode_index->timer, MINIIMUM_RETRY_INTERVAL);
            }
        }
    }

    if (total_remaining)
    {
        ez_log_v(TAG_SHADOW, "total_remaining:%d", total_remaining);
        rv = 1;
    }

    shadow_module_unlock();

    if (0 != rv)
    {
        hal_thread_sleep(1000);
    }

    return rv;
}

static int shadow_proc_chg()
{
    return shadow_proc_report();
}

static void shadow_status_reset()
{
    NODE *node_dev = NULL;
    NODE *node_index_set = NULL;
    NODE *node_index = NULL;
    NODE *node_domain = NULL;
    NODE *node_key = NULL;

    node_dev_t *pnode_dev = NULL;
    node_index_set_t *pnode_index_set = NULL;
    node_index_t *pnode_index = NULL;
    node_domain_t *pnode_domain = NULL;
    node_key_t *pnode_key = NULL;

    shadow_module_lock();

    while (NULL != (node_dev = shadow_module_get_next(&g_shaodw_modules, node_dev)))
    {
        pnode_dev = (node_dev_t *)node_dev;
        node_index_set = NULL;

        ez_log_v(TAG_SHADOW, "found dev:%s", pnode_dev->dev_sn);
        while (NULL != (node_index_set = shadow_module_get_next(&pnode_dev->type_lst, node_index_set)))
        {
            pnode_index_set = (node_index_set_t *)node_index_set;
            node_index = NULL;

            ez_log_v(TAG_SHADOW, "found index set:%s", pnode_index_set->type);
            while (NULL != (node_index = shadow_module_get_next(&pnode_index_set->index_lst, node_index)))
            {
                pnode_index = (node_index_t *)node_index;
                node_domain = NULL;
                pnode_index->need_sync = 1;
                hal_timer_destroy(pnode_index->timer);
                pnode_index->timer = NULL;

                ez_log_v(TAG_SHADOW, "found index:%d", pnode_index->index);
                while (NULL != (node_domain = shadow_module_get_next(&pnode_index->domain_lst, node_domain)))
                {
                    pnode_domain = (node_domain_t *)node_domain;
                    node_key = NULL;

                    ez_log_v(TAG_SHADOW, "found domain:%s", pnode_domain->domain_string);
                    while (NULL != (node_key = shadow_module_get_next(&pnode_domain->key_lst, node_key)))
                    {
                        pnode_key = (node_key_t *)node_key;
                        ez_log_v(TAG_SHADOW, "found key:%s", pnode_key->key);
                        pnode_key->need_report = 1;
                        hal_timer_destroy(pnode_key->timer);
                        pnode_key->timer = NULL;
                    }
                }
            }
        }
    }

    shadow_module_unlock();
}

static void shadow_status2sync()
{
    NODE *node_dev = NULL;
    NODE *node_index_set = NULL;
    NODE *node_index = NULL;

    node_dev_t *pnode_dev = NULL;
    node_index_set_t *pnode_index_set = NULL;
    node_index_t *pnode_index = NULL;

    shadow_module_lock();

    while (NULL != (node_dev = shadow_module_get_next(&g_shaodw_modules, node_dev)))
    {
        pnode_dev = (node_dev_t *)node_dev;
        node_index_set = NULL;

        ez_log_d(TAG_SHADOW, "found dev:%s", pnode_dev->dev_sn);
        while (NULL != (node_index_set = shadow_module_get_next(&pnode_dev->type_lst, node_index_set)))
        {
            pnode_index_set = (node_index_set_t *)node_index_set;
            node_index = NULL;

            ez_log_d(TAG_SHADOW, "found index set:%s", pnode_index_set->type);
            while (NULL != (node_index = shadow_module_get_next(&pnode_index_set->index_lst, node_index)))
            {
                pnode_index = (node_index_t *)node_index;
                pnode_index->need_sync = 1;
                hal_timer_destroy(pnode_index->timer);
                pnode_index->timer = NULL;
                pnode_index->retry_count = 0;
            }
        }
    }

    shadow_module_unlock();
}

static int32_t do_report(char *devsn, char *res_type, int index, char *domain, node_key_t *node_key)
{
    int rv = 0;
    ez_iot_shadow_value_t tlv = {0};
    bscJSON *json_value = NULL;
    ez_iot_shadow_res_t shadow_res = {0};
    ez_iot_shadow_business2cloud_param_t stGenParam = {0, (void *)&shadow_res, (int8_t *)domain, (int8_t *)node_key->key};

    if (0 == strcmp(shadow_default_name, devsn))
    {
        strncpy((char *)shadow_res.dev_serial, ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial"), sizeof(shadow_res.dev_serial) - 1);
    }
    else
    {
        strncpy((char *)shadow_res.dev_serial, (char *)devsn, sizeof(shadow_res.dev_serial) - 1);
    }

    strncpy((char *)shadow_res.dev_serial, devsn, sizeof(shadow_res.dev_serial) - 1);
    strncpy((char *)shadow_res.res_type, res_type, sizeof(shadow_res.res_type) - 1);
    shadow_res.local_index = index;

    do
    {
        if (!node_key->need_report)
        {
            ez_log_v(TAG_SHADOW, "no need report");
            break;
        }

        if (node_key->timer)
        {
            if (hal_time_isexpired(node_key->timer))
            {
                hal_timer_destroy(node_key->timer);
                node_key->timer = NULL;
            }
            else
            {
                rv = 1;
                break;
            }
        }

        if (node_key->json_value)
        {
            /* 通过ez_iot_shadow_push_value_v3接口上报 */
            json_value = bscJSON_Duplicate(node_key->json_value, 1);
        }
        else
        {
            if (NULL == node_key->dev2cloud)
            {
                ez_log_d(TAG_SHADOW, "no need report2");
                node_key->need_report = 0;
                break;
            }

            tlv.value = g_p2c_buf;
            tlv.length = PUSH_BUF_MAX_LEN;
            if (NULL == tlv.value)
            {
                ez_log_e(TAG_SHADOW, "do_report malloc");
                rv = -1;
                break;
            }

            memset(tlv.value, 0, PUSH_BUF_MAX_LEN);
            if (0 != node_key->dev2cloud(&tlv, &stGenParam))
            {
                ez_log_e(TAG_SHADOW, "dev2cloud cb err");
                node_key->need_report = 0;
                break;
            }

            if (NULL == (json_value = tlv2json(&tlv)))
            {
                ez_log_e(TAG_SHADOW, "tlv2json");
                rv = -1;
                break;
            }
        }

        uint32_t seq = 0;
        if (ezDevSDK_Shadow_Report_v3(devsn, res_type, index, domain, node_key->key, json_value, node_key->stat_ver, &seq))
        {
            ez_log_e(TAG_SHADOW, "report v3%d", rv);
            rv = -1;
            break;
        }

        ez_log_v(TAG_SHADOW, "send seq:%d", seq);
        node_key->msg_seq = seq;
        node_key->timer = hal_timer_creat();
        hal_time_countdown_ms(node_key->timer, MINIIMUM_RETRY_INTERVAL);

    } while (0);

    return rv;
}

int shadow_core_module_addv2(int32_t domain_id, uint16_t props_num, void *props)
{
    //TODO

    return -1;
}

int shadow_core_module_addv3(int8_t *sn, int8_t *res_type, int16_t index, int8_t *domain_id, uint16_t props_num, void *props)
{
    int rv = -1;
    node_dev_t *pnode_dev;
    node_index_set_t *pnode_indexs;
    node_index_t *pnode_index;
    int8_t *dev_sn = sn;

    if (0 == strcmp(ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial"), (char *)sn))
    {
        dev_sn = (int8_t *)shadow_default_name;
    }

    ez_log_v(TAG_SHADOW, "addv3, sn:%s", dev_sn);
    ez_log_v(TAG_SHADOW, "addv3, rtype:%s", res_type);
    ez_log_v(TAG_SHADOW, "addv3, index:%d", index);
    ez_log_v(TAG_SHADOW, "addv3, domain:%s", domain_id);
    ez_log_v(TAG_SHADOW, "addv3: num:%d", props_num);

    shadow_module_lock();

    do
    {
        pnode_dev = shadow_module_find_dev(&g_shaodw_modules, dev_sn);
        if (NULL == pnode_dev && NULL == (pnode_dev = shadow_module_add_dev(&g_shaodw_modules, dev_sn)))
        {
            ez_log_e(TAG_SHADOW, "add dev err");
            break;
        }

        pnode_indexs = shadow_module_find_indexs(&pnode_dev->type_lst, res_type);
        if (NULL == pnode_indexs && NULL == (pnode_indexs = shadow_module_add_index_set(&pnode_dev->type_lst, res_type)))
        {
            ez_log_e(TAG_SHADOW, "add index set err");
            break;
        }

        pnode_index = shadow_module_find_index(&pnode_indexs->index_lst, index);
        if (NULL == pnode_index && NULL == (pnode_index = shadow_module_add_index(&pnode_indexs->index_lst, index)))
        {
            ez_log_e(TAG_SHADOW, "add index err");
            break;
        }

        if (NULL == shadow_module_add_domain(&pnode_index->domain_lst, domain_id, props_num, props, shadow_prot_ver_3))
        {
            ez_log_e(TAG_SHADOW, "add domain err");
            break;
        }

        rv = 0;
    } while (0);

    ez_log_d(TAG_SHADOW, "shadow_core_module_addv3, dev count:%d", ezdev_lstCount(&g_shaodw_modules));

    shadow_module_unlock();

    return rv;
}

int shadow_core_module_delete(int8_t *sn, int8_t *res_type, int16_t index, int8_t *domain_id)
{
    shadow_module_lock();
    shadow_module_delete_domain(sn, res_type, index, domain_id);
    shadow_module_unlock();

    return 0;
}

int shadow_core_propertie_changed(int8_t *sn, int8_t *res_type, int16_t index, int8_t *domain_id, int8_t *pkey, void *value)
{
    int rv = -1;
    node_dev_t *pnode_dev;
    node_index_set_t *pnode_indexs;
    node_index_t *pnode_index;
    node_domain_t *pnode_domain;
    node_key_t *pnode_key;
    int8_t *dev_sn = sn;

    if (0 == strcmp(ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial"), (char *)sn))
    {
        dev_sn = (int8_t *)shadow_default_name;
    }

    shadow_module_lock();

    do
    {
        pnode_dev = shadow_module_find_dev(&g_shaodw_modules, dev_sn);
        if (NULL == pnode_dev)
        {
            ez_log_e(TAG_SHADOW, "dev not found!, sn:%s", dev_sn);
            break;
        }

        pnode_indexs = shadow_module_find_indexs(&pnode_dev->type_lst, res_type);
        if (NULL == pnode_indexs)
        {
            ez_log_e(TAG_SHADOW, "indexs not found!, t:%s", res_type);
            break;
        }

        pnode_index = shadow_module_find_index(&pnode_indexs->index_lst, index);
        if (NULL == pnode_index)
        {
            ez_log_e(TAG_SHADOW, "index not found!, i:%d", index);
            break;
        }
        pnode_domain = shadow_module_find_domain(&pnode_index->domain_lst, domain_id);
        if (NULL == pnode_domain)
        {
            ez_log_e(TAG_SHADOW, "domain not found!, t:%s", domain_id);
            break;
        }
        pnode_key = shadow_module_find_key(&pnode_domain->key_lst, pkey);
        if (NULL == pnode_key)
        {
            ez_log_e(TAG_SHADOW, "pkey not found!, t:%s", pkey);
            break;
        }
        hal_timer_destroy(pnode_key->timer);

        pnode_key->timer = NULL;
        pnode_key->stat_ver++;
        pnode_key->need_report = 1;
        pnode_key->msg_seq = 0;
        if (pnode_key->json_value)
        {
            bscJSON_Delete((bscJSON *)pnode_key->json_value);
            pnode_key->json_value = NULL;
        }

        if (NULL != value)
        {
            pnode_key->json_value = tlv2json((ez_iot_shadow_value_t *)value);
        }

        rv = 0;
    } while (0);
    shadow_module_unlock();

    return rv;
}

int shadow_core_cloud_data_in(void *shadow_res, uint32_t seq, int8_t *business_type, int8_t *payload)
{
    int rv = -1;
    ez_iot_shadow_res_t *pshadow_res = (ez_iot_shadow_res_t *)shadow_res;

    ez_log_v(TAG_SHADOW, "data in:%s", business_type);
    ez_log_v(TAG_SHADOW, "sn:%s", (char *)pshadow_res->dev_serial);
    ez_log_v(TAG_SHADOW, "type:%s", (char *)pshadow_res->res_type);
    ez_log_v(TAG_SHADOW, "index:%d", pshadow_res->local_index);
    ez_log_v(TAG_SHADOW, "seq:%d", seq);

    do
    {
        if (0 == strcmp("set", (char *)business_type))
        {
            rv = shadow_set2dev((char *)pshadow_res->dev_serial, (char *)pshadow_res->res_type, pshadow_res->local_index, (char *)payload);
            Shadow_Set_Reply_v3((char *)pshadow_res->dev_serial, (char *)pshadow_res->res_type, pshadow_res->local_index, rv, seq);
        }
        else if (0 == strcmp("query_reply", (char *)business_type))
        {
            shadow_reply2query((char *)pshadow_res->dev_serial, (char *)pshadow_res->res_type, pshadow_res->local_index, (char *)payload);
        }
        else if (0 == strcmp("report", (char *)business_type))
        {
            // 目前响应没有实现，用本地QOS0模拟
            // if (0 != Shadow_Check_Reply_v3((char *)payload))
            // {
            //     break;
            // }

            shadow_reply2report(seq, *(int32_t*)payload);
        }
        else
        {
            break;
        }
    } while (0);

    shadow_core_event_occured(shadow_event_type_recv);
    return rv;
}

static bool shadow_module_init()
{
    g_hlock = hal_thread_mutex_create();
    if (NULL == g_hlock)
    {
        return false;
    }

    return true;
}

static void shadow_module_lock()
{
    if (NULL == g_hlock)
    {
        return;
    }

    hal_thread_mutex_lock(g_hlock);
}

static void shadow_module_unlock()
{
    if (NULL == g_hlock)
    {
        return;
    }

    hal_thread_mutex_unlock(g_hlock);
}

static void shadow_module_deinit()
{
    if (NULL == g_hlock)
    {
        return;
    }

    hal_thread_mutex_destroy(g_hlock);
    g_hlock = NULL;
}

static void shadow_module_delete_domain(int8_t *sn, int8_t *res_type, int16_t index, int8_t *domain_id)
{
    //TODO
}

static node_dev_t *shadow_module_find_dev(LIST *lst, int8_t *sn)
{
    node_dev_t *pnode = NULL;

    if (NULL == lst || 0 == ezdev_lstCount(lst))
    {
        return NULL;
    }

    LIST_FOR_EACH(node_dev_t, pnode, lst)
    {
        if (0 != strcmp(pnode->dev_sn, (char *)sn))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return pnode;
}

static node_index_set_t *shadow_module_find_indexs(LIST *lst, int8_t *res_type)
{
    node_index_set_t *pnode = NULL;

    if (NULL == lst || 0 == ezdev_lstCount(lst))
    {
        return NULL;
    }

    LIST_FOR_EACH(node_index_set_t, pnode, lst)
    {
        if (0 != strcmp(pnode->type, (char *)res_type))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return pnode;
}

static node_index_t *shadow_module_find_index(LIST *lst, int16_t index)
{
    node_index_t *pnode = NULL;

    if (NULL == lst || 0 == ezdev_lstCount(lst))
    {
        return NULL;
    }

    LIST_FOR_EACH(node_index_t, pnode, lst)
    {
        if (index != pnode->index)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return pnode;
}

static node_domain_t *shadow_module_find_domain(LIST *lst, int8_t *domain_id)
{
    node_domain_t *pnode = NULL;

    if (NULL == lst || 0 == ezdev_lstCount(lst))
    {
        return NULL;
    }

    LIST_FOR_EACH(node_domain_t, pnode, lst)
    {
        if (0 != strcmp(pnode->domain_string, (char *)domain_id))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return pnode;
}

static node_key_t *shadow_module_find_key(LIST *lst, int8_t *key)
{
    node_key_t *pnode = NULL;

    if (NULL == lst || 0 == ezdev_lstCount(lst))
    {
        return NULL;
    }

    LIST_FOR_EACH(node_key_t, pnode, lst)
    {
        if (0 != strcmp(pnode->key, (char *)key))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return pnode;
}

static node_dev_t *shadow_module_add_dev(LIST *lst, int8_t *sn)
{
    node_dev_t *pnode = NULL;

    if (NULL == lst)
    {
        return NULL;
    }

    pnode = shadow_module_find_dev(lst, sn);
    if (NULL != pnode)
    {
        return pnode;
    }

    pnode = (node_dev_t *)malloc(sizeof(node_dev_t));
    if (NULL == pnode)
    {
        return NULL;
    }

    memset(pnode, 0, sizeof(node_dev_t));
    strncpy(pnode->dev_sn, (char *)sn, sizeof(pnode->dev_sn) - 1);
    ezdev_lstAdd(lst, &pnode->node);

    return pnode;
}

static node_index_set_t *shadow_module_add_index_set(LIST *lst, int8_t *res_type)
{
    node_index_set_t *pnode = NULL;

    if (NULL == lst)
    {
        return NULL;
    }

    pnode = shadow_module_find_indexs(lst, res_type);
    if (NULL != pnode)
    {
        return pnode;
    }

    pnode = (node_index_set_t *)malloc(sizeof(node_index_set_t));
    if (NULL == pnode)
    {
        return NULL;
    }

    memset(pnode, 0, sizeof(node_index_set_t));
    if (0 == strlen((char *)res_type))
    {
        strncpy(pnode->type, (char *)shadow_default_name, sizeof(pnode->type) - 1);
    }
    else
    {
        strncpy(pnode->type, (char *)res_type, sizeof(pnode->type) - 1);
    }

    ezdev_lstAdd(lst, &pnode->node);

    return pnode;
}

static node_index_t *shadow_module_add_index(LIST *lst, int16_t index)
{
    node_index_t *pnode = NULL;

    if (NULL == lst)
    {
        return NULL;
    }

    pnode = shadow_module_find_index(lst, index);
    if (NULL != pnode)
    {
        return pnode;
    }

    pnode = (node_index_t *)malloc(sizeof(node_index_t));
    if (NULL == pnode)
    {
        return NULL;
    }

    memset(pnode, 0, sizeof(node_index_t));
    pnode->index = index;
    pnode->need_sync = 1;
    pnode->retry_count = 0;
    ezdev_lstAdd(lst, &pnode->node);

    return pnode;
}

static node_domain_t *shadow_module_add_domain(LIST *lst, int8_t *domain_id, uint16_t props_num, void *props, shadow_prot_ver_e ver)
{
    node_domain_t *rv = NULL;
    node_domain_t *pnode_domain = NULL;
    size_t i;

    if (NULL == lst || NULL == domain_id || 0 == props_num || NULL == props)
    {
        return NULL;
    }

    do
    {
        pnode_domain = shadow_module_find_domain(lst, domain_id);
        if (NULL == pnode_domain)
        {
            pnode_domain = (node_domain_t *)malloc(sizeof(node_domain_t));
            if (NULL == pnode_domain)
            {
                break;
            }

            memset(pnode_domain, 0, sizeof(node_domain_t));
            strncpy(pnode_domain->domain_string, (char *)domain_id, sizeof(pnode_domain->domain_string) - 1);
            pnode_domain->prot_ver = ver;
            ezdev_lstAdd(lst, &pnode_domain->node);
        }

        ez_iot_shadow_business_t *pservices = (ez_iot_shadow_business_t *)props;
        for (i = 0; i < props_num; i++)
        {
            node_key_t *pnode_key = shadow_module_find_key(&pnode_domain->key_lst, pservices[i].key);
            if (pnode_key)
            {
                if (pnode_key->cloud2dev != pservices[i].business2dev)
                {
                    pnode_key->cloud2dev = pservices[i].business2dev;
                }

                if (pnode_key->dev2cloud != pservices[i].business2cloud)
                {
                    pnode_key->dev2cloud = pservices[i].business2cloud;
                }

                pnode_key->need_report = 1;
                continue;
            }
            else
            {
                pnode_key = (node_key_t *)malloc(sizeof(node_key_t));
                ez_log_v(TAG_SHADOW, "addv3: key:%s", (char *)pservices[i].key);
                memset(pnode_key, 0, sizeof(node_key_t));
                strncpy(pnode_key->key, (char *)pservices[i].key, sizeof(pnode_key->key) - 1);
                pnode_key->cloud2dev = pservices[i].business2dev;
                pnode_key->dev2cloud = pservices[i].business2cloud;
                pnode_key->stat_ver = 0;
                pnode_key->msg_seq = 0;
                pnode_key->need_report = 1;

                ezdev_lstAdd(&pnode_domain->key_lst, &pnode_key->node);
            }
        }

        rv = pnode_domain;
    } while (0);

    return rv;
}

static void *shadow_module_get_next(LIST *lst, NODE *node)
{
    if (NULL == lst)
    {
        return NULL;
    }

    if (NULL == node)
    {
        return ezdev_lstFirst(lst);
    }

    return ezdev_lstNext(node);
}

static bscJSON *tlv2json(ez_iot_shadow_value_t *ptlv)
{
    bscJSON *json_value = NULL;
    if (!ptlv || 0 == ptlv->length)
        return NULL;

    ez_log_d(TAG_SHADOW, "tlv type:%d", ptlv->type);
    ez_log_d(TAG_SHADOW, "tlv size:%d", ptlv->length);
    ez_log_d(TAG_SHADOW, "tlv value:%d", ptlv->value_int);

    switch (ptlv->type)
    {
    case shd_data_type_bool:
        json_value = bscJSON_CreateBool(ptlv->value_int);
        break;
    case shd_data_type_int:
        json_value = bscJSON_CreateNumber(ptlv->value_int);
        break;
    case shd_data_type_double:
        json_value = bscJSON_CreateNumber(ptlv->value_double);
        break;
    case shd_data_type_string:
        json_value = bscJSON_CreateString(ptlv->value);
        break;
    case shd_data_type_array:
    case shd_data_type_object:
        json_value = bscJSON_Parse(ptlv->value);
        break;
    default:
        break;
    }

    return json_value;
}

static void shadow_reply2report(uint32_t seq, int32_t code)
{
    NODE *node_dev = NULL;
    NODE *node_index_set = NULL;
    NODE *node_index = NULL;
    NODE *node_domain = NULL;
    NODE *node_key = NULL;

    node_dev_t *pnode_dev = NULL;
    node_index_set_t *pnode_index_set = NULL;
    node_index_t *pnode_index = NULL;
    node_domain_t *pnode_domain = NULL;
    node_key_t *pnode_key = NULL;

    shadow_module_lock();

    while (NULL != (node_dev = shadow_module_get_next(&g_shaodw_modules, node_dev)))
    {
        pnode_dev = (node_dev_t *)node_dev;
        node_index_set = NULL;

        while (NULL != (node_index_set = shadow_module_get_next(&pnode_dev->type_lst, node_index_set)))
        {
            pnode_index_set = (node_index_set_t *)node_index_set;
            node_index = NULL;

            while (NULL != (node_index = shadow_module_get_next(&pnode_index_set->index_lst, node_index)))
            {
                pnode_index = (node_index_t *)node_index;
                node_domain = NULL;

                while (NULL != (node_domain = shadow_module_get_next(&pnode_index->domain_lst, node_domain)))
                {
                    pnode_domain = (node_domain_t *)node_domain;
                    node_key = NULL;

                    while (NULL != (node_key = shadow_module_get_next(&pnode_domain->key_lst, node_key)))
                    {
                        pnode_key = (node_key_t *)node_key;

                        ez_log_v(TAG_SHADOW, "local seq:%d, ack seq:%d, code:%d", pnode_key->msg_seq, seq, code);
                        if (pnode_key->msg_seq == seq)
                        {
                            ez_log_d(TAG_SHADOW, "seq match, code:%d", code);
                            if (0 == code)
                            {
                                pnode_key->need_report = 0;
                                bscJSON_Delete((bscJSON *)pnode_key->json_value);
                                pnode_key->json_value = NULL;
                            }

                            pnode_key->msg_seq = 0;
                            hal_timer_destroy(pnode_key->timer);
                            pnode_key->timer = NULL;

                            goto done;
                        }
                    }
                }
            }
        }
    }

done:
    shadow_module_unlock();
}

static int shadow_set2dev(char *devsn, char *res_type, int index, char *payload)
{
    bscJSON *json_root = NULL;
    bscJSON *json_state = NULL;
    bscJSON *json_desired = NULL;
    bscJSON *json_domain = NULL;
    bscJSON *json_identifier = NULL;
    bscJSON *json_value = NULL;

    int rv = -1;

    do
    {
        if (NULL == (json_root = bscJSON_Parse(payload)))
        {
            break;
        }

        if (NULL == (json_state = bscJSON_GetObjectItem(json_root, "state")))
        {
            break;
        }

        if (NULL == (json_desired = bscJSON_GetObjectItem(json_state, "desired")))
        {
            break;
        }

        if (NULL == (json_domain = bscJSON_GetObjectItem(json_desired, "domain")))
        {
            break;
        }

        if (NULL == (json_identifier = bscJSON_GetObjectItem(json_desired, "identifier")))
        {
            break;
        }

        if (NULL == (json_value = bscJSON_GetObjectItem(json_desired, "value")))
        {
            break;
        }

        rv = do_set(devsn, res_type, index, json_domain->valuestring, json_identifier->valuestring, json_value);
    } while (0);

    if (0 != rv)
    {
        ez_log_e(TAG_SHADOW, "reply invalid!");
        ez_log_d(TAG_SHADOW, "reply:%s", payload);
    }

    if (json_root)
    {
        bscJSON_Delete(json_root);
    }

    return rv;
}

static void shadow_status_sync_disable(char *devsn, char *res_type, int index)
{
    node_dev_t *pnode_dev;
    node_index_set_t *pnode_indexs;
    node_index_t *pnode_index;

    shadow_module_lock();

    do
    {
        if (NULL == (pnode_dev = shadow_module_find_dev(&g_shaodw_modules, (int8_t *)devsn)))
        {
            ez_log_e(TAG_SHADOW, "dev not found!");
            break;
        }

        if (NULL == (pnode_indexs = shadow_module_find_indexs(&pnode_dev->type_lst, (int8_t *)res_type)))
        {
            ez_log_e(TAG_SHADOW, "indexs not found!");
            break;
        }

        if (NULL == (pnode_index = shadow_module_find_index(&pnode_indexs->index_lst, index)))
        {
            ez_log_e(TAG_SHADOW, "index not found!");
            break;
        }

        pnode_index->need_sync = 0;
        hal_timer_destroy(pnode_index->timer);
        pnode_index->timer = NULL;
        pnode_index->retry_count = 0;

    } while (0);

    shadow_module_unlock();
}

static void shadow_reply2query(char *devsn, char *res_type, int index, char *payload)
{
    bscJSON *json_root = NULL;
    bscJSON *json_code = NULL;
    bscJSON *json_state = NULL;
    bscJSON *json_desired = NULL;
    bscJSON *json_domain_array = NULL;
    bscJSON *json_domain = NULL;
    bscJSON *json_identifier = NULL;
    bscJSON *json_value = NULL;

    int rv = 0;

    do
    {
        if (NULL == (json_root = bscJSON_Parse(payload)))
        {
            rv = -1;
            break;
        }

        if (NULL == (json_code = bscJSON_GetObjectItem(json_root, "code")))
        {
            rv = -1;
            break;
        }

        if (bscJSON_Number != json_code->type || 0 != json_code->valueint)
        {
            //服务内部出错，不再重试
            ez_log_e(TAG_SHADOW, "query_reply code:%d", json_code->valueint);
            rv = -2;
            break;
        }

        if (NULL == (json_state = bscJSON_GetObjectItem(json_root, "state")))
        {
            rv = -1;
            break;
        }

        if (NULL == (json_desired = bscJSON_GetObjectItem(json_state, "desired")) || bscJSON_Array != json_desired->type)
        {
            //没有期望状态
            break;
        }

        if (0 == bscJSON_GetArraySize(json_desired))
        {
            break;
        }

        for (size_t i = 0; i < bscJSON_GetArraySize(json_desired); i++)
        {
            if (NULL == (json_domain_array = bscJSON_GetArrayItem(json_desired, i)))
            {
                continue;
            }

            if (NULL == (json_domain = bscJSON_GetObjectItem(json_domain_array, "domain")))
            {
                rv = -1;
                continue;
            }

            if (NULL == (json_identifier = bscJSON_GetObjectItem(json_domain_array, "identifier")))
            {
                rv = -1;
                continue;
            }

            if (NULL == (json_value = bscJSON_GetObjectItem(json_domain_array, "value")))
            {
                rv = -1;
                continue;
            }

            rv |= do_set(devsn, res_type, index, json_domain->valuestring, json_identifier->valuestring, json_value);
        }
    } while (0);

    if (0 != rv)
    {
        ez_log_e(TAG_SHADOW, "query_reply invalid! rv=%d", rv);
        ez_log_d(TAG_SHADOW, "query_reply:%s", payload);
    }

    shadow_status_sync_disable(devsn, res_type, index);

    if (json_root)
    {
        bscJSON_Delete(json_root);
    }
}

static int32_t do_set(char *devsn, char *res_type, int index, char *domain, char *key, bscJSON *value)
{
    int rv = -1;
    node_dev_t *pnode_dev;
    node_index_set_t *pnode_indexs;
    node_index_t *pnode_index;
    node_domain_t *pnode_domain = NULL;
    node_key_t *pnode_key = NULL;
    ez_iot_shadow_value_t tlv = {0};
    shadow_cloud2dev_cb cloud2dev_cb = NULL;

    ez_iot_shadow_res_t shadow_res = {0};
    ez_iot_shadow_business2dev_param_t pstParseSync = {(int8_t *)&shadow_res, (int8_t *)domain, (int8_t *)key};

    if (0 == strcmp(shadow_default_name, devsn))
    {
        strncpy((char *)shadow_res.dev_serial, ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial"), sizeof(shadow_res.dev_serial) - 1);
    }
    else
    {
        strncpy((char *)shadow_res.dev_serial, (char *)devsn, sizeof(shadow_res.dev_serial) - 1);
    }

    strncpy((char *)shadow_res.res_type, (char *)res_type, sizeof(shadow_res.res_type) - 1);
    shadow_res.local_index = index;

    ez_log_v(TAG_SHADOW, "do_set");

    shadow_module_lock();

    do
    {
        if (NULL == (pnode_dev = shadow_module_find_dev(&g_shaodw_modules, (int8_t *)devsn)))
        {
            ez_log_e(TAG_SHADOW, "dev not found!");
            break;
        }

        if (NULL == (pnode_indexs = shadow_module_find_indexs(&pnode_dev->type_lst, (int8_t *)res_type)))
        {
            ez_log_e(TAG_SHADOW, "indexs not found!");
            break;
        }

        if (NULL == (pnode_index = shadow_module_find_index(&pnode_indexs->index_lst, index)))
        {
            ez_log_e(TAG_SHADOW, "index not found!");
            break;
        }

        if (NULL == (pnode_domain = shadow_module_find_domain(&pnode_index->domain_lst, (int8_t *)domain)))
        {
            ez_log_e(TAG_SHADOW, "domain not found!");
            break;
        }

        if (NULL == (pnode_key = shadow_module_find_key(&pnode_domain->key_lst, (int8_t *)key)))
        {
            ez_log_e(TAG_SHADOW, "key not found!");
            break;
        }

        if (pnode_key->cloud2dev == NULL)
        {
            rv = -1;
            ez_log_e(TAG_SHADOW, "cloud2dev is null");
            break;
        }

        if (0 != (rv = json2tlv((bscJSON *)value, &tlv)))
        {
            ez_log_e(TAG_SHADOW, "json2tlv");
            break;
        }

        cloud2dev_cb = pnode_key->cloud2dev;

        rv = 0;
    } while (0);

    shadow_module_unlock();

    if (0 == rv && 0 != (rv = cloud2dev_cb(&tlv, &pstParseSync)))
    {
        ez_log_e(TAG_SHADOW, "cloud2dev err, key=%s", key);
    }

    tlv_destroy(&tlv);

    return rv;
}

static int32_t json2tlv(bscJSON *pvalue, ez_iot_shadow_value_t *ptlv)
{
    int32_t rv = 0;

    if (!pvalue || !ptlv)
        return -1;

    memset(ptlv, 0, sizeof(ez_iot_shadow_value_t));
    switch (pvalue->type)
    {
    case bscJSON_False:
    case bscJSON_True:
        ptlv->value_bool = (bool)pvalue->valueint;
        ptlv->length = sizeof(pvalue->valueint);
        ptlv->type = shd_data_type_bool;
        break;
    case bscJSON_Number:
    {
        if (!is_double(pvalue->valueint, pvalue->valuedouble))
        {
            ptlv->value_int = pvalue->valueint;
            ptlv->length = sizeof(pvalue->valueint);
            ptlv->type = shd_data_type_int;
        }
        else
        {
            ptlv->value_double = pvalue->valuedouble;
            ptlv->length = sizeof(pvalue->valuedouble);
            ptlv->type = shd_data_type_double;
        }
    }
    break;
    case bscJSON_String:
        ptlv->value = pvalue->valuestring;
        ptlv->length = strlen(pvalue->valuestring);
        ptlv->type = shd_data_type_string;
        break;
    case bscJSON_Array:
    {
        ptlv->value = bscJSON_PrintUnformatted(pvalue);
        if (NULL == ptlv->value)
        {
            rv = -1;
            break;
        }

        ptlv->length = strlen(ptlv->value);
        ptlv->type = shd_data_type_array;
        break;
    }
    case bscJSON_Object:
    {
        ptlv->value = bscJSON_PrintUnformatted(pvalue);
        if (NULL == ptlv->value)
        {
            rv = -1;
            break;
        }

        ptlv->length = strlen(ptlv->value);
        ptlv->type = shd_data_type_object;
        break;
    }
    default:
        rv = -1;
        break;
    }

    return rv;
}

static void tlv_destroy(ez_iot_shadow_value_t *ptlv)
{
    if (!ptlv)
    {
        return;
    }

    if (NULL != ptlv->value &&
        (shd_data_type_array == ptlv->type || shd_data_type_object == ptlv->type))
    {
        free(ptlv->value);
    }

    memset(ptlv, 0, sizeof(ez_iot_shadow_value_t));
}

static bool is_double(int i, double d)
{
    bool rv = true;

    if (fabs(((double)i) - d) <= DBL_EPSILON && d <= INT_MAX && d >= INT_MIN)
    {
        rv = false;
    }

    return rv;
}
