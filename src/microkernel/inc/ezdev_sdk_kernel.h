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

#ifndef H_EZDEV_SDK_KERNEL_H_
#define H_EZDEV_SDK_KERNEL_H_

#include "base_typedef.h"
#include "ezdev_sdk_kernel_error.h"
#include "ezdev_sdk_kernel_struct.h"

/**
 * @addtogroup micro_kernel 微内核模块
 * 微内核是提供给各个领域模块接入的核心模块，内部主要包括：设备接入平台、领域模块管理、设备风控等功能
 *  \{
 */

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 *  \brief		微内核初始化接口（非线程安全）
 *  \method		ezdev_sdk_kernel_init
 *	\note		萤石设备接入SDK 微内核初始化接口，只支持单设备模式，此接口在ezDevSDK_boot模块中调用，各个领域模块无需关心
 *  \param[in] 	server_name						服务地址 支持域名
 *  \param[in] 	server_port						服务监听的端口
 *  \param[in] 	kernel_platform_handle			跨平台接口实现，这里面的接口在ezDevSDK_boot里必须要实现，内部会做参数检测
 *  \param[in] 	kernel_event_notice_cb			微内核内部消息回调
 *  \param[in] 	dev_config_info					配置信息通过json串的形式,以下分别为SAP和license模式认证需要的json数据格式
 *	\details
 *				{
 *				"dev_auth_mode":0,										选填,默认0;SAP认证模式
 *				"dev_access_mode":0										选填,默认0;设备接入模式  0-普通（2.0）   1-HUB（2.0）
 *				"dev_status":1,											必填;设备工作状态 1：正常工作模式  5：待机(或睡眠)工作模式
 *				"dev_subserial":"411444968",							必填;设备短序列号(最大16)
 *				"dev_verification_code":"ABCDEF",						必填;设备验证码---严格不能改变，变更会导致设备无法上线(最大16)
 *				"dev_serial":"DS-2CD8464F-EI0120120923CCRR411444968",	必填;设备长序列号(最大64)
 *				"dev_firmwareversion":"V2.2.0 build150205",				必填;设备固件版本号(最大64)						
 *				"dev_type":"DS-2CD8464F-EI",							必填;设备型号(最大64)						
 *				"dev_typedisplay":"DS-2CD8464F-EI",						必填;设备显示型号(最大64)
 *				"dev_mac":"004048C5E1B8",								必填;设备网上物理地址(最大64)
 *				"dev_nickname":"C1(411444968)",							必填;设备昵称(最大64)	
 *				"dev_firmwareidentificationcode":"00000001000",			必填;设备固件识别码(最大256)
 *				"dev_oeminfo",											必填;设备OEM信息
 *				}
 *	\details
 *				{
 *				"dev_auth_mode":1,										选填,默认0;License认证模式
 *				"dev_access_mode":0										选填,默认0;设备接入模式  0-普通（2.0）   1-HUB（2.0）
 *				"dev_productKey":"HIK_PARKING_PLANTFORM",				必填;通过license申请接口申请出来：productKey
 *				"dev_deviceName":"HIKPP12345",							必填;通过license申请接口申请出来：dev_deviceName
 *				"dev_deviceLicense":"Lm9HhDdtvqWXR2F52or6p3",			必填;通过license申请接口申请出来：dev_deviceLicense
 *				"dev_mac":"004048C5E1B8",								必填;设备网上物理地址(最大64)
 *				"dev_nickname":"C1(411444968)",							必填;设备昵称(最大64)	
 *				"dev_firmwareversion":"V5.1.3 build 170712",			必填;设备软件版本号
 *				}
 *	\param[in]	reg_das_info					走快速注册上线模式流程需要的信息
 *  \param[in]  reg_mode						注册模式
 *  \details
 *	1---正常设备(平台默认值)
 *	2---wifi托管低功耗设备(现已有,表示电池设备当前托管状态)
 *	3---RF托管低功耗设备(本次新增, 表示电池设备当前托管状态)
 *	4---RF管理(本次新增, 表示支持RF托管,由Base设备上报)
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_json_invalid、ezdev_sdk_kernel_json_format、 \n
 *				ezdev_sdk_kernel_value_load、ezdev_sdk_kernel_invald_call
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_init(const ezdev_sdk_srv_info_t *psrv_info, const ezdev_sdk_dev_info_t *pdev_info,
																  const ezdev_sdk_kernel_platform_handle* kernel_platform_handle,
																  sdk_kernel_event_notice kernel_event_notice_cb);

/**
 *  \brief		微内核反始化接口（非线程安全）
 *  \method		ezdev_sdk_kernel_fini
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_fini();

/** 
 *  \brief		微内核领域模块加载接口（非线程安全）
 *  \method		ezdev_sdk_kernel_extend_load
 *  \param[in] 	external_extend		领域模块信息
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_extend_existed、\n
 *				ezdev_sdk_kernel_extend_full
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_extend_load(const ezdev_sdk_kernel_extend* external_extend);

/** 
 *  \brief		扩展模块加载接口（非线程安全）
 *  \method		ezdev_sdk_kernel_extend_load_v3
 *  \param[in] 	external_extend		扩展模块信息
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_extend_existed、\n
 *				ezdev_sdk_kernel_extend_full
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_extend_load_v3(const ezdev_sdk_kernel_extend_v3* extend_info);

/** 
 *  \brief		hub模块加载接口（非线程安全）
 *  \method		ezdev_sdk_kernel_hub_extend_load
 *  \param[in] 	hub_extend	领域模块信息
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_extend_existed、\n
 *				ezdev_sdk_kernel_extend_full
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_hub_extend_load(const ezdev_sdk_kernel_hub_extend* hub_extend);

/** 
 *  \brief		微内核通用领域加载接口（非线程安全）
 *	\note		单独为通用领域模块设置的接口，在SDKboot模块中调用，上层领域和应用不需要关心。如果不设置的话，则所有通用领域的指令都会被过滤掉
 *  \method		ezdev_sdk_kernel_common_module_load
 *  \param[in] 	common_module	通用领域模块信息
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_params_invalid
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_common_module_load(const ezdev_sdk_kernel_common_module* common_module);

/** 
 *  \brief		微内核启动接口（非线程安全）
 *	\note		如果微内核已经启动，重复调用会返错
 *  \method		ezdev_sdk_kernel_start
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_start();

/** 
 *  \brief		微内核停止接口（非线程安全）
 *  \method		ezdev_sdk_kernel_stop
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_stop();

/** 
 *  \brief		微内核内部业务驱动接口，通过外部线程驱动接口，内部执行业务
 *  \method		ezdev_sdk_kernel_yield
 *	\note		阻塞式调用
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_buffer_too_small、\n
 *				ezdev_sdk_kernel_internal、ezdev_sdk_kernel_value_load、ezdev_sdk_kernel_value_save、ezdev_sdk_kernel_memory、NET_ERROR、\n
 *				LBS_ERROR、SECRETKEY_ERROR、DAS_ERROR
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_yield();

/** 
 *  \brief		微内核用户业务驱动接口，通过外部线程驱动接口，用于消息分发到上层领域和应用
 *  \method		ezdev_sdk_kernel_yield_user
 *	\note		从消息队列取出消息，通过回调的方式分发到上层领域和应用
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_extend_no_find
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_yield_user();

/** 
 *  \brief		微内核数据发送接口（线程安全）
 *  \method		ezdev_sdk_kernel_send
 *	\note		只负责发消息，不管消息内容
 *				非阻塞式调用
 *  \param[in] 	pubmsg	消息内容
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_data_len_range、\n
 *				ezdev_sdk_kernel_memory、ezdev_sdk_kernel_queue_full、ezdev_sdk_kernel_extend_no_find、ezdev_sdk_kernel_force_domain_risk、\n
 *				ezdev_sdk_kernel_force_cmd_risk
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_send(ezdev_sdk_kernel_pubmsg* pubmsg);


/** 
 *  \brief		微内核数据发送接口 v3协议（线程安全）
 *  \method		ezdev_sdk_kernel_send
 *	\note		只负责发消息，不管消息内容
 *				非阻塞式调用
 *  \param[in] 	pubmsg	消息内容
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_data_len_range、\n
 *				ezdev_sdk_kernel_memory、ezdev_sdk_kernel_queue_full、ezdev_sdk_kernel_extend_no_find、ezdev_sdk_kernel_force_domain_risk、\n
 *				ezdev_sdk_kernel_force_cmd_risk
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_send_v3(ezdev_sdk_kernel_pubmsg_v3* pubmsg_v3);

/** 
 *  \brief		设置socket参数
 *  \method		ezdev_sdk_kernel_set_net_option
 *	\note		设置socket参数，需要在ezdev_sdk_kernel_start前调用
 *  \param[in] 	optname 操作类型, 1 绑定到某张网卡 3 设备接入链路断开重连
 * 	\param[in] 	optval 操作参数
 *  \param[in] 	optlen 操作参数长度
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_buffer_too_small
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_set_net_option(int optname, const void* optval, int optlen);

/** 
 *  \brief		萤石设备接入SDK 微内核 设备信息获取接口
 *  \method		ezdev_sdk_kernel_getdevinfo_bykey
 *  \param[in] 	key	查找键值
 *  \return 	成功返回value值指针 失败返回"invalidkey"
 */
EZDEV_SDK_KERNEL_API const char* ezdev_sdk_kernel_getdevinfo_bykey(const char* key);

/** 
 *  \brief			获取微内核的版本号，二次调用
 *  \method			ezdev_sdk_kernel_get_sdk_version
 *  \param[out]		pbuf 微内核版本
 *  \param[inout] 	pbuflen 如果pbuf为空，返回待拷贝数据长度，否则返回真实拷贝长度
 *  \return			ezdev_sdk_kernel_succ、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_buffer_too_small
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_get_sdk_version(char* pbuf, int* pbuflen);

/** 
 *  \brief			获取LBS、DAS服务器信息接口，二次调用
 *  \method			ezdev_sdk_kernel_get_server_info
 *  \param[out]		ptr_server_info 服务器信息数组
 *  \param[inout] 	ptr_count 如果ptr_server_info为空，返回待拷贝数据的数量，否则返回真实拷贝数量
 *  \return			ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_buffer_too_small
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_get_server_info(server_info_s* ptr_server_info, int *ptr_count);
/** 
 *  \brief			show_key功能接口
 *  \method			ezdev_sdk_kernel_show_key_info
 *  \param[out]		show_key信息数组
 *  \return			ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_buffer_too_small
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_show_key_info(showkey_info* ptr_showkey_info);

#ifdef __cplusplus
}
#endif

/**
 * \}
 */
#endif //H_EZDEV_SDK_KERNEL_H_