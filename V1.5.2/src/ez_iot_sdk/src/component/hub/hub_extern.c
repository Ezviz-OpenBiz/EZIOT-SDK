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
#include "hub_extern.h"
#include "ezconfig.h"
#include "mcuconfig.h"
#include "ez_iot_log.h"
#include "ezdev_sdk_kernel.h"
#include "ez_protocol.h"
#include "hub_func.h"

#define hub_cmd_version "v1.0.0"

static void hub_extend_start_cb(EZDEV_SDK_PTR pUser)
{
    hub_subdev_list_report();
}

static void hub_extend_stop_cb(EZDEV_SDK_PTR pUser)
{
}

static void hub_extend_data_route_cb(ezdev_sdk_kernel_submsg *ptr_submsg, EZDEV_SDK_PTR pUser)
{
    if (ptr_submsg == NULL)
    {
        return;
    }

    ez_log_w(TAG_HUB, "cmd:%d", ptr_submsg->msg_command_id);
    ez_log_d(TAG_HUB, "seq:%d, len:%d, data:%s", ptr_submsg->msg_seq, ptr_submsg->buf_len, (char*)ptr_submsg->buf);

    if (kPu2CenPltHubAuthChildDeviceRsp == ptr_submsg->msg_command_id)
    {
        hub_subdev_auth_done(ptr_submsg->buf, ptr_submsg->buf_len);
    }
}

static void hub_extend_event_cb(ezdev_sdk_kernel_event *ptr_event, EZDEV_SDK_PTR pUser)
{
    if (sdk_kernel_event_online == ptr_event->event_type ||
        sdk_kernel_event_switchover == ptr_event->event_type)
    {
        hub_subdev_list_report();
    }
}

int hub_extern_init()
{
    ezdev_sdk_kernel_extend extern_info;
    memset(&extern_info, 0, sizeof(ezdev_sdk_kernel_extend));

    extern_info.domain_id = hub_module_id;
    extern_info.pUser = NULL;
    extern_info.ezdev_sdk_kernel_extend_start = hub_extend_start_cb;
    extern_info.ezdev_sdk_kernel_extend_stop = hub_extend_stop_cb;
    extern_info.ezdev_sdk_kernel_extend_data_route = hub_extend_data_route_cb;
    extern_info.ezdev_sdk_kernel_extend_event = hub_extend_event_cb;

    strncpy(extern_info.extend_module_name, hub_module_name, ezdev_sdk_extend_name_len);
    strncpy(extern_info.extend_module_version, hub_module_version, version_max_len);

    int ret = ezdev_sdk_kernel_extend_load(&extern_info);
    return ret;
}

int hub_extern_finit()
{
    return 0;
}

int hub_send_msg_to_platform(const char *msg, int msg_len, int cmd_id, unsigned char msg_response, unsigned int msg_seq)
{
    ezdev_sdk_kernel_pubmsg pubmsg;
    memset(&pubmsg, 0, sizeof(ezdev_sdk_kernel_pubmsg));

    pubmsg.msg_response = msg_response;
    pubmsg.msg_seq = msg_seq;

    pubmsg.msg_body = (unsigned char *)msg;
    pubmsg.msg_body_len = msg_len;

    pubmsg.msg_domain_id = hub_module_id;
    pubmsg.msg_command_id = cmd_id;

    strncpy(pubmsg.command_ver, hub_module_version, version_max_len - 1);

    ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_send(&pubmsg);
    if (sdk_error != ezdev_sdk_kernel_succ)
    {
        return -1;
    }

    return 0;
}