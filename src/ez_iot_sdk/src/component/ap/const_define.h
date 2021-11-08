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

#ifndef CONST_DEFINE_H
#define CONST_DEFINE_H

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "bscJSON.h"
#include "ez_iot_ap.h"

#define MAX_AP_LIST_NUM 20

#define AP_OK 0
#define ERROR 1

//  http method
#define METHOD_NULL 0x00   // NULL
#define METHOD_GET 0x01    // GET
#define METHOD_POST 0x02   // POST
#define METHOD_PUT 0x04    // PUT
#define METHOD_DELETE 0x10 // DELETE

// subStatusCode
#define COMBSTA_OK 0
#define COMBSTA_DEV_BUSY 24
#define COMBSTA_DEV_ERR 31
#define COMBSTA_NOT_SUPPORT 41
#define COMBSTA_LOG_PRIVI 42
#define COMBSTA_BAD_AUTHOR 43
#define COMBSTA_METHOD_NOT_ALLOWED 44
#define COMBSTA_INVAL_OPER 46
#define COMBSTA_INVAL_PARAM 61

// HTTP CODE TO BE DEFINED
#define HTTP_OK 200           // OK
#define HTTP_BAD_REQUEST 400  // bad request
#define HTTP_UNAUTHORIZED 401 // unauthorized
#define HTTP_NOT_FOUND 404    // not found
#define HTTP_NOT_ALLOWED 405  // method not allowed
#define HTTP_SERVER_ERROR 500 // interval server error

// min num of a and b
#define MINNUM(a, b) (((a) > (b)) ? (b) : (a))

typedef char INT8;
typedef unsigned char UINT8;
typedef int INT32;

typedef struct tag_CGI_PAGE
{
    bscJSON *json_root;

} CGI_PAGE;

typedef struct tag_AP_NET_RESP_DES
{
    int httpcode;
    int statuscode;
    int detail_statcode;
    char detail_statstr[128];

    char httpbody[2048];
} AP_NET_RESP_DES;

typedef struct tag_AP_NET_REQ_DES
{
    int method;
    char url[256];
    char httpbody[1024];
    bscJSON *jsonDoc;
} AP_NET_REQ_DES;

typedef struct tag_AP_NET_DES
{
    AP_NET_REQ_DES req;
    AP_NET_RESP_DES resp;
    ez_iot_ap_wifi_info_t wifi_info;
} AP_NET_DES;

#endif
