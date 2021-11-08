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
 *2021-03-11      xurongjun
 *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ezcloud_access.h"
#include "ezcloud_ota.h"

int main(int argc, char **argv)
{
    system("mkdir -p cache");

    ez_cloud_init();
    ez_ota_init();
    ez_cloud_start();
    ez_ota_start();

    while (1)
    {
        char message[100];
        fputs("exit           :q\n", stdout);
        fgets(message, 100, stdin);

        if (0 != strcmp(message, "q\n") || 0 != strcmp(message, "Q\n"))
        {
            break;
        }
    }

    return 0;
}