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

#ifndef __DNA_LIBC_H
#define __DNA_LIBC_H

#include "dna_common.h"
#ifdef CONFIG_DNA_LOGDUMP_MODULE
#include "dna_logdump.h"
#endif

#ifdef __cplusplus
    extern "C" {
#endif

#ifdef CONFIG_COMPILE_USER_IMAGE

#include "dna_kernel.h"

#else

#define DNA_PRINTF_ENABLE               1

#if (DNA_PRINTF_ENABLE == 1)
#if defined(__mw3xx__)
extern int wmprintf(const char * format, ...);
#define dna_stdout                      wmprintf
#elif defined(__rtl8710__) && defined(__GNUC__)
#define dna_stdout                      rtl_printf
#else
#define dna_stdout                      printf
#endif
#ifndef CONFIG_DNA_LOGDUMP_MODULE
#define dna_printf                      dna_stdout
#else
#define dna_printf(_fmt_, ...)          \
do {\
        if(dna_logdump_is_enable()) \
            dna_logdump(_fmt_, ##__VA_ARGS__); \
        else \
            dna_stdout(_fmt_, ##__VA_ARGS__); \
} while(0)
#endif
#else
#define dna_printf(_fmt_, ...)
#endif

#define dna_snprintf                    snprintf
#define dna_sprintf                     sprintf
#define dna_vsnprintf                   vsnprintf
#define dna_sscanf                      sscanf

#endif

unsigned int dna_strlen(const char * s);

unsigned int dna_strnlen(const char * s, size_t maxlen);

void * dna_memset(void * buf, int ch, size_t len);

void * dna_memcpy(void * dest, const void * src, size_t n);

void * dna_memmove(void * dest, const void * src, size_t n);

char * dna_strcpy(char * dest, const char * src);

char * dna_strncpy(char * dest, const char * src, size_t num);

int dna_strcmp(const char * str1, const char * str2);

int dna_strncmp(const char * str1, const char * str2, size_t num);

char * dna_strstr(char * str1, const char * str2);

char * dna_strcat(char * dest, char * src);

char * dna_strchr(const char * s, char c);

int dna_memcmp(const void * buff1, const void * buff2, size_t num);

int dna_atoi(const char * str);

int dna_abs(int n);

unsigned long dna_strtoul(const char *nptr,char **endptr,int base);

long dna_strtol(const char *nptr,char **endptr,int base);

int dna_strcasecmp(const char *s1, const char *s2);

#ifdef __cplusplus
}
#endif

#endif

