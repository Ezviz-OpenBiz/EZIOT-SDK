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
* 2018-08-09     liuxiangchen
*******************************************************************************/

#ifndef H_SHADOW_EXTERN_H_
#define H_SHADOW_EXTERN_H_

#ifdef __cplusplus
extern "C"
{
#endif


typedef enum
{
    STAUS_PUSH_OFF,
    STAUS_PUSH_ON,
}status_push_flag;


typedef enum{
    SHADOW_INITIAL_STATUS = 0 ,    // shadow初始状态，初始化的时候即处于该状态
    SHADOW_REPORT_ADD_STATUS ,     // 增量上报状态，当发生重连，快速重连，online事件时，如果状态处于2则切换到该状态执行增量上报
    SHADOW_STATUS_CHECK ,          // 循环检测状态，该状态负责循环检测是否有装填需要上报
    SHADOW_EXIT_STATUS ,          //  shadow强制退出状态，反初始化的时候设置
}EZDEVSDK_SHADOW_STATUS_E;



int Shadow_extern_init(void);

int Shadow_extern_fini(void);

int set_start_push_flag(int value);

int get_start_push_flag();

int Shadow_Set_Status(int value);

int Shadow_get_status(void);

int Shadow_Set_Switchover_flag(int value);

int Shadow_Get_Switchover_flag();

void shadow_set_register_status(int status);

int  shadow_get_register_status();

#ifdef __cplusplus
}
#endif


#endif 




