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
#include <stdio.h>
#include <stdlib.h>
#include <lwip/sys.h>
#include "hal_time.h"

typedef struct
{
    uint32_t end_time;
} freertos_time;

void *hal_timer_creat()
{
    freertos_time *freertostime = NULL;
    freertostime = (freertos_time *)malloc(sizeof(freertos_time));
    if (freertostime == NULL)
    {
        return NULL;
    }

    freertostime->end_time = sys_now();
    return (void *)freertostime;
}

uint8_t hal_time_isexpired_bydiff(void *timer, uint32_t time_ms)
{
    freertos_time *freertostime = (freertos_time *)timer;
    if (freertostime == NULL)
    {
        return (char)1;
    }

    uint32_t cur_time = sys_now();
    if (cur_time - freertostime->end_time >= (time_ms))
    {
        return (char)1;
    }
    else
    {
        return (char)0;
    }
}

uint8_t hal_time_isexpired(void *timer)
{
    freertos_time *freertostime = (freertos_time *)timer;

    if (freertostime == NULL)
    {
        return (char)1;
    }

    uint32_t cur_time = sys_now();
    if (cur_time >= freertostime->end_time)
    {
        return (char)1;
    }
    else
    {
        return (char)0;
    }
}

void hal_time_countdown_ms(void *timer, uint32_t timeout)
{
    freertos_time *freertostime = (freertos_time *)timer;
    if (freertostime == NULL)
    {
        return;
    }

    uint32_t cur_time = sys_now();
    freertostime->end_time = cur_time + timeout;
}

void hal_time_countdown(void *timer, uint32_t timeout)
{
    hal_time_countdown_ms(timer, timeout * 1000);
}

uint32_t hal_time_left_ms(void *timer)
{
    freertos_time *freertostime = (freertos_time *)timer;
    if (freertostime == NULL)
    {
        return 0;
    }

    uint32_t cur_time = sys_now();
    if (freertostime->end_time <= cur_time)
    {
        return 0;
    }
    else
    {
        return (freertostime->end_time - cur_time);
    }
}

void hal_timer_destroy(void *timer)
{
    freertos_time *freertostime = (freertos_time *)timer;

    if (freertostime == NULL)
    {
        return;
    }

    free(freertostime);
    freertostime = NULL;
}