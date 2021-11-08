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
#include "ez_iot_def.h"
#include "profile_parse.h"
#include "ez_iot_tsl.h"

#ifdef EZ_IOT_SDK
#include "ez_iot_shadow.h"
#else
#include "ezDevSDK_shadow.h"
#define ez_iot_shadow_res_t ezDevSDK_Shadow_Res_t
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        ez_iot_shadow_res_t shadow_res;
        uint8_t domain[32];
    } shadow_report_key_t;

    /**
     * @brief 初始化描述文件适配器
     * 
     * @param data_cbs 描述文件读写接口
     * @param things_cbs 物模型处理回调接口
     */
    int32_t ez_iot_tsl_adapter_init(tsl_things_callbacks_t *things_cbs);

    /**
     * @brief 增加一个设备
     * 
     * @param dev_info 设备信息
     * @return int32_t 0表示成功、-1表示失败
     */
    int32_t ez_iot_tsl_adapter_add(tsl_devinfo_t *dev_info);

    /**
     * @brief 删除一个设备
     * 
     * @param dev_info 设备信息
     * @return int32_t 0表示成功、-1表示失败
     */
    int32_t ez_iot_tsl_adapter_del(tsl_devinfo_t *pevinfo);

    /**
     * @brief 删除功能描述文件
     * 
     * @param dev_subserial 设备序列号
     * @return true 成功
     * @return false 找不到对应功能描述文件或文件损坏
     */
    bool ez_iot_tsl_adapter_profile_del(const int8_t * dev_subserial);

    /**
     * @brief 功能描述文件下载链接
     * 
     * @param url 功能描述文件下载链接
     * @param expire 超时时间
     * @param md5 摘要
     * @param dev_sn 序列号
     */
    void ez_iot_tsl_adapter_profile_url(char *dev_sn, char *url, char *md5, int expire);

    /**
     * @brief 反初始化
     * 
     */
    void ez_iot_tsl_adapter_deinit();

    /**
     * @brief 校验属性功能点数据是否合法
     * 
     * @param sn 序列号
     * @param rsc_info 通道信息
     * @param key_info 功能点
     * @param value 功能点数据
     * @return ez_errno_tsl_value_type 
     * @return ez_errno_tsl_value_illegal 
     */
    int32_t ez_iot_tsl_property_value_legal(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, const tsl_value_t *value);

    /**
     * @brief 校验事件功能点数据是否合法
     * 
     * @param sn 序列号
     * @param rsc_info 通道信息
     * @param key_info 功能点
     * @param value 功能点数据
     * @return ez_errno_tsl_value_type 
     * @return ez_errno_tsl_value_illegal 
     */
    int32_t ez_iot_tsl_event_value_legal(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, const tsl_value_t *value);

    /**
     * @brief 校验操作功能点是否合法
     * 
     * @param sn 序列号
     * @param rsc_info 通道信息
     * @param key_info 功能点
     * @return ez_errno_tsl_dev_not_found 
     * @return ez_errno_tsl_rsctype_not_found 
     * @return ez_errno_tsl_index_not_found 
     * @return ez_errno_tsl_domain_not_found 
     * @return ez_errno_tsl_key_not_found 
     */
    int32_t ez_iot_tsl_action_value_legal(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, const tsl_value_t *value);

    /**
     * @brief 通过domain和key反查属性的res_type/rsc_category
     * 
     * @param sn 序列号
     * @param key_info 功能点
     * @param res_type 资源类型缓冲区
     * @param len 资源类型缓冲区长度
     * @return int32_t 
     * @return ez_errno_succ 
     * @return ez_errno_tsl_rsctype_not_found 
     * @return ez_errno_tsl_profile_loading 
     * @return ez_errno_tsl_dev_not_found 
     * @return ez_errno_tsl_key_not_found 
     */
    int32_t tsl_find_property_rsc_by_keyinfo(const int8_t *sn, const tsl_key_info_t *key_info, int8_t *res_type, int32_t len);

    /**
     * @brief 通过domain和key反查事件的res_type/rsc_category
     * 
     * @param sn 序列号
     * @param key_info 功能点
     * @param res_type 资源类型缓冲区
     * @param len 资源类型缓冲区长度
     * @return int32_t 
     * @return ez_errno_succ 
     * @return ez_errno_tsl_rsctype_not_found 
     * @return ez_errno_tsl_profile_loading 
     * @return ez_errno_tsl_dev_not_found 
     * @return ez_errno_tsl_key_not_found 
     */
    int32_t tsl_find_event_rsc_by_keyinfo(const int8_t *sn, const tsl_key_info_t *key_info, int8_t *res_type, int32_t len);

#ifdef __cplusplus
}
#endif