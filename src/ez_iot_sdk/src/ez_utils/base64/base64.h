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
* 2021-02-03     xurongjun
*******************************************************************************/

#ifndef _BASE64_H_
#define _BASE64_H_
#include <stdlib.h>

int ez_base64_encode(unsigned char *dst, size_t dlen, size_t *olen,
                     const unsigned char *src, size_t slen);

#endif
