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
#include <stdio.h>
#include <string.h>
#include "ez_iot_ota.h"
#include "ezconfig.h"
#include "mcuconfig.h"


static ota_msg_cb_t g_ota_cb;

void  ez_ota_user_init(ota_msg_cb_t cb)
{
    g_ota_cb = cb;
}

ota_msg_cb_t *ez_ota_get_callback()
{
    return &g_ota_cb;
}
