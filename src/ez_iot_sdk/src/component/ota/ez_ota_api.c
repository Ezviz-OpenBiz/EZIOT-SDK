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
* 2021/01/01    shenhongyin
*******************************************************************************/

#include <string.h>
#include "ezconfig.h"
#include "mcuconfig.h"
#include "ez_ota_def.h"
#include "ez_iot_errno.h"
#include "ez_iot_log.h"
#include "ez_iot_ota.h"
#include "ez_ota.h"
#include "ez_ota_extern.h"
#include "ez_ota_user.h"

static int g_ota_inited = 0;

#ifdef __cplusplus
extern "C"
{
#endif

EZIOT_API ez_err_e ez_iot_ota_init(ota_init_t *pota_init)
{
    ez_err_e suc = ez_errno_succ;
    ez_log_d(TAG_OTA, "ez_ota_init enter");
	if (!pota_init)
    {
		return ez_errno_ota_param_invalid;
	}
	if (!pota_init->cb.ota_recv_msg)
    {
		ez_log_e(TAG_OTA, "ota_init, cb recv_msg err");
		return ez_errno_ota_param_invalid;
	}
    ez_ota_user_init(pota_init->cb);
    suc = ez_ota_extern_init();
    if( ez_errno_succ != suc)
    {
        ez_log_d(TAG_OTA, "ota_extern_init err 0x%x",suc);
        return suc;
    }  
    g_ota_inited = 1;
     ez_log_d(TAG_OTA, "ez_ota_init success");

    return suc;
}

EZIOT_API ez_err_e ez_iot_ota_modules_report(const ota_res_t *pres, const ota_modules_t* pmodules, uint32_t timeout_ms)
{
    if(0 == g_ota_inited)
    {
        return ez_errno_ota_not_init;
    }
    if( NULL == pmodules)
    {
         ez_log_e(TAG_OTA,"ez_ota_report_module_info input err");
        return ez_errno_ota_param_invalid;
    }

    return  ezdev_ota_module_info_report(pres, pmodules, timeout_ms);
}

EZIOT_API ez_err_e ez_iot_ota_status_ready(const ota_res_t *pres, int8_t* mod_name)
{
     if (!g_ota_inited)
    {
        return ez_errno_ota_not_init;
    }
    return ez_progress_report(pres, mod_name, NULL, ota_code_none, ota_state_ready, 0);
}

EZIOT_API ez_err_e ez_iot_ota_status_succ(const ota_res_t *pres, int8_t* mod_name)
{
     if (!g_ota_inited)
    {
        return ez_errno_ota_not_init;
    }
    return ez_progress_report(pres, mod_name, NULL, ota_code_none, ota_state_succ, 0);
}

EZIOT_API ez_err_e ez_iot_ota_status_fail(const ota_res_t *pres, int8_t* mod_name, int8_t* perr_msg, ota_errcode_e code)
{
     if (!g_ota_inited)
    {
        return ez_errno_ota_not_init;
    }
    return ez_progress_report(pres, mod_name, perr_msg, code, ota_state_failed, 0);
}

EZIOT_API ez_err_e ez_iot_ota_progress_report(const ota_res_t *pres, int8_t* mod_name, ota_status_e status, int16_t progress)
{
    if (!g_ota_inited)
    {
        return ez_errno_ota_not_init;
    }
    if(progress < 0||progress > 100)
    {
        return ez_errno_ota_param_invalid;
    }
  
    return ez_progress_report(pres, mod_name, NULL, ota_code_none, status, progress);
}

/* 
	input_info:下载的地址相关信息
	file_cb：应用层要实现的写flash 操作 & 或者写串口部分 
	notify：各阶段完成时应用层的处理函数
	user_data：用户上下文，可用于信息透传，传到底层在透出来，也可以帮助底层信息校验，

*/
EZIOT_API ez_err_e ez_iot_ota_download(ota_download_info_t *input_info, get_file_cb file_cb, notify_cb notify, void* user_data)
{
    if (!g_ota_inited)
    {
        return ez_errno_ota_not_init;
    }
    if(NULL== input_info || 0 == strlen((char*)input_info->url)|| input_info->block_size <= 0\
     || input_info->total_size <= 0 ||NULL == file_cb||NULL == notify)
    {
         ez_log_e(TAG_OTA,"ota_file_download input param err");
        return ez_errno_ota_param_invalid;
    }

    return ez_ota_file_download(input_info, file_cb, notify, user_data);
}

EZIOT_API ez_err_e ez_iot_ota_deinit()
{
    if (!g_ota_inited)
    {
        return ez_errno_ota_not_init;
    }
    g_ota_inited = 0;
    ez_ota_extern_fini();
    return ez_errno_succ;
}

#ifdef __cplusplus
}
#endif

