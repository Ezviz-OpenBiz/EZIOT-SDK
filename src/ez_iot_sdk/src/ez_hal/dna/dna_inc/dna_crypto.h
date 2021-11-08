/*******************************************************************************
*
*               COPYRIGHT (c) 2015-2016 Broadlink Corporation
*                         All Rights Reserved
*
* The source code contained or described herein and all documents
* related to the source code ("Material") are owned by Broadlink
* Corporation or its licensors.  Title to the Material remains
* with Broadlink Corporation or its suppliers and licensors.
*
* The Material is protected by worldwide copyright and trade secret
* laws and treaty provisions. No part of the Material may be used,
* copied, reproduced, modified, published, uploaded, posted, transmitted,
* distributed, or disclosed in any way except in accordance with the
* applicable license agreement.
*
* No license under any patent, copyright, trade secret or other
* intellectual property right is granted to or conferred upon you by
* disclosure or delivery of the Materials, either expressly, by
* implication, inducement, estoppel, except in accordance with the
* applicable license agreement.
*
* Unless otherwise agreed by Broadlink in writing, you may not remove or
* alter this notice or any other notice embedded in Materials by Broadlink
* or Broadlink's suppliers or licensors in any way.
*
*******************************************************************************/

#ifndef __DNA_CRYPTO_H
#define __DNA_CRYPTO_H

#include "dna_compiler.h"

#ifdef __cplusplus
    extern "C" {
#endif

#define DNA_AES_BLOCK_SIZE          16
#define DNA_AES_IV_SIZE             16
#define DNA_AES_KEY_128BIT          16
#define DNA_AES_KEY_256BIT          32

#define DNA_RC4_KEY_128BIT          16

#define DNA_NO_PADDING              0     /* So raw data must 16 bytes alignment */
#define DNA_ZERO_PADDING            1     /* ANSI X.923 */
#define DNA_PKCS5_PADDING           2
#define DNA_PKCS7_PADDING           3

#define DNA_MD5_DIGEST_SIZE         16
#define DNA_SHA1_DIGEST_SIZE        20
#define DNA_SHA224_DIGEST_SIZE      28
#define DNA_SHA256_DIGEST_SIZE      32
#define DNA_SHA384_DIGEST_SIZE      48
#define DNA_SHA512_DIGEST_SIZE      64

enum {
    DNA_SHA1 = 0,
    DNA_SHA224,
    DNA_SHA256,
    DNA_SHA384,
    DNA_SHA512,
};

/**
 * AES Encrypt.
 *
 * \param[in] input plain text.
 * \param[in] input_len plain text actual length.
 * \param[in] iv initialization vector (NULL: ECB)
 * \param[in] key secret key
 * \param[in] key_size DNA_AES_KEY_128BIT or DNA_AES_KEY_256BIT
 * \param[in] pad_type DNA_NO_PADDING / DNA_ZERO_PADDING / DNA_PKCS(5)7_PADDING
 * \param[out] output Cipher text returned by this function.
 *
 * \return Cipher text actual length, -DNA_FAIL on fail.
 */
int dna_aes_encrypt(
    unsigned char * input,
    int input_len,
    const unsigned char * iv,
    const unsigned char * key,
    int key_size,
    int pad_type,
    unsigned char * output) DNA_COMPILER_SECTION_SRAM;

/**
 * AES Decrypt.
 *
 * \param[in] input cipher text, *MUST* 16bytes alignment.
 * \param[in] input_len cipher text actual length.
 * \param[in] iv initialization vector (NULL: ECB)
 * \param[in] key secret key
 * \param[in] key_size DNA_AES_KEY_128BIT or DNA_AES_KEY_256BIT
 * \param[in] pad_type DNA_NO_PADDING / DNA_ZERO_PADDING / DNA_PKCS(5)7_PADDING
 * \param[out] output Plain text returned by this function.
 *
 * \return Plain text actual length, -DNA_FAIL on fail.
 */
int dna_aes_decrypt(
    unsigned char * input,
    int input_len,
    const unsigned char * iv,
    const unsigned char * key,
    int key_size,
    int pad_type,
    unsigned char * output) DNA_COMPILER_SECTION_SRAM;

/**
 * MD5 digest.
 *
 * \param[in] input plain text.
 * \param[in] input_len plain text actual length.
 * \param[out] output Digest data returned by this function.
 *
 * \return Digest data actual length (16bytes), -DNA_FAIL on fail.
 */
int dna_md5(
    const unsigned char * input,
    int input_len,
    unsigned char * output);

/**
 * SHA digest.
 *
 * \param[in] type SHA1/SHA224/SHA256/SHA384/SHA512
 * \param[in] input plain text.
 * \param[in] input_len plain text actual length.
 * \param[out] output Digest data returned by this function.
 *
 * \return Digest data actual length (20/28/32/48/64bytes), -DNA_FAIL on fail.
 */
int dna_sha(
    int type,
    const unsigned char * input,
    int input_len,
    unsigned char * output);

/**
 *  RC4 crypto.
 *
 * \param[in] input plain text.
 * \param[out] output Crypto data returned by this function.
 * \param[in] len input/output data length.
 * \param[in] key secret key.
 * \param[in] key_size DNA_RC4_KEY_128BIT
 *
 * Return acctual crypto data length, -DNA_FAIL on fail.
 */
int dna_rc4(
    const unsigned char *input,
    unsigned char *output,
    unsigned int len,
    unsigned char *key,
    unsigned int key_size);

#ifdef __cplusplus
}
#endif

#endif

