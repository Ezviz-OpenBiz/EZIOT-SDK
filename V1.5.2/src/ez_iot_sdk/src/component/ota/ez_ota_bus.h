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
#ifndef _EZ_OTA_BUS_H_
#define _EZ_OTA_BUS_H_

#include <stdio.h>

#include "ez_iot_errno.h"
#include "ez_iot_ota.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define EZ_OTA_RSP 1
#define EZ_OTA_REQ 0

ez_err_e  ez_ota_send_msg_to_platform(unsigned char* msg, int msg_len, const ota_res_t *pres, const char*  msg_type,\
                                      const char* method, int response, unsigned int* msg_seq, int msg_qos);
#ifdef __cplusplus
}
#endif
#endif