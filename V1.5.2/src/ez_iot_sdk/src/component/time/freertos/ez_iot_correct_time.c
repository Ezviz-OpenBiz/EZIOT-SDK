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
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ezconfig.h"
#include "mcuconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/apps/sntp.h"

#include "hal_time.h"
#include "ez_iot_log.h"
#include "ez_iot.h"

static char g_ntp_server[32] = {0};
extern char g_time_zone[8];
int8_t ez_iot_correct_time(const char *ntp_server, const char *time_zone_info, int daylight, const char *daylight_string)
{
    int8_t rv = -1;
    char zone[64] = {0};
    int i = 0;

    do
    {
        if (NULL == ntp_server || NULL == time_zone_info)
        {
            ez_log_e(TAG_TIME, "sntp param error.");
            break;
        }
		//strcpy(g_time_zone, time_zone_info);
		
		strcpy(g_time_zone, time_zone_info+3);// UTC+08:00，这里需要去除UTC 这个字段,给事件上报时传时间。
		ez_log_w(TAG_TIME, "ntp_server:%s, time_zone_info:%s, daylight:%d, daylight_string:%s", ntp_server, time_zone_info, daylight, daylight_string);

		if (0 != strlen(ntp_server))
		{
            memset(g_ntp_server, 0, sizeof(g_ntp_server));
            strncpy(g_ntp_server, ntp_server, sizeof(g_ntp_server) - 1);
            sntp_setservername(0, g_ntp_server);
            sntp_init();
        }

        for (i = 0; i < strlen(time_zone_info); i++)
        {
            if (time_zone_info[i] == '+')
            {
                sprintf(zone, "CST-%s", &time_zone_info[i + 1]);
                break;
            }
            if (time_zone_info[i] == '-')
            {
                sprintf(zone, "CST+%s", &time_zone_info[i + 1]);
                break;
            }
        }

        if (i == strlen(time_zone_info))
        {
            sprintf(zone, "%s", "CST+0");
        }

        if (0 != daylight)
        {
            ez_log_i(TAG_TIME, "daylight_string:%s.", daylight_string);
            strcat(zone, daylight_string);
        }

        ez_log_i(TAG_TIME, "zone string: %s", zone);
        if (0 != setenv("TZ", zone, 1))
        {
            ez_log_e(TAG_TIME, "Set env failed.");
            break;
        }

        tzset();

        rv = 0;
    } while (0);

    return rv;
}