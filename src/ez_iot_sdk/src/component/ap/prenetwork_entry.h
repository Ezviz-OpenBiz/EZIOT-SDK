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

#ifndef __PRENETWORK_ENTRY_H__
#define __PRENETWORK_ENTRY_H__

//#ifdef PRENETWORK
#include "const_define.h"
#include "hal_wifi_drv.h"

INT32 prenetwork_entry(AP_NET_DES *web);
int encapsulate_httpbody_with_errcode(AP_NET_DES *ap_net);
#endif
//#endif
