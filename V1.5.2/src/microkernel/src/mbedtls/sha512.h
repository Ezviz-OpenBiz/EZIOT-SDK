/**
 * \file sha512.h
 *
 * \brief SHA-384 and SHA-512 cryptographic hash function
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
#ifndef BSCOMPTLS_SHA512_H
#define BSCOMPTLS_SHA512_H

#if !defined(BSCOMPTLS_CONFIG_FILE)
#include "config.h"
#else
#include BSCOMPTLS_CONFIG_FILE
#endif

#include <stddef.h>
#include <stdint.h>

#if !defined(BSCOMPTLS_SHA512_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          SHA-512 context structure
 */
typedef struct
{
    uint64_t total[2];          /*!< number of bytes processed  */
    uint64_t state[8];          /*!< intermediate digest state  */
    unsigned char buffer[128];  /*!< data block being processed */
    int is384;                  /*!< 0 => SHA-512, else SHA-384 */
}
bscomptls_sha512_context;

/**
 * \brief          Initialize SHA-512 context
 *
 * \param ctx      SHA-512 context to be initialized
 */
void bscomptls_sha512_init( bscomptls_sha512_context *ctx );

/**
 * \brief          Clear SHA-512 context
 *
 * \param ctx      SHA-512 context to be cleared
 */
void bscomptls_sha512_free( bscomptls_sha512_context *ctx );

/**
 * \brief          Clone (the state of) a SHA-512 context
 *
 * \param dst      The destination context
 * \param src      The context to be cloned
 */
void bscomptls_sha512_clone( bscomptls_sha512_context *dst,
                           const bscomptls_sha512_context *src );

/**
 * \brief          SHA-512 context setup
 *
 * \param ctx      context to be initialized
 * \param is384    0 = use SHA512, 1 = use SHA384
 */
void bscomptls_sha512_starts( bscomptls_sha512_context *ctx, int is384 );

/**
 * \brief          SHA-512 process buffer
 *
 * \param ctx      SHA-512 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void bscomptls_sha512_update( bscomptls_sha512_context *ctx, const unsigned char *input,
                    size_t ilen );

/**
 * \brief          SHA-512 final digest
 *
 * \param ctx      SHA-512 context
 * \param output   SHA-384/512 checksum result
 */
void bscomptls_sha512_finish( bscomptls_sha512_context *ctx, unsigned char output[64] );

#ifdef __cplusplus
}
#endif

#else  /* BSCOMPTLS_SHA512_ALT */
#include "sha512_alt.h"
#endif /* BSCOMPTLS_SHA512_ALT */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Output = SHA-512( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SHA-384/512 checksum result
 * \param is384    0 = use SHA512, 1 = use SHA384
 */
void bscomptls_sha512( const unsigned char *input, size_t ilen,
             unsigned char output[64], int is384 );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int bscomptls_sha512_self_test( int verbose );

/* Internal use */
void bscomptls_sha512_process( bscomptls_sha512_context *ctx, const unsigned char data[128] );

#ifdef __cplusplus
}
#endif

#endif /* bscomptls_sha512.h */
