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
#include "hal_thread.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>

typedef struct
{
    xSemaphoreHandle lock;
} sdk_mutex_platform;

typedef struct thread_handle_platform
{
    xTaskHandle thread_hd;
    void *thread_arg;
    void (*task_do)(void *user_data);
    char thread_name[16];
} thread_handle;

static void sdk_thread_fun(void *aArg)
{
    thread_handle *hd = (thread_handle *)aArg;
    if (hd == NULL)
    {
        return;
    }

    hd->task_do(hd->thread_arg);

    do
    {
        hal_thread_sleep(100);
    } while (1);
}

void *hal_thread_create(int8_t *thread_name, hal_thread_fun_t thread_fun, int32_t stack_size, int32_t priority, void *arg)
{
    int rc = 0;
    thread_handle *handle = (thread_handle *)malloc(sizeof(thread_handle));
    if (handle == NULL)
    {
        return NULL;
    }

    memset(handle, 0, sizeof(thread_handle));
    handle->task_do = thread_fun;
    handle->thread_arg = arg;
    strncpy(handle->thread_name, (const char *)thread_name, sizeof(handle->thread_name) - 1);

    rc = xTaskCreate(sdk_thread_fun,      /* The function that implements the task. */
                     handle->thread_name, /* Just a text name for the task to aid debugging. */
                     stack_size,          /* The stack size is defined in FreeRTOSIPConfig.h. */
                     (void *)handle,      /* The task parameter, not used in this case. */
                     priority,            /* The priority assigned to the task is defined in FreeRTOSConfig.h. */
                     &handle->thread_hd); /* The task handle is not used. */

    if (rc != pdPASS)
    {
        free(handle);
        return NULL;
    }

    return handle;
}

int hal_thread_destroy(void *handle)
{
    if (handle == NULL)
    {
        return -1;
    }

    thread_handle *thandle = (thread_handle *)handle;
    xTaskHandle handle_copy = thandle->thread_hd;
    free(thandle);

    if (handle_copy != 0)
    {
        vTaskDelete(handle_copy);
    }

    return 0;
}

void *hal_thread_mutex_create()
{
    sdk_mutex_platform *ptr_mutex_platform = NULL;
    ptr_mutex_platform = (sdk_mutex_platform *)malloc(sizeof(sdk_mutex_platform));
    if (ptr_mutex_platform == NULL)
    {
        return NULL;
    }

    ptr_mutex_platform->lock = xSemaphoreCreateMutex();
    if (ptr_mutex_platform->lock == NULL)
    {
        free(ptr_mutex_platform);
        ptr_mutex_platform = NULL;
    }

    xSemaphoreGive(ptr_mutex_platform->lock);

    return (void *)ptr_mutex_platform;
}

void hal_thread_mutex_destroy(void *ptr_mutex)
{
    sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)ptr_mutex;
    if (ptr_mutex_platform == NULL)
    {
        return;
    }

    vSemaphoreDelete(ptr_mutex_platform->lock);
    free(ptr_mutex_platform);

    ptr_mutex_platform = NULL;
}

int hal_thread_mutex_lock(void *ptr_mutex)
{
    sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)ptr_mutex;
    if (ptr_mutex_platform == NULL)
    {
        return -1;
    }

    if (xSemaphoreTake(ptr_mutex_platform->lock, portMAX_DELAY) == pdTRUE)
    {
        return 0;
    }

    return -1;
}

int hal_thread_mutex_unlock(void *ptr_mutex)
{
    sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)ptr_mutex;
    if (ptr_mutex_platform == NULL)
    {
        return -1;
    }

    if (xSemaphoreGive(ptr_mutex_platform->lock) == pdTRUE)
    {
        return 0;
    }

    return -1;
}

void hal_thread_sleep(unsigned int time_ms)
{
    portTickType xTicksToWait = time_ms / portTICK_RATE_MS /*portTICK_PERIOD_MS*/;
    vTaskDelay(xTicksToWait);
}