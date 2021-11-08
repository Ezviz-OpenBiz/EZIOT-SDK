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
#ifndef _EZ_IOT_TSL_CONN_H
#define _EZ_IOT_TSL_CONN_H

#include <stdint.h>
#include "connectivity_parse.h"

typedef struct
{
    char dev_sn[48];
    char domain_identifier[32];
    char identifier[32];
    union
    {
        double data_num;
        int8_t data_str[128];
    } data;
} conn_dev_info;

void ez_iot_tsl_conn_init();

void ez_iot_tsl_conn_finit();

int32_t ez_iot_tsl_add_conn(conn_info_t *conn_info);

int32_t ez_iot_tsl_del_all_conn();

int32_t ez_iot_tsl_conn_info_process(const int8_t *key, const int8_t *value);

int32_t ez_iot_tsl_check_conn_do(conn_dev_info *dev_info);

#endif