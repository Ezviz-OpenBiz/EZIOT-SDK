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

#ifndef H_EZDEV_SDK_KERNEL_H_
#define H_EZDEV_SDK_KERNEL_H_

#include "base_typedef.h"
#include "ezdev_sdk_kernel_error.h"
#include "ezdev_sdk_kernel_struct.h"

/**
 * @addtogroup micro_kernel ΢�ں�ģ��
 * ΢�ں����ṩ����������ģ�����ĺ���ģ�飬�ڲ���Ҫ�������豸����ƽ̨������ģ������豸��صȹ���
 *  \{
 */

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 *  \brief		΢�ں˳�ʼ���ӿڣ����̰߳�ȫ��
 *  \method		ezdev_sdk_kernel_init
 *	\note		өʯ�豸����SDK ΢�ں˳�ʼ���ӿڣ�ֻ֧�ֵ��豸ģʽ���˽ӿ���ezDevSDK_bootģ���е��ã���������ģ���������
 *  \param[in] 	server_name						�����ַ ֧������
 *  \param[in] 	server_port						��������Ķ˿�
 *  \param[in] 	kernel_platform_handle			��ƽ̨�ӿ�ʵ�֣�������Ľӿ���ezDevSDK_boot�����Ҫʵ�֣��ڲ������������
 *  \param[in] 	kernel_event_notice_cb			΢�ں��ڲ���Ϣ�ص�
 *  \param[in] 	dev_config_info					������Ϣͨ��json������ʽ,���·ֱ�ΪSAP��licenseģʽ��֤��Ҫ��json���ݸ�ʽ
 *	\details
 *				{
 *				"dev_auth_mode":0,										ѡ��,Ĭ��0;SAP��֤ģʽ
 *				"dev_access_mode":0										ѡ��,Ĭ��0;�豸����ģʽ  0-��ͨ��2.0��   1-HUB��2.0��
 *				"dev_status":1,											����;�豸����״̬ 1����������ģʽ  5������(��˯��)����ģʽ
 *				"dev_subserial":"411444968",							����;�豸�����к�(���16)
 *				"dev_verification_code":"ABCDEF",						����;�豸��֤��---�ϸ��ܸı䣬����ᵼ���豸�޷�����(���16)
 *				"dev_serial":"DS-2CD8464F-EI0120120923CCRR411444968",	����;�豸�����к�(���64)
 *				"dev_firmwareversion":"V2.2.0 build150205",				����;�豸�̼��汾��(���64)						
 *				"dev_type":"DS-2CD8464F-EI",							����;�豸�ͺ�(���64)						
 *				"dev_typedisplay":"DS-2CD8464F-EI",						����;�豸��ʾ�ͺ�(���64)
 *				"dev_mac":"004048C5E1B8",								����;�豸���������ַ(���64)
 *				"dev_nickname":"C1(411444968)",							����;�豸�ǳ�(���64)	
 *				"dev_firmwareidentificationcode":"00000001000",			����;�豸�̼�ʶ����(���256)
 *				"dev_oeminfo",											����;�豸OEM��Ϣ
 *				}
 *	\details
 *				{
 *				"dev_auth_mode":1,										ѡ��,Ĭ��0;License��֤ģʽ
 *				"dev_access_mode":0										ѡ��,Ĭ��0;�豸����ģʽ  0-��ͨ��2.0��   1-HUB��2.0��
 *				"dev_productKey":"HIK_PARKING_PLANTFORM",				����;ͨ��license����ӿ����������productKey
 *				"dev_deviceName":"HIKPP12345",							����;ͨ��license����ӿ����������dev_deviceName
 *				"dev_deviceLicense":"Lm9HhDdtvqWXR2F52or6p3",			����;ͨ��license����ӿ����������dev_deviceLicense
 *				"dev_mac":"004048C5E1B8",								����;�豸���������ַ(���64)
 *				"dev_nickname":"C1(411444968)",							����;�豸�ǳ�(���64)	
 *				"dev_firmwareversion":"V5.1.3 build 170712",			����;�豸����汾��
 *				}
 *	\param[in]	reg_das_info					�߿���ע������ģʽ������Ҫ����Ϣ
 *  \param[in]  reg_mode						ע��ģʽ
 *  \details
 *	1---�����豸(ƽ̨Ĭ��ֵ)
 *	2---wifi�йܵ͹����豸(������,��ʾ����豸��ǰ�й�״̬)
 *	3---RF�йܵ͹����豸(��������, ��ʾ����豸��ǰ�й�״̬)
 *	4---RF����(��������, ��ʾ֧��RF�й�,��Base�豸�ϱ�)
 *  \return 	ezdev_sdk_kernel_succ��ezdev_sdk_kernel_params_invalid��ezdev_sdk_kernel_json_invalid��ezdev_sdk_kernel_json_format�� \n
 *				ezdev_sdk_kernel_value_load��ezdev_sdk_kernel_invald_call
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_init(const ezdev_sdk_srv_info_t *psrv_info, const ezdev_sdk_dev_info_t *pdev_info,
																  const ezdev_sdk_kernel_platform_handle* kernel_platform_handle,
																  sdk_kernel_event_notice kernel_event_notice_cb);

/**
 *  \brief		΢�ں˷�ʼ���ӿڣ����̰߳�ȫ��
 *  \method		ezdev_sdk_kernel_fini
 *  \return 	ezdev_sdk_kernel_succ��ezdev_sdk_kernel_invald_call
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_fini();

/** 
 *  \brief		΢�ں�����ģ����ؽӿڣ����̰߳�ȫ��
 *  \method		ezdev_sdk_kernel_extend_load
 *  \param[in] 	external_extend		����ģ����Ϣ
 *  \return 	ezdev_sdk_kernel_succ��ezdev_sdk_kernel_invald_call��ezdev_sdk_kernel_params_invalid��ezdev_sdk_kernel_extend_existed��\n
 *				ezdev_sdk_kernel_extend_full
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_extend_load(const ezdev_sdk_kernel_extend* external_extend);

/** 
 *  \brief		��չģ����ؽӿڣ����̰߳�ȫ��
 *  \method		ezdev_sdk_kernel_extend_load_v3
 *  \param[in] 	external_extend		��չģ����Ϣ
 *  \return 	ezdev_sdk_kernel_succ��ezdev_sdk_kernel_invald_call��ezdev_sdk_kernel_params_invalid��ezdev_sdk_kernel_extend_existed��\n
 *				ezdev_sdk_kernel_extend_full
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_extend_load_v3(const ezdev_sdk_kernel_extend_v3* extend_info);

/** 
 *  \brief		hubģ����ؽӿڣ����̰߳�ȫ��
 *  \method		ezdev_sdk_kernel_hub_extend_load
 *  \param[in] 	hub_extend	����ģ����Ϣ
 *  \return 	ezdev_sdk_kernel_succ��ezdev_sdk_kernel_invald_call��ezdev_sdk_kernel_params_invalid��ezdev_sdk_kernel_extend_existed��\n
 *				ezdev_sdk_kernel_extend_full
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_hub_extend_load(const ezdev_sdk_kernel_hub_extend* hub_extend);

/** 
 *  \brief		΢�ں�ͨ��������ؽӿڣ����̰߳�ȫ��
 *	\note		����Ϊͨ������ģ�����õĽӿڣ���SDKbootģ���е��ã��ϲ������Ӧ�ò���Ҫ���ġ���������õĻ���������ͨ�������ָ��ᱻ���˵�
 *  \method		ezdev_sdk_kernel_common_module_load
 *  \param[in] 	common_module	ͨ������ģ����Ϣ
 *  \return 	ezdev_sdk_kernel_succ��ezdev_sdk_kernel_invald_call��ezdev_sdk_kernel_params_invalid
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_common_module_load(const ezdev_sdk_kernel_common_module* common_module);

/** 
 *  \brief		΢�ں������ӿڣ����̰߳�ȫ��
 *	\note		���΢�ں��Ѿ��������ظ����û᷵��
 *  \method		ezdev_sdk_kernel_start
 *  \return 	ezdev_sdk_kernel_succ��ezdev_sdk_kernel_invald_call
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_start();

/** 
 *  \brief		΢�ں�ֹͣ�ӿڣ����̰߳�ȫ��
 *  \method		ezdev_sdk_kernel_stop
 *  \return 	ezdev_sdk_kernel_succ��ezdev_sdk_kernel_invald_call
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_stop();

/** 
 *  \brief		΢�ں��ڲ�ҵ�������ӿڣ�ͨ���ⲿ�߳������ӿڣ��ڲ�ִ��ҵ��
 *  \method		ezdev_sdk_kernel_yield
 *	\note		����ʽ����
 *  \return 	ezdev_sdk_kernel_succ��ezdev_sdk_kernel_invald_call��ezdev_sdk_kernel_params_invalid��ezdev_sdk_kernel_buffer_too_small��\n
 *				ezdev_sdk_kernel_internal��ezdev_sdk_kernel_value_load��ezdev_sdk_kernel_value_save��ezdev_sdk_kernel_memory��NET_ERROR��\n
 *				LBS_ERROR��SECRETKEY_ERROR��DAS_ERROR
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_yield();

/** 
 *  \brief		΢�ں��û�ҵ�������ӿڣ�ͨ���ⲿ�߳������ӿڣ�������Ϣ�ַ����ϲ������Ӧ��
 *  \method		ezdev_sdk_kernel_yield_user
 *	\note		����Ϣ����ȡ����Ϣ��ͨ���ص��ķ�ʽ�ַ����ϲ������Ӧ��
 *  \return 	ezdev_sdk_kernel_succ��ezdev_sdk_kernel_invald_call��ezdev_sdk_kernel_extend_no_find
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_yield_user();

/** 
 *  \brief		΢�ں����ݷ��ͽӿڣ��̰߳�ȫ��
 *  \method		ezdev_sdk_kernel_send
 *	\note		ֻ������Ϣ��������Ϣ����
 *				������ʽ����
 *  \param[in] 	pubmsg	��Ϣ����
 *  \return 	ezdev_sdk_kernel_succ��ezdev_sdk_kernel_invald_call��ezdev_sdk_kernel_params_invalid��ezdev_sdk_kernel_data_len_range��\n
 *				ezdev_sdk_kernel_memory��ezdev_sdk_kernel_queue_full��ezdev_sdk_kernel_extend_no_find��ezdev_sdk_kernel_force_domain_risk��\n
 *				ezdev_sdk_kernel_force_cmd_risk
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_send(ezdev_sdk_kernel_pubmsg* pubmsg);


/** 
 *  \brief		΢�ں����ݷ��ͽӿ� v3Э�飨�̰߳�ȫ��
 *  \method		ezdev_sdk_kernel_send
 *	\note		ֻ������Ϣ��������Ϣ����
 *				������ʽ����
 *  \param[in] 	pubmsg	��Ϣ����
 *  \return 	ezdev_sdk_kernel_succ��ezdev_sdk_kernel_invald_call��ezdev_sdk_kernel_params_invalid��ezdev_sdk_kernel_data_len_range��\n
 *				ezdev_sdk_kernel_memory��ezdev_sdk_kernel_queue_full��ezdev_sdk_kernel_extend_no_find��ezdev_sdk_kernel_force_domain_risk��\n
 *				ezdev_sdk_kernel_force_cmd_risk
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_send_v3(ezdev_sdk_kernel_pubmsg_v3* pubmsg_v3);

/** 
 *  \brief		����socket����
 *  \method		ezdev_sdk_kernel_set_net_option
 *	\note		����socket��������Ҫ��ezdev_sdk_kernel_startǰ����
 *  \param[in] 	optname ��������, 1 �󶨵�ĳ������ 3 �豸������·�Ͽ�����
 * 	\param[in] 	optval ��������
 *  \param[in] 	optlen ������������
 *  \return 	ezdev_sdk_kernel_succ��ezdev_sdk_kernel_params_invalid��ezdev_sdk_kernel_buffer_too_small
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_set_net_option(int optname, const void* optval, int optlen);

/** 
 *  \brief		өʯ�豸����SDK ΢�ں� �豸��Ϣ��ȡ�ӿ�
 *  \method		ezdev_sdk_kernel_getdevinfo_bykey
 *  \param[in] 	key	���Ҽ�ֵ
 *  \return 	�ɹ�����valueֵָ�� ʧ�ܷ���"invalidkey"
 */
EZDEV_SDK_KERNEL_API const char* ezdev_sdk_kernel_getdevinfo_bykey(const char* key);

/** 
 *  \brief			��ȡ΢�ں˵İ汾�ţ����ε���
 *  \method			ezdev_sdk_kernel_get_sdk_version
 *  \param[out]		pbuf ΢�ں˰汾
 *  \param[inout] 	pbuflen ���pbufΪ�գ����ش��������ݳ��ȣ����򷵻���ʵ��������
 *  \return			ezdev_sdk_kernel_succ��ezdev_sdk_kernel_params_invalid��ezdev_sdk_kernel_buffer_too_small
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_get_sdk_version(char* pbuf, int* pbuflen);

/** 
 *  \brief			��ȡLBS��DAS��������Ϣ�ӿڣ����ε���
 *  \method			ezdev_sdk_kernel_get_server_info
 *  \param[out]		ptr_server_info ��������Ϣ����
 *  \param[inout] 	ptr_count ���ptr_server_infoΪ�գ����ش��������ݵ����������򷵻���ʵ��������
 *  \return			ezdev_sdk_kernel_succ��ezdev_sdk_kernel_invald_call��ezdev_sdk_kernel_params_invalid��ezdev_sdk_kernel_buffer_too_small
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_get_server_info(server_info_s* ptr_server_info, int *ptr_count);
/** 
 *  \brief			show_key���ܽӿ�
 *  \method			ezdev_sdk_kernel_show_key_info
 *  \param[out]		show_key��Ϣ����
 *  \return			ezdev_sdk_kernel_succ��ezdev_sdk_kernel_invald_call��ezdev_sdk_kernel_params_invalid��ezdev_sdk_kernel_buffer_too_small
 */
EZDEV_SDK_KERNEL_API ezdev_sdk_kernel_error ezdev_sdk_kernel_show_key_info(showkey_info* ptr_showkey_info);

#ifdef __cplusplus
}
#endif

/**
 * \}
 */
#endif //H_EZDEV_SDK_KERNEL_H_