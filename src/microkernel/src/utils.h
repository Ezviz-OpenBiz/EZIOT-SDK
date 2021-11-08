 /*******************************************************************************
 * Copyright 漏 2017-2021 Ezviz Inc.
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
#ifndef _EZDEVSDK_UTILS_H
#define _EZDEVSDK_UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 *  \brief		随机数生成
 *  \method		ezRandomGen
 *  \param[out] buf				接收缓冲区
 *  \param[in] 	len				缓冲区长度,同时也是生成随机数长度
 *  \return		成功返0,失败返非0
 */
int ezRandomGen(unsigned char *buf, unsigned int len);

/** 
 *  \brief			RSA公钥加密，默认PKCS#1 v1.5填充方式
 *  \method			ezRsaEncrypt
 *  \param[in]		pIn				明文
 *  \param[in] 		iInLen			明文长度
 *  \param[out] 	pOut			密文
 *  \param[in/out] 	iOutLen			密文长度
 *  \param[in] 		pN				N值，16进制字符串,"00112233FF"
 *  \param[in] 		pE				E值，16进制字符串
 *  \return							成功返0,失败返1
 */
int ezRsaEncrypt(const unsigned char *pIn, int iInLen, unsigned char *pOut, int *iOutLen, const char *pN, const char *pE);

/** 
 *  \brief			RSA私钥解密，默认PKCS#1 v1.5填充方式
 *  \method			ezRsaDecrypt
 *  \param[in]		pIn				密文
 *  \param[in] 		iInLen			密文长度
 *  \param[out] 	pOut			明文
 *  \param[in/out] 	iOutLen			明文长度
 *  \param[in] 		pN				N值，16进制字符串,"00112233FF"
 *  \param[in] 		pD				D值，16进制字符串
 *  \return							成功返0,失败返1
 */
int ezRsaDecrypt(const unsigned char *pIn, int iInLen, unsigned char *pOut, int *iOutLen, const char *pP, const char *pQ, 
				 const char *pN, const char *pD, const char *pE);

/** 
 *  \brief			内部错误码转外部错误码
 *  \method			mkiE2ezE
 */
unsigned int mkiE2ezE(unsigned int mkernel_err);

/** 
 *  \brief			获取当前模块编译日期
 *  \method			get_module_build_date
 */
int get_module_build_date(char* pbuf);

#ifdef __cplusplus
}
#endif

#endif	//_EZDEVSDK_UTILS_H