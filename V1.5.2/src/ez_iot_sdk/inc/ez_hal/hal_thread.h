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
 * 2019-11-07     XuRongjun    first version
 *******************************************************************************/
#ifndef _HAL_THREAD_H_
#define _HAL_THREAD_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*hal_thread_fun_t)(void *user_data);

    /**
     * @brief 创建任务
     * 
     * @param[in] thread_name 任务名称
     * @param[in] thread_fun 任务函数
     * @param[in] stack_size 任务栈大小
     * @param[in] priority 任务优先级(同各平台保持一致，如freertos参考FreeRTOSConfig.h定义)
     * @param[in] user_data 用户参数
     * @return void* 任务对象句柄，NULL表示失败，!NULL表示成功。
     */
    void *hal_thread_create(int8_t *thread_name, hal_thread_fun_t thread_fun, int32_t stack_size, int32_t priority, void *user_data);

    /**
     * @brief 销毁任务对象，等待任务销毁成功后返回，参考pthread_join。
     * 
     * @param[in] handle 任务对象句柄
     * @return int 0表示成功，-1表示失败。
     */
    int hal_thread_destroy(void *handle);

    /**
     * @brief 分离任务并销毁任务对象，参考pthread_detach。
     * 
     * @param[in] handle 任务对象句柄
     * @return int 0表示成功，-1表示失败。
     */
    int hal_thread_detach(void *handle);

    /**
     * @brief 创建互斥量对象
     * 
     * @return void* 互斥量对象句柄，NULL表示失败，!NULL表示成功。
     */
    void *hal_thread_mutex_create();

    /**
     * @brief 销毁互斥量对象
     * 
     * @param[in] ptr_mutex 互斥量对象句柄
     */
    void hal_thread_mutex_destroy(void *ptr_mutex);

    /**
     * @brief 上锁
     * 
     * @param[in] ptr_mutex 互斥量对象句柄
     * @return int 0表示成功，-1表示失败。
     */
    int hal_thread_mutex_lock(void *ptr_mutex);

    /**
     * @brief 解锁
     * 
     * @param[in] ptr_mutex 互斥量对象句柄
     * @return int 0表示成功，-1表示失败。
     */
    int hal_thread_mutex_unlock(void *ptr_mutex);

    /**
     * @brief 任务休眠
     * 
     * @param[in] time_ms 休眠时间(毫秒)
     */
    void hal_thread_sleep(unsigned int time_ms);

#ifdef __cplusplus
}
#endif

#endif