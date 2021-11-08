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
#include "mcuconfig.h"
#include "hal_semaphore.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>

typedef struct
{
    xSemaphoreHandle sem;
} sdk_sem_platform;

void *hal_semaphore_create(void)
{
    sdk_sem_platform *ptr_sem_platform = NULL;
    ptr_sem_platform = (sdk_sem_platform *)malloc(sizeof(sdk_sem_platform));
    if (ptr_sem_platform == NULL)
    {
        return NULL;
    }

    ptr_sem_platform->sem = xSemaphoreCreateBinary();
    if (ptr_sem_platform->sem == NULL)
    {
        free(ptr_sem_platform);
        ptr_sem_platform = NULL;
    }

    return (void *)ptr_sem_platform;
}

void hal_semaphore_destroy(void *ptr_sem)
{
    sdk_sem_platform *ptr_sem_platform = (sdk_sem_platform *)ptr_sem;
    if (ptr_sem_platform == NULL)
    {
        return;
    }

    vSemaphoreDelete(ptr_sem_platform->sem);
    free(ptr_sem_platform);

    ptr_sem_platform = NULL;
}

int hal_semaphore_wait(void *ptr_sem)
{
    sdk_sem_platform *ptr_sem_platform = (sdk_sem_platform *)ptr_sem;
    if (ptr_sem_platform == NULL)
    {
        return -1;
    }

    if (xSemaphoreTake(ptr_sem_platform->sem, portMAX_DELAY) == pdTRUE)
    {
        return 0;
    }

    return -1;
}

int hal_semaphore_wait_ms(void *ptr_sem, long time_ms)
{
    sdk_sem_platform *ptr_sem_platform = (sdk_sem_platform *)ptr_sem;
    if (ptr_sem_platform == NULL)
    {
        return -1;
    }

    if (xSemaphoreTake(ptr_sem_platform->sem, time_ms / portTICK_RATE_MS) == pdTRUE)
    {
        return 0;
    }

    return -1;
}

int hal_semaphore_post(void *ptr_sem)
{
    sdk_sem_platform *ptr_sem_platform = (sdk_sem_platform *)ptr_sem;
    if (ptr_sem_platform == NULL)
    {
        return -1;
    }

    return xSemaphoreGive(ptr_sem_platform->sem);
}