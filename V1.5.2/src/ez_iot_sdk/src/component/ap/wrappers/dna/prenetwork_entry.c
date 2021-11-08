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
#include "ez_iot_log.h"

#include "prenetwork_entry.h"
#include "prenetwork_root.h"

INT32 prenetwork_entry(AP_NET_DES *ap_net)
{
    AP_NET_RESP_DES *resp = NULL;
    AP_NET_REQ_DES *req = NULL;
    CGI_PAGE page;
    char *tmppath = NULL;
    int len = 0;

    int statuscode = COMBSTA_INVAL_OPER;
    if(NULL == ap_net)
    {
        return ERROR;
    }

    memset(&page, 0, sizeof(page));
    resp = &(ap_net->resp);
    req =  &(ap_net->req);
    resp->httpcode = HTTP_NOT_FOUND;
    resp->detail_statcode = -1;
    memset(resp->detail_statstr, 0, sizeof(resp->detail_statstr));
    memset(resp->httpbody, 0, sizeof(resp->httpbody)); 

    if ( (METHOD_GET == ap_net->req.method )
        || (METHOD_PUT == ap_net->req.method) )
    {
        tmppath = ap_net->req.url;

        len = strlen(tmppath);
        if(tmppath[len -1]=='/')
        {
            tmppath[len-1] = 0;
        }

        if (0 == strcmp(tmppath, "/PreNetwork/SecurityAndAccessPoint") && ap_net->req.method == METHOD_GET)
        {
            statuscode = pre_SecurityAndAccessPoint_basic(ap_net, &page);
        }
        else if (0 == strcmp(tmppath, "/PreNetwork/WifiConfig") && ap_net->req.method == METHOD_PUT)
        {
            if(NULL == req->httpbody)
            {
                ez_log_d(TAG_AP, "put wificonfig httpbody NULL.");
                statuscode = COMBSTA_INVAL_OPER;
            }
            else
            {
                req->jsonDoc = bscJSON_Parse(req->httpbody);
                if(NULL == req->jsonDoc)
                {
                    ez_log_d(TAG_AP, "put wificonfig httpbody parse failed.");
                    statuscode = COMBSTA_INVAL_OPER;
                }
                else
                {
                    statuscode = pre_WifiConfig_basic(ap_net, &page);
                }
            }
        }
        else
        {
            ez_log_d(TAG_AP, "NOT SUPPORT OPERATION.");
            statuscode = COMBSTA_INVAL_OPER;
        }
        
        switch (statuscode)
        {
        case COMBSTA_OK:
            ap_net->resp.httpcode = HTTP_OK;
            strcpy(ap_net->resp.detail_statstr, "OK");
            break;
        case COMBSTA_BAD_AUTHOR:
            ap_net->resp.httpcode = HTTP_UNAUTHORIZED;
            strcpy(ap_net->resp.detail_statstr, "BadAuthorization");
            break;

        case COMBSTA_METHOD_NOT_ALLOWED:
            ap_net->resp.httpcode = HTTP_NOT_ALLOWED;
            strcpy(ap_net->resp.detail_statstr, "MethodNotAllowed");
            break;
        case COMBSTA_DEV_ERR:
            ap_net->resp.httpcode = HTTP_SERVER_ERROR;
            strcpy(ap_net->resp.detail_statstr, "DeviceError");
            break;
        case COMBSTA_INVAL_OPER:
        case COMBSTA_INVAL_PARAM:
        default:
            ap_net->resp.httpcode = HTTP_BAD_REQUEST;
            strcpy(ap_net->resp.detail_statstr, "InvalidOperation");
            break;
        }
    }
    else
    {
        ez_log_d(TAG_AP, "Request method not support.");
    }
    
    return AP_OK;
}