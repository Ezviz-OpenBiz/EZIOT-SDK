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
#ifndef H_ASE_SUPPORT_H_
#define H_ASE_SUPPORT_H_


#define ASE_SUPPORT_INTERFACE	\
	extern void buf_padding(unsigned char* buf, EZDEV_SDK_INT32 padding_len ,EZDEV_SDK_INT32 len); \
	extern EZDEV_SDK_UINT32 calculate_padding_len(EZDEV_SDK_UINT32 len); \
	extern mkernel_internal_error aes_cbc_128_dec_padding(const unsigned char aes_key[16], \
													  const unsigned char *input_buf, EZDEV_SDK_UINT32 input_length, \
													  unsigned char *output_buf, EZDEV_SDK_UINT32 *output_buf_len); \
	extern mkernel_internal_error aes_cbc_128_enc_padding(const unsigned char aes_key[16], \
													  unsigned char *input_buf, EZDEV_SDK_UINT32 input_length, EZDEV_SDK_UINT32 input_length_padding,\
													  unsigned char *output_buf, EZDEV_SDK_UINT32 *output_length);



#endif