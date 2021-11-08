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

#ifndef __UTEST_DEF_H__
#define __UTEST_DEF_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef RT_VERSION
#include <rtthread.h>
#include <rtdbg.h>
#else

#ifdef _BL_SDK_
#define LOG_I(fmt, args...) printf(fmt "\r\n", ##args)
#define LOG_D(fmt, args...) printf(fmt "\r\n", ##args)
#define LOG_E(fmt, args...) printf(fmt "\r\n", ##args)
#else
#define LOG_I(fmt, args...) printf(fmt "\n", ##args)
#define LOG_D(fmt, args...) printf(fmt "\n", ##args)
#define LOG_E(fmt, args...) printf(fmt "\n", ##args)
#endif
#define rt_thread_t
#define rt_memset memset
#define rt_strcmp strcmp
#define rt_strncpy strncpy
#define rt_strlen strlen
#define rt_kprintf printf
#define rt_memcmp memcmp

#ifdef _RTOS_
#include "FreeRTOS.h"
#include "task.h"
#define rt_thread_mdelay vTaskDelay
#else //linux
#include <unistd.h>
#define rt_thread_mdelay usleep
#endif

#define INIT_COMPONENT_EXPORT(...)
#define MSH_CMD_EXPORT_ALIAS(...)

typedef signed char rt_int8_t;      /**<  8bit integer type */
typedef signed short rt_int16_t;    /**< 16bit integer type */
typedef signed int rt_int32_t;      /**< 32bit integer type */
typedef unsigned char rt_uint8_t;   /**<  8bit unsigned integer type */
typedef unsigned short rt_uint16_t; /**< 16bit unsigned integer type */
typedef unsigned int rt_uint32_t;   /**< 32bit unsigned integer type */

#ifdef ARCH_CPU_64BIT
typedef signed long rt_int64_t;     /**< 64bit integer type */
typedef unsigned long rt_uint64_t;  /**< 64bit unsigned integer type */
#else
typedef signed long long rt_int64_t;    /**< 64bit integer type */
typedef unsigned long long rt_uint64_t; /**< 64bit unsigned integer type */
#endif

typedef int rt_bool_t;            /**< boolean type */
typedef long rt_base_t;           /**< Nbit CPU related date type */
typedef unsigned long rt_ubase_t; /**< Nbit unsigned CPU related data type */

typedef rt_base_t rt_err_t;    /**< Type for error number */
typedef rt_uint32_t rt_time_t; /**< Type for time stamp */
typedef rt_uint32_t rt_tick_t; /**< Type for tick count */
typedef rt_base_t rt_flag_t;   /**< Type for flags */
typedef rt_ubase_t rt_size_t;  /**< Type for size number */
typedef rt_ubase_t rt_dev_t;   /**< Type for device */
typedef rt_base_t rt_off_t;    /**< Type for offset */

/* boolean type definitions */
#define RT_TRUE 1  /**< boolean true  */
#define RT_FALSE 0 /**< boolean fails */
#define RT_EOK 0   /**< There is no error */
#define RT_ERROR 1 /**< A generic error happens */
#define RT_NULL (0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __UTEST_DEF_H__ */