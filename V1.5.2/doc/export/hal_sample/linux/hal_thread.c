#include <hal_thread.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    pthread_mutex_t lock;
} sdk_mutex_platform;

typedef struct thread_handle_platform
{
    pthread_t thread_hd;
    void *thread_arg;
    void (*task_do)(void *user_data);
    char thread_name[16];
} thread_handle;

static void* sdk_thread_fun(void *aArg)
{
    thread_handle *hd = (thread_handle *)aArg;
    if (hd == NULL)
    {
        return NULL;
    }

    prctl(PR_SET_NAME, hd->thread_name);
    hd->task_do(hd->thread_arg);
    return NULL;
}

void *hal_thread_create(int8_t *thread_name, hal_thread_fun_t thread_fun, int32_t stack_size, int32_t priority, void *arg)
{
    thread_handle *handle = (thread_handle *)malloc(sizeof(thread_handle));
    if (handle == NULL)
    {
        return NULL;
    }

    memset(handle, 0, sizeof(thread_handle));
    handle->task_do = thread_fun;
    handle->thread_arg = arg;
    strncpy(handle->thread_name, (char*)thread_name, sizeof(handle->thread_name) - 1);

    if (pthread_create(&handle->thread_hd, NULL, sdk_thread_fun, (void *)handle) != 0)
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

    if (thandle->thread_hd != 0)
    {
        void *pres = NULL;
        pthread_join(thandle->thread_hd, &pres);
    }

    free(thandle);

    return 0;
}


int hal_thread_detach(void *handle)
{
    if (handle == NULL)
    {
        return -1;
    }

    thread_handle *thandle = (thread_handle *)handle;

    if (thandle->thread_hd != 0)
    {   
        pthread_detach(thandle->thread_hd);
    }

    free(thandle);

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

    pthread_mutex_init(&ptr_mutex_platform->lock, NULL);

    return (void *)ptr_mutex_platform;
}

void hal_thread_mutex_destroy(void *ptr_mutex)
{
    sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)ptr_mutex;
    if (ptr_mutex_platform == NULL)
    {
        return;
    }
    pthread_mutex_destroy(&ptr_mutex_platform->lock);
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

    pthread_mutex_lock(&ptr_mutex_platform->lock);
    return 0;
}

int hal_thread_mutex_unlock(void *ptr_mutex)
{
    sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)ptr_mutex;
    if (ptr_mutex_platform == NULL)
    {
        return -1;
    }

    pthread_mutex_unlock(&ptr_mutex_platform->lock);
    return 0;
}

void hal_thread_sleep(unsigned int time_ms)
{
    usleep((int)(time_ms * 1000));
}