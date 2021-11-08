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
* 2017/3/2       panlong
*******************************************************************************/


#ifndef H_EZDEV_SDK_KERNEL_STRUCT_H_
#define H_EZDEV_SDK_KERNEL_STRUCT_H_

#include "base_typedef.h"
#include "ezdev_sdk_kernel_error.h"
#include <stdint.h>

#define version_max_len               32     ///< 版本长度
#define ezdev_sdk_extend_name_len     32     ///< 扩展模块名字长度
#define ezdev_sdk_ip_max_len          64     ///< ip最长长度
#define ezdev_sdk_timezone_max_len    32     ///< timezone最长长度
#define ezdev_sdk_sessionkey_len      16     ///< 设备会话秘钥长度，默认是16个字节
#define ezdev_sdk_devid_len           32     ///< 设备唯一标识长度  默认是32字节的字符串
#define ezdev_sdk_masterkey_len       16     ///< 设备mastekey长度 ,默认是16个字节
#define ezdev_sdk_token_len 		  64     ///< 设备的token长度
#define ezdev_sdk_verify_code_maxlen  48     ///< 设备验证码支持最大长度(对应licence认证中product_key)
#define ezdev_sdk_domain_id_len       64
#define ezdev_sdk_resource_id_len     64
#define ezdev_sdk_resource_type_len   64
#define ezdev_sdk_identifier_len      64
#define ezdev_sdk_max_serial_len      72
#define ezdev_sdk_business_type_len   64


#if !defined(ezdev_sdk_name_len)
#define ezdev_sdk_name_len 64 ///< 设备SDK 一些命名的长度
#endif

typedef void *ezdev_sdk_net_work;
typedef void *ezdev_sdk_time;
typedef void *ezdev_sdk_mutex;


/*扩展模块的id*/
typedef enum
{
    sdk_event,       ///<  事件
    sdk_attribution, ///<  属性
    sdk_service,     ///<  操作
    sdk_shadow,      ///<  影子模块
    sdk_ota,
} sdk_kernel_extend_id;

/**
 * 日志级别信息.
 */
typedef enum
{
    sdk_log_error,
    sdk_log_warn,
    sdk_log_info,
    sdk_log_debug,
    sdk_log_trace
} sdk_log_level;

/**
 * 关键信息类型
 */
typedef enum
{
    sdk_keyvalue_devid,     ///< 设备唯一标识  首次设备上线后会分配 一定要写入flash
    sdk_keyvalue_masterkey, ///< 设备masterkey 首次设备上线后会分配 尽量写入flash
    sdk_keyvalue_count      ///< 枚举上限 用来判定越界
} sdk_keyvalue_type;

/**
 * 关键信息类型
 */
typedef enum
{
    sdk_curingdata_secretkey, ///< 验证码不合规设备重新申请的secretkey，一定要固化
    sdk_curingdata_count      ///< 枚举上限，用来判定越界
} sdk_curingdata_type;

/**
 * @addtogroup micro_kernel
 *  \{
 */

enum QOS_T
{
    QOS_T0, ///< 最多一次送到
    QOS_T1, ///< 至少一次送到
    QOS_T2  ///< 准确一次送达
};

/**
 * \brief 往服务器发送的消息结构体
 */
typedef struct
{
    EZDEV_SDK_UINT8 msg_response;        ///< 0:非响应消息 1:响应消息
    enum QOS_T msg_qos;                  ///< 消息QOS类型
    EZDEV_SDK_UINT32 msg_seq;            ///< 消息seq值
    unsigned char *msg_body;             ///< 消息缓冲区
    EZDEV_SDK_UINT32 msg_body_len;       ///< 消息长度，最大不能超过16K
    EZDEV_SDK_UINT32 msg_domain_id;      ///< 领域模块ID
    EZDEV_SDK_UINT32 msg_command_id;     ///< 指令ID
    EZDEV_SDK_PTR    externel_ctx;       ///< 外部自定义数据，在消息发送回执中带回
    EZDEV_SDK_UINT32 externel_ctx_len;   ///< 外部自定义数据长度
    char command_ver[version_max_len];   ///< 指令版本
} ezdev_sdk_kernel_pubmsg;

/**
 * \brief 往服务器发送的消息结构体 3.0协议
 */
typedef struct
{
    enum QOS_T msg_qos;                  ///< 消息QOS类型
    EZDEV_SDK_UINT8 msg_response;        ///< 0:非响应消息 1:响应消息
    EZDEV_SDK_UINT32 msg_seq;            ///< 消息seq值
    EZDEV_SDK_UINT32 msg_body_len;       ///< 消息长度，最大不能超过16K
    EZDEV_SDK_PTR externel_ctx;          ///< 外部自定义数据，在消息发送回执中带回
    EZDEV_SDK_UINT32 externel_ctx_len;   ///< 外部自定义数据长度
    EZDEV_SDK_UINT16 extend_id;          ///< 扩展id
    unsigned char *msg_body;             ///< 消息缓冲区
    char command_ver[version_max_len];   ///< 指令版本
    char domain_id[ezdev_sdk_domain_id_len];         ///< 消息领域id
    char resource_id[ezdev_sdk_resource_id_len];     ///< 设备资源id
    char resource_type[ezdev_sdk_resource_type_len]; ///< 设备资源类型
    char business_type[ezdev_sdk_business_type_len]; ///< "report" / "query" / "set_reply" / "operate_reply"
    char identifier[ezdev_sdk_identifier_len];       ///< 业务标识
    char sub_serial[ezdev_sdk_max_serial_len];       ///< 子设备序列号,如果非hub模式, 这个参数不需要填写
}  ezdev_sdk_kernel_pubmsg_v3;


/**
 * \brief 订阅的消息
 */
typedef struct
{
    EZDEV_SDK_UINT32 msg_seq;          ///< 消息seq值
    void *buf;                         ///< 消息缓冲区
    EZDEV_SDK_UINT32 buf_len;          ///< 消息长度
    EZDEV_SDK_INT32 msg_domain_id;     ///< 扩展模块ID（领域ID）
    EZDEV_SDK_INT32 msg_command_id;    ///< 指令ID
    char command_ver[version_max_len]; ///< 指令版本 

} ezdev_sdk_kernel_submsg;

/**
 * \brief 订阅的消息 v3.0 协议
 */
typedef struct
{
    void *buf;                                       ///< 消息缓冲区
    EZDEV_SDK_UINT8  msg_response;                   ///< 0:非响应消息 1:响应消息
    EZDEV_SDK_UINT32 msg_seq;                        ///< 消息seq值
    EZDEV_SDK_UINT32 buf_len;                        ///< 消息长度
    EZDEV_SDK_UINT16 extend_id;                      ///< 扩展模块ID（领域ID） event/attribute/service
    char command_ver[version_max_len];               ///< 指令版本, 暂时传'3.0'
    char domain_id[ezdev_sdk_domain_id_len];         ///< 消息领域id
    char resource_id[ezdev_sdk_resource_id_len];     ///< 资源id
    char resource_type[ezdev_sdk_resource_type_len]; ///< 资源类型
    char business_type[ezdev_sdk_business_type_len]; ///< "set" / "operate",对应于ezdev_sdk_kernel_pubmsg_v3中的 business_type："set_reply", "operate_reply"
    char identifier[ezdev_sdk_identifier_len];       ///<
    char sub_serial[ezdev_sdk_max_serial_len];       ///< 
} ezdev_sdk_kernel_submsg_v3;


/**
 * \brief 事件类型
 */
typedef enum
{
    sdk_kernel_event_online,                    ///< event_context == sdk_sessionkey_context     设备上线
    sdk_kernel_event_break,                     ///< event_context == sdk_offline_context        设备离线
    sdk_kernel_event_switchover,                ///< event_context == sdk_switchover_context     平台地址发送切换
    sdk_kernel_event_invaild_authcode,          ///< event_context == null                       验证码不合规且没有绑定用户，交给上层APP处理
    sdk_kernel_event_fast_reg_online,           ///< event_context == sdk_sessionkey_context     设备快速上线
    sdk_kernel_event_runtime_err,               ///< evnet_context == sdk_runtime_err_context    设备sdk运行时错误信息
    sdk_kernel_event_reconnect_success,         ///< evnet_context == NULL                       重连成功事件回调  
    sdk_kernel_event_heartbeat_interval_changed ///< event_context == int                        das心跳发生改变事件回调
} sdk_kernel_event_type;

/**
 * \brief 事件结构体
 */
typedef struct
{
    sdk_kernel_event_type event_type;
    void *event_context;
} ezdev_sdk_kernel_event;

typedef struct
{
    EZDEV_SDK_UINT16 das_port;                           ///< das服务器的TCP端口号
    EZDEV_SDK_UINT16 das_udp_port;                       ///< das服务器UDP端口号
    char das_ip[ezdev_sdk_ip_max_len];                   ///< das服务器IP
    char lbs_ip[ezdev_sdk_ip_max_len];                   ///< lbs服务器IP
    unsigned char session_key[ezdev_sdk_sessionkey_len]; ///< 会话秘钥
    int das_socket;                                      ///< das的socket
    char das_domain[ezdev_sdk_ip_max_len];               ///< das的域名
    char das_serverid[ezdev_sdk_name_len];               ///< das的serverid
} sdk_sessionkey_context;

typedef struct
{
    EZDEV_SDK_UINT32 last_error;       ///< 错误码
    char das_ip[ezdev_sdk_ip_max_len]; ///< das服务器IP
    char lbs_ip[ezdev_sdk_ip_max_len]; ///< lbs服务器IP
} sdk_offline_context;

typedef struct
{
    EZDEV_SDK_UINT16 das_udp_port;
    char das_ip[ezdev_sdk_ip_max_len];
    char lbs_ip[ezdev_sdk_ip_max_len];
    unsigned char session_key[ezdev_sdk_sessionkey_len];
    unsigned char lbs_domain[ezdev_sdk_ip_max_len];
} sdk_switchover_context;

typedef enum
{
    TAG_ACCESS,     ///< 设备接入萤石云错误    err_ctx == NULL
    TAG_MSG_ACK,    ///< 设备信令发送回执      err_ctx == sdk_send_msg_ack_context
    TAG_MSG_ACK_V3, ///< 设备信令发送回执      err_ctx == sdk_send_msg_ack_context_v3
} err_tag_e;

/** 
 *  \details	err_tag==TAG_ACCESS，	err_code为ezdev_sdk_kernel_succ、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_buffer_too_small、\n
 *										ezdev_sdk_kernel_internal、ezdev_sdk_kernel_value_load、ezdev_sdk_kernel_value_save、ezdev_sdk_kernel_memory、\n
 *										NET_ERROR、LBS_ERROR、SECRETKEY_ERROR、DAS_ERROR
 *
 *	\details	err_tag==TAG_MSG_ACK，	err_code为ezdev_sdk_kernel_succ、ezdev_sdk_kernel_net_transmit、ezdev_sdk_kernel_extend_no_find、\n
 *										ezdev_sdk_kernel_force_domain_risk、ezdev_sdk_kernel_force_cmd_risk
 */
typedef struct
{
    err_tag_e err_tag;               ///< 错误码类型
    ezdev_sdk_kernel_error err_code; ///< 错误码
    EZDEV_SDK_PTR err_ctx;           ///< 错误码上线文
} sdk_runtime_err_context;

typedef struct
{
    EZDEV_SDK_UINT32 msg_domain_id;    ///< 领域模块ID
    EZDEV_SDK_UINT32 msg_command_id;   ///< 指令ID
    EZDEV_SDK_UINT32 msg_seq;          ///< 消息seq值
    enum QOS_T msg_qos;                ///< 消息QOS类型
    EZDEV_SDK_PTR externel_ctx;        ///< 外部自定义数据，由信令发送接口传进
    EZDEV_SDK_UINT32 externel_ctx_len; ///< 外部自定义数据长度
} sdk_send_msg_ack_context;


typedef struct
{
    enum QOS_T msg_qos;                ///< 消息QOS类型
    EZDEV_SDK_UINT16 extend_id;    ///< 领域模块ID
    EZDEV_SDK_UINT32 msg_seq;          ///< 消息seq值
    EZDEV_SDK_PTR externel_ctx;        ///< 外部自定义数据，由信令发送接口传进
    EZDEV_SDK_UINT32 externel_ctx_len; ///< 外部自定义数据长度
    char domain_id[ezdev_sdk_domain_id_len];         ///< 消息领域id
    char resource_id[ezdev_sdk_resource_id_len];     ///< 设备资源id
    char resource_type[ezdev_sdk_resource_type_len]; ///< 设备资源类型
    char business_type[ezdev_sdk_business_type_len]; ///< "report" / "query" / "set_reply" / "operate_reply"
    char identifier[ezdev_sdk_identifier_len];       ///< 业务标识
    char sub_serial[ezdev_sdk_max_serial_len];       ///< 子设备序列号,如果非hub模式, 这个参数不需要填写

} sdk_send_msg_ack_context_v3;

/**
 * 低功耗设备快速上线.
 */
typedef struct
{
    EZDEV_SDK_INT8 bLightreg;							 ///< 指定是否快速上线,0(否),1是wifi快速重连，2是RF快速重连
    EZDEV_SDK_UINT16 das_port;                           ///< das端口
    EZDEV_SDK_UINT16 das_udp_port;                       ///< das udp端口
    int das_socket;                                      ///< 上次下线保修的socket,可以不指定
    char das_address[ezdev_sdk_ip_max_len];              ///< das IP地址
    char das_domain[ezdev_sdk_ip_max_len];               ///< das 域名
    char das_serverid[ezdev_sdk_name_len];               ///< das serverid
    unsigned char session_key[ezdev_sdk_sessionkey_len]; ///< das session key
} kernel_das_info;

/**
 * \brief 领域注册信息
 */
typedef struct
{
    EZDEV_SDK_UINT32 domain_id;                                                                           ///< extendID 对应到业务领域
    void (*ezdev_sdk_kernel_extend_start)(EZDEV_SDK_PTR pUser);                                           ///< 微内核启动
    void (*ezdev_sdk_kernel_extend_stop)(EZDEV_SDK_PTR pUser);                                            ///< 微内核停止
    void (*ezdev_sdk_kernel_extend_data_route)(ezdev_sdk_kernel_submsg *ptr_submsg, EZDEV_SDK_PTR pUser); ///< 数据路由（领域回调函数）
    void (*ezdev_sdk_kernel_extend_event)(ezdev_sdk_kernel_event *ptr_event, EZDEV_SDK_PTR pUser);        ///< 事件回调（启动、停止、上线、下线等事件）
    EZDEV_SDK_PTR *pUser;                                                                                 ///< 用户指针（一般为NULL 也可以用于携带用户数据指针）
    char extend_module_name[ezdev_sdk_extend_name_len];                                                   ///< 模块名字
    char extend_module_version[version_max_len];                                                          ///< 模块版本号
} ezdev_sdk_kernel_extend;


/**
 * \brief 扩展模块注册信息
 */
typedef struct
{
    EZDEV_SDK_UINT16 extend_id;                                                                           ///< extend_id 对应到业务层属性/事件/操作/元数据等
    void (*ezdev_sdk_kernel_data_route)(ezdev_sdk_kernel_submsg_v3 *ptr_submsg, EZDEV_SDK_PTR pUser);     ///< 数据路由（按照extend_id路由）
    void (*ezdev_sdk_kernel_extend_event)(ezdev_sdk_kernel_event *ptr_event, EZDEV_SDK_PTR pUser);        ///< 事件回调（启动、停止、上线、下线等事件）
    EZDEV_SDK_PTR *pUser;                                                                                 ///< 用户指针（一般为NULL 也可以用于携带用户数据指针）
    char extend_module_name[ezdev_sdk_extend_name_len];                                                   ///< 模块名字
} ezdev_sdk_kernel_extend_v3;

/**
 * \brief 通用领域注册专用接口  领域ID是固定的
 */
typedef struct
{
	EZDEV_SDK_UINT32 domain_id; ///<	extendID 对应到业务领域
	EZDEV_SDK_INT8(*ezdev_sdk_kernel_common_module_data_handle)(ezdev_sdk_kernel_submsg *ptr_submsg, EZDEV_SDK_PTR pUser); ///<	回调是单线程出来的
	EZDEV_SDK_PTR pUser;
} ezdev_sdk_kernel_common_module;


/**
 * \brief 注册hub模块
 */
typedef struct 
{
    EZDEV_SDK_UINT16 extend_id;                                               
    void (*extend_hub_data_route)(ezdev_sdk_kernel_submsg_v3 *ptr_submsg); 
} ezdev_sdk_kernel_hub_extend;

/**
 * \brief 微内核内部使用的跨平台接口.
 */
typedef struct
{
    ezdev_sdk_net_work (*net_work_create)(char *nic_name);
    ezdev_sdk_kernel_error (*net_work_connect)(ezdev_sdk_net_work net_work, const char *server_ip, EZDEV_SDK_INT32 server_port, EZDEV_SDK_INT32 timeout_ms, char szRealIp[ezdev_sdk_ip_max_len]);
    ezdev_sdk_kernel_error (*net_work_read)(ezdev_sdk_net_work net_work, unsigned char *read_buf, EZDEV_SDK_INT32 read_buf_maxsize, EZDEV_SDK_INT32 read_timeout_ms);
    ezdev_sdk_kernel_error (*net_work_write)(ezdev_sdk_net_work net_work, unsigned char *write_buf, EZDEV_SDK_INT32 write_buf_size, EZDEV_SDK_INT32 write_timeout_ms, EZDEV_SDK_INT32 *real_write_buf_size);
    void (*net_work_disconnect)(ezdev_sdk_net_work net_work);
    void (*net_work_destroy)(ezdev_sdk_net_work net_work);
    int (*net_work_getsocket)(ezdev_sdk_net_work net_work);

	ezdev_sdk_time (*time_creator)(void);
	char (*time_isexpired_bydiff)(ezdev_sdk_time sdktime, EZDEV_SDK_UINT32 time_ms);
	char (*time_isexpired)(ezdev_sdk_time sdktime);
	void (*time_countdownms)(ezdev_sdk_time sdktime, EZDEV_SDK_UINT32 time_count);
	void (*time_countdown)(ezdev_sdk_time sdktime, EZDEV_SDK_UINT32 time_count);
	void (*time_destroy)(ezdev_sdk_time sdktime);
	EZDEV_SDK_UINT32 (*time_leftms)(ezdev_sdk_time sdktime);
    void (*time_sleep)(unsigned int time_ms);
	
	void (*key_value_load)(sdk_keyvalue_type valuetype, unsigned char* keyvalue, EZDEV_SDK_INT32 keyvalue_maxsize);						///<	读信息的函数，必须处理secretkey的读操作
	EZDEV_SDK_INT32 (*key_value_save)(sdk_keyvalue_type valuetype, unsigned char* keyvalue, EZDEV_SDK_INT32 keyvalue_size);				///<	写信息的函数，必须处理secretkey的写操作

	EZDEV_SDK_INT32 (*curing_data_load)(sdk_curingdata_type valuetype, unsigned char* keyvalue, EZDEV_SDK_INT32 *keyvalue_maxsize);		///<	读信息的函数，必须处理secretkey的读操作
	EZDEV_SDK_INT32 (*curing_data_save)(sdk_curingdata_type valuetype, unsigned char* keyvalue, EZDEV_SDK_INT32 keyvalue_size);			///<	写信息的函数，必须处理secretkey的写操作
	
	void (*sdk_kernel_log)(sdk_log_level level, EZDEV_SDK_INT32 sdk_error, EZDEV_SDK_INT32 othercode, const char * buf);
	
	ezdev_sdk_mutex (*thread_mutex_create)();
	void (*thread_mutex_destroy)(ezdev_sdk_mutex ptr_mutex);
	int (*thread_mutex_lock)(ezdev_sdk_mutex ptr_mutex);
	int (*thread_mutex_unlock)(ezdev_sdk_mutex ptr_mutex);

} ezdev_sdk_kernel_platform_handle;

/**
 * \}
 */

/* STUN信息 */
typedef struct
{
    EZDEV_SDK_UINT16 stun_interval;
    EZDEV_SDK_UINT16 stun1_port;
    EZDEV_SDK_UINT16 stun2_port;
    char stun1_address[ezdev_sdk_ip_max_len];
    char stun1_domain[ezdev_sdk_ip_max_len];
    char stun2_address[ezdev_sdk_ip_max_len];
    char stun2_domain[ezdev_sdk_ip_max_len];
} stun_info;

typedef struct
{
    char lbs_domain[ezdev_sdk_ip_max_len];      ///< lbs服务器域名
    char lbs_ip[ezdev_sdk_ip_max_len];          ///< lbs服务器IP
    EZDEV_SDK_UINT16 lbs_port;                  ///< lbs服务器的TCP端口号
    char das_domain[ezdev_sdk_ip_max_len];      ///< das服务器域名
    char das_ip[ezdev_sdk_ip_max_len];          ///< das服务器IP
    EZDEV_SDK_UINT16 das_port;                  ///< das服务器的TCP端口号
    EZDEV_SDK_UINT16 das_udp_port;              ///< das服务器UDP端口号
    int das_socket;                             ///< das的sock
    char session_key[ezdev_sdk_sessionkey_len]; ///< 会话秘钥
} server_info_s;

typedef struct
{
    unsigned char master_key[ezdev_sdk_masterkey_len + 1];
    unsigned char dev_id[ezdev_sdk_devid_len + 1];
    unsigned char dev_verification_code[ezdev_sdk_verify_code_maxlen + 1];
} showkey_info;

typedef struct
{
    int8_t *svr_name;
    uint16_t svr_port;
} ezdev_sdk_srv_info_t;

typedef struct
{
    int8_t auth_mode;
    int8_t dev_subserial[72];
    int8_t dev_verification_code[48];
    int8_t dev_serial[72];
    int8_t dev_firmwareversion[64];
    int8_t dev_type[64];
    int8_t dev_typedisplay[64];
    int8_t dev_mac[64];
    int8_t dev_nickname[64];
    int8_t dev_id[32];
    int8_t dev_master_key[16];
} ezdev_sdk_dev_info_t;

typedef void (*sdk_kernel_event_notice)(ezdev_sdk_kernel_event *ptr_event);
#endif //H_EZDEV_SDK_KERNEL_STRUCT_H_
