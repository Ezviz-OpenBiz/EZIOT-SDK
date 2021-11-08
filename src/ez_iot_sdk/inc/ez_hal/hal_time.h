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

#ifndef _HAL_TIME_H_
#define _HAL_TIME_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 创建定时器对象
     * 
     * @return void* 定时器对象句柄，NULL表示失败，!NULL表示成功。
     */
    void *hal_timer_creat();

    /**
     * @brief 判断是否超时
     * 
     * @param[in] timer 定时器对象句柄
     * @param[in] time_ms 超时范围
     * @return uint8_t 1表示超时，0表示未超时
     */
    uint8_t hal_time_isexpired_bydiff(void *timer, uint32_t time_ms);

    /**
     * @brief 判断是否超时
     * 
     * @param[in] timer 定时器对象句柄
     * @return uint8_t 1表示超时，0表示未超时
     */
    uint8_t hal_time_isexpired(void *timer);

    /**
     * @brief 重置定时器
     * 
     * @param[in] timer 定时器对象句柄
     * @param[in] timeout 预设超时时间(毫秒)
     */
    void hal_time_countdown_ms(void *timer, uint32_t timeout);

    /**
     * @brief 重置定时器
     * 
     * @param[in] time 定时器对象句柄
     * @param[in] timeout 预设超时时间(秒)
     */
    void hal_time_countdown(void *timer, uint32_t timeout);

    /**
     * @brief 获取剩余时间
     * 
     * @param[in] time 定时器对象句柄
     * @return uint32_t 剩余时间
     */
    uint32_t hal_time_left_ms(void *timer);

    /**
     * @brief 销毁定时器
     * 
     * @param[in] timer 定时器对象句柄
     */
    void hal_timer_destroy(void *timer);

#ifdef __cplusplus
}
#endif

#endif