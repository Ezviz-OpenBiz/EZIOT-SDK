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

#ifndef _EZ_IOT_WIFI_AP_H
#define _EZ_IOT_WIFI_AP_H

#include <stdbool.h>
#include "ez_iot_errno.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MIN_TIME_OUT 1      // 配网最小窗口期，单位min
#define MAX_TIME_OUT 60     // 配网最大窗口期，单位min
#define DEFAULT_TIME_OUT 15 // 默认配网窗口期，15min

    // 设备信息，用于ap初始化
    typedef struct
    {
        char ap_ssid[32 + 1];       // ap ssid
        char ap_password[64 + 1];   // ap password, 不需要密码则传空
        char auth_mode;             // ap auth mode, 0 for no ap_password 

        char dev_serial[72];        // 设备序列号
        char dev_type[64];          // 设备类型
        char dev_version[64];       // 设备版本号
        char res;
    } ez_iot_ap_dev_info_t;
    
    // 配网过程获取到的信息
    typedef struct
    {
        char ssid[32+1];        // 设备连接的wifi ssid
        char password[64+1];    // 设备连接的wifi password
        
        char cc[4];             // 国家码cc
        char res[2];
        char token[128];        // app端校验的token
    
        char domain[128];       // 设备注册平台地址;
        char device_id[32];     // 设备uuid，可选配置
        int err_code;           // wifi配置错误码，见ez_iot_errno.h 中 ez_errno_ap_XXXX错误码定义
    } ez_iot_ap_wifi_info_t;

    /**
     *  @brief  给上层wifi信息的回调函数
     *  @param  wifi_info: 给应用层的数据
     *  @info   回调中先判断err_code，若err_code为ez_errno_succ，则可以从参数wifi_info中拿到ssid和密码
     *          否则，均为配网失败
     *  @warn   回调中不能处理过多的业务，否则可能会导致栈溢出
     */
    typedef void (*wifi_info_cb)(ez_iot_ap_wifi_info_t *wifi_info);
	
	/**
     *  @brief      wifi功能初始化，需要在ez_iot_ap_init和ez_iot_wifi_init调用之前
     *  @return     成功：ez_errno_succ，失败：other
     */
    ez_err_e ez_iot_wifi_init();

    /**
     *  @brief      station模式下连接路由器 
     *  @return     成功：ez_errno_succ，失败：other
     */
    ez_err_e ez_iot_sta_init(char *ssid, char *password);

    /**
     *  @fn         AP模块初始化，包括初始化ap以及启动httpserver
     *  @info       httpserver默认ip为192.168.4.1，端口设置为80
     *  @param[in]  dev_info：设备信息
     *  @param[in]  cb：回调函数，用于获取wifi设置状态
     *  @param[in]  timeout：wifi配置超时时间，范围1-60，单位min，默认15min
     *  @param[in]  support_apsta：是否支持ap、sta共存模式
     *  @return     成功：ez_errno_succ，失败：other
     */
    ez_err_e ez_iot_ap_init(ez_iot_ap_dev_info_t *dev_info, wifi_info_cb cb, int timeout, bool support_apsta);

    /**
     *  @fn     AP模块反初始化，包括停掉httpserver和ap模块
     *  @return     成功：ez_errno_succ，失败：other
     */
    ez_err_e ez_iot_ap_finit();

    /**
     *  @fn         蓝牙配网模块初始化，启动blufi
     *  @param[in]  dev_info：设备信息
     *  @param[in]  cb：回调函数，用于获取wifi设置状态
     *  @param[in]  timeout：wifi配置超时时间，范围1-60，单位min，默认15min
     *  @return     成功：ez_errno_succ，失败：other
     */
    ez_err_e ez_iot_ble_init(ez_iot_ap_dev_info_t *dev_info, wifi_info_cb cb, int timeout);

    /**
     * @fn      ble模块反初始化
     */
    ez_err_e ez_iot_ble_finit();

#ifdef __cplusplus
}
#endif

#endif //EZVIZ_AP_NET_API_H