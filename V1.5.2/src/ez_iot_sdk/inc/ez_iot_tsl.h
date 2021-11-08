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
* 2019-11-07     XuRongjun    first version
*******************************************************************************/


#ifndef _EZ_IOT_TSL_ENGINE_H_
#define _EZ_IOT_TSL_ENGINE_H_

#include <stdint.h>
#include <stdbool.h>
#include "ez_iot_errno.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum
	 {
		 ez_iot_tsl_property, ///< 属性,↓
		 ez_iot_tsl_action,   ///< 操作,↓
		 ez_iot_tsl_event,	 ///< 事件,
	 } tsl_type_e;


    typedef enum
    {
        tsl_data_type_bool,
        tsl_data_type_int,
        tsl_data_type_double,
        tsl_data_type_string,
        tsl_data_type_array,
        tsl_data_type_object,
        tsl_data_type_null,

        tsl_data_type_max,
    } tsl_data_type_e;

    typedef struct
    {
        int8_t *domain; ///< 功能点所属于的领域。如key=switch，其在视频领域表示关闭摄像头，照明领域表示关闭灯光。
        int8_t *key;    ///< 功能点键值
    } tsl_key_info_t;

    typedef struct
    {
        tsl_data_type_e type;
        int size;
        union
        {
            bool value_bool;
            int value_int;
            double value_double;
            void *value; /* 复杂类型的数据 */
        };
    } tsl_value_t;

    typedef struct
    {
        int8_t *key;
        tsl_value_t value;
    } tsl_param_t;

    typedef struct
    {
        int8_t *dev_subserial;       ///< 序列号, 必填, max 72字节
        int8_t *dev_type;            ///< 型号, 必填, max 64字节
        int8_t *dev_firmwareversion; ///< 版本号, 必填, max 64字节
    } tsl_devinfo_t;

    typedef struct
    {
        int8_t *res_type;    ///< 通道类型（按键通道类型或者报警通道类型）
        int8_t *local_index; ///< 通道号
    } tsl_rsc_info_t;

    typedef struct
    {
        /**
        * @brief 平台下发操作
        * 
        * @param sn 序列号
        * @param rsc_info 通道信息
        * @param key_info 操作功能点信息
        * @param value_in 操作入参
        * @param value_out 操作出参，所申请内存由内部释放
        * @return int32_t 0表示成功，-1表示失败
        */
        int32_t (*action2dev)(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info,
                              const tsl_value_t *value_in, tsl_value_t *value_out);

        /**
        * @brief 获取属性向平台上报
        * 
        * @param sn 序列号
        * @param rsc_info 通道信息
        * @param key_info 操作功能点信息
        * @param value_out 属性数据
        * @return int32_t 0表示成功，-1表示失败
        */
        int32_t (*property2cloud)(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, tsl_value_t *value_out);

        /**
        * @brief 平台属性下发
        * 
        * @param sn 序列号
        * @param rsc_info 通道信息
        * @param key_info 操作功能点信息
        * @param value 属性数据
        * @return int32_t 0表示成功，-1表示失败
        */
        int32_t (*property2dev)(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, const tsl_value_t *value);

    } tsl_things_callbacks_t;

    /**
    * @brief 模块初始化
    * 
    * @param pdata_cbs 
    * @return ez_err_e 
    */
    ez_err_e ez_iot_tsl_init(tsl_things_callbacks_t *ptsl_cbs);

    /**
    * @brief 向平台上报一条属性
    * 
    * @param sn 设备序列号
    * @param rsc_info 通道信息
    * @param key_info 操作功能点信息
    * @param value 属性数据
    * @return ez_err_e 
    */
    ez_err_e ez_iot_tsl_property_report(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, const tsl_value_t *value);

    /**
    * @brief 向平台上报一个事件
    * 
    * @param sn 设备序列号
    * @param rsc_info 通道信息
    * @param key_info 功能点信息
    * @param value 事件数据，一般为根据物模型组好的json报文
    * @return ez_err_e 
    */
    ez_err_e ez_iot_tsl_event_report(const int8_t *sn, const tsl_rsc_info_t *rsc_info, const tsl_key_info_t *key_info, const tsl_value_t *value);

    /**
    * @brief tsl反初始化
    * 
    * @return ez_err_e 
    */
    ez_err_e ez_iot_tsl_deinit(void);



	ez_err_e ez_iot_tsl_check_value_legal(const char *key, int type, const tsl_devinfo_t *dev_info, tsl_value_t *value);

#ifdef __cplusplus
}
#endif

#endif //_EZ_IOT_TSL_ENGINE_H_
