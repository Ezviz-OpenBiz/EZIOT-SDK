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
 * 2017/3/4       panlong
 *******************************************************************************/
#ifndef H_SDK_KERNEL_DEF_H_
#define H_SDK_KERNEL_DEF_H_

#include "base_typedef.h"
#include "ezdev_sdk_kernel_struct.h"

#define ezdev_sdk_recv_topic_len 256             ///<	设备SDK 一些命名的长度
#define ezdev_sdk_type_len 16                    ///<	设备SDK 类型长度
#define ezdev_sdk_name_len 64                    ///<	设备SDK 一些命名的长度
#define ezdev_sdk_sharekey_len 32                ///<	设备share key 长度
#define ezdev_sdk_devserial_maxlen 72            ///< 设备序列号支持最大长度
#define ezdev_sdk_total_len 128                  ///<	用于临时变量的存放
#define ezdev_sdk_identificationcode_max_len 256 ///<	设备固件识别码最大长度
#define ezdev_sdk_md5_len 32                     ///<	md5长度
#define ezdev_sdk_productkey_len 32              ///<	productkey最长的长度
#define ezdev_sdk_json_default_size 1024         ///<	bscJSON_PrintBuffered 调用时给的默认大小，减少多次malloc/free过程
#define ezdev_sdk_domain_id 1100                 ///< 设备主动下线时，内部发送下线消息使用的领域id
#define ezdev_sdk_offline_cmd_id 0X00002807      ///< 设备主动下线时发送的指令id
#define ezdev_sdk_cmd_version "v1.0.0"           ///< 指令版本
#define version_max_len 32                       ///<	版本长度
#define QOS_T1 1                                 ///< 采用Qos1


#define SendISAPIReq 0x00004D01             ///< isapi协议，根据协议过滤发送报文大小
#define SendISAPIRsp 0x00004D02             ///<
#define SendISAPIEventReq 0x00004D03        ///<
#define SendISAPIEventRsp 0x00004D03        ///<
#define ezdev_sdk_com_msg_max_buf 1024 * 16 ///< 非isapi消息报文最大允许发送的size


#define ezdev_sdk_model_type_len 16 ///<	设备SDK 类型长度

#ifdef RAM_LIMIT
//主要是给realtek使用
#define ezdev_sdk_send_buf_max 1024 * 2
#define ezdev_sdk_recv_buf_max 1024 * 2

#define lbs_send_buf_max 1024 * 2
#define lbs_recv_buf_max 1024 * 2

#define ezdev_sdk_extend_count 8 ///<	支持的扩展模块数量

/**
* \brief   SDK 一个领域支持的风控指令 最大数
*/
#define ezdev_sdk_risk_control_cmd_max 8
#define ezdev_sdk_queue_max 32
#else //RAM_LIMIT
/**
* \brief   DAS MQTT 会话使用的缓存
*/
#define ezdev_sdk_send_buf_max			1024*2
#define ezdev_sdk_recv_buf_max			1024*2

#define lbs_send_buf_max			1024*2
#define lbs_recv_buf_max			1024*2

#ifdef _FREE_RTOS_32_
#define	ezdev_sdk_extend_count	 5				///< 支持的扩展模块数量
#elif defined(_FREE_RTOS_)
#define	ezdev_sdk_extend_count	 4
#else
#define	ezdev_sdk_extend_count	 8
#endif


/**
* \brief   SDK 一个领域支持的风控指令 最大数
*/
#define ezdev_sdk_risk_control_cmd_max 64
#define	ezdev_sdk_queue_max						16

#endif //RAM_LIMIT

#define ezdev_sdk_das_default_keepaliveinterval 30 ///<	DAS默认心跳时间
#define ezdev_sdk_sharekey_salt "www.88075998.com"

#define ezdev_sdk_max_publish_count 2 ///<	最多发布的次数
#define ezdev_sdk_msg_type_req 1      ///<	das信令类型:请求
#define ezdev_sdk_msg_type_rsp 2      ///<	das信令类型:响应

typedef enum
{
    sdk_no_risk_control = 0, ///<	不风控
    sdk_risk_control = 1,    ///<	风控
} sdk_risk_control_flag;

// /**
// * \brief   设备SDK扩展模块ID定义
// */
// typedef enum
// {
// 	ezdev_sdk_extend_begin = 0,				///<	扩展模块起始
// 	ezdev_sdk_extend_count = 16				///<	支持的扩展模块数量
// }ezdev_sdk_extend_id;

/**
* \brief   标记SDK入口状态
*/
typedef enum
{
    sdk_entrance_normal,     ///<	SDK正常入口
    sdk_entrance_switchover, ///<	SDK入口切换
    sdk_entrance_authcode_invalid
} sdk_entrance_state;

/**
* \brief   标记SDK状态
*/
typedef enum
{
    sdk_idle0 = 0, ///<	刚创建未初始化
    sdk_idle,      ///<	空闲		初始化状态
    sdk_start,     ///<	sdk处于启动
    sdk_stop,      ///<	sdk处于关闭
    sdk_idle2,     ///<	空闲		关闭后的状态
} sdk_state;

/**
* \brief   标记SDK与cloud连接状态
*/
typedef enum
{
    sdk_cnt_unredirect = 0,      ///<	未重定向状态		需要到LBS去做重定向
    sdk_cnt_redirected = 1,      ///<	重定向完成			需要到DAS上注册
    sdk_cnt_das_reged = 2,       ///<	完成注册			需要接收和发送指令
    sdk_cnt_das_break = 4,       ///<	与DAS处于中断状态	需要重连
    sdk_cnt_das_fast_reg = 5,    ///<	设备快速上线
    sdk_cnt_das_fast_reg_v3 = 6, ///<    RF快速重连
} sdk_cloud_cnt_state;

typedef enum
{
    sdk_dev_auth_sap = 0,    ///<	SAP认证
    sdk_dev_auth_license = 1 ///<	license认证
} sdk_dev_auth_mode;

typedef enum
{
    sdk_v3_unreg = 0, ///<	V3协议未使用
    sdk_v3_reged,
} sdk_v3_reg_status;

/**
* \brief   与LBS交互协议
*/
typedef struct
{
    unsigned char *head_buf;
    EZDEV_SDK_UINT8 head_buf_Len; ///<	可生气5个字节大小以上
    EZDEV_SDK_UINT8 head_buf_off;

    unsigned char *payload_buf;
    EZDEV_SDK_UINT32 payload_buf_Len;
    EZDEV_SDK_UINT32 payload_buf_off;
} lbs_packet;

/**
* \brief   与LBS交互通用协议体
*/
typedef struct
{
    unsigned char pro_form_version;      ///<	交互协议形式版本号
    unsigned char pro_type_low_version;  ///<	交互协议类型版本号(低)
    unsigned char pro_type_high_version; ///<	交互协议类型版本号(高)
} lbs_common;

typedef struct
{
    char server_name[ezdev_sdk_name_len]; ///<	lbs 域名
    char server_ip[ezdev_sdk_name_len];   ///<	lbs Ip
    EZDEV_SDK_INT16 server_port;          ///<	lbs port
} ezdev_server_info;

typedef struct
{
    EZDEV_SDK_UINT16 das_port;
    EZDEV_SDK_UINT16 das_udp_port;
    char das_address[ezdev_sdk_ip_max_len];
    char das_domain[ezdev_sdk_ip_max_len];
    char das_serverid[ezdev_sdk_name_len];
} das_info;

/**
 * \brief   设备基本信息
 */
typedef struct
{
    EZDEV_SDK_UINT16 dev_access_mode;                                          ///		设备接入模式  0-普通（2.0）   1-HUB（2.0）
    sdk_dev_auth_mode dev_auth_mode;                                           ///<    认证模式：0 SAP认证   1 licence认证
    EZDEV_SDK_UINT16 dev_status;                                               ///<	设备工作状态  1：正常工作模式  5：待机(或睡眠)工作模式
    char dev_subserial[ezdev_sdk_devserial_maxlen];                            ///<	设备短序列号(对应licence认证中device_id)
    char dev_verification_code[ezdev_sdk_verify_code_maxlen];                  ///<	设备验证码(对应licence认证中licence)
    char dev_serial[ezdev_sdk_devserial_maxlen];                               ///<	设备长序列号
    char dev_firmwareversion[ezdev_sdk_name_len];                              ///<	设备固件版本号
    char dev_type[ezdev_sdk_name_len];                                         ///<	设备型号
    char dev_typedisplay[ezdev_sdk_name_len];                                  ///<	设备显示型号
    char dev_mac[ezdev_sdk_name_len];                                          ///<	设备网上物理地址
    char dev_nickname[ezdev_sdk_name_len];                                     ///<	设备昵称
    char dev_firmwareidentificationcode[1]; 								   ///<	设备固件识别码
    EZDEV_SDK_UINT32 dev_oeminfo;                                              ///<	设备的OEM信息
} dev_basic_info;

/**
* \brief   领域信息
*/
typedef struct
{
    sdk_risk_control_flag domain_risk;                               ///<	领域是否被风控
    EZDEV_SDK_UINT32 cmd_risk_array[ezdev_sdk_risk_control_cmd_max]; ///<	领域内被风控的指令
    ezdev_sdk_kernel_extend kernel_extend;                           ///<	SDK注册进来的领域扩展
} ezdev_sdk_kernel_domain_info;

/**
* \brief   领域信息
*/
typedef struct
{
    ezdev_sdk_kernel_extend_v3 kernel_extend; ///<	SDK注册进来的领域扩展
} ezdev_sdk_kernel_domain_info_v3;

typedef struct
{
    EZDEV_SDK_UINT8 lbs_redirect_times;  ///<	记录lbs连续重定向的次数
    EZDEV_SDK_UINT8 das_retry_times;     ///<	记录das连续重连的次数
    EZDEV_SDK_BOOL secretkey_applied;    ///<	是否已经申请过secretkey
    EZDEV_SDK_UINT16 secretkey_interval; ///<	申请secretkey出错后重试间隔(s)
    EZDEV_SDK_UINT32 secretkey_duration; ///<	申请secretkey出错后重试总时间(s)
	EZDEV_SDK_UINT8		hub_reg_status;										    ///<	�Ƿ�ע��hubģ��
    sdk_v3_reg_status v3_reg_status;     ///<	V3协议是否使用

    sdk_entrance_state entr_state;  ///<	sdk入口状态
    sdk_state my_state;             ///<	sdk状态
    sdk_cloud_cnt_state cnt_state;  ///<	连接状态
    ezdev_sdk_time cnt_state_timer; ///<	重连相关的定时器

//	char dev_subserial[ezdev_sdk_devserial_maxlen];
    unsigned char master_key[ezdev_sdk_masterkey_len];
    unsigned char dev_id[ezdev_sdk_devid_len];
    unsigned char session_key[ezdev_sdk_sessionkey_len];
	unsigned char token[ezdev_sdk_token_len];

    EZDEV_SDK_UINT32 das_keepalive_interval; ///<	DAS心跳时间间隔
    dev_basic_info dev_info;                 ///<	设备基础信息

    ezdev_server_info server_info; ///<	lbs服务信息
    das_info redirect_das_info;    ///<	lbs重定向过来的das信息


    char szMainVersion[version_max_len]; ///<	SDK主版本号

    EZDEV_SDK_UINT8 reg_mode;                         ///<	设备注册模式
    ezdev_sdk_kernel_platform_handle platform_handle; ///<	lbs 交互中使用
    sdk_risk_control_flag access_risk;                ///<	接入风控标识
} ezdev_sdk_kernel;

typedef struct
{
    EZDEV_SDK_UINT8 random_1;
    EZDEV_SDK_UINT8 random_2;
    EZDEV_SDK_UINT8 random_3;
    EZDEV_SDK_UINT8 random_4;

    EZDEV_SDK_UINT16 dev_access_mode;
    sdk_dev_auth_mode dev_auth_mode;
    char dev_subserial[ezdev_sdk_devserial_maxlen];
    unsigned char master_key[ezdev_sdk_masterkey_len];
    unsigned char dev_id[ezdev_sdk_devid_len];

    unsigned char session_key[ezdev_sdk_sessionkey_len];
    unsigned char share_key[ezdev_sdk_sharekey_len];
    EZDEV_SDK_UINT16 share_key_len;

    lbs_packet global_out_packet; ///<*	lbs 发送缓冲区
    lbs_packet global_in_packet;  ///<*	lbs 接收缓冲区

    ezdev_sdk_net_work lbs_net_work;
} lbs_affair;

/**
 * \brief   设备发布消息的消息存储载体
 */
typedef struct
{
    EZDEV_SDK_UINT16 max_send_count;      ///<	最大发布次数，send后--
    ezdev_sdk_kernel_pubmsg msg_conntext; ///<	发布的消息内容
} ezdev_sdk_kernel_pubmsg_exchange;

/**
 * \brief   设备发布消息的消息存储载体 3.0协议
 */
typedef struct
{
    EZDEV_SDK_UINT16 max_send_count;            ///<	最大发布次数，send后--
    ezdev_sdk_kernel_pubmsg_v3 msg_conntext_v3; ///<	发布的消息内容
} ezdev_sdk_kernel_pubmsg_exchange_v3;

typedef enum
{
    extend_cb_start, ///<	ezdev_sdk_kernel_extend_start
    extend_cb_stop,  ///<	ezdev_sdk_kernel_extend_stop
    extend_cb_event  ///<	ezdev_sdk_kernel_extend_event
} extend_cb_type;

/**
* \brief   kernel内部使用的扩展异步回调消息
*/
typedef struct
{
    extend_cb_type cb_type;
    ezdev_sdk_kernel_event cb_event;
} ezdev_sdk_kernel_inner_cb_notic;

void ezdev_sdk_kernel_log (sdk_log_level level, int sdk_error, int othercode, \
						   const char *fmt, ...);

ezdev_sdk_kernel *get_ezdev_sdk_kernel();

#if defined(_WIN32) || defined(_WIN64)
#define ezdev_sdk_kernel_log_error(sdk_error, othercode, ...) ezdev_sdk_kernel_log(sdk_log_error, sdk_error, othercode, __VA_ARGS__)
#define ezdev_sdk_kernel_log_warn(sdk_error, othercode, ...) ezdev_sdk_kernel_log(sdk_log_warn, sdk_error, othercode, __VA_ARGS__)
#define ezdev_sdk_kernel_log_info(sdk_error, othercode, ...) ezdev_sdk_kernel_log(sdk_log_info, sdk_error, othercode, __VA_ARGS__)
#define ezdev_sdk_kernel_log_debug(sdk_error, othercode, ...) ezdev_sdk_kernel_log(sdk_log_debug, sdk_error, othercode, __VA_ARGS__)
#define ezdev_sdk_kernel_log_trace(sdk_error, othercode, ...) ezdev_sdk_kernel_log(sdk_log_trace, sdk_error, othercode, __VA_ARGS__)
#else
#define ezdev_sdk_kernel_log_error(sdk_error, othercode, args...) ezdev_sdk_kernel_log(sdk_log_error, sdk_error, othercode, ##args)
#define ezdev_sdk_kernel_log_warn(sdk_error, othercode, args...) ezdev_sdk_kernel_log(sdk_log_warn, sdk_error, othercode, ##args)
#define ezdev_sdk_kernel_log_info(sdk_error, othercode, args...) ezdev_sdk_kernel_log(sdk_log_info, sdk_error, othercode, ##args)
#define ezdev_sdk_kernel_log_debug(sdk_error, othercode, args...) ezdev_sdk_kernel_log(sdk_log_debug, sdk_error, othercode, ##args)
#define ezdev_sdk_kernel_log_trace(sdk_error, othercode, args...) ezdev_sdk_kernel_log(sdk_log_trace, sdk_error, othercode, ##args)
#endif

#define global_ezdev_sdk_kernel get_ezdev_sdk_kernel()

#endif