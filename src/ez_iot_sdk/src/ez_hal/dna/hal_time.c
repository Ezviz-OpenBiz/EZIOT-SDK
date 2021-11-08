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
#include "hal_time.h"
#include "dna_inc/dna_os.h"

typedef struct 
{
	unsigned long time_record;
}linux_time;

void *hal_timer_creat()
{
    linux_time* linuxtime = NULL;
	linuxtime = (linux_time*)malloc(sizeof(linux_time));
	if (linuxtime == NULL)
	{
		return NULL;
	}

	linuxtime->time_record = 0;
	return linuxtime;
}

uint8_t hal_time_isexpired_bydiff(void *timer, uint32_t time_ms)
{
    linux_time now = {0};
	linux_time* linuxtime = (linux_time*)timer;
	if (linuxtime == NULL)
	{
		return (char)1;
	}

	now.time_record = dna_system_ticks();

	if(now.time_record - linuxtime->time_record > time_ms)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t hal_time_isexpired(void *timer)
{
    linux_time now = {0};
	linux_time* linuxtime = (linux_time*)timer;
	if (linuxtime == NULL)
	{
		return (char)1;
	}

	now.time_record = dna_system_ticks();
	return now.time_record > linuxtime->time_record;
}

void hal_time_countdown_ms(void *timer, uint32_t timeout)
{
    linux_time now = {0};
	linux_time* linuxtime = (linux_time*)timer;
	if (linuxtime == NULL)
	{
		return;
	}

	now.time_record = dna_system_ticks();
	linuxtime->time_record = now.time_record + timeout;
}

void hal_time_countdown(void *timer, uint32_t timeout)
{
    linux_time now = {0};
	linux_time* linuxtime = (linux_time*)timer;
	if (linuxtime == NULL)
	{
		return;
	}

	now.time_record = dna_system_ticks();
	linuxtime->time_record = now.time_record + timeout * 1000;
}

uint32_t hal_time_left_ms(void *timer)
{
    linux_time now = {0};
	linux_time* linuxtime = (linux_time*)timer;
	if (linuxtime == NULL)
	{
		return 0;
	}

	now.time_record = dna_system_ticks();

	return (linuxtime->time_record - now.time_record < 0) ? 0 : linuxtime->time_record - now.time_record;
}

void hal_timer_destroy(void *timer)
{
    linux_time* linuxtime = (linux_time*)timer;
	if (linuxtime == NULL)
	{
		return;
	}
	
	free(linuxtime);
	linuxtime = NULL;
}