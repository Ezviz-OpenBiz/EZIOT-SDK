/*
 *  Generic ASN.1 parsing
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

#if !defined(BSCOMPTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include BSCOMPTLS_CONFIG_FILE
#endif

#if defined(BSCOMPTLS_ASN1_PARSE_C)

#include "mbedtls/asn1.h"

#include <string.h>

#if defined(BSCOMPTLS_BIGNUM_C)
#include "mbedtls/bignum.h"
#endif

#if defined(BSCOMPTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdlib.h>
#define bscomptls_calloc    calloc
#define bscomptls_free       free
#endif

/* Implementation that should never be optimized out by the compiler */
static void bscomptls_zeroize( void *v, size_t n ) {
    volatile unsigned char *p = (unsigned char*)v; while( n-- ) *p++ = 0;
}

/*
 * ASN.1 DER decoding routines
 */
int bscomptls_asn1_get_len( unsigned char **p,
                  const unsigned char *end,
                  size_t *len )
{
    if( ( end - *p ) < 1 )
        return( BSCOMPTLS_ERR_ASN1_OUT_OF_DATA );

    if( ( **p & 0x80 ) == 0 )
        *len = *(*p)++;
    else
    {
        switch( **p & 0x7F )
        {
        case 1:
            if( ( end - *p ) < 2 )
                return( BSCOMPTLS_ERR_ASN1_OUT_OF_DATA );

            *len = (*p)[1];
            (*p) += 2;
            break;

        case 2:
            if( ( end - *p ) < 3 )
                return( BSCOMPTLS_ERR_ASN1_OUT_OF_DATA );

            *len = ( (size_t)(*p)[1] << 8 ) | (*p)[2];
            (*p) += 3;
            break;

        case 3:
            if( ( end - *p ) < 4 )
                return( BSCOMPTLS_ERR_ASN1_OUT_OF_DATA );

            *len = ( (size_t)(*p)[1] << 16 ) |
                   ( (size_t)(*p)[2] << 8  ) | (*p)[3];
            (*p) += 4;
            break;

        case 4:
            if( ( end - *p ) < 5 )
                return( BSCOMPTLS_ERR_ASN1_OUT_OF_DATA );

            *len = ( (size_t)(*p)[1] << 24 ) | ( (size_t)(*p)[2] << 16 ) |
                   ( (size_t)(*p)[3] << 8  ) |           (*p)[4];
            (*p) += 5;
            break;

        default:
            return( BSCOMPTLS_ERR_ASN1_INVALID_LENGTH );
        }
    }

    if( *len > (size_t) ( end - *p ) )
        return( BSCOMPTLS_ERR_ASN1_OUT_OF_DATA );

    return( 0 );
}

int bscomptls_asn1_get_tag( unsigned char **p,
                  const unsigned char *end,
                  size_t *len, int tag )
{
    if( ( end - *p ) < 1 )
        return( BSCOMPTLS_ERR_ASN1_OUT_OF_DATA );

    if( **p != tag )
        return( BSCOMPTLS_ERR_ASN1_UNEXPECTED_TAG );

    (*p)++;

    return( bscomptls_asn1_get_len( p, end, len ) );
}

int bscomptls_asn1_get_bool( unsigned char **p,
                   const unsigned char *end,
                   int *val )
{
    int ret;
    size_t len;

    if( ( ret = bscomptls_asn1_get_tag( p, end, &len, BSCOMPTLS_ASN1_BOOLEAN ) ) != 0 )
        return( ret );

    if( len != 1 )
        return( BSCOMPTLS_ERR_ASN1_INVALID_LENGTH );

    *val = ( **p != 0 ) ? 1 : 0;
    (*p)++;

    return( 0 );
}

int bscomptls_asn1_get_int( unsigned char **p,
                  const unsigned char *end,
                  int *val )
{
    int ret;
    size_t len;

    if( ( ret = bscomptls_asn1_get_tag( p, end, &len, BSCOMPTLS_ASN1_INTEGER ) ) != 0 )
        return( ret );

    if( len == 0 || len > sizeof( int ) || ( **p & 0x80 ) != 0 )
        return( BSCOMPTLS_ERR_ASN1_INVALID_LENGTH );

    *val = 0;

    while( len-- > 0 )
    {
        *val = ( *val << 8 ) | **p;
        (*p)++;
    }

    return( 0 );
}

#if defined(BSCOMPTLS_BIGNUM_C)
int bscomptls_asn1_get_mpi( unsigned char **p,
                  const unsigned char *end,
                  bscomptls_mpi *X )
{
    int ret;
    size_t len;

    if( ( ret = bscomptls_asn1_get_tag( p, end, &len, BSCOMPTLS_ASN1_INTEGER ) ) != 0 )
        return( ret );

    ret = bscomptls_mpi_read_binary( X, *p, len );

    *p += len;

    return( ret );
}
#endif /* BSCOMPTLS_BIGNUM_C */

int bscomptls_asn1_get_bitstring( unsigned char **p, const unsigned char *end,
                        bscomptls_asn1_bitstring *bs)
{
    int ret;

    /* Certificate type is a single byte bitstring */
    if( ( ret = bscomptls_asn1_get_tag( p, end, &bs->len, BSCOMPTLS_ASN1_BIT_STRING ) ) != 0 )
        return( ret );

    /* Check length, subtract one for actual bit string length */
    if( bs->len < 1 )
        return( BSCOMPTLS_ERR_ASN1_OUT_OF_DATA );
    bs->len -= 1;

    /* Get number of unused bits, ensure unused bits <= 7 */
    bs->unused_bits = **p;
    if( bs->unused_bits > 7 )
        return( BSCOMPTLS_ERR_ASN1_INVALID_LENGTH );
    (*p)++;

    /* Get actual bitstring */
    bs->p = *p;
    *p += bs->len;

    if( *p != end )
        return( BSCOMPTLS_ERR_ASN1_LENGTH_MISMATCH );

    return( 0 );
}

/*
 * Get a bit string without unused bits
 */
int bscomptls_asn1_get_bitstring_null( unsigned char **p, const unsigned char *end,
                             size_t *len )
{
    int ret;

    if( ( ret = bscomptls_asn1_get_tag( p, end, len, BSCOMPTLS_ASN1_BIT_STRING ) ) != 0 )
        return( ret );

    if( (*len)-- < 2 || *(*p)++ != 0 )
        return( BSCOMPTLS_ERR_ASN1_INVALID_DATA );

    return( 0 );
}



/*
 *  Parses and splits an ASN.1 "SEQUENCE OF <tag>"
 */
int bscomptls_asn1_get_sequence_of( unsigned char **p,
                          const unsigned char *end,
                          bscomptls_asn1_sequence *cur,
                          int tag)
{
    int ret;
    size_t len;
    bscomptls_asn1_buf *buf;

    /* Get main sequence tag */
    if( ( ret = bscomptls_asn1_get_tag( p, end, &len,
            BSCOMPTLS_ASN1_CONSTRUCTED | BSCOMPTLS_ASN1_SEQUENCE ) ) != 0 )
        return( ret );

    if( *p + len != end )
        return( BSCOMPTLS_ERR_ASN1_LENGTH_MISMATCH );

    while( *p < end )
    {
        buf = &(cur->buf);
        buf->tag = **p;

        if( ( ret = bscomptls_asn1_get_tag( p, end, &buf->len, tag ) ) != 0 )
            return( ret );

        buf->p = *p;
        *p += buf->len;

        /* Allocate and assign next pointer */
        if( *p < end )
        {
            cur->next = (bscomptls_asn1_sequence*)bscomptls_calloc( 1,
                                            sizeof( bscomptls_asn1_sequence ) );

            if( cur->next == NULL )
                return( BSCOMPTLS_ERR_ASN1_ALLOC_FAILED );

            cur = cur->next;
        }
    }

    /* Set final sequence entry's next pointer to NULL */
    cur->next = NULL;

    if( *p != end )
        return( BSCOMPTLS_ERR_ASN1_LENGTH_MISMATCH );

    return( 0 );
}

int bscomptls_asn1_get_alg( unsigned char **p,
                  const unsigned char *end,
                  bscomptls_asn1_buf *alg, bscomptls_asn1_buf *params )
{
    int ret;
    size_t len;

    if( ( ret = bscomptls_asn1_get_tag( p, end, &len,
            BSCOMPTLS_ASN1_CONSTRUCTED | BSCOMPTLS_ASN1_SEQUENCE ) ) != 0 )
        return( ret );

    if( ( end - *p ) < 1 )
        return( BSCOMPTLS_ERR_ASN1_OUT_OF_DATA );

    alg->tag = **p;
    end = *p + len;

    if( ( ret = bscomptls_asn1_get_tag( p, end, &alg->len, BSCOMPTLS_ASN1_OID ) ) != 0 )
        return( ret );

    alg->p = *p;
    *p += alg->len;

    if( *p == end )
    {
        bscomptls_zeroize( params, sizeof(bscomptls_asn1_buf) );
        return( 0 );
    }

    params->tag = **p;
    (*p)++;

    if( ( ret = bscomptls_asn1_get_len( p, end, &params->len ) ) != 0 )
        return( ret );

    params->p = *p;
    *p += params->len;

    if( *p != end )
        return( BSCOMPTLS_ERR_ASN1_LENGTH_MISMATCH );

    return( 0 );
}

int bscomptls_asn1_get_alg_null( unsigned char **p,
                       const unsigned char *end,
                       bscomptls_asn1_buf *alg )
{
    int ret;
    bscomptls_asn1_buf params;

    memset( &params, 0, sizeof(bscomptls_asn1_buf) );

    if( ( ret = bscomptls_asn1_get_alg( p, end, alg, &params ) ) != 0 )
        return( ret );

    if( ( params.tag != BSCOMPTLS_ASN1_NULL && params.tag != 0 ) || params.len != 0 )
        return( BSCOMPTLS_ERR_ASN1_INVALID_DATA );

    return( 0 );
}

void bscomptls_asn1_free_named_data( bscomptls_asn1_named_data *cur )
{
    if( cur == NULL )
        return;

    bscomptls_free( cur->oid.p );
    bscomptls_free( cur->val.p );

    bscomptls_zeroize( cur, sizeof( bscomptls_asn1_named_data ) );
}

void bscomptls_asn1_free_named_data_list( bscomptls_asn1_named_data **head )
{
    bscomptls_asn1_named_data *cur;

    while( ( cur = *head ) != NULL )
    {
        *head = cur->next;
        bscomptls_asn1_free_named_data( cur );
        bscomptls_free( cur );
    }
}

bscomptls_asn1_named_data *bscomptls_asn1_find_named_data( bscomptls_asn1_named_data *list,
                                       const char *oid, size_t len )
{
    while( list != NULL )
    {
        if( list->oid.len == len &&
            memcmp( list->oid.p, oid, len ) == 0 )
        {
            break;
        }

        list = list->next;
    }

    return( list );
}

#endif /* BSCOMPTLS_ASN1_PARSE_C */
