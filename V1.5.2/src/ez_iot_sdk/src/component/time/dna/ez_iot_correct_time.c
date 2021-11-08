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
#include "dna_libc.h"
#include "dna_sockets.h"
#include "dna_tinydns.h"
#include "dna_os.h"

#include "ez_iot_log.h"
#include "ez_iot.h"

#include "lwipopts.h"
#include "lwip/apps/sntp.h"

static void initialize_sntp(const char *ntp_server)
{
    ez_log_w(TAG_TIME, "initializing sntp.");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, (char *)ntp_server);
    sntp_init();
}

int8_t ez_iot_correct_time(const char *ntp_server, int time_zone)
{
    if (NULL == ntp_server || time_zone < -12 || time_zone > 12)
    {
        ez_log_w(TAG_TIME, "correct time param error.");
        return -1;
    }

    initialize_sntp(ntp_server);

    // wait for time to be set
    int retry = 1;
    const int retry_count = 10;

    dna_time_t dna_t = {0};
    while (dna_t.year < 2016 && retry <= retry_count)
    {
        ez_log_w(TAG_TIME, "waiting for correct time... (%d,%d)", retry, retry_count);
        dna_task_sleep(2000);

        dna_get_time(&dna_t, 0);
        retry++;
    }

    if (retry > 10)
    {
        ez_log_w(TAG_TIME, "correct time failed.");
        return -1;
    }

    unsigned long time_current = dna_utc_timestamp();
    dna_gmtime(time_current, &dna_t);
    dna_set_time(&dna_t, 0);

    sntp_stop();
    return 0;
}