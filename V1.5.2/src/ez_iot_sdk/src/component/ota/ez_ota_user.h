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

#ifndef H_EZ_OTA_USER_H_
#define H_EZ_OTA_USER_H_

#include "ez_iot_ota.h"

#ifdef  __cplusplus
extern "C" {
#endif

void  ez_ota_user_init(ota_msg_cb_t cb);

ota_msg_cb_t *ez_ota_get_callback();

#ifdef __cplusplus
}
#endif

#endif
