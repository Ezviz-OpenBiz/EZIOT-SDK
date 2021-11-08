/**
 * @file ez_iot_ctx.h
 * @author xurongjun (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2019-11-11
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef _EZ_IOT_CTX_H_
#define _EZ_IOT_CTX_H_

#include <ez_iot.h>
#include "ez_protocol.h"

typedef enum
{
    status_uninit,
    status_init,
    status_ready,
    status_online,
    status_disconnect,
    status_offline,
} dev_status_t;

typedef struct
{
    dev_status_t status;
    int8_t need_query_bingding;
    int8_t token[128];
    int8_t user_id[33];
    ez_iot_callbacks_t cbs;
    ez_iotsdk_upgrade_file_t upgrade_info;
} ez_iot_ctx_t;

void ez_iot_set_callbacks(const ez_iot_callbacks_t *pcbs);

ez_iot_callbacks_t *ez_iot_get_callbacks(void);

void ez_iot_set_status(dev_status_t stauts);

bool ez_iot_is_inited();

bool ez_iot_is_ready();

bool ez_iot_is_online();

void ez_iot_ota_clear_info(void);

void ez_iot_ota_set_info(ez_iotsdk_upgrade_file_t *file_info);

ez_iotsdk_upgrade_file_t *ez_iot_ota_get_info();

void ez_iot_clear_token(void);

void ez_iot_set_token(int8_t *token);

int8_t *ez_iot_get_token();

void ez_iot_set_binding_id(int8_t *user_id);

/* 清除绑定标记 */
void ez_iot_clear_binding_id();

/* 判断设备是否被绑定 */
int8_t ez_iot_is_binding();

/* 设置需要查询绑定关系 */
void ez_iot_set_need_query_binding();

/* 判断是否需要查询绑定关系 */
int8_t ez_iot_is_need_query_binding();

/* 清除是否需要查询绑定关系标记 */
void ez_iot_clear_query_binding();

#endif