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

#ifndef _HAL_THREAD_SEM_H_
#define _HAL_THREAD_SEM_H_

/**
 * @brief 创建信号量对象
 * 
 * @return void* 信号量句柄，NULL表示失败，!NULL表示成功。
 */
void *hal_semaphore_create(void);

/**
 * @brief 销毁信号量对象
 * 
 * @param[in] ptr_sem 信号量句柄
 */
void hal_semaphore_destroy(void *ptr_sem);

/**
 * @brief 等待信号
 * 
 * @param[in] ptr_sem 信号量句柄
 * @return int 0表示成功，-1表示失败。
 */
int hal_semaphore_wait(void *ptr_sem);

/**
 * @brief 等待信号
 * 
 * @param[in] ptr_sem 信号量句柄
 * @param[in] time_ms 超时时间(毫秒)
 * @return int 0表示成功，-1表示失败。
 */
int hal_semaphore_wait_ms(void *ptr_sem, long time_ms);

/**
 * @brief 发送信号
 * 
 * @param ptr_sem 信号量句柄
 * @return int 0表示成功，-1表示失败。
 */
int hal_semaphore_post(void *ptr_sem);

#endif