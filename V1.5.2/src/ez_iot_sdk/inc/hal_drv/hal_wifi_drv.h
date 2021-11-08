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

#ifndef _EZVIZ_WIFI_BSP_API_H_
#ifdef __cplusplus
extern "C"
{
#endif
#define _EZVIZ_WIFI_BSP_API_H_

#include <stdbool.h>
#include <stdint.h>

    typedef enum
    {
        EZVIZ_WIFI_STATE_PARAMETER_ERROR = 101,
        EZVIZ_WIFI_STATE_NOT_CONNECT = 102,
        EZVIZ_WIFI_STATE_CONNECT_SUCCESS = 104,
        EZVIZ_WIFI_STATE_UNKNOW = 105,
        EZVIZ_WIFI_STATE_PASSWORD_ERROR = 106,
        EZVIZ_WIFI_STATE_AP_STA_CONNECTED = 107,

        EZVIZ_WIFI_STATE_BEACON_TIMEOUT = 200,
        EZVIZ_WIFI_STATE_NO_AP_FOUND = 201,
        EZVIZ_WIFI_STATE_AUTH_FAIL = 202,
        EZVIZ_WIFI_STATE_ASSOC_FAIL = 203,
        EZVIZ_WIFI_STATE_HANDSHAKE_TIMEOUT = 204,
        EZVIZ_WIFI_STATE_BASIC_RATE_NOT_SUPPORT = 205,
    } iot_wifi_state_t;

    typedef struct
    {
        char ssid[32 + 1];     // 设备连接的wifi ssid
        char password[64 + 1]; // 设备连接的wifi password
        char token[128];       // app端校验的token
        char domain[128];      // 设备注册平台地址;
        char PID[128];         // 厂家识别码
        char res[2];
    } wifi_info_t;

    typedef struct tag_WIFI_INFO_LIST
    {
        uint8_t authmode;
        int8_t rssi;
        uint8_t channel;
        char bssid[6];
        char ssid[33];
        char res[2];
    } wifi_info_list_t;

/*************************************************************************************************
*                                        WIFI CONFIG                                             *                                                         
*************************************************************************************************/

    /**
     *  @brief  wifi功能初始化，需要再ap或staion模式启动之前调用
     */
    void iot_wifi_init(void);

    /**
     *  @brief  启动ap模式
     *  @param  ssid: 设备ap ssid
     *  @param  pwd： 设备ap password
     *  @param  authmode：设备ap加密模式，若pwd为空或长度为0，则authmode为OPEN，见：wifi_auth_mode_t
     *  @param  ap_sta：是否支持ap和station模式共存，用于优化配网体验
     *  @return 0：成功，其他：失败
     */
    int iot_ap_start(char *ssid, char *pwd, int authmode, bool ap_sta);

    /*启动sta模式*/
    /**
     *  @brief  启动station模式
     *  @return 0：成功，其他：失败
     */
    int iot_sta_start();
    
    /**
     *  @brief  停止ap模式
     */
    void iot_ap_stop();

    /**
     *  @brief  停止station模式
     */
    void iot_sta_stop();

    /*sta或ap+sta模式下扫描wifi列表*/
    /**
     *  @brief  station模式或ap+station模式下扫描wifi列表
     *  @param  max_ap_num：本次扫描获取的ap列表最大size
     *  @param  ap_list：数组，其size为max_ap_num，结构体为wifi_info_list_t
     *  @info   wifi列表，按照rssi从大到小的顺序排列
     *  @return 实际扫描到的wifi个数
     */
    uint16_t iot_sta_get_scan_list(uint16_t max_ap_num, wifi_info_list_t *ap_list);

    /**
     *  @brief  station模式下连接指定ssid
     *  @return 0：成功，其他：失败
     */
    int iot_sta_connect(char *ssid, char *pwd);

    /**
     *  @brief  station模式下断开连接
     */
    void iot_sta_disconnect();

    /**
     *  @brief  获取当前连接成功的wifi信号强度
     *  @param  rssi：返回获取到的rssi
     *  @return 0：成功，其他：失败
     */
    int iot_get_ap_rssi(int8_t *rssi);

    /**
     *  @brief  获取当前wifi状态，可以随时调用，只返回连接网络过程中的状态，不关注ap模式下的状态
     *  @return iot_wifi_state_t，当前wifi连接状态，主要用于返回配网错误码
     */
    iot_wifi_state_t iot_sta_get_state(void);
    
    /**
     *  @brief  wifi反初始化接口
     */
    void iot_wifi_finit(void);



/************************** 以下接口，若无使用场景，可以不实现 **************************/

    /*sta模式下，是否获取到ip地址*/
    /**
     *  @brief  station模式下，是否获取到ip地址
     *  @return true：已经获取到ip地址，false：未获取到ip地址
     *  @info   判断获取ip地址，用于设备上线
     */
    bool iot_sta_if_got_ip();

    /*设置国家码*/
    void iot_set_country_code(char *country_code);

    /** ip地址回调注册，用于上层保存IP地址 **/
    typedef void (*ip_info_cb)(void *ip_info);
    void iot_wifi_cb_register(ip_info_cb cb);
    
    /** 设备hostname设置回调，用于设备在合适的时机设置hostname **/
    typedef void (*set_sta_hostname_cb)(void);
    void iot_hostname_cb_register(set_sta_hostname_cb cb);

    typedef void (*sta_update_cb)(int sta, int reset);
    typedef void (*ssid_update_cb)(char *ssid);
    void iot_netmgr_cb_register(sta_update_cb sta_cb, ssid_update_cb ssid_cb);

#ifdef __cplusplus
}
#endif
#endif /* _EZVIZ_WIFI_BSP_API_H_ */
