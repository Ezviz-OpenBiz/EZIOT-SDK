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
* 2020-08-17     xurongjun
*******************************************************************************/

#ifndef _SHADOW_DEF_H_
#define _SHADOW_DEF_H_

#define shadow_default_name "global"

/* 连续在线未发生变更强制上报时间 */
#define FORCE_SYNC_TIME 1000 * 60 * 60 * 24

/* 发送请求最小重试间隔 */
#define MINIIMUM_RETRY_INTERVAL 60 * 1000

/* 发送同步请求最大请求次数 */
#define MAXIMUM_RETRY_TIMES 3

/* 最大信令缓冲区 */
#define PUSH_BUF_MAX_LEN 1024 * 2

#endif