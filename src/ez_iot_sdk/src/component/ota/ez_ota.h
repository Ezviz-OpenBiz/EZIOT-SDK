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

#ifndef _EZ_OTA_H_
#define _EZ_OTA_H_

#include "ez_iot_ota.h"

#ifdef __cplusplus
extern "C"
{
#endif

ez_err_e ez_progress_report(const ota_res_t *pres, const int8_t* pmodule, const int8_t*perr_msg, ota_errcode_e errcode, int8_t status, int16_t progress);

ez_err_e  ez_ota_file_download(ota_download_info_t *input_info, get_file_cb file_cb, notify_cb notify, void* user_data);


#ifdef __cplusplus
}
#endif


#endif

