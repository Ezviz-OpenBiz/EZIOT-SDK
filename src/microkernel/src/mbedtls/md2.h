/**
 * \file md2.h
 *
 * \brief MD2 message digest algorithm (hash function)
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
#ifndef BSCOMPTLS_MD2_H
#define BSCOMPTLS_MD2_H

#if !defined(BSCOMPTLS_CONFIG_FILE)
#include "config.h"
#else
#include BSCOMPTLS_CONFIG_FILE
#endif

#include <stddef.h>

#if !defined(BSCOMPTLS_MD2_ALT)
// Regular implementation
//

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          MD2 context structure
 */
typedef struct
{
    unsigned char cksum[16];    /*!< checksum of the data block */
    unsigned char state[48];    /*!< intermediate digest state  */
    unsigned char buffer[16];   /*!< data block being processed */
    size_t left;                /*!< amount of data in buffer   */
}
bscomptls_md2_context;

/**
 * \brief          Initialize MD2 context
 *
 * \param ctx      MD2 context to be initialized
 */
void bscomptls_md2_init( bscomptls_md2_context *ctx );

/**
 * \brief          Clear MD2 context
 *
 * \param ctx      MD2 context to be cleared
 */
void bscomptls_md2_free( bscomptls_md2_context *ctx );

/**
 * \brief          Clone (the state of) an MD2 context
 *
 * \param dst      The destination context
 * \param src      The context to be cloned
 */
void bscomptls_md2_clone( bscomptls_md2_context *dst,
                        const bscomptls_md2_context *src );

/**
 * \brief          MD2 context setup
 *
 * \param ctx      context to be initialized
 */
void bscomptls_md2_starts( bscomptls_md2_context *ctx );

/**
 * \brief          MD2 process buffer
 *
 * \param ctx      MD2 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void bscomptls_md2_update( bscomptls_md2_context *ctx, const unsigned char *input, size_t ilen );

/**
 * \brief          MD2 final digest
 *
 * \param ctx      MD2 context
 * \param output   MD2 checksum result
 */
void bscomptls_md2_finish( bscomptls_md2_context *ctx, unsigned char output[16] );

#ifdef __cplusplus
}
#endif

#else  /* BSCOMPTLS_MD2_ALT */
#include "md2_alt.h"
#endif /* BSCOMPTLS_MD2_ALT */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Output = MD2( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   MD2 checksum result
 */
void bscomptls_md2( const unsigned char *input, size_t ilen, unsigned char output[16] );

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int bscomptls_md2_self_test( int verbose );

/* Internal use */
void bscomptls_md2_process( bscomptls_md2_context *ctx );

#ifdef __cplusplus
}
#endif

#endif /* bscomptls_md2.h */
