/*
 *  dna_adapter.h -- provide hardware platform operation interface.
 *  In order to achieve the dna-system is not related to any platform,
 *  It can porting to any platform easily, so dna-system 
 *  Must provide platform interface list to adapter any platform, they 
 *  Need the third party user implementation.
 *
 *  ORIGINAL AUTHOR: Xu Chun (chun.xu@broadlink.com.cn)
 *
 *  Copyright (c) 2016 Broadlink Corporation
 */

#ifndef __DNA_ADAPTER_H
#define __DNA_ADAPTER_H

#include "dna_libc.h"
#include "dna_os.h"
#include "dna_crypto.h"
#include "dna_flash.h"
#include "dna_nvram.h"
#include "dna_ota.h"
#include "dna_wlan.h"
#include "dna_uart.h"
#include "dna_gpio.h"
#include "dna_pwm.h"
#include "dna_i2c.h"
#include "dna_i2csoft.h"
#include "dna_sockets.h"
#include "dna_platform.h"
#include "dna_hardtimer.h"
#endif

