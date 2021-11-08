#include "ez_iot_def.h"
#include "ez_iot.h"
#include "ez_iot_yeild.h"
#include "ez_iot_errno.h"
#include "ez_protocol.h"
#include "shadow.h"
#include "ezdev_sdk_kernel.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ezdev_sdk_kernel_ex.h"

#define SDK_VERSION "V1.5.0"

static void *g_ez_kernel_thread = NULL;
extern bool ez_kv_callback_set(ez_iot_kv_callbacks_t *kv_cb);
static int32_t g_running = 0;

ez_err_e ez_iot_init(const ez_iot_srv_info_t *psrv_info, const ez_iot_dev_info_t *pdev_info, const ez_iot_callbacks_t *pcbs, const ez_iot_kv_callbacks_t *pkvcbs)
{
    ez_log_w(TAG_SDK, "init");

    ez_err_e ez_rv = ez_errno_succ;
    kv_err_e kv_rv = ez_kv_no_err;
    ezdev_sdk_dev_info_t kernel_dev_info = {0};
    int8_t user_id[32 + 1] = {0};
    int8_t dev_id[32] = {0};

    if (ez_iot_is_inited())
    {
        return ez_errno_inited;
    }

    ezdev_sdk_kernel_extend extern_info = {0};
    ezdev_sdk_kernel_platform_handle kernel_platform_handle = {0};
    extern_info.ezdev_sdk_kernel_extend_start = extend_start_cb;
    extern_info.ezdev_sdk_kernel_extend_stop = extend_stop_cb;
    extern_info.ezdev_sdk_kernel_extend_data_route = extend_data_route_cb;
    extern_info.ezdev_sdk_kernel_extend_event = extend_event_cb;

    CHECK_COND_RETURN(!psrv_info, ez_errno_param_invalid);
    CHECK_COND_RETURN(!pdev_info, ez_errno_param_invalid);
    CHECK_COND_RETURN(!pcbs, ez_errno_param_invalid);
    CHECK_COND_RETURN(!psrv_info->svr_name, ez_errno_param_invalid);
    CHECK_COND_RETURN(!pcbs->recv_msg, ez_errno_param_invalid);
    CHECK_COND_RETURN(!pcbs->recv_event, ez_errno_param_invalid);
    CHECK_COND_RETURN(!ez_kv_callback_set((ez_iot_kv_callbacks_t *)pkvcbs), ez_errno_param_invalid);

    ez_log_d(TAG_SDK, "lbs:%s", psrv_info->svr_name);
    ez_log_d(TAG_SDK, "port:%d", psrv_info->svr_port);
    ez_log_d(TAG_SDK, "authm:%d", pdev_info->auth_mode);
    ez_log_d(TAG_SDK, "vcode:%s", pdev_info->dev_verification_code);
    ez_log_d(TAG_SDK, "subsn:%s", pdev_info->dev_subserial);
    ez_log_d(TAG_SDK, "sn:%s", pdev_info->dev_serial);
    ez_log_d(TAG_SDK, "fmw_ver:%s", pdev_info->dev_firmwareversion);
    ez_log_d(TAG_SDK, "sdk_ver:%s", ez_iot_get_sdk_version());
    ez_log_d(TAG_SDK, "type:%s", pdev_info->dev_type);
    ez_log_d(TAG_SDK, "display_type:%s", pdev_info->dev_typedisplay);

    kernel_platform_handle.net_work_create = hal_net_tcp_create;
    kernel_platform_handle.net_work_connect = hal_net_tcp_connect;
    kernel_platform_handle.net_work_read = hal_net_tcp_read;
    kernel_platform_handle.net_work_write = hal_net_tcp_write;
    kernel_platform_handle.net_work_disconnect = hal_net_tcp_disconnect;
    kernel_platform_handle.net_work_destroy = hal_net_tcp_destroy;
    kernel_platform_handle.time_creator = hal_timer_creat;
    kernel_platform_handle.time_isexpired_bydiff = hal_time_isexpired_bydiff;
    kernel_platform_handle.time_isexpired = hal_time_isexpired;
    kernel_platform_handle.time_countdownms = hal_time_countdown_ms;
    kernel_platform_handle.time_countdown = hal_time_countdown;
    kernel_platform_handle.time_leftms = hal_time_left_ms;
    kernel_platform_handle.time_destroy = hal_timer_destroy;
    kernel_platform_handle.thread_mutex_create = hal_thread_mutex_create;
    kernel_platform_handle.thread_mutex_destroy = hal_thread_mutex_destroy;
    kernel_platform_handle.thread_mutex_lock = hal_thread_mutex_lock;
    kernel_platform_handle.thread_mutex_unlock = hal_thread_mutex_unlock;
    kernel_platform_handle.time_sleep = hal_thread_sleep;
    kernel_platform_handle.key_value_save = kernel_value_save;
    kernel_platform_handle.sdk_kernel_log = kernel_log_notice;

    if (ez_kv_no_err != (kv_rv = ez_kv_init()))
    {
        ez_log_e(TAG_SDK, "kv init:%d", kv_rv);
        return ez_errno_kv;
    }

    memcpy(&kernel_dev_info, pdev_info, sizeof(ezdev_sdk_dev_info_t));
    uint32_t dev_id_len = sizeof(dev_id);

    uint32_t masterkey_len = sizeof(kernel_dev_info.dev_master_key);
    uint32_t user_id_len = sizeof(user_id) - 1;

    if (ez_kv_no_err != (kv_rv = ez_kv_raw_get((int8_t *)EZ_BASIC_KEY_DEVID, dev_id, &dev_id_len)) ||
        ez_kv_no_err != (kv_rv = ez_kv_raw_get((int8_t *)EZ_BASIC_KEY_MASTERKEY, kernel_dev_info.dev_master_key, &masterkey_len)) ||
        ez_kv_no_err != (kv_rv = ez_kv_raw_get((int8_t *)EZ_BASIC_KEY_USERID, user_id, &user_id_len)))
    {
        ez_log_e(TAG_SDK, "kv get:%d", kv_rv);
        return ez_errno_kv;
    }

    if (0 != memcmp((char *)pdev_info->dev_id, (char *)dev_id, sizeof(pdev_info->dev_id)))
    {
        ez_log_w(TAG_SDK, "devid mismatch!!!");
        ez_log_d(TAG_SDK, "devid in:");
        ez_log_hexdump(TAG_SDK, 16, (uint8_t *)pdev_info->dev_id, sizeof(pdev_info->dev_id));
        ez_log_d(TAG_SDK, "devid cache:");
        ez_log_hexdump(TAG_SDK, 16, (uint8_t *)dev_id, sizeof(dev_id));
    }

    ez_log_d(TAG_SDK, "masterkey:%s", kernel_dev_info.dev_master_key);
	ez_log_d(TAG_SDK, "userid:%s", user_id);
    ez_log_d(TAG_SDK, "devid:%s", pdev_info->dev_id);

    ez_rv = ezdev_sdk_kernel_init((ezdev_sdk_srv_info_t *)psrv_info, &kernel_dev_info, &kernel_platform_handle, ez_kernel_event_notice);
    CHECK_COND_RETURN(ez_rv, ez_errno_internal);

    extern_info.domain_id = KERNEL_BASE_DOMAIN_ID;
    strncpy(extern_info.extend_module_name, KERNEL_BASE_DOMAIN_NAME, ezdev_sdk_extend_name_len);
    strncpy(extern_info.extend_module_version, KERNEL_EXTEND_VER_DEFAULT, ezdev_sdk_extend_name_len);

    ez_rv = ezdev_sdk_kernel_extend_load(&extern_info);
    CHECK_COND_RETURN(ez_rv, ez_errno_internal);

    ez_iot_set_callbacks(pcbs);
    ez_iot_set_status(status_init);
    ez_iot_set_binding_id(user_id);

    return ez_rv;
}

ez_err_e ez_iot_start(void)
{
    ez_log_w(TAG_SDK, "start");
    ez_err_e ez_rv = ez_errno_succ;
    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_not_init);

    ez_rv = ezdev_sdk_kernel_start();
    CHECK_COND_RETURN(ez_rv, ez_errno_internal);

    g_running = 1;
    g_ez_kernel_thread = hal_thread_create((int8_t *)"ez_kernel_yeild", ez_kernel_yeild_thread, 1024 * 6, 6, (void*)&g_running);
    if (NULL == g_ez_kernel_thread)
    {
        g_running = 0;
        ezdev_sdk_kernel_stop();
        ez_rv = ez_errno_memory;
    }

    ez_iot_set_status(status_ready);

    return ez_rv;
}

ez_err_e ez_iot_stop(void)
{
    ez_log_w(TAG_SDK, "stop");
    ez_err_e ez_rv = ez_errno_succ;

    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_not_init);
    CHECK_COND_RETURN(!ez_iot_is_ready(), ez_errno_not_ready);

    if (g_ez_kernel_thread)
    {
        g_running = 0;
        hal_thread_destroy(g_ez_kernel_thread);
        g_ez_kernel_thread = NULL;
    }

    ez_rv = ezdev_sdk_kernel_stop();
    CHECK_COND_RETURN(ez_rv, ez_errno_internal);
    ez_iot_set_status(status_init);
    return ez_rv;
}

void ez_iot_fini(void)
{
    ez_log_w(TAG_SDK, "fini");
    if (!ez_iot_is_inited())
    {
        return;
    }

    ezdev_sdk_kernel_fini();
    ez_iot_set_status(status_uninit);
    ez_iot_clear_token();
}

ez_err_e ez_iot_binding(int8_t *dev_token)
{
    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_not_init);
    CHECK_COND_RETURN(!ez_iot_is_ready(), ez_errno_not_ready);

    ez_iot_set_token(dev_token);
    ez_iot_clear_query_binding();
    return dev2cloud_banding_req(dev_token);
}

ez_err_e ez_iot_contact_binding(int32_t challenge_code)
{
    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_not_init);
    CHECK_COND_RETURN(!ez_iot_is_ready(), ez_errno_not_ready);
    int32_t response_code = challenge_code + 1;

    ez_iot_clear_query_binding();
    return dev2cloud_contact_bind_req(response_code);
}

int8_t *ez_iot_get_sdk_version(void)
{
    static char buf[64] = {0};

    sprintf(buf, "%s%s", "IOT-SDK ", SDK_VERSION);

    return (int8_t *)buf;
}

int8_t ez_iot_set_keepalive(uint16_t internal)
{
    CHECK_COND_RETURN(!ez_iot_is_inited(), ez_errno_not_init);
    CHECK_COND_RETURN(!ez_iot_is_ready(), ez_errno_not_ready);

    ez_log_w(TAG_SDK, "set keepalive");
    if (0 != ezdev_sdk_kernel_set_keepalive_interval(internal, 0))
    {
        ez_log_e(TAG_SDK, "set keepalive error.");
        return -1;
    }

    return 0;
}
