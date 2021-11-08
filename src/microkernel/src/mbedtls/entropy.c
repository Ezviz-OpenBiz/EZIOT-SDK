/*
 *  Entropy accumulator implementation
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

#if !defined(BSCOMPTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include BSCOMPTLS_CONFIG_FILE
#endif

#if defined(BSCOMPTLS_ENTROPY_C)

#if defined(BSCOMPTLS_TEST_NULL_ENTROPY)
#warning "**** WARNING!  BSCOMPTLS_TEST_NULL_ENTROPY defined! "
#warning "**** THIS BUILD HAS NO DEFINED ENTROPY SOURCES "
#warning "**** THIS BUILD IS *NOT* SUITABLE FOR PRODUCTION USE "
#endif

#include "mbedtls/entropy.h"
#include "mbedtls/entropy_poll.h"

#include <string.h>

#if defined(BSCOMPTLS_FS_IO)
#include <stdio.h>
#endif

#if defined(BSCOMPTLS_ENTROPY_NV_SEED)
#include "mbedtls/platform.h"
#endif

#if defined(BSCOMPTLS_SELF_TEST)
#if defined(BSCOMPTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define bscomptls_printf     printf
#endif /* BSCOMPTLS_PLATFORM_C */
#endif /* BSCOMPTLS_SELF_TEST */

#if defined(BSCOMPTLS_HAVEGE_C)
#include "mbedtls/havege.h"
#endif

/* Implementation that should never be optimized out by the compiler */
static void bscomptls_zeroize( void *v, size_t n ) {
    volatile unsigned char *p = v; while( n-- ) *p++ = 0;
}

#define ENTROPY_MAX_LOOP    256     /**< Maximum amount to loop before error */

void bscomptls_entropy_init( bscomptls_entropy_context *ctx )
{
    memset( ctx, 0, sizeof(bscomptls_entropy_context) );

#if defined(BSCOMPTLS_THREADING_C)
    bscomptls_mutex_init( &ctx->mutex );
#endif

#if defined(BSCOMPTLS_ENTROPY_SHA512_ACCUMULATOR)
    bscomptls_sha512_starts( &ctx->accumulator, 0 );
#else
    bscomptls_sha256_starts( &ctx->accumulator, 0 );
#endif
#if defined(BSCOMPTLS_HAVEGE_C)
    bscomptls_havege_init( &ctx->havege_data );
#endif

#if defined(BSCOMPTLS_TEST_NULL_ENTROPY)
    bscomptls_entropy_add_source( ctx, bscomptls_null_entropy_poll, NULL,
                                1, BSCOMPTLS_ENTROPY_SOURCE_STRONG );
#endif

#if !defined(BSCOMPTLS_NO_DEFAULT_ENTROPY_SOURCES)
#if !defined(BSCOMPTLS_NO_PLATFORM_ENTROPY)
    bscomptls_entropy_add_source( ctx, bscomptls_platform_entropy_poll, NULL,
                                BSCOMPTLS_ENTROPY_MIN_PLATFORM,
                                BSCOMPTLS_ENTROPY_SOURCE_STRONG );
#endif
#if defined(BSCOMPTLS_TIMING_C)
    bscomptls_entropy_add_source( ctx, bscomptls_hardclock_poll, NULL,
                                BSCOMPTLS_ENTROPY_MIN_HARDCLOCK,
                                BSCOMPTLS_ENTROPY_SOURCE_WEAK );
#endif
#if defined(BSCOMPTLS_HAVEGE_C)
    bscomptls_entropy_add_source( ctx, bscomptls_havege_poll, &ctx->havege_data,
                                BSCOMPTLS_ENTROPY_MIN_HAVEGE,
                                BSCOMPTLS_ENTROPY_SOURCE_STRONG );
#endif
#if defined(BSCOMPTLS_ENTROPY_HARDWARE_ALT)
    bscomptls_entropy_add_source( ctx, bscomptls_hardware_poll, NULL,
                                BSCOMPTLS_ENTROPY_MIN_HARDWARE,
                                BSCOMPTLS_ENTROPY_SOURCE_STRONG );
#endif
#if defined(BSCOMPTLS_ENTROPY_NV_SEED)
    bscomptls_entropy_add_source( ctx, bscomptls_nv_seed_poll, NULL,
                                BSCOMPTLS_ENTROPY_BLOCK_SIZE,
                                BSCOMPTLS_ENTROPY_SOURCE_STRONG );
#endif
#endif /* BSCOMPTLS_NO_DEFAULT_ENTROPY_SOURCES */
}

void bscomptls_entropy_free( bscomptls_entropy_context *ctx )
{
#if defined(BSCOMPTLS_HAVEGE_C)
    bscomptls_havege_free( &ctx->havege_data );
#endif
#if defined(BSCOMPTLS_THREADING_C)
    bscomptls_mutex_free( &ctx->mutex );
#endif
    bscomptls_zeroize( ctx, sizeof( bscomptls_entropy_context ) );
}

int bscomptls_entropy_add_source( bscomptls_entropy_context *ctx,
                        bscomptls_entropy_f_source_ptr f_source, void *p_source,
                        size_t threshold, int strong )
{
    int index, ret = 0;

#if defined(BSCOMPTLS_THREADING_C)
    if( ( ret = bscomptls_mutex_lock( &ctx->mutex ) ) != 0 )
        return( ret );
#endif

    index = ctx->source_count;
    if( index >= BSCOMPTLS_ENTROPY_MAX_SOURCES )
    {
        ret = BSCOMPTLS_ERR_ENTROPY_MAX_SOURCES;
        goto exit;
    }

    ctx->source[index].f_source  = f_source;
    ctx->source[index].p_source  = p_source;
    ctx->source[index].threshold = threshold;
    ctx->source[index].strong    = strong;

    ctx->source_count++;

exit:
#if defined(BSCOMPTLS_THREADING_C)
    if( bscomptls_mutex_unlock( &ctx->mutex ) != 0 )
        return( BSCOMPTLS_ERR_THREADING_MUTEX_ERROR );
#endif

    return( ret );
}

/*
 * Entropy accumulator update
 */
static int entropy_update( bscomptls_entropy_context *ctx, unsigned char source_id,
                           const unsigned char *data, size_t len )
{
    unsigned char header[2];
    unsigned char tmp[BSCOMPTLS_ENTROPY_BLOCK_SIZE];
    size_t use_len = len;
    const unsigned char *p = data;

    if( use_len > BSCOMPTLS_ENTROPY_BLOCK_SIZE )
    {
#if defined(BSCOMPTLS_ENTROPY_SHA512_ACCUMULATOR)
        bscomptls_sha512( data, len, tmp, 0 );
#else
        bscomptls_sha256( data, len, tmp, 0 );
#endif
        p = tmp;
        use_len = BSCOMPTLS_ENTROPY_BLOCK_SIZE;
    }

    header[0] = source_id;
    header[1] = use_len & 0xFF;

#if defined(BSCOMPTLS_ENTROPY_SHA512_ACCUMULATOR)
    bscomptls_sha512_update( &ctx->accumulator, header, 2 );
    bscomptls_sha512_update( &ctx->accumulator, p, use_len );
#else
    bscomptls_sha256_update( &ctx->accumulator, header, 2 );
    bscomptls_sha256_update( &ctx->accumulator, p, use_len );
#endif

    return( 0 );
}

int bscomptls_entropy_update_manual( bscomptls_entropy_context *ctx,
                           const unsigned char *data, size_t len )
{
    int ret;

#if defined(BSCOMPTLS_THREADING_C)
    if( ( ret = bscomptls_mutex_lock( &ctx->mutex ) ) != 0 )
        return( ret );
#endif

    ret = entropy_update( ctx, BSCOMPTLS_ENTROPY_SOURCE_MANUAL, data, len );

#if defined(BSCOMPTLS_THREADING_C)
    if( bscomptls_mutex_unlock( &ctx->mutex ) != 0 )
        return( BSCOMPTLS_ERR_THREADING_MUTEX_ERROR );
#endif

    return( ret );
}

/*
 * Run through the different sources to add entropy to our accumulator
 */
static int entropy_gather_internal( bscomptls_entropy_context *ctx )
{
    int ret, i, have_one_strong = 0;
    unsigned char buf[BSCOMPTLS_ENTROPY_MAX_GATHER];
    size_t olen;

    if( ctx->source_count == 0 )
        return( BSCOMPTLS_ERR_ENTROPY_NO_SOURCES_DEFINED );

    /*
     * Run through our entropy sources
     */
    for( i = 0; i < ctx->source_count; i++ )
    {
        if( ctx->source[i].strong == BSCOMPTLS_ENTROPY_SOURCE_STRONG )
            have_one_strong = 1;

        olen = 0;
        if( ( ret = ctx->source[i].f_source( ctx->source[i].p_source,
                        buf, BSCOMPTLS_ENTROPY_MAX_GATHER, &olen ) ) != 0 )
        {
            return( ret );
        }

        /*
         * Add if we actually gathered something
         */
        if( olen > 0 )
        {
            entropy_update( ctx, (unsigned char) i, buf, olen );
            ctx->source[i].size += olen;
        }
    }

    if( have_one_strong == 0 )
        return( BSCOMPTLS_ERR_ENTROPY_NO_STRONG_SOURCE );

    return( 0 );
}

/*
 * Thread-safe wrapper for entropy_gather_internal()
 */
int bscomptls_entropy_gather( bscomptls_entropy_context *ctx )
{
    int ret;

#if defined(BSCOMPTLS_THREADING_C)
    if( ( ret = bscomptls_mutex_lock( &ctx->mutex ) ) != 0 )
        return( ret );
#endif

    ret = entropy_gather_internal( ctx );

#if defined(BSCOMPTLS_THREADING_C)
    if( bscomptls_mutex_unlock( &ctx->mutex ) != 0 )
        return( BSCOMPTLS_ERR_THREADING_MUTEX_ERROR );
#endif

    return( ret );
}

int bscomptls_entropy_func( void *data, unsigned char *output, size_t len )
{
    int ret, count = 0, i, done;
    bscomptls_entropy_context *ctx = (bscomptls_entropy_context *) data;
    unsigned char buf[BSCOMPTLS_ENTROPY_BLOCK_SIZE];

    if( len > BSCOMPTLS_ENTROPY_BLOCK_SIZE )
        return( BSCOMPTLS_ERR_ENTROPY_SOURCE_FAILED );

#if defined(BSCOMPTLS_ENTROPY_NV_SEED)
    /* Update the NV entropy seed before generating any entropy for outside
     * use.
     */
    if( ctx->initial_entropy_run == 0 )
    {
        ctx->initial_entropy_run = 1;
        if( ( ret = bscomptls_entropy_update_nv_seed( ctx ) ) != 0 )
            return( ret );
    }
#endif

#if defined(BSCOMPTLS_THREADING_C)
    if( ( ret = bscomptls_mutex_lock( &ctx->mutex ) ) != 0 )
        return( ret );
#endif

    /*
     * Always gather extra entropy before a call
     */
    do
    {
        if( count++ > ENTROPY_MAX_LOOP )
        {
            ret = BSCOMPTLS_ERR_ENTROPY_SOURCE_FAILED;
            goto exit;
        }

        if( ( ret = entropy_gather_internal( ctx ) ) != 0 )
            goto exit;

        done = 1;
        for( i = 0; i < ctx->source_count; i++ )
            if( ctx->source[i].size < ctx->source[i].threshold )
                done = 0;
    }
    while( ! done );

    memset( buf, 0, BSCOMPTLS_ENTROPY_BLOCK_SIZE );

#if defined(BSCOMPTLS_ENTROPY_SHA512_ACCUMULATOR)
    bscomptls_sha512_finish( &ctx->accumulator, buf );

    /*
     * Reset accumulator and counters and recycle existing entropy
     */
    memset( &ctx->accumulator, 0, sizeof( bscomptls_sha512_context ) );
    bscomptls_sha512_starts( &ctx->accumulator, 0 );
    bscomptls_sha512_update( &ctx->accumulator, buf, BSCOMPTLS_ENTROPY_BLOCK_SIZE );

    /*
     * Perform second SHA-512 on entropy
     */
    bscomptls_sha512( buf, BSCOMPTLS_ENTROPY_BLOCK_SIZE, buf, 0 );
#else /* BSCOMPTLS_ENTROPY_SHA512_ACCUMULATOR */
    bscomptls_sha256_finish( &ctx->accumulator, buf );

    /*
     * Reset accumulator and counters and recycle existing entropy
     */
    memset( &ctx->accumulator, 0, sizeof( bscomptls_sha256_context ) );
    bscomptls_sha256_starts( &ctx->accumulator, 0 );
    bscomptls_sha256_update( &ctx->accumulator, buf, BSCOMPTLS_ENTROPY_BLOCK_SIZE );

    /*
     * Perform second SHA-256 on entropy
     */
    bscomptls_sha256( buf, BSCOMPTLS_ENTROPY_BLOCK_SIZE, buf, 0 );
#endif /* BSCOMPTLS_ENTROPY_SHA512_ACCUMULATOR */

    for( i = 0; i < ctx->source_count; i++ )
        ctx->source[i].size = 0;

    memcpy( output, buf, len );

    ret = 0;

exit:
#if defined(BSCOMPTLS_THREADING_C)
    if( bscomptls_mutex_unlock( &ctx->mutex ) != 0 )
        return( BSCOMPTLS_ERR_THREADING_MUTEX_ERROR );
#endif

    return( ret );
}

#if defined(BSCOMPTLS_ENTROPY_NV_SEED)
int bscomptls_entropy_update_nv_seed( bscomptls_entropy_context *ctx )
{
    int ret = BSCOMPTLS_ERR_ENTROPY_FILE_IO_ERROR;
    unsigned char buf[ BSCOMPTLS_ENTROPY_MAX_SEED_SIZE ];

    /* Read new seed  and write it to NV */
    if( ( ret = bscomptls_entropy_func( ctx, buf, BSCOMPTLS_ENTROPY_BLOCK_SIZE ) ) != 0 )
        return( ret );

    if( bscomptls_nv_seed_write( buf, BSCOMPTLS_ENTROPY_BLOCK_SIZE ) < 0 )
        return( BSCOMPTLS_ERR_ENTROPY_FILE_IO_ERROR );

    /* Manually update the remaining stream with a separator value to diverge */
    memset( buf, 0, BSCOMPTLS_ENTROPY_BLOCK_SIZE );
    bscomptls_entropy_update_manual( ctx, buf, BSCOMPTLS_ENTROPY_BLOCK_SIZE );

    return( 0 );
}
#endif /* BSCOMPTLS_ENTROPY_NV_SEED */

#if defined(BSCOMPTLS_FS_IO)
int bscomptls_entropy_write_seed_file( bscomptls_entropy_context *ctx, const char *path )
{
    int ret = BSCOMPTLS_ERR_ENTROPY_FILE_IO_ERROR;
    FILE *f;
    unsigned char buf[BSCOMPTLS_ENTROPY_BLOCK_SIZE];

    if( ( f = fopen( path, "wb" ) ) == NULL )
        return( BSCOMPTLS_ERR_ENTROPY_FILE_IO_ERROR );

    if( ( ret = bscomptls_entropy_func( ctx, buf, BSCOMPTLS_ENTROPY_BLOCK_SIZE ) ) != 0 )
        goto exit;

    if( fwrite( buf, 1, BSCOMPTLS_ENTROPY_BLOCK_SIZE, f ) != BSCOMPTLS_ENTROPY_BLOCK_SIZE )
    {
        ret = BSCOMPTLS_ERR_ENTROPY_FILE_IO_ERROR;
        goto exit;
    }

    ret = 0;

exit:
    fclose( f );
    return( ret );
}

int bscomptls_entropy_update_seed_file( bscomptls_entropy_context *ctx, const char *path )
{
    FILE *f;
    size_t n;
    unsigned char buf[ BSCOMPTLS_ENTROPY_MAX_SEED_SIZE ];

    if( ( f = fopen( path, "rb" ) ) == NULL )
        return( BSCOMPTLS_ERR_ENTROPY_FILE_IO_ERROR );

    fseek( f, 0, SEEK_END );
    n = (size_t) ftell( f );
    fseek( f, 0, SEEK_SET );

    if( n > BSCOMPTLS_ENTROPY_MAX_SEED_SIZE )
        n = BSCOMPTLS_ENTROPY_MAX_SEED_SIZE;

    if( fread( buf, 1, n, f ) != n )
    {
        fclose( f );
        return( BSCOMPTLS_ERR_ENTROPY_FILE_IO_ERROR );
    }

    fclose( f );

    bscomptls_entropy_update_manual( ctx, buf, n );

    return( bscomptls_entropy_write_seed_file( ctx, path ) );
}
#endif /* BSCOMPTLS_FS_IO */

#if defined(BSCOMPTLS_SELF_TEST)
#if !defined(BSCOMPTLS_TEST_NULL_ENTROPY)
/*
 * Dummy source function
 */
static int entropy_dummy_source( void *data, unsigned char *output,
                                 size_t len, size_t *olen )
{
    ((void) data);

    memset( output, 0x2a, len );
    *olen = len;

    return( 0 );
}
#endif /* !BSCOMPTLS_TEST_NULL_ENTROPY */

#if defined(BSCOMPTLS_ENTROPY_HARDWARE_ALT)

static int bscomptls_entropy_source_self_test_gather( unsigned char *buf, size_t buf_len )
{
    int ret = 0;
    size_t entropy_len = 0;
    size_t olen = 0;
    size_t attempts = buf_len;

    while( attempts > 0 && entropy_len < buf_len )
    {
        if( ( ret = bscomptls_hardware_poll( NULL, buf + entropy_len,
            buf_len - entropy_len, &olen ) ) != 0 )
            return( ret );

        entropy_len += olen;
        attempts--;
    }

    if( entropy_len < buf_len )
    {
        ret = 1;
    }

    return( ret );
}


static int bscomptls_entropy_source_self_test_check_bits( const unsigned char *buf,
                                                        size_t buf_len )
{
    unsigned char set= 0xFF;
    unsigned char unset = 0x00;
    size_t i;

    for( i = 0; i < buf_len; i++ )
    {
        set &= buf[i];
        unset |= buf[i];
    }

    return( set == 0xFF || unset == 0x00 );
}

/*
 * A test to ensure hat the entropy sources are functioning correctly
 * and there is no obvious failure. The test performs the following checks:
 *  - The entropy source is not providing only 0s (all bits unset) or 1s (all
 *    bits set).
 *  - The entropy source is not providing values in a pattern. Because the
 *    hardware could be providing data in an arbitrary length, this check polls
 *    the hardware entropy source twice and compares the result to ensure they
 *    are not equal.
 *  - The error code returned by the entropy source is not an error.
 */
int bscomptls_entropy_source_self_test( int verbose )
{
    int ret = 0;
    unsigned char buf0[2 * sizeof( unsigned long long int )];
    unsigned char buf1[2 * sizeof( unsigned long long int )];

    if( verbose != 0 )
        bscomptls_printf( "  ENTROPY_BIAS test: " );

    memset( buf0, 0x00, sizeof( buf0 ) );
    memset( buf1, 0x00, sizeof( buf1 ) );

    if( ( ret = bscomptls_entropy_source_self_test_gather( buf0, sizeof( buf0 ) ) ) != 0 )
        goto cleanup;
    if( ( ret = bscomptls_entropy_source_self_test_gather( buf1, sizeof( buf1 ) ) ) != 0 )
        goto cleanup;

    /* Make sure that the returned values are not all 0 or 1 */
    if( ( ret = bscomptls_entropy_source_self_test_check_bits( buf0, sizeof( buf0 ) ) ) != 0 )
        goto cleanup;
    if( ( ret = bscomptls_entropy_source_self_test_check_bits( buf1, sizeof( buf1 ) ) ) != 0 )
        goto cleanup;

    /* Make sure that the entropy source is not returning values in a
     * pattern */
    ret = memcmp( buf0, buf1, sizeof( buf0 ) ) == 0;

cleanup:
    if( verbose != 0 )
    {
        if( ret != 0 )
            bscomptls_printf( "failed\n" );
        else
            bscomptls_printf( "passed\n" );

        bscomptls_printf( "\n" );
    }

    return( ret != 0 );
}

#endif /* BSCOMPTLS_ENTROPY_HARDWARE_ALT */

/*
 * The actual entropy quality is hard to test, but we can at least
 * test that the functions don't cause errors and write the correct
 * amount of data to buffers.
 */
int bscomptls_entropy_self_test( int verbose )
{
    int ret = 1;
#if !defined(BSCOMPTLS_TEST_NULL_ENTROPY)
    bscomptls_entropy_context ctx;
    unsigned char buf[BSCOMPTLS_ENTROPY_BLOCK_SIZE] = { 0 };
    unsigned char acc[BSCOMPTLS_ENTROPY_BLOCK_SIZE] = { 0 };
    size_t i, j;
#endif /* !BSCOMPTLS_TEST_NULL_ENTROPY */

    if( verbose != 0 )
        bscomptls_printf( "  ENTROPY test: " );

#if !defined(BSCOMPTLS_TEST_NULL_ENTROPY)
    bscomptls_entropy_init( &ctx );

    /* First do a gather to make sure we have default sources */
    if( ( ret = bscomptls_entropy_gather( &ctx ) ) != 0 )
        goto cleanup;

    ret = bscomptls_entropy_add_source( &ctx, entropy_dummy_source, NULL, 16,
                                      BSCOMPTLS_ENTROPY_SOURCE_WEAK );
    if( ret != 0 )
        goto cleanup;

    if( ( ret = bscomptls_entropy_update_manual( &ctx, buf, sizeof buf ) ) != 0 )
        goto cleanup;

    /*
     * To test that bscomptls_entropy_func writes correct number of bytes:
     * - use the whole buffer and rely on ASan to detect overruns
     * - collect entropy 8 times and OR the result in an accumulator:
     *   any byte should then be 0 with probably 2^(-64), so requiring
     *   each of the 32 or 64 bytes to be non-zero has a false failure rate
     *   of at most 2^(-58) which is acceptable.
     */
    for( i = 0; i < 8; i++ )
    {
        if( ( ret = bscomptls_entropy_func( &ctx, buf, sizeof( buf ) ) ) != 0 )
            goto cleanup;

        for( j = 0; j < sizeof( buf ); j++ )
            acc[j] |= buf[j];
    }

    for( j = 0; j < sizeof( buf ); j++ )
    {
        if( acc[j] == 0 )
        {
            ret = 1;
            goto cleanup;
        }
    }

#if defined(BSCOMPTLS_ENTROPY_HARDWARE_ALT)
    if( ( ret = bscomptls_entropy_source_self_test( 0 ) ) != 0 )
        goto cleanup;
#endif

cleanup:
    bscomptls_entropy_free( &ctx );
#endif /* !BSCOMPTLS_TEST_NULL_ENTROPY */

    if( verbose != 0 )
    {
        if( ret != 0 )
            bscomptls_printf( "failed\n" );
        else
            bscomptls_printf( "passed\n" );

        bscomptls_printf( "\n" );
    }

    return( ret != 0 );
}
#endif /* BSCOMPTLS_SELF_TEST */

#endif /* BSCOMPTLS_ENTROPY_C */
