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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "ez_iot_log.h"
#include "ez_iot_ctx.h"
#include "hal_time.h"
#include "hal_thread.h"
#include "ez_iot_errno.h"
#include "ez_iot_shadow.h"
#include "shadow.h"
#include "shadow_func.h"
#include "shadow_extern.h"
#include "bscJSON.h"
#include "shadow_core.h"
#include "ezconfig.h"
#include "mcuconfig.h"

#define SHADOW_DOMAIN_REGISTER_MAX_NUM 10
#define MAXIMUM_RETRY_TIME 300 * 1000
#define PUSH_BUF_MAX_LEN 1024 * 2

static int g_pstObj_init_flag;
static void *g_pthread;

typedef struct
{
    int8_t ucIsUsed;
    ez_iot_shadow_module_t stCb;

} Shadow_RegDomain_S;

typedef struct
{
    int8_t ucInit;
    Shadow_RegDomain_S astDomain;

} Shadow_CtrlObj_S;

static Shadow_CtrlObj_S g_stShadow_Obj = {0};

extern void Shadow_Status_Manager(void *ptr);

static void Shadow_Object_fint(void)
{
    Shadow_CtrlObj_S *pstObj = &g_stShadow_Obj;
    pstObj->ucInit = 0;
    g_pstObj_init_flag = 0;

    Shadow_extern_fini();
    pstObj->astDomain.ucIsUsed = 0;
    memset(&pstObj->astDomain.stCb, 0, sizeof(pstObj->astDomain.stCb));
    ez_log_d(TAG_SHADOW, "Object_fint end!");
}

int shadow_init(void)
{
    ez_log_w(TAG_SHADOW, "init");
    int iRet = -1;

    Shadow_CtrlObj_S *pstObj = &g_stShadow_Obj;
    if (pstObj->ucInit == 1)
    {
        return -1;
    }

    pstObj->ucInit = 1;
    g_pstObj_init_flag = 1;

    iRet = Shadow_extern_init();
    if (iRet != 0)
    {
        goto fail;
    }

    shadow_core_start();

    return 0;

fail:

    ez_log_e(TAG_SHADOW, "init failed");

    shadow_fnit();

    return iRet;
}

ez_err_e ez_iot_shadow_init(void)
{
    return shadow_init();
}

void ez_iot_shadow_fini(void)
{
    if (NULL != g_pthread)
    {
        hal_thread_destroy(g_pthread);
    }

    return shadow_fnit();
}

ez_err_e ez_iot_shadow_register(ez_iot_shadow_module_t *module, uint8_t *handle)
{
    ez_log_w(TAG_SHADOW, "reg");

    return -1;
}

void shadow_fnit(void)
{
    ez_log_w(TAG_SHADOW, "fnit");

    Shadow_CtrlObj_S *pstObj = &g_stShadow_Obj;

    Shadow_Set_Status(SHADOW_EXIT_STATUS); //ǿ���˳�

    do
    {
        if (pstObj->ucInit == 0)
        {
            break;
        }

        Shadow_Object_fint();

    } while (0);

    ez_log_d(TAG_SHADOW, "fnit end!");
}

 

ez_err_e ez_iot_shadow_register_v3(ez_iot_shadow_res_t *pres, int8_t *domain_id, ez_iot_shadow_module_t *module)
{
    ez_err_e rv;
    if (NULL == pres || NULL == domain_id || NULL == module)
    {
        ez_log_e(TAG_SHADOW, "reg param err");
        return ez_errno_tsl_internal;
    }

    rv = shadow_core_module_addv3(pres->dev_serial, pres->res_type, pres->local_index, domain_id, module->num, (void *)module->business);
    if (0 == rv)
    {
        shadow_core_event_occured(shadow_event_type_add);
    }

    return rv;
}

ez_err_e ez_iot_shadow_push_v3(ez_iot_shadow_res_t *pres, int8_t *domain_id, int8_t *pkey, ez_iot_shadow_value_t *pvalue)
{
    ez_err_e rv;
    ez_iot_shadow_module_t module;
    ez_iot_shadow_business_t business;

    if (NULL == pres || NULL == domain_id || NULL == pkey)
    {
        ez_log_e(TAG_SHADOW, "push param err");
        return ez_errno_tsl_param_invalid;
    }

    rv = shadow_core_propertie_changed(pres->dev_serial, pres->res_type, pres->local_index, domain_id, pkey, pvalue);
    if (-1 == rv)
    {
        memset(&module, 0, sizeof(module));
        memset(&business, 0, sizeof(business));
        module.business = &business;
        module.num = 1;
        strncpy((char *)business.key, (char *)pkey, sizeof(business.key) - 1);

        rv = shadow_core_module_addv3(pres->dev_serial, pres->res_type, pres->local_index, domain_id, module.num, (void *)module.business);
        if (0 == rv)
        {
            shadow_core_event_occured(shadow_event_type_add);
            rv = shadow_core_propertie_changed(pres->dev_serial, pres->res_type, pres->local_index, domain_id, pkey, pvalue);
        }
    }

    if (0 == rv)
    {
        shadow_core_event_occured(shadow_event_type_report);
    }

    return rv;
}