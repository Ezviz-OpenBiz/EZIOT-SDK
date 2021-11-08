/*******************************************************************************
* Copyright 漏 2017-2021 Ezviz Inc.
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

/**
* @addtogroup micro_kernel 
*
* @section 微内核错误码信息
*
*  \sa ezdev_sdk_kernel_error
* 
*/

#ifndef H_EZDEV_SDK_KERNEL_ERROR_H_
#define H_EZDEV_SDK_KERNEL_ERROR_H_

#if defined (_WIN32) || defined(_WIN64)

#ifdef MICRO_KERNEL_EXPORTS
#define EZDEV_SDK_KERNEL_API	__declspec(dllexport)
#else
#define EZDEV_SDK_KERNEL_API __declspec(dllimport)
#endif

#define EZDEV_SDK_KERNEL_CALLBACK __stdcall
#else
#define EZDEV_SDK_KERNEL_API 

#define EZDEV_SDK_KERNEL_CALLBACK 
#endif

#define BASE_ERROR		0xE0000000
#define NET_ERROR		0x00000100
#define LBS_ERROR		0x00010000
#define SECRETKEY_ERROR	0x00010100
#define DAS_ERROR		0x00020000

typedef enum 
{
	ezdev_sdk_kernel_succ 						= 0,								///< 成功
	ezdev_sdk_kernel_internal					= BASE_ERROR+1,						///< 内部错误
	ezdev_sdk_kernel_invald_call				= BASE_ERROR+2,						///< 调用流程有误
	ezdev_sdk_kernel_params_invalid				= BASE_ERROR+3,						///< 参数非法
	ezdev_sdk_kernel_buffer_too_small			= BASE_ERROR+4,						///< 缓冲区大小不足
	ezdev_sdk_kernel_data_len_range				= BASE_ERROR+5,						///< 数据大小超出范围
	ezdev_sdk_kernel_memory						= BASE_ERROR+6,						///< 内存异常
	ezdev_sdk_kernel_json_invalid				= BASE_ERROR+7,						///< 非法的json数据
	ezdev_sdk_kernel_json_format				= BASE_ERROR+8,						///< json数据有误
	ezdev_sdk_kernel_extend_no_find				= BASE_ERROR+9,						///< 信令路由找不到对应的注册领域
	ezdev_sdk_kernel_extend_full				= BASE_ERROR+10,					///< 扩展注册已满
	ezdev_sdk_kernel_extend_existed				= BASE_ERROR+11,					///< 扩展已经注册
	ezdev_sdk_kernel_queue_full					= BASE_ERROR+12,					///< 消息队列已满
	ezdev_sdk_kernel_value_load					= BASE_ERROR+13,					///< 获取设备数据失败
	ezdev_sdk_kernel_value_save					= BASE_ERROR+14,					///< 保存数据至设备失败
    ezdev_sdk_kernel_msg_stop_distribute	    = BASE_ERROR+15,					///< 设备正在停止,上层消息停止下发

	ezdev_sdk_kernel_net_create					= (BASE_ERROR+NET_ERROR)+1,			///< 创建socket失败
	ezdev_sdk_kernel_net_connect				= (BASE_ERROR+NET_ERROR)+2,			///< 网络连接失败
	ezdev_sdk_kernel_net_disconnected			= (BASE_ERROR+NET_ERROR)+3,			///< 网络连接断开
	ezdev_sdk_kernel_net_transmit				= (BASE_ERROR+NET_ERROR)+4,			///< 数据传输失败
	ezdev_sdk_kernel_net_dns					= (BASE_ERROR+NET_ERROR)+5,			///< 域名解析失败

	ezdev_sdk_kernel_lbs_authcode_mismatch		= (BASE_ERROR+LBS_ERROR)+1,			///< 验证码不一致
	ezdev_sdk_kernel_lbs_invalid_call			= (BASE_ERROR+LBS_ERROR)+2,			///< 和服务器的交互流程有问题
	ezdev_sdk_kernel_lbs_invalid_data			= (BASE_ERROR+LBS_ERROR)+3,			///< 服务器收到错误数据
	ezdev_sdk_kernel_lbs_devid_mismatch			= (BASE_ERROR+LBS_ERROR)+4,			///< devid不一致
	ezdev_sdk_kernel_lbs_masterkey_mismatch		= (BASE_ERROR+LBS_ERROR)+5,			///< masterkey不一致
	ezdev_sdk_kernel_lbs_sessionkey_mismatch	= (BASE_ERROR+LBS_ERROR)+6,			///< sessionkey不一致
	ezdev_sdk_kernel_lbs_invalid_dev			= (BASE_ERROR+LBS_ERROR)+7,			///< 无效设备
	ezdev_sdk_kernel_lbs_server_crypto			= (BASE_ERROR+LBS_ERROR)+8,			///< 服务器加解密失败
	ezdev_sdk_kernel_lbs_get_data				= (BASE_ERROR+LBS_ERROR)+9,			///< 获取das或者stun信息失败
	ezdev_sdk_kernel_lbs_server_exception		= (BASE_ERROR+LBS_ERROR)+10,		///< 服务器内部异常

	ezdev_sdk_kernel_secretkey_decrypt_fail 	= (BASE_ERROR+SECRETKEY_ERROR)+0,	///< 申请secretkey报文平台解密失败
	ezdev_sdk_kernel_secretkey_overflow_windows = (BASE_ERROR+SECRETKEY_ERROR)+1,	///< 申请secretkey请求不在窗口期
	ezdev_sdk_kernel_secretkey_no_user			= (BASE_ERROR+SECRETKEY_ERROR)+2,	///< 设备未绑定用户
	ezdev_sdk_kernel_secretkey_sn_not_exist		= (BASE_ERROR+SECRETKEY_ERROR)+3,	///< 设备不存在
	ezdev_sdk_kernel_secretkey_again			= (BASE_ERROR+SECRETKEY_ERROR)+4,	///< 设备重复申请

 	ezdev_sdk_kernel_das_nosupport_protocol_ver	= (BASE_ERROR+DAS_ERROR)+1,			///< 不支持的协议版本
 	ezdev_sdk_kernel_das_client_id_invalid		= (BASE_ERROR+DAS_ERROR)+2,			///< 不合格的客户端标识符
 	ezdev_sdk_kernel_das_server_unusable		= (BASE_ERROR+DAS_ERROR)+3,			///< 服务端不可用(服务器内部异常)
 	ezdev_sdk_kernel_das_invalid_username		= (BASE_ERROR+DAS_ERROR)+4,			///< 无效的用户名或密码（现阶段暂不使用）
 	ezdev_sdk_kernel_das_unauthorized			= (BASE_ERROR+DAS_ERROR)+5,			///< 未授权（现阶段暂不使用）
	ezdev_sdk_kernel_das_session_invaild		= (BASE_ERROR+DAS_ERROR)+6,			///< 接入会话失效
	ezdev_sdk_kernel_das_force_offline			= (BASE_ERROR+DAS_ERROR)+7,			///< 设备被强制下线
 	ezdev_sdk_kernel_das_force_dev_risk			= (BASE_ERROR+DAS_ERROR)+8,			///< 设备被风控（黑名单）
	ezdev_sdk_kernel_das_force_domain_risk		= (BASE_ERROR+DAS_ERROR)+9,			///< 领域被风控
	ezdev_sdk_kernel_das_force_cmd_risk			= (BASE_ERROR+DAS_ERROR)+10,		///< 指令被风控
}ezdev_sdk_kernel_error;

#endif

