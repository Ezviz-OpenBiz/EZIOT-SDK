/*
 ** Copyright (c) 1990- 1993, 1996 Open Software Foundation, Inc.
 ** Copyright (c) 1989 by Hewlett-Packard Company, Palo Alto, Ca. &
 ** Digital Equipment Corporation, Maynard, Mass.
 ** Copyright (c) 1998 Microsoft.
 ** To anyone who acknowledges that this file is provided "AS IS"
 ** without any express or implied warranty: permission to use, copy,
 ** modify, and distribute this file for any purpose is hereby
 ** granted without fee, provided that the above copyright notices and
 ** this notice appears in all source code copies, and that none of
 ** the names of Open Software Foundation, Inc., Hewlett-Packard
 ** Company, or Digital Equipment Corporation be used in advertising
 ** or publicity pertaining to distribution of the software without
 ** specific, written prior permission.  Neither Open Software
 ** Foundation, Inc., Hewlett-Packard Company, Microsoft, nor Digital Equipment
 ** Corporation makes any representations about the suitability of
 ** this software for any purpose.
 */

#include <sys/unistd.h>
#include <string.h>
#include <stdio.h>
#include "mbedtls/md5.h"
#include "ieee_node.h"
#include "ezconfig.h"
#include "mcuconfig.h"

#if defined(_FREE_RTOS_) || defined(_FREE_RTOS_32_)
#include "esp_system.h"
#endif

/*-----------------------------------------------------------------------------*/
/** 
 * system dependent call to get IEEE node ID.
 * This sample implementation generates a random node ID
 */
void get_ieee_node_identifier(uuid_node_t *node)
{
    char seed[16];
    static int inited = 0;
    static uuid_node_t saved_node;

    if (!inited)
    {
        get_random_info(seed);
        seed[0] |= 0x80;
        memcpy(&saved_node, seed, sizeof(uuid_node_t));
        inited = 1;
    };

    *node = saved_node;
}

/*-----------------------------------------------------------------------------*/
/*
 * system dependent call to get the current system time
 * Returned as 100ns ticks since Oct 15, 1582, but resolution may be
 * less than 100ns.
 */

void get_system_time(uuid_time_t *uuid_time)
{
    struct timeval tp;

    gettimeofday(&tp, (struct timezone *)0);

    /*
       Offset between UUID formatted times and Unix formatted times.
       UUID UTC base time is October 15, 1582.
       Unix base time is January 1, 1970.
       */
    *uuid_time = (tp.tv_sec * 10000000) + (tp.tv_usec * 10) +
                 I64(0x01B21DD213814000);
    ; //lint !e647
}

/*-----------------------------------------------------------------------------*/
void get_random_info(char seed[16])
{
    bscomptls_md5_context c = {0};
    typedef struct
    {
        //struct sysinfo s;
        struct timeval t;
        char hostname[257];
    } randomness;
    randomness r;

    /* Initialize memory area so that valgrind does not complain */
    memset(&r, 0, sizeof r);

    /* Get some random stuff */
    gettimeofday(&r.t, (struct timezone *)0);
#if defined(_FREE_RTOS_) || defined(_FREE_RTOS_32_)
    esp_base_mac_addr_get((uint8_t *)r.hostname);
#else
    (void)gethostname(r.hostname, 256);
#endif

    /* MD5 it */
    bscomptls_md5_init(&c);
    bscomptls_md5_starts(&c);
    bscomptls_md5_update(&c, (unsigned char *)&r, sizeof r);
    bscomptls_md5_finish(&c, (unsigned char *)seed);
}
