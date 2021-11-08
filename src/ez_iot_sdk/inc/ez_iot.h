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

#ifndef _EZ_IOT_H_
#define _EZ_IOT_H_

#include <stddef.h>

#include <stdint.h>
#include <stdbool.h>
#include "ez_iot_errno.h"

typedef struct
{
    int8_t *svr_name;  ///<服务器域名
    uint16_t svr_port; ///<服务器端口号
} ez_iot_srv_info_t;

typedef struct
{
    int8_t auth_mode;                 ///< 认证模式：0 SAP认证   1 licence认证
    int8_t dev_subserial[72];         ///< 设备短序列号(对应licence认证中productKey:deviceName)
    int8_t dev_verification_code[48]; ///< 设备验证码(对应licence认证中deviceLicense)
    int8_t dev_serial[72];            ///< 设备长序列号(对应licence认证中productKey:deviceName)
    int8_t dev_firmwareversion[64];   ///< 设备固件版本号
    int8_t dev_type[64];              ///< 设备型号(对应licence认证中productKey)
    int8_t dev_typedisplay[64];       ///< 设备显示型号(对应licence认证中productKey)
    int8_t dev_mac[64];               ///< 设备网上物理地址
    int8_t dev_nickname[64];          ///< 设备昵称
    
    int8_t dev_id[32];                ///< 设备的devid，由配网时候颁发或者上线时候更新
    int8_t dev_masterkey[16]; ///< 设备的masterkey，由首次上线时候生成或者按平台策略更新
    int8_t dev_token[128];    ///< 添加设备用，由配网的时候颁发，上线后完成绑定
    int8_t user_id[33];       ///< 设备所属用户的id，设备绑定用户后保存
    int8_t res1[3];
} ez_iot_dev_info_t;

typedef enum
{
    ez_iot_event_online,          ///< 设备上线
    ez_iot_event_offline,         ///< 设备掉线
    ez_iot_event_disconnect,      ///< 设备网络断开连接
    ez_iot_event_reconnect,       ///< 设备网络重连成功
    ez_iot_event_devid_update,    ///< 设备devid更新，需要外部保存至flash
    ez_iot_event_masterkey_update, ///< 设备masterkey更新，需要外部保存至flash
    ez_iot_event_svrname_update,  ///< 设备上线的域名更新，下次上线需要用新的域名，需要外部保存至flash
    ez_iot_event_keepalive_update ///< 设备心跳时间更变
} ez_iot_event_t;

typedef struct
{
    char user_id[64];   ///< 绑定设备的用户ID
    char user_name[64]; ///< 绑定设备的用户名字
} ez_iot_on_binding_t;

typedef struct
{
    char host[64];     ///< 服务器域名
    int port;          ///< 服务器端口
    int interval;      ///< 校时间隔(s)
    char timezone[16]; ///< 设备时区
} ez_iot_ntp_info_t;

typedef struct
{
    int32_t challenge_code;  ///< 挑战码
    int32_t validity_period; ///< 有效期(秒)
} ez_iot_challenge_t;

typedef enum
{
    ez_iot_msg_binding,         ///< 通知设备被用户绑定, msg data: \c ez_iot_on_binding_t
    ez_iot_msg_unbinding,       ///< 通知设备被用户解除绑定, msg data: null
    ez_iot_msg_ntp_info,        ///< 服务下发ntp校时信息, msg data: \c ez_iot_ntp_info_t
    ez_iot_msg_contact_binding, ///< 服务下绑定请求, 需要设备物理确认后调用 \c ez_iot_contact_binding完成响应, msg data: \c ez_iot_challenge_t
} ez_iot_cloud2dev_msg_type_t;

typedef struct
{
    /* 接收来自云端的信令 */
    int32_t (*recv_msg)(ez_iot_cloud2dev_msg_type_t msg_type, void *data, int len);

    /* 接收来自本地的事件 */
    int32_t (*recv_event)(ez_iot_event_t event_type, void *data, int len);
} ez_iot_callbacks_t;

/* kv error code */
typedef enum
{
    ez_kv_no_err,
    ez_kv_read_err,
    ez_kv_write_err,
    ez_kv_not_init_err,
    ez_kv_name_err,
    ez_kv_name_exist_err,
    ez_kv_saved_full_err,
    ez_kv_init_failed,
} kv_err_e;

typedef struct
{
    char *key;
    void *value;
    size_t value_len;
} ez_iot_kv_node_t;

typedef struct
{
    ez_iot_kv_node_t *kvs;
    size_t num;
} ez_iot_kv_t;

typedef struct
{
    /**
     * The KV database initialization.
     *
     * @param default_kv the default KV
     * @return kv_err_e
     */
    kv_err_e (*ez_kv_init)(ez_iot_kv_t *default_kv);

    /**
     * Set a raw KV. If it value is NULL, delete it.
     * If not find it in flash, then create it.
     *
     * @param key KV name
     * @param value KV value
     * @param value KV value size
     * @return kv_err_e
     */
    kv_err_e (*ez_kv_raw_set)(const int8_t *key, int8_t *value, uint32_t length);

    /**
     * Get a raw KV value by key name.
     *
     * @param key KV name
     * @param value KV value
     * @param length KV value length.If it value is NULL, get the length and return 0(success).
     * @return kv_err_e
     */
    kv_err_e (*ez_kv_raw_get)(const int8_t *key, int8_t *value, uint32_t *length);

    /**
     * Delete an KV.
     *
     * @param key KV name
     * @return kv_err_e
     */
    kv_err_e (*ez_kv_del)(const int8_t *key);

    /**
     * Delete the KV pair by its prefix.
     *
     * @param key KV name prefix
     * @return kv_err_e
     */
    kv_err_e (*ez_kv_del_by_prefix)(const int8_t *key_prefix);

    /**
     * Print all KV.
     */
    void (*ez_kv_print)(void);

    /**
     * The KV database finalization.
     */
    void (*ez_kv_deinit)(void);
} ez_iot_kv_callbacks_t;

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief SDK初始化接口
     * 
     * @param psrv_info 接入云端的服务器信息 
     * @param pdev_info 接入云端的设备信息
     * @param pcbs 接收事件和信令的回调接口
     * @return ez_err_e 
     */
    ez_err_e ez_iot_init(const ez_iot_srv_info_t *psrv_info, const ez_iot_dev_info_t *pdev_info, const ez_iot_callbacks_t *pcbs, const ez_iot_kv_callbacks_t *pkvcbs);

    /**
     * @brief SDK启动接口
     * 
     * @return ez_err_e 
     */
    ez_err_e ez_iot_start(void);

    /**
     * @brief SDK停止接口
     * 
     * @return ez_err_e 
     */
    ez_err_e ez_iot_stop(void);

    /**
     * @brief SDK反初始化接口
     * 
     */
    void ez_iot_fini(void);

    /**
     * @brief 设备绑定至用户账号
     * 
     * @param dev_token: 最大长度64字节
     * @return ez_err_e 
     */
    ez_err_e ez_iot_binding(int8_t *dev_token);

    /**
     * @brief 接触式绑定响应，需用户参与物理确认后调用，如按键等
     * 
     * @param challenge_code 服务下发的挑战信息
     * @return ez_err_e 
     */
    ez_err_e ez_iot_contact_binding(int32_t challenge_code);

    /**
     * @brief 获取SDK版本号
     * 
     * @return int8_t* max length:64
     */
    int8_t *ez_iot_get_sdk_version(void);

    /**
     * @brief 校时，阻塞进行
     * @param  ntp_server： ntp服务器地址, 若为空，则不进行ntp校时，直接设置时区和夏令时
     * @param  time_zone_info： 时区信息，如北京时间为"UTC+08:00"
     * @param  daylight:  夏令时，0：非夏令时，1：夏令时
     * @param  daylight_string: 夏令时设置的字符串
     * @return 0：时间校正成功，非0：校正失败
     */
    int8_t ez_iot_correct_time(const char *ntp_server, const char *time_zone_info, int daylight, const char *daylight_string);

    /**
     * @brief 设置心跳保活间隔
     * 
     * @param internal 
     * @return int8_t 
     */
    int8_t ez_iot_set_keepalive(uint16_t internal);

#ifdef __cplusplus
}
#endif

#endif