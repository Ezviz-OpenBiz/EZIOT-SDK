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
  
 *******************************************************************************/
#ifdef RT_THREAD
#include <rtthread.h>
#include <finsh.h>
#endif

#include "ez_iot_log.h"
#include "hal_thread.h"
#define TAG_FILTER "T_APP_FILTER"
#define KEYWORD_FILTER "ezapp"
#define NEWLINE_SIGN "\r\n"
#define SHOW_DELAY_TIME_MS 3000

static void test_log(void)
{
    ez_log_a(TAG_APP, "hello word!");
    ez_log_e(TAG_APP, "hello word!");
    ez_log_w(TAG_APP, "hello word!");
    ez_log_i(TAG_APP, "hello word!");
    ez_log_d(TAG_APP, "hello word!");
    ez_log_v(TAG_APP, "hello word!");
}

static void test_log_tag(void)
{
    ez_log_a(TAG_FILTER, "hello word!");
    ez_log_e(TAG_FILTER, "hello word!");
    ez_log_w(TAG_FILTER, "hello word!");
    ez_log_i(TAG_FILTER, "hello word!");
    ez_log_d(TAG_FILTER, "hello word!");
    ez_log_v(TAG_FILTER, "hello word!");
}

static void test_log_keyword(void)
{
    ez_log_a(TAG_APP, "ezapp, easy your life!");
    ez_log_e(TAG_APP, "ezapp, easy your life!");
    ez_log_w(TAG_APP, "ezapp, easy your life!");
    ez_log_i(TAG_APP, "ezapp, easy your life!");
    ez_log_d(TAG_APP, "ezapp, easy your life!");
    ez_log_v(TAG_APP, "ezapp, easy your life!");
}

int example_hello(int argc, char **argv)
{
    printf("log level set:%d%s", EZ_ELOG_LVL_ASSERT, NEWLINE_SIGN);
    ez_iot_log_filter_lvl(EZ_ELOG_LVL_ASSERT);
    test_log();

    printf(NEWLINE_SIGN);
    hal_thread_sleep(SHOW_DELAY_TIME_MS);

    printf("log level set:%d%s", EZ_ELOG_LVL_ERROR, NEWLINE_SIGN);
    ez_iot_log_filter_lvl(EZ_ELOG_LVL_ERROR);
    test_log();

    printf(NEWLINE_SIGN);
    hal_thread_sleep(SHOW_DELAY_TIME_MS);

    printf("log level set:%d%s", EZ_ELOG_LVL_WARN, NEWLINE_SIGN);
    ez_iot_log_filter_lvl(EZ_ELOG_LVL_WARN);
    test_log();

    printf(NEWLINE_SIGN);
    hal_thread_sleep(SHOW_DELAY_TIME_MS);

    printf("log level set:%d%s", EZ_ELOG_LVL_INFO, NEWLINE_SIGN);
    ez_iot_log_filter_lvl(EZ_ELOG_LVL_INFO);
    test_log();

    printf(NEWLINE_SIGN);
    hal_thread_sleep(SHOW_DELAY_TIME_MS);

    printf("log level set:%d%s", EZ_ELOG_LVL_DEBUG, NEWLINE_SIGN);
    ez_iot_log_filter_lvl(EZ_ELOG_LVL_DEBUG);
    test_log();

    printf(NEWLINE_SIGN);
    hal_thread_sleep(SHOW_DELAY_TIME_MS);

    printf("log level set:%d%s", EZ_ELOG_LVL_VERBOSE, NEWLINE_SIGN);
    ez_iot_log_filter_lvl(EZ_ELOG_LVL_VERBOSE);
    test_log();

    printf(NEWLINE_SIGN);
    hal_thread_sleep(SHOW_DELAY_TIME_MS);

    printf("You can only see logs with tag:%s%s", TAG_FILTER, NEWLINE_SIGN);
    ez_iot_log_filter_tag(TAG_FILTER);
    test_log();
    test_log_tag();

    printf(NEWLINE_SIGN);
    hal_thread_sleep(SHOW_DELAY_TIME_MS);

    printf("You can only see logs with keyword:%s%s", KEYWORD_FILTER, NEWLINE_SIGN);
    ez_iot_log_filter_tag("");
    ez_iot_log_filter_kw(KEYWORD_FILTER);
    test_log();
    test_log_tag();
    test_log_keyword();

    return 0;
}

int example_log(int argc, char **argv)
{
    ez_iot_log_init();
    ez_iot_log_start();
    ez_iot_log_filter_lvl(EZ_IOT_TEST_SDK_DBG_LVL);
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_hello, run ez-iot-sdk example helloword);
MSH_CMD_EXPORT(example_log, init ez-iot-sdk log componnent);
#else
int main(int argc, char **argv)
{
    return example_hello(argc, argv);
}
#endif