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

#include "mkernel_internal_error.h"
#include "sdk_kernel_def.h"
#include <string.h>
#include <stdio.h>
#include "ezconfig.h"
#include "mcuconfig.h"

#include "mbedtls/ecdh.h"
#include "mbedtls/config.h"
#include "mbedtls/bignum.h"
#include "mbedtls/ecp.h"
#include "mbedtls/platform.h"
#include "mbedtls/error.h"
#include "mbedtls/pk.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "ezdev_ecdh_support.h"

static int rnd_std_rand( void *rng_state, unsigned char *output, size_t len )
{
#if !defined(__OpenBSD__)
    size_t i;

    if( rng_state != NULL )
        rng_state  = NULL;

    for( i = 0; i < len; ++i )
        output[i] = rand();
#else
    if( rng_state != NULL )
        rng_state = NULL;

    arc4random_buf( output, len );
#endif /* !OpenBSD */

    return( 0 );
}
static int rnd_pseudo_rand( void *rng_state, unsigned char *output, size_t len )
{
    rnd_pseudo_info *info = (rnd_pseudo_info *) rng_state;
    uint32_t i, *k, sum, delta=0x9E3779B9;
    unsigned char result[4], *out = output;

    if( rng_state == NULL )
        return( rnd_std_rand( NULL, output, len ) );

    k = info->key;

    while( len > 0 )
    {
        size_t use_len = ( len > 4 ) ? 4 : len;
        sum = 0;

        for( i = 0; i < 32; i++ )
        {
            info->v0 += ( ( ( info->v1 << 4 ) ^ ( info->v1 >> 5 ) )
                            + info->v1 ) ^ ( sum + k[sum & 3] );
            sum += delta;
            info->v1 += ( ( ( info->v0 << 4 ) ^ ( info->v0 >> 5 ) )
                            + info->v0 ) ^ ( sum + k[( sum>>11 ) & 3] );
        }

        PUT_UINT32_BE( info->v0, result, 0 );
        memcpy( out, result, use_len );
        len -= use_len;
        out += 4;
    }

    return( 0 );
}


mkernel_internal_error generate_public_key(bscomptls_ecdh_context* ctx_client, unsigned char* pubkey, EZDEV_SDK_UINT32* pubkey_len)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    unsigned char buf[128];
    EZDEV_SDK_UINT32 public_key_len = 0;
    rnd_pseudo_info rnd_info;
    int  ret = 0;

    do
    {
        if(ctx_client == NULL || pubkey == NULL || pubkey_len == NULL)
        {
            sdk_error = mkernel_internal_input_param_invalid;
            ezdev_sdk_kernel_log_warn(0, 0, "generate_public_key input param error");
            break;
        }

        ret = bscomptls_ecp_group_load( &ctx_client->grp, BSCOMPTLS_ECP_DP_SECP384R1);
        if(ret != 0)
        {   
            sdk_error = mkernel_internal_bscomptls_ecp_group_load_err;
            ezdev_sdk_kernel_log_warn(0, 0, "ecp_group_load err,ret: %d", ret);
            break;
        }

        memset( buf, 0x00, sizeof( buf )); 

        ret = bscomptls_ecdh_make_public(ctx_client, &public_key_len, buf, 128, &rnd_pseudo_rand, &rnd_info );
        if(ret != 0)
        {   
            sdk_error = mkernel_internal_bscomptls_ecdh_read_public_err;
            ezdev_sdk_kernel_log_warn(0, 0, "ecdh_make_public err,ret:%d", ret);
            break;
        } 
        /*首字节表示publickey的长度，这里要注意*/
        memcpy(pubkey, buf + 1, public_key_len -1);
        *pubkey_len = public_key_len -1;
      
    }while(0);
    
    return sdk_error;
}


mkernel_internal_error generate_master_key(bscomptls_ecdh_context* ctx_client, unsigned char* peer_pubkey,EZDEV_SDK_UINT32 peer_pubkey_len,\
                                           unsigned char* masterkey, EZDEV_SDK_UINT32 *masterkey_len)
{
    int ret = 0;
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    rnd_pseudo_info rnd_info;
    EZDEV_SDK_UINT32 master_key_len = 0;
    unsigned char input_key[ezdev_sdk_ecdh_key_len+1]={0};
    
    do
    {
        if(ctx_client == NULL||peer_pubkey== NULL||masterkey==NULL|| masterkey_len==NULL||peer_pubkey_len != ezdev_sdk_ecdh_key_len)
        {
            ezdev_sdk_kernel_log_warn(0, 0, "gen master key invalid param, len:%d",peer_pubkey_len);
            sdk_error = mkernel_internal_input_param_invalid;
            break;
        }
        input_key[0] = ezdev_sdk_ecdh_publickey_len;// 首字节填充public长度97
        memcpy(&input_key[1], peer_pubkey, peer_pubkey_len);// 将97个字符的publickey拷贝进去
        ret = bscomptls_ecdh_read_public(ctx_client, input_key, ezdev_sdk_ecdh_key_len + 1);
        if(ret!=0)
        {
            ezdev_sdk_kernel_log_warn(0, 0, "ec read_public error,ret:%d", ret);
            sdk_error = mkernel_internal_bscomptls_ecdh_read_public_err;
            break;
        }
        ret = bscomptls_ecdh_calc_secret( ctx_client, &master_key_len, masterkey, 128, &rnd_pseudo_rand, &rnd_info );
        if(ret!=0)
        {
            ezdev_sdk_kernel_log_warn(0, ret, "ec calc_secret err");
            sdk_error = mkernel_internal_bscomptls_ecdh_calc_secret_err;
            break;
        }

        *masterkey_len = master_key_len;

    }while(0);
    
    return sdk_error;
}

