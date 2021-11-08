#include "hal_time.h"
#include <time.h>
#include <stdlib.h>

#define TIMESPEC_THOUSAND 1000
#define TIMESPEC_MILLION 1000000
#define TIMESPEC_BILLION 1000000000

#define Platform_Timespec_Add(a, b, result)              \
    do                                                   \
    {                                                    \
        (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;    \
        (result)->tv_nsec = (a)->tv_nsec + (b)->tv_nsec; \
        if ((result)->tv_nsec >= TIMESPEC_BILLION)       \
        {                                                \
            ++(result)->tv_sec;                          \
            (result)->tv_nsec -= TIMESPEC_BILLION;       \
        }                                                \
    } while (0)

#define Platform_Timespec_Sub(a, b, result)              \
    do                                                   \
    {                                                    \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
        (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec; \
        if ((result)->tv_nsec < 0)                       \
        {                                                \
            --(result)->tv_sec;                          \
            (result)->tv_nsec += TIMESPEC_BILLION;       \
        }                                                \
    } while (0)

typedef struct
{
    struct timespec time_record;
} linux_time;

void *hal_timer_creat()
{
    linux_time *linuxtime = NULL;
    linuxtime = (linux_time *)malloc(sizeof(linux_time));
    if (linuxtime == NULL)
    {
        return NULL;
    }

    linuxtime->time_record = (struct timespec){0, 0};
    return (void *)linuxtime;
}

uint8_t hal_time_isexpired_bydiff(void *timer, uint32_t time_ms)
{
    struct timespec now, res;
    linux_time *linuxtime = (linux_time *)timer;
    if (linuxtime == NULL)
    {
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    Platform_Timespec_Sub(&now, &linuxtime->time_record, &res);

    if (res.tv_sec < 0)
    {
        return 0;
    }
    else if (res.tv_sec == 0)
    {
        if ((res.tv_nsec / TIMESPEC_MILLION) > time_ms)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if ((res.tv_sec * TIMESPEC_THOUSAND + res.tv_nsec / TIMESPEC_MILLION) > time_ms)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}

uint8_t hal_time_isexpired(void *timer)
{
    struct timespec now, res;
    linux_time *linuxtime = (linux_time *)timer;
    if (linuxtime == NULL)
    {
        return (char)1;
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    Platform_Timespec_Sub(&linuxtime->time_record, &now, &res);
    return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_nsec <= 0);
}

void hal_time_countdown_ms(void *timer, uint32_t timeout)
{
    struct timespec now;
    struct timespec interval = {timeout / TIMESPEC_THOUSAND, (timeout % TIMESPEC_THOUSAND) * TIMESPEC_MILLION};
    linux_time *linuxtime = (linux_time *)timer;
    if (linuxtime == NULL)
    {
        return;
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    Platform_Timespec_Add(&now, &interval, &linuxtime->time_record);
}

void hal_time_countdown(void *timer, uint32_t timeout)
{
    struct timespec now;
    struct timespec interval = {timeout, 0};
    linux_time *linuxtime = (linux_time *)timer;
    if (linuxtime == NULL)
    {
        return;
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    Platform_Timespec_Add(&now, &interval, &linuxtime->time_record);
}

uint32_t hal_time_left_ms(void *timer)
{
    struct timespec now, res;
    linux_time *linuxtime = (linux_time *)timer;
    if (linuxtime == NULL)
    {
        return 0;
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    Platform_Timespec_Sub(&linuxtime->time_record, &now, &res);
    return (res.tv_sec < 0) ? 0 : res.tv_sec * TIMESPEC_THOUSAND + res.tv_nsec / TIMESPEC_MILLION;
}

void hal_timer_destroy(void *timer)
{
    linux_time *linuxtime = (linux_time *)timer;
    if (linuxtime == NULL)
    {
        return;
    }

    free(linuxtime);
    linuxtime = NULL;
}