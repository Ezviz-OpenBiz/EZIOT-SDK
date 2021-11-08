#ifndef MCUCONFIG_H__
#define MCUCONFIG_H__

/* Fill in the mcu custom parameters */
#include <stdio.h>

#define printf    system_printf
#define malloc    os_malloc
#define free      os_free
#define calloc    os_calloc
#define zalloc    os_zalloc
#define realloc   os_realloc
#define strspn    os_strspn
#define strcspn   os_strcspn
#define strdup    os_strdup
#define localtime os_localtime
#define time      os_time
#define sprintf   os_sprintf
#define snprintf  os_snprintf
#define strdup    os_strdup
#define vsnprintf os_vsnprintf
// #define clock_gettime  os_clock_gettime
// #define gettimeofday   os_gettimeofday

extern void   system_printf(const char *f, ...);
extern void * os_malloc(size_t size);
extern void   os_free(void *ptr);
extern void * os_calloc( size_t nmemb, size_t size );
extern void * os_zalloc(size_t size);
extern void * os_realloc(void *ptr, size_t size);
extern int os_sprintf(char *str, const char *format, ...);
extern int os_snprintf(char *str, size_t size, const char *format, ...);

// #ifndef _TIMEVAL_DEFINED
// #define _TIMEVAL_DEFINED
// /*
//  * Structure returned by gettimeofday(2) system call,
//  * and used in other calls.
//  */
// struct timeval {
//     long    tv_sec;     /* seconds */
//     long    tv_usec;    /* and microseconds */
// };
// #endif /* _TIMEVAL_DEFINED */

// #ifndef _TIMESPEC_DEFINED
// #define _TIMESPEC_DEFINED
// /*
//  * Structure defined by POSIX.1b to be like a timeval.
//  */
// struct timespec {
//     time_t  tv_sec;     /* seconds */
//     long    tv_nsec;    /* and nanoseconds */
// };
// #endif /* _TIMESPEC_DEFINED */

extern int os_clock_gettime(int clk_id, struct timespec *t);
extern int os_gettimeofday(struct timeval *t);

#endif