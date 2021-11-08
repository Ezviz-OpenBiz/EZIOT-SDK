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
#include "ezconfig.h"
#include "mcuconfig.h"
#include "ezdev_sdk_kernel.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ezdev_sdk_kernel_error.h"
#include "ez_iot_log.h"
#include "ez_ota_def.h"
#include "ez_ota_bus.h"
#include "ez_ota.h"


ez_err_e  ez_ota_send_msg_to_platform(unsigned char* msg, int msg_len, const ota_res_t *pres,const char* msg_type,\
                                      const char* method, int response, unsigned int* msg_seq, int msg_qos)
{
	ezdev_sdk_kernel_pubmsg_v3 pubmsg;
	ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_succ;
	memset(&pubmsg, 0, sizeof(ezdev_sdk_kernel_pubmsg_v3));

    if(NULL == msg || msg_len <= 0 || NULL == method ||NULL == msg_type)
	{
		 ez_log_e(TAG_OTA,"ota_send to_platform, input null");
		return ez_errno_ota_param_invalid;
	}
	if (EZ_OTA_RSP == response)
	{
		pubmsg.msg_seq = *msg_seq;
	}
	pubmsg.msg_response = response;
	pubmsg.msg_qos  = (enum QOS_T)msg_qos;
	pubmsg.msg_body = msg;
	pubmsg.msg_body_len = msg_len;

	strncpy(pubmsg.resource_type, "global", ezdev_sdk_resource_type_len - 1);
	strncpy(pubmsg.resource_id, "0", ezdev_sdk_resource_id_len - 1);
	if(pres&&strlen((char*)pres->dev_serial)>0)
	{
		 ez_log_d(TAG_OTA,"ota dev_serial:%s", pres->dev_serial);
        strncpy(pubmsg.sub_serial, (char*)pres->dev_serial, ezdev_sdk_max_serial_len - 1);
	}

	pubmsg.extend_id  = sdk_ota;
	strncpy(pubmsg.identifier, method, sizeof(pubmsg.identifier) - 1);
	strncpy(pubmsg.business_type, msg_type, sizeof(pubmsg.business_type) - 1);

	sdk_error = ezdev_sdk_kernel_send_v3(&pubmsg);
    if (sdk_error != ezdev_sdk_kernel_succ)
    {
		 ez_log_e(TAG_OTA,"ota_send_msg_to_platform failed: %#02x", sdk_error);
        return ez_errno_ota_msg_send_err;
    }

	if (EZ_OTA_REQ == response)
	{
		 ez_log_d(TAG_OTA,"ota_send_msg_to_platform seq: %d", pubmsg.msg_seq);
		*msg_seq = pubmsg.msg_seq;
	}

	return ez_errno_succ;
}