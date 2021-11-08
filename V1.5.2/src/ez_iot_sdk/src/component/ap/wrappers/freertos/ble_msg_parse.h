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
#ifndef _EZVIZ_BLE_MSG_PARSE_H_
#define _EZVIZ_BLE_MSG_PARSE_H_


#include <stdint.h>
#include "ez_iot_ap.h"

int process_ble_request(const uint8_t *req, uint32_t req_len, uint8_t *rsp);
int set_wifi_info_cb(wifi_info_cb cb);
int get_wifi_config_status();
ez_iot_ap_wifi_info_t *get_wifi_config();

#endif /*  */