/**
 * @file ez_iot_yeild.h
 * @author xurongjun (xurongjun@ezvizlife.com)
 * @brief 
 * @version 0.1
 * @date 2019-11-09
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef _EZ_IOT_EXTEND_H_
#define _EZ_IOT_EXTEND_H_

#include "ezdev_sdk_kernel.h"
#include "ezdev_sdk_kernel_struct.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void ez_kernel_event_notice(ezdev_sdk_kernel_event *ptr_event);

    void ez_kernel_yeild_thread(void *user_data);

    void extend_start_cb(void *pUser);

    void extend_stop_cb(void *pUser);

    void extend_data_route_cb(ezdev_sdk_kernel_submsg *ptr_submsg, void *pUser);

    void extend_event_cb(ezdev_sdk_kernel_event *ptr_event, void *pUser);

    int32_t kernel_value_save(sdk_keyvalue_type valuetype, int8_t *keyvalue, int32_t keyvalue_size);

    void kernel_log_notice(sdk_log_level level, int32_t sdk_error, int32_t othercode, const int8_t *buf);

#ifdef __cplusplus
}
#endif

#endif