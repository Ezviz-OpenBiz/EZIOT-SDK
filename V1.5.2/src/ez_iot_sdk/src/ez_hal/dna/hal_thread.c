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
#include <string.h>
#include <stdlib.h>
#include "dna_inc/dna_os.h"

#define MUTEX_WAIT_FOREVER (~0UL)

typedef struct
{
	dna_mutex_handle_t lock;
} sdk_mutex_platform;

typedef struct thread_handle_platform
{
	dna_task_handle_t thread_hd;
	void *thread_arg;
	unsigned int (*task_do)(void *user_data);
	char thread_name[16];
} thread_handle;

static void *sdk_thread_fun(void *aArg)
{
	thread_handle *hd = (thread_handle *)aArg;
	if (hd == NULL)
	{
		return NULL;
	}

	hd->task_do(hd->thread_arg);

	do
	{
		dna_task_sleep(100);
	} while (1);

	return NULL;
}

void* hal_thread_create(int8_t* thread_name, hal_thread_fun_t thread_fun, int32_t stack_size, int32_t priority, void* arg)
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
	strncpy(handle->thread_name, thread_name, sizeof(handle->thread_name) - 1);

	handle->thread_hd = dna_task_create(handle->thread_name, stack_size, priority, sdk_thread_fun, (void *)handle);
	if (NULL == handle->thread_hd)
	{
		free(handle);
		printf("dna_task_create error\n");
		return NULL;
	}

	printf("dna_task_create succ\n");
	return handle;
}

int hal_thread_destroy(void *handle)
{
	if (handle == NULL)
	{
		return -1;
	}
	if (NULL != ((thread_handle *)handle)->thread_hd)
	{
		dna_task_delete(((thread_handle *)handle)->thread_hd);
	}
	free(handle);

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

	ptr_mutex_platform->lock = dna_mutex_create(NULL);
	if (ptr_mutex_platform->lock == NULL)
	{
		free(ptr_mutex_platform);
		ptr_mutex_platform = NULL;
	}

	return (void *)ptr_mutex_platform;
}

void hal_thread_mutex_destroy(void *ptr_mutex)
{
	sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)ptr_mutex;
	if (ptr_mutex_platform == NULL)
	{
		return;
	}

	dna_mutex_delete(&ptr_mutex_platform->lock);

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

	dna_mutex_get(ptr_mutex_platform->lock, MUTEX_WAIT_FOREVER);

	return 0;
}

int hal_thread_mutex_unlock(void *ptr_mutex)
{
	sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)ptr_mutex;
	if (ptr_mutex_platform == NULL)
	{
		return -1;
	}

	dna_mutex_put(ptr_mutex_platform->lock);

	return 0;
}

void hal_thread_sleep(unsigned int time_ms)
{
	dna_task_sleep((unsigned long)time_ms);
}