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
 * 2020-08-13    xurongjun     
 *******************************************************************************/
#ifndef _HUB_FUNC_H_
#define _HUB_FUNC_H_

#include "ez_iot_hub.h"

typedef struct
{
    int8_t authm;         ///< 认证模式：0-SAP设备, 1-licence设备
    int8_t type[32 + 1];  ///< 子设备型号(licence设备为productKey)
    int8_t sn[16 + 1];    ///< 子设备序列号(licence设备为deviceName)
    int8_t vcode[32 + 1]; ///< 子设备验证码(对应licence认证中deviceLicense)
    int8_t ver[32 + 1];   ///< 子设备固件版本号
    int8_t uuid[16 + 1];  ///< 子设备局域网唯一标识
    int8_t sta;           ///< 在线状态
    int8_t access;        ///< 认证状态：0-未认证，1-已认证
} hub_subdev_info_internal_t;

/**
 * @brief 初始化Hub子设备管理模块
 * 
 * @param phub_cbs 子设备添加回调
 * @return int ez_errno_succ、ez_errno_hub_internal
 */
int hub_func_init(const hub_callbacks_t *phub_cbs);

/**
 * @brief 反初始化Hub子设备管理模块
 * 
 */
void hub_func_deinit();

/**
 * @brief 新增一个子设备
 * 
 * @param subdev_info 子设备信息
 * @return ez_err_e ez_errno_succ、ez_errno_hub_storage、ez_errno_hub_memory、ez_errno_hub_subdev_existed、ez_errno_hub_out_of_range
 */
ez_err_e hub_add_do(const hub_subdev_info_internal_t *subdev_info);

/**
 * @brief 删除一个子设备
 * 
 * @param subdev_sn 子设备序列号
 * @return ez_err_e ez_errno_succ、ez_errno_hub_storage、ez_errno_hub_memory、ez_errno_hub_subdev_not_found
 */
ez_err_e hub_del_do(const int8_t *subdev_sn);

/**
 * @brief 更新子设备版本号
 * 
 * @param subdev_sn 子设备序列号
 * @param subdev_ver 新的版本号
 * @return ez_err_e ez_errno_succ、ez_errno_hub_storage、ez_errno_hub_memory、ez_errno_hub_subdev_not_found
 */
ez_err_e hub_ver_update_do(const int8_t *subdev_sn, const int8_t *subdev_ver);

/**
 * @brief 更新子设备状态
 * 
 * @param subdev_sn 子设备序列号
 * @param online 子设备在线状态
 * @return ez_err_e ez_errno_succ、ez_errno_hub_storage、ez_errno_hub_memory、ez_errno_hub_subdev_not_found
 */
ez_err_e hub_status_update_do(const int8_t *subdev_sn, bool online);

/**
 * @brief 查询子设备信息
 * 
 * @param subdev_sn 子设备序列号
 * @param subdev_info 子设备信息
 * @return ez_err_e ez_errno_succ、ez_errno_hub_storage、ez_errno_hub_memory、ez_errno_hub_subdev_not_found
 */
ez_err_e hub_subdev_query(const int8_t *subdev_sn, hub_subdev_info_internal_t *subdev_info);

/**
 * @brief 枚举子设备
 * 
 * @param subdev_info 子设备信息
 * @return ez_err_e ez_errno_succ、ez_errno_hub_storage、ez_errno_hub_memory、ez_errno_hub_subdev_not_found、ez_errno_hub_enum_end
 */
ez_err_e hub_subdev_next(hub_subdev_info_internal_t *subdev_info);

/**
 * @brief 清空子设备
 * 
 * @return ez_err_e ez_errno_succ、ez_errno_hub_storage
 */
ez_err_e hub_clean_do(void);

/**
 * @brief 上报子设备关联关系
 * 
 * @return int 
 */
int hub_subdev_list_report(void);

/**
 * @brief 更新子设备在线状态
 * 
 * @return int 
 */
int hub_subdev_sta_report(void);

/**
 * @brief 认证子设备
 * 
 * @param subdev_info 需要认证的子设备信息
 * @return int 信令发送结果
 */
int hub_subdev_auth_do(void *subdev_info);

/**
 * @brief 子设备认证结果
 * 
 * @param buf 协议报文
 * @param len 协议报文长度
 */
void hub_subdev_auth_done(void *buf, int len);

/**
 * @brief 注册物模型，模块初始化时调用
 * 
 * @return ez_err_e 
 */
ez_err_e hub_tsl_reg_all(void);

/**
 * @brief 注销物模型，模块反初始化时调用
 * 
 * @return ez_err_e 
 */
ez_err_e hub_tsl_unreg_all(void);

#endif