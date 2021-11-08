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
#ifndef H_EZDEV_ECDH_SUPPORT_H_
#define H_EZDEV_ECDH_SUPPORT_H_

#include "mkernel_internal_error.h"
#include "sdk_kernel_def.h"
#include <string.h>
#include <stdio.h>
#include "mbedtls/ecdh.h"
#include "base_typedef.h"

#define ezdev_sdk_ecdh_key_len 97
#define ezdev_sdk_ecdh_publickey_len  0x61


#ifndef PUT_UINT32_BE
#define PUT_UINT32_BE(n,b,i)                            \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
}
#endif

typedef struct
{
    uint32_t key[16];
    uint32_t v0, v1;
} rnd_pseudo_info;

    mkernel_internal_error generate_public_key(bscomptls_ecdh_context* ctx_client, unsigned char* pubkey, EZDEV_SDK_UINT32* pubkey_len); 
    mkernel_internal_error generate_master_key(bscomptls_ecdh_context* ctx_client, unsigned char* peer_pubkey, EZDEV_SDK_UINT32 peer_pubkey_len, unsigned char* masterkey, EZDEV_SDK_UINT32 *masterkey_len);

	
	

#endif //H_EZDEV_ECDH_SUPPORT_H_