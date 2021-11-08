#include "ez_iot_def.h"

static ez_iot_ctx_t g_ez_iot_ctx = {0};

void ez_iot_set_callbacks(const ez_iot_callbacks_t *pcbs)
{
    if (pcbs)
    {
        memcpy(&g_ez_iot_ctx.cbs, pcbs, sizeof(g_ez_iot_ctx.cbs));
    }
}

ez_iot_callbacks_t *ez_iot_get_callbacks(void)
{
    return &g_ez_iot_ctx.cbs;
}

void ez_iot_set_status(dev_status_t stauts)
{
    g_ez_iot_ctx.status = stauts;
}

bool ez_iot_is_inited()
{
    return (status_uninit != g_ez_iot_ctx.status);
}

bool ez_iot_is_ready()
{
    return (status_init < g_ez_iot_ctx.status);
}

bool ez_iot_is_online()
{
    return (status_online == g_ez_iot_ctx.status);
}

void ez_iot_ota_clear_info(void)
{
    memset(&g_ez_iot_ctx.upgrade_info, 0, sizeof(g_ez_iot_ctx.upgrade_info));
}

void ez_iot_ota_set_info(ez_iotsdk_upgrade_file_t *file_info)
{
    memcpy((void *)&g_ez_iot_ctx.upgrade_info, (void *)file_info, sizeof(g_ez_iot_ctx.upgrade_info));
}

ez_iotsdk_upgrade_file_t *ez_iot_ota_get_info()
{
    return &g_ez_iot_ctx.upgrade_info;
}

void ez_iot_clear_token(void)
{
    memset(g_ez_iot_ctx.token, 0, sizeof(g_ez_iot_ctx.token));
}

void ez_iot_set_token(int8_t *token)
{
    memset(g_ez_iot_ctx.token, 0, sizeof(g_ez_iot_ctx.token));
    strncpy((char *)g_ez_iot_ctx.token, (const char *)token, sizeof(g_ez_iot_ctx.token) - 1);
}

int8_t *ez_iot_get_token()
{
    return g_ez_iot_ctx.token;
}

void ez_iot_set_binding_id(int8_t *user_id)
{
    memset(g_ez_iot_ctx.user_id, 0, sizeof(g_ez_iot_ctx.user_id));
    strncpy((void *)&g_ez_iot_ctx.user_id, (char *)user_id, sizeof(g_ez_iot_ctx.user_id) - 1);
}

void ez_iot_clear_binding_id()
{
    memset(g_ez_iot_ctx.user_id, 0, sizeof(g_ez_iot_ctx.user_id));
}

int8_t ez_iot_is_binding()
{
    if (0 == strlen((const char *)g_ez_iot_ctx.user_id))
    {
        return 0;
    }

    return 1;
}

void ez_iot_set_need_query_binding()
{
    g_ez_iot_ctx.need_query_bingding = 0;
}

int8_t ez_iot_is_need_query_binding()
{
    return g_ez_iot_ctx.need_query_bingding ? 0 : 1;
}

void ez_iot_clear_query_binding()
{
    g_ez_iot_ctx.need_query_bingding = 1;
}