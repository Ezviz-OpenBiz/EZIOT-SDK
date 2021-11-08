/*
 *  dna_common.h -- provide dna-system common define.
 *
 *  ORIGINAL AUTHOR: Xu Chun (chun.xu@broadlink.com.cn)
 *
 *  Copyright (c) 2016 Broadlink Corporation
 */

#ifndef __DNA_COMMON_H
#define __DNA_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#ifdef AUTOCONFIG
#include "autoconf.h"
#endif
#include "dna_sdkconfig.h"
#include "dna_compiler.h"

/* dna-system general type */
typedef void *                                      dna_handle_t;

#ifndef NULL
#define NULL                                        (0)
#endif

#ifndef TRUE
#define TRUE                                        (1)
#endif

#ifndef FALSE
#define FALSE                                       (0)
#endif

#ifndef BIT
#define BIT(n)                                      (1UL << (n))
#endif

#ifndef min
#define min(a, b)                                   (((a) < (b)) ? (a) : (b))
#endif

#endif

