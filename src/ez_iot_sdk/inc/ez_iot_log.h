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
#ifndef _EZ_IOT_LOG_H_
#define _EZ_IOT_LOG_H_

#include <stdint.h>

/* output log's level */
#define EZ_ELOG_LVL_ASSERT 0  ///< 致命错误，导致整个程序无法继续运行
#define EZ_ELOG_LVL_ERROR 1   ///< 某个业务出错，不影响其他业务
#define EZ_ELOG_LVL_WARN 2    ///< 打印业务过程中必要的关键信息，尽量简短（WARN<=会记入文件）
#define EZ_ELOG_LVL_INFO 3    ///< 较详细的信息（不允许刷屏）
#define EZ_ELOG_LVL_DEBUG 4   ///< 更为详细的信息，每行带有行号（不允许刷屏）
#define EZ_ELOG_LVL_VERBOSE 5 ///< 不限制打印，每行带有行号，默认不开启。（不允许刷屏）

#define TAG_SDK "T_SDK"
#define TAG_SHADOW "T_SHADOW"
#define TAG_AP "T_AP"
#define TAG_OTA "T_OTA"
#define TAG_APP "T_APP"
#define TAG_TIME "T_TIME"
#define TAG_ATCMD "T_ATCMD"
#define TAG_BLE "T_BLE"
#define TAG_REX_UART "T_REX_UART"
#define TAG_HUB "T_HUB"
#define TAG_TSL "T_TSL"
#define TAG_AT "T_AT"

#ifdef __cplusplus
extern "C"
{
#endif

    int32_t ez_iot_log_init(void);
    void ez_iot_log_start(void);
    void ez_iot_log_stop(void);

    /**
     * set log filter's level\tag\keyword
     */
    void ez_iot_log_filter_lvl(uint8_t level);
    uint8_t ez_iot_log_filter_lvl_get(void);
    void ez_iot_log_filter_tag(const char *tag);
    void ez_iot_log_filter_kw(const char *keyword);
	void ez_iot_log_rec_start(uint8_t bLogRecStart);
	
    /**
     * dump the hex format data to log
     *
     * @param name name for hex object, it will show on log header
     * @param width hex number for every line, such as: 16, 32
     * @param buf hex buffer
     * @param size buffer size
     */
    void ez_log_hexdump(const char *tag, uint8_t width, uint8_t *buf, uint16_t size);

    extern void elog_output(uint8_t level, const char *tag, const char *file, const char *func,
                            const long line, const char *format, ...);

    #define ez_log_a(tag, ...) elog_output(EZ_ELOG_LVL_ASSERT, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
    #define ez_log_e(tag, ...) elog_output(EZ_ELOG_LVL_ERROR, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
    #define ez_log_w(tag, ...) elog_output(EZ_ELOG_LVL_WARN, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
    #define ez_log_i(tag, ...) elog_output(EZ_ELOG_LVL_INFO, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
    #define ez_log_d(tag, ...) elog_output(EZ_ELOG_LVL_DEBUG, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
    #define ez_log_v(tag, ...) elog_output(EZ_ELOG_LVL_VERBOSE, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif