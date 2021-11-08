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
#include "ezdev_sdk_kernel_access.h"
#include "mkernel_internal_error.h"
#include "sdk_kernel_def.h"
#include "lbs_transport.h"
#include "das_transport.h"
#include "ezdev_sdk_kernel_risk_control.h"
#include "ezdev_sdk_kernel_event.h"
#include "ezdev_sdk_kernel.h"
#include "utils.h"
#include "ezxml.h"
#include "ase_support.h"
#include "ezconfig.h"
#include "mcuconfig.h"

LBS_TRANSPORT_INTERFACE
DAS_TRANSPORT_INTERFACE
EZDEV_SDK_KERNEL_RISK_CONTROL_INTERFACE
EZDEV_SDK_KERNEL_EVENT_INTERFACE
ASE_SUPPORT_INTERFACE

extern ezdev_sdk_kernel g_ezdev_sdk_kernel;

static mkernel_internal_error cnt_state_lbs_redirect(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT8 nUpper)
{
	/**
	* \brief   启动SDK 存在 几个情况 dev_id 是否存在   masterkey是否存在
	*			完成上线
		微内核上线前需要到LBS进行认证，首次上线走“四步认证”，生成masterkey、device_id、sessionkey。
		后续上线走“三步认证”，生成sessionkey。sessionkey是微内核和DAS之间的会话密钥，
只在会话期间才有效，当微内核获取新的会话密钥后，老的会话密钥失效。

	
	*             
	*/
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	int type = 0;
	if (strcmp("", (const char*)sdk_kernel->dev_id) == 0)
	{
		/**
		* \brief   走认证上线流程 需要创建das_id  masterkey
		*             
		*/
		sdk_error = lbs_redirect_createdevid_with_auth(sdk_kernel, nUpper);
		type = 0;
	}
	else
	{
		if (strcmp("", (const char*)sdk_kernel->master_key) == 0)
		{
			/**
			* \brief   走认证上线流程 需要创建masterkey
			*             
			*/
			sdk_error = lbs_redirect_with_auth(sdk_kernel, nUpper);
			type = 1;
		}
		else
		{
			/**
			* \brief   刷新sesseion key
			*             
			*/
			sdk_error = lbs_redirect(sdk_kernel);
			type = 2;
		}
	}
	
	ezdev_sdk_kernel_log_info(sdk_error, 0, "cnt_state_lbs_redirect result:%d, type:%d", sdk_error, type);

	if (sdk_error == mkernel_internal_platform_devid_inconformity)
	{
		memset(sdk_kernel->dev_id, 0, ezdev_sdk_devid_len);
	}

	if (sdk_error == mkernel_internal_platform_masterkey_invalid)
	{
		memset(sdk_kernel->master_key, 0, ezdev_sdk_masterkey_len);
	}

	/**
	 * 如果验证出错，可能是由于摘要计算大小写原因（历史原因），先尝试一下摘要小写的情况，
	 */
	if (sdk_error == mkernel_internal_platform_lbs_signcheck_error && nUpper != 0)
	{
		sdk_error = cnt_state_lbs_redirect(sdk_kernel, 0);
		ezdev_sdk_kernel_log_info(sdk_error, 0, "lbs return :%d, and nUpper != 0", sdk_error);
	}
	
	// test
// 	sdk_error = ezdev_sdk_kernel_succ;
// 	memcpy(sdk_kernel->master_key, "1234567890123456", ezdev_sdk_masterkey_len);
// 	memcpy(sdk_kernel->dev_id, "12345678901234561234567890123456", ezdev_sdk_devid_len);
// 	memcpy(sdk_kernel->session_key, "1234567890123456", ezdev_sdk_sessionkey_len);
// 
// 	memcpy(sdk_kernel->redirect_das_info.das_address,"10.82.3.19", 10);
// 	sdk_kernel->redirect_das_info.das_port = 8883;

	return sdk_error;
}

static mkernel_internal_error cnt_state_das_reged(ezdev_sdk_kernel* sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	das_unreg(sdk_kernel);
	sdk_error = das_reg(sdk_kernel);
	return sdk_error;
}

static mkernel_internal_error cnt_state_das_fast_reg(ezdev_sdk_kernel* sdk_kernel)
{
	/**
	 * \brief   sdk wifi快速重连过程
	 */
	return das_light_reg_v2(sdk_kernel);
}

static mkernel_internal_error cnt_state_das_fast_reg_v3(ezdev_sdk_kernel* sdk_kernel)
{
	/**
	 * \brief   sdk RF快速重连过程
	 */
	return das_light_reg_v3(sdk_kernel);
}

static mkernel_internal_error cnt_state_das_work(ezdev_sdk_kernel* sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	sdk_error = das_yield(sdk_kernel);
	return sdk_error;
}

static mkernel_internal_error cnt_state_das_retry(ezdev_sdk_kernel* sdk_kernel)
{
	/**
	 * \brief   sdk重连过程
	 */
	return das_light_reg(sdk_kernel);
}

static mkernel_internal_error cnt_lbs_redirect_do(ezdev_sdk_kernel* sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	if (sdk_entrance_authcode_invalid == sdk_kernel->entr_state)
	{
		//如果是因为申请secretkey失败，需要根据服务器配置的时间间隔和总时长来进行重试
		if (!sdk_kernel->platform_handle.time_isexpired_bydiff(sdk_kernel->cnt_state_timer, sdk_kernel->secretkey_interval*1000) || 
			sdk_kernel->lbs_redirect_times > sdk_kernel->secretkey_duration)
		{
			return sdk_error;
		}
	}
	else
	{
		if (sdk_kernel->lbs_redirect_times && !sdk_kernel->platform_handle.time_isexpired_bydiff(sdk_kernel->cnt_state_timer, sdk_kernel->lbs_redirect_times*2000))
		{
			return sdk_error;
		}
	}

	sdk_kernel->platform_handle.time_countdown(sdk_kernel->cnt_state_timer, 0);
	ezdev_sdk_kernel_log_trace(0, 0, "lbs retry, times:%d ", sdk_kernel->lbs_redirect_times);
	sdk_error = cnt_state_lbs_redirect(sdk_kernel, 1);

	if (sdk_error == mkernel_internal_succ)
	{
		sdk_kernel->cnt_state = sdk_cnt_redirected;
		sdk_kernel->lbs_redirect_times = 0;
		sdk_kernel->das_retry_times = 0;
	}
	else
	{
		broadcast_runtime_err(TAG_ACCESS, mkiE2ezE(sdk_error), NULL, 0);

		if (mkernel_internal_net_connect_error == sdk_error||mkernel_internal_net_gethostbyname_error == sdk_error)  ///<	如果是连接出错的话则不做衰变
		{
			sdk_kernel->lbs_redirect_times = 1;
		}
		else
		{
			/* 重定向失败 计数 */
			if (++sdk_kernel->lbs_redirect_times >= 60)
				sdk_kernel->lbs_redirect_times = 60;
		}
	}

	return sdk_error;
}

static mkernel_internal_error cnt_das_reg_do(ezdev_sdk_kernel* sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	if (sdk_kernel->das_retry_times && !sdk_kernel->platform_handle.time_isexpired_bydiff(sdk_kernel->cnt_state_timer, sdk_kernel->das_retry_times*2000))
	{
		return sdk_error;
	}
	
	sdk_kernel->platform_handle.time_countdown(sdk_kernel->cnt_state_timer, 0);
	ezdev_sdk_kernel_log_trace(0, 0, "cnt_state_das_reged, times:%d ", sdk_kernel->das_retry_times);

	sdk_error = cnt_state_das_reged(sdk_kernel);
	if(sdk_error == mkernel_internal_mqtt_session_exist)
	{
		/* 服务端会话还存在，走快速重连 */
		sdk_error = cnt_state_das_fast_reg(sdk_kernel);
	}

	if (sdk_error == mkernel_internal_succ)
	{
		sdk_kernel->cnt_state = sdk_cnt_das_reged;
		sdk_kernel->lbs_redirect_times = 0;
		sdk_kernel->das_retry_times = 0;
		if (sdk_kernel->entr_state == sdk_entrance_switchover)
		{
			sdk_switchover_context context = {0};
			context.das_udp_port = sdk_kernel->redirect_das_info.das_udp_port;
			memcpy(context.das_ip, sdk_kernel->redirect_das_info.das_address, ezdev_sdk_ip_max_len);
			memcpy(context.lbs_ip, sdk_kernel->server_info.server_ip, ezdev_sdk_ip_max_len);
			memcpy(context.session_key, sdk_kernel->session_key, ezdev_sdk_sessionkey_len);
			memcpy(context.lbs_domain, sdk_kernel->server_info.server_name, ezdev_sdk_ip_max_len);
			ezdev_sdk_kernel_log_error(sdk_error, 0, "sdk_kernel_event_switchover");
			broadcast_user_event(sdk_kernel_event_switchover, (void*)&context, sizeof(context));
			sdk_kernel->entr_state = sdk_entrance_normal;
		}
		else
		{
			sdk_sessionkey_context context = {0};
			context.das_udp_port = sdk_kernel->redirect_das_info.das_udp_port;
			context.das_port = sdk_kernel->redirect_das_info.das_port;
			memcpy(context.das_ip, sdk_kernel->redirect_das_info.das_address, ezdev_sdk_ip_max_len);
			memcpy(context.lbs_ip, sdk_kernel->server_info.server_ip, ezdev_sdk_ip_max_len);
			memcpy(context.session_key, sdk_kernel->session_key, ezdev_sdk_sessionkey_len);
			memcpy(context.das_domain, sdk_kernel->redirect_das_info.das_domain, ezdev_sdk_ip_max_len);
			memcpy(context.das_serverid, sdk_kernel->redirect_das_info.das_serverid, ezdev_sdk_ip_max_len);
			ezdev_sdk_kernel_log_warn(sdk_error, 0, "sdk_kernel_event_online");
			broadcast_user_event(sdk_kernel_event_online, (void*)&context, sizeof(context));
		}
	}
	else
	{
		ezdev_sdk_kernel_log_error(sdk_error, 0, "cnt_state_das_reged");
		broadcast_runtime_err(TAG_ACCESS, mkiE2ezE(sdk_error), NULL, 0);
		if (sdk_kernel->das_retry_times++ >= 5)
		{
			sdk_kernel->cnt_state = sdk_cnt_unredirect;
			sdk_kernel->lbs_redirect_times = 0;
			sdk_kernel->das_retry_times = 0;
		}
	}

	return sdk_error;
}

static mkernel_internal_error cnt_das_work_do(ezdev_sdk_kernel* sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	sdk_error = cnt_state_das_work(sdk_kernel);
	if (mkernel_internal_succ != sdk_error &&
		mkernel_internal_queue_empty != sdk_error)
	{
		ezdev_sdk_kernel_log_error(sdk_error, 0, "cnt_das_work_do");
		broadcast_runtime_err(TAG_ACCESS, mkiE2ezE(sdk_error), NULL, 0);

		if (mkernel_internal_das_need_reconnect == sdk_error )
		{
			sdk_kernel->cnt_state = sdk_cnt_das_break;
		}
	}

	return sdk_error;
}

static mkernel_internal_error cnt_das_retry_do(ezdev_sdk_kernel* sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	if (sdk_kernel->das_retry_times && !sdk_kernel->platform_handle.time_isexpired_bydiff(sdk_kernel->cnt_state_timer, sdk_kernel->das_retry_times*1000))
	{
		return sdk_error;
	}

	sdk_kernel->platform_handle.time_countdown(sdk_kernel->cnt_state_timer, 0);
	ezdev_sdk_kernel_log_trace(0, 0, "das retry, times:%d ", sdk_kernel->das_retry_times);

	sdk_error = cnt_state_das_retry(sdk_kernel);
	if (sdk_error == mkernel_internal_succ)
	{
		sdk_kernel->cnt_state = sdk_cnt_das_reged;
		sdk_kernel->lbs_redirect_times = 0;
		sdk_kernel->das_retry_times = 0;
		broadcast_user_event_reconnect_success();
	}
	else
	{
		broadcast_runtime_err(TAG_ACCESS, mkiE2ezE(sdk_error), NULL, 0);
		if (sdk_kernel->das_retry_times++ >= 3)
		{
			sdk_offline_context context = {0};

			context.last_error = sdk_error;
			memcpy(context.das_ip, sdk_kernel->redirect_das_info.das_address, ezdev_sdk_ip_max_len);
			memcpy(context.lbs_ip, sdk_kernel->server_info.server_ip, ezdev_sdk_ip_max_len);
			ezdev_sdk_kernel_log_error(sdk_error, 0, "sdk_kernel_event_break");
			broadcast_user_event(sdk_kernel_event_break, (void*)&context, sizeof(context));
			
			sdk_kernel->cnt_state = sdk_cnt_unredirect;
			sdk_kernel->lbs_redirect_times = 0;
			sdk_kernel->das_retry_times = 0;
		}
	}

	return sdk_error;
}

static mkernel_internal_error cnt_das_reg_fast_do(ezdev_sdk_kernel* sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	int iRetry_fastReg= 0;
	sdk_sessionkey_context context = {0};

	for (iRetry_fastReg = 0; iRetry_fastReg < 3; iRetry_fastReg++)//尝试3次
	{
		sdk_error = cnt_state_das_fast_reg(sdk_kernel);
		if (sdk_error == mkernel_internal_succ)
		{
			break;
		}
	}

	if (sdk_error == mkernel_internal_succ)
	{
		sdk_kernel->cnt_state = sdk_cnt_das_reged;
		sdk_kernel->lbs_redirect_times = 0;
		sdk_kernel->das_retry_times = 0;

		context.das_udp_port = sdk_kernel->redirect_das_info.das_udp_port;
		context.das_port = sdk_kernel->redirect_das_info.das_port;
		memcpy(context.das_ip, sdk_kernel->redirect_das_info.das_address, ezdev_sdk_ip_max_len);
		memcpy(context.lbs_ip, sdk_kernel->server_info.server_ip, ezdev_sdk_ip_max_len);
		memcpy(context.session_key, sdk_kernel->session_key, ezdev_sdk_sessionkey_len);
		memcpy(context.das_domain, sdk_kernel->redirect_das_info.das_domain, ezdev_sdk_ip_max_len);
		memcpy(context.das_serverid, sdk_kernel->redirect_das_info.das_serverid, ezdev_sdk_ip_max_len);
		broadcast_user_event(sdk_kernel_event_fast_reg_online, (void*)&context, sizeof(context));
	}
	else
	{
		broadcast_runtime_err(TAG_ACCESS, mkiE2ezE(sdk_error), NULL, 0);
		sdk_kernel->cnt_state = sdk_cnt_unredirect;
		sdk_kernel->lbs_redirect_times = 0;
		sdk_kernel->das_retry_times = 0;
	}

	return sdk_error;
}

static mkernel_internal_error cnt_das_reg_v3_fast_do(ezdev_sdk_kernel* sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	int iRetry_fastReg= 0;
	sdk_sessionkey_context context = {0};

	for (iRetry_fastReg = 0; iRetry_fastReg < 3; iRetry_fastReg++)//尝试3次
	{
		sdk_error = cnt_state_das_fast_reg_v3(sdk_kernel);
		if (sdk_error == mkernel_internal_succ)
		{
			break;
		}
	}

	if (sdk_error == mkernel_internal_succ)
	{
		sdk_kernel->cnt_state = sdk_cnt_das_reged;
		sdk_kernel->lbs_redirect_times = 0;
		sdk_kernel->das_retry_times = 0;

		context.das_udp_port = sdk_kernel->redirect_das_info.das_udp_port;
		context.das_port = sdk_kernel->redirect_das_info.das_port;
		memcpy(context.das_ip, sdk_kernel->redirect_das_info.das_address, ezdev_sdk_ip_max_len);
		memcpy(context.lbs_ip, sdk_kernel->server_info.server_ip, ezdev_sdk_ip_max_len);
		memcpy(context.session_key, sdk_kernel->session_key, ezdev_sdk_sessionkey_len);
		memcpy(context.das_domain, sdk_kernel->redirect_das_info.das_domain, ezdev_sdk_ip_max_len);
		memcpy(context.das_serverid, sdk_kernel->redirect_das_info.das_serverid, ezdev_sdk_ip_max_len);
		broadcast_user_event(sdk_kernel_event_fast_reg_online, (void*)&context, sizeof(context));
	}
	else
	{
		broadcast_runtime_err(TAG_ACCESS, mkiE2ezE(sdk_error), NULL, 0);
		sdk_kernel->cnt_state = sdk_cnt_unredirect;
		sdk_kernel->lbs_redirect_times = 0;
		sdk_kernel->das_retry_times = 0;
	}

	return sdk_error;
}
static mkernel_internal_error cnt_state_yield(ezdev_sdk_kernel* sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	if (check_access_risk_control(sdk_kernel))
	{
		return mkernel_internal_force_offline;
	}

	switch(sdk_kernel->cnt_state)
	{
		case sdk_cnt_unredirect:
		{
			sdk_error = cnt_lbs_redirect_do(sdk_kernel);
			break;
		}
		case sdk_cnt_redirected:
		{
			sdk_error = cnt_das_reg_do(sdk_kernel);
			break;
		}
		case sdk_cnt_das_reged:
		{
			sdk_error = cnt_das_work_do(sdk_kernel);
			break;
		}
		case sdk_cnt_das_break:
		{
			sdk_error = cnt_das_retry_do(sdk_kernel);
			break;
		}
		case sdk_cnt_das_fast_reg:
		{
			sdk_error = cnt_das_reg_fast_do(sdk_kernel);
			break;
		}
		case sdk_cnt_das_fast_reg_v3:
		{
			sdk_error = cnt_das_reg_v3_fast_do(sdk_kernel);
			break;
		}
		default:
		{
			sdk_error = mkernel_internal_internal_err;
		}
	}

	if (mkernel_internal_mqtt_blacklist == sdk_error)
	{
		add_access_risk_control(sdk_kernel);
		ezdev_sdk_kernel_log_info(sdk_error, sdk_error, "access risk control");
	}
	else if (mkernel_internal_mqtt_redirect == sdk_error)
	{
		sdk_offline_context context = {0};
		context.last_error = sdk_error;
		memcpy(context.das_ip, sdk_kernel->redirect_das_info.das_address, ezdev_sdk_ip_max_len);
		memcpy(context.lbs_ip, sdk_kernel->server_info.server_ip, ezdev_sdk_ip_max_len);
		broadcast_user_event(sdk_kernel_event_break, (void*)&context, sizeof(context));

		sdk_kernel->cnt_state = sdk_cnt_unredirect;
		sdk_kernel->lbs_redirect_times = 0;
		sdk_kernel->das_retry_times = 0;
		ezdev_sdk_kernel_log_error(sdk_error, sdk_error, "dev go to lbs redirect ");
	}

	return sdk_error;
}

mkernel_internal_error stop_yield(ezdev_sdk_kernel* sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	sdk_error = das_unreg(sdk_kernel);
	sdk_kernel->cnt_state = sdk_cnt_unredirect;
	return sdk_error;
}

mkernel_internal_error access_server_yield(ezdev_sdk_kernel* sdk_kernel)
{
	return cnt_state_yield(sdk_kernel);
}

mkernel_internal_error ezdev_sdk_kernel_inner_send(const ezdev_sdk_kernel_pubmsg* pubmsg)
{
	/* 由于对于body的处理后期需要用ase做加密，直接在这里做padding */
	EZDEV_SDK_INT32 input_length_padding = 0;
	ezdev_sdk_kernel_pubmsg_exchange* new_pubmsg_exchange = NULL; 
	mkernel_internal_error kernel_internal_error = mkernel_internal_succ;

	ezdev_sdk_kernel_log_trace(0, 0, "inner_send: domain:%d ,cmd:%d, seq:%d,len:%d, string:%s", pubmsg->msg_domain_id, pubmsg->msg_command_id, pubmsg->msg_seq, pubmsg->msg_body_len, pubmsg->msg_body);

	if (pubmsg->msg_body == NULL || pubmsg->msg_body_len == 0)
	{
		return mkernel_internal_input_param_invalid;
	}

	if(pubmsg->msg_body_len > ezdev_sdk_send_buf_max)
	{
		return mkernel_internal_msg_len_overrange;
	}

	new_pubmsg_exchange = (ezdev_sdk_kernel_pubmsg_exchange*)malloc(sizeof(ezdev_sdk_kernel_pubmsg_exchange));
	if (new_pubmsg_exchange == NULL)
	{
		return mkernel_internal_malloc_error;
	}
	
	memset(new_pubmsg_exchange, 0, sizeof(ezdev_sdk_kernel_pubmsg_exchange));
	strncpy(new_pubmsg_exchange->msg_conntext.command_ver, pubmsg->command_ver, version_max_len - 1);
	new_pubmsg_exchange->msg_conntext.msg_response = pubmsg->msg_response;
	new_pubmsg_exchange->msg_conntext.msg_qos = pubmsg->msg_qos;
	new_pubmsg_exchange->msg_conntext.msg_seq =	pubmsg->msg_seq;
	new_pubmsg_exchange->msg_conntext.msg_domain_id = pubmsg->msg_domain_id;
	new_pubmsg_exchange->msg_conntext.msg_command_id = pubmsg->msg_command_id;
	input_length_padding = pubmsg->msg_body_len;

	new_pubmsg_exchange->msg_conntext.msg_body = (unsigned char*)malloc(input_length_padding);
	if (new_pubmsg_exchange->msg_conntext.msg_body == NULL)
	{
		free(new_pubmsg_exchange);
		new_pubmsg_exchange = NULL;

		ezdev_sdk_kernel_log_error(mkernel_internal_malloc_error, mkernel_internal_malloc_error, "malloc input_length_padding err");
		return mkernel_internal_malloc_error;
	}
	memset(new_pubmsg_exchange->msg_conntext.msg_body, 0, input_length_padding);
	new_pubmsg_exchange->msg_conntext.msg_body_len = input_length_padding;
	memcpy(new_pubmsg_exchange->msg_conntext.msg_body ,pubmsg->msg_body,  pubmsg->msg_body_len);

	buf_padding(new_pubmsg_exchange->msg_conntext.msg_body, input_length_padding, pubmsg->msg_body_len);

	new_pubmsg_exchange->max_send_count = 1;	// 主动下线消息只发一次

	/*非阻塞式往消息队列里push内容 最终由SDKboot模块创建的主线程驱动消息发送*/
	kernel_internal_error= das_send_pubmsg_async(&g_ezdev_sdk_kernel, new_pubmsg_exchange);
	if (kernel_internal_error != mkernel_internal_succ)
	{
		if (new_pubmsg_exchange != NULL)
		{
			if (new_pubmsg_exchange->msg_conntext.msg_body != NULL)
			{
				free(new_pubmsg_exchange->msg_conntext.msg_body);
				new_pubmsg_exchange->msg_conntext.msg_body=NULL;
			}

			free(new_pubmsg_exchange);
			new_pubmsg_exchange = NULL;
		}
	}

	return kernel_internal_error;
}

mkernel_internal_error send_offline_msg_to_platform(EZDEV_SDK_UINT32 seq)
{
    
	ezxml_t req;
	ezdev_sdk_kernel_pubmsg pubmsg;
    unsigned char* msg_body_offline;
    mkernel_internal_error sdk_error = mkernel_internal_succ;

	req = ezxml_new("Request");
	ezxml_add_child(req, "DevSerial",1);
	ezxml_set_txt(req->child, ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial"));
	ezxml_add_child(req, "Authorization",2);

	msg_body_offline = (unsigned char*)ezxml_toxml(req);

	memset(&pubmsg, 0, sizeof(ezdev_sdk_kernel_pubmsg));

	pubmsg.msg_response = 0;
    pubmsg.msg_qos = QOS_T1;
	pubmsg.msg_seq = seq;

	pubmsg.msg_body = msg_body_offline;
	pubmsg.msg_body_len = strlen((const char*)msg_body_offline);

	pubmsg.msg_domain_id = ezdev_sdk_domain_id;
	pubmsg.msg_command_id = ezdev_sdk_offline_cmd_id;

	strncpy(pubmsg.command_ver, ezdev_sdk_cmd_version, version_max_len - 1);
 
    sdk_error= ezdev_sdk_kernel_inner_send(&pubmsg);

    if(mkernel_internal_succ != sdk_error)
    {
        ezdev_sdk_kernel_log_info(sdk_error, sdk_error, "s offline msg failed,");
    }
    
	free(msg_body_offline);	
	msg_body_offline = NULL;
	ezxml_free(req);

	return sdk_error;
}