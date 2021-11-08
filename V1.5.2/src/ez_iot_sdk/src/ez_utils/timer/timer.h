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

#ifndef __COMMON_TIMER_TIMER_H__
#define __COMMON_TIMER_TIMER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

    void *ez_timer_create(char *name, int time_out, bool reload, int (*fun)(void));
    int ez_timer_reset(void *handle);
    int ez_timer_change_period(void *handle, int new_time_out);
    int ez_timer_delete(void *handle);

#ifdef __cplusplus
}
#endif

#endif
