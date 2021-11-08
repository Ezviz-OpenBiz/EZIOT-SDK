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
* 2019-11-13     xurongjun
*******************************************************************************/
#ifndef _EZ_IOT_SHADOW_H_
#define _EZ_IOT_SHADOW_H_

#include <stdint.h>
#include <stdbool.h>
#include "ez_iot_errno.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        ez_iot_shadow_business_type_status = 0,  ///< 影子状态
        ez_iot_shadow_business_type_offline_cmd, ///< 离线指令
        ez_iot_shadow_business_type_max,
    } ez_iot_shadow_business_type_t;

    typedef struct
    {
        const void *pres;      ///< 目标资源(shadow V3)，状态给到哪个设备的哪个通道（无子设备和单通道忽略）。参见ezDevSDK_Shadow_Res_t
        const int8_t *pdomain; ///< 领域id(shadow V3)
        const int8_t *pkey;    ///< 属性id(shadow V3)
    } ez_iot_shadow_business2dev_param_t;

    typedef struct
    {
        uint32_t ver;          ///< 版本号，用于shadow的同步
        const void *pres;      ///< 目标资源(shadow V3)，从哪个设备的哪个通道获取状态（无子设备和单通道忽略）。参见ezDevSDK_Shadow_Res_t
        const int8_t *pdomain; ///< 领域id(shadow V3)
        const int8_t *pkey;    ///< 属性id(shadow V3)
    } ez_iot_shadow_business2cloud_param_t;

    typedef struct
    {
        enum
        {
            shd_data_type_bool,
            shd_data_type_int,
            shd_data_type_double,
            shd_data_type_string,
            shd_data_type_array,
            shd_data_type_object
        } type;

        int32_t length; ///< 状态值长度
        union
        {
            bool value_bool;
            int value_int;
            double value_double;
            void *value; /* 复杂类型的数据 */
        };
    } ez_iot_shadow_value_t;

    typedef struct
    {
        int8_t key[64];     ///< shadow业务的key
        int16_t type;       ///< shadow业务的类型, 参见 ez_iot_shadow_business_type_t
        uint32_t ver;       ///< shadow业务的版本号
        uint32_t seq;       ///< 状态发送后的seq
        int8_t need_report; ///< 是否需要上报标志

        /**
        * @brief 平台的shadow业务同步给设备
        * @param[in] pjson_value 业务报文
        * @param[in] ppram 保留入参
        * @return int32_t 成功0，失败-1 
        */
        int32_t (*business2dev)(const ez_iot_shadow_value_t *pvalue, ez_iot_shadow_business2dev_param_t *ppram);

        /**
        * @brief 设备的shadow业务同步给平台
        * @param[out] ppjson_value 组织上报的的业务报文
        * @param[in] ppram 组织报文的入参
        * @return int32_t 成功0，失败-1 
        */
        int32_t (*business2cloud)(ez_iot_shadow_value_t *pvalue, ez_iot_shadow_business2cloud_param_t *ppram);

        /* 保留字段 */
        int8_t *reserved;
    } ez_iot_shadow_business_t;

    typedef struct
    {
        int8_t reset;                       ///< 设备重置，清空所有缓存状态
        int16_t num;                        ///< 注册业务的数量
        ez_iot_shadow_business_t *business; ///< 注册的业务
    } ez_iot_shadow_module_t;

    /**
    * @brief 描述某个资源。设备的某种资源类型集合的某个通道
    * 
    */
    typedef struct
    {
        int8_t dev_serial[72]; ///< 设备序列号
        int8_t res_type[64];   ///< 通道类型（某种类型通道的集合，视频通道集合或者报警通道集合）
        int16_t local_index;   ///< 通道号（例如报警通道0或者报警通道1）
    } ez_iot_shadow_res_t;

    /**
    * @brief 初始化shadow模块
    * 
    * @return ez_err_e 
    */
    ez_err_e ez_iot_shadow_init(void);

    /**
    * @brief 注册上报的业务
    * 
    * @param module[in] 注册业务模块
    * @param handle[out] 分配的句柄
    * @return ez_err_e 
    */
    ez_err_e ez_iot_shadow_register(ez_iot_shadow_module_t *module, uint8_t *handle);

    /**
    * @brief 主动触发业务上报
    * 
    * @param handle 已注册模块的句柄
    * @param key 上报业务的key 
    * @return ez_err_e 
    */
    ez_err_e ez_iot_shadow_push(uint8_t handle, int8_t *key);

    /**
    * @brief 向shadow反注册一个领域模块
    * 
    * @param handle 领域的操作句柄
    * @return int 
    */
    ez_err_e ez_iot_shadow_unregister(int32_t handle);

    /**
    * @brief 向shadow注册一个领域模块
    * 
    * @param pres 资源（设备/子设备/通道）
    * @param domain_id 领域以及对应的key
    * @param module 领域key列表
    * @return ez_err_e 
    */
    ez_err_e ez_iot_shadow_register_v3(ez_iot_shadow_res_t *pres, int8_t *domain_id, ez_iot_shadow_module_t *module);

    /**
    * @brief key值发生更变，触发上报
    * 
    * @param pres 资源（设备/子设备/通道）
    * @param domain_id 领域id
    * @param pkey 变更的key值
    * @param pvalue 变更的value
    * @return ez_err_e 
    */
    ez_err_e ez_iot_shadow_push_v3(ez_iot_shadow_res_t *pres, int8_t *domain_id, int8_t *pkey, ez_iot_shadow_value_t *pvalue);

    /**
    * @brief 反初始化shadow模块
    * 
    * @return ez_err_e 
    */
    void ez_iot_shadow_fini(void);

#ifdef __cplusplus
}
#endif

#endif