/**
 * \file platform.h
 *
 * \brief mbed TLS Platform abstraction layer
 *
 *  Copyright (C) 2006-2016, ARM Limited, All Rights Reserved
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
#ifndef BSCOMPTLS_PLATFORM_H
#define BSCOMPTLS_PLATFORM_H

#if !defined(BSCOMPTLS_CONFIG_FILE)
#include "config.h"
#else
#include BSCOMPTLS_CONFIG_FILE
#endif

#if defined(BSCOMPTLS_HAVE_TIME)
#include "mbedtls/platform_time.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \name SECTION: Module settings
 *
 * The configuration options you can set for this module are in this section.
 * Either change them in config.h or define them on the compiler command line.
 * \{
 */

#if !defined(BSCOMPTLS_PLATFORM_NO_STD_FUNCTIONS)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#if !defined(BSCOMPTLS_PLATFORM_STD_SNPRINTF)
#if defined(_WIN32)
#define BSCOMPTLS_PLATFORM_STD_SNPRINTF   bscomptls_platform_win32_snprintf /**< Default snprintf to use  */
#else
#define BSCOMPTLS_PLATFORM_STD_SNPRINTF   snprintf /**< Default snprintf to use  */
#endif
#endif
#if !defined(BSCOMPTLS_PLATFORM_STD_PRINTF)
#define BSCOMPTLS_PLATFORM_STD_PRINTF   printf /**< Default printf to use  */
#endif
#if !defined(BSCOMPTLS_PLATFORM_STD_FPRINTF)
#define BSCOMPTLS_PLATFORM_STD_FPRINTF fprintf /**< Default fprintf to use */
#endif
#if !defined(BSCOMPTLS_PLATFORM_STD_CALLOC)
#define BSCOMPTLS_PLATFORM_STD_CALLOC   calloc /**< Default allocator to use */
#endif
#if !defined(BSCOMPTLS_PLATFORM_STD_FREE)
#define BSCOMPTLS_PLATFORM_STD_FREE       free /**< Default free to use */
#endif
#if !defined(BSCOMPTLS_PLATFORM_STD_EXIT)
#define BSCOMPTLS_PLATFORM_STD_EXIT      exit /**< Default exit to use */
#endif
#if !defined(BSCOMPTLS_PLATFORM_STD_TIME)
#define BSCOMPTLS_PLATFORM_STD_TIME       time    /**< Default time to use */
#endif
#if !defined(BSCOMPTLS_PLATFORM_STD_EXIT_SUCCESS)
#define BSCOMPTLS_PLATFORM_STD_EXIT_SUCCESS  EXIT_SUCCESS /**< Default exit value to use */
#endif
#if !defined(BSCOMPTLS_PLATFORM_STD_EXIT_FAILURE)
#define BSCOMPTLS_PLATFORM_STD_EXIT_FAILURE  EXIT_FAILURE /**< Default exit value to use */
#endif
#if defined(BSCOMPTLS_FS_IO)
#if !defined(BSCOMPTLS_PLATFORM_STD_NV_SEED_READ)
#define BSCOMPTLS_PLATFORM_STD_NV_SEED_READ   bscomptls_platform_std_nv_seed_read
#endif
#if !defined(BSCOMPTLS_PLATFORM_STD_NV_SEED_WRITE)
#define BSCOMPTLS_PLATFORM_STD_NV_SEED_WRITE  bscomptls_platform_std_nv_seed_write
#endif
#if !defined(BSCOMPTLS_PLATFORM_STD_NV_SEED_FILE)
#define BSCOMPTLS_PLATFORM_STD_NV_SEED_FILE   "seedfile"
#endif
#endif /* BSCOMPTLS_FS_IO */
#else /* BSCOMPTLS_PLATFORM_NO_STD_FUNCTIONS */
#if defined(BSCOMPTLS_PLATFORM_STD_MEM_HDR)
#include BSCOMPTLS_PLATFORM_STD_MEM_HDR
#endif
#endif /* BSCOMPTLS_PLATFORM_NO_STD_FUNCTIONS */


/* \} name SECTION: Module settings */

/*
 * The function pointers for calloc and free
 */
#if defined(BSCOMPTLS_PLATFORM_MEMORY)
#if defined(BSCOMPTLS_PLATFORM_FREE_MACRO) && \
    defined(BSCOMPTLS_PLATFORM_CALLOC_MACRO)
#define bscomptls_free       BSCOMPTLS_PLATFORM_FREE_MACRO
#define bscomptls_calloc     BSCOMPTLS_PLATFORM_CALLOC_MACRO
#else
/* For size_t */
#include <stddef.h>
extern void * (*bscomptls_calloc)( size_t n, size_t size );
extern void (*bscomptls_free)( void *ptr );

/**
 * \brief   Set your own memory implementation function pointers
 *
 * \param calloc_func   the calloc function implementation
 * \param free_func     the free function implementation
 *
 * \return              0 if successful
 */
int bscomptls_platform_set_calloc_free( void * (*calloc_func)( size_t, size_t ),
                              void (*free_func)( void * ) );
#endif /* BSCOMPTLS_PLATFORM_FREE_MACRO && BSCOMPTLS_PLATFORM_CALLOC_MACRO */
#else /* !BSCOMPTLS_PLATFORM_MEMORY */
#define bscomptls_free       free
#define bscomptls_calloc     calloc
#endif /* BSCOMPTLS_PLATFORM_MEMORY && !BSCOMPTLS_PLATFORM_{FREE,CALLOC}_MACRO */

/*
 * The function pointers for fprintf
 */
#if defined(BSCOMPTLS_PLATFORM_FPRINTF_ALT)
/* We need FILE * */
#include <stdio.h>
extern int (*bscomptls_fprintf)( FILE *stream, const char *format, ... );

/**
 * \brief   Set your own fprintf function pointer
 *
 * \param fprintf_func   the fprintf function implementation
 *
 * \return              0
 */
int bscomptls_platform_set_fprintf( int (*fprintf_func)( FILE *stream, const char *,
                                               ... ) );
#else
#if defined(BSCOMPTLS_PLATFORM_FPRINTF_MACRO)
#define bscomptls_fprintf    BSCOMPTLS_PLATFORM_FPRINTF_MACRO
#else
#define bscomptls_fprintf    fprintf
#endif /* BSCOMPTLS_PLATFORM_FPRINTF_MACRO */
#endif /* BSCOMPTLS_PLATFORM_FPRINTF_ALT */

/*
 * The function pointers for printf
 */
#if defined(BSCOMPTLS_PLATFORM_PRINTF_ALT)
extern int (*bscomptls_printf)( const char *format, ... );

/**
 * \brief   Set your own printf function pointer
 *
 * \param printf_func   the printf function implementation
 *
 * \return              0
 */
int bscomptls_platform_set_printf( int (*printf_func)( const char *, ... ) );
#else /* !BSCOMPTLS_PLATFORM_PRINTF_ALT */
#if defined(BSCOMPTLS_PLATFORM_PRINTF_MACRO)
#define bscomptls_printf     BSCOMPTLS_PLATFORM_PRINTF_MACRO
#else
#define bscomptls_printf     printf
#endif /* BSCOMPTLS_PLATFORM_PRINTF_MACRO */
#endif /* BSCOMPTLS_PLATFORM_PRINTF_ALT */

/*
 * The function pointers for snprintf
 *
 * The snprintf implementation should conform to C99:
 * - it *must* always correctly zero-terminate the buffer
 *   (except when n == 0, then it must leave the buffer untouched)
 * - however it is acceptable to return -1 instead of the required length when
 *   the destination buffer is too short.
 */
#if defined(_WIN32)
/* For Windows (inc. MSYS2), we provide our own fixed implementation */
int bscomptls_platform_win32_snprintf( char *s, size_t n, const char *fmt, ... );
#endif

#if defined(BSCOMPTLS_PLATFORM_SNPRINTF_ALT)
extern int (*bscomptls_snprintf)( char * s, size_t n, const char * format, ... );

/**
 * \brief   Set your own snprintf function pointer
 *
 * \param snprintf_func   the snprintf function implementation
 *
 * \return              0
 */
int bscomptls_platform_set_snprintf( int (*snprintf_func)( char * s, size_t n,
                                                 const char * format, ... ) );
#else /* BSCOMPTLS_PLATFORM_SNPRINTF_ALT */
#if defined(BSCOMPTLS_PLATFORM_SNPRINTF_MACRO)
#define bscomptls_snprintf   BSCOMPTLS_PLATFORM_SNPRINTF_MACRO
#else
#define bscomptls_snprintf   snprintf
#endif /* BSCOMPTLS_PLATFORM_SNPRINTF_MACRO */
#endif /* BSCOMPTLS_PLATFORM_SNPRINTF_ALT */

/*
 * The function pointers for exit
 */
#if defined(BSCOMPTLS_PLATFORM_EXIT_ALT)
extern void (*bscomptls_exit)( int status );

/**
 * \brief   Set your own exit function pointer
 *
 * \param exit_func   the exit function implementation
 *
 * \return              0
 */
int bscomptls_platform_set_exit( void (*exit_func)( int status ) );
#else
#if defined(BSCOMPTLS_PLATFORM_EXIT_MACRO)
#define bscomptls_exit   BSCOMPTLS_PLATFORM_EXIT_MACRO
#else
#define bscomptls_exit   exit
#endif /* BSCOMPTLS_PLATFORM_EXIT_MACRO */
#endif /* BSCOMPTLS_PLATFORM_EXIT_ALT */

/*
 * The default exit values
 */
#if defined(BSCOMPTLS_PLATFORM_STD_EXIT_SUCCESS)
#define BSCOMPTLS_EXIT_SUCCESS BSCOMPTLS_PLATFORM_STD_EXIT_SUCCESS
#else
#define BSCOMPTLS_EXIT_SUCCESS 0
#endif
#if defined(BSCOMPTLS_PLATFORM_STD_EXIT_FAILURE)
#define BSCOMPTLS_EXIT_FAILURE BSCOMPTLS_PLATFORM_STD_EXIT_FAILURE
#else
#define BSCOMPTLS_EXIT_FAILURE 1
#endif

/*
 * The function pointers for reading from and writing a seed file to
 * Non-Volatile storage (NV) in a platform-independent way
 *
 * Only enabled when the NV seed entropy source is enabled
 */
#if defined(BSCOMPTLS_ENTROPY_NV_SEED)
#if !defined(BSCOMPTLS_PLATFORM_NO_STD_FUNCTIONS) && defined(BSCOMPTLS_FS_IO)
/* Internal standard platform definitions */
int bscomptls_platform_std_nv_seed_read( unsigned char *buf, size_t buf_len );
int bscomptls_platform_std_nv_seed_write( unsigned char *buf, size_t buf_len );
#endif

#if defined(BSCOMPTLS_PLATFORM_NV_SEED_ALT)
extern int (*bscomptls_nv_seed_read)( unsigned char *buf, size_t buf_len );
extern int (*bscomptls_nv_seed_write)( unsigned char *buf, size_t buf_len );

/**
 * \brief   Set your own seed file writing/reading functions
 *
 * \param   nv_seed_read_func   the seed reading function implementation
 * \param   nv_seed_write_func  the seed writing function implementation
 *
 * \return              0
 */
int bscomptls_platform_set_nv_seed(
            int (*nv_seed_read_func)( unsigned char *buf, size_t buf_len ),
            int (*nv_seed_write_func)( unsigned char *buf, size_t buf_len )
            );
#else
#if defined(BSCOMPTLS_PLATFORM_NV_SEED_READ_MACRO) && \
    defined(BSCOMPTLS_PLATFORM_NV_SEED_WRITE_MACRO)
#define bscomptls_nv_seed_read    BSCOMPTLS_PLATFORM_NV_SEED_READ_MACRO
#define bscomptls_nv_seed_write   BSCOMPTLS_PLATFORM_NV_SEED_WRITE_MACRO
#else
#define bscomptls_nv_seed_read    bscomptls_platform_std_nv_seed_read
#define bscomptls_nv_seed_write   bscomptls_platform_std_nv_seed_write
#endif
#endif /* BSCOMPTLS_PLATFORM_NV_SEED_ALT */
#endif /* BSCOMPTLS_ENTROPY_NV_SEED */

#ifdef __cplusplus
}
#endif

#endif /* platform.h */
