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
#include "timer.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "hal_thread.h"
#include "ezlist/ezlist.h"

typedef struct
{
    char name[32];    // 可以重复
    size_t id;        // 输出,唯一标示这个timer_struct ,不可以重复
    bool reload;      // 可重复触发
    int time_out;     // 超时时间ms
    int count_down;   // 超时时间/100
    int (*fun)(void); // 超时后的回调
} ez_timer_struct;

typedef struct timer_list
{
    ezlist_t *list; //ez_timer_struct
    int msec;       //轮询时间，默认100ms
    bool invalid;
    void *thread;
} timer_list;

static void *g_status_mutex;

static timer_list g_list;

//引用计数
static int g_instance_count = 0;

static void timer_routine(void *param)
{
    while (g_list.invalid)
    {
        hal_thread_mutex_lock(g_status_mutex);
        int list_size = ezlist_size(g_list.list);
        size_t node_size = sizeof(ez_timer_struct);
        for (size_t i = 0; i < list_size; i++)
        {
            ez_timer_struct *node = ezlist_getat(g_list.list, i, &node_size, false);
            if (node != NULL)
            {
                if (0 == node->count_down)
                {
                    if (node->fun != NULL)
                    {
                        node->fun();
                    }

                    if (!node->reload)
                    {
                        ezlist_removeat(g_list.list, i);
                        break;
                    }
                    else
                    {
                        node->count_down = node->time_out / 100;
                    }

                }
                if (0 <= node->count_down)
                {
                    node->count_down--;
                }
            }
        }
        hal_thread_mutex_unlock(g_status_mutex);

        hal_thread_sleep(g_list.msec);
    }
}

int ez_timer_init(void)
{
    g_status_mutex = hal_thread_mutex_create();
    if (NULL == g_status_mutex)
    {
        return -1;
    }

    hal_thread_mutex_lock(g_status_mutex);
    if (g_instance_count++ > 0)
    {
        hal_thread_mutex_unlock(g_status_mutex);
        return 0;
    }

    memset(&g_list, 0, sizeof(g_list));
    g_list.thread = hal_thread_create((int8_t *)"ez_timer", timer_routine, 4 * 1024, 13, (void *)&g_list);
    if (NULL == g_list.thread)
    {
        hal_thread_mutex_unlock(g_status_mutex);
        return -1;
    }
    g_list.invalid = true;
    g_list.msec = 100;
    g_list.list = ezlist(ezlist_THREADSAFE);
    hal_thread_mutex_unlock(g_status_mutex);
    return 0;
}

int ez_timer_fini(void)
{
    hal_thread_mutex_lock(g_status_mutex);
    if (--g_instance_count > 0)
    {
        hal_thread_mutex_unlock(g_status_mutex);
        return 0;
    }

    if (g_list.invalid)
    {
        int list_size = ezlist_size(g_list.list);
        if (0 == list_size)
        {
            g_list.invalid = false;
            hal_thread_destroy(g_list.thread);
            g_list.thread = NULL;

            ezlist_free(g_list.list);
        }
    }

    hal_thread_mutex_unlock(g_status_mutex);

    return 0;
}

void *ez_timer_create(char *name, int time_out, bool reload, int (*fun)(void))
{
    static size_t id = 0;

    if (false == g_list.invalid)
    {
        if (0 != ez_timer_init())
        {
            return NULL;
        }
    }

    ez_timer_struct timer = {0};
    strncpy(timer.name, name, sizeof(timer.name) - 1);
    timer.id = ++id;
    timer.fun = fun;
    timer.time_out = time_out;
    timer.reload = reload;
    timer.count_down = time_out / g_list.msec;

    hal_thread_mutex_lock(g_status_mutex);
    ezlist_addlast(g_list.list, (void *)&timer, sizeof(ez_timer_struct));
    hal_thread_mutex_unlock(g_status_mutex);

    return (void *)timer.id;
}

int ez_timer_delete(void *handle)
{
    if (false == g_list.invalid)
    {
        return -1;
    }

    if (NULL == handle)
    {
        return -1;
    }

    size_t id = (size_t)handle;

    hal_thread_mutex_lock(g_status_mutex);
    size_t list_size = ezlist_size(g_list.list);
    size_t node_size = sizeof(ez_timer_struct);
    for (size_t i = 0; i < list_size; i++)
    {
        ez_timer_struct *timer = ezlist_getat(g_list.list, i, &node_size, false);
        if (id == timer->id)
        {
            ezlist_removeat(g_list.list, i);
            break;
        }
    }
    hal_thread_mutex_unlock(g_status_mutex);

    if (1 == list_size)
    {
        ez_timer_fini();
    }
    return 0;
}

int ez_timer_reset(void *handle)
{
    if (false == g_list.invalid)
    {
        return -1;
    }

    if (NULL == handle)
    {
        return -1;
    }

    size_t id = (size_t)handle;

    hal_thread_mutex_lock(g_status_mutex);
    size_t list_size = ezlist_size(g_list.list);
    size_t node_size = sizeof(ez_timer_struct);
    for (size_t i = 0; i < list_size; i++)
    {
        ez_timer_struct *timer = ezlist_getat(g_list.list, i, &node_size, false);
        if (id == timer->id)
        {
            timer->count_down = timer->time_out / g_list.msec;
            break;
        }
    }
    hal_thread_mutex_unlock(g_status_mutex);
    return 0;
}

int ez_timer_change_period(void *handle, int new_time_out)
{
    if (false == g_list.invalid)
    {
        return -1;
    }

    if (NULL == handle)
    {
        return -1;
    }

    size_t id = (size_t)handle;

    hal_thread_mutex_lock(g_status_mutex);
    size_t list_size = ezlist_size(g_list.list);
    size_t node_size = sizeof(ez_timer_struct);
    for (size_t i = 0; i < list_size; i++)
    {
        ez_timer_struct *timer = ezlist_getat(g_list.list, i, &node_size, false);
        if (id == timer->id)
        {
            timer->time_out = new_time_out;
            if (timer->count_down >= new_time_out / g_list.msec)
            {
                timer->count_down = new_time_out / g_list.msec;
            }
            else
            {
                timer->count_down += (new_time_out - timer->time_out) / g_list.msec;
            }
            break;
        }
    }
    hal_thread_mutex_unlock(g_status_mutex);
    return 0;
}