/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-19     MurphyZhao   the first version
 */

#ifndef __UTEST_H__
#define __UTEST_H__

#include <stdint.h>
#include <stdbool.h>
#include "utest_def.h"
#include "utest_log.h"
#include "utest_assert.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * utest_error
 * 
 * @brief Test result.
 * 
 * @member UTEST_PASSED Test success.
 * @member UTEST_FAILED Test failed.
 * @member UTEST_PASSED Test skipped.
 * 
*/
enum utest_error
{
    UTEST_PASSED  = 0,
    UTEST_FAILED  = 1,
    UTEST_SKIPPED = 2
};
typedef enum utest_error utest_err_e;

/**
 * utest
 * 
 * @brief utest data structure.
 * 
 * @member error      Error number from enum `utest_error`.
 * @member passed_num Total number of tests passed.
 * @member failed_num Total number of tests failed.
 * 
*/
struct utest
{
    utest_err_e error;
    uint32_t passed_num;
    uint32_t failed_num;
};
typedef struct utest *utest_t;

/**
 * utest_tc_export
 * 
 * @brief utest testcase data structure.
 *        Will export the data to `UtestTcTab` section in flash.
 * 
 * @member name        Testcase name.
 * @member run_timeout Testcase maximum test time (Time unit: seconds).
 * @member init        Necessary initialization before executing the test case function.
 * @member tc          Total number of tests failed.
 * @member cleanup     Total number of tests failed.
 * 
*/
struct utest_tc_export {
    const char  *name;
    uint32_t     run_timeout;
    rt_err_t   (*init)(void);
    void       (*tc)(void);
    rt_err_t   (*cleanup)(void);
};
typedef struct utest_tc_export *utest_tc_export_t;

/**
 * test_unit_func
 * 
 * @brief Unit test handler function pointer.
 * 
*/
typedef void (*test_unit_func)(void);

/**
 * utest_unit_run
 * 
 * @brief Unit test function executor.
 *        No need for the user to call this function directly
 * 
 * @param func           Unit test function.
 * @param unit_func_name Unit test function name.
 * 
 * @return void
 * 
*/
void utest_unit_run(test_unit_func func, const char *unit_func_name);

/**
 * utest_handle_get
 * 
 * @brief Get the utest data structure handle.
 *        No need for the user to call this function directly
 * 
 * @param void
 * 
 * @return utest_t type. (struct utest *)
 * 
*/
utest_t utest_handle_get(void);

/**
 * UTEST_NAME_MAX_LEN
 * 
 * @brief Testcase name maximum length.
 * 
*/
#define UTEST_NAME_MAX_LEN (128u)

/**
 * UTEST_TC_EXPORT
 * 
 * @brief Export testcase function to `UtestTcTab` section in flash.
 *        Used in application layer.
 * 
 * @param testcase The testcase function.
 * @param name     The testcase name.
 * @param init     The initialization function of the test case.
 * @param cleanup  The cleanup function of the test case.
 * @param timeout  Testcase maximum test time (Time unit: seconds).
 * 
 * @return None
 * 
*/
#ifndef UTEST_TC_USE_CONSTRUCTOR
#define UTEST_TC_EXPORT(testcase, name, init, cleanup, timeout)                \
    RT_USED static const struct utest_tc_export _utest_testcase                \
    SECTION("UtestTcTab") =                                                    \
    {                                                                          \
        name,                                                                  \
        timeout,                                                               \
        init,                                                                  \
        testcase,                                                              \
        cleanup                                                                \
    }
#else
    void utest_testcase_run(int argc, char** argv);

    void utest_tc_list(void);

#ifndef UTEST_CASE_CONSTRUCTOR_EXPORT
    #define UTEST_CASE_CONSTRUCTOR_EXPORT (103)
#endif

typedef rt_err_t (*tc_con_t)(void);
typedef void (*tc_fun_t)(void);
extern void uctest_insert_table(tc_fun_t testcase, const char *name, tc_con_t init, tc_con_t cleanup, uint32_t timeout);
#define UTEST_TC_EXPORT(testcase, init, cleanup, timeout)                       \
    __attribute__((constructor(UTEST_CASE_CONSTRUCTOR_EXPORT))) void __unit_##testcase##init##cleanup##timeout()  \
    {                                                                                 \
        uctest_insert_table(testcase, #testcase, init, cleanup, timeout);                  \
    }
#endif

/**
 * UTEST_UNIT_RUN
 * 
 * @brief Unit test function executor.
 *        Used in `testcase` function in application.
 * 
 * @param test_unit_func Unit test function
 * 
 * @return None
 * 
*/
#define UTEST_UNIT_RUN(test_unit_func)                                         \
    utest_unit_run(test_unit_func, #test_unit_func);                           \
    if(utest_handle_get()->failed_num != 0) return;
#ifdef __cplusplus
}
#endif

#endif /* __UTEST_H__ */
