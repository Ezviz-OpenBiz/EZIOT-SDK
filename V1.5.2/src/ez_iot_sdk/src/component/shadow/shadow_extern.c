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
#include "hal_thread.h"
#include "shadow.h"
#include "shadow_func.h"
#include "shadow_extern.h"
#include "ez_iot_log.h"
#include "shadow_core.h"
#include "ezconfig.h"
#include "mcuconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    static int g_status_push_flag = 0;
    static int g_shadow_status = SHADOW_INITIAL_STATUS;
    static int g_switchover_flag = 0;
    static void *g_mutex;
    static int g_register_status = 0; //SDK在线状态

    static void Shadow_extend_start_cb(EZDEV_SDK_PTR pUser)
    {
        ez_log_d(TAG_SHADOW, "start_cb");
    }

    static void Shadow_extend_stop_cb(EZDEV_SDK_PTR pUser)
    {
        ez_log_d(TAG_SHADOW, "stop_cb");
    }

    static void Shadow_extend_data_route_cb(ezdev_sdk_kernel_submsg *ptr_submsg, EZDEV_SDK_PTR pUser)
    {

    }

    static void Shadow_v3_extend_data_route_cb(ezdev_sdk_kernel_submsg_v3 *psub_msg, EZDEV_SDK_PTR puser)
    {
        ez_iot_shadow_res_t shadow_res = {0};

        if (sdk_shadow != psub_msg->extend_id)
        {
            ez_log_e(TAG_SHADOW, "extend_id mismatch");
            return;
        }

        strncpy((char *)shadow_res.dev_serial, strlen(psub_msg->sub_serial) ? psub_msg->sub_serial : "global", sizeof(shadow_res.dev_serial) - 1);
        strncpy((char *)shadow_res.res_type, psub_msg->resource_type, sizeof(shadow_res.res_type) - 1);
        shadow_res.local_index = atoi(psub_msg->resource_id);

        shadow_core_cloud_data_in((void *)&shadow_res, psub_msg->msg_seq, (int8_t *)psub_msg->business_type, (int8_t *)psub_msg->buf);
    }

    static void Shadow_extend_event_cb(ezdev_sdk_kernel_event *ptr_event, EZDEV_SDK_PTR pUser)
    {
        switch (ptr_event->event_type)
        {
        case sdk_kernel_event_online:
        {
            shadow_core_event_occured(shadow_event_type_online);
            break;
        }
        case sdk_kernel_event_fast_reg_online:
        {
            break;
        }
        case sdk_kernel_event_break:
        {
            shadow_core_event_occured(shadow_event_type_offline);
            break;
        }
        case sdk_kernel_event_switchover:
        {
            g_switchover_flag = 1;
            shadow_core_event_occured(shadow_event_type_reset);
            break;
        }
        case sdk_kernel_event_runtime_err:
        {
            sdk_runtime_err_context *err_ctx = (sdk_runtime_err_context *)ptr_event->event_context;
            if (TAG_MSG_ACK == err_ctx->err_tag)
            {
                sdk_send_msg_ack_context *ack_ctx = (sdk_send_msg_ack_context *)err_ctx->err_ctx;

                ez_log_d(TAG_SHADOW, "runtime err,code= %d ,seq= %d", err_ctx->err_code, ack_ctx->msg_seq);
            }
            else if (TAG_MSG_ACK_V3 == err_ctx->err_tag)
            {
                sdk_runtime_err_context *err_ctx = (sdk_runtime_err_context *)ptr_event->event_context;
                sdk_send_msg_ack_context_v3 *ack_ctx = (sdk_send_msg_ack_context_v3 *)err_ctx->err_ctx;
                ez_iot_shadow_res_t shadow_res = {0};
                int32_t err_code = err_ctx->err_code;

                if (ack_ctx->extend_id == sdk_shadow)
                {
                    strncpy((char *)shadow_res.dev_serial, ack_ctx->sub_serial, sizeof(shadow_res.dev_serial) - 1);
                    strncpy((char *)shadow_res.res_type, ack_ctx->resource_type, sizeof(shadow_res.res_type) - 1);
                    shadow_res.local_index = atoi(ack_ctx->resource_id);

                    shadow_core_cloud_data_in((void *)&shadow_res, ack_ctx->msg_seq, (int8_t *)ack_ctx->business_type, (int8_t *)&err_code);
                }
            }

            break;
        }
        case sdk_kernel_event_reconnect_success:
        {
            shadow_core_event_occured(shadow_event_type_offline);
            break;
        }
        default:
            break;
        }
    }

    int Shadow_extern_init(void)
    {
        ezdev_sdk_kernel_extend extern_info;
        ezdev_sdk_kernel_extend_v3 extend_info_v3;
        memset(&extern_info, 0, sizeof(ezdev_sdk_kernel_extend));
        memset(&extend_info_v3, 0, sizeof(ezdev_sdk_kernel_extend_v3));
        extern_info.domain_id = shadow_module_id;
        extern_info.pUser = NULL;
        extern_info.ezdev_sdk_kernel_extend_start = Shadow_extend_start_cb;
        extern_info.ezdev_sdk_kernel_extend_stop = Shadow_extend_stop_cb;
        extern_info.ezdev_sdk_kernel_extend_data_route = Shadow_extend_data_route_cb;
        extern_info.ezdev_sdk_kernel_extend_event = Shadow_extend_event_cb;

        snprintf(extern_info.extend_module_name, ezdev_sdk_extend_name_len, "%s", shadow_module_name);
        snprintf(extern_info.extend_module_version, version_max_len, "%s", shadow_module_version);

        int ret = ezdev_sdk_kernel_extend_load(&extern_info);
        if (ret != 0)
        {
            return -1;
        }

        extend_info_v3.extend_id = sdk_shadow;
        extend_info_v3.pUser = NULL;
        extend_info_v3.ezdev_sdk_kernel_data_route = Shadow_v3_extend_data_route_cb;
        strncpy(extend_info_v3.extend_module_name, "shadow", sizeof(extend_info_v3.extend_module_name) - 1);
        ret = ezdev_sdk_kernel_extend_load_v3(&extend_info_v3);
        if (ez_errno_succ != ret)
        {
            ez_log_e(TAG_SDK, "sdk kernel load_v3 error.");
            return -1;
        }

        g_mutex = hal_thread_mutex_create();
        if (NULL == g_mutex)
        {
            ez_log_e(TAG_SHADOW, "create g_mutex err!");
            return -1;
        }

        Shadow_Set_Status(SHADOW_INITIAL_STATUS);

        g_status_push_flag = 0;
        g_switchover_flag = 0;
        g_register_status = SHADOW_REG_STATUS_OFF;

        return 0;
    }

    int Shadow_get_status(void)
    {
        return g_shadow_status;
    }

    int Shadow_Set_Status(int value)
    {
        ez_log_d(TAG_SHADOW, "Set_Status value :%d", value);

        hal_thread_mutex_lock(g_mutex);
        g_shadow_status = value;
        hal_thread_mutex_unlock(g_mutex);
        return g_shadow_status;
    }

    int Shadow_Get_Switchover_flag()
    {
        return g_switchover_flag;
    }

    int Shadow_Set_Switchover_flag(int value)
    {
        g_switchover_flag = value;
        return g_switchover_flag;
    }

    int set_start_push_flag(int value)
    {
        g_status_push_flag = value;

        return g_status_push_flag;
    }

    int get_start_push_flag()
    {
        return g_status_push_flag;
    }

    void shadow_set_register_status(int status)
    {
        g_register_status = status;
    }

    int shadow_get_register_status()
    {
        return g_register_status;
    }

    int Shadow_extern_fini(void)
    {
        g_status_push_flag = 0;
        g_switchover_flag = 0;
        g_register_status = SHADOW_REG_STATUS_OFF;
        g_shadow_status = SHADOW_INITIAL_STATUS;

        if (g_mutex)
        {
            hal_thread_mutex_destroy(g_mutex);
        }

        ez_log_d(TAG_SHADOW, "extern_fini end!");

        return 0;
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */
