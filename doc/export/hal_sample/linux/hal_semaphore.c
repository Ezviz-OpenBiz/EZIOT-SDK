#include "hal_semaphore.h"
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <time.h>
#include <errno.h>

typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int num;
} sdk_sem_platform;

void *hal_semaphore_create(void)
{
    sdk_sem_platform *ptr_sem_platform = NULL;
    ptr_sem_platform = (sdk_sem_platform *)malloc(sizeof(sdk_sem_platform));
    if (ptr_sem_platform == NULL)
    {
        return NULL;
    }

    pthread_condattr_t m_condAttr;
    memset(&m_condAttr, 0, sizeof(pthread_condattr_t));
    pthread_condattr_setclock(&m_condAttr, CLOCK_MONOTONIC);

    pthread_mutex_init(&(ptr_sem_platform->mutex), NULL);
    pthread_cond_init(&(ptr_sem_platform->cond), &m_condAttr);
    ptr_sem_platform->num = 0;

    return (void *)ptr_sem_platform;
}

void hal_semaphore_destroy(void *ptr_sem)
{
    sdk_sem_platform *ptr_sem_platform = (sdk_sem_platform *)ptr_sem;
    if (ptr_sem_platform == NULL)
    {
        return;
    }

    pthread_mutex_destroy(&ptr_sem_platform->mutex);
    pthread_cond_destroy(&ptr_sem_platform->cond);
    free(ptr_sem_platform);
}

int hal_semaphore_wait(void *ptr_sem)
{
    sdk_sem_platform *ptr_sem_platform = (sdk_sem_platform *)ptr_sem;
    if (ptr_sem_platform == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&ptr_sem_platform->mutex);
    ptr_sem_platform->num = 0;

    pthread_cond_wait(&ptr_sem_platform->cond, &ptr_sem_platform->mutex);

    ptr_sem_platform->num = 0;
    pthread_mutex_unlock(&ptr_sem_platform->mutex);

    return 0;
}

int hal_semaphore_wait_ms(void *ptr_sem, long time_ms)
{
    int ret = -1;
    struct timespec tv = {0};

    sdk_sem_platform *ptr_sem_platform = (sdk_sem_platform *)ptr_sem;
    if (ptr_sem_platform == NULL)
    {
        return -1;
    }

    if (0 != clock_gettime(CLOCK_MONOTONIC, &tv))
    {
        return -1;
    }

    pthread_mutex_lock(&ptr_sem_platform->mutex);
    ptr_sem_platform->num = 0;

    tv.tv_sec += time_ms / 1000;
    tv.tv_nsec += time_ms % 1000 * 1000000;
    if (ETIMEDOUT != pthread_cond_timedwait(&ptr_sem_platform->cond, &ptr_sem_platform->mutex, &tv))
    {
        ret = 0;
    }

    ptr_sem_platform->num = 0;
    pthread_mutex_unlock(&ptr_sem_platform->mutex);

    return ret;
}

int hal_semaphore_post(void *ptr_sem)
{
    sdk_sem_platform *ptr_sem_platform = (sdk_sem_platform *)ptr_sem;
    if (ptr_sem_platform == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&ptr_sem_platform->mutex);
    ptr_sem_platform->num = 1;
    pthread_cond_signal(&ptr_sem_platform->cond);
    pthread_mutex_unlock(&ptr_sem_platform->mutex);

    return 0;
}