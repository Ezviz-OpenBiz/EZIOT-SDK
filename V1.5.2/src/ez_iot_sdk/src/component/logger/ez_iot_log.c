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
#include <stdarg.h>
#include "ez_iot_log.h"
#include "elog.h"

int32_t ez_iot_log_init(void)
{
    int32_t rv = elog_init();

    if (ELOG_NO_ERR == rv)
    {
        elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
        elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME | ELOG_FMT_LINE);
        elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME | ELOG_FMT_LINE);
        elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME | ELOG_FMT_LINE);
        elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_FUNC & ~ELOG_FMT_P_INFO & ~ELOG_FMT_DIR);
        elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL);
#ifdef ELOG_COLOR_ENABLE
        elog_set_text_color_enabled(true);
#endif
    }

    return rv;
}

void ez_iot_log_start(void)
{
    elog_start();
}

void ez_iot_log_stop(void)
{
    /* enable output */
    elog_set_output_enabled(false);

#if defined(ELOG_ASYNC_OUTPUT_ENABLE)
    elog_async_enabled(false);
#elif defined(ELOG_BUF_OUTPUT_ENABLE)
    elog_buf_enabled(false);
#endif
}

void ez_iot_log_output(uint8_t level, const char *tag, const char *file, const char *func,
                       const long line, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    elog_output(level, tag, file, func, line, format, ap);
    va_end(ap);
}

void ez_iot_log_filter_lvl(uint8_t level)
{
    elog_set_filter_lvl(level);
}

uint8_t ez_iot_log_filter_lvl_get(void)
{
    return elog_get_filter_lvl();
}

void ez_iot_log_filter_tag(const char *tag)
{
    elog_set_filter_tag(tag);
}

void ez_iot_log_filter_kw(const char *keyword)
{
    elog_set_filter_kw(keyword);
}

void ez_log_hexdump(const char *tag, uint8_t width, uint8_t *buf, uint16_t size)
{
    elog_hexdump(tag, width, buf, size);
}