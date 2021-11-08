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
* 2021/01/01    shenhongyin
*******************************************************************************/

#ifndef H_EZ_OTA_DEF_H_
#define H_EZ_OTA_DEF_H_


#define ota_module_name			"ota"	
#define ota_version		        "V1.0.0"

/*brief 内部的三种状态，设备只有在ota_state_upgradeable、ota_state_succ、ota_state_fail三种状态下,
 *        云端才能判断设备是否可升级。
 * APP 可以用，不会覆盖设备实际的状态，假设设备升级出现故障，上报了1~6错误都可以通过平台查询
 */
#define ota_state_ready    0
#define ota_state_succ     7    
#define ota_state_failed   8

#define min_report_interval  5*1000


#ifdef  __cplusplus
extern "C" {
#endif


typedef enum
{
    ez_ota_offline,
    ez_ota_online,
}ez_ota_online_status;

#ifdef __cplusplus
}
#endif

#endif

