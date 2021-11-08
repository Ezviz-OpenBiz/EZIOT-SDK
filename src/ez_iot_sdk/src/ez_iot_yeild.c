#include "ez_iot_def.h"
#include "ez_iot_yeild.h"
#include "ez_protocol.h"
#include "ezdev_sdk_kernel_struct.h"

/* 检查设备绑定关系 */
static void binding_status_check(void *timer);

void ez_kernel_event_notice(ezdev_sdk_kernel_event *ptr_event)
{
    if (!ptr_event)
        return;

    ez_log_v(TAG_SDK, "event t:%d", ptr_event->event_type);

    switch (ptr_event->event_type)
    {
    case sdk_kernel_event_online:
        ez_iot_set_status(status_online);
        ez_iot_get_callbacks()->recv_event(ez_iot_event_online, NULL, 0);
        break;
    case sdk_kernel_event_break:
        ez_iot_set_status(status_disconnect);
        ez_iot_get_callbacks()->recv_event(ez_iot_event_offline, NULL, 0);
        break;
    case sdk_kernel_event_switchover:
        ez_iot_set_status(status_online);
        sdk_switchover_context *pctx = (sdk_switchover_context *)ptr_event->event_context;
        ez_iot_get_callbacks()->recv_event(ez_iot_event_online, NULL, 0);
        if (NULL != pctx && NULL != pctx->lbs_domain)
        {
            ez_iot_get_callbacks()->recv_event(ez_iot_event_svrname_update, pctx->lbs_domain, strlen((const char *)pctx->lbs_domain));
        }
        break;
    case sdk_kernel_event_runtime_err:
        break;
    case sdk_kernel_event_reconnect_success:
        ez_iot_set_status(status_online);
        ez_iot_get_callbacks()->recv_event(ez_iot_event_reconnect, NULL, 0);
        break;

    case sdk_kernel_event_heartbeat_interval_changed:
        ez_iot_get_callbacks()->recv_event(ez_iot_event_keepalive_update, ptr_event->event_context, sizeof(int));
        break;

    default:
        break;
    }
}

void extend_start_cb(void *pUser)
{
}

void extend_stop_cb(void *pUser)
{
}

void extend_data_route_cb(ezdev_sdk_kernel_submsg *ptr_submsg, void *pUser)
{
    int cmd_id = 0;
    int result_code = 0;
    uint32_t msg_seq = 0;
    if (ptr_submsg == NULL)
    {
        return;
    }

    cmd_id = ptr_submsg->msg_command_id;
    msg_seq = ptr_submsg->msg_seq;
    ez_log_w(TAG_SDK, "cmd:%d", cmd_id);
    ez_log_d(TAG_SDK, "seq:%d, len:%d, data:%s", msg_seq, ptr_submsg->buf_len, (char*)ptr_submsg->buf);

    if (kCenPlt2PuHubTransferMsgReq == cmd_id)
    {
        /* code */
    }

    switch (cmd_id)
    {
    case kCenPlt2PuDomainConfig:
        cloud2dev_set_ntpinfo_req(ptr_submsg->buf, ptr_submsg->buf_len);
        break;
    case kCenPlt2PuSetUserIdReq:
        result_code = cloud2dev_rep_bushandle(ptr_submsg->buf, ptr_submsg->buf_len,
                                              kCenPlt2PuSetUserIdReq, msg_seq, cloud2dev_setuserid_req);
        break;
    case kPu2CenPltGetUserListRsp:
        result_code = dev2cloud_query_banding_rsp(ptr_submsg->buf, ptr_submsg->buf_len);
        break;
    case kPu2CenPltBindUserWithTokenRsp:
        ///< 如果是绑定流程，收到平台回的绑定响应后再去查询绑定状态
        ez_iot_set_need_query_binding();
        // result_code = cloud2dev_banding_rsp();
        break;
    case Pu2CenPltQueryFeatureProfileRsp:
        result_code = dev2cloud_query_profile_url_rsp(ptr_submsg->buf, ptr_submsg->buf_len);
        break;
    case kCenPlt2PuBindUserTouchReq:
        result_code = cloud2dev_contact_bind(ptr_submsg->buf, ptr_submsg->buf_len, msg_seq);
        break;
    default:
        break;
    }

    if (0 != result_code)
    {
        ez_log_e(TAG_SDK, "rv:%d", result_code);
    }
}

void extend_event_cb(ezdev_sdk_kernel_event *ptr_event, void *pUser)
{
}

void ez_kernel_yeild_thread(void *user_data)
{
    ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_succ;
    int32_t *running = (int32_t*)user_data;

    void *timer = hal_timer_creat();
    hal_time_countdown_ms(timer, 0);

    do
    {
        sdk_error = ezdev_sdk_kernel_yield();
        if (sdk_error == ezdev_sdk_kernel_das_force_offline)
        {
            break;
        }

        sdk_error = ezdev_sdk_kernel_yield_user();
        if (sdk_error == ezdev_sdk_kernel_das_force_offline)
        {
            break;
        }

        binding_status_check(timer);

        hal_thread_sleep(10);
    } while (*running && sdk_error != ezdev_sdk_kernel_invald_call);

    hal_timer_destroy(timer);

    return;
}

int32_t kernel_value_save(sdk_keyvalue_type valuetype, int8_t *keyvalue, int32_t keyvalue_size)
{
    int32_t rv = ezdev_sdk_kernel_succ;

    if (valuetype == sdk_keyvalue_devid)
    {
        rv = ez_iot_get_callbacks()->recv_event(ez_iot_event_devid_update, keyvalue, keyvalue_size);
        ez_kv_raw_set((int8_t *)EZ_BASIC_KEY_DEVID, keyvalue, (uint32_t)keyvalue_size);
    }
    else if (valuetype == sdk_keyvalue_masterkey)
    {
        rv = ez_kv_raw_set((int8_t *)EZ_BASIC_KEY_MASTERKEY, keyvalue, (uint32_t)keyvalue_size);
    }

    return rv;
}

void kernel_log_notice(sdk_log_level level, int32_t sdk_error, int32_t othercode, const int8_t *buf)
{
    switch (level)
    {
    case sdk_log_error:
        ez_log_e(TAG_SDK, "e1:%d, e2:%d, %s", sdk_error, othercode, buf);
        break;
    case sdk_log_warn:
        ez_log_w(TAG_SDK, "e1:%d, e2:%d, %s", sdk_error, othercode, buf);
        break;
    case sdk_log_info:
        ez_log_i(TAG_SDK, "e1:%d, e2:%d, %s", sdk_error, othercode, buf);
        break;
    case sdk_log_debug:
        ez_log_d(TAG_SDK, "e1:%d, e2:%d, %s", sdk_error, othercode, buf);
        break;
    case sdk_log_trace:
        ez_log_v(TAG_SDK, "e1:%d, e2:%d, %s", sdk_error, othercode, buf);
        break;
    default:
        break;
    }
}

static void binding_status_check(void *timer)
{
    if (NULL == timer || !ez_iot_is_online())
    {
        return;
    }

    if (1 != ez_iot_is_need_query_binding())
    {
        return;
    }

    if (0 == hal_time_isexpired(timer))
    {
        return;
    }

    dev2cloud_query_banding_req();
    hal_time_countdown(timer, 60);
}