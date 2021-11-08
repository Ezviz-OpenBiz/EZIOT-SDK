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
 * 2017/3/4       panlong
 *******************************************************************************/


#ifndef H_MKERNEL_INTERNAL_ERROR_H_
#define H_MKERNEL_INTERNAL_ERROR_H_

typedef enum 
{
	mkernel_internal_succ = 0,							///<		成功
	mkernel_internal_no_start,							///<		SDK未启动
	mkernel_internal_haven_stop,						///<		SDK已经关闭，对于SDK来说可以进入卸载流程
	mkernel_internal_invald_call,						///<		您做了一个非法调用，请检查调用流程
	mkernel_internal_das_need_reconnect,				///<		DAS需要重连
	mkernel_internal_input_param_invalid,				///<		传入的参数非法
	mkernel_internal_lbs_connect_error,					///<		连接lbs错误
	mkernel_internal_mem_lack,							///<		配置的内存不够
	mkernel_internal_net_gethostbyname_error,			///<		域名解析失败
	mkernel_internal_create_sock_error,					///<		创建socket失败
	mkernel_internal_net_connect_error,					///<		socket 连接失败
	mkernel_internal_net_connect_timeout,				///<		socket 连接超时
	mkernel_internal_net_send_error,					///<		lbs 发送错误
	mkernel_internal_net_read_error,					///<		lbs 接收错误
	mkernel_internal_net_read_error_request,			///<		接收到错误的响应
	mkernel_internal_net_socket_error,					///<		socket error
	mkernel_internal_net_socket_timeout,				///<		socket timeout
	mkernel_internal_net_socket_closed,					///<		socket closed
	mkernel_internal_net_send_buf_full,					///<		send buf full
	mkernel_internal_malloc_error,						///<		申请内存错误
	mkernel_internal_json_parse_error,					///<		解析json错误
	mkernel_internal_get_error_json,					///<		错误json报文，指令与报文格式不一致
	mkernel_internal_json_format_error,					///<		json格式化失败
	mkernel_internal_xml_parse_error,					///<		解析xml错误
	mkernel_internal_get_error_xml,						///<		错误xml报文，指令与报文格式不一致
	mkernel_internal_rev_invalid_packet,				///<		接收到一个非法报文

	mkernel_internal_force_offline,						///<		设备被强制下线		接收到这个错误表示微内核内部已经停止工作了，需要外出完成调用关闭接口释放资源
	mkernel_internal_force_domain_risk,					///<		设备领域被风控掉了
	mkernel_internal_force_cmd_risk,					///<		设备指令被风控掉了

	mkernel_internal_net_poll_err,						///<		poll error
	mkernel_internal_net_getsockopt_error,				///<		getsockopt
	mkernel_internal_net_socket_err,					///<		SO_ERROR
	mkernel_internal_net_poll_event_err,				///<		poll_event_err
	mkernel_internal_value_load_err,					///<		获取数据失败
	mkernel_internal_value_save_err,					///<		保存数据失败
	mkernel_internal_internal_err,						///<		内部错误
	mkernel_internal_msg_len_overrange,                  ///<        消息长度超出范围

	mkernel_internal_call_mqtt_connect = 100,			///<		调用MQTT 注册
	mkernel_internal_call_mqtt_sub_error,				///<		调用MQTT 订阅topic
	mkernel_internal_call_mqtt_pub_error,				///<		调用MQTT 发布失败
	mkernel_internal_call_mqtt_yield_error,				///<		调用MQTT 驱动
	mkernel_internal_call_mqtt_disconnect,				///<		调用MQTT 注销
	mkernel_internal_call_mqtt_buffer_too_short,		///<		调用MQTT 分配空间太小

	mkernel_internal_extend_id_error = 200,				///<		扩展ID错误
	mkernel_internal_extend_unreg,						///<		扩展未注册
	mkernel_internal_extend_full,						///<		扩展注册已满
	mkernel_internal_extend_ready,						///<		扩展已经注册
	mkernel_internal_extend_no_find,					///<		扩展未找到
	mkernel_internal_common_ready,						///<		通用模块已经注册了（只支持注册一次）
	mkernel_internal_hub_extend_no_find,                ///<        未注册hub模块

	mkernel_internal_aes_input_len = 300,				///<		aes输入的数据长度不对
	mkernel_internal_casll_mbedtls_setdeckey_error,		///<		调用设置秘钥错误
	mkernel_internal_casll_mbedtls_crypt_error,			///<		调用加密接口错误
	mkernel_internal_aes_padding_unmatched,				///<		aes padding方式不匹配
	mkernel_internal_bscomptls_ecp_group_load_err,       ///< 
	mkernel_internal_bscomptls_ecdh_read_public_err,
	mkernel_internal_bscomptls_ecdh_calc_secret_err,
	

	mkernel_internal_queue_empty = 500,					///<		队列为空
	mkernel_internal_queue_uninit,						///<		队列未初始化
	mkernel_internal_queue_error,						///<		队列内部出现顺序错误
	mkernel_internal_queue_full,						///<		队列满

	mkernel_internal_platform_appoint_error =600,		///<		接收到与协议约定不一致
	mkernel_internal_sign_check_error,					///<		签名校验错误
	mkernel_internal_hmac_error,						///<		hmac签名失败
	mkernel_internal_dec_error,							///<		解密失败	
	mkernel_internal_enc_error,							///<		加密失败
	mkernel_internal_random1_check_error,				///<		随机码1校验错误
	mkernel_internal_hmac_compare_error,						///<		hmac签名失败

	mkernel_internal_platform_error = 10000,					///<	平台返回错误
	mkernel_internal_platform_lbs_signcheck_error,				///<	sign验证失败,验证码不匹配									
	mkernel_internal_platform_lbs_order_error,					///<	信令时序有问题
	mkernel_internal_platform_invalid_data,						///<	无效数据
	mkernel_internal_platform_devid_inconformity,				///<	设备上传的DEVID与平台记录不一致（可能是串号设备） 重新生成devid
	mkernel_internal_platform_query_authcode_error,				///<	查询验证码失败,没有这台设备
	mkernel_internal_platform_query_authcode_redis,				///<	查询redis失败
	mkernel_internal_platform_dec_error,						///<	解密失败
	mkernel_internal_platform_enc_error	,						///<	加密失败
	mkernel_internal_platform_getstun_error,				 	///<	获取das信息或stun信息失败
	mkernel_internal_platform_masterkey_invalid,				///<	10 maskey失效
	mkernel_internal_platform_stun_sessionkey_inconformity,		///<	获取Stun信息 设备端与服务端session key不一致
	mkernel_internal_platform_stun_process_invalid,				///<	获取Stun信息 流程不对
	mkernel_internal_platform_das_process_invalid,				///<	获取Das信息 流程不对

	mkernel_internal_platform_lbs_sign_decrypt_err,             ///<	HMac384签名校验错误
	mkernel_internal_platform_lbs_gen_keys_err,                 ///<    lbs生成秘钥错误
	mkernel_internal_platform_lbs_ecdh_check_err,               ///<    lbs ecdh
	mkernel_internal_platform_lbs_check_ecdh_sign_fail,
	mkernel_internal_platform_lbs_check_sessionkey_fail,        ///<   sessionkey验证失败
	mkernel_internal_platform_lbs_sign_check_fail,              ///<   auth_i校验sign失败、申请device id校验sign失败

	mkernel_internal_platform_secretkey_decrypt_fail = 10100,	///<	解密失败
	mkernel_internal_platform_secretkey_overflow_windows,		///<	请求不在窗口期
	mkernel_internal_platform_secretkey_no_user,				///<	设备未绑定用户
	mkernel_internal_platform_secretkey_serial_not_exist,		///<	设备不存在
	mkernel_internal_platform_secretkey_again,					///<	设备重复申请
	mkernel_internal_platform_error_end = 11000,

	mkernel_internal_mqtt_error_begin = 11000,
 	mkernel_internal_mqtt_nosupport_protocol_version,			///<	MQTT 连接已拒绝，不支持的协议版本
 	mkernel_internal_mqtt_unqualified_client_id,				///<	MQTT 连接已拒绝，不合格的客户端标识符
 	mkernel_internal_mqtt_server_unusable,						///<	MQTT 连接已拒绝，服务端不可用
 	mkernel_internal_mqtt_invalid_username,						///<	MQTT 连接已拒绝，无效的用户名或密码
 	mkernel_internal_mqtt_unauthorized,							///<	MQTT 连接已拒绝，未授权
 	mkernel_internal_mqtt_blacklist =	11010,					///<	设备被加入到黑名单
 	mkernel_internal_mqtt_redirect =	11011,					///<	设备会话失效(需要重定向)
	mkernel_internal_mqtt_session_exist =	11012,				///<	设备会话已存在
	mkernel_internal_mqtt_error_end = 12000,
}mkernel_internal_error;

#endif

