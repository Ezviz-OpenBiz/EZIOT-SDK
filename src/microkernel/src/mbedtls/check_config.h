/**
 * \file check_config.h
 *
 * \brief Consistency checks for configuration options
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

/*
 * It is recommended to include this file from your config.h
 * in order to catch dependency issues early.
 */

#ifndef BSCOMPTLS_CHECK_CONFIG_H
#define BSCOMPTLS_CHECK_CONFIG_H

/*
 * We assume CHAR_BIT is 8 in many places. In practice, this is true on our
 * target platforms, so not an issue, but let's just be extra sure.
 */
#include <limits.h>
#if CHAR_BIT != 8
#error "mbed TLS requires a platform with 8-bit chars"
#endif

#if defined(_WIN32)
#if !defined(BSCOMPTLS_PLATFORM_C)
#error "BSCOMPTLS_PLATFORM_C is required on Windows"
#endif

/* Fix the config here. Not convenient to put an #ifdef _WIN32 in config.h as
 * it would confuse config.pl. */
#if !defined(BSCOMPTLS_PLATFORM_SNPRINTF_ALT) && \
    !defined(BSCOMPTLS_PLATFORM_SNPRINTF_MACRO)
#define BSCOMPTLS_PLATFORM_SNPRINTF_ALT
#endif
#endif /* _WIN32 */

#if defined(TARGET_LIKE_MBED) && \
    ( defined(BSCOMPTLS_NET_C) || defined(BSCOMPTLS_TIMING_C) )
#error "The NET and TIMING modules are not available for mbed OS - please use the network and timing functions provided by mbed OS"
#endif

#if defined(BSCOMPTLS_DEPRECATED_WARNING) && \
    !defined(__GNUC__) && !defined(__clang__)
#error "BSCOMPTLS_DEPRECATED_WARNING only works with GCC and Clang"
#endif

#if defined(BSCOMPTLS_HAVE_TIME_DATE) && !defined(BSCOMPTLS_HAVE_TIME)
#error "BSCOMPTLS_HAVE_TIME_DATE without BSCOMPTLS_HAVE_TIME does not make sense"
#endif

#if defined(BSCOMPTLS_AESNI_C) && !defined(BSCOMPTLS_HAVE_ASM)
#error "BSCOMPTLS_AESNI_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_CTR_DRBG_C) && !defined(BSCOMPTLS_AES_C)
#error "BSCOMPTLS_CTR_DRBG_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_DHM_C) && !defined(BSCOMPTLS_BIGNUM_C)
#error "BSCOMPTLS_DHM_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_CMAC_C) && \
    !defined(BSCOMPTLS_AES_C) && !defined(BSCOMPTLS_DES_C)
#error "BSCOMPTLS_CMAC_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_ECDH_C) && !defined(BSCOMPTLS_ECP_C)
#error "BSCOMPTLS_ECDH_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_ECDSA_C) &&            \
    ( !defined(BSCOMPTLS_ECP_C) ||           \
      !defined(BSCOMPTLS_ASN1_PARSE_C) ||    \
      !defined(BSCOMPTLS_ASN1_WRITE_C) )
#error "BSCOMPTLS_ECDSA_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_ECJPAKE_C) &&           \
    ( !defined(BSCOMPTLS_ECP_C) || !defined(BSCOMPTLS_MD_C) )
#error "BSCOMPTLS_ECJPAKE_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_ECDSA_DETERMINISTIC) && !defined(BSCOMPTLS_HMAC_DRBG_C)
#error "BSCOMPTLS_ECDSA_DETERMINISTIC defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_ECP_C) && ( !defined(BSCOMPTLS_BIGNUM_C) || (   \
    !defined(BSCOMPTLS_ECP_DP_SECP192R1_ENABLED) &&                  \
    !defined(BSCOMPTLS_ECP_DP_SECP224R1_ENABLED) &&                  \
    !defined(BSCOMPTLS_ECP_DP_SECP256R1_ENABLED) &&                  \
    !defined(BSCOMPTLS_ECP_DP_SECP384R1_ENABLED) &&                  \
    !defined(BSCOMPTLS_ECP_DP_SECP521R1_ENABLED) &&                  \
    !defined(BSCOMPTLS_ECP_DP_BP256R1_ENABLED)   &&                  \
    !defined(BSCOMPTLS_ECP_DP_BP384R1_ENABLED)   &&                  \
    !defined(BSCOMPTLS_ECP_DP_BP512R1_ENABLED)   &&                  \
    !defined(BSCOMPTLS_ECP_DP_SECP192K1_ENABLED) &&                  \
    !defined(BSCOMPTLS_ECP_DP_SECP224K1_ENABLED) &&                  \
    !defined(BSCOMPTLS_ECP_DP_SECP256K1_ENABLED) ) )
#error "BSCOMPTLS_ECP_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_ENTROPY_C) && (!defined(BSCOMPTLS_SHA512_C) &&      \
                                    !defined(BSCOMPTLS_SHA256_C))
#error "BSCOMPTLS_ENTROPY_C defined, but not all prerequisites"
#endif
#if defined(BSCOMPTLS_ENTROPY_C) && defined(BSCOMPTLS_SHA512_C) &&         \
    defined(BSCOMPTLS_CTR_DRBG_ENTROPY_LEN) && (BSCOMPTLS_CTR_DRBG_ENTROPY_LEN > 64)
#error "BSCOMPTLS_CTR_DRBG_ENTROPY_LEN value too high"
#endif
#if defined(BSCOMPTLS_ENTROPY_C) &&                                            \
    ( !defined(BSCOMPTLS_SHA512_C) || defined(BSCOMPTLS_ENTROPY_FORCE_SHA256) ) \
    && defined(BSCOMPTLS_CTR_DRBG_ENTROPY_LEN) && (BSCOMPTLS_CTR_DRBG_ENTROPY_LEN > 32)
#error "BSCOMPTLS_CTR_DRBG_ENTROPY_LEN value too high"
#endif
#if defined(BSCOMPTLS_ENTROPY_C) && \
    defined(BSCOMPTLS_ENTROPY_FORCE_SHA256) && !defined(BSCOMPTLS_SHA256_C)
#error "BSCOMPTLS_ENTROPY_FORCE_SHA256 defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_TEST_NULL_ENTROPY) && \
    ( !defined(BSCOMPTLS_ENTROPY_C) || !defined(BSCOMPTLS_NO_DEFAULT_ENTROPY_SOURCES) )
#error "BSCOMPTLS_TEST_NULL_ENTROPY defined, but not all prerequisites"
#endif
#if defined(BSCOMPTLS_TEST_NULL_ENTROPY) && \
     ( defined(BSCOMPTLS_ENTROPY_NV_SEED) || defined(BSCOMPTLS_ENTROPY_HARDWARE_ALT) || \
    defined(BSCOMPTLS_HAVEGE_C) )
#error "BSCOMPTLS_TEST_NULL_ENTROPY defined, but entropy sources too"
#endif

#if defined(BSCOMPTLS_GCM_C) && (                                        \
        !defined(BSCOMPTLS_AES_C) && !defined(BSCOMPTLS_CAMELLIA_C) )
#error "BSCOMPTLS_GCM_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_HAVEGE_C) && !defined(BSCOMPTLS_TIMING_C)
#error "BSCOMPTLS_HAVEGE_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_HMAC_DRBG_C) && !defined(BSCOMPTLS_MD_C)
#error "BSCOMPTLS_HMAC_DRBG_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_KEY_EXCHANGE_ECDH_ECDSA_ENABLED) &&                 \
    ( !defined(BSCOMPTLS_ECDH_C) || !defined(BSCOMPTLS_X509_CRT_PARSE_C) )
#error "BSCOMPTLS_KEY_EXCHANGE_ECDH_ECDSA_ENABLED defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_KEY_EXCHANGE_ECDH_RSA_ENABLED) &&                 \
    ( !defined(BSCOMPTLS_ECDH_C) || !defined(BSCOMPTLS_X509_CRT_PARSE_C) )
#error "BSCOMPTLS_KEY_EXCHANGE_ECDH_RSA_ENABLED defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_KEY_EXCHANGE_DHE_PSK_ENABLED) && !defined(BSCOMPTLS_DHM_C)
#error "BSCOMPTLS_KEY_EXCHANGE_DHE_PSK_ENABLED defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_KEY_EXCHANGE_ECDHE_PSK_ENABLED) &&                     \
    !defined(BSCOMPTLS_ECDH_C)
#error "BSCOMPTLS_KEY_EXCHANGE_ECDHE_PSK_ENABLED defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_KEY_EXCHANGE_DHE_RSA_ENABLED) &&                   \
    ( !defined(BSCOMPTLS_DHM_C) || !defined(BSCOMPTLS_RSA_C) ||           \
      !defined(BSCOMPTLS_X509_CRT_PARSE_C) || !defined(BSCOMPTLS_PKCS1_V15) )
#error "BSCOMPTLS_KEY_EXCHANGE_DHE_RSA_ENABLED defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED) &&                 \
    ( !defined(BSCOMPTLS_ECDH_C) || !defined(BSCOMPTLS_RSA_C) ||          \
      !defined(BSCOMPTLS_X509_CRT_PARSE_C) || !defined(BSCOMPTLS_PKCS1_V15) )
#error "BSCOMPTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED) &&                 \
    ( !defined(BSCOMPTLS_ECDH_C) || !defined(BSCOMPTLS_ECDSA_C) ||          \
      !defined(BSCOMPTLS_X509_CRT_PARSE_C) )
#error "BSCOMPTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_KEY_EXCHANGE_RSA_PSK_ENABLED) &&                   \
    ( !defined(BSCOMPTLS_RSA_C) || !defined(BSCOMPTLS_X509_CRT_PARSE_C) || \
      !defined(BSCOMPTLS_PKCS1_V15) )
#error "BSCOMPTLS_KEY_EXCHANGE_RSA_PSK_ENABLED defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_KEY_EXCHANGE_RSA_ENABLED) &&                       \
    ( !defined(BSCOMPTLS_RSA_C) || !defined(BSCOMPTLS_X509_CRT_PARSE_C) || \
      !defined(BSCOMPTLS_PKCS1_V15) )
#error "BSCOMPTLS_KEY_EXCHANGE_RSA_ENABLED defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_KEY_EXCHANGE_ECJPAKE_ENABLED) &&                    \
    ( !defined(BSCOMPTLS_ECJPAKE_C) || !defined(BSCOMPTLS_SHA256_C) ||      \
      !defined(BSCOMPTLS_ECP_DP_SECP256R1_ENABLED) )
#error "BSCOMPTLS_KEY_EXCHANGE_ECJPAKE_ENABLED defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_MEMORY_BUFFER_ALLOC_C) &&                          \
    ( !defined(BSCOMPTLS_PLATFORM_C) || !defined(BSCOMPTLS_PLATFORM_MEMORY) )
#error "BSCOMPTLS_MEMORY_BUFFER_ALLOC_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PADLOCK_C) && !defined(BSCOMPTLS_HAVE_ASM)
#error "BSCOMPTLS_PADLOCK_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PEM_PARSE_C) && !defined(BSCOMPTLS_BASE64_C)
#error "BSCOMPTLS_PEM_PARSE_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PEM_WRITE_C) && !defined(BSCOMPTLS_BASE64_C)
#error "BSCOMPTLS_PEM_WRITE_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PK_C) && \
    ( !defined(BSCOMPTLS_RSA_C) && !defined(BSCOMPTLS_ECP_C) )
#error "BSCOMPTLS_PK_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PK_PARSE_C) && !defined(BSCOMPTLS_PK_C)
#error "BSCOMPTLS_PK_PARSE_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PK_WRITE_C) && !defined(BSCOMPTLS_PK_C)
#error "BSCOMPTLS_PK_WRITE_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PKCS11_C) && !defined(BSCOMPTLS_PK_C)
#error "BSCOMPTLS_PKCS11_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_EXIT_ALT) && !defined(BSCOMPTLS_PLATFORM_C)
#error "BSCOMPTLS_PLATFORM_EXIT_ALT defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_EXIT_MACRO) && !defined(BSCOMPTLS_PLATFORM_C)
#error "BSCOMPTLS_PLATFORM_EXIT_MACRO defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_EXIT_MACRO) &&\
    ( defined(BSCOMPTLS_PLATFORM_STD_EXIT) ||\
        defined(BSCOMPTLS_PLATFORM_EXIT_ALT) )
#error "BSCOMPTLS_PLATFORM_EXIT_MACRO and BSCOMPTLS_PLATFORM_STD_EXIT/BSCOMPTLS_PLATFORM_EXIT_ALT cannot be defined simultaneously"
#endif

#if defined(BSCOMPTLS_PLATFORM_TIME_ALT) &&\
    ( !defined(BSCOMPTLS_PLATFORM_C) ||\
        !defined(BSCOMPTLS_HAVE_TIME) )
#error "BSCOMPTLS_PLATFORM_TIME_ALT defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_TIME_MACRO) &&\
    ( !defined(BSCOMPTLS_PLATFORM_C) ||\
        !defined(BSCOMPTLS_HAVE_TIME) )
#error "BSCOMPTLS_PLATFORM_TIME_MACRO defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_TIME_TYPE_MACRO) &&\
    ( !defined(BSCOMPTLS_PLATFORM_C) ||\
        !defined(BSCOMPTLS_HAVE_TIME) )
#error "BSCOMPTLS_PLATFORM_TIME_TYPE_MACRO defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_TIME_MACRO) &&\
    ( defined(BSCOMPTLS_PLATFORM_STD_TIME) ||\
        defined(BSCOMPTLS_PLATFORM_TIME_ALT) )
#error "BSCOMPTLS_PLATFORM_TIME_MACRO and BSCOMPTLS_PLATFORM_STD_TIME/BSCOMPTLS_PLATFORM_TIME_ALT cannot be defined simultaneously"
#endif

#if defined(BSCOMPTLS_PLATFORM_TIME_TYPE_MACRO) &&\
    ( defined(BSCOMPTLS_PLATFORM_STD_TIME) ||\
        defined(BSCOMPTLS_PLATFORM_TIME_ALT) )
#error "BSCOMPTLS_PLATFORM_TIME_TYPE_MACRO and BSCOMPTLS_PLATFORM_STD_TIME/BSCOMPTLS_PLATFORM_TIME_ALT cannot be defined simultaneously"
#endif

#if defined(BSCOMPTLS_PLATFORM_FPRINTF_ALT) && !defined(BSCOMPTLS_PLATFORM_C)
#error "BSCOMPTLS_PLATFORM_FPRINTF_ALT defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_FPRINTF_MACRO) && !defined(BSCOMPTLS_PLATFORM_C)
#error "BSCOMPTLS_PLATFORM_FPRINTF_MACRO defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_FPRINTF_MACRO) &&\
    ( defined(BSCOMPTLS_PLATFORM_STD_FPRINTF) ||\
        defined(BSCOMPTLS_PLATFORM_FPRINTF_ALT) )
#error "BSCOMPTLS_PLATFORM_FPRINTF_MACRO and BSCOMPTLS_PLATFORM_STD_FPRINTF/BSCOMPTLS_PLATFORM_FPRINTF_ALT cannot be defined simultaneously"
#endif

#if defined(BSCOMPTLS_PLATFORM_FREE_MACRO) &&\
    ( !defined(BSCOMPTLS_PLATFORM_C) || !defined(BSCOMPTLS_PLATFORM_MEMORY) )
#error "BSCOMPTLS_PLATFORM_FREE_MACRO defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_FREE_MACRO) &&\
    defined(BSCOMPTLS_PLATFORM_STD_FREE)
#error "BSCOMPTLS_PLATFORM_FREE_MACRO and BSCOMPTLS_PLATFORM_STD_FREE cannot be defined simultaneously"
#endif

#if defined(BSCOMPTLS_PLATFORM_FREE_MACRO) && !defined(BSCOMPTLS_PLATFORM_CALLOC_MACRO)
#error "BSCOMPTLS_PLATFORM_CALLOC_MACRO must be defined if BSCOMPTLS_PLATFORM_FREE_MACRO is"
#endif

#if defined(BSCOMPTLS_PLATFORM_CALLOC_MACRO) &&\
    ( !defined(BSCOMPTLS_PLATFORM_C) || !defined(BSCOMPTLS_PLATFORM_MEMORY) )
#error "BSCOMPTLS_PLATFORM_CALLOC_MACRO defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_CALLOC_MACRO) &&\
    defined(BSCOMPTLS_PLATFORM_STD_CALLOC)
#error "BSCOMPTLS_PLATFORM_CALLOC_MACRO and BSCOMPTLS_PLATFORM_STD_CALLOC cannot be defined simultaneously"
#endif

#if defined(BSCOMPTLS_PLATFORM_CALLOC_MACRO) && !defined(BSCOMPTLS_PLATFORM_FREE_MACRO)
#error "BSCOMPTLS_PLATFORM_FREE_MACRO must be defined if BSCOMPTLS_PLATFORM_CALLOC_MACRO is"
#endif

#if defined(BSCOMPTLS_PLATFORM_MEMORY) && !defined(BSCOMPTLS_PLATFORM_C)
#error "BSCOMPTLS_PLATFORM_MEMORY defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_PRINTF_ALT) && !defined(BSCOMPTLS_PLATFORM_C)
#error "BSCOMPTLS_PLATFORM_PRINTF_ALT defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_PRINTF_MACRO) && !defined(BSCOMPTLS_PLATFORM_C)
#error "BSCOMPTLS_PLATFORM_PRINTF_MACRO defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_PRINTF_MACRO) &&\
    ( defined(BSCOMPTLS_PLATFORM_STD_PRINTF) ||\
        defined(BSCOMPTLS_PLATFORM_PRINTF_ALT) )
#error "BSCOMPTLS_PLATFORM_PRINTF_MACRO and BSCOMPTLS_PLATFORM_STD_PRINTF/BSCOMPTLS_PLATFORM_PRINTF_ALT cannot be defined simultaneously"
#endif

#if defined(BSCOMPTLS_PLATFORM_SNPRINTF_ALT) && !defined(BSCOMPTLS_PLATFORM_C)
#error "BSCOMPTLS_PLATFORM_SNPRINTF_ALT defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_SNPRINTF_MACRO) && !defined(BSCOMPTLS_PLATFORM_C)
#error "BSCOMPTLS_PLATFORM_SNPRINTF_MACRO defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_SNPRINTF_MACRO) &&\
    ( defined(BSCOMPTLS_PLATFORM_STD_SNPRINTF) ||\
        defined(BSCOMPTLS_PLATFORM_SNPRINTF_ALT) )
#error "BSCOMPTLS_PLATFORM_SNPRINTF_MACRO and BSCOMPTLS_PLATFORM_STD_SNPRINTF/BSCOMPTLS_PLATFORM_SNPRINTF_ALT cannot be defined simultaneously"
#endif

#if defined(BSCOMPTLS_PLATFORM_STD_MEM_HDR) &&\
    !defined(BSCOMPTLS_PLATFORM_NO_STD_FUNCTIONS)
#error "BSCOMPTLS_PLATFORM_STD_MEM_HDR defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_STD_CALLOC) && !defined(BSCOMPTLS_PLATFORM_MEMORY)
#error "BSCOMPTLS_PLATFORM_STD_CALLOC defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_STD_CALLOC) && !defined(BSCOMPTLS_PLATFORM_MEMORY)
#error "BSCOMPTLS_PLATFORM_STD_CALLOC defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_STD_FREE) && !defined(BSCOMPTLS_PLATFORM_MEMORY)
#error "BSCOMPTLS_PLATFORM_STD_FREE defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_STD_EXIT) &&\
    !defined(BSCOMPTLS_PLATFORM_EXIT_ALT)
#error "BSCOMPTLS_PLATFORM_STD_EXIT defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_STD_TIME) &&\
    ( !defined(BSCOMPTLS_PLATFORM_TIME_ALT) ||\
        !defined(BSCOMPTLS_HAVE_TIME) )
#error "BSCOMPTLS_PLATFORM_STD_TIME defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_STD_FPRINTF) &&\
    !defined(BSCOMPTLS_PLATFORM_FPRINTF_ALT)
#error "BSCOMPTLS_PLATFORM_STD_FPRINTF defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_STD_PRINTF) &&\
    !defined(BSCOMPTLS_PLATFORM_PRINTF_ALT)
#error "BSCOMPTLS_PLATFORM_STD_PRINTF defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_STD_SNPRINTF) &&\
    !defined(BSCOMPTLS_PLATFORM_SNPRINTF_ALT)
#error "BSCOMPTLS_PLATFORM_STD_SNPRINTF defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_ENTROPY_NV_SEED) &&\
    ( !defined(BSCOMPTLS_PLATFORM_C) || !defined(BSCOMPTLS_ENTROPY_C) )
#error "BSCOMPTLS_ENTROPY_NV_SEED defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_NV_SEED_ALT) &&\
    !defined(BSCOMPTLS_ENTROPY_NV_SEED)
#error "BSCOMPTLS_PLATFORM_NV_SEED_ALT defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_STD_NV_SEED_READ) &&\
    !defined(BSCOMPTLS_PLATFORM_NV_SEED_ALT)
#error "BSCOMPTLS_PLATFORM_STD_NV_SEED_READ defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_STD_NV_SEED_WRITE) &&\
    !defined(BSCOMPTLS_PLATFORM_NV_SEED_ALT)
#error "BSCOMPTLS_PLATFORM_STD_NV_SEED_WRITE defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_PLATFORM_NV_SEED_READ_MACRO) &&\
    ( defined(BSCOMPTLS_PLATFORM_STD_NV_SEED_READ) ||\
      defined(BSCOMPTLS_PLATFORM_NV_SEED_ALT) )
#error "BSCOMPTLS_PLATFORM_NV_SEED_READ_MACRO and BSCOMPTLS_PLATFORM_STD_NV_SEED_READ cannot be defined simultaneously"
#endif

#if defined(BSCOMPTLS_PLATFORM_NV_SEED_WRITE_MACRO) &&\
    ( defined(BSCOMPTLS_PLATFORM_STD_NV_SEED_WRITE) ||\
      defined(BSCOMPTLS_PLATFORM_NV_SEED_ALT) )
#error "BSCOMPTLS_PLATFORM_NV_SEED_WRITE_MACRO and BSCOMPTLS_PLATFORM_STD_NV_SEED_WRITE cannot be defined simultaneously"
#endif

#if defined(BSCOMPTLS_RSA_C) && ( !defined(BSCOMPTLS_BIGNUM_C) ||         \
    !defined(BSCOMPTLS_OID_C) )
#error "BSCOMPTLS_RSA_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_RSA_C) && ( !defined(BSCOMPTLS_PKCS1_V21) &&         \
    !defined(BSCOMPTLS_PKCS1_V15) )
#error "BSCOMPTLS_RSA_C defined, but none of the PKCS1 versions enabled"
#endif

#if defined(BSCOMPTLS_X509_RSASSA_PSS_SUPPORT) &&                        \
    ( !defined(BSCOMPTLS_RSA_C) || !defined(BSCOMPTLS_PKCS1_V21) )
#error "BSCOMPTLS_X509_RSASSA_PSS_SUPPORT defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_SSL_PROTO_SSL3) && ( !defined(BSCOMPTLS_MD5_C) ||     \
    !defined(BSCOMPTLS_SHA1_C) )
#error "BSCOMPTLS_SSL_PROTO_SSL3 defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_SSL_PROTO_TLS1) && ( !defined(BSCOMPTLS_MD5_C) ||     \
    !defined(BSCOMPTLS_SHA1_C) )
#error "BSCOMPTLS_SSL_PROTO_TLS1 defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_SSL_PROTO_TLS1_1) && ( !defined(BSCOMPTLS_MD5_C) ||     \
    !defined(BSCOMPTLS_SHA1_C) )
#error "BSCOMPTLS_SSL_PROTO_TLS1_1 defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_SSL_PROTO_TLS1_2) && ( !defined(BSCOMPTLS_SHA1_C) &&     \
    !defined(BSCOMPTLS_SHA256_C) && !defined(BSCOMPTLS_SHA512_C) )
#error "BSCOMPTLS_SSL_PROTO_TLS1_2 defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_SSL_PROTO_DTLS)     && \
    !defined(BSCOMPTLS_SSL_PROTO_TLS1_1)  && \
    !defined(BSCOMPTLS_SSL_PROTO_TLS1_2)
#error "BSCOMPTLS_SSL_PROTO_DTLS defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_SSL_CLI_C) && !defined(BSCOMPTLS_SSL_TLS_C)
#error "BSCOMPTLS_SSL_CLI_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_SSL_TLS_C) && ( !defined(BSCOMPTLS_CIPHER_C) ||     \
    !defined(BSCOMPTLS_MD_C) )
#error "BSCOMPTLS_SSL_TLS_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_SSL_SRV_C) && !defined(BSCOMPTLS_SSL_TLS_C)
#error "BSCOMPTLS_SSL_SRV_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_SSL_TLS_C) && (!defined(BSCOMPTLS_SSL_PROTO_SSL3) && \
    !defined(BSCOMPTLS_SSL_PROTO_TLS1) && !defined(BSCOMPTLS_SSL_PROTO_TLS1_1) && \
    !defined(BSCOMPTLS_SSL_PROTO_TLS1_2))
#error "BSCOMPTLS_SSL_TLS_C defined, but no protocols are active"
#endif

#if defined(BSCOMPTLS_SSL_TLS_C) && (defined(BSCOMPTLS_SSL_PROTO_SSL3) && \
    defined(BSCOMPTLS_SSL_PROTO_TLS1_1) && !defined(BSCOMPTLS_SSL_PROTO_TLS1))
#error "Illegal protocol selection"
#endif

#if defined(BSCOMPTLS_SSL_TLS_C) && (defined(BSCOMPTLS_SSL_PROTO_TLS1) && \
    defined(BSCOMPTLS_SSL_PROTO_TLS1_2) && !defined(BSCOMPTLS_SSL_PROTO_TLS1_1))
#error "Illegal protocol selection"
#endif

#if defined(BSCOMPTLS_SSL_TLS_C) && (defined(BSCOMPTLS_SSL_PROTO_SSL3) && \
    defined(BSCOMPTLS_SSL_PROTO_TLS1_2) && (!defined(BSCOMPTLS_SSL_PROTO_TLS1) || \
    !defined(BSCOMPTLS_SSL_PROTO_TLS1_1)))
#error "Illegal protocol selection"
#endif

#if defined(BSCOMPTLS_SSL_DTLS_HELLO_VERIFY) && !defined(BSCOMPTLS_SSL_PROTO_DTLS)
#error "BSCOMPTLS_SSL_DTLS_HELLO_VERIFY  defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_SSL_DTLS_CLIENT_PORT_REUSE) && \
    !defined(BSCOMPTLS_SSL_DTLS_HELLO_VERIFY)
#error "BSCOMPTLS_SSL_DTLS_CLIENT_PORT_REUSE  defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_SSL_DTLS_ANTI_REPLAY) &&                              \
    ( !defined(BSCOMPTLS_SSL_TLS_C) || !defined(BSCOMPTLS_SSL_PROTO_DTLS) )
#error "BSCOMPTLS_SSL_DTLS_ANTI_REPLAY  defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_SSL_DTLS_BADMAC_LIMIT) &&                              \
    ( !defined(BSCOMPTLS_SSL_TLS_C) || !defined(BSCOMPTLS_SSL_PROTO_DTLS) )
#error "BSCOMPTLS_SSL_DTLS_BADMAC_LIMIT  defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_SSL_ENCRYPT_THEN_MAC) &&   \
    !defined(BSCOMPTLS_SSL_PROTO_TLS1)   &&      \
    !defined(BSCOMPTLS_SSL_PROTO_TLS1_1) &&      \
    !defined(BSCOMPTLS_SSL_PROTO_TLS1_2)
#error "BSCOMPTLS_SSL_ENCRYPT_THEN_MAC defined, but not all prerequsites"
#endif

#if defined(BSCOMPTLS_SSL_EXTENDED_MASTER_SECRET) && \
    !defined(BSCOMPTLS_SSL_PROTO_TLS1)   &&          \
    !defined(BSCOMPTLS_SSL_PROTO_TLS1_1) &&          \
    !defined(BSCOMPTLS_SSL_PROTO_TLS1_2)
#error "BSCOMPTLS_SSL_EXTENDED_MASTER_SECRET defined, but not all prerequsites"
#endif

#if defined(BSCOMPTLS_SSL_TICKET_C) && !defined(BSCOMPTLS_CIPHER_C)
#error "BSCOMPTLS_SSL_TICKET_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_SSL_CBC_RECORD_SPLITTING) && \
    !defined(BSCOMPTLS_SSL_PROTO_SSL3) && !defined(BSCOMPTLS_SSL_PROTO_TLS1)
#error "BSCOMPTLS_SSL_CBC_RECORD_SPLITTING defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_SSL_SERVER_NAME_INDICATION) && \
        !defined(BSCOMPTLS_X509_CRT_PARSE_C)
#error "BSCOMPTLS_SSL_SERVER_NAME_INDICATION defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_THREADING_PTHREAD)
#if !defined(BSCOMPTLS_THREADING_C) || defined(BSCOMPTLS_THREADING_IMPL)
#error "BSCOMPTLS_THREADING_PTHREAD defined, but not all prerequisites"
#endif
#define BSCOMPTLS_THREADING_IMPL
#endif

#if defined(BSCOMPTLS_THREADING_ALT)
#if !defined(BSCOMPTLS_THREADING_C) || defined(BSCOMPTLS_THREADING_IMPL)
#error "BSCOMPTLS_THREADING_ALT defined, but not all prerequisites"
#endif
#define BSCOMPTLS_THREADING_IMPL
#endif

#if defined(BSCOMPTLS_THREADING_C) && !defined(BSCOMPTLS_THREADING_IMPL)
#error "BSCOMPTLS_THREADING_C defined, single threading implementation required"
#endif
#undef BSCOMPTLS_THREADING_IMPL

#if defined(BSCOMPTLS_VERSION_FEATURES) && !defined(BSCOMPTLS_VERSION_C)
#error "BSCOMPTLS_VERSION_FEATURES defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_X509_USE_C) && ( !defined(BSCOMPTLS_BIGNUM_C) ||  \
    !defined(BSCOMPTLS_OID_C) || !defined(BSCOMPTLS_ASN1_PARSE_C) ||      \
    !defined(BSCOMPTLS_PK_PARSE_C) )
#error "BSCOMPTLS_X509_USE_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_X509_CREATE_C) && ( !defined(BSCOMPTLS_BIGNUM_C) ||  \
    !defined(BSCOMPTLS_OID_C) || !defined(BSCOMPTLS_ASN1_WRITE_C) ||       \
    !defined(BSCOMPTLS_PK_WRITE_C) )
#error "BSCOMPTLS_X509_CREATE_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_X509_CRT_PARSE_C) && ( !defined(BSCOMPTLS_X509_USE_C) )
#error "BSCOMPTLS_X509_CRT_PARSE_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_X509_CRL_PARSE_C) && ( !defined(BSCOMPTLS_X509_USE_C) )
#error "BSCOMPTLS_X509_CRL_PARSE_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_X509_CSR_PARSE_C) && ( !defined(BSCOMPTLS_X509_USE_C) )
#error "BSCOMPTLS_X509_CSR_PARSE_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_X509_CRT_WRITE_C) && ( !defined(BSCOMPTLS_X509_CREATE_C) )
#error "BSCOMPTLS_X509_CRT_WRITE_C defined, but not all prerequisites"
#endif

#if defined(BSCOMPTLS_X509_CSR_WRITE_C) && ( !defined(BSCOMPTLS_X509_CREATE_C) )
#error "BSCOMPTLS_X509_CSR_WRITE_C defined, but not all prerequisites"
#endif

/*
 * Avoid warning from -pedantic. This is a convenient place for this
 * workaround since this is included by every single file before the
 * #if defined(BSCOMPTLS_xxx_C) that results in emtpy translation units.
 */
typedef int bscomptls_iso_c_forbids_empty_translation_units;

#endif /* BSCOMPTLS_CHECK_CONFIG_H */
