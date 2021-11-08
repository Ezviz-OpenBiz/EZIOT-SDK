/*******************************************************************************
*
*               COPYRIGHT (c) 2015-2016 Broadlink Corporation
*                         All Rights Reserved
*
* The source code contained or described herein and all documents
* related to the source code ("Material") are owned by Broadlink
* Corporation or its licensors.  Title to the Material remains
* with Broadlink Corporation or its suppliers and licensors.
*
* The Material is protected by worldwide copyright and trade secret
* laws and treaty provisions. No part of the Material may be used,
* copied, reproduced, modified, published, uploaded, posted, transmitted,
* distributed, or disclosed in any way except in accordance with the
* applicable license agreement.
*
* No license under any patent, copyright, trade secret or other
* intellectual property right is granted to or conferred upon you by
* disclosure or delivery of the Materials, either expressly, by
* implication, inducement, estoppel, except in accordance with the
* applicable license agreement.
*
* Unless otherwise agreed by Broadlink in writing, you may not remove or
* alter this notice or any other notice embedded in Materials by Broadlink
* or Broadlink's suppliers or licensors in any way.
*
*******************************************************************************/

#ifndef __DNA_OS_H
#define __DNA_OS_H

#include "dna_compiler.h"

#ifdef __cplusplus
    extern "C" {
#endif

/**
 *  @brief OS Timer handle definition
 */
typedef void * dna_timer_handle_t;

/**
 *  @brief OS Task (thread) handle definition
 */
typedef void * dna_task_handle_t;

/**
 *  @brief OS mutex-lock handle definition
 */
typedef void * dna_mutex_handle_t;

/**
 *  @brief OS semaphore handle definition
 */
typedef void * dna_semaphore_handle_t;

/**
 *  @brief OS reader-writer lock handle definition
 */
typedef void * dna_rwlock_handle_t;

/**
 *  @brief OS msg-queue handle definition
 */
typedef void * dna_queue_handle_t;

#define DNA_GMT0                      0     /* UTC */
#define DNA_GMT8                      8     /* China */
#define DNA_GMT9					  9		/* Japan */

typedef struct dna_time { 
    unsigned short year;
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char weekday;
    unsigned char day;
    unsigned char month;
} dna_time_t;

#define DNA_WAIT_FOREVER              (~0UL)
#define DNA_NO_WAIT                   0

#define DNA_RTOS_PRIO_0               0     /* Low */
#define DNA_RTOS_PRIO_1               1
#define DNA_RTOS_PRIO_2               2
#define DNA_RTOS_PRIO_3               3
#define DNA_RTOS_PRIO_4               4     /* High */

/* Reboot reason code */
enum {
    DNA_REBOOT_REASON_HARDWARE = 0, /* PWR or BOR */
    DNA_REBOOT_REASON_WATCHDOG_TIMEOUT,
    DNA_REBOOT_REASON_WATCHDOG_RESET,
    DNA_REBOOT_REASON_SOFTWARE,
};

typedef void (* dna_soft_reboot_cb_t)(void);

enum{
	DNA_REBOOT_REASON_SOFTWARE_NORMAL,
	DNA_REBOOT_REASON_SOFTWARE_TASK_DIE,
	DNA_REBOOT_REASON_SOFTWARE_LONG_OFFLINE,
};

enum{
	DNA_BACKUP_REG0,
	DNA_BACKUP_REG1,
	DNA_BACKUP_REG2,
	DNA_BACKUP_REG3,
};

/** Create timer
 *
 * This function creates a timer.
 *
 * @param[in] name Name of the timer (optional)
 * @param[in] ticks Period in millisecond
 * @param[in] callback Timer expire callback function
 * @param[in] data Timer callback data
 *
 * @return Handle to the timer, NULL on error.
 *
 * @note: if timer create success, then it auto start.
 */
dna_timer_handle_t dna_timer_create(
    const char * name,
    unsigned long ms,
    void (* callback)(void * data),
    void * data);

/** Activate timer
 *
 * This function activates (or starts) a timer that was previously created using
 * dna_timer_create(). If the timer had already started and was already in the
 * active state, then this call is equivalent to timer reset.
 *
 * @param[in] handle Pointer to a timer handle
 *
 * @return DNA_SUCCESS if timer activated successfully
 * @return -DNA_FAIL if timer fails to activate
 */
int dna_timer_activate(dna_timer_handle_t handle);

/** Deactivate timer
 *
 * This function deactivates (or stops) a timer that was previously started.
 *
 * @param [in] handle populated by dna_timer_create()
 *
 * @return DNA_SUCCESS on success
 * @return -DNA_FAIL on failure
 */
int dna_timer_deactivate(dna_timer_handle_t handle);

/** Delete timer
 *
 * This function deletes a timer.
 *
 * @param[in] handle Pointer to a timer handle
 *
 * @return DNA_SUCCESS on success
 * @return -DNA_FAIL on failure
 */
int dna_timer_delete(dna_timer_handle_t handle);

/** Create new task
 *
 * This function starts a new task (thread).  The new task starts execution by
 * invoking threadfn(). The parameter arg is passed as the sole argument of
 * threadfn().
 *
 * After finishing execution, the new task should call:
 * - dna_task_delete() to delete itself
 *
 * Failing to do this and just returning from threadfn() will result in
 * undefined behavior.
 *
 *
 * @param[in] name Name of the new thread. A copy of this string will be
 * made by the OS for itself. The maximum name length is defined by the
 * macro configMAX_TASK_NAME_LEN in FreeRTOS header file . Any name length
 * above it will be truncated.
 * @param[in] size stack size (bytes, at least 1024KiB)
 * @param[in] priority The priority of the new task. One value among
 * DNA_RTOS_PRIO_0, DNA_RTOS_PRIO_1, DNA_RTOS_PRIO_2, DNA_RTOS_PRIO_3 and DNA_RTOS_PRIO_4 should be
 * passed. DNA_RTOS_PRIO_4 represents the highest priority and DNA_RTOS_PRIO_0
 * represents the lowest priority.
 * @param[in] threadfn Function pointer to new task function
 * @param[in] data The sole argument passed to threadfn()
 *
 * @return Handle to the task, NULL on error.
 */
dna_task_handle_t dna_task_create(
    const char * name,
    int size,
    int priority,
    void (* threadfn)(void * data),
    void * data);

/** Terminate a task
 *
 * This function deletes a task. The task being deleted will be removed from
 * all ready, blocked, suspended and event lists.
 *
 * @param[in] handle Pointer to the task handle of the task to be
 * deleted. If self deletion is required NULL should be passed.
 *
 * @return DNA_SUCCESS if operation success
 * @return -DNA_FAIL if operation fails
 */
int dna_task_delete(dna_task_handle_t handle);

/** Get current task handle
 *
 * @return Handle to current task, NULL on error.
 */
dna_task_handle_t dna_get_current_task_handle(void);

/** Sleep for specified number of millisecond
 *
 * Any other thread can wake up this task specifically using the API
 * dna_task_wakeup()
 *
 * @param[in]  millisecond number of sleep
 */
void dna_task_sleep(unsigned long ms);

/**
 * Wake up a task from sleep.
 *
 * @param[in] handle Pointer to a handle for the task.
 */
void dna_task_wakeup(dna_task_handle_t handle);

/** Setup idle function
 *
 * This function sets up a callback function which will be called whenever the
 * system enters the idle thread context.
 *
 *  @param[in] func The callback function
 *
 *  @return DNA_SUCCESS on success
 *  @return -DNA_FAIL on error
 */
int dna_setup_idle_func(void (*func) (void));

/** Remove idle function
 *
 *  This function removes an idle callback function that was registered
 *  previously using dna_setup_idle_function().
 *
 *  @param[in] func The callback function
 *
 *  @return DNA_SUCCESS on success
 *  @return -DNA_FAIL on error
 */
int dna_remove_idle_func(void (*func)(void));

/** Setup tick function
 *
 * This function sets up a callback function which will be called whenever the
 * system enters the tick handler context.
 *
 *  @param[in] func The callback function
 *
 *  @return DNA_SUCCESS on success
 *  @return -DNA_FAIL on error
 */
int dna_setup_tick_func(void (*func) (void));

/** Remove tick function
 *
 *  This function removes an tick callback function that was registered
 *  previously using dna_setup_tick_function().
 *
 *  @param[in] func The callback function
 *
 *  @return DNA_SUCCESS on success
 *  @return -DNA_FAIL on error
 */
int dna_remove_tick_func(void (*func)(void));

/** Create mutex-lock
 *
 * This function creates a mutex.
 *
 * @param [in] name Name of the mutex (optional)
 *
 * @return Handle to the mutex-lock, NULL on error.
 */
dna_mutex_handle_t dna_mutex_create(const char * name);

/** Acquire mutex
 *
 * This function acquires a mutex. Only one thread can acquire a mutex at any
 * given time. If already acquired the callers will be blocked for the specified
 * time duration.
 *
 * @param[in] handle Pointer to mutex handle
 * @param[in] wait The maximum amount of time (millisecond), if set 0, then noblocking
 *
 * @return DNA_SUCCESS when mutex is acquired
 * @return -DNA_E_INVAL if invalid parameters are passed
 * @return -DNA_FAIL on failure
 */
int dna_mutex_get(dna_mutex_handle_t handle, unsigned long wait);

/** Release mutex
 *
 * This function releases a mutex previously acquired using dna_mutex_get().
 *
 * @note The mutex should be released from the same thread context from which it
 * was acquired.
 *
 * @param[in] handle Pointer to the mutex handle
 *
 * @return DNA_SUCCESS when mutex is released
 * @return -DNA_E_INVAL if invalid parameters are passed
 * @return -DNA_FAIL on failure
 */
int dna_mutex_put(dna_mutex_handle_t handle);

/** Delete mutex
 *
 * This function deletes a mutex.
 *
 * @param[in] handle Pointer to the mutex handle
 *
 * @note A mutex should not be deleted if other tasks are blocked on it.
 *
 * @return DNA_SUCCESS on success
 */
int dna_mutex_delete(dna_mutex_handle_t handle);

/** Create binary semaphore
 *
 * This function creates a binary semaphore. A binary semaphore can be acquired
 * by only one entity at a given time.
 *
 * @param[out] mhandle Pointer to a semaphore handle
 * @param[in] name Name of the semaphore
 * @param[in] maxcount The maximum count value that can be reached. When
 * the semaphore reaches this value it can no longer be 'put'
 * @param[in] initcount The count value assigned to the semaphore when it
 * is created. For e.g. If '0' is passed, then dna_semaphore_get() will
 * block until some other thread does an dna_semaphore_put().
 *
 * @return DNA_SUCCESS on success
 * @return -DNA_FAIL on error
 */
dna_semaphore_handle_t dna_semaphore_create(const char * name, unsigned long maxcount, unsigned long initcount);

/** Acquire semaphore
 *
 * This function acquires a semaphore. At a given time, a binary semaphore can
 * be acquired only once, while a counting semaphore can be acquired as many as
 * 'count' number of times. Once this condition is reached, the other callers of
 * this function will be blocked for the specified time duration.
 *
 * @param[in] handle Pointer to semaphore handle
 * @param[in] wait The maximum amount of time (millisecond), if set 0, then noblocking
 *
 * @return DNA_SUCCESS when semaphore is acquired
 * @return -DNA_E_INVAL if invalid parameters are passed
 * @return -DNA_FAIL on failure
 */
int dna_semaphore_get(dna_semaphore_handle_t handle, unsigned long wait);

/** Release semaphore
 *
 * This function releases a semaphore previously acquired using
 * os_semaphore_get().
 *
 * @note This function can also be called from interrupt-context.
 *
 * @param[in] mhandle Pointer to a semaphore handle
 *
 * @return DNA_SUCCESS when semaphore is released
 * @return -DNA_E_INVAL if invalid parameters are passed
 * @return -DNA_FAIL on failure
 */
int dna_semaphore_put(dna_semaphore_handle_t handle);

/** Delete a semaphore
 *
 * This function deletes the semaphore.
 *
 * @param [in] mhandle Pointer to a semaphore handle
 *
 * @note Do not delete a semaphore that has tasks blocked on it (tasks that are
 * in the Blocked state waiting for the semaphore to become available)
 *
 * @return DNA_SUCCESS on success
 */
int dna_semaphore_delete(dna_semaphore_handle_t handle);

/** Create reader-writer lock
 *
 * This function creates a reader-writer lock.
 *
 * @param[in] mutex_name Name of the mutex
 * @param[in] lock_name Name of the lock
 *
 * @return Handle to the reader-writer lock, NULL on error.
 */
dna_rwlock_handle_t dna_rwlock_create(
    const char * mutex_name,
    const char * lock_name);

/** Acquire writer lock
 *
 * This function acquires a writer lock. While readers can acquire the lock on a
 * sharing basis, writers acquire the lock in an exclusive manner.
 *
 * @param[in] handle Pointer to the reader-writer lock handle
 * @param[in] wait The maximum amount of time (millisecond), if set 0, then noblocking
 *
 * @return  DNA_SUCCESS on success
 * @return  -DNA_FAIL on error
 *
 */
int dna_rwlock_write_lock(dna_rwlock_handle_t handle, unsigned long wait);

/** Release writer lock
 *
 * This function releases a writer lock previously acquired using
 * dna_rwlock_write_lock().
 *
 * @param[in] handle Pointer to the reader-writer lock handle
 *
 * @return  DNA_SUCCESS on success
 * @return  -DNA_FAIL on error
 */
int dna_rwlock_write_unlock(dna_rwlock_handle_t handle);

/** Acquire reader lock
 *
 * This function acquires a reader lock. While readers can acquire the lock on a
 * sharing basis, writers acquire the lock in an exclusive manner.
 *
 * @param[in] handle Pointer to the reader-writer lock handle
 * @param[in] wait The maximum amount of time (millisecond), if set 0, then noblocking
 *
 * @return  DNA_SUCCESS on success
 * @return  -DNA_FAIL on error
 *
 */
int dna_rwlock_read_lock(dna_rwlock_handle_t handle, unsigned long wait);

/** Release reader lock
 *
 * This function releases a reader lock previously acquired using
 * dna_rwlock_read_lock().
 *
 * @param[in] handle Pointer to the reader-writer lock handle
 *
 * @return  DNA_SUCCESS on success
 * @return  -DNA_FAIL on error
 */
int dna_rwlock_read_unlock(dna_rwlock_handle_t handle);

/** Delete a reader-write lock
 *
 * This function deletes a reader-writer lock.
 *
 * @param[in] handle Pointer to the reader-writer lock handle
 *
 * @return DNA_SUCCESS on success
 */
int dna_rwlock_delete(dna_rwlock_handle_t handle);

/** Create an OS msg-queue
 *
 * This function creates a new queue instance. This allocates the storage
 * required by the new queue and returns a handle for the queue.
 *
 * @param[in] name String specifying the name of the queue (optional)
 * @param[in] msgsize The number of bytes each item in the queue will
 * require. Items are queued by copy, not by reference, so this is the
 * number of bytes that will be copied for each posted item. Each item on
 * the queue must be the same size.
 * @param[in] msgcnt item count.
 *
 * @return Handle to the mutex-lock, NULL on error.
 */
dna_queue_handle_t dna_queue_create(
    const char * name,
    unsigned int msgsize,
    unsigned int msgcnt);

/** Post an item to the back of the queue.
 *
 * This function posts an item to the back of a queue. The item is queued by
 * copy, not by reference. This function can also be called from an interrupt
 * service routine.
 *
 * @param[in] handle Pointer to the handle of the queue
 * @param[in] msg A pointer to the item that is to be placed on the
 * queue. The size of the items the queue will hold was defined when the
 * queue was created, so this many bytes will be copied from msg
 * into the queue storage area.
 * @param[in] wait The maximum amount of time (millisecond), if set 0, then noblocking
 *
 * @return DNA_SUCCESS if send operation was successful
 * @return -DNA_E_INVAL if invalid parameters are passed
 * @return -DNA_FAIL if send operation failed
 */
int dna_queue_write(
    dna_queue_handle_t handle,
    const void * msg,
    unsigned long wait);

/** Receive an item from queue
 *
 * This function receives an item from a queue. The item is received by copy so
 * a buffer of adequate size must be provided. The number of bytes copied into
 * the buffer was defined when the queue was created.
 *
 * \note This function must not be used in an interrupt service routine.
 *
 * @param[in] handle Pointer to handle of the queue
 * @param[out] msg Pointer to the buffer into which the received item will
 * be copied. The size of the items in the queue was defined when the queue was
 * created. This pointer should point to a buffer as many bytes in size.
 * @param[in] wait The maximum amount of time (millisecond), if set 0, then noblocking
 *
 * @return DNA_SUCCESS if receive operation was successful
 * @return -DNA_E_INVAL if invalid parameters are passed
 * @return -DNA_FAIL if receive operation failed
 */
int dna_queue_read(
    dna_queue_handle_t handle,
    void * msg,
    unsigned long wait);

/** Delete msg-queue
 *
 * This function deletes a queue. It frees all the memory allocated for storing
 * of items placed on the queue.
 *
 * @param[in] handle Pointer to handle of the queue to be deleted.
 *
 * @return Currently always returns DNA_SUCCESS
 */
int dna_queue_delete(dna_queue_handle_t handle);

/** System reboot
 *
 * This function used for reboot system (such as soft reset).
 *
 */
void dna_system_reboot(void);

/** Software reboot (Chip manufacturer implemented)
 *
 * This function used for reboot software, but keep the registers status.
 * Flow: ROM -> BOOT -> Main
 */
void dna_system_soft_reboot(void);

/** Software reboot (Broadlink custom implemented)
 *
 * This function used for reboot software, but keep the registers status.
 * Flow: set PC to Reset_Handler
 */
void dna_soft_reboot(void);

/** Reboot reason.
 *
 * This function used for get reboot reason (whether is hardware reboot or software reboot).
 *
 */
int dna_reboot_reason(void);

/** Software reboot callback set.
 *
 * This function used for set callback when software reboot.
 *
 */

int dna_soft_reboot_callback_set(void (*func) (void));

/** System ticks
 *
 * This function used for get system ticks (millisecond, From bootup to now).
 *
 * @return number of milliseconds
 */
unsigned long dna_system_ticks(void) DNA_COMPILER_SECTION_SRAM;

/** UTC timestamp
 *
 * This function used for get UTC timestamp (second).
 *
 * @return number of seconds
 */
unsigned long dna_utc_timestamp(void);

/** System timestamp
 *
 * This function used for get system timestamp (second, From bootup to now).
 *
 * @return number of seconds
 */
unsigned long dna_bootup_timestamp(void);

unsigned long dna_bootup_elapsed_ms(void);

unsigned long dna_bootup_elapsed_us(void);

/*
 *  @brief: Convert time format to timestamp (UTC)
 *
 *  This function used for convert time format to unix timestamp
 *
 *  @param[in]  time DNA standard time format
 *  @return     unix timestamp
 */
unsigned long dna_mktime(dna_time_t time);

/** Covert timestamp (UTC) to time format.
 *
 *  This function used for convert timestamp to time format.
 *
 *  @param[in] timstamp UTC timestamp
 *  @param[out] time DNA standard time format
 *  @return error code (dna_errno.h)
 */
int dna_gmtime(unsigned long timestamp, dna_time_t * time);

/** Set system time
 *
 * This function used for set system time.
 *
 * @param[in] time input time information.
 * @param[in] timezone input time timezone information
 * @return error code (dna_errno.h)
 */
int dna_set_time(dna_time_t * time, int timezone);

/** Get system time
 *
 * This function used for get system time.
 *
 * @param[out] time output time information.
 * @param[in] timezone output time timezone information
 * @return error code (dna_errno.h)
 */
int dna_get_time(dna_time_t * time, int timezone);

void * dna_malloc(unsigned int size);

void * dna_calloc(unsigned int size);

void * dna_realloc(void * ptr, unsigned int size);

void dna_free(void * ptr);

void dna_os_enter_critical_section(void);

void dna_os_exit_critical_section(void);

void dna_srand(unsigned int seeds);

int dna_rand(void);

void dna_delay_ms(unsigned long ms);

void dna_delay_us(unsigned long us);

/* Task healthmon */
int dna_task_monitor(
        dna_task_handle_t handle,
        unsigned int check_interval, unsigned char consecutive_failures,
        int (* is_die)(unsigned long cur_msec));


int dna_task_stack_high_watermark(dna_task_handle_t handle);

int dna_task_free_stack(dna_task_handle_t handle);


#ifdef __cplusplus
}
#endif

#endif

