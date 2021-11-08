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
#include "ezconfig.h"
#include "mcuconfig.h"
#include "ez_iot_def.h"
#include "ez_iot_hub.h"
#include "hub_extern.h"
#include "hub_func.h"

static int g_hub_inited = 0;

ez_err_e ez_iot_hub_init(hub_callbacks_t *phub_cbs)
{
    ez_log_w(TAG_HUB, "init");
    ez_err_e rv = 0;

    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_hub_not_init);
    CHECK_COND_RETURN(g_hub_inited, ez_errno_succ);
    CHECK_COND_RETURN(!phub_cbs, ez_errno_hub_param_invalid);
    CHECK_COND_RETURN(!phub_cbs->recv_event, ez_errno_hub_param_invalid);
    CHECK_COND_RETURN(hub_extern_init(), ez_errno_hub_internal);
    CHECK_COND_RETURN(hub_func_init(phub_cbs), ez_errno_hub_internal);

    g_hub_inited = 1;
    return rv;
}

ez_err_e ez_iot_hub_add(const hub_subdev_info_t *subdev_info)
{
    ez_log_w(TAG_HUB, "add");

    hub_subdev_info_internal_t subdev_obj = {0};

    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_hub_not_init);
    CHECK_COND_RETURN(!subdev_info, ez_errno_hub_param_invalid);
    memcpy(&subdev_obj, subdev_info, sizeof(hub_subdev_info_t));

    return hub_add_do(&subdev_obj);
}

ez_err_e ez_iot_hub_del(const int8_t *subdev_sn)
{
    ez_log_w(TAG_HUB, "del");

    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_hub_not_init);
    CHECK_COND_RETURN(!subdev_sn, ez_errno_hub_param_invalid);

    return hub_del_do(subdev_sn);
}

ez_err_e ez_iot_hub_ver_update(const int8_t *subdev_sn, const int8_t *subdev_ver)
{
    ez_log_w(TAG_HUB, "ver up");

    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_hub_not_init);
    CHECK_COND_RETURN(!subdev_sn, ez_errno_hub_param_invalid);
    CHECK_COND_RETURN(!subdev_ver, ez_errno_hub_param_invalid);

    return hub_ver_update_do(subdev_sn, subdev_ver);
}

ez_err_e ez_iot_hub_status_update(const int8_t *subdev_sn, bool online)
{
    ez_log_w(TAG_HUB, "sta up");

    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_hub_not_init);
    CHECK_COND_RETURN(!subdev_sn, ez_errno_hub_param_invalid);

    return hub_status_update_do(subdev_sn, online);
}

ez_err_e ez_iot_hub_subdev_query(const int8_t *subdev_sn, hub_subdev_info_t *subdev_info)
{
    ez_err_e rv = 0;
    hub_subdev_info_internal_t subdev_obj = {0};

    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_hub_not_init);
    CHECK_COND_RETURN(!subdev_sn, ez_errno_hub_param_invalid);
    CHECK_COND_RETURN(!subdev_info, ez_errno_hub_param_invalid);
    memcpy(&subdev_obj, subdev_info, sizeof(hub_subdev_info_t));

    if (ez_errno_succ == (rv = hub_subdev_query(subdev_sn, &subdev_obj)))
    {
        memcpy(subdev_info, &subdev_obj, sizeof(hub_subdev_info_t));
    }

    return rv;
}

ez_err_e ez_iot_hub_subdev_next(hub_subdev_info_t *subdev_info)
{
    ez_err_e rv = 0;
    hub_subdev_info_internal_t subdev_obj = {0};

    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_hub_not_init);
    CHECK_COND_RETURN(!subdev_info, ez_errno_hub_param_invalid);
    memcpy(&subdev_obj, subdev_info, sizeof(hub_subdev_info_t));

    if (ez_errno_succ == (rv = hub_subdev_next(&subdev_obj)))
    {
        memcpy(subdev_info, &subdev_obj, sizeof(hub_subdev_info_t));
    }

    return rv;
}

ez_err_e ez_iot_hub_clean(void)
{
    ez_log_w(TAG_HUB, "clean");

    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_hub_not_init);

    return hub_clean_do();
}

ez_err_e ez_iot_hub_deinit(void)
{
    ez_log_w(TAG_HUB, "deinit");

    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_hub_not_init);
    CHECK_COND_RETURN(hub_extern_finit(), ez_errno_hub_internal);
    hub_func_deinit();

    g_hub_inited = 0;
    return ez_errno_succ;
}