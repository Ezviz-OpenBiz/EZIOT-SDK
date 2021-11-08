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

#ifndef _EZ_IOT_OTA_H_
#define _EZ_IOT_OTA_H_

#if defined(_WIN32) || defined(_WIN64)

#ifdef GLOBAL_DECL_EXPORTS
#define EZIOT_API __declspec(dllexport)
#else //GLOBAL_DECL_EXPORTS
#define EZIOT_API __declspec(dllimport)
#endif //GLOBAL_DECL_EXPORTS

#define GLOBAL_CALLBACK __stdcall

#else
#define EZIOT_API

#define GLOBAL_CALLBACK
#endif

#include <stdint.h>
#include "ez_iot_errno.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        ota_code_none = 0,                     ///< 没有错误
        ota_code_file_size = 0x00500032,       ///< 下文件大小和查询的信息不匹配
        ota_code_file_size_range = 0x00500033, ///< 下文件大小超过分区大小
        ota_code_dns = 0x00500034,             ///< 下载地址域名解析失败
        ota_code_download = 0x00500035,        ///< 下载出错（超过最大重试次数）
        ota_code_digest = 0x00500036,          ///< 底层接口调用失败
        ota_code_sign = 0x00500037,            ///< 签名校验失败
        ota_code_damage = 0x00500038,          ///< 升级包损坏
        ota_code_mem = 0x00500039,             ///< 内存不足
        ota_code_burn = 0x0050003a,            ///< 烧录固件出错
        ota_code_genneral = 0x0050003b,        ///< 未知错误
        ota_code_cancel = 0x0050003c,          ///< 主动停止
        ota_code_recovery = 0x0050003d,        ///< 升级失败，通过备份系统或最小系统恢复
        ota_code_uart_err = 0x0050003e,        ///< 外挂设备升级失败
        ota_code_max = 0x0050004f
    } ota_errcode_e;


    /**
     * \brief   服务下发的ota消息通知回调
     * \warning 不能阻塞
     * \note    
     */
    typedef enum
    {
        start_upgrade,      ///< 通知设备执行升级,收到该消息后,设备开始执行升级动作 data: ota_upgrade_info_t*,
    } ota_event_e;

    /**
     * @brief 升级过程状态变化，设备只有在ota_state_upgradeable、ota_state_succ、ota_state_fail三种状态下,
     *        云端才能判断设备是否可升级。
     */
    typedef enum
    {
        ota_state_starting = 1,           ///< 开始升级
        ota_state_downloading = 2,        ///< 正在下载
        ota_state_download_completed = 3, ///< 下载完成
        ota_state_burning = 4,            ///< 正在烧录
        ota_state_burning_completed = 5,  ///< 烧录完成
        ota_state_rebooting = 6,          ///< 正在重启
    } ota_status_e;

    typedef enum
    {
        ota_progress_min = 1,

        ota_progress_max = 100
    } ota_progress_e;

    typedef struct
    {
        int8_t url[270];       ///< 包下载路径,不带http/https前缀。设备根据自己的能力选择下载通道。
        int8_t fw_ver_dst[64]; ///< 差分包对应的原升级包版本号，只有这个版本的固件才能通过差分包升级至fw_ver版本的固件
        int8_t degist[32 + 1]; ///< 升级包的摘要值,包含'\0'
        int size;              ///< 软件包大小，单位Byte
    } ota_file_diff_t;

    typedef struct
    {
        int8_t mod_name[56];    ///< 模块名称,iot 互联模组：PID,e.g41X9TLMJB9GOA2ABOXKLEP, 透传模组对应的mcu 模组为PID_MCU
        int8_t url[270];        ///< 包下载路径,不带http/https前缀，iot互联模组url：ezviz-ota.cn-sh2.ufileos.com/e7135e21d0be26c483ce90e1c09a131f
        int8_t fw_ver[32];      ///< 升级包版本号 e.g V1.1.5 build 210602
        int8_t degist[32 + 1];  ///< 升级包的摘要值,包含'\0'
        int32_t size;           ///< 软件包大小，单位Byte
        ota_file_diff_t *pdiffs;///< 该版本对应的的差分包
    } ota_file_info_t;

     /**
    * \brief   设备升级模块信息
    */
    typedef struct
    {
        int8_t *mod_name;  ///<  模块名称, max 256字节
        int8_t *fw_ver;    ///< 模块固件版本号, max 64字节
    } ota_module_t;

    /**
    * \brief   设备升级模块信息列表
    */
    typedef struct
    {
        int8_t num;                   ///< 升级模块数量，最多支持16组
        ota_module_t *plist;       ///< 升级模块信息列表
    } ota_modules_t;

    typedef enum
    {
        result_suc,                 ///<下载成功
        result_failed,              ///<下载失败
    } ota_cb_result_e;

    typedef struct
    {
        int retry_max;               ///< 下载最大重试次数
        int file_num;                ///< 升级信息数量
        int interval;                ///< 上报升级进度间隔
        ota_file_info_t *pota_files; ///< 升级信息
    } ota_upgrade_info_t;

    typedef struct
    {
        int8_t  url[286];   ///< 升级包的url信息,带http/https前缀，270+前缀长度
        int8_t  degist[33]; ///< 升级包摘要
        int32_t block_size; ///< 接收缓冲区大小\单次接收大小
        int32_t total_size; ///< 升级包的总大小
        int32_t timeout_s;  ///< 下载超时时间
        int32_t retry_max;  ///< 下载最大重试次数
    } ota_download_info_t;

   /** 
    * @brief 接收服务下发的升级消息回调,例：通知设备升级消息
    */
    typedef struct
    {
        int8_t dev_serial[72]; ///< 子设备序列号
    } ota_res_t;

    /** 
    * @brief 接收服务下发的升级消息回调,例：通知设备升级消息
    * @param pres  用于标识子设备,非空，长度为 0 表示主设备,长度非0 表示子设备
    * @param event 消息类型,参考ota_event_e
    * @param data  对应ota_event_e类型中的数据结构体
    * @param len  sizeof(struct)
    * return 处理出错返回对应的错误码
    */
    typedef struct 
    {
        int (*ota_recv_msg)(ota_res_t* pres, ota_event_e event, void *data, int len);  
    } ota_msg_cb_t;

    /**
    * \brief	ota初始化参数
    */
    typedef struct 
    {
        ota_msg_cb_t  cb;	///< 回调函数
    } ota_init_t;
    
    /**
     * @brief 升级包下载数据回调
     * @param total_len :package total size
     * @param offset    :offset of current download package
     * @param data &len :current download package data & len 
     * @param remain_len:the size left to process in next cb.
     * @param user_data :user input data
     * @return int 
     */
    typedef int (*get_file_cb)(uint32_t total_len, uint32_t offset, void *data, uint32_t len, void* user_data);
    /**
     * @brief 升级包下载过程通知回调
     * @param result  下载结果,0表示成功 非0表示失败
     * @param user_data: 用户自定义协议
     * @return int 
     */
    typedef void (*notify_cb)(ota_cb_result_e result, void* user_data);
    
    /**
     * @brief 升级组件初始化
     * 
     * @param pdata_cbs 
     * @return ez_err_e 
     */
    EZIOT_API ez_err_e ez_iot_ota_init(ota_init_t *pota_init);

    /**
     * @brief 上报升级模块信息,同步接口，请勿在SDK的任何消息回调里调用该接口
     * @param pres 设备信息，NULL标识当前设备，!NULL标识子设备
     * @param pmodules 设备模块信息列表,可以有多个模块，
     * @param  timeout_ms   指定超时时间及Qos
     * @return ez_err_e 
     */
    EZIOT_API ez_err_e ez_iot_ota_modules_report(const ota_res_t *pres, const ota_modules_t* pmodules, uint32_t timeout_ms);

    /**
     * @brief 通知升级服务，设备待升级，设备开机上线上报一次即可。
     * 
     * @param pres 设备信息，NULL标识当前设备，!NULL标识子设备
     * @return ez_err_e 
     */
    EZIOT_API ez_err_e ez_iot_ota_status_ready(const ota_res_t *pres, int8_t* pmodule);

    /**
     * @brief 升级成功信息上报
     * 
     * @param pres 设备信息，NULL标识当前设备，!NULL标识子设备
     * @return ez_err_e 
     */
    EZIOT_API ez_err_e ez_iot_ota_status_succ(const ota_res_t *pres, int8_t* pmodule);

    /**
     * @brief 升级失败信息上报
     * 
     * @param pres 设备信息，NULL标识当前设备，!NULL标识子设备
     * @param pmodule 固件模块名称
     * @param perr_msg 升级失败错误码
     * @param code 升级失败错误码
     * @return ez_err_e 
     */
    EZIOT_API ez_err_e ez_iot_ota_status_fail(const ota_res_t *pres, int8_t* pmodule, int8_t* perr_msg, ota_errcode_e code);

    /**
     * @brief 上报设备升级进度,请按照服务下发间隔时间上报,
     * @param pres 设备信息（设备序列号），NULL标识当前设备，!NULL标识子设备
     * @param pmodule 模块信息，NULL标识当前设备，!NULL标识子设备
     * @param status 设备升级状态
     * @param progress 升级进度 1-100
     * @return ez_err_e 
     * 
     * note:app点击升级后,需要在10s时间内上报一次进度信息,否则app端可能会直接提示升级成功，正常可报stating
     * 
     */
    EZIOT_API ez_err_e ez_iot_ota_progress_report(const ota_res_t *pres, int8_t* pmodule, ota_status_e status, int16_t progress);

    /**
     * @brief 下载升级包
     * @param input_info 升级信息
     * @param get_file   下载数据回调
     * @param notify      下载过程信息通知，包括超时,c
     * @return ez_err_e 
     */
    EZIOT_API ez_err_e ez_iot_ota_download(ota_download_info_t *input_info, get_file_cb file_cb, notify_cb notify, void *user_data);

    /**
     * @brief 升级模块反初始化
     * 
     */
    EZIOT_API ez_err_e ez_iot_ota_deinit();

	typedef struct
	{
		int pack_total_len;
		int per_pack_len; // output
		char check_sum[32+1];
		char version[32];
	} relate_dev_ota_t;
	
	typedef struct
	{
		char dev_serial[72];		  ///< 设备长序列号(对应licence认证中productKey:deviceName)
		char dev_firmwareversion[64]; ///< 设备固件版本号
		char dev_type[64];			  ///< 设备型号(对应licence认证中productKey)
	} relate_dev_info_t;
	
	
#ifdef __cplusplus
}
#endif

#endif
