/**
 * \file timing.h
 *
 * \brief Portable interface to the CPU cycle counter
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */
#ifndef BSCOMPTLS_TIMING_H
#define BSCOMPTLS_TIMING_H

#if !defined(BSCOMPTLS_CONFIG_FILE)
#include "config.h"
#else
#include BSCOMPTLS_CONFIG_FILE
#endif

#if !defined(BSCOMPTLS_TIMING_ALT)
// Regular implementation
//

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          timer structure
 */
struct bscomptls_timing_hr_time
{
    unsigned char opaque[32];
};

/**
 * \brief          Context for bscomptls_timing_set/get_delay()
 */
typedef struct
{
    struct bscomptls_timing_hr_time   timer;
    uint32_t                        int_ms;
    uint32_t                        fin_ms;
} bscomptls_timing_delay_context;

extern volatile int bscomptls_timing_alarmed;

/**
 * \brief          Return the CPU cycle counter value
 *
 * \warning        This is only a best effort! Do not rely on this!
 *                 In particular, it is known to be unreliable on virtual
 *                 machines.
 */
unsigned long bscomptls_timing_hardclock( void );

/**
 * \brief          Return the elapsed time in milliseconds
 *
 * \param val      points to a timer structure
 * \param reset    if set to 1, the timer is restarted
 */
unsigned long bscomptls_timing_get_timer( struct bscomptls_timing_hr_time *val, int reset );

/**
 * \brief          Setup an alarm clock
 *
 * \param seconds  delay before the "bscomptls_timing_alarmed" flag is set
 *
 * \warning        Only one alarm at a time  is supported. In a threaded
 *                 context, this means one for the whole process, not one per
 *                 thread.
 */
void bscomptls_set_alarm( int seconds );

/**
 * \brief          Set a pair of delays to watch
 *                 (See \c bscomptls_timing_get_delay().)
 *
 * \param data     Pointer to timing data
 *                 Must point to a valid \c bscomptls_timing_delay_context struct.
 * \param int_ms   First (intermediate) delay in milliseconds.
 * \param fin_ms   Second (final) delay in milliseconds.
 *                 Pass 0 to cancel the current delay.
 */
void bscomptls_timing_set_delay( void *data, uint32_t int_ms, uint32_t fin_ms );

/**
 * \brief          Get the status of delays
 *                 (Memory helper: number of delays passed.)
 *
 * \param data     Pointer to timing data
 *                 Must point to a valid \c bscomptls_timing_delay_context struct.
 *
 * \return         -1 if cancelled (fin_ms = 0)
 *                  0 if none of the delays are passed,
 *                  1 if only the intermediate delay is passed,
 *                  2 if the final delay is passed.
 */
int bscomptls_timing_get_delay( void *data );

#ifdef __cplusplus
}
#endif

#else  /* BSCOMPTLS_TIMING_ALT */
#include "timing_alt.h"
#endif /* BSCOMPTLS_TIMING_ALT */

#ifdef __cplusplus
extern "C" {
#endif

#if defined(BSCOMPTLS_SELF_TEST)
/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if a test failed
 */
int bscomptls_timing_self_test( int verbose );
#endif

#ifdef __cplusplus
}
#endif

#endif /* timing.h */
