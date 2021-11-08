/**
 * \file padlock.h
 *
 * \brief VIA PadLock ACE for HW encryption/decryption supported by some
 *        processors
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
#ifndef BSCOMPTLS_PADLOCK_H
#define BSCOMPTLS_PADLOCK_H

#include "aes.h"

#define BSCOMPTLS_ERR_PADLOCK_DATA_MISALIGNED               -0x0030  /**< Input data should be aligned. */

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define BSCOMPTLS_HAVE_ASAN
#endif
#endif

/* Some versions of ASan result in errors about not enough registers */
#if defined(BSCOMPTLS_HAVE_ASM) && defined(__GNUC__) && defined(__i386__) && \
    !defined(BSCOMPTLS_HAVE_ASAN)

#ifndef BSCOMPTLS_HAVE_X86
#define BSCOMPTLS_HAVE_X86
#endif

#include <stdint.h>

#define BSCOMPTLS_PADLOCK_RNG 0x000C
#define BSCOMPTLS_PADLOCK_ACE 0x00C0
#define BSCOMPTLS_PADLOCK_PHE 0x0C00
#define BSCOMPTLS_PADLOCK_PMM 0x3000

#define BSCOMPTLS_PADLOCK_ALIGN16(x) (uint32_t *) (16 + ((int32_t) x & ~15))

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          PadLock detection routine
 *
 * \param feature  The feature to detect
 *
 * \return         1 if CPU has support for the feature, 0 otherwise
 */
int bscomptls_padlock_has_support( int feature );

/**
 * \brief          PadLock AES-ECB block en(de)cryption
 *
 * \param ctx      AES context
 * \param mode     BSCOMPTLS_AES_ENCRYPT or BSCOMPTLS_AES_DECRYPT
 * \param input    16-byte input block
 * \param output   16-byte output block
 *
 * \return         0 if success, 1 if operation failed
 */
int bscomptls_padlock_xcryptecb( bscomptls_aes_context *ctx,
                       int mode,
                       const unsigned char input[16],
                       unsigned char output[16] );

/**
 * \brief          PadLock AES-CBC buffer en(de)cryption
 *
 * \param ctx      AES context
 * \param mode     BSCOMPTLS_AES_ENCRYPT or BSCOMPTLS_AES_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 *
 * \return         0 if success, 1 if operation failed
 */
int bscomptls_padlock_xcryptcbc( bscomptls_aes_context *ctx,
                       int mode,
                       size_t length,
                       unsigned char iv[16],
                       const unsigned char *input,
                       unsigned char *output );

#ifdef __cplusplus
}
#endif

#endif /* HAVE_X86  */

#endif /* padlock.h */
