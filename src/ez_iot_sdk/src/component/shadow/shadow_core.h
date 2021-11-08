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
* 2020-08-17     xurongjun
*******************************************************************************/


#ifndef _SHADOW_CORE_H_
#define _SHADOW_CORE_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        shadow_event_type_reset,   ///< 切换平台
        shadow_event_type_add,     ///< 增加模块
        shadow_event_type_online,  ///< 设备上线/断线重连
        shadow_event_type_offline, ///< 设备掉线
        shadow_event_type_report,  ///< 状态上报
        shadow_event_type_recv,    ///< 收到数据
    } shadow_event_type_e;

    /**
     * @brief 驱动shadow模块开始运行
     * 
     * @return true 
     * @return false 
     */
    bool shadow_core_start();

    /**
     * @brief shadow模块停止运行
     * 
     */
    void shadow_core_stop();

    /**
     * @brief 本地环境相关事件（上下线、模块增删、主动上报）发生，信号通知触发至内部做相应动作。
     * 
     * @param event_type 事件类型
     */
    void shadow_core_event_occured(shadow_event_type_e event_type);

    /**
     * @brief v2协议模块注册接口
     * 
     * @param domain_id 领域id
     * @param props_num 属性数量
     * @param props 属性列表
     * @return int 返回handle，-1表示失败
     */
    int shadow_core_module_addv2(int32_t domain_id, uint16_t props_num, void *props);

    /**
     * @brief v3协议模块注册接口
     * 
     * @param sn 主设备/子设备序列号
     * @param res_type 通道类型（某种类型通道的集合，视频通道集合或者报警通道集合）
     * @param index 通道号（例如报警通道0或者报警通道1）
     * @param domain_id 领域id
     * @param props_num 属性数量
     * @param props 属性列表
     * @return int 返回handle，-1表示失败 
     */
    int shadow_core_module_addv3(int8_t *sn, int8_t *res_type, int16_t index, int8_t *domain_id, uint16_t props_num, void *props);

    /**
     * @brief 删除领域模块
     * 
     * @param handle 领域对应句柄
     * @return int 
     */
    int shadow_core_module_delete(int8_t *sn, int8_t *res_type, int16_t index, int8_t *domain_id);

    /**
     * @brief key值发生更变，触发上报。
     * 
     * @param handle 领域的操作句柄
     * @param pcKey 需要上报的key值
     * @return int 
     */
    int shadow_core_propertie_changed(int8_t *sn, int8_t *res_type, int16_t index, int8_t *domain_id, int8_t *pkey, void *pvalue);

    /**
     * @brief 云端下发命令入口
     * 
     * @param shadow_res 设备资源
     * @param seq 业务时序值
     * @param payload 消息报文体
     * @return int 
     */
    int shadow_core_cloud_data_in(void *shadow_res, uint32_t seq, int8_t *business_type, int8_t *payload);

#ifdef __cplusplus
}
#endif

#endif
