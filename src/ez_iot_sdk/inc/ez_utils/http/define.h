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
#ifndef _EZ_HTTP_DEFINE_H
#define _EZ_HTTP_DEFINE_H

#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>

#include "ez_iot_log.h"

#ifndef RT_NULL
#define RT_NULL 0
#endif

#ifndef RT_FALSE
#define RT_FALSE false
#endif


#ifndef rt_tick_from_millisecond
#define rt_tick_from_millisecond(ms) (ms)
#endif

#define RT_ASSERT(p) \
    if (NULL == p)   \
        ;

#ifndef rt_bool_t
#define rt_bool_t bool
#endif

#ifndef rt_strlen
#define rt_strlen strlen
#endif

#ifndef rt_strncpy
#define rt_strncpy strncpy
#endif

#ifndef rt_strstr
#define rt_strstr strstr
#endif

#ifndef rt_malloc
#define rt_malloc malloc
#endif

#ifndef rt_calloc
#define rt_calloc calloc
#endif

#ifndef rt_realloc
#define rt_realloc realloc
#endif

#ifndef rt_memset
#define rt_memset memset
#endif

#ifndef rt_free
#define rt_free free
#endif

#ifndef rt_strdup
#define rt_strdup strdup
#endif

#ifndef rt_int32_t
#define rt_int32_t int
#endif

#ifndef rt_vsnprintf
#define rt_vsnprintf vsnprintf
#endif

#ifndef rt_snprintf
#define rt_snprintf snprintf
#endif

#ifndef rt_strcmp
#define rt_strcmp strcmp
#endif

#endif
