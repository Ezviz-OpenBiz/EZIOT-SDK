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
* 2018/8/9       liuxiangchen
*******************************************************************************/

#ifndef H_EZDEVSDK_SHADOW_H_
#define H_EZDEVSDK_SHADOW_H_


#include <stdlib.h>
#include "ez_iot_shadow.h"



#ifdef __cplusplus
extern "C"
{
#endif


#define shadow_module_id		    1002
#define shadow_module_name			"shadow"
#define shadow_module_version		"V2.0.0"	   
#define shadow_cmd_version			"v1.0.0"
#define reg_module_id                99999
#define SPV                          2            /*shadow协议版本号*/



#define SHADOW_KEY_INTERCONNECTION_RULE   "InterconnectionRule"  
#define SHADOW_KEY_DEL_TRUSTEESHIP        "Trusteeship"         


/*shadow领域用到的指令id定义*/
typedef enum{
	kPu2CenPltReportAllStatusReq    =    10,
	kPu2CenPltReportAddStatusReq    =    11,
	kPu2CenPltReportDelStatusReq    =    12,
	kCenPlt2PuSyncStatusNotifyReq   =    22,
    kCenPlt2PuSyncStatusNotifyRsp   =    23,
	kPu2CenPltSyncStatusReq         =    33,
	kPu2CenPltSyncStatusRsp         =    34,
	kPu2CenPltQuerySyncListReq      =    35,
	kPu2CenPltQuerySyncListRsp      =    36,
	kCenPlt2PuSyncStatusNotifyReqV2   =   0X00004F40,
    kCenPlt2PuSyncStatusNotifyRspV2   =   0X00004F41,
}SHADOW_PROTOCOL_DEFINE_N;

typedef enum{
	EZDEVSDK_BIND_STATUS_UNKNOWN = 0,
	EZDEVSDK_BIND_STATUS_OFF,
	EZDEVSDK_BIND_STATUS_ON
}EZDEVSDK_REPORT_BIND_STATUS_E;


typedef enum
{
    SHADOW_REG_STATUS_OFF,
    SHADOW_REG_STATUS_ON,
} SHADOW_REGISTER_STATUS_E;

int shadow_init(void);
void shadow_fnit(void);

#ifdef __cplusplus
}
#endif

#endif

