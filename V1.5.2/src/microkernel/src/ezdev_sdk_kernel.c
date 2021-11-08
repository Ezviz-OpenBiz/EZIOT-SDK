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
#include "ezdev_sdk_kernel.h"
#include "platform.h"
#include "sdk_kernel_def.h"
#include "ezdev_sdk_kernel_extend.h"
#include "ezdev_sdk_kernel_access.h"
#include "das_transport.h"
#include "json_parser.h"
#include "ase_support.h"
#include "ezdev_sdk_kernel_common.h"
#include "ezdev_sdk_kernel_extend.h"
#include "ezdev_sdk_kernel_event.h"
#include "mkernel_internal_error.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ezdev_sdk_kernel_risk_control.h"
#include "bscJSON.h"
#include "utils.h"
#include "dev_protocol_def.h"
#include "ezdev_sdk_kerne_queuel.h"
#include "ezconfig.h"
#include "mcuconfig.h"

EZDEV_SDK_KERNEL_RISK_CONTROL_INTERFACE
EZDEV_SDK_KERNEL_ACCESS_INTERFACE

extern ezdev_sdk_kernel g_ezdev_sdk_kernel;
extern char g_binding_nic[ezdev_sdk_name_len];
extern EZDEV_SDK_UINT32 g_das_transport_seq;
static const char *g_default_value = "invalidkey";
static ezdev_sdk_mutex g_mutex_lock;

#define CHECK_NULL_RETURN(p, rv) \
    if (NULL == (p))             \
    {                            \
        return (rv);             \
    }

#define CHECK_FALSE_RETURN(condition, rv) \
    if (!(condition))                     \
    {                                     \
        return (rv);                      \
    }

static EZDEV_SDK_UINT32 genaral_seq()
{
    int seq = 0;
    g_ezdev_sdk_kernel.platform_handle.thread_mutex_lock(g_mutex_lock);
    seq = ++g_das_transport_seq;
    g_ezdev_sdk_kernel.platform_handle.thread_mutex_unlock(g_mutex_lock);
    return seq;
}

EZDEV_SDK_KERNEL_EXTEND_INTERFACE
DAS_TRANSPORT_INTERFACE
EZDEV_SDK_KERNEL_ACCESS_INTERFACE
JSON_PARSER_INTERFACE
EZDEV_SDK_KERNEL_COMMON_INTERFACE
ASE_SUPPORT_INTERFACE
EZDEV_SDK_KERNEL_EVENT_INTERFACE
EXTERN_QUEUE_FUN(pubmsg_exchange)

static void *_calloc_func(size_t nmemb, size_t size)
{
    size_t mem_size;
    void *ptr = NULL;

    mem_size = nmemb * size;
    ptr = malloc(mem_size);

    if (ptr)
        memset(ptr, 0, mem_size);

    return ptr;
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_init(const ezdev_sdk_srv_info_t *psrv_info, const ezdev_sdk_dev_info_t *pdev_info,
                                             const ezdev_sdk_kernel_platform_handle *kernel_platform_handle,
                                             sdk_kernel_event_notice kernel_event_notice_cb)

{
    ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_succ;

    if (sdk_idle0 != g_ezdev_sdk_kernel.my_state && sdk_idle2 != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    memset(&g_ezdev_sdk_kernel, 0, sizeof(g_ezdev_sdk_kernel));
    bscomptls_platform_set_calloc_free(_calloc_func, free);

    CHECK_NULL_RETURN(psrv_info, ezdev_sdk_kernel_params_invalid);
    CHECK_NULL_RETURN(pdev_info, ezdev_sdk_kernel_params_invalid);
    CHECK_NULL_RETURN(kernel_platform_handle, ezdev_sdk_kernel_params_invalid);
    CHECK_NULL_RETURN(kernel_event_notice_cb, ezdev_sdk_kernel_params_invalid);
    CHECK_NULL_RETURN(psrv_info->svr_name, ezdev_sdk_kernel_params_invalid);
    CHECK_FALSE_RETURN(strlen((const char *)psrv_info->svr_name) < ezdev_sdk_name_len, ezdev_sdk_kernel_params_invalid);

    /* 初始化服務信息 */
    g_ezdev_sdk_kernel.das_keepalive_interval = ezdev_sdk_das_default_keepaliveinterval;
    strncpy(g_ezdev_sdk_kernel.server_info.server_name, (char *)psrv_info->svr_name, ezdev_sdk_name_len - 1);
    g_ezdev_sdk_kernel.server_info.server_port = psrv_info->svr_port;

    /* 初始化devinfo */
    g_ezdev_sdk_kernel.dev_info.dev_auth_mode = pdev_info->auth_mode;
    g_ezdev_sdk_kernel.dev_info.dev_status = 1;
    g_ezdev_sdk_kernel.dev_info.dev_oeminfo = 0;
    strncpy(g_ezdev_sdk_kernel.dev_info.dev_subserial, (char *)pdev_info->dev_subserial, sizeof(g_ezdev_sdk_kernel.dev_info.dev_subserial) - 1);
    strncpy(g_ezdev_sdk_kernel.dev_info.dev_verification_code, (char *)pdev_info->dev_verification_code, sizeof(g_ezdev_sdk_kernel.dev_info.dev_verification_code) - 1);
    strncpy(g_ezdev_sdk_kernel.dev_info.dev_serial, (char *)pdev_info->dev_serial, sizeof(g_ezdev_sdk_kernel.dev_info.dev_serial) - 1);
    strncpy(g_ezdev_sdk_kernel.dev_info.dev_firmwareversion, (char *)pdev_info->dev_firmwareversion, sizeof(g_ezdev_sdk_kernel.dev_info.dev_firmwareversion) - 1);
    strncpy(g_ezdev_sdk_kernel.dev_info.dev_type, (char *)pdev_info->dev_type, sizeof(g_ezdev_sdk_kernel.dev_info.dev_type) - 1);
    strncpy(g_ezdev_sdk_kernel.dev_info.dev_typedisplay, (char *)pdev_info->dev_typedisplay, sizeof(g_ezdev_sdk_kernel.dev_info.dev_typedisplay) - 1);
    strncpy(g_ezdev_sdk_kernel.dev_info.dev_mac, (char *)pdev_info->dev_mac, sizeof(g_ezdev_sdk_kernel.dev_info.dev_mac) - 1);
    strncpy(g_ezdev_sdk_kernel.dev_info.dev_nickname, (char *)pdev_info->dev_nickname, sizeof(g_ezdev_sdk_kernel.dev_info.dev_nickname) - 1);

    strncpy((char *)g_ezdev_sdk_kernel.master_key, (char *)pdev_info->dev_master_key, sizeof(g_ezdev_sdk_kernel.master_key));
    strncpy((char *)g_ezdev_sdk_kernel.dev_id, (char *)pdev_info->dev_id, sizeof(g_ezdev_sdk_kernel.dev_id));

    /* 设置回调函数 */
    memcpy(&g_ezdev_sdk_kernel.platform_handle, kernel_platform_handle, sizeof(ezdev_sdk_kernel_platform_handle));

    /* 初始化链接状态 */
    g_ezdev_sdk_kernel.lbs_redirect_times = 0;
    g_ezdev_sdk_kernel.das_retry_times = 0;
    g_ezdev_sdk_kernel.entr_state = sdk_entrance_normal;
    g_ezdev_sdk_kernel.my_state = sdk_idle0;
    g_ezdev_sdk_kernel.cnt_state = sdk_cnt_unredirect;
    g_ezdev_sdk_kernel.cnt_state_timer = g_ezdev_sdk_kernel.platform_handle.time_creator();
    g_ezdev_sdk_kernel.access_risk = sdk_no_risk_control;
    g_ezdev_sdk_kernel.reg_mode = 1;

    /* 初始化领域和公共领域 */
    extend_init(kernel_event_notice_cb);

    /* 初始化MQTT和消息队列 */
    das_object_init(&g_ezdev_sdk_kernel);
    g_mutex_lock = g_ezdev_sdk_kernel.platform_handle.thread_mutex_create();
    g_das_transport_seq = 0;
    g_ezdev_sdk_kernel.my_state = sdk_idle;

    return sdk_error;
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_fini()
{
    if (sdk_idle != g_ezdev_sdk_kernel.my_state && sdk_stop != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    das_object_fini(&g_ezdev_sdk_kernel);
    extend_fini();
    common_module_fini();

    g_ezdev_sdk_kernel.platform_handle.time_destroy(g_ezdev_sdk_kernel.cnt_state_timer);
    g_ezdev_sdk_kernel.platform_handle.thread_mutex_destroy(g_mutex_lock);
    g_mutex_lock = 0;

    memset(&g_ezdev_sdk_kernel, 0, sizeof(g_ezdev_sdk_kernel));
    return ezdev_sdk_kernel_succ;
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_extend_load(const ezdev_sdk_kernel_extend *external_extend)
{
    if (sdk_idle != g_ezdev_sdk_kernel.my_state && sdk_start != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    if (NULL == external_extend ||
        NULL == external_extend->ezdev_sdk_kernel_extend_data_route ||
        NULL == external_extend->ezdev_sdk_kernel_extend_start ||
        NULL == external_extend->ezdev_sdk_kernel_extend_event ||
        NULL == external_extend->ezdev_sdk_kernel_extend_stop)
    {
        return ezdev_sdk_kernel_params_invalid;
    }

    return mkiE2ezE(extend_load(external_extend));
}


ezdev_sdk_kernel_error ezdev_sdk_kernel_extend_load_v3(const ezdev_sdk_kernel_extend_v3 *external_extend_v3)
{
    if (sdk_idle != g_ezdev_sdk_kernel.my_state && sdk_start != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    if (NULL == external_extend_v3 || NULL == external_extend_v3->ezdev_sdk_kernel_data_route)
    {
        return ezdev_sdk_kernel_params_invalid;
    }

    g_ezdev_sdk_kernel.v3_reg_status = sdk_v3_reged;

    return mkiE2ezE(extend_load_v3(external_extend_v3));
}


ezdev_sdk_kernel_error ezdev_sdk_kernel_common_module_load(const ezdev_sdk_kernel_common_module *common_module)
{
    if (sdk_idle != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    if (NULL == common_module || NULL == common_module->ezdev_sdk_kernel_common_module_data_handle)
    {
        return ezdev_sdk_kernel_params_invalid;
    }

    return mkiE2ezE(common_module_load(common_module));
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_hub_extend_load(const  ezdev_sdk_kernel_hub_extend *extend_info)
{
    ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_succ;
    if (sdk_idle != g_ezdev_sdk_kernel.my_state && sdk_start != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }
    sdk_error = mkiE2ezE(sdk_hub_extend_load(extend_info));
    
    g_ezdev_sdk_kernel.hub_reg_status = 1;
    
    return sdk_error; 
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_start()
{
    if (sdk_idle != g_ezdev_sdk_kernel.my_state && sdk_stop !=  g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    g_ezdev_sdk_kernel.my_state = sdk_start;
    broadcast_user_start();
    return ezdev_sdk_kernel_succ;
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_stop()
{
    int wait4times = 0;
    ezdev_sdk_kernel_pubmsg_exchange *ptr_pubmsg_exchange = NULL;

    if (sdk_start != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    if (check_access_risk_control(&g_ezdev_sdk_kernel))
    {
        ezdev_sdk_kernel_log_error(0, 0, "dev is in black list true ");
        return ezdev_sdk_kernel_das_force_dev_risk;
    }

    g_ezdev_sdk_kernel.my_state = sdk_stop;
    clear_queue_pubmsg_exchange();
    send_offline_msg_to_platform(genaral_seq());

    while (sdk_cnt_das_reged == g_ezdev_sdk_kernel.cnt_state)
    {
        if (mkernel_internal_succ != access_server_yield(&g_ezdev_sdk_kernel))
            break;

        if (mkernel_internal_queue_empty == get_queue_pubmsg_exchange(&ptr_pubmsg_exchange))
            break;

        g_ezdev_sdk_kernel.platform_handle.time_sleep(100);
        if (wait4times++ < 3)
            break;
    }

    stop_yield(&g_ezdev_sdk_kernel);

    return ezdev_sdk_kernel_succ;
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_yield()
{
    if (sdk_start != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    return mkiE2ezE(access_server_yield(&g_ezdev_sdk_kernel));
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_yield_user()
{
    if (sdk_start != g_ezdev_sdk_kernel.my_state)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    return mkiE2ezE(extend_yield(&g_ezdev_sdk_kernel));
}

ezdev_sdk_kernel_error ezdev_sdk_kernel_send(ezdev_sdk_kernel_pubmsg *pubmsg)
{
    /* 由于对于body的处理后期需要用ase做加密，直接在这里做padding */
    EZDEV_SDK_INT32 input_length_padding = 0;
    ezdev_sdk_kernel_pubmsg_exchange *new_pubmsg_exchange = NULL;
    ezdev_sdk_kernel_error kernel_error = ezdev_sdk_kernel_succ;
    char cRiskResult = 0;

    if (g_ezdev_sdk_kernel.my_state != sdk_start)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    ezdev_sdk_kernel_log_debug(0, 0, "send, domain:%d , cmd:%d, seq:%d, len:%d, string:%s", pubmsg->msg_domain_id, pubmsg->msg_command_id, pubmsg->msg_seq, pubmsg->msg_body_len, pubmsg->msg_body);

    if (pubmsg->msg_body == NULL || pubmsg->msg_body_len == 0)
    {
        return ezdev_sdk_kernel_params_invalid;
    }
    if(pubmsg->msg_command_id != SendISAPIReq && pubmsg->msg_command_id != SendISAPIRsp && \
       pubmsg->msg_command_id !=SendISAPIEventReq && pubmsg->msg_command_id != SendISAPIEventRsp)
    {
        if(pubmsg->msg_body_len > ezdev_sdk_com_msg_max_buf)
        {
            return ezdev_sdk_kernel_data_len_range;
        }
    }

    if (pubmsg->msg_body_len > ezdev_sdk_send_buf_max)
    {
        return ezdev_sdk_kernel_data_len_range;
    }

    cRiskResult = check_cmd_risk_control(&g_ezdev_sdk_kernel, pubmsg->msg_domain_id, pubmsg->msg_command_id);
    if (1 == cRiskResult)
        return mkiE2ezE(mkernel_internal_extend_no_find);
    else if (2 == cRiskResult)
        return mkiE2ezE(mkernel_internal_force_domain_risk);
    else if (3 == cRiskResult)
        return mkiE2ezE(mkernel_internal_force_cmd_risk);

    new_pubmsg_exchange = (ezdev_sdk_kernel_pubmsg_exchange *)malloc(sizeof(ezdev_sdk_kernel_pubmsg_exchange));
    if (new_pubmsg_exchange == NULL)
    {
        return ezdev_sdk_kernel_memory;
    }

    memset(new_pubmsg_exchange, 0, sizeof(ezdev_sdk_kernel_pubmsg_exchange));
    strncpy(new_pubmsg_exchange->msg_conntext.command_ver, pubmsg->command_ver, version_max_len - 1);
    new_pubmsg_exchange->msg_conntext.msg_response = pubmsg->msg_response;
    new_pubmsg_exchange->msg_conntext.msg_qos = pubmsg->msg_qos;
    new_pubmsg_exchange->msg_conntext.msg_seq = pubmsg->msg_seq;
    new_pubmsg_exchange->msg_conntext.msg_domain_id = pubmsg->msg_domain_id;
    new_pubmsg_exchange->msg_conntext.msg_command_id = pubmsg->msg_command_id;

    input_length_padding = pubmsg->msg_body_len;
    new_pubmsg_exchange->msg_conntext.msg_body = (unsigned char *)malloc(input_length_padding);
    if (new_pubmsg_exchange->msg_conntext.msg_body == NULL)
    {
        free(new_pubmsg_exchange);
        new_pubmsg_exchange = NULL;

        ezdev_sdk_kernel_log_error(ezdev_sdk_kernel_memory, ezdev_sdk_kernel_memory, "malloc input_length_padding:%d err", input_length_padding);
        return ezdev_sdk_kernel_memory;
    }
    memset(new_pubmsg_exchange->msg_conntext.msg_body, 0, input_length_padding);
    new_pubmsg_exchange->msg_conntext.msg_body_len = input_length_padding;
    memcpy(new_pubmsg_exchange->msg_conntext.msg_body, pubmsg->msg_body, pubmsg->msg_body_len);
    buf_padding(new_pubmsg_exchange->msg_conntext.msg_body, input_length_padding, pubmsg->msg_body_len);
    new_pubmsg_exchange->max_send_count = ezdev_sdk_max_publish_count; //默认最多发送2次

    if (pubmsg->msg_response == 0)
    {
        pubmsg->msg_seq = genaral_seq();
        new_pubmsg_exchange->msg_conntext.msg_seq = pubmsg->msg_seq;
    }

    if (NULL != pubmsg->externel_ctx && 0 != pubmsg->externel_ctx_len)
    {
        new_pubmsg_exchange->msg_conntext.externel_ctx = (unsigned char *)malloc(pubmsg->externel_ctx_len);
        if (NULL == new_pubmsg_exchange->msg_conntext.externel_ctx)
        {
            free(new_pubmsg_exchange->msg_conntext.msg_body);
            free(new_pubmsg_exchange);
            ezdev_sdk_kernel_log_error(ezdev_sdk_kernel_memory, 0, "malloc externel_ctx:%d err", pubmsg->externel_ctx_len);
            return ezdev_sdk_kernel_memory;
        }

        memcpy(new_pubmsg_exchange->msg_conntext.externel_ctx, pubmsg->externel_ctx, pubmsg->externel_ctx_len);
        new_pubmsg_exchange->msg_conntext.externel_ctx_len = pubmsg->externel_ctx_len;
    }

    /*非阻塞式往消息队列里push内容 最终由SDKboot模块创建的主线程驱动消息发送*/
    kernel_error = mkiE2ezE(das_send_pubmsg_async(&g_ezdev_sdk_kernel, new_pubmsg_exchange));
    if (kernel_error != ezdev_sdk_kernel_succ)
    {
        if (new_pubmsg_exchange != NULL)
        {
            if (new_pubmsg_exchange->msg_conntext.msg_body != NULL)
            {
                free(new_pubmsg_exchange->msg_conntext.msg_body);
                new_pubmsg_exchange->msg_conntext.msg_body = NULL;
            }

            if (NULL != new_pubmsg_exchange->msg_conntext.externel_ctx)
            {
                free(new_pubmsg_exchange->msg_conntext.externel_ctx);
            }

            free(new_pubmsg_exchange);
            new_pubmsg_exchange = NULL;
        }
    }
    return kernel_error;
}



ezdev_sdk_kernel_error ezdev_sdk_kernel_send_v3(ezdev_sdk_kernel_pubmsg_v3 *pubmsg)
{
    /* 由于对于body的处理后期需要用ase做加密，直接在这里做padding */
    EZDEV_SDK_INT32 input_length_padding = 0;
    ezdev_sdk_kernel_pubmsg_exchange_v3 *new_pubmsg_exchange = NULL;
    ezdev_sdk_kernel_error kernel_error = ezdev_sdk_kernel_succ;

    if (g_ezdev_sdk_kernel.my_state != sdk_start)
    {
        return ezdev_sdk_kernel_invald_call;
    }

    if (pubmsg->msg_body == NULL || pubmsg->msg_body_len == 0)
    {
        ezdev_sdk_kernel_log_info(0, 0, "ezdev_sdk_kernel_send_v3 input msg invalid");
        return ezdev_sdk_kernel_params_invalid;
    }

    if (pubmsg->msg_body_len > ezdev_sdk_send_buf_max)
    {
        return ezdev_sdk_kernel_data_len_range;
    }
    
    ezdev_sdk_kernel_log_debug(0, 0,"send buffer size: %d", pubmsg->msg_body_len);

    new_pubmsg_exchange = (ezdev_sdk_kernel_pubmsg_exchange_v3 *)malloc(sizeof(ezdev_sdk_kernel_pubmsg_exchange_v3));
    if (new_pubmsg_exchange == NULL)
    {
        return ezdev_sdk_kernel_memory;
    }

    memset(new_pubmsg_exchange, 0, sizeof(ezdev_sdk_kernel_pubmsg_exchange_v3));
    strncpy(new_pubmsg_exchange->msg_conntext_v3.command_ver, pubmsg->command_ver, version_max_len - 1);
    new_pubmsg_exchange->msg_conntext_v3.msg_response = pubmsg->msg_response;
    new_pubmsg_exchange->msg_conntext_v3.msg_qos = pubmsg->msg_qos;
    new_pubmsg_exchange->msg_conntext_v3.msg_seq = pubmsg->msg_seq;
    new_pubmsg_exchange->msg_conntext_v3.extend_id = pubmsg->extend_id;

    strncpy(new_pubmsg_exchange->msg_conntext_v3.domain_id, pubmsg->domain_id, ezdev_sdk_domain_id_len - 1);
    strncpy(new_pubmsg_exchange->msg_conntext_v3.resource_id, pubmsg->resource_id, ezdev_sdk_resource_id_len - 1);
    strncpy(new_pubmsg_exchange->msg_conntext_v3.resource_type, pubmsg->resource_type, ezdev_sdk_resource_type_len - 1);
    strncpy(new_pubmsg_exchange->msg_conntext_v3.business_type, pubmsg->business_type, ezdev_sdk_business_type_len - 1);
    strncpy(new_pubmsg_exchange->msg_conntext_v3.identifier, pubmsg->identifier, ezdev_sdk_identifier_len - 1);
    strncpy(new_pubmsg_exchange->msg_conntext_v3.sub_serial, pubmsg->sub_serial, ezdev_sdk_max_serial_len - 1);
 
    input_length_padding = pubmsg->msg_body_len;
    new_pubmsg_exchange->msg_conntext_v3.msg_body = (unsigned char *)malloc(input_length_padding);
    if (new_pubmsg_exchange->msg_conntext_v3.msg_body == NULL)
    {
        free(new_pubmsg_exchange);
        new_pubmsg_exchange = NULL;

        ezdev_sdk_kernel_log_error(ezdev_sdk_kernel_memory, 0, "malloc input_length_padding:%d error", input_length_padding);
        return ezdev_sdk_kernel_memory;
    }

    memset(new_pubmsg_exchange->msg_conntext_v3.msg_body, 0, input_length_padding);
    new_pubmsg_exchange->msg_conntext_v3.msg_body_len = input_length_padding;
    memcpy(new_pubmsg_exchange->msg_conntext_v3.msg_body, pubmsg->msg_body, pubmsg->msg_body_len);
    buf_padding(new_pubmsg_exchange->msg_conntext_v3.msg_body, input_length_padding, pubmsg->msg_body_len);
    new_pubmsg_exchange->max_send_count = ezdev_sdk_max_publish_count; //默认最多发送2次

    if (pubmsg->msg_response == 0)
    {
        pubmsg->msg_seq = genaral_seq();
        new_pubmsg_exchange->msg_conntext_v3.msg_seq = pubmsg->msg_seq;
    }

     ezdev_sdk_kernel_log_info(0, 0, "sdk send v3:dev_sn:%s, extend_id:%d ,domain_id:%s, resource_id:%s,resource_type:%s,business_type:%s, identifier:%s, seq:%d ,payload:%s", pubmsg->sub_serial, pubmsg->extend_id, \
                              pubmsg->domain_id, pubmsg->resource_id, pubmsg->resource_type, pubmsg->business_type, pubmsg->identifier, pubmsg->msg_seq, pubmsg->msg_body);

    if (NULL != pubmsg->externel_ctx && 0 != pubmsg->externel_ctx_len)
    {
        new_pubmsg_exchange->msg_conntext_v3.externel_ctx = (unsigned char *)malloc(pubmsg->externel_ctx_len);
        if (NULL == new_pubmsg_exchange->msg_conntext_v3.externel_ctx)
        {
            free(new_pubmsg_exchange->msg_conntext_v3.msg_body);
            free(new_pubmsg_exchange);
            ezdev_sdk_kernel_log_error(ezdev_sdk_kernel_memory, 0, "malloc externel_ctx:%d error", pubmsg->externel_ctx_len);
            return ezdev_sdk_kernel_memory;
        }

        memcpy(new_pubmsg_exchange->msg_conntext_v3.externel_ctx, pubmsg->externel_ctx, pubmsg->externel_ctx_len);
        new_pubmsg_exchange->msg_conntext_v3.externel_ctx_len = pubmsg->externel_ctx_len;
    }

    /*非阻塞式往消息队列里push内容 最终由SDKboot模块创建的主线程驱动消息发送*/
    kernel_error = mkiE2ezE(das_send_pubmsg_async_v3(&g_ezdev_sdk_kernel, new_pubmsg_exchange));
    if (kernel_error != ezdev_sdk_kernel_succ)
    {
        if (new_pubmsg_exchange != NULL)
        {
            if (new_pubmsg_exchange->msg_conntext_v3.msg_body != NULL)
            {
                free(new_pubmsg_exchange->msg_conntext_v3.msg_body);
                new_pubmsg_exchange->msg_conntext_v3.msg_body = NULL;
            }

            if (NULL != new_pubmsg_exchange->msg_conntext_v3.externel_ctx)
            {
                free(new_pubmsg_exchange->msg_conntext_v3.externel_ctx);
            }

            free(new_pubmsg_exchange);
            new_pubmsg_exchange = NULL;
        }
    }
    return kernel_error;
}

EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_set_net_option(int optname, const void *optval, int optlen)
{
    ezdev_sdk_kernel_error rv = ezdev_sdk_kernel_succ;

    switch (optname)
    {
    case 1:
    {
        if (NULL == optval || optlen + 1 > sizeof(g_binding_nic))
        {
            rv = ezdev_sdk_kernel_buffer_too_small;
            break;
        }

        memset(g_binding_nic, 0, sizeof(g_binding_nic));
        memcpy(g_binding_nic, (char *)optval, optlen);
        break;
    }
    case 3:
    {
        if (sdk_cnt_das_reged == g_ezdev_sdk_kernel.cnt_state)
            g_ezdev_sdk_kernel.cnt_state = sdk_cnt_das_break;

        break;
    }
    default:
    {
        rv = ezdev_sdk_kernel_params_invalid;
        break;
    }
    }

    return rv;
}

const char *inner_get(const char *key)
{
    if (strcmp(key, "dev_subserial") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_subserial;
    }
    else if (strcmp(key, "dev_serial") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_serial;
    }
    else if (strcmp(key, "dev_firmwareversion") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_firmwareversion;
    }
    else if (strcmp(key, "dev_type") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_type;
    }
    else if (strcmp(key, "dev_typedisplay") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_typedisplay;
    }
    else if (strcmp(key, "dev_mac") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_mac;
    }
    else if (strcmp(key, "dev_nickname") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_nickname;
    }
    else if (strcmp(key, "dev_firmwareidentificationcode") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_firmwareidentificationcode;
    }
    else if (strcmp(key, "dev_verification_code") == 0)
    {
        return g_ezdev_sdk_kernel.dev_info.dev_verification_code;
    }
    else if (strcmp(key, "dev_binding_nic") == 0)
    {
        return g_binding_nic;
    }
    else
    {
        return g_default_value;
    }
}

const char *ezdev_sdk_kernel_getdevinfo_bykey(const char *key)
{
    if (g_ezdev_sdk_kernel.my_state == sdk_idle0)
    {
        return g_default_value;
    }

    //ezdev_sdk_kernel_log_trace(0, 0, "ezdev_sdk_kernel_getdevinfo_bykey key:%s, value:%s", key, inner_get(key));
    return inner_get(key);
}

EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_get_sdk_version(char *pbuf, int *pbuflen)
{
    ezdev_sdk_kernel_error rv = ezdev_sdk_kernel_succ;
    char buf[64] = {0};

    if (NULL == pbuflen)
        return ezdev_sdk_kernel_params_invalid;

    get_module_build_date(buf);

    do
    {
        if (NULL == pbuf)
        {
            *pbuflen = strlen(buf) + strlen(DEV_ACCESS_DOMAIN_VERSION);
            break;
        }

        if (*pbuflen < strlen(buf) + strlen(DEV_ACCESS_DOMAIN_VERSION))
        {
            rv = ezdev_sdk_kernel_buffer_too_small;
            break;
        }

        sprintf(pbuf, "%s%s", DEV_ACCESS_DOMAIN_VERSION, buf);
        *pbuflen = strlen(buf) + strlen(DEV_ACCESS_DOMAIN_VERSION);
    } while (0);

    return rv;
}

EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_get_server_info(server_info_s *ptr_server_info, int *ptr_count)
{
    if (g_ezdev_sdk_kernel.my_state != sdk_start)
        return ezdev_sdk_kernel_invald_call;

    if (NULL == ptr_count || (NULL != ptr_server_info && 1 > *ptr_count))
        return ezdev_sdk_kernel_params_invalid;

    if (NULL == ptr_server_info)
    {
        //现在只有一对LBS、DAS
        *ptr_count = 1;
        return ezdev_sdk_kernel_succ;
    }

    strncpy((*ptr_server_info).lbs_domain, g_ezdev_sdk_kernel.server_info.server_name, ezdev_sdk_ip_max_len - 1);
    strncpy((*ptr_server_info).lbs_ip, g_ezdev_sdk_kernel.server_info.server_ip, ezdev_sdk_ip_max_len - 1);
    (*ptr_server_info).lbs_port = g_ezdev_sdk_kernel.server_info.server_port;

    strncpy((*ptr_server_info).das_domain, g_ezdev_sdk_kernel.redirect_das_info.das_domain, ezdev_sdk_ip_max_len - 1);
    strncpy((*ptr_server_info).das_ip, g_ezdev_sdk_kernel.redirect_das_info.das_address, ezdev_sdk_ip_max_len - 1);
    (*ptr_server_info).das_port = g_ezdev_sdk_kernel.redirect_das_info.das_port;
    (*ptr_server_info).das_udp_port = g_ezdev_sdk_kernel.redirect_das_info.das_udp_port;
    (*ptr_server_info).das_socket = ezdev_sdk_kernel_get_das_socket(&g_ezdev_sdk_kernel);
    memcpy((*ptr_server_info).session_key, g_ezdev_sdk_kernel.session_key, ezdev_sdk_sessionkey_len);
    *ptr_count = 1;

    return ezdev_sdk_kernel_succ;
}

EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_show_key_info(showkey_info *ptr_showkey_info)
{
    if (g_ezdev_sdk_kernel.my_state != sdk_start)
        return ezdev_sdk_kernel_invald_call;

    if (NULL == ptr_showkey_info)
        return ezdev_sdk_kernel_params_invalid;

    memset(ptr_showkey_info, 0, sizeof(showkey_info));

    if (0 != strlen((const char *)g_ezdev_sdk_kernel.master_key))
    {
        strncpy((char *)ptr_showkey_info->master_key, (const char *)g_ezdev_sdk_kernel.master_key, ezdev_sdk_masterkey_len);
    }
    if (0 != strlen((const char *)g_ezdev_sdk_kernel.dev_id))
    {
        strncpy((char *)ptr_showkey_info->dev_id, (const char *)g_ezdev_sdk_kernel.dev_id, ezdev_sdk_devid_len);
    }
    if (0 != strlen((const char *)g_ezdev_sdk_kernel.dev_info.dev_verification_code))
    {
        strncpy((char *)ptr_showkey_info->dev_verification_code, (const char *)g_ezdev_sdk_kernel.dev_info.dev_verification_code, ezdev_sdk_verify_code_maxlen);
    }

    return ezdev_sdk_kernel_succ;
}