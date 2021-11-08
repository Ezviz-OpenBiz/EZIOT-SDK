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

 *******************************************************************************/
#include "ezconfig.h"
#include "mcuconfig.h"

#include "ase_support.h"
#include "mbedtls/aes.h"
#include "mkernel_internal_error.h"
#include "base_typedef.h"

EZDEV_SDK_UINT32 calculate_padding_len(EZDEV_SDK_UINT32 len)
{
	EZDEV_SDK_UINT32 input_padding_len = 0;
	input_padding_len = (0x10 - (len & 0x0F));
	input_padding_len += len;
	return input_padding_len;
}

mkernel_internal_error aes_cbc_128_dec_padding(const unsigned char aes_key[16], 
											   const unsigned char *input_buf, EZDEV_SDK_UINT32 input_length, 
											   unsigned char *output_buf, EZDEV_SDK_UINT32 *output_buf_len)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	EZDEV_SDK_INT32 aes_result = 0;
	unsigned char iv[16];
	bscomptls_aes_context aes_ctx;
	EZDEV_SDK_UINT32 index = 0;
	unsigned char padding_char = 0;
	unsigned char other_char = 0;

	do 
	{
		bscomptls_aes_init( &aes_ctx );
		memset((void*)iv, 0, 16);
		for(index=0;index<8;index++)
		{
			iv[index]=index + 0x30;
		}

		if (input_length==0 || input_length%16 != 0)
		{
			sdk_error = mkernel_internal_aes_input_len;
			break;
		}

		aes_result = bscomptls_aes_setkey_dec(&aes_ctx, aes_key, 128);
		if (aes_result != 0)
		{
			sdk_error = mkernel_internal_casll_mbedtls_setdeckey_error;
			break;
		}
		aes_result = bscomptls_aes_crypt_cbc(&aes_ctx, BSCOMPTLS_AES_DECRYPT, input_length, iv, input_buf, output_buf);
		if (aes_result != 0)
		{
			sdk_error = mkernel_internal_casll_mbedtls_crypt_error;
			break;
		}

		padding_char = output_buf[input_length-1];

		if (padding_char > 16 || padding_char <= 0)
		{
			sdk_error = mkernel_internal_aes_padding_unmatched;
			break;
		}

		for (index=1; index<=(EZDEV_SDK_UINT32)padding_char; index++)
		{
			other_char = output_buf[input_length - index];
			if (other_char != padding_char)
			{
				break;
			}
		}

		if (other_char != padding_char)
		{
			sdk_error = mkernel_internal_aes_padding_unmatched;
			break;
		}

		*output_buf_len = (input_length - (int)padding_char);

	} while (0);
	
	bscomptls_aes_free(&aes_ctx);
	return sdk_error;
}

void buf_padding(unsigned char* buf, EZDEV_SDK_INT32 padding_len ,EZDEV_SDK_INT32 len)
{
	EZDEV_SDK_INT32 i=0;
	EZDEV_SDK_INT32 padding_char = 0;

	padding_char = (padding_len - len);

	for(i=len; i < padding_len; i++)
	{
		buf[i] = (unsigned char)padding_char;
	}

	return ;
}

mkernel_internal_error aes_cbc_128_enc_padding(const unsigned char aes_key[16], \
											   unsigned char *input_buf, EZDEV_SDK_UINT32 input_length, EZDEV_SDK_UINT32 input_length_padding, \
											   unsigned char *output_buf, EZDEV_SDK_UINT32 *output_length)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	EZDEV_SDK_INT32 aes_result = 0;
	unsigned char iv[16];
	bscomptls_aes_context aes_ctx;
	EZDEV_SDK_INT32 i=0;
	unsigned char padding_char = 0;

	padding_char = (input_length_padding - input_length);

	for(i=input_length; i < input_length_padding; i++)
	{
		input_buf[i] = padding_char;
	}

	do 
	{
		bscomptls_aes_init( &aes_ctx );
		memset(iv, 0, 16);
		for(i=0;i<8;i++)
		{
			iv[i]=i + 0x30;
		}

		aes_result = bscomptls_aes_setkey_enc(&aes_ctx, aes_key, 128);
		if (aes_result != 0)
		{
			sdk_error = mkernel_internal_casll_mbedtls_setdeckey_error;
			break;
		}
		aes_result = bscomptls_aes_crypt_cbc(&aes_ctx, BSCOMPTLS_AES_ENCRYPT, input_length_padding, iv, input_buf, output_buf);
		if (aes_result != 0)
		{
			sdk_error = mkernel_internal_casll_mbedtls_crypt_error;
			break;
		}
	} while (0);

	bscomptls_aes_free(&aes_ctx);

	if (sdk_error == mkernel_internal_succ)
	{
		*output_length = input_length_padding;
	}

	return sdk_error;
}