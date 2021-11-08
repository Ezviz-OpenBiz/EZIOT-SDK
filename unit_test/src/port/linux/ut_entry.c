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
#include <stdio.h>
#include <stdlib.h>
#include "ut_config.h"
#include "utest.h"
#include "ez_iot_log.h"

int main(int argc, char **argv)
{
  utest_log_lv_set(EZ_IOT_UT_DBG_LVL);

  ez_iot_log_init();
  ez_iot_log_start();
  ez_iot_log_filter_lvl(EZ_IOT_SDK_DBG_LVL);

  system("mkdir -p cache");
  utest_testcase_run(argc, argv);

  return 0;
}