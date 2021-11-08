/**
 * \file ctr_drbg.h
 *
 * \brief CTR_DRBG based on AES-256 (NIST SP 800-90)
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
#ifndef BSCOMPTLS_CTR_DRBG_H
#define BSCOMPTLS_CTR_DRBG_H

#include "aes.h"

#if defined(BSCOMPTLS_THREADING_C)
#include "mbedtls/threading.h"
#endif

#define BSCOMPTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED        -0x0034  /**< The entropy source failed. */
#define BSCOMPTLS_ERR_CTR_DRBG_REQUEST_TOO_BIG              -0x0036  /**< Too many random requested in single call. */
#define BSCOMPTLS_ERR_CTR_DRBG_INPUT_TOO_BIG                -0x0038  /**< Input too large (Entropy + additional). */
#define BSCOMPTLS_ERR_CTR_DRBG_FILE_IO_ERROR                -0x003A  /**< Read/write error in file. */

#define BSCOMPTLS_CTR_DRBG_BLOCKSIZE          16      /**< Block size used by the cipher                  */
#define BSCOMPTLS_CTR_DRBG_KEYSIZE            32      /**< Key size used by the cipher                    */
#define BSCOMPTLS_CTR_DRBG_KEYBITS            ( BSCOMPTLS_CTR_DRBG_KEYSIZE * 8 )
#define BSCOMPTLS_CTR_DRBG_SEEDLEN            ( BSCOMPTLS_CTR_DRBG_KEYSIZE + BSCOMPTLS_CTR_DRBG_BLOCKSIZE )
                                            /**< The seed length (counter + AES key)            */

/**
 * \name SECTION: Module settings
 *
 * The configuration options you can set for this module are in this section.
 * Either change them in config.h or define them on the compiler command line.
 * \{
 */

#if !defined(BSCOMPTLS_CTR_DRBG_ENTROPY_LEN)
#if defined(BSCOMPTLS_SHA512_C) && !defined(BSCOMPTLS_ENTROPY_FORCE_SHA256)
#define BSCOMPTLS_CTR_DRBG_ENTROPY_LEN        48      /**< Amount of entropy used per seed by default (48 with SHA-512, 32 with SHA-256) */
#else
#define BSCOMPTLS_CTR_DRBG_ENTROPY_LEN        32      /**< Amount of entropy used per seed by default (48 with SHA-512, 32 with SHA-256) */
#endif
#endif

#if !defined(BSCOMPTLS_CTR_DRBG_RESEED_INTERVAL)
#define BSCOMPTLS_CTR_DRBG_RESEED_INTERVAL    10000   /**< Interval before reseed is performed by default */
#endif

#if !defined(BSCOMPTLS_CTR_DRBG_MAX_INPUT)
#define BSCOMPTLS_CTR_DRBG_MAX_INPUT          256     /**< Maximum number of additional input bytes */
#endif

#if !defined(BSCOMPTLS_CTR_DRBG_MAX_REQUEST)
#define BSCOMPTLS_CTR_DRBG_MAX_REQUEST        1024    /**< Maximum number of requested bytes per call */
#endif

#if !defined(BSCOMPTLS_CTR_DRBG_MAX_SEED_INPUT)
#define BSCOMPTLS_CTR_DRBG_MAX_SEED_INPUT     384     /**< Maximum size of (re)seed buffer */
#endif

/* \} name SECTION: Module settings */

#define BSCOMPTLS_CTR_DRBG_PR_OFF             0       /**< No prediction resistance       */
#define BSCOMPTLS_CTR_DRBG_PR_ON              1       /**< Prediction resistance enabled  */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          CTR_DRBG context structure
 */
typedef struct
{
    unsigned char counter[16];  /*!<  counter (V)       */
    int reseed_counter;         /*!<  reseed counter    */
    int prediction_resistance;  /*!<  enable prediction resistance (Automatic
                                      reseed before every random generation)  */
    size_t entropy_len;         /*!<  amount of entropy grabbed on each
                                      (re)seed          */
    int reseed_interval;        /*!<  reseed interval   */

    bscomptls_aes_context aes_ctx;        /*!<  AES context       */

    /*
     * Callbacks (Entropy)
     */
    int (*f_entropy)(void *, unsigned char *, size_t);

    void *p_entropy;            /*!<  context for the entropy function */

#if defined(BSCOMPTLS_THREADING_C)
    bscomptls_threading_mutex_t mutex;
#endif
}
bscomptls_ctr_drbg_context;

/**
 * \brief               CTR_DRBG context initialization
 *                      Makes the context ready for bscomptls_ctr_drbg_seed() or
 *                      bscomptls_ctr_drbg_free().
 *
 * \param ctx           CTR_DRBG context to be initialized
 */
void bscomptls_ctr_drbg_init( bscomptls_ctr_drbg_context *ctx );

/**
 * \brief               CTR_DRBG initial seeding
 *                      Seed and setup entropy source for future reseeds.
 *
 * Note: Personalization data can be provided in addition to the more generic
 *       entropy source to make this instantiation as unique as possible.
 *
 * \param ctx           CTR_DRBG context to be seeded
 * \param f_entropy     Entropy callback (p_entropy, buffer to fill, buffer
 *                      length)
 * \param p_entropy     Entropy context
 * \param custom        Personalization data (Device specific identifiers)
 *                      (Can be NULL)
 * \param len           Length of personalization data
 *
 * \return              0 if successful, or
 *                      BSCOMPTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED
 */
int bscomptls_ctr_drbg_seed( bscomptls_ctr_drbg_context *ctx,
                   int (*f_entropy)(void *, unsigned char *, size_t),
                   void *p_entropy,
                   const unsigned char *custom,
                   size_t len );

/**
 * \brief               Clear CTR_CRBG context data
 *
 * \param ctx           CTR_DRBG context to clear
 */
void bscomptls_ctr_drbg_free( bscomptls_ctr_drbg_context *ctx );

/**
 * \brief               Enable / disable prediction resistance (Default: Off)
 *
 * Note: If enabled, entropy is used for ctx->entropy_len before each call!
 *       Only use this if you have ample supply of good entropy!
 *
 * \param ctx           CTR_DRBG context
 * \param resistance    BSCOMPTLS_CTR_DRBG_PR_ON or BSCOMPTLS_CTR_DRBG_PR_OFF
 */
void bscomptls_ctr_drbg_set_prediction_resistance( bscomptls_ctr_drbg_context *ctx,
                                         int resistance );

/**
 * \brief               Set the amount of entropy grabbed on each (re)seed
 *                      (Default: BSCOMPTLS_CTR_DRBG_ENTROPY_LEN)
 *
 * \param ctx           CTR_DRBG context
 * \param len           Amount of entropy to grab
 */
void bscomptls_ctr_drbg_set_entropy_len( bscomptls_ctr_drbg_context *ctx,
                               size_t len );

/**
 * \brief               Set the reseed interval
 *                      (Default: BSCOMPTLS_CTR_DRBG_RESEED_INTERVAL)
 *
 * \param ctx           CTR_DRBG context
 * \param interval      Reseed interval
 */
void bscomptls_ctr_drbg_set_reseed_interval( bscomptls_ctr_drbg_context *ctx,
                                   int interval );

/**
 * \brief               CTR_DRBG reseeding (extracts data from entropy source)
 *
 * \param ctx           CTR_DRBG context
 * \param additional    Additional data to add to state (Can be NULL)
 * \param len           Length of additional data
 *
 * \return              0 if successful, or
 *                      BSCOMPTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED
 */
int bscomptls_ctr_drbg_reseed( bscomptls_ctr_drbg_context *ctx,
                     const unsigned char *additional, size_t len );

/**
 * \brief               CTR_DRBG update state
 *
 * \param ctx           CTR_DRBG context
 * \param additional    Additional data to update state with
 * \param add_len       Length of additional data
 *
 * \note                If add_len is greater than BSCOMPTLS_CTR_DRBG_MAX_SEED_INPUT,
 *                      only the first BSCOMPTLS_CTR_DRBG_MAX_SEED_INPUT bytes are used,
 *                      the remaining ones are silently discarded.
 */
void bscomptls_ctr_drbg_update( bscomptls_ctr_drbg_context *ctx,
                      const unsigned char *additional, size_t add_len );

/**
 * \brief               CTR_DRBG generate random with additional update input
 *
 * Note: Automatically reseeds if reseed_counter is reached.
 *
 * \param p_rng         CTR_DRBG context
 * \param output        Buffer to fill
 * \param output_len    Length of the buffer
 * \param additional    Additional data to update with (Can be NULL)
 * \param add_len       Length of additional data
 *
 * \return              0 if successful, or
 *                      BSCOMPTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED, or
 *                      BSCOMPTLS_ERR_CTR_DRBG_REQUEST_TOO_BIG
 */
int bscomptls_ctr_drbg_random_with_add( void *p_rng,
                              unsigned char *output, size_t output_len,
                              const unsigned char *additional, size_t add_len );

/**
 * \brief               CTR_DRBG generate random
 *
 * Note: Automatically reseeds if reseed_counter is reached.
 *
 * \param p_rng         CTR_DRBG context
 * \param output        Buffer to fill
 * \param output_len    Length of the buffer
 *
 * \return              0 if successful, or
 *                      BSCOMPTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED, or
 *                      BSCOMPTLS_ERR_CTR_DRBG_REQUEST_TOO_BIG
 */
int bscomptls_ctr_drbg_random( void *p_rng,
                     unsigned char *output, size_t output_len );

#if defined(BSCOMPTLS_FS_IO)
/**
 * \brief               Write a seed file
 *
 * \param ctx           CTR_DRBG context
 * \param path          Name of the file
 *
 * \return              0 if successful,
 *                      BSCOMPTLS_ERR_CTR_DRBG_FILE_IO_ERROR on file error, or
 *                      BSCOMPTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED
 */
int bscomptls_ctr_drbg_write_seed_file( bscomptls_ctr_drbg_context *ctx, const char *path );

/**
 * \brief               Read and update a seed file. Seed is added to this
 *                      instance
 *
 * \param ctx           CTR_DRBG context
 * \param path          Name of the file
 *
 * \return              0 if successful,
 *                      BSCOMPTLS_ERR_CTR_DRBG_FILE_IO_ERROR on file error,
 *                      BSCOMPTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED or
 *                      BSCOMPTLS_ERR_CTR_DRBG_INPUT_TOO_BIG
 */
int bscomptls_ctr_drbg_update_seed_file( bscomptls_ctr_drbg_context *ctx, const char *path );
#endif /* BSCOMPTLS_FS_IO */

/**
 * \brief               Checkup routine
 *
 * \return              0 if successful, or 1 if the test failed
 */
int bscomptls_ctr_drbg_self_test( int verbose );

/* Internal functions (do not call directly) */
int bscomptls_ctr_drbg_seed_entropy_len( bscomptls_ctr_drbg_context *,
                               int (*)(void *, unsigned char *, size_t), void *,
                               const unsigned char *, size_t, size_t );

#ifdef __cplusplus
}
#endif

#endif /* ctr_drbg.h */
