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
#ifndef _EZ_IOT_TSL_INFO_CHECK_H_
#define _EZ_IOT_TSL_INFO_CHECK_H_

#include <stdint.h>
#include <stdbool.h>

int check_schema_value(const void *schema_dsc, const void *tsl_value);

#endif //_EZ_IOT_TSL_INFO_CHECK_H_