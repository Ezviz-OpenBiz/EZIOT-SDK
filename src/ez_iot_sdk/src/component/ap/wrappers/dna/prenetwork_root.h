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

#ifndef __PRENETWORK_ROOT_H__
#define __PRENETWORK_ROOT_H__

#include "const_define.h"

int pre_test_json_struct_func(AP_NET_DES *ap_net, CGI_PAGE *page);
int pre_SecurityAndAccessPoint_basic(AP_NET_DES *ap_net, CGI_PAGE *page);
int pre_WifiConfig_basic(AP_NET_DES *ap_net, CGI_PAGE *page);

#endif
