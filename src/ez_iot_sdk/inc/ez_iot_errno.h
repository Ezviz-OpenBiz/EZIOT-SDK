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
 * 2019-11-07     XuRongjun    first version
 *******************************************************************************/

#ifndef _EZ_IOT_ERRNO_H_
#define _EZ_IOT_ERRNO_H_

typedef enum ez_err
{
    ez_errno_fail                   = -1,
    ez_errno_succ                   = 0,                        ///< success
    ez_errno_not_init               = ez_errno_succ + 0x01,     ///< The sdk core module is not initialized
    ez_errno_not_ready              = ez_errno_succ + 0x02,     ///< The sdk core module is not started
    ez_errno_param_invalid          = ez_errno_succ + 0x03,     ///< The input parameters is illegal, it may be that some parameters can not be null or out of range
    ez_errno_internal               = ez_errno_succ + 0x04,     ///< Unknown error
    ez_errno_memory                 = ez_errno_succ + 0x05,     ///< Out of memory
    ez_errno_inited                 = ez_errno_succ + 0x06,     ///< The sdk core module has been initialized
    ez_errno_kv                     = ez_errno_succ + 0x07,     ///< The kv interface error occured

    ez_errno_ap_base                = 0x00010000,
    ez_errno_ap_app_connected       = ez_errno_ap_base + 0x02,  ///< app connected
    ez_errno_ap_connecting_route    = ez_errno_ap_base + 0x03,  ///< connecting
    ez_errno_ap_connect_failed      = ez_errno_ap_base + 0x04,  ///< connect failed 
    ez_errno_ap_wifi_config_timeout = ez_errno_ap_base + 0x05,  ///< connect timeout

    ez_errno_ota_base               = 0x00020000,               ///< Tsl interface error code base
    ez_errno_ota_not_init           = ez_errno_ota_base + 0x01, ///< The ota module is not initialized
    ez_errno_ota_not_ready          = ez_errno_ota_base + 0x02, ///< The sdk core module is not started
    ez_errno_ota_param_invalid      = ez_errno_ota_base + 0x03, ///< The input parameters is illegal, it may be that some parameters can not be null or out of range
    ez_errno_ota_internal           = ez_errno_ota_base + 0x04, ///< Unknown error
    ez_errno_ota_memory             = ez_errno_ota_base + 0x05, ///< Out of memory
    ez_errno_ota_register_failed    = ez_errno_ota_base + 0x06, ///< register_failed 
    ez_errno_ota_json_creat_err     = ez_errno_ota_base + 0x07, ///< json_creat_err
    ez_errno_ota_json_format_err    = ez_errno_ota_base + 0x08, ///< json_format_err 
    ez_errno_ota_msg_send_err       = ez_errno_ota_base + 0x09, ///< msg_send_err
    ez_errno_ota_time_out           = ez_errno_ota_base + 0xA,  ///< time_out
    ez_errno_ota_device_offline     = ez_errno_ota_base + 0xB,  ///< device_offline 
    ez_errno_ota_download_already   = ez_errno_ota_base + 0xC,  ///< download already 

    ez_errno_hub_base               = 0x00030000,               ///< hub interface error code base
    ez_errno_hub_not_init           = ez_errno_hub_base + 0x01, ///< The hub module or sdk core module is not initialized
    ez_errno_hub_param_invalid      = ez_errno_hub_base + 0x02, ///< The input parameters is illegal, it may be that some parameters can not be null or out of range
    ez_errno_hub_internal           = ez_errno_hub_base + 0x03, ///< Unknown error
    ez_errno_hub_storage            = ez_errno_hub_base + 0x04, ///< flash operation has failed.
    ez_errno_hub_subdev_not_found   = ez_errno_hub_base + 0x05, ///< Can not found subdev
    ez_errno_hub_enum_end           = ez_errno_hub_base + 0x06, ///< End of enumeration, no more data
    ez_errno_hub_memory             = ez_errno_hub_base + 0x07, ///< Out of memory
    ez_errno_hub_out_of_range       = ez_errno_hub_base + 0x08, ///< Sub count out of range
    ez_errno_hub_subdev_existed     = ez_errno_hub_base + 0x09, ///< The sub device already exists

    ez_errno_tsl_base               = 0x00040000,               ///< Tsl interface error code base
    ez_errno_tsl_not_init           = ez_errno_tsl_base + 0x01, ///< The tsl module is not initialized
    ez_errno_tsl_not_ready          = ez_errno_tsl_base + 0x02, ///< The sdk core module is not started
    ez_errno_tsl_param_invalid      = ez_errno_tsl_base + 0x03, ///< The input parameters is illegal, it may be that some parameters can not be null or out of range
    ez_errno_tsl_internal           = ez_errno_tsl_base + 0x04, ///< Unknown error
    ez_errno_tsl_memory             = ez_errno_tsl_base + 0x05, ///< Out of memory
    ez_errno_tsl_dev_not_found      = ez_errno_tsl_base + 0x06, ///< Can't find the device, need to register first
    ez_errno_tsl_rsctype_not_found  = ez_errno_tsl_base + 0x07, ///< The rsc_type is illegal, is not defined in the profile
    ez_errno_tsl_index_not_found    = ez_errno_tsl_base + 0x08, ///< The local index is illegal, is not defined in the profile
    ez_errno_tsl_domain_not_found   = ez_errno_tsl_base + 0x09, ///< The domain is illegal, is not defined in the profile
    ez_errno_tsl_key_not_found      = ez_errno_tsl_base + 0x0a, ///< The Key is illegal, is not defined in the profile
    ez_errno_tsl_value_type         = ez_errno_tsl_base + 0x0b, ///< The type of the value does not match the definition
    ez_errno_tsl_value_illegal      = ez_errno_tsl_base + 0x0c, ///< The value out of the defined range
    ez_errno_tsl_profile_loading    = ez_errno_tsl_base + 0x0d, ///< The device profile is loading

    ez_errno_httpd_base             = 0x00050000,                 ///< Starting number of HTTPD error codes  
    ez_errno_httpd_handlers_full    = ez_errno_httpd_base + 0x01, ///< All slots for registering URI handlers have been consumed
    ez_errno_httpd_handler_exists   = ez_errno_httpd_base + 0x02, ///< URI handler with same method and target URI already registered
    ez_errno_httpd_invalid_req      = ez_errno_httpd_base + 0x03, ///< Invalid request pointer
    ez_errno_httpd_result_trunc     = ez_errno_httpd_base + 0x04, ///< Result string truncated
    ez_errno_httpd_resp_hdr         = ez_errno_httpd_base + 0x05, ///< Response header field larger than supported
    ez_errno_httpd_resp_send        = ez_errno_httpd_base + 0x06, ///< Error occured while sending response packet
    ez_errno_httpd_alloc_mem        = ez_errno_httpd_base + 0x07, ///< Failed to dynamically allocate memory for resource
    ez_errno_httpd_task             = ez_errno_httpd_base + 0x08, ///< Failed to launch server task/thread
    ez_errno_httpd_not_found        = ez_errno_httpd_base + 0x08, ///< Failed to launch server task/thread

} ez_err_e;

#endif