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
#ifndef H_DEV_PROTOCOL_DEF_H_ 
#define H_DEV_PROTOCOL_DEF_H_


#define DEV_ACCESS_DOMAIN_VERSION									"V2.3.2"	///<	当前接入领域的版本号


#define DEV_PROTOCOL_AUTHENTICATION_I          						0x1			///<	设备与平台认证_I,ECDH密钥交换请求
#define DEV_PROTOCOL_AUTHENTICATION_II          					0x2			///<	设备与平台认证_II,ECDH密钥交换响应
#define DEV_PROTOCOL_AUTHENTICATION_III          					0x3			///<	保留
#define DEV_PROTOCOL_REFRESHSESSIONKEY_REQ          		        0x4			///<	更新SessionKey请求 
#define DEV_PROTOCOL_AUTHENTICATION_V          					    0x5			///<	保留
#define DEV_PROTOCOL_REFRESHSESSIONKEY_RSP          		        0x6			///<	更新SessionKey响应
#define DEV_PROTOCOL_REQUEST_DEVID       					        0x7			///<	设备向平台请求设备ID
#define DEV_PROTOCOL_RESPONSE_DEVID           					    0x8			///<	平台向设备响应设备ID
#define DEV_PROTOCOL_APPLY_DEVID_CFM        				        0x9			///<	设备向平台确认设备ID
#define DEV_PROTOCOL_CRYPTO_DATA_REQ          						0xA			///<	获取Das(Stun) info requset 
#define DEV_PROTOCOL_CRYPTO_DATA_RSP          						0xB			///<	获取Das(Stun) info response
#define DEV_PROTOCOL_STUN_REFRESHSESSIONKEY_I         				0xC			///<	获取Stun info的sessionkey_I 
#define DEV_PROTOCOL_STUN_REFRESHSESSIONKEY_II          			0xD			///<	获取Stun info的sessionkey_II 
#define DEV_PROTOCOL_STUN_REFRESHSESSIONKEY_III          			0xE			///<	获取Stun info的sessionkey_III 
#define DEV_PROTOCOL_GET_SECRETKEY				          			0xF			///<	验证码不合规设备用来申请secretkey

#define DEV_PROTOCOL_LBS_FORM_VERSION							0x01		///<	与LBS交互协议形式版本号
#define DEV_PROTOCOL_LBS_LOW_TYPE_VERSION						0x01		///<	与LBS交互协议类型版本号
#define DEV_PROTOCOL_LBS_LOW_TYPE_VERSION_LICENSE				0x02		///<	与LBS交互协议类型版本号 --- 支持license解析的版本
#define DEV_PROTOCOL_LBS_HIGH_TYPE_VERSION						0x00		///<	与LBS交互协议类型版本号


#define DEV_PROTOCOL_LBS						0			///<	成功
#define DEV_PROTOCOL_LBS_SIGNCHECK_ERROR		1			///<	sign验证失败
#define DEV_PROTOCOL_LBS_ORDER_ERROR			2			///<	信令时序有问题
#define DEV_PROTOCOL_LBS_INVALID_DATA			3			///<	无效数据
#define DEV_PROTOCOL_LBS_DEVID_INCONFORMITY		4			///<	设备上传的DEVID与平台记录不一致（可能是串号设备）
#define DEV_PROTOCOL_LBS_QUERY_AUTHCODE_ERROR	5			///<	查询验证码失败
#define DEV_PROTOCOL_LBS_QUERY_AUTHCODE_REDIS	6			///<	查询redis失败
#define DEV_PROTOCOL_LBS_DEC_ERROR				7			///<	解密失败
#define DEV_PROTOCOL_LBS_ENC_ERROR				8			///<	加密失败
#define DEV_PROTOCOL_LBS_GETSTUN_ERROR			9			///<	获取das信息或stun信息失败


#define DEV_PROTOCOL_MQTT_SUCC								0			///<	MQTT 成功
#define DEV_PROTOCOL_MQTT_NOSUPPORT_PROTOCOL_VERSION		1			///<	MQTT 连接已拒绝，不支持的协议版本
#define DEV_PROTOCOL_MQTT_UNQUALIFIED_CLIENT_ID				2			///<	MQTT 连接已拒绝，不合格的客户端标识符
#define DEV_PROTOCOL_MQTT_SERVER_UNUSABLE					3			///<	MQTT 连接已拒绝，服务端不可用
#define DEV_PROTOCOL_MQTT_INVALID_USERNAME					4			///<	MQTT 连接已拒绝，无效的用户名或密码
#define DEV_PROTOCOL_MQTT_UNAUTHORIZED						5			///<	MQTT 连接已拒绝，未授权
#define DEV_PROTOCOL_MQTT_BLACKLIST							10			///<	设备被加入到黑名单
#define DEV_PROTOCOL_MQTT_REDIRECT							11			///<	设备会话失效(需要重定向)


#define	DAS_CMD_DOMAIN										1000			///<	设备接入
#define DAS_CMD_COMMON_FUN									1001			///<	通用功能

#define DAS_CMD_CENPLT2PUDOMAINCONFIG						0x0001		///<	平台下发接入领域系统配置
#define DAS_CMD_CENPLT2PURISKCONFIG							0x0002		///<	平台下发风控指令
#define DAS_CMD_CENPLT2PUOFFLINE							0x0003		///<	平台通知设备下线
// #define DAS_CMD_CENPLT2PUOFFLINEREQ							0x2805		///<	平台要求设备离线请求
// #define DAS_CMD_CENPLT2PUOFFLINERSP							0x2806		///<	平台要求设备离线响应
// #define	DAS_CMD_CENPLT2DASDEVICEKICKOUTREQ					0x301D		///<	平台向Das发送踢设备下线请求
// #define	DAS_CMD_CENPLT2DASDEVICEKICKOUTRSP					0x301E		///<	平台向Das发送踢设备下线响应
#define DAS_CMD_CENPLT2PUSETKEEPALIVETIMEREQ				0x3450		///<	平台向设备设置心跳间隔请求
#define DAS_CMD_CENPLT2PUSETKEEPALIVETIMERSP				0x3451		///<	平台向设备设置心跳间隔响应
#define DAS_CMD_CENPLT2PUSETLBSDOMAINNAMEBYDASREQ			0x3479		///<	平台向设备下发重定向请求
#define DAS_CMD_CENPLT2PUSETLBSDOMAINNAMEBYDASRSP			0x347A		///<	平台向设备下发重定向响应
#define DAS_CMD_CENPLT2PUSETLBSDOMAINNAMEREQ				0x491F		///<	平台向设备下发重定向请求
#define DAS_CMD_CENPLT2PUSETLBSDOMAINNAMERSP				0x4920		///<	平台向设备下发重定向响应


#define DAS_CMD_COMM_DOMAIN_PU2CENPLTUPDATESTATUS					0x0001		///<	通用状态上报
#define	DAS_CMD_COMM_DOMAIN_CENPLT2PUSETSTATUS						0x0002		///<	通用状态设置

#define	DAS_CMD_CENPLT2PUSETSWITCHENABLEREQ					0x490B		///<	平台向设备设置开关状态请求
#define DAS_CMD_CENPLT2PUSETSWITCHENABLERSP					0x490C		///<	平台向设备设置开关状态相应

#define	DAS_CMD_CENPLT2PUQUERYSTATUSREQ						0x3061		///<	平台向设备获取状态请求

#define	DAS_CMD_CENPLT2PUSETDEVPLANREQ						0x492B		///<	平台向设备设置计划请求
#define	DAS_CMD_CENPLT2PUSETDEVPLANRSP						0x492C		///<	平台向设备设置计划响应

#define DAS_CMD_CENPLT2PUSETCANARYTESTSTATUSREQ				0x4967		///<	平台向设备设置灰度发布状态请求
#define DAS_CMD_CENPLT2PUSETCANARYTESTSTATUSRSP				0x4968		///<	平台向设备设置灰度发布状态响应

#define DAS_CMD_PU2CENPLTUPGRADEREQ							0x2863		///<	设备向平台查询升级请求	
#define DAS_CMD_PU2CENPLTUPGRADERSP							0x2864		///<	设备向平台查询升级响应
#define DAS_CMD_PU2CENPLTSETKEEPALIVETIMEREQ                0X3452      ///<    设备向平台设置心跳间隔请求
#define DAS_CMD_PU2CENPLTSETKEEPALIVETIMERSP                0X3453      ///<    设备向平台设置心跳间隔响应

#define DEV_COMMON_CENPLT2PUSETKEYVALUEREQ					0x498D		///<	平台向设备设置键值类状态请求

#endif